/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <jni.h>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <osl/detail/android-bootstrap.h>

#include <LibreOfficeKit/LibreOfficeKit.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "LibreOfficeKit", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "LibreOfficeKit", __VA_ARGS__))

/* These are valid / used in all apps. */
extern const char* data_dir;
extern const char* cache_dir;
extern void* apk_file;
extern int apk_file_size;
AAssetManager* native_asset_manager;

extern void Java_org_libreoffice_android_Bootstrap_putenv(JNIEnv* env, jobject clazz, jstring string);
extern void Java_org_libreoffice_android_Bootstrap_redirect_1stdio(JNIEnv* env, jobject clazz, jboolean state);

extern LibreOfficeKit* libreofficekit_hook(const char* install_path);

static char *full_program_dir = NULL;

/// Call the same method from Bootstrap.
__attribute__ ((visibility("default")))
void
Java_org_libreoffice_kit_LibreOfficeKit_putenv
    (JNIEnv* env, jobject clazz, jstring string)
{
    Java_org_libreoffice_android_Bootstrap_putenv(env, clazz, string);
}

/// Call the same method from Bootstrap.
__attribute__ ((visibility("default")))
void Java_org_libreoffice_kit_LibreOfficeKit_redirectStdio
    (JNIEnv* env, jobject clazz, jboolean state)
{
    Java_org_libreoffice_android_Bootstrap_redirect_1stdio(env, clazz, state);
}

/// Initialize the LibreOfficeKit.
__attribute__ ((visibility("default")))
jboolean libreofficekit_initialize(JNIEnv* env,
     jstring dataDir, jstring cacheDir, jstring apkFile, jobject assetManager)
{
    struct stat st;
    int fd;
    const char *dataDirPath;
    const char *cacheDirPath;
    const char *apkFilePath;

    const char *fontsConf = "/etc/fonts/fonts.conf";
    char *fontsConfPath;

    setenv("OOO_DISABLE_RECOVERY", "1", 1);

    native_asset_manager = AAssetManager_fromJava(env, assetManager);

    dataDirPath = (*env)->GetStringUTFChars(env, dataDir, NULL);
    data_dir = strdup(dataDirPath);
    (*env)->ReleaseStringUTFChars(env, dataDir, dataDirPath);

    cacheDirPath = (*env)->GetStringUTFChars(env, cacheDir, NULL);
    cache_dir = strdup(cacheDirPath);
    (*env)->ReleaseStringUTFChars(env, cacheDir, cacheDirPath);

    // TMPDIR is used by osl_getTempDirURL()
    setenv("TMPDIR", cache_dir, 1);

    fontsConfPath = malloc(strlen(data_dir) + sizeof(fontsConf));
    strcpy(fontsConfPath, data_dir);
    strcat(fontsConfPath, fontsConf);

    fd = open(fontsConfPath, O_RDONLY);
    if (fd != -1) {
        close(fd);
        LOGI("Setting FONTCONFIG_FILE to %s", fontsConfPath);
        setenv("FONTCONFIG_FILE", fontsConfPath, 1);
    }
    free(fontsConfPath);

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
    {
        LOGE("setup_cdir failed");
        return JNI_FALSE;
    }

    if (!setup_assets_tree())
    {
        LOGE("setup_assets_tree failed");
        return JNI_FALSE;
    }

    LOGI("LibreOfficeKit: libreofficekit_initialize finished");

    return JNI_TRUE;
}

/// Initialize the LibreOfficeKit.
__attribute__ ((visibility("default")))
jboolean Java_org_libreoffice_kit_LibreOfficeKit_initializeNative
    (JNIEnv* env, jobject clazz,
     jstring dataDir, jstring cacheDir, jstring apkFile, jobject assetManager)
{
    const char program_dir[] = "/program";
    size_t data_dir_len;

    (void) clazz;
    libreofficekit_initialize(env, dataDir, cacheDir, apkFile, assetManager);

    // LibreOfficeKit expects a path to the program/ directory
    free(full_program_dir);
    data_dir_len = strlen(data_dir);
    full_program_dir = malloc(data_dir_len + sizeof(program_dir));

    strncpy(full_program_dir, data_dir, data_dir_len);
    strncpy(full_program_dir + data_dir_len, program_dir, sizeof(program_dir));

    // Initialize LibreOfficeKit
    if (!libreofficekit_hook(full_program_dir))
    {
        LOGE("libreofficekit_hook returned null");
        return JNI_FALSE;
    }

    LOGI("LibreOfficeKit successfully initialized");

    return JNI_TRUE;
}

__attribute__ ((visibility("default")))
jobject Java_org_libreoffice_kit_LibreOfficeKit_getLibreOfficeKitHandle
    (JNIEnv* env, jobject clazz)
{
    LibreOfficeKit* aOffice;

    (void) env;
    (void) clazz;

    aOffice = libreofficekit_hook(full_program_dir);

    return (*env)->NewDirectByteBuffer(env, (void*) aOffice, sizeof(LibreOfficeKit));
}

__attribute__ ((visibility("default")))
AAssetManager *
lo_get_native_assetmgr(void)
{
    return native_asset_manager;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
