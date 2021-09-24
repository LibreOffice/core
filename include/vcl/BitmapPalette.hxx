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
#include <vector>
#include <o3tl/cow_wrapper.hxx>

class ImplBitmapPalette;

class VCL_DLLPUBLIC BitmapPalette
{
    friend class SalBitmap;
    friend class BitmapAccess;

public:

    SAL_DLLPRIVATE const BitmapColor* ImplGetColorBuffer() const;

    SAL_DLLPRIVATE BitmapColor* ImplGetColorBuffer();

    BitmapChecksum GetChecksum() const;

    BitmapPalette();
    BitmapPalette( const BitmapPalette& );
    BitmapPalette( BitmapPalette&& ) noexcept;
    BitmapPalette(std::initializer_list<BitmapColor> aBitmapColor);
    explicit BitmapPalette(sal_uInt16 nCount);
    ~BitmapPalette();

    BitmapPalette& operator=( const BitmapPalette& );
    BitmapPalette& operator=( BitmapPalette&& ) noexcept;

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

    sal_uInt16 GetBestIndex(const BitmapColor& rCol) const;

    /// Returns true if the palette is 8-bit grey palette.
    bool IsGreyPalette8Bit() const;
    /// Returns true if the palette is a grey palette (may not be 8-bit).
    bool IsGreyPaletteAny() const;

    typedef o3tl::cow_wrapper< ImplBitmapPalette > ImplType;

private:
friend class ::std::optional<BitmapPalette>;
friend class ::o3tl::cow_optional<BitmapPalette>;

    BitmapPalette(std::nullopt_t) noexcept;
    BitmapPalette(const BitmapPalette &, std::nullopt_t) noexcept;

    ImplType mpImpl;
};

namespace std
{
    /** Specialise std::optional template for the case where we are wrapping a o3tl::cow_wrapper
        type, and we can make the pointer inside the cow_wrapper act as an empty value,
        and save ourselves some storage */
    template<>
    class VCL_DLLPUBLIC optional<BitmapPalette> final : public o3tl::cow_optional<BitmapPalette>
    {
    public:
        using cow_optional::cow_optional; // inherit constructors
        optional(const optional&) = default;
        optional(optional&&) = default;
        optional& operator=(const optional&) = default;
        optional& operator=(optional&&) = default;
        ~optional();
        void reset();
    };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
