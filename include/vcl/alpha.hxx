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

#ifndef INCLUDED_VCL_ALPHA_HXX
#define INCLUDED_VCL_ALPHA_HXX

#include <sal/config.h>

#include <utility>

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>
#include <vcl/region.hxx>

class VCL_DLLPUBLIC AlphaMask final
{
public:
                AlphaMask();
    explicit    AlphaMask( const Bitmap& rBitmap );
                AlphaMask( const AlphaMask& rAlphaMask );
                AlphaMask( AlphaMask&& rAlphaMask );
    explicit    AlphaMask( const Size& rSizePixel, const sal_uInt8* pEraseTransparency = nullptr );
                ~AlphaMask();

    AlphaMask&  operator=( const Bitmap& rBitmap );
    AlphaMask&  operator=( const AlphaMask& rAlphaMask ) { maBitmap = rAlphaMask.maBitmap; return *this; }
    AlphaMask&  operator=( AlphaMask&& rAlphaMask ) noexcept { maBitmap = std::move(rAlphaMask.maBitmap); return *this; }
    bool        operator==( const AlphaMask& rAlphaMask ) const { return maBitmap == rAlphaMask.maBitmap; }
    bool        operator!=( const AlphaMask& rAlphaMask ) const { return maBitmap != rAlphaMask.maBitmap; }

    Bitmap const & GetBitmap() const { return maBitmap; }

    void        Erase( sal_uInt8 cTransparency );
    void        BlendWith(const AlphaMask& rOther);

    /** Perform boolean OR operation with another alpha-mask

        @param rMask
        The mask bitmap in the selected combine operation

        @return true, if the operation was completed successfully.
     */
    bool        AlphaCombineOr( const AlphaMask& rMask );

    // check if alpha is used, returns true if at least one pixel has transparence
    bool        hasAlpha() const;

    bool IsEmpty() const { return maBitmap.IsEmpty(); }

    vcl::PixelFormat getPixelFormat() const { return maBitmap.getPixelFormat(); }

    Size GetSizePixel() const { return maBitmap.GetSizePixel(); }

    void SetPrefSize( const Size& rSize ) { maBitmap.SetPrefSize(rSize); }

    void SetPrefMapMode( const MapMode& rMapMode ) { maBitmap.SetPrefMapMode(rMapMode); }

    BitmapChecksum GetChecksum() const { return maBitmap.GetChecksum(); }

    bool Invert();

    bool Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag = BmpScaleFlag::Default ) { return maBitmap.Scale(rNewSize, nScaleFlag); }

    bool Convert( BmpConversion eConversion ) { return maBitmap.Convert(eConversion); }

    vcl::Region CreateRegion( const Color& rColor, const tools::Rectangle& rRect ) const { return maBitmap.CreateRegion(rColor, rRect); }

private:
    friend class ::OutputDevice;

    Bitmap maBitmap;
};

#endif // INCLUDED_VCL_ALPHA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
