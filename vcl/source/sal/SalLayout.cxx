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

#include <iostream>
#include <iomanip>

#include <sal/config.h>
#include <sal/log.hxx>

#include <cstdio>

#include <math.h>

#include <salgdi.hxx>
#include <sallayout.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <i18nlangtag/lang.h>

#include <vcl/svapp.hxx>

#include <unicode/ubidi.h>
#include <unicode/uchar.h>

#include <algorithm>
#include <memory>

#include <impglyphitem.hxx>

namespace vcl
{
namespace sal
{
SalLayout::SalLayout()
    : mnMinCharPos(-1)
    , mnEndCharPos(-1)
    , mnUnitsPerPixel(1)
    , mnOrientation(0)
    , maDrawOffset(0, 0)
{
}

SalLayout::~SalLayout() {}

void SalLayout::AdjustLayout(ImplLayoutArgs& rArgs)
{
    mnMinCharPos = rArgs.mnMinCharPos;
    mnEndCharPos = rArgs.mnEndCharPos;
    mnOrientation = rArgs.mnOrientation;
}

Point SalLayout::GetDrawPosition(const Point& rRelative) const
{
    Point aPos = maDrawBase;
    Point aOfs = rRelative + maDrawOffset;

    if (mnOrientation == 0_deg10)
        aPos += aOfs;
    else
    {
        // cache trigonometric results
        static Degree10 nOldOrientation(0);
        static double fCos = 1.0, fSin = 0.0;
        if (nOldOrientation != mnOrientation)
        {
            nOldOrientation = mnOrientation;
            double fRad = toRadians(mnOrientation);
            fCos = cos(fRad);
            fSin = sin(fRad);
        }

        double fX = aOfs.X();
        double fY = aOfs.Y();
        tools::Long nX = static_cast<tools::Long>(+fCos * fX + fSin * fY);
        tools::Long nY = static_cast<tools::Long>(+fCos * fY - fSin * fX);
        aPos += Point(nX, nY);
    }

    return aPos;
}

bool SalLayout::GetOutline(basegfx::B2DPolyPolygonVector& rVector) const
{
    bool bAllOk = true;
    bool bOneOk = false;

    basegfx::B2DPolyPolygon aGlyphOutline;

    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (GetNextGlyph(&pGlyph, aPos, nStart))
    {
        // get outline of individual glyph, ignoring "empty" glyphs
        bool bSuccess = pGlyph->GetGlyphOutline(aGlyphOutline);
        bAllOk &= bSuccess;
        bOneOk |= bSuccess;
        // only add non-empty outlines
        if (bSuccess && (aGlyphOutline.count() > 0))
        {
            if (aPos.X() || aPos.Y())
            {
                aGlyphOutline.transform(
                    basegfx::utils::createTranslateB2DHomMatrix(aPos.X(), aPos.Y()));
            }

            // insert outline at correct position
            rVector.push_back(aGlyphOutline);
        }
    }

    return (bAllOk && bOneOk);
}

bool SalLayout::GetBoundRect(tools::Rectangle& rRect) const
{
    bool bRet = false;
    rRect.SetEmpty();

    tools::Rectangle aRectangle;

    Point aPos;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (GetNextGlyph(&pGlyph, aPos, nStart))
    {
        // get bounding rectangle of individual glyph
        if (pGlyph->GetGlyphBoundRect(aRectangle))
        {
            // merge rectangle
            aRectangle += aPos;
            if (rRect.IsEmpty())
                rRect = aRectangle;
            else
                rRect.Union(aRectangle);
            bRet = true;
        }
    }

    return bRet;
}
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
