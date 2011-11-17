/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http: *www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Tor Lillqvist <tml@iki.fi> (initial developer)
 * Copyright (C) 2011 SUSE Linux http://suse.com (initial developer's employer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <jni.h>

#include <linux/elf.h>

#include <android/log.h>

#include "lo-bootstrap.h"

#include "android_native_app_glue.c"

#undef LOGI
#undef LOGW

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "lo-bootstrap", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "lo-bootstrap", __VA_ARGS__))

struct engine {
    int dummy;
};

static struct android_app *app;
static JNIEnv *jni_env;
static const char **library_locations;
static int (*lo_main)(int, const char **);
static int lo_main_argc;
static const char **lo_main_argv;

static void
engine_handle_cmd(struct android_app* app,
                  int32_t cmd)
{
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
    case APP_CMD_SAVE_STATE:
        break;
    case APP_CMD_INIT_WINDOW:
        break;
    case APP_CMD_TERM_WINDOW:
        break;
    case APP_CMD_GAINED_FOCUS:
        break;
    case APP_CMD_LOST_FOCUS:
        break;
    }
}

static char *
read_section(int fd,
             Elf32_Shdr *shdr)
{
    char *result;

    result = malloc(shdr->sh_size);
    if (lseek(fd, shdr->sh_offset, SEEK_SET) < 0) {
        close(fd);
        free(result);
        return NULL;
    }
    if (read(fd, result, shdr->sh_size) < shdr->sh_size) {
        close(fd);
        free(result);
        return NULL;
    }

    return result;
}

static void
free_ptrarray(void **pa)
{
    void **rover = pa;

    while (*rover != NULL)
        free(*rover++);

    free(pa);
}

jobjectArray
Java_org_libreoffice_android_Bootstrap_dlneeds(JNIEnv* env,
                                               jobject clazz,
                                               jstring library)
{
    char **needed;
    int n_needed;
    const jbyte *libName;
    jclass String;
    jobjectArray result;

    libName = (*env)->GetStringUTFChars(env, library, NULL);

    needed = lo_dlneeds(libName);

    (*env)->ReleaseStringUTFChars(env, library, libName);

    if (needed == NULL)
        return NULL;

    n_needed = 0;
    while (needed[n_needed] != NULL)
        n_needed++;

    /* Allocate return value */

    String = (*env)->FindClass(env, "java/lang/String");
    if (String == NULL) {
        LOGI("Could not find the String class");
        free_ptrarray((void **) needed);
        return NULL;
    }

    result = (*env)->NewObjectArray(env, n_needed, String, NULL);
    if (result == NULL) {
        LOGI("Could not create the String array");
        free_ptrarray((void **) needed);
        return NULL;
    }

    for (n_needed = 0; needed[n_needed] != NULL; n_needed++)
        (*env)->SetObjectArrayElement(env, result, n_needed, (*env)->NewStringUTF(env, needed[n_needed]));

    free_ptrarray((void **) needed);

    return result;
}

jint
Java_org_libreoffice_android_Bootstrap_dlopen(JNIEnv* env,
                                              jobject clazz,
                                              jstring library)
{
    const jbyte *libName = (*env)->GetStringUTFChars(env, library, NULL);
    void *p = lo_dlopen (libName);

    (*env)->ReleaseStringUTFChars(env, library, libName);

    return (jint) p;
}

jint
Java_org_libreoffice_android_Bootstrap_dlsym(JNIEnv* env,
                                             jobject clazz,
                                             jint handle,
                                             jstring symbol)
{
    const jbyte *symName = (*env)->GetStringUTFChars(env, symbol, NULL);
    void *p = lo_dlsym ((void *) handle, symName);

    (*env)->ReleaseStringUTFChars(env, symbol, symName);

    return (jint) p;
}

jint
Java_org_libreoffice_android_Bootstrap_dlcall(JNIEnv* env,
                                              jobject clazz,
                                              jint function,
                                              jobject argument)
{
    jclass StringArray = (*env)->FindClass(env, "[Ljava/lang/String;");

    if (StringArray == NULL) {
        LOGI("Could not find String[] class");
        return 0;
    }

    if ((*env)->IsInstanceOf(env, argument, StringArray)) {
        int argc = (*env)->GetArrayLength(env, argument);
        const char **argv = malloc(sizeof(char *) * (argc+1));
        int i, result;
        for (i = 0; i < argc; i++) {
            argv[i] = (*env)->GetStringUTFChars(env, (*env)->GetObjectArrayElement(env, argument, i), NULL);
            LOGI("argv[%d] = %s", i, argv[i]);
        }
        argv[argc] = NULL;

        result = lo_dlcall_argc_argv((void *) function, argc, argv);

        for (i = 0; i < argc; i++)
            (*env)->ReleaseStringUTFChars(env, (*env)->GetObjectArrayElement(env, argument, i), argv[i]);

        free(argv);
        return result;
    }

    return 0;
}

// public native boolean setup(String dataDir,
//                             String[] ld_library_path);

jboolean
Java_org_libreoffice_android_Bootstrap_setup__Ljava_lang_String_2_3Ljava_lang_String_2(JNIEnv* env,
                                                                                       jobject this,
                                                                                       jstring dataDir,
                                                                                       jobjectArray ld_library_path)
{
    int i, n;
    const jbyte *dataDirPath;
    char *lib_dir;

    n = (*env)->GetArrayLength(env, ld_library_path);

    library_locations = malloc((n+2) * sizeof(char *));

    dataDirPath = (*env)->GetStringUTFChars(env, dataDir, NULL);

    lib_dir = malloc(strlen(dataDirPath) + 5);
    strcpy(lib_dir, dataDirPath);
    strcat(lib_dir, "/lib");

    (*env)->ReleaseStringUTFChars(env, dataDir, dataDirPath);

    library_locations[0] = lib_dir;

    for (i = 0; i < n; i++) {
        const jbyte *s = (*env)->GetStringUTFChars(env, (*env)->GetObjectArrayElement(env, ld_library_path, i), NULL);
        library_locations[i+1] = strdup(s);
        (*env)->ReleaseStringUTFChars(env, (*env)->GetObjectArrayElement(env, ld_library_path, i), s);
    }

    library_locations[n+1] = NULL;

    for (n = 0; library_locations[n] != NULL; n++)
        LOGI("library_locations[%d] = %s", n, library_locations[n]);

    return JNI_TRUE;
}

// public native boolean setup(int lo_main_ptr,
//                             Object lo_main_argument);

jboolean
Java_org_libreoffice_android_Bootstrap_setup__ILjava_lang_Object_2(JNIEnv* env,
                                                                   jobject this,
                                                                   void *lo_main_ptr,
                                                                   jobject lo_main_argument)
{
    jclass StringArray;
    int i;

    lo_main = lo_main_ptr;

    StringArray = (*env)->FindClass(env, "[Ljava/lang/String;");
    if (StringArray == NULL) {
        LOGI("Could not find String[] class");
        return JNI_FALSE;
    }

    if (!(*env)->IsInstanceOf(env, lo_main_argument, StringArray)) {
        LOGI("lo_main_argument is not a String[]?");
        return JNI_FALSE;
    }

    lo_main_argc = (*env)->GetArrayLength(env, lo_main_argument);
    lo_main_argv = malloc(sizeof(char *) * (lo_main_argc+1));

    for (i = 0; i < lo_main_argc; i++) {
        const jbyte *s = (*env)->GetStringUTFChars(env, (*env)->GetObjectArrayElement(env, lo_main_argument, i), NULL);
        lo_main_argv[i] = strdup(s);
        (*env)->ReleaseStringUTFChars(env, (*env)->GetObjectArrayElement(env, lo_main_argument, i), s);
        LOGI("argv[%d] = %s", i, lo_main_argv[i]);
    }
    lo_main_argv[lo_main_argc] = NULL;

    return JNI_TRUE;
}

char **
lo_dlneeds(const char *library)
{
    int i, fd;
    int n_needed;
    char **result;
    char *shstrtab, *dynstr;
    Elf32_Ehdr hdr;
    Elf32_Shdr shdr;
    Elf32_Dyn dyn;

    /* Open library and read ELF header */

    fd = open(library, O_RDONLY);

    if (fd == -1) {
        LOGI("lo_dlneeds: Could not open library %s: %s", library, strerror(errno));
        return NULL;
    }

    if (read(fd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOGI("lo_dlneeds: Could not read ELF header of %s", library);
        close(fd);
        return NULL;
    }

    /* Read in .shstrtab */

    if (lseek(fd, hdr.e_shoff + hdr.e_shstrndx * sizeof(shdr), SEEK_SET) < 0) {
        LOGI("lo_dlneeds: Could not seek to .shstrtab section header of %s", library);
        close(fd);
        return NULL;
    }
    if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
        LOGI("lo_dlneeds: Could not read section header of %s", library);
        close(fd);
        return NULL;
    }

    shstrtab = read_section(fd, &shdr);
    if (shstrtab == NULL)
        return NULL;

    /* Read section headers, looking for .dynstr section */

    if (lseek(fd, hdr.e_shoff, SEEK_SET) < 0) {
        LOGI("lo_dlneeds: Could not seek to section headers of %s", library);
        close(fd);
        return NULL;
    }
    for (i = 0; i < hdr.e_shnum; i++) {
        if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
            LOGI("lo_dlneeds: Could not read section header of %s", library);
            close(fd);
            return NULL;
        }
        if (shdr.sh_type == SHT_STRTAB &&
            strcmp(shstrtab + shdr.sh_name, ".dynstr") == 0) {
            dynstr = read_section(fd, &shdr);
            if (dynstr == NULL) {
                free(shstrtab);
                return NULL;
            }
            break;
        }
    }

    if (i == hdr.e_shnum) {
        LOGI("lo_dlneeds: No .dynstr section in %s", library);
        close(fd);
        return NULL;
    }

    /* Read section headers, looking for .dynamic section */

    if (lseek(fd, hdr.e_shoff, SEEK_SET) < 0) {
        LOGI("lo_dlneeds: Could not seek to section headers of %s", library);
        close(fd);
        return NULL;
    }
    for (i = 0; i < hdr.e_shnum; i++) {
        if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
            LOGI("lo_dlneeds: Could not read section header of %s", library);
            close(fd);
            return NULL;
        }
        if (shdr.sh_type == SHT_DYNAMIC) {
            int dynoff;
            int *libnames;

            /* Count number of DT_NEEDED entries */
            n_needed = 0;
            if (lseek(fd, shdr.sh_offset, SEEK_SET) < 0) {
                LOGI("lo_dlneeds: Could not seek to .dynamic section of %s", library);
                close(fd);
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < sizeof(dyn)) {
                    LOGI("lo_dlneeds: Could not read .dynamic entry of %s", library);
                    close(fd);
                    return NULL;
                }
                if (dyn.d_tag == DT_NEEDED)
                    n_needed++;
            }

            /* LOGI("Found %d DT_NEEDED libs", n_needed); */

            result = malloc((n_needed+1) * sizeof(char *));

            n_needed = 0;
            if (lseek(fd, shdr.sh_offset, SEEK_SET) < 0) {
                LOGI("lo_dlneeds: Could not seek to .dynamic section of %s", library);
                close(fd);
                free(result);
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < sizeof(dyn)) {
                    LOGI("lo_dlneeds: Could not read .dynamic entry in %s", library);
                    close(fd);
                    free(result);
                    return NULL;
                }
                if (dyn.d_tag == DT_NEEDED) {
                    /* LOGI("needs: %s\n", dynstr + dyn.d_un.d_val); */
                    result[n_needed] = strdup(dynstr + dyn.d_un.d_val);
                    n_needed++;
                }
            }

            close(fd);
            free(dynstr);
            free(shstrtab);
            result[n_needed] = NULL;
            return result;
        }
    }

    LOGI("lo_dlneeds: Could not find .dynamic section in %s", library);
    close(fd);
    return NULL;
}

void *
lo_dlopen(const char *library)
{
    /*
     * We should *not* try to just dlopen() the bare library name
     * first, as the stupid dynamic linker remembers for each library
     * basename if loading it has failed. Thus if you try loading it
     * once, and it fails because of missing needed libraries, and
     * your load those, and then try again, it fails with an
     * infuriating message "failed to load previously" in the log.
     *
     * We *must* first dlopen() all needed libraries, recursively. It
     * shouldn't matter if we dlopen() a library that already is
     * loaded, dlopen() just returns the same value then.
     */

    typedef struct loadedLib {
        const char *name;
        void *handle;
        struct loadedLib *next;
    } *loadedLib;
    static loadedLib loaded_libraries = NULL;

    loadedLib rover;
    loadedLib new_loaded_lib;

    struct stat st;
    void *p;
    char *full_name;
    char **needed;
    int i;
    int found;

    rover = loaded_libraries;
    while (rover != NULL &&
           strcmp(rover->name, library) != 0)
        rover = rover->next;

    if (rover != NULL)
        return rover->handle;

    /* LOGI("lo_dlopen(%s)", library); */

    found = 0;
    if (library[0] == '/') {
        full_name = strdup(library);

        if (stat(full_name, &st) == 0 &&
            S_ISREG(st.st_mode))
            found = 1;
        else
            free(full_name);
    } else {
        for (i = 0; !found && library_locations[i] != NULL; i++) {
            full_name = malloc(strlen(library_locations[i]) + 1 + strlen(library) + 1);
            strcpy(full_name, library_locations[i]);
            strcat(full_name, "/");
            strcat(full_name, library);

            if (stat(full_name, &st) == 0 &&
                S_ISREG(st.st_mode))
                found = 1;
            else
                free(full_name);
        }
    }

    if (!found) {
        LOGI("lo_dlopen: Library %s not found", library);
        return NULL;
    }

    needed = lo_dlneeds(full_name);
    if (needed == NULL) {
        free(full_name);
        return NULL;
    }

    for (i = 0; needed[i] != NULL; i++) {
        if (lo_dlopen(needed[i]) == NULL) {
            free_ptrarray((void **) needed);
            free(full_name);
            return NULL;
        }
    }

    p = dlopen(full_name, RTLD_LOCAL);
    LOGI("dlopen(%s) = %p", full_name, p);
    free(full_name);
    if (p == NULL)
        LOGI("lo_dlopen: Error from dlopen(%s): %s", library, dlerror());

    new_loaded_lib = malloc(sizeof(*new_loaded_lib));
    new_loaded_lib->name = strdup(library);
    new_loaded_lib->handle = p;

    new_loaded_lib->next = loaded_libraries;
    loaded_libraries = new_loaded_lib;

    return p;
}

void *
lo_dlsym(void *handle,
         const char *symbol)
{
    void *p = dlsym(handle, symbol);
    /* LOGI("dlsym(%p, %s) = %p", handle, symbol, p); */
    if (p == NULL)
        LOGI("lo_dlsym: %s", dlerror());
    return p;
}

int
lo_dladdr(void *addr,
          Dl_info *info)
{
    FILE *maps;
    char line[200];
    int result;

    result = dladdr(addr, info);
    if (result != 0)
        LOGI("dladdr(%p) = { %s:%p, %s:%p ]",
             addr,
             info->dli_fname, info->dli_fbase,
             info->dli_sname ? info->dli_sname : "(none)", info->dli_saddr);
    else {
        LOGI("dladdr(%p) = 0", addr);
        return 0;
    }

    maps = fopen("/proc/self/maps", "r");
    if (maps == NULL) {
        LOGI("lo_dladdr: Could not open /proc/self/maps: %s", strerror(errno));
        return 0;
    }
    while (fgets(line, sizeof(line), maps) != NULL &&
           line[strlen(line)-1] == '\n') {
        void *lo, *hi;
        char file[sizeof(line)];
        file[0] = '\0';
        if (sscanf(line, "%x-%x %*s %*x %*x:%*x %*d %[^\n]", &lo, &hi, file) == 3) {
            /* LOGI("got %p-%p: %s", lo, hi, file); */
            if (addr >= lo && addr < hi) {
                if (info->dli_fbase != lo) {
                    LOGI("lo_dladdr: Base for %s in /proc/self/maps %p doesn't match what dladdr() said", file, lo);
                    fclose(maps);
                    return 0;
                }
                info->dli_fname = strdup(file);
                break;
            }
        }
    }
    fclose(maps);

    return result;
}

int
lo_dlcall_argc_argv(void *function,
                    int argc,
                    const char **argv)
{
    int (*fp)(int, const char **) = function;
    int result = fp(argc, argv);

    return result;
}

void android_main(struct android_app* state)
{
    struct engine engine;

    app = state;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;

    lo_main(lo_main_argc, lo_main_argv);

    exit(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
