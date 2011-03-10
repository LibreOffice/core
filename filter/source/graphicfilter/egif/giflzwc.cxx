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

#include <tools/stream.hxx>
#include "giflzwc.hxx"

// ----------------------------
// - GIFImageDataOutputStream -
// ----------------------------

class GIFImageDataOutputStream
{
private:

    void        FlushBlockBuf();
    inline void FlushBitsBufsFullBytes();

    SvStream&   rStream;
    sal_uInt8*      pBlockBuf;
    sal_uInt8       nBlockBufSize;
    sal_uLong       nBitsBuf;
    sal_uInt16      nBitsBufSize;

public:

                GIFImageDataOutputStream( SvStream & rGIF, sal_uInt8 nLZWDataSize );
                ~GIFImageDataOutputStream();

    inline void WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen );
};

// ------------------------------------------------------------------------

inline void GIFImageDataOutputStream::FlushBitsBufsFullBytes()
{
    while (nBitsBufSize>=8)
    {
        if( nBlockBufSize==255 )
            FlushBlockBuf();

        pBlockBuf[nBlockBufSize++] = (sal_uInt8) nBitsBuf;
        nBitsBuf >>= 8;
        nBitsBufSize -= 8;
    }
}

// ------------------------------------------------------------------------

inline void GIFImageDataOutputStream::WriteBits( sal_uInt16 nCode, sal_uInt16 nCodeLen )
{
    if( nBitsBufSize+nCodeLen>32 )
        FlushBitsBufsFullBytes();

    nBitsBuf |= (sal_uLong) nCode << nBitsBufSize;
    nBitsBufSize = nBitsBufSize + nCodeLen;
}

// ------------------------------------------------------------------------

GIFImageDataOutputStream::GIFImageDataOutputStream( SvStream & rGIF, sal_uInt8 nLZWDataSize ) :
        rStream(rGIF)
{
    pBlockBuf = new sal_uInt8[ 255 ];
    nBlockBufSize = 0;
    nBitsBufSize = 0;
    nBitsBuf = 0;
    rStream << nLZWDataSize;
}

// ------------------------------------------------------------------------


GIFImageDataOutputStream::~GIFImageDataOutputStream()
{
    WriteBits(0,7);
    FlushBitsBufsFullBytes();
    FlushBlockBuf();
    rStream << (sal_uInt8)0;
    delete[] pBlockBuf;
}

// ------------------------------------------------------------------------

void GIFImageDataOutputStream::FlushBlockBuf()
{
    if( nBlockBufSize )
    {
        rStream << (sal_uInt8) nBlockBufSize;
        rStream.Write( pBlockBuf,nBlockBufSize );
        nBlockBufSize = 0;
    }
}

// -------------------
// - GIFLZWCTreeNode -
// -------------------

struct GIFLZWCTreeNode
{

    GIFLZWCTreeNode*    pBrother;       // naechster Knoten, der den selben Vater hat
    GIFLZWCTreeNode*    pFirstChild;    // erster Sohn
    sal_uInt16              nCode;          // Der Code fuer den String von Pixelwerten, der sich ergibt, wenn
    sal_uInt16              nValue;         // Der Pixelwert
};

// --------------------
// - GIFLZWCompressor -
// --------------------

GIFLZWCompressor::GIFLZWCompressor()
{
    pIDOS=NULL;
}

// ------------------------------------------------------------------------

GIFLZWCompressor::~GIFLZWCompressor()
{
    if (pIDOS!=NULL) EndCompression();
}

// ------------------------------------------------------------------------

void GIFLZWCompressor::StartCompression( SvStream& rGIF, sal_uInt16 nPixelSize )
{
    if( !pIDOS )
    {
        sal_uInt16 i;

        nDataSize = nPixelSize;

        if( nDataSize < 2 )
            nDataSize=2;

        nClearCode=1<<nDataSize;
        nEOICode=nClearCode+1;
        nTableSize=nEOICode+1;
        nCodeSize=nDataSize+1;

        pIDOS=new GIFImageDataOutputStream(rGIF,(sal_uInt8)nDataSize);
        pTable=new GIFLZWCTreeNode[4096];

        for (i=0; i<4096; i++)
        {
            pTable[i].pBrother = pTable[i].pFirstChild = NULL;
            pTable[i].nValue = (sal_uInt8) ( pTable[i].nCode = i );
        }

        pPrefix = NULL;
        pIDOS->WriteBits( nClearCode,nCodeSize );
    }
}

// ------------------------------------------------------------------------

void GIFLZWCompressor::Compress( HPBYTE pSrc, sal_uLong nSize )
{
    if( pIDOS )
    {
        GIFLZWCTreeNode*    p;
        sal_uInt16              i;
        sal_uInt8               nV;

        if( !pPrefix && nSize )
        {
            pPrefix=pTable+(*pSrc++);
            nSize--;
        }

        while( nSize )
        {
            nSize--;
            nV=*pSrc++;
            for( p=pPrefix->pFirstChild; p!=NULL; p=p->pBrother )
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
                        pTable[i].pFirstChild=NULL;

                    nCodeSize=nDataSize+1;
                    nTableSize=nEOICode+1;
                }
                else
                {
                    if(nTableSize==(sal_uInt16)(1<<nCodeSize))
                        nCodeSize++;

                    p=pTable+(nTableSize++);
                    p->pBrother=pPrefix->pFirstChild;
                    pPrefix->pFirstChild=p;
                    p->nValue=nV;
                    p->pFirstChild=NULL;
                }

                pPrefix=pTable+nV;
            }
        }
    }
}

// ------------------------------------------------------------------------

void GIFLZWCompressor::EndCompression()
{
    if( pIDOS )
    {
        if( pPrefix )
            pIDOS->WriteBits(pPrefix->nCode,nCodeSize);

        pIDOS->WriteBits( nEOICode,nCodeSize );
        delete[] pTable;
        delete pIDOS;
        pIDOS=NULL;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
