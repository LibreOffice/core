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

#include <svgtools.hxx>
#include <sal/log.hxx>

namespace svgio::svgreader
{

double SvgNumber::solveNonPercentage(const InfoProvider& rInfoProvider) const
{
    if (!isSet())
    {
        SAL_WARN("svgio", "SvgNumber not set (!)");
        return 0.0;
    }

    switch (meUnit)
    {
        case SvgUnit::em:
            return mfNumber * rInfoProvider.getCurrentFontSizeInherited();
        case SvgUnit::ex:
            return mfNumber * rInfoProvider.getCurrentXHeightInherited() * 0.5;
        case SvgUnit::px:
            return mfNumber;
        case SvgUnit::pt:
            return mfNumber * F_SVG_PIXEL_PER_INCH / 72.0;
        case SvgUnit::pc:
            return mfNumber * F_SVG_PIXEL_PER_INCH / 6.0;
        case SvgUnit::cm:
            return mfNumber * F_SVG_PIXEL_PER_INCH / 2.54;
        case SvgUnit::mm:
            return mfNumber * 0.1 * F_SVG_PIXEL_PER_INCH / 2.54;
        case SvgUnit::in:
            return mfNumber * F_SVG_PIXEL_PER_INCH;
        case SvgUnit::none:
        {
            SAL_WARN("svgio", "Design error, this case should have been handled in the caller");
            return mfNumber;
        }
        case SvgUnit::percent:
        {
            SAL_WARN("svgio", "Do not use with percentage!");
            break;
        }
    }

    return 0.0;
}

double SvgNumber::solve(const InfoProvider& rInfoProvider, NumberType aNumberType) const
{
    if (!isSet())
    {
        SAL_WARN("svgio", "SvgNumber not set (!)");
        return 0.0;
    }

    if (meUnit == SvgUnit::percent)
    {
        double fRetval(mfNumber * 0.01);
        basegfx::B2DRange aViewPort = rInfoProvider.getCurrentViewPort();

        if ( aViewPort.isEmpty() )
        {
            SAL_WARN("svgio", "Design error, this case should have been handled in the caller");
            // no viewPort, assume a normal page size (A4)
            aViewPort = basegfx::B2DRange(
                0.0,
                0.0,
                210.0 * F_SVG_PIXEL_PER_INCH / 2.54,
                297.0 * F_SVG_PIXEL_PER_INCH / 2.54);

        }

        if ( !aViewPort.isEmpty() )
        {
            if (NumberType::xcoordinate == aNumberType)
            {
                // it's a x-coordinate, relative to current width (w)
                fRetval *= aViewPort.getWidth();
            }
            else if (NumberType::ycoordinate == aNumberType)
            {
                // it's a y-coordinate, relative to current height (h)
                fRetval *= aViewPort.getHeight();
            }
            else // length
            {
                // it's a length, relative to sqrt(w*w + h*h)/sqrt(2)
                const double fCurrentWidth(aViewPort.getWidth());
                const double fCurrentHeight(aViewPort.getHeight());
                const double fCurrentLength(
                    sqrt(fCurrentWidth * fCurrentWidth + fCurrentHeight * fCurrentHeight)/sqrt(2.0));

                fRetval *= fCurrentLength;
            }
        }

        return fRetval;
    }

    return solveNonPercentage( rInfoProvider);
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
