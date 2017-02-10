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

#define UNICODE
#define _UNICODE
#include "systools/win32/uwinapi.h"

#include "osl/file.hxx"

#include <file-impl.hxx>
#include "file_url.h"
#include "file_error.h"

#include "osl/diagnose.h"
#include "rtl/alloc.h"
#include "rtl/byteseq.h"
#include "rtl/ustring.hxx"

#include <stdio.h>
#include <tchar.h>

#include <algorithm>
#include <limits>

#ifdef max /* conflict w/ std::numeric_limits<T>::max() */
#undef max
#endif
#ifdef min
#undef min
#endif

// File handle implementation

struct FileHandle_Impl
{
    CRITICAL_SECTION m_mutex;
    HANDLE           m_hFile;

    /** State
     */
    enum StateBits
    {
        STATE_SEEKABLE  = 1, /* open() sets, iff regular file */
        STATE_READABLE  = 2, /* open() sets, read() requires */
        STATE_WRITEABLE = 4, /* open() sets, write() requires */
        STATE_MODIFIED  = 8  /* write() sets, flush() resets */
    };
    int          m_state;

    sal_uInt64   m_size;    /* file size */
    LONGLONG     m_offset;  /* physical offset from begin of file */
    LONGLONG     m_filepos; /* logical offset from begin of file */

    LONGLONG     m_bufptr;  /* buffer offset from begin of file */
    SIZE_T       m_buflen;  /* buffer filled [0, m_bufsiz - 1] */

    SIZE_T       m_bufsiz;
    sal_uInt8 *  m_buffer;

    explicit FileHandle_Impl (HANDLE hFile);
    ~FileHandle_Impl();

    static void*  operator new(size_t n);
    static void   operator delete(void * p, size_t);
    static SIZE_T getpagesize();

    sal_uInt64    getPos() const;
    oslFileError  setPos (sal_uInt64 uPos);

    sal_uInt64    getSize() const;
    oslFileError  setSize (sal_uInt64 uPos);

    oslFileError readAt (
        LONGLONG     nOffset,
        void *       pBuffer,
        DWORD        nBytesRequested,
        sal_uInt64 * pBytesRead);

    oslFileError writeAt (
        LONGLONG     nOffset,
        void const * pBuffer,
        DWORD        nBytesToWrite,
        sal_uInt64 * pBytesWritten);

    oslFileError readFileAt (
        LONGLONG     nOffset,
        void *       pBuffer,
        sal_uInt64   uBytesRequested,
        sal_uInt64 * pBytesRead);

    oslFileError writeFileAt (
        LONGLONG     nOffset,
        void const * pBuffer,
        sal_uInt64   uBytesToWrite,
        sal_uInt64 * pBytesWritten);

    oslFileError readLineAt (
        LONGLONG        nOffset,
        sal_Sequence ** ppSequence,
        sal_uInt64 *    pBytesRead);

    static oslFileError writeSequence_Impl (
        sal_Sequence ** ppSequence,
        SIZE_T *        pnOffset,
        const void *    pBuffer,
        SIZE_T          nBytes);

    oslFileError syncFile();

    /** Buffer cache / allocator.
     */
    class Allocator
    {
        rtl_cache_type * m_cache;
        SIZE_T           m_bufsiz;

        Allocator (Allocator const &) = delete;
        Allocator & operator= (Allocator const &) = delete;

    public:
        static Allocator & get();

        void allocate (sal_uInt8 ** ppBuffer, SIZE_T * pnSize);
        void deallocate (sal_uInt8 * pBuffer);

    protected:
        Allocator();
        ~Allocator();
    };

    /** Guard.
     */
    class Guard
    {
        LPCRITICAL_SECTION m_mutex;

    public:
        explicit Guard(LPCRITICAL_SECTION pMutex);
        ~Guard();
    };
};

FileHandle_Impl::Allocator &
FileHandle_Impl::Allocator::get()
{
    static Allocator g_aBufferAllocator;
    return g_aBufferAllocator;
}

FileHandle_Impl::Allocator::Allocator()
    : m_cache  (nullptr),
      m_bufsiz (0)
{
    SIZE_T const pagesize = FileHandle_Impl::getpagesize();
    m_cache = rtl_cache_create (
        "osl_file_buffer_cache", pagesize, 0, nullptr, nullptr, nullptr,
        nullptr, nullptr, 0);
    if (nullptr != m_cache)
        m_bufsiz = pagesize;
}

FileHandle_Impl::Allocator::~Allocator()
{
    rtl_cache_destroy(m_cache);
    m_cache = nullptr;
}

void FileHandle_Impl::Allocator::allocate (sal_uInt8 ** ppBuffer, SIZE_T * pnSize)
{
    OSL_PRECOND((nullptr != ppBuffer) && (nullptr != pnSize), "FileHandle_Impl::Allocator::allocate(): contract violation");
    *ppBuffer = static_cast< sal_uInt8* >(rtl_cache_alloc(m_cache));
    *pnSize = m_bufsiz;
}

void FileHandle_Impl::Allocator::deallocate (sal_uInt8 * pBuffer)
{
    if (nullptr != pBuffer)
        rtl_cache_free (m_cache, pBuffer);
}

FileHandle_Impl::Guard::Guard(LPCRITICAL_SECTION pMutex)
    : m_mutex (pMutex)
{
    OSL_PRECOND (m_mutex != nullptr, "FileHandle_Impl::Guard::Guard(): null pointer.");
    ::EnterCriticalSection (m_mutex);
}
FileHandle_Impl::Guard::~Guard()
{
    OSL_PRECOND (m_mutex != nullptr, "FileHandle_Impl::Guard::~Guard(): null pointer.");
    ::LeaveCriticalSection (m_mutex);
}

FileHandle_Impl::FileHandle_Impl(HANDLE hFile)
    : m_hFile   (hFile),
      m_state   (STATE_READABLE | STATE_WRITEABLE),
      m_size    (0),
      m_offset  (0),
      m_filepos (0),
      m_bufptr  (-1),
      m_buflen  (0),
      m_bufsiz  (0),
      m_buffer  (nullptr)
{
    ::InitializeCriticalSection (&m_mutex);
    Allocator::get().allocate (&m_buffer, &m_bufsiz);
    if (m_buffer != nullptr)
        memset (m_buffer, 0, m_bufsiz);
}

FileHandle_Impl::~FileHandle_Impl()
{
    Allocator::get().deallocate (m_buffer);
    m_buffer = nullptr;
    ::DeleteCriticalSection (&m_mutex);
}

void * FileHandle_Impl::operator new(size_t n)
{
    return rtl_allocateMemory(n);
}

void FileHandle_Impl::operator delete(void * p, size_t)
{
    rtl_freeMemory(p);
}

SIZE_T FileHandle_Impl::getpagesize()
{
    SYSTEM_INFO info;
    ::GetSystemInfo (&info);
    return sal::static_int_cast< SIZE_T >(info.dwPageSize);
}

sal_uInt64 FileHandle_Impl::getPos() const
{
    return sal::static_int_cast< sal_uInt64 >(m_filepos);
}

oslFileError FileHandle_Impl::setPos (sal_uInt64 uPos)
{
    m_filepos = sal::static_int_cast< LONGLONG >(uPos);
    return osl_File_E_None;
}

sal_uInt64 FileHandle_Impl::getSize() const
{
    LONGLONG bufend = std::max((LONGLONG)(0), m_bufptr) + m_buflen;
    return std::max(m_size, sal::static_int_cast< sal_uInt64 >(bufend));
}

oslFileError FileHandle_Impl::setSize (sal_uInt64 uSize)
{
    LARGE_INTEGER nDstPos; nDstPos.QuadPart = sal::static_int_cast< LONGLONG >(uSize);
    if (!::SetFilePointerEx(m_hFile, nDstPos, nullptr, FILE_BEGIN))
        return oslTranslateFileError( GetLastError() );

    if (!::SetEndOfFile(m_hFile))
        return oslTranslateFileError( GetLastError() );
    m_size = uSize;

    nDstPos.QuadPart = m_offset;
    if (!::SetFilePointerEx(m_hFile, nDstPos, nullptr, FILE_BEGIN))
        return oslTranslateFileError( GetLastError() );

    return osl_File_E_None;
}

oslFileError FileHandle_Impl::readAt (
    LONGLONG     nOffset,
    void *       pBuffer,
    DWORD        nBytesRequested,
    sal_uInt64 * pBytesRead)
{
    OSL_PRECOND(m_state & STATE_SEEKABLE, "FileHandle_Impl::readAt(): not seekable");
    if (!(m_state & STATE_SEEKABLE))
        return osl_File_E_SPIPE;

    OSL_PRECOND(m_state & STATE_READABLE, "FileHandle_Impl::readAt(): not readable");
    if (!(m_state & STATE_READABLE))
        return osl_File_E_BADF;

    if (nOffset != m_offset)
    {
        LARGE_INTEGER liOffset; liOffset.QuadPart = nOffset;
        if (!::SetFilePointerEx(m_hFile, liOffset, nullptr, FILE_BEGIN))
            return oslTranslateFileError( GetLastError() );
        m_offset = nOffset;
    }

    DWORD dwDone = 0;
    if (!::ReadFile(m_hFile, pBuffer, nBytesRequested, &dwDone, nullptr))
        return oslTranslateFileError( GetLastError() );
    m_offset += dwDone;

    *pBytesRead = dwDone;
    return osl_File_E_None;
}

oslFileError FileHandle_Impl::writeAt (
    LONGLONG     nOffset,
    void const * pBuffer,
    DWORD        nBytesToWrite,
    sal_uInt64 * pBytesWritten)
{
    OSL_PRECOND(m_state & STATE_SEEKABLE, "FileHandle_Impl::writeAt(): not seekable");
    if (!(m_state & STATE_SEEKABLE))
        return osl_File_E_SPIPE;

    OSL_PRECOND(m_state & STATE_WRITEABLE, "FileHandle_Impl::writeAt(): not writeable");
    if (!(m_state & STATE_WRITEABLE))
        return osl_File_E_BADF;

    if (nOffset != m_offset)
    {
        LARGE_INTEGER liOffset; liOffset.QuadPart = nOffset;
        if (!::SetFilePointerEx (m_hFile, liOffset, nullptr, FILE_BEGIN))
            return oslTranslateFileError( GetLastError() );
        m_offset = nOffset;
    }

    DWORD dwDone = 0;
    if (!::WriteFile(m_hFile, pBuffer, nBytesToWrite, &dwDone, nullptr))
        return oslTranslateFileError( GetLastError() );
    m_offset += dwDone;

    m_size = std::max(m_size, sal::static_int_cast< sal_uInt64 >(m_offset));

    *pBytesWritten = dwDone;
    return osl_File_E_None;
}

oslFileError FileHandle_Impl::readFileAt (
    LONGLONG     nOffset,
    void *       pBuffer,
    sal_uInt64   uBytesRequested,
    sal_uInt64 * pBytesRead)
{
    static sal_uInt64 const g_limit_dword = std::numeric_limits< DWORD >::max();
    if (g_limit_dword < uBytesRequested)
        return osl_File_E_OVERFLOW;
    DWORD nBytesRequested = sal::static_int_cast< DWORD >(uBytesRequested);

    if (0 == (m_state & STATE_SEEKABLE))
    {
        // not seekable (pipe)
        DWORD dwDone = 0;
        if (!::ReadFile(m_hFile, pBuffer, nBytesRequested, &dwDone, nullptr))
            return oslTranslateFileError( GetLastError() );
        *pBytesRead = dwDone;
        return osl_File_E_None;
    }
    else if (nullptr == m_buffer)
    {
        // not buffered
        return readAt (nOffset, pBuffer, nBytesRequested, pBytesRead);
    }
    else
    {
        sal_uInt8 * buffer = static_cast< sal_uInt8* >(pBuffer);
        for (*pBytesRead = 0; nBytesRequested > 0; )
        {
            LONGLONG const bufptr = (nOffset / m_bufsiz) * m_bufsiz;
            SIZE_T   const bufpos = (nOffset % m_bufsiz);

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
                    result = readAt (nOffset, &(buffer[*pBytesRead]), nBytesRequested, &uDone);
                    if (result != osl_File_E_None)
                        return result;

                    nBytesRequested -= sal::static_int_cast< DWORD >(uDone);
                    *pBytesRead += uDone;
                    return osl_File_E_None;
                }

                // update buffer (pointer)
                sal_uInt64 uDone = 0;
                result = readAt (bufptr, m_buffer, m_bufsiz, &uDone);
                if (result != osl_File_E_None)
                    return result;
                m_bufptr = bufptr;
                m_buflen = sal::static_int_cast< SIZE_T >(uDone);
            }
            if (bufpos >= m_buflen)
            {
                // end of file
                return osl_File_E_None;
            }

            SIZE_T const bytes = std::min(m_buflen - bufpos, (SIZE_T) nBytesRequested);
            memcpy (&(buffer[*pBytesRead]), &(m_buffer[bufpos]), bytes);
            nBytesRequested -= bytes;
            *pBytesRead += bytes;
            nOffset += bytes;
        }
        return osl_File_E_None;
    }
}

oslFileError FileHandle_Impl::writeFileAt (
    LONGLONG     nOffset,
    void const * pBuffer,
    sal_uInt64   uBytesToWrite,
    sal_uInt64 * pBytesWritten)
{
    static sal_uInt64 const g_limit_dword = std::numeric_limits< DWORD >::max();
    if (g_limit_dword < uBytesToWrite)
        return osl_File_E_OVERFLOW;
    DWORD nBytesToWrite = sal::static_int_cast< DWORD >(uBytesToWrite);

    if (0 == (m_state & STATE_SEEKABLE))
    {
        // not seekable (pipe)
        DWORD dwDone = 0;
        if (!::WriteFile(m_hFile, pBuffer, nBytesToWrite, &dwDone, nullptr))
            return oslTranslateFileError( GetLastError() );
        *pBytesWritten = dwDone;
        return osl_File_E_None;
    }
    else if (nullptr == m_buffer)
    {
        // not buffered
        return writeAt(nOffset, pBuffer, nBytesToWrite, pBytesWritten);
    }
    else
    {
        sal_uInt8 const * buffer = static_cast< sal_uInt8 const* >(pBuffer);
        for (*pBytesWritten = 0; nBytesToWrite > 0; )
        {
            LONGLONG const bufptr = (nOffset / m_bufsiz) * m_bufsiz;
            SIZE_T   const bufpos = (nOffset % m_bufsiz);
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
                    result = writeAt (nOffset, &(buffer[*pBytesWritten]), nBytesToWrite, &uDone);
                    if (result != osl_File_E_None)
                        return result;
                    if (uDone != nBytesToWrite)
                        return osl_File_E_IO;

                    nBytesToWrite -= sal::static_int_cast< DWORD >(uDone);
                    *pBytesWritten += uDone;
                    return osl_File_E_None;
                }

                // update buffer (pointer)
                sal_uInt64 uDone = 0;
                result = readAt (bufptr, m_buffer, m_bufsiz, &uDone);
                if (result != osl_File_E_None)
                    return result;
                m_bufptr = bufptr;
                m_buflen = sal::static_int_cast< SIZE_T >(uDone);
            }

            SIZE_T const bytes = std::min(m_bufsiz - bufpos, (SIZE_T) nBytesToWrite);
            memcpy (&(m_buffer[bufpos]), &(buffer[*pBytesWritten]), bytes);
            nBytesToWrite -= bytes;
            *pBytesWritten += bytes;
            nOffset += bytes;

            m_buflen = std::max(m_buflen, bufpos + bytes);
            m_state |= STATE_MODIFIED;
        }
        return osl_File_E_None;
    }
}

oslFileError FileHandle_Impl::readLineAt (
    LONGLONG        nOffset,
    sal_Sequence ** ppSequence,
    sal_uInt64 *    pBytesRead)
{
    oslFileError result = osl_File_E_None;

    LONGLONG bufptr = (nOffset / m_bufsiz) * m_bufsiz;
    if (bufptr != m_bufptr)
    {
        /* flush current buffer */
        result = syncFile();
        if (result != osl_File_E_None)
            return result;

        /* update buffer (pointer) */
        sal_uInt64 uDone = 0;
        result = readAt (bufptr, m_buffer, m_bufsiz, &uDone);
        if (result != osl_File_E_None)
            return result;

        m_bufptr = bufptr;
        m_buflen = sal::static_int_cast< SIZE_T >(uDone);
    }

    static int const LINE_STATE_BEGIN = 0;
    static int const LINE_STATE_CR    = 1;
    static int const LINE_STATE_LF    = 2;

    SIZE_T bufpos = sal::static_int_cast< SIZE_T >(nOffset - m_bufptr), curpos = bufpos, dstpos = 0;
    int    state  = (bufpos >= m_buflen) ? LINE_STATE_LF : LINE_STATE_BEGIN;

    for ( ; state != LINE_STATE_LF; )
    {
        if (curpos >= m_buflen)
        {
            /* buffer examined */
            if (0 < (curpos - bufpos))
            {
                /* flush buffer to sequence */
                result = writeSequence_Impl (
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
                result = readAt (bufptr, m_buffer, m_bufsiz, &uDone);
                if (result != osl_File_E_None)
                    return result;
                m_bufptr = bufptr;
                m_buflen = sal::static_int_cast< SIZE_T >(uDone);
            }

            bufpos = sal::static_int_cast< SIZE_T >(nOffset - m_bufptr);
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
                /* store (and eat) the newline char */
                m_buffer[curpos] = 0x0A;
                curpos++;

                /* flush buffer to sequence */
                result = writeSequence_Impl (
                    ppSequence, &dstpos, &(m_buffer[bufpos]), curpos - bufpos - 1);
                if (result != osl_File_E_None)
                    return result;
                *pBytesRead += curpos - bufpos;
                nOffset += curpos - bufpos;
            }
            break;
        }
    }

    result = writeSequence_Impl (ppSequence, &dstpos, nullptr, 0);
    if (result != osl_File_E_None)
        return result;
    if (0 < dstpos)
        return osl_File_E_None;
    if (bufpos >= m_buflen)
        return osl_File_E_AGAIN;
    return osl_File_E_None;
}

oslFileError FileHandle_Impl::writeSequence_Impl (
    sal_Sequence ** ppSequence,
    SIZE_T *        pnOffset,
    const void *    pBuffer,
    SIZE_T          nBytes)
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
    if (*ppSequence != nullptr)
    {
        /* fill sequence */
        memcpy(&((*ppSequence)->elements[*pnOffset]), pBuffer, nBytes);
        *pnOffset += nBytes;
    }
    return (*ppSequence != nullptr) ? osl_File_E_None : osl_File_E_NOMEM;
}

oslFileError FileHandle_Impl::syncFile()
{
    oslFileError result = osl_File_E_None;
    if (m_state & STATE_MODIFIED)
    {
        sal_uInt64 uDone = 0;
        result = writeAt (m_bufptr, m_buffer, m_buflen, &uDone);
        if (result != osl_File_E_None)
            return result;
        if (uDone != m_buflen)
            return osl_File_E_IO;
        m_state &= ~STATE_MODIFIED;
    }
    return result;
}

// File I/O functions

extern "C" oslFileHandle
SAL_CALL osl_createFileHandleFromOSHandle (
    HANDLE     hFile,
    sal_uInt32 uFlags)
{
    if ( !IsValidHandle(hFile) )
        return nullptr; // EINVAL

    FileHandle_Impl * pImpl = new FileHandle_Impl(hFile);
    if (pImpl == nullptr)
    {
        // cleanup and fail
        (void) ::CloseHandle(hFile);
        return nullptr; // ENOMEM
    }

    /* check for regular file */
    if (FILE_TYPE_DISK == GetFileType(hFile))
    {
        /* mark seekable */
        pImpl->m_state |= FileHandle_Impl::STATE_SEEKABLE;

        /* init current size */
        LARGE_INTEGER uSize = { { 0, 0 } };
        (void) ::GetFileSizeEx(hFile, &uSize);
        pImpl->m_size = (sal::static_int_cast<sal_uInt64>(uSize.HighPart) << 32) + uSize.LowPart;
    }

    if (!(uFlags & osl_File_OpenFlag_Read))
        pImpl->m_state &= ~FileHandle_Impl::STATE_READABLE;
    if (!(uFlags & osl_File_OpenFlag_Write))
        pImpl->m_state &= ~FileHandle_Impl::STATE_WRITEABLE;

    SAL_WARN_IF(
        !((uFlags & osl_File_OpenFlag_Read) || (uFlags & osl_File_OpenFlag_Write)),
        "sal.osl",
        "osl_createFileHandleFromOSHandle(): missing read/write access flags");
    return static_cast<oslFileHandle>(pImpl);
}

oslFileError
SAL_CALL osl_openFile(
    rtl_uString *   strPath,
    oslFileHandle * pHandle,
    sal_uInt32      uFlags )
{
    rtl_uString * strSysPath = nullptr;
    oslFileError result = osl_getSystemPathFromFileURL_( strPath, &strSysPath, false );
    if (result != osl_File_E_None)
        return result;

    DWORD dwAccess = GENERIC_READ, dwShare = FILE_SHARE_READ, dwCreation = 0, dwAttributes = 0;

    if ( uFlags & osl_File_OpenFlag_Write )
        dwAccess |= GENERIC_WRITE;
    else
        dwShare  |= FILE_SHARE_WRITE;

    if ( uFlags & osl_File_OpenFlag_NoLock )
        dwShare  |= FILE_SHARE_WRITE;

    if ( uFlags & osl_File_OpenFlag_Create )
        dwCreation |= CREATE_NEW;
    else
        dwCreation |= OPEN_EXISTING;

    HANDLE hFile = CreateFileW(
        rtl_uString_getStr( strSysPath ),
        dwAccess, dwShare, nullptr, dwCreation, dwAttributes, nullptr );

    // @@@ ERROR HANDLING @@@
    if ( !IsValidHandle( hFile ) )
        result = oslTranslateFileError( GetLastError() );

    *pHandle = osl_createFileHandleFromOSHandle (hFile, uFlags | osl_File_OpenFlag_Read);

    rtl_uString_release( strSysPath );
    return result;
}

oslFileError
SAL_CALL osl_syncFile(oslFileHandle Handle)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);
    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile))
        return osl_File_E_INVAL;

    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));

    oslFileError result = pImpl->syncFile();
    if (result != osl_File_E_None)
        return result;

    if (!FlushFileBuffers(pImpl->m_hFile))
        return oslTranslateFileError(GetLastError());

    return osl_File_E_None;
}

oslFileError
SAL_CALL osl_closeFile(oslFileHandle Handle)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);
    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile))
        return osl_File_E_INVAL;

    ::EnterCriticalSection (&(pImpl->m_mutex));

    oslFileError result = pImpl->syncFile();
    if (result != osl_File_E_None)
    {
        /* ignore double failure */
        (void)::CloseHandle(pImpl->m_hFile);
    }
    else if (!::CloseHandle(pImpl->m_hFile))
    {
        /* translate error code */
        result = oslTranslateFileError( GetLastError() );
    }

    ::LeaveCriticalSection (&(pImpl->m_mutex));
    delete pImpl;
    return result;
}

oslFileError
SAL_CALL osl_mapFile(
    oslFileHandle Handle,
    void**        ppAddr,
    sal_uInt64    uLength,
    sal_uInt64    uOffset,
    sal_uInt32    uFlags)
{
    struct FileMapping
    {
        HANDLE m_handle;

        explicit FileMapping (HANDLE hMap)
            : m_handle (hMap)
        {}

        ~FileMapping()
        {
            (void)::CloseHandle(m_handle);
        }
    };

    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);
    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == ppAddr))
        return osl_File_E_INVAL;
    *ppAddr = nullptr;

    static SIZE_T const nLimit = std::numeric_limits< SIZE_T >::max();
    if (uLength > nLimit)
        return osl_File_E_OVERFLOW;
    SIZE_T const nLength = sal::static_int_cast< SIZE_T >(uLength);

    FileMapping aMap( ::CreateFileMapping (pImpl->m_hFile, nullptr, SEC_COMMIT | PAGE_READONLY, 0, 0, nullptr) );
    if (!IsValidHandle(aMap.m_handle))
        return oslTranslateFileError( GetLastError() );

    DWORD const dwOffsetHi = sal::static_int_cast<DWORD>(uOffset >> 32);
    DWORD const dwOffsetLo = sal::static_int_cast<DWORD>(uOffset & 0xFFFFFFFF);

    *ppAddr = ::MapViewOfFile( aMap.m_handle, FILE_MAP_READ, dwOffsetHi, dwOffsetLo, nLength );
    if (nullptr == *ppAddr)
        return oslTranslateFileError( GetLastError() );

    if (uFlags & osl_File_MapFlag_RandomAccess)
    {
        // Determine memory pagesize.
        SYSTEM_INFO info;
        ::GetSystemInfo( &info );
        DWORD const dwPageSize = info.dwPageSize;

        /*
         * Pagein, touching first byte of each memory page.
         * Note: volatile disables optimizing the loop away.
         */
        BYTE * pData (static_cast<BYTE*>(*ppAddr));
        SIZE_T nSize (nLength);

        volatile BYTE c = 0;
        while (nSize > dwPageSize)
        {
            c ^= pData[0];
            pData += dwPageSize;
            nSize -= dwPageSize;
        }
        if (nSize > 0)
        {
            c ^= pData[0];
        }
    }
    return osl_File_E_None;
}

oslFileError
SAL_CALL osl_unmapFile(void* pAddr, sal_uInt64 /* uLength */)
{
    if (nullptr == pAddr)
        return osl_File_E_INVAL;

    if (!::UnmapViewOfFile (pAddr))
        return oslTranslateFileError( GetLastError() );

    return osl_File_E_None;
}

oslFileError
SAL_CALL osl_unmapMappedFile(oslFileHandle /* Handle */, void* pAddr, sal_uInt64 uLength)
{
    return osl_unmapFile( pAddr, uLength );
}

oslFileError
SAL_CALL osl_readLine(
    oslFileHandle   Handle,
    sal_Sequence ** ppSequence)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);
    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == ppSequence))
        return osl_File_E_INVAL;
    sal_uInt64 uBytesRead = 0;

    // read at current filepos; filepos += uBytesRead;
    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    oslFileError result = pImpl->readLineAt (
        pImpl->m_filepos, ppSequence, &uBytesRead);
    if (result == osl_File_E_None)
        pImpl->m_filepos += uBytesRead;
    return result;
}

oslFileError
SAL_CALL osl_readFile(
    oslFileHandle Handle,
    void *        pBuffer,
    sal_uInt64    uBytesRequested,
    sal_uInt64 *  pBytesRead)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);
    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == pBuffer) || (nullptr == pBytesRead))
        return osl_File_E_INVAL;

    // read at current filepos; filepos += *pBytesRead;
    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    oslFileError result = pImpl->readFileAt (
        pImpl->m_filepos, pBuffer, uBytesRequested, pBytesRead);
    if (result == osl_File_E_None)
        pImpl->m_filepos += *pBytesRead;
    return result;
}

oslFileError
SAL_CALL osl_writeFile(
    oslFileHandle Handle,
    const void *  pBuffer,
    sal_uInt64    uBytesToWrite,
    sal_uInt64 *  pBytesWritten )
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == pBuffer) || (nullptr == pBytesWritten))
        return osl_File_E_INVAL;

    // write at current filepos; filepos += *pBytesWritten;
    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    oslFileError result = pImpl->writeFileAt (
        pImpl->m_filepos, pBuffer, uBytesToWrite, pBytesWritten);
    if (result == osl_File_E_None)
        pImpl->m_filepos += *pBytesWritten;
    return result;
}

oslFileError
SAL_CALL osl_readFileAt(
    oslFileHandle Handle,
    sal_uInt64    uOffset,
    void*         pBuffer,
    sal_uInt64    uBytesRequested,
    sal_uInt64*   pBytesRead)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == pBuffer) || (nullptr == pBytesRead))
        return osl_File_E_INVAL;
    if (0 == (pImpl->m_state & FileHandle_Impl::STATE_SEEKABLE))
        return osl_File_E_SPIPE;

    static sal_uInt64 const g_limit_longlong = std::numeric_limits< LONGLONG >::max();
    if (g_limit_longlong < uOffset)
        return osl_File_E_OVERFLOW;
    LONGLONG const nOffset = sal::static_int_cast< LONGLONG >(uOffset);

    // read at specified fileptr
    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    return pImpl->readFileAt (nOffset, pBuffer, uBytesRequested, pBytesRead);
}

oslFileError
SAL_CALL osl_writeFileAt(
    oslFileHandle Handle,
    sal_uInt64    uOffset,
    const void*   pBuffer,
    sal_uInt64    uBytesToWrite,
    sal_uInt64*   pBytesWritten)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == pBuffer) || (nullptr == pBytesWritten))
        return osl_File_E_INVAL;
    if (0 == (pImpl->m_state & FileHandle_Impl::STATE_SEEKABLE))
        return osl_File_E_SPIPE;

    static sal_uInt64 const g_limit_longlong = std::numeric_limits< LONGLONG >::max();
    if (g_limit_longlong < uOffset)
        return osl_File_E_OVERFLOW;
    LONGLONG const nOffset = sal::static_int_cast< LONGLONG >(uOffset);

    // write at specified fileptr
    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    return pImpl->writeFileAt (nOffset, pBuffer, uBytesToWrite, pBytesWritten);
}

oslFileError
SAL_CALL osl_isEndOfFile (oslFileHandle Handle, sal_Bool *pIsEOF)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == pIsEOF))
        return osl_File_E_INVAL;

    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    *pIsEOF = (pImpl->getPos() == pImpl->getSize());
    return osl_File_E_None;
}

oslFileError
SAL_CALL osl_getFilePos(oslFileHandle Handle, sal_uInt64 *pPos)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);
    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == pPos))
        return osl_File_E_INVAL;

    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    *pPos = pImpl->getPos();
    return osl_File_E_None;
}

oslFileError
SAL_CALL osl_setFilePos(oslFileHandle Handle, sal_uInt32 uHow, sal_Int64 uOffset)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);
    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile))
        return osl_File_E_INVAL;

    static sal_Int64 const g_limit_longlong = std::numeric_limits< LONGLONG >::max();
    if (g_limit_longlong < uOffset)
        return osl_File_E_OVERFLOW;
    LONGLONG nPos = 0, nOffset = sal::static_int_cast< LONGLONG >(uOffset);

    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    switch (uHow)
    {
        case osl_Pos_Absolut:
            if (0 > nOffset)
                return osl_File_E_INVAL;
            break;

        case osl_Pos_Current:
            nPos = sal::static_int_cast< LONGLONG >(pImpl->getPos());
            if ((0 > nOffset) && (-1*nOffset > nPos))
                return osl_File_E_INVAL;
            if (g_limit_longlong < nPos + nOffset)
                return osl_File_E_OVERFLOW;
            break;

        case osl_Pos_End:
            nPos = sal::static_int_cast< LONGLONG >(pImpl->getSize());
            if ((0 > nOffset) && (-1*nOffset > nPos))
                return osl_File_E_INVAL;
            if (g_limit_longlong < nPos + nOffset)
                return osl_File_E_OVERFLOW;
            break;

        default:
            return osl_File_E_INVAL;
    }

    return pImpl->setPos (nPos + nOffset);
}

oslFileError
SAL_CALL osl_getFileSize (oslFileHandle Handle, sal_uInt64 *pSize)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile) || (nullptr == pSize))
        return osl_File_E_INVAL;

    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    *pSize = pImpl->getSize();
    return osl_File_E_None;
}

oslFileError
SAL_CALL osl_setFileSize (oslFileHandle Handle, sal_uInt64 uSize)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((nullptr == pImpl) || !IsValidHandle(pImpl->m_hFile))
        return osl_File_E_INVAL;
    if (0 == (pImpl->m_state & FileHandle_Impl::STATE_WRITEABLE))
        return osl_File_E_BADF;

    static sal_uInt64 const g_limit_longlong = std::numeric_limits< LONGLONG >::max();
    if (g_limit_longlong < uSize)
        return osl_File_E_OVERFLOW;

    FileHandle_Impl::Guard lock (&(pImpl->m_mutex));
    oslFileError result = pImpl->syncFile();
    if (result != osl_File_E_None)
        return result;
    pImpl->m_bufptr = -1;
    pImpl->m_buflen = 0;

    return pImpl->setSize (uSize);
}

// File handling functions

oslFileError SAL_CALL osl_removeFile( rtl_uString* strPath )
{
    rtl_uString *strSysPath = nullptr;
    oslFileError    error = osl_getSystemPathFromFileURL_( strPath, &strSysPath, false );

    if ( osl_File_E_None == error )
    {
        if ( DeleteFile( rtl_uString_getStr( strSysPath ) ) )
            error = osl_File_E_None;
        else
            error = oslTranslateFileError( GetLastError() );

        rtl_uString_release( strSysPath );
    }
    return error;
}

oslFileError SAL_CALL osl_copyFile( rtl_uString* strPath, rtl_uString *strDestPath )
{
    rtl_uString *strSysPath = nullptr, *strSysDestPath = nullptr;
    oslFileError    error = osl_getSystemPathFromFileURL_( strPath, &strSysPath, false );

    if ( osl_File_E_None == error )
        error = osl_getSystemPathFromFileURL_( strDestPath, &strSysDestPath, false );

    if ( osl_File_E_None == error )
    {
        LPCWSTR src = rtl_uString_getStr( strSysPath );
        LPCWSTR dst = rtl_uString_getStr( strSysDestPath );

        if ( CopyFileW( src, dst, FALSE ) )
            error = osl_File_E_None;
        else
            error = oslTranslateFileError( GetLastError() );
    }

    if ( strSysPath )
        rtl_uString_release( strSysPath );
    if ( strSysDestPath )
        rtl_uString_release( strSysDestPath );

    return error;
}

oslFileError SAL_CALL osl_moveFile( rtl_uString* strPath, rtl_uString *strDestPath )
{
    rtl_uString *strSysPath = nullptr, *strSysDestPath = nullptr;
    oslFileError    error = osl_getSystemPathFromFileURL_( strPath, &strSysPath, false );

    if ( osl_File_E_None == error )
        error = osl_getSystemPathFromFileURL_( strDestPath, &strSysDestPath, false );

    if ( osl_File_E_None == error )
    {
        LPCWSTR src = rtl_uString_getStr( strSysPath );
        LPCWSTR dst = rtl_uString_getStr( strSysDestPath );

        if ( MoveFileEx( src, dst, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING ) )
            error = osl_File_E_None;
        else
            error = oslTranslateFileError( GetLastError() );
    }

    if ( strSysPath )
        rtl_uString_release( strSysPath );
    if ( strSysDestPath )
        rtl_uString_release( strSysDestPath );

    return error;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
