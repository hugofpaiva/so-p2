# Sistemas Operativos - Trabalho 2

## Problema genérico de gestão de recursos: Fumadores

Este trabalho prático foi desenvolvido com o objetivo de compreender os mecanismos associados à execução de processos e *threads*. 

Para empreender este propósito, foi pedido que se solucionasse um problema que envolve vários fumadores com necessidades distintas para fumar. Dito isto, implementou-se um programa em *C* que simula e soluciona o problema recorrendo a semáforos e a memória partilhada, de modo a sincronizar os vários processos independentes.

##  Preparação
Estas instruções ajudarão a executar os programas desenvolvidos.

### Requisitos
Para compilar os programas, é necessário ter instalado um compilador de *C* na sua máquina, como por exemplo *gcc*. 
As próximas instruções devem ser executadas na raiz do repositório.

### Compilar
Os seguintes comandos compilam o programa com a solução desenvolvida ou com a solução do Professor (esta última necessita de ser executada em sistemas Linux):

**Solução desenvolvida:**
```
cd src 
make all
```

**Solução do Professor:**
```
cd src 
make all_bin
```

### Executar
Ambas as soluções, após a compilação desejada, são executadas da seguinte maneira:
```
cd run
./probSemSharedMemSmokers
```

## Detalhes

É possível encontrar todos os detalhes, nomeadamente os resultados, no [Relatório do Trabalho](/relatorio/SO_Report_2.pdf).

## Autores

 - **Hugo Paiva de Almeida - 93195**
 - **Carolina Araújo - 93248**
