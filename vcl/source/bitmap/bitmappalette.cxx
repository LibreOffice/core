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

#include <config_features.h>

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/helpers.hxx>

#include <vcl/BitmapPalette.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/outdev.hxx>

#include <svdata.hxx>
#include <salinst.hxx>

class ImplBitmapPalette
{
public:
    ImplBitmapPalette(std::initializer_list<BitmapColor> aBitmapColor)
        : maBitmapColor(aBitmapColor)
    {
    }
    ImplBitmapPalette(const BitmapColor* first, const BitmapColor* last)
        : maBitmapColor(first, last)
    {
    }
    ImplBitmapPalette() {}
    ImplBitmapPalette(sal_uInt16 nCount)
        : maBitmapColor(nCount)
    {
    }
    std::vector<BitmapColor>& GetBitmapData() { return maBitmapColor; }
    const std::vector<BitmapColor>& GetBitmapData() const { return maBitmapColor; }
    bool operator==(const ImplBitmapPalette& rBitmapPalette) const
    {
        return maBitmapColor == rBitmapPalette.maBitmapColor;
    }

private:
    std::vector<BitmapColor> maBitmapColor;
};

namespace
{
BitmapPalette::ImplType& GetGlobalDefault()
{
    static BitmapPalette::ImplType gDefault;
    return gDefault;
}
}

BitmapPalette::BitmapPalette()
    : mpImpl(GetGlobalDefault())
{
}

BitmapPalette::BitmapPalette(const BitmapPalette& rOther)
    : mpImpl(rOther.mpImpl)
{
}

BitmapPalette::BitmapPalette(BitmapPalette&& rOther) noexcept
    : mpImpl(std::move(rOther.mpImpl))
{
}

BitmapPalette::BitmapPalette(std::initializer_list<BitmapColor> aBitmapColor)
    : mpImpl(aBitmapColor)
{
}

BitmapPalette::BitmapPalette(const BitmapColor* first, const BitmapColor* last)
    : mpImpl({ first, last })
{
}

BitmapPalette::BitmapPalette(sal_uInt16 nCount)
    : mpImpl(nCount)
{
}

BitmapPalette::~BitmapPalette() {}

BitmapPalette& BitmapPalette::operator=(const BitmapPalette& rOther)
{
    mpImpl = rOther.mpImpl;
    return *this;
}

BitmapPalette& BitmapPalette::operator=(BitmapPalette&& rOther) noexcept
{
    mpImpl = std::move(rOther.mpImpl);
    return *this;
}

const BitmapColor* BitmapPalette::ImplGetColorBuffer() const
{
    return mpImpl->GetBitmapData().data();
}

BitmapColor* BitmapPalette::ImplGetColorBuffer() { return mpImpl->GetBitmapData().data(); }

BitmapChecksum BitmapPalette::GetChecksum() const
{
    auto const& rBitmapData = mpImpl->GetBitmapData();
    return vcl_get_checksum(0, rBitmapData.data(), rBitmapData.size() * sizeof(BitmapColor));
}

bool BitmapPalette::operator==(const BitmapPalette& rOther) const
{
    return mpImpl == rOther.mpImpl;
}

bool BitmapPalette::operator!() const { return mpImpl->GetBitmapData().empty(); }

sal_uInt16 BitmapPalette::GetEntryCount() const { return mpImpl->GetBitmapData().size(); }

void BitmapPalette::SetEntryCount(sal_uInt16 nCount) { mpImpl->GetBitmapData().resize(nCount); }

const BitmapColor& BitmapPalette::operator[](sal_uInt16 nIndex) const
{
    assert(nIndex < mpImpl->GetBitmapData().size() && "Palette index is out of range");
    return mpImpl->GetBitmapData()[nIndex];
}

BitmapColor& BitmapPalette::operator[](sal_uInt16 nIndex)
{
    assert(nIndex < mpImpl->GetBitmapData().size() && "Palette index is out of range");
    return mpImpl->GetBitmapData()[nIndex];
}

sal_uInt16 BitmapPalette::GetBestIndex(const BitmapColor& rCol) const
{
    auto const& rBitmapColor = mpImpl->GetBitmapData();
    sal_uInt16 nRetIndex = 0;

    if (!rBitmapColor.empty())
    {
        for (size_t j = 0; j < rBitmapColor.size(); ++j)
        {
            if (rCol == rBitmapColor[j])
            {
                return j;
            }
        }

        sal_uInt16 nLastErr = SAL_MAX_UINT16;
        for (size_t i = 0; i < rBitmapColor.size(); ++i)
        {
            const sal_uInt16 nActErr = rCol.GetColorError(rBitmapColor[i]);
            if (nActErr < nLastErr)
            {
                nLastErr = nActErr;
                nRetIndex = i;
            }
        }
    }

    return nRetIndex;
}

bool BitmapPalette::IsGreyPaletteAny() const
{
    auto const& rBitmapColor = mpImpl->GetBitmapData();
    const int nEntryCount = GetEntryCount();
    if (!nEntryCount) // NOTE: an empty palette means 1:1 mapping
        return true;
    // See above: only certain entry values will result in a valid call to GetGreyPalette
    if (nEntryCount == 2 || nEntryCount == 4 || nEntryCount == 16 || nEntryCount == 256)
    {
        const BitmapPalette& rGreyPalette = Bitmap::GetGreyPalette(nEntryCount);
        if (rGreyPalette == *this)
            return true;
    }

    bool bRet = false;
    // TODO: is it worth to compare the entries for the general case?
    if (nEntryCount == 2)
    {
        const BitmapColor& rCol0(rBitmapColor[0]);
        const BitmapColor& rCol1(rBitmapColor[1]);
        bRet = rCol0.GetRed() == rCol0.GetGreen() && rCol0.GetRed() == rCol0.GetBlue()
               && rCol1.GetRed() == rCol1.GetGreen() && rCol1.GetRed() == rCol1.GetBlue();
    }
    return bRet;
}

bool BitmapPalette::IsGreyPalette8Bit() const
{
    auto const& rBitmapColor = mpImpl->GetBitmapData();
    const int nEntryCount = GetEntryCount();
    if (!nEntryCount) // NOTE: an empty palette means 1:1 mapping
        return true;
    if (nEntryCount != 256)
        return false;
    for (sal_uInt16 i = 0; i < 256; ++i)
    {
        if (rBitmapColor[i] != BitmapColor(i, i, i))
            return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
