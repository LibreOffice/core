/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>
#include <o3tl/safeint.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <sal/log.hxx>
#include <osl/detail/file.h>
#include <rtl/byteseq.h>
#include <rtl/string.hxx>

#include "system.hxx"
#include "createfilehandlefromfd.hxx"
#include "file_error_transl.hxx"
#include "file_impl.hxx"
#include "file_url.hxx"
#include "uunxapi.hxx"
#include "unixerrnostring.hxx"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <fcntl.h>
#include <limits>
#include <limits.h>

#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(MACOSX)

#include <sys/param.h>
#include <sys/mount.h>
#define HAVE_O_EXLOCK

#include <CoreFoundation/CoreFoundation.h>

#endif /* MACOSX */

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <o3tl/string_view.hxx>
#include <vector>
#endif

#ifdef LINUX
#include <sys/vfs.h>
// As documented by the kernel
#define SMB_SUPER_MAGIC  static_cast<__fsword_t>(0x517B)
#define CIFS_SUPER_MAGIC static_cast<__fsword_t>(0xFF534D42)
#define SMB2_SUPER_MAGIC static_cast<__fsword_t>(0xFE534D42)
#endif

namespace {

enum class State
{
    Seekable  = 1, /*< default */
    Readable  = 2, /*< default */
    Writeable = 4, /*< open() sets, write() requires, else osl_File_E_BADF */
    Modified  = 8  /*< write() sets, flush() resets  */
};

}

template<> struct o3tl::typed_flags<State>: o3tl::is_typed_flags<State, 15> {};

namespace {

struct FileHandle_Impl
{
    pthread_mutex_t m_mutex;
    OString         m_strFilePath; /*< holds native file path */
    int             m_fd;

    enum Kind
    {
        KIND_FD = 1,
        KIND_MEM = 2
    };
    int          m_kind;
    /** State
     */
    State m_state;

    sal_uInt64   m_size;    /*< file size */
    off_t        m_offset;  /*< physical offset from begin of file */
    // m_fileptr is hit hard in some situations, where the overhead of a mutex starts to show up, so use an atomic
    std::atomic<off_t> m_fileptr; /*< logical offset from begin of file */

    off_t        m_bufptr;  /*< buffer offset from begin of file */
    size_t       m_buflen;  /*< buffer filled [0, m_bufsiz - 1] */

    size_t       m_bufsiz;
    sal_uInt8 *  m_buffer;
#ifdef ANDROID
    rtl_String*  m_memstreambuf; /*< used for in-memory streams */
#endif

    explicit FileHandle_Impl(int fd, Kind kind = KIND_FD, OString path = "<anon>"_ostr);
    ~FileHandle_Impl();

    static size_t getpagesize();

    sal_uInt64   getPos() const;
    void setPos(sal_uInt64 uPos);

    sal_uInt64   getSize() const;
    oslFileError setSize(sal_uInt64 uSize);

    oslFileError readAt(
        off_t        nOffset,
        void*        pBuffer,
        size_t       nBytesRequested,
        sal_uInt64*  pBytesRead);

    oslFileError writeAt(
        off_t        nOffset,
        void const*  pBuffer,
        size_t       nBytesToWrite,
        sal_uInt64*  pBytesWritten);

    oslFileError readFileAt(
        off_t        nOffset,
        void*        pBuffer,
        size_t       nBytesRequested,
        sal_uInt64*  pBytesRead);

    oslFileError writeFileAt(
        off_t        nOffset,
        void const*  pBuffer,
        size_t       nBytesToWrite,
        sal_uInt64*  pBytesWritten);

    oslFileError readLineAt(
        off_t           nOffset,
        sal_Sequence**  ppSequence,
        sal_uInt64*     pBytesRead);

    static oslFileError writeSequence_Impl(
        sal_Sequence**  ppSequence,
        size_t*         pnOffset,
        const void*     pBuffer,
        size_t          nBytes);

    oslFileError syncFile();

    class Guard
    {
        pthread_mutex_t *m_mutex;

    public:
        explicit Guard(pthread_mutex_t *pMutex);
        ~Guard();
    };
};

}

FileHandle_Impl::Guard::Guard(pthread_mutex_t * pMutex)
    : m_mutex(pMutex)
{
    assert(m_mutex);
    (void) pthread_mutex_lock(m_mutex); // ignoring EINVAL if a null mutex is passed ...
}

FileHandle_Impl::Guard::~Guard()
{
    assert(m_mutex);
    (void) pthread_mutex_unlock(m_mutex);
}

FileHandle_Impl::FileHandle_Impl(int fd, enum Kind kind, OString path)
    : m_strFilePath(std::move(path)),
      m_fd      (fd),
      m_kind    (kind),
      m_state   (State::Seekable | State::Readable),
      m_size    (0),
      m_offset  (0),
      m_fileptr (0),
      m_bufptr  (-1),
      m_buflen  (0),
      m_bufsiz  (0),
      m_buffer  (nullptr)
{
    (void) pthread_mutex_init(&m_mutex, nullptr);
    if (m_kind == KIND_FD)
    {
        size_t const pagesize = getpagesize();
        if (pagesize != size_t(-1))
        {
            m_bufsiz = pagesize;
            m_buffer = static_cast<sal_uInt8 *>(calloc(1, m_bufsiz));
        }
    }
}

FileHandle_Impl::~FileHandle_Impl()
{
    if (m_kind == KIND_FD)
    {
        free(m_buffer);
        m_buffer = nullptr;
    }

    (void) pthread_mutex_destroy(&m_mutex); // ignoring EBUSY ...
}

size_t FileHandle_Impl::getpagesize()
{
    return sal::static_int_cast< size_t >(::sysconf(_SC_PAGESIZE));
}

sal_uInt64 FileHandle_Impl::getPos() const
{
    return sal::static_int_cast< sal_uInt64 >(m_fileptr.load());
}

void FileHandle_Impl::setPos(sal_uInt64 uPos)
{
    m_fileptr = sal::static_int_cast< off_t >(uPos);
}

sal_uInt64 FileHandle_Impl::getSize() const
{
    off_t const bufend = std::max(off_t(0), m_bufptr) + m_buflen;
    return std::max(m_size, sal::static_int_cast< sal_uInt64 >(bufend));
}

oslFileError FileHandle_Impl::setSize(sal_uInt64 uSize)
{
    off_t const nSize = sal::static_int_cast< off_t >(uSize);
    if (ftruncate_with_name(m_fd, nSize, m_strFilePath) == -1)
    {
        /* Failure. Save original result. Try fallback algorithm */
        oslFileError result = oslTranslateFileError(errno);

        /* Check against current size. Fail upon 'shrink' */
        if (uSize <= getSize())
        {
            /* Failure upon 'shrink'. Return original result */
            return result;
        }

        /* Save current position */
        off_t const nCurPos = lseek(m_fd, off_t(0), SEEK_CUR);
        if (nCurPos == off_t(-1))
        {
            int e = errno;
            SAL_INFO("sal.file", "lseek(" << m_fd << ",0,SEEK_CUR): " << UnixErrnoString(e));
            return result;
        }
        else
            SAL_INFO("sal.file", "lseek(" << m_fd << ",0,SEEK_CUR): OK");

        /* Try 'expand' via 'lseek()' and 'write()' */
        if (lseek(m_fd, static_cast<off_t>(nSize - 1), SEEK_SET) == -1)
        {
            int e = errno;
            SAL_INFO("sal.file", "lseek(" << m_fd << "," << nSize - 1 << ",SEEK_SET): " << UnixErrnoString(e));
            return result;
        }
        else
            SAL_INFO("sal.file", "lseek(" << m_fd << "," << nSize - 1 << ",SEEK_SET): OK");

        if (write(m_fd, "", size_t(1)) == -1)
        {
            /* Failure. Restore saved position */
            int e = errno;
            SAL_INFO("sal.file", "write(" << m_fd << ",\"\",1): " << UnixErrnoString(e));
            (void) lseek(m_fd, nCurPos, SEEK_SET);
            return result;
        }
        else
            SAL_INFO("sal.file", "write(" << m_fd << ",\"\",1): OK");

        /* Success. Restore saved position */
        if (lseek(m_fd, nCurPos, SEEK_SET) == -1)
            return result;
    }

    m_size = sal::static_int_cast< sal_uInt64 >(nSize);
    return osl_File_E_None;
}

oslFileError FileHandle_Impl::readAt(
    off_t        nOffset,
    void *       pBuffer,
    size_t       nBytesRequested,
    sal_uInt64 * pBytesRead)
{
    SAL_WARN_IF(!(m_state & State::Seekable), "sal.osl", "FileHandle_Impl::readAt(): not seekable");
    if (!(m_state & State::Seekable))
        return osl_File_E_SPIPE;

    SAL_WARN_IF(!(m_state & State::Readable), "sal.osl", "FileHandle_Impl::readAt(): not readable");
    if (!(m_state & State::Readable))
        return osl_File_E_BADF;

    if (m_kind == KIND_MEM)
    {
        ssize_t nBytes;

        m_offset = nOffset;

        if (o3tl::make_unsigned(m_offset) >= m_size)
        {
            nBytes = 0;
        }
        else
        {
            nBytes = std::min(nBytesRequested, static_cast<size_t>(m_size - m_offset));
            memmove(pBuffer, m_buffer + m_offset, nBytes);
            m_offset += nBytes;
        }
        *pBytesRead = nBytes;
        return osl_File_E_None;
    }

    ssize_t nBytes = ::pread(m_fd, pBuffer, nBytesRequested, nOffset);
    if ((nBytes == -1) && (errno == EOVERFLOW))
    {
        /* Some 'pread()'s fail with EOVERFLOW when reading at (or past)
         * end-of-file, different from 'lseek() + read()' behaviour.
         * Returning '0 bytes read' and 'osl_File_E_None' instead.
         */
        nBytes = 0;
    }

    if (nBytes == -1)
        return oslTranslateFileError(errno);

    *pBytesRead = nBytes;

    return osl_File_E_None;
}

oslFileError FileHandle_Impl::writeAt(
    off_t        nOffset,
    void const * pBuffer,
    size_t       nBytesToWrite,
    sal_uInt64 * pBytesWritten)
{
    SAL_WARN_IF(!(m_state & State::Seekable), "sal.osl", "FileHandle_Impl::writeAt(): not seekable");
    if (!(m_state & State::Seekable))
        return osl_File_E_SPIPE;

    SAL_WARN_IF(!(m_state & State::Writeable), "sal.osl", "FileHandle_Impl::writeAt(): not writeable");
    if (!(m_state & State::Writeable))
        return osl_File_E_BADF;

    ssize_t nBytes = ::pwrite(m_fd, pBuffer, nBytesToWrite, nOffset);
    if (nBytes == -1)
        return oslTranslateFileError(errno);

    m_size = std::max(m_size, sal::static_int_cast< sal_uInt64 >(nOffset + nBytes));

    *pBytesWritten = nBytes;

    return osl_File_E_None;
}

oslFileError FileHandle_Impl::readFileAt(
    off_t        nOffset,
    void*        pBuffer,
    size_t       nBytesRequested,
    sal_uInt64*  pBytesRead)
{
    if (!(m_state & State::Seekable))
    {
        // not seekable (pipe)
        ssize_t nBytes = ::read(m_fd, pBuffer, nBytesRequested);
        if (nBytes == -1)
            return oslTranslateFileError(errno);

        *pBytesRead = nBytes;

        return osl_File_E_None;
    }

    if (m_kind == KIND_MEM || !m_buffer)
    {
        // not buffered
        return readAt(nOffset, pBuffer, nBytesRequested, pBytesRead);
    }

    sal_uInt8 *buffer = static_cast<sal_uInt8*>(pBuffer);
    for (*pBytesRead = 0; nBytesRequested > 0; )
    {
        off_t  const bufptr = (nOffset / m_bufsiz) * m_bufsiz;
        size_t const bufpos = nOffset % m_bufsiz;

        if (bufptr != m_bufptr)
        {
            // flush current buffer
            oslFileError result = syncFile();
            if (result != osl_File_E_None)
                return result;

            m_bufptr = -1;
            m_buflen = 0;

            if (nBytesRequested >= m_bufsiz)
            {
                // buffer too small, read through from file
                sal_uInt64 uDone = 0;
                result = readAt(nOffset, &(buffer[*pBytesRead]), nBytesRequested, &uDone);
                if (result != osl_File_E_None)
                    return result;

                *pBytesRead += uDone;

                return osl_File_E_None;
            }

            // update buffer (pointer)
            sal_uInt64 uDone = 0;
            result = readAt(bufptr, m_buffer, m_bufsiz, &uDone);
            if (result != osl_File_E_None)
                return result;

            m_bufptr = bufptr;
            m_buflen = uDone;
        }

        if (bufpos >= m_buflen)
        {
            // end of file
            return osl_File_E_None;
        }

        size_t const bytes = std::min(m_buflen - bufpos, nBytesRequested);
        SAL_INFO("sal.fileio", "FileHandle_Impl::readFileAt(" << m_fd << ", " << nOffset << ", " << bytes << ")");

        memcpy(&(buffer[*pBytesRead]), &(m_buffer[bufpos]), bytes);
        nBytesRequested -= bytes;
        *pBytesRead += bytes;
        nOffset += bytes;
    }

    return osl_File_E_None;
}

oslFileError FileHandle_Impl::writeFileAt(
    off_t        nOffset,
    void const * pBuffer,
    size_t       nBytesToWrite,
    sal_uInt64 * pBytesWritten)
{
    if (!(m_state & State::Seekable))
    {
        // not seekable (pipe)
        ssize_t nBytes = ::write(m_fd, pBuffer, nBytesToWrite);
        if (nBytes == -1)
            return oslTranslateFileError(errno);

        *pBytesWritten = nBytes;

        return osl_File_E_None;
    }
    if (!m_buffer)
    {
        // not buffered
        return writeAt(nOffset, pBuffer, nBytesToWrite, pBytesWritten);
    }

    sal_uInt8 const * buffer = static_cast<sal_uInt8 const *>(pBuffer);
    for (*pBytesWritten = 0; nBytesToWrite > 0;)
    {
        off_t const bufptr = (nOffset / m_bufsiz) * m_bufsiz;
        size_t const bufpos = nOffset % m_bufsiz;
        if (bufptr != m_bufptr)
        {
            // flush current buffer
            oslFileError result = syncFile();
            if (result != osl_File_E_None)
                return result;
            m_bufptr = -1;
            m_buflen = 0;

            if (nBytesToWrite >= m_bufsiz)
            {
                // buffer too small, write through to file
                sal_uInt64 uDone = 0;
                result = writeAt(nOffset, &(buffer[*pBytesWritten]), nBytesToWrite, &uDone);
                if (result != osl_File_E_None)
                    return result;

                if (uDone != nBytesToWrite)
                    return osl_File_E_IO;

                *pBytesWritten += uDone;

                return osl_File_E_None;
            }

            // update buffer (pointer)
            sal_uInt64 uDone = 0;
            result = readAt(bufptr, m_buffer, m_bufsiz, &uDone);
            if (result != osl_File_E_None)
                return result;

            m_bufptr = bufptr;
            m_buflen = uDone;
        }

        size_t const bytes = std::min(m_bufsiz - bufpos, nBytesToWrite);
        SAL_INFO("sal.fileio", "FileHandle_Impl::writeFileAt(" << m_fd << ", " << nOffset << ", " << bytes << ")");

        memcpy(&(m_buffer[bufpos]), &(buffer[*pBytesWritten]), bytes);
        nBytesToWrite -= bytes;
        *pBytesWritten += bytes;
        nOffset += bytes;

        m_buflen = std::max(m_buflen, bufpos + bytes);
        m_state |= State::Modified;
    }

    return osl_File_E_None;
}

oslFileError FileHandle_Impl::readLineAt(
    off_t           nOffset,
    sal_Sequence ** ppSequence,
    sal_uInt64 *    pBytesRead)
{
    oslFileError result = osl_File_E_None;

    off_t bufptr = nOffset / m_bufsiz * m_bufsiz;
    if (bufptr != m_bufptr)
    {
        /* flush current buffer */
        result = syncFile();
        if (result != osl_File_E_None)
            return result;

        /* update buffer (pointer) */
        sal_uInt64 uDone = 0;
        result = readAt(bufptr, m_buffer, m_bufsiz, &uDone);
        if (result != osl_File_E_None)
            return result;

        m_bufptr = bufptr;
        m_buflen = uDone;
    }

    static int const LINE_STATE_BEGIN = 0;
    static int const LINE_STATE_CR    = 1;
    static int const LINE_STATE_LF    = 2;

    size_t bufpos = nOffset - m_bufptr, curpos = bufpos, dstpos = 0;
    int state = (bufpos >= m_buflen) ? LINE_STATE_LF : LINE_STATE_BEGIN;

    while (state != LINE_STATE_LF)
    {
        if (curpos >= m_buflen)
        {
            /* buffer examined */
            if ((curpos - bufpos) > 0)
            {
                /* flush buffer to sequence */
                result = writeSequence_Impl(
                    ppSequence, &dstpos, &(m_buffer[bufpos]), curpos - bufpos);
                if (result != osl_File_E_None)
                    return result;

                *pBytesRead += curpos - bufpos;
                nOffset += curpos - bufpos;
            }

            bufptr = nOffset / m_bufsiz * m_bufsiz;
            if (bufptr != m_bufptr)
            {
                /* update buffer (pointer) */
                sal_uInt64 uDone = 0;
                result = readAt(bufptr, m_buffer, m_bufsiz, &uDone);
                if (result != osl_File_E_None)
                    return result;

                m_bufptr = bufptr;
                m_buflen = uDone;
            }

            bufpos = nOffset - m_bufptr;
            curpos = bufpos;
            if (bufpos >= m_buflen)
                break;
        }

        switch (state)
        {
        case LINE_STATE_CR:
            state = LINE_STATE_LF;
            switch (m_buffer[curpos])
            {
            case 0x0A: /* CRLF */
                /* eat current char */
                curpos++;
                break;
            default: /* single CR */
                /* keep current char */
                break;
            }
            break;
        default:
            /* determine next state */
            switch (m_buffer[curpos])
            {
            case 0x0A: /* single LF */
                state = LINE_STATE_LF;
                break;
            case 0x0D: /* CR */
                state = LINE_STATE_CR;
                break;
            default: /* advance to next char */
                curpos++;
                break;
            }
            if (state != LINE_STATE_BEGIN)
            {
                /* skip the newline char */
                curpos++;

                /* flush buffer to sequence */
                result = writeSequence_Impl(
                    ppSequence, &dstpos, &(m_buffer[bufpos]), curpos - bufpos - 1);
                if (result != osl_File_E_None)
                    return result;

                *pBytesRead += curpos - bufpos;
                nOffset += curpos - bufpos;
            }
            break;
        }
    }

    result = writeSequence_Impl(ppSequence, &dstpos, nullptr, 0);
    if (result != osl_File_E_None)
        return result;

    if (dstpos > 0)
        return osl_File_E_None;

    if (bufpos >= m_buflen)
        return osl_File_E_AGAIN;

    return osl_File_E_None;
}

oslFileError FileHandle_Impl::writeSequence_Impl(
    sal_Sequence ** ppSequence,
    size_t *        pnOffset,
    const void *    pBuffer,
    size_t          nBytes)
{
    sal_Int32 nElements = *pnOffset + nBytes;
    if (!*ppSequence)
    {
        /* construct sequence */
        rtl_byte_sequence_constructNoDefault(ppSequence, nElements);
    }
    else if (nElements != (*ppSequence)->nElements)
    {
        /* resize sequence */
        rtl_byte_sequence_realloc(ppSequence, nElements);
    }

    if (*ppSequence && nBytes != 0)
    {
        /* fill sequence */
        memcpy(&((*ppSequence)->elements[*pnOffset]), pBuffer, nBytes);
        *pnOffset += nBytes;
    }

    return (*ppSequence) ? osl_File_E_None : osl_File_E_NOMEM;
}

oslFileError FileHandle_Impl::syncFile()
{
    oslFileError result = osl_File_E_None;
    if (m_state & State::Modified)
    {
        sal_uInt64 uDone = 0;
        result = writeAt(m_bufptr, m_buffer, m_buflen, &uDone);
        if (result != osl_File_E_None)
            return result;

        if (uDone != m_buflen)
            return osl_File_E_IO;

        m_state &= ~State::Modified;
    }

    return result;
}

oslFileHandle osl::detail::createFileHandleFromFD(int fd)
{
    if (fd == -1)
        return nullptr; // EINVAL

    struct stat aFileStat;
    if (fstat(fd, &aFileStat) == -1)
        return nullptr; // EBADF

    FileHandle_Impl *pImpl = new FileHandle_Impl(fd);

    // assume writeable
    pImpl->m_state |= State::Writeable;
    if (!S_ISREG(aFileStat.st_mode))
    {
        /* not a regular file, mark not seekable */
        pImpl->m_state &= ~State::Seekable;
    }
    else
    {
        /* regular file, init current size */
        pImpl->m_size = sal::static_int_cast< sal_uInt64 >(aFileStat.st_size);
    }

    SAL_INFO("sal.file", "osl::detail::createFileHandleFromFD(" << pImpl->m_fd << ", writeable) => " << pImpl->m_strFilePath);

    return static_cast<oslFileHandle>(pImpl);
}

static void osl_file_adjustLockFlags(const OString& path, int *flags, sal_uInt32 *uFlags)
{
#ifdef MACOSX
    (void) uFlags;

    /*
     * The AFP implementation of MacOS X 10.4 treats O_EXLOCK in a way
     * that makes it impossible for OOo to create a backup copy of the
     * file it keeps opened. OTOH O_SHLOCK for AFP behaves as desired by
     * the OOo file handling, so we need to check the path of the file
     * for the filesystem name.
     */
    struct statfs s;
    if(statfs(path.getStr(), &s) >= 0)
    {
        if(strncmp("afpfs", s.f_fstypename, 5) == 0)
        {
            *flags &= ~O_EXLOCK;
            *flags |=  O_SHLOCK;
        }
        else
        {
            /* Needed flags to allow opening a webdav file */
            *flags &= ~(O_EXLOCK | O_SHLOCK | O_NONBLOCK);
        }
    }
#elif defined(LINUX)
    (void) flags;

    /* get filesystem info */
    struct statfs aFileStatFs;
    if (statfs(path.getStr(), &aFileStatFs) < 0)
    {
        int e = errno;
        SAL_INFO("sal.file", "statfs(" << path << "): " << UnixErrnoString(e));
    }
    else
    {
        SAL_INFO("sal.file", "statfs(" << path << "): OK");

        // We avoid locking if on a Linux CIFS mount otherwise this
        // fill fail later on when opening the file for reading
        // during backup creation at save time (even though this is a
        // write lock and not a read lock).
        // Fixes the following bug:
        // https://bugs.documentfoundation.org/show_bug.cgi?id=55004
        switch (aFileStatFs.f_type) {
        case SMB_SUPER_MAGIC:
        case CIFS_SUPER_MAGIC:
        case SMB2_SUPER_MAGIC:
            *uFlags |= osl_File_OpenFlag_NoLock;
            break;
        default:
            break;
        }
    }
#else
    (void) path;
    (void) flags;
    (void) uFlags;
#endif
}

static bool osl_file_queryLocking(sal_uInt32 uFlags)
{
#if !defined HAVE_O_EXLOCK
    if (!(uFlags & osl_File_OpenFlag_NoLock)
        && ((uFlags & osl_File_OpenFlag_Write)
            || (uFlags & osl_File_OpenFlag_Create)))
    {
        static bool enabled = getenv("SAL_ENABLE_FILE_LOCKING") != nullptr;
            // getenv is not thread safe, so minimize use of result
        return enabled;
    }
#else
    (void) uFlags;
#endif
    return false;
}

#ifdef HAVE_O_EXLOCK
#define OPEN_WRITE_FLAGS ( O_RDWR | O_EXLOCK | O_NONBLOCK )
#define OPEN_CREATE_FLAGS ( O_CREAT | O_RDWR | O_EXLOCK | O_NONBLOCK )
#else
#define OPEN_WRITE_FLAGS ( O_RDWR )
#define OPEN_CREATE_FLAGS ( O_CREAT | O_RDWR )
#endif

#if defined ANDROID

namespace {

static oslFileError openMemoryAsFile(const OString &rData,
                                     oslFileHandle *pHandle,
                                     const OString& path)
{
    const char *address = rData.getStr();
    size_t size = rData.getLength();

    FileHandle_Impl *pImpl = new FileHandle_Impl(-1, FileHandle_Impl::KIND_MEM, path);
    pImpl->m_size = sal::static_int_cast< sal_uInt64 >(size);

    *pHandle = (oslFileHandle)(pImpl);

    pImpl->m_bufptr = 0;
    pImpl->m_buflen = size;
    pImpl->m_memstreambuf = rData.pData;
    rtl_string_acquire(pImpl->m_memstreambuf);

    pImpl->m_bufsiz = size;
    pImpl->m_buffer = reinterpret_cast<sal_uInt8*>(const_cast<char *>(address));

    return osl_File_E_None;
}

/*
 * Reading files from /assets/ on Android via a transition into the VM
 * shows on profiles and is rather slow; so we cache small files as
 * used by UNO, UI-builder etc.
 */
class AndroidFileCache {
public:
    struct Entry {
        OString maFilePath;
        OString maData;
    };
    AndroidFileCache(size_t nElements)
        : mnCur(0)
    {
        maEntries.resize(nElements);
        assert (maEntries.size() == nElements);
    }
    Entry *find(const char *cpFilePath)
    {
        for (auto &it : maEntries)
        {
            if (!strcmp(it.maFilePath.getStr(), cpFilePath))
                return &it;
        }
        return nullptr;
    }
    // no clever LRU - but - good enough for now.
    void insert(const char *cpFilePath, OString &rData)
    {
        assert (maEntries.size() > 0);
        if (++mnCur >= maEntries.size())
            mnCur = 0;
        maEntries[mnCur].maFilePath = OString(cpFilePath, strlen(cpFilePath));
        maEntries[mnCur].maData = rData;
    }
    static AndroidFileCache &getHitCache()
    {
        static AndroidFileCache *pCache = new AndroidFileCache(16);
        return *pCache;
    }
    static AndroidFileCache &getMissCache()
    {
        static AndroidFileCache *pCache = new AndroidFileCache(32);
        return *pCache;
    }
private:
    size_t             mnCur;
    std::vector<Entry> maEntries;
};

} // namespace

#endif

oslFileError openFilePath(const OString& filePath, oslFileHandle* pHandle,
                          sal_uInt32 uFlags, mode_t mode)
{
    oslFileError eRet;

#ifdef ANDROID
    /* Opening a file from /assets read-only means
     * we should mmap it from the .apk file
     */
    if (o3tl::starts_with(filePath, "/assets/"))
    {
        OString aData;
        bool bCache = true;

        const char *cpAssetsPath = filePath.getStr() + sizeof("/assets/") - 1;
        // some requests are /assets//foo...
        if (cpAssetsPath[0] == '/')
        {
            __android_log_print(ANDROID_LOG_DEBUG,"libo:sal/osl/unx/file", "double-slash in path: %s", filePath.getStr());
            cpAssetsPath++;
        }

        AndroidFileCache::Entry *pHit = AndroidFileCache::getHitCache().find(cpAssetsPath);
        if (pHit)
            aData = pHit->maData;

        else
        {
            bCache = false;
            AndroidFileCache::Entry *pMiss = AndroidFileCache::getMissCache().find(cpAssetsPath);
            if (pMiss)
            {
                errno = ENOENT;
                __android_log_print(ANDROID_LOG_ERROR,"libo:sal/osl/unx/file", "miss cache: failed to open %s", filePath.getStr());
                return osl_File_E_NOENT;
            }
            AAssetManager* mgr = lo_get_native_assetmgr();
            AAsset* asset = AAssetManager_open(mgr, cpAssetsPath, AASSET_MODE_BUFFER);
            if (!asset)
            {
                AndroidFileCache::getMissCache().insert(cpAssetsPath, aData);
                errno = ENOENT;
                __android_log_print(ANDROID_LOG_ERROR,"libo:sal/osl/unx/file", "failed to open %s", filePath.getStr());
                return osl_File_E_NOENT;
            }
            else
            {
                rtl_String *pData = nullptr;
                size_t size = AAsset_getLength(asset);
                rtl_string_new_WithLength(&pData, size);
                pData->length = size;
                AAsset_read(asset, pData->buffer, size);
                AAsset_close(asset);

                aData = OString(pData, SAL_NO_ACQUIRE);

                if (pData->length < 50 * 1024)
                    AndroidFileCache::getHitCache().insert(cpAssetsPath, aData);
            }
        }

        if (uFlags & osl_File_OpenFlag_Write)
        {
            // It seems to work better to silently "open" it read-only
            // and let write attempts, if any, fail later. Otherwise
            // loading a document from /assets fails with that idiotic
            // "General Error" dialog...
        }
        SAL_INFO("sal.file", "osl_openFile(" << filePath << ") => '" << cpAssetsPath << "'"
                 << aData.getLength() << " bytes from file " << (bCache ? "cache" : "system"));
        return openMemoryAsFile(aData, pHandle, filePath);
    }
#endif

    /* set mode and flags */
    int defmode = (uFlags & osl_File_OpenFlag_Private) ? S_IRUSR : S_IRUSR | S_IRGRP | S_IROTH;
    int flags = O_RDONLY;

    if (uFlags & osl_File_OpenFlag_Write)
    {
        defmode |= (uFlags & osl_File_OpenFlag_Private) ? S_IWUSR : S_IWUSR | S_IWGRP | S_IWOTH;
        flags = OPEN_WRITE_FLAGS;
    }

    if (uFlags & osl_File_OpenFlag_Create)
    {
        defmode |= (uFlags & osl_File_OpenFlag_Private) ? S_IWUSR : S_IWUSR | S_IWGRP | S_IWOTH;
        flags = OPEN_CREATE_FLAGS;
    }

    if (mode == mode_t(-1))
        mode = defmode;

    /* Check for flags passed in from SvFileStream::Open() */
    if (uFlags & osl_File_OpenFlag_Trunc)
        flags |= O_TRUNC;

    if (!(uFlags & osl_File_OpenFlag_NoExcl))
        flags |= O_EXCL;

    if (uFlags & osl_File_OpenFlag_NoLock)
    {
#ifdef HAVE_O_EXLOCK
        flags &= ~(O_EXLOCK | O_SHLOCK | O_NONBLOCK);
#endif /* HAVE_O_EXLOCK */
    }
    else
    {
        osl_file_adjustLockFlags (filePath, &flags, &uFlags);
    }

    // O_EXCL can be set only when O_CREAT is set
    if (flags & O_EXCL && !(flags & O_CREAT))
        flags &= ~O_EXCL;

    /* open the file */
    int fd = open_c( filePath, flags, mode );
    if (fd == -1)
    {
        return oslTranslateFileError(errno);
    }

#if !HAVE_FEATURE_MACOSX_SANDBOX
    /* reset O_NONBLOCK flag */
    if (flags & O_NONBLOCK)
    {
        int f = fcntl(fd, F_GETFL, 0);
        if (f == -1)
        {
            int e = errno;
            SAL_INFO("sal.file", "fcntl(" << fd << ",F_GETFL,0): " << UnixErrnoString(e));
            eRet = oslTranslateFileError(e);
            (void) close(fd);
            SAL_INFO("sal.file", "close(" << fd << ")");
            return eRet;
        }
        else
            SAL_INFO("sal.file", "fcntl(" << fd << ",F_GETFL,0): OK");

        if (fcntl(fd, F_SETFL, (f & ~O_NONBLOCK)) == -1)
        {
            int e = errno;
            SAL_INFO("sal.file", "fcntl(" << fd << ",F_SETFL,(f & ~O_NONBLOCK)): " << UnixErrnoString(e));
            eRet = oslTranslateFileError(e);
            (void) close(fd);
            SAL_INFO("sal.file", "close(" << fd << ")");
            return eRet;
        }
        else
            SAL_INFO("sal.file", "fcntl(" << fd << ",F_SETFL,(f & ~O_NONBLOCK)): OK");
    }
#endif

    /* get file status (mode, size) */
    struct stat aFileStat;
    if (fstat(fd, &aFileStat) == -1)
    {
        int e = errno;
        SAL_INFO("sal.file", "fstat(" << fd << "): " << UnixErrnoString(e));
        eRet = oslTranslateFileError(e);
        (void) close(fd);
        SAL_INFO("sal.file", "close(" << fd << ")");
        return eRet;
    }
    else
        SAL_INFO("sal.file", "fstat(" << fd << "): OK");

    if (!S_ISREG(aFileStat.st_mode))
    {
        /* we only open regular files here */
        SAL_INFO("sal.file", "osl_openFile(" << filePath << "): not a regular file");
        (void) close(fd);
        SAL_INFO("sal.file", "close(" << fd << ")");
        return osl_File_E_INVAL;
    }

    if (osl_file_queryLocking(uFlags))
    {
#ifdef MACOSX
        if (flock(fd, LOCK_EX | LOCK_NB) == -1)
        {
            int e = errno;
            SAL_INFO("sal.file", "flock(" << fd << ",LOCK_EX|LOCK_NB): " << UnixErrnoString(e));
            /* Mac OSX returns ENOTSUP for webdav drives. We should try read lock */

            // Restore errno after possibly having been overwritten by the SAL_INFO above...
            errno = e;
            if ((errno != ENOTSUP) || ((flock(fd, LOCK_SH | LOCK_NB) == 1) && (errno != ENOTSUP)))
            {
                eRet = oslTranslateFileError(errno);
                (void) close(fd);
                SAL_INFO("sal.file", "close(" << fd << ")");
                return eRet;
            }
        }
        else
            SAL_INFO("sal.file", "flock(" << fd << ",LOCK_EX|LOCK_NB): OK");
#else   /* F_SETLK */
        struct flock aflock;

        aflock.l_type = F_WRLCK;
        aflock.l_whence = SEEK_SET;
        aflock.l_start = 0;
        aflock.l_len = 0;

        if (fcntl(fd, F_SETLK, &aflock) == -1)
        {
            int e = errno;
            SAL_INFO("sal.file", "fcntl(" << fd << ",F_SETLK): " << UnixErrnoString(e));
            eRet = oslTranslateFileError(e);
            (void) close(fd);
            SAL_INFO("sal.file", "close(" << fd << ")");
            return eRet;
        }
#endif  /* F_SETLK */
    }

    /* allocate memory for impl structure */
    FileHandle_Impl *pImpl = new FileHandle_Impl(fd, FileHandle_Impl::KIND_FD, filePath);
    if (flags & O_RDWR)
        pImpl->m_state |= State::Writeable;

    pImpl->m_size = sal::static_int_cast< sal_uInt64 >(aFileStat.st_size);

    *pHandle = static_cast<oslFileHandle>(pImpl);

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_openFile(rtl_uString* ustrFileURL, oslFileHandle* pHandle, sal_uInt32 uFlags)
{
    return openFile(ustrFileURL, pHandle, uFlags, mode_t(-1));
}

oslFileError openFile(rtl_uString* ustrFileURL, oslFileHandle* pHandle, sal_uInt32 uFlags, mode_t mode)
{
    oslFileError eRet;

    if ((!ustrFileURL) || (ustrFileURL->length == 0) || (!pHandle))
        return osl_File_E_INVAL;

    /* convert file URL to system path */
    char buffer[PATH_MAX];
    eRet = FileURLToPath(buffer, sizeof(buffer), ustrFileURL);
    if (eRet != osl_File_E_None)
        return eRet;

#ifdef MACOSX
    if (macxp_resolveAlias(buffer, sizeof(buffer)) != 0)
        return oslTranslateFileError(errno);
#endif /* MACOSX */

    return openFilePath(buffer, pHandle, uFlags, mode);
}

oslFileError SAL_CALL osl_closeFile(oslFileHandle Handle)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if (!pImpl)
        return osl_File_E_INVAL;

    if (pImpl->m_kind == FileHandle_Impl::KIND_MEM)
    {
#ifdef ANDROID
        rtl_string_release(pImpl->m_memstreambuf);
        pImpl->m_memstreambuf = nullptr;

        pImpl->m_buffer = NULL;
#endif
        delete pImpl;
        return osl_File_E_None;
    }

    if (pImpl->m_fd < 0)
        return osl_File_E_INVAL;

    (void) pthread_mutex_lock(&(pImpl->m_mutex));

    /* close(2) implicitly (and unconditionally) unlocks */
    oslFileError result = pImpl->syncFile();
    if (result != osl_File_E_None)
    {
        /* close, ignoring double failure */
        (void) close(pImpl->m_fd);
        SAL_INFO("sal.file", "close(" << pImpl->m_fd << ")");
    }
    else if (close(pImpl->m_fd) == -1)
    {
        int e = errno;
        SAL_INFO("sal.file", "close(" << pImpl->m_fd << "): " << UnixErrnoString(e));
        /* translate error code */
        result = oslTranslateFileError(e);
    }
    else
        SAL_INFO("sal.file", "close(" << pImpl->m_fd << "): OK");

    (void) pthread_mutex_unlock(&(pImpl->m_mutex));
    delete pImpl;
    return result;
}

oslFileError SAL_CALL osl_syncFile(oslFileHandle Handle)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)))
        return osl_File_E_INVAL;

    if (pImpl->m_kind == FileHandle_Impl::KIND_MEM)
        return osl_File_E_None;

    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));

    oslFileError result = pImpl->syncFile();

    if (result != osl_File_E_None)
        return result;

    if (fsync(pImpl->m_fd) == -1)
    {
        int e = errno;
        SAL_INFO("sal.file", "fsync(" << pImpl->m_fd << "): " << UnixErrnoString(e));
        return oslTranslateFileError(e);
    }
    else
        SAL_INFO("sal.file", "fsync(" << pImpl->m_fd << "): OK");

    return osl_File_E_None;
}

const off_t MAX_OFF_T = std::numeric_limits< off_t >::max();

namespace {

// coverity[result_independent_of_operands] - crossplatform requirement
template<typename T> bool exceedsMaxOffT(T n) { return n > MAX_OFF_T; }

// coverity[result_independent_of_operands] - crossplatform requirement
template<typename T> bool exceedsMinOffT(T n)
{ return n < std::numeric_limits<off_t>::min(); }

}

oslFileError SAL_CALL osl_mapFile(
    oslFileHandle Handle,
    void**        ppAddr,
    sal_uInt64    uLength,
    sal_uInt64    uOffset,
    sal_uInt32    uFlags
)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)) || (!ppAddr))
        return osl_File_E_INVAL;

    *ppAddr = nullptr;

    if (uLength > SAL_MAX_SIZE)
        return osl_File_E_OVERFLOW;

    size_t const nLength = sal::static_int_cast< size_t >(uLength);

    if (exceedsMaxOffT(uOffset))
        return osl_File_E_OVERFLOW;

    if (pImpl->m_kind == FileHandle_Impl::KIND_MEM)
    {
        *ppAddr = pImpl->m_buffer + uOffset;
        return osl_File_E_None;
    }

    off_t const nOffset = sal::static_int_cast< off_t >(uOffset);

    void* p = mmap(nullptr, nLength, PROT_READ, MAP_SHARED, pImpl->m_fd, nOffset);

    if (p == MAP_FAILED)
        return oslTranslateFileError(errno);

    *ppAddr = p;

    if (uFlags & osl_File_MapFlag_RandomAccess)
    {
        // Determine memory pagesize.
        size_t const nPageSize = FileHandle_Impl::getpagesize();
        if (nPageSize != size_t(-1))
        {
            /*
             * Pagein, touching first byte of every memory page.
             * Note: volatile disables optimizing the loop away.
             */
            sal_uInt8 volatile *pData(static_cast<sal_uInt8*>(*ppAddr));
            size_t nSize(nLength);

            while (nSize > nPageSize)
            {
                pData[0];
                pData += nPageSize;
                nSize -= nPageSize;
            }

            if (nSize > 0)
                pData[0];
        }
    }

    if (uFlags & osl_File_MapFlag_WillNeed)
    {
        // On Linux, madvise(..., MADV_WILLNEED) appears to have the undesirable
        // effect of not returning until the data has actually been paged in, so
        // that its net effect would typically be to slow down the process
        // (which could start processing at the beginning of the data while the
        // OS simultaneously pages in the rest); on other platforms, it remains
        // to be evaluated whether madvise or equivalent is available and
        // actually useful:
#if defined MACOSX || (defined(__sun) && (!defined(__XOPEN_OR_POSIX) || defined(_XPG6) || defined(__EXTENSIONS__)))
        int e = posix_madvise(p, nLength, POSIX_MADV_WILLNEED);
        if (e != 0)
            SAL_INFO("sal.file", "posix_madvise(..., POSIX_MADV_WILLNEED) failed with " << e);

#elif defined __sun
        if (madvise(static_cast< caddr_t >(p), nLength, MADV_WILLNEED) != 0)
            SAL_INFO("sal.file", "madvise(..., MADV_WILLNEED) failed with " << UnixErrnoString(errno));
#endif
    }

    return osl_File_E_None;
}

static oslFileError unmapFile(void* pAddr, sal_uInt64 uLength)
{
    if (!pAddr)
        return osl_File_E_INVAL;

    if (uLength > SAL_MAX_SIZE)
        return osl_File_E_OVERFLOW;

    size_t const nLength = sal::static_int_cast< size_t >(uLength);

    if (munmap(pAddr, nLength) == -1)
        return oslTranslateFileError(errno);

    return osl_File_E_None;
}

#ifndef ANDROID

// Note that osl_unmapFile() just won't work on Android in general
// where for (uncompressed) files inside the .apk, in the /assets
// folder osl_mapFile just returns a pointer to the file inside the
// already mmapped .apk archive.

oslFileError SAL_CALL osl_unmapFile(void* pAddr, sal_uInt64 uLength)
{
    return unmapFile(pAddr, uLength);
}

#endif

oslFileError SAL_CALL osl_unmapMappedFile(oslFileHandle Handle, void* pAddr, sal_uInt64 uLength)
{
    FileHandle_Impl *pImpl = static_cast<FileHandle_Impl*>(Handle);

    if (!pImpl)
        return osl_File_E_INVAL;

    if (pImpl->m_kind == FileHandle_Impl::KIND_FD)
        return unmapFile(pAddr, uLength);

    // For parts of already mmapped "parent" files, whose mapping we
    // can't change, not much we can or should do...
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_readLine(
    oslFileHandle   Handle,
    sal_Sequence ** ppSequence)
{
    FileHandle_Impl *pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)) || (!ppSequence))
        return osl_File_E_INVAL;

    sal_uInt64 uBytesRead = 0;

    // read at current fileptr; fileptr += uBytesRead;
    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));
    oslFileError result = pImpl->readLineAt(pImpl->m_fileptr, ppSequence, &uBytesRead);

    if (result == osl_File_E_None)
        pImpl->m_fileptr += uBytesRead;

    return result;
}

oslFileError SAL_CALL osl_readFile(
    oslFileHandle Handle,
    void *        pBuffer,
    sal_uInt64    uBytesRequested,
    sal_uInt64 *  pBytesRead)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)) || (!pBuffer) || (!pBytesRead))
        return osl_File_E_INVAL;

    static sal_uInt64 const g_limit_ssize_t = std::numeric_limits< ssize_t >::max();
    if (g_limit_ssize_t < uBytesRequested)
        return osl_File_E_OVERFLOW;

    size_t const nBytesRequested = sal::static_int_cast< size_t >(uBytesRequested);

    // read at current fileptr; fileptr += *pBytesRead;
    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));
    oslFileError result = pImpl->readFileAt(pImpl->m_fileptr, pBuffer, nBytesRequested, pBytesRead);

    if (result == osl_File_E_None)
        pImpl->m_fileptr += *pBytesRead;

    return result;
}

oslFileError SAL_CALL osl_writeFile(
    oslFileHandle Handle,
    const void *  pBuffer,
    sal_uInt64    uBytesToWrite,
    sal_uInt64 *  pBytesWritten)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || (pImpl->m_fd == -1) || (!pBuffer) || (!pBytesWritten))
        return osl_File_E_INVAL;

    if (!(pImpl->m_state & State::Writeable))
        return osl_File_E_BADF;

    static sal_uInt64 const g_limit_ssize_t = std::numeric_limits< ssize_t >::max();
    if (g_limit_ssize_t < uBytesToWrite)
        return osl_File_E_OVERFLOW;

    size_t const nBytesToWrite = sal::static_int_cast< size_t >(uBytesToWrite);

    // write at current fileptr; fileptr += *pBytesWritten;
    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));
    oslFileError result = pImpl->writeFileAt(pImpl->m_fileptr, pBuffer, nBytesToWrite, pBytesWritten);
    if (result == osl_File_E_None)
        pImpl->m_fileptr += *pBytesWritten;

    return result;
}

oslFileError SAL_CALL osl_readFileAt(
    oslFileHandle Handle,
    sal_uInt64    uOffset,
    void*         pBuffer,
    sal_uInt64    uBytesRequested,
    sal_uInt64*   pBytesRead)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)) || (!pBuffer) || (!pBytesRead))
        return osl_File_E_INVAL;

    if (!(pImpl->m_state & State::Seekable))
        return osl_File_E_SPIPE;

    if (exceedsMaxOffT(uOffset))
        return osl_File_E_OVERFLOW;

    off_t const nOffset = sal::static_int_cast< off_t >(uOffset);

    static sal_uInt64 const g_limit_ssize_t = std::numeric_limits< ssize_t >::max();
    if (g_limit_ssize_t < uBytesRequested)
        return osl_File_E_OVERFLOW;

    size_t const nBytesRequested = sal::static_int_cast< size_t >(uBytesRequested);

    // read at specified fileptr
    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));

    return pImpl->readFileAt(nOffset, pBuffer, nBytesRequested, pBytesRead);
}

oslFileError SAL_CALL osl_writeFileAt(
    oslFileHandle Handle,
    sal_uInt64    uOffset,
    const void*   pBuffer,
    sal_uInt64    uBytesToWrite,
    sal_uInt64*   pBytesWritten)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((!pImpl) || (pImpl->m_fd == -1) || (!pBuffer) || (!pBytesWritten))
        return osl_File_E_INVAL;

    if (!(pImpl->m_state & State::Seekable))
        return osl_File_E_SPIPE;

    if (!(pImpl->m_state & State::Writeable))
        return osl_File_E_BADF;

    if (exceedsMaxOffT(uOffset))
        return osl_File_E_OVERFLOW;

    off_t const nOffset = sal::static_int_cast< off_t >(uOffset);

    static sal_uInt64 const g_limit_ssize_t = std::numeric_limits< ssize_t >::max();
    if (g_limit_ssize_t < uBytesToWrite)
        return osl_File_E_OVERFLOW;

    size_t const nBytesToWrite = sal::static_int_cast< size_t >(uBytesToWrite);

    // write at specified fileptr
    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));

    return pImpl->writeFileAt(nOffset, pBuffer, nBytesToWrite, pBytesWritten);
}

oslFileError SAL_CALL osl_isEndOfFile(oslFileHandle Handle, sal_Bool *pIsEOF)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)) || (!pIsEOF))
        return osl_File_E_INVAL;

    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));
    *pIsEOF = (pImpl->getPos() == pImpl->getSize());

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_getFilePos(oslFileHandle Handle, sal_uInt64* pPos)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)) || (!pPos))
        return osl_File_E_INVAL;

    // no need to lock because pos is atomic
    *pPos = pImpl->getPos();

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_setFilePos(oslFileHandle Handle, sal_uInt32 uHow, sal_Int64 uOffset)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)))
        return osl_File_E_INVAL;

    if (exceedsMaxOffT(uOffset) || exceedsMinOffT(uOffset))
        return osl_File_E_OVERFLOW;

    off_t nPos = 0, nOffset = sal::static_int_cast< off_t >(uOffset);

    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));
    switch (uHow)
    {
        case osl_Pos_Absolut:
            if (nOffset < 0)
                return osl_File_E_INVAL;
            break;

        case osl_Pos_Current:
            nPos = sal::static_int_cast< off_t >(pImpl->getPos());
            if ((nOffset < 0) && (nPos < -1*nOffset))
                return osl_File_E_INVAL;

            assert(nPos >= 0);
            if (nOffset > MAX_OFF_T - nPos)
                return osl_File_E_OVERFLOW;
            break;

        case osl_Pos_End:
            nPos = sal::static_int_cast< off_t >(pImpl->getSize());
            if ((nOffset < 0) && (nPos < -1*nOffset))
                return osl_File_E_INVAL;

            assert(nPos >= 0);
            if (nOffset > MAX_OFF_T - nPos)
                return osl_File_E_OVERFLOW;
            break;

        default:
            return osl_File_E_INVAL;
    }

    pImpl->setPos(nPos + nOffset);
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_getFileSize(oslFileHandle Handle, sal_uInt64* pSize)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || ((pImpl->m_kind == FileHandle_Impl::KIND_FD) && (pImpl->m_fd == -1)) || (!pSize))
        return osl_File_E_INVAL;

    FileHandle_Impl::Guard lock(&(pImpl->m_mutex));
    *pSize = pImpl->getSize();

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_setFileSize(oslFileHandle Handle, sal_uInt64 uSize)
{
    FileHandle_Impl* pImpl = static_cast< FileHandle_Impl* >(Handle);

    if ((!pImpl) || (pImpl->m_fd == -1))
        return osl_File_E_INVAL;

    if (!(pImpl->m_state & State::Writeable))
        return osl_File_E_BADF;

    if (exceedsMaxOffT(uSize))
        return osl_File_E_OVERFLOW;

    oslFileError result = pImpl->syncFile();
    if (result != osl_File_E_None)
        return result;

    pImpl->m_bufptr = -1;
    pImpl->m_buflen = 0;

    return pImpl->setSize(uSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
