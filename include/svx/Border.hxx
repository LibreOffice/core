/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/units/Length.hxx>
#include <tools/gen.hxx>

namespace svx
{
/** Represents the borders or margins from the sides of a rectangular object.
 *
 * Values represent the length from the sides of a rectangular object.
 * Used mainly to define the borders/margins for a page.
 *
 * Includes the base unit, which is used for conversion to tools::Rectangle,
 * or for leftUnit, rightUnit, upperUnit, lowerUnit getters. These are needed
 * for compatibility and will eventually go unused when the code is converted
 * to use gfx::Length.
 */
class Border
{
private:
    gfx::Length maLeft;
    gfx::Length maRight;
    gfx::Length maUpper;
    gfx::Length maLower;
    gfx::LengthUnit meUnit;

public:
    Border(gfx::LengthUnit eUnit = gfx::LengthUnit::hmm)
        : maLeft(0_emu)
        , maRight(0_emu)
        , maUpper(0_emu)
        , maLower(0_emu)
        , meUnit(eUnit)
    {
    }

    gfx::Length const& left() const { return maLeft; }
    gfx::Length const& right() const { return maRight; }
    gfx::Length const& upper() const { return maUpper; }
    gfx::Length const& lower() const { return maLower; }

    gfx::Length const& getLeft() const { return maLeft; }
    gfx::Length const& getRight() const { return maRight; }
    gfx::Length const& getUpper() const { return maUpper; }
    gfx::Length const& getLower() const { return maLower; }

    tools::Long leftUnit() const { return maLeft.as(meUnit); }
    tools::Long rightUnit() const { return maRight.as(meUnit); }
    tools::Long upperUnit() const { return maUpper.as(meUnit); }
    tools::Long lowerUnit() const { return maLower.as(meUnit); }

    tools::Rectangle toToolsRect() const
    {
        return tools::Rectangle(leftUnit(), upperUnit(), rightUnit(), lowerUnit());
    }

    bool isEmpty() const
    {
        return maLeft == 0_emu && maRight == 0_emu && maUpper == 0_emu && maLower == 0_emu;
    }

    void setLeft(gfx::Length const& rLeft) { maLeft = rLeft; }

    void setRight(gfx::Length const& rRight) { maRight = rRight; }

    void setUpper(gfx::Length const& rUpper) { maUpper = rUpper; }

    void setLower(gfx::Length const& rLower) { maLower = rLower; }
};

} // end svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
