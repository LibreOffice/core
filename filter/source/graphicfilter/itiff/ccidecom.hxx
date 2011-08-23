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

#ifndef _CCIDECOM_HXX
#define _CCIDECOM_HXX

#include <tools/stream.hxx>


#define CCI_OPTION_2D				1		// 2D-Komprimierung (statt 1D)
#define CCI_OPTION_EOL				2		// EOL-Codes am Ende jeder Zeile vorhanden
#define CCI_OPTION_BYTEALIGNEOL		4		// Fuellbits vor jedem EOL-Code, so dass
                                            // Ende von EOL auf Bytes aligend
#define CCI_OPTION_BYTEALIGNROW		8		// Rows beginnen immer auf Byte-Grenze
#define CCI_OPTION_INVERSEBITORDER	16

// Eintrag in eine Huffman-Tabelle:
struct CCIHuffmanTableEntry {
    USHORT nValue;    // Der Daten-Wert.
    USHORT nCode;     // Der Code durch den der Daten-Wert repraesentiert wird.
    USHORT nCodeBits; // Laenge des Codes in Bits.
};


// Eintrag in eine Hash-Tabelle zur schnellen Dekodierung
struct CCILookUpTableEntry {
    USHORT nValue;
    USHORT nCodeBits;
};


class CCIDecompressor {

public:

    CCIDecompressor( ULONG nOptions, UINT32 nImageWidth );
    ~CCIDecompressor();

    void StartDecompression( SvStream & rIStream );

    BOOL DecompressScanline(BYTE * pTarget, ULONG nTargetBits );

private:

    void MakeLookUp(const CCIHuffmanTableEntry * pHufTab,
                    const CCIHuffmanTableEntry * pHufTabSave,
                    CCILookUpTableEntry * pLookUp,
                    USHORT nHuffmanTableSize,
                    USHORT nMaxCodeBits);

    BOOL ReadEOL( UINT32 nMaxFillBits );

    BOOL Read2DTag();

    BYTE ReadBlackOrWhite();

    USHORT ReadCodeAndDecode(const CCILookUpTableEntry * pLookUp,
                             USHORT nMaxCodeBits);

    void FillBits(BYTE * pTarget, USHORT nTargetBits,
                  USHORT nBitPos, USHORT nNumBits,
                  BYTE nBlackOrWhite);

    USHORT CountBits(const BYTE * pData, USHORT nDataSizeBits,
                     USHORT nBitPos, BYTE nBlackOrWhite);

    void Read1DScanlineData(BYTE * pTarget, USHORT nTargetBits);

    void Read2DScanlineData(BYTE * pTarget, USHORT nTargetBits);

    BOOL bTableBad;

    BOOL bStatus;

    BYTE* pByteSwap;

    SvStream * pIStream;

    UINT32 nEOLCount;

    UINT32 nWidth;

    ULONG nOptions;

    BOOL bFirstEOL;

    CCILookUpTableEntry * pWhiteLookUp;
    CCILookUpTableEntry * pBlackLookUp;
    CCILookUpTableEntry * p2DModeLookUp;
    CCILookUpTableEntry * pUncompLookUp;

    ULONG nInputBitsBuf;
    USHORT nInputBitsBufSize;

    BYTE * pLastLine;
    ULONG nLastLineSize;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
