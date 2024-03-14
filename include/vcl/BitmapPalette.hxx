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

#include <vcl/dllapi.h>
#include <vcl/BitmapColor.hxx>
#include <vcl/checksum.hxx>
#include <o3tl/cow_wrapper.hxx>

#include <array>

class ImplBitmapPalette;

class VCL_DLLPUBLIC BitmapPalette
{
    friend class SalBitmap;
    friend class BitmapAccess;

public:

    SAL_DLLPRIVATE const BitmapColor* ImplGetColorBuffer() const;

    SAL_DLLPRIVATE BitmapColor* ImplGetColorBuffer();

    SAL_DLLPRIVATE BitmapChecksum GetChecksum() const;

    BitmapPalette();
    BitmapPalette( const BitmapPalette& );
    SAL_DLLPRIVATE BitmapPalette( BitmapPalette&& ) noexcept;
    BitmapPalette(std::initializer_list<BitmapColor> aBitmapColor);
    template <size_t N> BitmapPalette(const std::array<BitmapColor, N>& colors);
    explicit BitmapPalette(sal_uInt16 nCount);
    ~BitmapPalette();

    BitmapPalette& operator=( const BitmapPalette& );
    SAL_DLLPRIVATE BitmapPalette& operator=( BitmapPalette&& ) noexcept;

    bool operator==( const BitmapPalette& rBitmapPalette ) const;
    bool operator!=(const BitmapPalette& rBitmapPalette) const
    {
        return !( *this == rBitmapPalette );
    }
    bool operator!() const;

    sal_uInt16 GetEntryCount() const;
    void SetEntryCount(sal_uInt16 nCount);

    const BitmapColor& operator[](sal_uInt16 nIndex) const;
    BitmapColor& operator[](sal_uInt16 nIndex);

    /// Returns the BitmapColor (i.e. palette index) that is either an exact match
    /// of the required color, or failing that, the entry that is the closest i.e. least error
    /// as measured by Color::GetColorError.
    sal_uInt16 GetBestIndex(const BitmapColor& rCol) const;
    /// Returns the BitmapColor (i.e. palette index) that is an exact match
    /// of the required color. Returns SAL_MAX_UINT16 if nothing found.
    SAL_DLLPRIVATE sal_uInt16 GetMatchingIndex(const BitmapColor& rCol) const;

    /// Returns true if the palette is 8-bit grey palette.
    bool IsGreyPalette8Bit() const;
    /// Returns true if the palette is a grey palette (may not be 8-bit).
    SAL_DLLPRIVATE bool IsGreyPaletteAny() const;

    typedef o3tl::cow_wrapper< ImplBitmapPalette > ImplType;

private:
    BitmapPalette(const BitmapColor* first, const BitmapColor* last);

    ImplType mpImpl;
};

template <size_t N>
BitmapPalette::BitmapPalette(const std::array<BitmapColor, N>& colors)
    : BitmapPalette(colors.data(), colors.data() + N)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
