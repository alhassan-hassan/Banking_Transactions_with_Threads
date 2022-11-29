
#include <string.h>
#include <pthread.h>  // POSIX thread
#include <stdio.h>    // C I/O library
#include <stdlib.h>   // C standard library
#include <sys/wait.h> // declarations for waiting
#include <unistd.h>   // UNIX standard symbolic constants and types
#include <stdbool.h>

// creating a lock variable: mutex
pthread_mutex_t mutex;

/**
 * An account is a structure that represents a bank account.
 * @property {int} balance - The amount of money in the account.
 */
typedef struct __account
{
    int balance;
} account;

/**
 * The type of transaction.
 */
typedef enum __transaction_t
{
    Withdraw,
    Deposit,
    Unknown
} transaction_t;

// Contains all the transactors
typedef enum __user {
    Wife,
    Husband
} user;


/**
 * An account is a structure that represents a bank account.
 * @property {int} balance - The amount of money in the account.
 */
typedef struct __transaction
{
    transaction_t transactionType;
    int transactionAmount;
} transaction;

// Keeps track of the list of transactions
typedef struct __transactionQueue
{
    int queueSize;
    transaction *transactions;
} transactionQueue;

/**
 * ExecuteTransactionsStruct is a struct that contains a pointer to an account and a pointer to a
 * transactionQueue.
 * @property {account} transactingAccount - The account that is currently being transacted.
 * @property {transactionQueue} currentTransactionQueue - A queue of transactions that are to be
 * executed.
 */
typedef struct __executeTransactionsStruct
{
    account *transactingAccount;
    transactionQueue *currentTransactionQueue;
    user transactor;

} executeTransactionsStruct;

// pre-definitions
void *parseTransactions(void *voidFileName, int argc);
transaction createTransaction(char *transactionRecord);
void *executeTransactions(void *voidExecuteTransactionsStruct);
void processTransaction(account *transactingAccount, transaction *currentTransaction, user transactor);
int deposit(void *currentAccount, int amount, user transactor);
int withdraw(void *currentAccount, int amount, user transactor);

// This function gets the length of nonempty lines in a file
int getTransactionsLength(char fileName[])
{

    FILE *ptr;
    ptr = fopen(fileName, "r");

    int length = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // loop through and track only nonempty lines
    while ((read = getline(&line, &len, ptr)) != -1)
    {
        if (strlen(line) > 1)
            length++;
    }

    return length;
}

/**
 * It reads a file, and returns a struct containing the number of transactions in the file, and an
 * array of transactions
 *
 * Args:
 *   voidFileName (void): THe name of the file.
 *
 * Returns:
 *   A void pointer to a transactionQueue struct.
 */
void *parseTransactions(void *fileName, int argc){  
    // if file does not follow the right format, exit
    if (argc != 4){
        printf("bank: amount file1 file2\n");
        exit(1);
    }

    FILE *ptr;
    char line[15];
    ptr = fopen(fileName, "r");
    int track = 0;
    if (ptr == NULL)
    {
        printf("bank: cannot open file\n");
        exit(1);
    }

    // create the queues that will store the transactions array
    int lenFile = getTransactionsLength(fileName);
    transactionQueue *queueTransactions = malloc(sizeof(transactionQueue));
    transaction *allTransactions = malloc(lenFile * sizeof(transaction));

    while (fgets(line, sizeof(line), ptr))
    {
        // do, only when the file begines with initials of transaction type
        if (line[0] == 'w' || line[0] == 'd')
        {
            // get the components of the transaction
            char *action = strtok(line, " ");
            char *amount = strtok(NULL, " ");

            // Appending to transactions queue of every transaction
            if (atoi(amount) > 0) {
                if (strcasecmp(action, "deposit") == 0)
                {
                    allTransactions[track].transactionType = Deposit;
                }
                else if (strcasecmp(action, "withdraw") == 0)
                {
                    allTransactions[track].transactionType = Withdraw;
                }
                else
                {
                    allTransactions[track].transactionType = Unknown;
                }

                
                    allTransactions[track].transactionAmount = atoi(amount);
            }

            track++;
        }
    }

    //  Updating the values of the struct components of the queue
    queueTransactions->transactions = allTransactions;
    queueTransactions->queueSize = lenFile;

    fclose(ptr);
    return (void *)queueTransactions;
}

/**
 * It withdraws money from the provided account
 *
 * Args:
 *   account (void *): The account to be used
 *   amount  (int): the amount to be withdrawn
 */
int withdraw(void *currentAccount, int amount, user transactor){
    // This is a critical section, apply locks
    pthread_mutex_lock(&mutex);
    account *curAccount = (account *)currentAccount;
    int whatWillBeLeft = curAccount->balance - amount;

    // Only withdraw if resulting balance is positive
    if (whatWillBeLeft >= 0){
        curAccount->balance = whatWillBeLeft;

        // Determining the user
        if (transactor == 0) {
            
            printf("Withdraw: %d, User: Wife, Account balance after: %d\n", amount, whatWillBeLeft);
        } else {
            printf("Withdraw: %d, User: Husband, Account balance after: %d\n", amount, whatWillBeLeft);
        }
    }
    else {
            if (transactor == 0) {
                printf("Withdraw: %d, User: Wife, Transaction declined\n", amount);
            } else {
                
                printf("Withdraw: %d, User: Husband, Transaction decline\n", amount);
            }
        }
    // Free the locks
    pthread_mutex_unlock(&mutex);

    return 0;
}

/**
 * Deposit an amount into provided account
 *
 * Args:
 *   account (void *): The account to be used
 *   amount  (int): The amount to be deposited
 */
int deposit(void *currentAccount, int amount, user transactor)
{
    // Apply locks
    pthread_mutex_lock(&mutex);
    account *curAccount = (account *)currentAccount;

    // Update the balance with the deposit
    curAccount->balance += amount;
    if (transactor == 0) {
        printf("Deposit: %d, User: Wife, Account balance after: %d\n", amount, curAccount->balance);
    } else {
        printf("Deposit: %d, User: Husband, Account balance after: %d\n", amount, curAccount->balance);
    }
    pthread_mutex_unlock(&mutex);

    return 0;
}

/**
 * "This function takes a pointer to an account and a pointer to a transaction,
 * and then processes the transaction on the account."
 *
 * Args:
 *   transactingAccount (account): This is a pointer to account that is being transacted on.
 *   currentTransaction (transaction): a pointer to a transaction struct
 */
void processTransaction(account *transactingAccount, transaction *currentTransaction, user transactor)
{
    switch (currentTransaction->transactionType)
    {
    case Deposit:
        deposit(transactingAccount, currentTransaction->transactionAmount, transactor);
        break;
    case Withdraw:
        withdraw(transactingAccount, currentTransaction->transactionAmount, transactor);
        break;
    default:
        break;
    }
}

/**
 * Process a queue of transactions
 *
 * Args:
 *   voidExecuteTransactionsStruct (void): a void pointer to the executeTransactionsStruct struct
 */
void *executeTransactions(void *voidExecuteTransactionsStruct){
    executeTransactionsStruct *allActivities = (executeTransactionsStruct *) voidExecuteTransactionsStruct;

    int queueIndx = 0;

    // Get eack queue and process it.
    while (queueIndx < allActivities->currentTransactionQueue->queueSize)
    {
        processTransaction(allActivities->transactingAccount, &(allActivities->currentTransactionQueue->transactions[queueIndx]), allActivities->transactor);
        queueIndx++;
    }
    return (void *) 0;
}

// The main function which aids in running the code.
int main(int argc, char *argv[]){
    if (argc != 4){
        printf("bank: amount file1 file2\n");
        return 1;
    }

    account *coupleAccount = malloc(sizeof(account) * 8);
    coupleAccount->balance = atoi(argv[1]);

    pthread_t hthread; pthread_t wthread;

    // Placeholders for the files passed.
    char *husband;
    char *wife;

    // getting the respective files
    husband = argv[2];
    wife = argv[3];

    // Declaring queues for both transactors
    transactionQueue *transaction_husband = malloc(sizeof(transactionQueue));
    transactionQueue *transaction_wife = malloc(sizeof(transactionQueue));

    // Setting the initial transactions to the first transaction of each file
    transaction_husband = (transactionQueue *) parseTransactions(husband, argc);
    transaction_husband->transactions->transactionAmount = transaction_husband->transactions->transactionAmount;

    transaction_wife = (transactionQueue *) parseTransactions(wife, argc);
    transaction_wife->transactions->transactionAmount = transaction_wife->transactions->transactionAmount;

    // Creatting a pointer to the structs that contain all the transactions of each user
    executeTransactionsStruct *husband_struct = malloc(sizeof(executeTransactions));
    executeTransactionsStruct *wife_struct = malloc(sizeof(executeTransactions));

    //  Setting the values of the components of the struct
    husband_struct->transactingAccount = coupleAccount;
    wife_struct->transactingAccount = coupleAccount;

    husband_struct->currentTransactionQueue = transaction_husband;
    wife_struct->currentTransactionQueue = transaction_wife;

    //  Printing the the output, using threads
    printf("Opening balance: %d\n", coupleAccount->balance);
    pthread_create(&hthread, NULL, executeTransactions, husband_struct);
    pthread_create(&wthread, NULL, executeTransactions, wife_struct);

    pthread_join(hthread, NULL);
    pthread_join(wthread, NULL);

    printf("Closing balance: %d\n", coupleAccount->balance);

    return 0;
}