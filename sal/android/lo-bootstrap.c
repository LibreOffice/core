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
#include <sys/time.h>

#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>

#include <jni.h>

#include <linux/elf.h>

#include <android/log.h>

#include "uthash.h"

#include "osl/detail/android-bootstrap.h"

#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#include "android_native_app_glue.c"

#pragma GCC diagnostic warning "-Wdeclaration-after-statement"

#undef LOGI
#undef LOGW

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "lo-bootstrap", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "lo-bootstrap", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "lo-bootstrap", __VA_ARGS__))
#define LOGF(...) ((void)__android_log_print(ANDROID_LOG_FATAL, "lo-bootstrap", __VA_ARGS__))

#define ROUND_DOWN(ptr,multiple) (void *)(((unsigned) (ptr)) & ~((multiple)-1))

#define MAX(a,b) ((a) > (b) ? (a) : (b))

struct engine {
    int dummy;
};

/* These vars are valid / used only when this library is used from
 *  NativeActivity-based apps.
 */
static struct android_app *app;
static int (*lo_main)(int, const char **);
static int lo_main_argc;
static const char **lo_main_argv;
static int sleep_time = 0;

/* These are valid / used in all apps. */
static const char *data_dir;
static const char **library_locations;
static void *apk_file;
static int apk_file_size;
static JavaVM *the_java_vm;

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

static struct cdir_entry *cdir_start;
static uint16_t cdir_entries;

/* Data structure to turn Zip's list in arbitrary order of
 * hierarchical pathnames (not necessarily including entries for
 * directories) into an actual hierarchical directory tree, so that we
 * can iterate over directory entries properly in the dirent style
 * functions.
 */

typedef struct direntry *direntry;

struct direntry {
    UT_hash_handle hh;
    enum { REGULAR, DIRECTORY } kind;
    int ino;
    union {
        struct cdir_entry *file;
        direntry subdir;
    };
};

struct lo_apk_dir {
    direntry cur;
};

static direntry assets = NULL;

static uint32_t
cdir_entry_size(struct cdir_entry *entry)
{
    return sizeof(*entry) +
        letoh16(entry->filename_size) +
        letoh16(entry->extra_field_size) +
        letoh16(entry->file_comment_size);
}

static int
setup_cdir(void)
{
    struct cdir_end *dirend = (struct cdir_end *)((char *) apk_file + apk_file_size - sizeof(*dirend));
    uint32_t cdir_offset;

    while ((void *)dirend > apk_file &&
           letoh32(dirend->signature) != CDIR_END_SIG)
        dirend = (struct cdir_end *)((char *)dirend - 1);
    if (letoh32(dirend->signature) != CDIR_END_SIG) {
        LOGE("setup_cdir: Could not find end of central directory record");
        return 0;
    }

    cdir_offset = letoh32(dirend->cdir_offset);

    cdir_entries = letoh16(dirend->cdir_entries);
    cdir_start = (struct cdir_entry *)((char *)apk_file + cdir_offset);

    return 1;
}

static struct cdir_entry *
find_cdir_entry(struct cdir_entry *entry, int count, const char *name)
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

static void
handle_one_asset(struct cdir_entry *entry)
{
    /* In the .apk there are no initial slashes */
    const char *p = entry->data + sizeof("assets/")-1;
    const char *z = entry->data + entry->filename_size;
    direntry *dir = &assets;
    static int ino = 1;

    while (p < z) {
        const char *q = p;
        direntry old, new;

        while (q < z && *q != '/')
            q++;
        HASH_FIND(hh, *dir, p, (unsigned)(q - p), old);
        if (*q == '/') {
            if (old == NULL) {
                new = malloc(sizeof(*new));
                new->ino = ino++;
                new->kind = DIRECTORY;
                new->subdir = NULL;
                HASH_ADD_KEYPTR(hh, *dir, p, (unsigned)(q - p), new);
                dir = &new->subdir;
            } else {
                dir = &old->subdir;
            }
            p = q + 1;
        } else {
            if (old == NULL) {
                new = malloc(sizeof(*new));
                new->ino = ino++;
                new->kind = REGULAR;
                new->file = entry;
                HASH_ADD_KEYPTR(hh, *dir, p, (unsigned)(q - p), new);
            } else {
                LOGE("duplicate entry in apk: %.*s", entry->filename_size, entry->data);
            }
            p = q;
        }
        (void) dir;
    }
}

static int
setup_assets_tree(void)
{
    int count = cdir_entries;
    struct cdir_entry *entry = cdir_start;

    while (count--) {
        if (letoh16(entry->filename_size) >= sizeof("assets/")-1 &&
            memcmp(entry->data, "assets/", sizeof("assets/")-1) == 0)
            handle_one_asset(entry);
        entry = (struct cdir_entry *)((char *)entry + cdir_entry_size(entry));
    }
    return 1;
}

static void
engine_handle_cmd(struct android_app* state,
                  int32_t cmd)
{
    (void) state;

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
    if (read(fd, result, shdr->sh_size) < (int) shdr->sh_size) {
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

/* The lo-bootstrap shared library is always loaded from Java, so
 * this is always called by JNI first.
 */
__attribute__ ((visibility("default")))
jint
JNI_OnLoad(JavaVM* vm, void* reserved)
{
    (void) reserved;

    the_java_vm = vm;

    return JNI_VERSION_1_2;
}

__attribute__ ((visibility("default")))
jobjectArray
Java_org_libreoffice_android_Bootstrap_dlneeds(JNIEnv* env,
                                               jobject clazz,
                                               jstring library)
{
    char **needed;
    int n_needed;
    const char *libName;
    jclass String;
    jobjectArray result;

    (void) clazz;

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

__attribute__ ((visibility("default")))
jint
Java_org_libreoffice_android_Bootstrap_dlopen(JNIEnv* env,
                                              jobject clazz,
                                              jstring library)
{
    const char *libName;
    void *p;

    (void) clazz;

    libName = (*env)->GetStringUTFChars(env, library, NULL);
    p = lo_dlopen (libName);
    (*env)->ReleaseStringUTFChars(env, library, libName);

    return (jint) p;
}

__attribute__ ((visibility("default")))
jint
Java_org_libreoffice_android_Bootstrap_dlsym(JNIEnv* env,
                                             jobject clazz,
                                             jint handle,
                                             jstring symbol)
{
    const char *symName;
    void *p;

    (void) clazz;

    symName = (*env)->GetStringUTFChars(env, symbol, NULL);
    p = lo_dlsym ((void *) handle, symName);
    (*env)->ReleaseStringUTFChars(env, symbol, symName);

    return (jint) p;
}

__attribute__ ((visibility("default")))
jint
Java_org_libreoffice_android_Bootstrap_dlcall(JNIEnv* env,
                                              jobject clazz,
                                              jint function,
                                              jobject argument)
{
    jclass StringArray;

    (void) clazz;

    StringArray = (*env)->FindClass(env, "[Ljava/lang/String;");
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

__attribute__ ((visibility("default")))
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
    const char *dataDirPath;
    const char *apkFilePath;
    char *lib_dir;

    (void) clazz;

    n = (*env)->GetArrayLength(env, ld_library_path);

    library_locations = malloc((n+2) * sizeof(char *));

    dataDirPath = (*env)->GetStringUTFChars(env, dataDir, NULL);

    data_dir = strdup(dataDirPath);

    lib_dir = malloc(strlen(dataDirPath) + 5);
    strcpy(lib_dir, dataDirPath);
    strcat(lib_dir, "/lib");

    (*env)->ReleaseStringUTFChars(env, dataDir, dataDirPath);

    library_locations[0] = lib_dir;

    for (i = 0; i < n; i++) {
        const char *s = (*env)->GetStringUTFChars(env, (*env)->GetObjectArrayElement(env, ld_library_path, i), NULL);
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

    if (!setup_cdir())
        return JNI_FALSE;

    if (!setup_assets_tree())
        return JNI_FALSE;

    return JNI_TRUE;
}

static jboolean
get_jni_string_array(JNIEnv *env,
                     const char *function_and_parameter_name,
                     jobject strv,
                     int *argc,
                     const char ***argv)
{
    jclass StringArray;
    int i;

    StringArray = (*env)->FindClass(env, "[Ljava/lang/String;");
    if (StringArray == NULL) {
        LOGE("Could not find String[] class");
        return JNI_FALSE;
    }

    if (!(*env)->IsInstanceOf(env, strv, StringArray)) {
        LOGE("%s is not a String[]?", function_and_parameter_name);
        return JNI_FALSE;
    }

    *argc = (*env)->GetArrayLength(env, strv);
    *argv = malloc(sizeof(char *) * (*argc+1));

    for (i = 0; i < *argc; i++) {
        const char *s = (*env)->GetStringUTFChars(env, (*env)->GetObjectArrayElement(env, strv, i), NULL);
        (*argv)[i] = strdup(s);
        (*env)->ReleaseStringUTFChars(env, (*env)->GetObjectArrayElement(env, strv, i), s);
        /* LOGI("argv[%d] = %s", i, lo_main_argv[i]); */
    }
    (*argv)[*argc] = NULL;

    return JNI_TRUE;
}


// public static native boolean setup(int lo_main_ptr,
//                                    Object lo_main_argument,
//                                    int lo_main_delay);

__attribute__ ((visibility("default")))
jboolean
Java_org_libreoffice_android_Bootstrap_setup__ILjava_lang_Object_2I(JNIEnv* env,
                                                                    jobject clazz,
                                                                    void *lo_main_ptr,
                                                                    jobject lo_main_argument,
                                                                    jint lo_main_delay)
{
    (void) clazz;

    lo_main = lo_main_ptr;

    if (!get_jni_string_array(env, "setup: lo_main_argument", lo_main_argument, &lo_main_argc, &lo_main_argv))
        return JNI_FALSE;

    sleep_time = lo_main_delay;

    return JNI_TRUE;
}

// public static native int getpid();

__attribute__ ((visibility("default")))
jint
Java_org_libreoffice_android_Bootstrap_getpid(JNIEnv* env,
                                              jobject clazz)
{
    (void) env;
    (void) clazz;

    return getpid();
}


// public static native void system(String cmdline);

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_system(JNIEnv* env,
                                              jobject clazz,
                                              jstring cmdline)
{
    const char *s;

    (void) clazz;

    s = (*env)->GetStringUTFChars(env, cmdline, NULL);

    LOGI("system(%s)", s);

    system(s);

    (*env)->ReleaseStringUTFChars(env, cmdline, s);
}

// public static native void putenv(String string);

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_putenv(JNIEnv* env,
                                              jobject clazz,
                                              jstring string)
{
    const char *s;

    (void) clazz;

    s = (*env)->GetStringUTFChars(env, string, NULL);

    LOGI("putenv(%s)", s);

    putenv(s);

    (*env)->ReleaseStringUTFChars(env, string, s);
}

__attribute__ ((visibility("default")))
char **
lo_dlneeds(const char *library)
{
    int i, fd;
    int n_needed;
    char **result;
    char *shstrtab;
    char *dynstr = NULL;
    Elf32_Ehdr hdr;
    Elf32_Shdr shdr;
    Elf32_Dyn dyn;

    /* Open library and read ELF header */

    fd = open(library, O_RDONLY);

    if (fd == -1) {
        LOGE("lo_dlneeds: Could not open library %s: %s", library, strerror(errno));
        return NULL;
    }

    if (read(fd, &hdr, sizeof(hdr)) < (int) sizeof(hdr)) {
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
    if (read(fd, &shdr, sizeof(shdr)) < (int) sizeof(shdr)) {
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
        if (read(fd, &shdr, sizeof(shdr)) < (int) sizeof(shdr)) {
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
        if (read(fd, &shdr, sizeof(shdr)) < (int) sizeof(shdr)) {
            LOGE("lo_dlneeds: Could not read section header of %s", library);
            close(fd);
            return NULL;
        }
        if (shdr.sh_type == SHT_DYNAMIC) {
            size_t dynoff;

            /* Count number of DT_NEEDED entries */
            n_needed = 0;
            if (lseek(fd, shdr.sh_offset, SEEK_SET) < 0) {
                LOGE("lo_dlneeds: Could not seek to .dynamic section of %s", library);
                close(fd);
                return NULL;
            }
            for (dynoff = 0; dynoff < shdr.sh_size; dynoff += sizeof(dyn)) {
                if (read(fd, &dyn, sizeof(dyn)) < (int) sizeof(dyn)) {
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
                if (read(fd, &dyn, sizeof(dyn)) < (int) sizeof(dyn)) {
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
            if (dynstr)
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

__attribute__ ((visibility("default")))
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

    struct timeval tv0, tv1, tvdiff;

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

    gettimeofday(&tv0, NULL);
    p = dlopen(full_name, RTLD_LOCAL);
    gettimeofday(&tv1, NULL);
    timersub(&tv1, &tv0, &tvdiff);
    LOGI("dlopen(%s) = %p, %ld.%03lds",
         full_name, p,
         (long) tvdiff.tv_sec, (long) tvdiff.tv_usec / 1000);
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

__attribute__ ((visibility("default")))
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

__attribute__ ((visibility("default")))
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
        if (sscanf(line, "%x-%x %*s %*x %*x:%*x %*d %[^\n]", (unsigned *) &lo, (unsigned *) &hi, file) == 3) {
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

__attribute__ ((visibility("default")))
int
lo_dlclose(void *handle)
{
    /* As we don't know when the reference count for a dlopened shared
     * object drops to zero, we wouldn't know when to remove it from
     * our list, so we can't call dlclose().
     */
    LOGI("lo_dlclose(%p)", handle);

    return 0;
}

__attribute__ ((visibility("default")))
void *
lo_apkentry(const char *filename,
            size_t *size)
{
    struct cdir_entry *entry;
    struct local_file_header *file;
    void *data;

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

__attribute__ ((visibility("default")))
lo_apk_dir *
lo_apk_opendir(const char *dirname)
{
    /* In the .apk there are no initial slashes, but the parameter passed to
     * us does have it.
     */
    const char *p = dirname + sizeof("/assets/")-1;
    direntry dir = assets;

    if (!*p) {
        lo_apk_dir *result = malloc(sizeof(*result));
        result->cur = assets;
        return result;
    }

    while (1) {
        const char *q = p;
        direntry entry;

        while (*q && *q != '/')
            q++;

        HASH_FIND(hh, dir, p, (unsigned)(q - p), entry);

        if (entry == NULL && *q == '/') {
            errno = ENOENT;
            return NULL;
        } else if (entry == NULL) {
            /* Empty directories, or directories containing only "hidden"
             * files (like the .gitignore in sc/qa/unit/qpro/indeterminate)
             * are not present in the .apk. So we need to pretend that any
             * directory that doesn't exist as a parent of an entry in the
             * .apk *does* exist but is empty.
             */
            lo_apk_dir *result = malloc(sizeof(*result));
            result->cur = NULL;
            return result;
        }

        if (entry->kind != DIRECTORY) {
            errno = ENOTDIR;
            return NULL;
        }

        if (!q[0] || !q[1]) {
            lo_apk_dir *result = malloc(sizeof(*result));
            result->cur = entry->subdir;
            return result;
        }

        dir = entry->subdir;
        p = q + 1;
    }
}

__attribute__ ((visibility("default")))
struct dirent *
lo_apk_readdir(lo_apk_dir *dirp)
{
    static struct dirent result;

    if (dirp->cur == NULL) {
        /* LOGI("lo_apk_readdir(%p) = NULL", dirp); */
        return NULL;
    }

    result.d_ino = dirp->cur->ino;
    result.d_off = 0;
    result.d_reclen = 0;

    if (dirp->cur->kind == DIRECTORY)
        result.d_type = DT_DIR;
    else
        result.d_type = DT_REG;

    memcpy(result.d_name, dirp->cur->hh.key, dirp->cur->hh.keylen);
    result.d_name[dirp->cur->hh.keylen] = '\0';

    dirp->cur = dirp->cur->hh.next;

    /* LOGI("lo_apk_readdir(%p) = %s:%s", dirp, result.d_type == DT_DIR ? "DIR" : "REG", result.d_name); */

    return &result;
}

__attribute__ ((visibility("default")))
int
lo_apk_closedir(lo_apk_dir *dirp)
{
    free(dirp);

    /* LOGI("lo_apk_closedir(%p)", dirp); */

    return 0;
}

static int
new_stat(const char *path,
         struct stat *statp,
         struct cdir_entry *entry,
         int mode,
         int fake_ino)
{
    struct tm tm;

    memset(statp, 0, sizeof(*statp));
    statp->st_mode = mode | S_IRUSR;
    statp->st_nlink = 1;

    statp->st_uid = getuid();
    statp->st_gid = getgid();

    if (entry != NULL)
        statp->st_size = entry->uncompressed_size;
    else
        statp->st_size = 0;
    statp->st_blksize = 512;
    if (statp->st_size == 0)
        statp->st_blocks = 0;
    else
        statp->st_blocks = (statp->st_size - 1) / statp->st_blksize + 1;

    statp->st_atime = time(NULL);

    memset(&tm, 0, sizeof(tm));
    tm.tm_sec = (letoh16(entry->lastmod_time) & 0x1F) * 2;
    tm.tm_min = (letoh16(entry->lastmod_time) >> 5) & 0x3F;
    tm.tm_hour = (letoh16(entry->lastmod_time) >> 11) & 0x1F;
    tm.tm_mday = letoh16(entry->lastmod_date) & 0x1F;
    tm.tm_mon = ((letoh16(entry->lastmod_date) >> 5) & 0x0F) - 1;
    tm.tm_year = ((letoh16(entry->lastmod_date) >> 9) & 0x7F) + 80;

    statp->st_mtime = mktime(&tm);
    statp->st_ctime = statp->st_mtime;

    statp->st_ino = fake_ino;

    (void) path;
    /* LOGI("lo_apk_lstat(%s) = { mode=%o, size=%lld, ino=%lld mtime=%.24s }",
         path, statp->st_mode, statp->st_size, statp->st_ino,
         ctime((const time_t *) &statp->st_mtime)); */

    return 0;
}

__attribute__ ((visibility("default")))
int
lo_apk_lstat(const char *path,
             struct stat *statp)
{
    const char *pn = path;
    int count = cdir_entries;
    struct cdir_entry *entry = cdir_start;
    size_t name_size;

    if (*pn == '/') {
        pn++;
        if (!pn[0])
            return new_stat(path, statp, NULL, S_IFDIR | S_IXUSR, 1);
    }

    name_size = strlen(pn);
    while (count--)
    {
        if (letoh16(entry->filename_size) >= name_size &&
            !memcmp(entry->data, pn, name_size) &&
            (letoh16(entry->filename_size) == name_size || entry->data[name_size] == '/'))
            break;
        entry = (struct cdir_entry *)((char *)entry + cdir_entry_size(entry));
    }
    if (count >= 0) {
        if (letoh16(entry->filename_size) == name_size)
            return new_stat(path, statp, entry, S_IFREG, cdir_entries - count + 1);
        else
            return new_stat(path, statp, entry, S_IFDIR | S_IXUSR, cdir_entries - count + 1);
    }

    errno = ENOENT;
    return -1;
}

__attribute__ ((visibility("default")))
int
lo_dlcall_argc_argv(void *function,
                    int argc,
                    const char **argv)
{
    int (*fp)(int, const char **) = function;
    int result = fp(argc, argv);

    return result;
}

#ifdef ARM

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

    /* Poke in a jump to replacement_code instead */
    ((unsigned *) code)[0] = 0xe51ff004; /* ldr pc, [pc, #-4] */
    ((unsigned *) code)[1] = (unsigned) replacement_code;
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
#endif // ARM

// static native void patch_libgnustl_shared();
__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_patch_1libgnustl_1shared(JNIEnv* env,
                                                                jobject clazz)
{
    (void) env;
    (void) clazz;

#ifdef ARM
    patch_libgnustl_shared();
#endif
}

#define UNPACK_TREE "/assets/unpack"

static int
mkdir_p(const char *dirname)
{
    char *p = malloc(strlen(dirname) + 1);
    const char *q = dirname + 1;
    const char *slash;

    do {
        slash = strchr(q, '/');
        if (slash == NULL)
            slash = q + strlen(q);
        memcpy(p, dirname, slash-dirname);
        p[slash-dirname] = '\0';
        if (mkdir(p, 0700) == -1 && errno != EEXIST) {
            LOGE("mkdir_p: Could not create %s: %s", p, strerror(errno));
            free(p);
            return 0;
        }
        if (*slash)
            q = slash + 1;
    } while (*slash);

    free(p);
    return 1;
}

static void
extract_files(const char *prefix)
{
    lo_apk_dir *tree = lo_apk_opendir(prefix);
    struct dirent *dent;

    if (tree == NULL)
        return;

    while ((dent = lo_apk_readdir(tree)) != NULL) {
        if (strcmp(dent->d_name, ".") == 0 ||
            strcmp(dent->d_name, "..") == 0)
            continue;

        if (dent->d_type == DT_DIR) {
            char *subdir = malloc(strlen(prefix) + 1 + strlen(dent->d_name) + 1);
            strcpy(subdir, prefix);
            strcat(subdir, "/");
            strcat(subdir, dent->d_name);
            extract_files(subdir);
            free(subdir);
        } else {
            char *filename;
            char *newfilename;
            const char *apkentry;
            size_t size;
            struct stat st;
            FILE *f;

            filename = malloc(strlen(prefix) + 1 + strlen(dent->d_name) + 1);
            strcpy(filename, prefix);
            strcat(filename, "/");
            strcat(filename, dent->d_name);

            apkentry = lo_apkentry(filename, &size);
            if (apkentry == NULL) {
                LOGE("extract_files: Could not find %s in .apk", filename);
                free(filename);
                continue;
            }

            newfilename = malloc(strlen(data_dir) + 1 + strlen(prefix) - sizeof(UNPACK_TREE) + 1 + strlen(dent->d_name) + 1);
            strcpy(newfilename, data_dir);
            strcat(newfilename, "/");
            strcat(newfilename, prefix + sizeof(UNPACK_TREE));

            if (!mkdir_p(newfilename)) {
                free(filename);
                free(newfilename);
                continue;
            }

            strcat(newfilename, "/");
            strcat(newfilename, dent->d_name);

            if (stat(newfilename, &st) == 0 &&
                st.st_size == size) {
                free(filename);
                free(newfilename);
                continue;
            }

            f = fopen(newfilename, "w");
            if (f == NULL) {
                LOGE("extract_files: Could not open %s for writing: %s", newfilename, strerror(errno));
                free(filename);
                free(newfilename);
                continue;
            }

            if (fwrite(apkentry, size, 1, f) != 1) {
                LOGE("extract_files: Could not write %d bytes to %s: %s", size, newfilename, strerror(errno));
            }

            LOGI("extract_files: Copied %s to %s: %d bytes", filename, newfilename, size);

            fclose(f);

            free(filename);
            free(newfilename);
        }
    }
    lo_apk_closedir(tree);
}

// static native void extract_files();

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_extract_1files(JNIEnv* env,
                                                      jobject clazz)
{
    (void) env;
    (void) clazz;

    extract_files(UNPACK_TREE);
}

/* Android's JNI works only to libraries loaded through Java's
 * System.loadLibrary(), it seems. Not to functions loaded by a dlopen() call
 * in native code. For instance, to call a function in libvcllo.so, we need to
 * have its JNI wrapper here, and then call the VCL function from it. Oh well,
 * one could say it's clean to have all the Android-specific JNI functions
 * here in this file.
 */

// public static native void initVCL();

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_initVCL(JNIEnv* env,
                                               jobject clazz)
{
    void (*InitVCLWrapper)(void);
    (void) env;
    (void) clazz;

    /* This obviously should be called only after libvcllo.so has been loaded */

    InitVCLWrapper = dlsym(RTLD_DEFAULT, "InitVCLWrapper");
    if (InitVCLWrapper == NULL) {
        LOGE("InitVCL: InitVCLWrapper not found");
        return;
    }
    (*InitVCLWrapper)();
}

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_setCommandArgs(JNIEnv* env,
                                                      jobject clazz,
                                                      jobject argv)
{
    char **c_argv;
    int c_argc;
    Dl_info lo_bootstrap_info;
    void (*osl_setCommandArgs)(int, char **);

    (void) clazz;

    if (!get_jni_string_array(env, "setCommandArgs :argv", argv, &c_argc, (const char ***) &c_argv))
        return;

    if (lo_dladdr(Java_org_libreoffice_android_Bootstrap_setCommandArgs, &lo_bootstrap_info) != 0) {
        char *new_argv0 = malloc(strlen(lo_bootstrap_info.dli_fname) + strlen(c_argv[0]));
        char *slash;
        strcpy(new_argv0, lo_bootstrap_info.dli_fname);
        slash = strrchr(new_argv0, '/');
        if (slash != NULL)
            *slash = '\0';
        slash = strrchr(new_argv0, '/');
        strcpy(slash+1, c_argv[0]);
        free(c_argv[0]);
        c_argv[0] = new_argv0;
    }

    osl_setCommandArgs = dlsym(RTLD_DEFAULT, "osl_setCommandArgs");
    if (osl_setCommandArgs == NULL) {
        LOGE("setCommandArgs: osl_setCommandArgs not found");
        return;
    }
    (*osl_setCommandArgs)(c_argc, c_argv);
}

__attribute__ ((visibility("default")))
jint
Java_org_libreoffice_android_Bootstrap_createWindowFoo(JNIEnv* env,
                                                       jobject clazz)
{
    int (*createWindowFoo)(void);
    (void) env;
    (void) clazz;

    lo_dlopen("libvcllo.so");
    createWindowFoo = dlsym(RTLD_DEFAULT, "createWindowFoo");
    if (createWindowFoo == NULL) {
        LOGE("createWindowFoo: createWindowFoo not found");
        return 0;
    }
    return (*createWindowFoo)();
}


/* Code for reading lines from the pipe based on the (Apache-licensed) Android
 * logwrapper.c
 */

static int
read_from(int fd, const char *tag, char *buffer, int *sz, int *a, int *b, size_t sizeof_buffer)
{
    int nread;

    nread = read(fd, buffer+*b, sizeof_buffer - 1 - *b);
    *sz = nread;

    if (nread == -1) {
        LOGE("redirect_thread: Reading from %d failed: %s", fd, strerror(errno));
        close(fd);
        return -1;
    }

    if (nread == 0) {
        LOGI("redirect_thread: EOF from fd %d", fd);
        close(fd);
        return 0;
    }

    *sz += *b;

    for (*b = 0; *b < *sz; (*b)++) {
        if (buffer[*b] == '\n') {
            buffer[*b] = '\0';
            __android_log_print(ANDROID_LOG_INFO, tag, "%s", &buffer[*a]);
            *a = *b + 1;
        }
    }

    if (*a == 0 && *b == (int) sizeof_buffer - 1) {
        // buffer is full, flush
        buffer[*b] = '\0';
        __android_log_print(ANDROID_LOG_INFO, tag, "%s", &buffer[*a]);
        *b = 0;
    } else if (*a != *b) {
        // Keep left-overs
        *b -= *a;
        memmove(buffer, &buffer[*a], *b);
        *a = 0;
    } else {
        *a = 0;
        *b = 0;
    }

    return nread;
}

static int stdout_pipe[2], stderr_pipe[2];

static void *
redirect_thread(void *arg)
{
    char buffer[2][4096];
    int a[2] = { 0, 0 };
    int b[2] = { 0, 0 };
    int sz[2];

    (void) arg;

    while (1) {
        fd_set readfds;
        int nfds = 0;

        FD_ZERO(&readfds);
        if (stdout_pipe[0] != -1) {
            FD_SET(stdout_pipe[0], &readfds);
            nfds = MAX(nfds, stdout_pipe[0] + 1);
        }
        if (stderr_pipe[0] != -1) {
            FD_SET(stderr_pipe[0], &readfds);
            nfds = MAX(nfds, stderr_pipe[0] + 1);
        }
        if (nfds == 0) {
            LOGI("redirect_thread: Nothing to read any more, thread exiting");
            return NULL;
        }

        if (select(nfds, &readfds, NULL, NULL, NULL) == -1) {
            LOGE("redirect_thread: select failed: %s, thread exiting", strerror(errno));
            close(stdout_pipe[0]);
            stdout_pipe[0] = -1;
            close(stderr_pipe[0]);
            stderr_pipe[0] = -1;
            return NULL;
        }

        if (stdout_pipe[0] != -1 &&
            FD_ISSET(stdout_pipe[0], &readfds)) {
            if (read_from(stdout_pipe[0], "stdout", buffer[0], &sz[0], &a[0], &b[0], sizeof(buffer[0])) <= 0) {
                stdout_pipe[0] = -1;
            }
        }

        if (stderr_pipe[0] != -1 &&
            FD_ISSET(stderr_pipe[0], &readfds)) {
            if (read_from(stderr_pipe[0], "stderr", buffer[1], &sz[1], &a[1], &b[1], sizeof(buffer[1])) <= 0) {
                stderr_pipe[0] = -1;
            }
        }
    }
}

static int
redirect_to_null(void)
{
    int null = open("/dev/null", O_WRONLY);
    if (null == -1) {
        LOGE("redirect_stdio: Could not open /dev/null: %s", strerror(errno));
        /* If we can't redirect stdout or stderr to /dev/null, just close them
         * then instead. Huh?
         */
        close(1);
        close(2);
        return 0;
    }
    if (dup2(null, 1) == -1) {
        LOGE("redirect_stdio: Could not dup2 %d to 1: %s", null, strerror(errno));
        close(null);
        close(1);
        close(2);
        return 0;
    }
    if (dup2(null, 2) == -1) {
        LOGE("redirect_stdio: Could not dup2 %d to 2: %s", null, strerror(errno));
        close(null);
        close(1);
        close(2);
        return 0;
    }
    close(null);
    return 1;
}

__attribute__ ((visibility("default")))
jboolean
Java_org_libreoffice_android_Bootstrap_redirect_1stdio(JNIEnv* env,
                                                       jobject clazz,
                                                       jboolean state)
{
    static jboolean current = JNI_FALSE;
    pthread_t thread;
    pthread_attr_t attr;

    (void) env;
    (void) clazz;

   if (state == current)
        return current;

    if (state == JNI_FALSE) {
        if (!redirect_to_null())
            return current;
    } else {
        if (pipe(stdout_pipe) == -1) {
            LOGE("redirect_stdio: Could not create pipes: %s", strerror(errno));
            return current;
        }
        if (pipe(stderr_pipe) == -1) {
            LOGE("redirect_stdio: Could not create pipes: %s", strerror(errno));
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            return current;
        }
        LOGI("redirect_stdio: stdout pipe: [%d,%d], stderr pipe: [%d,%d]",
             stdout_pipe[0], stdout_pipe[1], stderr_pipe[0], stderr_pipe[1]);

        if (dup2(stdout_pipe[1], 1) == -1) {
            LOGE("redirect_stdio: Could not dup2 %d to 1: %s", stdout_pipe[1], strerror(errno));
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            close(stderr_pipe[0]);
            close(stderr_pipe[1]);
            return current;
        }

        if (dup2(stderr_pipe[1], 2) == -1) {
            LOGE("redirect_stdio: Could not dup2 %d to 2: %s", stdout_pipe[1], strerror(errno));
            /* stdout has already been redirected to its pipe, so redirect
             * it back to /dev/null
             */
            redirect_to_null();
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            close(stderr_pipe[0]);
            close(stderr_pipe[1]);
            return current;
        }
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        if (pthread_attr_init(&attr) != 0 ||
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 ||
            pthread_create(&thread, &attr, redirect_thread, NULL) != 0) {
            LOGE("redirect_stdio: Could not create thread: %s", strerror(errno));
            redirect_to_null();
            close(stdout_pipe[0]);
            close(stderr_pipe[0]);
            return current;
        }
    }
    current = state;
    return current;
}

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_twiddle_1BGR_1to_1RGBA(JNIEnv* env,
                                                              jobject clazz,
                                                              jbyteArray source,
                                                              jint offset,
                                                              jint width,
                                                              jint height,
                                                              jobject destination)
{
    jbyte *dst = (jbyte*) (*env)->GetDirectBufferAddress(env, destination);
    void *a = (*env)->GetPrimitiveArrayCritical(env, source, NULL);
    jbyte *src = ((jbyte *) a) + offset;

    jbyte *srcp;
    jbyte *dstp = dst;
    int step = ((((width * 3) - 1) / 4) + 1) * 4;

    int i, j;

    (void) clazz;

    if (height > 0) {
        srcp = src + step * (height - 1);
        step = -step;
    } else {
        srcp = src;
    }

    LOGI("twiddle: src=%p, srcp=%p, dstp=%p, step=%d", src, srcp, dstp, step);

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            *dstp++ = srcp[j*3+2];
            *dstp++ = srcp[j*3+1];
            *dstp++ = srcp[j*3+0];
            *dstp++ = 0xFF;
        }
        srcp += step;
    }

    (*env)->ReleasePrimitiveArrayCritical(env, source, a, 0);
}

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_force_1full_1alpha_1array(JNIEnv* env,
                                                                 jobject clazz,
                                                                 jbyteArray array,
                                                                 jint offset,
                                                                 jint length)
{
    void *a = (*env)->GetPrimitiveArrayCritical(env, array, NULL);
    jbyte *p = ((jbyte *) a) + offset;

    int i;

    (void) clazz;

    for (i = 0; i < length; i += 4) {
        p[3] = 0xFF;
        p += 4;
    }

    (*env)->ReleasePrimitiveArrayCritical(env, array, a, 0);
}

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_force_1full_1alpha_1bb(JNIEnv* env,
                                                              jobject clazz,
                                                              jobject buffer,
                                                              jint offset,
                                                              jint length)
{
    jbyte *p = (*env)->GetDirectBufferAddress(env, buffer) + offset;

    int i;

    (void) clazz;

    for (i = 0; i < length; i += 4) {
        p[3] = 0xFF;
        p += 4;
    }
}

__attribute__ ((visibility("default")))
jlong
Java_org_libreoffice_android_Bootstrap_address_1of_1direct_1byte_1buffer(JNIEnv *env,
                                                                         jobject bbuffer)
{
    return (jlong) (intptr_t) (*env)->GetDirectBufferAddress(env, bbuffer);
}

__attribute__ ((visibility("default")))
JavaVM *
lo_get_javavm(void)
{
    return the_java_vm;
}

__attribute__ ((visibility("default")))
struct android_app *
lo_get_app(void)
{
    return app;
}

__attribute__ ((visibility("default")))
void
android_main(struct android_app* state)
{
    jint nRet;
    JNIEnv *pEnv = NULL;
    struct engine engine;
    Dl_info lo_main_info;
    JavaVMAttachArgs aArgs = {
        JNI_VERSION_1_2,
        "LibreOfficeThread",
        NULL
    };

    fprintf (stderr, "android_main in thread: %d\n", (int)pthread_self());

    if (sleep_time != 0) {
        LOGI("android_main: Sleeping for %d seconds, start ndk-gdb NOW if that is your intention", sleep_time);
        sleep(sleep_time);
    }

    nRet = (*(*state->activity->vm)->AttachCurrentThreadAsDaemon)(state->activity->vm, &pEnv, &aArgs);
    fprintf (stderr, "attach thread returned %d %p\n", nRet, pEnv);

    app = state;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;

    if (lo_dladdr(lo_main, &lo_main_info) != 0) {
        lo_main_argv[0] = lo_main_info.dli_fname;
    }

    lo_main(lo_main_argc, lo_main_argv);
    nRet = (*(*state->activity->vm)->DetachCurrentThread)(state->activity->vm);
    fprintf (stderr, "exit android_main\n");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
