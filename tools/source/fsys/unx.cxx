/*************************************************************************
 *
 *  $RCSfile: unx.cxx,v $
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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#if defined HPUX || defined LINUX || defined IRIX
#include <mntent.h>
#define mnttab mntent
#elif defined SCO
#include <mnttab.h>
#elif defined AIX
#include <sys/mntctl.h>
#include <sys/vmount.h>
extern "C" int mntctl( int cmd, size_t size, char* buf );
#elif defined S390
#include <sys/mntent.h>
#define mnttab w_mntent
#elif defined(NETBSD) || defined(FREEBSD) || defined(MACOSX)
#elif defined DECUNIX
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

#include <debug.hxx>
#include <list.hxx>
#include <fsys.hxx>
#include "comdep.hxx"

DECLARE_LIST( DirEntryList, DirEntry* )
DECLARE_LIST( FSysSortList, FSysSort* )
DECLARE_LIST( FileStatList, FileStat* )

#if defined SOLARIS || defined SINIX
#define MOUNTSPECIAL mnt_special
#define MOUNTPOINT   mnt_mountp
#define MOUNTOPTS    mnt_mntopts
#define MOUNTFS      mnt_fstype
#elif defined SCO
#define MNTTAB       "/etc/mnttab"
#define MOUNTSPECIAL mt_dev
#define MOUNTPOINT   mt_filsys
#elif defined S390
#define MOUNTSPECIAL mnt_fsname
#define MOUNTPOINT   mnt_mountpoint
#else
#define MOUNTSPECIAL mnt_fsname
#define MOUNTPOINT   mnt_dir
#define MOUNTFS      mnt_type
#endif

static ByteString sLastCaseSensitiveDir = "";
static BOOL   bLastCaseSensitive    = FALSE;

struct mymnttab
{
    dev_t mountdevice;
    ByteString mountspecial;
    ByteString mountpoint;
    ByteString mymnttab_filesystem;
    mymnttab() { mountdevice = (dev_t) -1; }
};


#if defined(NETBSD) || defined(FREEBSD) || defined(MACOSX)
BOOL GetMountEntry(dev_t dev, struct mymnttab *mytab)
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
#if defined SOLARIS || defined SINIX
    FILE *fp = fopen (MNTTAB, "r");
    if (! fp)
        return FALSE;
    struct mnttab mnt[1];
    while (getmntent (fp, mnt) != -1)
#elif defined SCO
    FILE *fp = fopen (MNTTAB, "r");
    if (! fp)
        return FALSE;
    struct mnttab mnt[1];
    while (fread (&mnt, sizeof mnt, 1, fp) > 0)
#elif defined DECUNIX || defined AIX || defined S390
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
#ifndef SCO
        mytab->mymnttab_filesystem = mnt->MOUNTFS;
#else
        mytab->mymnttab_filesystem = "ext2";        //default ist case sensitiv unter unix
#endif
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
|*    Beschreibung
|*    Ersterstellung    TPF 25.02.1999
|*    Letzte Aenderung  TPF 25.02.1999
|*
*************************************************************************/

BOOL DirEntry::IsCaseSensitive( FSysPathStyle eFormatter ) const
{

    if (eFormatter==FSYS_STYLE_HOST)
    {
#ifdef NETBSD
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 26.04.91
|*    Letzte Aenderung  MA 02.12.91 13:30
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 04.03.92
|*    Letzte Aenderung
|*
*************************************************************************/

static mymnttab mymnt;
static String aEmptyString;

String DirEntry::GetVolume() const
{
  DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry aPath( *this );
    aPath.ToAbs();

    struct stat buf;
    while (stat (ByteString(aPath.GetFull(), osl_getThreadTextEncoding()).GetBuffer(), &buf))
    {
        if (aPath.Level() <= 1)
            return aEmptyString;
        aPath = aPath [1];
    }

    return ((buf.st_dev == mymnt.mountdevice ||
                GetMountEntry(buf.st_dev, &mymnt)) ? String(mymnt.mountspecial, osl_getThreadTextEncoding()) : aEmptyString);
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
            return aEmptyString;
        aPath = aPath [1];
    }
    return ((buf.st_dev == mymnt.mountdevice ||
                GetMountEntry(buf.st_dev, &mymnt)) ? String( mymnt.mountpoint, osl_getThreadTextEncoding()) : aEmptyString);
}

/*************************************************************************
|*
|*    DirEntry::SetCWD()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 26.04.91
|*    Letzte Aenderung  DV 04.11.92
|*
*************************************************************************/

BOOL DirEntry::SetCWD( BOOL bSloppy )
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MA 05.11.91
|*    Letzte Aenderung  MA 07.11.91
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 11.06.91
|*    Letzte Aenderung  MA 07.11.91
|*
*************************************************************************/
#if SUPD < 345
BOOL FileStat::Update( const DirEntry& rDirEntry )
{
    return Update( rDirEntry, FALSE );
}
#endif

BOOL FileStat::Update( const DirEntry& rDirEntry, BOOL bAccessRemovableDevice )
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
    return pBuf;
}

/*************************************************************************
|*
|*    DirEntry::GetPathStyle() const
|*
|*    Beschreibung
|*    Ersterstellung    MI 11.05.95
|*    Letzte Aenderung  MI 11.05.95
|*
*************************************************************************/

FSysPathStyle DirEntry::GetPathStyle( const String &rDevice )
{
    return FSYS_STYLE_UNX;
}

/*************************************************************************
|*
|*    FileStat::SetDateTime
|*
|*    Ersterstellung    PB  27.06.97
|*    Letzte Aenderung
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

ErrCode FileStat::QueryDiskSpace( const String &rPath,
                                  BigInt &rFreeBytes, BigInt &rTotalBytes )
{
    return ERRCODE_IO_NOTSUPPORTED;
}

//=========================================================================

void FSysEnableSysErrorBox( BOOL bEnable )
{
}

