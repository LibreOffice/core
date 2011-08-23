/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_filter.hxx"

#include "lzwdecom.hxx"

LZWDecompressor::LZWDecompressor()
{
    USHORT i;

    pTable=new LZWTableEntry[4096];
    pOutBuf=new BYTE[4096];
    for (i=0; i<4096; i++)
    {
        pTable[i].nPrevCode=0;
        pTable[i].nDataCount=1;
        pTable[i].nData=(BYTE)i;
    }
    pIStream=NULL;
    bFirst = TRUE;
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

    bEOIFound=FALSE;

    nOutBufDataLen=0;

    *pIStream >> nInputBitsBuf;

    nInputBitsBufSize=8;

    if ( bFirst )
    {
        bInvert = nInputBitsBuf == 1;
        bFirst = FALSE;
    }

    if ( bInvert )
        nInputBitsBuf = ( ( nInputBitsBuf & 1 ) << 7 ) | ( ( nInputBitsBuf & 2 ) << 5 ) | ( ( nInputBitsBuf & 4 ) << 3 ) | ( ( nInputBitsBuf & 8 ) << 1 ) | ( ( nInputBitsBuf & 16 ) >> 1 ) | ( ( nInputBitsBuf & 32 ) >> 3 ) | ( ( nInputBitsBuf & 64 ) >> 5 ) | ( (nInputBitsBuf & 128 ) >> 7 );
}


ULONG LZWDecompressor::Decompress(BYTE * pTarget, ULONG nMaxCount)
{
    ULONG nCount;

    if (pIStream==NULL) return 0;

    nCount=0;
    for (;;) {

        if (pIStream->GetError()) break;

        if (((ULONG)nOutBufDataLen)>=nMaxCount) {
            nOutBufDataLen = nOutBufDataLen - (USHORT)nMaxCount;
            nCount+=nMaxCount;
            while (nMaxCount>0) {
                *(pTarget++)=*(pOutBufData++);
                nMaxCount--;
            }
            break;
        }

        nMaxCount-=(ULONG)nOutBufDataLen;
        nCount+=nOutBufDataLen;
        while (nOutBufDataLen>0) {
            *(pTarget++)=*(pOutBufData++);
            nOutBufDataLen--;
        }

        if (bEOIFound==TRUE) break;

        DecompressSome();

    }

    return nCount;
}


USHORT LZWDecompressor::GetNextCode()
{
    USHORT nBits,nCode;

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


void LZWDecompressor::AddToTable(USHORT nPrevCode, USHORT nCodeFirstData)
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
    USHORT i,nCode;

    nCode=GetNextCode();
    if (nCode==256) {
        nTableSize=258;
        nCode=GetNextCode();
        if (nCode==257) { bEOIFound=TRUE; return; }
    }
    else if (nCode<nTableSize) AddToTable(nOldCode,nCode);
    else if (nCode==nTableSize) AddToTable(nOldCode,nOldCode);
    else { bEOIFound=TRUE; return; }

    nOldCode=nCode;

    nOutBufDataLen=pTable[nCode].nDataCount;
    pOutBufData=pOutBuf+nOutBufDataLen;
    for (i=0; i<nOutBufDataLen; i++) {
        *(--pOutBufData)=pTable[nCode].nData;
        nCode=pTable[nCode].nPrevCode;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
