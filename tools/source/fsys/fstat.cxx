/*************************************************************************
 *
 *  $RCSfile: fstat.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#if defined(WNT) || defined(OS2) || defined(UNX)
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
#ifdef WNT
    DWORD nRes = GetFileAttributes( (LPCTSTR) aFPath.GetBuffer() );
    return ULONG_MAX != nRes &&
           ( FILE_ATTRIBUTE_READONLY & nRes ) == FILE_ATTRIBUTE_READONLY;
#endif

#ifdef OS2
    FILESTATUS3 aFileStat;
    APIRET nRet = DosQueryPathInfo( (PSZ)aFPath.GetBuffer(), 1, &aFileStat, sizeof(aFileStat) );
    switch ( nRet )
    {
        case NO_ERROR:
            return FILE_READONLY == ( aFileStat.attrFile & FILE_READONLY );

        case ERROR_SHARING_VIOLATION:
            return ERRCODE_IO_LOCKVIOLATION;

        default:
            return ERRCODE_IO_NOTEXISTS;
    }
#endif

#ifdef UNX
    /* could we stat the object? */
    struct stat aBuf;
    if (stat(aFPath.GetBuffer(), &aBuf))
        return FALSE;
    /* jupp, is writable for user? */
    return((aBuf.st_mode & S_IWUSR) != S_IWUSR);
#endif
    return FALSE;
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

#ifdef WNT
    DWORD nRes = GetFileAttributes( (LPCTSTR) aFPath.GetBuffer() );
    if ( ULONG_MAX != nRes )
        nRes = SetFileAttributes( (LPCTSTR) aFPath.GetBuffer(),
                    ( nRes & ~FILE_ATTRIBUTE_READONLY ) |
                    ( bRO ? FILE_ATTRIBUTE_READONLY : 0 ) );
    return ( ULONG_MAX == nRes ) ? ERRCODE_IO_UNKNOWN : 0;
#endif

#ifdef OS2
    FILESTATUS3 aFileStat;
    APIRET nRet = DosQueryPathInfo( (PSZ)aFPath.GetBuffer(), 1, &aFileStat, sizeof(aFileStat) );
    if ( !nRet )
    {
        aFileStat.attrFile = ( aFileStat.attrFile & ~FILE_READONLY ) |
                             ( bRO ? FILE_READONLY : 0 );
        nRet = DosSetPathInfo( (PSZ)aFPath.GetBuffer(), 1, &aFileStat, sizeof(aFileStat), 0 );
    }
    switch ( nRet )
    {
        case NO_ERROR:
            return ERRCODE_NONE;

        case ERROR_SHARING_VIOLATION:
            return ERRCODE_IO_LOCKVIOLATION;

        default:
            return ERRCODE_IO_NOTEXISTS;
    }
#endif

#ifdef UNX
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
#endif
    return ERRCODE_IO_NOTSUPPORTED;
}

/*************************************************************************
|*
|*    FileStat::SetDateTime
|*
|*    Ersterstellung    PB  27.06.97
|*    Letzte Aenderung
|*
*************************************************************************/
#if defined(WIN) | defined(WNT) | defined(OS2)

void FileStat::SetDateTime( const String& rFileName,
                            const DateTime& rNewDateTime )
{
    ByteString aFileName(rFileName, osl_getThreadTextEncoding());

    Date aNewDate = rNewDateTime;
    Time aNewTime = rNewDateTime;
#if defined(WIN)
    unsigned date = 0;
    unsigned time = 0;

    date  = (unsigned) aNewDate.GetDay();
    date |= (unsigned)(aNewDate.GetMonth() << 5);
    date |= (unsigned)((aNewDate.GetYear() -  1980) << 9);
    time  = (unsigned)(aNewTime.GetSec() / 2);
    time |= (unsigned)(aNewTime.GetMin() << 5);
    time |= (unsigned)(aNewTime.GetHour() << 11);


    FILE* pFile = fopen( aFileName.GetBuffer(), "a" );

    if ( pFile != NULL )
    {
        _dos_setftime( fileno(pFile), date, time );
        fclose( pFile );
    }

#elif defined( WNT )

    TIME_ZONE_INFORMATION aTZI;
    DWORD dwTZI = GetTimeZoneInformation( &aTZI );

    if ( dwTZI != (DWORD)-1 && dwTZI != TIME_ZONE_ID_UNKNOWN )
    {
        // 1. Korrektur der Zeitzone
        short nDiff = (short)aTZI.Bias;
        Time aOldTime = aNewTime; // alte Zeit merken

        // 2. evt. Korrektur Sommer-/Winterzeit
        if ( dwTZI == TIME_ZONE_ID_DAYLIGHT )
            nDiff += (short)aTZI.DaylightBias;

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

#endif
#ifdef OS2
    // open file
    ULONG nAction = FILE_EXISTED;
    HFILE hFile = 0;
    ULONG nFlags = OPEN_FLAGS_WRITE_THROUGH |
                   OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_CACHE   |
                   OPEN_FLAGS_RANDOM        | OPEN_FLAGS_NOINHERIT  |
                   OPEN_SHARE_DENYNONE      | OPEN_ACCESS_READWRITE;

    APIRET nRet = DosOpen((PSZ)aFileName.GetBuffer(), &hFile, (PULONG)&nAction,
                          0/*size*/, FILE_NORMAL,
                          OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                          nFlags, 0/*ea*/);

    if ( nRet == 0 )
    {
        FILESTATUS3 FileInfoBuffer;

        nRet = DosQueryFileInfo(
            hFile, 1, &FileInfoBuffer, sizeof(FileInfoBuffer));

        if ( nRet == 0 )
        {
            FDATE aNewDate;
            FTIME aNewTime;

             // create date and time words
            aNewDate.day     = rNewDateTime.GetDay();
            aNewDate.month   = rNewDateTime.GetMonth();
            aNewDate.year    = rNewDateTime.GetYear() - 1980;
            aNewTime.twosecs = rNewDateTime.GetSec() / 2;
            aNewTime.minutes = rNewDateTime.GetMin();
            aNewTime.hours   = rNewDateTime.GetHour();

            // set file date and time
            FileInfoBuffer.fdateCreation   = aNewDate;
            FileInfoBuffer.ftimeCreation   = aNewTime;
            FileInfoBuffer.fdateLastAccess = aNewDate;
            FileInfoBuffer.ftimeLastAccess = aNewTime;
            FileInfoBuffer.fdateLastWrite  = aNewDate;
            FileInfoBuffer.ftimeLastWrite  = aNewTime;

            DosSetFileInfo(hFile, 1, &FileInfoBuffer, sizeof(FileInfoBuffer));
        }
        DosClose(hFile);
    }
#endif
}

#endif
/*
FileStatMembers *FileStat::GetAllMembers()
{
    FileStatMembers *members = new FileStatMembers;
    members->nError = nError;
    members->nKindFlags = nKindFlags;
    members->nSize = nSize;
    members->aCreator = aCreator;
    members->aType = aType;
    members->aDateCreated = aDateCreated.GetDate();
    members->aTimeCreated = aTimeCreated.GetTime();
    members->aDateAccessed = aDateAccessed.GetDate();
    members->aTimeAccessed = aTimeAccessed.GetTime();
    members->aDateModified = aDateModified.GetDate();
    members->aTimeModified = aTimeModified.GetTime();
    return members;
}

void FileStat::InitMembers(FileStatMembers *members)
{
    nError = members->nError;
    members->nKindFlags = nKindFlags;
    members->nSize = nSize;
    members->aCreator = aCreator;
    members->aType = aType;
    aDateCreated.SetDate(members->aDateCreated);
    aTimeCreated.SetTime(members->aTimeCreated);
    aDateAccessed.SetDate(members->aDateAccessed);
    aTimeAccessed.SetTime(members->aTimeAccessed);
    aDateModified.SetDate(members->aDateModified);
    aTimeModified.SetTime(members->aTimeModified);
}
*/


