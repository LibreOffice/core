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

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0500
#include "systools/win32/uwinapi.h"

#include "osl/file.h"

#include "file_url.h"
#include <sal/macros.h>
#include "file_error.h"

#include "path_helper.hxx"

#include "osl/diagnose.h"
#include "osl/time.h"
#include "rtl/alloc.h"
#include "rtl/ustring.hxx"

#include <tchar.h>
#ifdef __MINGW32__
#include <ctype.h>
#endif

//#####################################################
static const wchar_t UNC_PREFIX[] = L"\\\\";
static const wchar_t BACKSLASH = '\\';
static const wchar_t SLASH = '/';

//#####################################################
extern "C" BOOL TimeValueToFileTime(const TimeValue *cpTimeVal, FILETIME *pFTime)
{
    SYSTEMTIME  BaseSysTime;
    FILETIME    BaseFileTime;
    FILETIME    FTime;
    BOOL        fSuccess = FALSE;

    BaseSysTime.wYear         = 1970;
    BaseSysTime.wMonth        = 1;
    BaseSysTime.wDayOfWeek    = 0;
    BaseSysTime.wDay          = 1;
    BaseSysTime.wHour         = 0;
    BaseSysTime.wMinute       = 0;
    BaseSysTime.wSecond       = 0;
    BaseSysTime.wMilliseconds = 0;

    if (cpTimeVal==NULL)
        return fSuccess;

    if ( SystemTimeToFileTime(&BaseSysTime, &BaseFileTime) )
    {
        __int64 timeValue;

        __int64 localTime = cpTimeVal->Seconds*(__int64)10000000+cpTimeVal->Nanosec/100;
        *(__int64 *)&FTime=localTime;
        fSuccess = 0 <= (timeValue= *((__int64 *)&BaseFileTime) + *((__int64 *) &FTime));
        if (fSuccess)
            *(__int64 *)pFTime=timeValue;
    }
    return fSuccess;
}

//#####################################################
extern "C" BOOL FileTimeToTimeValue(const FILETIME *cpFTime, TimeValue *pTimeVal)
{
    SYSTEMTIME  BaseSysTime;
    FILETIME    BaseFileTime;
    BOOL        fSuccess = FALSE;   /* Assume failure */

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

        fSuccess = 0 <= (Value = *((__int64 *)cpFTime) - *((__int64 *)&BaseFileTime));

        if ( fSuccess )
        {
            pTimeVal->Seconds  = (unsigned long) (Value / 10000000L);
            pTimeVal->Nanosec  = (unsigned long)((Value % 10000000L) * 100);
        }
    }
    return fSuccess;
}

//#####################################################
namespace /* private */
{
    //#####################################################
    struct Component
    {
        Component() :
            begin_(0), end_(0)
        {}

        bool isPresent() const
        { return (static_cast<sal_IntPtr>(end_ - begin_) > 0); }

        const sal_Unicode* begin_;
        const sal_Unicode* end_;
    };

    //#####################################################
    struct UNCComponents
    {
        Component server_;
        Component share_;
        Component resource_;
    };

    //#####################################################
    inline bool is_UNC_path(const sal_Unicode* path)
    { return (0 == wcsncmp(UNC_PREFIX, reinterpret_cast<LPCWSTR>(path), SAL_N_ELEMENTS(UNC_PREFIX) - 1)); }

    //#####################################################
    inline bool is_UNC_path(const rtl::OUString& path)
    { return is_UNC_path(path.getStr()); }

    //#####################################################
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

        OSL_POSTCOND(puncc->server_.isPresent() && puncc->share_.isPresent(), \
        "Postcondition violated: Invalid UNC path detected");
    }

    //#####################################################
    void parse_UNC_path(const rtl::OUString& path, UNCComponents* puncc)
    { parse_UNC_path(path.getStr(), puncc); }


    //#####################################################
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
            has_parent = !osl::systemPathIsLogicalDrivePattern(path);
        }
        return has_parent;
    }

    //#####################################################
    inline bool has_path_parent(const rtl::OUString& path)
    { return has_path_parent(path.getStr()); }

} // end namespace private

//#####################################################
// volume handling functions
//#####################################################

//#####################################################
oslFileError SAL_CALL osl_acquireVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
    {
        rtl_uString_acquire( (rtl_uString *)Handle );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

//#####################################################
oslFileError SAL_CALL osl_releaseVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
    {
        rtl_uString_release( (rtl_uString *)Handle );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

//#####################################################
oslFileError SAL_CALL osl_getVolumeDeviceMountPath( oslVolumeDeviceHandle Handle, rtl_uString **pstrPath )
{
    if ( Handle && pstrPath )
    {
        rtl_uString_assign( pstrPath, (rtl_uString *)Handle );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

//##################################################################
// directory handling functions
//##################################################################

#define DIRECTORYITEM_DRIVE     0
#define DIRECTORYITEM_FILE      1
#define DIRECTORYITEM_SERVER    2

struct DirectoryItem_Impl
{
    UINT uType;
    union {
        WIN32_FIND_DATA FindData;
        TCHAR           cDriveString[MAX_PATH];
    };
    rtl_uString*    m_pFullPath;
    BOOL            bFullPathNormalized;
    int             nRefCount;
};

//#####################################################

#define DIRECTORYTYPE_LOCALROOT     0
#define DIRECTORYTYPE_NETROOT       1
#define DIRECTORYTYPE_NETRESORCE    2
#define DIRECTORYTYPE_FILESYSTEM    3

struct Directory_Impl
{
    UINT uType;
    union {
        HANDLE  hDirectory;
        HANDLE  hEnumDrives;
    };
    rtl_uString*    m_pDirectoryPath;
};

//#####################################################

typedef struct tagDRIVEENUM
{
    LPCTSTR lpIdent;
    TCHAR   cBuffer[/*('Z' - 'A' + 1) * sizeof("A:\\") + 1*/256];
    LPCTSTR lpCurrent;
} DRIVEENUM, * PDRIVEENUM, FAR * LPDRIVEENUM;

//#####################################################

static HANDLE WINAPI OpenLogicalDrivesEnum(void)
{
    LPDRIVEENUM pEnum = (LPDRIVEENUM)HeapAlloc( GetProcessHeap(), 0, sizeof(DRIVEENUM) );
    if ( pEnum )
    {
        DWORD dwNumCopied = GetLogicalDriveStrings( (sizeof(pEnum->cBuffer) - 1) / sizeof(TCHAR), pEnum->cBuffer );

        if ( dwNumCopied && dwNumCopied < sizeof(pEnum->cBuffer) / sizeof(TCHAR) )
        {
            pEnum->lpCurrent = pEnum->cBuffer;
            pEnum->lpIdent = L"tagDRIVEENUM";
        }
        else
        {
            HeapFree( GetProcessHeap(), 0, pEnum );
            pEnum = NULL;
        }
    }
    return pEnum ? (HANDLE)pEnum : INVALID_HANDLE_VALUE;
}

//#####################################################
static BOOL WINAPI EnumLogicalDrives(HANDLE hEnum, LPTSTR lpBuffer)
{
    BOOL        fSuccess = FALSE;
    LPDRIVEENUM pEnum = (LPDRIVEENUM)hEnum;

    if ( pEnum )
    {
        int nLen = _tcslen( pEnum->lpCurrent );

        if ( nLen )
        {
            CopyMemory( lpBuffer, pEnum->lpCurrent, (nLen + 1) * sizeof(TCHAR) );
            pEnum->lpCurrent += nLen + 1;
            fSuccess = TRUE;
        }
        else
            SetLastError( ERROR_NO_MORE_FILES );
    }
    else
        SetLastError( ERROR_INVALID_HANDLE );

    return fSuccess;
}

//#####################################################
static BOOL WINAPI CloseLogicalDrivesEnum(HANDLE hEnum)
{
    BOOL        fSuccess = FALSE;
    LPDRIVEENUM pEnum = (LPDRIVEENUM)hEnum;

    if ( pEnum )
    {
        HeapFree( GetProcessHeap(), 0, pEnum );
        fSuccess = TRUE;
    }
    else
        SetLastError( ERROR_INVALID_HANDLE );

    return fSuccess;
}

//#####################################################
typedef struct tagDIRECTORY
{
    HANDLE          hFind;
    WIN32_FIND_DATA aFirstData;
} DIRECTORY, *PDIRECTORY, FAR *LPDIRECTORY;

//#####################################################
static HANDLE WINAPI OpenDirectory( rtl_uString* pPath)
{
    LPDIRECTORY pDirectory = NULL;

    if ( pPath )
    {
        sal_uInt32 nLen = rtl_uString_getLength( pPath );
        if ( nLen )
        {
            const TCHAR* pSuffix = 0;
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

            TCHAR* szFileMask = reinterpret_cast< TCHAR* >( rtl_allocateMemory( sizeof( TCHAR ) * ( nLen + nSuffLen + 1 ) ) );

            _tcscpy( szFileMask, reinterpret_cast<LPCTSTR>( rtl_uString_getStr( pPath ) ) );
            _tcscat( szFileMask, pSuffix );

            pDirectory = (LPDIRECTORY)HeapAlloc(GetProcessHeap(), 0, sizeof(DIRECTORY));
            pDirectory->hFind = FindFirstFile(szFileMask, &pDirectory->aFirstData);

            if (!IsValidHandle(pDirectory->hFind))
            {
                if ( GetLastError() != ERROR_NO_MORE_FILES )
                {
                    HeapFree(GetProcessHeap(), 0, pDirectory);
                    pDirectory = NULL;
                }
            }
            rtl_freeMemory(szFileMask);
        }
    }

    return (HANDLE)pDirectory;
}

//#####################################################
BOOL WINAPI EnumDirectory(HANDLE hDirectory, LPWIN32_FIND_DATA pFindData)
{
    BOOL        fSuccess = FALSE;
    LPDIRECTORY pDirectory = (LPDIRECTORY)hDirectory;

    if ( pDirectory )
    {
        BOOL    fValid;

        do
        {
            if ( pDirectory->aFirstData.cFileName[0] )
            {
                *pFindData = pDirectory->aFirstData;
                fSuccess = TRUE;
                pDirectory->aFirstData.cFileName[0] = 0;
            }
            else if ( IsValidHandle( pDirectory->hFind ) )
                fSuccess = FindNextFile( pDirectory->hFind, pFindData );
            else
            {
                fSuccess = FALSE;
                SetLastError( ERROR_NO_MORE_FILES );
            }

            fValid = fSuccess && _tcscmp( TEXT("."), pFindData->cFileName ) != 0 && _tcscmp( TEXT(".."), pFindData->cFileName ) != 0;

        } while( fSuccess && !fValid );
    }
    else
        SetLastError( ERROR_INVALID_HANDLE );

    return fSuccess;
}

//#####################################################
static BOOL WINAPI CloseDirectory(HANDLE hDirectory)
{
    BOOL        fSuccess = FALSE;
    LPDIRECTORY pDirectory = (LPDIRECTORY)hDirectory;

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

//#####################################################
static oslFileError osl_openLocalRoot(
    rtl_uString *strDirectoryPath, oslDirectory *pDirectory)
{
    rtl_uString     *strSysPath = NULL;
    oslFileError    error;

    if ( !pDirectory )
        return osl_File_E_INVAL;

    *pDirectory = NULL;

    error = _osl_getSystemPathFromFileURL( strDirectoryPath, &strSysPath, sal_False );
    if ( osl_File_E_None == error )
    {
        Directory_Impl  *pDirImpl;

        pDirImpl = reinterpret_cast<Directory_Impl*>(rtl_allocateMemory( sizeof(Directory_Impl)));
        ZeroMemory( pDirImpl, sizeof(Directory_Impl) );
        rtl_uString_newFromString( &pDirImpl->m_pDirectoryPath, strSysPath );

        /* Append backslash if neccessary */

        /* @@@ToDo
           use function ensure backslash
        */
        sal_uInt32 nLen = rtl_uString_getLength( pDirImpl->m_pDirectoryPath );
        if ( nLen && pDirImpl->m_pDirectoryPath->buffer[nLen - 1] != L'\\' )
        {
            rtl_uString* pCurDir = 0;
            rtl_uString* pBackSlash = 0;

            rtl_uString_assign( &pCurDir, pDirImpl->m_pDirectoryPath );
            rtl_uString_newFromAscii( &pBackSlash, "\\" );
            rtl_uString_newConcat( &pDirImpl->m_pDirectoryPath, pCurDir, pBackSlash );
            rtl_uString_release( pBackSlash );
            rtl_uString_release( pCurDir );
        }

        pDirImpl->uType = DIRECTORYTYPE_LOCALROOT;
        pDirImpl->hEnumDrives = OpenLogicalDrivesEnum();

        /* @@@ToDo
           Use IsValidHandle(...)
        */
        if ( pDirImpl->hEnumDrives != INVALID_HANDLE_VALUE )
        {
            *pDirectory = (oslDirectory)pDirImpl;
            error = osl_File_E_None;
        }
        else
        {
            if ( pDirImpl )
            {
                if ( pDirImpl->m_pDirectoryPath )
                {
                    rtl_uString_release( pDirImpl->m_pDirectoryPath );
                    pDirImpl->m_pDirectoryPath = 0;
                }

                rtl_freeMemory(pDirImpl);
                pDirImpl = 0;
            }

            error = oslTranslateFileError( GetLastError() );
        }

        rtl_uString_release( strSysPath );
    }
    return error;
}

//#####################################################
static oslFileError SAL_CALL osl_openFileDirectory(
    rtl_uString *strDirectoryPath, oslDirectory *pDirectory)
{
    oslFileError error = osl_File_E_None;

    if ( !pDirectory )
        return osl_File_E_INVAL;
    *pDirectory = NULL;

    Directory_Impl *pDirImpl = reinterpret_cast<Directory_Impl*>(rtl_allocateMemory(sizeof(Directory_Impl)));
    ZeroMemory( pDirImpl, sizeof(Directory_Impl) );
    rtl_uString_newFromString( &pDirImpl->m_pDirectoryPath, strDirectoryPath );

    /* Append backslash if neccessary */

    /* @@@ToDo
       use function ensure backslash
    */
    sal_uInt32 nLen = rtl_uString_getLength( pDirImpl->m_pDirectoryPath );
    if ( nLen && pDirImpl->m_pDirectoryPath->buffer[nLen - 1] != L'\\' )
    {
        rtl_uString* pCurDir = 0;
        rtl_uString* pBackSlash = 0;

        rtl_uString_assign( &pCurDir, pDirImpl->m_pDirectoryPath );
        rtl_uString_newFromAscii( &pBackSlash, "\\" );
        rtl_uString_newConcat( &pDirImpl->m_pDirectoryPath, pCurDir, pBackSlash );
        rtl_uString_release( pBackSlash );
        rtl_uString_release( pCurDir );
    }


    pDirImpl->uType = DIRECTORYTYPE_FILESYSTEM;
    pDirImpl->hDirectory = OpenDirectory( pDirImpl->m_pDirectoryPath );

    if ( !pDirImpl->hDirectory )
    {
        error = oslTranslateFileError( GetLastError() );

        if ( pDirImpl->m_pDirectoryPath )
        {
            rtl_uString_release( pDirImpl->m_pDirectoryPath );
            pDirImpl->m_pDirectoryPath = 0;
        }

        rtl_freeMemory(pDirImpl), pDirImpl = 0;
    }

    *pDirectory = (oslDirectory)(pDirImpl);
    return error;
}

//#####################################################
static oslFileError SAL_CALL osl_openNetworkServer(
    rtl_uString *strSysDirPath, oslDirectory *pDirectory)
{
    NETRESOURCEW    aNetResource;
    HANDLE          hEnum;
    DWORD           dwError;

    ZeroMemory( &aNetResource, sizeof(aNetResource) );

    aNetResource.lpRemoteName = reinterpret_cast<LPWSTR>(strSysDirPath->buffer);

    dwError = WNetOpenEnumW(
        RESOURCE_GLOBALNET,
        RESOURCETYPE_DISK,
        RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER,
        &aNetResource,
        &hEnum );

    if ( ERROR_SUCCESS == dwError )
    {
        Directory_Impl  *pDirImpl;

        pDirImpl = reinterpret_cast<Directory_Impl*>(rtl_allocateMemory(sizeof(Directory_Impl)));
        ZeroMemory( pDirImpl, sizeof(Directory_Impl) );
        pDirImpl->uType = DIRECTORYTYPE_NETROOT;
        pDirImpl->hDirectory = hEnum;
        *pDirectory = (oslDirectory)pDirImpl;
    }
    return oslTranslateFileError( dwError );
}

//#############################################
static DWORD create_dir_with_callback(
    rtl_uString * dir_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    // Create the specified directory and call the
    // user specified callback function. On success
    // the function returns ERROR_SUCCESS else a Win32 error code.

    BOOL bCreated = FALSE;

    bCreated = CreateDirectoryW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( dir_path )), NULL );

    if ( bCreated )
    {
        if (aDirectoryCreationCallbackFunc)
        {
            rtl::OUString url;
            _osl_getFileURLFromSystemPath(dir_path, &(url.pData));
            aDirectoryCreationCallbackFunc(pData, url.pData);
        }
        return ERROR_SUCCESS;
    }
    return GetLastError();
}

//#############################################
static int path_make_parent(sal_Unicode* path)
{
    /*  Cut off the last part of the given path to
    get the parent only, e.g. 'c:\dir\subdir' ->
    'c:\dir' or '\\share\sub\dir' -> '\\share\sub'
    @return The position where the path has been cut
    off (this is the posistion of the last backslash).
    If there are no more parents 0 will be returned,
    e.g. 'c:\' or '\\Share' have no more parents */

    OSL_PRECOND(rtl_ustr_indexOfChar(path, SLASH) == -1, "Path must not contain slashes");
    OSL_PRECOND(has_path_parent(path), "Path must have a parent");

    sal_Unicode* pos_last_backslash = path + rtl_ustr_lastIndexOfChar(path, BACKSLASH);
    *pos_last_backslash = 0;
    return (pos_last_backslash - path);
}

//#############################################
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

    if (ERROR_SUCCESS != w32_error)
        return w32_error;

    return create_dir_recursively_(dir_path, aDirectoryCreationCallbackFunc, pData);
}

//#############################################
oslFileError SAL_CALL osl_createDirectoryPath(
    rtl_uString* aDirectoryUrl,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    if (aDirectoryUrl == NULL)
        return osl_File_E_INVAL;

    rtl::OUString sys_path;
    oslFileError osl_error =
        _osl_getSystemPathFromFileURL(aDirectoryUrl, &sys_path.pData, sal_False);

    if (osl_error != osl_File_E_None)
        return osl_error;

    osl::systemPathRemoveSeparator(sys_path);

    // const_cast because sys_path is a local copy
    // which we want to modify inplace instead of
    // coyp it into another buffer on the heap again
    return oslTranslateFileError(create_dir_recursively_(
        sys_path.pData, aDirectoryCreationCallbackFunc, pData));
}

//#####################################################
oslFileError SAL_CALL osl_createDirectory(rtl_uString* strPath)
{
    rtl_uString *strSysPath = NULL;
    oslFileError    error = _osl_getSystemPathFromFileURL( strPath, &strSysPath, sal_False );

    if ( osl_File_E_None == error )
    {
        BOOL bCreated = FALSE;

        bCreated = CreateDirectoryW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( strSysPath )), NULL );

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
    }
    return error;
}

//#####################################################
oslFileError SAL_CALL osl_removeDirectory(rtl_uString* strPath)
{
    rtl_uString *strSysPath = NULL;
    oslFileError    error = _osl_getSystemPathFromFileURL( strPath, &strSysPath, sal_False );

    if ( osl_File_E_None == error )
    {
        if ( RemoveDirectory( reinterpret_cast<LPCTSTR>(rtl_uString_getStr( strSysPath )) ) )
            error = osl_File_E_None;
        else
            error = oslTranslateFileError( GetLastError() );

        rtl_uString_release( strSysPath );
    }
    return error;
}

//#####################################################
oslFileError SAL_CALL osl_openDirectory(rtl_uString *strDirectoryPath, oslDirectory *pDirectory)
{
    oslFileError    error;

    if ( 0 == rtl_ustr_ascii_compareIgnoreAsciiCase( strDirectoryPath->buffer, "file:///" ) )
        error = osl_openLocalRoot( strDirectoryPath, pDirectory );
    else
    {
        rtl_uString *strSysDirectoryPath = NULL;
        DWORD       dwPathType;

        error = _osl_getSystemPathFromFileURL( strDirectoryPath, &strSysDirectoryPath, sal_False );

        if ( osl_File_E_None != error )
                return error;

        dwPathType = IsValidFilePath( strSysDirectoryPath, NULL, VALIDATEPATH_NORMAL, NULL );

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

//#####################################################
static oslFileError SAL_CALL osl_getNextNetResource(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 /*uHint*/ )
{
    Directory_Impl      *pDirImpl = (Directory_Impl *)Directory;
    DirectoryItem_Impl  *pItemImpl = NULL;
    BYTE                buffer[16384];
    LPNETRESOURCEW      lpNetResource = (LPNETRESOURCEW)buffer;
    DWORD               dwError, dwCount, dwBufSize;

    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = NULL;

    if ( !pDirImpl )
        return osl_File_E_INVAL;

    dwCount = 1;
    dwBufSize = sizeof(buffer);
    dwError = WNetEnumResource( pDirImpl->hDirectory, &dwCount, lpNetResource, &dwBufSize );

    switch ( dwError )
    {
        case NO_ERROR:
        case ERROR_MORE_DATA:
        {
            pItemImpl = reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));
            if ( !pItemImpl )
                return osl_File_E_NOMEM;

            ZeroMemory( pItemImpl, sizeof(DirectoryItem_Impl) );
            pItemImpl->uType = DIRECTORYITEM_DRIVE;
            osl_acquireDirectoryItem( (oslDirectoryItem)pItemImpl );

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

//#####################################################
static oslFileError SAL_CALL osl_getNextDrive(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 /*uHint*/ )
{
    Directory_Impl      *pDirImpl = (Directory_Impl *)Directory;
    DirectoryItem_Impl  *pItemImpl = NULL;
    BOOL                fSuccess;

    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = NULL;

    if ( !pDirImpl )
        return osl_File_E_INVAL;

    pItemImpl = reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));
    if ( !pItemImpl )
        return osl_File_E_NOMEM;

    ZeroMemory( pItemImpl, sizeof(DirectoryItem_Impl) );
    pItemImpl->uType = DIRECTORYITEM_DRIVE;
    osl_acquireDirectoryItem( (oslDirectoryItem)pItemImpl );
    fSuccess = EnumLogicalDrives( pDirImpl->hEnumDrives, pItemImpl->cDriveString );

    if ( fSuccess )
    {
        *pItem = pItemImpl;
        return osl_File_E_None;
    }
    else
    {
        if ( pItemImpl->m_pFullPath )
        {
            rtl_uString_release( pItemImpl->m_pFullPath );
            pItemImpl->m_pFullPath = 0;
        }

        rtl_freeMemory( pItemImpl );
        return oslTranslateFileError( GetLastError() );
    }
}

//#####################################################
static oslFileError SAL_CALL osl_getNextFileItem(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 /*uHint*/)
{
    Directory_Impl      *pDirImpl = (Directory_Impl *)Directory;
    DirectoryItem_Impl  *pItemImpl = NULL;
    BOOL                fFound;

    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = NULL;

    if ( !pDirImpl )
        return osl_File_E_INVAL;

    pItemImpl = reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));
    if ( !pItemImpl )
        return osl_File_E_NOMEM;

    memset( pItemImpl, 0, sizeof(DirectoryItem_Impl) );
    fFound = EnumDirectory( pDirImpl->hDirectory, &pItemImpl->FindData );

    if ( fFound )
    {
        pItemImpl->uType = DIRECTORYITEM_FILE;
        pItemImpl->nRefCount = 1;

        rtl_uString* pTmpFileName = 0;
        rtl_uString_newFromStr( &pTmpFileName,  reinterpret_cast<const sal_Unicode *>(pItemImpl->FindData.cFileName) );
        rtl_uString_newConcat( &pItemImpl->m_pFullPath, pDirImpl->m_pDirectoryPath, pTmpFileName );
        rtl_uString_release( pTmpFileName );

        pItemImpl->bFullPathNormalized = FALSE;
        *pItem = (oslDirectoryItem)pItemImpl;
        return osl_File_E_None;
    }
    else
    {
        if ( pItemImpl->m_pFullPath )
        {
            rtl_uString_release( pItemImpl->m_pFullPath );
            pItemImpl->m_pFullPath = 0;
        }

        rtl_freeMemory( pItemImpl );
        return oslTranslateFileError( GetLastError() );
    }
}

//#####################################################
oslFileError SAL_CALL osl_getNextDirectoryItem(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 uHint)
{
    Directory_Impl      *pDirImpl = (Directory_Impl *)Directory;

    /* Assume failure */

    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = NULL;

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

//#####################################################
oslFileError SAL_CALL osl_closeDirectory(oslDirectory Directory)
{
    Directory_Impl  *pDirImpl = (Directory_Impl *)Directory;
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

        if ( pDirImpl->m_pDirectoryPath )
        {
            rtl_uString_release( pDirImpl->m_pDirectoryPath );
            pDirImpl->m_pDirectoryPath = 0;
        }

        rtl_freeMemory(pDirImpl);
    }
    return eError;
}

//#####################################################
/* Different types of paths */
typedef enum _PATHTYPE
{
    PATHTYPE_SYNTAXERROR = 0,
    PATHTYPE_NETROOT,
    PATHTYPE_NETSERVER,
    PATHTYPE_VOLUME,
    PATHTYPE_FILE
} PATHTYPE;

oslFileError SAL_CALL osl_getDirectoryItem(rtl_uString *strFilePath, oslDirectoryItem *pItem)
{
    oslFileError    error = osl_File_E_None;
    rtl_uString*    strSysFilePath = NULL;
    PATHTYPE        type = PATHTYPE_FILE;
    DWORD           dwPathType;

    /* Assume failure */

    if ( !pItem )
        return osl_File_E_INVAL;

    *pItem = NULL;


    error = _osl_getSystemPathFromFileURL( strFilePath, &strSysFilePath, sal_False );

    if ( osl_File_E_None != error )
            return error;

    dwPathType = IsValidFilePath( strSysFilePath, NULL, VALIDATEPATH_NORMAL, NULL );

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
            DirectoryItem_Impl* pItemImpl =
                reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));

            if ( !pItemImpl )
                error = osl_File_E_NOMEM;

            if ( osl_File_E_None == error )
            {
                ZeroMemory( pItemImpl, sizeof(DirectoryItem_Impl) );
                pItemImpl->uType = DIRECTORYITEM_SERVER;

                osl_acquireDirectoryItem( (oslDirectoryItem)pItemImpl );
                rtl_uString_newFromString( &pItemImpl->m_pFullPath, strSysFilePath );

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
            DirectoryItem_Impl* pItemImpl =
                reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));

            if ( !pItemImpl )
                error = osl_File_E_NOMEM;

            if ( osl_File_E_None == error )
            {
                ZeroMemory( pItemImpl, sizeof(DirectoryItem_Impl) );
                pItemImpl->uType = DIRECTORYITEM_DRIVE;

                osl_acquireDirectoryItem( (oslDirectoryItem)pItemImpl );

                _tcscpy( pItemImpl->cDriveString, reinterpret_cast<LPCTSTR>(strSysFilePath->buffer) );
                pItemImpl->cDriveString[0] = toupper( pItemImpl->cDriveString[0] );

                if ( pItemImpl->cDriveString[_tcslen(pItemImpl->cDriveString) - 1] != '\\' )
                    _tcscat( pItemImpl->cDriveString, TEXT( "\\" ) );

                *pItem = pItemImpl;
            }
        }
        break;
    case PATHTYPE_SYNTAXERROR:
    case PATHTYPE_NETROOT:
    case PATHTYPE_FILE:
        {
            HANDLE              hFind;
            WIN32_FIND_DATA     aFindData;

            if ( strSysFilePath->length > 0 && strSysFilePath->buffer[strSysFilePath->length - 1] == '\\' )
                rtl_uString_newFromStr_WithLength( &strSysFilePath, strSysFilePath->buffer, strSysFilePath->length - 1 );

            hFind = FindFirstFile( reinterpret_cast<LPCTSTR>(rtl_uString_getStr(strSysFilePath)), &aFindData );

            if ( hFind != INVALID_HANDLE_VALUE )
            {
                DirectoryItem_Impl  *pItemImpl =
                    reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));

                ZeroMemory( pItemImpl, sizeof(DirectoryItem_Impl) );
                osl_acquireDirectoryItem( (oslDirectoryItem)pItemImpl );

                CopyMemory( &pItemImpl->FindData, &aFindData, sizeof(WIN32_FIND_DATA) );
                rtl_uString_newFromString( &pItemImpl->m_pFullPath, strSysFilePath );

                // MT: This costs 600ms startup time on fast v60x!
                // GetCaseCorrectPathName( pItemImpl->szFullPath, pItemImpl->szFullPath, sizeof(pItemImpl->szFullPath) );

                pItemImpl->uType = DIRECTORYITEM_FILE;
                *pItem = pItemImpl;
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

//#####################################################
oslFileError SAL_CALL osl_acquireDirectoryItem( oslDirectoryItem Item )
{
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    pItemImpl->nRefCount++;
    return osl_File_E_None;
}

//#####################################################
oslFileError SAL_CALL osl_releaseDirectoryItem( oslDirectoryItem Item )
{
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    if ( ! --pItemImpl->nRefCount )
    {
        if ( pItemImpl->m_pFullPath )
        {
            rtl_uString_release( pItemImpl->m_pFullPath );
            pItemImpl->m_pFullPath = 0;
        }

        rtl_freeMemory( pItemImpl );
    }

    return osl_File_E_None;
}


sal_Bool
SAL_CALL osl_identicalDirectoryItem( oslDirectoryItem a, oslDirectoryItem b)
{
    DirectoryItem_Impl *pA = (DirectoryItem_Impl *) a;
    DirectoryItem_Impl *pB = (DirectoryItem_Impl *) b;
    if (a == b)
        return sal_True;
    /* same name => same item, unless renaming / moving madness has occurred */
    if (rtl_ustr_compare_WithLength(
                pA->m_pFullPath->buffer, pA->m_pFullPath->length,
                pB->m_pFullPath->buffer, pB->m_pFullPath->length ) == 0)
        return sal_True;

    // FIXME: as/when/if this is used in anger on Windows we could
    // do better here.

    return sal_False;
}

//#####################################################
// volume / file info handling functions
//#####################################################

//#####################################################
static inline bool is_floppy_A_present()
{ return (GetLogicalDrives() & 1); }

//#####################################################
static inline bool is_floppy_B_present()
{ return (GetLogicalDrives() & 2); }

//#####################################################
bool is_floppy_volume_mount_point(const rtl::OUString& path)
{
    // determines if a volume mount point shows to a floppy
    // disk by comparing the unique volume names
    static const LPCWSTR FLOPPY_A = L"A:\\";
    static const LPCWSTR FLOPPY_B = L"B:\\";

    rtl::OUString p(path);
    osl::systemPathEnsureSeparator(p);

    TCHAR vn[51];
    if (GetVolumeNameForVolumeMountPoint(reinterpret_cast<LPCTSTR>(p.getStr()), vn, SAL_N_ELEMENTS(vn)))
    {
        TCHAR vnfloppy[51];
        if (is_floppy_A_present() &&
            GetVolumeNameForVolumeMountPoint(FLOPPY_A, vnfloppy, SAL_N_ELEMENTS(vnfloppy)) &&
            (0 == wcscmp(vn, vnfloppy)))
            return true;

        if (is_floppy_B_present() &&
            GetVolumeNameForVolumeMountPoint(FLOPPY_B, vnfloppy, SAL_N_ELEMENTS(vnfloppy)) &&
            (0 == wcscmp(vn, vnfloppy)))
            return true;
    }
    return false;
}

//################################################
static bool is_floppy_drive(const rtl::OUString& path)
{
    static const LPCWSTR FLOPPY_DRV_LETTERS = TEXT("AaBb");

    // we must take into account that even a floppy
    // drive may be mounted to a directory so checking
    // for the drive letter alone is not sufficient
    // we must compare the unique volume name with
    // that of the available floppy disks

    const sal_Unicode* pszPath = path.getStr();
    return ((wcschr(FLOPPY_DRV_LETTERS, pszPath[0]) && (L':' == pszPath[1])) || is_floppy_volume_mount_point(path));
}

//#####################################################
static bool is_volume_mount_point(const rtl::OUString& path)
{
    rtl::OUString p(path);
    osl::systemPathRemoveSeparator(p);

    bool  is_volume_root = false;

    if (!is_floppy_drive(p))
    {
        DWORD fattr = GetFileAttributes(reinterpret_cast<LPCTSTR>(p.getStr()));

        if ((INVALID_FILE_ATTRIBUTES != fattr) &&
            (FILE_ATTRIBUTE_REPARSE_POINT & fattr))
        {
            WIN32_FIND_DATA find_data;
            HANDLE h_find = FindFirstFile(reinterpret_cast<LPCTSTR>(p.getStr()), &find_data);

            if (IsValidHandle(h_find) &&
                (FILE_ATTRIBUTE_REPARSE_POINT & find_data.dwFileAttributes) &&
                (IO_REPARSE_TAG_MOUNT_POINT == find_data.dwReserved0))
            {
                is_volume_root = true;
            }
            if (IsValidHandle(h_find))
                FindClose(h_find);
        }
    }
    return is_volume_root;
}

//#############################################
static UINT get_volume_mount_point_drive_type(const rtl::OUString& path)
{
    if (0 == path.getLength())
        return GetDriveType(NULL);

    rtl::OUString p(path);
    osl::systemPathEnsureSeparator(p);

    TCHAR vn[51];
    if (GetVolumeNameForVolumeMountPoint(reinterpret_cast<LPCTSTR>(p.getStr()), vn, SAL_N_ELEMENTS(vn)))
        return GetDriveType(vn);

    return DRIVE_NO_ROOT_DIR;
}

//#############################################
static inline bool is_drivetype_request(sal_uInt32 field_mask)
{
    return (field_mask & osl_VolumeInfo_Mask_Attributes);
}

//#############################################
static oslFileError osl_get_drive_type(
    const rtl::OUString& path, oslVolumeInfo* pInfo)
{
    // GetDriveType fails on empty volume mount points
    // see Knowledge Base Q244089
    UINT drive_type;
    if (is_volume_mount_point(path))
        drive_type = get_volume_mount_point_drive_type(path);
    else
        drive_type = GetDriveType(reinterpret_cast<LPCTSTR>(path.getStr()));

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

//#############################################
static inline bool is_volume_space_info_request(sal_uInt32 field_mask)
{
    return (field_mask &
            (osl_VolumeInfo_Mask_TotalSpace |
             osl_VolumeInfo_Mask_UsedSpace  |
             osl_VolumeInfo_Mask_FreeSpace));
}

//#############################################
static void get_volume_space_information(
    const rtl::OUString& path, oslVolumeInfo *pInfo)
{
    BOOL ret = GetDiskFreeSpaceEx(
        reinterpret_cast<LPCTSTR>(path.getStr()),
        (PULARGE_INTEGER)&(pInfo->uFreeSpace),
        (PULARGE_INTEGER)&(pInfo->uTotalSpace),
        NULL);

    if (ret)
    {
        pInfo->uUsedSpace    = pInfo->uTotalSpace - pInfo->uFreeSpace;
        pInfo->uValidFields |= osl_VolumeInfo_Mask_TotalSpace |
            osl_VolumeInfo_Mask_UsedSpace |
            osl_VolumeInfo_Mask_FreeSpace;
    }
}

//#############################################
static inline bool is_filesystem_attributes_request(sal_uInt32 field_mask)
{
    return (field_mask &
            (osl_VolumeInfo_Mask_MaxNameLength |
             osl_VolumeInfo_Mask_MaxPathLength |
             osl_VolumeInfo_Mask_FileSystemName |
             osl_VolumeInfo_Mask_FileSystemCaseHandling));
}

//#############################################
static oslFileError get_filesystem_attributes(
    const rtl::OUString& path, sal_uInt32 field_mask, oslVolumeInfo* pInfo)
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

        LPCTSTR pszPath = reinterpret_cast<LPCTSTR>(path.getStr());
        if (GetVolumeInformation(pszPath, vn, MAX_PATH+1, &serial, &mcl, &flags, fsn, MAX_PATH+1))
        {
            // Currently sal does not use this value, instead MAX_PATH is used
            pInfo->uValidFields   |= osl_VolumeInfo_Mask_MaxNameLength;
            pInfo->uMaxNameLength  = mcl;

            // Should the uMaxPathLength be set to 32767, "\\?\" prefix allowes it
            pInfo->uValidFields   |= osl_VolumeInfo_Mask_MaxPathLength;
            pInfo->uMaxPathLength  = MAX_PATH;

            pInfo->uValidFields   |= osl_VolumeInfo_Mask_FileSystemName;
            rtl_uString_newFromStr(&pInfo->ustrFileSystemName, reinterpret_cast<const sal_Unicode*>(fsn));

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

//#####################################################
static bool path_get_parent(rtl::OUString& path)
{
    OSL_PRECOND(path.lastIndexOf(SLASH) == -1, "Path must not have slashes");

    if (!has_path_parent(path))
    {
        sal_Int32 i = path.lastIndexOf(BACKSLASH);
        if (-1 < i)
        {
            path = rtl::OUString(path.getStr(), i);
            return true;
        }
    }
    return false;
}

//#####################################################
static void path_travel_to_volume_root(const rtl::OUString& system_path, rtl::OUString& volume_root)
{
    rtl::OUString sys_path(system_path);

    while(!is_volume_mount_point(sys_path) && path_get_parent(sys_path))
        /**/;

    volume_root = sys_path;
    osl::systemPathEnsureSeparator(volume_root);
}

//#############################################
oslFileError SAL_CALL osl_getVolumeInformation(
    rtl_uString *ustrURL, oslVolumeInfo *pInfo, sal_uInt32 uFieldMask )
{
    if (!pInfo)
        return osl_File_E_INVAL;

    rtl::OUString system_path;
    oslFileError error = _osl_getSystemPathFromFileURL(ustrURL, &system_path.pData, sal_False);

    if (osl_File_E_None != error)
        return error;

    rtl::OUString volume_root;
    path_travel_to_volume_root(system_path, volume_root);

    pInfo->uValidFields = 0;

    if ((error = get_filesystem_attributes(volume_root, uFieldMask, pInfo)) != osl_File_E_None)
        return error;

    if (is_volume_space_info_request(uFieldMask))
        get_volume_space_information(volume_root, pInfo);

    if (uFieldMask & osl_VolumeInfo_Mask_DeviceHandle)
    {
        pInfo->uValidFields |= osl_VolumeInfo_Mask_DeviceHandle;
        osl_getFileURLFromSystemPath(volume_root.pData, (rtl_uString**)&pInfo->pDeviceHandle);
    }

    return osl_File_E_None;
}

//#####################################################
static oslFileError SAL_CALL osl_getDriveInfo(
    oslDirectoryItem Item, oslFileStatus *pStatus, sal_uInt32 uFieldMask)
{
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;
    TCHAR               cDrive[3] = TEXT("A:");
    TCHAR               cRoot[4] = TEXT("A:\\");

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
                    rtl_uString_newFromStr_WithLength( &pStatus->ustrFileName, reinterpret_cast<const sal_Unicode*>(&lpFirstBkSlash[1]), lpLastBkSlash - lpFirstBkSlash - 1 );
                else
                    rtl_uString_newFromStr( &pStatus->ustrFileName, reinterpret_cast<const sal_Unicode*>(&lpFirstBkSlash[1]) );
                pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
            }
        }
        else switch ( GetDriveType( cRoot ) )
        {
            case DRIVE_REMOTE:
            {
                TCHAR szBuffer[1024];
                DWORD const dwBufsizeConst = SAL_N_ELEMENTS(szBuffer);
                DWORD dwBufsize = dwBufsizeConst;

                DWORD dwResult = WNetGetConnection( cDrive, szBuffer, &dwBufsize );
                if ( NO_ERROR == dwResult )
                {
                    TCHAR szFileName[dwBufsizeConst + 16];

                    swprintf( szFileName, L"%s [%s]", cDrive, szBuffer );
                    rtl_uString_newFromStr( &pStatus->ustrFileName, reinterpret_cast<const sal_Unicode*>(szFileName) );
                }
                else
                    rtl_uString_newFromStr( &pStatus->ustrFileName, reinterpret_cast<const sal_Unicode*>(cDrive) );
            }
            pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
            break;
            case DRIVE_FIXED:
            {
                TCHAR szVolumeNameBuffer[1024];
                DWORD const dwBufsizeConst = SAL_N_ELEMENTS(szVolumeNameBuffer);

                if ( GetVolumeInformation( cRoot, szVolumeNameBuffer, dwBufsizeConst, NULL, NULL, NULL, NULL, 0 ) )
                {
                    TCHAR   szFileName[dwBufsizeConst + 16];

                    swprintf( szFileName, L"%s [%s]", cDrive, szVolumeNameBuffer );
                    rtl_uString_newFromStr( &pStatus->ustrFileName, reinterpret_cast<const sal_Unicode*>(szFileName) );
                }
                else
                    rtl_uString_newFromStr( &pStatus->ustrFileName, reinterpret_cast<const sal_Unicode*>(cDrive) );
            }
            pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
            break;
            case DRIVE_CDROM:
            case DRIVE_REMOVABLE:
                pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
                rtl_uString_newFromStr( &pStatus->ustrFileName, reinterpret_cast<const sal_Unicode*>(cRoot) );
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
        rtl_uString *ustrSystemPath = NULL;

        rtl_uString_newFromStr( &ustrSystemPath, reinterpret_cast<const sal_Unicode*>(pItemImpl->cDriveString) );
        osl_getFileURLFromSystemPath( ustrSystemPath, &pStatus->ustrFileURL );
        rtl_uString_release( ustrSystemPath );
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }
    return osl_File_E_None;
}

//#####################################################
static oslFileError SAL_CALL osl_getServerInfo(
    oslDirectoryItem Item, oslFileStatus *pStatus, sal_uInt32 uFieldMask )
{
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;
    if ( !pItemImpl )
        return osl_File_E_INVAL;

    pStatus->uValidFields = 0;

    //  pStatus->uValidFields |= osl_FileStatus_Mask_FileName;

    //  if ( _tcscmp( pItemImpl->FindData.cFileName, TEXT(".") ) == 0 )
    //      rtl_uString_newFromAscii( &pStatus->ustrFileName, "/" );
    //  else
    //      rtl_uString_newFromStr( &pStatus->ustrFileName, pItemImpl->FindData.cFileName );

    pStatus->eType = osl_File_Type_Directory;
    pStatus->uValidFields |= osl_FileStatus_Mask_Type;

    if ( uFieldMask & osl_FileStatus_Mask_FileURL )
    {
        osl_getFileURLFromSystemPath( pItemImpl->m_pFullPath, &pStatus->ustrFileURL );
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }
    return osl_File_E_None;
}

//#############################################
oslFileError SAL_CALL osl_getFileStatus(
    oslDirectoryItem Item,
    oslFileStatus *pStatus,
    sal_uInt32 uFieldMask )
{
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;

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

    if ( uFieldMask & osl_FileStatus_Mask_Validate )
    {
        HANDLE  hFind = FindFirstFile( reinterpret_cast<LPCTSTR>( rtl_uString_getStr( pItemImpl->m_pFullPath ) ), &pItemImpl->FindData );

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

    /* Most of the fields are already set, regardless of requiered fields */

    rtl_uString_newFromStr( &pStatus->ustrFileName, reinterpret_cast<const sal_Unicode*>(pItemImpl->FindData.cFileName) );
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

    pStatus->uFileSize = (sal_uInt64)pItemImpl->FindData.nFileSizeLow + ((sal_uInt64)pItemImpl->FindData.nFileSizeHigh << 32);
    pStatus->uValidFields |= osl_FileStatus_Mask_FileSize;

    if ( uFieldMask & osl_FileStatus_Mask_LinkTargetURL )
    {
        osl_getFileURLFromSystemPath( pItemImpl->m_pFullPath, &pStatus->ustrLinkTargetURL );

        pStatus->uValidFields |= osl_FileStatus_Mask_LinkTargetURL;
    }

    if ( uFieldMask & osl_FileStatus_Mask_FileURL )
    {
        if ( !pItemImpl->bFullPathNormalized )
        {
            ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
            sal_uInt32 nNewLen = GetCaseCorrectPathName( reinterpret_cast<LPCTSTR>( rtl_uString_getStr( pItemImpl->m_pFullPath ) ),
                                                      ::osl::mingw_reinterpret_cast<LPTSTR>( aBuffer ),
                                                      aBuffer.getBufSizeInSymbols(),
                                                      sal_True );

            if ( nNewLen )
            {
                rtl_uString_newFromStr( &pItemImpl->m_pFullPath, aBuffer );
                pItemImpl->bFullPathNormalized = TRUE;
            }
        }

        osl_getFileURLFromSystemPath( pItemImpl->m_pFullPath, &pStatus->ustrFileURL );
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }

    return osl_File_E_None;
}

//#####################################################
// file attributes handling functions
//#####################################################

//#############################################
oslFileError SAL_CALL osl_setFileAttributes(
    rtl_uString *ustrFileURL,
    sal_uInt64 uAttributes )
{
    oslFileError    error;
    rtl_uString     *ustrSysPath = NULL;
    DWORD           dwFileAttributes;
    BOOL            fSuccess;

    // Converts the normalized path into a systempath
    error = _osl_getSystemPathFromFileURL( ustrFileURL, &ustrSysPath, sal_False );

    if ( osl_File_E_None != error )
        return error;

    dwFileAttributes = GetFileAttributes( reinterpret_cast<LPCTSTR>(rtl_uString_getStr(ustrSysPath)) );

    if ( (DWORD)-1 != dwFileAttributes )
    {
        dwFileAttributes &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);

        if ( uAttributes & osl_File_Attribute_ReadOnly )
            dwFileAttributes |= FILE_ATTRIBUTE_READONLY;

        if ( uAttributes & osl_File_Attribute_Hidden )
            dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;

        fSuccess = SetFileAttributes( reinterpret_cast<LPCTSTR>(rtl_uString_getStr(ustrSysPath)), dwFileAttributes );
    }
    else
        fSuccess = FALSE;

    if ( !fSuccess )
        error = oslTranslateFileError( GetLastError() );

    rtl_uString_release( ustrSysPath );

    return error;
}

//#####################################################
oslFileError SAL_CALL osl_setFileTime(
    rtl_uString *filePath,
    const TimeValue *aCreationTime,
    const TimeValue *aLastAccessTime,
    const TimeValue *aLastWriteTime)
{
    oslFileError error;
    rtl_uString *sysPath=NULL;
    FILETIME *lpCreationTime=NULL;
    FILETIME *lpLastAccessTime=NULL;
    FILETIME *lpLastWriteTime=NULL;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    HANDLE hFile;
    BOOL fSuccess;


    error=_osl_getSystemPathFromFileURL(filePath, &sysPath, sal_False);

    if (error==osl_File_E_INVAL)
        return error;

    hFile=CreateFileW(reinterpret_cast<LPCWSTR>(rtl_uString_getStr(sysPath)), GENERIC_WRITE, 0, NULL , OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
