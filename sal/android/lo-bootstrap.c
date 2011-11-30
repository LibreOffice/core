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
 * Zip parsing code lifted from Mozilla's other-licenses/android/APKOpen.cpp,
 * by Michael Wu <mwu@mozilla.com>.
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
#include <sys/mman.h>

#include <jni.h>

#include <linux/elf.h>

#include <android/log.h>

#include "lo-bootstrap.h"

#include "android_native_app_glue.c"

#undef LOGI
#undef LOGW

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "lo-bootstrap", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "lo-bootstrap", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "lo-bootstrap", __VA_ARGS__))
#define LOGF(...) ((void)__android_log_print(ANDROID_LOG_FATAL, "lo-bootstrap", __VA_ARGS__))

#define ROUND_DOWN(ptr,multiple) (void *)(((unsigned) (ptr)) & ~((multiple)-1))

struct engine {
    int dummy;
};

static struct android_app *app;
static const char **library_locations;
static void *apk_file;
static int apk_file_size;
static int (*lo_main)(int, const char **);
static int lo_main_argc;
static const char **lo_main_argv;
static int sleep_time = 0;

/* Zip data structures */

/* compression methods */
#define STORE    0
#define DEFLATE  8
#define LZMA    14

struct local_file_header {
    uint32_t signature;
    uint16_t min_version;
    uint16_t general_flag;
    uint16_t compression;
    uint16_t lastmod_time;
    uint16_t lastmod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_size;
    uint16_t extra_field_size;
    char     data[0];
} __attribute__((__packed__));

struct cdir_entry {
    uint32_t signature;
    uint16_t creator_version;
    uint16_t min_version;
    uint16_t general_flag;
    uint16_t compression;
    uint16_t lastmod_time;
    uint16_t lastmod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_size;
    uint16_t extra_field_size;
    uint16_t file_comment_size;
    uint16_t disk_num;
    uint16_t internal_attr;
    uint32_t external_attr;
    uint32_t offset;
    char     data[0];
} __attribute__((__packed__));

#define CDIR_END_SIG 0x06054b50

struct cdir_end {
    uint32_t signature;
    uint16_t disk_num;
    uint16_t cdir_disk;
    uint16_t disk_entries;
    uint16_t cdir_entries;
    uint32_t cdir_size;
    uint32_t cdir_offset;
    uint16_t comment_size;
    char     comment[0];
} __attribute__((__packed__));

/* End of Zip data structures */

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
        LOGE("Could not find the String class");
        free_ptrarray((void **) needed);
        return NULL;
    }

    result = (*env)->NewObjectArray(env, n_needed, String, NULL);
    if (result == NULL) {
        LOGE("Could not create the String array");
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
        LOGE("Could not find String[] class");
        return 0;
    }

    if ((*env)->IsInstanceOf(env, argument, StringArray)) {
        int argc = (*env)->GetArrayLength(env, argument);
        const char **argv = malloc(sizeof(char *) * (argc+1));
        int i, result;
        for (i = 0; i < argc; i++) {
            argv[i] = (*env)->GetStringUTFChars(env, (*env)->GetObjectArrayElement(env, argument, i), NULL);
            /* LOGI("argv[%d] = %s", i, argv[i]); */
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

// public static native boolean setup(String dataDir,
//                                    String apkFile,
//                                    String[] ld_library_path);

jboolean
Java_org_libreoffice_android_Bootstrap_setup__Ljava_lang_String_2Ljava_lang_String_2_3Ljava_lang_String_2
    (JNIEnv* env,
     jobject clazz,
     jstring dataDir,
     jstring apkFile,
     jobjectArray ld_library_path)
{
    struct stat st;
    int i, n, fd;
    const jbyte *dataDirPath;
    const jbyte *apkFilePath;
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

    apkFilePath =  (*env)->GetStringUTFChars(env, apkFile, NULL);

    fd = open(apkFilePath, O_RDONLY);
    if (fd == -1) {
        LOGE("Could not open %s", apkFilePath);
        (*env)->ReleaseStringUTFChars(env, apkFile, apkFilePath);
        return JNI_FALSE;
    }
    if (fstat(fd, &st) == -1) {
        LOGE("Could not fstat %s", apkFilePath);
        close(fd);
        (*env)->ReleaseStringUTFChars(env, apkFile, apkFilePath);
        return JNI_FALSE;
    }
    apk_file = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    if (apk_file == MAP_FAILED) {
        LOGE("Could not mmap %s", apkFilePath);
        (*env)->ReleaseStringUTFChars(env, apkFile, apkFilePath);
        return JNI_FALSE;
    }
    apk_file_size = st.st_size;

    (*env)->ReleaseStringUTFChars(env, apkFile, apkFilePath);

    return JNI_TRUE;
}

// public statuc native boolean setup(int lo_main_ptr,
//                                    Object lo_main_argument,
//                                    int lo_main_delay);

jboolean
Java_org_libreoffice_android_Bootstrap_setup__ILjava_lang_Object_2I(JNIEnv* env,
                                                                    jobject clazz,
                                                                    void *lo_main_ptr,
                                                                    jobject lo_main_argument,
                                                                    jint lo_main_delay)
{
    jclass StringArray;
    int i;

    lo_main = lo_main_ptr;

    StringArray = (*env)->FindClass(env, "[Ljava/lang/String;");
    if (StringArray == NULL) {
        LOGE("Could not find String[] class");
        return JNI_FALSE;
    }

    if (!(*env)->IsInstanceOf(env, lo_main_argument, StringArray)) {
        LOGE("lo_main_argument is not a String[]?");
        return JNI_FALSE;
    }

    lo_main_argc = (*env)->GetArrayLength(env, lo_main_argument);
    lo_main_argv = malloc(sizeof(char *) * (lo_main_argc+1));

    for (i = 0; i < lo_main_argc; i++) {
        const jbyte *s = (*env)->GetStringUTFChars(env, (*env)->GetObjectArrayElement(env, lo_main_argument, i), NULL);
        lo_main_argv[i] = strdup(s);
        (*env)->ReleaseStringUTFChars(env, (*env)->GetObjectArrayElement(env, lo_main_argument, i), s);
        /* LOGI("argv[%d] = %s", i, lo_main_argv[i]); */
    }
    lo_main_argv[lo_main_argc] = NULL;

    sleep_time = lo_main_delay;

    return JNI_TRUE;
}

// public static native int getpid();

jint
Java_org_libreoffice_android_Bootstrap_getpid(JNIEnv* env,
                                              jobject clazz)
{
    return getpid();
}


// public static native void system(String cmdline);

jint
Java_org_libreoffice_android_Bootstrap_system(JNIEnv* env,
                                              jobject clazz,
                                              jstring cmdline)
{
    const jbyte *s = (*env)->GetStringUTFChars(env, cmdline, NULL);

    LOGI("system(%s)", s);

    system(s);

    (*env)->ReleaseStringUTFChars(env, cmdline, s);
}

// public static native void putenv(String string);

void
Java_org_libreoffice_android_Bootstrap_putenv(JNIEnv* env,
                                              jobject clazz,
                                              jstring string)
{
    const jbyte *s = (*env)->GetStringUTFChars(env, string, NULL);

    LOGI("putenv(%s)", s);

    putenv(s);

    (*env)->ReleaseStringUTFChars(env, string, s);
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
        LOGE("lo_dlneeds: Could not open library %s: %s", library, strerror(errno));
        return NULL;
    }

    if (read(fd, &hdr, sizeof(hdr)) < sizeof(hdr)) {
        LOGE("lo_dlneeds: Could not read ELF header of %s", library);
        close(fd);
        return NULL;
    }

    /* Read in .shstrtab */

    if (lseek(fd, hdr.e_shoff + hdr.e_shstrndx * sizeof(shdr), SEEK_SET) < 0) {
        LOGE("lo_dlneeds: Could not seek to .shstrtab section header of %s", library);
        close(fd);
        return NULL;
    }
    if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
        LOGE("lo_dlneeds: Could not read section header of %s", library);
        close(fd);
        return NULL;
    }

    shstrtab = read_section(fd, &shdr);
    if (shstrtab == NULL)
        return NULL;

    /* Read section headers, looking for .dynstr section */

    if (lseek(fd, hdr.e_shoff, SEEK_SET) < 0) {
        LOGE("lo_dlneeds: Could not seek to section headers of %s", library);
        close(fd);
        return NULL;
    }
    for (i = 0; i < hdr.e_shnum; i++) {
        if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
            LOGE("lo_dlneeds: Could not read section header of %s", library);
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
        LOGE("lo_dlneeds: No .dynstr section in %s", library);
        close(fd);
        return NULL;
    }

    /* Read section headers, looking for .dynamic section */

    if (lseek(fd, hdr.e_shoff, SEEK_SET) < 0) {
        LOGE("lo_dlneeds: Could not seek to section headers of %s", library);
        close(fd);
        return NULL;
    }
    for (i = 0; i < hdr.e_shnum; i++) {
        if (read(fd, &shdr, sizeof(shdr)) < sizeof(shdr)) {
            LOGE("lo_dlneeds: Could not read section header of %s", library);
            close(fd);
            return NULL;
        }
        if (shdr.sh_type == SHT_DYNAMIC) {
            int dynoff;
            int *libnames;

            /* Count number of DT_NEEDED entries */
            n_needed = 0;
            if (lseek(fd, shdr.sh_offset, SEEK_SET) < 0) {
                LOGE("lo_dlneeds: Could not seek to .dynamic section of %s", library);
                close(fd);
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < sizeof(dyn)) {
                    LOGE("lo_dlneeds: Could not read .dynamic entry of %s", library);
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
                LOGE("lo_dlneeds: Could not seek to .dynamic section of %s", library);
                close(fd);
                free(result);
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < sizeof(dyn)) {
                    LOGE("lo_dlneeds: Could not read .dynamic entry in %s", library);
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

    LOGE("lo_dlneeds: Could not find .dynamic section in %s", library);
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
        LOGE("lo_dlopen: Library %s not found", library);
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
    free_ptrarray((void **) needed);

    p = dlopen(full_name, RTLD_LOCAL);
    LOGI("dlopen(%s) = %p", full_name, p);
    free(full_name);
    if (p == NULL)
        LOGE("lo_dlopen: Error from dlopen(%s): %s", library, dlerror());

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
        LOGE("lo_dlsym: %s", dlerror());
    return p;
}

int
lo_dladdr(void *addr,
          Dl_info *info)
{
    FILE *maps;
    char line[200];
    int result;
    int found;

    result = dladdr(addr, info);
    if (result == 0) {
        /* LOGI("dladdr(%p) = 0", addr); */
        return 0;
    }

    maps = fopen("/proc/self/maps", "r");
    if (maps == NULL) {
        LOGE("lo_dladdr: Could not open /proc/self/maps: %s", strerror(errno));
        return 0;
    }

    found = 0;
    while (fgets(line, sizeof(line), maps) != NULL &&
           line[strlen(line)-1] == '\n') {
        void *lo, *hi;
        char file[sizeof(line)];
        file[0] = '\0';
        if (sscanf(line, "%x-%x %*s %*x %*x:%*x %*d %[^\n]", &lo, &hi, file) == 3) {
            /* LOGI("got %p-%p: %s", lo, hi, file); */
            if (addr >= lo && addr < hi) {
                if (info->dli_fbase != lo) {
                    LOGE("lo_dladdr: Base for %s in /proc/self/maps %p doesn't match what dladdr() said", file, lo);
                    fclose(maps);
                    return 0;
                }
                /* LOGI("dladdr(%p) = { %s:%p, %s:%p }: %s",
                     addr,
                     info->dli_fname, info->dli_fbase,
                     info->dli_sname ? info->dli_sname : "(none)", info->dli_saddr,
                     file); */
                info->dli_fname = strdup(file);
                found = 1;
                break;
            }
        }
    }
    if (!found)
        LOGE("lo_dladdr: Did not find %p in /proc/self/maps", addr);
    fclose(maps);

    return result;
}

static uint32_t cdir_entry_size (struct cdir_entry *entry)
{
    return sizeof(*entry) +
        letoh16(entry->filename_size) +
        letoh16(entry->extra_field_size) +
        letoh16(entry->file_comment_size);
}

static struct cdir_entry *
find_cdir_entry (struct cdir_entry *entry, int count, const char *name)
{
    size_t name_size = strlen(name);
    while (count--) {
        if (letoh16(entry->filename_size) == name_size &&
            !memcmp(entry->data, name, name_size))
            return entry;
        entry = (struct cdir_entry *)((char *)entry + cdir_entry_size(entry));
    }
    return NULL;
}

void *
lo_apkentry(const char *filename,
            size_t *size)
{
    struct cdir_end *dirend = (struct cdir_end *)((char *) apk_file + apk_file_size - sizeof(*dirend));
    uint32_t cdir_offset;
    uint16_t cdir_entries;
    struct cdir_entry *cdir_start;
    struct cdir_entry *entry;
    struct local_file_header *file;
    void *data;

    while ((void *)dirend > apk_file &&
           letoh32(dirend->signature) != CDIR_END_SIG)
        dirend = (struct cdir_end *)((char *)dirend - 1);
    if (letoh32(dirend->signature) != CDIR_END_SIG) {
        LOGE("lo_apkentry: Could not find end of central directory record");
        return;
    }

    cdir_offset = letoh32(dirend->cdir_offset);
    cdir_entries = letoh16(dirend->cdir_entries);
    cdir_start = (struct cdir_entry *)((char *)apk_file + cdir_offset);

    if (*filename == '/')
        filename++;

    entry = find_cdir_entry(cdir_start, cdir_entries, filename);

    if (entry == NULL) {
        LOGE("lo_apkentry: Could not find %s", filename);
        return NULL;
    }
    file = (struct local_file_header *)((char *)apk_file + letoh32(entry->offset));

    if (letoh16(file->compression) != STORE) {
        LOGE("lo_apkentry: File %s is compressed", filename);
        return NULL;
    }

    data = ((char *)&file->data) + letoh16(file->filename_size) + letoh16(file->extra_field_size);
    *size = file->uncompressed_size;

    /* LOGI("lo_apkentry(%s): %p, %d", filename, data, *size); */

    return data;
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

/* There is a bug in std::type_info::operator== and
 * std::type_info::before() in libgnustl_shared.so in NDK r7 at
 * least. They compare the type name pointers instead of comparing the
 * type name strings. See
 * http://code.google.com/p/android/issues/detail?id=22165 . So patch
 * that, poke in jumps to our own code snippets below instead.
 */

/* Replacement std::type_info::operator== */

__asm("    .arm\n"
      "    .global replacement_operator_equals_arm\n"
      "replacement_operator_equals_arm:\n"
      "    push {lr}\n"

      /* Load name pointers into r0 and r1 */
      "    ldr r0, [r0, #4]\n"
      "    ldr r1, [r1, #4]\n"

      /* First compare pointers */
      "    cmp r0, r1\n"

      /* If equal, return true */
      "    beq .L.equals.1\n"

      /* Otherwise call strcmp */
      "    bl strcmp\n"

      /* And return true or false */
      "    cmp r0, #0\n"
      "    moveq r0, #1\n"
      "    movne r0, #0\n"
      "    b .L.equals.9\n"

      ".L.equals.1:\n"
      "    mov r0, #1\n"

      ".L.equals.9:\n"
      "    pop {pc}\n"
      );

extern unsigned int replacement_operator_equals_arm;

/* The ARM (not Thumb) code of the operator== in NDK r7 */
static unsigned int expected_operator_equals_r7_code[] = {
    0xe5903004, /* ldr r3, [r0, #4] */
    0xe5910004, /* ldr r0, [r1, #4] */
    0xe1530000, /* cmp r3, r0 */
    0x13a00000, /* movne, #0 */
    0x03a00001, /* moveq r0, #1 */
    0xe12fff1e  /* bx lr */
};

/* Ditto for  std::type_info::before() */

__asm("    .arm\n"
      "    .global replacement_method_before_arm\n"
      "replacement_method_before_arm:\n"
      "    push {lr}\n"

      /* Load name pointers into r0 and r1 */
      "    ldr r0, [r0, #4]\n"
      "    ldr r1, [r1, #4]\n"

      /* First compare pointers */
      "    cmp r0, r1\n"

      /* If equal, return false */
      "    beq .L.before.1\n"

      /* Otherwise call strcmp */
      "    bl strcmp\n"

      /* And return true or false */
      "    cmp r0, #0\n"
      "    movlt r0, #1\n"
      "    movge r0, #0\n"
      "    b .L.before.9\n"

      ".L.before.1:\n"
      "    mov r0, #0\n"

      ".L.before.9:\n"
      "    pop {pc}\n"
      );

extern unsigned int replacement_method_before_arm;

static unsigned int expected_method_before_r7_code[] = {
    0xe5903004, /* ldr r3, [r0, #4] */
    0xe5910004, /* ldr r0, [r1, #4] */
    0xe1530000, /* cmp r3, r0 */
    0x23a00000, /* movcs r0, #0 */
    0x33a00001, /* movcc r0, #1 */
    0xe12fff1e  /* bx lr */
};

static void
patch(const char *symbol,
      const char *plaintext,
      unsigned *expected_code,
      size_t expected_code_size,
      unsigned *replacement_code)
{

    void *libgnustl_shared;
    void *code;

    void *base;
    size_t size;

    /* libgnustl_shared.so should be already loaded as we build
     * all LO code against it, so as we have loaded the .so
     * containing lo_main() already, libgnustl_shared.so will have
     * been brought in, too.
     */
    libgnustl_shared = dlopen("libgnustl_shared.so", RTLD_LOCAL);
    if (libgnustl_shared == NULL) {
        LOGF("android_main: libgnustl_shared.so not mapped??");
        exit(0);
    }

    code = dlsym(libgnustl_shared, symbol);
    if (code == NULL) {
        LOGF("android_main: %s not found!?", plaintext);
        exit(0);
    }
    /* LOGI("%s is at %p", plaintext, operator_equals); */

    if ((((unsigned) code) & 0x03) != 0) {
        LOGE("android_main: Address of %s is not at word boundary, huh?", plaintext);
        return;
    }

    if ((((unsigned) &replacement_code) & 0x03) != 0) {
        LOGE("android_main: Address of replacement %s is not at word boundary, huh?", plaintext);
        return;
    }

    if (memcmp(code, expected_code, expected_code_size) != 0) {
        LOGI("android_main: Code for %s does not match that in NDK r7; not patching it", plaintext);
        return;
    }

    base = ROUND_DOWN(code, getpagesize());
    size = code + sizeof(expected_code_size) - ROUND_DOWN(code, getpagesize());
    if (mprotect(base, size, PROT_READ|PROT_WRITE|PROT_EXEC) == -1) {
        LOGE("android_main: mprotect() failed: %s", strerror(errno));
        return;
    }

    /* Poke a "b replacement_code" into it instead */
    *((unsigned *) code) =
        (0xEA000000  |
         ((((int) replacement_code - ((int) code + 8)) / 4) & 0x00FFFFFF));
}

static void
patch_libgnustl_shared(void)
{
    patch("_ZNKSt9type_infoeqERKS_",
          "std::type_info::operator==",
          expected_operator_equals_r7_code,
          sizeof(expected_operator_equals_r7_code),
          &replacement_operator_equals_arm);

    patch("_ZNKSt9type_info6beforeERKS_",
          "std::type_info::before()",
          expected_method_before_r7_code,
          sizeof(expected_method_before_r7_code),
          &replacement_method_before_arm);
}

void
Java_org_libreoffice_android_Bootstrap_patch_libgnustl_shared(JNIEnv* env,
                                                              jobject clazz)
{
    patch_libgnustl_shared();
}

JavaVM *
lo_get_javavm(void)
{
    return app->activity->vm;
}

void
android_main(struct android_app* state)
{
    struct engine engine;
    Dl_info lo_main_info;

    app = state;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;

    if (lo_dladdr(lo_main, &lo_main_info) != 0) {
        lo_main_argv[0] = lo_main_info.dli_fname;
    }

    if (sleep_time != 0)
        sleep(sleep_time);

    patch_libgnustl_shared();

    lo_main(lo_main_argc, lo_main_argv);

    exit(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
