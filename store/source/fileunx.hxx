/*************************************************************************
 *
 *  $RCSfile: fileunx.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 14:47:42 $
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

#ifndef INCLUDED_STORE_FILEUNX_HXX
#define INCLUDED_STORE_FILEUNX_HXX

#define _USE_UNIX98 /* _XOPEN_SOURCE=500 */
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(FREEBSD) || defined(NETBSD) || defined(MACOSX)
#define EDEADLOCK EDEADLK
#endif /* FREEBSD || NETBSD || MACOSX */

typedef int HSTORE;

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
#if defined(EDEADLOCK)
    { EDEADLOCK, store_E_LockingViolation },
#endif /* EDEADLOCK */
    { EBADF,     store_E_InvalidHandle    },
    { EINVAL,    store_E_InvalidParameter },
};

/*
 * __store_errno.
 */
inline sal_Int32 __store_errno (void)
{
    return (sal_Int32)errno;
}

/*
 * __store_malign.
 */
#if   defined(FREEBSD) || defined(LINUX) || defined(MACOSX)
inline sal_uInt32 __store_malign (void)
{
    return (sal_uInt32)::getpagesize();
}
#elif defined(IRIX) || defined(SOLARIS)
inline sal_uInt32 __store_malign (void)
{
    return (sal_uInt32)::sysconf (_SC_PAGESIZE);
}
#else
inline sal_uInt32 __store_malign (void)
{
    return (sal_uInt32)(-1);
}
#endif /* FREEBSD || IRIX || LINUX || SOLARIS || MACOSX*/

/*
 * __store_fmap (readonly).
 */
inline HSTORE __store_fmap (HSTORE hFile)
{
    // Avoid hMap = dup (hFile); may result in EMFILE.
    return hFile;
}

/*
 * __store_funmap.
 */
inline void __store_funmap (HSTORE hMap)
{
    // Nothing to do, see '__store_fmap()'.
}

/*
 * __store_mmap (readonly, shared).
 */
inline sal_uInt8* __store_mmap (HSTORE h, sal_uInt32 k, sal_uInt32 n)
{
    void * p = ::mmap (NULL, (size_t)n, PROT_READ, MAP_SHARED, h, (off_t)k);
    return ((p != MAP_FAILED) ? (sal_uInt8*)p : 0);
}

/*
 * __store_munmap.
 */
inline void __store_munmap (sal_uInt8 *p, sal_uInt32 n)
{
    ::munmap ((char *)p, (size_t)n);
}

/*
 * __store_fopen.
 */
inline storeError __store_fopen (
    const sal_Char *pszName, sal_uInt32 nMode, HSTORE &rhFile)
{
    // Access mode.
    int nAccessMode = O_RDONLY;

    if (nMode & store_File_OpenWrite)
        nAccessMode = O_RDWR;

    if (nAccessMode == O_RDONLY)
        nMode |= store_File_OpenNoCreate;

    if ((!(nMode & store_File_OpenNoCreate)) && (!(nAccessMode == O_RDONLY)))
        nAccessMode |= O_CREAT;

    if (nMode & store_File_OpenTruncate)
        nAccessMode |= O_TRUNC;

    // Share mode.
    int nShareMode = S_IREAD | S_IROTH | S_IRGRP;

    if (nMode & store_File_OpenWrite)
        nShareMode |= (S_IWRITE | S_IWOTH | S_IWGRP);

    // Create file handle.
    if ((rhFile = ::open (pszName, nAccessMode, nShareMode)) < 0)
    {
        rhFile = 0;
        return ERROR_FROM_NATIVE(errno);
    }

#ifdef SOLARIS /* see workaround comment below */

    /*
     * Workaround for SunOS <= 5.7:
     *
     * 'mmap()' fails on posix (advisory) locked (F_SETLK) NFS file handles.
     * Using non-posix F_SHARE / F_UNSHARE instead.
     */

    // Acquire (advisory) Share Access (Multiple Reader | Single Writer)
    struct fshare share;

    if (nMode & store_File_OpenWrite)
    {
        share.f_access = F_RWACC; /* Request own read and write access */
        share.f_deny   = F_RWDNY; /* Deny other's read and write access */
    }
    else
    {
        share.f_access = F_RDACC; /* Request own read-only access */
        share.f_deny   = F_WRDNY; /* Deny other's write access */
    }

    share.f_id = 0;

    if (::fcntl (rhFile, F_SHARE, &share) < 0)
    {
        // Save original result.
        storeError result;
        if ((errno == EACCES) || (errno == EAGAIN))
            result = store_E_LockingViolation;
        else
            result = ERROR_FROM_NATIVE(errno);

        // Close file handle.
        ::close (rhFile); rhFile = 0;

        // Finish.
        return (result);
    }

#else  /* POSIX */

    // Acquire (advisory) Lock (Multiple Reader | Single Writer)
    struct flock lock;

    if (nMode & store_File_OpenWrite)
        lock.l_type = F_WRLCK;
    else
        lock.l_type = F_RDLCK;

    lock.l_whence = SEEK_SET;
    lock.l_start  = 0;
    lock.l_len    = 0;

    if (::fcntl (rhFile, F_SETLK, &lock) < 0)
    {
        // Save original result.
        storeError result;
        if ((errno == EACCES) || (errno == EAGAIN))
            result = store_E_LockingViolation;
        else
            result = ERROR_FROM_NATIVE(errno);

        // Close file handle.
        ::close (rhFile); rhFile = 0;

        // Finish.
        return (result);
    }

#endif /* SOLARIS || POSIX */

    int nFlags = ::fcntl (rhFile, F_GETFD, 0);
    if (!(nFlags < 0))
    {
        // Set close-on-exec flag.
        nFlags |= FD_CLOEXEC;
        ::fcntl (rhFile, F_SETFD, nFlags);
    }
    return store_E_None;
}

/*
 * __store_fread.
 */
inline storeError __store_fread (
    HSTORE h, sal_uInt32 offset, void *p, sal_uInt32 n, sal_uInt32 &k)
{
#if defined(LINUX) || defined(SOLARIS)

    k = (sal_uInt32)::pread (h, (char*)p, (size_t)n, (off_t)offset);
    if ((k == (sal_uInt32)(-1)) && (errno == EOVERFLOW))
    {
        /*
         * Workaround for 'pread()' failure at end-of-file:
         *
         * Some 'pread()'s fail with EOVERFLOW when reading at (or past)
         * end-of-file, different from 'lseek() + read()' behaviour.
         * Returning '0 bytes read' and 'store_E_None' instead.
         */
        k = 0;
    }

#else /* LINUX || SOLARIS */

    if (::lseek (h, (off_t)offset, SEEK_SET) < 0)
        return ERROR_FROM_NATIVE(errno);

    k = (sal_uInt32)::read (h, (char *)p, (size_t)n);

#endif /* LINUX || SOLARIS */

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
#if defined(LINUX) || defined(SOLARIS)

    k = (sal_uInt32)::pwrite (h, (char*)p, (size_t)n, (off_t)offset);

#else /* LINUX || SOLARIS */

    if (::lseek (h, (off_t)offset, SEEK_SET) < 0)
        return ERROR_FROM_NATIVE(errno);

    k = (sal_uInt32)::write (h, (char *)p, (size_t)n);

#endif /* LINUX || SOLARIS */

    if (k == (sal_uInt32)(-1))
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_fsize.
 */
inline storeError __store_fsize (HSTORE h, sal_uInt32 &k)
{
    k = (sal_uInt32)::lseek (h, (off_t)0, SEEK_END);
    if (k == (sal_uInt32)(-1))
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_ftrunc.
 */
inline storeError __store_ftrunc (HSTORE h, sal_uInt32 n)
{
    if (::ftruncate (h, (off_t)n) < 0)
    {
        // Save original result.
        storeError result = ERROR_FROM_NATIVE(errno);

        // Check against current size. Fail upon 'shrink'.
        sal_uInt32 k = (sal_uInt32)::lseek (h, (off_t)0, SEEK_END);
        if (k == (sal_uInt32)(-1))
            return (result);
        if ((0 <= n) && (n <= k))
            return (result);

        // Try 'expand' via 'lseek()' and 'write()'.
        if (::lseek (h, (off_t)(n - 1), SEEK_SET) < 0)
            return (result);
        if (::write (h, (char*)"", (size_t)1) < 0)
            return (result);
    }
    return store_E_None;
}

/*
 * __store_fsync.
 */
inline void __store_fsync (HSTORE h)
{
    ::fsync (h);
}

/*
 * __store_fclose.
 */
inline void __store_fclose (HSTORE h)
{
#ifdef SOLARIS /* see comment in __store_fopen() */

    // Release (advisory) Share Access (Multiple Reader | Single Writer)
    struct fshare share;

    share.f_access = 0;
    share.f_deny = 0;
    share.f_id = 0;

    ::fcntl (h, F_UNSHARE, &share);

#else  /* POSIX */

    // Release (advisory) Lock (Multiple Reader | Single Writer)
    struct flock lock;

    lock.l_type   = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start  = 0;
    lock.l_len    = 0;

    ::fcntl (h, F_SETLK, &lock);

#endif /* SOLARIS || POSIX */

    // Close file handle.
    ::close (h);
}

#endif /* INCLUDED_STORE_FILEUNX_HXX */
