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

/* The lo-native-code shared library is always loaded from Java, so this is
 * always called by JNI first.
 */
__attribute__ ((visibility("default")))
jint
JNI_OnLoad(JavaVM* vm, void* reserved)
{
    (void) reserved;

    the_java_vm = vm;

    return JNI_VERSION_1_2;
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


// public static native boolean setup(Object lo_main_argument,
//                                    int lo_main_delay);

__attribute__ ((visibility("default")))
jboolean
Java_org_libreoffice_android_Bootstrap_setup__Ljava_lang_Object_2I(JNIEnv* env,
                                                                    jobject clazz,
                                                                    jobject lo_main_argument,
                                                                    jint lo_main_delay)
{
    (void) clazz;

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

    {
        static int beenhere=0;
        if (!beenhere) {
            LOGI("lo-bootstrap: Sleeping for 20 seconds, start ndk-gdb NOW if that is your intention");
            sleep(20);
            beenhere = 1;
        }
    }

    (*env)->ReleaseStringUTFChars(env, string, s);
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
 * System.loadLibrary(), it seems. But now with just one big app-specific .so
 * on Android, that would not be a problem, but for historical reasons, we
 * have JNI wrappers here, and then call the VCL etc function from them. Oh
 * well, one could say it's clean to have all the Android-specific JNI
 * functions here in this file.
 */

// public static native void initVCL();

extern void InitVCLWrapper(void);

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_initVCL(JNIEnv* env,
                                               jobject clazz)
{
    (void) env;
    (void) clazz;

    InitVCLWrapper();
}

extern void osl_setCommandArgs(int, char **);

__attribute__ ((visibility("default")))
void
Java_org_libreoffice_android_Bootstrap_setCommandArgs(JNIEnv* env,
                                                      jobject clazz,
                                                      jobject argv)
{
    char **c_argv;
    int c_argc;
    Dl_info lo_bootstrap_info;

    (void) clazz;

    if (!get_jni_string_array(env, "setCommandArgs :argv", argv, &c_argc, (const char ***) &c_argv))
        return;

    if (dladdr(Java_org_libreoffice_android_Bootstrap_setCommandArgs, &lo_bootstrap_info) != 0) {
        char *new_argv0 = malloc(strlen(lo_bootstrap_info.dli_fname) + strlen(c_argv[0]));
        char *slash;
        strcpy(new_argv0, lo_bootstrap_info.dli_fname);
        slash = strrchr(new_argv0, '/');
        if (slash != NULL)
            *slash = '\0';
        slash = strrchr(new_argv0, '/');
        if (slash != NULL)
            strcpy(slash+1, c_argv[0]);
        else
            strcpy(new_argv0, c_argv[0]);
        free(c_argv[0]);
        c_argv[0] = new_argv0;
    }

    osl_setCommandArgs(c_argc, c_argv);
}

extern int createWindowFoo(void);

__attribute__ ((visibility("default")))
jint
Java_org_libreoffice_android_Bootstrap_createWindowFoo(JNIEnv* env,
                                                       jobject clazz)
{
    (void) env;
    (void) clazz;

    return createWindowFoo();
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

/* Note that android_main() is used only in NativeActivity-based apps.  Only
 * the android/qa/sc unit test app is such, and it is unclear whether there is
 * any reason to continue maintaining that buildable.
 */
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

    /* Look up lo_main() dynamically even if it is in the same .so as this code,
     * but that is only in the case for code built to be used in a NativeActivity-based app.
     */
    lo_main = dlsym(RTLD_DEFAULT, "lo_main");

    if (dladdr(lo_main, &lo_main_info) != 0) {
        lo_main_argv[0] = lo_main_info.dli_fname;
    }

    lo_main(lo_main_argc, lo_main_argv);
    nRet = (*(*state->activity->vm)->DetachCurrentThread)(state->activity->vm);
    fprintf (stderr, "exit android_main\n");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
