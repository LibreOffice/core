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
#include <docmodel/uno/UnoTheme.hxx>

#include <utility>
#include <libxml/xmlwriter.h>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <o3tl/enumrange.hxx>
#include <o3tl/underlyingenumvalue.hxx>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/util/XTheme.hpp>

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
    , mpColorSet(new ColorSet(*rTheme.getColorSet()))
    , maFontScheme(rTheme.maFontScheme)
{
}

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
    aMap[u"Name"_ustr] <<= maName;

    if (mpColorSet)
    {
        std::vector<util::Color> aColorScheme;
        for (auto eThemeColorType : o3tl::enumrange<model::ThemeColorType>())
        {
            Color aColor = mpColorSet->getColor(eThemeColorType);
            aColorScheme.push_back(sal_Int32(aColor));
        }

        aMap[u"ColorSchemeName"_ustr] <<= mpColorSet->getName();
        aMap[u"ColorScheme"_ustr] <<= comphelper::containerToSequence(aColorScheme);
    }

    rVal <<= aMap.getAsConstPropertyValueList();
}

static std::shared_ptr<model::ColorSet> makeColorSet(const OUString& name,
                                                     const uno::Sequence<util::Color>& colors)
{
    auto colorset = std::make_shared<model::ColorSet>(name);
    for (auto eThemeColorType : o3tl::enumrange<model::ThemeColorType>())
    {
        const size_t nIndex(o3tl::to_underlying(eThemeColorType));
        if (nIndex >= colors.size())
            break;
        const Color aColor(ColorTransparency, colors[nIndex]);
        colorset->add(eThemeColorType, aColor);
    }
    return colorset;
}

std::shared_ptr<Theme> Theme::FromAny(const uno::Any& rVal)
{
    if (!rVal.hasValue())
        return {};

    if (css::uno::Reference<css::util::XTheme> xTheme; rVal >>= xTheme)
    {
        if (!xTheme)
            return {};

        if (auto* pUnoTheme = dynamic_cast<UnoTheme*>(xTheme.get()))
            return pUnoTheme->getTheme();

        auto pTheme = std::make_shared<Theme>(xTheme->getName());
        if (auto aColors = xTheme->getColorSet(); aColors.hasElements())
            pTheme->setColorSet(makeColorSet(xTheme->getName(), aColors)); // Reuse theme name
        return pTheme;
    }

    comphelper::SequenceAsHashMap aMap(rVal);

    OUString aThemeName;
    if (auto it = aMap.find(u"Name"_ustr); it != aMap.end())
        it->second >>= aThemeName;
    else
        return {};
    auto pTheme = std::make_shared<Theme>(aThemeName);

    if (auto it = aMap.find(u"ColorSchemeName"_ustr); it != aMap.end())
    {
        OUString aColorSchemeName;
        it->second >>= aColorSchemeName;

        pTheme->setColorSet(makeColorSet(
            aColorSchemeName,
            aMap.getUnpackedValueOrDefault(u"ColorScheme"_ustr, uno::Sequence<util::Color>{})));
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
