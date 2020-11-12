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
#include <basegfx/vector/b2dsize.hxx>
#include <tools/gen.hxx>

namespace gfx
{
class Size2DLWrap : public Size2DL
{
    LengthUnit meUnit;

public:
    static Size2DLWrap create(Size const& rSize, LengthUnit eUnit = LengthUnit::hmm)
    {
        if (rSize.IsEmpty())
            return Size2DLWrap(0_emu, 0_emu, eUnit);
        auto width = Length::from(eUnit, rSize.getWidth());
        auto height = Length::from(eUnit, rSize.getHeight());
        return Size2DLWrap(width, height, eUnit);
    }

    Size2DLWrap(LengthUnit eUnit = LengthUnit::hmm)
        : Size2DL(0_emu, 0_emu)
        , meUnit(eUnit)
    {
    }

    Size2DLWrap(gfx::Length fX, gfx::Length fY, LengthUnit eUnit = LengthUnit::hmm)
        : Size2DL(fX, fY)
        , meUnit(eUnit)
    {
    }

    Size2DLWrap(Size2DLWrap const& rSize)
        : Size2DL(rSize)
        , meUnit(rSize.meUnit)
    {
    }

    Size2DLWrap& operator=(Size2DLWrap const& rOther)
    {
        setWidth(rOther.getWidth());
        setHeight(rOther.getHeight());
        meUnit = rOther.meUnit;
        return *this;
    }

    double getUnitWidth() const { return getWidth().as(meUnit); }
    double getUnitHeight() const { return getHeight().as(meUnit); }

    void setUnitWidth(double fWidth) { setWidth(gfx::Length::from(meUnit, fWidth)); }
    void setUnitHeight(double fHeight) { return setHeight(gfx::Length::from(meUnit, fHeight)); }

    basegfx::B2DSize toB2DSize() const { return basegfx::B2DSize(getUnitWidth(), getUnitHeight()); }

    Size toToolsSize() const
    {
        auto width = getUnitWidth();
        auto height = getUnitHeight();
        return Size(basegfx::fround(width), basegfx::fround(height));
    }

    gfx::LengthUnit getUnit() const { return meUnit; }

    using Size2DL::operator+=;
    using Size2DL::operator-=;
    using Size2DL::operator*=;
    using Size2DL::operator/=;
    using Size2DL::operator-;
};

} // end namespace gfx
