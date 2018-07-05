/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/ColorSets.hxx>

namespace svx
{

ColorSet::ColorSet(OUString const & aColorSetName)
    : maColorSetName(aColorSetName)
    , maColors(12)
{}

ColorSets::ColorSets()
{}

ColorSets::~ColorSets()
{}

void ColorSets::init()
{
    {
        ColorSet aColorSet("Breeze");
        aColorSet.add(0,  0x232629);
        aColorSet.add(1,  0xFCFCFC);
        aColorSet.add(2,  0x31363B);
        aColorSet.add(3,  0xEFF0F1);
        aColorSet.add(4,  0xDA4453);
        aColorSet.add(5,  0xF47750);
        aColorSet.add(6,  0xFDBC4B);
        aColorSet.add(7,  0xC9CE3B);
        aColorSet.add(8,  0x1CDC9A);
        aColorSet.add(9,  0x2ECC71);
        aColorSet.add(10, 0x1D99F3);
        aColorSet.add(11, 0x3DAEE9);
        maColorSets.push_back(aColorSet);
    }
    {
        ColorSet aColorSet("Tango");
        aColorSet.add(0,  0x000000);
        aColorSet.add(1,  0xFFFFFF);
        aColorSet.add(2,  0x2E3436);
        aColorSet.add(3,  0xBABDB6);
        aColorSet.add(4,  0x3465A4);
        aColorSet.add(5,  0x73D216);
        aColorSet.add(6,  0xF57900);
        aColorSet.add(7,  0x888A85);
        aColorSet.add(8,  0xEDD400);
        aColorSet.add(9,  0xEF2929);
        aColorSet.add(10, 0x75507B);
        aColorSet.add(11, 0x555753);
        maColorSets.push_back(aColorSet);
    }
    {
        ColorSet aColorSet("Material Blue");
        aColorSet.add(0,  0x212121);
        aColorSet.add(1,  0xFFFFFF);
        aColorSet.add(2,  0x37474F);
        aColorSet.add(3,  0xECEFF1);
        aColorSet.add(4,  0x7986CB);
        aColorSet.add(5,  0x303F9F);
        aColorSet.add(6,  0x64B5F6);
        aColorSet.add(7,  0x1976D2);
        aColorSet.add(8,  0x4FC3F7);
        aColorSet.add(9,  0x0277BD);
        aColorSet.add(10, 0x4DD0E1);
        aColorSet.add(11, 0x0097A7);
        maColorSets.push_back(aColorSet);
    }
    {
        ColorSet aColorSet("Material Red");
        aColorSet.add(0,  0x212121);
        aColorSet.add(1,  0xFFFFFF);
        aColorSet.add(2,  0x424242);
        aColorSet.add(3,  0xF5F5F5);
        aColorSet.add(4,  0xFF9800);
        aColorSet.add(5,  0xFF6D00);
        aColorSet.add(6,  0xFF5722);
        aColorSet.add(7,  0xDD2C00);
        aColorSet.add(8,  0xF44336);
        aColorSet.add(9,  0xD50000);
        aColorSet.add(10, 0xE91E63);
        aColorSet.add(11, 0xC51162);
        maColorSets.push_back(aColorSet);
    }
    {
        ColorSet aColorSet("Material Green");
        aColorSet.add(0,  0x212121);
        aColorSet.add(1,  0xFFFFFF);
        aColorSet.add(2,  0x424242);
        aColorSet.add(3,  0xF5F5F5);
        aColorSet.add(4,  0x009688);
        aColorSet.add(5,  0x00bfa5);
        aColorSet.add(6,  0x4caf50);
        aColorSet.add(7,  0x00c853);
        aColorSet.add(8,  0x8bc34a);
        aColorSet.add(9,  0x64dd17);
        aColorSet.add(10, 0xcddc39);
        aColorSet.add(11, 0xaeea00);
        maColorSets.push_back(aColorSet);
    }
}

const ColorSet& ColorSets::getColorSet(const OUString& rName)
{
    for (ColorSet & rColorSet : maColorSets)
    {
        if (rColorSet.getName() == rName)
            return rColorSet;
    }
    return maColorSets[0];
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
