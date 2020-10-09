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

#ifndef INCLUDED_VCL_INC_BMPFAST_HXX
#define INCLUDED_VCL_INC_BMPFAST_HXX

#include <vcl/dllapi.h>
#include <vcl/Scanline.hxx>

class BitmapWriteAccess;
class BitmapReadAccess;
struct BitmapBuffer;
class BitmapColor;
struct SalTwoRect;

// the bmpfast functions have signatures with good compatibility to
// their canonic counterparts, which employ the GetPixel/SetPixel methods

VCL_DLLPUBLIC bool ImplFastBitmapConversion( BitmapBuffer& rDst, const BitmapBuffer& rSrc,
        const SalTwoRect& rTwoRect );

bool ImplFastCopyScanline( long nY, BitmapBuffer& rDst, const BitmapBuffer& rSrc);
bool ImplFastCopyScanline( long nY, BitmapBuffer& rDst, ConstScanline aSrcScanline,
    ScanlineFormat nSrcScanlineFormat, sal_uInt32 nSrcScanlineSize);

bool ImplFastBitmapBlending( BitmapWriteAccess const & rDst,
    const BitmapReadAccess& rSrc, const BitmapReadAccess& rMask,
    const SalTwoRect& rTwoRect );

bool ImplFastEraseBitmap( BitmapBuffer&, const BitmapColor& );

#endif // INCLUDED_VCL_INC_BMPFAST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
