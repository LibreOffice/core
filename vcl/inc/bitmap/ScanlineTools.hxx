/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_BITMAP_SCANLINETOOLS_HXX
#define INCLUDED_VCL_INC_BITMAP_SCANLINETOOLS_HXX

#include <tools/color.hxx>
#include <vcl/BitmapPalette.hxx>

namespace vcl::bitmap
{
class ScanlineTransformer
{
public:
    virtual void startLine(sal_uInt8* pLine) = 0;
    virtual void skipPixel(sal_uInt32 nPixel) = 0;
    virtual Color readPixel() = 0;
    virtual void writePixel(Color nColor) = 0;

    virtual ~ScanlineTransformer() = default;
};

class ScanlineTransformer_ARGB final : public ScanlineTransformer
{
private:
    sal_uInt8* pData;

public:
    virtual void startLine(sal_uInt8* pLine) override { pData = pLine; }

    virtual void skipPixel(sal_uInt32 nPixel) override { pData += nPixel << 2; }

    virtual Color readPixel() override
    {
        const Color aColor(ColorAlpha, pData[4], pData[1], pData[2], pData[3]);
        pData += 4;
        return aColor;
    }

    virtual void writePixel(Color nColor) override
    {
        *pData++ = nColor.GetAlpha();
        *pData++ = nColor.GetRed();
        *pData++ = nColor.GetGreen();
        *pData++ = nColor.GetBlue();
    }
};

class ScanlineTransformer_BGR final : public ScanlineTransformer
{
private:
    sal_uInt8* pData;

public:
    virtual void startLine(sal_uInt8* pLine) override { pData = pLine; }

    virtual void skipPixel(sal_uInt32 nPixel) override { pData += (nPixel << 1) + nPixel; }

    virtual Color readPixel() override
    {
        const Color aColor(pData[2], pData[1], pData[0]);
        pData += 3;
        return aColor;
    }

    virtual void writePixel(Color nColor) override
    {
        *pData++ = nColor.GetBlue();
        *pData++ = nColor.GetGreen();
        *pData++ = nColor.GetRed();
    }
};

class ScanlineTransformer_8BitPalette final : public ScanlineTransformer
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;

public:
    explicit ScanlineTransformer_8BitPalette(const BitmapPalette& rPalette)
        : pData(nullptr)
        , mrPalette(rPalette)
    {
    }

    virtual void startLine(sal_uInt8* pLine) override { pData = pLine; }

    virtual void skipPixel(sal_uInt32 nPixel) override { pData += nPixel; }

    virtual Color readPixel() override
    {
        const sal_uInt8 nIndex(*pData++);
        if (nIndex < mrPalette.GetEntryCount())
            return mrPalette[nIndex];
        else
            return COL_BLACK;
    }

    virtual void writePixel(Color nColor) override
    {
        *pData++ = static_cast<sal_uInt8>(mrPalette.GetBestIndex(nColor));
    }
};

class ScanlineTransformer_4BitPalette final : public ScanlineTransformer
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;
    sal_uInt32 mnShift;

public:
    explicit ScanlineTransformer_4BitPalette(const BitmapPalette& rPalette)
        : pData(nullptr)
        , mrPalette(rPalette)
        , mnX(0)
        , mnShift(0)
    {
    }

    virtual void skipPixel(sal_uInt32 nPixel) override
    {
        mnX += nPixel;
        if (nPixel & 1) // is nPixel an odd number
            mnShift ^= 4;
    }

    virtual void startLine(sal_uInt8* pLine) override
    {
        pData = pLine;
        mnX = 0;
        mnShift = 4;
    }

    virtual Color readPixel() override
    {
        const sal_uInt32 nDataIndex = mnX / 2;
        const sal_uInt8 nIndex((pData[nDataIndex] >> mnShift) & 0x0f);
        mnX++;
        mnShift ^= 4;

        if (nIndex < mrPalette.GetEntryCount())
            return mrPalette[nIndex];
        else
            return COL_BLACK;
    }

    virtual void writePixel(Color nColor) override
    {
        const sal_uInt32 nDataIndex = mnX / 2;
        const sal_uInt8 nColorIndex = mrPalette.GetBestIndex(nColor);
        pData[nDataIndex] |= (nColorIndex & 0x0f) << mnShift;
        mnX++;
        mnShift ^= 4;
    }
};

class ScanlineTransformer_1BitPalette final : public ScanlineTransformer
{
private:
    sal_uInt8* pData;
    const BitmapPalette& mrPalette;
    sal_uInt32 mnX;

public:
    explicit ScanlineTransformer_1BitPalette(const BitmapPalette& rPalette)
        : pData(nullptr)
        , mrPalette(rPalette)
        , mnX(0)
    {
    }

    virtual void skipPixel(sal_uInt32 nPixel) override { mnX += nPixel; }

    virtual void startLine(sal_uInt8* pLine) override
    {
        pData = pLine;
        mnX = 0;
    }

    virtual Color readPixel() override
    {
        const sal_uInt8 nIndex((pData[mnX >> 3] >> (7 - (mnX & 7))) & 1);
        mnX++;

        if (nIndex < mrPalette.GetEntryCount())
            return mrPalette[nIndex];
        else
            return COL_BLACK;
    }

    virtual void writePixel(Color nColor) override
    {
        if (mrPalette.GetBestIndex(nColor) & 1)
            pData[mnX >> 3] |= 1 << (7 - (mnX & 7));
        else
            pData[mnX >> 3] &= ~(1 << (7 - (mnX & 7)));
        mnX++;
    }
};

std::unique_ptr<ScanlineTransformer> getScanlineTransformer(sal_uInt16 nBits,
                                                            const BitmapPalette& rPalette)
{
    switch (nBits)
    {
        case 1:
            return std::make_unique<ScanlineTransformer_1BitPalette>(rPalette);
        case 4:
            return std::make_unique<ScanlineTransformer_4BitPalette>(rPalette);
        case 8:
            return std::make_unique<ScanlineTransformer_8BitPalette>(rPalette);
        case 24:
            return std::make_unique<ScanlineTransformer_BGR>();
        case 32:
            return std::make_unique<ScanlineTransformer_ARGB>();
        default:
            assert(false);
            break;
    }
    return nullptr;
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
