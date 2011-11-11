/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




/************************************************************************
 *   ToDo
 *
 *   Fix osl_getCanonicalName
 *
 *   - Fix: check for corresponding struct sizes in exported functions
 *   - check size/use of oslDirectory
 *   - check size/use of oslDirectoryItem
 *   - check size/use of oslFileStatus
 *   - check size/use of oslVolumeDeviceHandle
 *   - check size/use of oslVolumeInfo
 *   - check size/use of oslFileHandle
 ***********************************************************************/

#define INCL_DOSDEVIOCTL                        // OS2 device definitions

#include "system.h"
#include <rtl/alloc.h>

#include "osl/file.hxx"


#include <sal/types.h>
#include <osl/thread.h>
#include <osl/diagnose.h>
#include "file_error_transl.h"
#include <osl/time.h>

#ifndef _FILE_URL_H_
#include "file_url.h"
#endif

#include "file_path_helper.hxx"
#include "uunxapi.hxx"

#ifndef _STRING_H_
#include <string.h>
#endif

#ifndef _CTYPE_H_
#include <ctype.h>
#endif

#ifndef _WCHAR_H_
#include <wchar.h>
#endif

#include <algorithm>

#include <limits>
#include <sys/mman.h>

#if OSL_DEBUG_LEVEL > 1
    extern void debug_ustring(rtl_uString*);
#endif


#ifdef DEBUG_OSL_FILE
#   define PERROR( a, b ) perror( a ); fprintf( stderr, b )
#else
#   define PERROR( a, b )
#endif

extern "C" oslFileHandle osl_createFileHandleFromFD( int fd );

    struct errentry errtable[] = {
        {  NO_ERROR,             osl_File_E_None     },  /* 0 */
        {  ERROR_INVALID_FUNCTION,       osl_File_E_INVAL    },  /* 1 */
        {  ERROR_FILE_NOT_FOUND,         osl_File_E_NOENT    },  /* 2 */
        {  ERROR_PATH_NOT_FOUND,         osl_File_E_NOENT    },  /* 3 */
        {  ERROR_TOO_MANY_OPEN_FILES,    osl_File_E_MFILE    },  /* 4 */
        {  ERROR_ACCESS_DENIED,          osl_File_E_ACCES    },  /* 5 */
        {  ERROR_INVALID_HANDLE,         osl_File_E_BADF     },  /* 6 */
        {  ERROR_ARENA_TRASHED,          osl_File_E_NOMEM    },  /* 7 */
        {  ERROR_NOT_ENOUGH_MEMORY,      osl_File_E_NOMEM    },  /* 8 */
        {  ERROR_INVALID_BLOCK,          osl_File_E_NOMEM    },  /* 9 */
        {  ERROR_BAD_ENVIRONMENT,        osl_File_E_2BIG     },  /* 10 */
        {  ERROR_BAD_FORMAT,             osl_File_E_NOEXEC   },  /* 11 */
        {  ERROR_INVALID_ACCESS,         osl_File_E_INVAL    },  /* 12 */
        {  ERROR_INVALID_DATA,           osl_File_E_INVAL    },  /* 13 */
        {  ERROR_INVALID_DRIVE,          osl_File_E_NOENT    },  /* 15 */
        {  ERROR_CURRENT_DIRECTORY,      osl_File_E_ACCES    },  /* 16 */
        {  ERROR_NOT_SAME_DEVICE,        osl_File_E_XDEV     },  /* 17 */
        {  ERROR_NO_MORE_FILES,          osl_File_E_NOENT    },  /* 18 */
        {  ERROR_NOT_READY,              osl_File_E_NOTREADY },  /* 21 */
        {  ERROR_LOCK_VIOLATION,         osl_File_E_ACCES    },  /* 33 */
        {  ERROR_BAD_NETPATH,            osl_File_E_NOENT    },  /* 53 */
        {  ERROR_NETWORK_ACCESS_DENIED,  osl_File_E_ACCES    },  /* 65 */
        {  ERROR_BAD_NET_NAME,           osl_File_E_NOENT    },  /* 67 */
        {  ERROR_FILE_EXISTS,            osl_File_E_EXIST    },  /* 80 */
        {  ERROR_CANNOT_MAKE,            osl_File_E_ACCES    },  /* 82 */
        {  ERROR_FAIL_I24,               osl_File_E_ACCES    },  /* 83 */
        {  ERROR_INVALID_PARAMETER,      osl_File_E_INVAL    },  /* 87 */
        {  ERROR_NO_PROC_SLOTS,          osl_File_E_AGAIN    },  /* 89 */
        {  ERROR_DRIVE_LOCKED,           osl_File_E_ACCES    },  /* 108 */
        {  ERROR_BROKEN_PIPE,            osl_File_E_PIPE     },  /* 109 */
        {  ERROR_DISK_FULL,              osl_File_E_NOSPC    },  /* 112 */
        {  ERROR_INVALID_TARGET_HANDLE,  osl_File_E_BADF     },  /* 114 */
        {  ERROR_INVALID_HANDLE,         osl_File_E_INVAL    },  /* 124 */
        {  ERROR_WAIT_NO_CHILDREN,       osl_File_E_CHILD    },  /* 128 */
        {  ERROR_CHILD_NOT_COMPLETE,     osl_File_E_CHILD    },  /* 129 */
        {  ERROR_DIRECT_ACCESS_HANDLE,   osl_File_E_BADF     },  /* 130 */
        {  ERROR_NEGATIVE_SEEK,          osl_File_E_INVAL    },  /* 131 */
        {  ERROR_SEEK_ON_DEVICE,         osl_File_E_ACCES    },  /* 132 */
        {  ERROR_DIR_NOT_EMPTY,          osl_File_E_NOTEMPTY },  /* 145 */
        {  ERROR_NOT_LOCKED,             osl_File_E_ACCES    },  /* 158 */
        {  ERROR_BAD_PATHNAME,           osl_File_E_NOENT    },  /* 161 */
        {  ERROR_MAX_THRDS_REACHED,      osl_File_E_AGAIN    },  /* 164 */
        {  ERROR_LOCK_FAILED,            osl_File_E_ACCES    },  /* 167 */
        {  ERROR_ALREADY_EXISTS,         osl_File_E_EXIST    },  /* 183 */
        {  ERROR_FILENAME_EXCED_RANGE,   osl_File_E_NOENT    },  /* 206 */
        {  ERROR_NESTING_NOT_ALLOWED,    osl_File_E_AGAIN    },  /* 215 */
        {  ERROR_DIRECTORY,              osl_File_E_NOENT    },  /* 267 */
        //{  ERROR_NOT_ENOUGH_QUOTA,       osl_File_E_NOMEM    }    /* 1816 */
    };

    #define ELEMENTS_OF_ARRAY(arr) (sizeof(arr)/(sizeof((arr)[0])))

    //#####################################################
    oslFileError MapError(APIRET dwError)
    {
        for (int i = 0; i < ELEMENTS_OF_ARRAY(errtable); ++i )
        {
            if (dwError == errtable[i].oscode)
                return static_cast<oslFileError>(errtable[i].errnocode);
        }
        return osl_File_E_INVAL;
    }

#ifdef DEBUG_OSL_FILE
#   define OSL_FILE_TRACE 0 ? (void)(0) : osl_trace
#   define PERROR( a, b ) perror( a ); fprintf( stderr, b )
#else
#   define OSL_FILE_TRACE 1 ? (void)(0) : osl_trace
#   define PERROR( a, b )
#endif

//##################################################################
// File handle implementation
//##################################################################
struct FileHandle_Impl
{
    rtl_String * m_strFilePath; /* holds native file path */
    int          m_fd;

    /** State
     */
    enum StateBits
    {
        STATE_SEEKABLE  = 1, /* default */
        STATE_READABLE  = 2, /* default */
        STATE_WRITEABLE = 4, /* open() sets, write() requires, else osl_File_E_BADF */
        STATE_MODIFIED  = 8  /* write() sets, flush() resets  */
    };
    int          m_state;

    sal_uInt64   m_size;    /* file size */
    off_t     m_offset;  /* physical offset from begin of file */
    //off_t     m_filepos; /* logical offset from begin of file */
    off_t        m_fileptr; /* logical offset from begin of file */

    off_t     m_bufptr;  /* buffer offset from begin of file */
    size_t       m_buflen;  /* buffer filled [0, m_bufsiz - 1] */

    size_t       m_bufsiz;
    sal_uInt8 *  m_buffer;

    explicit FileHandle_Impl (int fd, char const * path = "<anon>");
    ~FileHandle_Impl();

    static void*  operator new(size_t n);
    static void   operator delete(void * p, size_t);
    static size_t getpagesize();

    sal_uInt64    getPos() const;
    oslFileError  setPos (sal_uInt64 uPos);

    sal_uInt64    getSize() const;
    oslFileError  setSize (sal_uInt64 uPos);

    oslFileError readAt (
        off_t        nOffset,
        void *       pBuffer,
        size_t       nBytesRequested,
        sal_uInt64 * pBytesRead);

    oslFileError writeAt (
        off_t        nOffset,
        void const * pBuffer,
        size_t       nBytesToWrite,
        sal_uInt64 * pBytesWritten);

    oslFileError readFileAt (
        off_t        nOffset,
        void *       pBuffer,
        size_t       nBytesRequested,
        sal_uInt64 * pBytesRead);

    oslFileError writeFileAt (
        off_t        nOffset,
        void const * pBuffer,
        size_t       nBytesToWrite,
        sal_uInt64 * pBytesWritten);

    oslFileError readLineAt (
        LONGLONG        nOffset,
        sal_Sequence ** ppSequence,
        sal_uInt64 *    pBytesRead);

    oslFileError writeSequence_Impl (
        sal_Sequence ** ppSequence,
        size_t *        pnOffset,
        const void *    pBuffer,
        size_t          nBytes);

    oslFileError syncFile();

    /** Buffer cache / allocator.
     */
    class Allocator
    {
        rtl_cache_type * m_cache;
        size_t           m_bufsiz;

        Allocator (Allocator const &);
        Allocator & operator= (Allocator const &);

    public:
        static Allocator & get();

        void allocate (sal_uInt8 ** ppBuffer, size_t * pnSize);
        void deallocate (sal_uInt8 * pBuffer);

    protected:
        Allocator();
        ~Allocator();
    };
};

FileHandle_Impl::Allocator &
FileHandle_Impl::Allocator::get()
{
    static Allocator g_aBufferAllocator;
    return g_aBufferAllocator;
}

FileHandle_Impl::Allocator::Allocator()
    : m_cache  (0),
      m_bufsiz (0)
{
    size_t const pagesize = FileHandle_Impl::getpagesize();
    m_cache = rtl_cache_create (
        "osl_file_buffer_cache", pagesize, 0, 0, 0, 0, 0, 0, 0);
    if (0 != m_cache)
        m_bufsiz = pagesize;
}

FileHandle_Impl::Allocator::~Allocator()
{
    rtl_cache_destroy(m_cache), m_cache = 0;
}

void FileHandle_Impl::Allocator::allocate (sal_uInt8 ** ppBuffer, size_t * pnSize)
{
    OSL_PRECOND((0 != ppBuffer) && (0 != pnSize), "FileHandle_Impl::Allocator::allocate(): contract violation");
    *ppBuffer = static_cast< sal_uInt8* >(rtl_cache_alloc(m_cache)), *pnSize = m_bufsiz;
}

void FileHandle_Impl::Allocator::deallocate (sal_uInt8 * pBuffer)
{
    if (0 != pBuffer)
        rtl_cache_free (m_cache, pBuffer);
}

FileHandle_Impl::FileHandle_Impl (int fd, char const * path)
    : m_strFilePath (0),
      m_fd      (fd),
      m_state   (STATE_SEEKABLE | STATE_READABLE),
      m_size    (0),
      m_offset  (0),
      m_fileptr (0),
      m_bufptr  (-1),
      m_buflen  (0),
      m_bufsiz  (0),
      m_buffer  (0)
{
    rtl_string_newFromStr (&m_strFilePath, path);
    Allocator::get().allocate (&m_buffer, &m_bufsiz);
    if (m_buffer != 0)
        memset (m_buffer, 0, m_bufsiz);
}

FileHandle_Impl::~FileHandle_Impl()
{
    Allocator::get().deallocate (m_buffer), m_buffer = 0;
    rtl_string_release (m_strFilePath), m_strFilePath = 0;
}

void * FileHandle_Impl::operator new(size_t n)
{
    return rtl_allocateMemory(n);
}

void FileHandle_Impl::operator delete(void * p, size_t)
{
    rtl_freeMemory(p);
}

size_t FileHandle_Impl::getpagesize()
{
    ULONG ulPageSize;
    DosQuerySysInfo(QSV_PAGE_SIZE, QSV_PAGE_SIZE, &ulPageSize, sizeof(ULONG));
    return sal::static_int_cast< size_t >(ulPageSize);
}

sal_uInt64 FileHandle_Impl::getPos() const
{
    return sal::static_int_cast< sal_uInt64 >(m_fileptr);
}

oslFileError FileHandle_Impl::setPos (sal_uInt64 uPos)
{
    m_fileptr = sal::static_int_cast< LONGLONG >(uPos);
    return osl_File_E_None;
}

sal_uInt64 FileHandle_Impl::getSize() const
{
    LONGLONG bufend = std::max((LONGLONG)(0), m_bufptr) + m_buflen;
    return std::max(m_size, sal::static_int_cast< sal_uInt64 >(bufend));
}

oslFileError FileHandle_Impl::setSize (sal_uInt64 uSize)
{
    off_t const nSize = sal::static_int_cast< off_t >(uSize);
    if (-1 == ftruncate (m_fd, nSize))
    {
        /* Failure. Save original result. Try fallback algorithm */
        oslFileError result = oslTranslateFileError (OSL_FET_ERROR, errno);

        /* Check against current size. Fail upon 'shrink' */
        if (uSize <= getSize())
        {
            /* Failure upon 'shrink'. Return original result */
            return (result);
        }

        /* Save current position */
        off_t const nCurPos = (off_t)lseek (m_fd, (off_t)0, SEEK_CUR);
        if (nCurPos == (off_t)(-1))
            return (result);

        /* Try 'expand' via 'lseek()' and 'write()' */
        if (-1 == lseek (m_fd, (off_t)(nSize - 1), SEEK_SET))
            return (result);

        if (-1 == write (m_fd, (char*)"", (size_t)1))
        {
            /* Failure. Restore saved position */
            (void) lseek (m_fd, (off_t)(nCurPos), SEEK_SET);
            return (result);
        }

        /* Success. Restore saved position */
        if (-1 == lseek (m_fd, (off_t)nCurPos, SEEK_SET))
            return (result);
    }

    OSL_FILE_TRACE("osl_setFileSize(%d, %lld) => %ld", m_fd, getSize(), nSize);
    m_size = sal::static_int_cast< sal_uInt64 >(nSize);
    return osl_File_E_None;
}

oslFileError FileHandle_Impl::readAt (
    off_t        nOffset,
    void *       pBuffer,
    size_t       nBytesRequested,
    sal_uInt64 * pBytesRead)
{
    OSL_PRECOND((m_state & STATE_SEEKABLE), "FileHandle_Impl::readAt(): not seekable");
    if (!(m_state & STATE_SEEKABLE))
        return osl_File_E_SPIPE;

    OSL_PRECOND((m_state & STATE_READABLE), "FileHandle_Impl::readAt(): not readable");
    if (!(m_state & STATE_READABLE))
        return osl_File_E_BADF;

    if (nOffset != m_offset)
    {
        if (-1 == ::lseek (m_fd, nOffset, SEEK_SET))
            return oslTranslateFileError (OSL_FET_ERROR, errno);
        m_offset = nOffset;
    }

    ssize_t nBytes = ::read (m_fd, pBuffer, nBytesRequested);
    if (-1 == nBytes)
        return oslTranslateFileError (OSL_FET_ERROR, errno);
    m_offset += nBytes;

    OSL_FILE_TRACE("FileHandle_Impl::readAt(%d, %lld, %ld)", m_fd, nOffset, nBytes);
    *pBytesRead = nBytes;
    return osl_File_E_None;
}

oslFileError FileHandle_Impl::writeAt (
    off_t        nOffset,
    void const * pBuffer,
    size_t       nBytesToWrite,
    sal_uInt64 * pBytesWritten)
{
    OSL_PRECOND((m_state & STATE_SEEKABLE), "FileHandle_Impl::writeAt(): not seekable");
    if (!(m_state & STATE_SEEKABLE))
        return osl_File_E_SPIPE;

    OSL_PRECOND((m_state & STATE_WRITEABLE), "FileHandle_Impl::writeAt(): not writeable");
    if (!(m_state & STATE_WRITEABLE))
        return osl_File_E_BADF;

    if (nOffset != m_offset)
    {
        if (-1 == ::lseek (m_fd, nOffset, SEEK_SET))
            return oslTranslateFileError (OSL_FET_ERROR, errno);
        m_offset = nOffset;
    }

    ssize_t nBytes = ::write (m_fd, pBuffer, nBytesToWrite);
    if (-1 == nBytes)
        return oslTranslateFileError (OSL_FET_ERROR, errno);
    m_offset += nBytes;

    OSL_FILE_TRACE("FileHandle_Impl::writeAt(%d, %lld, %ld)", m_fd, nOffset, nBytes);
    m_size = std::max (m_size, sal::static_int_cast< sal_uInt64 >(nOffset + nBytes));

    *pBytesWritten = nBytes;
    return osl_File_E_None;
}

oslFileError FileHandle_Impl::readFileAt (
    off_t        nOffset,
    void *       pBuffer,
    size_t       nBytesRequested,
    sal_uInt64 * pBytesRead)
{
    if (0 == (m_state & STATE_SEEKABLE))
    {
        // not seekable (pipe)
        ssize_t nBytes = ::read (m_fd, pBuffer, nBytesRequested);
        if (-1 == nBytes)
            return oslTranslateFileError (OSL_FET_ERROR, errno);
        *pBytesRead = nBytes;
        return osl_File_E_None;
    }
    else if (0 == m_buffer)
    {
        // not buffered
        return readAt (nOffset, pBuffer, nBytesRequested, pBytesRead);
    }
    else
    {
        sal_uInt8 * buffer = static_cast<sal_uInt8*>(pBuffer);
        for (*pBytesRead = 0; nBytesRequested > 0; )
        {
            off_t  const bufptr = (nOffset / m_bufsiz) * m_bufsiz;
            size_t const bufpos = (nOffset % m_bufsiz);

            if (bufptr != m_bufptr)
            {
                // flush current buffer
                oslFileError result = syncFile();
                if (result != osl_File_E_None)
                    return (result);

                if (nBytesRequested >= m_bufsiz)
                {
                    // buffer too small, read through from file
                    sal_uInt64 uDone = 0;
                    result = readAt (nOffset, &(buffer[*pBytesRead]), nBytesRequested, &uDone);
                    if (result != osl_File_E_None)
                        return (result);

                    nBytesRequested -= uDone, *pBytesRead += uDone;
                    return osl_File_E_None;
                }

                // update buffer (pointer)
                sal_uInt64 uDone = 0;
                result = readAt (bufptr, m_buffer, m_bufsiz, &uDone);
                if (result != osl_File_E_None)
                    return (result);
                m_bufptr = bufptr, m_buflen = uDone;
            }
            if (bufpos >= m_buflen)
            {
                // end of file
                return osl_File_E_None;
            }

            size_t const bytes = std::min (m_buflen - bufpos, nBytesRequested);
            OSL_FILE_TRACE("FileHandle_Impl::readFileAt(%d, %lld, %ld)", m_fd, nOffset, bytes);

            memcpy (&(buffer[*pBytesRead]), &(m_buffer[bufpos]), bytes);
            nBytesRequested -= bytes, *pBytesRead += bytes, nOffset += bytes;
        }
        return osl_File_E_None;
    }
}

oslFileError FileHandle_Impl::writeFileAt (
    off_t        nOffset,
    void const * pBuffer,
    size_t       nBytesToWrite,
    sal_uInt64 * pBytesWritten)
{
    if (0 == (m_state & STATE_SEEKABLE))
    {
        // not seekable (pipe)
        ssize_t nBytes = ::write (m_fd, pBuffer, nBytesToWrite);
        if (-1 == nBytes)
            return oslTranslateFileError (OSL_FET_ERROR, errno);
        *pBytesWritten = nBytes;
        return osl_File_E_None;
    }
    else if (0 == m_buffer)
    {
        // not buffered
        return writeAt (nOffset, pBuffer, nBytesToWrite, pBytesWritten);
    }
    else
    {
        sal_uInt8 const * buffer = static_cast<sal_uInt8 const *>(pBuffer);
        for (*pBytesWritten = 0; nBytesToWrite > 0; )
        {
            off_t  const bufptr = (nOffset / m_bufsiz) * m_bufsiz;
            size_t const bufpos = (nOffset % m_bufsiz);
            if (bufptr != m_bufptr)
            {
                // flush current buffer
                oslFileError result = syncFile();
                if (result != osl_File_E_None)
                    return (result);

                if (nBytesToWrite >= m_bufsiz)
                {
                    // buffer to small, write through to file
                    sal_uInt64 uDone = 0;
                    result = writeAt (nOffset, &(buffer[*pBytesWritten]), nBytesToWrite, &uDone);
                    if (result != osl_File_E_None)
                        return (result);
                    if (uDone != nBytesToWrite)
                        return osl_File_E_IO;

                    nBytesToWrite -= uDone, *pBytesWritten += uDone;
                    return osl_File_E_None;
                }

                // update buffer (pointer)
                sal_uInt64 uDone = 0;
                result = readAt (bufptr, m_buffer, m_bufsiz, &uDone);
                if (result != osl_File_E_None)
                    return (result);
                m_bufptr = bufptr, m_buflen = uDone;
            }

            size_t const bytes = std::min (m_bufsiz - bufpos, nBytesToWrite);
            OSL_FILE_TRACE("FileHandle_Impl::writeFileAt(%d, %lld, %ld)", m_fd, nOffset, bytes);

            memcpy (&(m_buffer[bufpos]), &(buffer[*pBytesWritten]), bytes);
            nBytesToWrite -= bytes, *pBytesWritten += bytes, nOffset += bytes;

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
            return (result);

        /* update buffer (pointer) */
        sal_uInt64 uDone = 0;
        result = readAt (bufptr, m_buffer, m_bufsiz, &uDone);
        if (result != osl_File_E_None)
            return (result);

        m_bufptr = bufptr, m_buflen = sal::static_int_cast< size_t >(uDone);
    }

    static int const LINE_STATE_BEGIN = 0;
    static int const LINE_STATE_CR    = 1;
    static int const LINE_STATE_LF    = 2;

    size_t bufpos = sal::static_int_cast< size_t >(nOffset - m_bufptr), curpos = bufpos, dstpos = 0;
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
                    return (result);
                *pBytesRead += curpos - bufpos, nOffset += curpos - bufpos;
            }

            bufptr = nOffset / m_bufsiz * m_bufsiz;
            if (bufptr != m_bufptr)
            {
                /* update buffer (pointer) */
                sal_uInt64 uDone = 0;
                result = readAt (bufptr, m_buffer, m_bufsiz, &uDone);
                if (result != osl_File_E_None)
                    return (result);
                m_bufptr = bufptr, m_buflen = sal::static_int_cast< size_t >(uDone);
            }

            bufpos = sal::static_int_cast< size_t >(nOffset - m_bufptr), curpos = bufpos;
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
                m_buffer[curpos] = 0x0A, curpos++;

                /* flush buffer to sequence */
                result = writeSequence_Impl (
                    ppSequence, &dstpos, &(m_buffer[bufpos]), curpos - bufpos - 1);
                if (result != osl_File_E_None)
                    return (result);
                *pBytesRead += curpos - bufpos, nOffset += curpos - bufpos;
            }
            break;
        }
    }

    result = writeSequence_Impl (ppSequence, &dstpos, 0, 0);
    if (result != osl_File_E_None)
        return (result);
    if (0 < dstpos)
        return osl_File_E_None;
    if (bufpos >= m_buflen)
        return osl_File_E_AGAIN;
    return osl_File_E_None;
}

oslFileError FileHandle_Impl::writeSequence_Impl (
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
    if (*ppSequence != 0)
    {
        /* fill sequence */
        memcpy(&((*ppSequence)->elements[*pnOffset]), pBuffer, nBytes), *pnOffset += nBytes;
    }
    return (*ppSequence != 0) ? osl_File_E_None : osl_File_E_NOMEM;
}

oslFileError FileHandle_Impl::syncFile()
{
    oslFileError result = osl_File_E_None;
    if (m_state & STATE_MODIFIED)
    {
        sal_uInt64 uDone = 0;
        result = writeAt (m_bufptr, m_buffer, m_buflen, &uDone);
        if (result != osl_File_E_None)
            return (result);
        if (uDone != m_buflen)
            return osl_File_E_IO;
        m_state &= ~STATE_MODIFIED;
    }
    return (result);
}


/******************************************************************************
 *
 *                  static members
 *
 *****************************************************************************/

static const char * pFileLockEnvVar = (char *) -1;


/******************************************************************************
 *
 *                  C-String Function Declarations
 *
 *****************************************************************************/

static oslFileError osl_psz_getVolumeInformation(const sal_Char* , oslVolumeInfo* pInfo, sal_uInt32 uFieldMask);
static oslFileError osl_psz_removeFile(const sal_Char* pszPath);
static oslFileError osl_psz_createDirectory(const sal_Char* pszPath);
static oslFileError osl_psz_removeDirectory(const sal_Char* pszPath);
static oslFileError osl_psz_copyFile(const sal_Char* pszPath, const sal_Char* pszDestPath);
static oslFileError osl_psz_moveFile(const sal_Char* pszPath, const sal_Char* pszDestPath);
static oslFileError osl_psz_setFileTime(const sal_Char* strFilePath, const TimeValue* pCreationTime, const TimeValue* pLastAccessTime, const TimeValue* pLastWriteTime);


/******************************************************************************
 *
 *                  Static Module Utility Function Declarations
 *
 *****************************************************************************/

static oslFileError  oslDoCopy(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, mode_t nMode, size_t nSourceSize, int DestFileExists);
static oslFileError  oslChangeFileModes(const sal_Char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID);
static int           oslDoCopyLink(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName);
static int           oslDoCopyFile(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, size_t nSourceSize, mode_t mode);
static oslFileError  oslDoMoveFile(const sal_Char* pszPath, const sal_Char* pszDestPath);
rtl_uString*  oslMakeUStrFromPsz(const sal_Char* pszStr,rtl_uString** uStr);

/******************************************************************************
 *
 *                  Non-Static Utility Function Declarations
 *
 *****************************************************************************/

extern "C" int UnicodeToText( char *, size_t, const sal_Unicode *, sal_Int32 );
extern "C" int TextToUnicode(
    const char* text, size_t text_buffer_size,  sal_Unicode* unic_text, sal_Int32 unic_text_buffer_size);

/******************************************************************************
 *
 *                  'removeable device' aka floppy functions
 *
 *****************************************************************************/

static oslVolumeDeviceHandle  osl_isFloppyDrive(const sal_Char* pszPath);
static oslFileError   osl_mountFloppy(oslVolumeDeviceHandle hFloppy);
static oslFileError   osl_unmountFloppy(oslVolumeDeviceHandle hFloppy);

#ifdef DEBUG_OSL_FILE
static void           osl_printFloppyHandle(oslVolumeDeviceHandleImpl* hFloppy);
#endif

/**********************************************
 * _osl_openLocalRoot
 * enumerate available drives
 *********************************************/
static oslFileError _osl_openLocalRoot( rtl_uString *strDirectoryPath, oslDirectory *pDirectory)
{
    rtl_uString     *ustrSystemPath = NULL;
    oslFileError    error;

    if ( !pDirectory )
        return osl_File_E_INVAL;

    *pDirectory = NULL;

    error = osl_getSystemPathFromFileURL_Ex( strDirectoryPath, &ustrSystemPath, sal_False );

    if ( osl_File_E_None == error )
    {
        /* create and initialize impl structure */
        DirectoryImpl* pDirImpl = (DirectoryImpl*) rtl_allocateMemory( sizeof(DirectoryImpl) );
        if( pDirImpl )
        {
            ULONG   ulDriveNum;
            APIRET  rc;
            pDirImpl->uType = DIRECTORYTYPE_LOCALROOT;
            pDirImpl->ustrPath = ustrSystemPath;
            rc = DosQueryCurrentDisk (&ulDriveNum, &pDirImpl->ulDriveMap);
            pDirImpl->pDirStruct = 0;
            pDirImpl->ulNextDrive = 1;
            pDirImpl->ulNextDriveMask = 1;

            // determine number of floppy-drives
            BYTE nFloppies;
            rc = DosDevConfig( (void*) &nFloppies, DEVINFO_FLOPPY );
            if (nFloppies == 0) {
                // if no floppies, start with 3rd drive (C:)
                pDirImpl->ulNextDrive = 3;
                pDirImpl->ulNextDriveMask <<= 2;
            } else if (nFloppies == 1) {
                // mask drive B (second bit) in this case
                pDirImpl->ulDriveMap &= ~0x02;
            }
            *pDirectory = (oslDirectory) pDirImpl;
            return osl_File_E_None;
        }
        else
        {
            errno = osl_File_E_NOMEM;
        }

    }

    rtl_uString_release( ustrSystemPath );
    return error;
}

/**********************************************
 * _osl_getNextDrive
 *********************************************/
static oslFileError SAL_CALL _osl_getNextDrive(
    oslDirectory Directory, oslDirectoryItem *pItem, sal_uInt32 uHint )
{
    DirectoryImpl   *pDirImpl = (DirectoryImpl *)Directory;
    DirectoryItem_Impl  *pItemImpl = NULL;
    rtl_uString         * ustrDrive = NULL;
    BOOL                fSuccess;
    char                buffer[3];

    uHint = uHint; /* avoid warnings */

    if ( !pItem )
        return osl_File_E_INVAL;

    *pItem = NULL;

    if ( !pDirImpl )
        return osl_File_E_INVAL;

    while( pDirImpl->ulNextDrive <= 26)
    {
        // exit if  bit==1 -> drive found
        if (pDirImpl->ulDriveMap & pDirImpl->ulNextDriveMask) {

            /* convert file name to unicode */
            buffer[0] = '@' + pDirImpl->ulNextDrive;
            buffer[1] = ':';
            buffer[2] = 0;

            pItemImpl = (DirectoryItem_Impl*) rtl_allocateMemory(sizeof(DirectoryItem_Impl));
            if ( !pItemImpl )
                return osl_File_E_NOMEM;

            memset( pItemImpl, 0, sizeof(DirectoryItem_Impl) );
            pItemImpl->uType = DIRECTORYITEM_DRIVE;
            pItemImpl->nRefCount = 1;

            rtl_string2UString( &pItemImpl->ustrDrive, buffer, 3,
                osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
            OSL_ASSERT(pItemImpl->ustrDrive != 0);

            /* use drive as directory item */
            *pItem = (oslDirectoryItem) pItemImpl;
        }
        // scan next bit position
        pDirImpl->ulNextDrive++;
        pDirImpl->ulNextDriveMask <<= 1;

        if (*pItem) // item assigned, return now.
            return osl_File_E_None;
    }

    // no more items
    return osl_File_E_NOENT;
}

/**********************************************
 * _osl_readdir_impl_
 *
 * readdir wrapper, filters out "." and ".."
 * on request
 *********************************************/

static struct dirent* _osl_readdir_impl_(DIR* pdir, sal_Bool bFilterLocalAndParentDir)
{
    struct dirent* pdirent;

    while ((pdirent = readdir(pdir)) != NULL)
    {
        if (bFilterLocalAndParentDir &&
            ((0 == strcmp(pdirent->d_name, ".")) || (0 == strcmp(pdirent->d_name, ".."))))
            continue;
        else
            break;
    }

    return pdirent;
}

/*******************************************************************
 *  osl_openDirectory
 ******************************************************************/

oslFileError SAL_CALL osl_openDirectory(rtl_uString* ustrDirectoryURL, oslDirectory* pDirectory)
{
    rtl_uString* ustrSystemPath = NULL;
    oslFileError eRet;

    char path[PATH_MAX];

    OSL_ASSERT(ustrDirectoryURL && (ustrDirectoryURL->length > 0));
    OSL_ASSERT(pDirectory);

    if (0 == ustrDirectoryURL->length )
        return osl_File_E_INVAL;

    if ( 0 == rtl_ustr_compareIgnoreAsciiCase( ustrDirectoryURL->buffer, (const sal_Unicode*)L"file:///" ) )
        return _osl_openLocalRoot( ustrDirectoryURL, pDirectory );

    /* convert file URL to system path */
    eRet = osl_getSystemPathFromFileURL_Ex(ustrDirectoryURL, &ustrSystemPath, sal_False);

    if( osl_File_E_None != eRet )
        return eRet;

    osl_systemPathRemoveSeparator(ustrSystemPath);

    /* convert unicode path to text */
    if ( UnicodeToText( path, PATH_MAX, ustrSystemPath->buffer, ustrSystemPath->length ) )
    {
        // if only the drive is specified (x:), add a \ (x:\) otherwise current
        // directory is browsed instead of root.
        if (strlen( path) == 2 && path[1] == ':')
            strcat( path, "\\");
        /* open directory */
        DIR *pdir = opendir( path );

        if( pdir )
        {
            /* create and initialize impl structure */
            DirectoryImpl* pDirImpl = (DirectoryImpl*) rtl_allocateMemory( sizeof(DirectoryImpl) );

            if( pDirImpl )
            {
                pDirImpl->uType = DIRECTORYTYPE_FILESYSTEM;
                pDirImpl->pDirStruct = pdir;
                pDirImpl->ustrPath = ustrSystemPath;

                *pDirectory = (oslDirectory) pDirImpl;
                return osl_File_E_None;
            }
            else
            {
                errno = ENOMEM;
                closedir( pdir );
            }
        }
        else
            /* should be removed by optimizer in product version */
            PERROR( "osl_openDirectory", path );
    }

    rtl_uString_release( ustrSystemPath );

    return oslTranslateFileError(OSL_FET_ERROR, errno);
}


/****************************************************************************
 *  osl_getNextDirectoryItem
 ***************************************************************************/

oslFileError SAL_CALL osl_getNextDirectoryItem(oslDirectory Directory, oslDirectoryItem* pItem, sal_uInt32 uHint)
{
    DirectoryImpl* pDirImpl     = (DirectoryImpl*)Directory;
    DirectoryItem_Impl  *pItemImpl = NULL;
    rtl_uString*      ustrFileName = NULL;
    rtl_uString*      ustrFilePath = NULL;
    struct dirent*    pEntry;

    OSL_ASSERT(Directory);
    OSL_ASSERT(pItem);

    if ((NULL == Directory) || (NULL == pItem))
        return osl_File_E_INVAL;

    if ( pDirImpl->uType == DIRECTORYTYPE_LOCALROOT)
        return _osl_getNextDrive( Directory, pItem, uHint );

    pEntry = _osl_readdir_impl_(pDirImpl->pDirStruct, sal_True);

    if (NULL == pEntry)
        return osl_File_E_NOENT;

    pItemImpl = (DirectoryItem_Impl*) rtl_allocateMemory(sizeof(DirectoryItem_Impl));
    if ( !pItemImpl )
        return osl_File_E_NOMEM;

    memset( pItemImpl, 0, sizeof(DirectoryItem_Impl) );
    pItemImpl->uType = DIRECTORYITEM_FILE;
    pItemImpl->nRefCount = 1;
    pItemImpl->d_attr = pEntry->d_attr;

    /* convert file name to unicode */
    rtl_string2UString( &ustrFileName, pEntry->d_name, strlen( pEntry->d_name ),
        osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
    OSL_ASSERT(ustrFileName != 0);

    osl_systemPathMakeAbsolutePath(pDirImpl->ustrPath, ustrFileName, &pItemImpl->ustrFilePath);
    rtl_uString_release( ustrFileName );

    *pItem = (oslDirectoryItem)pItemImpl;
    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_closeDirectory */
/****************************************************************************/

oslFileError SAL_CALL osl_closeDirectory( oslDirectory Directory )
{
    DirectoryImpl* pDirImpl = (DirectoryImpl*) Directory;
    oslFileError err = osl_File_E_None;

    OSL_ASSERT( Directory );

    if( NULL == pDirImpl )
        return osl_File_E_INVAL;

    switch ( pDirImpl->uType )
    {
    case DIRECTORYTYPE_FILESYSTEM:
        if( closedir( pDirImpl->pDirStruct ) )
            err = oslTranslateFileError(OSL_FET_ERROR, errno);
        break;
    case DIRECTORYTYPE_LOCALROOT:
        err = osl_File_E_None;
        break;
#if 0
    case DIRECTORYTYPE_NETROOT:
        {
            DWORD err = WNetCloseEnum(pDirImpl->hDirectory);
            eError = (err == NO_ERROR) ? osl_File_E_None : MapError(err);
        }
        break;
#endif
    default:
        OSL_ENSURE( 0, "Invalid directory type" );
        break;
    }

    /* cleanup members */
    rtl_uString_release( pDirImpl->ustrPath );

    rtl_freeMemory( pDirImpl );

    return err;
}

/****************************************************************************/
/*  osl_getDirectoryItem */
/****************************************************************************/

oslFileError SAL_CALL osl_getDirectoryItem( rtl_uString* ustrFileURL, oslDirectoryItem* pItem )
{
    rtl_uString*    strSysFilePath = NULL;
    oslFileError    error      = osl_File_E_INVAL;
    ULONG           dwPathType;
    PATHTYPE        type = PATHTYPE_FILE;

    OSL_ASSERT(ustrFileURL);
    OSL_ASSERT(pItem);

    /* Assume failure */
    if ( !pItem )
        return osl_File_E_INVAL;
    *pItem = NULL;

    if (0 == ustrFileURL->length || NULL == pItem)
        return osl_File_E_INVAL;

    error = osl_getSystemPathFromFileURL_Ex(ustrFileURL, &strSysFilePath, sal_False);

    if (osl_File_E_None != error)
        return error;

    dwPathType = IsValidFilePath( strSysFilePath->buffer, NULL, VALIDATEPATH_NORMAL );

    if ( dwPathType & PATHTYPE_IS_VOLUME )
        type = PATHTYPE_VOLUME;
    else if ( dwPathType & PATHTYPE_IS_SERVER )
        type = PATHTYPE_NETSERVER;
    else
        type = PATHTYPE_FILE;

    switch ( type )
    {
    case PATHTYPE_NETSERVER:
        {
            DirectoryItem_Impl* pItemImpl =
                reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));

            if ( !pItemImpl )
                error = osl_File_E_NOMEM;

            if ( osl_File_E_None == error )
            {
                memset( pItemImpl, 0, sizeof(DirectoryItem_Impl) );
                pItemImpl->uType = DIRECTORYITEM_SERVER;
                pItemImpl->nRefCount = 1;
                rtl_uString_assign( &pItemImpl->ustrFilePath, strSysFilePath );

                *pItem = pItemImpl;
            }
        }
        break;
    case PATHTYPE_VOLUME:
        {
            DirectoryItem_Impl* pItemImpl =
                reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));

            if ( !pItemImpl )
                error = osl_File_E_NOMEM;

            if ( osl_File_E_None == error )
            {
                memset( pItemImpl, 0, sizeof(DirectoryItem_Impl) );
                pItemImpl->uType = DIRECTORYITEM_DRIVE;
                pItemImpl->nRefCount = 1;
                rtl_uString_assign( &pItemImpl->ustrDrive, strSysFilePath );

                if ( pItemImpl->ustrDrive->buffer[pItemImpl->ustrDrive->length-1] != sal_Unicode('\\') )
                    rtl_uString_newConcat( &pItemImpl->ustrDrive,
                                            pItemImpl->ustrDrive, rtl::OUString::createFromAscii( "\\" ).pData);

                *pItem = pItemImpl;
            }
        }
        break;
    default:
    case PATHTYPE_FILE:
        {
            if ( strSysFilePath->length > 0 && strSysFilePath->buffer[strSysFilePath->length - 1] == '\\' )
                rtl_uString_newFromStr_WithLength( &strSysFilePath, strSysFilePath->buffer, strSysFilePath->length - 1 );

            if (0 == access_u(strSysFilePath, F_OK))
            {
                DirectoryItem_Impl  *pItemImpl =
                    reinterpret_cast<DirectoryItem_Impl*>(rtl_allocateMemory(sizeof(DirectoryItem_Impl)));

                memset( pItemImpl, 0, sizeof(DirectoryItem_Impl) );
                pItemImpl->uType = DIRECTORYITEM_FILE;
                pItemImpl->nRefCount = 1;
                rtl_uString_assign( &pItemImpl->ustrFilePath, strSysFilePath );

                *pItem = pItemImpl;
            }
            else
                error = oslTranslateFileError(OSL_FET_ERROR, errno);
        }
        break;
    }

    if ( strSysFilePath )
        rtl_uString_release( strSysFilePath );

    return error;
}

/****************************************************************************/
/*  osl_acquireDirectoryItem */
/****************************************************************************/

oslFileError osl_acquireDirectoryItem( oslDirectoryItem Item )
{
    OSL_ASSERT( Item );
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    pItemImpl->nRefCount++;
    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_releaseDirectoryItem */
/****************************************************************************/

oslFileError osl_releaseDirectoryItem( oslDirectoryItem Item )
{
    OSL_ASSERT( Item );
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    if ( ! --pItemImpl->nRefCount )
    {
        if (pItemImpl->ustrFilePath)
            rtl_uString_release( pItemImpl->ustrFilePath );
        if (pItemImpl->ustrDrive)
            rtl_uString_release( pItemImpl->ustrDrive );
        rtl_freeMemory( pItemImpl );
    }
    return osl_File_E_None;
}

/****************************************************************************
 *  osl_createFileHandleFromFD
 ***************************************************************************/
extern "C" oslFileHandle osl_createFileHandleFromFD( int fd )
{
    if (-1 == fd)
        return 0; // EINVAL

    struct stat aFileStat;
    if (-1 == fstat (fd, &aFileStat))
        return 0; // EBADF

    FileHandle_Impl * pImpl = new FileHandle_Impl (fd);
    if (0 == pImpl)
        return 0; // ENOMEM

    // assume writeable
    pImpl->m_state |= FileHandle_Impl::STATE_WRITEABLE;
    if (!S_ISREG(aFileStat.st_mode))
    {
        /* not a regular file, mark not seekable */
        pImpl->m_state &= ~FileHandle_Impl::STATE_SEEKABLE;
    }
    else
    {
        /* regular file, init current size */
        pImpl->m_size = sal::static_int_cast< sal_uInt64 >(aFileStat.st_size);
    }

    OSL_FILE_TRACE("osl_createFileHandleFromFD(%d, writeable) => %s",
                   pImpl->m_fd, rtl_string_getStr(pImpl->m_strFilePath));
    return (oslFileHandle)(pImpl);
}

/*******************************************************************
 * osl_file_adjustLockFlags
 ******************************************************************/
static int osl_file_adjustLockFlags (const char * path, int flags)
{
#ifdef MACOSX
    /*
     * The AFP implementation of MacOS X 10.4 treats O_EXLOCK in a way
     * that makes it impossible for OOo to create a backup copy of the
     * file it keeps opened. OTOH O_SHLOCK for AFP behaves as desired by
     * the OOo file handling, so we need to check the path of the file
     * for the filesystem name.
     */
    struct statfs s;
    if( 0 <= statfs( path, &s ) )
    {
        if( 0 == strncmp("afpfs", s.f_fstypename, 5) )
        {
            flags &= ~O_EXLOCK;
            flags |=  O_SHLOCK;
        }
        else
        {
            /* Needed flags to allow opening a webdav file */
            flags &= ~(O_EXLOCK | O_SHLOCK | O_NONBLOCK);
        }
    }
#endif /* MACOSX */

    (void) path;
    return flags;
}

/****************************************************************************
 *  osl_file_queryLocking
 ***************************************************************************/
struct Locking_Impl
{
    int m_enabled;
    Locking_Impl() : m_enabled(0)
    {
#ifndef HAVE_O_EXLOCK
        m_enabled = ((getenv("SAL_ENABLE_FILE_LOCKING") != 0) || (getenv("STAR_ENABLE_FILE_LOCKING") != 0));
#endif /* HAVE_O_EXLOCK */
    }
};
static int osl_file_queryLocking (sal_uInt32 uFlags)
{
    if (!(uFlags & osl_File_OpenFlag_NoLock))
    {
        if ((uFlags & osl_File_OpenFlag_Write) || (uFlags & osl_File_OpenFlag_Create))
        {
            static Locking_Impl g_locking;
            return (g_locking.m_enabled != 0);
        }
    }
    return 0;
}

/****************************************************************************
 *  osl_openFile
 ***************************************************************************/
#ifdef HAVE_O_EXLOCK
#define OPEN_WRITE_FLAGS ( O_RDWR | O_EXLOCK | O_NONBLOCK )
#define OPEN_CREATE_FLAGS ( O_CREAT | O_EXCL | O_RDWR | O_EXLOCK | O_NONBLOCK )
#else
#define OPEN_WRITE_FLAGS ( O_RDWR )
#define OPEN_CREATE_FLAGS ( O_CREAT | O_EXCL | O_RDWR )
#endif

oslFileError
SAL_CALL osl_openFile( rtl_uString* ustrFileURL, oslFileHandle* pHandle, sal_uInt32 uFlags )
{
    oslFileError eRet;

    if ((ustrFileURL == 0) || (ustrFileURL->length == 0) || (pHandle == 0))
        return osl_File_E_INVAL;

    /* convert file URL to system path */
    char buffer[PATH_MAX];
    eRet = FileURLToPath (buffer, sizeof(buffer), ustrFileURL);
    if (eRet != osl_File_E_None)
        return eRet;
#ifdef MACOSX
    if (macxp_resolveAlias (buffer, sizeof(buffer)) != 0)
        return oslTranslateFileError (OSL_FET_ERROR, errno);
#endif /* MACOSX */

    /* set mode and flags */
    int mode  = S_IRUSR | S_IRGRP | S_IROTH;
    int flags = O_RDONLY;
    if (uFlags & osl_File_OpenFlag_Write)
    {
        mode |= S_IWUSR | S_IWGRP | S_IWOTH;
        flags = OPEN_WRITE_FLAGS;
    }
    if (uFlags & osl_File_OpenFlag_Create)
    {
        mode |= S_IWUSR | S_IWGRP | S_IWOTH;
        flags = OPEN_CREATE_FLAGS;
    }
    if (uFlags & osl_File_OpenFlag_NoLock)
    {
#ifdef HAVE_O_EXLOCK
        flags &= ~(O_EXLOCK | O_SHLOCK | O_NONBLOCK);
#endif /* HAVE_O_EXLOCK */
    }
    else
    {
        flags = osl_file_adjustLockFlags (buffer, flags);
    }

    /* open the file */
    int fd = open( buffer, flags | O_BINARY, mode );
    if (-1 == fd)
        return oslTranslateFileError (OSL_FET_ERROR, errno);

    /* reset O_NONBLOCK flag */
    if (flags & O_NONBLOCK)
    {
        int f = fcntl (fd, F_GETFL, 0);
        if (-1 == f)
        {
            eRet = oslTranslateFileError (OSL_FET_ERROR, errno);
            (void) close(fd);
            return eRet;
        }
        if (-1 == fcntl (fd, F_SETFL, (f & ~O_NONBLOCK)))
        {
            eRet = oslTranslateFileError (OSL_FET_ERROR, errno);
            (void) close(fd);
            return eRet;
        }
    }

    /* get file status (mode, size) */
    struct stat aFileStat;
    if (-1 == fstat (fd, &aFileStat))
    {
        eRet = oslTranslateFileError (OSL_FET_ERROR, errno);
        (void) close(fd);
        return eRet;
    }
    if (!S_ISREG(aFileStat.st_mode))
    {
        /* we only open regular files here */
        (void) close(fd);
        return osl_File_E_INVAL;
    }

    if (osl_file_queryLocking (uFlags))
    {
#ifdef MACOSX
        if (-1 == flock (fd, LOCK_EX | LOCK_NB))
        {
            /* Mac OSX returns ENOTSUP for webdav drives. We should try read lock */
            if ((errno != ENOTSUP) || ((-1 == flock (fd, LOCK_SH | LOCK_NB)) && (errno != ENOTSUP)))
            {
                eRet = oslTranslateFileError (OSL_FET_ERROR, errno);
                (void) close(fd);
                return eRet;
            }
        }
#else   /* F_SETLK */
        {
            struct flock aflock;

            aflock.l_type = F_WRLCK;
            aflock.l_whence = SEEK_SET;
            aflock.l_start = 0;
            aflock.l_len = 0;

            if (-1 == fcntl (fd, F_SETLK, &aflock))
            {
                eRet = oslTranslateFileError (OSL_FET_ERROR, errno);
                (void) close(fd);
                return eRet;
            }
        }
#endif  /* F_SETLK */
    }

    /* allocate memory for impl structure */
    FileHandle_Impl * pImpl = new FileHandle_Impl (fd, buffer);
    if (!pImpl)
    {
        eRet = oslTranslateFileError (OSL_FET_ERROR, ENOMEM);
        (void) close(fd);
        return eRet;
    }
    if (flags & O_RDWR)
        pImpl->m_state |= FileHandle_Impl::STATE_WRITEABLE;
    pImpl->m_size = sal::static_int_cast< sal_uInt64 >(aFileStat.st_size);

    OSL_TRACE("osl_openFile(%d, %s) => %s", pImpl->m_fd,
              flags & O_RDWR ? "writeable":"readonly",
              rtl_string_getStr(pImpl->m_strFilePath));

    *pHandle = (oslFileHandle)(pImpl);
    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_closeFile */
/****************************************************************************/
oslFileError
SAL_CALL osl_closeFile( oslFileHandle Handle )
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((pImpl == 0) || (pImpl->m_fd < 0))
        return osl_File_E_INVAL;

    /* close(2) implicitly (and unconditionally) unlocks */
    OSL_TRACE("osl_closeFile(%d) => %s", pImpl->m_fd, rtl_string_getStr(pImpl->m_strFilePath));
    oslFileError result = pImpl->syncFile();
    if (result != osl_File_E_None)
    {
        /* close, ignoring double failure */
        (void) close (pImpl->m_fd);
    }
    else if (-1 == close (pImpl->m_fd))
    {
        /* translate error code */
        result = oslTranslateFileError (OSL_FET_ERROR, errno);
    }

    delete pImpl;
    return (result);
}

/************************************************
 * osl_syncFile
 ***********************************************/
oslFileError
SAL_CALL osl_syncFile(oslFileHandle Handle)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd))
        return osl_File_E_INVAL;

    OSL_FILE_TRACE("osl_syncFile(%d)", pImpl->m_fd);
    oslFileError result = pImpl->syncFile();
    if (result != osl_File_E_None)
        return (result);
    if (-1 == fsync (pImpl->m_fd))
        return oslTranslateFileError (OSL_FET_ERROR, errno);

    return osl_File_E_None;
}

/*******************************************
    osl_mapFile
********************************************/
oslFileError
SAL_CALL osl_mapFile (
    oslFileHandle Handle,
    void**        ppAddr,
    sal_uInt64    uLength,
    sal_uInt64    uOffset,
    sal_uInt32    uFlags
)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == ppAddr))
        return osl_File_E_INVAL;
    *ppAddr = 0;

    static sal_uInt64 const g_limit_size_t = std::numeric_limits< size_t >::max();
    if (g_limit_size_t < uLength)
        return osl_File_E_OVERFLOW;
    size_t const nLength = sal::static_int_cast< size_t >(uLength);

    static sal_uInt64 const g_limit_off_t = std::numeric_limits< off_t >::max();
    if (g_limit_off_t < uOffset)
        return osl_File_E_OVERFLOW;
    off_t const nOffset = sal::static_int_cast< off_t >(uOffset);

#ifdef SAL_OS2 // YD mmap does not support shared
    void* p = mmap(NULL, nLength, PROT_READ, MAP_PRIVATE, pImpl->m_fd, nOffset);
#else
    void* p = mmap(NULL, nLength, PROT_READ, MAP_SHARED, pImpl->m_fd, nOffset);
#endif
    if (MAP_FAILED == p)
        return oslTranslateFileError(OSL_FET_ERROR, errno);
    *ppAddr = p;

    if (uFlags & osl_File_MapFlag_RandomAccess)
    {
        // Determine memory pagesize.
        size_t const nPageSize = FileHandle_Impl::getpagesize();
        if (size_t(-1) != nPageSize)
        {
            /*
             * Pagein, touching first byte of every memory page.
             * Note: volatile disables optimizing the loop away.
             */
            sal_uInt8 * pData (reinterpret_cast<sal_uInt8*>(*ppAddr));
            size_t      nSize (nLength);

            volatile sal_uInt8 c = 0;
            while (nSize > nPageSize)
            {
                c ^= pData[0];
                pData += nPageSize;
                nSize -= nPageSize;
            }
            if (nSize > 0)
            {
                c^= pData[0];
                pData += nSize;
                nSize -= nSize;
            }
        }
    }
    return osl_File_E_None;
}

/*******************************************
    osl_unmapFile
********************************************/
oslFileError
SAL_CALL osl_unmapFile (void* pAddr, sal_uInt64 uLength)
{
    if (0 == pAddr)
        return osl_File_E_INVAL;

    static sal_uInt64 const g_limit_size_t = std::numeric_limits< size_t >::max();
    if (g_limit_size_t < uLength)
        return osl_File_E_OVERFLOW;
    size_t const nLength = sal::static_int_cast< size_t >(uLength);

    if (-1 == munmap(static_cast<char*>(pAddr), nLength))
        return oslTranslateFileError(OSL_FET_ERROR, errno);

    return osl_File_E_None;
}

/*******************************************
    osl_readLine
********************************************/
oslFileError
SAL_CALL osl_readLine (
    oslFileHandle   Handle,
    sal_Sequence ** ppSequence)
{
    FileHandle_Impl * pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == ppSequence))
        return osl_File_E_INVAL;
    sal_uInt64 uBytesRead = 0;

    // read at current fileptr; fileptr += uBytesRead;
    oslFileError result = pImpl->readLineAt (
        pImpl->m_fileptr, ppSequence, &uBytesRead);
    if (result == osl_File_E_None)
        pImpl->m_fileptr += uBytesRead;
    return (result);
}

/*******************************************
    osl_readFile
********************************************/
oslFileError
SAL_CALL osl_readFile (
    oslFileHandle Handle,
    void *        pBuffer,
    sal_uInt64    uBytesRequested,
    sal_uInt64 *  pBytesRead)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == pBuffer) || (0 == pBytesRead))
        return osl_File_E_INVAL;

    static sal_uInt64 const g_limit_ssize_t = std::numeric_limits< ssize_t >::max();
    if (g_limit_ssize_t < uBytesRequested)
        return osl_File_E_OVERFLOW;
    size_t const nBytesRequested = sal::static_int_cast< size_t >(uBytesRequested);

    // read at current fileptr; fileptr += *pBytesRead;
    oslFileError result = pImpl->readFileAt (
        pImpl->m_fileptr, pBuffer, nBytesRequested, pBytesRead);
    if (result == osl_File_E_None)
        pImpl->m_fileptr += *pBytesRead;
    return (result);
}

/*******************************************
    osl_writeFile
********************************************/
oslFileError
SAL_CALL osl_writeFile (
    oslFileHandle Handle,
    const void *  pBuffer,
    sal_uInt64    uBytesToWrite,
    sal_uInt64 *  pBytesWritten)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == pBuffer) || (0 == pBytesWritten))
        return osl_File_E_INVAL;
    if (0 == (pImpl->m_state & FileHandle_Impl::STATE_WRITEABLE))
        return osl_File_E_BADF;

    static sal_uInt64 const g_limit_ssize_t = std::numeric_limits< ssize_t >::max();
    if (g_limit_ssize_t < uBytesToWrite)
        return osl_File_E_OVERFLOW;
    size_t const nBytesToWrite = sal::static_int_cast< size_t >(uBytesToWrite);

    // write at current fileptr; fileptr += *pBytesWritten;
    oslFileError result = pImpl->writeFileAt (
        pImpl->m_fileptr, pBuffer, nBytesToWrite, pBytesWritten);
    if (result == osl_File_E_None)
        pImpl->m_fileptr += *pBytesWritten;
    return (result);
}

/*******************************************
    osl_readFileAt
********************************************/
oslFileError
SAL_CALL osl_readFileAt (
    oslFileHandle Handle,
    sal_uInt64    uOffset,
    void*         pBuffer,
    sal_uInt64    uBytesRequested,
    sal_uInt64*   pBytesRead)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == pBuffer) || (0 == pBytesRead))
        return osl_File_E_INVAL;
    if (0 == (pImpl->m_state & FileHandle_Impl::STATE_SEEKABLE))
        return osl_File_E_SPIPE;

    static sal_uInt64 const g_limit_off_t = std::numeric_limits< off_t >::max();
    if (g_limit_off_t < uOffset)
        return osl_File_E_OVERFLOW;
    off_t const nOffset = sal::static_int_cast< off_t >(uOffset);

    static sal_uInt64 const g_limit_ssize_t = std::numeric_limits< ssize_t >::max();
    if (g_limit_ssize_t < uBytesRequested)
        return osl_File_E_OVERFLOW;
    size_t const nBytesRequested = sal::static_int_cast< size_t >(uBytesRequested);

    // read at specified fileptr
    return pImpl->readFileAt (nOffset, pBuffer, nBytesRequested, pBytesRead);
}

/*******************************************
    osl_writeFileAt
********************************************/
oslFileError
SAL_CALL osl_writeFileAt (
    oslFileHandle Handle,
    sal_uInt64    uOffset,
    const void*   pBuffer,
    sal_uInt64    uBytesToWrite,
    sal_uInt64*   pBytesWritten)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == pBuffer) || (0 == pBytesWritten))
        return osl_File_E_INVAL;
    if (0 == (pImpl->m_state & FileHandle_Impl::STATE_SEEKABLE))
        return osl_File_E_SPIPE;
    if (0 == (pImpl->m_state & FileHandle_Impl::STATE_WRITEABLE))
        return osl_File_E_BADF;

    static sal_uInt64 const g_limit_off_t = std::numeric_limits< off_t >::max();
    if (g_limit_off_t < uOffset)
        return osl_File_E_OVERFLOW;
    off_t const nOffset = sal::static_int_cast< off_t >(uOffset);

    static sal_uInt64 const g_limit_ssize_t = std::numeric_limits< ssize_t >::max();
    if (g_limit_ssize_t < uBytesToWrite)
        return osl_File_E_OVERFLOW;
    size_t const nBytesToWrite = sal::static_int_cast< size_t >(uBytesToWrite);

    // write at specified fileptr
    return pImpl->writeFileAt (nOffset, pBuffer, nBytesToWrite, pBytesWritten);
}

/****************************************************************************/
/*  osl_isEndOfFile */
/****************************************************************************/
oslFileError
SAL_CALL osl_isEndOfFile( oslFileHandle Handle, sal_Bool *pIsEOF )
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == pIsEOF))
        return osl_File_E_INVAL;

    *pIsEOF = (pImpl->getPos() == pImpl->getSize());
    return osl_File_E_None;
}

/************************************************
 * osl_getFilePos
 ***********************************************/
oslFileError
SAL_CALL osl_getFilePos( oslFileHandle Handle, sal_uInt64* pPos )
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == pPos))
        return osl_File_E_INVAL;

    *pPos = pImpl->getPos();
    return osl_File_E_None;
}

/*******************************************
    osl_setFilePos
********************************************/
oslFileError
SAL_CALL osl_setFilePos (oslFileHandle Handle, sal_uInt32 uHow, sal_Int64 uOffset)
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd))
        return osl_File_E_INVAL;

    static sal_Int64 const g_limit_off_t = std::numeric_limits< off_t >::max();
    if (g_limit_off_t < uOffset)
        return osl_File_E_OVERFLOW;
    off_t nPos = 0, nOffset = sal::static_int_cast< off_t >(uOffset);

    switch(uHow)
    {
        case osl_Pos_Absolut:
            if (0 > nOffset)
                return osl_File_E_INVAL;
            break;

        case osl_Pos_Current:
            nPos = sal::static_int_cast< off_t >(pImpl->getPos());
            if ((0 > nOffset) && (-1*nOffset > nPos))
                return osl_File_E_INVAL;
            if (g_limit_off_t < nPos + nOffset)
                return osl_File_E_OVERFLOW;
            break;

        case osl_Pos_End:
            nPos = sal::static_int_cast< off_t >(pImpl->getSize());
            if ((0 > nOffset) && (-1*nOffset > nPos))
                return osl_File_E_INVAL;
            if (g_limit_off_t < nPos + nOffset)
                return osl_File_E_OVERFLOW;
            break;

        default:
            return osl_File_E_INVAL;
    }

    return pImpl->setPos (nPos + nOffset);
}

/****************************************************************************
 *  osl_getFileSize
 ****************************************************************************/
oslFileError
SAL_CALL osl_getFileSize( oslFileHandle Handle, sal_uInt64* pSize )
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd) || (0 == pSize))
        return osl_File_E_INVAL;

    *pSize = pImpl->getSize();
    return osl_File_E_None;
}

/************************************************
 * osl_setFileSize
 ***********************************************/
oslFileError
SAL_CALL osl_setFileSize( oslFileHandle Handle, sal_uInt64 uSize )
{
    FileHandle_Impl* pImpl = static_cast<FileHandle_Impl*>(Handle);

    if ((0 == pImpl) || (-1 == pImpl->m_fd))
        return osl_File_E_INVAL;
    if (0 == (pImpl->m_state & FileHandle_Impl::STATE_WRITEABLE))
        return osl_File_E_BADF;

    static sal_uInt64 const g_limit_off_t = std::numeric_limits< off_t >::max();
    if (g_limit_off_t < uSize)
        return osl_File_E_OVERFLOW;

    oslFileError result = pImpl->syncFile();
    if (result != osl_File_E_None)
        return (result);
    pImpl->m_bufptr = -1, pImpl->m_buflen = 0;

    return pImpl->setSize (uSize);
}

/****************************************************************************/
/*  osl_moveFile */
/****************************************************************************/

oslFileError osl_moveFile( rtl_uString* ustrFileURL, rtl_uString* ustrDestURL )
{
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];
    oslFileError eRet;
    APIRET rc;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( ustrDestURL );

    /* convert source url to system path */
    eRet = FileURLToPath( srcPath, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    /* convert destination url to system path */
    eRet = FileURLToPath( destPath, PATH_MAX, ustrDestURL );
    if( eRet != osl_File_E_None )
        return eRet;

    //YD 01/05/06 rename() can overwrite existing files.
    rc = DosDelete( (PCSZ)destPath);
    rc = DosMove( (PCSZ)srcPath, (PCSZ)destPath);
    if (!rc)
        eRet = osl_File_E_None;
    else
        eRet = MapError( rc);

    return eRet;
}

/****************************************************************************/
/*  osl_copyFile */
/****************************************************************************/

#define TMP_DEST_FILE_EXTENSION ".osl-tmp"

static oslFileError oslDoCopy(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, mode_t nMode, size_t nSourceSize, int DestFileExists)
{
    int      nRet=0;
    sal_Char pszTmpDestFile[PATH_MAX];
    size_t   size_tmp_dest_buff = sizeof(pszTmpDestFile);

    /* Quick fix for #106048, the whole copy file function seems
       to be erroneous anyway and needs to be rewritten.
       Besides osl_copyFile is currently not used from OO/SO code.
    */
    memset(pszTmpDestFile, 0, size_tmp_dest_buff);

    if ( DestFileExists )
    {
        strncpy(pszTmpDestFile, pszDestFileName, size_tmp_dest_buff - 1);

        if ((strlen(pszTmpDestFile) + strlen(TMP_DEST_FILE_EXTENSION)) >= size_tmp_dest_buff)
            return osl_File_E_NAMETOOLONG;

        strncat(pszTmpDestFile, TMP_DEST_FILE_EXTENSION, strlen(TMP_DEST_FILE_EXTENSION));

        /* FIXME: what if pszTmpDestFile already exists? */
        /*        with getcanonical??? */
        nRet=rename(pszDestFileName,pszTmpDestFile);
    }

    /* mfe: should be S_ISREG */
    if ( !S_ISLNK(nMode) )
    {
        /* copy SourceFile to DestFile */
        nRet = oslDoCopyFile(pszSourceFileName,pszDestFileName,nSourceSize, nMode);
    }
    /* mfe: OK redundant at the moment */
    else if ( S_ISLNK(nMode) )
    {
        nRet = oslDoCopyLink(pszSourceFileName,pszDestFileName);
    }
    else
    {
        /* mfe: what to do here? */
        nRet=ENOSYS;
    }

    if ( nRet > 0 && DestFileExists == 1 )
    {
        unlink(pszDestFileName);
        rename(pszTmpDestFile,pszDestFileName);
    }

    if ( nRet > 0 )
    {
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    if ( DestFileExists == 1 )
    {
        unlink(pszTmpDestFile);
    }

    return osl_File_E_None;
}

/*****************************************
 * oslChangeFileModes
 ****************************************/

static oslFileError oslChangeFileModes( const sal_Char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID)
{
    int nRet=0;
    struct utimbuf aTimeBuffer;

    nRet = chmod(pszFileName,nMode);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    aTimeBuffer.actime=nAcTime;
    aTimeBuffer.modtime=nModTime;
    nRet=utime(pszFileName,&aTimeBuffer);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    if ( nUID != getuid() )
    {
        nUID=getuid();
    }

    nRet=chown(pszFileName,nUID,nGID);
    if ( nRet < 0 )
    {
        nRet=errno;

        /* mfe: do not return an error here! */
        /* return oslTranslateFileError(nRet);*/
    }

    return osl_File_E_None;
}

/*****************************************
 * oslDoCopyLink
 ****************************************/

static int oslDoCopyLink(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName)
{
    int nRet=0;

    /* mfe: if dest file is symbolic link remove the link and place the file instead (hro says so) */
    /* mfe: if source is a link copy the link and not the file it points to (hro says so) */
    sal_Char pszLinkContent[PATH_MAX];

    pszLinkContent[0] = '\0';

    nRet = readlink(pszSourceFileName,pszLinkContent,PATH_MAX);

    if ( nRet < 0 )
    {
        nRet=errno;
        return nRet;
    }
    else
        pszLinkContent[ nRet ] = 0;

    nRet = symlink(pszLinkContent,pszDestFileName);

    if ( nRet < 0 )
    {
        nRet=errno;
        return nRet;
    }

    return 0;
}

/*****************************************
 * oslDoCopyFile
 ****************************************/

static int oslDoCopyFile(const sal_Char* pszSourceFileName, const sal_Char* pszDestFileName, size_t nSourceSize, mode_t mode)
{
    int SourceFileFD=0;
    int DestFileFD=0;
    int nRet=0;
    void* pSourceFile=0;
    char    buffer[ 4096];

    SourceFileFD=open(pszSourceFileName,O_RDONLY | O_BINARY);
    if ( SourceFileFD < 0 )
    {
        nRet=errno;
        return nRet;
    }

    DestFileFD=open(pszDestFileName, O_WRONLY | O_CREAT | O_BINARY, mode);
    if ( DestFileFD < 0 )
    {
        nRet=errno;
        close(SourceFileFD);
        return nRet;
    }

    /* HACK: because memory mapping fails on various
       platforms if the size of the source file is  0 byte */
    if (0 == nSourceSize)
    {
        close(SourceFileFD);
        close(DestFileFD);
        return 0;
    }

    while( (nRet = read(SourceFileFD, buffer, sizeof(buffer))) !=0 )
    {
        nRet = write( DestFileFD, buffer, nRet);
    }

    close(SourceFileFD);
    close(DestFileFD);

    return nRet;
}

static oslFileError osl_psz_copyFile( const sal_Char* pszPath, const sal_Char* pszDestPath )
{
    time_t nAcTime=0;
    time_t nModTime=0;
    uid_t nUID=0;
    gid_t nGID=0;
    int nRet=0;
    mode_t nMode=0;
    struct stat aFileStat;
    oslFileError tErr=osl_File_E_invalidError;
    size_t nSourceSize=0;
    int DestFileExists=1;

    /* mfe: does the source file really exists? */
    nRet = lstat(pszPath,&aFileStat);

    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    /* mfe: we do only copy files here! */
    if ( S_ISDIR(aFileStat.st_mode) )
    {
        return osl_File_E_ISDIR;
    }

    nSourceSize=(size_t)aFileStat.st_size;
    nMode=aFileStat.st_mode;
    nAcTime=aFileStat.st_atime;
    nModTime=aFileStat.st_mtime;
    nUID=aFileStat.st_uid;
    nGID=aFileStat.st_gid;

    nRet = stat(pszDestPath,&aFileStat);
    if ( nRet < 0 )
    {
        nRet=errno;

        if ( nRet == ENOENT )
        {
            DestFileExists=0;
        }
/*        return oslTranslateFileError(nRet);*/
    }

    /* mfe: the destination file must not be a directory! */
    if ( nRet == 0 && S_ISDIR(aFileStat.st_mode) )
    {
        return osl_File_E_ISDIR;
    }
    else
    {
        /* mfe: file does not exists or is no dir */
    }

    tErr = oslDoCopy(pszPath,pszDestPath,nMode,nSourceSize,DestFileExists);

    if ( tErr != osl_File_E_None )
    {
        return tErr;
    }

    /*
     *   mfe: ignore return code
     *        since only  the success of the copy is
     *        important
     */
    oslChangeFileModes(pszDestPath,nMode,nAcTime,nModTime,nUID,nGID);

    return tErr;
}

oslFileError osl_copyFile( rtl_uString* ustrFileURL, rtl_uString* ustrDestURL )
{
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];
    oslFileError eRet;
    APIRET rc;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( ustrDestURL );

    /* convert source url to system path */
    eRet = FileURLToPath( srcPath, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    /* convert destination url to system path */
    eRet = FileURLToPath( destPath, PATH_MAX, ustrDestURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return osl_psz_copyFile( srcPath, destPath );
}

/****************************************************************************/
/*  osl_removeFile */
/****************************************************************************/

oslFileError osl_removeFile( rtl_uString* ustrFileURL )
{
    char path[PATH_MAX];
    oslFileError eRet;
    APIRET rc;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    rc = DosDelete( (PCSZ)path);
    if (!rc)
        eRet = osl_File_E_None;
    else
        eRet = MapError( rc);

    return eRet;
}

/****************************************************************************/
/*  osl_getVolumeInformation */
/****************************************************************************/

#define TXFSDC_BLOCKR         0x00              // block device removable
#define TXFSDC_GETBPB         0x00              // get device bpb info
#define TXFSBPB_REMOVABLE     0x08              // BPB attribute for removable

typedef struct drivecmd
{
   BYTE                cmd;                     // 0=unlock 1=lock 2=eject
   BYTE                drv;                     // 0=A, 1=B 2=C ...
} DRIVECMD;                                     // end of struct "drivecmd"

#pragma pack(push, 1)                           // byte packing
typedef struct txfs_ebpb                        // ext. boot parameter block
{                                               // at offset 0x0b in bootsector
   USHORT              SectSize;                // 0B bytes per sector
   BYTE                ClustSize;               // 0D sectors per cluster
   USHORT              FatOffset;               // 0E sectors to 1st FAT
   BYTE                NrOfFats;                // 10 nr of FATS     (FAT only)
   USHORT              RootEntries;             // 11 Max entries \ (FAT only)
   USHORT              Sectors;                 // 13 nr of sectors if <  64K
   BYTE                MediaType;               // 15 mediatype (F8 for HD)
   USHORT              FatSectors;              // 16 sectors/FAT (FAT only)
   USHORT              LogGeoSect;              // 18 sectors/Track
   USHORT              LogGeoHead;              // 1a nr of heads
   ULONG               HiddenSectors;           // 1c sector-offset from MBR/EBR
   ULONG               BigSectors;              // 20 nr of sectors if >= 64K
} TXFS_EBPB;                                    // last byte is at offset 0x23

typedef struct drivebpb
{
   TXFS_EBPB           ebpb;                    // extended BPB
   BYTE                reserved[6];
   USHORT              cyls;
   BYTE                type;
   USHORT              attributes;              // device attributes
   BYTE                fill[6];                 // documented for IOCtl
} DRIVEBPB;                                     // end of struct "drivebpb"

struct CDInfo {
    USHORT usCount;
    USHORT usFirst;
};

#pragma pack(pop)

/*****************************************************************************/
// Get number of cdrom readers
/*****************************************************************************/
BOOL GetCDInfo( CDInfo * pCDInfo )
{
    HFILE hFileCD;
    ULONG ulAction;

    if( NO_ERROR == DosOpen( (PCSZ)"\\DEV\\CD-ROM2$",
                            &hFileCD, &ulAction, 0, FILE_NORMAL,
                            OPEN_ACTION_OPEN_IF_EXISTS,
                            OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, NULL )) {
        ULONG  ulDataSize = sizeof(CDInfo);
        APIRET rc = DosDevIOCtl( hFileCD, 0x82, 0x60, NULL, 0,
                                 NULL, (PVOID)pCDInfo, ulDataSize, &ulDataSize);
        DosClose( hFileCD);
        if(rc == NO_ERROR)
            return TRUE;
    }
    // failed
    pCDInfo->usFirst = 0;
    pCDInfo->usCount = 0;
    return FALSE;
}

/*****************************************************************************/
// Determine if unit is a cdrom or not
/*****************************************************************************/
BOOL DriveIsCDROM(UINT uiDrive, CDInfo *pCDInfo)
{
    return (uiDrive >= pCDInfo->usFirst)
            && (uiDrive < (pCDInfo->usFirst + pCDInfo->usCount));
}

/*****************************************************************************/
// Determine attached fstype, e.g. HPFS for specified drive
/*****************************************************************************/
BOOL TxFsType                                   // RET   FS type resolved
(
   char               *drive,                   // IN    Drive specification
   char               *fstype,                  // OUT   Attached FS type
   char               *details                  // OUT   details (UNC) or NULL
)
{
   BOOL                rc = FALSE;
   FSQBUFFER2         *fsinfo;                     // Attached FS info
   ULONG               fsdlen = 2048;              // Fs info data length

   strcpy(fstype, "none");
   if (details)
   {
      strcpy(details, "");
   }
   if ((fsinfo = (FSQBUFFER2*)calloc(1, fsdlen)) != NULL)
   {
      if (DosQFSAttach((PCSZ)drive, 0, 1, fsinfo, &fsdlen) == NO_ERROR)
      {
         strcpy(fstype, (char*) fsinfo->szName + fsinfo->cbName +1);
         if (details && (fsinfo->cbFSAData != 0))
         {
            strcpy( details, (char*) fsinfo->szName + fsinfo->cbName +
                                              fsinfo->cbFSDName +2);
         }
         rc = TRUE;
      }
      free(fsinfo);
   }
   return (rc);
}                                               // end 'TxFsType'
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
// Determine if a driveletter represents a removable medium/device
/*****************************************************************************/
BOOL TxFsIsRemovable                            // RET   drive is removable
(
   char               *drive                    // IN    Driveletter to test
)
{
   BOOL                rc = FALSE;
   DRIVECMD            IOCtl;
   DRIVEBPB            RemAt;
   ULONG               DataLen;
   ULONG               ParmLen;
   BYTE                NoRem;

   DosError( FERR_DISABLEHARDERR);              // avoid 'not ready' popups

   ParmLen   = sizeof(IOCtl);
   IOCtl.cmd = TXFSDC_BLOCKR;
   IOCtl.drv = toupper(drive[0]) - 'A';
   DataLen   = sizeof(NoRem);

   if (DosDevIOCtl((HFILE) -1, IOCTL_DISK,
                               DSK_BLOCKREMOVABLE,
                               &IOCtl, ParmLen, &ParmLen,
                               &NoRem, DataLen, &DataLen) == NO_ERROR)
   {
      if (NoRem)                                // non-removable sofar, check
      {                                         // BPB as well (USB devices)
         ParmLen   = sizeof(IOCtl);
         IOCtl.cmd = TXFSDC_GETBPB;
         IOCtl.drv = toupper(drive[0]) - 'A';
         DataLen   = sizeof(RemAt);

         if (DosDevIOCtl((HFILE) -1, IOCTL_DISK,
                                     DSK_GETDEVICEPARAMS,
                                     &IOCtl, ParmLen, &ParmLen,
                                     &RemAt, DataLen, &DataLen) == NO_ERROR)

         {
            if (RemAt.attributes & TXFSBPB_REMOVABLE)
            {
               rc = TRUE;                       // removable, probably USB
            }
         }
      }
      else
      {
         rc = TRUE;                             // removable block device
      }
   }
   DosError( FERR_ENABLEHARDERR);               // enable criterror handler
   return (rc);
}                                               // end 'TxFsIsRemovable'
/*---------------------------------------------------------------------------*/

static oslFileError get_drive_type(const char* path, oslVolumeInfo* pInfo)
{
    char        Drive_Letter = toupper( *path);
    char        fstype[ 64];

    pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;

    // check for floppy A/B
    BYTE    nFloppies;
    APIRET  rc;
    rc = DosDevConfig( (void*) &nFloppies, DEVINFO_FLOPPY );
    if ((Drive_Letter - 'A') < nFloppies) {
        pInfo->uAttributes |= osl_Volume_Attribute_Removeable;
        pInfo->uAttributes |= osl_Volume_Attribute_FloppyDisk;
        return osl_File_E_None;
    }

    // query system for CD drives
    CDInfo cdInfo;
    GetCDInfo(&cdInfo);

    // query if drive is a CDROM
    if (DriveIsCDROM( Drive_Letter - 'A', &cdInfo))
        pInfo->uAttributes |= osl_Volume_Attribute_CompactDisc | osl_Volume_Attribute_Removeable;

    if (TxFsIsRemovable( (char*)path))
        pInfo->uAttributes |= osl_Volume_Attribute_Removeable;

    if (TxFsType( (char*)path, fstype, NULL) == FALSE) {
        // query failed, assume fixed disk
        pInfo->uAttributes |= osl_Volume_Attribute_FixedDisk;
        return osl_File_E_None;
    }

    //- Note, connected Win-NT drives use the REAL FS-name like NTFS!
    if ((strncasecmp( fstype, "LAN", 3) == 0)           //- OS/2 LAN drives
        || (strncasecmp( fstype, "NDFS", 4) == 0)   //- NetDrive
        || (strncasecmp( fstype, "REMOTE", 5) == 0)  )  //- NT disconnected
        pInfo->uAttributes |= osl_Volume_Attribute_Remote;
    else if (strncasecmp( fstype, "RAMFS", 5) == 0)
        pInfo->uAttributes |= osl_Volume_Attribute_RAMDisk;
    else if ((strncasecmp( fstype, "CD",  2) == 0)      // OS2:CDFS, DOS/WIN:CDROM
        || (strncasecmp( fstype, "UDF", 3) == 0)   )    // OS2:UDF DVD's
        pInfo->uAttributes |= osl_Volume_Attribute_CompactDisc | osl_Volume_Attribute_Removeable;
    else
        pInfo->uAttributes |= osl_Volume_Attribute_FixedDisk;

    return osl_File_E_None;
}

//#############################################
inline bool is_volume_space_info_request(sal_uInt32 field_mask)
{
    return (field_mask &
            (osl_VolumeInfo_Mask_TotalSpace |
             osl_VolumeInfo_Mask_UsedSpace  |
             osl_VolumeInfo_Mask_FreeSpace));
}

//#############################################
static void get_volume_space_information(const char* path, oslVolumeInfo *pInfo)
{
    FSALLOCATE aFSInfoBuf;
    ULONG nDriveNumber = toupper( *path) - 'A' + 1;

    // disable error popups
    DosError(FERR_DISABLEHARDERR);
    APIRET rc = DosQueryFSInfo( nDriveNumber, FSIL_ALLOC,
                                &aFSInfoBuf, sizeof(aFSInfoBuf) );
    // enable error popups
    DosError(FERR_ENABLEHARDERR);
    if (!rc)
    {
        uint64_t aBytesPerCluster( uint64_t(aFSInfoBuf.cbSector) *
                                 uint64_t(aFSInfoBuf.cSectorUnit) );
        pInfo->uFreeSpace = aBytesPerCluster * uint64_t(aFSInfoBuf.cUnitAvail);
        pInfo->uTotalSpace = aBytesPerCluster * uint64_t(aFSInfoBuf.cUnit);
        pInfo->uUsedSpace    = pInfo->uTotalSpace - pInfo->uFreeSpace;
        pInfo->uValidFields |= osl_VolumeInfo_Mask_TotalSpace |
                               osl_VolumeInfo_Mask_UsedSpace |
                               osl_VolumeInfo_Mask_FreeSpace;
    }
}

//#############################################
inline bool is_filesystem_attributes_request(sal_uInt32 field_mask)
{
    return (field_mask &
            (osl_VolumeInfo_Mask_MaxNameLength |
             osl_VolumeInfo_Mask_MaxPathLength |
             osl_VolumeInfo_Mask_FileSystemName |
             osl_VolumeInfo_Mask_FileSystemCaseHandling));
}

//#############################################
inline bool is_drivetype_request(sal_uInt32 field_mask)
{
    return (field_mask & osl_VolumeInfo_Mask_Attributes);
}

typedef struct _FSQBUFFER_
{
    FSQBUFFER2  aBuf;
    UCHAR       sBuf[64];
} FSQBUFFER_;

//#############################################
static oslFileError get_filesystem_attributes(const char* path, sal_uInt32 field_mask, oslVolumeInfo* pInfo)
{
    pInfo->uAttributes = 0;

    oslFileError osl_error = osl_File_E_None;

    // osl_get_drive_type must be called first because
    // this function resets osl_VolumeInfo_Mask_Attributes
    // on failure
    if (is_drivetype_request(field_mask))
        osl_error = get_drive_type(path, pInfo);

    if ((osl_File_E_None == osl_error) && is_filesystem_attributes_request(field_mask))
    {
        FSQBUFFER_  aBuf;
        ULONG       nBufLen;
        APIRET      nRet;

        nBufLen = sizeof( aBuf );
        // disable error popups
        DosError(FERR_DISABLEHARDERR);
        nRet = DosQueryFSAttach( (PCSZ)path, 0, FSAIL_QUERYNAME, (_FSQBUFFER2*) &aBuf, &nBufLen );
        if ( !nRet )
        {
            char *pType = (char*)(aBuf.aBuf.szName + aBuf.aBuf.cbName + 1);
            pInfo->uValidFields   |= osl_VolumeInfo_Mask_MaxNameLength;
            pInfo->uMaxNameLength  = _MAX_FNAME;

            pInfo->uValidFields   |= osl_VolumeInfo_Mask_MaxPathLength;
            pInfo->uMaxPathLength  = _MAX_PATH;

            pInfo->uValidFields   |= osl_VolumeInfo_Mask_FileSystemName;
            rtl_uString_newFromAscii(&pInfo->ustrFileSystemName, pType);

            // case is preserved always except for FAT
            if (strcmp( pType, "FAT" ))
                pInfo->uAttributes |= osl_Volume_Attribute_Case_Is_Preserved;

            pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;
        }
        // enable error popups
        DosError(FERR_ENABLEHARDERR);
    }
    return osl_error;
}

oslFileError SAL_CALL osl_getVolumeInformation( rtl_uString* ustrDirectoryURL, oslVolumeInfo* pInfo, sal_uInt32 uFieldMask )
{
    char volume_root[PATH_MAX];
    oslFileError error;

    OSL_ASSERT( ustrDirectoryURL );
    OSL_ASSERT( pInfo );

    /* convert directory url to system path */
    error = FileURLToPath( volume_root, PATH_MAX, ustrDirectoryURL );
    if( error != osl_File_E_None )
        return error;

    if (!pInfo)
        return osl_File_E_INVAL;

    pInfo->uValidFields = 0;

    if ((error = get_filesystem_attributes(volume_root, uFieldMask, pInfo)) != osl_File_E_None)
        return error;

    if (is_volume_space_info_request(uFieldMask))
        get_volume_space_information(volume_root, pInfo);

    if (uFieldMask & osl_VolumeInfo_Mask_DeviceHandle)
    {
        pInfo->uValidFields |= osl_VolumeInfo_Mask_DeviceHandle;
        rtl_uString* uVolumeRoot;
        rtl_uString_newFromAscii( &uVolumeRoot, volume_root);
        osl_getFileURLFromSystemPath( uVolumeRoot, (rtl_uString**)&pInfo->pDeviceHandle);
        rtl_uString_release( uVolumeRoot);
    }

    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_getFileStatus */
/****************************************************************************/
static oslFileError _osl_getDriveInfo(
    oslDirectoryItem Item, oslFileStatus *pStatus, sal_uInt32 uFieldMask)
{
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;
    sal_Unicode         cDrive[3];
    sal_Unicode         cRoot[4];

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    pStatus->uValidFields = 0;

    cDrive[0] = pItemImpl->ustrDrive->buffer[0];
    cDrive[1] = (sal_Unicode)':';
    cDrive[2] = 0;
    cRoot[0] = pItemImpl->ustrDrive->buffer[0];
    cRoot[1] = (sal_Unicode)':';
    cRoot[2] = 0;

    if ( uFieldMask & osl_FileStatus_Mask_FileName )
    {
        if ( pItemImpl->ustrDrive->buffer[0] == '\\' &&
            pItemImpl->ustrDrive->buffer[1] == '\\' )
        {
            LPCWSTR lpFirstBkSlash = wcschr( (const wchar_t*)&pItemImpl->ustrDrive->buffer[2], '\\' );

            if ( lpFirstBkSlash && lpFirstBkSlash[1] )
            {
                LPCWSTR lpLastBkSlash = wcschr( (const wchar_t*)&lpFirstBkSlash[1], '\\' );

                if ( lpLastBkSlash )
                    rtl_uString_newFromStr_WithLength( &pStatus->ustrFileName, (sal_Unicode*)&lpFirstBkSlash[1], lpLastBkSlash - lpFirstBkSlash - 1 );
                else
                    rtl_uString_newFromStr( &pStatus->ustrFileName, (sal_Unicode*)&lpFirstBkSlash[1] );
                pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
            }
        }
        else
        {
            FSINFO  aFSInfoBuf;
            ULONG   ulFSInfoLevel = FSIL_VOLSER;
            ULONG   nDriveNumber;
            char    szFileName[ _MAX_PATH];

            nDriveNumber = toupper(*cDrive) - 'A' + 1;
            memset( &aFSInfoBuf, 0, sizeof(FSINFO) );
            // disable error popups
            DosError(FERR_DISABLEHARDERR);
            APIRET rc = DosQueryFSInfo( nDriveNumber, ulFSInfoLevel, &aFSInfoBuf, sizeof(FSINFO) );
            // enable error popups
            DosError(FERR_ENABLEHARDERR);
            memset( szFileName, 0, sizeof( szFileName));
            *szFileName = toupper(*cDrive);
            strcat( szFileName, ": [");
            if ( !rc || aFSInfoBuf.vol.cch)
                strncat( szFileName, aFSInfoBuf.vol.szVolLabel, aFSInfoBuf.vol.cch);
            strcat( szFileName, "]");
            rtl_uString_newFromAscii( &pStatus->ustrFileName, szFileName );

            pStatus->uValidFields |= osl_FileStatus_Mask_FileName;
        }
    }

    pStatus->eType = osl_File_Type_Volume;
    pStatus->uValidFields |= osl_FileStatus_Mask_Type;

    if ( uFieldMask & osl_FileStatus_Mask_FileURL )
    {
        rtl_uString *ustrSystemPath = NULL;

        rtl_uString_newFromStr( &ustrSystemPath, pItemImpl->ustrDrive->buffer );
        osl_getFileURLFromSystemPath( ustrSystemPath, &pStatus->ustrFileURL );
        rtl_uString_release( ustrSystemPath );
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }

    return osl_File_E_None;
}

oslFileError SAL_CALL osl_getFileStatus(
    oslDirectoryItem Item,
    oslFileStatus *pStatus,
    sal_uInt32 uFieldMask )
{
    DirectoryItem_Impl  *pItemImpl = (DirectoryItem_Impl *)Item;
    struct stat file_stat;

    if ( !pItemImpl )
        return osl_File_E_INVAL;

    if ( pItemImpl->uType == DIRECTORYITEM_DRIVE)
        return _osl_getDriveInfo( Item, pStatus, uFieldMask );

    osl::lstat(pItemImpl->ustrFilePath, file_stat);
    if ( uFieldMask & osl_FileStatus_Mask_Validate )
    {
        uFieldMask &= ~ osl_FileStatus_Mask_Validate;
    }

    /* If no fields to retrieve left ignore pStatus */
    if ( !uFieldMask )
        return osl_File_E_None;

    /* Otherwise, this must be a valid pointer */
    if ( !pStatus )
        return osl_File_E_INVAL;

    if ( pStatus->uStructSize != sizeof(oslFileStatus) )
        return osl_File_E_INVAL;

    pStatus->uValidFields = 0;

    /* File time stamps */

    if ( (uFieldMask & osl_FileStatus_Mask_ModifyTime))
    {
        pStatus->aModifyTime.Seconds  = file_stat.st_mtime;
        pStatus->aModifyTime.Nanosec  = 0;
        pStatus->uValidFields |= osl_FileStatus_Mask_ModifyTime;
    }

    if ( (uFieldMask & osl_FileStatus_Mask_AccessTime))
    {
        pStatus->aAccessTime.Seconds  = file_stat.st_atime;
        pStatus->aAccessTime.Nanosec  = 0;
        pStatus->uValidFields |= osl_FileStatus_Mask_AccessTime;
    }

    if ( (uFieldMask & osl_FileStatus_Mask_CreationTime))
    {
        pStatus->aAccessTime.Seconds  = file_stat.st_birthtime;
        pStatus->aAccessTime.Nanosec  = 0;
        pStatus->uValidFields |= osl_FileStatus_Mask_CreationTime;
    }

    /* Most of the fields are already set, regardless of requiered fields */

    osl_systemPathGetFileNameOrLastDirectoryPart(pItemImpl->ustrFilePath, &pStatus->ustrFileName);
    pStatus->uValidFields |= osl_FileStatus_Mask_FileName;

    if (S_ISLNK(file_stat.st_mode))
       pStatus->eType = osl_File_Type_Link;
    else if (S_ISDIR(file_stat.st_mode))
       pStatus->eType = osl_File_Type_Directory;
    else if (S_ISREG(file_stat.st_mode))
       pStatus->eType = osl_File_Type_Regular;
    else if (S_ISFIFO(file_stat.st_mode))
       pStatus->eType = osl_File_Type_Fifo;
    else if (S_ISSOCK(file_stat.st_mode))
       pStatus->eType = osl_File_Type_Socket;
    else if (S_ISCHR(file_stat.st_mode) || S_ISBLK(file_stat.st_mode))
       pStatus->eType = osl_File_Type_Special;
    else
       pStatus->eType = osl_File_Type_Unknown;

    pStatus->uValidFields |= osl_FileStatus_Mask_Type;

    pStatus->uAttributes = pItemImpl->d_attr;
    pStatus->uValidFields |= osl_FileStatus_Mask_Attributes;

    pStatus->uFileSize = file_stat.st_size;
    pStatus->uValidFields |= osl_FileStatus_Mask_FileSize;

    if ( uFieldMask & osl_FileStatus_Mask_LinkTargetURL )
    {
        rtl_uString *ustrFullPath = NULL;

        rtl_uString_newFromStr( &ustrFullPath, rtl_uString_getStr(pItemImpl->ustrFilePath) );
        osl_getFileURLFromSystemPath( ustrFullPath, &pStatus->ustrLinkTargetURL );
        rtl_uString_release( ustrFullPath );

        pStatus->uValidFields |= osl_FileStatus_Mask_LinkTargetURL;
    }

    if ( uFieldMask & osl_FileStatus_Mask_FileURL )
    {
        rtl_uString *ustrFullPath = NULL;

        rtl_uString_newFromStr( &ustrFullPath, rtl_uString_getStr(pItemImpl->ustrFilePath) );
        osl_getFileURLFromSystemPath( ustrFullPath, &pStatus->ustrFileURL );
        rtl_uString_release( ustrFullPath );
        pStatus->uValidFields |= osl_FileStatus_Mask_FileURL;
    }

    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_createDirectory */
/****************************************************************************/

oslFileError osl_createDirectory( rtl_uString* ustrDirectoryURL )
{
    char path[PATH_MAX];
    oslFileError eRet;
    APIRET rc;

    OSL_ASSERT( ustrDirectoryURL );

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

    rc = DosCreateDir( (PCSZ)path, NULL);
    if (rc == ERROR_ACCESS_DENIED)
       rc=ERROR_FILE_EXISTS;

    if (!rc)
        eRet = osl_File_E_None;
    else
        eRet = MapError( rc);

    return eRet;
}

/****************************************************************************/
/*  osl_removeDirectory */
/****************************************************************************/

oslFileError osl_removeDirectory( rtl_uString* ustrDirectoryURL )
{
    char path[PATH_MAX];
    oslFileError eRet;
    APIRET rc;

    OSL_ASSERT( ustrDirectoryURL );

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

    rc = DosDeleteDir( (PCSZ)path);
    if (!rc)
        eRet = osl_File_E_None;
    else
        eRet = MapError( rc);

    return eRet;
}

//#############################################
int path_make_parent(sal_Unicode* path)
{
    int i = rtl_ustr_lastIndexOfChar(path, '/');

    if (i > 0)
    {
        *(path + i) = 0;
        return i;
    }
    else
        return 0;
}

//#############################################
int create_dir_with_callback(
    sal_Unicode* directory_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    int mode = S_IRWXU | S_IRWXG | S_IRWXO;

    if (osl::mkdir(directory_path, mode) == 0)
    {
        if (aDirectoryCreationCallbackFunc)
        {
            rtl::OUString url;
            osl::FileBase::getFileURLFromSystemPath(directory_path, url);
            aDirectoryCreationCallbackFunc(pData, url.pData);
        }
        return 0;
    }
    return errno;
}

//#############################################
oslFileError create_dir_recursively_(
    sal_Unicode* dir_path,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    OSL_PRECOND((rtl_ustr_getLength(dir_path) > 0) && ((dir_path + (rtl_ustr_getLength(dir_path) - 1)) != (dir_path + rtl_ustr_lastIndexOfChar(dir_path, '/'))), \
    "Path must not end with a slash");

    int native_err = create_dir_with_callback(
        dir_path, aDirectoryCreationCallbackFunc, pData);

    if (native_err == 0)
        return osl_File_E_None;

    if (native_err != ENOENT)
        return oslTranslateFileError(OSL_FET_ERROR, native_err);

    // we step back until '/a_dir' at maximum because
    // we should get an error unequal ENOENT when
    // we try to create 'a_dir' at '/' and would so
    // return before
    int pos = path_make_parent(dir_path);

    oslFileError osl_error = create_dir_recursively_(
        dir_path, aDirectoryCreationCallbackFunc, pData);

    if (osl_File_E_None != osl_error)
        return osl_error;

       dir_path[pos] = '/';

    return create_dir_recursively_(dir_path, aDirectoryCreationCallbackFunc, pData);
}

//#######################################
oslFileError SAL_CALL osl_createDirectoryPath(
    rtl_uString* aDirectoryUrl,
    oslDirectoryCreationCallbackFunc aDirectoryCreationCallbackFunc,
    void* pData)
{
    if (aDirectoryUrl == NULL)
        return osl_File_E_INVAL;

    rtl::OUString sys_path;
    oslFileError osl_error = osl_getSystemPathFromFileURL_Ex(
        aDirectoryUrl, &sys_path.pData, sal_False);

    if (osl_error != osl_File_E_None)
        return osl_error;

    osl::systemPathRemoveSeparator(sys_path);

    // const_cast because sys_path is a local copy which we want to modify inplace instead of
    // coyp it into another buffer on the heap again
    return create_dir_recursively_(sys_path.pData->buffer, aDirectoryCreationCallbackFunc, pData);
}

/****************************************************************************/
/*  osl_getCanonicalName */
/****************************************************************************/

oslFileError osl_getCanonicalName( rtl_uString* ustrFileURL, rtl_uString** pustrValidURL )
{
    OSL_ENSURE(sal_False, "osl_getCanonicalName not implemented");

    rtl_uString_newFromString(pustrValidURL, ustrFileURL);
    return osl_File_E_None;
}


/****************************************************************************/
/*  osl_setFileAttributes */
/****************************************************************************/

oslFileError osl_setFileAttributes( rtl_uString* ustrFileURL, sal_uInt64 uAttributes )
{
    char         path[PATH_MAX];
    oslFileError eRet;
    FILESTATUS3  fsts3ConfigInfo;
    ULONG        ulBufSize     = sizeof(FILESTATUS3);
    APIRET       rc            = NO_ERROR;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    /* query current attributes */
    rc = DosQueryPathInfo( (PCSZ)path, FIL_STANDARD, &fsts3ConfigInfo, ulBufSize);
    if (rc != NO_ERROR)
        return MapError( rc);

    /* set/reset readonly/hidden (see w32\file.cxx) */
    fsts3ConfigInfo.attrFile &= ~(FILE_READONLY | FILE_HIDDEN);
    if ( uAttributes & osl_File_Attribute_ReadOnly )
        fsts3ConfigInfo.attrFile |= FILE_READONLY;
    if ( uAttributes & osl_File_Attribute_Hidden )
        fsts3ConfigInfo.attrFile |= FILE_HIDDEN;

    /* write new attributes */
    rc = DosSetPathInfo( (PCSZ)path, FIL_STANDARD, &fsts3ConfigInfo, ulBufSize, 0);
    if (rc != NO_ERROR)
        return MapError( rc);

    /* everything ok */
    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_setFileTime */
/****************************************************************************/

oslFileError osl_setFileTime( rtl_uString* ustrFileURL, const TimeValue* pCreationTime,
                              const TimeValue* pLastAccessTime, const TimeValue* pLastWriteTime )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return osl_psz_setFileTime( path, pCreationTime, pLastAccessTime, pLastWriteTime );
}

/******************************************************************************
 *
 *                  Exported Module Functions
 *             (independent of C or Unicode Strings)
 *
 *****************************************************************************/



/******************************************************************************
 *
 *                  C-String Versions of Exported Module Functions
 *
 *****************************************************************************/


/******************************************
 * osl_psz_setFileTime
 *****************************************/

static oslFileError osl_psz_setFileTime( const sal_Char* pszFilePath,
                                  const TimeValue* /*pCreationTime*/,
                                  const TimeValue* pLastAccessTime,
                                  const TimeValue* pLastWriteTime )
{
    int nRet=0;
    struct utimbuf aTimeBuffer;
    struct stat aFileStat;
#ifdef DEBUG_OSL_FILE
    struct tm* pTM=0;
#endif

    nRet = lstat(pszFilePath,&aFileStat);

    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"File Times are (in localtime):\n");
    pTM=localtime(&aFileStat.st_ctime);
    fprintf(stderr,"CreationTime is '%s'\n",asctime(pTM));
    pTM=localtime(&aFileStat.st_atime);
    fprintf(stderr,"AccessTime   is '%s'\n",asctime(pTM));
    pTM=localtime(&aFileStat.st_mtime);
    fprintf(stderr,"Modification is '%s'\n",asctime(pTM));

    fprintf(stderr,"File Times are (in UTC):\n");
    fprintf(stderr,"CreationTime is '%s'\n",ctime(&aFileStat.st_ctime));
    fprintf(stderr,"AccessTime   is '%s'\n",ctime(&aTimeBuffer.actime));
    fprintf(stderr,"Modification is '%s'\n",ctime(&aTimeBuffer.modtime));
#endif

    if ( pLastAccessTime != 0 )
    {
        aTimeBuffer.actime=pLastAccessTime->Seconds;
    }
    else
    {
        aTimeBuffer.actime=aFileStat.st_atime;
    }

    if ( pLastWriteTime != 0 )
    {
        aTimeBuffer.modtime=pLastWriteTime->Seconds;
    }
    else
    {
        aTimeBuffer.modtime=aFileStat.st_mtime;
    }

    /* mfe: Creation time not used here! */

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"File Times are (in localtime):\n");
    pTM=localtime(&aFileStat.st_ctime);
    fprintf(stderr,"CreationTime now '%s'\n",asctime(pTM));
    pTM=localtime(&aTimeBuffer.actime);
    fprintf(stderr,"AccessTime   now '%s'\n",asctime(pTM));
    pTM=localtime(&aTimeBuffer.modtime);
    fprintf(stderr,"Modification now '%s'\n",asctime(pTM));

    fprintf(stderr,"File Times are (in UTC):\n");
    fprintf(stderr,"CreationTime now '%s'\n",ctime(&aFileStat.st_ctime));
    fprintf(stderr,"AccessTime   now '%s'\n",ctime(&aTimeBuffer.actime));
    fprintf(stderr,"Modification now '%s'\n",ctime(&aTimeBuffer.modtime));
#endif

    nRet=utime(pszFilePath,&aTimeBuffer);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(OSL_FET_ERROR, nRet);
    }

    return osl_File_E_None;
}


/******************************************************************************
 *
 *                  Utility Functions
 *
 *****************************************************************************/


/*****************************************
 * oslMakeUStrFromPsz
 ****************************************/

rtl_uString* oslMakeUStrFromPsz(const sal_Char* pszStr, rtl_uString** ustrValid)
{
    rtl_string2UString(
        ustrValid,
        pszStr,
        rtl_str_getLength( pszStr ),
        osl_getThreadTextEncoding(),
        OUSTRING_TO_OSTRING_CVTFLAGS );
    OSL_ASSERT(*ustrValid != 0);

    return *ustrValid;
}

/*****************************************************************************
 * UnicodeToText
 * converting unicode to text manually saves us the penalty of a temporary
 * rtl_String object.
 ****************************************************************************/

int UnicodeToText( char * buffer, size_t bufLen, const sal_Unicode * uniText, sal_Int32 uniTextLen )
{
    rtl_UnicodeToTextConverter hConverter;
    sal_uInt32   nInfo;
    sal_Size   nSrcChars, nDestBytes;

    /* stolen from rtl/string.c */
    hConverter = rtl_createUnicodeToTextConverter( osl_getThreadTextEncoding() );

    nDestBytes = rtl_convertUnicodeToText( hConverter, 0, uniText, uniTextLen,
                                           buffer, bufLen,
                                           OUSTRING_TO_OSTRING_CVTFLAGS | RTL_UNICODETOTEXT_FLAGS_FLUSH,
                                           &nInfo, &nSrcChars );

    rtl_destroyUnicodeToTextConverter( hConverter );

    if( nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL )
    {
        errno = EOVERFLOW;
        return 0;
    }

    /* ensure trailing '\0' */
    buffer[nDestBytes] = '\0';

    return nDestBytes;
}

/*****************************************************************************
   TextToUnicode

   @param text
          The text to convert.

   @param text_buffer_size
          The number of characters.

   @param unic_text
          The unicode buffer.

   @param unic_text_buffer_size
             The size in characters of the unicode buffer.

 ****************************************************************************/

int TextToUnicode(
    const char*  text,
    size_t       text_buffer_size,
    sal_Unicode* unic_text,
    sal_Int32    unic_text_buffer_size)
{
    rtl_TextToUnicodeConverter hConverter;
    sal_uInt32 nInfo;
    sal_Size nSrcChars;
    sal_Size nDestBytes;

    /* stolen from rtl/string.c */
    hConverter = rtl_createTextToUnicodeConverter(osl_getThreadTextEncoding());

    nDestBytes = rtl_convertTextToUnicode(hConverter,
                                          0,
                                          text,  text_buffer_size,
                                          unic_text, unic_text_buffer_size,
                                          OSTRING_TO_OUSTRING_CVTFLAGS | RTL_TEXTTOUNICODE_FLAGS_FLUSH,
                                          &nInfo, &nSrcChars);

    rtl_destroyTextToUnicodeConverter(hConverter);

    if (nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL)
    {
        errno = EOVERFLOW;
        return 0;
    }

    /* ensure trailing '\0' */
    unic_text[nDestBytes] = '\0';

    return nDestBytes;
}

/******************************************************************************
 *
 *                  GENERIC FLOPPY FUNCTIONS
 *
 *****************************************************************************/

/*****************************************
 * osl_unmountVolumeDevice
 ****************************************/
oslFileError osl_unmountVolumeDevice( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
        return osl_File_E_None;
    else
        return osl_File_E_INVAL;
}

/*****************************************
 * osl_automountVolumeDevice
 ****************************************/
oslFileError osl_automountVolumeDevice( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
        return osl_File_E_None;
    else
        return osl_File_E_INVAL;
}

/*****************************************
 * osl_getVolumeDeviceMountPath
 ****************************************/
oslFileError osl_getVolumeDeviceMountPath( oslVolumeDeviceHandle Handle, rtl_uString **pstrPath )
{
    if ( Handle && pstrPath )
    {
        rtl_uString_assign( pstrPath, (rtl_uString *)Handle );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

/*****************************************
 * osl_acquireVolumeDeviceHandle
 ****************************************/

oslFileError SAL_CALL osl_acquireVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
    {
        rtl_uString_acquire( (rtl_uString *)Handle );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

/*****************************************
 * osl_releaseVolumeDeviceHandle
 ****************************************/

oslFileError osl_releaseVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    if ( Handle )
    {
        rtl_uString_release( (rtl_uString *)Handle );
        return osl_File_E_None;
    }
    else
        return osl_File_E_INVAL;
}

/******************************************************************************
 *
 *                  OS/2 FLOPPY FUNCTIONS
 *
 *****************************************************************************/
static oslVolumeDeviceHandle osl_isFloppyDrive(const sal_Char* pszPath)
{
    return NULL;
}

