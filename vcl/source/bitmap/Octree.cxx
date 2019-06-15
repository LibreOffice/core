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

#include <vcl/bitmapaccess.hxx>

#include <bitmap/Octree.hxx>

namespace
{
constexpr size_t OCTREE_BITS = 5;
constexpr size_t OCTREE_BITS_1 = 10;

constexpr sal_uLong gnBits = 8 - OCTREE_BITS;

} // end anonymous namespace

class ImpNodeCache
{
private:
    OctreeNode* mpActNode;

public:
    ImpNodeCache(const sal_uLong nInitSize)
        : mpActNode(nullptr)
    {
        const sal_uLong nSize = nInitSize + 4;

        for (sal_uLong i = 0; i < nSize; i++)
        {
            OctreeNode* pNewNode = new OctreeNode;

            pNewNode->pNextInCache = mpActNode;
            mpActNode = pNewNode;
        }
    }

    ~ImpNodeCache()
    {
        while (mpActNode)
        {
            OctreeNode* pNode = mpActNode;

            mpActNode = pNode->pNextInCache;
            delete pNode;
        }
    }

    OctreeNode* ImplGetFreeNode()
    {
        OctreeNode* pNode;

        if (!mpActNode)
        {
            mpActNode = new OctreeNode;
            mpActNode->pNextInCache = nullptr;
        }

        pNode = mpActNode;
        mpActNode = pNode->pNextInCache;
        memset(pNode, 0, sizeof(OctreeNode));

        return pNode;
    }
    void ImplReleaseNode(OctreeNode* pNode)
    {
        pNode->pNextInCache = mpActNode;
        mpActNode = pNode;
    }
};

Octree::Octree(const BitmapReadAccess& rReadAcc, sal_uLong nColors)
    : mnLeafCount(0)
    , mnLevel(0)
    , pTree(nullptr)
    , mpReduce(OCTREE_BITS + 1, nullptr)
    , mpColor(nullptr)
    , mpNodeCache(std::make_unique<ImpNodeCache>(nColors))
    , mpAccess(&rReadAcc)
    , mnPalIndex(0)
{
    sal_uLong nMax(nColors);

    if (!!*mpAccess)
    {
        const long nWidth = mpAccess->Width();
        const long nHeight = mpAccess->Height();

        if (mpAccess->HasPalette())
        {
            for (long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = mpAccess->GetScanline(nY);
                for (long nX = 0; nX < nWidth; nX++)
                {
                    mpColor = &mpAccess->GetPaletteColor(mpAccess->GetIndexFromData(pScanline, nX));
                    mnLevel = 0;
                    add(&pTree);

                    while (mnLeafCount > nMax)
                        reduce();
                }
            }
        }
        else
        {
            BitmapColor aColor;

            mpColor = &aColor;

            for (long nY = 0; nY < nHeight; nY++)
            {
                Scanline pScanline = mpAccess->GetScanline(nY);
                for (long nX = 0; nX < nWidth; nX++)
                {
                    aColor = mpAccess->GetPixelFromData(pScanline, nX);
                    mnLevel = 0;
                    add(&pTree);

                    while (mnLeafCount > nMax)
                        reduce();
                }
            }
        }
    }
}

Octree::~Octree() { deleteOctree(&pTree); }

void Octree::deleteOctree(OctreeNode** ppNode)
{
    for (OctreeNode* i : (*ppNode)->pChild)
    {
        if (i)
            deleteOctree(&i);
    }

    mpNodeCache->ImplReleaseNode(*ppNode);
    *ppNode = nullptr;
}

void Octree::add(OctreeNode** ppNode)
{
    // possibly generate new nodes
    if (!*ppNode)
    {
        *ppNode = mpNodeCache->ImplGetFreeNode();
        (*ppNode)->bLeaf = (OCTREE_BITS == mnLevel);

        if ((*ppNode)->bLeaf)
            mnLeafCount++;
        else
        {
            (*ppNode)->pNext = mpReduce[mnLevel];
            mpReduce[mnLevel] = *ppNode;
        }
    }

    if ((*ppNode)->bLeaf)
    {
        (*ppNode)->nCount++;
        (*ppNode)->nRed += mpColor->GetRed();
        (*ppNode)->nGreen += mpColor->GetGreen();
        (*ppNode)->nBlue += mpColor->GetBlue();
    }
    else
    {
        const sal_uLong nShift = 7 - mnLevel;
        const sal_uInt8 cMask = 0x80 >> mnLevel;
        const sal_uLong nIndex = (((mpColor->GetRed() & cMask) >> nShift) << 2)
                                 | (((mpColor->GetGreen() & cMask) >> nShift) << 1)
                                 | ((mpColor->GetBlue() & cMask) >> nShift);

        mnLevel++;
        add(&(*ppNode)->pChild[nIndex]);
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

    for (sal_uLong i = 0; i < 8; i++)
    {
        if (pNode->pChild[i])
        {
            OctreeNode* pChild = pNode->pChild[i];

            nRedSum += pChild->nRed;
            nGreenSum += pChild->nGreen;
            nBlueSum += pChild->nBlue;
            pNode->nCount += pChild->nCount;

            mpNodeCache->ImplReleaseNode(pNode->pChild[i]);
            pNode->pChild[i] = nullptr;
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
        for (OctreeNode* i : pNode->pChild)
        {
            if (i)
            {
                CreatePalette(i);
            }
        }
    }
}

void Octree::GetPalIndex(OctreeNode* pNode)
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

        GetPalIndex(pNode->pChild[nIndex]);
    }
}

const BitmapPalette& Octree::GetPalette()
{
    maPalette.SetEntryCount(sal_uInt16(mnLeafCount));
    mnPalIndex = 0;
    CreatePalette(pTree);
    return maPalette;
}

sal_uInt16 Octree::GetBestPaletteIndex(const BitmapColor& rColor)
{
    mpColor = &rColor;
    mnPalIndex = 65535;
    mnLevel = 0;
    GetPalIndex(pTree);
    return mnPalIndex;
}

InverseColorMap::InverseColorMap(const BitmapPalette& rPal)
{
    const int nColorMax = 1 << OCTREE_BITS;
    const unsigned long xsqr = 1 << (gnBits << 1);
    const unsigned long xsqr2 = xsqr << 1;
    const int nColors = rPal.GetEntryCount();
    const long x = 1 << gnBits;
    const long x2 = x >> 1;
    sal_uLong r, g, b;
    long rxx, gxx, bxx;

    ImplCreateBuffers(nColorMax);

    for (int nIndex = 0; nIndex < nColors; nIndex++)
    {
        const BitmapColor& rColor = rPal[static_cast<sal_uInt16>(nIndex)];
        const long cRed = rColor.GetRed();
        const long cGreen = rColor.GetGreen();
        const long cBlue = rColor.GetBlue();

        long rdist = cRed - x2;
        long gdist = cGreen - x2;
        long bdist = cBlue - x2;
        rdist = rdist * rdist + gdist * gdist + bdist * bdist;

        const long crinc = (xsqr - (cRed << gnBits)) << 1;
        const long cginc = (xsqr - (cGreen << gnBits)) << 1;
        const long cbinc = (xsqr - (cBlue << gnBits)) << 1;

        sal_uLong* cdp = reinterpret_cast<sal_uLong*>(mpBuffer.data());
        sal_uInt8* crgbp = mpMap.data();

        for (r = 0, rxx = crinc; r < nColorMax; rdist += rxx, r++, rxx += xsqr2)
        {
            for (g = 0, gdist = rdist, gxx = cginc; g < nColorMax; gdist += gxx, g++, gxx += xsqr2)
            {
                for (b = 0, bdist = gdist, bxx = cbinc; b < nColorMax;
                     bdist += bxx, b++, cdp++, crgbp++, bxx += xsqr2)
                    if (!nIndex || static_cast<long>(*cdp) > bdist)
                    {
                        *cdp = bdist;
                        *crgbp = static_cast<sal_uInt8>(nIndex);
                    }
            }
        }
    }
}

InverseColorMap::~InverseColorMap() {}

void InverseColorMap::ImplCreateBuffers(const sal_uLong nMax)
{
    const sal_uLong nCount = nMax * nMax * nMax;
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
