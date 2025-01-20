/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/datatransfer/XTransferableSupplier.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include <memory>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>

#include <comphelper/classids.hxx>
#include <comphelper/processfactory.hxx>
#include <svx/svdpage.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/theme/Theme.hxx>
#include <ThemeColorChanger.hxx>
#include <sot/exchange.hxx>
#include <svx/ColorSets.hxx>
#include <vcl/transfer.hxx>

using namespace css;

namespace
{
class SwCoreThemeTest : public SwModelTestBase
{
public:
    SwCoreThemeTest()
        : SwModelTestBase(u"/sw/qa/core/theme/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testThemeColorInHeading)
{
    createSwDoc("ThemeColorInHeading.docx");

    auto xComplexColor = getProperty<uno::Reference<util::XComplexColor>>(getParagraph(1),
                                                                          u"CharComplexColor"_ustr);
    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testThemeColorInHeadingODT)
{
    createSwDoc("ThemeColorInHeading.fodt");

    auto xComplexColor = getProperty<uno::Reference<util::XComplexColor>>(getParagraph(1),
                                                                          u"CharComplexColor"_ustr);
    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aComplexColor.getThemeColorType());
}

void checkFillStyles(std::vector<model::FillStyle> const& rStyleList)
{
    CPPUNIT_ASSERT_EQUAL(size_t(3), rStyleList.size());
    // Fill style 1
    {
        model::FillStyle const& rFillStyle = rStyleList.at(0);
        CPPUNIT_ASSERT(rFillStyle.mpFill);
        CPPUNIT_ASSERT_EQUAL(model::FillType::Solid, rFillStyle.mpFill->meType);
        auto* pSolidFill = static_cast<model::SolidFill*>(rFillStyle.mpFill.get());
        CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, pSolidFill->maColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSolidFill->maColor.getTransformations().size());
    }

    // Fill style 2
    {
        model::FillStyle const& rFillStyle = rStyleList.at(1);
        CPPUNIT_ASSERT(rFillStyle.mpFill);
        CPPUNIT_ASSERT_EQUAL(model::FillType::Gradient, rFillStyle.mpFill->meType);
        auto* pGradientFill = static_cast<model::GradientFill*>(rFillStyle.mpFill.get());

        // Gradient Properties
        CPPUNIT_ASSERT_EQUAL(model::GradientType::Linear, pGradientFill->meGradientType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5400000), pGradientFill->maLinearGradient.mnAngle);
        CPPUNIT_ASSERT_EQUAL(false, pGradientFill->maLinearGradient.mbScaled);

        // Gradient stops
        CPPUNIT_ASSERT_EQUAL(size_t(3), pGradientFill->maGradientStops.size());
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[0];
            CPPUNIT_ASSERT_EQUAL(0.0, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.getType());
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.getTransformations().size());

            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(11000), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10500), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(6700), rTrasnsformation.mnValue);
            }
        }
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[1];
            CPPUNIT_ASSERT_EQUAL(0.5, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.getType());
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.getTransformations().size());

            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10500), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10300), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(7300), rTrasnsformation.mnValue);
            }
        }
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[2];
            CPPUNIT_ASSERT_EQUAL(1.0, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.getType());
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.getTransformations().size());

            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10500), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10900), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(8100), rTrasnsformation.mnValue);
            }
        }
    }

    // Fill style 3
    {
        model::FillStyle const& rFillStyle = rStyleList.at(2);
        CPPUNIT_ASSERT(rFillStyle.mpFill);
        CPPUNIT_ASSERT_EQUAL(model::FillType::Gradient, rFillStyle.mpFill->meType);
        auto* pGradientFill = static_cast<model::GradientFill*>(rFillStyle.mpFill.get());

        // Gradient Properties
        CPPUNIT_ASSERT_EQUAL(model::GradientType::Linear, pGradientFill->meGradientType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5400000), pGradientFill->maLinearGradient.mnAngle);
        CPPUNIT_ASSERT_EQUAL(false, pGradientFill->maLinearGradient.mbScaled);

        // Gradient Stops
        CPPUNIT_ASSERT_EQUAL(size_t(3), pGradientFill->maGradientStops.size());
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[0];
            CPPUNIT_ASSERT_EQUAL(0.0, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.getType());
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.getTransformations().size());
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10300), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10200), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(9400), rTrasnsformation.mnValue);
            }
        }
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[1];
            CPPUNIT_ASSERT_EQUAL(0.5, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.getType());
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.getTransformations().size());
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(11000), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10000), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Shade, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10000), rTrasnsformation.mnValue);
            }
        }
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[2];
            CPPUNIT_ASSERT_EQUAL(1.0, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.getType());
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.getTransformations().size());
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(9900), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(12000), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.getTransformations()[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Shade, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(7800), rTrasnsformation.mnValue);
            }
        }
    }
}

void checkLineStyles(std::vector<model::LineStyle> const& rStyleList)
{
    CPPUNIT_ASSERT_EQUAL(size_t(3), rStyleList.size());
    // Line style 1
    {
        model::LineStyle const& rLineStyle = rStyleList.at(0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6350), rLineStyle.mnWidth);
        CPPUNIT_ASSERT_EQUAL(model::CapType::Flat, rLineStyle.meCapType);
        CPPUNIT_ASSERT_EQUAL(model::PenAlignmentType::Center, rLineStyle.mePenAlignment);
        CPPUNIT_ASSERT_EQUAL(model::CompoundLineType::Single, rLineStyle.meCompoundLineType);
        CPPUNIT_ASSERT_EQUAL(model::PresetDashType::Solid, rLineStyle.maLineDash.mePresetType);
        CPPUNIT_ASSERT_EQUAL(model::LineJoinType::Miter, rLineStyle.maLineJoin.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(800000), rLineStyle.maLineJoin.mnMiterLimit);

        model::FillStyle const& rFillStyle = rLineStyle.maLineFillStyle;
        CPPUNIT_ASSERT(rFillStyle.mpFill);
        CPPUNIT_ASSERT_EQUAL(model::FillType::Solid, rFillStyle.mpFill->meType);
        auto* pSolidFill = static_cast<model::SolidFill*>(rFillStyle.mpFill.get());
        CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, pSolidFill->maColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSolidFill->maColor.getTransformations().size());
    }

    // Line style 2
    {
        model::LineStyle const& rLineStyle = rStyleList.at(1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(12700), rLineStyle.mnWidth);
        CPPUNIT_ASSERT_EQUAL(model::CapType::Flat, rLineStyle.meCapType);
        CPPUNIT_ASSERT_EQUAL(model::PenAlignmentType::Center, rLineStyle.mePenAlignment);
        CPPUNIT_ASSERT_EQUAL(model::CompoundLineType::Single, rLineStyle.meCompoundLineType);
        CPPUNIT_ASSERT_EQUAL(model::PresetDashType::Solid, rLineStyle.maLineDash.mePresetType);
        CPPUNIT_ASSERT_EQUAL(model::LineJoinType::Miter, rLineStyle.maLineJoin.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(800000), rLineStyle.maLineJoin.mnMiterLimit);

        model::FillStyle const& rFillStyle = rLineStyle.maLineFillStyle;
        CPPUNIT_ASSERT(rFillStyle.mpFill);
        CPPUNIT_ASSERT_EQUAL(model::FillType::Solid, rFillStyle.mpFill->meType);
        auto* pSolidFill = static_cast<model::SolidFill*>(rFillStyle.mpFill.get());
        CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, pSolidFill->maColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSolidFill->maColor.getTransformations().size());
    }

    // Line style 3
    {
        model::LineStyle const& rLineStyle = rStyleList.at(2);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19050), rLineStyle.mnWidth);
        CPPUNIT_ASSERT_EQUAL(model::CapType::Flat, rLineStyle.meCapType);
        CPPUNIT_ASSERT_EQUAL(model::PenAlignmentType::Center, rLineStyle.mePenAlignment);
        CPPUNIT_ASSERT_EQUAL(model::CompoundLineType::Single, rLineStyle.meCompoundLineType);
        CPPUNIT_ASSERT_EQUAL(model::PresetDashType::Solid, rLineStyle.maLineDash.mePresetType);
        CPPUNIT_ASSERT_EQUAL(model::LineJoinType::Miter, rLineStyle.maLineJoin.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(800000), rLineStyle.maLineJoin.mnMiterLimit);

        model::FillStyle const& rFillStyle = rLineStyle.maLineFillStyle;
        CPPUNIT_ASSERT(rFillStyle.mpFill);
        CPPUNIT_ASSERT_EQUAL(model::FillType::Solid, rFillStyle.mpFill->meType);
        auto* pSolidFill = static_cast<model::SolidFill*>(rFillStyle.mpFill.get());
        CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, pSolidFill->maColor.getType());
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSolidFill->maColor.getTransformations().size());
    }
}

void checkEffects(std::vector<model::EffectStyle> const& rEffectStyleList)
{
    CPPUNIT_ASSERT_EQUAL(size_t(3), rEffectStyleList.size());
    {
        model::EffectStyle rEffectStyle = rEffectStyleList[0];
        CPPUNIT_ASSERT_EQUAL(size_t(0), rEffectStyle.maEffectList.size());
    }

    {
        model::EffectStyle rEffectStyle = rEffectStyleList[1];
        CPPUNIT_ASSERT_EQUAL(size_t(0), rEffectStyle.maEffectList.size());
    }

    {
        model::EffectStyle rEffectStyle = rEffectStyleList[2];
        CPPUNIT_ASSERT_EQUAL(size_t(1), rEffectStyle.maEffectList.size());
        model::Effect const& rEffect = rEffectStyle.maEffectList[0];

        CPPUNIT_ASSERT_EQUAL(model::EffectType::OuterShadow, rEffect.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(57150), rEffect.mnBlurRadius);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19050), rEffect.mnDistance);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5400000), rEffect.mnDirection);
        CPPUNIT_ASSERT_EQUAL(model::RectangleAlignment::Center, rEffect.meAlignment);
        CPPUNIT_ASSERT_EQUAL(false, rEffect.mbRotateWithShape);

        CPPUNIT_ASSERT_EQUAL(model::ColorType::RGB, rEffect.maColor.getType());
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, rEffect.maColor.getRGB());

        CPPUNIT_ASSERT_EQUAL(size_t(1), rEffect.maColor.getTransformations().size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::Alpha,
                             rEffect.maColor.getTransformations()[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6300), rEffect.maColor.getTransformations()[0].mnValue);
    }
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testDrawPageThemeExistsDOCX)
{
    createSwDoc("ThemeColorInHeading.docx");
    SwDoc* pDoc = getSwDoc();

    SdrModel* pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    CPPUNIT_ASSERT(pModel);
    auto const& pTheme = pModel->getTheme();
    CPPUNIT_ASSERT(pTheme);
    CPPUNIT_ASSERT_EQUAL(u"Office Theme"_ustr, pTheme->GetName());

    auto pColorSet = pTheme->getColorSet();
    CPPUNIT_ASSERT(pColorSet);
    CPPUNIT_ASSERT_EQUAL(u"Orange"_ustr, pColorSet->getName());

    CPPUNIT_ASSERT_EQUAL(Color(0xE48312), pTheme->GetColor(model::ThemeColorType::Accent1));
    CPPUNIT_ASSERT_EQUAL(Color(0xBD582C), pTheme->GetColor(model::ThemeColorType::Accent2));
    CPPUNIT_ASSERT_EQUAL(Color(0x865640), pTheme->GetColor(model::ThemeColorType::Accent3));
    CPPUNIT_ASSERT_EQUAL(Color(0x9B8357), pTheme->GetColor(model::ThemeColorType::Accent4));
    CPPUNIT_ASSERT_EQUAL(Color(0xC2BC80), pTheme->GetColor(model::ThemeColorType::Accent5));
    CPPUNIT_ASSERT_EQUAL(Color(0x94A088), pTheme->GetColor(model::ThemeColorType::Accent6));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pTheme->GetColor(model::ThemeColorType::Dark1));
    CPPUNIT_ASSERT_EQUAL(Color(0x637052), pTheme->GetColor(model::ThemeColorType::Dark2));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pTheme->GetColor(model::ThemeColorType::Light1));
    CPPUNIT_ASSERT_EQUAL(Color(0xCCDDEA), pTheme->GetColor(model::ThemeColorType::Light2));

    model::FontScheme const& rFontScheme = pTheme->getFontScheme();
    CPPUNIT_ASSERT_EQUAL(u"Calibri Light"_ustr, rFontScheme.getMajorLatin().maTypeface);
    CPPUNIT_ASSERT_EQUAL(u"Calibri"_ustr, rFontScheme.getMinorLatin().maTypeface);
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMajorAsian().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMinorAsian().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMajorComplex().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMinorComplex().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(size_t(47), rFontScheme.getMajorSupplementalFontList().size());
    CPPUNIT_ASSERT_EQUAL(size_t(47), rFontScheme.getMinorSupplementalFontList().size());
    CPPUNIT_ASSERT_EQUAL(u"Angsana New"_ustr, rFontScheme.findMajorSupplementalTypeface(u"Thai"));
    CPPUNIT_ASSERT_EQUAL(u"Cordia New"_ustr, rFontScheme.findMinorSupplementalTypeface(u"Thai"));

    model::FormatScheme const& rFormatScheme = pTheme->getFormatScheme();
    checkFillStyles(rFormatScheme.getFillStyleList());
    checkLineStyles(rFormatScheme.getLineStyleList());
    checkEffects(rFormatScheme.getEffectStyleList());
    CPPUNIT_ASSERT_EQUAL(size_t(3), rFormatScheme.getBackgroundFillStyleList().size());
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testDrawPageThemeExistsODT)
{
    createSwDoc("ThemeColorInHeading.fodt");
    SwDoc* pDoc = getSwDoc();

    SdrModel* pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    CPPUNIT_ASSERT(pModel);
    auto const& pTheme = pModel->getTheme();
    CPPUNIT_ASSERT(pTheme);
    CPPUNIT_ASSERT_EQUAL(u"Office Theme"_ustr, pTheme->GetName());

    auto pColorSet = pTheme->getColorSet();
    CPPUNIT_ASSERT(pColorSet);
    CPPUNIT_ASSERT_EQUAL(u"Orange"_ustr, pColorSet->getName());

    CPPUNIT_ASSERT_EQUAL(Color(0xE48312), pTheme->GetColor(model::ThemeColorType::Accent1));
    CPPUNIT_ASSERT_EQUAL(Color(0xBD582C), pTheme->GetColor(model::ThemeColorType::Accent2));
    CPPUNIT_ASSERT_EQUAL(Color(0x865640), pTheme->GetColor(model::ThemeColorType::Accent3));
    CPPUNIT_ASSERT_EQUAL(Color(0x9B8357), pTheme->GetColor(model::ThemeColorType::Accent4));
    CPPUNIT_ASSERT_EQUAL(Color(0xC2BC80), pTheme->GetColor(model::ThemeColorType::Accent5));
    CPPUNIT_ASSERT_EQUAL(Color(0x94A088), pTheme->GetColor(model::ThemeColorType::Accent6));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, pTheme->GetColor(model::ThemeColorType::Dark1));
    CPPUNIT_ASSERT_EQUAL(Color(0x637052), pTheme->GetColor(model::ThemeColorType::Dark2));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, pTheme->GetColor(model::ThemeColorType::Light1));
    CPPUNIT_ASSERT_EQUAL(Color(0xCCDDEA), pTheme->GetColor(model::ThemeColorType::Light2));
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testThemeChanging)
{
    createSwDoc("ThemeColorInHeading.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SdrModel* pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    CPPUNIT_ASSERT(pModel);

    // Check current theme colors
    {
        auto const& pTheme = pModel->getTheme();
        CPPUNIT_ASSERT(pTheme);
        CPPUNIT_ASSERT_EQUAL(u"Office Theme"_ustr, pTheme->GetName());

        auto pColorSet = pTheme->getColorSet();
        CPPUNIT_ASSERT(pColorSet);
        CPPUNIT_ASSERT_EQUAL(u"Orange"_ustr, pColorSet->getName());
        CPPUNIT_ASSERT_EQUAL(Color(0xE48312), pTheme->GetColor(model::ThemeColorType::Accent1));
    }

    // Change theme colors
    {
        auto const& rColorSets = svx::ColorSets::get();
        auto pNewColorSet
            = std::make_shared<model::ColorSet>(*rColorSets.getColorSet(u"LibreOffice"));
        // check that the theme colors are as expected
        CPPUNIT_ASSERT_EQUAL(u"LibreOffice"_ustr, pNewColorSet->getName());

        sw::ThemeColorChanger aChanger(getSwDocShell());
        aChanger.apply(pNewColorSet);
    }

    // Check new theme colors
    {
        auto const& pTheme = pModel->getTheme();
        CPPUNIT_ASSERT(pTheme);
        CPPUNIT_ASSERT_EQUAL(u"Office Theme"_ustr, pTheme->GetName());

        auto pColorSet = pTheme->getColorSet();
        CPPUNIT_ASSERT(pColorSet);
        CPPUNIT_ASSERT_EQUAL(u"LibreOffice"_ustr, pColorSet->getName());
        CPPUNIT_ASSERT_EQUAL(Color(0x18A303), pTheme->GetColor(model::ThemeColorType::Accent1));
    }

    // Undo
    pWrtShell->Undo();

    // Check theme colors have been reverted
    {
        auto const& pTheme = pModel->getTheme();
        CPPUNIT_ASSERT(pTheme);
        CPPUNIT_ASSERT_EQUAL(u"Office Theme"_ustr, pTheme->GetName());

        auto pColorSet = pTheme->getColorSet();
        CPPUNIT_ASSERT(pColorSet);
        CPPUNIT_ASSERT_EQUAL(u"Orange"_ustr, pColorSet->getName());
        CPPUNIT_ASSERT_EQUAL(Color(0xE48312), pTheme->GetColor(model::ThemeColorType::Accent1));
    }

    // Redo
    pWrtShell->Redo();

    // Check theme colors have been applied again
    {
        auto const& pTheme = pModel->getTheme();
        CPPUNIT_ASSERT(pTheme);
        CPPUNIT_ASSERT_EQUAL(u"Office Theme"_ustr, pTheme->GetName());

        auto pColorSet = pTheme->getColorSet();
        CPPUNIT_ASSERT(pColorSet);
        CPPUNIT_ASSERT_EQUAL(u"LibreOffice"_ustr, pColorSet->getName());
        CPPUNIT_ASSERT_EQUAL(Color(0x18A303), pTheme->GetColor(model::ThemeColorType::Accent1));
    }
}

// A simple transferable, that provides only EMBED_SOURCE and OBJECTDESCRIPTOR flavors, taking
// data from an ODT file. This makes the transferable behave just like clipboard content created
// by Writer in a different instance, taking SwTransferable::PasteOLE path.
class TestSimpleFileTransferable : public cppu::WeakImplHelper<css::datatransfer::XTransferable>
{
public:
    TestSimpleFileTransferable(const OUString& fileURL);
    css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& flavor) override;
    css::uno::Sequence<css::datatransfer::DataFlavor> SAL_CALL getTransferDataFlavors() override;
    sal_Bool SAL_CALL isDataFlavorSupported(const css::datatransfer::DataFlavor& flavor) override;

private:
    OUString m_fileURL;
};

TestSimpleFileTransferable::TestSimpleFileTransferable(const OUString& fileURL)
    : m_fileURL(fileURL)
{
}

css::uno::Any
TestSimpleFileTransferable::getTransferData(const css::datatransfer::DataFlavor& flavor)
{
    if (flavor.MimeType == SotExchange::GetFormatMimeType(SotClipboardFormatId::EMBED_SOURCE))
    {
        auto xSFA(ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext()));
        auto xInputStream = xSFA->openFileRead(m_fileURL);
        sal_Int32 bytes = xInputStream->available();
        css::uno::Sequence<sal_Int8> data;
        CPPUNIT_ASSERT_EQUAL(bytes, xInputStream->readBytes(data, bytes));
        return css::uno::Any(data);
    }
    if (flavor.MimeType == SotExchange::GetFormatMimeType(SotClipboardFormatId::OBJECTDESCRIPTOR))
    {
        TransferableObjectDescriptor aDesc;
        aDesc.maClassName = SvGlobalName(SO3_SW_CLASSID);
        SvMemoryStream aMemStm(1024, 1024);
        WriteTransferableObjectDescriptor(aMemStm, aDesc);
        css::uno::Sequence<sal_Int8> data(static_cast<const sal_Int8*>(aMemStm.GetData()),
                                          aMemStm.GetSize());
        return css::uno::Any(data);
    }
    return {};
}

css::uno::Sequence<css::datatransfer::DataFlavor>
TestSimpleFileTransferable::getTransferDataFlavors()
{
    css::datatransfer::DataFlavor embed_source;
    SotExchange::GetFormatDataFlavor(SotClipboardFormatId::EMBED_SOURCE, embed_source);
    css::datatransfer::DataFlavor objectdescriptor;
    SotExchange::GetFormatDataFlavor(SotClipboardFormatId::OBJECTDESCRIPTOR, objectdescriptor);
    return { embed_source, objectdescriptor };
}

sal_Bool
TestSimpleFileTransferable::isDataFlavorSupported(const css::datatransfer::DataFlavor& flavor)
{
    if (flavor.MimeType == SotExchange::GetFormatMimeType(SotClipboardFormatId::EMBED_SOURCE))
        return true;
    if (flavor.MimeType == SotExchange::GetFormatMimeType(SotClipboardFormatId::OBJECTDESCRIPTOR))
        return true;
    return false;
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testTdf162715_customTransferable)
{
    // Given a document with a custom theme:
    createSwDoc("theme_foo.fodt");

    auto pDoc = getSwDoc();

    auto pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    CPPUNIT_ASSERT(pModel);
    auto pTheme = pModel->getTheme().get();
    CPPUNIT_ASSERT(pTheme);
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pTheme->GetName());
    CPPUNIT_ASSERT_EQUAL(u"colors_foo"_ustr, pTheme->getColorSet()->getName());
    CPPUNIT_ASSERT_EQUAL(Color(0x000080), pTheme->GetColor(model::ThemeColorType::Dark1));
    CPPUNIT_ASSERT_EQUAL(Color(0x008000), pTheme->GetColor(model::ThemeColorType::Dark2));

    // Select all and check the original text in the document:
    auto pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SelAll();
    CPPUNIT_ASSERT_EQUAL(u"Theme foo"_ustr, pWrtShell->GetSelText());

    // Create a transferable from another document with another custom theme,
    // and insert (paste) its content over the selection:
    css::uno::Reference<css::datatransfer::XTransferable> xTransferable(
        new TestSimpleFileTransferable(createFileURL(u"theme_bar.odt")));
    css::uno::Reference<css::frame::XModel> xModel(mxComponent, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::datatransfer::XTransferableSupplier> xTS(
        xModel->getCurrentController(), css::uno::UNO_QUERY_THROW);
    xTS->insertTransferable(xTransferable);

    // Check that the paste is successful (the text has been replaced):
    pWrtShell->SelAll();
    CPPUNIT_ASSERT_EQUAL(u"Theme bar"_ustr, pWrtShell->GetSelText());

    // The original theme must not be replaced.
    pTheme = pModel->getTheme().get();
    CPPUNIT_ASSERT(pTheme);
    // Without the fix, this would fail, because the name was "bar":
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pTheme->GetName());
    CPPUNIT_ASSERT_EQUAL(u"colors_foo"_ustr, pTheme->getColorSet()->getName());
    CPPUNIT_ASSERT_EQUAL(Color(0x000080), pTheme->GetColor(model::ThemeColorType::Dark1));
    CPPUNIT_ASSERT_EQUAL(Color(0x008000), pTheme->GetColor(model::ThemeColorType::Dark2));
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testTdf162715_ownTransferable)
{
    css::uno::Reference<css::datatransfer::XTransferable> xTransferable;
    {
        // Given a document with a custom theme:
        createSwDoc("theme_bar.odt");

        auto pDoc = getSwDoc();

        auto pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
        CPPUNIT_ASSERT(pModel);
        auto pTheme = pModel->getTheme().get();
        CPPUNIT_ASSERT(pTheme);
        CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, pTheme->GetName());
        CPPUNIT_ASSERT_EQUAL(u"colors_bar"_ustr, pTheme->getColorSet()->getName());
        CPPUNIT_ASSERT_EQUAL(Color(0x606000), pTheme->GetColor(model::ThemeColorType::Dark1));
        CPPUNIT_ASSERT_EQUAL(Color(0x800000), pTheme->GetColor(model::ThemeColorType::Dark2));

        // Select all and check the original text in the document:
        auto pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->SelAll();
        CPPUNIT_ASSERT_EQUAL(u"Theme bar"_ustr, pWrtShell->GetSelText());

        // Create a normal Writer's transferable out of the selection:
        css::uno::Reference<css::frame::XModel> xModel(mxComponent, css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::datatransfer::XTransferableSupplier> xTS(
            xModel->getCurrentController(), css::uno::UNO_QUERY_THROW);
        xTransferable = xTS->getTransferable();
    }
    {
        // Open another document with another custom theme:
        createSwDoc("theme_foo.fodt");

        auto pDoc = getSwDoc();

        auto pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
        CPPUNIT_ASSERT(pModel);
        auto pTheme = pModel->getTheme().get();
        CPPUNIT_ASSERT(pTheme);
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pTheme->GetName());
        CPPUNIT_ASSERT_EQUAL(u"colors_foo"_ustr, pTheme->getColorSet()->getName());
        CPPUNIT_ASSERT_EQUAL(Color(0x000080), pTheme->GetColor(model::ThemeColorType::Dark1));
        CPPUNIT_ASSERT_EQUAL(Color(0x008000), pTheme->GetColor(model::ThemeColorType::Dark2));

        // Select all and check the original text in the second document:
        auto pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->SelAll();
        CPPUNIT_ASSERT_EQUAL(u"Theme foo"_ustr, pWrtShell->GetSelText());

        // Insert (paste) the previously created transferable's content over the selection:
        css::uno::Reference<css::frame::XModel> xModel(mxComponent, css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::datatransfer::XTransferableSupplier> xTS(
            xModel->getCurrentController(), css::uno::UNO_QUERY_THROW);
        xTS->insertTransferable(xTransferable);

        // Check that the paste is successful (the text has been replaced):
        pWrtShell->SelAll();
        CPPUNIT_ASSERT_EQUAL(u"Theme bar"_ustr, pWrtShell->GetSelText());

        // The original theme must not be replaced.
        pTheme = pModel->getTheme().get();
        CPPUNIT_ASSERT(pTheme);
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pTheme->GetName());
        CPPUNIT_ASSERT_EQUAL(u"colors_foo"_ustr, pTheme->getColorSet()->getName());
        CPPUNIT_ASSERT_EQUAL(Color(0x000080), pTheme->GetColor(model::ThemeColorType::Dark1));
        CPPUNIT_ASSERT_EQUAL(Color(0x008000), pTheme->GetColor(model::ThemeColorType::Dark2));
    }
}

} // end anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
