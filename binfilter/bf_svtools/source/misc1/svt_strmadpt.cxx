/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <functional> // needed under Solaris when including <algorithm>...

#include <algorithm>
#include <limits>
#include <set>

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef SVTOOLS_INSTRM_HXX
#include <instrm.hxx>
#endif
#ifndef SVTOOLS_OUTSTRM_HXX
#include <outstrm.hxx>
#endif
#ifndef SVTOOLS_STRMADPT_HXX
#include <strmadpt.hxx>
#endif

using namespace com::sun::star;

namespace binfilter
{

//============================================================================
class SvDataPipe_Impl
{
public:
    enum SeekResult { SEEK_BEFORE_MARKED, SEEK_OK, SEEK_PAST_END };

private:
    struct Page
    {
        Page * m_pPrev;
        Page * m_pNext;
        sal_Int8 * m_pStart;
        sal_Int8 * m_pRead;
        sal_Int8 * m_pEnd;
        sal_uInt32 m_nOffset;
        sal_Int8 m_aBuffer[1];
    };

    std::multiset< sal_uInt32 > m_aMarks;
    Page * m_pFirstPage;
    Page * m_pReadPage;
    Page * m_pWritePage;
    sal_Int8 * m_pReadBuffer;
    sal_uInt32 m_nReadBufferSize;
    sal_uInt32 m_nReadBufferFilled;
    sal_uInt32 m_nPageSize;
    sal_uInt32 m_nMinPages;
    sal_uInt32 m_nMaxPages;
    sal_uInt32 m_nPages;
    bool m_bEOF;

    bool remove(Page * pPage);

public:
    inline SvDataPipe_Impl(sal_uInt32 nThePageSize = 1000,
                           sal_uInt32 nTheMinPages = 100,
                           sal_uInt32 nTheMaxPages
                               = std::numeric_limits< sal_uInt32 >::max());

    ~SvDataPipe_Impl();

    inline void setReadBuffer(sal_Int8 * pBuffer, sal_uInt32 nSize);

    sal_uInt32 read();

    void clearReadBuffer() { m_pReadBuffer = 0; }

    sal_uInt32 write(sal_Int8 const * pBuffer, sal_uInt32 nSize);

    void setEOF() { m_bEOF = true; }

    inline bool isEOF() const;

    bool addMark(sal_uInt32 nPosition);

    bool removeMark(sal_uInt32 nPosition);

    inline sal_uInt32 getReadPosition() const;

    SeekResult setReadPosition(sal_uInt32 nPosition);
};

SvDataPipe_Impl::SvDataPipe_Impl(sal_uInt32 nThePageSize,
                                 sal_uInt32 nTheMinPages,
                                 sal_uInt32 nTheMaxPages):
    m_pFirstPage(0),
    m_pReadPage(0),
    m_pWritePage(0),
    m_pReadBuffer(0),
    m_nPageSize(std::min< sal_uInt32 >(
                    std::max< sal_uInt32 >(nThePageSize, sal_uInt32(1)),
                    sal_uInt32(std::numeric_limits< sal_uInt32 >::max()
                                   - sizeof (Page) + 1))),
    m_nMinPages(std::max< sal_uInt32 >(nTheMinPages, sal_uInt32(1))),
    m_nMaxPages(std::max< sal_uInt32 >(nTheMaxPages, sal_uInt32(1))),
    m_nPages(0),
    m_bEOF(false)
{}

inline void SvDataPipe_Impl::setReadBuffer(sal_Int8 * pBuffer,
                                           sal_uInt32 nSize)
{
    m_pReadBuffer = pBuffer;
    m_nReadBufferSize = nSize;
    m_nReadBufferFilled = 0;
}

inline bool SvDataPipe_Impl::isEOF() const
{
    return m_bEOF && m_pReadPage == m_pWritePage
           && (!m_pReadPage || m_pReadPage->m_pRead == m_pReadPage->m_pEnd);
}

inline sal_uInt32 SvDataPipe_Impl::getReadPosition() const
{
    return m_pReadPage == 0 ? 0 :
                              m_pReadPage->m_nOffset
                                  + (m_pReadPage->m_pRead
                                         - m_pReadPage->m_aBuffer);
}

//============================================================================
//
//  SvOutputStreamOpenLockBytes
//
//============================================================================

TYPEINIT1(SvOutputStreamOpenLockBytes, SvOpenLockBytes)

//============================================================================
// virtual
ErrCode SvOutputStreamOpenLockBytes::ReadAt(ULONG, void *, ULONG, ULONG *)
    const
{
    return ERRCODE_IO_CANTREAD;
}

//============================================================================
// virtual
ErrCode SvOutputStreamOpenLockBytes::WriteAt(ULONG nPos, void const * pBuffer,
                                             ULONG nCount, ULONG * pWritten)
{
    if (nPos != m_nPosition)
        return ERRCODE_IO_CANTWRITE;
    return FillAppend(pBuffer, nCount, pWritten);
}

//============================================================================
// virtual
ErrCode SvOutputStreamOpenLockBytes::Flush() const
{
    if (!m_xOutputStream.is())
        return ERRCODE_IO_CANTWRITE;
    try
    {
        m_xOutputStream->flush();
    }
    catch (io::IOException)
    {
        return ERRCODE_IO_CANTWRITE;
    }
    return ERRCODE_NONE;
}

//============================================================================
// virtual
ErrCode SvOutputStreamOpenLockBytes::SetSize(ULONG)
{
    return ERRCODE_IO_NOTSUPPORTED;
}

//============================================================================
// virtual
ErrCode SvOutputStreamOpenLockBytes::Stat(SvLockBytesStat * pStat,
                                          SvLockBytesStatFlag) const
{
    if (pStat)
        pStat->nSize = m_nPosition;
    return ERRCODE_NONE;
}

//============================================================================
// virtual
ErrCode SvOutputStreamOpenLockBytes::FillAppend(void const * pBuffer,
                                                ULONG nCount,
                                                ULONG * pWritten)
{
    if (!m_xOutputStream.is())
        return ERRCODE_IO_CANTWRITE;
    if (nCount > 0
        && nCount > std::numeric_limits< ULONG >::max() - m_nPosition)
    {
        nCount = std::numeric_limits< ULONG >::max() - m_nPosition;
        if (nCount == 0)
            return ERRCODE_IO_CANTWRITE;
    }
    try
    {
        m_xOutputStream->
            writeBytes(uno::Sequence< sal_Int8 >(
                           static_cast< sal_Int8 const * >(pBuffer), nCount));
    }
    catch (io::IOException)
    {
        return ERRCODE_IO_CANTWRITE;
    }
    m_nPosition += nCount;
    if (pWritten)
        *pWritten = nCount;
    return ERRCODE_NONE;
}

//============================================================================
// virtual
ULONG SvOutputStreamOpenLockBytes::Tell() const
{
    return m_nPosition;
}

//============================================================================
// virtual
ULONG SvOutputStreamOpenLockBytes::Seek(ULONG)
{
    return m_nPosition;
}

//============================================================================
// virtual
void SvOutputStreamOpenLockBytes::Terminate()
{
    if (m_xOutputStream.is())
        try
        {
            m_xOutputStream->closeOutput();
        }
        catch (io::IOException) {}
}

//============================================================================
//
//  SvLockBytesInputStream
//
//============================================================================

// virtual
uno::Any SAL_CALL SvLockBytesInputStream::queryInterface(uno::Type const &
                                                             rType)
    throw (uno::RuntimeException)
{
    uno::Any
        aReturn(cppu::queryInterface(rType,
                                     static_cast< io::XInputStream * >(this),
                                     static_cast< io::XSeekable * >(this)));
    return aReturn.hasValue() ? aReturn : OWeakObject::queryInterface(rType);
}

//============================================================================
// virtual
void SAL_CALL SvLockBytesInputStream::acquire()	throw ()
{
    OWeakObject::acquire();
}

//============================================================================
// virtual
void SAL_CALL SvLockBytesInputStream::release() throw ()
{
    OWeakObject::release();
}

//============================================================================
// virtual
sal_Int32 SAL_CALL
SvLockBytesInputStream::readBytes(uno::Sequence< sal_Int8 > & rData,
                                  sal_Int32 nBytesToRead)
    throw (io::IOException, uno::RuntimeException)
{
    if (!m_xLockBytes.Is())
        throw io::NotConnectedException();
    if (nBytesToRead < 0
        || m_nPosition > std::numeric_limits< ULONG >::max()
           && nBytesToRead > 0)
        throw io::IOException();
    rData.realloc(nBytesToRead);
    sal_Int32 nSize = 0;
    while (nSize < nBytesToRead)
    {
        ULONG nCount;
        ErrCode nError = m_xLockBytes->ReadAt(ULONG(m_nPosition),
                                              rData.getArray() + nSize,
                                              nBytesToRead - nSize, &nCount);
        if (nError != ERRCODE_NONE && nError != ERRCODE_IO_PENDING)
            throw io::IOException();
        m_nPosition += nCount;
        nSize += nCount;
        if (nError == ERRCODE_NONE && nCount == 0)
            break;
    }
    rData.realloc(nSize);
    return nSize;
}

//============================================================================
// virtual
sal_Int32 SAL_CALL
SvLockBytesInputStream::readSomeBytes(uno::Sequence< sal_Int8 > & rData,
                                      sal_Int32 nMaxBytesToRead)
    throw (io::IOException, uno::RuntimeException)
{
    if (!m_xLockBytes.Is())
        throw io::NotConnectedException();
    if (m_nPosition > std::numeric_limits< ULONG >::max()
        && nMaxBytesToRead > 0)
        throw io::IOException();
    rData.realloc(nMaxBytesToRead);
    ULONG nCount = 0;
    if (nMaxBytesToRead > 0)
    {
        ErrCode nError;
        do
        {
            nError = m_xLockBytes->ReadAt(ULONG(m_nPosition),
                                          rData.getArray(),
                                          nMaxBytesToRead < 0 ?
                                              0 : nMaxBytesToRead,
                                          &nCount);
            if (nError != ERRCODE_NONE && nError != ERRCODE_IO_PENDING)
                throw io::IOException();
            m_nPosition += nCount;
        }
        while (nCount == 0 && nError == ERRCODE_IO_PENDING);
    }
    rData.realloc(sal_Int32(nCount));
    return sal_Int32(nCount);
}

//============================================================================
// virtual
void SAL_CALL SvLockBytesInputStream::skipBytes(sal_Int32 nBytesToSkip)
    throw (io::IOException, uno::RuntimeException)
{
    if (!m_xLockBytes.Is())
        throw io::NotConnectedException();
    if (nBytesToSkip < 0)
        throw io::IOException();
    if (m_nPosition + nBytesToSkip < 0)
        throw io::BufferSizeExceededException();
    m_nPosition += nBytesToSkip;
}

//============================================================================
// virtual
sal_Int32 SAL_CALL SvLockBytesInputStream::available()
    throw (io::IOException, uno::RuntimeException)
{
    if (!m_xLockBytes.Is())
        throw io::NotConnectedException();
    SvLockBytesStat aStat;
    if (m_xLockBytes->Stat(&aStat, SVSTATFLAG_DEFAULT) != ERRCODE_NONE)
        throw io::IOException();
    return aStat.nSize <= m_nPosition ?
               0 :
               std::min< sal_uInt32 >(
                   sal_uInt32(aStat.nSize - m_nPosition),
                   sal_uInt32(std::numeric_limits< sal_Int32 >::max()));
}

//============================================================================
// virtual
void SAL_CALL SvLockBytesInputStream::closeInput()
    throw (io::IOException, uno::RuntimeException)
{
    if (!m_xLockBytes.Is())
        throw io::NotConnectedException();
    m_xLockBytes = 0;
}

//============================================================================
// virtual
void SAL_CALL SvLockBytesInputStream::seek(sal_Int64 nLocation)
    throw (lang::IllegalArgumentException, io::IOException,
           uno::RuntimeException)
{
    if (nLocation < 0)
        throw lang::IllegalArgumentException();
    if (!m_xLockBytes.Is())
        throw io::NotConnectedException();
    m_nPosition = nLocation;
}

//============================================================================
// virtual
sal_Int64 SAL_CALL SvLockBytesInputStream::getPosition()
    throw (io::IOException, uno::RuntimeException)
{
    if (!m_xLockBytes.Is())
        throw io::NotConnectedException();
    return m_nPosition;
}

//============================================================================
// virtual
sal_Int64 SAL_CALL SvLockBytesInputStream::getLength()
    throw (io::IOException, uno::RuntimeException)
{
    if (!m_xLockBytes.Is())
        throw io::NotConnectedException();
    SvLockBytesStat aStat;
    if (m_xLockBytes->Stat(&aStat, SVSTATFLAG_DEFAULT) != ERRCODE_NONE)
        throw io::IOException();
    return aStat.nSize;
}

//============================================================================
//
//  SvInputStream
//
//============================================================================

bool SvInputStream::open()
{
    if (GetError() != ERRCODE_NONE)
        return false;
    if (!(m_xSeekable.is() || m_pPipe))
    {
        if (!m_xStream.is())
        {
            SetError(ERRCODE_IO_INVALIDDEVICE);
            return false;
        }
        m_xSeekable
            = uno::Reference< io::XSeekable >(m_xStream, uno::UNO_QUERY);
        if (!m_xSeekable.is())
            m_pPipe = new SvDataPipe_Impl;
    }
    return true;
}

//============================================================================
// virtual
ULONG SvInputStream::GetData(void * pData, ULONG nSize)
{
    if (!open())
    {
        SetError(ERRCODE_IO_CANTREAD);
        return 0;
    }
    sal_uInt32 nRead = 0;
    if (m_xSeekable.is())
    {
        if (m_nSeekedFrom != STREAM_SEEK_TO_END)
        {
            try
            {
                m_xSeekable->seek(m_nSeekedFrom);
            }
            catch (io::IOException)
            {
                SetError(ERRCODE_IO_CANTREAD);
                return 0;
            }
            m_nSeekedFrom = STREAM_SEEK_TO_END;
        }
        for (;;)
        {
            sal_Int32 nRemain
                = sal_Int32(
                    std::min(ULONG(nSize - nRead),
                             ULONG(std::numeric_limits< sal_Int32 >::max())));
            if (nRemain == 0)
                break;
            uno::Sequence< sal_Int8 > aBuffer;
            sal_Int32 nCount;
            try
            {
                nCount = m_xStream->readBytes(aBuffer, nRemain);
            }
            catch (io::IOException)
            {
                SetError(ERRCODE_IO_CANTREAD);
                return nRead;
            }
            rtl_copyMemory(static_cast< sal_Int8 * >(pData) + nRead,
                           aBuffer.getConstArray(), sal_uInt32(nCount));
            nRead += nCount;
            if (nCount < nRemain)
                break;
        }
    }
    else
    {
        if (m_nSeekedFrom != STREAM_SEEK_TO_END)
        {
            SetError(ERRCODE_IO_CANTREAD);
            return 0;
        }
        m_pPipe->setReadBuffer(static_cast< sal_Int8 * >(pData), nSize);
        nRead = m_pPipe->read();
        if (nRead < nSize && !m_pPipe->isEOF())
            for (;;)
            {
                sal_Int32 nRemain
                    = sal_Int32(
                        std::min(
                            ULONG(nSize - nRead),
                            ULONG(std::numeric_limits< sal_Int32 >::max())));
                if (nRemain == 0)
                    break;
                uno::Sequence< sal_Int8 > aBuffer;
                sal_Int32 nCount;
                try
                {
                    nCount = m_xStream->readBytes(aBuffer, nRemain);
                }
                catch (io::IOException)
                {
                    SetError(ERRCODE_IO_CANTREAD);
                    break;
                }
                m_pPipe->write(aBuffer.getConstArray(), sal_uInt32(nCount));
                nRead += m_pPipe->read();
                if (nCount < nRemain)
                {
                    m_xStream->closeInput();
                    m_pPipe->setEOF();
                    break;
                }
            }
        m_pPipe->clearReadBuffer();
    }
    return nRead;
}

//============================================================================
// virtual
ULONG SvInputStream::PutData(void const *, ULONG)
{
    SetError(ERRCODE_IO_NOTSUPPORTED);
    return 0;
}

//============================================================================
// virtual
void SvInputStream::FlushData()
{}

//============================================================================
// virtual
ULONG SvInputStream::SeekPos(ULONG nPos)
{
    if (open())
        if (nPos == STREAM_SEEK_TO_END)
            if (m_nSeekedFrom == STREAM_SEEK_TO_END)
            {
                if (m_xSeekable.is())
                    try
                    {
                        sal_Int64 nLength = m_xSeekable->getLength();
                        if (nLength < STREAM_SEEK_TO_END)
                        {
                            m_nSeekedFrom = Tell();
                            return ULONG(nLength);
                        }
                    }
                    catch (io::IOException) {}
                else
                    return Tell(); //@@@
            }
            else
                return Tell();
        else if (nPos == m_nSeekedFrom)
        {
            m_nSeekedFrom = STREAM_SEEK_TO_END;
            return nPos;
        }
        else if (m_xSeekable.is())
            try
            {
                m_xSeekable->seek(nPos);
                m_nSeekedFrom = STREAM_SEEK_TO_END;
                return nPos;
            }
            catch (io::IOException) {}
        else if (m_pPipe->setReadPosition(nPos) == SvDataPipe_Impl::SEEK_OK)
        {
            m_nSeekedFrom = STREAM_SEEK_TO_END;
            return nPos;
        }
    SetError(ERRCODE_IO_CANTSEEK);
    return Tell();
}

//============================================================================
// virtual
void SvInputStream::SetSize(ULONG)
{
    SetError(ERRCODE_IO_NOTSUPPORTED);
}

//============================================================================
SvInputStream::SvInputStream(
        com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
                const &
            rTheStream):
    m_xStream(rTheStream),
    m_pPipe(0),
    m_nSeekedFrom(STREAM_SEEK_TO_END)
{
    SetBufferSize(0);
}

//============================================================================
// virtual
SvInputStream::~SvInputStream()
{
    if (m_xStream.is())
        try
        {
            m_xStream->closeInput();
        }
        catch (io::IOException) {}
    delete m_pPipe;
}

//============================================================================
// virtual
USHORT SvInputStream::IsA() const
{
    return 0;
}

//============================================================================
// virtual
void SvInputStream::AddMark(ULONG nPos)
{
    if (open() && m_pPipe)
        m_pPipe->addMark(nPos);
}

//============================================================================
// virtual
void SvInputStream::RemoveMark(ULONG nPos)
{
    if (open() && m_pPipe)
        m_pPipe->removeMark(nPos);
}

//============================================================================
//
//  SvDataPipe_Impl
//
//============================================================================

bool SvDataPipe_Impl::remove(Page * pPage)
{
    if (pPage != m_pFirstPage || m_pReadPage == m_pFirstPage
        || !m_aMarks.empty()
           && *m_aMarks.begin() < m_pFirstPage->m_nOffset + m_nPageSize)
        return false;

    m_pFirstPage = m_pFirstPage->m_pNext;

    if (m_nPages <= m_nMinPages)
        return true;

    pPage->m_pPrev->m_pNext = pPage->m_pNext;
    pPage->m_pNext->m_pPrev = pPage->m_pPrev;
    rtl_freeMemory(pPage);
    --m_nPages;

    return true;
}

//============================================================================
SvDataPipe_Impl::~SvDataPipe_Impl()
{
    if (m_pFirstPage != 0)
        for (Page * pPage = m_pFirstPage;;)
        {
            Page * pNext = pPage->m_pNext;
            rtl_freeMemory(pPage);
            if (pNext == m_pFirstPage)
                break;
            pPage = pNext;
        }
}

//============================================================================
sal_uInt32 SvDataPipe_Impl::read()
{
    if (m_pReadBuffer == 0 || m_nReadBufferSize == 0 || m_pReadPage == 0)
        return 0;

    sal_uInt32 nSize = m_nReadBufferSize;
    sal_uInt32 nRemain = m_nReadBufferSize - m_nReadBufferFilled;

    m_pReadBuffer += m_nReadBufferFilled;
    m_nReadBufferSize -= m_nReadBufferFilled;
    m_nReadBufferFilled = 0;

    while (nRemain > 0)
    {
        sal_uInt32 nBlock = std::min(sal_uInt32(m_pReadPage->m_pEnd
                                                    - m_pReadPage->m_pRead),
                                     nRemain);
        rtl_copyMemory(m_pReadBuffer, m_pReadPage->m_pRead, nBlock);
        m_pReadPage->m_pRead += nBlock;
        m_pReadBuffer += nBlock;
        m_nReadBufferSize -= nBlock;
        m_nReadBufferFilled = 0;
        nRemain -= nBlock;

        if (m_pReadPage == m_pWritePage)
            break;

        if (m_pReadPage->m_pRead == m_pReadPage->m_pEnd)
        {
            Page * pRemove = m_pReadPage;
            m_pReadPage = pRemove->m_pNext;
            remove(pRemove);
        }
    }

    return nSize - nRemain;
}

//============================================================================
sal_uInt32 SvDataPipe_Impl::write(sal_Int8 const * pBuffer, sal_uInt32 nSize)
{
    if (nSize == 0)
        return 0;

    if (m_pWritePage == 0)
    {
        m_pFirstPage
            = static_cast< Page * >(rtl_allocateMemory(sizeof (Page)
                                                           + m_nPageSize
                                                           - 1));
        m_pFirstPage->m_pPrev = m_pFirstPage;
        m_pFirstPage->m_pNext = m_pFirstPage;
        m_pFirstPage->m_pStart = m_pFirstPage->m_aBuffer;
        m_pFirstPage->m_pRead = m_pFirstPage->m_aBuffer;
        m_pFirstPage->m_pEnd = m_pFirstPage->m_aBuffer;
        m_pFirstPage->m_nOffset = 0;
        m_pReadPage = m_pFirstPage;
        m_pWritePage = m_pFirstPage;
        ++m_nPages;
    }

    sal_uInt32 nRemain = nSize;

    if (m_pReadBuffer != 0 && m_pReadPage == m_pWritePage
        && m_pReadPage->m_pRead == m_pWritePage->m_pEnd)
    {
        sal_uInt32 nBlock = std::min(nRemain,
                                     sal_uInt32(m_nReadBufferSize
                                                    - m_nReadBufferFilled));
        sal_uInt32 nPosition = m_pWritePage->m_nOffset
                                   + (m_pWritePage->m_pEnd
                                          - m_pWritePage->m_aBuffer);
        if (!m_aMarks.empty())
            nBlock = *m_aMarks.begin() > nPosition ?
                         std::min(nBlock, sal_uInt32(*m_aMarks.begin()
                                                         - nPosition)) :
                         0;

        if (nBlock > 0)
        {
            rtl_copyMemory(m_pReadBuffer + m_nReadBufferFilled, pBuffer,
                           nBlock);
            m_nReadBufferFilled += nBlock;
            nRemain -= nBlock;

            nPosition += nBlock;
            m_pWritePage->m_nOffset = (nPosition / m_nPageSize) * m_nPageSize;
            m_pWritePage->m_pStart = m_pWritePage->m_aBuffer
                                         + nPosition % m_nPageSize;
            m_pWritePage->m_pRead = m_pWritePage->m_pStart;
            m_pWritePage->m_pEnd = m_pWritePage->m_pStart;
        }
    }

    if (nRemain > 0)
        for (;;)
        {
            sal_uInt32 nBlock
                = std::min(sal_uInt32(m_pWritePage->m_aBuffer + m_nPageSize
                                          - m_pWritePage->m_pEnd),
                           nRemain);
            rtl_copyMemory(m_pWritePage->m_pEnd, pBuffer, nBlock);
            m_pWritePage->m_pEnd += nBlock;
            pBuffer += nBlock;
            nRemain -= nBlock;

            if (nRemain == 0)
                break;

            if (m_pWritePage->m_pNext == m_pFirstPage)
            {
                if (m_nPages == m_nMaxPages)
                    break;

                Page * pNew
                    = static_cast< Page * >(rtl_allocateMemory(
                                                sizeof (Page) + m_nPageSize
                                                    - 1));
                pNew->m_pPrev = m_pWritePage;
                pNew->m_pNext = m_pWritePage->m_pNext;

                m_pWritePage->m_pNext->m_pPrev = pNew;
                m_pWritePage->m_pNext = pNew;
                ++m_nPages;
            }

            m_pWritePage->m_pNext->m_nOffset = m_pWritePage->m_nOffset
                                                   + m_nPageSize;
            m_pWritePage = m_pWritePage->m_pNext;
            m_pWritePage->m_pStart = m_pWritePage->m_aBuffer;
            m_pWritePage->m_pRead = m_pWritePage->m_aBuffer;
            m_pWritePage->m_pEnd = m_pWritePage->m_aBuffer;
        }

    return nSize - nRemain;
}

//============================================================================
bool SvDataPipe_Impl::addMark(sal_uInt32 nPosition)
{
    if (m_pFirstPage != 0 && m_pFirstPage->m_nOffset > nPosition)
        return false;
    m_aMarks.insert(nPosition);
    return true;
}

//============================================================================
bool SvDataPipe_Impl::removeMark(sal_uInt32 nPosition)
{
    std::multiset< sal_uInt32 >::iterator t = m_aMarks.find(nPosition);
    if (t == m_aMarks.end())
        return false;
    m_aMarks.erase(t);
    while (remove(m_pFirstPage));
    return true;
}

//============================================================================
SvDataPipe_Impl::SeekResult SvDataPipe_Impl::setReadPosition(sal_uInt32
                                                                 nPosition)
{
    if (m_pFirstPage == 0)
        return nPosition == 0 ? SEEK_OK : SEEK_PAST_END;

    if (nPosition
            <= m_pReadPage->m_nOffset
                   + (m_pReadPage->m_pRead - m_pReadPage->m_aBuffer))
    {
        if (nPosition
                < m_pFirstPage->m_nOffset
                      + (m_pFirstPage->m_pStart - m_pFirstPage->m_aBuffer))
            return SEEK_BEFORE_MARKED;

        while (nPosition < m_pReadPage->m_nOffset)
        {
            m_pReadPage->m_pRead = m_pReadPage->m_pStart;
            m_pReadPage = m_pReadPage->m_pPrev;
        }
    }
    else
    {
        if (nPosition
                > m_pWritePage->m_nOffset
                      + (m_pWritePage->m_pEnd - m_pWritePage->m_aBuffer))
            return SEEK_PAST_END;

        while (m_pReadPage != m_pWritePage
               && nPosition >= m_pReadPage->m_nOffset + m_nPageSize)
        {
            Page * pRemove = m_pReadPage;
            m_pReadPage = pRemove->m_pNext;
            remove(pRemove);
        }
    }

    m_pReadPage->m_pRead = m_pReadPage->m_aBuffer
                               + (nPosition - m_pReadPage->m_nOffset);
    return SEEK_OK;
}

}
