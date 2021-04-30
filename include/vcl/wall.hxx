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

#ifndef INCLUDED_VCL_WALL_HXX
#define INCLUDED_VCL_WALL_HXX

#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/gradient.hxx>
#include <vcl/dllapi.h>
#include <optional>

class Gradient;
class BitmapEx;
class ImplWallpaper;

enum class WallpaperStyle
{
    NONE,
    Tile,
    Center,
    Scale,
    TopLeft,
    Top,
    TopRight,
    Left,
    Right,
    BottomLeft,
    Bottom,
    BottomRight,
    ApplicationGradient          // defines a gradient that internally covers the whole application
                                 // and uses a color derived from the face color
};

class VCL_DLLPUBLIC Wallpaper
{
public:
    SAL_DLLPRIVATE void             ImplSetCachedBitmap( BitmapEx& rBmp ) const;
    SAL_DLLPRIVATE const BitmapEx*  ImplGetCachedBitmap() const;
    SAL_DLLPRIVATE void             ImplReleaseCachedBitmap() const;

private:
    SAL_DLLPRIVATE static Gradient  ImplGetApplicationGradient();

public:
                    Wallpaper();
                    Wallpaper( const Wallpaper& rWallpaper );
                    Wallpaper( Wallpaper&& rWallpaper );
                    Wallpaper( const Color& rColor );
                    explicit Wallpaper( const BitmapEx& rBmpEx );
                    ~Wallpaper();

    void            SetColor( const Color& rColor );
    const Color&    GetColor() const { return maColor; }

    void            SetStyle( WallpaperStyle eStyle );
    WallpaperStyle  GetStyle() const { return meStyle; }

    void            SetBitmap( const BitmapEx& rBitmap );
    const BitmapEx & GetBitmap() const;
    bool            IsBitmap() const;

    void            SetGradient( const Gradient& rGradient );
    Gradient        GetGradient() const;
    bool            IsGradient() const;

    void            SetRect( const tools::Rectangle& rRect ) { maRect = rRect; }
    const tools::Rectangle & GetRect() const { return maRect; }
    bool            IsRect() const;

    bool            IsFixed() const;
    bool            IsScrollable() const;

    Wallpaper&      operator=( const Wallpaper& rWallpaper );
    Wallpaper&      operator=( Wallpaper&& rWallpaper );

    bool            operator==( const Wallpaper& rWallpaper ) const;
    bool            operator!=( const Wallpaper& rWallpaper ) const
                        { return !(Wallpaper::operator==( rWallpaper )); }

    bool            IsEmpty() const
    {
        return GetStyle() == WallpaperStyle::NONE && GetColor() == COL_TRANSPARENT &&
               !IsBitmap() && !IsGradient() && !IsRect();
    }

    friend SvStream& ReadWallpaper( SvStream& rIStm, Wallpaper& rWallpaper );
    friend SvStream& WriteWallpaper( SvStream& rOStm, const Wallpaper& rWallpaper );
private:
    tools::Rectangle            maRect;
    BitmapEx                    maBitmap;
    mutable BitmapEx            maCache;
    std::optional<Gradient>     mpGradient;
    Color                       maColor;
    WallpaperStyle              meStyle;
};

#endif // INCLUDED_VCL_WALL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
