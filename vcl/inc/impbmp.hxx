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

#ifndef INCLUDED_VCL_INC_IMPBMP_HXX
#define INCLUDED_VCL_INC_IMPBMP_HXX

#include <memory>
#include <vcl/salbtype.hxx>

extern const sal_uLong nVCLRLut[ 6 ];
extern const sal_uLong nVCLGLut[ 6 ];
extern const sal_uLong nVCLBLut[ 6 ];
extern const sal_uLong nVCLDitherLut[ 256 ];
extern const sal_uLong nVCLLut[ 256 ];

struct BitmapBuffer;
class SalBitmap;
class BitmapPalette;
class SalGraphics;
class Bitmap;

class ImpBitmap
{
private:
    std::unique_ptr<SalBitmap>
                        mpSalBitmap;

public:
                        ImpBitmap();
                        /**
                         * takes ownership
                         */
                        ImpBitmap(SalBitmap* pBitmap);
                        ~ImpBitmap();

    bool                ImplIsEqual(const ImpBitmap& rBmp) const;

    SalBitmap*          ImplGetSalBitmap() const { return mpSalBitmap.get(); }

    bool                ImplCreate( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal );
    bool                ImplCreate( const ImpBitmap& rImpBitmap );
    bool                ImplCreate( const ImpBitmap& rImpBitmap, SalGraphics* pGraphics );
    bool                ImplCreate( const ImpBitmap& rImpBitmap, sal_uInt16 nNewBitCount );

    Size                ImplGetSize() const;
    sal_uInt16          ImplGetBitCount() const;

    BitmapBuffer*       ImplAcquireBuffer( BitmapAccessMode nMode );
    void                ImplReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode );

    BitmapChecksum      ImplGetChecksum() const;
    void                ImplInvalidateChecksum();

    bool                ImplScalingSupported() const;
    bool                ImplScale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag );
    bool                ImplReplace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol );
    bool                ImplConvert( BmpConversion eConversion );
};

#endif // INCLUDED_VCL_INC_IMPBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
