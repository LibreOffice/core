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

#include "osl/file.h"

#include "osl/diagnose.h"
#include "osl/thread.h"
#include "rtl/alloc.h"

#include "file_error_transl.hxx"
#include "file_url.hxx"
#include "system.hxx"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sal/macros.h>

#ifdef HAVE_STATFS_H
#undef HAVE_STATFS_H
#endif

#if defined(LINUX) && defined(__FreeBSD_kernel__)
#undef LINUX
#define FREEBSD 1
#endif

#if defined(SOLARIS)

#include <sys/mnttab.h>
#include <sys/statvfs.h>
#define  HAVE_STATFS_H

#elif defined(LINUX)

#include <mntent.h>
#include <sys/vfs.h>
#define  HAVE_STATFS_H

#elif defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD) || defined(DRAGONFLY)

#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#define  HAVE_STATFS_H

#elif defined(MACOSX)

#include <sys/param.h>
#include <sys/mount.h>
#define HAVE_STATFS_H

#endif /* HAVE_STATFS_H */

/************************************************************************
 *   ToDo
 *
 *   - Fix: check for corresponding struct sizes in exported functions
 *   - check size/use of oslVolumeDeviceHandle
 *   - check size/use of oslVolumeInfo
 ***********************************************************************/
/******************************************************************************
 *
 *                  Data Type Definition
 *
 ******************************************************************************/

typedef struct _oslVolumeDeviceHandleImpl
{
    sal_Char pszMountPoint[PATH_MAX];
    sal_Char pszFilePath[PATH_MAX];
    sal_Char pszDevice[PATH_MAX];
    sal_Char ident[4];
    sal_uInt32   RefCount;
} oslVolumeDeviceHandleImpl;

/******************************************************************************
 *
 *                  C-String Function Declarations
 *
 *****************************************************************************/

static oslFileError osl_psz_getVolumeInformation(const sal_Char* , oslVolumeInfo* pInfo, sal_uInt32 uFieldMask);

/****************************************************************************/
/*  osl_getVolumeInformation */
/****************************************************************************/

oslFileError osl_getVolumeInformation( rtl_uString* ustrDirectoryURL, oslVolumeInfo* pInfo, sal_uInt32 uFieldMask )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrDirectoryURL );
    OSL_ASSERT( pInfo );

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_getVolumeInformation( path, pInfo, uFieldMask);
}

/******************************************************************************
 *
 *                  C-String Versions of Exported Module Functions
 *
 *****************************************************************************/

#ifdef HAVE_STATFS_H

#if defined(FREEBSD) || defined(MACOSX) || defined(OPENBSD) || defined(DRAGONFLY)
#   define __OSL_STATFS_STRUCT                  struct statfs
#   define __OSL_STATFS(dir, sfs)               statfs((dir), (sfs))
#   define __OSL_STATFS_BLKSIZ(a)               ((sal_uInt64)((a).f_bsize))
#   define __OSL_STATFS_TYPENAME(a)             ((a).f_fstypename)
#if defined(OPENBSD)
#   define __OSL_STATFS_ISREMOTE(a)             (rtl_str_compare((a).f_fstypename, "nfs") == 0)
#else
#   define __OSL_STATFS_ISREMOTE(a)             (((a).f_type & MNT_LOCAL) == 0)
#endif

/* always return true if queried for the properties of
   the file system. If you think this is wrong under any
   of the target platforms fix it!!!! */
#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a)  (true)
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a) (true)
#endif /* FREEBSD || MACOSX || OPENBSD */

#if defined(NETBSD)

#include <sys/param.h>

#   define __OSL_STATFS_STRUCT              struct statvfs
#   define __OSL_STATFS(dir, sfs)           statvfs((dir), (sfs))
#   define __OSL_STATFS_ISREMOTE(a)         (((a).f_flag & ST_LOCAL) == 0)

#   define __OSL_STATFS_BLKSIZ(a)           ((sal_uInt64)((a).f_bsize))
#   define __OSL_STATFS_TYPENAME(a)         ((a).f_fstypename)

#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a) (strcmp((a).f_fstypename, "msdos") != 0 && strcmp((a).f_fstypename, "ntfs") != 0 && strcmp((a).f_fstypename, "smbfs") != 0)
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a)    (strcmp((a).f_fstypename, "msdos") != 0)
#endif /* NETBSD */

#if defined(LINUX)
#   define __OSL_NFS_SUPER_MAGIC                 0x6969
#   define __OSL_SMB_SUPER_MAGIC                 0x517B
#   define __OSL_MSDOS_SUPER_MAGIC               0x4d44
#   define __OSL_NTFS_SUPER_MAGIC                0x5346544e
#   define __OSL_STATFS_STRUCT                   struct statfs
#   define __OSL_STATFS(dir, sfs)                statfs((dir), (sfs))
#   define __OSL_STATFS_BLKSIZ(a)                ((sal_uInt64)((a).f_bsize))
#   define __OSL_STATFS_IS_NFS(a)                (__OSL_NFS_SUPER_MAGIC == (a).f_type)
#   define __OSL_STATFS_IS_SMB(a)                (__OSL_SMB_SUPER_MAGIC == (a).f_type)
#   define __OSL_STATFS_ISREMOTE(a)              (__OSL_STATFS_IS_NFS((a)) || __OSL_STATFS_IS_SMB((a)))
#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a)  ((__OSL_MSDOS_SUPER_MAGIC != (a).f_type) && (__OSL_NTFS_SUPER_MAGIC != (a).f_type))
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a) ((__OSL_MSDOS_SUPER_MAGIC != (a).f_type))
#endif /* LINUX */

#if defined(SOLARIS)
#   define __OSL_STATFS_STRUCT                   struct statvfs
#   define __OSL_STATFS(dir, sfs)                statvfs((dir), (sfs))
#   define __OSL_STATFS_BLKSIZ(a)                ((sal_uInt64)((a).f_frsize))
#   define __OSL_STATFS_TYPENAME(a)              ((a).f_basetype)
#   define __OSL_STATFS_ISREMOTE(a)              (rtl_str_compare((a).f_basetype, "nfs") == 0)

/* always return true if queried for the properties of
   the file system. If you think this is wrong under any
   of the target platforms fix it!!!! */
#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a)  (true)
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a) (true)
#endif /* SOLARIS */

#   define __OSL_STATFS_INIT(a)         (memset(&(a), 0, sizeof(__OSL_STATFS_STRUCT)))

#else /* no statfs available */

#   define __OSL_STATFS_STRUCT                   struct dummy {int i;}
#   define __OSL_STATFS_INIT(a)                  ((void)a)
#   define __OSL_STATFS(dir, sfs)                (1)
#   define __OSL_STATFS_ISREMOTE(sfs)            (false)
#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a)  (true)
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a) (true)
#endif /* HAVE_STATFS_H */

static oslFileError osl_psz_getVolumeInformation (
    const sal_Char* pszDirectory, oslVolumeInfo* pInfo, sal_uInt32 uFieldMask)
{
    if (!pInfo)
        return osl_File_E_INVAL;

    pInfo->uValidFields = 0;
    pInfo->uAttributes  = 0;
    pInfo->uTotalSpace = 0;
    pInfo->uFreeSpace  = 0;
    pInfo->uUsedSpace  = 0;

    if ((uFieldMask
         & (osl_VolumeInfo_Mask_Attributes | osl_VolumeInfo_Mask_TotalSpace
            | osl_VolumeInfo_Mask_UsedSpace | osl_VolumeInfo_Mask_FreeSpace
            | osl_VolumeInfo_Mask_FileSystemName
            | osl_VolumeInfo_Mask_FileSystemCaseHandling))
        != 0)
    {
        __OSL_STATFS_STRUCT sfs;
        __OSL_STATFS_INIT(sfs);
        if ((__OSL_STATFS(pszDirectory, &sfs)) < 0)
        {
            oslFileError result = oslTranslateFileError(OSL_FET_ERROR, errno);
            return result;
        }

        /* FIXME: how to detect the kind of storage (fixed, cdrom, ...) */
        if (uFieldMask & osl_VolumeInfo_Mask_Attributes)
        {
            if (__OSL_STATFS_ISREMOTE(sfs))
                pInfo->uAttributes  |= osl_Volume_Attribute_Remote;

            pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;
        }

        if (uFieldMask & osl_VolumeInfo_Mask_FileSystemCaseHandling)
        {
            if (__OSL_STATFS_IS_CASE_SENSITIVE_FS(sfs))
                pInfo->uAttributes |= osl_Volume_Attribute_Case_Sensitive;

            if (__OSL_STATFS_IS_CASE_PRESERVING_FS(sfs))
                pInfo->uAttributes |= osl_Volume_Attribute_Case_Is_Preserved;

            pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;
        }

#if defined(__OSL_STATFS_BLKSIZ)

        if ((uFieldMask & osl_VolumeInfo_Mask_TotalSpace) ||
            (uFieldMask & osl_VolumeInfo_Mask_UsedSpace))
        {
            pInfo->uTotalSpace   = __OSL_STATFS_BLKSIZ(sfs);
            pInfo->uTotalSpace  *= (sal_uInt64)(sfs.f_blocks);
            pInfo->uValidFields |= osl_VolumeInfo_Mask_TotalSpace;
        }

        if ((uFieldMask & osl_VolumeInfo_Mask_FreeSpace) ||
            (uFieldMask & osl_VolumeInfo_Mask_UsedSpace))
        {
            pInfo->uFreeSpace = __OSL_STATFS_BLKSIZ(sfs);

            if (getuid() == 0)
                pInfo->uFreeSpace *= (sal_uInt64)(sfs.f_bfree);
            else
                pInfo->uFreeSpace *= (sal_uInt64)(sfs.f_bavail);

            pInfo->uValidFields |= osl_VolumeInfo_Mask_FreeSpace;
        }

        if ((pInfo->uValidFields & osl_VolumeInfo_Mask_TotalSpace) &&
            (pInfo->uValidFields & osl_VolumeInfo_Mask_FreeSpace ))
        {
            pInfo->uUsedSpace    = pInfo->uTotalSpace - pInfo->uFreeSpace;
            pInfo->uValidFields |= osl_VolumeInfo_Mask_UsedSpace;
        }

#endif  /* __OSL_STATFS_BLKSIZ */

#if defined(__OSL_STATFS_TYPENAME)

        if (uFieldMask & osl_VolumeInfo_Mask_FileSystemName)
        {
            rtl_string2UString(
                &(pInfo->ustrFileSystemName),
                __OSL_STATFS_TYPENAME(sfs),
                rtl_str_getLength(__OSL_STATFS_TYPENAME(sfs)),
                osl_getThreadTextEncoding(),
                OUSTRING_TO_OSTRING_CVTFLAGS);
            OSL_ASSERT(pInfo->ustrFileSystemName != nullptr);

            pInfo->uValidFields |= osl_VolumeInfo_Mask_FileSystemName;
        }

#endif /* __OSL_STATFS_TYPENAME */
    }

    pInfo->uMaxNameLength = 0;
    if (uFieldMask & osl_VolumeInfo_Mask_MaxNameLength)
    {
        long nLen = pathconf(pszDirectory, _PC_NAME_MAX);
        if (nLen > 0)
        {
            pInfo->uMaxNameLength = (sal_uInt32)nLen;
            pInfo->uValidFields |= osl_VolumeInfo_Mask_MaxNameLength;
        }
    }

    pInfo->uMaxPathLength = 0;
    if (uFieldMask & osl_VolumeInfo_Mask_MaxPathLength)
    {
        long nLen = pathconf (pszDirectory, _PC_PATH_MAX);
        if (nLen > 0)
        {
            pInfo->uMaxPathLength  = (sal_uInt32)nLen;
            pInfo->uValidFields   |= osl_VolumeInfo_Mask_MaxPathLength;
        }
    }

    return osl_File_E_None;
}

/******************************************************************************
 *
 *                  GENERIC FLOPPY FUNCTIONS
 *
 *****************************************************************************/

/*****************************************
 * osl_getVolumeDeviceMountPath
 ****************************************/
static rtl_uString* oslMakeUStrFromPsz(const sal_Char* pszStr, rtl_uString** ustrValid)
{
    rtl_string2UString(
        ustrValid,
        pszStr,
        rtl_str_getLength( pszStr ),
        osl_getThreadTextEncoding(),
        OUSTRING_TO_OSTRING_CVTFLAGS );
    OSL_ASSERT(*ustrValid != nullptr);

    return *ustrValid;
}

oslFileError osl_getVolumeDeviceMountPath( oslVolumeDeviceHandle Handle, rtl_uString **pstrPath )
{
    oslVolumeDeviceHandleImpl* pItem = static_cast<oslVolumeDeviceHandleImpl*>(Handle);
    sal_Char Buffer[PATH_MAX];

    Buffer[0] = '\0';

    if ( pItem == nullptr || pstrPath == nullptr )
    {
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
        return osl_File_E_INVAL;
    }

    snprintf(Buffer, sizeof(Buffer), "file://%s", pItem->pszMountPoint);

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Mount Point is: '%s'\n",Buffer);
#endif

    oslMakeUStrFromPsz(Buffer, pstrPath);

    return osl_File_E_None;
}

/*****************************************
 * osl_acquireVolumeDeviceHandle
 ****************************************/

oslFileError SAL_CALL osl_acquireVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    oslVolumeDeviceHandleImpl* pItem =static_cast<oslVolumeDeviceHandleImpl*>(Handle);

    if ( pItem == nullptr )
    {
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
        return osl_File_E_INVAL;
    }

    ++pItem->RefCount;

    return osl_File_E_None;
}

/*****************************************
 * osl_releaseVolumeDeviceHandle
 ****************************************/

oslFileError osl_releaseVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    oslVolumeDeviceHandleImpl* pItem =static_cast<oslVolumeDeviceHandleImpl*>(Handle);

    if ( pItem == nullptr )
    {
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
        return osl_File_E_INVAL;
    }

    --pItem->RefCount;

    if ( pItem->RefCount == 0 )
    {
        rtl_freeMemory(pItem);
    }

    return osl_File_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
