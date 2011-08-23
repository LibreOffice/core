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


