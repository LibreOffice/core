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


#include "lzwdecom.hxx"
#include <sal/log.hxx>
#include <tools/stream.hxx>

constexpr sal_Int16 CODE_CLEAR = 256;
constexpr sal_Int16 CODE_EOI = 257;
constexpr sal_Int16 CODE_FIRSTMULTICHAR = 258;

LZWDecompressor::LZWDecompressor()
    : m_pIStream(nullptr)
    , m_aTable{{}}
    , m_nTableCurrentId(0)
    , m_bEOIFound(false)
    , m_bInvert(false)
    , m_nOldCode(0)
    , m_pOutBufData(nullptr)
    , m_nOutBufDataLen(0)
    , m_nInputBitsBuf(0)
    , m_nInputBitsBufSize(0)
{
}


void LZWDecompressor::StartDecompression(SvStream & rIStream)
{
    for (sal_uInt16 i=0; i<MAX_TABLE_SIZE; i++)
    {
        m_aTable[i].nPrevCode = 0;
        m_aTable[i].nDataCount = 1;
        m_aTable[i].nData = static_cast<sal_uInt8>(i);
    }
    m_pIStream = &rIStream;

    m_nTableCurrentId = CODE_FIRSTMULTICHAR;

    m_bEOIFound = false;

    m_nOldCode = 0;
    m_pOutBufData = nullptr;

    m_nOutBufDataLen = 0;

    m_pIStream->ReadUChar( m_nInputBitsBuf );

    m_nInputBitsBufSize=8;

    m_bInvert = m_nInputBitsBuf == 1;

    if ( m_bInvert )
    {
        m_nInputBitsBuf = ( ( m_nInputBitsBuf & 1 ) << 7 ) | ( ( m_nInputBitsBuf & 2 ) << 5 ) | ( ( m_nInputBitsBuf & 4 ) << 3 ) | ( ( m_nInputBitsBuf & 8 ) << 1 ) | ( ( m_nInputBitsBuf & 16 ) >> 1 ) | ( ( m_nInputBitsBuf & 32 ) >> 3 ) | ( ( m_nInputBitsBuf & 64 ) >> 5 ) | ( (m_nInputBitsBuf & 128 ) >> 7 );
    }
}


sal_uInt64 LZWDecompressor::Decompress(sal_uInt8 * pTarget, sal_uInt32 nMaxCount)
{
    if (m_pIStream == nullptr) return 0;

    sal_uInt32 nCount=0;
    for (;;) {

        if (m_pIStream->GetError()) break;

        if (m_nOutBufDataLen >= nMaxCount) {
            m_nOutBufDataLen = m_nOutBufDataLen - static_cast<sal_uInt16>(nMaxCount);
            nCount += nMaxCount;
            while (nMaxCount > 0) {
                *(pTarget++)=*(m_pOutBufData++);
                nMaxCount--;
            }
            break;
        }

        nMaxCount -= static_cast<sal_uInt32>(m_nOutBufDataLen);
        nCount += m_nOutBufDataLen;
        while (m_nOutBufDataLen>0) {
            *(pTarget++) = *(m_pOutBufData++);
            m_nOutBufDataLen--;
        }

        if (m_bEOIFound) break;

        DecompressSome();

    }

    return nCount;
}


sal_uInt16 LZWDecompressor::GetNextCode()
{
    sal_uInt16 nBits;

    if      (m_nTableCurrentId<511)  nBits = 9;
    else if (m_nTableCurrentId<1023) nBits = 10;
    else if (m_nTableCurrentId<2047) nBits = 11;
    else                      nBits = 12;

    sal_uInt16 nCode = 0;
    do {
        if (m_nInputBitsBufSize <= nBits)
        {
            nCode = (nCode << m_nInputBitsBufSize) | m_nInputBitsBuf;
            nBits = nBits - m_nInputBitsBufSize;
            m_pIStream->ReadUChar( m_nInputBitsBuf );
            if ( m_bInvert )
            {
                m_nInputBitsBuf = ( ( m_nInputBitsBuf & 1 ) << 7 ) | ( ( m_nInputBitsBuf & 2 ) << 5 ) | ( ( m_nInputBitsBuf & 4 ) << 3 ) | ( ( m_nInputBitsBuf & 8 ) << 1 ) | ( ( m_nInputBitsBuf & 16 ) >> 1 ) | ( ( m_nInputBitsBuf & 32 ) >> 3 ) | ( ( m_nInputBitsBuf & 64 ) >> 5 ) | ( (m_nInputBitsBuf & 128 ) >> 7 );
            }
            m_nInputBitsBufSize=8;
        }
        else
        {
            nCode = (nCode<<nBits) | ( m_nInputBitsBuf >> (m_nInputBitsBufSize-nBits) );
            m_nInputBitsBufSize = m_nInputBitsBufSize - nBits;
            m_nInputBitsBuf &= 0x00ff >> (8 - m_nInputBitsBufSize);
            nBits = 0;
        }
    } while (nBits > 0);

    return nCode;
}


void LZWDecompressor::AddToTable(sal_uInt16 nPrevCode, sal_uInt16 nCodeFirstData)
{
    if (m_nTableCurrentId >= MAX_TABLE_SIZE)
    {
        //It might be possible to force emit a 256 to flush the buffer and try
        //to continue later?
        SAL_WARN("filter.tiff", "Too much data at scanline");
        m_bEOIFound = true;
        return;
    }

    unsigned char aSeenIndexes[MAX_TABLE_SIZE] = {0};
    while (m_aTable[nCodeFirstData].nDataCount > 1)
    {
        if (aSeenIndexes[nCodeFirstData])
        {
            SAL_WARN("filter.tiff", "Loop in chain");
            m_bEOIFound = true;
            return;
        }
        aSeenIndexes[nCodeFirstData] = 1;
        nCodeFirstData = m_aTable[nCodeFirstData].nPrevCode;
    }

    m_aTable[m_nTableCurrentId].nPrevCode = nPrevCode;
    m_aTable[m_nTableCurrentId].nDataCount = m_aTable[nPrevCode].nDataCount+1;
    m_aTable[m_nTableCurrentId].nData = m_aTable[nCodeFirstData].nData;

    m_nTableCurrentId++;
}


void LZWDecompressor::DecompressSome()
{
    sal_uInt16 nCode = GetNextCode();
    if (nCode == CODE_CLEAR)
    {
        m_nTableCurrentId = CODE_FIRSTMULTICHAR;
        nCode = GetNextCode();
        if (nCode == CODE_EOI)
        {
            m_bEOIFound = true;
        }
    }
    else if (nCode < m_nTableCurrentId)
        AddToTable(m_nOldCode, nCode);
    else if (nCode == m_nTableCurrentId)
        AddToTable(m_nOldCode, m_nOldCode);
    else
    {
        m_bEOIFound = true;
    }

    if (m_bEOIFound)
        return;

    m_nOldCode = nCode;

    m_nOutBufDataLen = m_aTable[nCode].nDataCount;
    m_pOutBufData = m_pOutBuf.data() + m_nOutBufDataLen;
    for (sal_uInt16 i = 0; i < m_nOutBufDataLen; i++)
    {
        *(--m_pOutBufData) = m_aTable[nCode].nData;
        nCode = m_aTable[nCode].nPrevCode;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
