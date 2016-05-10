/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "lwpobjstrm.hxx"
#include "lwptools.hxx"

#include <sal/types.h>
#include <tools/solar.h>
#include <memory>

/**
 * @descr  ctor() from LwpSvStream
 */
LwpObjectStream::LwpObjectStream(LwpSvStream *pStrm, bool isCompressed, sal_uInt16 size)
    :m_pContentBuf(nullptr), m_nBufSize(size), m_nReadPos(0),
    m_pStrm(pStrm), m_bCompressed(isCompressed)
{
    assert(size<IO_BUFFERSIZE);
    ReadStream();
}
/**
 * @descr  read object data from stream
 */
void LwpObjectStream::ReadStream()
{
    if(m_nBufSize == 0)
    {
        m_pContentBuf = nullptr;
    }
    else
    {
        Read2Buffer();
    }
}
/**
 * @descr  read object data from stream to buffer
 */
void LwpObjectStream::Read2Buffer()
{
    if( m_pContentBuf )
    {
        ReleaseBuffer();
    }

    m_nReadPos = 0;

    if( m_bCompressed )
    {
        std::unique_ptr<sal_uInt8[]> xCompressBuf(new sal_uInt8[m_nBufSize]);

        sal_uInt8* pCompressBuffer = xCompressBuf.get();
        memset(pCompressBuffer, 0, m_nBufSize);
        m_pStrm->Read(pCompressBuffer, m_nBufSize);

        sal_uInt8 pTempDst[IO_BUFFERSIZE];
        m_nBufSize = DecompressBuffer(pTempDst, pCompressBuffer, m_nBufSize);
        assert( m_nBufSize < IO_BUFFERSIZE);

        m_pContentBuf = AllocBuffer(m_nBufSize);
        memcpy(m_pContentBuf, pTempDst, m_nBufSize);
        //delete [] pTempDst;

    }
    else
    {
        m_pContentBuf = AllocBuffer(m_nBufSize);
        m_pStrm->Read(m_pContentBuf, m_nBufSize);
    }
}
/**
 * @descr  alloc size of buffer
 */
sal_uInt8* LwpObjectStream::AllocBuffer(sal_uInt16 size)
{
    if(size<=100)
    {
        return m_SmallBuffer;
    }
    else
    {
        return new sal_uInt8[size];
    }
}
/**
 * @descr  signal complete to release object buffer
 */
void LwpObjectStream::ReadComplete()
{
    ReleaseBuffer();
}

LwpObjectStream::~LwpObjectStream()
{
    ReleaseBuffer();
}
/**
 * @descr  release object buffer
 */
void LwpObjectStream::ReleaseBuffer()
{

    if(m_nBufSize>100)
    {
        if(m_pContentBuf)
        {
            delete [] m_pContentBuf;
            m_pContentBuf = nullptr;
        }
    }
}
/**
 * @descr  read len bytes from object stream to buffer
 */
sal_uInt16 LwpObjectStream::QuickRead(void* buf, sal_uInt16 len)
{
    memset(buf, 0, len);
    if( len > m_nBufSize - m_nReadPos )
    {
        SAL_WARN("lwp", "read request longer than buffer");
        len = m_nBufSize - m_nReadPos;
    }
    if( m_pContentBuf && len)
    {
        memcpy(buf, m_pContentBuf+m_nReadPos, len);
        m_nReadPos += len;
    }
    return len;
}
/**
 * @descr  SeekRel pos in object stream/buffer
 */
void LwpObjectStream::SeekRel(sal_uInt16 pos)
{
    if( pos > m_nBufSize - m_nReadPos)
        pos = m_nBufSize - m_nReadPos;
    m_nReadPos +=pos;
}
/**
 * @descr  Seek to pos in object buffer/buffer
 */
bool LwpObjectStream::Seek( sal_uInt16 pos)
{
    if (pos < m_nBufSize)
    {
        m_nReadPos = pos;
        return true;
    }
    return false;
}

/**
 * @descr  Quick read sal_Bool
 */
bool LwpObjectStream::QuickReadBool(bool *pFailure)
{
    SVBT16 aValue = {0};
    sal_uInt16 nRead = QuickRead(aValue, sizeof(aValue));
    if (pFailure)
        *pFailure = (nRead != sizeof(aValue));
    return static_cast<bool>(SVBT16ToShort(aValue));
}
/**
 * @descr  Quick read sal_uInt32
 */
sal_uInt32 LwpObjectStream::QuickReaduInt32(bool *pFailure)
{
    SVBT32 aValue = {0};
    sal_uInt16 nRead = QuickRead(aValue, sizeof(aValue));
    if (pFailure)
        *pFailure = (nRead != sizeof(aValue));
    return SVBT32ToUInt32(aValue);
}
/**
 * @descr  Quick read sal_uInt32
 */
sal_uInt16 LwpObjectStream::QuickReaduInt16(bool *pFailure)
{
    SVBT16 aValue = {0};
    sal_uInt16 nRead = QuickRead(aValue, sizeof(aValue));
    if (pFailure)
        *pFailure = (nRead != sizeof(aValue));
    return SVBT16ToShort(aValue);
}
/**
 * @descr  Quick read sal_Int32
 */
sal_Int32 LwpObjectStream::QuickReadInt32(bool *pFailure)
{
    SVBT32 aValue = {0};
    sal_uInt16 nRead = QuickRead(aValue, sizeof(aValue));
    if (pFailure)
        *pFailure = (nRead != sizeof(aValue));
    return static_cast<sal_Int32>(SVBT32ToUInt32(aValue));
}
/**
 * @descr  Quick read sal_Int16
 */
sal_Int16 LwpObjectStream::QuickReadInt16(bool *pFailure)
{
    SVBT16 aValue = {0};
    sal_uInt16 nRead = QuickRead(aValue, sizeof(aValue));
    if (pFailure)
        *pFailure = (nRead != sizeof(aValue));

    return static_cast<sal_Int16>(SVBT16ToShort(aValue));
}
/**
 * @descr  Quick read sal_uInt8
 */
sal_uInt8 LwpObjectStream::QuickReaduInt8(bool *pFailure)
{
    sal_uInt8 aValue = 0;
    sal_uInt16 nRead = QuickRead(&aValue, sizeof(aValue));
    if (pFailure)
        *pFailure = (nRead != sizeof(aValue));
    return aValue;
}
/**
 * @descr  Quick read double
 */
double LwpObjectStream::QuickReadDouble(bool *pFailure)
{
    union
    {
        double d;
        sal_uInt8 c[8];
    } s;
    memset(s.c, 0, sizeof(s.c));
    sal_uInt16 nRead = QuickRead(s.c, sizeof(s.c));
    if (pFailure)
        *pFailure = (nRead != sizeof(s.c));
#if defined(OSL_BIGENDIAN)
    for (size_t i = 0; i < 4; ++i)
        std::swap(s.c[i], s.c[7-i]);
#endif
    return s.d;
}
/**
 * @descr  skip extra bytes
 */
void LwpObjectStream::SkipExtra()
{
    sal_uInt16 extra = QuickReaduInt16();
    while (extra != 0)
        extra = QuickReaduInt16();
}
/**
 * @descr  check if extra bytes
 */
sal_uInt16 LwpObjectStream::CheckExtra()
{
    return QuickReaduInt16();
}
/**
 * @descr  decompress data buffer from pSrc to pDst
 *        Refer to the CAmiPro40File::DecompressObject(~) in LWP
 */
sal_uInt16 LwpObjectStream::DecompressBuffer(sal_uInt8* pDst, sal_uInt8* pSrc, sal_uInt16 Size)
{
    sal_uInt16 Cnt;
    sal_uInt32 DstSize = 0;

    while (Size)
    {
        switch (*pSrc & 0xC0)
        {
            case 0x00:
                // 1 - 64 bytes of 0
                // Code 00zzzzzz
                // where zzzzzz is the count - 1 of compressed 0 bytes

                Cnt = (*pSrc++ & 0x3F) + 1;
                if (DstSize+Cnt >= IO_BUFFERSIZE)
                    throw BadDecompress();
                memset(pDst, 0, Cnt);
                pDst += Cnt;
                DstSize += Cnt;
                Size--;
                break;

            case 0x40:
                // 1 - 8 zeros followed by 1 - 8 non-zero
                // Code 01zzznnn binary
                // where zzz is the count - 1 of compressed zero bytes
                // and nnn is the count - 1 of following non-zero bytes

                Cnt = ((*pSrc & 0x38) >> 3) + 1;
                if (DstSize+Cnt >= IO_BUFFERSIZE)
                    throw BadDecompress();
                memset(pDst, 0, Cnt);
                pDst += Cnt;
                DstSize += Cnt;
                Cnt = (*pSrc++ & 0x07) + 1;
                if (Size < Cnt + 1)
                    throw BadDecompress();
                Size -= Cnt + 1;
                if (DstSize+Cnt >= IO_BUFFERSIZE)
                    throw BadDecompress();
                memcpy(pDst, pSrc, Cnt);
                pDst += Cnt;
                DstSize += Cnt;
                pSrc += Cnt;
                break;

            case 0x80:
                // 1 0 followed by 1 - 64 bytes of non-zero
                // Code 0x80 (or 0x40 if 8 or less non-zero)
                // Code 10nnnnnn binary
                // where nnnnnn is the count - 1 of following non-zero bytes

                *pDst++ = 0;
                DstSize++;
                SAL_FALLTHROUGH;

            case 0xC0:
                // 1 - 64 bytes of non-zero
                // Code = 11nnnnnn binary
                // nnnnnn is the count less 1 of following non-zero bytes

                Cnt = (*pSrc++ & 0x3F) + 1;
                if (Size < Cnt + 1)
                    throw BadDecompress();
                Size -= Cnt + 1;
                if (DstSize+Cnt >= IO_BUFFERSIZE)
                    throw BadDecompress();
                memcpy(pDst, pSrc, Cnt);
                pDst += Cnt;
                DstSize += Cnt;
                pSrc += Cnt;
                break;
        }
        assert(DstSize < IO_BUFFERSIZE);
        if (DstSize >= IO_BUFFERSIZE)
            throw BadDecompress();

    }
    return static_cast<sal_uInt16>(DstSize);
}
/**
 * @descr  quick read string with 1252
 */
OUString LwpObjectStream::QuickReadStringPtr()
{
    sal_uInt16 diskSize;

    diskSize = QuickReaduInt16();
    QuickReaduInt16(); //len

    OUString str;
    rtl_TextEncoding rEncode =  RTL_TEXTENCODING_MS_1252;
    LwpTools::QuickReadUnicode(this, str, diskSize-sizeof(diskSize), rEncode);
    return str;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
