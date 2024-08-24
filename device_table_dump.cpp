#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

typedef uint32_t audio_devices_t;

#include "platform.h"
#include "platform_c.h"

#define AUDIO_HAL_LIB "/tmp/audio.primary.atoll.so"
#define GET_SND_DEVICE_SYM "platform_get_snd_device_name"
#define GET_SND_DEVICE_IDX_SYM "platform_get_snd_device_index"
#define GET_SND_DEVICE_ACDB_ID_SYM "platform_get_snd_device_acdb_id"

typedef int snd_device_t;
typedef const char* (*platform_get_snd_device_name_t)(snd_device_t);
typedef int (*platform_get_snd_device_index_t)(char*);
typedef int (*platform_get_snd_device_acdb_id_t)(snd_device_t);

#define LOAD_SYM(handle, type, var, sym) \
    type var = (type) dlsym(handle, sym); \
    if (!var) { \
        printf("Failed to load the symbol %s (%s)\n", sym, dlerror()); \
        exit(1); \
    }

int main() {
    void *handle;

    printf("Loading %s\n", AUDIO_HAL_LIB);
    handle = dlopen(AUDIO_HAL_LIB, RTLD_LOCAL | RTLD_LAZY);
    if (!handle) {
        printf("Failed to open the library %s (%s)\n", AUDIO_HAL_LIB, dlerror());
        return 1;
    }

    LOAD_SYM(handle, platform_get_snd_device_name_t, platform_get_snd_device_name, GET_SND_DEVICE_SYM);
    LOAD_SYM(handle, platform_get_snd_device_index_t, platform_get_snd_device_index, GET_SND_DEVICE_IDX_SYM);
    LOAD_SYM(handle, platform_get_snd_device_acdb_id_t, platform_get_snd_device_acdb_id, GET_SND_DEVICE_ACDB_ID_SYM);

    for (const char* snd_device_name_const : SND_DEVICES_NAMES_SEC) {
        char *snd_device_name = strdup(snd_device_name_const);
        int snd_device = platform_get_snd_device_index(snd_device_name);
        const char* backend = platform_get_snd_device_name(snd_device);
        int acdb_id = platform_get_snd_device_acdb_id(snd_device);

        if (!strcmp(backend, "dummy")) continue;

        printf("[%s] \"%s\" (%d)\n", snd_device_name, backend, acdb_id);
        free(snd_device_name);
    }

    dlclose(handle);
    return 0;
}
