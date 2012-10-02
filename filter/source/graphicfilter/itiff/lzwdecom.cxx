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

LZWDecompressor::LZWDecompressor()
    : pOutBufData(NULL)
{
    sal_uInt16 i;

    pTable=new LZWTableEntry[4096];
    pOutBuf=new sal_uInt8[4096];
    for (i=0; i<4096; i++)
    {
        pTable[i].nPrevCode=0;
        pTable[i].nDataCount=1;
        pTable[i].nData=(sal_uInt8)i;
    }
    pIStream=NULL;
    bFirst = sal_True;
    nOldCode = 0;
}


LZWDecompressor::~LZWDecompressor()
{
    delete[] pOutBuf;
    delete[] pTable;
}


void LZWDecompressor::StartDecompression(SvStream & rIStream)
{
    pIStream=&rIStream;

    nTableSize=258;

    bEOIFound=sal_False;

    nOutBufDataLen=0;

    *pIStream >> nInputBitsBuf;

    nInputBitsBufSize=8;

    if ( bFirst )
    {
        bInvert = nInputBitsBuf == 1;
        bFirst = sal_False;
    }

    if ( bInvert )
        nInputBitsBuf = ( ( nInputBitsBuf & 1 ) << 7 ) | ( ( nInputBitsBuf & 2 ) << 5 ) | ( ( nInputBitsBuf & 4 ) << 3 ) | ( ( nInputBitsBuf & 8 ) << 1 ) | ( ( nInputBitsBuf & 16 ) >> 1 ) | ( ( nInputBitsBuf & 32 ) >> 3 ) | ( ( nInputBitsBuf & 64 ) >> 5 ) | ( (nInputBitsBuf & 128 ) >> 7 );
}


sal_uLong LZWDecompressor::Decompress(sal_uInt8 * pTarget, sal_uLong nMaxCount)
{
    sal_uLong nCount;

    if (pIStream==NULL) return 0;

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

        if (bEOIFound==sal_True) break;

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
            *pIStream >> nInputBitsBuf;
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
    while (pTable[nCodeFirstData].nDataCount>1)
        nCodeFirstData=pTable[nCodeFirstData].nPrevCode;

    pTable[nTableSize].nPrevCode=nPrevCode;
    pTable[nTableSize].nDataCount=pTable[nPrevCode].nDataCount+1;
    pTable[nTableSize].nData=pTable[nCodeFirstData].nData;

    nTableSize++;
}


void LZWDecompressor::DecompressSome()
{
    sal_uInt16 i,nCode;

    nCode=GetNextCode();
    if (nCode==256) {
        nTableSize=258;
        nCode=GetNextCode();
        if (nCode==257) { bEOIFound=sal_True; return; }
    }
    else if (nCode<nTableSize) AddToTable(nOldCode,nCode);
    else if (nCode==nTableSize) AddToTable(nOldCode,nOldCode);
    else { bEOIFound=sal_True; return; }

    nOldCode=nCode;

    nOutBufDataLen=pTable[nCode].nDataCount;
    pOutBufData=pOutBuf+nOutBufDataLen;
    for (i=0; i<nOutBufDataLen; i++) {
        *(--pOutBufData)=pTable[nCode].nData;
        nCode=pTable[nCode].nPrevCode;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
