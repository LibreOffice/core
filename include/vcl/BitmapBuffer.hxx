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

#ifndef INCLUDED_VCL_BITMAPBUFFER_HXX
#define INCLUDED_VCL_BITMAPBUFFER_HXX

#include <sal/config.h>

#include <memory>

#include <vcl/dllapi.h>
#include <vcl/BitmapPalette.hxx>
#include <vcl/ColorMask.hxx>
#include <vcl/Scanline.hxx>

#include <memory>

struct SalTwoRect;

struct VCL_DLLPUBLIC BitmapBuffer
{
    ScanlineFormat       mnFormat;
    long            mnWidth;
    long            mnHeight;
    long            mnScanlineSize;
    sal_uInt16      mnBitCount;
    ColorMask       maColorMask;
    BitmapPalette   maPalette;
    sal_uInt8*      mpBits;
};

VCL_DLLPUBLIC std::unique_ptr<BitmapBuffer> StretchAndConvert(
    const BitmapBuffer& rSrcBuffer, const SalTwoRect& rTwoRect,
    ScanlineFormat nDstBitmapFormat, const BitmapPalette* pDstPal = nullptr, const ColorMask* pDstMask = nullptr );

#endif // INCLUDED_VCL_BITMAPBUFFER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
