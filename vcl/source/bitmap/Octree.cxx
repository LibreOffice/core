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

#include <vcl/BitmapReadAccess.hxx>

#include <bitmap/Octree.hxx>

namespace
{
constexpr size_t OCTREE_BITS = 5;
constexpr size_t OCTREE_BITS_1 = 10;

constexpr sal_uLong gnBits = 8 - OCTREE_BITS;

} // end anonymous namespace

Octree::Octree(const BitmapReadAccess& rReadAcc, sal_uLong nColors)
    : mnLeafCount(0)
    , mnLevel(0)
    , mpReduce(OCTREE_BITS + 1, nullptr)
    , mpColor(nullptr)
    , mnPalIndex(0)
{
    const BitmapReadAccess* pAccess = &rReadAcc;
    sal_uLong nMax(nColors);

    if (!*pAccess)
        return;

    const tools::Long nWidth = pAccess->Width();
    const tools::Long nHeight = pAccess->Height();

    if (pAccess->HasPalette())
    {
        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pAccess->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
            {
                mpColor = &pAccess->GetPaletteColor(pAccess->GetIndexFromData(pScanline, nX));
                mnLevel = 0;
                add(pTree);

                while (mnLeafCount > nMax)
                    reduce();
            }
        }
    }
    else
    {
        BitmapColor aColor;

        mpColor = &aColor;

        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanline = pAccess->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
            {
                aColor = pAccess->GetPixelFromData(pScanline, nX);
                mnLevel = 0;
                add(pTree);

                while (mnLeafCount > nMax)
                    reduce();
            }
        }
    }
}

Octree::~Octree() {}

void Octree::add(std::unique_ptr<OctreeNode>& rpNode)
{
    // possibly generate new nodes
    if (!rpNode)
    {
        rpNode.reset(new OctreeNode);
        rpNode->bLeaf = (OCTREE_BITS == mnLevel);

        if (rpNode->bLeaf)
            mnLeafCount++;
        else
        {
            rpNode->pNext = mpReduce[mnLevel];
            mpReduce[mnLevel] = rpNode.get();
        }
    }

    if (rpNode->bLeaf)
    {
        rpNode->nCount++;
        rpNode->nRed += mpColor->GetRed();
        rpNode->nGreen += mpColor->GetGreen();
        rpNode->nBlue += mpColor->GetBlue();
    }
    else
    {
        const sal_uLong nShift = 7 - mnLevel;
        const sal_uInt8 cMask = 0x80 >> mnLevel;
        const sal_uLong nIndex = (((mpColor->GetRed() & cMask) >> nShift) << 2)
                                 | (((mpColor->GetGreen() & cMask) >> nShift) << 1)
                                 | ((mpColor->GetBlue() & cMask) >> nShift);

        mnLevel++;
        add(rpNode->pChild[nIndex]);
    }
}

void Octree::reduce()
{
    OctreeNode* pNode;
    sal_uLong nRedSum = 0;
    sal_uLong nGreenSum = 0;
    sal_uLong nBlueSum = 0;
    sal_uLong nChildren = 0;

    sal_uLong nIndex = OCTREE_BITS - 1;
    while (nIndex > 0 && !mpReduce[nIndex])
    {
        nIndex--;
    }

    pNode = mpReduce[nIndex];
    mpReduce[nIndex] = pNode->pNext;

    for (unsigned int i = 0; i < 8; i++)
    {
        if (pNode->pChild[i])
        {
            OctreeNode* pChild = pNode->pChild[i].get();

            nRedSum += pChild->nRed;
            nGreenSum += pChild->nGreen;
            nBlueSum += pChild->nBlue;
            pNode->nCount += pChild->nCount;

            pNode->pChild[i].reset();
            nChildren++;
        }
    }

    pNode->bLeaf = true;
    pNode->nRed = nRedSum;
    pNode->nGreen = nGreenSum;
    pNode->nBlue = nBlueSum;
    mnLeafCount -= --nChildren;
}

void Octree::CreatePalette(OctreeNode* pNode)
{
    if (pNode->bLeaf)
    {
        pNode->nPalIndex = mnPalIndex;
        maPalette[mnPalIndex++] = BitmapColor(sal_uInt8(double(pNode->nRed) / pNode->nCount),
                                              sal_uInt8(double(pNode->nGreen) / pNode->nCount),
                                              sal_uInt8(double(pNode->nBlue) / pNode->nCount));
    }
    else
    {
        for (auto const& i : pNode->pChild)
        {
            if (i)
            {
                CreatePalette(i.get());
            }
        }
    }
}

void Octree::GetPalIndex(const OctreeNode* pNode)
{
    if (pNode->bLeaf)
        mnPalIndex = pNode->nPalIndex;
    else
    {
        const sal_uLong nShift = 7 - mnLevel;
        const sal_uInt8 cMask = 0x80 >> mnLevel;
        mnLevel++;
        const sal_uLong nIndex = (((mpColor->GetRed() & cMask) >> nShift) << 2)
                                 | (((mpColor->GetGreen() & cMask) >> nShift) << 1)
                                 | ((mpColor->GetBlue() & cMask) >> nShift);

        GetPalIndex(pNode->pChild[nIndex].get());
    }
}

const BitmapPalette& Octree::GetPalette()
{
    maPalette.SetEntryCount(sal_uInt16(mnLeafCount));
    mnPalIndex = 0;
    CreatePalette(pTree.get());
    return maPalette;
}

sal_uInt16 Octree::GetBestPaletteIndex(const BitmapColor& rColor)
{
    mpColor = &rColor;
    mnPalIndex = 65535;
    mnLevel = 0;
    GetPalIndex(pTree.get());
    return mnPalIndex;
}

constexpr int nColorMax = 1 << OCTREE_BITS;

InverseColorMap::InverseColorMap(const BitmapPalette& rPal)
{
    const unsigned long xsqr = 1 << (gnBits << 1);
    const unsigned long xsqr2 = xsqr << 1;
    const int nColors = rPal.GetEntryCount();
    const tools::Long x = 1 << gnBits;
    const tools::Long x2 = x >> 1;
    sal_uLong r, g, b;
    tools::Long rxx, gxx, bxx;

    ImplCreateBuffers();

    for (int nIndex = 0; nIndex < nColors; nIndex++)
    {
        const BitmapColor& rColor = rPal[static_cast<sal_uInt16>(nIndex)];
        const tools::Long cRed = rColor.GetRed();
        const tools::Long cGreen = rColor.GetGreen();
        const tools::Long cBlue = rColor.GetBlue();

        tools::Long rdist = cRed - x2;
        tools::Long gdist = cGreen - x2;
        tools::Long bdist = cBlue - x2;
        rdist = rdist * rdist + gdist * gdist + bdist * bdist;

        const tools::Long crinc = (xsqr - (cRed << gnBits)) << 1;
        const tools::Long cginc = (xsqr - (cGreen << gnBits)) << 1;
        const tools::Long cbinc = (xsqr - (cBlue << gnBits)) << 1;

        sal_uLong* cdp = reinterpret_cast<sal_uLong*>(mpBuffer.data());
        sal_uInt8* crgbp = mpMap.data();

        for (r = 0, rxx = crinc; r < nColorMax; rdist += rxx, r++, rxx += xsqr2)
        {
            for (g = 0, gdist = rdist, gxx = cginc; g < nColorMax; gdist += gxx, g++, gxx += xsqr2)
            {
                for (b = 0, bdist = gdist, bxx = cbinc; b < nColorMax;
                     bdist += bxx, b++, cdp++, crgbp++, bxx += xsqr2)
                    if (!nIndex || static_cast<tools::Long>(*cdp) > bdist)
                    {
                        *cdp = bdist;
                        *crgbp = static_cast<sal_uInt8>(nIndex);
                    }
            }
        }
    }
}

InverseColorMap::~InverseColorMap() {}

void InverseColorMap::ImplCreateBuffers()
{
    const sal_uLong nCount = nColorMax * nColorMax * nColorMax;
    const sal_uLong nSize = nCount * sizeof(sal_uLong);

    mpMap.resize(nCount, 0x00);
    mpBuffer.resize(nSize, 0xff);
}

sal_uInt16 InverseColorMap::GetBestPaletteIndex(const BitmapColor& rColor)
{
    return mpMap[((static_cast<sal_uLong>(rColor.GetRed()) >> gnBits) << OCTREE_BITS_1)
                 | ((static_cast<sal_uLong>(rColor.GetGreen()) >> gnBits) << OCTREE_BITS)
                 | (static_cast<sal_uLong>(rColor.GetBlue()) >> gnBits)];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
