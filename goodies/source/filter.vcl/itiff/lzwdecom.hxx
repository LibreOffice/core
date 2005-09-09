/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lzwdecom.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:03:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _LZWDECOM_HXX
#define _LZWDECOM_HXX

#include <tools/stream.hxx>

struct LZWTableEntry {
    USHORT nPrevCode;
    USHORT nDataCount;
    BYTE nData;
};

class LZWDecompressor {

public:

    LZWDecompressor();
    ~LZWDecompressor();

    void StartDecompression(SvStream & rIStream);

    ULONG Decompress(BYTE * pTarget, ULONG nMaxCount);
        // Liefert die Anzahl der geschriebenen Bytes, wenn < nMaxCount,
        // sind keine weiteren Daten zu entpacken, oder es ist ein
        // Fehler aufgetreten.

private:

    USHORT GetNextCode();
    void AddToTable(USHORT nPrevCode, USHORT nCodeFirstData);
    void DecompressSome();

    SvStream * pIStream;

    LZWTableEntry * pTable;
    USHORT nTableSize;

    BOOL bEOIFound, bInvert, bFirst;

    USHORT nOldCode;

    BYTE * pOutBuf;
    BYTE * pOutBufData;
    USHORT nOutBufDataLen;

    BYTE nInputBitsBuf;
    USHORT nInputBitsBufSize;
};


#endif


