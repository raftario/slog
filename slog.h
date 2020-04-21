#ifndef __SLOG_H
#define __SLOG_H

/*
SLOG_SIZE - Maximum length of log messages
SLOG_PTHREAD - Use pthread mutexes to prevent race conditions
*/

#ifndef SLOG_SIZE
#define SLOG_SIZE 512
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef SLOG_PTHREAD
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Log level
typedef enum slog_level { SLOG_TRACE, SLOG_DEBUG, SLOG_INFO, SLOG_WARN, SLOG_ERROR, SLOG_FATAL } slog_level_t;

// Returns a string representation for the given log level
char* slog_level_string(slog_level_t level) {
    switch (level) {
    case SLOG_TRACE:
        return "TRACE";
    case SLOG_DEBUG:
        return "DEBUG";
    case SLOG_INFO:
        return "INFO";
    case SLOG_WARN:
        return "WARN";
    case SLOG_ERROR:
        return "ERROR";
    case SLOG_FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

// Log
typedef struct slog {
    // Level
    slog_level_t level;
    // Message
    char* message;

    // Timestamp
    time_t time;

    // File this log has been sent from
    char* file;
    // Function this log has been sent from
    char* func;
    // Line number this log has been sent from
    int line;
} slog_t;

// Log handler
typedef void (*slog_handler_t)(slog_t*, void*);

typedef void (*slog_init_t)();
typedef void (*slog_uninit_t)();

struct __slog_handler_node {
    slog_handler_t handler;

    slog_init_t init;
    slog_uninit_t uninit;

    struct __slog_handler_node* next;
};

static struct __slog_handler_node* __slog_handlers = NULL;

#ifdef SLOG_PTHREAD
static pthread_mutex_t __slog_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

// Initialises slog and log handlers
void slog_init() {
#ifdef SLOG_PTHREAD
    pthread_mutex_lock(&__slog_lock);
#endif

    struct __slog_handler_node* ptr = __slog_handlers;
    while (ptr != NULL) {
        if (ptr->init != NULL) {
            ptr->init();
        }
        ptr = ptr->next;
    }

#ifdef SLOG_PTHREAD
    pthread_mutex_unlock(&__slog_lock);
#endif
}

// Uninitialises log handlers and slog
void slog_uninit() {
#ifdef SLOG_PTHREAD
    pthread_mutex_lock(&__slog_lock);
#endif

    struct __slog_handler_node* current = __slog_handlers;
    struct __slog_handler_node* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

#ifdef SLOG_PTHREAD
    pthread_mutex_unlock(&__slog_lock);
#endif
}

// Adds a new log handler
void slog_handlers_register(slog_handler_t handler, slog_init_t init, slog_uninit_t uninit) {
    struct __slog_handler_node* node = (struct __slog_handler_node*)malloc(sizeof(struct __slog_handler_node));
    node->handler = handler;
    node->init = init;
    node->uninit = uninit;
    node->next = __slog_handlers;

#ifdef SLOG_PTHREAD
    pthread_mutex_lock(&__slog_lock);
#endif
    __slog_handlers = node;
#ifdef SLOG_PTHREAD
    pthread_mutex_unlock(&__slog_lock);
#endif
}

void __slog(slog_t log, void* args) {
#ifdef SLOG_PTHREAD
    pthread_mutex_lock(&__slog_lock);
#endif

    struct __slog_handler_node* ptr = __slog_handlers;
    while (ptr != NULL) {
        if (ptr->handler != NULL) {
            ptr->handler(&log, args);
        }
        ptr = ptr->next;
    }

#ifdef SLOG_PTHREAD
    pthread_mutex_unlock(&__slog_lock);
#endif
}

// Format and log a message with the specified level
#define SLOG(level, args, format, ...)                                                                                 \
    do {                                                                                                               \
        char message[SLOG_SIZE] = {0};                                                                                 \
        snprintf(message, SLOG_SIZE, format, __VA_ARGS__);                                                             \
        SLOG_RAW(level, args, message);                                                                                \
    } while (0)

// Log a message with the specified level
#define SLOG_RAW(level, args, message)                                                                                 \
    do {                                                                                                               \
        struct timespec ts;                                                                                            \
        slog_t log = {level, (char*)message, time(NULL), (char*)__FILE__, (char*)__PRETTY_FUNCTION__, __LINE__};       \
        __slog(log, args);                                                                                             \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
