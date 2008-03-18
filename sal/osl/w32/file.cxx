/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: file.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 13:16:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0500
#include "systools\win32\uwinapi.h"

#include "path_helper.hxx"

#include "sal/types.h"

#include "osl/file.hxx"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "rtl/alloc.h"
#include "rtl/tencinfo.h"
#include "osl/thread.h"
#include "osl/mutex.h"
#include "rtl/byteseq.h"
#include "osl/time.h"
//#include <rtl/logfile.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#ifdef __MINGW32__
#include <wchar.h>
#include <ctype.h>
#endif
#include <malloc.h>
#include <algorithm>

//#####################################################
// BEGIN global
//#####################################################

extern "C" oslMutex g_CurrentDirectoryMutex; /* Initialized in dllentry.c */
oslMutex g_CurrentDirectoryMutex;

//#####################################################
extern "C" BOOL TimeValueToFileTime(const TimeValue *cpTimeVal, FILETIME *pFTime)
{
    SYSTEMTIME  BaseSysTime;
    FILETIME    BaseFileTime;
    FILETIME    FTime;
    __int64     localTime;
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
        localTime=cpTimeVal->Seconds*(__int64)10000000+cpTimeVal->Nanosec/100;
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
extern "C" oslFileHandle SAL_CALL osl_createFileHandleFromOSHandle(HANDLE hFile)
{
    if ( IsValidHandle(hFile) )
        return (oslFileHandle)hFile;
    else
        return NULL;
}

//#####################################################
// End global
//#####################################################


using namespace osl;

#define ELEMENTS_OF_ARRAY(arr) (sizeof(arr)/(sizeof((arr)[0])))

// Allocate n number of t's on the stack return a pointer to it in p
#ifdef __MINGW32__
#define STACK_ALLOC(p, t, n) (p) = reinterpret_cast<t*>(_alloca((n)*sizeof(t)));
#else
#define STACK_ALLOC(p, t, n) __try {(p) = reinterpret_cast<t*>(_alloca((n)*sizeof(t)));} \
                             __except(EXCEPTION_EXECUTE_HANDLER) {(p) = NULL;}
#endif

#if OSL_DEBUG_LEVEL > 0
#define OSL_ENSURE_FILE( cond, msg, file ) ( (cond) ?  (void)0 : _osl_warnFile( msg, file ) )
#else
#define OSL_ENSURE_FILE( cond, msg, file ) ((void)0)
#endif

#define PATHTYPE_ERROR                      0
#define PATHTYPE_RELATIVE                   1
#define PATHTYPE_ABSOLUTE_UNC               2
#define PATHTYPE_ABSOLUTE_LOCAL             3
#define PATHTYPE_MASK_TYPE                  0xFF
#define PATHTYPE_IS_VOLUME                  0x0100
#define PATHTYPE_IS_SERVER                  0x0200

#define VALIDATEPATH_NORMAL                 0x0000
#define VALIDATEPATH_ALLOW_WILDCARDS        0x0001
#define VALIDATEPATH_ALLOW_ELLIPSE          0x0002
#define VALIDATEPATH_ALLOW_RELATIVE         0x0004
#define VALIDATEPATH_ALLOW_UNC              0x0008

#define WSTR_SYSTEM_ROOT_PATH               L"\\\\.\\"

typedef struct {
    UINT    uType;

    union {
        WIN32_FIND_DATA FindData;
        TCHAR           cDriveString[MAX_PATH];
    };
    TCHAR           szFullPath[MAX_PATH];
    BOOL            bFullPathNormalized;
    int             nRefCount;
}DirectoryItem_Impl;

#define DIRECTORYTYPE_LOCALROOT     0
#define DIRECTORYTYPE_NETROOT       1
#define DIRECTORYTYPE_NETRESORCE    2
#define DIRECTORYTYPE_FILESYSTEM    3

#define DIRECTORYITEM_DRIVE     0
#define DIRECTORYITEM_FILE      1
#define DIRECTORYITEM_SERVER    2

typedef struct {
    UINT    uType;
    union {
        HANDLE  hDirectory;
        HANDLE  hEnumDrives;
    };
    TCHAR   szDirectoryPath[MAX_PATH];
} Directory_Impl;

/* Different types of paths */
typedef enum _PATHTYPE
{
    PATHTYPE_SYNTAXERROR = 0,
    PATHTYPE_NETROOT,
    PATHTYPE_NETSERVER,
    PATHTYPE_VOLUME,
    PATHTYPE_FILE
} PATHTYPE;


namespace /* private */
{
    // forward
    void _osl_warnFile(const char*, rtl_uString*);
    oslFileError SAL_CALL _osl_getFileURLFromSystemPath(rtl_uString* , rtl_uString**);
    DWORD WINAPI IsValidFilePath(rtl_uString*, LPCTSTR*, DWORD, rtl_uString**);
    HANDLE WINAPI OpenLogicalDrivesEnum(void);
    BOOL WINAPI EnumLogicalDrives(HANDLE, LPTSTR);
    BOOL WINAPI CloseLogicalDrivesEnum(HANDLE);
    HANDLE WINAPI OpenDirectory(LPCTSTR);
    BOOL WINAPI CloseDirectory(HANDLE);
    BOOL WINAPI EnumDirectory(HANDLE, LPWIN32_FIND_DATA);
    DWORD WINAPI GetCaseCorrectPathName(LPCTSTR, LPTSTR, DWORD);
    oslFileError SAL_CALL _osl_getSystemPathFromFileURL(rtl_uString*, rtl_uString**, sal_Bool);

    /* OS error to errno values mapping table */
    struct errentry {
        unsigned long oscode;   /* OS return value */
        int errnocode;          /* System V error code */
    };

    struct errentry errtable[] = {
        {  ERROR_SUCCESS,                osl_File_E_None     },  /* 0 */
        {  ERROR_INVALID_FUNCTION,       osl_File_E_INVAL    },  /* 1 */
        {  ERROR_FILE_NOT_FOUND,         osl_File_E_NOENT    },  /* 2 */
        {  ERROR_PATH_NOT_FOUND,         osl_File_E_NOENT    },  /* 3 */
        {  ERROR_TOO_MANY_OPEN_FILES,    osl_File_E_MFILE    },  /* 4 */
        {  ERROR_ACCESS_DENIED,          osl_File_E_ACCES    },  /* 5 */
        {  ERROR_INVALID_HANDLE,         osl_File_E_BADF     },  /* 6 */
        {  ERROR_ARENA_TRASHED,          osl_File_E_NOMEM    },  /* 7 */
        {  ERROR_NOT_ENOUGH_MEMORY,      osl_File_E_NOMEM    },  /* 8 */
        {  ERROR_INVALID_BLOCK,          osl_File_E_NOMEM    },  /* 9 */
        {  ERROR_BAD_ENVIRONMENT,        osl_File_E_2BIG     },  /* 10 */
        {  ERROR_BAD_FORMAT,             osl_File_E_NOEXEC   },  /* 11 */
        {  ERROR_INVALID_ACCESS,         osl_File_E_INVAL    },  /* 12 */
        {  ERROR_INVALID_DATA,           osl_File_E_INVAL    },  /* 13 */
        {  ERROR_INVALID_DRIVE,          osl_File_E_NOENT    },  /* 15 */
        {  ERROR_CURRENT_DIRECTORY,      osl_File_E_ACCES    },  /* 16 */
        {  ERROR_NOT_SAME_DEVICE,        osl_File_E_XDEV     },  /* 17 */
        {  ERROR_NO_MORE_FILES,          osl_File_E_NOENT    },  /* 18 */
        {  ERROR_NOT_READY,              osl_File_E_NOTREADY },  /* 21 */
        {  ERROR_LOCK_VIOLATION,         osl_File_E_ACCES    },  /* 33 */
        {  ERROR_BAD_NETPATH,            osl_File_E_NOENT    },  /* 53 */
        {  ERROR_NETWORK_ACCESS_DENIED,  osl_File_E_ACCES    },  /* 65 */
        {  ERROR_BAD_NET_NAME,           osl_File_E_NOENT    },  /* 67 */
        {  ERROR_FILE_EXISTS,            osl_File_E_EXIST    },  /* 80 */
        {  ERROR_CANNOT_MAKE,            osl_File_E_ACCES    },  /* 82 */
        {  ERROR_FAIL_I24,               osl_File_E_ACCES    },  /* 83 */
        {  ERROR_INVALID_PARAMETER,      osl_File_E_INVAL    },  /* 87 */
        {  ERROR_NO_PROC_SLOTS,          osl_File_E_AGAIN    },  /* 89 */
        {  ERROR_DRIVE_LOCKED,           osl_File_E_ACCES    },  /* 108 */
        {  ERROR_BROKEN_PIPE,            osl_File_E_PIPE     },  /* 109 */
        {  ERROR_DISK_FULL,              osl_File_E_NOSPC    },  /* 112 */
        {  ERROR_INVALID_TARGET_HANDLE,  osl_File_E_BADF     },  /* 114 */
        {  ERROR_INVALID_HANDLE,         osl_File_E_INVAL    },  /* 124 */
        {  ERROR_WAIT_NO_CHILDREN,       osl_File_E_CHILD    },  /* 128 */
        {  ERROR_CHILD_NOT_COMPLETE,     osl_File_E_CHILD    },  /* 129 */
        {  ERROR_DIRECT_ACCESS_HANDLE,   osl_File_E_BADF     },  /* 130 */
        {  ERROR_NEGATIVE_SEEK,          osl_File_E_INVAL    },  /* 131 */
        {  ERROR_SEEK_ON_DEVICE,         osl_File_E_ACCES    },  /* 132 */
        {  ERROR_DIR_NOT_EMPTY,          osl_File_E_NOTEMPTY },  /* 145 */
        {  ERROR_NOT_LOCKED,             osl_File_E_ACCES    },  /* 158 */
        {  ERROR_BAD_PATHNAME,           osl_File_E_NOENT    },  /* 161 */
        {  ERROR_MAX_THRDS_REACHED,      osl_File_E_AGAIN    },  /* 164 */
        {  ERROR_LOCK_FAILED,            osl_File_E_ACCES    },  /* 167 */
        {  ERROR_ALREADY_EXISTS,         osl_File_E_EXIST    },  /* 183 */
        {  ERROR_FILENAME_EXCED_RANGE,   osl_File_E_NOENT    },  /* 206 */
        {  ERROR_NESTING_NOT_ALLOWED,    osl_File_E_AGAIN    },  /* 215 */
        {  ERROR_DIRECTORY,              osl_File_E_NOENT    },  /* 267 */
        {  ERROR_NOT_ENOUGH_QUOTA,       osl_File_E_NOMEM    },  /* 1816 */
        {  ERROR_UNEXP_NET_ERR,          osl_File_E_NETWORK  }   /* 59 */
    };

    /* The following two constants must be the minimum and maximum
    values in the (contiguous) range of osl_File_E_xec Failure errors. */
    #define MIN_EXEC_ERROR ERROR_INVALID_STARTING_CODESEG
    #define MAX_EXEC_ERROR ERROR_INFLOOP_IN_RELOC_CHAIN

    /* These are the low and high value in the range of errors that are
    access violations */
    #define MIN_EACCES_RANGE ERROR_WRITE_PROTECT
    #define MAX_EACCES_RANGE ERROR_SHARING_BUFFER_EXCEEDED

    //#####################################################
    oslFileError MapError(DWORD dwError)
    {
        for (int i = 0; i < ELEMENTS_OF_ARRAY(errtable); ++i )
        {
            if (dwError == errtable[i].oscode)
                return static_cast<oslFileError>(errtable[i].errnocode);
        }

        /* The error code wasn't in the table.  We check for a range of
        osl_File_E_ACCES errors or exec failure errors (ENOEXEC).
        Otherwise osl_File_E_INVAL is returned. */
        if ( dwError >= MIN_EACCES_RANGE && dwError <= MAX_EACCES_RANGE)
            return osl_File_E_ACCES;
        else if ( dwError >= MIN_EXEC_ERROR && dwError <= MAX_EXEC_ERROR)
            return osl_File_E_NOEXEC;
        else
            return osl_File_E_INVAL;
    }

    //#####################################################
    oslFileError SAL_CALL osl_openLocalRoot(
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
            _tcscpy( pDirImpl->szDirectoryPath, reinterpret_cast<LPCTSTR>(rtl_uString_getStr(strSysPath)) );

            /* Append backslash if neccessary */

            /* @@@ToDo
               use function ensure backslash
            */
            if ( pDirImpl->szDirectoryPath[_tcslen(pDirImpl->szDirectoryPath) - 1] != L'\\' )
                _tcscat( pDirImpl->szDirectoryPath, L"\\" );

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
                    rtl_freeMemory(pDirImpl);

                error = MapError( GetLastError() );
            }

            rtl_uString_release( strSysPath );
        }
        return error;
    }

    //#####################################################
    oslFileError SAL_CALL osl_openFileDirectory(
        rtl_uString *strDirectoryPath, oslDirectory *pDirectory)
    {
        // MT: Done in osl_openDirectory!
//      rtl_uString     *strSysPath = NULL;
        oslFileError    error;

        //MT: Not done in osl_openNetworkServer, why here?
        if ( !pDirectory )
            return osl_File_E_INVAL;

        *pDirectory = NULL;

        // MT: Done in osl_openDirectory!
//      error = _osl_getSystemPathFromFileURL( strDirectoryPath, &strSysPath, sal_False );
//      if ( osl_File_E_None == error )
        {
            Directory_Impl  *pDirImpl;

            pDirImpl = reinterpret_cast<Directory_Impl*>(rtl_allocateMemory(sizeof(Directory_Impl)));
            _tcscpy( pDirImpl->szDirectoryPath, reinterpret_cast<LPCTSTR>(rtl_uString_getStr(strDirectoryPath)) );

            /* Append backslash if neccessary */

            /* @@@ToDo
               use function ensure backslash
            */
            if ( pDirImpl->szDirectoryPath[_tcslen(pDirImpl->szDirectoryPath) - 1] != L'\\' )
                _tcscat( pDirImpl->szDirectoryPath, L"\\" );
            // MT: ???
            // GetCaseCorrectPathName( pDirImpl->szDirectoryPath, pDirImpl->szDirectoryPath, sizeof(pDirImpl->szDirectoryPath) );

            pDirImpl->uType = DIRECTORYTYPE_FILESYSTEM;
            pDirImpl->hDirectory = OpenDirectory( pDirImpl->szDirectoryPath );

            if ( pDirImpl->hDirectory )
            {
                *pDirectory = (oslDirectory)pDirImpl;
                error = osl_File_E_None;
            }
            else
            {
                if ( pDirImpl )
                    rtl_freeMemory(pDirImpl);

                error = MapError( GetLastError() );
            }

//          rtl_uString_release( strSysPath );
        }
        return error;
    }

    typedef struct tagDIRECTORY
    {
        HANDLE          hFind;
        WIN32_FIND_DATA aFirstData;
    } DIRECTORY, *PDIRECTORY, FAR *LPDIRECTORY;

    //#####################################################
    HANDLE WINAPI OpenDirectory(LPCTSTR lpszPath)
    {
        LPDIRECTORY pDirectory = (LPDIRECTORY)HeapAlloc(GetProcessHeap(), 0, sizeof(DIRECTORY));

        if (pDirectory)
        {
            TCHAR   szFileMask[MAX_PATH];
            int     nLen;

            _tcscpy( szFileMask, lpszPath );
            nLen = _tcslen( szFileMask );

            if (nLen && szFileMask[nLen-1] != '\\')
                _tcscat(szFileMask, TEXT("\\*.*"));
            else
                _tcscat(szFileMask, TEXT("*.*"));

            pDirectory->hFind = FindFirstFile(szFileMask, &pDirectory->aFirstData);

            if (!IsValidHandle(pDirectory->hFind))
            {
                HeapFree(GetProcessHeap(), 0, pDirectory);
                pDirectory = NULL;
            }
        }
        return (HANDLE)pDirectory;
    }

    //#####################################################
    BOOL WINAPI CloseDirectory(HANDLE hDirectory)
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
                else
                    fSuccess = FindNextFile( pDirectory->hFind, pFindData );

                fValid = fSuccess && _tcscmp( TEXT("."), pFindData->cFileName ) != 0 && _tcscmp( TEXT(".."), pFindData->cFileName ) != 0;

            } while( fSuccess && !fValid );
        }
        else
            SetLastError( ERROR_INVALID_HANDLE );

        return fSuccess;
    }

    //#####################################################
    oslFileError SAL_CALL osl_openNetworkServer(rtl_uString *strSysDirPath, oslDirectory *pDirectory)
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
            pDirImpl->uType = DIRECTORYTYPE_NETROOT;
            pDirImpl->hDirectory = hEnum;
            *pDirectory = (oslDirectory)pDirImpl;
        }
        return MapError( dwError );
    }

    //#####################################################
    oslFileError SAL_CALL osl_getNextNetResource(
        oslDirectory Directory,
        oslDirectoryItem *pItem,
        sal_uInt32 uHint )
    {
        Directory_Impl      *pDirImpl = (Directory_Impl *)Directory;
        DirectoryItem_Impl  *pItemImpl = NULL;
        BYTE                buffer[16384];
        LPNETRESOURCEW      lpNetResource = (LPNETRESOURCEW)buffer;
        DWORD               dwError, dwCount, dwBufSize;

        uHint = uHint; /* to get no warning */

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
            return MapError( dwError );
        }
    }

    //#####################################################
    oslFileError SAL_CALL osl_getNextDrive(
        oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 uHint )
    {
        Directory_Impl      *pDirImpl = (Directory_Impl *)Directory;
        DirectoryItem_Impl  *pItemImpl = NULL;
        BOOL                fSuccess;

        uHint = uHint; /* avoid warnings */

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
            rtl_freeMemory( pItemImpl );
            return MapError( GetLastError() );
        }
    }

    //#####################################################
    oslFileError SAL_CALL osl_getNextFileItem(
        oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 uHint)
    {
        Directory_Impl      *pDirImpl = (Directory_Impl *)Directory;
        DirectoryItem_Impl  *pItemImpl = NULL;
        BOOL                fFound;

        uHint = uHint; /* avoid warnings */

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
            _tcscpy( pItemImpl->szFullPath, pDirImpl->szDirectoryPath );
            _tcscat( pItemImpl->szFullPath, pItemImpl->FindData.cFileName );
            pItemImpl->bFullPathNormalized = FALSE;
            *pItem = (oslDirectoryItem)pItemImpl;
            return osl_File_E_None;
        }
        else
        {
            rtl_freeMemory( pItemImpl );
            return MapError( GetLastError() );
        }
    }

    //#####################################################
    oslFileError SAL_CALL osl_getDriveInfo(
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
            if ( pItemImpl->cDriveString[0] == '\\' &&
                pItemImpl->cDriveString[1] == '\\' )
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
                    TCHAR   szBuffer[1024];
                    DWORD   dwBufsize = ELEMENTS_OF_ARRAY(szBuffer);
                    DWORD   dwResult = WNetGetConnection( cDrive, szBuffer, &dwBufsize );

                    if ( NO_ERROR == dwResult )
                    {
                        TCHAR   szFileName[ELEMENTS_OF_ARRAY(szBuffer) + 16];

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
                    TCHAR   szVolumeNameBuffer[1024];

                    if ( GetVolumeInformation( cRoot, szVolumeNameBuffer, ELEMENTS_OF_ARRAY(szVolumeNameBuffer), NULL, NULL, NULL, NULL, 0 ) )
                    {
                        TCHAR   szFileName[ELEMENTS_OF_ARRAY(szVolumeNameBuffer) + 16];

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
    oslFileError SAL_CALL osl_getServerInfo(
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
            rtl_uString *ustrSystemPath = NULL;

            rtl_uString_newFromStr( &ustrSystemPath, reinterpret_cast<const sal_Unicode*>(pItemImpl->szFullPath) );
            osl_getFileURLFromSystemPath( ustrSystemPath, &pStatus->ustrFileURL );
            rtl_uString_release( ustrSystemPath );
            pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
        }
        return osl_File_E_None;
    }

    typedef struct tagDRIVEENUM {
        LPCTSTR lpIdent;
        TCHAR   cBuffer[/*('Z' - 'A' + 1) * sizeof("A:\\") + 1*/256];
        LPCTSTR lpCurrent;
    } DRIVEENUM, * PDRIVEENUM, FAR * LPDRIVEENUM;

    //#####################################################
    HANDLE WINAPI OpenLogicalDrivesEnum(void)
    {
        LPDRIVEENUM pEnum = NULL;

        pEnum = (LPDRIVEENUM)HeapAlloc( GetProcessHeap(), 0, sizeof(DRIVEENUM) );

        if ( pEnum )
        {
            DWORD   dwNumCopied = GetLogicalDriveStrings( (sizeof(pEnum->cBuffer) - 1) / sizeof(TCHAR), pEnum->cBuffer );

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
    BOOL WINAPI EnumLogicalDrives(HANDLE hEnum, LPTSTR lpBuffer)
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
    BOOL WINAPI CloseLogicalDrivesEnum(HANDLE hEnum)
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
    //Undocumented in SHELL32.DLL ordinal 35
    BOOL WINAPI PathRemoveFileSpec(LPTSTR lpPath)
    {
        BOOL    fSuccess = FALSE;   // Assume failure
        LPTSTR  lpLastBkSlash = _tcsrchr( lpPath, '\\' );
        LPTSTR  lpLastSlash = _tcsrchr( lpPath, '/' );
        LPTSTR  lpLastDelimiter = lpLastSlash > lpLastBkSlash ? lpLastSlash : lpLastBkSlash;

        if ( lpLastDelimiter )
        {
            if ( 0 == *(lpLastDelimiter + 1) )
            {
                if ( lpLastDelimiter > lpPath && *(lpLastDelimiter - 1) != ':' )
                {
                    *lpLastDelimiter = 0;
                    fSuccess = TRUE;
                }
            }
            else
            {
                *(++lpLastDelimiter) = 0;
                fSuccess = TRUE;
            }
        }
        return fSuccess;
    }

    //#####################################################
    // Undocumented in SHELL32.DLL ordinal 32
    LPTSTR WINAPI PathAddBackslash(LPTSTR lpPath)
    {
        LPTSTR  lpEndPath = NULL;

        if ( lpPath )
        {
            int     nLen = _tcslen(lpPath);

            if ( !nLen || lpPath[nLen-1] != '\\' && lpPath[nLen-1] != '/' && nLen < MAX_PATH - 1 )
            {
                lpEndPath = lpPath + nLen;
                *lpEndPath++ = '\\';
                *lpEndPath = 0;
            }
        }
        return lpEndPath;
    }

#if 1
    //#####################################################
    // Same as GetLongPathName but also 95/NT4
    DWORD WINAPI GetCaseCorrectPathNameEx(
        LPCTSTR lpszShortPath,  // file name
        LPTSTR  lpszLongPath,   // path buffer
        DWORD   cchBuffer,      // size of path buffer
        DWORD   nSkipLevels
    )
    {
//      log file doesn't work, because initialization of rtl log init() calls this method...
//      RTL_LOGFILE_TRACE1( "SAL: GetCaseCorrectPathNameEx: %s (Skip:%n)", lpszShortPath,nSkipLevels );

        TCHAR   szPath[MAX_PATH];
        BOOL    fSuccess;

        cchBuffer = cchBuffer; /* avoid warnings */

        _tcscpy( szPath, lpszShortPath );

        fSuccess = PathRemoveFileSpec( szPath );

        if ( fSuccess )
        {
            int nLen = _tcslen( szPath );
            LPCTSTR lpszFileSpec = lpszShortPath + nLen;
            BOOL    bSkipThis;

            if ( 0 == _tcscmp( lpszFileSpec, TEXT("..") ) )
            {
                bSkipThis = TRUE;
                nSkipLevels += 1;
            }
            else if (
                0 == _tcscmp( lpszFileSpec, TEXT(".") ) ||
                0 == _tcscmp( lpszFileSpec, TEXT("\\") ) ||
                0 == _tcscmp( lpszFileSpec, TEXT("/") )
                )
            {
                bSkipThis = TRUE;
            }
            else if ( nSkipLevels )
            {
                bSkipThis = TRUE;
                nSkipLevels--;
            }
            else
                bSkipThis = FALSE;

            GetCaseCorrectPathNameEx( szPath, szPath, MAX_PATH, nSkipLevels );

            PathAddBackslash( szPath );

            /* Analyze parent if not only a trailing backslash was cutted but a real file spec */
            if ( !bSkipThis )
            {
                WIN32_FIND_DATA aFindFileData;
                HANDLE  hFind = FindFirstFile( lpszShortPath, &aFindFileData );

                if ( IsValidHandle(hFind) )
                {
                    _tcscat( szPath, aFindFileData.cFileName[0] ? aFindFileData.cFileName : aFindFileData.cAlternateFileName );

                    FindClose( hFind );
                }
                else
                    return 0;
            }
        }
        else
        {
            /* File specification can't be removed therefore the short path is either a drive
               or a network share. If still levels to skip are left, the path specification
               tries to travel below the file system root */
            if ( nSkipLevels )
                return 0;

            _tcsupr( szPath );
        }

        _tcscpy( lpszLongPath, szPath );

        return _tcslen( lpszLongPath );
    }
#endif

#if 0
    inline size_t wcstoupper( LPWSTR lpStr )
    {
        size_t nLen = wcslen( lpStr );

        for ( LPWSTR p = lpStr; p < lpStr + nLen; p++ )
        {
            *p = towupper(*p);
        }

        return nLen;
    }

#endif

    //#####################################################
    DWORD WINAPI GetCaseCorrectPathName(
        LPCTSTR lpszShortPath,  // file name
        LPTSTR  lpszLongPath,   // path buffer
        DWORD   cchBuffer       // size of path buffer
    )
#if 0
    {
        /* Special handling for "\\.\" as system root */
        if ( lpszShortPath && 0 == wcscmp( lpszShortPath, WSTR_SYSTEM_ROOT_PATH ) )
        {
            if ( cchBuffer >= ELEMENTS_OF_ARRAY(WSTR_SYSTEM_ROOT_PATH) )
            {
                wcscpy( lpszLongPath, WSTR_SYSTEM_ROOT_PATH );
                return ELEMENTS_OF_ARRAY(WSTR_SYSTEM_ROOT_PATH) - 1;
            }
            else
                return ELEMENTS_OF_ARRAY(WSTR_SYSTEM_ROOT_PATH);
        }
        else
        {
            DWORD   nSrcLen = wcslen( lpszShortPath );

            if ( cchBuffer > nSrcLen )
            {
                wcscpy( lpszLongPath, lpszShortPath );
                wcstoupper( lpszLongPath );
            }
            else
                nSrcLen++;

            return nSrcLen;
        }
    }
#else
    {
        /* Special handling for "\\.\" as system root */
        if ( lpszShortPath && 0 == wcscmp( lpszShortPath, WSTR_SYSTEM_ROOT_PATH ) )
        {
            if ( cchBuffer >= ELEMENTS_OF_ARRAY(WSTR_SYSTEM_ROOT_PATH) )
            {
                wcscpy( lpszLongPath, WSTR_SYSTEM_ROOT_PATH );
                return ELEMENTS_OF_ARRAY(WSTR_SYSTEM_ROOT_PATH) - 1;
            }
            else
                return ELEMENTS_OF_ARRAY(WSTR_SYSTEM_ROOT_PATH) - 1;
        }
        else
            return GetCaseCorrectPathNameEx( lpszShortPath, lpszLongPath, cchBuffer, 0 );
    }

#endif

    //#####################################################
    #define CHARSET_SEPARATOR   TEXT("\\/")

    BOOL WINAPI IsValidFilePathComponent(
        LPCTSTR lpComponent, LPCTSTR *lppComponentEnd, DWORD dwFlags)
    {
        LPCTSTR lpComponentEnd = NULL;
        LPCTSTR lpCurrent = lpComponent;
        BOOL    fValid = TRUE;  /* Assume success */
        TCHAR   cLast = 0;

        /* Path component length must not exceed MAX_PATH */

        while ( !lpComponentEnd && lpCurrent && lpCurrent - lpComponent < MAX_PATH )
        {
            switch ( *lpCurrent )
            {
                /* Both backslash and slash determine the end of a path component */
            case '\0':
            case '/':
            case '\\':
                switch ( cLast )
                {
                    /* Component must not end with '.' or blank and can't be empty */

                case '.':
                    if ( dwFlags & VALIDATEPATH_ALLOW_ELLIPSE )
                    {
                        if ( 1 == lpCurrent - lpComponent )
                        {
                            /* Current directory is O.K. */
                            lpComponentEnd = lpCurrent;
                            break;
                        }
                        else if ( 2 == lpCurrent - lpComponent && '.' == *lpComponent )
                        {
                            /* Parent directory is O.K. */
                            lpComponentEnd = lpCurrent;
                            break;
                        }
                    }
                case 0:
                case ' ':
                    lpComponentEnd = lpCurrent - 1;
                    fValid = FALSE;
                    break;
                default:
                    lpComponentEnd = lpCurrent;
                    break;
                }
                break;
                /* '?' and '*' are valid wildcards but not valid file name characters */
            case '?':
            case '*':
                if ( dwFlags & VALIDATEPATH_ALLOW_WILDCARDS )
                    break;
                /* The following characters are reserved */
            case '<':
            case '>':
            case '\"':
            case '|':
            case ':':
                lpComponentEnd = lpCurrent;
                fValid = FALSE;
                break;
            default:
                /* Characters below ASCII 32 are not allowed */
                if ( *lpCurrent < ' ' )
                {
                    lpComponentEnd = lpCurrent;
                    fValid = FALSE;
                }
                break;
            }
            cLast = *lpCurrent++;
        }

        /*  If we don't reached the end of the component the length of the component was to long
            ( See condition of while loop ) */
        if ( !lpComponentEnd )
        {
            fValid = FALSE;
            lpComponentEnd = lpCurrent;
        }

        /* Test wether the component specifies a device name what is not allowed */

        // MT: PERFORMANCE:
        // This is very expensive. A lot of calls to _tcsicmp.
        // in SRC6870m71 67.000 calls of this method while empty office start result into more than 1.500.00 calls of _tcsicmp!
        // Possible optimizations
        // - Array should be const static
        // - Sorted array, use binary search
        // - More intelligent check for com1-9, lpt1-9
        // Maybe make szComponent upper case, don't search case intensitive
        // Talked to HRO: Could be removed. Shouldn't be used in OOo, and if used for something like a filename, it will lead to an error anyway.
        /*
        if ( fValid )
        {
            LPCTSTR alpDeviceNames[] =
            {
                TEXT("CON"),
                TEXT("PRN"),
                TEXT("AUX"),
                TEXT("CLOCK$"),
                TEXT("NUL"),
                TEXT("LPT1"),
                TEXT("LPT2"),
                TEXT("LPT3"),
                TEXT("LPT4"),
                TEXT("LPT5"),
                TEXT("LPT6"),
                TEXT("LPT7"),
                TEXT("LPT8"),
                TEXT("LPT9"),
                TEXT("COM1"),
                TEXT("COM2"),
                TEXT("COM3"),
                TEXT("COM4"),
                TEXT("COM5"),
                TEXT("COM6"),
                TEXT("COM7"),
                TEXT("COM8"),
                TEXT("COM9")
            };

            TCHAR   szComponent[MAX_PATH];
            int     nComponentLength;
            LPCTSTR lpDot;
            int     i;

            // A device name with an extension is also invalid
            lpDot = _tcschr( lpComponent, '.' );

            if ( !lpDot || lpDot > lpComponentEnd )
                nComponentLength = lpComponentEnd - lpComponent;
            else
                nComponentLength = lpDot - lpComponent;

            _tcsncpy( szComponent, lpComponent, nComponentLength );
            szComponent[nComponentLength] = 0;

            for ( i = 0; i < sizeof( alpDeviceNames ) / sizeof(LPCTSTR); i++ )
            {
                if ( 0 == _tcsicmp( szComponent, alpDeviceNames[i] ) )
                {
                    lpComponentEnd = lpComponent;
                    fValid = FALSE;
                    break;
                }
            }
        }
        */

        if ( fValid )
        {
            // Empty components are not allowed
            if ( lpComponentEnd - lpComponent < 1 )
                fValid = FALSE;

            // If we reached the end of the string NULL is returned
            else if ( !*lpComponentEnd )
                lpComponentEnd = NULL;

        }

        if ( lppComponentEnd )
            *lppComponentEnd = lpComponentEnd;

        return fValid;
    }

    //#####################################################
    DWORD WINAPI IsValidFilePath(rtl_uString *path, LPCTSTR *lppError, DWORD dwFlags, rtl_uString **corrected)
    {
        LPCTSTR lpszPath = reinterpret_cast< LPCTSTR >(path->buffer);
        LPCTSTR lpComponent;
        BOOL    fValid = TRUE;
        DWORD   dwPathType = PATHTYPE_ERROR;

        if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
            dwFlags |= VALIDATEPATH_ALLOW_ELLIPSE;

        if ( !lpszPath )
        {
            fValid = FALSE;
            lpComponent = lpszPath;
        }

        /* Test for UNC path notation */
        if ( 2 == _tcsspn( lpszPath, CHARSET_SEPARATOR ) )
        {
            /* Place the pointer behind the leading to backslashes */

            lpComponent = lpszPath + 2;

            fValid = IsValidFilePathComponent( lpComponent, &lpComponent, VALIDATEPATH_ALLOW_ELLIPSE );

            /* So far we have a valid servername. Now let's see if we also have a network resource */

            dwPathType = PATHTYPE_ABSOLUTE_UNC;

            if ( fValid )
            {
                if ( lpComponent &&  !*++lpComponent )
                    lpComponent = NULL;

                if ( !lpComponent )
                {
    #if 0
                    /* We only have a Server specification what is invalid */

                    lpComponent = lpszPath;
                    fValid = FALSE;
    #else
                    dwPathType |= PATHTYPE_IS_SERVER;
    #endif
                }
                else
                {
                    /* Now test the network resource */

                    fValid = IsValidFilePathComponent( lpComponent, &lpComponent, 0 );

                    /* If we now reached the end of the path, everything is O.K. */


                    if ( fValid && (!lpComponent || lpComponent && !*++lpComponent ) )
                    {
                        lpComponent = NULL;
                        dwPathType |= PATHTYPE_IS_VOLUME;
                    }
                }
            }
        }

        /* Local path verification. Must start with <drive>: */
        else if ( _istalpha( lpszPath[0] ) && ':' == lpszPath[1] )
        {
            /* Place pointer behind correct drive specification */

            lpComponent = lpszPath + 2;

            if ( 1 == _tcsspn( lpComponent, CHARSET_SEPARATOR ) )
                lpComponent++;
            else if ( *lpComponent )
                fValid = FALSE;

            dwPathType = PATHTYPE_ABSOLUTE_LOCAL;

            /* Now we are behind the backslash or it was a simple drive without backslash */

            if ( fValid && !*lpComponent )
            {
                lpComponent = NULL;
                dwPathType |= PATHTYPE_IS_VOLUME;
            }
        }

        /* Can be a relative path */
        else if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
        {
            lpComponent = lpszPath;

            /* Relative path can start with a backslash */

            if ( 1 == _tcsspn( lpComponent, CHARSET_SEPARATOR ) )
            {
                lpComponent++;
                if ( !*lpComponent )
                    lpComponent = NULL;
            }

            dwPathType = PATHTYPE_RELATIVE;
        }

        /* Anything else is an error */
        else
        {
            fValid = FALSE;
            lpComponent = lpszPath;
        }

        /* Now validate each component of the path */
        while ( fValid && lpComponent )
        {
            // Correct path by merging consecutive slashes:
            if (*lpComponent == '\\' && corrected != NULL) {
                sal_Int32 i = lpComponent - lpszPath;
                rtl_uString_newReplaceStrAt(corrected, path, i, 1, NULL);
                    //TODO: handle out-of-memory
                lpszPath = reinterpret_cast< LPCTSTR >((*corrected)->buffer);
                lpComponent = lpszPath + i;
            }

            fValid = IsValidFilePathComponent( lpComponent, &lpComponent, dwFlags );

            if ( fValid && lpComponent )
            {
                lpComponent++;

                /* If the string behind the backslash is empty, we've done */

                if ( !*lpComponent )
                    lpComponent = NULL;
            }
        }

        if ( fValid && _tcslen( lpszPath ) >= MAX_PATH )
        {
            fValid = FALSE;
            lpComponent = lpszPath + MAX_PATH;
        }

        if ( lppError )
            *lppError = lpComponent;

        return fValid ? dwPathType : PATHTYPE_ERROR;
    }

    //#####################################################
    bool is_floppy_drive(const rtl::OUString& path);

    //#####################################################
    struct Component
    {
        Component() :
            begin_(0), end_(0)
        {}

        bool isPresent() const
        { return (static_cast<sal_Int32>(end_ - begin_) > 0); }

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
    const wchar_t UNC_PREFIX[] = L"\\\\";
    const wchar_t BACKSLASH = '\\';
    const wchar_t SLASH = '/';

    bool is_UNC_path(const sal_Unicode* path)
    { return (0 == wcsncmp(UNC_PREFIX, reinterpret_cast<LPCWSTR>(path), ELEMENTS_OF_ARRAY(UNC_PREFIX) - 1)); }

    //#####################################################
    bool is_UNC_path(const rtl::OUString& path)
    { return is_UNC_path(path.getStr()); }

    //#####################################################
    void parse_UNC_path(const sal_Unicode* path, UNCComponents* puncc)
    {
        OSL_PRECOND(is_UNC_path(path), "Precondition violated: No UNC path");
        OSL_PRECOND(!wcschr(path, SLASH), "Path must not contain slashes");

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
    bool is_volume_mount_point(const rtl::OUString& path)
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

    //#####################################################
    // Has the given path a parent or are we already there,
    // e.g. 'c:\' or '\\server\share\'?
    bool has_path_parent(const sal_Unicode* path)
    {
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
    // @see bool has_path_parent(const sal_Unicode* path)
    bool has_path_parent(const rtl::OUString& path)
    { return has_path_parent(path.getStr()); }

    //#####################################################
    bool path_get_parent(rtl::OUString& path)
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

    //#############################################
    /*  Cut off the last part of the given path to
    get the parent only, e.g. 'c:\dir\subdir' ->
    'c:\dir' or '\\share\sub\dir' -> '\\share\sub'
    @return The position where the path has been cut
    off (this is the posistion of the last backslash).
    If there are no more parents 0 will be returned,
    e.g. 'c:\' or '\\Share' have no more parents */
    int path_make_parent(sal_Unicode* path)
    {
        OSL_PRECOND(!wcschr(path, SLASH), "Path must not contain slashes");
        OSL_PRECOND(has_path_parent(path), "Path must have a parent");

        sal_Unicode* pos_last_backslash = path + rtl_ustr_lastIndexOfChar(path, BACKSLASH);
        *pos_last_backslash = 0;
        return (pos_last_backslash - path);
    }

    //#####################################################
    void path_travel_to_volume_root(const rtl::OUString& system_path, rtl::OUString& volume_root)
    {
        rtl::OUString sys_path(system_path);

        while(!is_volume_mount_point(sys_path) && path_get_parent(sys_path))
            /**/;

        volume_root = sys_path;
        osl_systemPathEnsureSeparator(&volume_root.pData);
    }

    //#####################################################
    inline bool is_floppy_A_present()
    { return (GetLogicalDrives() & 1); }

    //#####################################################
    inline bool is_floppy_B_present()
    { return (GetLogicalDrives() & 2); }

    //#####################################################
    // determines if a volume mount point shows to a floppy
    // disk by comparing the unique volume names
    const LPWSTR FLOPPY_A = L"A:\\";
    const LPWSTR FLOPPY_B = L"B:\\";

    bool is_floppy_volume_mount_point(const rtl::OUString& path)
    {
        rtl::OUString p(path);
        osl_systemPathEnsureSeparator(&p.pData);

        TCHAR vn[51];
        if (GetVolumeNameForVolumeMountPoint(reinterpret_cast<LPCTSTR>(p.getStr()), vn, ELEMENTS_OF_ARRAY(vn)))
        {
            TCHAR vnfloppy[51];
            if (is_floppy_A_present() &&
                GetVolumeNameForVolumeMountPoint(FLOPPY_A, vnfloppy, ELEMENTS_OF_ARRAY(vnfloppy)) &&
                (0 == wcscmp(vn, vnfloppy)))
                return true;

            if (is_floppy_B_present() &&
                GetVolumeNameForVolumeMountPoint(FLOPPY_B, vnfloppy, ELEMENTS_OF_ARRAY(vnfloppy)) &&
                (0 == wcscmp(vn, vnfloppy)))
                return true;
        }
        return false;
    }

    //################################################
    // we must take into account that even a floppy
    // drive may be mounted to a directory so checking
    // for the drive letter alone is not sufficient
    // we must compare the unique volume name with
    // that of the available floppy disks
    LPCWSTR FLOPPY_DRV_LETTERS = TEXT("AaBb");

    bool is_floppy_drive(const rtl::OUString& path)
    {
        const sal_Unicode* pf = path.getStr();
        const sal_Unicode* ps = path.getStr() + 1;
        return ((wcschr(FLOPPY_DRV_LETTERS, *pf) && (L':' == *ps)) ||
                is_floppy_volume_mount_point(path));
    }

    //#############################################
    UINT get_volume_mount_point_drive_type(const rtl::OUString& path)
    {
        if (0 == path.getLength())
            return GetDriveType(NULL);

        rtl::OUString p(path);
        osl_systemPathEnsureSeparator(&p.pData);

        TCHAR vn[51];
        if (GetVolumeNameForVolumeMountPoint(reinterpret_cast<LPCTSTR>(p.getStr()), vn, ELEMENTS_OF_ARRAY(vn)))
            return GetDriveType(vn);

        return DRIVE_NO_ROOT_DIR;
    }

    //#############################################
    oslFileError osl_get_drive_type(const rtl::OUString& path, oslVolumeInfo* pInfo)
    {
        // GetDriveType fails on empty volume mount points
        // see Knowledge Base Q244089
        UINT drive_type;
        if (is_volume_mount_point(path))
            drive_type = get_volume_mount_point_drive_type(path);
        else
            drive_type = GetDriveType(reinterpret_cast<LPCTSTR>(path.getStr()));

        if (DRIVE_NO_ROOT_DIR == drive_type)
            return MapError(ERROR_INVALID_DRIVE);

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
    inline bool is_volume_space_info_request(sal_uInt32 field_mask)
    {
        return (field_mask &
                (osl_VolumeInfo_Mask_TotalSpace |
                 osl_VolumeInfo_Mask_UsedSpace  |
                 osl_VolumeInfo_Mask_FreeSpace));
    }

    //#############################################
    void get_volume_space_information(const rtl::OUString& path, oslVolumeInfo *pInfo)
    {
        BOOL ret = GetDiskFreeSpaceEx(
            reinterpret_cast<LPCTSTR>(path.getStr()),
            (PULARGE_INTEGER)&pInfo->uFreeSpace,
            (PULARGE_INTEGER)&pInfo->uTotalSpace,
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
    inline bool is_filesystem_attributes_request(sal_uInt32 field_mask)
    {
        return (field_mask &
                (osl_VolumeInfo_Mask_MaxNameLength |
                 osl_VolumeInfo_Mask_MaxPathLength |
                 osl_VolumeInfo_Mask_FileSystemName |
                 osl_VolumeInfo_Mask_FileSystemCaseHandling));
    }

    //#############################################
    inline bool is_drivetype_request(sal_uInt32 field_mask)
    {
        return (field_mask & osl_VolumeInfo_Mask_Attributes);
    }

    //#############################################
    oslFileError get_filesystem_attributes(const rtl::OUString& path, sal_uInt32 field_mask, oslVolumeInfo* pInfo)
    {
        pInfo->uAttributes = 0;

        oslFileError osl_error = osl_File_E_None;

        // osl_get_drive_type must be called first because
        // this function resets osl_VolumeInfo_Mask_Attributes
        // on failure
        if (is_drivetype_request(field_mask))
            osl_error = osl_get_drive_type(path, pInfo);

        if ((osl_File_E_None == osl_error) && is_filesystem_attributes_request(field_mask))
        {
            WCHAR vn[MAX_PATH];
            WCHAR fsn[MAX_PATH];
            DWORD serial;
            DWORD mcl;
            DWORD flags;

            if (GetVolumeInformation(reinterpret_cast<LPCTSTR>(path.getStr()), vn, MAX_PATH, &serial, &mcl, &flags, fsn, MAX_PATH))
            {
                pInfo->uValidFields   |= osl_VolumeInfo_Mask_MaxNameLength;
                pInfo->uMaxNameLength  = mcl;

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
        return osl_error;
    }

    //#############################################
    // Create the specified directory and call the
    // user specified callback function. On success
    // the function returns ERROR_SUCCESS else a
    // Win32 error code.
    DWORD create_dir_with_callback(
        sal_Unicode* dir_path,
        oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
        void* pData)
    {
        if (CreateDirectory(reinterpret_cast<LPCTSTR>(dir_path), NULL))
        {
            if (aDirectoryCreationCallbackFunc)
            {
                rtl::OUString url;
                FileBase::getFileURLFromSystemPath(dir_path, url);
                aDirectoryCreationCallbackFunc(pData, url.pData);
            }
            return ERROR_SUCCESS;
        }
        return GetLastError();
    }

    //#############################################
    DWORD create_dir_recursively_(
        sal_Unicode* dir_path,
        oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
        void* pData)
    {
        OSL_PRECOND(wcslen(dir_path) > 0 && (wcsrchr(dir_path, BACKSLASH) != (dir_path + (wcslen(dir_path) - 1))), \
        "Path must not end with a backslash");

        DWORD w32_error = create_dir_with_callback(
            dir_path, aDirectoryCreationCallbackFunc, pData);

        if (w32_error == ERROR_SUCCESS)
            return ERROR_SUCCESS;

        if ((w32_error != ERROR_PATH_NOT_FOUND) || !has_path_parent(dir_path))
            return w32_error;

        int pos = path_make_parent(dir_path);

        w32_error = create_dir_recursively_(
            dir_path, aDirectoryCreationCallbackFunc, pData);

        if (ERROR_SUCCESS != w32_error)
            return w32_error;

        dir_path[pos] = BACKSLASH;

        return create_dir_recursively_(
            dir_path, aDirectoryCreationCallbackFunc, pData);
    }

    //#####################################################
    // Temp file
    //#####################################################


    //#####################################################
    oslFileError osl_setup_base_directory_impl_(
        rtl_uString*  pustrDirectoryURL,
        rtl_uString** ppustr_base_dir)
    {
        rtl_uString* dir_url = 0;
        rtl_uString* dir     = 0;
        oslFileError error   = osl_File_E_None;

        if (pustrDirectoryURL)
            rtl_uString_assign(&dir_url, pustrDirectoryURL);
        else
            error = osl_getTempDirURL(&dir_url);

        if (osl_File_E_None == error)
        {
        error = _osl_getSystemPathFromFileURL(dir_url, &dir, sal_False);
        rtl_uString_release(dir_url);
        }

        if (osl_File_E_None == error )
        {
            rtl_uString_assign(ppustr_base_dir, dir);
            rtl_uString_release(dir);
        }

        return error;
    }

    //#####################################################
    oslFileError osl_setup_createTempFile_impl_(
        rtl_uString*   pustrDirectoryURL,
        oslFileHandle* pHandle,
        rtl_uString**  ppustrTempFileURL,
        rtl_uString**  ppustr_base_dir,
        sal_Bool*      b_delete_on_close)
    {
        oslFileError osl_error;

        OSL_PRECOND(((0 != pHandle) || (0 != ppustrTempFileURL)), "Invalid parameter!");

        if ((0 == pHandle) && (0 == ppustrTempFileURL))
        {
            osl_error = osl_File_E_INVAL;
        }
        else
        {
            osl_error = osl_setup_base_directory_impl_(
                pustrDirectoryURL, ppustr_base_dir);

            *b_delete_on_close = (sal_Bool)(0 == ppustrTempFileURL);
        }

        return osl_error;
    }

    //#####################################################
    oslFileError osl_win32_GetTempFileName_impl_(
        rtl_uString* base_directory, LPWSTR temp_file_name)
    {
        oslFileError osl_error = osl_File_E_None;

        if (0 == GetTempFileNameW(
                reinterpret_cast<LPCWSTR>(rtl_uString_getStr(base_directory)),
                L"",
                0,
                temp_file_name))
        {
            osl_error = MapError(GetLastError());
        }

        return osl_error;
    }

    //#####################################################
    sal_Bool osl_win32_CreateFile_impl_(
        LPCWSTR file_name, sal_Bool b_delete_on_close, oslFileHandle* p_handle)
    {
        DWORD  flags = FILE_ATTRIBUTE_NORMAL;
        HANDLE hFile;

        OSL_ASSERT(p_handle);

        if (b_delete_on_close)
            flags |= FILE_FLAG_DELETE_ON_CLOSE;

        hFile = CreateFileW(
            file_name,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            TRUNCATE_EXISTING,
            flags,
            NULL);

        if (IsValidHandle(hFile))
            *p_handle = (oslFileHandle)hFile;

        return (sal_Bool)IsValidHandle(hFile);
    }

    //#############################################
    oslFileError osl_createTempFile_impl_(
        rtl_uString*   base_directory,
        LPWSTR         tmp_name,
        sal_Bool       b_delete_on_close,
        oslFileHandle* pHandle,
        rtl_uString**  ppustrTempFileURL)
    {
        oslFileError osl_error;

        do
        {
            osl_error = osl_win32_GetTempFileName_impl_(base_directory, tmp_name);

            /*  if file could not be opened try again */

            if ((osl_File_E_None != osl_error) || (0 == pHandle) ||
                osl_win32_CreateFile_impl_(tmp_name, b_delete_on_close, pHandle))
                break;

        } while(1); // try until success

        if ((osl_File_E_None == osl_error) && !b_delete_on_close)
        {
            rtl_uString* pustr = 0;
            rtl_uString_newFromStr(&pustr, reinterpret_cast<const sal_Unicode*>(tmp_name));
            osl_getFileURLFromSystemPath(pustr, ppustrTempFileURL);
            rtl_uString_release(pustr);
        }

        return osl_error;
    }

    //#####################################################
    // End Temp file
    //#####################################################


    //#############################################
    sal_Bool _osl_decodeURL( rtl_String* strUTF8, rtl_uString** pstrDecodedURL )
    {
        sal_Char        *pBuffer;
        const sal_Char  *pSrcEnd;
        const sal_Char  *pSrc;
        sal_Char        *pDest;
        sal_Int32       nSrcLen;
        sal_Bool        bValidEncoded = sal_True;   /* Assume success */

        /* The resulting decoded string length is shorter or equal to the source length */

        nSrcLen = rtl_string_getLength(strUTF8);
        pBuffer = reinterpret_cast<sal_Char*>(rtl_allocateMemory(nSrcLen + 1));

        pDest = pBuffer;
        pSrc = rtl_string_getStr(strUTF8);
        pSrcEnd = pSrc + nSrcLen;

        /* Now decode the URL what should result in an UTF8 string */
        while ( bValidEncoded && pSrc < pSrcEnd )
        {
            switch ( *pSrc )
            {
            case '%':
                {
                    sal_Char    aToken[3];
                    sal_Char    aChar;

                    pSrc++;
                    aToken[0] = *pSrc++;
                    aToken[1] = *pSrc++;
                    aToken[2] = 0;

                    aChar = (sal_Char)strtoul( aToken, NULL, 16 );

                    /* The chars are path delimiters and must not be encoded */

                    if ( 0 == aChar || '\\' == aChar || '/' == aChar || ':' == aChar )
                        bValidEncoded = sal_False;
                    else
                        *pDest++ = aChar;
                }
                break;
            default:
                *pDest++ = *pSrc++;
                break;
            }
        }

        *pDest++ = 0;

        if ( bValidEncoded ) {
            rtl_string2UString( pstrDecodedURL, pBuffer, rtl_str_getLength(pBuffer), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );
            OSL_ASSERT(*pstrDecodedURL != 0);
        }

        rtl_freeMemory( pBuffer );

        return bValidEncoded;
    }

    //#############################################
    void _osl_encodeURL( rtl_uString *strURL, rtl_String **pstrEncodedURL )
    {
        /* Encode non ascii characters within the URL */

        rtl_String      *strUTF8 = NULL;
        sal_Char        *pszEncodedURL;
        const sal_Char  *pURLScan;
        sal_Char        *pURLDest;
        sal_Int32       nURLScanLen;
        sal_Int32       nURLScanCount;

        rtl_uString2String( &strUTF8, rtl_uString_getStr( strURL ), rtl_uString_getLength( strURL ), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );

        pszEncodedURL = (sal_Char*) rtl_allocateMemory( (rtl_string_getLength( strUTF8 ) * 3 + 1)  * sizeof(sal_Char) );

        pURLDest = pszEncodedURL;
        pURLScan = rtl_string_getStr( strUTF8 );
        nURLScanLen = rtl_string_getLength( strUTF8 );
        nURLScanCount = 0;

        while ( nURLScanCount < nURLScanLen )
        {
            sal_Char    cCurrent = *pURLScan;

            switch ( cCurrent )
            {
            default:
                if (!( ( cCurrent >= 'a' && cCurrent <= 'z' ) || ( cCurrent >= 'A' && cCurrent <= 'Z' ) || ( cCurrent >= '0' && cCurrent <= '9' ) ) )
                {
                    sprintf( pURLDest, "%%%02X", (unsigned char)cCurrent );
                    pURLDest += 3;
                    break;
                }
            case '!':
            case '\'':
            case '(':
            case ')':
            case '*':
            case '-':
            case '.':
            case '_':
            case '~':
            case '$':
            case '&':
            case '+':
            case ',':
            case '=':
            case '@':
            case ':':
            case '/':
            case '\\':
            case '|':
                *pURLDest++ = cCurrent;
                break;
            case 0:
                break;
            }

            pURLScan++;
            nURLScanCount++;
        }


        *pURLDest = 0;

        rtl_string_release( strUTF8 );
        rtl_string_newFromStr( pstrEncodedURL, pszEncodedURL );
        rtl_freeMemory( pszEncodedURL );
    }

    //#############################################
    oslFileError SAL_CALL _osl_getSystemPathFromFileURL( rtl_uString *strURL, rtl_uString **pustrPath, sal_Bool bAllowRelative )
    {
        rtl_String          *strUTF8 = NULL;
        rtl_uString         *strDecodedURL = NULL;
        rtl_uString         *strTempPath = NULL;
        const sal_Unicode   *pDecodedURL;
        sal_uInt32          nDecodedLen;
        sal_Bool            bValidEncoded;
        oslFileError        nError = osl_File_E_INVAL;  /* Assume failure */

        /*  If someone hasn't encoded the complete URL we convert it to UTF8 now to prevent from
            having a mixed encoded URL later */

        rtl_uString2String( &strUTF8, rtl_uString_getStr( strURL ), rtl_uString_getLength( strURL ), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );

        /* If the length of strUTF8 and strURL differs it indicates that the URL was not correct encoded */

        OSL_ENSURE_FILE(
            strUTF8->length == strURL->length ||
            0 != rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( strURL->buffer, strURL->length, "file:\\\\", 7 )
            ,"osl_getSystemPathFromFileURL: \"%s\" is not encoded !!!", strURL );

        bValidEncoded = _osl_decodeURL( strUTF8, &strDecodedURL );

        /* Release the encoded UTF8 string */

        rtl_string_release( strUTF8 );


        if ( bValidEncoded )
        {
            /* Replace backslashes and pipes */

            rtl_uString_newReplace( &strDecodedURL, strDecodedURL, '/', '\\' );
            rtl_uString_newReplace( &strDecodedURL, strDecodedURL, '|', ':' );

            pDecodedURL = rtl_uString_getStr( strDecodedURL );
            nDecodedLen = rtl_uString_getLength( strDecodedURL );

            /* Must start with "file://" */

            if ( 0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL, nDecodedLen, "file:\\\\", 7 ) )
            {
                sal_uInt32  nSkip;

                if ( 0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL, nDecodedLen, "file:\\\\\\", 8 ) )
                    nSkip = 8;
                else if (
                    0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL, nDecodedLen, "file:\\\\localhost\\", 17 ) ||
                    0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL, nDecodedLen, "file:\\\\127.0.0.1\\", 17 )
                )
                    nSkip = 17;
                else
                    nSkip = 5;

                /* Indicates local root */
                if ( nDecodedLen == nSkip )
                    rtl_uString_newFromStr_WithLength( &strTempPath, reinterpret_cast<const sal_Unicode*>(WSTR_SYSTEM_ROOT_PATH), ELEMENTS_OF_ARRAY(WSTR_SYSTEM_ROOT_PATH) - 1 );
                else
                    rtl_uString_newFromStr_WithLength( &strTempPath, pDecodedURL + nSkip, nDecodedLen - nSkip );

                if ( IsValidFilePath( strTempPath, NULL, VALIDATEPATH_ALLOW_ELLIPSE, &strTempPath ) )
                    nError = osl_File_E_None;
            }
            else if ( bAllowRelative )  /* This maybe a relative file URL */
            {
                rtl_uString_assign( &strTempPath, strDecodedURL );

                if ( IsValidFilePath( strTempPath, NULL, VALIDATEPATH_ALLOW_RELATIVE | VALIDATEPATH_ALLOW_ELLIPSE, &strTempPath ) )
                    nError = osl_File_E_None;
            }
        /*
            else
                OSL_ENSURE_FILE( !nError, "osl_getSystemPathFromFileURL: \"%s\" is not an absolute FileURL !!!", strURL );
         */

        }

        if ( strDecodedURL )
            rtl_uString_release( strDecodedURL );

        if ( osl_File_E_None == nError )
            rtl_uString_assign( pustrPath, strTempPath );

        if ( strTempPath )
            rtl_uString_release( strTempPath );

        /*
        OSL_ENSURE_FILE( !nError, "osl_getSystemPathFromFileURL: \"%s\" is not a FileURL !!!", strURL );
        */

        return nError;
    }

    //#############################################
    oslFileError SAL_CALL _osl_getFileURLFromSystemPath( rtl_uString* strPath, rtl_uString** pstrURL )
    {
        oslFileError nError = osl_File_E_INVAL; /* Assume failure */
        rtl_uString *strTempURL = NULL;
        DWORD dwPathType = PATHTYPE_ERROR;

        if (strPath)
            dwPathType = IsValidFilePath(strPath, NULL, VALIDATEPATH_ALLOW_RELATIVE, NULL);

        if (dwPathType)
        {
            rtl_uString *strTempPath = NULL;

            /* Replace backslashes */

            rtl_uString_newReplace( &strTempPath, strPath, '\\', '/' );

            switch ( dwPathType & PATHTYPE_MASK_TYPE )
            {
            case PATHTYPE_RELATIVE:
                rtl_uString_assign( &strTempURL, strTempPath );
                nError = osl_File_E_None;
                break;
            case PATHTYPE_ABSOLUTE_UNC:
                rtl_uString_newFromAscii( &strTempURL, "file:" );
                rtl_uString_newConcat( &strTempURL, strTempURL, strTempPath );
                nError = osl_File_E_None;
                break;
            case PATHTYPE_ABSOLUTE_LOCAL:
                rtl_uString_newFromAscii( &strTempURL, "file:///" );
                rtl_uString_newConcat( &strTempURL, strTempURL, strTempPath );
                nError = osl_File_E_None;
                break;
            default:
                break;
            }

            /* Release temp path */

            rtl_uString_release( strTempPath );
        }

        if ( osl_File_E_None == nError )
        {
            rtl_String  *strEncodedURL = NULL;

            /* Encode the URL */

            _osl_encodeURL( strTempURL, &strEncodedURL );

            /* Provide URL via unicode string */

            rtl_string2UString( pstrURL, rtl_string_getStr(strEncodedURL), rtl_string_getLength(strEncodedURL), RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );
            OSL_ASSERT(*pstrURL != 0);
            rtl_string_release( strEncodedURL );
        }

        /* Release temp URL */

        if ( strTempURL )
            rtl_uString_release( strTempURL );

        /*
        OSL_ENSURE_FILE( !nError, "osl_getFileURLFromSystemPath: \"%s\" is not a systemPath !!!", strPath );
        */

        return nError;
    }

#if OSL_DEBUG_LEVEL > 0

    //#####################################################
    void _osl_warnFile( const char *message, rtl_uString *ustrFile )
    {
        char szBuffer[2048];

        if (ustrFile)
        {
            rtl_String  *strFile = NULL;

            rtl_uString2String( &strFile, rtl_uString_getStr( ustrFile ), rtl_uString_getLength( ustrFile ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
            snprintf( szBuffer, sizeof(szBuffer), message, strFile->buffer );
            rtl_string_release( strFile );

            message = szBuffer;
        }
        OSL_ENSURE( 0, message );
    }

#endif // OSL_DEBUG_LEVEL > 0

} // end namespace private


//#####################################################
// Exported OSL API
//#####################################################


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

    systemPathRemoveSeparator(sys_path);

    // const_cast because sys_path is a local copy
    // which we want to modify inplace instead of
    // coyp it into another buffer on the heap again
    return MapError(create_dir_recursively_(
        sys_path.pData->buffer, aDirectoryCreationCallbackFunc, pData));
}

//#############################################
oslFileError SAL_CALL osl_createTempFile(
    rtl_uString*   pustrDirectoryURL,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL)
{
    rtl_uString*    base_directory = 0;
    LPWSTR          tmp_name;
    sal_Bool        b_delete_on_close;
    oslFileError    osl_error;

    osl_error = osl_setup_createTempFile_impl_(
        pustrDirectoryURL,
        pHandle,
        ppustrTempFileURL,
        &base_directory,
        &b_delete_on_close);

    if (osl_File_E_None != osl_error)
        return osl_error;

    /* allocate enough space on the stack */
    STACK_ALLOC(tmp_name, WCHAR, (rtl_uString_getLength(base_directory) + MAX_PATH));

    if (tmp_name)
    {
        osl_createTempFile_impl_(
            base_directory,
            tmp_name,
            b_delete_on_close,
            pHandle,
            ppustrTempFileURL);
    }
    else // stack alloc failed
    {
        osl_error = osl_File_E_NOMEM;
    }

    if (base_directory)
        rtl_uString_release(base_directory);

    return osl_error;
}

//#############################################
oslFileError SAL_CALL osl_getTempDirURL(rtl_uString** pustrTempDir)
{
    WCHAR   szBuffer[MAX_PATH];
    LPWSTR  lpBuffer = szBuffer;
    DWORD   nBufferLength = ELEMENTS_OF_ARRAY(szBuffer) - 1;

    DWORD           nLength;
    oslFileError    error;

    do
    {
        nLength = GetTempPathW( ELEMENTS_OF_ARRAY(szBuffer), lpBuffer );
        if ( nLength > nBufferLength )
        {
            nLength++;
            lpBuffer = reinterpret_cast<WCHAR*>(alloca( sizeof(WCHAR) * nLength ));
            nBufferLength = nLength - 1;
        }
    } while ( nLength > nBufferLength );

    if ( nLength )
    {
        rtl_uString *ustrTempPath = NULL;

        if ( '\\' == lpBuffer[nLength-1] )
            lpBuffer[nLength-1] = 0;

        rtl_uString_newFromStr( &ustrTempPath, reinterpret_cast<const sal_Unicode*>(lpBuffer) );

        error = osl_getFileURLFromSystemPath( ustrTempPath, pustrTempDir );

        rtl_uString_release( ustrTempPath );
    }
    else
        error = MapError( GetLastError() );

    return error;
}

//##################################################################
// File handling functions
//##################################################################


//#############################################
oslFileError SAL_CALL osl_openFile(
    rtl_uString *strPath, oslFileHandle *pHandle, sal_uInt32 uFlags )
{
    rtl_uString     *strSysPath = NULL;
    oslFileError    error = _osl_getSystemPathFromFileURL( strPath, &strSysPath, sal_False );

    if ( osl_File_E_None == error )
    {
        DWORD   dwAccess = 0, dwShare = FILE_SHARE_READ, dwCreation = 0, dwAttributes = 0;
        HANDLE  hFile;

        if ( uFlags & osl_File_OpenFlag_Read )
            dwAccess |= GENERIC_READ;

        if ( uFlags & osl_File_OpenFlag_Write )
            dwAccess |= GENERIC_WRITE;
        else
            dwShare  |= FILE_SHARE_WRITE;

        if ( uFlags & osl_File_OpenFlag_Create )
            dwCreation |= CREATE_NEW;
        else
            dwCreation |= OPEN_EXISTING;

        hFile = CreateFileW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( strSysPath )), dwAccess, dwShare, NULL, dwCreation, dwAttributes, NULL );

        *pHandle = osl_createFileHandleFromOSHandle( hFile );

        if ( !IsValidHandle( hFile ) )
            error = MapError( GetLastError() );

        rtl_uString_release( strSysPath );
    }

    return error;
}

//#############################################
oslFileError SAL_CALL osl_syncFile(oslFileHandle Handle)
{
    if (!IsValidHandle((HANDLE)Handle))
        return osl_File_E_INVAL;

    if (!FlushFileBuffers((HANDLE)Handle))
        return MapError(GetLastError());

    return osl_File_E_None;
}

//#############################################
oslFileError SAL_CALL osl_closeFile(oslFileHandle Handle)
{
    oslFileError    error;
    HANDLE          hFile = (HANDLE)Handle;

    if ( IsValidHandle(hFile) )
        error = CloseHandle( hFile ) ? osl_File_E_None : MapError( GetLastError() );
    else
        error = osl_File_E_INVAL;

    return error;
}

//#############################################
oslFileError SAL_CALL osl_isEndOfFile(oslFileHandle Handle, sal_Bool *pIsEOF)
{
    oslFileError    error = osl_File_E_INVAL;
    HANDLE          hFile = (HANDLE)Handle;

    if ( IsValidHandle(hFile) )
    {
        LONG        lDistanceToMove, lDistanceToMoveHigh;
        sal_uInt64  nCurPos;

        /*  Return value INVALID_SET_FILE_POINTER is no error indication and LastError could
            be set from previous IO call */

        SetLastError( NOERROR );

        lDistanceToMoveHigh = 0;
        lDistanceToMove = SetFilePointer( hFile, 0, &lDistanceToMoveHigh, FILE_CURRENT );

        error = MapError( GetLastError() );

        if ( osl_File_E_None == error )
        {
            nCurPos = (sal_uInt64)lDistanceToMove + ((sal_uInt64)lDistanceToMoveHigh << 32);

            lDistanceToMoveHigh = 0;
            lDistanceToMove = SetFilePointer( hFile, 0, &lDistanceToMoveHigh, FILE_END );

            error = MapError( GetLastError() );

            if ( osl_File_E_None == error )
            {
                sal_uInt64  nEndPos = (sal_uInt64)lDistanceToMove + ((sal_uInt64)lDistanceToMoveHigh << 32);

                *pIsEOF = (sal_Bool)(nEndPos == nCurPos);

                lDistanceToMoveHigh = (LONG)(nCurPos >> 32);
                SetFilePointer( hFile, (LONG)(nCurPos & 0xFFFFFFFF), &lDistanceToMoveHigh, FILE_BEGIN );

                error = MapError( GetLastError() );
            }
        }
    }
    return error;
}

//#############################################
oslFileError SAL_CALL osl_setFilePos(oslFileHandle Handle, sal_uInt32 uHow, sal_Int64 uPos)
{
    oslFileError    error;
    HANDLE          hFile = (HANDLE)Handle;

    if ( IsValidHandle(hFile) )
    {
        DWORD   dwMoveMethod;
        LONG    lDistanceToMove, lDistanceToMoveHigh;

        switch ( uHow )
        {
        case osl_Pos_Current:
            dwMoveMethod = FILE_CURRENT;
            break;
        case osl_Pos_End:
            dwMoveMethod = FILE_END;
            break;
        case osl_Pos_Absolut:
        default:
            dwMoveMethod = FILE_BEGIN;
            break;
        }

        lDistanceToMove = (LONG)(uPos & 0xFFFFFFFF);
        lDistanceToMoveHigh = (LONG)(uPos >> 32);


        SetLastError(0);
        SetFilePointer( hFile, lDistanceToMove, &lDistanceToMoveHigh, dwMoveMethod );

        error = MapError( GetLastError() );
    }
    else
        error = osl_File_E_INVAL;

    return error;
}

//#############################################
oslFileError SAL_CALL osl_getFilePos(oslFileHandle Handle, sal_uInt64 *pPos)
{
    oslFileError    error;
    HANDLE          hFile = (HANDLE)Handle;

    if ( IsValidHandle(hFile) )
    {
        LONG    lDistanceToMove, lDistanceToMoveHigh;

        /*  Return value INVALID_SET_FILE_POINTER is no error indication and LastError could
            be set from previous IO call */

        SetLastError( NOERROR );

        lDistanceToMoveHigh = 0;
        lDistanceToMove = SetFilePointer( hFile, 0, &lDistanceToMoveHigh, FILE_CURRENT );

        error = MapError( GetLastError() );

        if ( osl_File_E_None == error )
            *pPos = (sal_uInt64)lDistanceToMove + ((sal_uInt64)lDistanceToMoveHigh << 32);
    }
    else
        error = osl_File_E_INVAL;

    return error;
}

//#############################################
oslFileError SAL_CALL osl_getFileSize(oslFileHandle Handle, sal_uInt64 *pSize)
{
    HANDLE hFile = (HANDLE)Handle;
    if ( !IsValidHandle(hFile) )
        return osl_File_E_INVAL;

    DWORD nSize = GetFileSize(hFile, NULL);
    if (nSize == INVALID_FILE_SIZE)
    {
        DWORD nError = GetLastError();
        if (nError != NO_ERROR)
            return MapError(nError);
    }

    *pSize = (sal_uInt64)(nSize);
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_setFileSize(oslFileHandle Handle, sal_uInt64 uSize)
{
    oslFileError    error = error = osl_setFilePos( Handle, osl_Pos_Absolut, uSize );
    if ( osl_File_E_None == error )
    {
        if ( !SetEndOfFile( (HANDLE)Handle ) )
            error = MapError( osl_File_E_None );
    }

    return error;
}

//#############################################
oslFileError SAL_CALL osl_readFile(
    oslFileHandle Handle,
    void *pBuffer,
    sal_uInt64 uBytesRequested,
    sal_uInt64 *pBytesRead )
{
    oslFileError    error;
    HANDLE          hFile = (HANDLE)Handle;

    if ( IsValidHandle(hFile) )
    {
        DWORD   dwBytesToRead = (DWORD)uBytesRequested;
        DWORD   dwBytesRead;

        if ( ReadFile( hFile, pBuffer, dwBytesToRead, &dwBytesRead, NULL ) )
        {
            *pBytesRead = (sal_uInt64)dwBytesRead;
            error = osl_File_E_None;
        }
        else
            error = MapError( GetLastError() );
    }
    else
        error = osl_File_E_INVAL;

    return error;
}

//#############################################
oslFileError SAL_CALL osl_writeFile(
    oslFileHandle Handle,
    const void *pBuffer,
    sal_uInt64 uBytesToWrite,
    sal_uInt64 *pBytesWritten )
{
    oslFileError    error;
    HANDLE          hFile = (HANDLE)Handle;

    if ( IsValidHandle(hFile) )
    {
        DWORD   dwBytesToWrite = (DWORD)uBytesToWrite;
        DWORD   dwBytesWritten;

        if ( WriteFile( hFile, pBuffer, dwBytesToWrite, &dwBytesWritten, NULL ) )
        {
            *pBytesWritten = (sal_uInt64)dwBytesWritten;
            error = osl_File_E_None;
        }
        else
            error = MapError( GetLastError() );
    }
    else
        error = osl_File_E_INVAL;

    return error;
}

//#############################################
oslFileError SAL_CALL osl_removeFile( rtl_uString* strPath )
{
    rtl_uString *strSysPath = NULL;
    oslFileError    error = _osl_getSystemPathFromFileURL( strPath, &strSysPath, sal_False );

    if ( osl_File_E_None == error )
    {
        if ( DeleteFile( reinterpret_cast<LPCTSTR>(rtl_uString_getStr( strSysPath )) ) )
            error = osl_File_E_None;
        else
            error = MapError( GetLastError() );

        rtl_uString_release( strSysPath );
    }
    return error;
}

//#############################################
#define osl_File_CopyRecursive  0x0001
#define osl_File_CopyOverwrite  0x0002

oslFileError SAL_CALL osl_copyFile( rtl_uString* strPath, rtl_uString *strDestPath )
{
    rtl_uString *strSysPath = NULL, *strSysDestPath = NULL;
    oslFileError    error = _osl_getSystemPathFromFileURL( strPath, &strSysPath, sal_False );

    if ( osl_File_E_None == error )
        error = _osl_getSystemPathFromFileURL( strDestPath, &strSysDestPath, sal_False );

    if ( osl_File_E_None == error )
    {
        if ( CopyFile( reinterpret_cast<LPCTSTR>(rtl_uString_getStr( strSysPath )), reinterpret_cast<LPCTSTR>(rtl_uString_getStr( strSysDestPath )), FALSE ) )
            error = osl_File_E_None;
        else
            error = MapError( GetLastError() );
    }

    if ( strSysPath )
        rtl_uString_release( strSysPath );
    if ( strSysDestPath )
        rtl_uString_release( strSysDestPath );

    return error;
}

//#############################################
oslFileError SAL_CALL osl_moveFile( rtl_uString* strPath, rtl_uString *strDestPath )
{
    rtl_uString *strSysPath = NULL, *strSysDestPath = NULL;
    oslFileError    error = _osl_getSystemPathFromFileURL( strPath, &strSysPath, sal_False );

    if ( osl_File_E_None == error )
        error = _osl_getSystemPathFromFileURL( strDestPath, &strSysDestPath, sal_False );

    if ( osl_File_E_None == error )
    {
        if ( MoveFileEx( reinterpret_cast<LPCTSTR>(rtl_uString_getStr( strSysPath )), reinterpret_cast<LPCTSTR>(rtl_uString_getStr( strSysDestPath )), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING ) )
            error = osl_File_E_None;
        else
            error = MapError( GetLastError() );
    }

    if ( strSysPath )
        rtl_uString_release( strSysPath );
    if ( strSysDestPath )
        rtl_uString_release( strSysDestPath );

    return error;
}

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
        error = MapError( GetLastError() );

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

//#####################################################
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
        HANDLE  hFind = FindFirstFile( pItemImpl->szFullPath, &pItemImpl->FindData );

        if ( hFind != INVALID_HANDLE_VALUE )
            FindClose( hFind );
        else
            return MapError( GetLastError() );

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
        rtl_uString *ustrFullPath = NULL;

        rtl_uString_newFromStr( &ustrFullPath, reinterpret_cast<const sal_Unicode*>(pItemImpl->szFullPath) );
        osl_getFileURLFromSystemPath( ustrFullPath, &pStatus->ustrLinkTargetURL );
        rtl_uString_release( ustrFullPath );

        pStatus->uValidFields |= osl_FileStatus_Mask_LinkTargetURL;
    }

    if ( uFieldMask & osl_FileStatus_Mask_FileURL )
    {
        rtl_uString *ustrFullPath = NULL;


        if ( !pItemImpl->bFullPathNormalized )
        {
            GetCaseCorrectPathName( pItemImpl->szFullPath, pItemImpl->szFullPath, sizeof(pItemImpl->szFullPath) );
            pItemImpl->bFullPathNormalized = TRUE;
        }
        rtl_uString_newFromStr( &ustrFullPath, reinterpret_cast<const sal_Unicode*>(pItemImpl->szFullPath) );
        osl_getFileURLFromSystemPath( ustrFullPath, &pStatus->ustrFileURL );
        rtl_uString_release( ustrFullPath );
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }

    return osl_File_E_None;
}

//##################################################################
// directory handling functions
//##################################################################


//#####################################################
oslFileError SAL_CALL osl_createDirectory(rtl_uString* strPath)
{
    rtl_uString *strSysPath = NULL;
    oslFileError    error = _osl_getSystemPathFromFileURL( strPath, &strSysPath, sal_False );

    if ( osl_File_E_None == error )
    {
        if ( CreateDirectoryW( reinterpret_cast<LPCWSTR>(rtl_uString_getStr( strSysPath )), NULL ) )
            error = osl_File_E_None;
/*@@@ToDo
  The else case is a hack because the ucb or the webtop had some
  problems with the error code that CreateDirectory returns in
  case the path is only a logical drive, should be removed!
*/
        else
        {
            const sal_Unicode   *pBuffer = rtl_uString_getStr( strSysPath );
            sal_Int32           nLen = rtl_uString_getLength( strSysPath );

            if (
                ( pBuffer[0] >= 'A' && pBuffer[0] <= 'Z' ||
                  pBuffer[0] >= 'a' && pBuffer[0] <= 'z' ) &&
                pBuffer[1] == ':' && ( nLen ==2 || nLen == 3 && pBuffer[2] == '\\' )
                )
                SetLastError( ERROR_ALREADY_EXISTS );

            error = MapError( GetLastError() );
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
            error = MapError( GetLastError() );

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
//      WCHAR       szCorrectedPathName[MAX_PATH];
        DWORD       dwPathType;

        error = _osl_getSystemPathFromFileURL( strDirectoryPath, &strSysDirectoryPath, sal_False );

        if ( osl_File_E_None != error )
                return error;

        // MT Perform05
        /*
        if ( GetCaseCorrectPathName( strSysDirectoryPath->buffer, szCorrectedPathName, MAX_PATH ) )
        {
            rtl_uString_newFromStr( &strSysDirectoryPath, szCorrectedPathName );
        }
        */

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
oslFileError SAL_CALL osl_getNextDirectoryItem(oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 uHint)
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
            eError = CloseDirectory( pDirImpl->hDirectory ) ? osl_File_E_None : MapError( GetLastError() );
            break;
        case DIRECTORYTYPE_LOCALROOT:
            eError = CloseLogicalDrivesEnum( pDirImpl->hEnumDrives ) ? osl_File_E_None : MapError( GetLastError() );
            break;
        case DIRECTORYTYPE_NETROOT:
            {
                DWORD err = WNetCloseEnum(pDirImpl->hDirectory);
                eError = (err == NO_ERROR) ? osl_File_E_None : MapError(err);
            }
            break;
        default:
            OSL_ENSURE( 0, "Invalid directory type" );
            break;
        }

        rtl_freeMemory(pDirImpl);
    }
    return eError;
}

//#####################################################
oslFileError SAL_CALL osl_getDirectoryItem(rtl_uString *strFilePath, oslDirectoryItem *pItem)
{
    oslFileError    error = osl_File_E_None;
    rtl_uString*    strSysFilePath = NULL;
    PATHTYPE        type = PATHTYPE_FILE;
    DWORD           dwPathType;
//  TCHAR           szCorrectedPathName[MAX_PATH];

    /* Assume failure */

    if ( !pItem )
        return osl_File_E_INVAL;

    *pItem = NULL;


    error = _osl_getSystemPathFromFileURL( strFilePath, &strSysFilePath, sal_False );

    if ( osl_File_E_None != error )
            return error;

    // MT: I can't imagine a case where this is good for!
    /*
    if ( GetCaseCorrectPathName( strSysFilePath->buffer, szCorrectedPathName, MAX_PATH ) )
    {
        rtl_uString_newFromStr( &strSysFilePath, szCorrectedPathName );
    }
    */

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

                _tcscpy( pItemImpl->szFullPath, reinterpret_cast<LPCTSTR>(strSysFilePath->buffer) );

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
                pItemImpl->cDriveString[0] = _toupper( pItemImpl->cDriveString[0] );

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
                _tcscpy( pItemImpl->szFullPath, reinterpret_cast<LPCTSTR>(rtl_uString_getStr(strSysFilePath)) );

                // MT: This costs 600ms startup time on fast v60x!
                // GetCaseCorrectPathName( pItemImpl->szFullPath, pItemImpl->szFullPath, sizeof(pItemImpl->szFullPath) );

                pItemImpl->uType = DIRECTORYITEM_FILE;
                *pItem = pItemImpl;
                FindClose( hFind );
            }
            else
                error = MapError( GetLastError() );
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
        rtl_freeMemory( pItemImpl );
    return osl_File_E_None;
}

//#####################################################
oslFileError SAL_CALL osl_unmountVolumeDevice( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
        return osl_File_E_None;
    else
        return osl_File_E_INVAL;
}

//#####################################################
oslFileError SAL_CALL osl_automountVolumeDevice( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
        return osl_File_E_None;
    else
        return osl_File_E_INVAL;
}

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
// FileURL functions
//##################################################################


//#####################################################
oslFileError SAL_CALL osl_getFileURLFromSystemPath(
    rtl_uString* ustrPath, rtl_uString** pustrURL )
{
    return _osl_getFileURLFromSystemPath( ustrPath, pustrURL );
}

//#####################################################
oslFileError SAL_CALL osl_getSystemPathFromFileURL(
    rtl_uString *ustrURL, rtl_uString **pustrPath)
{
    return _osl_getSystemPathFromFileURL( ustrURL, pustrPath, sal_True );
}

//#####################################################
oslFileError SAL_CALL osl_searchFileURL(
    rtl_uString *ustrFileName,
    rtl_uString *ustrSystemSearchPath,
    rtl_uString **pustrPath)
{
    rtl_uString     *ustrUNCPath = NULL;
    rtl_uString     *ustrSysPath = NULL;
    oslFileError    error;

    /* First try to interpret the file name as an URL even a relative one */
    error = _osl_getSystemPathFromFileURL( ustrFileName, &ustrUNCPath, sal_True );

    /* So far we either have an UNC path or something invalid
       Now create a system path */
    if ( osl_File_E_None == error )
        error = _osl_getSystemPathFromFileURL( ustrUNCPath, &ustrSysPath, sal_True );

    if ( osl_File_E_None == error )
    {
        DWORD   nBufferLength;
        DWORD   dwResult;
        LPTSTR  lpBuffer = NULL;
        LPTSTR  lpszFilePart;

        /* Repeat calling SearchPath ...
           Start with MAX_PATH for the buffer. In most cases this
           will be enough and does not force the loop to runtwice */
        dwResult = MAX_PATH;

        do
        {
            /* If search path is empty use a NULL pointer instead according to MSDN documentation of SearchPath */
            LPCTSTR lpszSearchPath = ustrSystemSearchPath && ustrSystemSearchPath->length ? reinterpret_cast<LPCTSTR>(ustrSystemSearchPath->buffer) : NULL;
            LPCTSTR lpszSearchFile = reinterpret_cast<LPCTSTR>(ustrSysPath->buffer);

            /* Allocate space for buffer according to previous returned count of required chars */
            /* +1 is not neccessary if we follow MSDN documentation but for robustness we do so */
            nBufferLength = dwResult + 1;
            lpBuffer = lpBuffer ?
                reinterpret_cast<LPTSTR>(rtl_reallocateMemory(lpBuffer, nBufferLength * sizeof(TCHAR))) :
                reinterpret_cast<LPTSTR>(rtl_allocateMemory(nBufferLength * sizeof(TCHAR)));

            dwResult = SearchPath( lpszSearchPath, lpszSearchFile, NULL, nBufferLength, lpBuffer, &lpszFilePart );
        } while ( dwResult && dwResult >= nBufferLength );

        /*  ... until an error occures or buffer is large enough.
            dwResult == nBufferLength can not happen according to documentation but lets be robust ;-) */

        if ( dwResult )
        {
            rtl_uString_newFromStr( &ustrSysPath, reinterpret_cast<const sal_Unicode*>(lpBuffer) );
            error = osl_getFileURLFromSystemPath( ustrSysPath, pustrPath );
        }
        else
        {
            WIN32_FIND_DATA aFindFileData;
            HANDLE  hFind;

            /* Somthing went wrong, perhaps the path was absolute */
            error = MapError( GetLastError() );

            hFind = FindFirstFile( reinterpret_cast<LPCTSTR>(ustrSysPath->buffer), &aFindFileData );

            if ( IsValidHandle(hFind) )
            {
                error = osl_getFileURLFromSystemPath( ustrSysPath, pustrPath );
                FindClose( hFind );
            }
        }

        rtl_freeMemory( lpBuffer );
    }

    if ( ustrSysPath )
        rtl_uString_release( ustrSysPath );

    if ( ustrUNCPath )
        rtl_uString_release( ustrUNCPath );

    return error;
}

//#####################################################

oslFileError SAL_CALL osl_getAbsoluteFileURL( rtl_uString* ustrBaseURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL )
{
    oslFileError    eError;
    rtl_uString     *ustrRelSysPath = NULL;
    rtl_uString     *ustrBaseSysPath = NULL;

    if ( ustrBaseURL && ustrBaseURL->length )
    {
        eError = _osl_getSystemPathFromFileURL( ustrBaseURL, &ustrBaseSysPath, sal_False );
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with relative or invalid base URL" );

        eError = _osl_getSystemPathFromFileURL( ustrRelativeURL, &ustrRelSysPath, sal_True );
    }
    else
    {
        eError = _osl_getSystemPathFromFileURL( ustrRelativeURL, &ustrRelSysPath, sal_False );
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with empty base URL and/or invalid relative URL" );
    }

    if ( !eError )
    {
        TCHAR   szBuffer[MAX_PATH];
        TCHAR   szCurrentDir[MAX_PATH];
        LPTSTR  lpFilePart = NULL;
        DWORD   dwResult;

/*@@@ToDo
  Bad, bad hack, this only works if the base path
  really exists which is not necessary according
  to RFC2396
  The whole FileURL implementation should be merged
  with the rtl/uri class.
*/
        if ( ustrBaseSysPath )
        {
            osl_acquireMutex( g_CurrentDirectoryMutex );

            GetCurrentDirectory( MAX_PATH, szCurrentDir );
            SetCurrentDirectory( reinterpret_cast<LPCTSTR>(ustrBaseSysPath->buffer) );
        }

        dwResult = GetFullPathName( reinterpret_cast<LPCTSTR>(ustrRelSysPath->buffer), MAX_PATH, szBuffer, &lpFilePart );

        if ( ustrBaseSysPath )
        {
            SetCurrentDirectory( szCurrentDir );

            osl_releaseMutex( g_CurrentDirectoryMutex );
        }

        if ( dwResult )
        {
            if ( dwResult >= MAX_PATH )
                eError = osl_File_E_INVAL;
            else
            {
                rtl_uString *ustrAbsSysPath = NULL;

                rtl_uString_newFromStr( &ustrAbsSysPath, reinterpret_cast<const sal_Unicode*>(szBuffer) );

                eError = osl_getFileURLFromSystemPath( ustrAbsSysPath, pustrAbsoluteURL );

                if ( ustrAbsSysPath )
                    rtl_uString_release( ustrAbsSysPath );
            }
        }
        else
            eError = MapError( GetLastError() );
    }

    if ( ustrBaseSysPath )
        rtl_uString_release( ustrBaseSysPath );

    if ( ustrRelSysPath )
        rtl_uString_release( ustrRelSysPath );

    return  eError;
}

//#####################################################
oslFileError SAL_CALL osl_getCanonicalName( rtl_uString *strRequested, rtl_uString **strValid )
{
    rtl_uString_newFromString(strValid, strRequested);
    return osl_File_E_None;
}
