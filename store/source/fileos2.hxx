/*************************************************************************
 *
 *  $RCSfile: fileos2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:06:31 $
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

#ifndef INCLUDED_STORE_FILEOS2_HXX
#define INCLUDED_STORE_FILEOS2_HXX

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2def.h>
#include <bsedos.h>
#include <bseerr.h>

typedef HFILE HSTORE;

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
    { NO_ERROR,                   store_E_None             },
    { ERROR_FILE_NOT_FOUND,       store_E_NotExists        },
    { ERROR_PATH_NOT_FOUND,       store_E_NotExists        },
    { ERROR_ACCESS_DENIED,        store_E_AccessViolation  },
    { ERROR_SHARING_VIOLATION,    store_E_AccessViolation  },
    { ERROR_LOCK_VIOLATION,       store_E_LockingViolation },
    { ERROR_INVALID_ACCESS,       store_E_InvalidAccess    },
    { ERROR_INVALID_HANDLE,       store_E_InvalidHandle    },
    { ERROR_INVALID_PARAMETER,    store_E_InvalidParameter },
    { ERROR_FILENAME_EXCED_RANGE, store_E_NameTooLong      },
    { ERROR_TOO_MANY_OPEN_FILES,  store_E_NoMoreFiles      }
};

/*
 * __store_errno.
 */
inline sal_Int32 __store_errno (void)
{
    return (sal_Int32)errno;
}

/*
 * __store_malign (unsupported).
 */
inline sal_uInt32 __store_malign (void)
{
    return (sal_uInt32)(-1);
}

/*
 * __store_fmap (readonly, unsupported).
 */
inline HSTORE __store_fmap (HSTORE hFile)
{
    return ((HSTORE)0);
}

/*
 * __store_funmap.
 */
inline void __store_funmap (HSTORE hMap)
{
}

/*
 * __store_mmap (readonly, unsupported).
 */
inline sal_uInt8* __store_mmap (HSTORE h, sal_uInt32 k, sal_uInt32 n)
{
    return (sal_uInt8*)NULL;
}

/*
 * __store_munmap (unsupported).
 */
inline void __store_munmap (sal_uInt8 *p, sal_uInt32 n)
{
}

/*
 * __store_fopen.
 */
inline storeError __store_fopen (
    const sal_Char *pszName, sal_uInt32 nMode, HSTORE &rhFile)
{
    // Initialize [out] param.
    rhFile = 0;

    // Access mode.
    sal_uInt32 nAccessMode = OPEN_ACCESS_READONLY;
    if (nMode & store_File_OpenWrite)
        nAccessMode = OPEN_ACCESS_READWRITE;

    if (nAccessMode == OPEN_ACCESS_READONLY)
    {
        nMode |=  store_File_OpenNoCreate;
        nMode &= ~store_File_OpenTruncate;
    }

    // Share mode.
    sal_uInt32 nShareMode = OPEN_SHARE_DENYNONE;
    if (nMode & store_File_OpenWrite)
        nShareMode = OPEN_SHARE_DENYWRITE;

    // Open action.
    sal_uInt32 nOpenAction = 0, nDoneAction = 0;
    if (!(nMode & store_File_OpenNoCreate))
        nOpenAction = OPEN_ACTION_CREATE_IF_NEW; // Open always.
    else
        nOpenAction = OPEN_ACTION_FAIL_IF_NEW;   // Open existing.

    if (nMode & store_File_OpenTruncate)
        nOpenAction |= OPEN_ACTION_REPLACE_IF_EXISTS;
    else
        nOpenAction |= OPEN_ACTION_OPEN_IF_EXISTS;

    // Create file handle.
    APIRET result = ::DosOpen (
        pszName,
        &rhFile,
        &nDoneAction,
        0L,
        FILE_NORMAL,
        nOpenAction,
        nAccessMode | nShareMode | OPEN_FLAGS_NOINHERIT,
        0L);

    // Check result.
    if (result)
        return ERROR_FROM_NATIVE(result);
    else
        return store_E_None;
}

/*
 * __store_fread.
 */
inline storeError __store_fread (
    HSTORE h, sal_uInt32 offset, void *p, sal_uInt32 n, sal_uInt32 &k)
{
    APIRET result;
    if ((result = ::DosSetFilePtr (h, (long)offset, FILE_BEGIN, &k)) != 0)
        return ERROR_FROM_NATIVE(result);
    if ((result = ::DosRead (h, p, n, &k)) != 0)
        return ERROR_FROM_NATIVE(result);
    else
        return store_E_None;
}

/*
 * __store_fwrite.
 */
inline storeError __store_fwrite (
    HSTORE h, sal_uInt32 offset, const void *p, sal_uInt32 n, sal_uInt32 &k)
{
    APIRET result;
    if ((result = ::DosSetFilePtr (h, (long)offset, FILE_BEGIN, &k)) != 0)
        return ERROR_FROM_NATIVE(result);
    if ((result = ::DosWrite (h, (PVOID)p, n, &k)) != 0)
        return ERROR_FROM_NATIVE(result);
    else
        return store_E_None;
}

/*
 * __store_fseek.
 */
inline storeError __store_fseek (HSTORE h, sal_uInt32 n)
{
    sal_uInt32 k = 0;
    APIRET result = ::DosSetFilePtr (h, (long)n, FILE_BEGIN, &k);
    if (result)
        return ERROR_FROM_NATIVE(result);
    else
        return store_E_None;
}

/*
 * __store_fsize.
 */
inline storeError __store_fsize (HSTORE h, sal_uInt32 &k)
{
    APIRET result = ::DosSetFilePtr (h, 0L, FILE_END, &k);
    if (result)
        return ERROR_FROM_NATIVE(result);
    else
        return store_E_None;
}

/*
 * __store_ftrunc.
 */
inline storeError __store_ftrunc (HSTORE h, sal_uInt32 n)
{
    APIRET result = ::DosSetFileSize (h, n);
    if (result)
        return ERROR_FROM_NATIVE(result);
    else
        return store_E_None;
}

/*
 * __store_fsync.
 */
inline void __store_fsync (HSTORE h)
{
    ::DosResetBuffer (h);
}

/*
 * __store_fclose.
 */
inline void __store_fclose (HSTORE h)
{
    ::DosClose (h);
}

#endif /* INCLUDED_STORE_FILEOS2_HXX */
