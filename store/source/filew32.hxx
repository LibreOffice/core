/*************************************************************************
 *
 *  $RCSfile: filew32.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:06:34 $
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
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#ifndef INCLUDED_STORE_FILEW32_HXX
#define INCLUDED_STORE_FILEW32_HXX

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef HANDLE HSTORE;

/*========================================================================
 *
 * File I/O (inline) implementation.
 *
 *======================================================================*/
/*
 * __store_errcode_map.
 */
static const __store_errcode_mapping_st __store_errcode_map[] =
{
    { ERROR_SUCCESS,           store_E_None             },
    { ERROR_FILE_NOT_FOUND,    store_E_NotExists        },
    { ERROR_ACCESS_DENIED,     store_E_AccessViolation  },
    { ERROR_LOCK_FAILED,       store_E_LockingViolation },
    { ERROR_LOCK_VIOLATION,    store_E_LockingViolation },
    { ERROR_INVALID_HANDLE,    store_E_InvalidHandle    },
    { ERROR_INVALID_PARAMETER, store_E_InvalidParameter },
};

/*
 * __store_errno.
 */
inline sal_uInt32 __store_errno (void)
{
    return (sal_uInt32)::GetLastError();
}

/*
 * __store_malign.
 */
inline sal_uInt32 __store_malign (void)
{
    // Check Win32 platform.
    OSVERSIONINFO osinfo;

    osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ::GetVersionEx (&osinfo);

    if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        // Determine memory allocation granularity.
        SYSTEM_INFO info;

        ::GetSystemInfo (&info);
        return ((sal_uInt32)(info.dwAllocationGranularity));
    }
    return (sal_uInt32)(-1);
}

/*
 * __store_fmap (readonly).
 */
inline HSTORE __store_fmap (HSTORE hFile)
{
    return ::CreateFileMapping (
        hFile, NULL, SEC_COMMIT | PAGE_READONLY, 0, 0, NULL);
}

/*
 * __store_funmap.
 */
inline void __store_funmap (HSTORE hMap)
{
    ::CloseHandle (hMap);
}

/*
 * __store_mmap (readonly).
 */
inline sal_uInt8* __store_mmap (HSTORE h, sal_uInt32 k, sal_uInt32 n)
{
    return (sal_uInt8*)::MapViewOfFile (h, FILE_MAP_READ, 0, k, n);
}

/*
 * __store_munmap.
 */
inline void __store_munmap (sal_uInt8 *p, sal_uInt32 n)
{
    ::UnmapViewOfFile (p);
}

/*
 * __store_fopen.
 */
inline storeError __store_fopen (
    const sal_Char *pszName, sal_uInt32 nMode, HSTORE &rhFile)
{
    // Access mode.
    DWORD nAccessMode = GENERIC_READ;

    if (nMode & store_File_OpenWrite)
        nAccessMode |= GENERIC_WRITE;

    if (nAccessMode == GENERIC_READ)
        nMode |= store_File_OpenNoCreate;

    // Share mode.
    DWORD nShareMode = FILE_SHARE_READ;
    if (!(nMode & store_File_OpenWrite))
        nShareMode |= FILE_SHARE_WRITE;

    // Open action.
    DWORD nOpenAction = 0;
    if (!(nMode & store_File_OpenNoCreate))
    {
        // Open always.
        if (nMode & store_File_OpenTruncate)
            nOpenAction = CREATE_ALWAYS;
        else
            nOpenAction = OPEN_ALWAYS;
    }
    else
    {
        // Open existing.
        if (nMode & store_File_OpenTruncate)
            nOpenAction = TRUNCATE_EXISTING;
        else
            nOpenAction = OPEN_EXISTING;
    }

    // Create file handle.
    rhFile = ::CreateFile (
        pszName,
        nAccessMode,
        nShareMode,
        (LPSECURITY_ATTRIBUTES)NULL,
        nOpenAction,
        (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS),
        (HANDLE)NULL);

    // Check result and finish.
    if (rhFile == INVALID_HANDLE_VALUE)
    {
        rhFile = 0;
        return ERROR_FROM_NATIVE(::GetLastError());
    }
    return store_E_None;
}

/*
 * __store_fread.
 */
inline storeError __store_fread (
    HSTORE h, sal_uInt32 offset, void *p, sal_uInt32 n, sal_uInt32 &k)
{
    if (::SetFilePointer (h, offset, NULL, FILE_BEGIN) == (DWORD)(-1))
        return ERROR_FROM_NATIVE(::GetLastError());
    if (!::ReadFile (h, p, n, &k, NULL))
        return ERROR_FROM_NATIVE(::GetLastError());
    else
        return store_E_None;
}

/*
 * __store_fwrite.
 */
inline storeError __store_fwrite (
    HSTORE h, sal_uInt32 offset, const void *p, sal_uInt32 n, sal_uInt32 &k)
{
    if (::SetFilePointer (h, offset, NULL, FILE_BEGIN) == (DWORD)(-1))
        return ERROR_FROM_NATIVE(::GetLastError());
    if (!::WriteFile (h, p, n, &k, NULL))
        return ERROR_FROM_NATIVE(::GetLastError());
    else
        return store_E_None;
}

/*
 * __store_fseek.
 */
inline storeError __store_fseek (HSTORE h, sal_uInt32 n)
{
    DWORD k = ::SetFilePointer (h, n, NULL, FILE_BEGIN);
    if (k == (DWORD)(-1))
        return ERROR_FROM_NATIVE(::GetLastError());
    else
        return store_E_None;
}

/*
 * __store_fsize.
 */
inline storeError __store_fsize (HSTORE h, sal_uInt32 &k)
{
    k = (sal_uInt32)::GetFileSize (h, NULL);
    if (k == (sal_uInt32)(-1))
        return ERROR_FROM_NATIVE(::GetLastError());
    else
        return store_E_None;
}

/*
 * __store_ftrunc.
 */
inline storeError __store_ftrunc (HSTORE h, sal_uInt32 n)
{
    if (::SetFilePointer (h, n, NULL, FILE_BEGIN) == (DWORD)(-1))
        return ERROR_FROM_NATIVE(::GetLastError());
    if (!::SetEndOfFile (h))
        return ERROR_FROM_NATIVE(::GetLastError());
    else
        return store_E_None;
}

/*
 * __store_fsync.
 */
inline void __store_fsync (HSTORE h)
{
    ::FlushFileBuffers (h);
}

/*
 * __store_fclose.
 */
inline void __store_fclose (HSTORE h)
{
    ::CloseHandle (h);
}

#endif /* INCLUDED_STORE_FILEW32_HXX */
