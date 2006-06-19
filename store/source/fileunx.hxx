/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileunx.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:32:01 $
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
inline void __store_funmap (HSTORE)
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
    (void)::munmap ((char *)p, (size_t)n);
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
        (void)::close (rhFile); rhFile = 0;

        // Finish.
        return (result);
    }

    int nFlags = ::fcntl (rhFile, F_GETFD, 0);
    if (!(nFlags < 0))
    {
        // Set close-on-exec flag.
        nFlags |= FD_CLOEXEC;
        (void)::fcntl (rhFile, F_SETFD, nFlags);
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
        if (n <= k)
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
inline storeError __store_fsync (HSTORE h)
{
    if (::fsync (h) == -1)
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

/*
 * __store_fclose.
 */
inline storeError __store_fclose (HSTORE h)
{
    // Release (advisory) Lock (Multiple Reader | Single Writer)
    struct flock lock;

    lock.l_type   = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start  = 0;
    lock.l_len    = 0;

    (void)::fcntl (h, F_SETLK, &lock);

    // Close file handle.
    if (::close (h) == -1)
        return ERROR_FROM_NATIVE(errno);
    else
        return store_E_None;
}

#endif /* INCLUDED_STORE_FILEUNX_HXX */
