/*************************************************************************
 *
 *  $RCSfile: giflzwc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:11 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    BYTE*       pBlockBuf;
    BYTE        nBlockBufSize;
    ULONG       nBitsBuf;
    USHORT      nBitsBufSize;

public:

                GIFImageDataOutputStream( SvStream & rGIF, BYTE nLZWDataSize );
                ~GIFImageDataOutputStream();

    inline void WriteBits( USHORT nCode, USHORT nCodeLen );
};

// ------------------------------------------------------------------------

inline void GIFImageDataOutputStream::FlushBitsBufsFullBytes()
{
    while (nBitsBufSize>=8)
    {
        if( nBlockBufSize==255 )
            FlushBlockBuf();

        pBlockBuf[nBlockBufSize++] = (BYTE) nBitsBuf;
        nBitsBuf >>= 8;
        nBitsBufSize -= 8;
    }
}

// ------------------------------------------------------------------------

inline void GIFImageDataOutputStream::WriteBits( USHORT nCode, USHORT nCodeLen )
{
    if( nBitsBufSize+nCodeLen>32 )
        FlushBitsBufsFullBytes();

    nBitsBuf |= (ULONG) nCode << nBitsBufSize;
    nBitsBufSize += nCodeLen;
}

// ------------------------------------------------------------------------

GIFImageDataOutputStream::GIFImageDataOutputStream( SvStream & rGIF, BYTE nLZWDataSize ) :
        rStream(rGIF)
{
    pBlockBuf = new BYTE[ 255 ];
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
    rStream << (BYTE)0;
    delete[] pBlockBuf;
}

// ------------------------------------------------------------------------

void GIFImageDataOutputStream::FlushBlockBuf()
{
    if( nBlockBufSize )
    {
        rStream << (BYTE) nBlockBufSize;
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
    USHORT              nCode;          // Der Code fuer den String von Pixelwerten, der sich ergibt, wenn
    USHORT              nValue;         // Der Pixelwert
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

void GIFLZWCompressor::StartCompression( SvStream& rGIF, USHORT nPixelSize )
{
    if( !pIDOS )
    {
        USHORT i;

        nDataSize = nPixelSize;

        if( nDataSize < 2 )
            nDataSize=2;

        nClearCode=1<<nDataSize;
        nEOICode=nClearCode+1;
        nTableSize=nEOICode+1;
        nCodeSize=nDataSize+1;

        pIDOS=new GIFImageDataOutputStream(rGIF,(BYTE)nDataSize);
        pTable=new GIFLZWCTreeNode[4096];

        for (i=0; i<4096; i++)
        {
            pTable[i].pBrother = pTable[i].pFirstChild = NULL;
            pTable[i].nValue = (BYTE) ( pTable[i].nCode = i );
        }

        pPrefix = NULL;
        pIDOS->WriteBits( nClearCode,nCodeSize );
    }
}

// ------------------------------------------------------------------------

void GIFLZWCompressor::Compress( HPBYTE pSrc, ULONG nSize )
{
    if( pIDOS )
    {
        GIFLZWCTreeNode*    p;
        USHORT              i;
        BYTE                nV;

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
                    if(nTableSize==(USHORT)(1<<nCodeSize))
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
