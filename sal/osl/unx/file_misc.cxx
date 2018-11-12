/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <osl/file.hxx>
#include <osl/detail/file.h>

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/signal.h>
#include <rtl/alloc.h>
#include <rtl/string.hxx>
#include <sal/log.hxx>

#include "system.hxx"
#include "file_impl.hxx"
#include "file_error_transl.hxx"
#include "file_path_helper.hxx"
#include "file_url.hxx"
#include "uunxapi.hxx"
#include "readwrite_helper.hxx"
#include "unixerrnostring.hxx"

#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <algorithm>
#include <cassert>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

/************************************************************************
 *   TODO
 *
 *   - Fix: check for corresponding struct sizes in exported functions
 *   - check size/use of oslDirectory
 *   - check size/use of oslDirectoryItem
 ***********************************************************************/

typedef struct
{
    rtl_uString* ustrPath;           /* holds native directory path */
    DIR*         pDirStruct;
#ifdef ANDROID
    enum Kind
    {
        KIND_DIRENT = 1,
        KIND_ASSETS = 2
    };
    int eKind;
    lo_apk_dir*  pApkDirStruct;
#endif
} oslDirectoryImpl;

DirectoryItem_Impl::DirectoryItem_Impl(
    rtl_uString * ustrFilePath, unsigned char DType)
    : m_RefCount     (1),
      m_ustrFilePath (ustrFilePath),
      m_DType        (DType)
{
    if (m_ustrFilePath != nullptr)
        rtl_uString_acquire(m_ustrFilePath);
}
DirectoryItem_Impl::~DirectoryItem_Impl()
{
    if (m_ustrFilePath != nullptr)
        rtl_uString_release(m_ustrFilePath);
}

void * DirectoryItem_Impl::operator new(size_t n)
{
    return malloc(n);
}
void DirectoryItem_Impl::operator delete(void * p)
{
    free(p);
}

void DirectoryItem_Impl::acquire()
{
    ++m_RefCount;
}
void DirectoryItem_Impl::release()
{
    if (--m_RefCount == 0)
        delete this;
}

oslFileType DirectoryItem_Impl::getFileType() const
{
    switch (m_DType)
    {
#ifdef _DIRENT_HAVE_D_TYPE
        case DT_LNK:
            return osl_File_Type_Link;
        case DT_DIR:
            return osl_File_Type_Directory;
        case DT_REG:
            return osl_File_Type_Regular;
        case DT_FIFO:
            return osl_File_Type_Fifo;
        case DT_SOCK:
            return osl_File_Type_Socket;
        case DT_CHR:
        case DT_BLK:
            return osl_File_Type_Special;
#endif /* _DIRENT_HAVE_D_TYPE */
        default:
            break;
    }
    return osl_File_Type_Unknown;
}

static oslFileError osl_psz_createDirectory(
    char const * pszPath, sal_uInt32 flags);
static oslFileError osl_psz_removeDirectory(const sal_Char* pszPath);

oslFileError SAL_CALL osl_openDirectory(rtl_uString* ustrDirectoryURL, oslDirectory* pDirectory)
{
    rtl_uString* ustrSystemPath = nullptr;
    oslFileError eRet;

    char path[PATH_MAX];

    if ((ustrDirectoryURL == nullptr) || (ustrDirectoryURL->length == 0) || (pDirectory == nullptr))
        return osl_File_E_INVAL;

    /* convert file URL to system path */
    eRet = osl_getSystemPathFromFileURL_Ex(ustrDirectoryURL, &ustrSystemPath);

    if( eRet != osl_File_E_None )
        return eRet;

    osl_systemPathRemoveSeparator(ustrSystemPath);

    /* convert unicode path to text */
    if ( UnicodeToText( path, PATH_MAX, ustrSystemPath->buffer, ustrSystemPath->length )
#ifdef MACOSX
     && macxp_resolveAlias( path, PATH_MAX ) == 0
#endif /* MACOSX */
     )
    {
#ifdef ANDROID
        if( strncmp( path, "/assets/", sizeof( "/assets/" ) - 1) == 0 )
        {
            lo_apk_dir *pdir = lo_apk_opendir( path );

            if( pdir )
            {
                oslDirectoryImpl* pDirImpl = (oslDirectoryImpl*) malloc( sizeof(oslDirectoryImpl) );

                if( pDirImpl )
                    {
                        pDirImpl->eKind = oslDirectoryImpl::KIND_ASSETS;
                        pDirImpl->pApkDirStruct = pdir;
                        pDirImpl->ustrPath = ustrSystemPath;

                        *pDirectory = (oslDirectory) pDirImpl;
                        return osl_File_E_None;
                    }
                else
                    {
                        errno = ENOMEM;
                        lo_apk_closedir( pdir );
                    }
            }
        }
        else
#endif
        {
            /* open directory */
            DIR *pdir = opendir( path );

            if( pdir )
            {
                SAL_INFO("sal.file", "opendir(" << path << ") => " << pdir);

                /* create and initialize impl structure */
                oslDirectoryImpl* pDirImpl = static_cast<oslDirectoryImpl*>(malloc( sizeof(oslDirectoryImpl) ));

                if( pDirImpl )
                {
                    pDirImpl->pDirStruct = pdir;
                    pDirImpl->ustrPath = ustrSystemPath;
#ifdef ANDROID
                    pDirImpl->eKind = oslDirectoryImpl::KIND_DIRENT;
#endif
                    *pDirectory = static_cast<oslDirectory>(pDirImpl);
                    return osl_File_E_None;
                }
                errno = ENOMEM;
                closedir( pdir );
            }
            else
            {
                int e = errno;
                SAL_INFO("sal.file", "opendir(" << path << "): " << UnixErrnoString(e));
                // Restore errno after possible modification by SAL_INFO above
                errno = e;
            }
        }
    }

    rtl_uString_release( ustrSystemPath );

    return oslTranslateFileError(errno);
}

oslFileError SAL_CALL osl_closeDirectory(oslDirectory pDirectory)
{
    SAL_WARN_IF(!pDirectory, "sal.file", "pDirectory is nullptr");
    oslDirectoryImpl* pDirImpl = static_cast<oslDirectoryImpl*>(pDirectory);
    oslFileError err = osl_File_E_None;

    if (!pDirImpl)
        return osl_File_E_INVAL;

#ifdef ANDROID
    if (pDirImpl->eKind == oslDirectoryImpl::KIND_ASSETS)
    {
        if (lo_apk_closedir(pDirImpl->pApkDirStruct))
            err = osl_File_E_IO;
    }
    else
#endif
    {
        if (closedir( pDirImpl->pDirStruct) != 0)
        {
            int e = errno;
            SAL_INFO("sal.file", "closedir(" << pDirImpl->pDirStruct << "): " << UnixErrnoString(e));
            err = oslTranslateFileError(e);
        }
        else
            SAL_INFO("sal.file", "closedir(" << pDirImpl->pDirStruct << "): OK");
    }

    /* cleanup members */
    rtl_uString_release(pDirImpl->ustrPath);

    free(pDirImpl);

    return err;
}

/**********************************************
 * osl_readdir_impl_
 *
 * readdir wrapper, filters out "." and ".."
 * on request
 *********************************************/

static struct dirent* osl_readdir_impl_(DIR* pdir)
{
    struct dirent* pdirent;

    while ((pdirent = readdir(pdir)) != nullptr)
    {
        if ((strcmp(pdirent->d_name, ".") == 0) || (strcmp(pdirent->d_name, "..") == 0))
            continue;
        break;
    }

    return pdirent;
}

oslFileError SAL_CALL osl_getNextDirectoryItem(oslDirectory pDirectory,
        oslDirectoryItem* pItem, SAL_UNUSED_PARAMETER sal_uInt32 /*uHint*/)
{
    SAL_WARN_IF(!pDirectory, "sal.file", "pDirectory is nullptr");
    SAL_WARN_IF(!pItem, "sal.file", "pItem is nullptr");

    oslDirectoryImpl* pDirImpl = static_cast<oslDirectoryImpl*>(pDirectory);
    rtl_uString* ustrFileName = nullptr;
    rtl_uString* ustrFilePath = nullptr;
    struct dirent* pEntry;

    if ((pDirectory == nullptr) || (pItem == nullptr))
        return osl_File_E_INVAL;

#ifdef ANDROID
    if(pDirImpl->eKind == oslDirectoryImpl::KIND_ASSETS)
    {
        pEntry = lo_apk_readdir(pDirImpl->pApkDirStruct);
    }
    else
#endif
    {
        pEntry = osl_readdir_impl_(pDirImpl->pDirStruct);
    }

    if (!pEntry)
        return osl_File_E_NOENT;

#if defined(MACOSX)

    // convert decomposed filename to precomposed unicode
    char composed_name[BUFSIZ];
    CFMutableStringRef strRef = CFStringCreateMutable(nullptr, 0 );
    CFStringAppendCString(strRef, pEntry->d_name, kCFStringEncodingUTF8);  // UTF8 is default on Mac OSX
    CFStringNormalize(strRef, kCFStringNormalizationFormC);
    CFStringGetCString(strRef, composed_name, BUFSIZ, kCFStringEncodingUTF8);
    CFRelease(strRef);
    rtl_string2UString(&ustrFileName, composed_name, strlen(composed_name),
                       osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);

#else  // not MACOSX
    /* convert file name to unicode */
    rtl_string2UString(&ustrFileName, pEntry->d_name, strlen(pEntry->d_name),
                       osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
    assert(ustrFileName);

#endif

    osl_systemPathMakeAbsolutePath(pDirImpl->ustrPath, ustrFileName, &ustrFilePath);
    rtl_uString_release(ustrFileName);

    DirectoryItem_Impl* pImpl = static_cast< DirectoryItem_Impl* >(*pItem);
    if (pImpl)
    {
        pImpl->release();
        pImpl = nullptr;
    }
#ifdef _DIRENT_HAVE_D_TYPE
    pImpl = new DirectoryItem_Impl(ustrFilePath, pEntry->d_type);
#else
    pImpl = new DirectoryItem_Impl(ustrFilePath);
#endif /* _DIRENT_HAVE_D_TYPE */
    *pItem = pImpl;
    rtl_uString_release(ustrFilePath);

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_getDirectoryItem(rtl_uString* ustrFileURL, oslDirectoryItem* pItem)
{
    rtl_uString* ustrSystemPath = nullptr;
    oslFileError osl_error = osl_File_E_INVAL;

    if ((!ustrFileURL) || (ustrFileURL->length == 0) || (!pItem))
        return osl_File_E_INVAL;

    osl_error = osl_getSystemPathFromFileURL_Ex(ustrFileURL, &ustrSystemPath);
    if (osl_error != osl_File_E_None)
        return osl_error;

    osl_systemPathRemoveSeparator(ustrSystemPath);

    if (access_u(ustrSystemPath, F_OK) == -1)
    {
        osl_error = oslTranslateFileError(errno);
    }
    else
    {
        *pItem = new DirectoryItem_Impl(ustrSystemPath);
    }
    rtl_uString_release(ustrSystemPath);

    return osl_error;
}

oslFileError SAL_CALL osl_acquireDirectoryItem( oslDirectoryItem Item )
{
    DirectoryItem_Impl * pImpl = static_cast< DirectoryItem_Impl* >(Item);
    if (pImpl == nullptr)
        return osl_File_E_INVAL;

    pImpl->acquire();
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_releaseDirectoryItem( oslDirectoryItem Item )
{
    DirectoryItem_Impl * pImpl = static_cast< DirectoryItem_Impl* >(Item);
    if (pImpl == nullptr)
        return osl_File_E_INVAL;

    pImpl->release();
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_createDirectory( rtl_uString* ustrDirectoryURL )
{
    return osl_createDirectoryWithFlags(
        ustrDirectoryURL, osl_File_OpenFlag_Read | osl_File_OpenFlag_Write);
}

oslFileError osl_createDirectoryWithFlags(
    rtl_uString * ustrDirectoryURL, sal_uInt32 flags)
{
    char path[PATH_MAX];
    oslFileError eRet;

    SAL_WARN_IF((!ustrDirectoryURL) || (ustrDirectoryURL->length == 0),
                "sal.file", "Invalid directory URL");

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( errno );
#endif/* MACOSX */

    return osl_psz_createDirectory( path, flags );
}

oslFileError SAL_CALL osl_removeDirectory( rtl_uString* ustrDirectoryURL )
{
    char path[PATH_MAX];
    oslFileError eRet;

    SAL_WARN_IF((!ustrDirectoryURL) || (ustrDirectoryURL->length == 0),
                "sal.file", "Invalid directory URL");

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

#ifdef MACOSX
    if ( macxp_resolveAlias( path, PATH_MAX ) != 0 )
      return oslTranslateFileError( errno );
#endif/* MACOSX */

    return osl_psz_removeDirectory( path );
}

oslFileError osl_psz_createDirectory(char const * pszPath, sal_uInt32 flags)
{
    int nRet=0;
    int mode
        = (((flags & osl_File_OpenFlag_Read) == 0
            ? 0
            : ((flags & osl_File_OpenFlag_Private) == 0
               ? S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
               : S_IRUSR | S_IXUSR))
           | ((flags & osl_File_OpenFlag_Write) == 0
              ? 0
              : ((flags & osl_File_OpenFlag_Private) == 0
                 ? S_IWUSR | S_IWGRP | S_IWOTH
                 : S_IWUSR)));

    nRet = mkdir(pszPath,mode);

    if ( nRet < 0 )
    {
        nRet=errno;
        SAL_INFO("sal.file", "mkdir(" << pszPath << ",0" << std::oct << mode << std::dec << "): " << UnixErrnoString(nRet));
        return oslTranslateFileError(nRet);
    }
    else
        SAL_INFO("sal.file", "mkdir(" << pszPath << ",0" << std::oct << mode << std::dec << "): OK");

    return osl_File_E_None;
}

static oslFileError osl_psz_removeDirectory( const sal_Char* pszPath )
{
    int nRet=0;

    nRet = rmdir(pszPath);

    if ( nRet < 0 )
    {
        nRet=errno;
        SAL_INFO("sal.file", "rmdir(" << pszPath << "): " << UnixErrnoString(nRet));
        return oslTranslateFileError(nRet);
    }
    else
        SAL_INFO("sal.file", "rmdir(" << pszPath << "): OK");

    return osl_File_E_None;
}

static int path_make_parent(sal_Unicode* path)
{
    int i = rtl_ustr_lastIndexOfChar(path, '/');

    if (i > 0)
    {
        *(path + i) = 0;
        return i;
    }
    return 0;
}

static int create_dir_with_callback(
    sal_Unicode* directory_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    if (osl::mkdir(directory_path, S_IRWXU | S_IRWXG | S_IRWXO) == 0)
    {
        if (aDirectoryCreationCallbackFunc)
        {
            OUString url;
            osl::FileBase::getFileURLFromSystemPath(directory_path, url);
            aDirectoryCreationCallbackFunc(pData, url.pData);
        }
        return 0;
    }
    return errno;
}

static oslFileError create_dir_recursively_(
    sal_Unicode* dir_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    OSL_PRECOND((rtl_ustr_getLength(dir_path) > 0) && ((dir_path + (rtl_ustr_getLength(dir_path) - 1)) != (dir_path + rtl_ustr_lastIndexOfChar(dir_path, '/'))),
    "Path must not end with a slash");

    int native_err = create_dir_with_callback(
        dir_path, aDirectoryCreationCallbackFunc, pData);

    if (native_err == 0)
        return osl_File_E_None;

    if (native_err != ENOENT)
        return oslTranslateFileError(native_err);

    // we step back until '/a_dir' at maximum because
    // we should get an error unequal ENOENT when
    // we try to create 'a_dir' at '/' and would so
    // return before
    int pos = path_make_parent(dir_path);

    oslFileError osl_error = create_dir_recursively_(
        dir_path, aDirectoryCreationCallbackFunc, pData);

    if (osl_error != osl_File_E_None && osl_error != osl_File_E_EXIST)
        return osl_error;

    dir_path[pos] = '/';

    return create_dir_recursively_(dir_path, aDirectoryCreationCallbackFunc, pData);
}

oslFileError SAL_CALL osl_createDirectoryPath(
    rtl_uString* aDirectoryUrl,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    if (aDirectoryUrl == nullptr)
        return osl_File_E_INVAL;

    OUString sys_path;
    oslFileError osl_error = osl_getSystemPathFromFileURL_Ex(aDirectoryUrl, &sys_path.pData);

    if (osl_error != osl_File_E_None)
        return osl_error;

    osl::systemPathRemoveSeparator(sys_path);

    // const_cast because sys_path is a local copy which we want to modify inplace instead of
    // copy it into another buffer on the heap again
    return create_dir_recursively_(sys_path.pData->buffer, aDirectoryCreationCallbackFunc, pData);
}

static oslFileError osl_unlinkFile(const sal_Char* pszPath);
static oslFileError osl_psz_copyFile(const sal_Char* pszPath, const sal_Char* pszDestPath, bool preserveMetadata);
static oslFileError osl_psz_moveFile(const sal_Char* pszPath, const sal_Char* pszDestPath);

static oslFileError  oslDoCopy(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, mode_t nMode, size_t nSourceSize, int DestFileExists);
static void attemptChangeMetadata(const sal_Char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID);
static int           oslDoCopyLink(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName);
static int           oslDoCopyFile(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, size_t nSourceSize, mode_t mode);
static oslFileError  oslDoMoveFile(const sal_Char* pszPath, const sal_Char* pszDestPath);

oslFileError SAL_CALL osl_moveFile( rtl_uString* ustrFileURL, rtl_uString* ustrDestURL )
{
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];
    oslFileError eRet;

    SAL_WARN_IF((!ustrFileURL) || (ustrFileURL->length == 0), "sal.file", "Invalid source file URL");
    SAL_WARN_IF((!ustrDestURL) || (ustrDestURL->length == 0), "sal.file", "Invalid destination file URL");

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
      return oslTranslateFileError( errno );
#endif/* MACOSX */

    return oslDoMoveFile( srcPath, destPath );
}

oslFileError SAL_CALL osl_replaceFile(rtl_uString* ustrFileURL, rtl_uString* ustrDestURL)
{
    return osl_moveFile(ustrFileURL, ustrDestURL);
}

oslFileError SAL_CALL osl_copyFile( rtl_uString* ustrFileURL, rtl_uString* ustrDestURL )
{
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];
    oslFileError eRet;

    SAL_WARN_IF((!ustrFileURL) || (ustrFileURL->length == 0), "sal.file", "Invalid source file URL");
    SAL_WARN_IF((!ustrDestURL) || (ustrDestURL->length == 0), "sal.file", "Invalid destination file URL");

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
      return oslTranslateFileError( errno );
#endif/* MACOSX */

    return osl_psz_copyFile( srcPath, destPath, false );
}

oslFileError SAL_CALL osl_removeFile(rtl_uString* ustrFileURL)
{
    char path[PATH_MAX];
    oslFileError eRet;

    SAL_WARN_IF(!ustrFileURL || ustrFileURL->length == 0, "sal.file", "Invalid file URL");

    /* convert file url to system path */
    eRet = FileURLToPath(path, PATH_MAX, ustrFileURL);
    if (eRet != osl_File_E_None)
        return eRet;

#ifdef MACOSX
    if (macxp_resolveAlias(path, PATH_MAX) != 0)
      return oslTranslateFileError(errno);
#endif/* MACOSX */

    return osl_unlinkFile(path);
}

static oslFileError oslDoMoveFile(const sal_Char* pszPath, const sal_Char* pszDestPath)
{
    oslFileError tErr = osl_psz_moveFile(pszPath,pszDestPath);
    if (tErr == osl_File_E_None)
        return tErr;

    if (tErr != osl_File_E_XDEV)
        return tErr;

    tErr = osl_psz_copyFile(pszPath,pszDestPath, true);

    if (tErr != osl_File_E_None)
    {
        osl_unlinkFile(pszDestPath);
        return tErr;
    }

    tErr = osl_unlinkFile(pszPath);

    return tErr;
}

static oslFileError osl_unlinkFile(const sal_Char* pszPath)
{
    int nRet=0;
    struct stat aStat;

    nRet = lstat_c(pszPath,&aStat);
    if (nRet < 0)
    {
        nRet=errno;
        return oslTranslateFileError(nRet);
    }

    if (S_ISDIR(aStat.st_mode))
        return osl_File_E_ISDIR;

    nRet = unlink(pszPath);
    if (nRet < 0)
    {
        nRet=errno;
        SAL_INFO("sal.file", "unlink(" << pszPath << "): " << UnixErrnoString(nRet));
        return oslTranslateFileError(nRet);
    }
    else
        SAL_INFO("sal.file", "unlink(" << pszPath << "): OK");

    return osl_File_E_None;
}

static oslFileError osl_psz_moveFile(const sal_Char* pszPath, const sal_Char* pszDestPath)
{
    int nRet = 0;

    nRet = rename(pszPath,pszDestPath);

    if (nRet < 0)
    {
        nRet=errno;
        SAL_INFO("sal.file", "rename(" << pszPath << "," << pszDestPath << "): " << UnixErrnoString(nRet));
        return oslTranslateFileError(nRet);
    }
    else
        SAL_INFO("sal.file", "rename(" << pszPath << "," << pszDestPath << "): OK");

    return osl_File_E_None;
}

static oslFileError osl_psz_copyFile( const sal_Char* pszPath, const sal_Char* pszDestPath, bool preserveMetadata )
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
    nRet = lstat_c(pszPath,&aFileStat);

    if (nRet < 0)
    {
        nRet=errno;
        return oslTranslateFileError(nRet);
    }

    /* we do only copy files here */
    if (S_ISDIR(aFileStat.st_mode))
        return osl_File_E_ISDIR;

    nSourceSize = static_cast< size_t >(aFileStat.st_size);
    nMode = aFileStat.st_mode;
    nAcTime = aFileStat.st_atime;
    nModTime = aFileStat.st_mtime;
    nUID = aFileStat.st_uid;
    nGID = aFileStat.st_gid;

    nRet = stat_c(pszDestPath,&aFileStat);
    if (nRet < 0)
    {
        nRet=errno;

#ifdef IOS
        // Checking for nonexistent files at least in the iCloud cache directory (like
        // "/private/var/mobile/Library/Mobile Documents/com~apple~CloudDocs/helloodt0.odt" fails
        // with EPERM, not ENOENT.
        if (nRet == EPERM)
            DestFileExists=0;
#endif

        if (nRet == ENOENT)
            DestFileExists=0;
    }

    /* mfe: the destination file must not be a directory! */
    if (nRet == 0 && S_ISDIR(aFileStat.st_mode))
        return osl_File_E_ISDIR;

    /* mfe: file does not exists or is no dir */

    tErr = oslDoCopy(pszPath, pszDestPath, nMode, nSourceSize, DestFileExists);

    if (tErr != osl_File_E_None)
        return tErr;

    if (preserveMetadata)
        attemptChangeMetadata(pszDestPath, nMode, nAcTime, nModTime, nUID, nGID);

    return tErr;
}

static oslFileError oslDoCopy(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, mode_t nMode, size_t nSourceSize, int DestFileExists)
{
    int      nRet=0;

    OString tmpDestFile;
    if ( DestFileExists )
    {
        //TODO: better pick a temp file name instead of adding .osl-tmp:
        // use the destination file to avoid EXDEV /* Cross-device link */
        tmpDestFile = OString(pszDestFileName) + ".osl-tmp";
        if (rename(pszDestFileName, tmpDestFile.getStr()) != 0)
        {
            int e = errno;
            SAL_INFO("sal.file", "rename(" << pszDestFileName << ", " << tmpDestFile
                     << "): " << UnixErrnoString(e));
            if (e == ENOENT)
            {
                DestFileExists = 0;
            }
            else
            {
                return osl_File_E_EXIST; // for want of a better error code
            }
        }
        else
        {
            SAL_INFO("sal.file", "rename(" << pszDestFileName << ", " << tmpDestFile
                     << "): OK");
        }
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
        if (unlink(pszDestFileName) != 0)
        {
            int e = errno;
            SAL_INFO("sal.file", "unlink(" << pszDestFileName << "): " << UnixErrnoString(e));
        }
        else
            SAL_INFO("sal.file", "unlink(" << pszDestFileName << "): OK");

        if (rename(tmpDestFile.getStr(), pszDestFileName) != 0)
        {
            int e = errno;
            SAL_INFO("sal.file", "rename(" << tmpDestFile << ", " << pszDestFileName
                     << "): " << UnixErrnoString(e));
        }
        else
            SAL_INFO("sal.file", "rename(" << tmpDestFile << ", " << pszDestFileName << "): OK");
    }

    if ( nRet > 0 )
    {
        return oslTranslateFileError(nRet);
    }

    if ( DestFileExists == 1 )
    {
        unlink(tmpDestFile.getStr());
    }

    return osl_File_E_None;
}

void attemptChangeMetadata( const sal_Char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID)
{
    struct utimbuf aTimeBuffer;

#if !defined AT_FDCWD
    if (!S_ISLNK(nMode) && chmod(pszFileName, nMode) < 0)
#else
    if ( fchmodat(AT_FDCWD, pszFileName, nMode, AT_SYMLINK_NOFOLLOW) < 0 )
#endif
    {
        int e = errno;
        SAL_INFO("sal.file", "chmod(" << pszFileName << ",0" << std::oct << nMode << std::dec <<"): " << UnixErrnoString(e));
    }
    else
        SAL_INFO("sal.file", "chmod(" << pszFileName << ",0" << std::oct << nMode << std::dec <<"): OK");

    // No way to change utime of a symlink itself:
    if (!S_ISLNK(nMode))
    {
        aTimeBuffer.actime=nAcTime;
        aTimeBuffer.modtime=nModTime;
        if ( utime(pszFileName,&aTimeBuffer) < 0 )
        {
            int e = errno;
            SAL_INFO("sal.file", "utime(" << pszFileName << "): errno " << e);
        }
    }

    if ( nUID != getuid() )
    {
        nUID=getuid();
    }
    if ( lchown(pszFileName,nUID,nGID) < 0 )
    {
        int e = errno;
        SAL_INFO("sal.file", "lchown(" << pszFileName << "): errno " << e);
    }
    else
        SAL_INFO("sal.file", "lchown(" << pszFileName << "): OK");
}

static int oslDoCopyLink(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName)
{
    int nRet=0;

    /* mfe: if dest file is symbolic link remove the link and place the file instead (hro says so) */
    /* mfe: if source is a link copy the link and not the file it points to (hro says so) */
    sal_Char pszLinkContent[PATH_MAX+1];

    pszLinkContent[0] = '\0';

    nRet = readlink(pszSourceFileName,pszLinkContent,PATH_MAX);

    if ( nRet < 0 )
    {
        nRet=errno;
        return nRet;
    }

    pszLinkContent[ nRet ] = 0;

    nRet = symlink(pszLinkContent,pszDestFileName);

    if ( nRet < 0 )
    {
        nRet=errno;
        return nRet;
    }

    return 0;
}

static int oslDoCopyFile(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, size_t nSourceSize, mode_t mode)
{
    oslFileHandle SourceFileFH=nullptr;
    int DestFileFD=0;
    int nRet=0;

    if (openFilePath(pszSourceFileName,
                         &SourceFileFH,
                         osl_File_OpenFlag_Read|osl_File_OpenFlag_NoLock|osl_File_OpenFlag_NoExcl, mode_t(-1)) != osl_File_E_None)
    {
        // Let's hope errno is still set relevantly after openFilePath...
        nRet=errno;
        return nRet;
    }

    DestFileFD=open(pszDestFileName, O_WRONLY | O_CREAT, mode);

    if ( DestFileFD < 0 )
    {
        nRet=errno;
        SAL_INFO("sal.file", "open(" << pszDestFileName << ",O_WRONLY|O_CREAT,0" << std::oct << mode << std::dec << "): " << UnixErrnoString(nRet));
        osl_closeFile(SourceFileFH);
        return nRet;
    }
    else
        SAL_INFO("sal.file", "open(" << pszDestFileName << ",O_WRONLY|O_CREAT,0" << std::oct << mode << std::dec << "): OK");

    size_t nRemains = nSourceSize;

    if ( nRemains )
    {
        /* mmap has problems, try the direct streaming */
        char pBuffer[0x7FFF];

        do
        {
            size_t nToRead = std::min( sizeof(pBuffer), nRemains );
            sal_uInt64 nRead;
            bool succeeded;
            if ( osl_readFile( SourceFileFH, pBuffer, nToRead, &nRead ) != osl_File_E_None || nRead > nToRead || nRead == 0 )
                break;

            succeeded = safeWrite( DestFileFD, pBuffer, nRead );
            if ( !succeeded )
                break;

            // We know nRead <= nToRead, so it must fit in a size_t
            nRemains -= static_cast<size_t>(nRead);
        }
        while( nRemains );
    }

    if ( nRemains )
    {
        if ( errno )
            nRet = errno;
        else
            nRet = ENOSPC;
    }

    osl_closeFile( SourceFileFH );
    if ( close( DestFileFD ) == -1 )
    {
        int e = errno;
        SAL_INFO("sal.file", "close(" << DestFileFD << "): " << UnixErrnoString(e));
        if ( nRet == 0 )
            nRet = e;
    }
    else
        SAL_INFO("sal.file", "close(" << DestFileFD << "): OK");

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
