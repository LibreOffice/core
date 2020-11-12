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
#include <tools/gen.hxx>

namespace gfx
{
/** Derived Tuple2DL class which wraps convertion to the B2DTuple types.
 *
 * We need the unit type (typically twips or hmm) that will be used for conversion,
 * that will be used in toPoint call.
 */
class Tuple2DLWrap : public Tuple2DL
{
    LengthUnit meUnit;

public:
    static Tuple2DLWrap create(Point const& rPoint, LengthUnit eUnit = LengthUnit::hmm)
    {
        auto fX = Length::from(eUnit, rPoint.X());
        auto fY = Length::from(eUnit, rPoint.Y());
        return Tuple2DLWrap(fX, fY, eUnit);
    }

    Tuple2DLWrap(LengthUnit eUnit = LengthUnit::hmm)
        : Tuple2DL(0_emu, 0_emu)
        , meUnit(eUnit)
    {
    }

    Tuple2DLWrap(gfx::Length fX, gfx::Length fY, LengthUnit eUnit = LengthUnit::hmm)
        : Tuple2DL(fX, fY)
        , meUnit(eUnit)
    {
    }

    Tuple2DLWrap(Tuple2DLWrap const& rTuple)
        : Tuple2DL(rTuple)
        , meUnit(rTuple.meUnit)
    {
    }

    Tuple2DLWrap& operator=(Tuple2DLWrap const& rOther)
    {
        setX(rOther.getX());
        setY(rOther.getY());
        meUnit = rOther.meUnit;
        return *this;
    }

    double getUnitX() const { return getX().as(meUnit); }
    double getUnitY() const { return getY().as(meUnit); }

    void setUnitX(double fX) { return setX(gfx::Length::from(meUnit, fX)); }
    void setUnitY(double fY) { return setY(gfx::Length::from(meUnit, fY)); }

    Point toPoint() const
    {
        auto x = getUnitX();
        auto y = getUnitY();
        return Point(basegfx::fround(x), basegfx::fround(y));
    }

    gfx::LengthUnit getUnit() const { return meUnit; }

    using Tuple2DL::operator+=;
    using Tuple2DL::operator-=;
    using Tuple2DL::operator*=;
    using Tuple2DL::operator/=;
    using Tuple2DL::operator-;
};

} // end namespace gfx
