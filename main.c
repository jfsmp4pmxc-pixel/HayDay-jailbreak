#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>

void (*orig_exit)(int status);
const char *MY_BYPASS_DYLIB = "libBlueBypass.dylib"; 

bool has_unauthorized_frameworks() {
    uint32_t count = _dyld_image_count();
    
    for (uint32_t i = 0; i < count; i++) {
        const char *name = _dyld_get_image_name(i);
        if (!name) continue;

        if (strstr(name, "/System/Library/") || strstr(name, "/usr/lib/")) {
            continue;
        }
        
        if (strstr(name, "HayDay.app") || strstr(name, "hayday")) {
            continue;
        }

        if (strstr(name, MY_BYPASS_DYLIB)) {
            continue;
        }

        return true; 
    }
    
    return false;
}

void fake_exit(int status) {
    if (has_unauthorized_frameworks()) {
        if (orig_exit) orig_exit(status);
        else exit(status);
    } else {
        return; 
    }
}

typedef struct interpose_substitution {
    const void *replacement;
    const void *original;
} interpose_substitution_t;

__attribute__((used)) static const interpose_substitution_t interpositions[] \
__attribute__((section("__DATA,__interpose"))) = {
    { (const void *)(fake_exit), (const void *)(exit) }
};

__attribute__((constructor)) static void init() {
    orig_exit = dlsym(RTLD_NEXT, "exit");
}
