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

#include <SvgNumber.hxx>

#include <o3tl/unit_conversion.hxx>
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
        // See https://drafts.csswg.org/css-values-4/#font-relative-length
        case SvgUnit::em:
            return mfNumber * rInfoProvider.getCurrentFontSize();
        case SvgUnit::ex:
            return mfNumber * rInfoProvider.getCurrentXHeight();
        case SvgUnit::px:
            return mfNumber;
        case SvgUnit::pt:
            return o3tl::convert(mfNumber, o3tl::Length::pt, o3tl::Length::px);
        case SvgUnit::pc:
            return o3tl::convert(mfNumber, o3tl::Length::pc, o3tl::Length::px);
        case SvgUnit::cm:
            return o3tl::convert(mfNumber, o3tl::Length::cm, o3tl::Length::px);
        case SvgUnit::mm:
            return o3tl::convert(mfNumber, o3tl::Length::mm, o3tl::Length::px);
        case SvgUnit::in:
            return o3tl::convert(mfNumber, o3tl::Length::in, o3tl::Length::px);
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
                o3tl::convert(210.0, o3tl::Length::cm, o3tl::Length::px), // should it be mm?
                o3tl::convert(297.0, o3tl::Length::cm, o3tl::Length::px));

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
                // it's a length, relative to sqrt((w^2 + h^2)/2)
                const double fCurrentWidth(aViewPort.getWidth());
                const double fCurrentHeight(aViewPort.getHeight());
                const double fCurrentLength(
                    sqrt((fCurrentWidth * fCurrentWidth + fCurrentHeight * fCurrentHeight)/2.0));

                fRetval *= fCurrentLength;
            }
        }

        return fRetval;
    }

    return solveNonPercentage( rInfoProvider);
}

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
