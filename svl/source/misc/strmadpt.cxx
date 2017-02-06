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


#include <functional>

#include <algorithm>
#include <limits>
#include <set>
#include <string.h>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <osl/diagnose.h>
#include <rtl/alloc.h>
#include <cppuhelper/queryinterface.hxx>
#include <svl/instrm.hxx>
#include <svl/outstrm.hxx>

using namespace com::sun::star;

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
    sal_uInt32 m_nPages;
    bool m_bEOF;

    void remove(Page * pPage);

public:
    inline SvDataPipe_Impl();

    ~SvDataPipe_Impl();

    inline void setReadBuffer(sal_Int8 * pBuffer, sal_uInt32 nSize);

    sal_uInt32 read();

    void clearReadBuffer() { m_pReadBuffer = nullptr; }

    void write(sal_Int8 const * pBuffer, sal_uInt32 nSize);

    void setEOF() { m_bEOF = true; }

    inline bool isEOF() const;

    SeekResult setReadPosition(sal_uInt32 nPosition);
};

SvDataPipe_Impl::SvDataPipe_Impl()
    : m_pFirstPage( nullptr )
    , m_pReadPage( nullptr )
    , m_pWritePage( nullptr )
    , m_pReadBuffer( nullptr )
    , m_nReadBufferSize( 0 )
    , m_nReadBufferFilled( 0 )
    , m_nPageSize(std::min< sal_uInt32 >(
                          1000,
                          sal_uInt32(std::numeric_limits< sal_uInt32 >::max()
                                     - sizeof (Page) + 1)))
    , m_nPages( 0 )
    , m_bEOF( false )
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



//  SvInputStream

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
        m_xSeekable.set(m_xStream, uno::UNO_QUERY);
        if (!m_xSeekable.is())
            m_pPipe = new SvDataPipe_Impl;
    }
    return true;
}

// virtual
std::size_t SvInputStream::GetData(void * pData, std::size_t const nSize)
{
    if (!open())
    {
        SetError(ERRCODE_IO_CANTREAD);
        return 0;
    }
    // check if a truncated STREAM_SEEK_TO_END was passed
    assert(m_nSeekedFrom != SAL_MAX_UINT32);
    sal_uInt32 nRead = 0;
    if (m_xSeekable.is())
    {
        if (m_nSeekedFrom != STREAM_SEEK_TO_END)
        {
            try
            {
                m_xSeekable->seek(m_nSeekedFrom);
            }
            catch (const io::IOException&)
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
                    std::min(std::size_t(nSize - nRead),
                             std::size_t(std::numeric_limits<sal_Int32>::max())));
            if (nRemain == 0)
                break;
            uno::Sequence< sal_Int8 > aBuffer;
            sal_Int32 nCount;
            try
            {
                nCount = m_xStream->readBytes(aBuffer, nRemain);
            }
            catch (const io::IOException&)
            {
                SetError(ERRCODE_IO_CANTREAD);
                return nRead;
            }
            memcpy(static_cast< sal_Int8 * >(pData) + nRead,
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
                            std::size_t(nSize - nRead),
                            std::size_t(std::numeric_limits<sal_Int32>::max())));
                if (nRemain == 0)
                    break;
                uno::Sequence< sal_Int8 > aBuffer;
                sal_Int32 nCount;
                try
                {
                    nCount = m_xStream->readBytes(aBuffer, nRemain);
                }
                catch (const io::IOException&)
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

// virtual
std::size_t SvInputStream::PutData(void const *, std::size_t)
{
    SetError(ERRCODE_IO_NOTSUPPORTED);
    return 0;
}

// virtual
void SvInputStream::FlushData()
{}

// virtual
sal_uInt64 SvInputStream::SeekPos(sal_uInt64 const nPos)
{
    // check if a truncated STREAM_SEEK_TO_END was passed
    assert(nPos != SAL_MAX_UINT32);
    if (open())
    {
        if (nPos == STREAM_SEEK_TO_END)
        {
            if (m_nSeekedFrom == STREAM_SEEK_TO_END)
            {
                if (m_xSeekable.is())
                    try
                    {
                        sal_Int64 nLength = m_xSeekable->getLength();
                        OSL_ASSERT(nLength >= 0);
                        if (static_cast<sal_uInt64>(nLength)
                            < STREAM_SEEK_TO_END)
                        {
                            m_nSeekedFrom = Tell();
                            return sal_uInt64(nLength);
                        }
                    }
                    catch (const io::IOException&)
                    {
                    }
                else
                    return Tell(); //@@@
            }
            else
                return Tell();
        }
        else if (nPos == m_nSeekedFrom)
        {
            m_nSeekedFrom = STREAM_SEEK_TO_END;
            return nPos;
        }
        else if (m_xSeekable.is())
        {
            try
            {
                m_xSeekable->seek(nPos);
                m_nSeekedFrom = STREAM_SEEK_TO_END;
                return nPos;
            }
            catch (const io::IOException&)
            {
            }
        }
        else if (m_pPipe->setReadPosition(nPos) == SvDataPipe_Impl::SEEK_OK)
        {
            m_nSeekedFrom = STREAM_SEEK_TO_END;
            return nPos;
        }
        else if ( nPos > Tell() )
        {
            // Read out the bytes
            sal_Int32 nRead = nPos - Tell();
            uno::Sequence< sal_Int8 > aBuffer;
            m_xStream->readBytes( aBuffer, nRead );
            return nPos;
        }
        else if ( nPos == Tell() )
            return nPos;
    }
    SetError(ERRCODE_IO_CANTSEEK);
    return Tell();
}

// virtual
void SvInputStream::SetSize(sal_uInt64)
{
    SetError(ERRCODE_IO_NOTSUPPORTED);
}

SvInputStream::SvInputStream( css::uno::Reference< css::io::XInputStream > const & rTheStream):
    m_xStream(rTheStream),
    m_pPipe(nullptr),
    m_nSeekedFrom(STREAM_SEEK_TO_END)
{
    SetBufferSize(0);
}

// virtual
SvInputStream::~SvInputStream()
{
    if (m_xStream.is())
    {
        try
        {
            m_xStream->closeInput();
        }
        catch (const io::IOException&)
        {
        }
    }
    delete m_pPipe;
}

//  SvOutputStream

// virtual
std::size_t SvOutputStream::GetData(void *, std::size_t)
{
    SetError(ERRCODE_IO_NOTSUPPORTED);
    return 0;
}

// virtual
std::size_t SvOutputStream::PutData(void const * pData, std::size_t nSize)
{
    if (!m_xStream.is())
    {
        SetError(ERRCODE_IO_CANTWRITE);
        return 0;
    }
    std::size_t nWritten = 0;
    for (;;)
    {
        sal_Int32 nRemain
            = sal_Int32(
                std::min(std::size_t(nSize - nWritten),
                         std::size_t(std::numeric_limits<sal_Int32>::max())));
        if (nRemain == 0)
            break;
        try
        {
            m_xStream->writeBytes(uno::Sequence< sal_Int8 >(
                                      static_cast<const sal_Int8 * >(pData)
                                          + nWritten,
                                      nRemain));
        }
        catch (const io::IOException&)
        {
            SetError(ERRCODE_IO_CANTWRITE);
            break;
        }
        nWritten += nRemain;
    }
    return nWritten;
}

// virtual
sal_uInt64 SvOutputStream::SeekPos(sal_uInt64)
{
    SetError(ERRCODE_IO_NOTSUPPORTED);
    return 0;
}

// virtual
void SvOutputStream::FlushData()
{
    if (!m_xStream.is())
    {
        SetError(ERRCODE_IO_INVALIDDEVICE);
        return;
    }
    try
    {
        m_xStream->flush();
    }
    catch (const io::IOException&)
    {
    }
}

// virtual
void SvOutputStream::SetSize(sal_uInt64)
{
    SetError(ERRCODE_IO_NOTSUPPORTED);
}

SvOutputStream::SvOutputStream(uno::Reference< io::XOutputStream > const &
                                   rTheStream):
    m_xStream(rTheStream)
{
    SetBufferSize(0);
}

// virtual
SvOutputStream::~SvOutputStream()
{
    if (m_xStream.is())
    {
        try
        {
            m_xStream->closeOutput();
        }
        catch (const io::IOException&)
        {
        }
    }
}


//  SvDataPipe_Impl


void SvDataPipe_Impl::remove(Page * pPage)
{
    if (
        pPage != m_pFirstPage ||
        m_pReadPage == m_pFirstPage ||
        (
         !m_aMarks.empty() &&
         *m_aMarks.begin() < m_pFirstPage->m_nOffset + m_nPageSize
        )
       )
    {
        return;
    }

    m_pFirstPage = m_pFirstPage->m_pNext;

    if (m_nPages <= 100) // min pages
        return;

    pPage->m_pPrev->m_pNext = pPage->m_pNext;
    pPage->m_pNext->m_pPrev = pPage->m_pPrev;
    rtl_freeMemory(pPage);
    --m_nPages;
}

SvDataPipe_Impl::~SvDataPipe_Impl()
{
    if (m_pFirstPage != nullptr)
        for (Page * pPage = m_pFirstPage;;)
        {
            Page * pNext = pPage->m_pNext;
            rtl_freeMemory(pPage);
            if (pNext == m_pFirstPage)
                break;
            pPage = pNext;
        }
}

sal_uInt32 SvDataPipe_Impl::read()
{
    if (m_pReadBuffer == nullptr || m_nReadBufferSize == 0 || m_pReadPage == nullptr)
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
        memcpy(m_pReadBuffer, m_pReadPage->m_pRead, nBlock);
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

void SvDataPipe_Impl::write(sal_Int8 const * pBuffer, sal_uInt32 nSize)
{
    if (nSize == 0)
        return;

    if (m_pWritePage == nullptr)
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

    if (m_pReadBuffer != nullptr && m_pReadPage == m_pWritePage
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
            memcpy(m_pReadBuffer + m_nReadBufferFilled, pBuffer,
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
            memcpy(m_pWritePage->m_pEnd, pBuffer, nBlock);
            m_pWritePage->m_pEnd += nBlock;
            pBuffer += nBlock;
            nRemain -= nBlock;

            if (nRemain == 0)
                break;

            if (m_pWritePage->m_pNext == m_pFirstPage)
            {
                if (m_nPages == std::numeric_limits< sal_uInt32 >::max())
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
}

SvDataPipe_Impl::SeekResult SvDataPipe_Impl::setReadPosition(sal_uInt32
                                                                 nPosition)
{
    if (m_pFirstPage == nullptr)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
