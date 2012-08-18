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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>

#if defined LINUX || defined ANDROID
#include <mntent.h>
#define mnttab mntent
#elif defined AIX
#include <sys/mntctl.h>
#include <sys/vmount.h>
extern "C" int mntctl( int cmd, size_t size, char* buf );
#elif defined(NETBSD)
#include <sys/mount.h>
#elif defined(FREEBSD) || defined(MACOSX) || defined(OPENBSD) || \
      defined(DRAGONFLY) || defined(IOS)
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
    rtl::OString mountspecial;
    rtl::OString mountpoint;
    rtl::OString mymnttab_filesystem;
    mymnttab() { mountdevice = (dev_t) -1; }
};

#if defined(NETBSD) || defined(FREEBSD) || defined(MACOSX) || \
    defined(OPENBSD) || defined(DRAGONFLY) || defined(IOS)
sal_Bool GetMountEntry(dev_t /* dev */, struct mymnttab * /* mytab */ )
{
    DBG_WARNING( "Sorry, not implemented: GetMountEntry" );
    return sal_False;
}
#elif defined AIX
sal_Bool GetMountEntry(dev_t dev, struct mymnttab *mytab)
{
    int bufsize;
    if (mntctl (MCTL_QUERY, sizeof bufsize, (char*) &bufsize))
        return sal_False;

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
                return sal_True;
            }
        }
    free( buffer );
    return sal_False;
}
#else
static sal_Bool GetMountEntry(dev_t dev, struct mymnttab *mytab)
{
#if defined SOLARIS
    FILE *fp = fopen (MNTTAB, "r");
    if (! fp)
        return sal_False;
    struct mnttab mnt[1];
    while (getmntent (fp, mnt) != -1)
#elif defined AIX || defined ANDROID
    FILE *fp = NULL;
    if (! fp)
        return sal_False;
    struct mnttab mnt[1];
    while ( 0 )
#else
    FILE *fp = setmntent (MOUNTED, "r");
    if (! fp)
        return sal_False;
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

        return sal_True;
    }
#   ifdef LINUX
    /* #61624# dito */
    endmntent( fp );
#   else
    fclose (fp);
#   endif
    return sal_False;
}
#endif

sal_Bool DirEntry::ToAbs()
{
    if ( FSYS_FLAG_VOLUME == eFlag )
    {
        eFlag = FSYS_FLAG_ABSROOT;
        return sal_True;
    }

    if ( IsAbs() )
      return sal_True;

    char sBuf[MAXPATHLEN + 1];
    *this = DirEntry( String( getcwd( sBuf, MAXPATHLEN ), osl_getThreadTextEncoding() ) ) + *this;
    return IsAbs();
}

namespace { struct mymnt : public rtl::Static< mymnttab, mymnt > {}; }

String DirEntry::GetVolume() const
{
  DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry aPath( *this );
    aPath.ToAbs();

    struct stat buf;
    while (stat(rtl::OUStringToOString(aPath.GetFull(), osl_getThreadTextEncoding()).getStr(), &buf))
    {
        if (aPath.Level() <= 1)
            return String();
        aPath = aPath [1];
    }
    mymnttab &rMnt = mymnt::get();
    return ((buf.st_dev == rMnt.mountdevice ||
                GetMountEntry(buf.st_dev, &rMnt)) ?
                    rtl::OStringToOUString(rMnt.mountspecial, osl_getThreadTextEncoding()) :
                    rtl::OUString());
}

sal_Bool DirEntry::SetCWD( sal_Bool bSloppy ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    rtl::OString aPath(rtl::OUStringToOString(GetFull(), osl_getThreadTextEncoding()));
    if (!chdir(aPath.getStr()))
    {
        return sal_True;
    }
    else
    {
        if (bSloppy && !chdir(aPath.getStr()))
        {
            return sal_True;
        }
        else
        {
            return sal_False;
        }
    }
}

sal_uInt16 DirReader_Impl::Init()
{
    return 0;
}

sal_uInt16 DirReader_Impl::Read()
{
    if (!pDosDir)
    {
        pDosDir = opendir(rtl::OUStringToOString(aPath, osl_getThreadTextEncoding()).getStr());
    }

    if (!pDosDir)
    {
        bReady = sal_True;
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
            DirEntry *pTemp = new DirEntry(rtl::OString(pDosEntry->d_name), eFlag);
            if ( pParent )
                pTemp->ImpChangeParent( new DirEntry( *pParent ), sal_False);
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
                    pDir->ImpSortedInsert( pTemp, NULL );
                return 1;
            }
            else
                delete pTemp;
        }
    }
    else
        bReady = sal_True;
    return 0;
}

sal_Bool FileStat::Update( const DirEntry& rDirEntry, SAL_UNUSED_PARAMETER sal_Bool )
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
        return sal_False;
    }

    // Sonderbehandlung falls es sich um eine Root handelt
    if ( rDirEntry.eFlag == FSYS_FLAG_ABSROOT )
    {
        nKindFlags = FSYS_KIND_DIR;
        nError = FSYS_ERR_OK;
        return sal_True;
    }

    struct stat aStat;
    rtl::OString aPath(rtl::OUStringToOString(rDirEntry.GetFull(), osl_getThreadTextEncoding()));
    if (stat(aPath.getStr(), &aStat))
    {
        // pl: #67851#
        // do this here, because an existing filename containing "wildcards"
        // should be handled as a file, not a wildcard
        // note that this is not a solution, since filenames containing special characters
        // are handled badly across the whole Office

        // Sonderbehandlung falls es sich um eine Wildcard handelt
        rtl::OString aTempName(rtl::OUStringToOString(rDirEntry.GetName(), osl_getThreadTextEncoding()));
        if ( aTempName.indexOf('?') != -1 ||
             aTempName.indexOf('*') != -1 ||
             aTempName.indexOf(';') != -1 )
        {
            nKindFlags = FSYS_KIND_WILD;
            nError = FSYS_ERR_OK;
            return sal_True;
        }

        nError = FSYS_ERR_NOTEXISTS;
        return sal_False;
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

    return sal_True;
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
