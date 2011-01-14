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
    sal_uInt16 nPrevCode;
    sal_uInt16 nDataCount;
    sal_uInt8 nData;
};

class LZWDecompressor {

public:

    LZWDecompressor();
    ~LZWDecompressor();

    void StartDecompression(SvStream & rIStream);

    sal_uLong Decompress(sal_uInt8 * pTarget, sal_uLong nMaxCount);
        // Liefert die Anzahl der geschriebenen Bytes, wenn < nMaxCount,
        // sind keine weiteren Daten zu entpacken, oder es ist ein
        // Fehler aufgetreten.

private:

    sal_uInt16 GetNextCode();
    void AddToTable(sal_uInt16 nPrevCode, sal_uInt16 nCodeFirstData);
    void DecompressSome();

    SvStream * pIStream;

    LZWTableEntry * pTable;
    sal_uInt16 nTableSize;

    sal_Bool bEOIFound, bInvert, bFirst;

    sal_uInt16 nOldCode;

    sal_uInt8 * pOutBuf;
    sal_uInt8 * pOutBufData;
    sal_uInt16 nOutBufDataLen;

    sal_uInt8 nInputBitsBuf;
    sal_uInt16 nInputBitsBufSize;
};


#endif


