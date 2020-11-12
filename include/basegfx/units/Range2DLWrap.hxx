/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/units/LengthTypes.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <tools/gen.hxx>

namespace gfx
{
class Range2DLWrap : public Range2DL
{
    LengthUnit meUnit;
    mutable tools::Rectangle maRectangle;

public:
    static Range2DLWrap create(tools::Rectangle const& rRectangle,
                               LengthUnit eUnit = LengthUnit::hmm)
    {
        if (rRectangle.IsWidthEmpty() && rRectangle.IsHeightEmpty())
            return Range2DLWrap(eUnit);

        auto left = Length::from(eUnit, rRectangle.Left());
        auto top = Length::from(eUnit, rRectangle.Top());
        auto right = Length::from(eUnit, rRectangle.Right());
        auto bottom = Length::from(eUnit, rRectangle.Bottom());

        return Range2DLWrap(left, top, right, bottom, eUnit);
    }

    static Range2DLWrap create(basegfx::B2DRange const& rRange2D,
                               LengthUnit eUnit = LengthUnit::hmm)
    {
        if (rRange2D.isEmpty())
            return Range2DLWrap(eUnit);

        auto left = Length::from(eUnit, rRange2D.getMinX());
        auto top = Length::from(eUnit, rRange2D.getMinY());
        auto right = Length::from(eUnit, rRange2D.getMaxX());
        auto bottom = Length::from(eUnit, rRange2D.getMaxY());

        return Range2DLWrap(left, top, right, bottom, eUnit);
    }

    Range2DLWrap(LengthUnit eUnit = LengthUnit::hmm)
        : Range2DL()
        , meUnit(eUnit)
    {
    }

    Range2DLWrap(gfx::Length x1, gfx::Length y1, gfx::Length x2, gfx::Length y2,
                 LengthUnit eUnit = LengthUnit::hmm)
        : Range2DL(x1, y1, x2, y2)
        , meUnit(eUnit)
    {
    }

    tools::Rectangle const& toToolsRect() const
    {
        if (isEmpty())
        {
            maRectangle = tools::Rectangle();
        }
        else
        {
            auto left = getMinX().as(meUnit);
            auto top = getMinY().as(meUnit);
            auto right = getMaxX().as(meUnit);
            auto bottom = getMaxY().as(meUnit);

            if (left == right && top == bottom)
            {
                maRectangle = tools::Rectangle();
                maRectangle.Move(basegfx::fround(left), basegfx::fround(top));
            }
            else
            {
                maRectangle = tools::Rectangle(basegfx::fround(left), basegfx::fround(top),
                                               basegfx::fround(right), basegfx::fround(bottom));
            }
        }
        return maRectangle;
    }

    basegfx::B2DRange toB2DRect() const
    {
        if (isEmpty())
            return basegfx::B2DRange();
        auto left = getMinX().as(meUnit);
        auto top = getMinY().as(meUnit);
        auto right = getMaxX().as(meUnit);
        auto bottom = getMaxY().as(meUnit);
        return basegfx::B2DRange(left, top, right, bottom);
    }

    gfx::LengthUnit getUnit() const { return meUnit; }
};

} // end namespace gfx
