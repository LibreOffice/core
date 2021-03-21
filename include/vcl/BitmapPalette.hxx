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

#ifndef INCLUDED_VCL_BITMAPPALETTE_HXX
#define INCLUDED_VCL_BITMAPPALETTE_HXX

#include <vcl/dllapi.h>
#include <vcl/BitmapColor.hxx>
#include <vcl/checksum.hxx>
#include <vector>

class VCL_DLLPUBLIC BitmapPalette
{
    friend class SalBitmap;
    friend class BitmapAccess;

private:

    std::vector<BitmapColor> maBitmapColor;

public:

    SAL_DLLPRIVATE const BitmapColor* ImplGetColorBuffer() const
    {
        return maBitmapColor.data();
    }

    SAL_DLLPRIVATE BitmapColor* ImplGetColorBuffer()
    {
        return maBitmapColor.data();
    }

    BitmapChecksum GetChecksum() const
    {
        return vcl_get_checksum(0, maBitmapColor.data(), maBitmapColor.size() * sizeof(BitmapColor));
    }

public:

    BitmapPalette()
    {
    }

    BitmapPalette(std::initializer_list<BitmapColor> aBitmapColor) : maBitmapColor(aBitmapColor)
    {
    }

    BitmapPalette(sal_uInt16 nCount)
        : maBitmapColor(nCount)
    {
    }

    bool operator==( const BitmapPalette& rBitmapPalette ) const
    {
        return maBitmapColor == rBitmapPalette.maBitmapColor;
    }

    bool operator!=(const BitmapPalette& rBitmapPalette) const
    {
        return !( *this == rBitmapPalette );
    }

    bool operator!() const
    {
        return maBitmapColor.empty();
    }

    sal_uInt16 GetEntryCount() const
    {
        return maBitmapColor.size();
    }

    void SetEntryCount(sal_uInt16 nCount)
    {
        maBitmapColor.resize(nCount);
    }

    const BitmapColor& operator[](sal_uInt16 nIndex) const
    {
        assert(nIndex < maBitmapColor.size() && "Palette index is out of range");
        return maBitmapColor[nIndex];
    }

    BitmapColor& operator[](sal_uInt16 nIndex)
    {
        assert(nIndex < maBitmapColor.size() && "Palette index is out of range");
        return maBitmapColor[nIndex];
    }

    sal_uInt16 GetBestIndex(const BitmapColor& rCol) const
    {
        sal_uInt16 nRetIndex = 0;

        if (!maBitmapColor.empty())
        {
            for (size_t j = 0; j < maBitmapColor.size(); ++j)
            {
                if (rCol == maBitmapColor[j])
                {
                    return j;
                }
            }

            sal_uInt16 nLastErr = SAL_MAX_UINT16;
            for (size_t i = 0; i < maBitmapColor.size(); ++i)
            {
                const sal_uInt16 nActErr = rCol.GetColorError(maBitmapColor[i]);
                if ( nActErr < nLastErr )
                {
                    nLastErr = nActErr;
                    nRetIndex = i;
                }
            }
        }

        return nRetIndex;
    }

    /// Returns true if the palette is 8-bit grey palette.
    bool IsGreyPalette8Bit() const;
    /// Returns true if the palette is a grey palette (may not be 8-bit).
    bool IsGreyPaletteAny() const;
};

#endif // INCLUDED_VCL_BITMAPPALETTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
