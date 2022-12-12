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
#include <sal/log.hxx>
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
        if (nType == ThemeColorType::Unknown)
        {
            SAL_WARN("svx", "ColorSet::getColor with ThemeColorType::Unknown");
            return COL_AUTO;
        }
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

    void insert(ColorSet const& rColorSet);
};

struct SVXCORE_DLLPUBLIC ThemeSupplementalFont
{
    OUString maScript;
    OUString maTypeface;
};

struct SVXCORE_DLLPUBLIC ThemeFont
{
    OUString maTypeface;
    OUString maPanose;
    sal_Int16 maPitch;
    sal_Int16 maFamily;
    sal_Int32 maCharset;

    sal_Int16 getPitchFamily() const
    {
        return (maPitch & 0x0F) | (maFamily & 0x0F) << 4;
    }
};

class SVXCORE_DLLPUBLIC FontScheme
{
private:
    OUString maName;

    ThemeFont maMinorLatin;
    ThemeFont maMinorAsian;
    ThemeFont maMinorComplex;

    ThemeFont maMajorLatin;
    ThemeFont maMajorAsian;
    ThemeFont maMajorComplex;

    std::vector<ThemeSupplementalFont> maMinorSupplementalFontList;
    std::vector<ThemeSupplementalFont> maMajorSupplementalFontList;

public:
    FontScheme() = default;
    FontScheme(OUString const& rName)
        : maName(rName)
    {}

    const OUString& getName() const
    {
        return maName;
    }

    ThemeFont const& getMinorLatin() const
    {
        return maMinorLatin;
    }
    void setMinorLatin(ThemeFont const& aMinor)
    {
        maMinorLatin = aMinor;
    }

    ThemeFont const& getMinorAsian() const
    {
        return maMinorAsian;
    }
    void setMinorAsian(ThemeFont const& aMinor)
    {
        maMinorAsian = aMinor;
    }

    ThemeFont const& getMinorComplex() const
    {
        return maMinorComplex;
    }
    void setMinorComplex(ThemeFont const& aMinor)
    {
        maMinorComplex = aMinor;
    }

    ThemeFont const& getMajorLatin() const
    {
        return maMajorLatin;
    }
    void setMajorLatin(ThemeFont const& aMajor)
    {
        maMajorLatin = aMajor;
    }

    ThemeFont const& getMajorAsian() const
    {
        return maMajorAsian;
    }
    void setMajorAsian(ThemeFont const& aMajor)
    {
        maMajorAsian = aMajor;
    }

    ThemeFont const& getMajorComplex() const
    {
        return maMajorComplex;
    }
    void setMajorComplex(ThemeFont const& aMajor)
    {
        maMajorComplex = aMajor;
    }

    OUString findMinorSupplementalTypeface(std::u16string_view rScript) const
    {
        for (auto const& rSupplementalFont : maMinorSupplementalFontList)
        {
            if (rSupplementalFont.maScript == rScript)
                return rSupplementalFont.maTypeface;
        }
        return OUString();
    }

    std::vector<ThemeSupplementalFont> const& getMinorSupplementalFontList() const
    {
        return maMinorSupplementalFontList;
    }
    void setMinorSupplementalFontList(std::vector<ThemeSupplementalFont> const& rSupplementalFont)
    {
        maMinorSupplementalFontList = rSupplementalFont;
    }

    OUString findMajorSupplementalTypeface(std::u16string_view rScript) const
    {
        for (auto const& rSupplementalFont : maMajorSupplementalFontList)
        {
            if (rSupplementalFont.maScript == rScript)
                return rSupplementalFont.maTypeface;
        }
        return OUString();
    }

    std::vector<ThemeSupplementalFont> const& getMajorSupplementalFontList() const
    {
        return maMajorSupplementalFontList;
    }
    void setMajorSupplementalFontList(std::vector<ThemeSupplementalFont> const& rSupplementalFont)
    {
        maMajorSupplementalFontList = rSupplementalFont;
    }
};

/// A named theme has a named color set.
class SVXCORE_DLLPUBLIC Theme
{
private:
    OUString maName;
    std::unique_ptr<ColorSet> mpColorSet;

    FontScheme maFontScheme;

public:
    Theme(OUString const& rName);

    void setFontScheme(FontScheme const& rFontScheme)
    {
        maFontScheme = rFontScheme;
    }

    FontScheme const& getFontScheme() const { return maFontScheme; }

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
