/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <editeng/editengdllapi.h>
#include <tools/color.hxx>

class ColorSet
{
    OUString maColorSetName;
    std::vector<Color> maColors;

public:
    ColorSet(OUString const& aName);

    void add(sal_uInt32 nIndex, ::Color aColorData) { maColors[nIndex] = aColorData; }

    const OUString& getName() const { return maColorSetName; }
    const Color& getColor(sal_uInt32 nIndex) const { return maColors[nIndex]; }

    const std::vector<Color>& getColors() const { return maColors; }
};

class EDITENG_DLLPUBLIC ColorSets
{
    std::vector<ColorSet> maColorSets;
    sal_Int32 mnThemeColorSetIndex;
    ColorSets();

public:
    ~ColorSets();

    static ColorSets& get()
    {
        static ColorSets aColorSetsInstance;
        return aColorSetsInstance;
    }

    const ColorSet& getThemeColorSet() { return getColorSet(mnThemeColorSetIndex); }

    void setThemeColorSet(sal_Int32 nIndex);

    void setThemeColorSet(std::u16string_view rName);

    const std::vector<ColorSet>& getColorSets() const { return maColorSets; }

    const ColorSet& getColorSet(sal_uInt32 nIndex) { return maColorSets[nIndex]; }

    const ColorSet& getColorSet(std::u16string_view rName);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
