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

#include <docmodel/dllapi.h>
#include <vector>

#include <rtl/ustring.hxx>
#include <docmodel/theme/ThemeColor.hxx>
#include <docmodel/theme/ThemeColorType.hxx>
#include <docmodel/theme/Theme.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <tools/color.hxx>

typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace model
{
struct DOCMODEL_DLLPUBLIC ThemeSupplementalFont
{
    OUString maScript;
    OUString maTypeface;
};

struct DOCMODEL_DLLPUBLIC ThemeFont
{
    OUString maTypeface;
    OUString maPanose;
    sal_Int16 maPitch = 0;
    sal_Int16 maFamily = 0;
    sal_Int32 maCharset = 0;

    sal_Int16 getPitchFamily() const { return (maPitch & 0x0F) | (maFamily & 0x0F) << 4; }
};

class DOCMODEL_DLLPUBLIC FontScheme
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
    {
    }

    const OUString& getName() const { return maName; }

    ThemeFont const& getMinorLatin() const { return maMinorLatin; }
    void setMinorLatin(ThemeFont const& aMinor) { maMinorLatin = aMinor; }

    ThemeFont const& getMinorAsian() const { return maMinorAsian; }
    void setMinorAsian(ThemeFont const& aMinor) { maMinorAsian = aMinor; }

    ThemeFont const& getMinorComplex() const { return maMinorComplex; }
    void setMinorComplex(ThemeFont const& aMinor) { maMinorComplex = aMinor; }

    ThemeFont const& getMajorLatin() const { return maMajorLatin; }
    void setMajorLatin(ThemeFont const& aMajor) { maMajorLatin = aMajor; }

    ThemeFont const& getMajorAsian() const { return maMajorAsian; }
    void setMajorAsian(ThemeFont const& aMajor) { maMajorAsian = aMajor; }

    ThemeFont const& getMajorComplex() const { return maMajorComplex; }
    void setMajorComplex(ThemeFont const& aMajor) { maMajorComplex = aMajor; }

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
class DOCMODEL_DLLPUBLIC Theme
{
private:
    OUString maName;
    std::unique_ptr<model::ColorSet> mpColorSet;

    FontScheme maFontScheme;

public:
    Theme();
    Theme(OUString const& rName);

    Theme(Theme const& rTheme);

    void setFontScheme(FontScheme const& rFontScheme) { maFontScheme = rFontScheme; }

    FontScheme const& getFontScheme() const { return maFontScheme; }

    void SetColorSet(std::unique_ptr<ColorSet> pColorSet);
    const ColorSet* GetColorSet() const;
    ColorSet* GetColorSet();

    void SetName(const OUString& rName);
    const OUString& GetName() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    void ToAny(css::uno::Any& rVal) const;

    static std::unique_ptr<Theme> FromAny(const css::uno::Any& rVal);

    std::vector<Color> GetColors() const;

    Color GetColor(model::ThemeColorType eType) const;
};

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
