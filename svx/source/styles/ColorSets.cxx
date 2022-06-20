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

using namespace com::sun::star;

namespace
{
/// Updates a text portion to match a new color set, in case it already uses theme colors.
void UpdateTextPortionColorSet(const uno::Reference<beans::XPropertySet>& xPortion,
                               const svx::ColorSet& rColorSet)
{
    sal_Int16 nCharColorTheme = -1;
    xPortion->getPropertyValue(UNO_NAME_EDIT_CHAR_COLOR_THEME) >>= nCharColorTheme;
    if (nCharColorTheme < 0 || nCharColorTheme > 11)
    {
        return;
    }

    Color aColor = rColorSet.getColor(nCharColorTheme);
    sal_Int32 nCharColorLumMod{};
    xPortion->getPropertyValue(UNO_NAME_EDIT_CHAR_COLOR_LUM_MOD) >>= nCharColorLumMod;
    sal_Int32 nCharColorLumOff{};
    xPortion->getPropertyValue(UNO_NAME_EDIT_CHAR_COLOR_LUM_OFF) >>= nCharColorLumOff;
    if (nCharColorLumMod != 10000 || nCharColorLumOff != 0)
    {
        aColor.ApplyLumModOff(nCharColorLumMod, nCharColorLumOff);
    }

    sal_Int32 nCharColorTintOrShade{};
    xPortion->getPropertyValue(UNO_NAME_EDIT_CHAR_COLOR_TINT_OR_SHADE) >>= nCharColorTintOrShade;
    if (nCharColorTintOrShade != 0)
    {
        aColor.ApplyTintOrShade(nCharColorTintOrShade);
    }

    xPortion->setPropertyValue(UNO_NAME_EDIT_CHAR_COLOR,
                               uno::Any(static_cast<sal_Int32>(aColor)));
}

void UpdateFillColorSet(const uno::Reference<beans::XPropertySet>& xShape, const svx::ColorSet& rColorSet)
{
    if (!xShape->getPropertySetInfo()->hasPropertyByName(UNO_NAME_FILLCOLOR_THEME))
    {
        return;
    }

    sal_Int16 nFillColorTheme = -1;
    xShape->getPropertyValue(UNO_NAME_FILLCOLOR_THEME) >>= nFillColorTheme;
    if (nFillColorTheme < 0 || nFillColorTheme > 11)
    {
        return;
    }

    Color aColor = rColorSet.getColor(nFillColorTheme);
    sal_Int32 nFillColorLumMod{};
    xShape->getPropertyValue(UNO_NAME_FILLCOLOR_LUM_MOD) >>= nFillColorLumMod;
    sal_Int32 nFillColorLumOff{};
    xShape->getPropertyValue(UNO_NAME_FILLCOLOR_LUM_OFF) >>= nFillColorLumOff;
    if (nFillColorLumMod != 10000 || nFillColorLumOff != 0)
    {
        aColor.ApplyLumModOff(nFillColorLumMod, nFillColorLumOff);
    }

    xShape->setPropertyValue(UNO_NAME_FILLCOLOR, uno::Any(static_cast<sal_Int32>(aColor)));
}

void UpdateSdrObject(svx::Theme* pTheme, SdrObject* pObject)
{
    svx::ColorSet* pColorSet = pTheme->GetColorSet();
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

ColorSet::ColorSet(OUString const & aColorSetName)
    : maColorSetName(aColorSetName)
    , maColors(12)
{}

ColorSets::ColorSets()
{}

void ColorSet::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ColorSet"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("maColorSetName"),
                                      BAD_CAST(maColorSetName.toUtf8().getStr()));

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

ColorSets::~ColorSets()
{}

void ColorSets::init()
{
    {
        ColorSet aColorSet("Breeze");
        aColorSet.add(0,  0xFCFCFC);
        aColorSet.add(1,  0x232629);
        aColorSet.add(2,  0xEFF0F1);
        aColorSet.add(3,  0x31363B);
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
        ColorSet aColorSet("Material Blue");
        aColorSet.add(0,  0xFFFFFF);
        aColorSet.add(1,  0x212121);
        aColorSet.add(2,  0xECEFF1);
        aColorSet.add(3,  0x37474F);
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
        aColorSet.add(0,  0xFFFFFF);
        aColorSet.add(1,  0x212121);
        aColorSet.add(2,  0xF5F5F5);
        aColorSet.add(3,  0x424242);
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
        aColorSet.add(0,  0xFFFFFF);
        aColorSet.add(1,  0x212121);
        aColorSet.add(2,  0xF5F5F5);
        aColorSet.add(3,  0x424242);
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

const ColorSet& ColorSets::getColorSet(std::u16string_view rName)
{
    for (const ColorSet & rColorSet : maColorSets)
    {
        if (rColorSet.getName() == rName)
            return rColorSet;
    }
    return maColorSets[0];
}

Theme::Theme(const OUString& rName)
    : maName(rName)
{
}

Theme::~Theme() {}

void Theme::SetColorSet(std::unique_ptr<ColorSet> pColorSet) { mpColorSet = std::move(pColorSet); }

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
        for (size_t i = 0; i < 12; ++i)
        {
            aColorScheme.push_back(static_cast<sal_Int32>(mpColorSet->getColor(i)));
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
        for (size_t i = 0; i < aColors.size(); ++i)
        {
            if (i >= 12)
            {
                SAL_WARN("svx", "Theme::FromAny: too many colors in the color set");
                break;
            }

            pColorSet->add(i, Color(ColorTransparency, aColors[i]));
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
    {
        return {};
    }

    std::vector<Color> aColors;
    for (size_t i = 0; i < 12; ++i)
    {
        aColors.push_back(mpColorSet->getColor(i));
    }
    return aColors;
}

Color Theme::GetColor(ThemeColorType eType) const
{
    if (!mpColorSet)
    {
        return {};
    }

    return mpColorSet->getColor(static_cast<size_t>(eType));
}

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
