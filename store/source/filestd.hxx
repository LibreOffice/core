/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filestd.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_STORE_FILESTD_HXX
#define INCLUDED_STORE_FILESTD_HXX

#ifndef INCLUDED_ERRNO_H
#include <errno.h>
#define INCLUDED_ERRNO_H
#endif

#ifndef INCLUDED_STDIO_H
#include <stdio.h>
#define INCLUDED_STDIO_H
#endif

typedef FILE* HSTORE;

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
    { 0,         store_E_None             },
    { ENOENT,    store_E_NotExists        },
    { EACCES,    store_E_AccessViolation  },
    { EPERM,     store_E_AccessViolation  },
    { EAGAIN,    store_E_LockingViolation },
    { EDEADLOCK, store_E_LockingViolation },
    { EBADF,     store_E_InvalidHandle    },
    { EINVAL,    store_E_InvalidParameter },
    { ENOSPC,    store_E_OutOfSpace       },
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
    // Access mode.
    if (!(nMode & store_File_OpenWrite))
    {
        nMode |= store_File_OpenNoCreate;
        nMode &= ~store_File_OpenTruncate;
    }

    // Create file handle.
    if (nMode & store_File_OpenTruncate)
    {
        // Create always, truncate existing.
        rhFile = fopen (pszName, "wb+");
    }
    else if (nMode & store_File_OpenWrite)
    {
        // Open existing (rw).
        rhFile = fopen (pszName, "rb+");
        if (!(rhFile || (nMode & store_File_OpenNoCreate)))
        {
            // Try create (rw).
            rhFile = fopen (pszName, "wb+");
        }
    }
    else
    {
        // Open existing (ro).
        rhFile = fopen (pszName, "rb");
    }

    // Check result.
    if (!rhFile)
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_fread.
 */
inline storeError __store_fread (
    HSTORE h, sal_uInt32 offset, void *p, sal_uInt32 n, sal_uInt32 &k)
{
    if (::fseek (h, (long)offset, SEEK_SET) < 0)
        return ERROR_FROM_NATIVE(errno);

    k = (sal_uInt32)::fread (p, (size_t)1, (size_t)n, h);
    if (k == (sal_uInt32)(-1))
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_fwrite.
 */
inline storeError __store_fwrite (
    HSTORE h, sal_uInt32 offset, const void *p, sal_uInt32 n, sal_uInt32 &k)
{
    if (::fseek (h, (long)offset, SEEK_SET) < 0)
        return ERROR_FROM_NATIVE(errno);

    k = (sal_uInt32)::fwrite (p, (size_t)1, (size_t)n, h);
    if (k == (sal_uInt32)(-1))
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_fseek.
 */
inline storeError __store_fseek (HSTORE h, sal_uInt32 n)
{
    if (::fseek (h, (long)n, SEEK_SET) < 0)
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_fsize.
 */
inline storeError __store_fsize (HSTORE h, sal_uInt32 &k)
{
    if (::fseek (h, 0, SEEK_END) < 0)
        return ERROR_FROM_NATIVE(errno);
    k = (sal_uInt32)::ftell (h);
    if (k == (sal_uInt32)(-1))
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_ftrunc (unsupported).
 */
inline storeError __store_ftrunc (HSTORE h, sal_uInt32 n)
{
    return store_E_None;
}

/*
 * __store_fsync.
 */
inline storeError __store_fsync (HSTORE h)
{
    if (::fflush (h) < 0)
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_fclose.
 */
inline storeError __store_fclose (HSTORE h)
{
    if (::fclose (h) < 0)
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

#endif /* INCLUDED_STORE_FILESTD_HXX */
