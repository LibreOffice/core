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

#include <sstream>

#include <libxml/xmlwriter.h>

#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <editeng/unoprnms.hxx>
#include <docmodel/uno/UnoThemeColor.hxx>
#include <o3tl/enumrange.hxx>
#include <utility>

using namespace com::sun::star;

namespace
{
/// Updates a text portion to match a new color set, in case it already uses theme colors.
void UpdateTextPortionColorSet(const uno::Reference<beans::XPropertySet>& xPortion,
                               const svx::ColorSet& rColorSet)
{
    if (!xPortion->getPropertySetInfo()->hasPropertyByName(UNO_NAME_EDIT_CHAR_COLOR_THEME_REFERENCE))
        return;

    uno::Reference<util::XThemeColor> xThemeColor;
    xPortion->getPropertyValue(UNO_NAME_EDIT_CHAR_COLOR_THEME_REFERENCE) >>= xThemeColor;
    if (!xThemeColor.is())
        return;

    model::ThemeColor aThemeColor;
    model::theme::setFromXThemeColor(aThemeColor, xThemeColor);

    if (aThemeColor.getType() == model::ThemeColorType::Unknown)
        return;

    Color aColor = rColorSet.getColor(aThemeColor.getType());
    aColor = aThemeColor.applyTransformations(aColor);

    xPortion->setPropertyValue(UNO_NAME_EDIT_CHAR_COLOR, uno::Any(static_cast<sal_Int32>(aColor)));
}

void UpdateFillColorSet(const uno::Reference<beans::XPropertySet>& xShape, const svx::ColorSet& rColorSet)
{
    if (!xShape->getPropertySetInfo()->hasPropertyByName(UNO_NAME_FILLCOLOR_THEME_REFERENCE))
        return;

    uno::Reference<util::XThemeColor> xThemeColor;
    xShape->getPropertyValue(UNO_NAME_FILLCOLOR_THEME_REFERENCE) >>= xThemeColor;
    if (!xThemeColor.is())
        return;

    model::ThemeColor aThemeColor;
    model::theme::setFromXThemeColor(aThemeColor, xThemeColor);

    if (aThemeColor.getType() == model::ThemeColorType::Unknown)
        return;

    Color aColor = rColorSet.getColor(aThemeColor.getType());
    aColor = aThemeColor.applyTransformations(aColor);
    xShape->setPropertyValue(UNO_NAME_FILLCOLOR, uno::Any(static_cast<sal_Int32>(aColor)));
}

void UpdateSdrObject(svx::Theme* pTheme, SdrObject* pObject)
{
    const svx::ColorSet* pColorSet = pTheme->GetColorSet();
    if (!pColorSet)
    {
        return;
    }

    uno::Reference<drawing::XShape> xShape = pObject->getUnoShape();
    uno::Reference<text::XTextRange> xShapeText(xShape, uno::UNO_QUERY);
    if (xShapeText.is())
    {
        // E.g. group shapes have no text.
        uno::Reference<container::XEnumerationAccess> xText(xShapeText->getText(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParagraphs = xText->createEnumeration();
        while (xParagraphs->hasMoreElements())
        {
            uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);
            uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
            while (xPortions->hasMoreElements())
            {
                uno::Reference<beans::XPropertySet> xPortion(xPortions->nextElement(), uno::UNO_QUERY);
                UpdateTextPortionColorSet(xPortion, *pColorSet);
            }
        }
    }

    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    UpdateFillColorSet(xShapeProps, *pColorSet);
}
}

namespace svx
{

ColorSet::ColorSet(OUString const& rName)
    : maName(rName)
{}

void ColorSet::add(model::ThemeColorType eType, Color aColorData)
{
    if (eType == model::ThemeColorType::Unknown)
        return;
    maColors[sal_Int16(eType)] = aColorData;
}

void ColorSet::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ColorSet"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("maName"),
                                      BAD_CAST(maName.toUtf8().getStr()));

    for (const auto& rColor : maColors)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("Color"));
        std::stringstream ss;
        ss << rColor;
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(ss.str().c_str()));
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

ColorSets::ColorSets()
{}

ColorSets::~ColorSets()
{}

void ColorSets::init()
{
    //{
        ColorSet aColorSet("Breeze");
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
    //}
}

const ColorSet& ColorSets::getColorSet(std::u16string_view rName)
{
    for (const ColorSet & rColorSet : maColorSets)
    {
        if (rColorSet.getName() == rName)
            return rColorSet;
    }
    return maColorSets[0];
}

void ColorSets::insert(ColorSet const& rColorSet)
{
    maColorSets.push_back(rColorSet);
}

Theme::Theme(OUString const& rName)
    : maName(rName)
{
}

void Theme::SetColorSet(std::unique_ptr<ColorSet> pColorSet) { mpColorSet = std::move(pColorSet); }

const ColorSet* Theme::GetColorSet() const { return mpColorSet.get(); }

ColorSet* Theme::GetColorSet() { return mpColorSet.get(); }

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
    ColorSet* pColorSet = nullptr;

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
        auto pSet = std::make_unique<ColorSet>(aName);
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

void Theme::UpdateSdrPage(const SdrPage* pPage)
{
    for (size_t nObject = 0; nObject < pPage->GetObjCount(); ++nObject)
    {
        SdrObject* pObject = pPage->GetObj(nObject);
        UpdateSdrObject(this, pObject);
        SdrObjList* pList = pObject->GetSubList();
        if (pList)
        {
            SdrObjListIter aIter(pList, SdrIterMode::DeepWithGroups);
            while (aIter.IsMore())
            {
                UpdateSdrObject(this, aIter.Next());
            }
        }
    }
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
