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

#include <array>
#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <tools/color.hxx>

typedef struct _xmlTextWriter* xmlTextWriterPtr;
class SdrPage;

namespace svx
{

/// Offsets into the color list of a theme.
enum class ThemeColorType : sal_Int32
{
    Unknown = -1,
    Dark1 = 0,
    Light1 = 1,
    Dark2 = 2,
    Light2 = 3,
    Accent1 = 4,
    Accent2 = 5,
    Accent3 = 6,
    Accent4 = 7,
    Accent5 = 8,
    Accent6 = 9,
    Hyperlink = 10,
    FollowedHyperlink = 11,
    LAST = FollowedHyperlink
};

constexpr ThemeColorType convertToThemeColorType(sal_Int32 nIndex)
{
    if (nIndex < 0 || nIndex > 11)
        return ThemeColorType::Unknown;
    return static_cast<ThemeColorType>(nIndex);
}

class SVXCORE_DLLPUBLIC ColorSet
{
    OUString maName;
    std::array<Color, 12> maColors;

public:
    ColorSet(OUString const& rName);

    void add(sal_uInt32 nIndex, Color aColorData);

    const OUString& getName() const
    {
        return maName;
    }

    Color getColor(ThemeColorType nType) const
    {
        return maColors[size_t(nType)];
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

    const ColorSet& getColorSet(sal_uInt32 nIndex) const
    {
        return maColorSets[nIndex];
    }

    const ColorSet& getColorSet(std::u16string_view rName);
};

/// A named theme has a named color set.
class SVXCORE_DLLPUBLIC Theme
{
private:
    OUString maName;
    std::unique_ptr<ColorSet> mpColorSet;

public:
    Theme(OUString const& rName);

    void SetColorSet(std::unique_ptr<ColorSet> pColorSet);
    const ColorSet* GetColorSet() const;
    ColorSet* GetColorSet();

    void SetName(const OUString& rName);
    const OUString& GetName() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    void ToAny(css::uno::Any& rVal) const;

    static std::unique_ptr<Theme> FromAny(const css::uno::Any& rVal);

    void UpdateSdrPage(const SdrPage* pPage);

    std::vector<Color> GetColors() const;

    Color GetColor(ThemeColorType eType) const;
};

} // end of namespace svx

#endif // INCLUDED_SVX_COLORSETS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
