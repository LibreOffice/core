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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ITIFF_CCIDECOM_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ITIFF_CCIDECOM_HXX

#include <sal/types.h>
#include <tools/solar.h>

#define CCI_OPTION_2D               1       // 2D compression (instead of 1D)
#define CCI_OPTION_EOL              2       // There are EOL-Codes at the end of each line.
#define CCI_OPTION_BYTEALIGNEOL     4       // Filling bits before each EOL-Code, so that
                                            // the end of EOL is bytes aligned
#define CCI_OPTION_BYTEALIGNROW     8       // Rows always start byte aligned
#define CCI_OPTION_INVERSEBITORDER  16

// Entry in the Huffman table:
struct CCIHuffmanTableEntry {
    sal_uInt16 nValue;    // The data value.
    sal_uInt16 nCode;     // The code through which the data value is represented.
    sal_uInt16 nCodeBits; // Size of the code in bits.
};

// Entry in a hash table for daft decoding.
struct CCILookUpTableEntry {
    sal_uInt16 nValue;
    sal_uInt16 nCodeBits;
};

class SvStream;

class CCIDecompressor {

public:

    CCIDecompressor( sal_uLong nOptions, sal_uInt32 nImageWidth );
    ~CCIDecompressor();

    void StartDecompression( SvStream & rIStream );

    bool DecompressScanline(sal_uInt8 * pTarget, sal_uLong nTargetBits, bool bLastLine );

private:

    void MakeLookUp(const CCIHuffmanTableEntry * pHufTab,
                    const CCIHuffmanTableEntry * pHufTabSave,
                    CCILookUpTableEntry * pLookUp,
                    sal_uInt16 nHuffmanTableSize,
                    sal_uInt16 nMaxCodeBits);

    bool ReadEOL( sal_uInt32 nMaxFillBits );

    bool Read2DTag();

    sal_uInt8 ReadBlackOrWhite();

    sal_uInt16 ReadCodeAndDecode(const CCILookUpTableEntry * pLookUp,
                             sal_uInt16 nMaxCodeBits);

    static void FillBits(sal_uInt8 * pTarget, sal_uInt16 nTargetBits,
                  sal_uInt16 nBitPos, sal_uInt16 nNumBits,
                  sal_uInt8 nBlackOrWhite);

    static sal_uInt16 CountBits(const sal_uInt8 * pData, sal_uInt16 nDataSizeBits,
                     sal_uInt16 nBitPos, sal_uInt8 nBlackOrWhite);

    void Read1DScanlineData(sal_uInt8 * pTarget, sal_uInt16 nTargetBits);

    void Read2DScanlineData(sal_uInt8 * pTarget, sal_uInt16 nTargetBits);

    bool bTableBad;

    bool bStatus;

    sal_uInt8* pByteSwap;

    SvStream * pIStream;

    sal_uInt32 nEOLCount;

    sal_uInt32 nWidth;

    sal_uLong nOptions;

    bool bFirstEOL;

    CCILookUpTableEntry * pWhiteLookUp;
    CCILookUpTableEntry * pBlackLookUp;
    CCILookUpTableEntry * p2DModeLookUp;
    CCILookUpTableEntry * pUncompLookUp;

    sal_uLong nInputBitsBuf;
    sal_uInt16 nInputBitsBufSize;

    sal_uInt8 * pLastLine;
    sal_uLong nLastLineSize;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
