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


#include <tools/stream.hxx>
#include "giflzwc.hxx"
#include <array>


class GIFImageDataOutputStream
{
private:

    void        FlushBlockBuf();
    inline void FlushBitsBufsFullBytes();

    SvStream&   rStream;
    std::array<sal_uInt8, 255>
                    pBlockBuf;
    sal_uInt8       nBlockBufSize;
    sal_uInt32      nBitsBuf;
    sal_uInt16      nBitsBufSize;

public:

                GIFImageDataOutputStream( SvStream & rGIF, sal_uInt8 nLZWDataSize );
                ~GIFImageDataOutputStream();

    inline void WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen );
};


inline void GIFImageDataOutputStream::FlushBitsBufsFullBytes()
{
    while (nBitsBufSize>=8)
    {
        if( nBlockBufSize==255 )
            FlushBlockBuf();

        pBlockBuf[nBlockBufSize++] = static_cast<sal_uInt8>(nBitsBuf);
        nBitsBuf >>= 8;
        nBitsBufSize -= 8;
    }
}


inline void GIFImageDataOutputStream::WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen )
{
    if( nBitsBufSize+nCodeLen>32 )
        FlushBitsBufsFullBytes();

    nBitsBuf |= static_cast<sal_uInt32>(nCode) << nBitsBufSize;
    nBitsBufSize = nBitsBufSize + nCodeLen;
}


GIFImageDataOutputStream::GIFImageDataOutputStream( SvStream & rGIF, sal_uInt8 nLZWDataSize ) :
        rStream(rGIF), nBlockBufSize(0), nBitsBuf(0), nBitsBufSize(0)
{
    rStream.WriteUChar( nLZWDataSize );
}


GIFImageDataOutputStream::~GIFImageDataOutputStream()
{
    WriteBits(0,7);
    FlushBitsBufsFullBytes();
    FlushBlockBuf();
    rStream.WriteUChar( 0 );
}


void GIFImageDataOutputStream::FlushBlockBuf()
{
    if( nBlockBufSize )
    {
        rStream.WriteUChar( nBlockBufSize );
        rStream.WriteBytes(pBlockBuf.data(), nBlockBufSize);
        nBlockBufSize = 0;
    }
}


struct GIFLZWCTreeNode
{

    GIFLZWCTreeNode*    pBrother;       // next node which has the same father
    GIFLZWCTreeNode*    pFirstChild;    // first
    sal_uInt16          nCode;          // the code for the string of pixel values which comes about
    sal_uInt16          nValue;         // the pixel value
};


GIFLZWCompressor::GIFLZWCompressor()
    : pPrefix(nullptr), nDataSize(0), nClearCode(0),
      nEOICode(0), nTableSize(0), nCodeSize(0)
{
}


GIFLZWCompressor::~GIFLZWCompressor()
{
    if (pIDOS!=nullptr) EndCompression();
}


void GIFLZWCompressor::StartCompression( SvStream& rGIF, sal_uInt16 nPixelSize )
{
    if( pIDOS )
        return;

    sal_uInt16 i;

    nDataSize = nPixelSize;

    if( nDataSize < 2 )
        nDataSize=2;

    nClearCode=1<<nDataSize;
    nEOICode=nClearCode+1;
    nTableSize=nEOICode+1;
    nCodeSize=nDataSize+1;

    pIDOS.reset(new GIFImageDataOutputStream(rGIF,static_cast<sal_uInt8>(nDataSize)));
    pTable.reset(new GIFLZWCTreeNode[4096]);

    for (i=0; i<4096; i++)
    {
        pTable[i].pBrother = pTable[i].pFirstChild = nullptr;
        pTable[i].nCode = i;
        pTable[i].nValue = static_cast<sal_uInt8>( i );
    }

    pPrefix = nullptr;
    pIDOS->WriteBits( nClearCode,nCodeSize );
}

void GIFLZWCompressor::Compress(sal_uInt8* pSrc, sal_uInt32 nSize)
{
    if( !pIDOS )
        return;

    GIFLZWCTreeNode* p;
    sal_uInt16 i;
    sal_uInt8 nV;

    if( !pPrefix && nSize )
    {
        pPrefix=&pTable[*pSrc++];
        nSize--;
    }

    while( nSize )
    {
        nSize--;
        nV=*pSrc++;
        for( p=pPrefix->pFirstChild; p!=nullptr; p=p->pBrother )
        {
            if (p->nValue==nV)
                break;
        }

        if( p)
            pPrefix=p;
        else
        {
            pIDOS->WriteBits(pPrefix->nCode,nCodeSize);

            if (nTableSize==4096)
            {
                pIDOS->WriteBits(nClearCode,nCodeSize);

                for (i=0; i<nClearCode; i++)
                    pTable[i].pFirstChild=nullptr;

                nCodeSize=nDataSize+1;
                nTableSize=nEOICode+1;
            }
            else
            {
                if(nTableSize==static_cast<sal_uInt16>(1<<nCodeSize))
                    nCodeSize++;

                p=&pTable[nTableSize++];
                p->pBrother=pPrefix->pFirstChild;
                pPrefix->pFirstChild=p;
                p->nValue=nV;
                p->pFirstChild=nullptr;
            }

            pPrefix=&pTable[nV];
        }
    }
}

void GIFLZWCompressor::EndCompression()
{
    if( pIDOS )
    {
        if( pPrefix )
            pIDOS->WriteBits(pPrefix->nCode,nCodeSize);

        pIDOS->WriteBits( nEOICode,nCodeSize );
        pTable.reset();
        pIDOS.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
