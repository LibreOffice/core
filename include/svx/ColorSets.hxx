/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_COLORSETS_HXX
#define INCLUDED_SVX_COLORSETS_HXX

#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <tools/color.hxx>

typedef struct _xmlTextWriter* xmlTextWriterPtr;
class SdrPage;

namespace svx
{

class ColorSet
{
    OUString maColorSetName;
    std::vector<Color> maColors;
public:
    ColorSet(OUString const & aName);

    void add(sal_uInt32 nIndex, ::Color aColorData)
    {
        maColors[nIndex] = aColorData;
    }

    const OUString& getName() const
    {
        return maColorSetName;
    }
    const Color& getColor(sal_uInt32 nIndex) const
    {
        return maColors[nIndex];
    }

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SVXCORE_DLLPUBLIC ColorSets
{
    std::vector<ColorSet> maColorSets;
public:
    ColorSets();
    ~ColorSets();

    void init();
    const std::vector<ColorSet>& getColorSets() const
    {
        return maColorSets;
    }

    const ColorSet& getColorSet(sal_uInt32 nIndex)
    {
        return maColorSets[nIndex];
    }

    const ColorSet& getColorSet(std::u16string_view rName);
};

/// A named theme has a named color set.
class SVXCORE_DLLPUBLIC Theme
{
    OUString maName;
    std::unique_ptr<ColorSet> mpColorSet;

public:
    Theme(OUString const& rName);
    ~Theme();

    void SetColorSet(std::unique_ptr<ColorSet> pColorSet);
    ColorSet* GetColorSet();

    void SetName(const OUString& rName);
    const OUString& GetName() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    void ToAny(css::uno::Any& rVal) const;

    static std::unique_ptr<Theme> FromAny(const css::uno::Any& rVal);

    void UpdateSdrPage(SdrPage* pPage);
};

} // end of namespace svx

#endif // INCLUDED_SVX_COLORSETS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
