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
#include <memory>

class VCL_DLLPUBLIC BitmapColor final
{
private:
    sal_uInt8 mcBlueOrIndex;
    sal_uInt8 mcGreen;
    sal_uInt8 mcRed;
    sal_uInt8 mcAlpha;

public:

    inline              BitmapColor();
    constexpr           BitmapColor( sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue, sal_uInt8 cAlpha = 0 );

    inline              BitmapColor( const Color& rColor );
    explicit inline     BitmapColor( sal_uInt8 cIndex );

    inline bool         operator==( const BitmapColor& rBitmapColor ) const;
    inline bool         operator!=( const BitmapColor& rBitmapColor ) const;

    inline sal_uInt8    GetRed() const;
    inline void         SetRed( sal_uInt8 cRed );

    inline sal_uInt8    GetGreen() const;
    inline void         SetGreen( sal_uInt8 cGreen );

    inline sal_uInt8    GetBlue() const;
    inline void         SetBlue( sal_uInt8 cBlue );

    inline sal_uInt8    GetIndex() const;
    inline void         SetIndex( sal_uInt8 cIndex );

    Color               GetColor() const;

    inline sal_uInt8    GetAlpha() const;
    inline void         SetAlpha( sal_uInt8 cAlpha );

    inline sal_uInt8    GetBlueOrIndex() const;

    inline BitmapColor& Invert();

    inline sal_uInt8    GetLuminance() const;

    inline BitmapColor& Merge( const BitmapColor& rColor, sal_uInt8 cTransparency );

    inline sal_uInt16   GetColorError( const BitmapColor& rBitmapColor ) const;
};

template<typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator <<(std::basic_ostream<charT, traits>& rStream, const BitmapColor& rColor)
{
    return rStream << "#(" << std::hex << std::setfill ('0') << std::setw(2) << static_cast<int>(rColor.GetRed())
                           << std::setw(2) << static_cast<int>(rColor.GetGreen())
                           << std::setw(2) << static_cast<int>(rColor.GetBlueOrIndex())
                           << std::setw(2) << static_cast<int>(rColor.GetAlpha()) << ")";
}

inline BitmapColor::BitmapColor() :
            mcBlueOrIndex   (0),
            mcGreen         (0),
            mcRed           (0),
            mcAlpha         (0)
{
}

constexpr BitmapColor::BitmapColor(sal_uInt8 cRed, sal_uInt8 cGreen, sal_uInt8 cBlue, sal_uInt8 cAlpha) :
            mcBlueOrIndex   ( cBlue ),
            mcGreen         ( cGreen ),
            mcRed           ( cRed ),
            mcAlpha         ( cAlpha )
{
}

inline BitmapColor::BitmapColor( const Color& rColor ) :
            mcBlueOrIndex   ( rColor.GetBlue() ),
            mcGreen         ( rColor.GetGreen() ),
            mcRed           ( rColor.GetRed() ),
            mcAlpha         ( rColor.GetTransparency() )
{
}

inline BitmapColor::BitmapColor( sal_uInt8 cIndex ) :
            mcBlueOrIndex   ( cIndex ),
            mcGreen         ( 0 ),
            mcRed           ( 0 ),
            mcAlpha         ( 0 )
{
}

inline bool BitmapColor::operator==( const BitmapColor& rBitmapColor ) const
{
    return mcBlueOrIndex == rBitmapColor.mcBlueOrIndex &&
           mcGreen == rBitmapColor.mcGreen &&
           mcRed == rBitmapColor.mcRed &&
           mcAlpha == rBitmapColor.mcAlpha;
}

inline bool BitmapColor::operator!=( const BitmapColor& rBitmapColor ) const
{
    return !( *this == rBitmapColor );
}

inline sal_uInt8 BitmapColor::GetRed() const
{
    return mcRed;
}

inline void BitmapColor::SetRed( sal_uInt8 cRed )
{
    mcRed = cRed;
}

inline sal_uInt8 BitmapColor::GetGreen() const
{
    return mcGreen;
}

inline void BitmapColor::SetGreen( sal_uInt8 cGreen )
{
    mcGreen = cGreen;
}

inline sal_uInt8 BitmapColor::GetBlue() const
{
    return mcBlueOrIndex;
}

inline void BitmapColor::SetBlue( sal_uInt8 cBlue )
{
    mcBlueOrIndex = cBlue;
}

inline sal_uInt8 BitmapColor::GetIndex() const
{
    return mcBlueOrIndex;
}

inline void BitmapColor::SetIndex( sal_uInt8 cIndex )
{
    mcBlueOrIndex = cIndex;
}

inline Color BitmapColor::GetColor() const
{
    return Color(mcAlpha, mcRed, mcGreen, mcBlueOrIndex);
}

inline sal_uInt8 BitmapColor::GetAlpha() const
{
    return mcAlpha;
}

inline void BitmapColor::SetAlpha( sal_uInt8 cAlpha )
{
    mcAlpha = cAlpha;
}

inline sal_uInt8 BitmapColor::GetBlueOrIndex() const
{
    // #i47518# Yield a value regardless of mbIndex
    return mcBlueOrIndex;
}

inline BitmapColor& BitmapColor::Invert()
{
    mcBlueOrIndex = ~mcBlueOrIndex;
    mcGreen = ~mcGreen;
    mcRed = ~mcRed;

    return *this;
}

inline sal_uInt8 BitmapColor::GetLuminance() const
{
    return (static_cast<sal_uInt32>(mcBlueOrIndex) * 28
            + static_cast<sal_uInt32>(mcGreen) * 151
            + static_cast<sal_uInt32>(mcRed) * 77) >> 8;
}


inline BitmapColor& BitmapColor::Merge( const BitmapColor& rBitmapColor, sal_uInt8 cTransparency )
{
    mcBlueOrIndex = ColorChannelMerge( mcBlueOrIndex, rBitmapColor.mcBlueOrIndex, cTransparency );
    mcGreen = ColorChannelMerge( mcGreen, rBitmapColor.mcGreen, cTransparency );
    mcRed = ColorChannelMerge( mcRed, rBitmapColor.mcRed, cTransparency );

    return *this;
}


inline sal_uInt16 BitmapColor::GetColorError( const BitmapColor& rBitmapColor ) const
{
    return static_cast<sal_uInt16>(
        abs( static_cast<int>(mcBlueOrIndex) - static_cast<int>(rBitmapColor.mcBlueOrIndex) ) +
        abs( static_cast<int>(mcGreen) - static_cast<int>(rBitmapColor.mcGreen) ) +
        abs( static_cast<int>(mcRed) - static_cast<int>(rBitmapColor.mcRed) ) );
}

#endif // INCLUDED_VCL_BITMAPCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
