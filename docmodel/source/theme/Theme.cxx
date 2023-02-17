/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/theme/Theme.hxx>

#include <utility>
#include <libxml/xmlwriter.h>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <o3tl/enumrange.hxx>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star;

namespace model
{
Theme::Theme() = default;

Theme::Theme(OUString const& rName)
    : maName(rName)
{
}

Theme::Theme(Theme const& rTheme)
    : maName(rTheme.maName)
    , mpColorSet(new ColorSet(*rTheme.GetColorSet()))
    , maFontScheme(rTheme.maFontScheme)
{
}

void Theme::SetColorSet(std::unique_ptr<model::ColorSet> pColorSet)
{
    mpColorSet = std::move(pColorSet);
}

const model::ColorSet* Theme::GetColorSet() const { return mpColorSet.get(); }

model::ColorSet* Theme::GetColorSet() { return mpColorSet.get(); }

void Theme::SetName(const OUString& rName) { maName = rName; }

const OUString& Theme::GetName() const { return maName; }

void Theme::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("Theme"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("maName"),
                                      BAD_CAST(maName.toUtf8().getStr()));

    if (mpColorSet)
    {
        mpColorSet->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

void Theme::ToAny(uno::Any& rVal) const
{
    comphelper::SequenceAsHashMap aMap;
    aMap["Name"] <<= maName;

    if (mpColorSet)
    {
        std::vector<util::Color> aColorScheme;
        for (auto eThemeColorType : o3tl::enumrange<model::ThemeColorType>())
        {
            if (eThemeColorType != model::ThemeColorType::Unknown)
            {
                Color aColor = mpColorSet->getColor(eThemeColorType);
                aColorScheme.push_back(sal_Int32(aColor));
            }
        }

        aMap["ColorSchemeName"] <<= mpColorSet->getName();
        aMap["ColorScheme"] <<= comphelper::containerToSequence(aColorScheme);
    }

    rVal <<= aMap.getAsConstPropertyValueList();
}

std::unique_ptr<Theme> Theme::FromAny(const uno::Any& rVal)
{
    comphelper::SequenceAsHashMap aMap(rVal);
    std::unique_ptr<Theme> pTheme;
    model::ColorSet* pColorSet = nullptr;

    auto it = aMap.find("Name");
    if (it != aMap.end())
    {
        OUString aName;
        it->second >>= aName;
        pTheme = std::make_unique<Theme>(aName);
    }

    it = aMap.find("ColorSchemeName");
    if (it != aMap.end() && pTheme)
    {
        OUString aName;
        it->second >>= aName;
        auto pSet = std::make_unique<model::ColorSet>(aName);
        pTheme->SetColorSet(std::move(pSet));
        pColorSet = pTheme->GetColorSet();
    }

    it = aMap.find("ColorScheme");
    if (it != aMap.end() && pColorSet)
    {
        uno::Sequence<util::Color> aColors;
        it->second >>= aColors;

        SAL_WARN_IF(aColors.size() > 12, "svx",
                    "Theme::FromAny: number of colors greater than max theme colors supported");

        for (auto eThemeColorType : o3tl::enumrange<model::ThemeColorType>())
        {
            if (eThemeColorType != model::ThemeColorType::Unknown)
            {
                size_t nIndex(static_cast<sal_Int16>(eThemeColorType));
                if (nIndex < aColors.size())
                {
                    Color aColor(ColorTransparency, aColors[nIndex]);
                    pColorSet->add(eThemeColorType, aColor);
                }
            }
        }
    }

    return pTheme;
}

std::vector<Color> Theme::GetColors() const
{
    if (!mpColorSet)
        return {};

    std::vector<Color> aColors;
    for (auto eThemeColorType : o3tl::enumrange<model::ThemeColorType>())
    {
        if (eThemeColorType != model::ThemeColorType::Unknown)
            aColors.push_back(mpColorSet->getColor(eThemeColorType));
    }
    return aColors;
}

Color Theme::GetColor(model::ThemeColorType eType) const
{
    if (!mpColorSet)
        return {};

    return mpColorSet->getColor(eType);
}

} // end of namespace model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
