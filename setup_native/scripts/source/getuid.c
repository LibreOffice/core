/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

#define UNUSED(x) (void)(x)

#ifdef _cplusplus
extern "C" {
#endif

#ifdef __sun

#include <sys/systeminfo.h>
#include <strings.h>

int   chown  (const char *path, uid_t owner, gid_t group) {return 0;}
int   lchown (const char *path, uid_t owner, gid_t group) {return 0;}
int   fchown (int fildes, uid_t owner, gid_t group)       {return 0;}

uid_t getuid  (void) {return 0;}
int stat(const char *path,  struct stat *buf);
#ifdef __notdef__
uid_t geteuid (void) {return 0;}
gid_t getgid  (void) {return 0;}
gid_t getegid (void) {return 0;}
#endif

int   setuid  (uid_t p)  {return 0;}
int   setgid  (gid_t p)  {return 0;}

/* This is to fool cpio and pkgmk */
int fstat(int fildes, struct stat *buf)
{
    int ret = 0;
    static int (*p_fstat) (int fildes, struct stat *buf) = NULL;
    if (p_fstat == NULL)
        p_fstat = (int (*)(int fildes, struct stat *buf))
            dlsym (RTLD_NEXT, "fstat");
    ret = (*p_fstat)(fildes, buf);
    if (buf != NULL)
    {
        buf->st_uid = 0; /* root */
        buf->st_gid = 2; /* bin */
    }

    return ret;
}

/* this is to fool mkdir, don't allow to remove owner execute right from directories */
int chmod(const char *path, mode_t mode)
{
    int ret = 0;
    static int (*p_chmod) (const char *path, mode_t mode) = NULL;
    if (p_chmod == NULL)
        p_chmod = (int (*)(const char *path, mode_t mode))
            dlsym (RTLD_NEXT, "chmod");

    if ((mode & S_IXUSR) == 0)
    {
        struct stat statbuf;
        if (stat(path, &statbuf) == 0)
        {
            if ((statbuf.st_mode & S_IFDIR) != 0)
                mode = (mode | S_IXUSR);
        }
    }

    ret = (*p_chmod)(path, mode);
    return ret;
}



/* This is to fool tar */
int fstatat64(int fildes, const char *path, struct stat64  *buf, int flag)
{
    int ret = 0;
    static int (*p_fstatat) (int fildes, const char *path, struct stat64 *buf, int flag) = NULL;
    if (p_fstatat == NULL)
        p_fstatat = (int (*)(int fildes, const char *path, struct stat64 *buf, int flag))
            dlsym (RTLD_NEXT, "fstatat64");
    ret = (*p_fstatat)(fildes, path, buf, flag);
    if (buf != NULL)
    {
        buf->st_uid = 0; /* root */
        buf->st_gid = 2; /* bin */
    }

    return ret;
}
#elif  defined LINUX

uid_t getuid  (void) {return 0;}
uid_t geteuid (void) {return 0;}
int setgid  (gid_t p) { UNUSED(p); return 0; }

/* This is to fool tar */
#ifdef X86_64
int __lxstat(int n, const char *path, struct stat *buf)
{
    int ret = 0;
    static int (*p_lstat) (int n, const char *path, struct stat *buf) = NULL;
    if (p_lstat == NULL)
        p_lstat = (int (*)(int n, const char *path, struct stat *buf))
            dlsym (RTLD_NEXT, "__lxstat");
    ret = (*p_lstat)(n, path, buf);
#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
    // __lxstat may be declared in system headers as taking nonnull argument
#endif
    assert(buf != NULL);
#if defined __GNUC__
#pragma GCC diagnostic pop
#endif
    buf->st_uid = 0; /* root */
    buf->st_gid = 0; /* root */
    return ret;
}
#else
int __lxstat64(int n, const char *path, struct stat64 *buf)
{
    int ret = 0;
    static int (*p_lstat) (int n, const char *path, struct stat64 *buf) = NULL;
    if (p_lstat == NULL)
        p_lstat = (int (*)(int n, const char *path, struct stat64 *buf))
            dlsym (RTLD_NEXT, "__lxstat64");
    ret = (*p_lstat)(n, path, buf);
    assert(buf != NULL);
    buf->st_uid = 0;
    buf->st_gid = 0;
    return ret;
}
#endif
#endif

#ifdef _cplusplus
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
