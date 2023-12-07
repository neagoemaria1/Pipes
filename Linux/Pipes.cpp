#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// functie destinata verificarii de numere prime
bool isPrime(int num) {
    if (num < 2) {
        return false;
    }
    for (int i = 2; i * i <= num; ++i) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}
// functie destinata pentru cautarea numerelor prime dintr-un anumit interval  si scrierea lor intr-un pipe
void findPrimes(int start, int end, int writePipe) {
    for (int num = start; num <= end; ++num) {
        if (isPrime(num)) {
            write(writePipe, &num, sizeof(num));
        }
    }
    close(writePipe);
}

int main() {
    // am definit nr de procese si lungimea intervalului pentru fiecare proces (10 * 1000 = 10 000 de nr)
    const int numProcesses = 10;
    const int rangeSize = 1000;

    int pipes[numProcesses][2];
    // pt fiecare proces avem si un pipe, care ne ajuta sa comunicam cu procesul parinte
    for (int i = 0; i < numProcesses; ++i) { 
        pipe(pipes[i]);
    //folosim fork pentru a crea procese de tip child, care vor comunica procesului parent numerele prime printr-un pipe
        pid_t pid = fork();

        if (pid == 0) {
            //Inchidem capatul neutilizat pentru citire al pipe-ului
            close(pipes[i][0]);  
            // am definit intervalul de numere alocate in functie de nr procesului
            int start = i * rangeSize + 1;
            int end = (i + 1) * rangeSize;
            // cautam numerele prime
            findPrimes(start, end, pipes[i][1]);
            //inchidem capatul de scriere inainte de iesirea din procesul copil
            close(pipes[i][1]);  
            exit(0);
        } else if (pid < 0) {
            //testare forkS
            std::cerr << "Error creating child process.\n";
            return 1;
        }
    }

    //Inchidem capatul de scriere al pipe-urilor în procesul parinte 
    for (int i = 0; i < numProcesses; ++i) {
        close(pipes[i][1]); 
    }

    std::cout << "Prime numbers:\n";
    // citim din pipe-ul creat de fiecare process, numerele prime si le afisam
    for (int i = 0; i < numProcesses; ++i) {
        int prime;
        while (read(pipes[i][0], &prime, sizeof(prime)) > 0) {
            std::cout << prime << " ";
        }
        close(pipes[i][0]);
    }

    // asteptam sa se incheie executia tuturor procesolor child
    for (int i = 0; i < numProcesses; ++i) {
        wait(NULL);
    }

    std::cout << "\n";
    return 0;
}
