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

#pragma once

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------
// undefine the macros defined in the winbase.h file in order to avoid
// warnings because of multiple defines
//------------------------------------------------------------------------

#ifdef LoadLibraryW
#undef LoadLibraryW
#endif

#ifdef LoadLibraryExW
#undef LoadLibraryExW
#endif

#ifdef GetModuleFileNameW
#undef GetModuleFileNameW
#endif

#ifdef GetLogicalDriveStringsW
#undef GetLogicalDriveStringsW
#endif

#ifdef DeleteFileW
#undef DeleteFileW
#endif

#ifdef CopyFileW
#undef CopyFileW
#endif

#ifdef MoveFileW
#undef MoveFileW
#endif

#ifdef MoveFileExW
#undef MoveFileExW
#endif

#ifdef CreateFileW
#undef CreateFileW
#endif

#ifdef RemoveDirectoryW
#undef RemoveDirectoryW
#endif

#ifdef CreateDirectoryW
#undef CreateDirectoryW
#endif

#ifdef CreateDirectoryExW
#undef CreateDirectoryExW
#endif

#ifdef CreateFileW
#undef CreateFileW
#endif

#ifdef GetLongPathNameW
#undef GetLongPathNameW
#endif

#ifdef GetLocaleInfoW
#undef GetLocaleInfoW
#endif

#ifdef GetFullPathNameW
#undef GetFullPathNameW
#endif

#ifdef CreateProcessW
#undef CreateProcessW
#endif

#ifdef CreateProcessAsUserW
#undef CreateProcessAsUserW
#endif

#ifdef GetEnvironmentVariableW
#undef GetEnvironmentVariableW
#endif

#ifdef GetDriveTypeW
#undef GetDriveTypeW
#endif

#ifdef GetCurrentDirectoryW
#undef GetCurrentDirectoryW
#endif

#ifdef SetCurrentDirectoryW
#undef SetCurrentDirectoryW
#endif

#ifdef GetVolumeInformationW
#undef GetVolumeInformationW
#endif


#ifdef GetDiskFreeSpaceExA
#undef GetDiskFreeSpaceExA
#endif

#ifdef GetDiskFreeSpaceExW
#undef GetDiskFreeSpaceExW
#endif

//------------------------------------------------------------------------
// set the compiler directives for the function pointer we declare below
// if we build sal or sal will be used as static library we define extern
// else sal exports the function pointers from a dll and we use __declspec
//------------------------------------------------------------------------

#define KERNEL9X_API extern

//------------------------------------------------------------------------
// declare function pointers to the appropriate kernel functions
//------------------------------------------------------------------------

//BOOL WINAPI RegisterServiceProcess( DWORD dwProcessID, BOOL fRegister );

KERNEL9X_API HMODULE (WINAPI *lpfnLoadLibraryExW ) (
    LPCWSTR lpLibFileName,  // file name of module
    HANDLE hFile,           // reserved, must be NULL
    DWORD dwFlags           // entry-point execution option
);

KERNEL9X_API DWORD (WINAPI *lpfnGetModuleFileNameW ) (
    HMODULE hModule,    // handle to module
    LPWSTR lpFilename,  // file name of module
    DWORD nSize         // size of buffer
);

KERNEL9X_API DWORD (WINAPI *lpfnGetLogicalDriveStringsW ) (
    DWORD nBufferLength,    // size of buffer
    LPWSTR lpBuffer         // drive strings buffer
);

KERNEL9X_API HANDLE ( WINAPI *lpfnCreateFileW )(
    LPCWSTR lpFileName,
    DWORD   dwDesiredAccess,
    DWORD   dwShareMode,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD   dwCreationDisposition,
    DWORD   dwFlagsAndAttributes,
    HANDLE  hTemplateFile
);

KERNEL9X_API DWORD WINAPI GetCanonicalPathNameA(
    LPCSTR lpszPath,            // file name
    LPSTR lpszCanonicalPath,    // path buffer
    DWORD cchBuffer             // size of path buffer
);

KERNEL9X_API DWORD WINAPI GetCanonicalPathNameW(
    LPCWSTR lpszPath,           // file name
    LPWSTR lpszCanonicalPath,   // path buffer
    DWORD cchBuffer             // size of path buffer
);

KERNEL9X_API HANDLE ( WINAPI * lpfnCreateFileW ) (
    LPCWSTR lpFileName,
    DWORD   dwDesiredAccess,
    DWORD   dwShareMode,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD   dwCreationDisposition,
    DWORD   dwFlagsAndAttributes,
    HANDLE  hTemplateFile );

KERNEL9X_API BOOL (WINAPI *lpfnDeleteFileW ) (
    LPCWSTR lpFileName          // file name
);

KERNEL9X_API BOOL (WINAPI *lpfnCopyFileW ) (
    LPCWSTR lpExistingFileName, // file name
    LPCWSTR lpNewFileName,      // new file name
    BOOL    bFailIfExist        // operation if file exists
);

KERNEL9X_API BOOL (WINAPI *lpfnMoveFileW ) (
    LPCWSTR lpExistingFileName, // file name
    LPCWSTR lpNewFileName       // new file name
);

KERNEL9X_API BOOL (WINAPI *lpfnMoveFileExW ) (
  LPCWSTR lpExistingFileName,   // file name
  LPCWSTR lpNewFileName,        // new file name
  DWORD dwFlags                 // move options
);

KERNEL9X_API BOOL (WINAPI *lpfnRemoveDirectoryW ) (
    LPCWSTR lpPathName          // directory name
);

KERNEL9X_API BOOL ( WINAPI * lpfnCreateDirectoryW ) (
    LPCWSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes );

KERNEL9X_API BOOL ( WINAPI * lpfnCreateDirectoryExW ) (
    LPCWSTR lpTemplateDirectory,
    LPCWSTR lpNewDirectory,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes );

KERNEL9X_API DWORD ( WINAPI * lpfnGetLongPathNameW ) (
  LPCWSTR lpszShortPath, // file name
  LPWSTR lpszLongPath,   // path buffer
  DWORD cchBuffer        // size of path buffer
);

// GetCanonicalPath is a tool function with no exact counterpart
// in the win32 api; we use nevertheless a function pointer
// because every variable etc. must root in the Kernel9x.lib else
// we loose our AutoSystoolInit object during linking
KERNEL9X_API DWORD ( WINAPI * lpfnGetCanonicalPathW ) (
    LPCWSTR lpszPath,           // file name
    LPWSTR lpszCanonicalPath,   // path buffer
    DWORD cchBuffer             // size of path buffer
);

KERNEL9X_API int ( WINAPI* lpfnGetLocaleInfoW ) (
    LCID Locale,        // locale identifier
    LCTYPE LCType,      // information type
    LPWSTR lpLCData,    // information buffer
    int cchData         // size of buffer
);

KERNEL9X_API DWORD ( WINAPI * lpfnGetFullPathNameW )(
    LPCWSTR lpFileName,     // file name
    DWORD   nBufferLength,  // size of path buffer
    LPWSTR  lpBuffer,       // path buffer
    LPWSTR  *lpFilePart     // address of file name in path
);

KERNEL9X_API BOOL ( WINAPI * lpfnCreateProcessW )(
    LPCWSTR lpApplicationName,                 // name of executable module
    LPWSTR lpCommandLine,                      // command line string
    LPSECURITY_ATTRIBUTES lpProcessAttributes, // SD
    LPSECURITY_ATTRIBUTES lpThreadAttributes,  // SD
    BOOL bInheritHandles,                      // handle inheritance option
    DWORD dwCreationFlags,                     // creation flags
    LPVOID lpEnvironment,                      // new environment block
    LPCWSTR lpCurrentDirectory,                // current directory name
    LPSTARTUPINFOW lpStartupInfo,              // startup information
    LPPROCESS_INFORMATION lpProcessInformation // process information
);

KERNEL9X_API BOOL ( WINAPI * lpfnCreateProcessAsUserW )(
    HANDLE hToken,                             // handle to user token
    LPCWSTR lpApplicationName,                 // name of executable module
    LPWSTR lpCommandLine,                      // command-line string
    LPSECURITY_ATTRIBUTES lpProcessAttributes, // SD
    LPSECURITY_ATTRIBUTES lpThreadAttributes,  // SD
    BOOL bInheritHandles,                      // inheritance option
    DWORD dwCreationFlags,                     // creation flags
    LPVOID lpEnvironment,                      // new environment block
    LPCWSTR lpCurrentDirectory,                // current directory name
    LPSTARTUPINFOW lpStartupInfo,              // startup information
    LPPROCESS_INFORMATION lpProcessInformation // process information
);

KERNEL9X_API DWORD ( WINAPI * lpfnGetEnvironmentVariableW )(
    LPCWSTR lpName,  // environment variable name
    LPWSTR lpBuffer, // buffer for variable value
    DWORD nSize      // size of buffer
);


KERNEL9X_API UINT ( WINAPI * lpfnGetDriveTypeW )(
    LPCWSTR lpRootPathName  // root directory
);

KERNEL9X_API DWORD ( WINAPI * lpfnGetCurrentDirectoryW )(
    DWORD  nBufferLength,   // size of directory buffer
    LPWSTR lpBuffer         // directory buffer
);

KERNEL9X_API BOOL ( WINAPI * lpfnSetCurrentDirectoryW )(
    LPCWSTR lpPathName  // new directory name
);

// GetVolumeInformation
KERNEL9X_API BOOL ( WINAPI* lpfnGetVolumeInformationW )(
    LPCWSTR lpRootPathName,             // root directory
    LPWSTR  lpVolumeNameBuffer,         // volume name buffer
    DWORD   nVolumeNameSize,            // length of name buffer
    LPDWORD lpVolumeSerialNumber,       // volume serial number
    LPDWORD lpMaximumComponentLength,   // maximum file name length
    LPDWORD lpFileSystemFlags,          // file system options
    LPWSTR  lpFileSystemName,           // file system name buffer
    DWORD   nFileSystemNameSize         // length of file system name buffer
);

// GetDiskFreeSpaceExA
KERNEL9X_API BOOL (WINAPI *lpfnGetDiskFreeSpaceExA)(
  LPCSTR lpDirectoryName,                  // directory name
  PULARGE_INTEGER lpFreeBytesAvailable,    // bytes available to caller
  PULARGE_INTEGER lpTotalNumberOfBytes,    // bytes on disk
  PULARGE_INTEGER lpTotalNumberOfFreeBytes // free bytes on disk
);

// GetDiskFreeSpaceExW
KERNEL9X_API BOOL (WINAPI *lpfnGetDiskFreeSpaceExW)(
  LPCWSTR lpDirectoryName,                 // directory name
  PULARGE_INTEGER lpFreeBytesAvailable,    // bytes available to caller
  PULARGE_INTEGER lpTotalNumberOfBytes,    // bytes on disk
  PULARGE_INTEGER lpTotalNumberOfFreeBytes // free bytes on disk
);

//------------------------------------------------------------------------
// redefine the above undefined macros so that the preprocessor replaces
// all occurrences of this macros with our function pointer
//------------------------------------------------------------------------

#define LoadLibraryExW              lpfnLoadLibraryExW
#define LoadLibraryW(c)             LoadLibraryExW(c, NULL, 0)
#define GetModuleFileNameW          lpfnGetModuleFileNameW
#define GetLogicalDriveStringsW     lpfnGetLogicalDriveStringsW
#define CreateFileW                 lpfnCreateFileW
#define DeleteFileW                 lpfnDeleteFileW
#define CopyFileW                   lpfnCopyFileW
#define MoveFileW                   lpfnMoveFileW
#define MoveFileExW                 lpfnMoveFileExW
#define RemoveDirectoryW            lpfnRemoveDirectoryW
#define CreateDirectoryW            lpfnCreateDirectoryW
#define CreateDirectoryExW          lpfnCreateDirectoryExW
#define GetLongPathNameW            lpfnGetLongPathNameW
#define GetFullPathNameW            lpfnGetFullPathNameW

#define GetCanonicalPath            lpfnGetCanonicalPathW
#define GetLocaleInfoW              lpfnGetLocaleInfoW

#define CreateProcessW              lpfnCreateProcessW
#define CreateProcessAsUserW        lpfnCreateProcessAsUserW
#define GetEnvironmentVariableW     lpfnGetEnvironmentVariableW
#define GetDriveTypeW               lpfnGetDriveTypeW

#define GetCurrentDirectoryW        lpfnGetCurrentDirectoryW
#define SetCurrentDirectoryW        lpfnSetCurrentDirectoryW

#define GetVolumeInformationW       lpfnGetVolumeInformationW
#define GetDiskFreeSpaceExA         lpfnGetDiskFreeSpaceExA
#define GetDiskFreeSpaceExW         lpfnGetDiskFreeSpaceExW

#ifdef __cplusplus
}
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
