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

#pragma once

#include <tools/long.hxx>
#include <vcl/BitmapPalette.hxx>

namespace vcl
{
// Write color information for 1, 4 and 8 bit palette bitmap scanlines.
class ScanlineWriter
{
    BitmapPalette& maPalette;
    sal_uInt8 const mnColorsPerByte; // number of colors that are stored in one byte
    sal_uInt8 const mnColorBitSize; // number of bits a color takes
    sal_uInt8 const mnColorBitMask; // bit mask used to isolate the color
    sal_uInt8* mpCurrentScanline;
    tools::Long mnX;

public:
    ScanlineWriter(BitmapPalette& aPalette, sal_Int8 nColorsPerByte)
        : maPalette(aPalette)
        , mnColorsPerByte(nColorsPerByte)
        , mnColorBitSize(
              8
              / mnColorsPerByte) // bit size is number of bit in a byte divided by number of colors per byte (8 / 2 = 4 for 4-bit)
        , mnColorBitMask((1 << mnColorBitSize) - 1) // calculate the bit mask from the bit size
        , mpCurrentScanline(nullptr)
        , mnX(0)
    {
    }

    static std::unique_ptr<ScanlineWriter> Create(sal_uInt16 nBits, BitmapPalette& aPalette)
    {
        switch (nBits)
        {
            case 1:
                return std::make_unique<ScanlineWriter>(aPalette, 8);
            case 4:
                return std::make_unique<ScanlineWriter>(aPalette, 2);
            case 8:
                return std::make_unique<ScanlineWriter>(aPalette, 1);
            default:
                abort();
        }
    }

    void writeRGB(sal_uInt8 nR, sal_uInt8 nG, sal_uInt8 nB)
    {
        // calculate to which index we will write
        tools::Long nScanlineIndex = mnX / mnColorsPerByte;

        // calculate the number of shifts to get the color information to the right place
        tools::Long nShift = (8 - mnColorBitSize) - ((mnX % mnColorsPerByte) * mnColorBitSize);

        sal_uInt16 nColorIndex = maPalette.GetBestIndex(BitmapColor(nR, nG, nB));
        mpCurrentScanline[nScanlineIndex] &= ~(mnColorBitMask << nShift); // clear
        mpCurrentScanline[nScanlineIndex] |= (nColorIndex & mnColorBitMask) << nShift; // set
        mnX++;
    }

    void nextLine(sal_uInt8* pScanline)
    {
        mnX = 0;
        mpCurrentScanline = pScanline;
    }
};

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
