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

#ifdef UNX
#include <errno.h>
#endif

#include <limits.h>
#include <string.h>

#include "comdep.hxx"
#include <tools/fsys.hxx>

FileStat::FileStat( const DirEntry& rDirEntry, FSysAccess nAccess )
:   // don't use Default-Ctors!
    aDateCreated( sal_uIntPtr(0) ),
    aTimeCreated( sal_uIntPtr(0) ),
    aDateModified( sal_uIntPtr(0) ),
    aTimeModified( sal_uIntPtr(0) ),
    aDateAccessed( sal_uIntPtr(0) ),
    aTimeAccessed( sal_uIntPtr(0) )
{
    sal_Bool bCached = FSYS_ACCESS_CACHED == (nAccess & FSYS_ACCESS_CACHED);
    sal_Bool bFloppy = FSYS_ACCESS_FLOPPY == (nAccess & FSYS_ACCESS_FLOPPY);

#ifdef FEAT_FSYS_DOUBLESPEED
    const FileStat *pStatFromDir = bCached ? rDirEntry.ImpGetStat() : 0;
    if ( pStatFromDir )
    {
        nError = pStatFromDir->nError;
        nKindFlags = pStatFromDir->nKindFlags;
        nSize = pStatFromDir->nSize;
        aCreator = pStatFromDir->aCreator;
        aType = pStatFromDir->aType;
        aDateCreated = pStatFromDir->aDateCreated;
        aTimeCreated = pStatFromDir->aTimeCreated;
        aDateModified = pStatFromDir->aDateModified;
        aTimeModified = pStatFromDir->aTimeModified;
        aDateAccessed = pStatFromDir->aDateAccessed;
        aTimeAccessed = pStatFromDir->aTimeAccessed;
    }
    else
#endif
        Update( rDirEntry, bFloppy );
}

sal_Bool FileStat::IsKind( DirEntryKind nKind ) const
{
    sal_Bool bRet = ( ( nKind == FSYS_KIND_UNKNOWN ) &&
                  ( nKindFlags == FSYS_KIND_UNKNOWN ) ) ||
                   ( ( nKindFlags & nKind ) == nKind );
    return bRet;
}

sal_Bool FileStat::GetReadOnlyFlag( const DirEntry &rEntry )
{
    rtl::OString aFPath(rtl::OUStringToOString(rEntry.GetFull(), osl_getThreadTextEncoding()));
#if defined WNT
    DWORD nRes = GetFileAttributes( (LPCTSTR) aFPath.getStr() );
    return ULONG_MAX != nRes &&
           ( FILE_ATTRIBUTE_READONLY & nRes ) == FILE_ATTRIBUTE_READONLY;
#elif defined UNX
    /* could we stat the object? */
    struct stat aBuf;
    if (stat(aFPath.getStr(), &aBuf))
        return sal_False;
    /* jupp, is writable for user? */
    return((aBuf.st_mode & S_IWUSR) != S_IWUSR);
#else
    return sal_False;
#endif
}

sal_uIntPtr FileStat::SetReadOnlyFlag( const DirEntry &rEntry, sal_Bool bRO )
{

    rtl::OString aFPath(rtl::OUStringToOString(rEntry.GetFull(), osl_getThreadTextEncoding()));

#if defined WNT
    DWORD nRes = GetFileAttributes( (LPCTSTR) aFPath.getStr() );
    if ( ULONG_MAX != nRes )
        nRes = SetFileAttributes( (LPCTSTR) aFPath.getStr(),
                    ( nRes & ~FILE_ATTRIBUTE_READONLY ) |
                    ( bRO ? FILE_ATTRIBUTE_READONLY : 0 ) );
    return ( ULONG_MAX == nRes ) ? ERRCODE_IO_UNKNOWN : 0;
#elif defined UNX
    /* first, stat the object to get permissions */
    struct stat aBuf;
    if (stat(aFPath.getStr(), &aBuf))
        return ERRCODE_IO_NOTEXISTS;
    /* set or clear write bit for user */
    mode_t nMode;
    if (bRO)
    {
        nMode = aBuf.st_mode & ~S_IWUSR;
        nMode = aBuf.st_mode & ~S_IWGRP;
        nMode = aBuf.st_mode & ~S_IWOTH;
    }
    else
        nMode = aBuf.st_mode | S_IWUSR;
    /* change it on fs */
    if (chmod(aFPath.getStr(), nMode))
    {
        switch (errno)
        {
            case EPERM :
            case EROFS :
                return ERRCODE_IO_ACCESSDENIED;
            default    :
                return ERRCODE_IO_NOTEXISTS;
        }
    }
    else
        return ERRCODE_NONE;
#else
    return ERRCODE_IO_NOTSUPPORTED;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
