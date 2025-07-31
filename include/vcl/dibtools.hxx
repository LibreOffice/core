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

// predefines

class SvStream;
class BitmapEx;
class Bitmap;
class AlphaMask;

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

bool VCL_DLLPUBLIC ReadDIBBitmapEx(
    Bitmap& rTarget,
    SvStream& rIStm,
    bool bFileHeader = true,
    bool bMSOFormat = false);

bool VCL_DLLPUBLIC ReadDIBV5(
    Bitmap& rTarget,
    AlphaMask& rTargetAlpha,
    SvStream& rIStm);

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

bool VCL_DLLPUBLIC WriteDIBBitmapEx(
    const Bitmap& rSource,
    SvStream& rOStm);

// needed in emfio for emf/wmf migration
sal_uInt32 VCL_DLLPUBLIC getDIBV5HeaderSize();

#endif // INCLUDED_VCL_DIBTOOLS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
