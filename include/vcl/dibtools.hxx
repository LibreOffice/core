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

#ifndef INCLUDED_VCL_DIBTOOLS_HXX
#define INCLUDED_VCL_DIBTOOLS_HXX

#include <vcl/dllapi.h>
#include <vcl/Scanline.hxx>

class SvStream;
class BitmapEx;
class Bitmap;
class AlphaMask;

#define DIBCOREHEADERSIZE       ( 12UL )
#define DIBINFOHEADERSIZE       ( sizeof(DIBInfoHeader) )
#define DIBV5HEADERSIZE         ( sizeof(DIBV5Header) )

// - DIBInfoHeader and DIBV5Header

typedef sal_Int32 FXPT2DOT30;

struct CIEXYZ
{
    FXPT2DOT30      aXyzX;
    FXPT2DOT30      aXyzY;
    FXPT2DOT30      aXyzZ;

    CIEXYZ()
    :   aXyzX(0),
        aXyzY(0),
        aXyzZ(0)
    {}
};

struct CIEXYZTriple
{
    CIEXYZ          aXyzRed;
    CIEXYZ          aXyzGreen;
    CIEXYZ          aXyzBlue;

    CIEXYZTriple()
    :   aXyzRed(),
        aXyzGreen(),
        aXyzBlue()
    {}
};

struct DIBInfoHeader
{
    sal_uInt32      nSize;
    sal_Int32       nWidth;
    sal_Int32       nHeight;
    sal_uInt16      nPlanes;
    sal_uInt16      nBitCount;
    sal_uInt32      nCompression;
    sal_uInt32      nSizeImage;
    sal_Int32       nXPelsPerMeter;
    sal_Int32       nYPelsPerMeter;
    sal_uInt32      nColsUsed;
    sal_uInt32      nColsImportant;

    DIBInfoHeader()
    :   nSize(0),
        nWidth(0),
        nHeight(0),
        nPlanes(0),
        nBitCount(0),
        nCompression(0),
        nSizeImage(0),
        nXPelsPerMeter(0),
        nYPelsPerMeter(0),
        nColsUsed(0),
        nColsImportant(0)
    {}
};

struct DIBV5Header : public DIBInfoHeader
{
    sal_uInt32      nV5RedMask;
    sal_uInt32      nV5GreenMask;
    sal_uInt32      nV5BlueMask;
    sal_uInt32      nV5AlphaMask;
    sal_uInt32      nV5CSType;
    CIEXYZTriple    aV5Endpoints;
    sal_uInt32      nV5GammaRed;
    sal_uInt32      nV5GammaGreen;
    sal_uInt32      nV5GammaBlue;
    sal_uInt32      nV5Intent;
    sal_uInt32      nV5ProfileData;
    sal_uInt32      nV5ProfileSize;
    sal_uInt32      nV5Reserved;

    DIBV5Header()
    :   DIBInfoHeader(),
        nV5RedMask(0),
        nV5GreenMask(0),
        nV5BlueMask(0),
        nV5AlphaMask(0),
        nV5CSType(0),
        aV5Endpoints(),
        nV5GammaRed(0),
        nV5GammaGreen(0),
        nV5GammaBlue(0),
        nV5Intent(0),
        nV5ProfileData(0),
        nV5ProfileSize(0),
        nV5Reserved(0)
    {}

    DIBV5Header& operator=(const DIBV5Header&) = default;
};

// - Compression defines

#define COMPRESS_NONE               ( 0 )
#define RLE_8                       ( 1 )
#define RLE_4                       ( 2 )
#define BITFIELDS                   ( 3UL )
#define ZCOMPRESS                   ( ('S'|('D'<<8UL)) | 0x01000000UL ) /* == 'SD01' (binary) */

bool VCL_DLLPUBLIC ReadDIB( // ReadDIB(rBitmap, rIStm, true);
    Bitmap& rTarget,
    SvStream& rIStm,
    bool bFileHeader,
    bool bMSOFormat=false);

bool VCL_DLLPUBLIC ReadDIBBitmapEx(
    BitmapEx& rTarget,
    SvStream& rIStm,
    bool bFileHeader = true,
    bool bMSOFormat = false);

bool VCL_DLLPUBLIC ReadDIBV5(
    Bitmap& rTarget,
    AlphaMask& rTargetAlpha,
    SvStream& rIStm,
    DIBV5Header* pHeader=nullptr);

bool VCL_DLLPUBLIC ReadRawDIB(
    BitmapEx& rTarget,
    const unsigned char* pBuf,
    const ScanlineFormat nFormat,
    const int nHeight,
    const int nStride);


bool VCL_DLLPUBLIC WriteDIB(
    const Bitmap& rSource,
    SvStream& rOStm,
    bool bCompressed,
    bool bFileHeader);

// compressed, with file header
bool VCL_DLLPUBLIC WriteDIB(
    const BitmapEx& rSource,
    SvStream& rOStm,
    bool bCompressed = true);

bool VCL_DLLPUBLIC WriteDIBBitmapEx(
    const BitmapEx& rSource,
    SvStream& rOStm);

// needed in emfio for emf/wmf migration
sal_uInt32 VCL_DLLPUBLIC getDIBV5HeaderSize();

#endif // INCLUDED_VCL_DIBTOOLS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
