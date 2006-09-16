/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fstat.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:53:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#if defined( WIN)
#include <stdio.h>
#include <dos.h>
#endif

#ifdef UNX
#include <errno.h>
#endif

#include <limits.h>
#include <string.h>

#include "comdep.hxx"
#include <fsys.hxx>

/*************************************************************************
|*
|*    FileStat::FileStat()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 11.06.91
|*    Letzte Aenderung  MI 11.06.91
|*
*************************************************************************/

FileStat::FileStat()
:   // don't use Default-Ctors!
    aDateCreated( ULONG(0) ),
    aTimeCreated( ULONG(0) ),
    aDateModified( ULONG(0) ),
    aTimeModified( ULONG(0) ),
    aDateAccessed( ULONG(0) ),
    aTimeAccessed( ULONG(0) )
{
    nSize = 0;
    nKindFlags = FSYS_KIND_UNKNOWN;
    nError = FSYS_ERR_OK;
}

/*************************************************************************
|*
|*    FileStat::FileStat()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 11.06.91
|*    Letzte Aenderung  MI 11.06.91
|*
*************************************************************************/

FileStat::FileStat( const DirEntry& rDirEntry, FSysAccess nAccess )
:   // don't use Default-Ctors!
    aDateCreated( ULONG(0) ),
    aTimeCreated( ULONG(0) ),
    aDateModified( ULONG(0) ),
    aTimeModified( ULONG(0) ),
    aDateAccessed( ULONG(0) ),
    aTimeAccessed( ULONG(0) )
{
    BOOL bCached = FSYS_ACCESS_CACHED == (nAccess & FSYS_ACCESS_CACHED);
    BOOL bFloppy = FSYS_ACCESS_FLOPPY == (nAccess & FSYS_ACCESS_FLOPPY);

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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MA 11.11.91
|*    Letzte Aenderung  MA 11.11.91
|*
*************************************************************************/

// TRUE  wenn die Instanz j"unger als rIsOlder ist.
// FALSE wenn die Instanz "alter oder gleich alt wie rIsOlder ist.

BOOL FileStat::IsYounger( const FileStat& rIsOlder ) const
{
    if ( aDateModified > rIsOlder.aDateModified )
        return TRUE;
    if ( ( aDateModified == rIsOlder.aDateModified ) &&
         ( aTimeModified > rIsOlder.aTimeModified ) )
        return TRUE;

    return FALSE;
}

/*************************************************************************
|*
|*    FileStat::IsKind()
|*
|*    Ersterstellung    MA 11.11.91 (?)
|*    Letzte Aenderung  KH 16.01.95
|*
*************************************************************************/

BOOL FileStat::IsKind( DirEntryKind nKind ) const
{
    BOOL bRet = ( ( nKind == FSYS_KIND_UNKNOWN ) &&
                  ( nKindFlags == FSYS_KIND_UNKNOWN ) ) ||
                   ( ( nKindFlags & nKind ) == nKind );
    return bRet;
}

/*************************************************************************
|*
|*    FileStat::HasReadOnlyFlag()
|*
|*    Ersterstellung    MI 06.03.97
|*    Letzte Aenderung  UT 01.07.98
|*
*************************************************************************/

BOOL FileStat::HasReadOnlyFlag()
{
#if defined WNT || defined UNX
    return TRUE;
#else
    return FALSE;
#endif
}

/*************************************************************************
|*
|*    FileStat::GetReadOnlyFlag()
|*
|*    Ersterstellung    MI 06.03.97
|*    Letzte Aenderung  UT 02.07.98
|*
*************************************************************************/

BOOL FileStat::GetReadOnlyFlag( const DirEntry &rEntry )
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
        return FALSE;
    /* jupp, is writable for user? */
    return((aBuf.st_mode & S_IWUSR) != S_IWUSR);
#else
    return FALSE;
#endif
}

/*************************************************************************
|*
|*    FileStat::SetReadOnlyFlag()
|*
|*    Ersterstellung    MI 06.03.97
|*    Letzte Aenderung  UT 01.07.98
|*
*************************************************************************/

ULONG FileStat::SetReadOnlyFlag( const DirEntry &rEntry, BOOL bRO )
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
|*    Ersterstellung    PB  27.06.97
|*    Letzte Aenderung
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
#endif
