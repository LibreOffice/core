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
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <unotools/configmgr.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/mslangid.hxx>

#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/event.hxx>

#include <outdev.h>
#include <window.h>
#include <sallayout.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <impglyphitem.hxx>
#include <PhysicalFontCollection.hxx>
#include <font/FeatureCollector.hxx>
#include <font/emphasismark.hxx>

#include <strings.hrc>

std::tuple<tools::PolyPolygon, bool, tools::Long, tools::Long, tools::Rectangle, tools::Rectangle>
GetEmphasisMark(FontEmphasisMark eEmphasis, tools::Long nHeight, sal_Int32 nDPIY)
{
    assert(nHeight);

    static const PolyFlags aAccentPolyFlags[24]
        = { PolyFlags::Normal,  PolyFlags::Control, PolyFlags::Control, PolyFlags::Normal,
            PolyFlags::Control, PolyFlags::Control, PolyFlags::Normal,  PolyFlags::Control,
            PolyFlags::Control, PolyFlags::Normal,  PolyFlags::Control, PolyFlags::Control,
            PolyFlags::Normal,  PolyFlags::Control, PolyFlags::Control, PolyFlags::Normal,
            PolyFlags::Control, PolyFlags::Control, PolyFlags::Normal,  PolyFlags::Normal,
            PolyFlags::Control, PolyFlags::Normal,  PolyFlags::Control, PolyFlags::Control };

    static const Point aAccentPos[24]
        = { { 78, 0 },    { 348, 79 },  { 599, 235 }, { 843, 469 }, { 938, 574 }, { 990, 669 },
            { 990, 773 }, { 990, 843 }, { 964, 895 }, { 921, 947 }, { 886, 982 }, { 860, 999 },
            { 825, 999 }, { 764, 999 }, { 721, 964 }, { 686, 895 }, { 625, 791 }, { 556, 660 },
            { 469, 504 }, { 400, 400 }, { 261, 252 }, { 61, 61 },   { 0, 27 },    { 9, 0 } };

    tools::Long nWidth = 0;
    tools::Long nYOff = 0;

    FontEmphasisMark nEmphasisStyle = eEmphasis & FontEmphasisMark::Style;
    tools::Long nDotSize = 0;

    bool bIsPolyLine = false;
    tools::PolyPolygon aPolyPoly;
    tools::Rectangle aRect1;
    tools::Rectangle aRect2;

    switch (nEmphasisStyle)
    {
        case FontEmphasisMark::Dot:
            // Dot has 55% of the height
            nDotSize = (nHeight * 550) / 1000;

            if (!nDotSize)
                nDotSize = 1;

            if (nDotSize <= 2)
            {
                aRect1 = tools::Rectangle(Point(), Size(nDotSize, nDotSize));
            }
            else
            {
                tools::Long nRad = nDotSize / 2;
                tools::Polygon aPoly(Point(nRad, nRad), nRad, nRad);
                aPolyPoly.Insert(aPoly);
            }
            nYOff = ((nHeight * 250) / 1000) / 2; // Center to the another EmphasisMarks
            nWidth = nDotSize;
            break;

        case FontEmphasisMark::Circle:
            // Dot has 80% of the height
            nDotSize = (nHeight * 800) / 1000;

            if (!nDotSize)
                nDotSize = 1;

            if (nDotSize <= 2)
            {
                aRect1 = tools::Rectangle(Point(), Size(nDotSize, nDotSize));
            }
            else
            {
                tools::Long nRad = nDotSize / 2;
                tools::Polygon aPoly(Point(nRad, nRad), nRad, nRad);
                aPolyPoly.Insert(aPoly);
                // BorderWidth is 15%
                tools::Long nBorder = (nDotSize * 150) / 1000;

                if (nBorder <= 1)
                {
                    bIsPolyLine = true;
                }
                else
                {
                    tools::Polygon aPoly2(Point(nRad, nRad), nRad - nBorder, nRad - nBorder);
                    aPolyPoly.Insert(aPoly2);
                }
            }

            nWidth = nDotSize;
            break;

        case FontEmphasisMark::Disc:
            // Dot has 80% of the height
            nDotSize = (nHeight * 800) / 1000;

            if (!nDotSize)
                nDotSize = 1;

            if (nDotSize <= 2)
            {
                aRect1 = tools::Rectangle(Point(), Size(nDotSize, nDotSize));
            }
            else
            {
                tools::Long nRad = nDotSize / 2;
                tools::Polygon aPoly(Point(nRad, nRad), nRad, nRad);
                aPolyPoly.Insert(aPoly);
            }

            nWidth = nDotSize;
            break;

        case FontEmphasisMark::Accent:
            // Dot has 80% of the height
            nDotSize = (nHeight * 800) / 1000;

            if (!nDotSize)
                nDotSize = 1;

            if (nDotSize <= 2)
            {
                if (nDotSize == 1)
                {
                    aRect1 = tools::Rectangle(Point(), Size(nDotSize, nDotSize));
                    nWidth = nDotSize;
                }
                else
                {
                    aRect1 = tools::Rectangle(Point(), Size(1, 1));
                    aRect2 = tools::Rectangle(Point(1, 1), Size(1, 1));
                }
            }
            else
            {
                tools::Polygon aPoly(SAL_N_ELEMENTS(aAccentPos), aAccentPos, aAccentPolyFlags);

                double dScale = static_cast<double>(nDotSize) / 1000.0;
                aPoly.Scale(dScale, dScale);

                tools::Polygon aTemp;
                aPoly.AdaptiveSubdivide(aTemp);

                tools::Rectangle aBoundRect = aTemp.GetBoundRect();
                nWidth = aBoundRect.GetWidth();
                nDotSize = aBoundRect.GetHeight();

                aPolyPoly.Insert(aTemp);
            }
            break;
        default:
            break;
    }

    // calculate position
    tools::Long nOffY = 1 + (nDPIY / 300); // one visible pixel space
    tools::Long nSpaceY = nHeight - nDotSize;
    if (nSpaceY >= nOffY * 2)
        nYOff += nOffY;

    if (!(eEmphasis & FontEmphasisMark::PosBelow))
        nYOff += nDotSize;

    return std::make_tuple(aPolyPoly, bIsPolyLine, nYOff, nWidth, aRect1, aRect2);
}

FontEmphasisMark GetEmphasisMarkStyle(const vcl::Font& rFont)
{
    FontEmphasisMark nEmphasisMark = rFont.GetEmphasisMark();

    // If no Position is set, then calculate the default position, which
    // depends on the language
    if (!(nEmphasisMark & (FontEmphasisMark::PosAbove | FontEmphasisMark::PosBelow)))
    {
        LanguageType eLang = rFont.GetLanguage();
        // In Chinese Simplified the EmphasisMarks are below/left
        if (MsLangId::isSimplifiedChinese(eLang))
            nEmphasisMark |= FontEmphasisMark::PosBelow;
        else
        {
            eLang = rFont.GetCJKContextLanguage();
            // In Chinese Simplified the EmphasisMarks are below/left
            if (MsLangId::isSimplifiedChinese(eLang))
                nEmphasisMark |= FontEmphasisMark::PosBelow;
            else
                nEmphasisMark |= FontEmphasisMark::PosAbove;
        }
    }

    return nEmphasisMark;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
