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

#include <utility>

#include <libxml/xmlwriter.h>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <editeng/unoprnms.hxx>
#include <docmodel/uno/UnoThemeColor.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <o3tl/enumrange.hxx>
#include <com/sun/star/util/Color.hpp>

using namespace com::sun::star;

namespace svx
{

ColorSets::ColorSets()
{}

ColorSets::~ColorSets()
{}

void ColorSets::init()
{
    {
        model::ColorSet aColorSet("LibreOffice");
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x000000);
        aColorSet.add(model::ThemeColorType::Light2, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Accent1, 0x18A303);
        aColorSet.add(model::ThemeColorType::Accent2, 0x0369A3);
        aColorSet.add(model::ThemeColorType::Accent3, 0xA33E03);
        aColorSet.add(model::ThemeColorType::Accent4, 0x8E03A3);
        aColorSet.add(model::ThemeColorType::Accent5, 0xC99C00);
        aColorSet.add(model::ThemeColorType::Accent6, 0xC9211E);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x0000EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x551A8B);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet("Rainbow");
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x1C1C1C);
        aColorSet.add(model::ThemeColorType::Light2, 0xDDDDDD);
        aColorSet.add(model::ThemeColorType::Accent1, 0xFF0000);
        aColorSet.add(model::ThemeColorType::Accent2, 0xFF8000);
        aColorSet.add(model::ThemeColorType::Accent3, 0xFFFF00);
        aColorSet.add(model::ThemeColorType::Accent4, 0x00A933);
        aColorSet.add(model::ThemeColorType::Accent5, 0x2A6099);
        aColorSet.add(model::ThemeColorType::Accent6, 0x800080);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x0000EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x551A8B);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet("Beach");
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0xFFBF00);
        aColorSet.add(model::ThemeColorType::Light2, 0x333333);
        aColorSet.add(model::ThemeColorType::Accent1, 0xFFF5CE);
        aColorSet.add(model::ThemeColorType::Accent2, 0xDEE6EF);
        aColorSet.add(model::ThemeColorType::Accent3, 0xE8F2A1);
        aColorSet.add(model::ThemeColorType::Accent4, 0xFFD7D7);
        aColorSet.add(model::ThemeColorType::Accent5, 0xDEE7E5);
        aColorSet.add(model::ThemeColorType::Accent6, 0xDDDBB6);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x7777EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0xEE77D7);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet("Sunset");
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x492300);
        aColorSet.add(model::ThemeColorType::Light2, 0xF6F9D4);
        aColorSet.add(model::ThemeColorType::Accent1, 0xFFFF00);
        aColorSet.add(model::ThemeColorType::Accent2, 0xFFBF00);
        aColorSet.add(model::ThemeColorType::Accent3, 0xFF8000);
        aColorSet.add(model::ThemeColorType::Accent4, 0xFF4000);
        aColorSet.add(model::ThemeColorType::Accent5, 0xBF0041);
        aColorSet.add(model::ThemeColorType::Accent6, 0x800080);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x0000EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x551A8B);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet("Ocean");
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x2A6099);
        aColorSet.add(model::ThemeColorType::Light2, 0xCCCCCC);
        aColorSet.add(model::ThemeColorType::Accent1, 0x800080);
        aColorSet.add(model::ThemeColorType::Accent2, 0x55308D);
        aColorSet.add(model::ThemeColorType::Accent3, 0x2A6099);
        aColorSet.add(model::ThemeColorType::Accent4, 0x158466);
        aColorSet.add(model::ThemeColorType::Accent5, 0x00A933);
        aColorSet.add(model::ThemeColorType::Accent6, 0x81D41A);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x0000EE);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x551A8B);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet("Forest");
        aColorSet.add(model::ThemeColorType::Dark1, 0x000000);
        aColorSet.add(model::ThemeColorType::Light1, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Dark2, 0x000000);
        aColorSet.add(model::ThemeColorType::Light2, 0xFFFFFF);
        aColorSet.add(model::ThemeColorType::Accent1, 0x813709);
        aColorSet.add(model::ThemeColorType::Accent2, 0x224B12);
        aColorSet.add(model::ThemeColorType::Accent3, 0x706E0C);
        aColorSet.add(model::ThemeColorType::Accent4, 0x355269);
        aColorSet.add(model::ThemeColorType::Accent5, 0xBE480A);
        aColorSet.add(model::ThemeColorType::Accent6, 0xBE480A);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x2A6099);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x800080);
        maColorSets.push_back(aColorSet);
    }
    {
        model::ColorSet aColorSet("Breeze");
        aColorSet.add(model::ThemeColorType::Dark1, 0x232629);
        aColorSet.add(model::ThemeColorType::Light1, 0xFCFCFC);
        aColorSet.add(model::ThemeColorType::Dark2, 0x31363B);
        aColorSet.add(model::ThemeColorType::Light2, 0xEFF0F1);
        aColorSet.add(model::ThemeColorType::Accent1, 0xDA4453);
        aColorSet.add(model::ThemeColorType::Accent2, 0xF47750);
        aColorSet.add(model::ThemeColorType::Accent3, 0xFDBC4B);
        aColorSet.add(model::ThemeColorType::Accent4, 0xC9CE3B);
        aColorSet.add(model::ThemeColorType::Accent5, 0x1CDC9A);
        aColorSet.add(model::ThemeColorType::Accent6, 0x2ECC71);
        aColorSet.add(model::ThemeColorType::Hyperlink, 0x1D99F3);
        aColorSet.add(model::ThemeColorType::FollowedHyperlink, 0x3DAEE9);
        maColorSets.push_back(aColorSet);
    }
}

const model::ColorSet& ColorSets::getColorSet(std::u16string_view rName)
{
    for (const model::ColorSet & rColorSet : maColorSets)
    {
        if (rColorSet.getName() == rName)
            return rColorSet;
    }
    return maColorSets[0];
}

void ColorSets::insert(model::ColorSet const& rColorSet)
{
    maColorSets.push_back(rColorSet);
}

Theme::Theme(OUString const& rName)
    : maName(rName)
{
}

void Theme::SetColorSet(std::unique_ptr<model::ColorSet> pColorSet) { mpColorSet = std::move(pColorSet); }

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

void Theme::ToAny(css::uno::Any& rVal) const
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

std::unique_ptr<Theme> Theme::FromAny(const css::uno::Any& rVal)
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

        SAL_WARN_IF(aColors.size() > 12, "svx", "Theme::FromAny: number of colors greater than max theme colors supported");

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

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
