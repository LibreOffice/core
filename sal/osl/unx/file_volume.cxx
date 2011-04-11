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

#include "osl/file.h"

#include "osl/diagnose.h"
#include "osl/thread.h"
#include "rtl/alloc.h"

#include "file_error_transl.h"
#include "file_url.h"
#include "system.h"

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
#include <sys/fs/ufs_quota.h>
static const sal_Char* MOUNTTAB="/etc/mnttab";

#elif defined(LINUX)

#include <mntent.h>
#include <sys/vfs.h>
#define  HAVE_STATFS_H
#include <sys/quota.h>
static const sal_Char* MOUNTTAB="/etc/mtab";

#elif defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD) || defined(DRAGONFLY)

#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <ufs/ufs/quota.h>
#define  HAVE_STATFS_H

/* No mounting table on *BSD
 * This information is stored only in the kernel. */
/* static const sal_Char* MOUNTTAB="/etc/mtab"; */

#elif defined(MACOSX)

#include <ufs/ufs/quota.h>
#include <sys/param.h>
#include <sys/mount.h>
#define HAVE_STATFS_H
// static const sal_Char* MOUNTTAB="/etc/mtab";

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
 *                  'removeable device' aka floppy functions
 *
 *****************************************************************************/

static oslVolumeDeviceHandle  osl_isFloppyDrive(const sal_Char* pszPath);
static oslFileError   osl_mountFloppy(oslVolumeDeviceHandle hFloppy);
static oslFileError   osl_unmountFloppy(oslVolumeDeviceHandle hFloppy);

#if defined(SOLARIS)
static sal_Bool       osl_isFloppyMounted(sal_Char* pszPath, sal_Char* pszMountPath);
static sal_Bool       osl_getFloppyMountEntry(const sal_Char* pszPath, sal_Char* pBuffer);
static sal_Bool       osl_checkFloppyPath(sal_Char* pszPath, sal_Char* pszFilePath, sal_Char* pszDevicePath);
#endif /* SOLARIS */

#if defined(LINUX)
static sal_Bool       osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice);
static sal_Bool       osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem);
#endif /* LINUX */

#ifdef DEBUG_OSL_FILE
static void           osl_printFloppyHandle(oslVolumeDeviceHandleImpl* hFloppy);
#endif /* DEBUG_OSL_FILE */

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
#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a)  (1)
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a) (1)
#endif /* FREEBSD || MACOSX || OPENBSD */

#if defined(NETBSD)

#include <sys/param.h>

/* statvfs() replaced statfs() in 2.99.9 */
# if __NetBSD_Version__ >= 299000900
       /* 2.0D or later */
#   define __OSL_STATFS_STRUCT              struct statvfs
#   define __OSL_STATFS(dir, sfs)           statvfs((dir), (sfs))
#   define __OSL_STATFS_ISREMOTE(a)         (((a).f_flag & ST_LOCAL) == 0)

# else
       /* version before 2.0D */
#   define __OSL_STATFS_STRUCT              struct statfs
#   define __OSL_STATFS(dir, sfs)           statfs((dir), (sfs))
#   define __OSL_STATFS_ISREMOTE(a)         (((a).f_type & MNT_LOCAL) == 0)

# endif /* >2.0D */

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
#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a)  (1)
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a) (1)
#endif /* SOLARIS */

#   define __OSL_STATFS_INIT(a)         (memset(&(a), 0, sizeof(__OSL_STATFS_STRUCT)))

#else /* no statfs available */

#   define __OSL_STATFS_STRUCT                   struct dummy {int i;}
#   define __OSL_STATFS_INIT(a)                  ((void)a)
#   define __OSL_STATFS(dir, sfs)                (1)
#   define __OSL_STATFS_ISREMOTE(sfs)            (0)
#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a)  (1)
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a) (1)
#endif /* HAVE_STATFS_H */


static oslFileError osl_psz_getVolumeInformation (
    const sal_Char* pszDirectory, oslVolumeInfo* pInfo, sal_uInt32 uFieldMask)
{
    __OSL_STATFS_STRUCT sfs;

    if (!pInfo)
        return osl_File_E_INVAL;

    __OSL_STATFS_INIT(sfs);

    pInfo->uValidFields = 0;
    pInfo->uAttributes  = 0;

    if ((__OSL_STATFS(pszDirectory, &sfs)) < 0)
    {
        oslFileError result = oslTranslateFileError(OSL_FET_ERROR, errno);
        return (result);
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

    pInfo->uTotalSpace = 0;
    pInfo->uFreeSpace  = 0;
    pInfo->uUsedSpace  = 0;

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

#endif  /* __OSL_STATFS_BLKSIZ */

    if ((pInfo->uValidFields & osl_VolumeInfo_Mask_TotalSpace) &&
        (pInfo->uValidFields & osl_VolumeInfo_Mask_FreeSpace ))
    {
        pInfo->uUsedSpace    = pInfo->uTotalSpace - pInfo->uFreeSpace;
        pInfo->uValidFields |= osl_VolumeInfo_Mask_UsedSpace;
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

#if defined(__OSL_STATFS_TYPENAME)

    if (uFieldMask & osl_VolumeInfo_Mask_FileSystemName)
    {
        rtl_string2UString(
            &(pInfo->ustrFileSystemName),
            __OSL_STATFS_TYPENAME(sfs),
            rtl_str_getLength(__OSL_STATFS_TYPENAME(sfs)),
            osl_getThreadTextEncoding(),
            OUSTRING_TO_OSTRING_CVTFLAGS);
        OSL_ASSERT(pInfo->ustrFileSystemName != 0);

        pInfo->uValidFields |= osl_VolumeInfo_Mask_FileSystemName;
    }

#endif /* __OSL_STATFS_TYPENAME */

    if (uFieldMask & osl_VolumeInfo_Mask_DeviceHandle)
    {
        /* FIXME: check also entries in mntent for the device
           and fill it with correct values */

        *pInfo->pDeviceHandle = osl_isFloppyDrive(pszDirectory);

        if (*pInfo->pDeviceHandle)
        {
            pInfo->uValidFields |= osl_VolumeInfo_Mask_DeviceHandle;
            pInfo->uAttributes  |= osl_Volume_Attribute_Removeable;
            pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;
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
 * osl_unmountVolumeDevice
 ****************************************/

oslFileError osl_unmountVolumeDevice( oslVolumeDeviceHandle Handle )
{
    oslFileError tErr = osl_File_E_NOSYS;

    tErr = osl_unmountFloppy(Handle);

     /* Perhaps current working directory is set to mount point */

     if ( tErr )
    {
        sal_Char *pszHomeDir = getenv("HOME");

        if ( pszHomeDir && strlen( pszHomeDir ) && 0 == chdir( pszHomeDir ) )
        {
            /* try again */

            tErr = osl_unmountFloppy(Handle);

            OSL_ENSURE( tErr, "osl_unmountvolumeDevice: CWD was set to volume mount point" );
        }
    }

    return tErr;
}

/*****************************************
 * osl_automountVolumeDevice
 ****************************************/

oslFileError osl_automountVolumeDevice( oslVolumeDeviceHandle Handle )
{
    oslFileError tErr = osl_File_E_NOSYS;

    tErr = osl_mountFloppy(Handle);

    return tErr;
}

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
    OSL_ASSERT(*ustrValid != 0);

    return *ustrValid;
}

oslFileError osl_getVolumeDeviceMountPath( oslVolumeDeviceHandle Handle, rtl_uString **pstrPath )
{
    oslVolumeDeviceHandleImpl* pItem = (oslVolumeDeviceHandleImpl*) Handle;
    sal_Char Buffer[PATH_MAX];

    Buffer[0] = '\0';

    if ( pItem == 0 || pstrPath == 0 )
    {
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
        return osl_File_E_INVAL;
    }

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Handle is:\n");
    osl_printFloppyHandle(pItem);
#endif

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
    oslVolumeDeviceHandleImpl* pItem =(oslVolumeDeviceHandleImpl*) Handle;

    if ( pItem == 0 )
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
    oslVolumeDeviceHandleImpl* pItem =(oslVolumeDeviceHandleImpl*) Handle;

    if ( pItem == 0 )
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

#if !defined(MACOSX) && !defined(AIX)

/*****************************************
 * osl_newVolumeDeviceHandleImpl
 ****************************************/

static oslVolumeDeviceHandleImpl* osl_newVolumeDeviceHandleImpl()
{
    oslVolumeDeviceHandleImpl* pHandle;
    const size_t               nSizeOfHandle = sizeof(oslVolumeDeviceHandleImpl);

    pHandle = (oslVolumeDeviceHandleImpl*) rtl_allocateMemory (nSizeOfHandle);
    if (pHandle != NULL)
    {
        pHandle->ident[0]         = 'O';
        pHandle->ident[1]         = 'V';
        pHandle->ident[2]         = 'D';
        pHandle->ident[3]         = 'H';
        pHandle->pszMountPoint[0] = '\0';
        pHandle->pszFilePath[0]   = '\0';
        pHandle->pszDevice[0]     = '\0';
        pHandle->RefCount         = 1;
    }
    return pHandle;
}

/*****************************************
 * osl_freeVolumeDeviceHandleImpl
 ****************************************/

static void osl_freeVolumeDeviceHandleImpl (oslVolumeDeviceHandleImpl* pHandle)
{
    if (pHandle != NULL)
        rtl_freeMemory (pHandle);
}
#endif

/******************************************************************************
 *
 *                  SOLARIS FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if defined(SOLARIS)
/* compare a given devicename with the typical device names on a Solaris box */
static sal_Bool
osl_isAFloppyDevice (const char* pDeviceName)
{
    const char* pFloppyDevice [] = {
        "/dev/fd",           "/dev/rfd",
        "/dev/diskette",     "/dev/rdiskette",
        "/vol/dev/diskette", "/vol/dev/rdiskette"
    };

    int i;
    for (i = 0; i < SAL_N_ELEMENTS(pFloppyDevice); i++)
    {
        if (strncmp(pDeviceName, pFloppyDevice[i], strlen(pFloppyDevice[i])) == 0)
            return sal_True;
    }
    return sal_False;
}

/* compare two directories whether the first may be a parent of the second. this
 * does not realpath() resolving */
static sal_Bool
osl_isAParentDirectory (const char* pParentDir, const char* pSubDir)
{
    return strncmp(pParentDir, pSubDir, strlen(pParentDir)) == 0;
}

/* the name of the routine is obviously silly. But anyway create a
 * oslVolumeDeviceHandle with correct mount point, device name and a resolved filepath
 * only if pszPath points to file or directory on a floppy */
static oslVolumeDeviceHandle
osl_isFloppyDrive(const sal_Char* pszPath)
{
    FILE*                       pMountTab;
    struct mnttab               aMountEnt;
    oslVolumeDeviceHandleImpl*  pHandle;

    if ((pHandle = osl_newVolumeDeviceHandleImpl()) == NULL)
    {
        return NULL;
    }
    if (realpath(pszPath, pHandle->pszFilePath) == NULL)
    {
        osl_freeVolumeDeviceHandleImpl (pHandle);
        return NULL;
    }
    if ((pMountTab = fopen (MOUNTTAB, "r")) == NULL)
    {
        osl_freeVolumeDeviceHandleImpl (pHandle);
        return NULL;
    }

    while (getmntent(pMountTab, &aMountEnt) == 0)
    {
        const char *pMountPoint = aMountEnt.mnt_mountp;
        const char *pDevice     = aMountEnt.mnt_special;
        if (   osl_isAParentDirectory (aMountEnt.mnt_mountp, pHandle->pszFilePath)
            && osl_isAFloppyDevice    (aMountEnt.mnt_special))
        {
            /* skip the last item for it is the name of the disk */
            char * pc = strrchr( aMountEnt.mnt_special, '/' );

            if ( NULL != pc )
            {
                int len = pc - aMountEnt.mnt_special;

                strncpy( pHandle->pszDevice, aMountEnt.mnt_special, len );
                pHandle->pszDevice[len] = '\0';
            }
            else
            {
                /* #106048 use save str functions to avoid buffer overflows */
                memset(pHandle->pszDevice, 0, sizeof(pHandle->pszDevice));
                strncpy(pHandle->pszDevice, aMountEnt.mnt_special, sizeof(pHandle->pszDevice) - 1);
            }

            /* remember the mount point */
            memset(pHandle->pszMountPoint, 0, sizeof(pHandle->pszMountPoint));
            strncpy(pHandle->pszMountPoint, aMountEnt.mnt_mountp, sizeof(pHandle->pszMountPoint) - 1);

            fclose (pMountTab);
            return pHandle;
        }
    }

    fclose (pMountTab);
    osl_freeVolumeDeviceHandleImpl (pHandle);
    return NULL;
}

static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{
    FILE*                       pMountTab;
    struct mnttab               aMountEnt;
    oslVolumeDeviceHandleImpl*  pHandle = (oslVolumeDeviceHandleImpl*) hFloppy;

    int nRet=0;
    sal_Char pszCmd[512] = "";

    if ( pHandle == 0 )
        return osl_File_E_INVAL;

    /* FIXME: don't know what this is good for */
    if ( pHandle->ident[0] != 'O' || pHandle->ident[1] != 'V' || pHandle->ident[2] != 'D' || pHandle->ident[3] != 'H' )
        return osl_File_E_INVAL;

    snprintf(pszCmd, sizeof(pszCmd), "eject -q %s > /dev/null 2>&1", pHandle->pszDevice);

    nRet = system( pszCmd );

    switch ( WEXITSTATUS(nRet) )
    {
    case 0:
        {
            /* lookup the device in mount tab again */
            if ((pMountTab = fopen (MOUNTTAB, "r")) == NULL)
                return osl_File_E_BUSY;

            while (getmntent(pMountTab, &aMountEnt) == 0)
            {
                const char *pMountPoint = aMountEnt.mnt_mountp;
                const char *pDevice     = aMountEnt.mnt_special;
                if ( 0 == strncmp( pHandle->pszDevice, aMountEnt.mnt_special, strlen(pHandle->pszDevice) ) )
                {
                    memset(pHandle->pszMountPoint, 0, sizeof(pHandle->pszMountPoint));
                    strncpy (pHandle->pszMountPoint, aMountEnt.mnt_mountp, sizeof(pHandle->pszMountPoint) - 1);

                    fclose (pMountTab);
                    return osl_File_E_None;
                }
            }

            fclose (pMountTab);
            return osl_File_E_BUSY;
        }
        //break; // break not necessary here, see return statements before

    case 1:
        return osl_File_E_BUSY;

    default:
        break;
    }

    return osl_File_E_BUSY;
}

static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
//    FILE*                       pMountTab;
//    struct mnttab               aMountEnt;
    oslVolumeDeviceHandleImpl*  pHandle = (oslVolumeDeviceHandleImpl*) hFloppy;

    int nRet=0;
    sal_Char pszCmd[512] = "";

    if ( pHandle == 0 )
        return osl_File_E_INVAL;

    /* FIXME: don't know what this is good for */
    if ( pHandle->ident[0] != 'O' || pHandle->ident[1] != 'V' || pHandle->ident[2] != 'D' || pHandle->ident[3] != 'H' )
        return osl_File_E_INVAL;

    snprintf(pszCmd, sizeof(pszCmd), "eject %s > /dev/null 2>&1", pHandle->pszDevice);

    nRet = system( pszCmd );

    switch ( WEXITSTATUS(nRet) )
    {
    case 0:
        {
            FILE*         pMountTab;
            struct mnttab aMountEnt;

            /* lookup if device is still in mount tab */
            if ((pMountTab = fopen (MOUNTTAB, "r")) == NULL)
                return osl_File_E_BUSY;

            while (getmntent(pMountTab, &aMountEnt) == 0)
            {
                const char *pMountPoint = aMountEnt.mnt_mountp;
                const char *pDevice     = aMountEnt.mnt_special;
                if ( 0 == strncmp( pHandle->pszDevice, aMountEnt.mnt_special, strlen(pHandle->pszDevice) ) )
                {
                    fclose (pMountTab);
                    return osl_File_E_BUSY;
                }
            }

            fclose (pMountTab);
            pHandle->pszMountPoint[0] = 0;
            return osl_File_E_None;
        }

        //break; //break not necessary, see return statements before

    case 1:
        return osl_File_E_NODEV;

    case 4:
        pHandle->pszMountPoint[0] = 0;
        return osl_File_E_None;

    default:
        break;
    }

    return osl_File_E_BUSY;
}

#endif /* SOLARIS */

/******************************************************************************
 *
 *                  LINUX FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if defined(LINUX)
static oslVolumeDeviceHandle
osl_isFloppyDrive (const sal_Char* pszPath)
{
    oslVolumeDeviceHandleImpl* pItem = osl_newVolumeDeviceHandleImpl();
    if (osl_getFloppyMountEntry(pszPath, pItem))
        return (oslVolumeDeviceHandle) pItem;

    osl_freeVolumeDeviceHandleImpl (pItem);
    return 0;
}
#endif /* LINUX */

#if defined(LINUX)
static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{
    sal_Bool bRet = sal_False;
    oslVolumeDeviceHandleImpl* pItem=0;
    int nRet;
    sal_Char  pszCmd[PATH_MAX];
    const sal_Char* pszMountProg = "mount";
    sal_Char* pszSuDo = 0;
    sal_Char* pszTmp = 0;

    pszCmd[0] = '\0';

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_mountFloppy\n");
#endif

    pItem = (oslVolumeDeviceHandleImpl*) hFloppy;

    if ( pItem == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_mountFloppy [pItem == 0]\n");
#endif

        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_mountFloppy [invalid handle]\n");
#endif
        return osl_File_E_INVAL;
    }

    bRet = osl_isFloppyMounted(pItem);
    if ( bRet == sal_True )
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"detected mounted floppy at '%s'\n",pItem->pszMountPoint);
#endif
        return osl_File_E_BUSY;
    }

    /* mfe: we can't use the mount(2) system call!!!   */
    /*      even if we are root                        */
    /*      since mtab is not updated!!!               */
    /*      but we need it to be updated               */
    /*      some "magic" must be done                  */

/*      nRet = mount(pItem->pszDevice,pItem->pszMountPoint,0,0,0); */
/*      if ( nRet != 0 ) */
/*      { */
/*          nRet=errno; */
/*  #ifdef DEBUG_OSL_FILE */
/*          perror("mount"); */
/*  #endif */
/*      } */

    pszTmp = getenv("SAL_MOUNT_MOUNTPROG");
    if ( pszTmp != 0 )
    {
        pszMountProg=pszTmp;
    }

    pszTmp=getenv("SAL_MOUNT_SU_DO");
    if ( pszTmp != 0 )
    {
        pszSuDo=pszTmp;
    }

    if ( pszSuDo != 0 )
    {
        snprintf(pszCmd, sizeof(pszCmd), "%s %s %s %s",pszSuDo,pszMountProg,pItem->pszDevice,pItem->pszMountPoint);
    }
    else
    {
        snprintf(pszCmd, sizeof(pszCmd), "%s %s",pszMountProg,pItem->pszMountPoint);
    }


#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"executing '%s'\n",pszCmd);
#endif

    nRet = system(pszCmd);

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"call returned '%i'\n",nRet);
    fprintf(stderr,"exit status is '%i'\n", WEXITSTATUS(nRet));
#endif


    switch ( WEXITSTATUS(nRet) )
    {
    case 0:
        nRet=0;
        break;

    case 2:
        nRet=EPERM;
        break;

    case 4:
        nRet=ENOENT;
        break;

    case 8:
        nRet=EINTR;
        break;

    case 16:
        nRet=EPERM;
        break;

    case 32:
        nRet=EBUSY;
        break;

    case 64:
        nRet=EAGAIN;
        break;

    default:
        nRet=EBUSY;
        break;
    }

    return ((0 == nRet) ? oslTranslateFileError(OSL_FET_SUCCESS, nRet) : oslTranslateFileError(OSL_FET_ERROR, nRet));
}
#endif /* LINUX */


#if defined(LINUX)
static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
    oslVolumeDeviceHandleImpl* pItem=0;
    int nRet=0;
    sal_Char pszCmd[PATH_MAX];
    sal_Char* pszTmp = 0;
    sal_Char* pszSuDo = 0;
    const sal_Char* pszUmountProg = "umount";

    pszCmd[0] = '\0';

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_unmountFloppy\n");
#endif

    pItem = (oslVolumeDeviceHandleImpl*) hFloppy;

    if ( pItem == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_unmountFloppy [pItem==0]\n");
#endif
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_unmountFloppy [invalid handle]\n");
#endif
        return osl_File_E_INVAL;
    }

    /* mfe: we can't use the umount(2) system call!!!  */
    /*      even if we are root                        */
    /*      since mtab is not updated!!!               */
    /*      but we need it to be updated               */
    /*      some "magic" must be done                  */

/*      nRet=umount(pItem->pszDevice); */
/*      if ( nRet != 0 ) */
/*      { */
/*          nRet = errno; */

/*  #ifdef DEBUG_OSL_FILE */
/*          perror("mount"); */
/*  #endif */
/*      } */


    pszTmp = getenv("SAL_MOUNT_UMOUNTPROG");
    if ( pszTmp != 0 )
    {
        pszUmountProg=pszTmp;
    }

    pszTmp = getenv("SAL_MOUNT_SU_DO");
    if ( pszTmp != 0 )
    {
        pszSuDo=pszTmp;
    }

    if ( pszSuDo != 0 )
    {
        snprintf(pszCmd, sizeof(pszCmd), "%s %s %s",pszSuDo,pszUmountProg,pItem->pszMountPoint);
    }
    else
    {
        snprintf(pszCmd, sizeof(pszCmd), "%s %s",pszUmountProg,pItem->pszMountPoint);
    }


#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"executing '%s'\n",pszCmd);
#endif

    nRet = system(pszCmd);

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"call returned '%i'\n",nRet);
    fprintf(stderr,"exit status is '%i'\n", WEXITSTATUS(nRet));
#endif

    switch ( WEXITSTATUS(nRet) )
    {
    case 0:
        nRet=0;
        break;

    default:
        nRet=EBUSY;
        break;
    }

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_unmountFloppy [ok]\n");
#endif

    return ((0 == nRet) ? oslTranslateFileError(OSL_FET_SUCCESS, nRet) : oslTranslateFileError(OSL_FET_ERROR, nRet));

/*    return osl_File_E_None;*/
}

#endif /* LINUX */

#if defined(LINUX)
static sal_Bool
osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem)
{
    struct mntent* pMountEnt;
    FILE*          pMountTab;

    pMountTab = setmntent (MOUNTTAB, "r");
    if (pMountTab == 0)
        return sal_False;

    while ((pMountEnt = getmntent(pMountTab)) != 0)
    {
        if (   strncmp(pMountEnt->mnt_dir,    pszPath,   strlen(pMountEnt->mnt_dir)) == 0
            && strncmp(pMountEnt->mnt_fsname, "/dev/fd", strlen("/dev/fd")) == 0)
        {
            memset(pItem->pszMountPoint, 0, sizeof(pItem->pszMountPoint));
            strncpy(pItem->pszMountPoint, pMountEnt->mnt_dir, sizeof(pItem->pszMountPoint) - 1);

            memset(pItem->pszFilePath, 0, sizeof(pItem->pszFilePath));
            strncpy(pItem->pszFilePath, pMountEnt->mnt_dir, sizeof(pItem->pszFilePath) - 1);

            memset(pItem->pszDevice, 0, sizeof(pItem->pszDevice));
            strncpy(pItem->pszDevice, pMountEnt->mnt_fsname, sizeof(pItem->pszDevice) - 1);

            endmntent (pMountTab);
            return sal_True;
        }
    }

    endmntent (pMountTab);
    return sal_False;
}
#endif /* LINUX */

#if defined(LINUX)
static sal_Bool
osl_isFloppyMounted (oslVolumeDeviceHandleImpl* pDevice)
{
    oslVolumeDeviceHandleImpl aItem;

    if (   osl_getFloppyMountEntry (pDevice->pszMountPoint, &aItem)
        && strcmp (aItem.pszMountPoint, pDevice->pszMountPoint) == 0
        && strcmp (aItem.pszDevice,     pDevice->pszDevice) == 0)
    {
        return sal_True;
    }
    return sal_False;
}
#endif /* LINUX */

/* NetBSD floppy functions have to be added here. Until we have done that,
 * we use the MACOSX definitions for nonexistent floppy.
 * */

/******************************************************************************
 *
 *                  MAC OS X FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if (defined(MACOSX) || defined(NETBSD) || defined(FREEBSD) || \
    defined(AIX) || defined(OPENBSD) || defined(DRAGONFLY))
static oslVolumeDeviceHandle osl_isFloppyDrive(const sal_Char* pszPath)
{
    (void)pszPath;
    return NULL;
}
#endif /* MACOSX */

#if ( defined(MACOSX) || defined(NETBSD) || defined(FREEBSD) || \
    defined(AIX) || defined(OPENBSD) || defined(DRAGONFLY))
static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{
    (void)hFloppy;
    return osl_File_E_BUSY;
}
#endif /* MACOSX */

#if ( defined(MACOSX) || defined(NETBSD) || defined(FREEBSD) || \
    defined(AIX) || defined(OPENBSD) || defined(DRAGONFLY))
static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
    (void)hFloppy;
    return osl_File_E_BUSY;
}
#endif /* MACOSX */

#if ( defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD) || defined(DRAGONFLY) )
static sal_Bool osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem)
{
    return sal_False;
}
#endif /* NETBSD || FREEBSD || OPENBSD */

#if ( defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD) || defined(DRAGONFLY) )
static sal_Bool osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice)
{
    return sal_False;
}
#endif /* NETBSD || FREEBSD || OPENBSD */


#ifdef DEBUG_OSL_FILE
static void osl_printFloppyHandle(oslVolumeDeviceHandleImpl* pItem)
{
    if (pItem == 0 )
    {
        fprintf(stderr,"NULL Handle\n");
        return;
    }
    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Invalid Handle]\n");
#endif
        return;
    }


    fprintf(stderr,"MountPoint : '%s'\n",pItem->pszMountPoint);
    fprintf(stderr,"FilePath   : '%s'\n",pItem->pszFilePath);
    fprintf(stderr,"Device     : '%s'\n",pItem->pszDevice);

    return;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
