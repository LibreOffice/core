/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: file.cxx,v $
 * $Revision: 1.18 $
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
#include "precompiled_sal.hxx"


/************************************************************************
 *   ToDo
 *
 *   Fix osl_getCanonicalName
 *
 *   - Fix: check for corresponding struct sizes in exported functions
 *   - check size/use of oslDirectory
 *   - check size/use of oslDirectoryItem
 *   - check size/use of oslFileStatus
 *   - check size/use of oslVolumeDeviceHandle
 *   - check size/use of oslVolumeInfo
 *   - check size/use of oslFileHandle
 ***********************************************************************/

#include <algorithm>
#include <limits>
#include "system.h"
#include <rtl/alloc.h>

#include "osl/file.hxx"


#include <sal/types.h>
#include <osl/thread.h>
#include <osl/diagnose.h>
#include "file_error_transl.h"
#include <osl/time.h>

#ifndef _FILE_URL_H_
#include "file_url.h"
#endif

#include "file_path_helper.hxx"
#include "uunxapi.hxx"


#include <sys/mman.h>

#ifdef HAVE_STATFS_H
#undef HAVE_STATFS_H
#endif

#ifndef _STRING_H
#include <string.h>
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
#include <ctype.h>
static const sal_Char* MOUNTTAB="/etc/mtab";

#elif defined(NETBSD) || defined(FREEBSD)
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <ufs/ufs/quota.h>
#include <ctype.h>
#define  HAVE_STATFS_H
/* No mounting table on *BSD
 * This information is stored only in the kernel. */
/* static const sal_Char* MOUNTTAB="/etc/mtab"; */

#elif defined(IRIX)
#include <mntent.h>
#include <sys/mount.h>
#include <sys/statvfs.h>
#define  HAVE_STATFS_H
#include <sys/quota.h>
#include <ctype.h>
static const sal_Char* MOUNTTAB="/etc/mtab";

#elif defined(MACOSX)
#include <ufs/ufs/quota.h>
#include <ctype.h>
// static const sal_Char* MOUNTTAB="/etc/mtab";

#include <sys/param.h>
#include <sys/mount.h>
#define HAVE_STATFS_H
#define HAVE_O_EXLOCK

// add MACOSX Time Value

#define TimeValue CFTimeValue
#include <CoreFoundation/CoreFoundation.h>
#undef TimeValue

#endif

#if OSL_DEBUG_LEVEL > 1

    extern void debug_ustring(rtl_uString*);

#endif


#ifdef DEBUG_OSL_FILE
#   define PERROR( a, b ) perror( a ); fprintf( stderr, b )
#else
#   define PERROR( a, b )
#endif

extern "C" oslFileHandle osl_createFileHandleFromFD( int fd );

/******************************************************************************
 *
 *                  Data Type Definition
 *
 ******************************************************************************/

#if 0
/* FIXME: reintroducing this may save some extra bytes per Item */
typedef struct
{
    rtl_uString* ustrFileName;       /* holds native file name */
    rtl_uString* ustrDirPath;        /* holds native dir path */
    sal_uInt32   RefCount;
} oslDirectoryItemImpl;
#endif

typedef struct
{
    rtl_uString* ustrPath;           /* holds native directory path */
    DIR*         pDirStruct;
} oslDirectoryImpl;


typedef struct
{
    rtl_uString* ustrFilePath;      /* holds native file path */
    int fd;
} oslFileHandleImpl;


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
 *                  static members
 *
 *****************************************************************************/

static const char * pFileLockEnvVar = (char *) -1;


/******************************************************************************
 *
 *                  C-String Function Declarations
 *
 *****************************************************************************/

static oslFileError osl_psz_getVolumeInformation(const sal_Char* , oslVolumeInfo* pInfo, sal_uInt32 uFieldMask);
static oslFileError osl_psz_removeFile(const sal_Char* pszPath);
static oslFileError osl_psz_createDirectory(const sal_Char* pszPath);
static oslFileError osl_psz_removeDirectory(const sal_Char* pszPath);
static oslFileError osl_psz_copyFile(const sal_Char* pszPath, const sal_Char* pszDestPath);
static oslFileError osl_psz_moveFile(const sal_Char* pszPath, const sal_Char* pszDestPath);
static oslFileError osl_psz_setFileAttributes(const sal_Char* pszFilePath, sal_uInt64 uAttributes);
static oslFileError osl_psz_setFileTime(const sal_Char* strFilePath, const TimeValue* pCreationTime, const TimeValue* pLastAccessTime, const TimeValue* pLastWriteTime);


/******************************************************************************
 *
 *                  Static Module Utility Function Declarations
 *
 *****************************************************************************/

static oslFileError  oslDoCopy(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, mode_t nMode, size_t nSourceSize, int DestFileExists);
static oslFileError  oslChangeFileModes(const sal_Char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID);
static int           oslDoCopyLink(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName);
static int           oslDoCopyFile(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, size_t nSourceSize, mode_t mode);
static oslFileError  oslDoMoveFile(const sal_Char* pszPath, const sal_Char* pszDestPath);
static rtl_uString*  oslMakeUStrFromPsz(const sal_Char* pszStr,rtl_uString** uStr);

/******************************************************************************
 *
 *                  Non-Static Utility Function Declarations
 *
 *****************************************************************************/

extern "C" int UnicodeToText( char *, size_t, const sal_Unicode *, sal_Int32 );
extern "C" int TextToUnicode(
    const char* text, size_t text_buffer_size,  sal_Unicode* unic_text, sal_Int32 unic_text_buffer_size);

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
#endif

#if defined(LINUX)
static sal_Bool       osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice);
static sal_Bool       osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem);
#endif


#if defined(IRIX)
static sal_Bool       osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice);
static sal_Bool       osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem);
#endif

#ifdef DEBUG_OSL_FILE
static void           osl_printFloppyHandle(oslVolumeDeviceHandleImpl* hFloppy);
#endif

#ifdef MACOSX

/*******************************************************************
 *  adjustLockFlags
 ******************************************************************/

/* The AFP implementation of MacOS X 10.4 treats O_EXLOCK in a way
 * that makes it impossible for OOo to create a backup copy of the
 * file it keeps opened. OTOH O_SHLOCK for AFP behaves as desired by
 * the OOo file handling, so we need to check the path of the file
 * for the filesystem name.
 */

static int adjustLockFlags(const char * path, int flags)
{
    struct statfs s;

    if( 0 <= statfs( path, &s ) )
    {
        if( 0 == strncmp("afpfs", s.f_fstypename, 5) )
        {
            flags &= ~O_EXLOCK;
            flags |= O_SHLOCK;
        }
    }

    return flags;
}

#endif


/*******************************************************************
 *  osl_openDirectory
 ******************************************************************/

oslFileError SAL_CALL osl_openDirectory(rtl_uString* ustrDirectoryURL, oslDirectory* pDirectory)
{
    rtl_uString* ustrSystemPath = NULL;
    oslFileError eRet;

    char path[PATH_MAX];

    OSL_ASSERT(ustrDirectoryURL && (ustrDirectoryURL->length > 0));
    OSL_ASSERT(pDirectory);

    if (0 == ustrDirectoryURL->length )
        return osl_File_E_INVAL;

    /* convert file URL to system path */
    eRet = osl_getSystemPathFromFileURL_Ex(ustrDirectoryURL, &ustrSystemPath, sal_False);

    if( osl_File_E_None != eRet )
        return eRet;

    osl_systemPathRemoveSeparator(ustrSystemPath);

    /* convert unicode path to text */
    if ( UnicodeToText( path, PATH_MAX, ustrSystemPath->buffer, ustrSystemPath->length )
#ifdef MACOSX
     && macxp_resolveAlias( path, PATH_MAX ) == 0
#endif /* MACOSX */
     )
    {
        /* open directory */
        DIR *pdir = opendir( path );

        if( pdir )
        {
            /* create and initialize impl structure */
            oslDirectoryImpl* pDirImpl = (oslDirectoryImpl*) rtl_allocateMemory( sizeof(oslDirectoryImpl) );

            if( pDirImpl )
            {
                pDirImpl->pDirStruct = pdir;
                pDirImpl->ustrPath = ustrSystemPath;

                *pDirectory = (oslDirectory) pDirImpl;
                return osl_File_E_None;
            }
            else
            {
                errno = ENOMEM;
                closedir( pdir );
            }
        }
        else
            /* should be removed by optimizer in product version */
            PERROR( "osl_openDirectory", path );
    }

    rtl_uString_release( ustrSystemPath );

    return oslTranslateFileError(OSL_FET_ERROR, errno);
}

/****************************************************************************/
/*  osl_closeDirectory */
/****************************************************************************/

oslFileError SAL_CALL osl_closeDirectory( oslDirectory Directory )
{
    oslDirectoryImpl* pDirImpl = (oslDirectoryImpl*) Directory;
    oslFileError err = osl_File_E_None;

    OSL_ASSERT( Directory );

    if( NULL == pDirImpl )
        return osl_File_E_INVAL;

    /* close directory */
    if( closedir( pDirImpl->pDirStruct ) )
    {
        err = oslTranslateFileError(OSL_FET_ERROR, errno);
    }

    /* cleanup members */
    rtl_uString_release( pDirImpl->ustrPath );

    rtl_freeMemory( pDirImpl );

    return err;
}

/**********************************************
 * osl_readdir_impl_
 *
 * readdir wrapper, filters out "." and ".."
 * on request
 *********************************************/

static struct dirent* osl_readdir_impl_(DIR* pdir, sal_Bool bFilterLocalAndParentDir)
{
    struct dirent* pdirent;

    while ((pdirent = readdir(pdir)) != NULL)
    {
        if (bFilterLocalAndParentDir &&
            ((0 == strcmp(pdirent->d_name, ".")) || (0 == strcmp(pdirent->d_name, ".."))))
            continue;
        else
            break;
    }

    return pdirent;
}

/****************************************************************************
 *  osl_getNextDirectoryItem
 ***************************************************************************/

oslFileError SAL_CALL osl_getNextDirectoryItem(oslDirectory Directory, oslDirectoryItem* pItem, sal_uInt32 /*uHint*/)
{
    oslDirectoryImpl* pDirImpl     = (oslDirectoryImpl*)Directory;
    rtl_uString*      ustrFileName = NULL;
    rtl_uString*      ustrFilePath = NULL;
    struct dirent*    pEntry;

    OSL_ASSERT(Directory);
    OSL_ASSERT(pItem);

    if ((NULL == Directory) || (NULL == pItem))
        return osl_File_E_INVAL;

    pEntry = osl_readdir_impl_(pDirImpl->pDirStruct, sal_True);

    if (NULL == pEntry)
        return osl_File_E_NOENT;


#if defined(MACOSX) && (BUILD_OS_MAJOR==10) && (BUILD_OS_MINOR>=2)

    // convert decomposed filename to precomposed unicode
    char composed_name[BUFSIZ];
    CFMutableStringRef strRef = CFStringCreateMutable (NULL, 0 );
    CFStringAppendCString( strRef, pEntry->d_name, kCFStringEncodingUTF8 );  //UTF8 is default on Mac OSX
    CFStringNormalize( strRef, kCFStringNormalizationFormC );
    CFStringGetCString( strRef, composed_name, BUFSIZ, kCFStringEncodingUTF8 );
    CFRelease( strRef );
    rtl_string2UString( &ustrFileName, composed_name, strlen( composed_name),
    osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );

#else  // not MACOSX
    /* convert file name to unicode */
    rtl_string2UString( &ustrFileName, pEntry->d_name, strlen( pEntry->d_name ),
        osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
    OSL_ASSERT(ustrFileName != 0);

#endif

    osl_systemPathMakeAbsolutePath(pDirImpl->ustrPath, ustrFileName, &ustrFilePath);
    rtl_uString_release( ustrFileName );

    /* use path as directory item */
    *pItem = (oslDirectoryItem) ustrFilePath;

    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_getDirectoryItem */
/****************************************************************************/

oslFileError SAL_CALL osl_getDirectoryItem( rtl_uString* ustrFileURL, oslDirectoryItem* pItem )
{
    rtl_uString* ustrSystemPath = NULL;
    oslFileError osl_error      = osl_File_E_INVAL;

    OSL_ASSERT(ustrFileURL);
    OSL_ASSERT(pItem);

    if (0 == ustrFileURL->length || NULL == pItem)
        return osl_File_E_INVAL;

    osl_error = osl_getSystemPathFromFileURL_Ex(ustrFileURL, &ustrSystemPath, sal_False);

    if (osl_File_E_None != osl_error)
        return osl_error;

    osl_systemPathRemoveSeparator(ustrSystemPath);

    if (0 == access_u(ustrSystemPath, F_OK))
    {
        *pItem = (oslDirectoryItem)ustrSystemPath;
        osl_error = osl_File_E_None;
    }
    else
    {
        osl_error = oslTranslateFileError(OSL_FET_ERROR, errno);
        rtl_uString_release(ustrSystemPath);
    }
    return osl_error;
}


/****************************************************************************/
/*  osl_acquireDirectoryItem */
/****************************************************************************/

oslFileError osl_acquireDirectoryItem( oslDirectoryItem Item )
{
    rtl_uString* ustrFilePath = (rtl_uString *) Item;

    OSL_ASSERT( Item );

    if( ustrFilePath )
        rtl_uString_acquire( ustrFilePath );

    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_releaseDirectoryItem */
/****************************************************************************/

oslFileError osl_releaseDirectoryItem( oslDirectoryItem Item )
{
    rtl_uString* ustrFilePath = (rtl_uString *) Item;

    OSL_ASSERT( Item );

    if( ustrFilePath )
        rtl_uString_release( ustrFilePath );

    return osl_File_E_None;
}

/****************************************************************************
 *  osl_createFileHandleFromFD
 ***************************************************************************/

oslFileHandle osl_createFileHandleFromFD( int fd )
{
    oslFileHandleImpl* pHandleImpl = NULL;

    if ( fd >= 0 )
    {
        pHandleImpl = (oslFileHandleImpl*) rtl_allocateMemory( sizeof(oslFileHandleImpl) );

        if( pHandleImpl )
        {
            pHandleImpl->ustrFilePath = NULL;
            rtl_uString_new( &pHandleImpl->ustrFilePath );
            pHandleImpl->fd = fd;
        }
    }

    return (oslFileHandle)pHandleImpl;
}


/****************************************************************************
 *  osl_openFile
 ***************************************************************************/

#ifdef HAVE_O_EXLOCK
#define OPEN_WRITE_FLAGS ( O_RDWR | O_EXLOCK | O_NONBLOCK )
#define OPEN_CREATE_FLAGS ( O_CREAT | O_EXCL | O_RDWR | O_EXLOCK | O_NONBLOCK )
#else
#define OPEN_WRITE_FLAGS ( O_RDWR )
#define OPEN_CREATE_FLAGS ( O_CREAT | O_EXCL | O_RDWR )
#endif

oslFileError osl_openFile( rtl_uString* ustrFileURL, oslFileHandle* pHandle, sal_uInt32 uFlags )
{
    oslFileHandleImpl* pHandleImpl = NULL;
    oslFileError eRet;
    rtl_uString* ustrFilePath = NULL;

    char buffer[PATH_MAX];
    int  fd;
    int  mode  = S_IRUSR | S_IRGRP | S_IROTH;
    int  flags = O_RDONLY;

    struct flock aflock;

    /* locking the complete file */
    aflock.l_type = 0;
    aflock.l_whence = SEEK_SET;
    aflock.l_start = 0;
    aflock.l_len = 0;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( pHandle );

    if( ( 0 == ustrFileURL->length ) )
        return osl_File_E_INVAL;

    /* convert file URL to system path */
    eRet = osl_getSystemPathFromFileURL( ustrFileURL, &ustrFilePath );

    if( osl_File_E_None != eRet )
        return eRet;

    osl_systemPathRemoveSeparator(ustrFilePath);

    /* convert unicode path to text */
    if( UnicodeToText( buffer, PATH_MAX, ustrFilePath->buffer, ustrFilePath->length )
#ifdef MACOSX
     && macxp_resolveAlias( buffer, PATH_MAX ) == 0
#endif /* MACOSX */
     )
    {
        /* we do not open devices or such here */
        if( !( uFlags & osl_File_OpenFlag_Create ) )
        {
            struct stat aFileStat;

            if( 0 > stat( buffer, &aFileStat ) )
            {
                PERROR( "osl_openFile", buffer );
                eRet = oslTranslateFileError(OSL_FET_ERROR, errno );
            }

            else if( !S_ISREG( aFileStat.st_mode ) )
            {
                eRet = osl_File_E_INVAL;
            }
        }

        if( osl_File_E_None == eRet )
        {
            /*
             * set flags and mode
             */

            if ( uFlags & osl_File_OpenFlag_Write )
            {
                mode |= S_IWUSR | S_IWGRP | S_IWOTH;
                flags = OPEN_WRITE_FLAGS;
#ifdef MACOSX
                flags = adjustLockFlags(buffer, flags);
#endif
                aflock.l_type = F_WRLCK;
            }

            if ( uFlags & osl_File_OpenFlag_Create )
            {
                mode |= S_IWUSR | S_IWGRP | S_IWOTH;
                flags = OPEN_CREATE_FLAGS;
#ifdef MACOSX
                flags = adjustLockFlags(buffer, flags);
#endif
            }

            /* open the file */
            fd = open( buffer, flags, mode );
            if ( fd >= 0 )
            {
#ifndef HAVE_O_EXLOCK
                /* check if file lock is enabled and clear l_type member of flock otherwise */
                if( (char *) -1 == pFileLockEnvVar )
                {
                    /* FIXME: this is not MT safe */
                    pFileLockEnvVar = getenv("SAL_ENABLE_FILE_LOCKING");

                    if( NULL == pFileLockEnvVar)
                        pFileLockEnvVar = getenv("STAR_ENABLE_FILE_LOCKING");
                }
#else
                /* disable range based locking */
                pFileLockEnvVar = NULL;

                /* remove the NONBLOCK flag again */
                flags = fcntl(fd, F_GETFL, NULL);
                flags &= ~O_NONBLOCK;
                if( 0 > fcntl(fd, F_GETFL, flags) )
                    return oslTranslateFileError(OSL_FET_ERROR, errno);
#endif
                if( NULL == pFileLockEnvVar )
                    aflock.l_type = 0;

                /* lock the file if flock.l_type is set */
                if( F_WRLCK != aflock.l_type || -1 != fcntl( fd, F_SETLK, &aflock ) )
                {
                    /* allocate memory for impl structure */
                    pHandleImpl = (oslFileHandleImpl*) rtl_allocateMemory( sizeof(oslFileHandleImpl) );
                    if( pHandleImpl )
                    {
                        pHandleImpl->ustrFilePath = ustrFilePath;
                        pHandleImpl->fd = fd;

                        *pHandle = (oslFileHandle) pHandleImpl;

                        return osl_File_E_None;
                    }
                    else
                    {
                        errno = ENOMEM;
                    }
                }

                close( fd );
            }

            PERROR( "osl_openFile", buffer );
            eRet = oslTranslateFileError(OSL_FET_ERROR, errno );
        }
    }
    else
        eRet = osl_File_E_INVAL;

    rtl_uString_release( ustrFilePath );
    return eRet;
}

/****************************************************************************/
/*  osl_closeFile */
/****************************************************************************/

oslFileError osl_closeFile( oslFileHandle Handle )
{
    oslFileHandleImpl* pHandleImpl = (oslFileHandleImpl *) Handle;
    oslFileError eRet = osl_File_E_INVAL;

    OSL_ASSERT( Handle );

    if( pHandleImpl )
    {
        rtl_uString_release( pHandleImpl->ustrFilePath );

        /* release file lock if locking is enabled */
        if( pFileLockEnvVar )
        {
            struct flock aflock;

            aflock.l_type = F_UNLCK;
            aflock.l_whence = SEEK_SET;
            aflock.l_start = 0;
            aflock.l_len = 0;

            /* FIXME: check if file is really locked ?  */

            /* release the file share lock on this file */
            if( -1 == fcntl( pHandleImpl->fd, F_SETLK, &aflock ) )
                PERROR( "osl_closeFile", "unlock failed" );
        }

        if( 0 > close( pHandleImpl->fd ) )
        {
            eRet = oslTranslateFileError(OSL_FET_ERROR, errno );
        }
        else
            eRet = osl_File_E_None;

        rtl_freeMemory( pHandleImpl );
    }

    return eRet;
}

/****************************************************************************/
/*  osl_isEndOfFile */
/****************************************************************************/

oslFileError SAL_CALL osl_isEndOfFile( oslFileHandle Handle, sal_Bool *pIsEOF )
{
    oslFileHandleImpl* pHandleImpl = (oslFileHandleImpl *) Handle;
    oslFileError eRet = osl_File_E_INVAL;

    if ( pHandleImpl)
    {
        long curPos = lseek( pHandleImpl->fd, 0, SEEK_CUR );

        if ( curPos >= 0 )
        {
            long endPos = lseek( pHandleImpl->fd, 0, SEEK_END  );

            if ( endPos >= 0 )
            {
                *pIsEOF = ( curPos == endPos );
                curPos = lseek( pHandleImpl->fd, curPos, SEEK_SET );

                if ( curPos >= 0 )
                    eRet = osl_File_E_None;
                else
                    eRet = oslTranslateFileError(OSL_FET_ERROR, errno );
            }
            else
                eRet = oslTranslateFileError(OSL_FET_ERROR, errno );
        }
        else
            eRet = oslTranslateFileError(OSL_FET_ERROR, errno );
    }

    return eRet;
}


/****************************************************************************/
/*  osl_moveFile */
/****************************************************************************/

oslFileError osl_moveFile( rtl_uString* ustrFileURL, rtl_uString* ustrDestURL )
{
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( ustrDestURL );

    /* convert source url to system path */
    eRet = FileURLToPath( srcPath, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    /* convert destination url to system path */
    eRet = FileURLToPath( destPath, PATH_MAX, ustrDestURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( srcPath, PATH_MAX ) != 0 || macxp_resolveAlias( destPath, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return oslDoMoveFile( srcPath, destPath );
}

/****************************************************************************/
/*  osl_copyFile */
/****************************************************************************/

oslFileError osl_copyFile( rtl_uString* ustrFileURL, rtl_uString* ustrDestURL )
{
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( ustrDestURL );

    /* convert source url to system path */
    eRet = FileURLToPath( srcPath, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    /* convert destination url to system path */
    eRet = FileURLToPath( destPath, PATH_MAX, ustrDestURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( srcPath, PATH_MAX ) != 0 || macxp_resolveAlias( destPath, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_copyFile( srcPath, destPath );
}

/****************************************************************************/
/*  osl_removeFile */
/****************************************************************************/

oslFileError osl_removeFile( rtl_uString* ustrFileURL )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_removeFile( path );
}

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

/****************************************************************************/
/*  osl_createDirectory */
/****************************************************************************/

oslFileError osl_createDirectory( rtl_uString* ustrDirectoryURL )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrDirectoryURL );

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_createDirectory( path );
}

/****************************************************************************/
/*  osl_removeDirectory */
/****************************************************************************/

oslFileError osl_removeDirectory( rtl_uString* ustrDirectoryURL )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrDirectoryURL );

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_removeDirectory( path );
}

//#############################################
int path_make_parent(sal_Unicode* path)
{
    int i = rtl_ustr_lastIndexOfChar(path, '/');

    if (i > 0)
    {
        *(path + i) = 0;
        return i;
    }
    else
        return 0;
}

//#############################################
int create_dir_with_callback(
    sal_Unicode* directory_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    int mode = S_IRWXU | S_IRWXG | S_IRWXO;

    if (osl::mkdir(directory_path, mode) == 0)
    {
        if (aDirectoryCreationCallbackFunc)
        {
            rtl::OUString url;
            osl::FileBase::getFileURLFromSystemPath(directory_path, url);
            aDirectoryCreationCallbackFunc(pData, url.pData);
        }
        return 0;
    }
    return errno;
}

//#############################################
oslFileError create_dir_recursively_(
    sal_Unicode* dir_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    OSL_PRECOND((rtl_ustr_getLength(dir_path) > 0) && ((dir_path + (rtl_ustr_getLength(dir_path) - 1)) != (dir_path + rtl_ustr_lastIndexOfChar(dir_path, '/'))), \
    "Path must not end with a slash");

    int native_err = create_dir_with_callback(
        dir_path, aDirectoryCreationCallbackFunc, pData);

    if (native_err == 0)
        return osl_File_E_None;

    if (native_err != ENOENT)
        return oslTranslateFileError(OSL_FET_ERROR, native_err);

    // we step back until '/a_dir' at maximum because
    // we should get an error unequal ENOENT when
    // we try to create 'a_dir' at '/' and would so
    // return before
    int pos = path_make_parent(dir_path);

    oslFileError osl_error = create_dir_recursively_(
        dir_path, aDirectoryCreationCallbackFunc, pData);

    if (osl_File_E_None != osl_error)
        return osl_error;

       dir_path[pos] = '/';

    return create_dir_recursively_(dir_path, aDirectoryCreationCallbackFunc, pData);
}

//#######################################
oslFileError SAL_CALL osl_createDirectoryPath(
    rtl_uString* aDirectoryUrl,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    if (aDirectoryUrl == NULL)
        return osl_File_E_INVAL;

    rtl::OUString sys_path;
    oslFileError osl_error = osl_getSystemPathFromFileURL_Ex(
        aDirectoryUrl, &sys_path.pData, sal_False);

    if (osl_error != osl_File_E_None)
        return osl_error;

    osl::systemPathRemoveSeparator(sys_path);

    // const_cast because sys_path is a local copy which we want to modify inplace instead of
    // coyp it into another buffer on the heap again
    return create_dir_recursively_(sys_path.pData->buffer, aDirectoryCreationCallbackFunc, pData);
}

/****************************************************************************/
/*  osl_getCanonicalName */
/****************************************************************************/

oslFileError osl_getCanonicalName( rtl_uString* ustrFileURL, rtl_uString** pustrValidURL )
{
    OSL_ENSURE(sal_False, "osl_getCanonicalName not implemented");

    rtl_uString_newFromString(pustrValidURL, ustrFileURL);
    return osl_File_E_None;
}


/****************************************************************************/
/*  osl_setFileAttributes */
/****************************************************************************/

oslFileError osl_setFileAttributes( rtl_uString* ustrFileURL, sal_uInt64 uAttributes )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_setFileAttributes( path, uAttributes );
}

/****************************************************************************/
/*  osl_setFileTime */
/****************************************************************************/

oslFileError osl_setFileTime( rtl_uString* ustrFileURL, const TimeValue* pCreationTime,
                              const TimeValue* pLastAccessTime, const TimeValue* pLastWriteTime )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( OSL_FET_ERROR, errno );
#endif/* MACOSX */

    return osl_psz_setFileTime( path, pCreationTime, pLastAccessTime, pLastWriteTime );
}

/******************************************************************************
 *
 *                  Exported Module Functions
 *             (independent of C or Unicode Strings)
 *
 *****************************************************************************/


/*******************************************
    osl_readFile
********************************************/

oslFileError osl_readFile(oslFileHandle Handle, void* pBuffer, sal_uInt64 uBytesRequested, sal_uInt64* pBytesRead)
{
    ssize_t            nBytes      = 0;
    oslFileHandleImpl* pHandleImpl = (oslFileHandleImpl*)Handle;

    if ((0 == pHandleImpl) || (pHandleImpl->fd < 0) || (0 == pBuffer) || (0 == pBytesRead))
        return osl_File_E_INVAL;

    nBytes = read(
        pHandleImpl->fd, pBuffer,
        ((uBytesRequested
          <= static_cast< size_t >(std::numeric_limits< ssize_t >::max()))
         ? static_cast< size_t >(uBytesRequested)
         : static_cast< size_t >(std::numeric_limits< ssize_t >::max())));

    if (-1 == nBytes)
        return oslTranslateFileError(OSL_FET_ERROR, errno);

    *pBytesRead = nBytes;
    return osl_File_E_None;
}

/*******************************************
    osl_writeFile
********************************************/

oslFileError osl_writeFile(oslFileHandle Handle, const void* pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64* pBytesWritten)
{
    ssize_t            nBytes      = 0;
    oslFileHandleImpl* pHandleImpl = (oslFileHandleImpl*)Handle;

    OSL_ASSERT(pHandleImpl);
    OSL_ASSERT(pBuffer);
    OSL_ASSERT(pBytesWritten);

    if ((0 == pHandleImpl) || (0 == pBuffer) || (0 == pBytesWritten))
        return osl_File_E_INVAL;

    OSL_ASSERT(pHandleImpl->fd >= 0);

    if (pHandleImpl->fd < 0)
        return osl_File_E_INVAL;

    nBytes = write(
        pHandleImpl->fd, pBuffer,
        ((uBytesToWrite
          <= static_cast< size_t >(std::numeric_limits< ssize_t >::max()))
         ? static_cast< size_t >(uBytesToWrite)
         : static_cast< size_t >(std::numeric_limits< ssize_t >::max())));

    if (-1 == nBytes)
        return oslTranslateFileError(OSL_FET_ERROR, errno);

    *pBytesWritten = nBytes;
    return osl_File_E_None;
}

/*******************************************
    osl_writeFile
********************************************/

oslFileError osl_setFilePos( oslFileHandle Handle, sal_uInt32 uHow, sal_Int64 uPos )
{
    oslFileHandleImpl* pHandleImpl=0;
    int nRet=0;
    off_t nOffset=0;

    pHandleImpl = (oslFileHandleImpl*) Handle;
    if ( pHandleImpl == 0 )
    {
        return osl_File_E_INVAL;
    }

    if ( pHandleImpl->fd < 0 )
    {
        return osl_File_E_INVAL;
    }

    /* FIXME mfe: setFilePos: Do we have any runtime function to determine LONG_MAX? */
    if ( uPos > LONG_MAX )
    {
        return osl_File_E_OVERFLOW;
    }

    nOffset=(off_t)uPos;

    switch(uHow)
    {
        case osl_Pos_Absolut:
            nOffset = lseek(pHandleImpl->fd,nOffset,SEEK_SET);
            break;

        case osl_Pos_Current:
            nOffset = lseek(pHandleImpl->fd,nOffset,SEEK_CUR);
            break;

        case osl_Pos_End:
            nOffset = lseek(pHandleImpl->fd,nOffset,SEEK_END);
            break;

        default:
            return osl_File_E_INVAL;
    }

    if ( nOffset < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    return osl_File_E_None;
}

/************************************************
 * osl_getFilePos
 ***********************************************/

oslFileError osl_getFilePos( oslFileHandle Handle, sal_uInt64* pPos )
{
    oslFileHandleImpl* pHandleImpl=0;
    off_t nOffset=0;
    int nRet=0;

    pHandleImpl = (oslFileHandleImpl*) Handle;
    if ( pHandleImpl == 0 || pPos == 0)
    {
        return osl_File_E_INVAL;
    }

    if ( pHandleImpl->fd < 0 )
    {
        return osl_File_E_INVAL;
    }

    nOffset = lseek(pHandleImpl->fd,0,SEEK_CUR);

    if (nOffset < 0)
    {
        nRet  =errno;

        /* *pPos =0; */

        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    *pPos=nOffset;

    return osl_File_E_None;
}

/****************************************************************************
 *  osl_getFileSize
 ****************************************************************************/

oslFileError osl_getFileSize( oslFileHandle Handle, sal_uInt64* pSize )
{
    oslFileHandleImpl* pHandleImpl=(oslFileHandleImpl*) Handle;
    if (pHandleImpl == 0)
        return osl_File_E_INVAL;

    struct stat file_stat;
    if (fstat(pHandleImpl->fd, &file_stat) == -1)
        return oslTranslateFileError(OSL_FET_ERROR, errno);

    *pSize = file_stat.st_size;
    return osl_File_E_None;
}

/************************************************
 * osl_setFileSize
 ***********************************************/

oslFileError osl_setFileSize( oslFileHandle Handle, sal_uInt64 uSize )
{
    oslFileHandleImpl* pHandleImpl=0;
    off_t nOffset=0;

    pHandleImpl = (oslFileHandleImpl*) Handle;
    if ( pHandleImpl == 0 )
    {
        return osl_File_E_INVAL;
    }

    if ( pHandleImpl->fd < 0 )
    {
        return osl_File_E_INVAL;
    }

    /* FIXME: mfe: setFileSize: Do we have any runtime function to determine LONG_MAX? */
    if ( uSize > LONG_MAX )
    {
        return osl_File_E_OVERFLOW;
    }

    nOffset = (off_t)uSize;
    if (ftruncate (pHandleImpl->fd, nOffset) < 0)
    {
        /* Failure. Try fallback algorithm */
        oslFileError result;
        struct stat  aStat;
        off_t        nCurPos;

        /* Save original result */
        result = oslTranslateFileError (OSL_FET_ERROR, errno);
        PERROR("ftruncate", "Try osl_setFileSize [fallback]\n");

        /* Check against current size. Fail upon 'shrink' */
        if (fstat (pHandleImpl->fd, &aStat) < 0)
        {
            PERROR("ftruncate: fstat", "Out osl_setFileSize [error]\n");
            return (result);
        }
        if ((0 <= nOffset) && (nOffset <= aStat.st_size))
        {
            /* Failure upon 'shrink'. Return original result */
            return (result);
        }

        /* Save current position */
        nCurPos = (off_t)lseek (pHandleImpl->fd, (off_t)0, SEEK_CUR);
        if (nCurPos == (off_t)(-1))
        {
            PERROR("ftruncate: lseek", "Out osl_setFileSize [error]\n");
            return (result);
        }

        /* Try 'expand' via 'lseek()' and 'write()' */
        if (lseek (pHandleImpl->fd, (off_t)(nOffset - 1), SEEK_SET) < 0)
        {
            PERROR("ftruncate: lseek", "Out osl_setFileSize [error]\n");
            return (result);
        }
        if (write (pHandleImpl->fd, (char*)"", (size_t)1) < 0)
        {
            /* Failure. Restore saved position */
            PERROR("ftruncate: write", "Out osl_setFileSize [error]\n");
            if (lseek (pHandleImpl->fd, (off_t)nCurPos, SEEK_SET) < 0)
            {
#ifdef DEBUG_OSL_FILE
                perror("ftruncate: lseek");
#endif /* DEBUG_OSL_FILE */
            }
            return (result);
        }

        /* Success. Restore saved position */
        if (lseek (pHandleImpl->fd, (off_t)nCurPos, SEEK_SET) < 0)
        {
            PERROR("ftruncate: lseek", "Out osl_setFileSize [error]");
            return (result);
        }
    }

    return (osl_File_E_None);
}

/*###############################################*/
oslFileError SAL_CALL osl_syncFile(oslFileHandle Handle)
{
    oslFileHandleImpl* handle_impl = (oslFileHandleImpl*)Handle;

    if (handle_impl == 0)
        return osl_File_E_INVAL;

    if (fsync(handle_impl->fd) == -1)
        return oslTranslateFileError(OSL_FET_ERROR, errno);

    return osl_File_E_None;
}

/******************************************************************************
 *
 *                  C-String Versions of Exported Module Functions
 *
 *****************************************************************************/

#ifdef HAVE_STATFS_H

#if defined(FREEBSD) || defined(NETBSD) || defined(MACOSX)
#   define __OSL_STATFS_STRUCT                  struct statfs
#   define __OSL_STATFS(dir, sfs)               statfs((dir), (sfs))
#   define __OSL_STATFS_BLKSIZ(a)               ((sal_uInt64)((a).f_bsize))
#   define __OSL_STATFS_TYPENAME(a)             ((a).f_fstypename)
#   define __OSL_STATFS_ISREMOTE(a)             (((a).f_type & MNT_LOCAL) == 0)

/* always return true if queried for the properties of
   the file system. If you think this is wrong under any
   of the target platforms fix it!!!! */
#   define __OSL_STATFS_IS_CASE_SENSITIVE_FS(a)  (1)
#   define __OSL_STATFS_IS_CASE_PRESERVING_FS(a) (1)
#endif /* FREEBSD || NETBSD */

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

#if defined(SOLARIS) || defined(IRIX)
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
#endif /* SOLARIS || IRIX*/

#   define __OSL_STATFS_INIT(a)         (memset(&(a), 0, sizeof(__OSL_STATFS_STRUCT)))

#else /* no statfs available */

#   define __OSL_STATFS_STRUCT                   struct dummy {int i;}
#   define __OSL_STATFS_INIT(a)                  ((void)0)
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

/*************************************
 * osl_psz_setFileAttributes
 ************************************/

static oslFileError osl_psz_setFileAttributes( const sal_Char* pszFilePath, sal_uInt64 uAttributes )
{
    oslFileError osl_error = osl_File_E_None;
    mode_t       nNewMode  = 0;

     OSL_ENSURE(!(osl_File_Attribute_Hidden & uAttributes), "osl_File_Attribute_Hidden doesn't work under Unix");

    if (uAttributes & osl_File_Attribute_OwnRead)
        nNewMode |= S_IRUSR;

    if (uAttributes & osl_File_Attribute_OwnWrite)
        nNewMode|=S_IWUSR;

    if  (uAttributes & osl_File_Attribute_OwnExe)
        nNewMode|=S_IXUSR;

    if (uAttributes & osl_File_Attribute_GrpRead)
        nNewMode|=S_IRGRP;

    if (uAttributes & osl_File_Attribute_GrpWrite)
        nNewMode|=S_IWGRP;

    if (uAttributes & osl_File_Attribute_GrpExe)
        nNewMode|=S_IXGRP;

    if (uAttributes & osl_File_Attribute_OthRead)
        nNewMode|=S_IROTH;

    if (uAttributes & osl_File_Attribute_OthWrite)
        nNewMode|=S_IWOTH;

    if (uAttributes & osl_File_Attribute_OthExe)
        nNewMode|=S_IXOTH;

    if (chmod(pszFilePath, nNewMode) < 0)
        osl_error = oslTranslateFileError(OSL_FET_ERROR, errno);

    return osl_error;
}

/******************************************
 * osl_psz_setFileTime
 *****************************************/

static oslFileError osl_psz_setFileTime( const sal_Char* pszFilePath,
                                  const TimeValue* /*pCreationTime*/,
                                  const TimeValue* pLastAccessTime,
                                  const TimeValue* pLastWriteTime )
{
    int nRet=0;
    struct utimbuf aTimeBuffer;
    struct stat aFileStat;
#ifdef DEBUG_OSL_FILE
    struct tm* pTM=0;
#endif

    nRet = lstat(pszFilePath,&aFileStat);

    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"File Times are (in localtime):\n");
    pTM=localtime(&aFileStat.st_ctime);
    fprintf(stderr,"CreationTime is '%s'\n",asctime(pTM));
    pTM=localtime(&aFileStat.st_atime);
    fprintf(stderr,"AccessTime   is '%s'\n",asctime(pTM));
    pTM=localtime(&aFileStat.st_mtime);
    fprintf(stderr,"Modification is '%s'\n",asctime(pTM));

    fprintf(stderr,"File Times are (in UTC):\n");
    fprintf(stderr,"CreationTime is '%s'\n",ctime(&aFileStat.st_ctime));
    fprintf(stderr,"AccessTime   is '%s'\n",ctime(&aTimeBuffer.actime));
    fprintf(stderr,"Modification is '%s'\n",ctime(&aTimeBuffer.modtime));
#endif

    if ( pLastAccessTime != 0 )
    {
        aTimeBuffer.actime=pLastAccessTime->Seconds;
    }
    else
    {
        aTimeBuffer.actime=aFileStat.st_atime;
    }

    if ( pLastWriteTime != 0 )
    {
        aTimeBuffer.modtime=pLastWriteTime->Seconds;
    }
    else
    {
        aTimeBuffer.modtime=aFileStat.st_mtime;
    }

    /* mfe: Creation time not used here! */

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"File Times are (in localtime):\n");
    pTM=localtime(&aFileStat.st_ctime);
    fprintf(stderr,"CreationTime now '%s'\n",asctime(pTM));
    pTM=localtime(&aTimeBuffer.actime);
    fprintf(stderr,"AccessTime   now '%s'\n",asctime(pTM));
    pTM=localtime(&aTimeBuffer.modtime);
    fprintf(stderr,"Modification now '%s'\n",asctime(pTM));

    fprintf(stderr,"File Times are (in UTC):\n");
    fprintf(stderr,"CreationTime now '%s'\n",ctime(&aFileStat.st_ctime));
    fprintf(stderr,"AccessTime   now '%s'\n",ctime(&aTimeBuffer.actime));
    fprintf(stderr,"Modification now '%s'\n",ctime(&aTimeBuffer.modtime));
#endif

    nRet=utime(pszFilePath,&aTimeBuffer);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    return osl_File_E_None;
}


/*****************************************
 * osl_psz_removeFile
 ****************************************/

static oslFileError osl_psz_removeFile( const sal_Char* pszPath )
{
    int nRet=0;
    struct stat aStat;

    nRet = lstat(pszPath,&aStat);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    if ( S_ISDIR(aStat.st_mode) )
    {
        return osl_File_E_ISDIR;
    }

    nRet = unlink(pszPath);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    return osl_File_E_None;
}

/*****************************************
 * osl_psz_createDirectory
 ****************************************/

static oslFileError osl_psz_createDirectory( const sal_Char* pszPath )
{
    int nRet=0;
    int mode = S_IRWXU | S_IRWXG | S_IRWXO;

    nRet = mkdir(pszPath,mode);

    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    return osl_File_E_None;
}

/*****************************************
 * osl_psz_removeDirectory
 ****************************************/

static oslFileError osl_psz_removeDirectory( const sal_Char* pszPath )
{
    int nRet=0;

    nRet = rmdir(pszPath);

    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    return osl_File_E_None;
}

/*****************************************
 * oslDoMoveFile
 ****************************************/

static oslFileError oslDoMoveFile( const sal_Char* pszPath, const sal_Char* pszDestPath)
{
    oslFileError tErr=osl_File_E_invalidError;

    tErr = osl_psz_moveFile(pszPath,pszDestPath);
    if ( tErr == osl_File_E_None )
    {
        return tErr;
    }

    if ( tErr != osl_File_E_XDEV )
    {
        return tErr;
    }

    tErr=osl_psz_copyFile(pszPath,pszDestPath);

    if ( tErr != osl_File_E_None )
    {
        oslFileError tErrRemove;
        tErrRemove=osl_psz_removeFile(pszDestPath);
        return tErr;
    }

    tErr=osl_psz_removeFile(pszPath);

    return tErr;
}

/*****************************************
 * osl_psz_moveFile
 ****************************************/

static oslFileError osl_psz_moveFile(const sal_Char* pszPath, const sal_Char* pszDestPath)
{

    int nRet = 0;

    nRet = rename(pszPath,pszDestPath);

    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    return osl_File_E_None;
}

/*****************************************
 * osl_psz_copyFile
 ****************************************/

static oslFileError osl_psz_copyFile( const sal_Char* pszPath, const sal_Char* pszDestPath )
{
    time_t nAcTime=0;
    time_t nModTime=0;
    uid_t nUID=0;
    gid_t nGID=0;
    int nRet=0;
    mode_t nMode=0;
    struct stat aFileStat;
    oslFileError tErr=osl_File_E_invalidError;
    size_t nSourceSize=0;
    int DestFileExists=1;

    /* mfe: does the source file really exists? */
    nRet = lstat(pszPath,&aFileStat);

    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    /* mfe: we do only copy files here! */
    if ( S_ISDIR(aFileStat.st_mode) )
    {
        return osl_File_E_ISDIR;
    }

    nSourceSize=(size_t)aFileStat.st_size;
    nMode=aFileStat.st_mode;
    nAcTime=aFileStat.st_atime;
    nModTime=aFileStat.st_mtime;
    nUID=aFileStat.st_uid;
    nGID=aFileStat.st_gid;

    nRet = stat(pszDestPath,&aFileStat);
    if ( nRet < 0 )
    {
        nRet=errno;

        if ( nRet == ENOENT )
        {
            DestFileExists=0;
        }
/*        return oslTranslateFileError(nRet);*/
    }

    /* mfe: the destination file must not be a directory! */
    if ( nRet == 0 && S_ISDIR(aFileStat.st_mode) )
    {
        return osl_File_E_ISDIR;
    }
    else
    {
        /* mfe: file does not exists or is no dir */
    }

    tErr = oslDoCopy(pszPath,pszDestPath,nMode,nSourceSize,DestFileExists);

    if ( tErr != osl_File_E_None )
    {
        return tErr;
    }

    /*
     *   mfe: ignore return code
     *        since only  the success of the copy is
     *        important
     */
    oslChangeFileModes(pszDestPath,nMode,nAcTime,nModTime,nUID,nGID);

    return tErr;
}


/******************************************************************************
 *
 *                  Utility Functions
 *
 *****************************************************************************/

/*****************************************
 * oslDoCopy
 ****************************************/

#define TMP_DEST_FILE_EXTENSION ".osl-tmp"

static oslFileError oslDoCopy(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, mode_t nMode, size_t nSourceSize, int DestFileExists)
{
    int      nRet=0;
    sal_Char pszTmpDestFile[PATH_MAX];
    size_t   size_tmp_dest_buff = sizeof(pszTmpDestFile);

    /* Quick fix for #106048, the whole copy file function seems
       to be erroneous anyway and needs to be rewritten.
       Besides osl_copyFile is currently not used from OO/SO code.
    */
    memset(pszTmpDestFile, 0, size_tmp_dest_buff);

    if ( DestFileExists )
    {
        strncpy(pszTmpDestFile, pszDestFileName, size_tmp_dest_buff - 1);

        if ((strlen(pszTmpDestFile) + strlen(TMP_DEST_FILE_EXTENSION)) >= size_tmp_dest_buff)
            return osl_File_E_NAMETOOLONG;

        strncat(pszTmpDestFile, TMP_DEST_FILE_EXTENSION, strlen(TMP_DEST_FILE_EXTENSION));

        /* FIXME: what if pszTmpDestFile already exists? */
        /*        with getcanonical??? */
        nRet=rename(pszDestFileName,pszTmpDestFile);
    }

    /* mfe: should be S_ISREG */
    if ( !S_ISLNK(nMode) )
    {
        /* copy SourceFile to DestFile */
        nRet = oslDoCopyFile(pszSourceFileName,pszDestFileName,nSourceSize, nMode);
    }
    /* mfe: OK redundant at the moment */
    else if ( S_ISLNK(nMode) )
    {
        nRet = oslDoCopyLink(pszSourceFileName,pszDestFileName);
    }
    else
    {
        /* mfe: what to do here? */
        nRet=ENOSYS;
    }

    if ( nRet > 0 && DestFileExists == 1 )
    {
        unlink(pszDestFileName);
        rename(pszTmpDestFile,pszDestFileName);
    }

    if ( nRet > 0 )
    {
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    if ( DestFileExists == 1 )
    {
        unlink(pszTmpDestFile);
    }

    return osl_File_E_None;
}

/*****************************************
 * oslChangeFileModes
 ****************************************/

static oslFileError oslChangeFileModes( const sal_Char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID)
{
    int nRet=0;
    struct utimbuf aTimeBuffer;

    nRet = chmod(pszFileName,nMode);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    aTimeBuffer.actime=nAcTime;
    aTimeBuffer.modtime=nModTime;
    nRet=utime(pszFileName,&aTimeBuffer);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    if ( nUID != getuid() )
    {
        nUID=getuid();
    }

    nRet=chown(pszFileName,nUID,nGID);
    if ( nRet < 0 )
    {
        nRet=errno;

        /* mfe: do not return an error here! */
        /* return oslTranslateFileError(nRet);*/
    }

    return osl_File_E_None;
}

/*****************************************
 * oslDoCopyLink
 ****************************************/

static int oslDoCopyLink(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName)
{
    int nRet=0;

    /* mfe: if dest file is symbolic link remove the link and place the file instead (hro says so) */
    /* mfe: if source is a link copy the link and not the file it points to (hro says so) */
    sal_Char pszLinkContent[PATH_MAX];

    pszLinkContent[0] = '\0';

    nRet = readlink(pszSourceFileName,pszLinkContent,PATH_MAX);

    if ( nRet < 0 )
    {
        nRet=errno;
        return nRet;
    }
    else
        pszLinkContent[ nRet ] = 0;

    nRet = symlink(pszLinkContent,pszDestFileName);

    if ( nRet < 0 )
    {
        nRet=errno;
        return nRet;
    }

    return 0;
}

/*****************************************
 * oslDoCopyFile
 ****************************************/

static int oslDoCopyFile(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, size_t nSourceSize, mode_t mode)
{
    int SourceFileFD=0;
    int DestFileFD=0;
    int nRet=0;
    void* pSourceFile=0;

    SourceFileFD=open(pszSourceFileName,O_RDONLY);
    if ( SourceFileFD < 0 )
    {
        nRet=errno;
        return nRet;
    }

    DestFileFD=open(pszDestFileName, O_WRONLY | O_CREAT, mode);

    if ( DestFileFD < 0 )
    {
        nRet=errno;
        close(SourceFileFD);
        return nRet;
    }

    /* HACK: because memory mapping fails on various
       platforms if the size of the source file is  0 byte */
    if (0 == nSourceSize)
    {
        close(SourceFileFD);
        close(DestFileFD);
        return 0;
    }

    /* FIXME doCopy: fall back code for systems not having mmap */
    /* mmap file -- open dest file -- write once -- fsync it */
    pSourceFile=mmap(0,nSourceSize,PROT_READ,MAP_PRIVATE,SourceFileFD,0);

    if ( pSourceFile == MAP_FAILED )
    {
        /* it's important to set nRet before the hack
           otherwise errno may be changed by lstat */
        nRet = errno;
        close(SourceFileFD);
        close(DestFileFD);

        return nRet;
    }

    nRet = write(DestFileFD,pSourceFile,nSourceSize);

    /* #112584# if 'write' could not write the requested number of bytes
             we have to fail of course; because it's not exactly specified if 'write'
            sets errno if less than requested byte could be written we set nRet
           explicitly to ENOSPC */
    if ((nRet < 0) || (nRet != sal::static_int_cast< int >(nSourceSize)))
    {
        if (nRet < 0)
            nRet = errno;
        else
            nRet = ENOSPC;

        close(SourceFileFD);
        close(DestFileFD);
        munmap((char*)pSourceFile,nSourceSize);
        return nRet;
    }

    nRet = munmap((char*)pSourceFile,nSourceSize);
    if ( nRet < 0 )
    {
        nRet=errno;
        close(SourceFileFD);
        close(DestFileFD);
        return nRet;
    }

    close(SourceFileFD);

    // Removed call to 'fsync' again (#112584#) and instead
    // evaluate the return value of 'close' in order to detect
    // and report ENOSPC and other erronous conditions on close
    if (close(DestFileFD) == -1)
        return errno;
    else
        return 0;
}

/*****************************************
 * oslMakeUStrFromPsz
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

/*****************************************************************************
 * UnicodeToText
 * converting unicode to text manually saves us the penalty of a temporary
 * rtl_String object.
 ****************************************************************************/

int UnicodeToText( char * buffer, size_t bufLen, const sal_Unicode * uniText, sal_Int32 uniTextLen )
{
    rtl_UnicodeToTextConverter hConverter;
    sal_uInt32   nInfo;
    sal_Size   nSrcChars, nDestBytes;

    /* stolen from rtl/string.c */
    hConverter = rtl_createUnicodeToTextConverter( osl_getThreadTextEncoding() );

    nDestBytes = rtl_convertUnicodeToText( hConverter, 0, uniText, uniTextLen,
                                           buffer, bufLen,
                                           OUSTRING_TO_OSTRING_CVTFLAGS | RTL_UNICODETOTEXT_FLAGS_FLUSH,
                                           &nInfo, &nSrcChars );

    rtl_destroyUnicodeToTextConverter( hConverter );

    if( nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL )
    {
        errno = EOVERFLOW;
        return 0;
    }

    /* ensure trailing '\0' */
    buffer[nDestBytes] = '\0';

    return nDestBytes;
}

/*****************************************************************************
   TextToUnicode

   @param text
          The text to convert.

   @param text_buffer_size
          The number of characters.

   @param unic_text
          The unicode buffer.

   @param unic_text_buffer_size
             The size in characters of the unicode buffer.

 ****************************************************************************/

int TextToUnicode(
    const char*  text,
    size_t       text_buffer_size,
    sal_Unicode* unic_text,
    sal_Int32    unic_text_buffer_size)
{
    rtl_TextToUnicodeConverter hConverter;
    sal_uInt32 nInfo;
    sal_Size nSrcChars;
    sal_Size nDestBytes;

    /* stolen from rtl/string.c */
    hConverter = rtl_createTextToUnicodeConverter(osl_getThreadTextEncoding());

    nDestBytes = rtl_convertTextToUnicode(hConverter,
                                          0,
                                          text,  text_buffer_size,
                                          unic_text, unic_text_buffer_size,
                                          OSTRING_TO_OUSTRING_CVTFLAGS | RTL_TEXTTOUNICODE_FLAGS_FLUSH,
                                          &nInfo, &nSrcChars);

    rtl_destroyTextToUnicodeConverter(hConverter);

    if (nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL)
    {
        errno = EOVERFLOW;
        return 0;
    }

    /* ensure trailing '\0' */
    unic_text[nDestBytes] = '\0';

    return nDestBytes;
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

#ifndef MACOSX

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
    for (i = 0; i < (sizeof(pFloppyDevice)/sizeof(pFloppyDevice[0])); i++)
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
    sal_Char* pszMountProg = "mount";
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
    sal_Char* pszUmountProg = "umount";

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

/******************************************************************************
 *
 *                  IRIX FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if defined(IRIX)
static oslVolumeDeviceHandle osl_isFloppyDrive(const sal_Char* pszPath)
{
    oslVolumeDeviceHandleImpl* pItem = osl_newVolumeDeviceHandleImpl ();
    sal_Bool bRet = sal_False;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_isFloppyDrive\n");
#endif

    bRet=osl_getFloppyMountEntry(pszPath,pItem);

    if ( bRet == sal_False )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_isFloppyDrive [not a floppy]\n");
#endif
        rtl_freeMemory(pItem);
        return 0;
    }


#ifdef DEBUG_OSL_FILE
    osl_printFloppyHandle(pItem);
#endif
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_isFloppyDrive [ok]\n");
#endif

    return (oslVolumeDeviceHandle) pItem;
}


static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{
    sal_Bool bRet = sal_False;
    oslVolumeDeviceHandleImpl* pItem=0;
    int nRet;
    sal_Char  pszCmd[PATH_MAX];
    sal_Char* pszMountProg = "mount";
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

static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
    oslVolumeDeviceHandleImpl* pItem=0;
    int nRet=0;
    sal_Char pszCmd[PATH_MAX];
    sal_Char* pszTmp = 0;
    sal_Char* pszSuDo = 0;
    sal_Char* pszUmountProg = "umount";

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

static sal_Bool osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem)
{
    struct mntent* pMountEnt=0;
    sal_Char buffer[PATH_MAX];
    FILE* mntfile=0;
    int nRet=0;

    buffer[0] = '\0';

    mntfile = setmntent(MOUNTTAB,"r");

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_getFloppyMountEntry\n");
#endif

    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, pszPath, sizeof(buffer) - 1);

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Checking mount of %s\n",buffer);
#endif


    if ( mntfile == 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("mounttab");
#endif
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getFloppyMountEntry [mntfile]\n");
#endif
        return sal_False;
    }

    pMountEnt=getmntent(mntfile);
    while ( pMountEnt != 0 )
    {
#ifdef DEBUG_OSL_FILE
/*           fprintf(stderr,"mnt_fsname : %s\n",pMountEnt->mnt_fsname); */
/*           fprintf(stderr,"mnt_dir    : %s\n",pMountEnt->mnt_dir); */
/*        fprintf(stderr,"mnt_type   : %s\n",pMountEnt->mnt_type);*/
#endif
        if ( strcmp(pMountEnt->mnt_dir,buffer) == 0 &&
             strncmp(pMountEnt->mnt_fsname,"/dev/fd",strlen("/dev/fd")) == 0 )
        {

            memset(pItem->pszMountPoint, 0, sizeof(pItem->pszMountPoint));
            strncpy(pItem->pszMountPoint, pMountEnt->mnt_dir, sizeof(pItem->pszMountPoint) - 1);

            memset(pItem->pszFilePath, 0, sizeof(pItem->pszFilePath));
            strncpy(pItem->pszFilePath, pMountEnt->mnt_dir, sizeof(pItem->pszFilePath) - 1);

            memset(pItem->pszDevice, 0, sizeof(pItem->pszDevice));
            strncpy(pItem->pszDevice, pMountEnt->mnt_fsname, sizeof(pItem->pszDevice) - 1);

            fclose(mntfile);
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"Mount Point found '%s'\n",pItem->pszMountPoint);
#endif
#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out osl_getFloppyMountEntry [found]\n");
#endif
            return sal_True;
        }
#ifdef DEBUG_OSL_FILE
/*        fprintf(stderr,"=================\n");*/
#endif
        pMountEnt=getmntent(mntfile);
    }

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_getFloppyMountEntry [not found]\n");
#endif

    fclose(mntfile);
    return sal_False;
}

static sal_Bool osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice)
{
    sal_Char buffer[PATH_MAX];
    oslVolumeDeviceHandleImpl* pItem=0;
    sal_Bool bRet=0;

    buffer[0] = '\0';

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_isFloppyMounted\n");
#endif

    pItem = osl_newVolumeDeviceHandleImpl ();
    if ( pItem == 0 )
        return osl_File_E_NOMEM;

    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, pDevice->pszMountPoint, sizeof(buffer) - 1);

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Checking mount of %s\n",buffer);
#endif

    bRet = osl_getFloppyMountEntry(buffer,pItem);

    if ( bRet == sal_False )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_isFloppyMounted [not mounted]\n");
#endif
        return sal_False;
    }

    if (strcmp(pItem->pszMountPoint, pDevice->pszMountPoint) == 0 &&
        strcmp(pItem->pszDevice,pDevice->pszDevice) == 0)
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_isFloppyMounted [is mounted]\n");
#endif
        rtl_freeMemory(pItem);
        return sal_True;
    }

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_isFloppyMounted [may be EBUSY]\n");
#endif

    rtl_freeMemory(pItem);
    return sal_False;
}
#endif /* IRIX */


/* NetBSD floppy functions have to be added here. Until we have done that,
 * we use the MACOSX definitions for nonexistent floppy.
 * */

/******************************************************************************
 *
 *                  MAC OS X FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if (defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
static oslVolumeDeviceHandle osl_isFloppyDrive(const sal_Char* pszPath)
{
    return NULL;
}
#endif /* MACOSX */

#if ( defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{
    return osl_File_E_BUSY;
}
#endif /* MACOSX */

#if ( defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
    return osl_File_E_BUSY;
}
#endif /* MACOSX */

#if ( defined(NETBSD) || defined(FREEBSD) )
static sal_Bool osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem)
{
    return sal_False;
}
#endif /* NETBSD || FREEBSD */

#if ( defined(NETBSD) || defined(FREEBSD) )
static sal_Bool osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice)
{
    return sal_False;
}
#endif /* NETBSD || FREEBSD */


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

