/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OSL_DETAIL_ANDROID_BOOTSTRAP_H
#define INCLUDED_OSL_DETAIL_ANDROID_BOOTSTRAP_H

#if defined(ANDROID)

#include <jni.h>
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <osl/detail/component-mapping.h>
#include <android/asset_manager.h>

typedef struct lo_apk_dir lo_apk_dir;

void *lo_apkentry(const char *filename,
                  size_t *size);

lo_apk_dir *lo_apk_opendir(const char *dirname);

struct dirent *lo_apk_readdir(lo_apk_dir *dirp);

int lo_apk_closedir(lo_apk_dir *dirp);

int lo_apk_lstat(const char *path, struct stat *statp);

/// "libreofficekit_" prefix, because it is exported from the .so, when we are
/// initializing the JNI externally.
void libreofficekit_set_javavm(JavaVM *vm);

JavaVM *lo_get_javavm(void);

const char *lo_get_app_data_dir(void);

AAssetManager *lo_get_native_assetmgr(void);

int setup_cdir(void);
int setup_assets_tree(void);

#ifdef __cplusplus
}
#endif

#endif // ANDROID

#endif // ANDROID_BOOTSTRAP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
