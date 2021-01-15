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

#ifndef INCLUDED_VCL_BITMAPCOLOR_HXX
#define INCLUDED_VCL_BITMAPCOLOR_HXX

#include <vcl/dllapi.h>
#include <tools/color.hxx>

class VCL_DLLPUBLIC BitmapColor final : public Color
{
public:
    inline              BitmapColor();
    constexpr           BitmapColor( sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue );
    constexpr           BitmapColor( ColorTransparencyTag, sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue, sal_uInt8 cTransparency );
    constexpr           BitmapColor( ColorAlphaTag, sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue, sal_uInt8 cAlpha );

    inline              BitmapColor( const Color& rColor );
    explicit inline     BitmapColor( sal_uInt8 cIndex );

    inline sal_uInt8    GetIndex() const;
    inline void         SetIndex( sal_uInt8 cIndex );
};

inline BitmapColor::BitmapColor()
{
}

inline BitmapColor::BitmapColor( const Color& rColor )
    : Color(rColor)
{
}

constexpr BitmapColor::BitmapColor(sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue)
    : Color(cRed, cGreen, cBlue)
{
}

constexpr BitmapColor::BitmapColor(ColorTransparencyTag, sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue, sal_uInt8 cTransparency)
    : Color(ColorTransparency, cTransparency, cRed, cGreen, cBlue)
{
}

constexpr BitmapColor::BitmapColor(ColorAlphaTag, sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue, sal_uInt8 cAlpha)
    : Color(ColorAlpha, cAlpha, cRed, cGreen, cBlue)
{
}

inline BitmapColor::BitmapColor( sal_uInt8 cIndex )
{
    SetIndex(cIndex);
}

inline sal_uInt8 BitmapColor::GetIndex() const
{
    return GetBlue();
}

inline void BitmapColor::SetIndex( sal_uInt8 cIndex )
{
    SetBlue(cIndex);
}

#endif // INCLUDED_VCL_BITMAPCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
