#include "slog.h"

void handler(slog_t* log, void* args) {
    char time[20] = {0};
    strftime(time, 20, "%F %T", gmtime(&log->time));
    char* level = slog_level_string(log->level);
    printf("[%s] [%s] [%s at line %i (%s)] %s\n", time, level, log->file, log->line, log->func, log->message);
}

int main() {
    slog_handlers_register(handler, NULL, NULL);
    slog_init();

    SLOG_RAW(SLOG_INFO, NULL, "Starting...");
    for (int i = 0; i < 4; i++) {
        SLOG(SLOG_INFO, NULL, "%i", i);
    }
    SLOG_RAW(SLOG_INFO, NULL, "Done.");

    slog_uninit();
    return 0;
}
