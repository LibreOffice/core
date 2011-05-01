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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

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

FileStat::FileStat()
:   // don't use Default-Ctors!
    aDateCreated( sal_uIntPtr(0) ),
    aTimeCreated( sal_uIntPtr(0) ),
    aDateModified( sal_uIntPtr(0) ),
    aTimeModified( sal_uIntPtr(0) ),
    aDateAccessed( sal_uIntPtr(0) ),
    aTimeAccessed( sal_uIntPtr(0) )
{
    nSize = 0;
    nKindFlags = FSYS_KIND_UNKNOWN;
    nError = FSYS_ERR_OK;
}

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
|*    FileStat::IsYounger()
|*
*************************************************************************/

// sal_True  wenn die Instanz j"unger als rIsOlder ist.
// sal_False wenn die Instanz "alter oder gleich alt wie rIsOlder ist.

sal_Bool FileStat::IsYounger( const FileStat& rIsOlder ) const
{
    if ( aDateModified > rIsOlder.aDateModified )
        return sal_True;
    if ( ( aDateModified == rIsOlder.aDateModified ) &&
         ( aTimeModified > rIsOlder.aTimeModified ) )
        return sal_True;

    return sal_False;
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

    ByteString aFPath(rEntry.GetFull(), osl_getThreadTextEncoding());
#if defined WNT
    DWORD nRes = GetFileAttributes( (LPCTSTR) aFPath.GetBuffer() );
    return ULONG_MAX != nRes &&
           ( FILE_ATTRIBUTE_READONLY & nRes ) == FILE_ATTRIBUTE_READONLY;
#elif defined UNX
    /* could we stat the object? */
    struct stat aBuf;
    if (stat(aFPath.GetBuffer(), &aBuf))
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

    ByteString aFPath(rEntry.GetFull(), osl_getThreadTextEncoding());

#if defined WNT
    DWORD nRes = GetFileAttributes( (LPCTSTR) aFPath.GetBuffer() );
    if ( ULONG_MAX != nRes )
        nRes = SetFileAttributes( (LPCTSTR) aFPath.GetBuffer(),
                    ( nRes & ~FILE_ATTRIBUTE_READONLY ) |
                    ( bRO ? FILE_ATTRIBUTE_READONLY : 0 ) );
    return ( ULONG_MAX == nRes ) ? ERRCODE_IO_UNKNOWN : 0;
#elif defined UNX
    /* first, stat the object to get permissions */
    struct stat aBuf;
    if (stat(aFPath.GetBuffer(), &aBuf))
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
    if (chmod(aFPath.GetBuffer(), nMode))
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

/*************************************************************************
|*
|*    FileStat::SetDateTime
|*
*************************************************************************/
#if defined WNT

void FileStat::SetDateTime( const String& rFileName,
                            const DateTime& rNewDateTime )
{
    ByteString aFileName(rFileName, osl_getThreadTextEncoding());

    Date aNewDate = rNewDateTime;
    Time aNewTime = rNewDateTime;

    TIME_ZONE_INFORMATION aTZI;
    DWORD dwTZI = GetTimeZoneInformation( &aTZI );

    if ( dwTZI != (DWORD)-1 && dwTZI != TIME_ZONE_ID_UNKNOWN )
    {
        // 1. Korrektur der Zeitzone
        LONG nDiff = aTZI.Bias;
        Time aOldTime = aNewTime; // alte Zeit merken

        // 2. evt. Korrektur Sommer-/Winterzeit
        if ( dwTZI == TIME_ZONE_ID_DAYLIGHT )
            nDiff += aTZI.DaylightBias;

        Time aDiff( abs( nDiff / 60 /*Min -> Std*/ ), 0 );

        if ( nDiff > 0 )
        {
            aNewTime += aDiff;                  // Stundenkorrektur

            // bei "Uberlauf korrigieren
            if ( aNewTime >= Time( 24, 0 ) )
                aNewTime -= Time( 24, 0 );

            // Tages"uberlauf?
            if ( aOldTime == Time( 0, 0 ) ||    // 00:00 -> 01:00
                 aNewTime < aOldTime )          // 23:00 -> 00:00 | 01:00 ...
                aNewDate++;
        }
        else if ( nDiff < 0 )
        {
            aNewTime -= aDiff;                  // Stundenkorrektur

            // negative Zeit (-1:00) korrigieren: 23:00
            if (aNewTime < Time( 0, 0 ) )
                aNewTime += Time( 24, 0 );

            // Tagesunterlauf ?
            if ( aOldTime == Time( 0, 0 ) ||    // 00:00 -> 23:00
                 aNewTime > aOldTime )          // 01:00 -> 23:00 | 22:00 ...
                aNewDate--;
        }
    }


    SYSTEMTIME aTime;
    aTime.wYear = aNewDate.GetYear();
    aTime.wMonth = aNewDate.GetMonth();
    aTime.wDayOfWeek = 0;
    aTime.wDay = aNewDate.GetDay();
    aTime.wHour = aNewTime.GetHour();
    aTime.wMinute = aNewTime.GetMin();
    aTime.wSecond = aNewTime.GetSec();
    aTime.wMilliseconds = 0;
    FILETIME aFileTime;
    SystemTimeToFileTime( &aTime, &aFileTime );

    HANDLE hFile = CreateFile( aFileName.GetBuffer(), GENERIC_WRITE, 0, 0,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );

    if ( hFile != INVALID_HANDLE_VALUE )
    {
        SetFileTime( hFile, &aFileTime, &aFileTime, &aFileTime );
        CloseHandle( hFile );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
