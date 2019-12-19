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
#include <cstring>
#include <memory>
#include <new>

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

namespace {

struct DirectoryImpl
{
    OString strPath;           /* holds native directory path */
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
};

}

DirectoryItem_Impl::DirectoryItem_Impl(
    rtl_String * strFilePath, unsigned char DType)
    : m_RefCount     (1),
      m_strFilePath (strFilePath),
      m_DType        (DType)
{
    if (m_strFilePath != nullptr)
        rtl_string_acquire(m_strFilePath);
}
DirectoryItem_Impl::~DirectoryItem_Impl()
{
    if (m_strFilePath != nullptr)
        rtl_string_release(m_strFilePath);
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
static oslFileError osl_psz_removeDirectory(const char* pszPath);

oslFileError SAL_CALL osl_openDirectory(rtl_uString* ustrDirectoryURL, oslDirectory* pDirectory)
{
    oslFileError eRet;

    OString path;

    if ((ustrDirectoryURL == nullptr) || (ustrDirectoryURL->length == 0) || (pDirectory == nullptr))
        return osl_File_E_INVAL;

    /* convert file URL to system path */
    eRet = osl::detail::convertUrlToPathname(OUString::unacquired(&ustrDirectoryURL), &path);

    if( eRet != osl_File_E_None )
        return eRet;

    osl_systemPathRemoveSeparator(path.pData);

#ifdef MACOSX
    {
        auto const n = std::max(int(path.getLength() + 1), int(PATH_MAX));
        auto const tmp = std::make_unique<char[]>(n);
        std::strcpy(tmp.get(), path.getStr());
        if (macxp_resolveAlias(tmp.get(), n) != 0) {
            return oslTranslateFileError(errno);
        }
        path = OString(tmp.get(), std::strlen(tmp.get()));
    }
#endif /* MACOSX */

#ifdef ANDROID
    if( strncmp( path.getStr(), "/assets/", sizeof( "/assets/" ) - 1) == 0 )
    {
        lo_apk_dir *pdir = lo_apk_opendir( path.getStr() );

        if( pdir )
        {
            DirectoryImpl* pDirImpl = new(std::nothrow) DirectoryImpl;

            if( pDirImpl )
            {
                pDirImpl->eKind = DirectoryImpl::KIND_ASSETS;
                pDirImpl->pApkDirStruct = pdir;
                pDirImpl->strPath = path;

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
        DIR *pdir = opendir( path.getStr() );

        if( pdir )
        {
            SAL_INFO("sal.file", "opendir(" << path << ") => " << pdir);

            /* create and initialize impl structure */
            DirectoryImpl* pDirImpl = new(std::nothrow) DirectoryImpl;

            if( pDirImpl )
            {
                pDirImpl->pDirStruct = pdir;
                pDirImpl->strPath = path;
#ifdef ANDROID
                pDirImpl->eKind = DirectoryImpl::KIND_DIRENT;
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

    return oslTranslateFileError(errno);
}

oslFileError SAL_CALL osl_closeDirectory(oslDirectory pDirectory)
{
    SAL_WARN_IF(!pDirectory, "sal.file", "pDirectory is nullptr");
    DirectoryImpl* pDirImpl = static_cast<DirectoryImpl*>(pDirectory);
    oslFileError err = osl_File_E_None;

    if (!pDirImpl)
        return osl_File_E_INVAL;

#ifdef ANDROID
    if (pDirImpl->eKind == DirectoryImpl::KIND_ASSETS)
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

    delete pDirImpl;

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

    DirectoryImpl* pDirImpl = static_cast<DirectoryImpl*>(pDirectory);
    OString strFileName;
    struct dirent* pEntry;

    if ((pDirectory == nullptr) || (pItem == nullptr))
        return osl_File_E_INVAL;

#ifdef ANDROID
    if(pDirImpl->eKind == DirectoryImpl::KIND_ASSETS)
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

    char const * filename = pEntry->d_name;

#if defined(MACOSX)
    // convert decomposed filename to precomposed UTF-8
    char composed_name[BUFSIZ];
    CFMutableStringRef strRef = CFStringCreateMutable(nullptr, 0 );
    CFStringAppendCString(strRef, filename, kCFStringEncodingUTF8);  // UTF8 is default on Mac OSX
    CFStringNormalize(strRef, kCFStringNormalizationFormC);
    CFStringGetCString(strRef, composed_name, BUFSIZ, kCFStringEncodingUTF8);
    CFRelease(strRef);
    filename = composed_name;
#endif

    strFileName = OString(filename, strlen(filename));

    auto const strFilePath = osl::systemPathMakeAbsolutePath(pDirImpl->strPath, strFileName);

    DirectoryItem_Impl* pImpl = static_cast< DirectoryItem_Impl* >(*pItem);
    if (pImpl)
    {
        pImpl->release();
        pImpl = nullptr;
    }
#ifdef _DIRENT_HAVE_D_TYPE
    pImpl = new DirectoryItem_Impl(strFilePath.pData, pEntry->d_type);
#else
    pImpl = new DirectoryItem_Impl(strFilePath.pData);
#endif /* _DIRENT_HAVE_D_TYPE */
    *pItem = pImpl;

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_getDirectoryItem(rtl_uString* ustrFileURL, oslDirectoryItem* pItem)
{
    OString strSystemPath;
    oslFileError osl_error = osl_File_E_INVAL;

    if ((!ustrFileURL) || (ustrFileURL->length == 0) || (!pItem))
        return osl_File_E_INVAL;

    osl_error = osl::detail::convertUrlToPathname(OUString::unacquired(&ustrFileURL), &strSystemPath);
    if (osl_error != osl_File_E_None)
        return osl_error;

    osl_systemPathRemoveSeparator(strSystemPath.pData);

    if (osl::access(strSystemPath, F_OK) == -1)
    {
        osl_error = oslTranslateFileError(errno);
    }
    else
    {
        *pItem = new DirectoryItem_Impl(strSystemPath.pData);
    }

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

static oslFileError osl_psz_removeDirectory( const char* pszPath )
{
    int nRet = rmdir(pszPath);

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

static int path_make_parent(char* path)
{
    int i = rtl_str_lastIndexOfChar(path, '/');

    if (i > 0)
    {
        *(path + i) = 0;
        return i;
    }
    return 0;
}

static int create_dir_with_callback(
    char* directory_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    if (osl::mkdir(directory_path, S_IRWXU | S_IRWXG | S_IRWXO) == 0)
    {
        if (aDirectoryCreationCallbackFunc)
        {
            OUString url;
            osl::detail::convertPathnameToUrl(directory_path, &url);
            aDirectoryCreationCallbackFunc(pData, url.pData);
        }
        return 0;
    }
    return errno;
}

static oslFileError create_dir_recursively_(
    char* dir_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    OSL_PRECOND((rtl_str_getLength(dir_path) > 0) && ((dir_path + (rtl_str_getLength(dir_path) - 1)) != (dir_path + rtl_str_lastIndexOfChar(dir_path, '/'))),
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

    OString sys_path;
    oslFileError osl_error = osl::detail::convertUrlToPathname(
        OUString::unacquired(&aDirectoryUrl), &sys_path);

    if (osl_error != osl_File_E_None)
        return osl_error;

    osl::systemPathRemoveSeparator(sys_path);

    // const_cast because sys_path is a local copy which we want to modify inplace instead of
    // copy it into another buffer on the heap again
    return create_dir_recursively_(sys_path.pData->buffer, aDirectoryCreationCallbackFunc, pData);
}

static oslFileError osl_unlinkFile(const char* pszPath);
static oslFileError osl_psz_copyFile(const char* pszPath, const char* pszDestPath, bool preserveMetadata);
static oslFileError osl_psz_moveFile(const char* pszPath, const char* pszDestPath);

static oslFileError  oslDoCopy(const char* pszSourceFileName, const char* pszDestFileName, mode_t nMode, size_t nSourceSize, bool DestFileExists);
static void attemptChangeMetadata(const char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID);
static int           oslDoCopyLink(const char* pszSourceFileName, const char* pszDestFileName);
static int           oslDoCopyFile(const char* pszSourceFileName, const char* pszDestFileName, size_t nSourceSize, mode_t mode);
static oslFileError  oslDoMoveFile(const char* pszPath, const char* pszDestPath);

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
    int nGid = -1;
    char destPath[PATH_MAX];
    oslFileError eRet = FileURLToPath(destPath, PATH_MAX, ustrDestURL);
    if (eRet == osl_File_E_None)
    {
        struct stat aFileStat;
        // coverity[fs_check_call] - unavoidable TOCTOU
        int nRet = stat(destPath, &aFileStat);
        if (nRet == -1)
        {
            nRet = errno;
            SAL_INFO("sal.file", "stat(" << destPath << "): " << UnixErrnoString(nRet));
        }
        else
        {
            nGid = aFileStat.st_gid;
        }
    }

    eRet = osl_moveFile(ustrFileURL, ustrDestURL);

    if (eRet == osl_File_E_None && nGid != -1)
    {
        int nRet = chown(destPath, -1, nGid);
        if (nRet == -1)
        {
            nRet = errno;
            SAL_INFO("sal.file",
                     "chown(" << destPath << "-1, " << nGid << "): " << UnixErrnoString(nRet));
        }
    }

    return eRet;
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

static oslFileError oslDoMoveFile(const char* pszPath, const char* pszDestPath)
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

static oslFileError osl_unlinkFile(const char* pszPath)
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

static oslFileError osl_psz_moveFile(const char* pszPath, const char* pszDestPath)
{
    int nRet = rename(pszPath,pszDestPath);

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

static oslFileError osl_psz_copyFile( const char* pszPath, const char* pszDestPath, bool preserveMetadata )
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
    bool DestFileExists=true;

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
            DestFileExists=false;
#endif

        if (nRet == ENOENT)
            DestFileExists=false;
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

static oslFileError oslDoCopy(const char* pszSourceFileName, const char* pszDestFileName, mode_t nMode, size_t nSourceSize, bool DestFileExists)
{
    int      nRet=0;

    OString tmpDestFile;
    if ( DestFileExists )
    {
        //TODO: better pick a temp file name instead of adding .osl-tmp:
        // use the destination file to avoid EXDEV /* Cross-device link */
        tmpDestFile = pszDestFileName + OStringLiteral(".osl-tmp");
        if (rename(pszDestFileName, tmpDestFile.getStr()) != 0)
        {
            int e = errno;
            SAL_INFO("sal.file", "rename(" << pszDestFileName << ", " << tmpDestFile
                     << "): " << UnixErrnoString(e));
            if (e == ENOENT)
            {
                DestFileExists = false;
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

    if ( nRet > 0 && DestFileExists )
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

    if ( DestFileExists )
    {
        unlink(tmpDestFile.getStr());
    }

    return osl_File_E_None;
}

void attemptChangeMetadata( const char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID)
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

static int oslDoCopyLink(const char* pszSourceFileName, const char* pszDestFileName)
{
    int nRet=0;

    /* mfe: if dest file is symbolic link remove the link and place the file instead (hro says so) */
    /* mfe: if source is a link copy the link and not the file it points to (hro says so) */
    char pszLinkContent[PATH_MAX+1];

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

static int oslDoCopyFile(const char* pszSourceFileName, const char* pszDestFileName, size_t nSourceSize, mode_t mode)
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
