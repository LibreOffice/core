/*************************************************************************
 *
 *  $RCSfile: lzwdecom.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:16 $
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

#include <tools/new.hxx>
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
    delete pOutBuf;
    delete pTable;
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
            nOutBufDataLen-=(USHORT)nMaxCount;
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
            nBits-=nInputBitsBufSize;
            *pIStream >> nInputBitsBuf;
            if ( bInvert )
                nInputBitsBuf = ( ( nInputBitsBuf & 1 ) << 7 ) | ( ( nInputBitsBuf & 2 ) << 5 ) | ( ( nInputBitsBuf & 4 ) << 3 ) | ( ( nInputBitsBuf & 8 ) << 1 ) | ( ( nInputBitsBuf & 16 ) >> 1 ) | ( ( nInputBitsBuf & 32 ) >> 3 ) | ( ( nInputBitsBuf & 64 ) >> 5 ) | ( (nInputBitsBuf & 128 ) >> 7 );
            nInputBitsBufSize=8;
        }
        else
        {
            nCode=(nCode<<nBits) | (nInputBitsBuf>>(nInputBitsBufSize-nBits));
            nInputBitsBufSize-=nBits;
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



