/*************************************************************************
 *
 *  $RCSfile: kernel9x.h,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: tra $ $Date: 2001-07-17 07:21:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifdef __cplusplus
}
#endif


