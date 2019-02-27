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

#ifndef INCLUDED_VCL_SOURCE_FILTER_IGIF_DECODE_HXX
#define INCLUDED_VCL_SOURCE_FILTER_IGIF_DECODE_HXX

#include <vcl/bitmapaccess.hxx>
#include <array>
#include <memory>

struct GIFLZWTableEntry;

class GIFLZWDecompressor
{
    std::unique_ptr<GIFLZWTableEntry[]>
                            pTable;
    std::array<sal_uInt8, 4096>
                            pOutBuf;
    sal_uInt8*              pOutBufData;
    sal_uInt8*              pBlockBuf;
    sal_uLong               nInputBitsBuf;
    bool                    bEOIFound;
    sal_uInt8 const         nDataSize;
    sal_uInt8               nBlockBufSize;
    sal_uInt8               nBlockBufPos;
    sal_uInt16 const        nClearCode;
    sal_uInt16 const        nEOICode;
    sal_uInt16              nTableSize;
    sal_uInt16              nCodeSize;
    sal_uInt16              nOldCode;
    sal_uInt16              nOutBufDataLen;
    sal_uInt16              nInputBitsBufSize;

    bool                AddToTable(sal_uInt16 nPrevCode, sal_uInt16 nCodeFirstData);
    bool                ProcessOneCode();

    GIFLZWDecompressor(const GIFLZWDecompressor&) = delete;
    GIFLZWDecompressor& operator=(const GIFLZWDecompressor&) = delete;
public:

    explicit GIFLZWDecompressor( sal_uInt8 cDataSize );
                        ~GIFLZWDecompressor();

    Scanline            DecompressBlock( sal_uInt8* pSrc, sal_uInt8 cBufSize, sal_uLong& rCount, bool& rEOI );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
