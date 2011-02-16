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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#if defined LINUX
#include <mntent.h>
#define mnttab mntent
#elif defined AIX
#include <sys/mntctl.h>
#include <sys/vmount.h>
extern "C" int mntctl( int cmd, size_t size, char* buf );
#elif defined(NETBSD)
#include <sys/mount.h>
#elif defined(FREEBSD) || defined(MACOSX) || defined(OPENBSD) || \
      defined(DECUNIX) || defined(DRAGONFLY)
struct mnttab
{
  char *mnt_dir;
  char *mnt_fsname;
};
#else
#include <sys/mnttab.h>
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#include <tools/debug.hxx>
#include <tools/list.hxx>
#include <tools/fsys.hxx>
#include "comdep.hxx"
#include <rtl/instance.hxx>

#if defined SOLARIS
#define MOUNTSPECIAL mnt_special
#define MOUNTPOINT   mnt_mountp
#define MOUNTOPTS    mnt_mntopts
#define MOUNTFS      mnt_fstype
#else
#define MOUNTSPECIAL mnt_fsname
#define MOUNTPOINT   mnt_dir
#define MOUNTFS      mnt_type
#endif

struct mymnttab
{
    dev_t mountdevice;
    ByteString mountspecial;
    ByteString mountpoint;
    ByteString mymnttab_filesystem;
    mymnttab() { mountdevice = (dev_t) -1; }
};


#if defined(NETBSD) || defined(FREEBSD) || defined(MACOSX) || \
    defined(OPENBSD) || defined(DRAGONFLY)
BOOL GetMountEntry(dev_t /* dev */, struct mymnttab * /* mytab */ )
{
    DBG_WARNING( "Sorry, not implemented: GetMountEntry" );
    return FALSE;
}

#elif defined AIX
BOOL GetMountEntry(dev_t dev, struct mymnttab *mytab)
{
    int bufsize;
    if (mntctl (MCTL_QUERY, sizeof bufsize, (char*) &bufsize))
        return FALSE;

    char* buffer = (char *)malloc( bufsize * sizeof(char) );
    if (mntctl (MCTL_QUERY, bufsize, buffer) != -1)
        for ( char* vmt = buffer;
                    vmt < buffer + bufsize;
                    vmt += ((struct vmount*)vmt)->vmt_length)
        {
            struct stat buf;
            char *mountp = vmt2dataptr((struct vmount*)vmt, VMT_STUB);
            if ((stat (mountp, &buf) != -1) && (buf.st_dev == dev))
            {
                mytab->mountpoint = mountp;
                mytab->mountspecial
                        = vmt2dataptr((struct vmount*)vmt, VMT_HOSTNAME);
                if (mytab->mountspecial.Len())
                    mytab->mountspecial += ':';
                mytab->mountspecial
                        += vmt2dataptr((struct vmount*)vmt, VMT_OBJECT);
                mytab->mountdevice = dev;
                free( buffer );
                return TRUE;
            }
        }
    free( buffer );
    return FALSE;
}

#else


static BOOL GetMountEntry(dev_t dev, struct mymnttab *mytab)
{
#if defined SOLARIS
    FILE *fp = fopen (MNTTAB, "r");
    if (! fp)
        return FALSE;
    struct mnttab mnt[1];
    while (getmntent (fp, mnt) != -1)
#elif defined DECUNIX || defined AIX
    FILE *fp = NULL;
    if (! fp)
        return FALSE;
    struct mnttab mnt[1];
    while ( 0 )
#else
    FILE *fp = setmntent (MOUNTED, "r");
    if (! fp)
        return FALSE;
    struct mnttab *mnt;
    while ((mnt = getmntent (fp)) != NULL)
#endif
    {
#ifdef SOLARIS
        char *devopt = NULL;
        if ( mnt->MOUNTOPTS != NULL )
            devopt = strstr (mnt->MOUNTOPTS, "dev=");
        if (devopt)
        {
            if (dev != (dev_t) strtoul (devopt+4, NULL, 16))
                continue;
        }
        else
#endif
        {
            struct stat buf;
            if ((stat (mnt->MOUNTPOINT, &buf) == -1) || (buf.st_dev != dev))
                continue;
        }
#       ifdef LINUX
        /* #61624# File mit setmntent oeffnen und mit fclose schliessen stoesst
           bei der glibc-2.1 auf wenig Gegenliebe */
        endmntent( fp );
#       else
        fclose (fp);
#       endif
        mytab->mountspecial = mnt->MOUNTSPECIAL;
        mytab->mountpoint   = mnt->MOUNTPOINT;
        mytab->mountdevice  = dev;
        mytab->mymnttab_filesystem = mnt->MOUNTFS;

        return TRUE;
    }
#   ifdef LINUX
    /* #61624# dito */
    endmntent( fp );
#   else
    fclose (fp);
#   endif
    return FALSE;
}

#endif

/************************************************************************
|*
|*    DirEntry::IsCaseSensitive()
|*
*************************************************************************/

BOOL DirEntry::IsCaseSensitive( FSysPathStyle eFormatter ) const
{

    if (eFormatter==FSYS_STYLE_HOST)
    {
#if defined(NETBSD) || defined(DRAGONFLY)
        return TRUE;
#else
        struct stat buf;
        DirEntry aPath(*this);
        aPath.ToAbs();

        while (stat (ByteString(aPath.GetFull(), osl_getThreadTextEncoding()).GetBuffer(), &buf))
        {
            if (aPath.Level() == 1)
            {
                return TRUE;    // ich bin unter UNIX, also ist der default im Zweifelsfall case sensitiv
            }
            aPath = aPath [1];
        }

        struct mymnttab fsmnt;
        GetMountEntry(buf.st_dev, &fsmnt);
        if ((fsmnt.mymnttab_filesystem.CompareTo("msdos")==COMPARE_EQUAL) ||
            (fsmnt.mymnttab_filesystem.CompareTo("umsdos")==COMPARE_EQUAL) ||
            (fsmnt.mymnttab_filesystem.CompareTo("vfat")==COMPARE_EQUAL) ||
            (fsmnt.mymnttab_filesystem.CompareTo("hpfs")==COMPARE_EQUAL) ||
            (fsmnt.mymnttab_filesystem.CompareTo("smb") ==COMPARE_EQUAL) ||
            (fsmnt.mymnttab_filesystem.CompareTo("ncpfs")==COMPARE_EQUAL))
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
#endif
    }
    else
    {
        BOOL isCaseSensitive = TRUE;    // ich bin unter UNIX, also ist der default im Zweifelsfall case sensitiv
        switch ( eFormatter )
        {
            case FSYS_STYLE_MAC:
            case FSYS_STYLE_FAT:
            case FSYS_STYLE_VFAT:
            case FSYS_STYLE_NTFS:
            case FSYS_STYLE_NWFS:
            case FSYS_STYLE_HPFS:
                {
                    isCaseSensitive = FALSE;
                    break;
                }
            case FSYS_STYLE_SYSV:
            case FSYS_STYLE_BSD:
            case FSYS_STYLE_DETECT:
                {
                    isCaseSensitive = TRUE;
                    break;
                }
            default:
                {
                    isCaseSensitive = TRUE; // ich bin unter UNIX, also ist der default im Zweifelsfall case sensitiv
                    break;
                }
        }
        return isCaseSensitive;
    }
}

/************************************************************************
|*
|*    DirEntry::ToAbs()
|*
*************************************************************************/

BOOL DirEntry::ToAbs()
{
    if ( FSYS_FLAG_VOLUME == eFlag )
    {
        eFlag = FSYS_FLAG_ABSROOT;
        return TRUE;
    }

    if ( IsAbs() )
      return TRUE;

    char sBuf[MAXPATHLEN + 1];
    *this = DirEntry( String( getcwd( sBuf, MAXPATHLEN ), osl_getThreadTextEncoding() ) ) + *this;
    return IsAbs();
}

/*************************************************************************
|*
|*    DirEntry::GetVolume()
|*
*************************************************************************/

namespace { struct mymnt : public rtl::Static< mymnttab, mymnt > {}; }

String DirEntry::GetVolume() const
{
  DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry aPath( *this );
    aPath.ToAbs();

    struct stat buf;
    while (stat (ByteString(aPath.GetFull(), osl_getThreadTextEncoding()).GetBuffer(), &buf))
    {
        if (aPath.Level() <= 1)
            return String();
        aPath = aPath [1];
    }
    mymnttab &rMnt = mymnt::get();
    return ((buf.st_dev == rMnt.mountdevice ||
                GetMountEntry(buf.st_dev, &rMnt)) ?
                    String(rMnt.mountspecial, osl_getThreadTextEncoding()) :
                    String());
}

DirEntry DirEntry::GetDevice() const
{
  DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry aPath( *this );
    aPath.ToAbs();

    struct stat buf;
    while (stat (ByteString(aPath.GetFull(), osl_getThreadTextEncoding()).GetBuffer(), &buf))
    {
        if (aPath.Level() <= 1)
            return String();
        aPath = aPath [1];
    }
    mymnttab &rMnt = mymnt::get();
    return ((buf.st_dev == rMnt.mountdevice ||
                GetMountEntry(buf.st_dev, &rMnt)) ?
                    String( rMnt.mountpoint, osl_getThreadTextEncoding()) :
                    String());
}

/*************************************************************************
|*
|*    DirEntry::SetCWD()
|*
*************************************************************************/

BOOL DirEntry::SetCWD( BOOL bSloppy ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );


    ByteString aPath( GetFull(), osl_getThreadTextEncoding());
    if ( !chdir( aPath.GetBuffer() ) )
    {
        return TRUE;
    }
    else
    {
        if ( bSloppy && !chdir(aPath.GetBuffer()) )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

//-------------------------------------------------------------------------

USHORT DirReader_Impl::Init()
{
    return 0;
}

//-------------------------------------------------------------------------

USHORT DirReader_Impl::Read()
{
    if (!pDosDir)
    {
        pDosDir = opendir( (char*) ByteString(aPath, osl_getThreadTextEncoding()).GetBuffer() );
    }

    if (!pDosDir)
    {
        bReady = TRUE;
        return 0;
    }

    // Directories und Files auflisten?
    if ( ( pDir->eAttrMask & FSYS_KIND_DIR || pDir->eAttrMask & FSYS_KIND_FILE ) &&
         ( ( pDosEntry = readdir( pDosDir ) ) != NULL ) )
    {
    String aD_Name(pDosEntry->d_name, osl_getThreadTextEncoding());
        if ( pDir->aNameMask.Matches( aD_Name  ) )
        {
            DirEntryFlag eFlag =
                    0 == strcmp( pDosEntry->d_name, "." ) ? FSYS_FLAG_CURRENT
                :   0 == strcmp( pDosEntry->d_name, ".." ) ? FSYS_FLAG_PARENT
                :   FSYS_FLAG_NORMAL;
            DirEntry *pTemp = new DirEntry( ByteString(pDosEntry->d_name), eFlag, FSYS_STYLE_UNX );
            if ( pParent )
                pTemp->ImpChangeParent( new DirEntry( *pParent ), FALSE);
            FileStat aStat( *pTemp );
            if ( ( ( ( pDir->eAttrMask & FSYS_KIND_DIR ) &&
                     ( aStat.IsKind( FSYS_KIND_DIR ) ) ) ||
                   ( ( pDir->eAttrMask & FSYS_KIND_FILE ) &&
                     !( aStat.IsKind( FSYS_KIND_DIR ) ) ) ) &&
                 !( pDir->eAttrMask & FSYS_KIND_VISIBLE &&
                    pDosEntry->d_name[0] == '.' ) )
            {
                if ( pDir->pStatLst ) //Status fuer Sort gewuenscht?
                    pDir->ImpSortedInsert( pTemp, new FileStat( aStat ) );
                else
                    pDir->ImpSortedInsert( pTemp, NULL );;
                return 1;
            }
            else
                delete pTemp;
        }
    }
    else
        bReady = TRUE;
    return 0;
}

/*************************************************************************
|*
|*    FileStat::FileStat()
|*
*************************************************************************/

FileStat::FileStat( const void *, const void * ):
    aDateCreated(0),
    aTimeCreated(0),
    aDateModified(0),
    aTimeModified(0),
    aDateAccessed(0),
    aTimeAccessed(0)
{
}

/*************************************************************************
|*
|*    FileStat::Update()
|*
*************************************************************************/
BOOL FileStat::Update( const DirEntry& rDirEntry, BOOL )
{

    nSize = 0;
    nKindFlags = 0;
    aCreator.Erase();
    aType.Erase();
    aDateCreated = Date(0);
    aTimeCreated = Time(0);
    aDateModified = Date(0);
    aTimeModified = Time(0);
    aDateAccessed = Date(0);
    aTimeAccessed = Time(0);

    if ( !rDirEntry.IsValid() )
    {
        nError = FSYS_ERR_NOTEXISTS;
        return FALSE;
    }

    // Sonderbehandlung falls es sich um eine Root handelt
    if ( rDirEntry.eFlag == FSYS_FLAG_ABSROOT )
    {
        nKindFlags = FSYS_KIND_DIR;
        nError = FSYS_ERR_OK;
        return TRUE;
    }

    struct stat aStat;
    ByteString aPath( rDirEntry.GetFull(), osl_getThreadTextEncoding() );
    if ( stat( (char*) aPath.GetBuffer(), &aStat ) )
    {
        // pl: #67851#
        // do this here, because an existing filename containing "wildcards"
        // should be handled as a file, not a wildcard
        // note that this is not a solution, since filenames containing special characters
        // are handled badly across the whole Office

        // Sonderbehandlung falls es sich um eine Wildcard handelt
        ByteString aTempName( rDirEntry.GetName(), osl_getThreadTextEncoding() );
        if ( strchr( (char*) aTempName.GetBuffer(), '?' ) ||
             strchr( (char*) aTempName.GetBuffer(), '*' ) ||
             strchr( (char*) aTempName.GetBuffer(), ';' ) )
        {
            nKindFlags = FSYS_KIND_WILD;
            nError = FSYS_ERR_OK;
            return TRUE;
        }

        nError = FSYS_ERR_NOTEXISTS;
        return FALSE;
    }

    nError = FSYS_ERR_OK;
    nSize = aStat.st_size;

    nKindFlags = FSYS_KIND_UNKNOWN;
    if ( ( aStat.st_mode & S_IFDIR ) == S_IFDIR )
        nKindFlags = nKindFlags | FSYS_KIND_DIR;
    if ( ( aStat.st_mode & S_IFREG ) == S_IFREG )
        nKindFlags = nKindFlags | FSYS_KIND_FILE;
    if ( ( aStat.st_mode & S_IFCHR ) == S_IFCHR )
        nKindFlags = nKindFlags | FSYS_KIND_DEV | FSYS_KIND_CHAR;
    if ( ( aStat.st_mode & S_IFBLK ) == S_IFBLK )
        nKindFlags = nKindFlags | FSYS_KIND_DEV | FSYS_KIND_BLOCK;
    if ( nKindFlags == FSYS_KIND_UNKNOWN )
        nKindFlags = nKindFlags | FSYS_KIND_FILE;

    Unx2DateAndTime( aStat.st_ctime, aTimeCreated, aDateCreated );
    Unx2DateAndTime( aStat.st_mtime, aTimeModified, aDateModified );
    Unx2DateAndTime( aStat.st_atime, aTimeAccessed, aDateAccessed );

    return TRUE;
}

//====================================================================

const char *TempDirImpl( char *pBuf )
{
#ifdef MACOSX
    // P_tmpdir is /var/tmp on Mac OS X, and it is not cleaned up on system
    // startup
    strcpy( pBuf, "/tmp" );
#else
    const char *pValue = getenv( "TEMP" );
    if ( !pValue )
        pValue = getenv( "TMP" );
    if ( pValue )
        strcpy( pBuf, pValue );
    else
        // auf Solaris und Linux ist P_tmpdir vorgesehen
        strcpy( pBuf, P_tmpdir );
        // hart auf "/tmp"  sollte wohl nur im Notfall verwendet werden
        //strcpy( pBuf, "/tmp" );
#endif /* MACOSX */

    return pBuf;
}

/*************************************************************************
|*
|*    DirEntry::GetPathStyle() const
|*
*************************************************************************/

FSysPathStyle DirEntry::GetPathStyle( const String & )
{
    return FSYS_STYLE_UNX;
}

/*************************************************************************
|*
|*    FileStat::SetDateTime
|*
*************************************************************************/

void FileStat::SetDateTime( const String& rFileName,
                const DateTime& rNewDateTime )
{
    tm times;

    times.tm_year = rNewDateTime.GetYear()  - 1900;     // 1997 -> 97
    times.tm_mon  = rNewDateTime.GetMonth() - 1;        // 0 == Januar!
    times.tm_mday = rNewDateTime.GetDay();

    times.tm_hour = rNewDateTime.GetHour();
    times.tm_min  = rNewDateTime.GetMin();
    times.tm_sec  = rNewDateTime.GetSec();

    times.tm_wday  = 0;
    times.tm_yday  = 0;
#ifdef SOLARIS
    times.tm_isdst = -1;
#else
    times.tm_isdst = 0;
#endif

    time_t time = mktime (&times);

    if (time != (time_t) -1)
    {
        struct utimbuf u_time;
        u_time.actime = time;
        u_time.modtime = time;
        utime (ByteString(rFileName, osl_getThreadTextEncoding()).GetBuffer(), &u_time);
    }
}

//=========================================================================

ErrCode FileStat::QueryDiskSpace( const String &, BigInt &, BigInt & )
{
    return ERRCODE_IO_NOTSUPPORTED;
}

//=========================================================================

void FSysEnableSysErrorBox( BOOL )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
