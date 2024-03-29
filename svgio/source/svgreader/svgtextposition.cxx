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

#include <svgtextposition.hxx>

namespace svgio::svgreader
{
SvgTextPosition::SvgTextPosition(SvgTextPosition* pParent, const SvgTspanNode& rSvgTspanNode)
    : mpParent(pParent)
    , maRotate(solveSvgNumberVector(rSvgTspanNode.getRotate(), rSvgTspanNode))
    , mfTextLength(0.0)
    , mnRotationIndex(0)
    , mbLengthAdjust(rSvgTspanNode.getLengthAdjust())
    , mbAbsoluteX(false)
{
    const InfoProvider& rInfoProvider(rSvgTspanNode);

    // get TextLength if provided
    if (rSvgTspanNode.getTextLength().isSet())
    {
        mfTextLength = rSvgTspanNode.getTextLength().solve(rInfoProvider);
    }

    // SVG does not really define in which units a \91rotate\92 for Text/TSpan is given,
    // but it seems to be degrees. Convert here to radians
    if (!maRotate.empty())
    {
        for (double& f : maRotate)
        {
            f = basegfx::deg2rad(f);
        }
    }

    // get text positions X
    const sal_uInt32 nSizeX(rSvgTspanNode.getX().size());

    if (nSizeX)
    {
        // we have absolute positions, get first one as current text position X
        maPosition.setX(rSvgTspanNode.getX()[0].solve(rInfoProvider, NumberType::xcoordinate));
        mbAbsoluteX = true;
    }
    else
    {
        // no absolute position, get from parent
        if (pParent)
        {
            maPosition.setX(pParent->getPosition().getX());
        }
    }

    const sal_uInt32 nSizeDx(rSvgTspanNode.getDx().size());
    if (nSizeDx)
    {
        // relative positions given, translate position derived from parent
        maPosition.setX(maPosition.getX()
                        + rSvgTspanNode.getDx()[0].solve(rInfoProvider, NumberType::xcoordinate));
    }

    // fill deltas to maX
    maX.reserve(nSizeX);

    for (sal_uInt32 a(1); a < std::max(nSizeX, nSizeDx); ++a)
    {
        if (a < nSizeX)
        {
            double nPos = rSvgTspanNode.getX()[a].solve(rInfoProvider, NumberType::xcoordinate)
                          - maPosition.getX();

            if (a < nSizeDx)
            {
                nPos += rSvgTspanNode.getDx()[a].solve(rInfoProvider, NumberType::xcoordinate);
            }

            maX.push_back(nPos);
        }
        else
        {
            // Apply them later since it also needs the character width to calculate
            // the final character position
            maDx.push_back(rSvgTspanNode.getDx()[a].solve(rInfoProvider, NumberType::xcoordinate));
        }
    }

    // get text positions Y
    const sal_uInt32 nSizeY(rSvgTspanNode.getY().size());

    if (nSizeY)
    {
        // we have absolute positions, get first one as current text position Y
        maPosition.setY(rSvgTspanNode.getY()[0].solve(rInfoProvider, NumberType::ycoordinate));
        mbAbsoluteX = true;
    }
    else
    {
        // no absolute position, get from parent
        if (pParent)
        {
            maPosition.setY(pParent->getPosition().getY());
        }
    }

    const sal_uInt32 nSizeDy(rSvgTspanNode.getDy().size());

    if (nSizeDy)
    {
        // relative positions given, translate position derived from parent
        maPosition.setY(maPosition.getY()
                        + rSvgTspanNode.getDy()[0].solve(rInfoProvider, NumberType::ycoordinate));
    }

    // fill deltas to maY
    maY.reserve(nSizeY);

    for (sal_uInt32 a(1); a < nSizeY; a++)
    {
        double nPos = rSvgTspanNode.getY()[a].solve(rInfoProvider, NumberType::ycoordinate)
                      - maPosition.getY();

        if (a < nSizeDy)
        {
            nPos += rSvgTspanNode.getDy()[a].solve(rInfoProvider, NumberType::ycoordinate);
        }

        maY.push_back(nPos);
    }
}

bool SvgTextPosition::isRotated() const
{
    if (maRotate.empty())
    {
        if (getParent())
        {
            return getParent()->isRotated();
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }
}

double SvgTextPosition::consumeRotation()
{
    double fRetval(0.0);

    if (maRotate.empty())
    {
        if (getParent())
        {
            fRetval = mpParent->consumeRotation();
        }
        else
        {
            fRetval = 0.0;
        }
    }
    else
    {
        const sal_uInt32 nSize(maRotate.size());

        if (mnRotationIndex < nSize)
        {
            fRetval = maRotate[mnRotationIndex++];
        }
        else
        {
            fRetval = maRotate[nSize - 1];
        }
    }

    return fRetval;
}

} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
