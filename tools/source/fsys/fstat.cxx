/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifdef UNX
#include <errno.h>
#endif

#include <limits.h>
#include <string.h>

#include "comdep.hxx"
#include <tools/fsys.hxx>

/*************************************************************************
|*
|*    FileStat::FileStat()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    FileStat::IsKind()
|*
*************************************************************************/

sal_Bool FileStat::IsKind( DirEntryKind nKind ) const
{
    sal_Bool bRet = ( ( nKind == FSYS_KIND_UNKNOWN ) &&
                  ( nKindFlags == FSYS_KIND_UNKNOWN ) ) ||
                   ( ( nKindFlags & nKind ) == nKind );
    return bRet;
}

/*************************************************************************
|*
|*    FileStat::GetReadOnlyFlag()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    FileStat::SetReadOnlyFlag()
|*
*************************************************************************/

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
