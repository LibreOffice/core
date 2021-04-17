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

#include <systools/win32/uwinapi.h>

#include "file_url.hxx"
#include "filetime.hxx"
#include "file_error.hxx"

#include "path_helper.hxx"

#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <rtl/character.hxx>
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

const wchar_t UNC_PREFIX[] = L"\\\\";
const wchar_t BACKSLASH = '\\';
const wchar_t SLASH = '/';

BOOL TimeValueToFileTime(const TimeValue *cpTimeVal, FILETIME *pFTime)
{
    SYSTEMTIME  BaseSysTime;
    FILETIME    BaseFileTime;
    FILETIME    FTime;
    bool        fSuccess = false;

    BaseSysTime.wYear         = 1970;
    BaseSysTime.wMonth        = 1;
    BaseSysTime.wDayOfWeek    = 0;
    BaseSysTime.wDay          = 1;
    BaseSysTime.wHour         = 0;
    BaseSysTime.wMinute       = 0;
    BaseSysTime.wSecond       = 0;
    BaseSysTime.wMilliseconds = 0;

    if (cpTimeVal==nullptr)
        return fSuccess;

    if ( SystemTimeToFileTime(&BaseSysTime, &BaseFileTime) )
    {
        __int64 timeValue;

        __int64 localTime = cpTimeVal->Seconds*__int64(10000000)+cpTimeVal->Nanosec/100;
        osl::detail::setFiletime(FTime, localTime);
        fSuccess = 0 <= (timeValue= osl::detail::getFiletime(BaseFileTime) + osl::detail::getFiletime(FTime));
        if (fSuccess)
            osl::detail::setFiletime(*pFTime, timeValue);
    }
    return fSuccess;
}

BOOL FileTimeToTimeValue(const FILETIME *cpFTime, TimeValue *pTimeVal)
{
    SYSTEMTIME  BaseSysTime;
    FILETIME    BaseFileTime;
    bool        fSuccess = false;   /* Assume failure */

    BaseSysTime.wYear         = 1970;
    BaseSysTime.wMonth        = 1;
    BaseSysTime.wDayOfWeek    = 0;
    BaseSysTime.wDay          = 1;
    BaseSysTime.wHour         = 0;
    BaseSysTime.wMinute       = 0;
    BaseSysTime.wSecond       = 0;
    BaseSysTime.wMilliseconds = 0;

    if ( SystemTimeToFileTime(&BaseSysTime, &BaseFileTime) )
    {
        __int64     Value;

        fSuccess = 0 <= (Value = osl::detail::getFiletime(*cpFTime) - osl::detail::getFiletime(BaseFileTime));

        if ( fSuccess )
        {
            pTimeVal->Seconds  = static_cast<unsigned long>(Value / 10000000L);
            pTimeVal->Nanosec  = static_cast<unsigned long>((Value % 10000000L) * 100);
        }
    }
    return fSuccess;
}

namespace
{

    struct Component
    {
        Component() :
            begin_(nullptr), end_(nullptr)
        {}

        bool isPresent() const
        { return (static_cast<sal_IntPtr>(end_ - begin_) > 0); }

        const sal_Unicode* begin_;
        const sal_Unicode* end_;
    };

    struct UNCComponents
    {
        Component server_;
        Component share_;
        Component resource_;
    };

    bool is_UNC_path(const sal_Unicode* path)
    { return (0 == wcsncmp(UNC_PREFIX, o3tl::toW(path), SAL_N_ELEMENTS(UNC_PREFIX) - 1)); }

    void parse_UNC_path(const sal_Unicode* path, UNCComponents* puncc)
    {
        OSL_PRECOND(is_UNC_path(path), "Precondition violated: No UNC path");
        OSL_PRECOND(rtl_ustr_indexOfChar(path, SLASH) == -1, "Path must not contain slashes");

        const sal_Unicode* pend = path + rtl_ustr_getLength(path);
        const sal_Unicode* ppos = path + 2;

        puncc->server_.begin_ = ppos;
        while ((ppos < pend) && (*ppos != BACKSLASH))
            ppos++;

        puncc->server_.end_ = ppos;

        if (BACKSLASH == *ppos)
        {
            puncc->share_.begin_ = ++ppos;
            while ((ppos < pend) && (*ppos != BACKSLASH))
                ppos++;

            puncc->share_.end_ = ppos;

            if (BACKSLASH == *ppos)
            {
                puncc->resource_.begin_ = ++ppos;
                while (ppos < pend)
                    ppos++;

                puncc->resource_.end_ = ppos;
            }
        }

        SAL_WARN_IF(!puncc->server_.isPresent() || !puncc->share_.isPresent(),
            "sal.osl",
            "Postcondition violated: Invalid UNC path detected");
    }

    bool has_path_parent(const sal_Unicode* path)
    {
        // Has the given path a parent or are we already there,
        // e.g. 'c:\' or '\\server\share\'?

        bool has_parent = false;
        if (is_UNC_path(path))
        {
            UNCComponents unc_comp;
            parse_UNC_path(path, &unc_comp);
            has_parent = unc_comp.resource_.isPresent();
        }
        else
        {
            has_parent = !osl::systemPathIsLogicalDrivePattern(OUString(path));
        }
        return has_parent;
    }

    bool has_path_parent(const OUString& path)
    { return has_path_parent(path.getStr()); }

}

oslFileError SAL_CALL osl_acquireVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
    {
        rtl_uString_acquire( static_cast<rtl_uString *>(Handle) );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

oslFileError SAL_CALL osl_releaseVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
    {
        rtl_uString_release( static_cast<rtl_uString *>(Handle) );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

oslFileError SAL_CALL osl_getVolumeDeviceMountPath( oslVolumeDeviceHandle Handle, rtl_uString **pstrPath )
{
    if ( Handle && pstrPath )
    {
        rtl_uString_assign( pstrPath, static_cast<rtl_uString *>(Handle) );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

#define DIRECTORYITEM_DRIVE     0
#define DIRECTORYITEM_FILE      1
#define DIRECTORYITEM_SERVER    2

namespace {

struct DirectoryItem_Impl
{
    UINT uType = 0;
    union {
        WIN32_FIND_DATAW FindData;
        WCHAR            cDriveString[MAX_PATH];
    };
    OUString        m_sFullPath;
    bool            bFullPathNormalized = false;
    int             nRefCount = 0;
};

}

#define DIRECTORYTYPE_LOCALROOT     0
#define DIRECTORYTYPE_NETROOT       1
#define DIRECTORYTYPE_FILESYSTEM    3

namespace {

struct Directory_Impl
{
    UINT uType = 0;
    union {
        HANDLE  hDirectory = 0;
        HANDLE  hEnumDrives;
    };
    OUString    m_sDirectoryPath;
};

typedef struct tagDRIVEENUM
{
    LPCWSTR lpIdent;
    WCHAR   cBuffer[/*('Z' - 'A' + 1) * sizeof("A:\\") + 1*/256];
    LPCWSTR lpCurrent;
} DRIVEENUM, *PDRIVEENUM, *LPDRIVEENUM;

}

static HANDLE WINAPI OpenLogicalDrivesEnum()
{
    LPDRIVEENUM pEnum = static_cast<LPDRIVEENUM>(HeapAlloc( GetProcessHeap(), 0, sizeof(DRIVEENUM) ));
    if ( pEnum )
    {
        DWORD dwNumCopied = GetLogicalDriveStringsW( SAL_N_ELEMENTS(pEnum->cBuffer) - 1, pEnum->cBuffer );

        if ( dwNumCopied && dwNumCopied < SAL_N_ELEMENTS(pEnum->cBuffer) )
        {
            pEnum->lpCurrent = pEnum->cBuffer;
            pEnum->lpIdent = L"tagDRIVEENUM";
        }
        else
        {
            HeapFree( GetProcessHeap(), 0, pEnum );
            pEnum = nullptr;
        }
    }
    return pEnum ? static_cast<HANDLE>(pEnum) : INVALID_HANDLE_VALUE;
}

static bool WINAPI EnumLogicalDrives(HANDLE hEnum, LPWSTR lpBuffer)
{
    LPDRIVEENUM pEnum = static_cast<LPDRIVEENUM>(hEnum);
    if ( !pEnum )
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return false;
    }

    int nLen = wcslen( pEnum->lpCurrent );
    if ( !nLen )
    {
        SetLastError( ERROR_NO_MORE_FILES );
        return false;
    }

    CopyMemory( lpBuffer, pEnum->lpCurrent, (nLen + 1) * sizeof(WCHAR) );
    pEnum->lpCurrent += nLen + 1;
    return true;
}

static bool WINAPI CloseLogicalDrivesEnum(HANDLE hEnum)
{
    bool        fSuccess = false;
    LPDRIVEENUM pEnum = static_cast<LPDRIVEENUM>(hEnum);

    if ( pEnum )
    {
        HeapFree( GetProcessHeap(), 0, pEnum );
        fSuccess = true;
    }
    else
        SetLastError( ERROR_INVALID_HANDLE );

    return fSuccess;
}

namespace {

typedef struct tagDIRECTORY
{
    HANDLE           hFind;
    WIN32_FIND_DATAW aFirstData;
} DIRECTORY, *PDIRECTORY, *LPDIRECTORY;

}

static HANDLE WINAPI OpenDirectory( rtl_uString* pPath)
{
    if ( !pPath )
        return nullptr;

    sal_uInt32 nLen = rtl_uString_getLength( pPath );
    if ( !nLen )
        return nullptr;

    const WCHAR* pSuffix = nullptr;
    sal_uInt32 nSuffLen = 0;
    if ( pPath->buffer[nLen - 1] != L'\\' )
    {
        pSuffix = L"\\*.*";
        nSuffLen = 4;
    }
    else
    {
        pSuffix = L"*.*";
        nSuffLen = 3;
    }

    WCHAR* szFileMask = static_cast< WCHAR* >( malloc( sizeof( WCHAR ) * ( nLen + nSuffLen + 1 ) ) );
    assert(szFileMask); // Don't handle OOM conditions
    wcscpy( szFileMask, o3tl::toW(rtl_uString_getStr( pPath )) );
    wcscat( szFileMask, pSuffix );

    LPDIRECTORY pDirectory = static_cast<LPDIRECTORY>(HeapAlloc(GetProcessHeap(), 0, sizeof(DIRECTORY)));
    assert(pDirectory); // Don't handle OOM conditions
    pDirectory->hFind = FindFirstFileW(szFileMask, &pDirectory->aFirstData);

    if (!IsValidHandle(pDirectory->hFind))
    {
        if ( GetLastError() != ERROR_NO_MORE_FILES )
        {
            HeapFree(GetProcessHeap(), 0, pDirectory);
            pDirectory = nullptr;
        }
    }
    free(szFileMask);

    return static_cast<HANDLE>(pDirectory);
}

static bool WINAPI EnumDirectory(HANDLE hDirectory, LPWIN32_FIND_DATAW pFindData)
{
    LPDIRECTORY pDirectory = static_cast<LPDIRECTORY>(hDirectory);
    if ( !pDirectory )
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return false;
    }

    bool fSuccess = false;
    bool fValid;
    do
    {
        if ( pDirectory->aFirstData.cFileName[0] )
        {
            *pFindData = pDirectory->aFirstData;
            fSuccess = true;
            pDirectory->aFirstData.cFileName[0] = 0;
        }
        else if ( IsValidHandle( pDirectory->hFind ) )
            fSuccess = FindNextFileW( pDirectory->hFind, pFindData );
        else
        {
            fSuccess = false;
            SetLastError( ERROR_NO_MORE_FILES );
        }

        fValid = fSuccess && wcscmp( L".", pFindData->cFileName ) != 0 && wcscmp( L"..", pFindData->cFileName ) != 0;

    } while( fSuccess && !fValid );

    return fSuccess;
}

static bool WINAPI CloseDirectory(HANDLE hDirectory)
{
    bool        fSuccess = false;
    LPDIRECTORY pDirectory = static_cast<LPDIRECTORY>(hDirectory);

    if (pDirectory)
    {
        if (IsValidHandle(pDirectory->hFind))
            fSuccess = FindClose(pDirectory->hFind);

        fSuccess = HeapFree(GetProcessHeap(), 0, pDirectory) && fSuccess;
    }
    else
        SetLastError(ERROR_INVALID_HANDLE);

    return fSuccess;
}

static oslFileError osl_openLocalRoot(
    rtl_uString *strDirectoryPath, oslDirectory *pDirectory)
{
    if ( !pDirectory )
        return osl_File_E_INVAL;

    *pDirectory = nullptr;

    rtl_uString     *strSysPath = nullptr;
    oslFileError     error = osl_getSystemPathFromFileURL_( strDirectoryPath, &strSysPath, false );
    if ( osl_File_E_None != error )
        return error;

    Directory_Impl* pDirImpl = new (std::nothrow) Directory_Impl;
    assert(pDirImpl); // Don't handle OOM conditions
    pDirImpl->m_sDirectoryPath = OUString(strSysPath, SAL_NO_ACQUIRE);

    /* Append backslash if necessary */

    /* @@@ToDo
       use function ensure backslash
    */
    sal_uInt32 nLen = pDirImpl->m_sDirectoryPath.getLength();
    if ( nLen && pDirImpl->m_sDirectoryPath[nLen - 1] != L'\\' )
    {
        pDirImpl->m_sDirectoryPath += "\\";
    }

    pDirImpl->uType = DIRECTORYTYPE_LOCALROOT;
    pDirImpl->hEnumDrives = OpenLogicalDrivesEnum();

    /* @@@ToDo
       Use IsValidHandle(...)
    */
    if ( pDirImpl->hEnumDrives != INVALID_HANDLE_VALUE )
    {
        *pDirectory = static_cast<oslDirectory>(pDirImpl);
        error = osl_File_E_None;
    }
    else
    {
        if ( pDirImpl )
        {
            delete pDirImpl;
            pDirImpl = nullptr;
        }

        error = oslTranslateFileError( GetLastError() );
    }
    return error;
}

static oslFileError osl_openFileDirectory(
    rtl_uString *strDirectoryPath, oslDirectory *pDirectory)
{
    oslFileError error = osl_File_E_None;

    if ( !pDirectory )
        return osl_File_E_INVAL;
    *pDirectory = nullptr;

    Directory_Impl *pDirImpl = new (std::nothrow) Directory_Impl;
    assert(pDirImpl); // Don't handle OOM conditions
    pDirImpl->m_sDirectoryPath = strDirectoryPath;

    /* Append backslash if necessary */

    /* @@@ToDo
       use function ensure backslash
    */
    sal_uInt32 nLen = pDirImpl->m_sDirectoryPath.getLength();
    if ( nLen && pDirImpl->m_sDirectoryPath[nLen - 1] != '\\' )
        pDirImpl->m_sDirectoryPath += "\\";

    pDirImpl->uType = DIRECTORYTYPE_FILESYSTEM;
    pDirImpl->hDirectory = OpenDirectory( pDirImpl->m_sDirectoryPath.pData );

    if ( !pDirImpl->hDirectory )
    {
        error = oslTranslateFileError( GetLastError() );

        delete pDirImpl;
        pDirImpl = nullptr;
    }

    *pDirectory = static_cast<oslDirectory>(pDirImpl);
    return error;
}

static oslFileError osl_openNetworkServer(
    rtl_uString *strSysDirPath, oslDirectory *pDirectory)
{
    NETRESOURCEW    aNetResource;
    HANDLE          hEnum;
    DWORD           dwError;

    ZeroMemory( &aNetResource, sizeof(aNetResource) );

    aNetResource.lpRemoteName = o3tl::toW(strSysDirPath->buffer);

    dwError = WNetOpenEnumW(
        RESOURCE_GLOBALNET,
        RESOURCETYPE_DISK,
        RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER,
        &aNetResource,
        &hEnum );

    if ( ERROR_SUCCESS == dwError )
    {
        Directory_Impl *pDirImpl = new (std::nothrow) Directory_Impl;
        assert(pDirImpl); // Don't handle OOM conditions
        pDirImpl->uType = DIRECTORYTYPE_NETROOT;
        pDirImpl->hDirectory = hEnum;
        *pDirectory = static_cast<oslDirectory>(pDirImpl);
    }
    return oslTranslateFileError( dwError );
}

static DWORD create_dir_with_callback(
    rtl_uString * dir_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    // Create the specified directory and call the
    // user specified callback function. On success
    // the function returns ERROR_SUCCESS else a Win32 error code.

    bool bCreated = CreateDirectoryW( o3tl::toW(rtl_uString_getStr( dir_path )), nullptr );

    if ( bCreated )
    {
        if (aDirectoryCreationCallbackFunc)
        {
            OUString url;
            osl_getFileURLFromSystemPath(dir_path, &(url.pData));
            aDirectoryCreationCallbackFunc(pData, url.pData);
        }
        return ERROR_SUCCESS;
    }
    return GetLastError();
}

static int path_make_parent(sal_Unicode* path)
{
    /*  Cut off the last part of the given path to
    get the parent only, e.g. 'c:\dir\subdir' ->
    'c:\dir' or '\\share\sub\dir' -> '\\share\sub'
    @return The position where the path has been cut
    off (this is the position of the last backslash).
    If there are no more parents 0 will be returned,
    e.g. 'c:\' or '\\Share' have no more parents */

    OSL_PRECOND(rtl_ustr_indexOfChar(path, SLASH) == -1, "Path must not contain slashes");
    OSL_PRECOND(has_path_parent(path), "Path must have a parent");

    sal_Unicode* pos_last_backslash = path + rtl_ustr_lastIndexOfChar(path, BACKSLASH);
    *pos_last_backslash = 0;
    return (pos_last_backslash - path);
}

static DWORD create_dir_recursively_(
    rtl_uString * dir_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    OSL_PRECOND(
        rtl_ustr_lastIndexOfChar_WithLength(dir_path->buffer, dir_path->length, BACKSLASH) != dir_path->length,
        "Path must not end with a backslash");

    DWORD w32_error = create_dir_with_callback(
        dir_path, aDirectoryCreationCallbackFunc, pData);
    if (w32_error == ERROR_SUCCESS)
        return ERROR_SUCCESS;

    if ((w32_error != ERROR_PATH_NOT_FOUND) || !has_path_parent(dir_path->buffer))
        return w32_error;

    int pos = path_make_parent(dir_path->buffer); // dir_path->buffer[pos] = 0, restore below

    w32_error = create_dir_recursively_(
        dir_path, aDirectoryCreationCallbackFunc, pData);

    dir_path->buffer[pos] = BACKSLASH; // restore

    if (ERROR_SUCCESS != w32_error && ERROR_ALREADY_EXISTS != w32_error)
        return w32_error;

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
    oslFileError osl_error =
        osl_getSystemPathFromFileURL_(aDirectoryUrl, &sys_path.pData, false);

    if (osl_error != osl_File_E_None)
        return osl_error;

    osl::systemPathRemoveSeparator(sys_path);

    // const_cast because sys_path is a local copy
    // which we want to modify inplace instead of
    // copy it into another buffer on the heap again
    return oslTranslateFileError(create_dir_recursively_(
        sys_path.pData, aDirectoryCreationCallbackFunc, pData));
}

oslFileError SAL_CALL osl_createDirectory(rtl_uString* strPath)
{
    return osl_createDirectoryWithFlags(
        strPath, osl_File_OpenFlag_Read | osl_File_OpenFlag_Write);
}

oslFileError osl_createDirectoryWithFlags(rtl_uString * strPath, sal_uInt32)
{
    rtl_uString *strSysPath = nullptr;
    oslFileError    error = osl_getSystemPathFromFileURL_( strPath, &strSysPath, false );

    if ( osl_File_E_None != error )
        return error;

    bool bCreated = CreateDirectoryW( o3tl::toW(rtl_uString_getStr( strSysPath )), nullptr );
    if ( !bCreated )
    {
        /*@@@ToDo
          The following case is a hack because the ucb or the webtop had some
          problems with the error code that CreateDirectory returns in
          case the path is only a logical drive, should be removed!
        */

        const sal_Unicode   *pBuffer = rtl_uString_getStr( strSysPath );
        sal_Int32           nLen = rtl_uString_getLength( strSysPath );

        if (
            ( ( pBuffer[0] >= 'A' && pBuffer[0] <= 'Z' ) ||
              ( pBuffer[0] >= 'a' && pBuffer[0] <= 'z' ) ) &&
            pBuffer[1] == ':' && ( nLen ==2 || ( nLen == 3 && pBuffer[2] == '\\' ) )
            )
            SetLastError( ERROR_ALREADY_EXISTS );

        error = oslTranslateFileError( GetLastError() );
    }

    rtl_uString_release( strSysPath );
    return error;
}

oslFileError SAL_CALL osl_removeDirectory(rtl_uString* strPath)
{
    rtl_uString *strSysPath = nullptr;
    oslFileError    error = osl_getSystemPathFromFileURL_( strPath, &strSysPath, false );

    if ( osl_File_E_None == error )
    {
        if ( RemoveDirectoryW( o3tl::toW(rtl_uString_getStr( strSysPath ) )) )
            error = osl_File_E_None;
        else
            error = oslTranslateFileError( GetLastError() );

        rtl_uString_release( strSysPath );
    }
    return error;
}

oslFileError SAL_CALL osl_openDirectory(rtl_uString *strDirectoryPath, oslDirectory *pDirectory)
{
    oslFileError    error;

    if ( 0 == rtl_ustr_ascii_compareIgnoreAsciiCase( strDirectoryPath->buffer, "file:///" ) )
        error = osl_openLocalRoot( strDirectoryPath, pDirectory );
    else
    {
        rtl_uString *strSysDirectoryPath = nullptr;
        DWORD       dwPathType;

        error = osl_getSystemPathFromFileURL_( strDirectoryPath, &strSysDirectoryPath, false );

        if ( osl_File_E_None != error )
                return error;

        dwPathType = IsValidFilePath( strSysDirectoryPath, VALIDATEPATH_NORMAL, nullptr );

        if ( dwPathType & PATHTYPE_IS_SERVER )
        {
            error = osl_openNetworkServer( strSysDirectoryPath, pDirectory );
        }
        else
            error = osl_openFileDirectory( strSysDirectoryPath, pDirectory );

        rtl_uString_release( strSysDirectoryPath );
    }
    return error;
}

static oslFileError osl_getNextNetResource(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 /*uHint*/ )
{
    Directory_Impl      *pDirImpl = static_cast<Directory_Impl *>(Directory);
    DirectoryItem_Impl  *pItemImpl = nullptr;
    BYTE                buffer[16384];
    LPNETRESOURCEW      lpNetResource = reinterpret_cast<LPNETRESOURCEW>(buffer);
    DWORD               dwError, dwCount, dwBufSize;

    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = nullptr;

    if ( !pDirImpl )
        return osl_File_E_INVAL;

    dwCount = 1;
    dwBufSize = sizeof(buffer);
    dwError = WNetEnumResourceW( pDirImpl->hDirectory, &dwCount, lpNetResource, &dwBufSize );

    switch ( dwError )
    {
        case NO_ERROR:
        case ERROR_MORE_DATA:
        {
            pItemImpl = new (std::nothrow) DirectoryItem_Impl;
            if ( !pItemImpl )
                return osl_File_E_NOMEM;

            pItemImpl->uType = DIRECTORYITEM_DRIVE;
            osl_acquireDirectoryItem( static_cast<oslDirectoryItem>(pItemImpl) );

            wcscpy( pItemImpl->cDriveString, lpNetResource->lpRemoteName );

            *pItem = pItemImpl;
        }
        return osl_File_E_None;
        case ERROR_NO_MORE_ITEMS:
            return osl_File_E_NOENT;
        default:
            return oslTranslateFileError( dwError );
    }
}

static oslFileError osl_getNextDrive(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 /*uHint*/ )
{
    Directory_Impl      *pDirImpl = static_cast<Directory_Impl *>(Directory);
    DirectoryItem_Impl  *pItemImpl = nullptr;
    bool                fSuccess;

    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = nullptr;

    if ( !pDirImpl )
        return osl_File_E_INVAL;

    pItemImpl = new (std::nothrow) DirectoryItem_Impl;
    if ( !pItemImpl )
        return osl_File_E_NOMEM;

    pItemImpl->uType = DIRECTORYITEM_DRIVE;
    osl_acquireDirectoryItem( static_cast<oslDirectoryItem>(pItemImpl) );
    fSuccess = EnumLogicalDrives( pDirImpl->hEnumDrives, pItemImpl->cDriveString );

    if ( fSuccess )
    {
        *pItem = pItemImpl;
        return osl_File_E_None;
    }
    else
    {
        delete pItemImpl;
        return oslTranslateFileError( GetLastError() );
    }
}

static oslFileError osl_getNextFileItem(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 /*uHint*/)
{
    Directory_Impl      *pDirImpl = static_cast<Directory_Impl *>(Directory);
    DirectoryItem_Impl  *pItemImpl = nullptr;
    bool                fFound;

    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = nullptr;

    if ( !pDirImpl )
        return osl_File_E_INVAL;

    pItemImpl = new (std::nothrow) DirectoryItem_Impl;
    if ( !pItemImpl )
        return osl_File_E_NOMEM;

    fFound = EnumDirectory( pDirImpl->hDirectory, &pItemImpl->FindData );
    if ( !fFound )
    {
        delete pItemImpl;
        return oslTranslateFileError( GetLastError() );
    }

    pItemImpl->uType = DIRECTORYITEM_FILE;
    pItemImpl->nRefCount = 1;

    pItemImpl->m_sFullPath = pDirImpl->m_sDirectoryPath + o3tl::toU(pItemImpl->FindData.cFileName);

    pItemImpl->bFullPathNormalized = true;
    *pItem = static_cast<oslDirectoryItem>(pItemImpl);
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_getNextDirectoryItem(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 uHint)
{
    Directory_Impl      *pDirImpl = static_cast<Directory_Impl *>(Directory);

    /* Assume failure */

    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = nullptr;

    if ( !pDirImpl )
        return osl_File_E_INVAL;

    switch ( pDirImpl->uType )
    {
    case DIRECTORYTYPE_LOCALROOT:
        return osl_getNextDrive( Directory, pItem, uHint );
    case DIRECTORYTYPE_NETROOT:
        return osl_getNextNetResource( Directory, pItem, uHint );
    case DIRECTORYTYPE_FILESYSTEM:
        return osl_getNextFileItem( Directory, pItem, uHint );
    default:
        return osl_File_E_INVAL;
    }
}

oslFileError SAL_CALL osl_closeDirectory(oslDirectory Directory)
{
    Directory_Impl  *pDirImpl = static_cast<Directory_Impl *>(Directory);
    oslFileError    eError = osl_File_E_INVAL;

    if ( pDirImpl )
    {
        switch ( pDirImpl->uType )
        {
        case DIRECTORYTYPE_FILESYSTEM:
            eError = CloseDirectory( pDirImpl->hDirectory ) ? osl_File_E_None : oslTranslateFileError( GetLastError() );
            break;
        case DIRECTORYTYPE_LOCALROOT:
            eError = CloseLogicalDrivesEnum( pDirImpl->hEnumDrives ) ? osl_File_E_None : oslTranslateFileError( GetLastError() );
            break;
        case DIRECTORYTYPE_NETROOT:
            {
                DWORD err = WNetCloseEnum(pDirImpl->hDirectory);
                eError = (err == NO_ERROR) ? osl_File_E_None : oslTranslateFileError(err);
            }
            break;
        default:
            OSL_FAIL( "Invalid directory type" );
            break;
        }

        delete pDirImpl;
    }
    return eError;
}

namespace {

/* Different types of paths */
enum PATHTYPE
{
    PATHTYPE_SYNTAXERROR = 0,
    PATHTYPE_NETROOT,
    PATHTYPE_NETSERVER,
    PATHTYPE_VOLUME,
    PATHTYPE_FILE
};

}

oslFileError SAL_CALL osl_getDirectoryItem(rtl_uString *strFilePath, oslDirectoryItem *pItem)
{
    oslFileError    error = osl_File_E_None;
    rtl_uString*    strSysFilePath = nullptr;
    PATHTYPE        type = PATHTYPE_FILE;
    DWORD           dwPathType;

    /* Assume failure */

    if ( !pItem )
        return osl_File_E_INVAL;

    *pItem = nullptr;

    error = osl_getSystemPathFromFileURL_( strFilePath, &strSysFilePath, false );

    if ( osl_File_E_None != error )
            return error;

    dwPathType = IsValidFilePath( strSysFilePath, VALIDATEPATH_NORMAL, nullptr );

    if ( dwPathType & PATHTYPE_IS_VOLUME )
        type = PATHTYPE_VOLUME;
    else if ( dwPathType & PATHTYPE_IS_SERVER )
        type = PATHTYPE_NETSERVER;
    else
        type = PATHTYPE_FILE;

    switch ( type )
    {
    case PATHTYPE_NETSERVER:
        {
            DirectoryItem_Impl* pItemImpl = new (std::nothrow) DirectoryItem_Impl;

            if ( !pItemImpl )
                error = osl_File_E_NOMEM;

            if ( osl_File_E_None == error )
            {
                pItemImpl->uType = DIRECTORYITEM_SERVER;

                osl_acquireDirectoryItem( static_cast<oslDirectoryItem>(pItemImpl) );
                pItemImpl->m_sFullPath = strSysFilePath;

                // Assign a title anyway
                {
                    int iSrc = 2;
                    int iDst = 0;

                    while( iSrc < strSysFilePath->length && strSysFilePath->buffer[iSrc] && strSysFilePath->buffer[iSrc] != '\\' )
                    {
                        pItemImpl->FindData.cFileName[iDst++] = strSysFilePath->buffer[iSrc++];
                    }
                }

                *pItem = pItemImpl;
            }
        }
        break;
    case PATHTYPE_VOLUME:
        {
            DirectoryItem_Impl* pItemImpl = new (std::nothrow) DirectoryItem_Impl;

            if ( !pItemImpl )
                error = osl_File_E_NOMEM;

            if ( osl_File_E_None == error )
            {
                pItemImpl->uType = DIRECTORYITEM_DRIVE;

                osl_acquireDirectoryItem( static_cast<oslDirectoryItem>(pItemImpl) );

                wcscpy( pItemImpl->cDriveString, o3tl::toW(strSysFilePath->buffer) );
                pItemImpl->cDriveString[0] = rtl::toAsciiUpperCase( pItemImpl->cDriveString[0] );

                if ( pItemImpl->cDriveString[wcslen(pItemImpl->cDriveString) - 1] != '\\' )
                    wcscat( pItemImpl->cDriveString, L"\\" );

                *pItem = pItemImpl;
            }
        }
        break;
    case PATHTYPE_SYNTAXERROR:
    case PATHTYPE_NETROOT:
    case PATHTYPE_FILE:
        {
            HANDLE              hFind;
            WIN32_FIND_DATAW     aFindData;

            if ( strSysFilePath->length > 0 && strSysFilePath->buffer[strSysFilePath->length - 1] == '\\' )
                rtl_uString_newFromStr_WithLength( &strSysFilePath, strSysFilePath->buffer, strSysFilePath->length - 1 );

            hFind = FindFirstFileW( o3tl::toW(rtl_uString_getStr(strSysFilePath)), &aFindData );

            if ( hFind != INVALID_HANDLE_VALUE )
            {
                DirectoryItem_Impl *pItemImpl = new (std::nothrow) DirectoryItem_Impl;
                if (!pItemImpl)
                    error = osl_File_E_NOMEM;

                if (osl_File_E_None == error)
                {
                    osl_acquireDirectoryItem(static_cast<oslDirectoryItem>(pItemImpl));

                    CopyMemory(&pItemImpl->FindData, &aFindData, sizeof(WIN32_FIND_DATAW));
                    pItemImpl->m_sFullPath = strSysFilePath;

                    // MT: This costs 600ms startup time on fast v60x!
                    // GetCaseCorrectPathName( pItemImpl->szFullPath, pItemImpl->szFullPath, sizeof(pItemImpl->szFullPath) );

                    pItemImpl->uType = DIRECTORYITEM_FILE;
                    *pItem = pItemImpl;
                }

                FindClose( hFind );
            }
            else
                error = oslTranslateFileError( GetLastError() );
        }
        break;
    }

    if ( strSysFilePath )
        rtl_uString_release( strSysFilePath );

    return error;
}

oslFileError SAL_CALL osl_acquireDirectoryItem( oslDirectoryItem Item )
{
    DirectoryItem_Impl  *pItemImpl = static_cast<DirectoryItem_Impl *>(Item);

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    pItemImpl->nRefCount++;
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_releaseDirectoryItem( oslDirectoryItem Item )
{
    DirectoryItem_Impl  *pItemImpl = static_cast<DirectoryItem_Impl *>(Item);

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    if ( ! --pItemImpl->nRefCount )
        delete pItemImpl;

    return osl_File_E_None;
}

sal_Bool
SAL_CALL osl_identicalDirectoryItem( oslDirectoryItem a, oslDirectoryItem b)
{
    DirectoryItem_Impl *pA = static_cast<DirectoryItem_Impl *>(a);
    DirectoryItem_Impl *pB = static_cast<DirectoryItem_Impl *>(b);
    if (a == b)
        return true;
    /* same name => same item, unless renaming / moving madness has occurred */
    if (pA->m_sFullPath == pB->m_sFullPath)
        return true;

    // FIXME: as/when/if this is used in anger on Windows we could
    // do better here.

    return false;
}

static bool is_floppy_A_present()
{ return (GetLogicalDrives() & 1); }

static bool is_floppy_B_present()
{ return (GetLogicalDrives() & 2); }

static bool is_floppy_volume_mount_point(const OUString& path)
{
    // determines if a volume mount point shows to a floppy
    // disk by comparing the unique volume names
    static const LPCWSTR FLOPPY_A = L"A:\\";
    static const LPCWSTR FLOPPY_B = L"B:\\";

    OUString p(path);
    osl::systemPathEnsureSeparator(p);

    WCHAR vn[51];
    if (GetVolumeNameForVolumeMountPointW(o3tl::toW(p.getStr()), vn, SAL_N_ELEMENTS(vn)))
    {
        WCHAR vnfloppy[51];
        if (is_floppy_A_present() &&
            GetVolumeNameForVolumeMountPointW(FLOPPY_A, vnfloppy, SAL_N_ELEMENTS(vnfloppy)) &&
            (0 == wcscmp(vn, vnfloppy)))
            return true;

        if (is_floppy_B_present() &&
            GetVolumeNameForVolumeMountPointW(FLOPPY_B, vnfloppy, SAL_N_ELEMENTS(vnfloppy)) &&
            (0 == wcscmp(vn, vnfloppy)))
            return true;
    }
    return false;
}

static bool is_floppy_drive(const OUString& path)
{
    static const LPCWSTR FLOPPY_DRV_LETTERS = L"AaBb";

    // we must take into account that even a floppy
    // drive may be mounted to a directory so checking
    // for the drive letter alone is not sufficient
    // we must compare the unique volume name with
    // that of the available floppy disks

    const sal_Unicode* pszPath = path.getStr();
    return ((wcschr(FLOPPY_DRV_LETTERS, pszPath[0]) && (L':' == pszPath[1])) || is_floppy_volume_mount_point(path));
}

static bool is_volume_mount_point(const OUString& path)
{
    OUString p(path);
    osl::systemPathRemoveSeparator(p);

    if (is_floppy_drive(p))
        return false;

    DWORD fattr = GetFileAttributesW(o3tl::toW(p.getStr()));
    if ((INVALID_FILE_ATTRIBUTES == fattr) ||
        !(FILE_ATTRIBUTE_REPARSE_POINT & fattr))
        return false;

    bool  is_volume_root = false;
    WIN32_FIND_DATAW find_data;
    HANDLE h_find = FindFirstFileW(o3tl::toW(p.getStr()), &find_data);

    if (IsValidHandle(h_find) &&
        (FILE_ATTRIBUTE_REPARSE_POINT & find_data.dwFileAttributes) &&
        (IO_REPARSE_TAG_MOUNT_POINT == find_data.dwReserved0))
    {
        is_volume_root = true;
    }
    if (IsValidHandle(h_find))
        FindClose(h_find);
    return is_volume_root;
}

static UINT get_volume_mount_point_drive_type(const OUString& path)
{
    if (0 == path.getLength())
        return GetDriveTypeW(nullptr);

    OUString p(path);
    osl::systemPathEnsureSeparator(p);

    WCHAR vn[51];
    if (GetVolumeNameForVolumeMountPointW(o3tl::toW(p.getStr()), vn, SAL_N_ELEMENTS(vn)))
        return GetDriveTypeW(vn);

    return DRIVE_NO_ROOT_DIR;
}

static bool is_drivetype_request(sal_uInt32 field_mask)
{
    return (field_mask & osl_VolumeInfo_Mask_Attributes);
}

static oslFileError osl_get_drive_type(
    const OUString& path, oslVolumeInfo* pInfo)
{
    // GetDriveType fails on empty volume mount points
    // see Knowledge Base Q244089
    UINT drive_type;
    if (is_volume_mount_point(path))
        drive_type = get_volume_mount_point_drive_type(path);
    else
        drive_type = GetDriveTypeW(o3tl::toW(path.getStr()));

    if (DRIVE_NO_ROOT_DIR == drive_type)
        return oslTranslateFileError(ERROR_INVALID_DRIVE);

    pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;

    switch (drive_type)
    {
        case DRIVE_CDROM:
            pInfo->uAttributes |= osl_Volume_Attribute_CompactDisc | osl_Volume_Attribute_Removeable;
            break;
        case DRIVE_REMOVABLE:
            pInfo->uAttributes |= osl_Volume_Attribute_Removeable;
            if (is_floppy_drive(path))
                pInfo->uAttributes |= osl_Volume_Attribute_FloppyDisk;
            break;
        case DRIVE_FIXED:
            pInfo->uAttributes |= osl_Volume_Attribute_FixedDisk;
            break;
        case DRIVE_RAMDISK:
            pInfo->uAttributes |= osl_Volume_Attribute_RAMDisk;
            break;
        case DRIVE_REMOTE:
            pInfo->uAttributes |= osl_Volume_Attribute_Remote;
            break;
        case DRIVE_UNKNOWN:
            pInfo->uAttributes = 0;
            break;
        default:
            pInfo->uValidFields &= ~osl_VolumeInfo_Mask_Attributes;
            pInfo->uAttributes = 0;
            break;
    }
    return osl_File_E_None;
}

static bool is_volume_space_info_request(sal_uInt32 field_mask)
{
    return (field_mask &
            (osl_VolumeInfo_Mask_TotalSpace |
             osl_VolumeInfo_Mask_UsedSpace  |
             osl_VolumeInfo_Mask_FreeSpace));
}

static void get_volume_space_information(
    const OUString& path, oslVolumeInfo *pInfo)
{
    bool ret = GetDiskFreeSpaceExW(
        o3tl::toW(path.getStr()),
        reinterpret_cast<PULARGE_INTEGER>(&pInfo->uFreeSpace),
        reinterpret_cast<PULARGE_INTEGER>(&pInfo->uTotalSpace),
        nullptr);

    if (ret)
    {
        pInfo->uUsedSpace    = pInfo->uTotalSpace - pInfo->uFreeSpace;
        pInfo->uValidFields |= osl_VolumeInfo_Mask_TotalSpace |
            osl_VolumeInfo_Mask_UsedSpace |
            osl_VolumeInfo_Mask_FreeSpace;
    }
}

static bool is_filesystem_attributes_request(sal_uInt32 field_mask)
{
    return (field_mask &
            (osl_VolumeInfo_Mask_MaxNameLength |
             osl_VolumeInfo_Mask_MaxPathLength |
             osl_VolumeInfo_Mask_FileSystemName |
             osl_VolumeInfo_Mask_FileSystemCaseHandling));
}

static oslFileError get_filesystem_attributes(
    const OUString& path, sal_uInt32 field_mask, oslVolumeInfo* pInfo)
{
    pInfo->uAttributes = 0;

    // osl_get_drive_type must be called first because
    // this function resets osl_VolumeInfo_Mask_Attributes
    // on failure
    if (is_drivetype_request(field_mask))
    {
        oslFileError osl_error = osl_get_drive_type(path, pInfo);
        if (osl_File_E_None != osl_error)
            return osl_error;
    }
    if (is_filesystem_attributes_request(field_mask))
    {
        /* the following two parameters can not be longer than MAX_PATH+1 */
        WCHAR vn[MAX_PATH+1];
        WCHAR fsn[MAX_PATH+1];

        DWORD serial;
        DWORD mcl;
        DWORD flags;

        LPCWSTR pszPath = o3tl::toW(path.getStr());
        if (GetVolumeInformationW(pszPath, vn, MAX_PATH+1, &serial, &mcl, &flags, fsn, MAX_PATH+1))
        {
            // Currently sal does not use this value, instead MAX_PATH is used
            pInfo->uValidFields   |= osl_VolumeInfo_Mask_MaxNameLength;
            pInfo->uMaxNameLength  = mcl;

            // Should the uMaxPathLength be set to 32767, "\\?\" prefix allows it
            pInfo->uValidFields   |= osl_VolumeInfo_Mask_MaxPathLength;
            pInfo->uMaxPathLength  = MAX_PATH;

            pInfo->uValidFields   |= osl_VolumeInfo_Mask_FileSystemName;
            rtl_uString_newFromStr(&pInfo->ustrFileSystemName, o3tl::toU(fsn));

            // volumes (even NTFS) will always be considered case
            // insensitive because the Win32 API is not able to
            // deal with case sensitive volumes see M$ Knowledge Base
            // article 100625 that's why we never set the attribute
            // osl_Volume_Attribute_Case_Sensitive

            if (flags & FS_CASE_IS_PRESERVED)
                pInfo->uAttributes |= osl_Volume_Attribute_Case_Is_Preserved;

            pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;
        }
    }
    return osl_File_E_None;
}

static bool path_get_parent(OUString& path)
{
    OSL_PRECOND(path.lastIndexOf(SLASH) == -1, "Path must not have slashes");

    if (!has_path_parent(path))
    {
        sal_Int32 i = path.lastIndexOf(BACKSLASH);
        if (-1 < i)
        {
            path = OUString(path.getStr(), i);
            return true;
        }
    }
    return false;
}

static void path_travel_to_volume_root(const OUString& system_path, OUString& volume_root)
{
    OUString sys_path(system_path);

    while(!is_volume_mount_point(sys_path) && path_get_parent(sys_path))
        /**/;

    volume_root = sys_path;
    osl::systemPathEnsureSeparator(volume_root);
}

oslFileError SAL_CALL osl_getVolumeInformation(
    rtl_uString *ustrURL, oslVolumeInfo *pInfo, sal_uInt32 uFieldMask )
{
    if (!pInfo)
        return osl_File_E_INVAL;

    OUString system_path;
    oslFileError error = osl_getSystemPathFromFileURL_(ustrURL, &system_path.pData, false);

    if (osl_File_E_None != error)
        return error;

    OUString volume_root;
    path_travel_to_volume_root(system_path, volume_root);

    pInfo->uValidFields = 0;

    if ((error = get_filesystem_attributes(volume_root, uFieldMask, pInfo)) != osl_File_E_None)
        return error;

    if (is_volume_space_info_request(uFieldMask))
        get_volume_space_information(volume_root, pInfo);

    if (uFieldMask & osl_VolumeInfo_Mask_DeviceHandle)
    {
        error = osl_getFileURLFromSystemPath(volume_root.pData, reinterpret_cast<rtl_uString**>(&pInfo->pDeviceHandle));
        if (error != osl_File_E_None)
            return error;
        pInfo->uValidFields |= osl_VolumeInfo_Mask_DeviceHandle;
    }

    return osl_File_E_None;
}

static oslFileError osl_getDriveInfo(
    oslDirectoryItem Item, oslFileStatus *pStatus, sal_uInt32 uFieldMask)
{
    DirectoryItem_Impl  *pItemImpl = static_cast<DirectoryItem_Impl *>(Item);
    WCHAR               cDrive[3] = L"A:";
    WCHAR               cRoot[4] = L"A:\\";

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    pStatus->uValidFields = 0;

    cDrive[0] = pItemImpl->cDriveString[0];
    cRoot[0] = pItemImpl->cDriveString[0];

    if ( uFieldMask & osl_FileStatus_Mask_FileName )
    {
        if ( pItemImpl->cDriveString[0] == '\\' && pItemImpl->cDriveString[1] == '\\' )
        {
            LPCWSTR lpFirstBkSlash = wcschr( &pItemImpl->cDriveString[2], '\\' );

            if ( lpFirstBkSlash && lpFirstBkSlash[1] )
            {
                LPCWSTR lpLastBkSlash = wcschr( &lpFirstBkSlash[1], '\\' );

                if ( lpLastBkSlash )
                    rtl_uString_newFromStr_WithLength( &pStatus->ustrFileName, o3tl::toU(&lpFirstBkSlash[1]), lpLastBkSlash - lpFirstBkSlash - 1 );
                else
                    rtl_uString_newFromStr( &pStatus->ustrFileName, o3tl::toU(&lpFirstBkSlash[1]) );
                pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
            }
        }
        else switch ( GetDriveTypeW( cRoot ) )
        {
            case DRIVE_REMOTE:
            {
                WCHAR szBuffer[1024];
                DWORD const dwBufsizeConst = SAL_N_ELEMENTS(szBuffer);
                DWORD dwBufsize = dwBufsizeConst;

                DWORD dwResult = WNetGetConnectionW( cDrive, szBuffer, &dwBufsize );
                if ( NO_ERROR == dwResult )
                {
                    WCHAR szFileName[dwBufsizeConst + 16];

                    swprintf( szFileName, L"%s [%s]", cDrive, szBuffer );
                    rtl_uString_newFromStr( &pStatus->ustrFileName, o3tl::toU(szFileName) );
                }
                else
                    rtl_uString_newFromStr( &pStatus->ustrFileName, o3tl::toU(cDrive) );
            }
            pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
            break;
            case DRIVE_FIXED:
            {
                WCHAR szVolumeNameBuffer[1024];
                DWORD const dwBufsizeConst = SAL_N_ELEMENTS(szVolumeNameBuffer);

                if ( GetVolumeInformationW( cRoot, szVolumeNameBuffer, dwBufsizeConst, nullptr, nullptr, nullptr, nullptr, 0 ) )
                {
                    WCHAR   szFileName[dwBufsizeConst + 16];

                    swprintf( szFileName, L"%s [%s]", cDrive, szVolumeNameBuffer );
                    rtl_uString_newFromStr( &pStatus->ustrFileName, o3tl::toU(szFileName) );
                }
                else
                    rtl_uString_newFromStr( &pStatus->ustrFileName, o3tl::toU(cDrive) );
            }
            pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
            break;
            case DRIVE_CDROM:
            case DRIVE_REMOVABLE:
                pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
                rtl_uString_newFromStr( &pStatus->ustrFileName, o3tl::toU(cRoot) );
                break;
            case DRIVE_UNKNOWN:
            default:
                break;
        }
    }

    pStatus->eType = osl_File_Type_Volume;
    pStatus->uValidFields |= osl_FileStatus_Mask_Type;

    if ( uFieldMask & osl_FileStatus_Mask_FileURL )
    {
        rtl_uString *ustrSystemPath = nullptr;

        rtl_uString_newFromStr( &ustrSystemPath, o3tl::toU(pItemImpl->cDriveString) );
        oslFileError error = osl_getFileURLFromSystemPath( ustrSystemPath, &pStatus->ustrFileURL );
        rtl_uString_release( ustrSystemPath );
        if (error != osl_File_E_None)
            return error;
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }
    return osl_File_E_None;
}

static oslFileError osl_getServerInfo(
    oslDirectoryItem Item, oslFileStatus *pStatus, sal_uInt32 uFieldMask )
{
    DirectoryItem_Impl  *pItemImpl = static_cast<DirectoryItem_Impl *>(Item);
    if ( !pItemImpl )
        return osl_File_E_INVAL;

    pStatus->uValidFields = 0;
    pStatus->eType = osl_File_Type_Directory;
    pStatus->uValidFields |= osl_FileStatus_Mask_Type;

    if ( uFieldMask & osl_FileStatus_Mask_FileURL )
    {
        oslFileError error = osl_getFileURLFromSystemPath( pItemImpl->m_sFullPath.pData, &pStatus->ustrFileURL );
        if (error != osl_File_E_None)
            return error;
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_getFileStatus(
    oslDirectoryItem Item,
    oslFileStatus *pStatus,
    sal_uInt32 uFieldMask )
{
    DirectoryItem_Impl  *pItemImpl = static_cast<DirectoryItem_Impl *>(Item);

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    switch ( pItemImpl->uType  )
    {
    case DIRECTORYITEM_DRIVE:
        return osl_getDriveInfo( Item, pStatus, uFieldMask );
    case DIRECTORYITEM_SERVER:
        return osl_getServerInfo( Item, pStatus, uFieldMask );
    default:
        break;
    }

    OUString sFullPath(pItemImpl->m_sFullPath);

    // Prefix long paths, windows API calls expect this prefix
    // (only local paths starting with something like C: or D:)
    if (sFullPath.getLength() >= MAX_PATH && isalpha(sFullPath[0]) && sFullPath[1] == ':')
        sFullPath = "\\\\?\\" + sFullPath;

    if ( uFieldMask & osl_FileStatus_Mask_Validate )
    {
        HANDLE  hFind = FindFirstFileW( o3tl::toW(sFullPath.getStr() ), &pItemImpl->FindData );

        if ( hFind != INVALID_HANDLE_VALUE )
            FindClose( hFind );
        else
            return oslTranslateFileError( GetLastError() );

        uFieldMask &= ~ osl_FileStatus_Mask_Validate;
    }

    /* If no fields to retrieve left ignore pStatus */
    if ( !uFieldMask )
        return osl_File_E_None;

    /* Otherwise, this must be a valid pointer */
    if ( !pStatus )
        return osl_File_E_INVAL;

    if ( pStatus->uStructSize != sizeof(oslFileStatus) )
        return osl_File_E_INVAL;

    pStatus->uValidFields = 0;

    /* File time stamps */

    if ( (uFieldMask & osl_FileStatus_Mask_ModifyTime) &&
        FileTimeToTimeValue( &pItemImpl->FindData.ftLastWriteTime, &pStatus->aModifyTime ) )
        pStatus->uValidFields |= osl_FileStatus_Mask_ModifyTime;

    if ( (uFieldMask & osl_FileStatus_Mask_AccessTime) &&
        FileTimeToTimeValue( &pItemImpl->FindData.ftLastAccessTime, &pStatus->aAccessTime ) )
        pStatus->uValidFields |= osl_FileStatus_Mask_AccessTime;

    if ( (uFieldMask & osl_FileStatus_Mask_CreationTime) &&
        FileTimeToTimeValue( &pItemImpl->FindData.ftCreationTime, &pStatus->aCreationTime ) )
        pStatus->uValidFields |= osl_FileStatus_Mask_CreationTime;

    /* Most of the fields are already set, regardless of required fields */

    rtl_uString_newFromStr( &pStatus->ustrFileName, o3tl::toU(pItemImpl->FindData.cFileName) );
    pStatus->uValidFields |= osl_FileStatus_Mask_FileName;

    if ((FILE_ATTRIBUTE_REPARSE_POINT & pItemImpl->FindData.dwFileAttributes) &&
        (IO_REPARSE_TAG_MOUNT_POINT == pItemImpl->FindData.dwReserved0))
        pStatus->eType = osl_File_Type_Volume;
    else if (pItemImpl->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        pStatus->eType = osl_File_Type_Directory;
    else
        pStatus->eType = osl_File_Type_Regular;

    pStatus->uValidFields |= osl_FileStatus_Mask_Type;

    pStatus->uAttributes = pItemImpl->FindData.dwFileAttributes;
    pStatus->uValidFields |= osl_FileStatus_Mask_Attributes;

    pStatus->uFileSize = static_cast<sal_uInt64>(pItemImpl->FindData.nFileSizeLow) + (static_cast<sal_uInt64>(pItemImpl->FindData.nFileSizeHigh) << 32);
    pStatus->uValidFields |= osl_FileStatus_Mask_FileSize;

    if ( uFieldMask & osl_FileStatus_Mask_LinkTargetURL )
    {
        oslFileError error = osl_getFileURLFromSystemPath( sFullPath.pData, &pStatus->ustrLinkTargetURL );
        if (error != osl_File_E_None)
            return error;

        pStatus->uValidFields |= osl_FileStatus_Mask_LinkTargetURL;
    }

    if ( uFieldMask & osl_FileStatus_Mask_FileURL )
    {
        if ( !pItemImpl->bFullPathNormalized )
        {
            ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
            sal_uInt32 nNewLen = GetCaseCorrectPathName( o3tl::toW( sFullPath.getStr() ),
                                                         o3tl::toW( aBuffer ),
                                                         aBuffer.getBufSizeInSymbols(),
                                                         true );

            if ( nNewLen )
            {
                pItemImpl->m_sFullPath = OUString(&*aBuffer, nNewLen);
                sFullPath = pItemImpl->m_sFullPath;
                pItemImpl->bFullPathNormalized = true;
            }
        }

        oslFileError error = osl_getFileURLFromSystemPath( sFullPath.pData, &pStatus->ustrFileURL );
        if (error != osl_File_E_None)
            return error;
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_setFileAttributes(
    rtl_uString *ustrFileURL,
    sal_uInt64 uAttributes )
{
    oslFileError    error;
    rtl_uString     *ustrSysPath = nullptr;
    DWORD           dwFileAttributes;
    bool            fSuccess;

    // Converts the normalized path into a systempath
    error = osl_getSystemPathFromFileURL_( ustrFileURL, &ustrSysPath, false );

    if ( osl_File_E_None != error )
        return error;

    dwFileAttributes = GetFileAttributesW( o3tl::toW(rtl_uString_getStr(ustrSysPath)) );

    if ( DWORD(-1) != dwFileAttributes )
    {
        dwFileAttributes &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);

        if ( uAttributes & osl_File_Attribute_ReadOnly )
            dwFileAttributes |= FILE_ATTRIBUTE_READONLY;

        if ( uAttributes & osl_File_Attribute_Hidden )
            dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;

        fSuccess = SetFileAttributesW( o3tl::toW(rtl_uString_getStr(ustrSysPath)), dwFileAttributes );
    }
    else
    {
        fSuccess = false;
    }

    if ( !fSuccess )
        error = oslTranslateFileError( GetLastError() );

    rtl_uString_release( ustrSysPath );

    return error;
}

oslFileError SAL_CALL osl_setFileTime(
    rtl_uString *filePath,
    const TimeValue *aCreationTime,
    const TimeValue *aLastAccessTime,
    const TimeValue *aLastWriteTime)
{
    oslFileError error;
    rtl_uString *sysPath=nullptr;
    FILETIME *lpCreationTime=nullptr;
    FILETIME *lpLastAccessTime=nullptr;
    FILETIME *lpLastWriteTime=nullptr;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    HANDLE hFile;
    bool fSuccess;

    error=osl_getSystemPathFromFileURL_(filePath, &sysPath, false);

    if (error==osl_File_E_INVAL)
        return error;

    hFile=CreateFileW(o3tl::toW(rtl_uString_getStr(sysPath)), GENERIC_WRITE, 0, nullptr , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    rtl_uString_release(sysPath);

    if (hFile==INVALID_HANDLE_VALUE)
        return osl_File_E_NOENT;

    if (TimeValueToFileTime(aCreationTime, &ftCreationTime))
        lpCreationTime=&ftCreationTime;

    if (TimeValueToFileTime(aLastAccessTime, &ftLastAccessTime))
        lpLastAccessTime=&ftLastAccessTime;

    if (TimeValueToFileTime(aLastWriteTime, &ftLastWriteTime))
        lpLastWriteTime=&ftLastWriteTime;

    fSuccess=SetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);

    CloseHandle(hFile);

    if (!fSuccess)
        return osl_File_E_INVAL;
    else
        return osl_File_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
