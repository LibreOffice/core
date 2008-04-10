/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: advapi9x.h,v $
 * $Revision: 1.7 $
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
extern "C"{
#endif

//------------------------------------------------------------------------
// undefine the macros defined in the shlobj.h file in order to avoid
// warnings because of multiple defines
//------------------------------------------------------------------------

// begin obsolete Win32 API functions -->
#ifdef RegOpenKey
#undef RegOpenKey
#endif
#ifdef RegEnumKey
#undef RegEnumKey
#endif
#ifdef RegCreateKey
#undef RegCreateKey
#endif
#ifdef RegQueryValue
#undef RegQueryValue
#endif
#ifdef RegSetValue
#undef RegSetValue
#endif
// <-- end obsolete Win32 functions

#ifdef RegOpenKeyExW
#undef RegOpenKeyExW
#endif
#ifdef RegEnumKeyExW
#undef RegEnumKeyExW
#endif
#ifdef RegCreateKeyExW
#undef RegCreateKeyExW
#endif
#ifdef RegDeleteKeyW
#undef RegDeleteKeyW
#endif
#ifdef RegEnumValueW
#undef RegEnumValueW
#endif
#ifdef RegQueryValueExW
#undef RegQueryValueExW
#endif
#ifdef RegSetValueExW
#undef RegSetValueExW
#endif
#ifdef RegDeleteValueW
#undef RegDeleteValueW
#endif
#ifdef RegQueryInfoKeyW
#undef RegQueryInfoKeyW
#endif

//------------------------------------------------------------------------
// set the compiler directives for the function pointer we declare below
// if we build sal or sal will be used as static library we define extern
// else sal exports the function pointers from a dll and we use __declspec
//------------------------------------------------------------------------

#define ADVAPI9X_API extern

    //------------------------------------------------------------------------
// declare function pointers to the appropriate shell functions
//------------------------------------------------------------------------

ADVAPI9X_API LONG (WINAPI * lpfnRegOpenKeyExW) (
  HKEY hKey,         // handle to open key
  LPCWSTR lpSubKey,  // subkey name
  DWORD ulOptions,   // reserved
  REGSAM samDesired, // security access mask
  PHKEY phkResult    // handle to open key
);

ADVAPI9X_API LONG (WINAPI *lpfnRegEnumKeyExW) (
  HKEY hKey,                  // handle to key to enumerate
  DWORD dwIndex,              // subkey index
  LPWSTR lpName,              // subkey name
  LPDWORD lpcName,            // size of subkey buffer
  LPDWORD lpReserved,         // reserved
  LPWSTR lpClass,             // class string buffer
  LPDWORD lpcClass,           // size of class string buffer
  PFILETIME lpftLastWriteTime // last write time
);

ADVAPI9X_API LONG (WINAPI *lpfnRegCreateKeyExW)(
  HKEY hKey,                                  // handle to open key
  LPCWSTR lpSubKey,                           // subkey name
  DWORD Reserved,                             // reserved
  LPWSTR lpClass,                             // class string
  DWORD dwOptions,                            // special options
  REGSAM samDesired,                          // desired security access
  LPSECURITY_ATTRIBUTES lpSecurityAttributes, // inheritance
  PHKEY phkResult,                            // key handle
  LPDWORD lpdwDisposition                     // disposition value buffer
);

ADVAPI9X_API LONG (WINAPI *lpfnRegDeleteKeyW) (
  HKEY hKey,         // handle to open key
  LPCWSTR lpSubKey   // subkey name
);

ADVAPI9X_API LONG (WINAPI *lpfnRegEnumValueW) (
  HKEY hKey,             // handle to key to query
  DWORD dwIndex,         // index of value to query
  LPWSTR lpValueName,    // value buffer
  LPDWORD lpcValueName,  // size of value buffer
  LPDWORD lpReserved,    // reserved
  LPDWORD lpType,        // type buffer
  LPBYTE lpData,         // data buffer
  LPDWORD lpcbData       // size of data buffer
);

ADVAPI9X_API LONG (WINAPI *lpfnRegQueryValueExW) (
  HKEY hKey,            // handle to key
  LPCWSTR lpValueName,  // value name
  LPDWORD lpReserved,   // reserved
  LPDWORD lpType,       // type buffer
  LPBYTE lpData,        // data buffer
  LPDWORD lpcbData      // size of data buffer
);

ADVAPI9X_API LONG (WINAPI *lpfnRegSetValueExW)(
  HKEY hKey,           // handle to key
  LPCWSTR lpValueName, // value name
  DWORD Reserved,      // reserved
  DWORD dwType,        // value type
  CONST BYTE *lpData,  // value data
  DWORD cbData         // size of value data
);

ADVAPI9X_API LONG (WINAPI *lpfnRegDeleteValueW) (
  HKEY hKey,            // handle to key
  LPCWSTR lpValueName   // value name
);

ADVAPI9X_API LONG (WINAPI *lpfnRegQueryInfoKeyW) (
    HKEY hKey,                      // handle to key to query
    LPWSTR lpClassW,                // address of buffer for class string
    LPDWORD lpcbClass,              // address of size of class string buffer
    LPDWORD lpReserved,             // reserved
    LPDWORD lpcSubKeys,             // address of buffer for number of
                                    // subkeys
    LPDWORD lpcbMaxSubKeyLen,       // address of buffer for longest subkey
                                    // name length
    LPDWORD lpcbMaxClassLen,        // address of buffer for longest class
                                    // string length
    LPDWORD lpcValues,              // address of buffer for number of value
                                    // entries
    LPDWORD lpcbMaxValueNameLen,    // address of buffer for longest
                                    // value name length
    LPDWORD lpcbMaxValueLen,        // address of buffer for longest value
                                    // data length
    LPDWORD lpcbSecurityDescriptor, // address of buffer for security
                                    // descriptor length
    PFILETIME lpftLastWriteTime     // address of buffer for last write time
);

//------------------------------------------------------------------------
// redefine the above undefined macros so that the preprocessor replaces
// all occurrences of this macros with our function pointer
//------------------------------------------------------------------------

#define RegOpenKeyExW    lpfnRegOpenKeyExW
#define RegEnumKeyExW    lpfnRegEnumKeyExW
#define RegCreateKeyExW  lpfnRegCreateKeyExW
#define RegDeleteKeyW    lpfnRegDeleteKeyW
#define RegEnumValueW    lpfnRegEnumValueW
#define RegQueryValueExW lpfnRegQueryValueExW
#define RegSetValueExW   lpfnRegSetValueExW
#define RegDeleteValueW  lpfnRegDeleteValueW
#define RegQueryInfoKeyW lpfnRegQueryInfoKeyW

#ifdef __cplusplus
}
#endif
