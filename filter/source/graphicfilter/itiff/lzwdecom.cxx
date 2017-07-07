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
#include <tools/stream.hxx>
#include <algorithm>
#include <set>

LZWDecompressor::LZWDecompressor()
    : pIStream(nullptr)
    , nTableSize(0)
    , bEOIFound(false)
    , bInvert(false)
    , bFirst(true)
    , nOldCode(0)
    , pOutBufData(nullptr)
    , nOutBufDataLen(0)
    , nInputBitsBuf(0)
    , nInputBitsBufSize(0)
{
    for (sal_uInt16 i=0; i<MAX_TABLE_SIZE; i++)
    {
        pTable[i].nPrevCode=0;
        pTable[i].nDataCount=1;
        pTable[i].nData=(sal_uInt8)i;
    }
}


LZWDecompressor::~LZWDecompressor()
{
}


void LZWDecompressor::StartDecompression(SvStream & rIStream)
{
    pIStream=&rIStream;

    nTableSize=258;

    bEOIFound=false;

    nOutBufDataLen=0;

    pIStream->ReadUChar( nInputBitsBuf );

    nInputBitsBufSize=8;

    if ( bFirst )
    {
        bInvert = nInputBitsBuf == 1;
        bFirst = false;
    }

    if ( bInvert )
        nInputBitsBuf = ( ( nInputBitsBuf & 1 ) << 7 ) | ( ( nInputBitsBuf & 2 ) << 5 ) | ( ( nInputBitsBuf & 4 ) << 3 ) | ( ( nInputBitsBuf & 8 ) << 1 ) | ( ( nInputBitsBuf & 16 ) >> 1 ) | ( ( nInputBitsBuf & 32 ) >> 3 ) | ( ( nInputBitsBuf & 64 ) >> 5 ) | ( (nInputBitsBuf & 128 ) >> 7 );
}


sal_uLong LZWDecompressor::Decompress(sal_uInt8 * pTarget, sal_uLong nMaxCount)
{
    sal_uLong nCount;

    if (pIStream==nullptr) return 0;

    nCount=0;
    for (;;) {

        if (pIStream->GetError()) break;

        if (((sal_uLong)nOutBufDataLen)>=nMaxCount) {
            nOutBufDataLen = nOutBufDataLen - (sal_uInt16)nMaxCount;
            nCount+=nMaxCount;
            while (nMaxCount>0) {
                *(pTarget++)=*(pOutBufData++);
                nMaxCount--;
            }
            break;
        }

        nMaxCount-=(sal_uLong)nOutBufDataLen;
        nCount+=nOutBufDataLen;
        while (nOutBufDataLen>0) {
            *(pTarget++)=*(pOutBufData++);
            nOutBufDataLen--;
        }

        if (bEOIFound) break;

        DecompressSome();

    }

    return nCount;
}


sal_uInt16 LZWDecompressor::GetNextCode()
{
    sal_uInt16 nBits,nCode;

    if      (nTableSize<511)  nBits=9;
    else if (nTableSize<1023) nBits=10;
    else if (nTableSize<2047) nBits=11;
    else                      nBits=12;

    nCode=0;
    do {
        if (nInputBitsBufSize<=nBits)
        {
            nCode=(nCode<<nInputBitsBufSize) | nInputBitsBuf;
            nBits = nBits - nInputBitsBufSize;
            pIStream->ReadUChar( nInputBitsBuf );
            if ( bInvert )
                nInputBitsBuf = ( ( nInputBitsBuf & 1 ) << 7 ) | ( ( nInputBitsBuf & 2 ) << 5 ) | ( ( nInputBitsBuf & 4 ) << 3 ) | ( ( nInputBitsBuf & 8 ) << 1 ) | ( ( nInputBitsBuf & 16 ) >> 1 ) | ( ( nInputBitsBuf & 32 ) >> 3 ) | ( ( nInputBitsBuf & 64 ) >> 5 ) | ( (nInputBitsBuf & 128 ) >> 7 );
            nInputBitsBufSize=8;
        }
        else
        {
            nCode=(nCode<<nBits) | (nInputBitsBuf>>(nInputBitsBufSize-nBits));
            nInputBitsBufSize = nInputBitsBufSize - nBits;
            nInputBitsBuf&=0x00ff>>(8-nInputBitsBufSize);
            nBits=0;
        }
    } while (nBits>0);

    return nCode;
}


void LZWDecompressor::AddToTable(sal_uInt16 nPrevCode, sal_uInt16 nCodeFirstData)
{
    if (nTableSize >= MAX_TABLE_SIZE)
    {
        //It might be possible to force emit a 256 to flush the buffer and try
        //to continue later?
        SAL_WARN("filter.tiff", "Too much data at scanline");
        bEOIFound = true;
        return;
    }

    unsigned char aSeenIndexes[MAX_TABLE_SIZE] = {0};
    while (pTable[nCodeFirstData].nDataCount>1)
    {
        if (aSeenIndexes[nCodeFirstData])
        {
            SAL_WARN("filter.tiff", "Loop in chain");
            bEOIFound = true;
            return;
        }
        aSeenIndexes[nCodeFirstData] = 1;
        nCodeFirstData=pTable[nCodeFirstData].nPrevCode;
    }

    pTable[nTableSize].nPrevCode=nPrevCode;
    pTable[nTableSize].nDataCount=pTable[nPrevCode].nDataCount+1;
    pTable[nTableSize].nData=pTable[nCodeFirstData].nData;

    nTableSize++;
}


void LZWDecompressor::DecompressSome()
{
    sal_uInt16 i,nCode;

    nCode=GetNextCode();
    if (nCode==256)
    {
        nTableSize=258;
        nCode=GetNextCode();
        if (nCode==257)
        {
            bEOIFound=true;
        }
    }
    else if (nCode<nTableSize)
        AddToTable(nOldCode,nCode);
    else if (nCode==nTableSize)
        AddToTable(nOldCode,nOldCode);
    else
    {
        bEOIFound=true;
    }

    if (bEOIFound)
        return;

    nOldCode=nCode;

    nOutBufDataLen=pTable[nCode].nDataCount;
    pOutBufData=pOutBuf.data()+nOutBufDataLen;
    for (i=0; i<nOutBufDataLen; i++)
    {
        *(--pOutBufData)=pTable[nCode].nData;
        nCode=pTable[nCode].nPrevCode;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
