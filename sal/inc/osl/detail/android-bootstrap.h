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

#ifndef ANDROID_BOOSTRAP_H
#define ANDROID_BOOSTRAP_H

#if defined(ANDROID)

#include <jni.h>
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <osl/detail/component-mapping.h>

typedef struct lo_apk_dir lo_apk_dir;

void *lo_apkentry(const char *filename,
                  size_t *size);

lo_apk_dir *lo_apk_opendir(const char *dirname);

struct dirent *lo_apk_readdir(lo_apk_dir *dirp);

int lo_apk_closedir(lo_apk_dir *dirp);

int lo_apk_lstat(const char *path, struct stat *statp);

int lo_dlcall_argc_argv(void *function,
                        int argc,
                        const char **argv);

JavaVM *lo_get_javavm(void);

struct android_app *lo_get_app(void);

#ifdef __cplusplus
}
#endif

#endif // ANDROID

#endif // ANDROID_BOOTSTRAP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
