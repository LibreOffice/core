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

#include <sal/log.hxx>
#include <tools/link.hxx>
#include <tools/poly.hxx>
#include <tools/helpers.hxx>

#include <vcl/BitmapReadAccess.hxx>
#include <vcl/Vectorizer.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>

#include "Chain.hxx"
#include "ChainMove.hxx"
#include "ColorSet.hxx"
#include "PointArray.hxx"
#include "VectorMap.hxx"

#include <array>
#include <memory>

namespace vcl::Vectorizer
{
static VectorMap* Expand(BitmapReadAccess* pRAcc, const Color& rColor);
static void Calculate(VectorMap* pMap, tools::PolyPolygon& rPolyPoly, sal_uInt8 cReduce);
static bool GetChain(VectorMap* pMap, const Point& rStartPt, Chain& rChain);
static bool IsUp(VectorMap const* pMap, tools::Long nY, tools::Long nX);
static void LimitPolyPoly(tools::PolyPolygon& rPolyPoly);

GDIMetaFile ProduceMetafile(const Bitmap& rColorBmp, sal_uInt8 cReduce,
                            const Link<tools::Long, void>* pProgress)
{
    VECT_PROGRESS(pProgress, 0);

    std::unique_ptr<Bitmap> xBmp(new Bitmap(rColorBmp));
    Bitmap::ScopedReadAccess pRAcc(*xBmp);

    GDIMetaFile aMtf;

    if (pRAcc)
    {
        tools::PolyPolygon aPolyPoly;
        double fPercent = 0.0;
        double fPercentStep_2 = 0.0;
        const tools::Long nWidth = pRAcc->Width();
        const tools::Long nHeight = pRAcc->Height();
        const sal_uInt16 nColorCount = pRAcc->GetPaletteEntryCount();
        sal_uInt16 n;
        std::array<ColorSet, 256> aColorSet;

        // get used palette colors and sort them from light to dark colors
        for (n = 0; n < nColorCount; n++)
        {
            aColorSet[n].mnIndex = n;
            aColorSet[n].maColor = pRAcc->GetPaletteColor(n);
        }

        for (tools::Long nY = 0; nY < nHeight; nY++)
        {
            Scanline pScanlineRead = pRAcc->GetScanline(nY);
            for (tools::Long nX = 0; nX < nWidth; nX++)
                aColorSet[pRAcc->GetIndexFromData(pScanlineRead, nX)].mbSet = true;
        }

        std::sort(aColorSet.begin(), aColorSet.end(), ColorSetCmpFnc);

        for (n = 0; n < 256; n++)
            if (!aColorSet[n].mbSet)
                break;

        if (n)
            fPercentStep_2 = 45.0 / n;

        fPercent += 10.0;
        VECT_PROGRESS(pProgress, FRound(fPercent));

        for (sal_uInt16 i = 0; i < n; i++)
        {
            const BitmapColor aBmpCol(pRAcc->GetPaletteColor(aColorSet[i].mnIndex));
            const Color aFindColor(aBmpCol.GetRed(), aBmpCol.GetGreen(), aBmpCol.GetBlue());
            std::unique_ptr<VectorMap> xMap(Expand(pRAcc.get(), aFindColor));

            fPercent += fPercentStep_2;
            VECT_PROGRESS(pProgress, FRound(fPercent));

            if (xMap)
            {
                aPolyPoly.Clear();
                Calculate(xMap.get(), aPolyPoly, cReduce);
                xMap.reset();

                if (aPolyPoly.Count())
                {
                    LimitPolyPoly(aPolyPoly);

                    aPolyPoly.Optimize(PolyOptimizeFlags::EDGES);

                    if (aPolyPoly.Count())
                    {
                        aMtf.AddAction(new MetaLineColorAction(aFindColor, true));
                        aMtf.AddAction(new MetaFillColorAction(aFindColor, true));
                        aMtf.AddAction(new MetaPolyPolygonAction(aPolyPoly));
                    }
                }
            }

            fPercent += fPercentStep_2;
            VECT_PROGRESS(pProgress, FRound(fPercent));
        }

        if (aMtf.GetActionSize())
        {
            MapMode aMap(MapUnit::Map100thMM);
            ScopedVclPtrInstance<VirtualDevice> aVDev;
            const Size aLogSize1(aVDev->PixelToLogic(Size(1, 1), aMap));

            aMtf.SetPrefMapMode(aMap);
            aMtf.SetPrefSize(Size(nWidth + 2, nHeight + 2));
            aMtf.Move(1, 1);
            aMtf.Scale(aLogSize1.Width(), aLogSize1.Height());

            return aMtf;
        }
    }

    pRAcc.reset();
    xBmp.reset();
    VECT_PROGRESS(pProgress, 100);

    return aMtf;
}

void LimitPolyPoly(tools::PolyPolygon& rPolyPoly)
{
    if (rPolyPoly.Count() <= VECT_POLY_MAX)
        return;

    tools::PolyPolygon aNewPolyPoly;
    tools::Long nReduce = 0;
    sal_uInt16 nNewCount;

    do
    {
        aNewPolyPoly.Clear();
        nReduce++;

        for (sal_uInt16 i = 0, nCount = rPolyPoly.Count(); i < nCount; i++)
        {
            const tools::Rectangle aBound(rPolyPoly[i].GetBoundRect());

            if (aBound.GetWidth() > nReduce && aBound.GetHeight() > nReduce)
            {
                if (rPolyPoly[i].GetSize())
                    aNewPolyPoly.Insert(rPolyPoly[i]);
            }
        }

        nNewCount = aNewPolyPoly.Count();
    } while (nNewCount > VECT_POLY_MAX);

    rPolyPoly = aNewPolyPoly;
}

VectorMap* Expand(BitmapReadAccess* pRAcc, const Color& rColor)
{
    VectorMap* pMap = nullptr;

    if (pRAcc && pRAcc->Width() && pRAcc->Height())
    {
        const tools::Long nOldWidth = pRAcc->Width();
        const tools::Long nOldHeight = pRAcc->Height();
        const tools::Long nNewWidth = (nOldWidth << 2) + 4;
        const tools::Long nNewHeight = (nOldHeight << 2) + 4;
        const BitmapColor aTest(pRAcc->GetBestMatchingColor(rColor));
        std::unique_ptr<tools::Long[]> pMapIn(new tools::Long[std::max(nOldWidth, nOldHeight)]);
        std::unique_ptr<tools::Long[]> pMapOut(new tools::Long[std::max(nOldWidth, nOldHeight)]);
        tools::Long nX, nY, nTmpX, nTmpY;

        pMap = new VectorMap(nNewWidth, nNewHeight);

        for (nX = 0; nX < nOldWidth; nX++)
            VECT_MAP(pMapIn, pMapOut, nX);

        for (nY = 0, nTmpY = 5; nY < nOldHeight; nY++, nTmpY += 4)
        {
            Scanline pScanlineRead = pRAcc->GetScanline(nY);
            for (nX = 0; nX < nOldWidth;)
            {
                if (pRAcc->GetPixelFromData(pScanlineRead, nX) == aTest)
                {
                    nTmpX = pMapIn[nX++];
                    nTmpY -= 3;

                    pMap->Set(nTmpY++, nTmpX, VECT_CONT_INDEX);
                    pMap->Set(nTmpY++, nTmpX, VECT_CONT_INDEX);
                    pMap->Set(nTmpY++, nTmpX, VECT_CONT_INDEX);
                    pMap->Set(nTmpY, nTmpX, VECT_CONT_INDEX);

                    while (nX < nOldWidth && pRAcc->GetPixelFromData(pScanlineRead, nX) == aTest)
                        nX++;

                    nTmpX = pMapOut[nX - 1];
                    nTmpY -= 3;

                    pMap->Set(nTmpY++, nTmpX, VECT_CONT_INDEX);
                    pMap->Set(nTmpY++, nTmpX, VECT_CONT_INDEX);
                    pMap->Set(nTmpY++, nTmpX, VECT_CONT_INDEX);
                    pMap->Set(nTmpY, nTmpX, VECT_CONT_INDEX);
                }
                else
                    nX++;
            }
        }

        for (nY = 0; nY < nOldHeight; nY++)
            VECT_MAP(pMapIn, pMapOut, nY);

        for (nX = 0, nTmpX = 5; nX < nOldWidth; nX++, nTmpX += 4)
        {
            for (nY = 0; nY < nOldHeight;)
            {
                if (pRAcc->GetPixel(nY, nX) == aTest)
                {
                    nTmpX -= 3;
                    nTmpY = pMapIn[nY++];

                    pMap->Set(nTmpY, nTmpX++, VECT_CONT_INDEX);
                    pMap->Set(nTmpY, nTmpX++, VECT_CONT_INDEX);
                    pMap->Set(nTmpY, nTmpX++, VECT_CONT_INDEX);
                    pMap->Set(nTmpY, nTmpX, VECT_CONT_INDEX);

                    while (nY < nOldHeight && pRAcc->GetPixel(nY, nX) == aTest)
                        nY++;

                    nTmpX -= 3;
                    nTmpY = pMapOut[nY - 1];

                    pMap->Set(nTmpY, nTmpX++, VECT_CONT_INDEX);
                    pMap->Set(nTmpY, nTmpX++, VECT_CONT_INDEX);
                    pMap->Set(nTmpY, nTmpX++, VECT_CONT_INDEX);
                    pMap->Set(nTmpY, nTmpX, VECT_CONT_INDEX);
                }
                else
                    nY++;
            }
        }
    }

    return pMap;
}

void Calculate(VectorMap* pMap, tools::PolyPolygon& rPolyPoly, sal_uInt8 cReduce)
{
    const tools::Long nWidth = pMap->Width(), nHeight = pMap->Height();

    for (tools::Long nY = 0; nY < nHeight; nY++)
    {
        tools::Long nX = 0;
        bool bInner = true;

        while (nX < nWidth)
        {
            // skip free
            while ((nX < nWidth) && pMap->IsFree(nY, nX))
                nX++;

            if (nX == nWidth)
                break;

            if (pMap->IsCont(nY, nX))
            {
                // new contour
                Chain aChain;
                const Point aStartPt(nX++, nY);

                // get chain code
                aChain.BeginAdd(aStartPt);
                GetChain(pMap, aStartPt, aChain);

                aChain.EndAdd(bInner ? VECT_POLY_OUTLINE_INNER : VECT_POLY_OUTLINE_OUTER);

                const tools::Polygon& rPoly = aChain.GetPoly();

                if (rPoly.GetSize() > 2)
                {
                    if (cReduce)
                    {
                        const tools::Rectangle aBound(rPoly.GetBoundRect());

                        if (aBound.GetWidth() > cReduce && aBound.GetHeight() > cReduce)
                            rPolyPoly.Insert(rPoly);
                    }
                    else
                        rPolyPoly.Insert(rPoly);
                }

                // skip rest of detected contour
                while (pMap->IsCont(nY, nX))
                    nX++;
            }
            else
            {
                // process done segment
                const tools::Long nStartSegX = nX++;

                while (pMap->IsDone(nY, nX))
                    nX++;

                if (((nX - nStartSegX) == 1)
                    || (IsUp(pMap, nY, nStartSegX) != IsUp(pMap, nY, nX - 1)))
                    bInner = !bInner;
            }
        }
    }
}

bool GetChain(VectorMap* pMap, const Point& rStartPt, Chain& rChain)
{
    tools::Long nActX = rStartPt.X();
    tools::Long nActY = rStartPt.Y();
    sal_uLong nFound;
    sal_uLong nLastDir = 0;
    sal_uLong nDir;

    do
    {
        nFound = 0;

        // first try last direction
        tools::Long nTryX = nActX + aMove[nLastDir].nDX;
        tools::Long nTryY = nActY + aMove[nLastDir].nDY;

        if (pMap->IsCont(nTryY, nTryX))
        {
            rChain.Add(static_cast<sal_uInt8>(nLastDir));
            nActY = nTryY;
            nActX = nTryX;
            pMap->Set(nActY, nActX, VECT_DONE_INDEX);
            nFound = 1;
        }
        else
        {
            // try other directions
            for (nDir = 0; nDir < 8; nDir++)
            {
                // we already tried nLastDir
                if (nDir != nLastDir)
                {
                    nTryX = nActX + aMove[nDir].nDX;
                    nTryY = nActY + aMove[nDir].nDY;

                    if (pMap->IsCont(nTryY, nTryX))
                    {
                        rChain.Add(static_cast<sal_uInt8>(nDir));
                        nActY = nTryY;
                        nActX = nTryX;
                        pMap->Set(nActY, nActX, VECT_DONE_INDEX);
                        nFound = 1;
                        nLastDir = nDir;
                        break;
                    }
                }
            }
        }
    } while (nFound);

    return true;
}

bool IsUp(VectorMap const* pMap, tools::Long nY, tools::Long nX)
{
    if (pMap->IsDone(nY - 1, nX))
        return true;
    else if (pMap->IsDone(nY + 1, nX))
        return false;
    else if (pMap->IsDone(nY - 1, nX - 1) || pMap->IsDone(nY - 1, nX + 1))
        return true;
    else
        return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
