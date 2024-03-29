/**
 *  \file semSharedMemAgent.c (implementation file)
 *
 *  \brief Problem name: Smokers
 *
 *  Synchronization based on semaphores and shared memory.
 *  Implementation with SVIPC.
 *
 *  Definition of the operations carried out by the agent:
 *     \li prepareIngredients
 *     \li waitForCigarette
 *     \li closeFactory
 *
 *  \author Nuno Lau - December 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#include "probConst.h"
#include "probDataStruct.h"
#include "logging.h"
#include "sharedDataSync.h"
#include "semaphore.h"
#include "sharedMemory.h"

/** \brief logging file name */
static char nFic[51];

/** \brief shared memory block access identifier */
static int shmid;

/** \brief semaphore set access identifier */
static int semgid;

/** \brief pointer to shared memory region */
static SHARED_DATA *sh;

static void prepareIngredients();
static void waitForCigarette();
static void closeFactory();

/**
 *  \brief Main program.
 *
 *  Its role is to generate the life cycle of one of intervening entities in the problem: the agent.
 */
int main(int argc, char *argv[])
{
    int key;    /*access key to shared memory and semaphore set */
    char *tinp; /* numerical parameters test flag */

    /* validation of command line parameters */

    if (argc != 4)
    {
        freopen("error_AG", "a", stderr);
        fprintf(stderr, "Number of parameters is incorrect!\n");
        return EXIT_FAILURE;
    }
    else
    {
        freopen(argv[3], "w", stderr);
        setbuf(stderr, NULL);
    }
    strcpy(nFic, argv[1]);
    key = (unsigned int)strtol(argv[2], &tinp, 0);
    if (*tinp != '\0')
    {
        fprintf(stderr, "Error on the access key communication!\n");
        return EXIT_FAILURE;
    }

    /* connection to the semaphore set and the shared memory region and mapping the shared region onto the
       process address space */
    if ((semgid = semConnect(key)) == -1)
    {
        perror("error on connecting to the semaphore set");
        return EXIT_FAILURE;
    }
    if ((shmid = shmemConnect(key)) == -1)
    {
        perror("error on connecting to the shared memory region");
        return EXIT_FAILURE;
    }
    if (shmemAttach(shmid, (void **)&sh) == -1)
    {
        perror("error on mapping the shared region on the process address space");
        return EXIT_FAILURE;
    }

    /* initialize random generator */
    srandom((unsigned int)getpid());

    /* simulation of the life cycle of the agent */

    int nOrders = 0;
    while (nOrders < sh->fSt.nOrders)
    {
        prepareIngredients();
        waitForCigarette();

        nOrders++;
    }

    closeFactory();

    /* unmapping the shared region off the process address space */

    if (shmemDettach(sh) == -1)
    {
        perror("error on unmapping the shared region off the process address space");
        return EXIT_FAILURE;
        ;
    }

    return EXIT_SUCCESS;
}

/**
 *  \brief agent prepares 2 ingredients
 *
 *  The agent updates state and randomly selects a pack of 2 different ingredients to be generated.
 *  The inventory is updated to new existences of ingredients.
 *  Both ingredients generated should be notified to watcher using different semaphores. 
 */
static void prepareIngredients()
{

    if (semDown(semgid, sh->mutex) == -1)
    { /* enter critical region */
        perror("error on the down operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }
    //Vou mudar o estado do agente para preparing porque vou começar a preparar os ingredientes
    sh->fSt.st.agentStat = PREPARING;

    int ing1 = rand() % 3;
    int ing2 = rand() % 3;

    while (ing1 == ing2)
    {

        ing2 = rand() % 3;
    }
    // guardar os ingredientes fabricados no inventário
    sh->fSt.ingredients[ing1]++;
    sh->fSt.ingredients[ing2]++;
    saveState(nFic, &sh->fSt);

    if (semUp(semgid, sh->mutex) == -1)
    { /* leave critical region */
        perror("error on the up operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }

    if (semUp(semgid, sh->ingredient[ing1]) == -1) //Ingrediente 1 passou a estar disponível. Vou notificar o watcher[ing1]
    {
        perror("error on the up operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }

    if (semUp(semgid, sh->ingredient[ing2]) == -1) //Ingrediente 2 passou a estar disponível. Vou notificar o watcher[ing2]
    {
        perror("error on the up operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }
}

/**
 *  \brief agent wait for smoker to complete cigarrete
 *
 *  The agent waits until the smoker completes the rolling of the cigarette. 
 *  The internal state should be updated.
 */
static void waitForCigarette()
{
    if (semDown(semgid, sh->mutex) == -1)
    { /* enter critical region */
        perror("error on the down operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }
    //mudar o estado
    sh->fSt.st.agentStat = WAITING_CIG;
    saveState(nFic, &sh->fSt);

    if (semUp(semgid, sh->mutex) == -1)
    { /* leave critical region */
        perror("error on the up operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }
    //O Agent vai ficar à espera que o smoker acabe de enrolar
    if (semDown(semgid, sh->waitCigarette) == -1)
    {
        perror("error on the down operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }
}

/**
 *  \brief agent closes factory of ingredients
 *
 *  The agent updates state and notifies watchers that the factory is closing. 
 */
static void closeFactory()
{
    if (semDown(semgid, sh->mutex) == -1)
    { /* enter critical region */
        perror("error on the down operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }
    //mudar o estado
    sh->fSt.st.agentStat = CLOSING_A;
    sh->fSt.closing = true;
    saveState(nFic, &sh->fSt);

    if (semUp(semgid, sh->mutex) == -1)
    { /* leave critical region */
        perror("error on the up operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }

    //Como os watchers estão à espera de ingredientes, ao notificar que os ingredientes estão disponíveis vou acordá-los de modo a que posteriormente eles também terminem
    
    if (semUp(semgid, sh->ingredient[0]) == -1)
    {
        perror("error on the up operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }

    if (semUp(semgid, sh->ingredient[1]) == -1)
    {
        perror("error on the up operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }

    if (semUp(semgid, sh->ingredient[2]) == -1)
    {
        perror("error on the up operation for semaphore access (AG)");
        exit(EXIT_FAILURE);
    }
}
