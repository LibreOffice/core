/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <svx/svdpage.hxx>
#include <docmodel/uno/UnoThemeColor.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace css;

namespace
{
class SwCoreThemeTest : public SwModelTestBase
{
public:
    SwCoreThemeTest()
        : SwModelTestBase("/sw/qa/core/theme/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testThemeColorInHeading)
{
    createSwDoc("ThemeColorInHeading.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    auto xThemeColor = getProperty<uno::Reference<util::XThemeColor>>(getParagraph(1),
                                                                      "CharColorThemeReference");
    model::ThemeColor aThemeColor;
    model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1, aThemeColor.getType());
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

        CPPUNIT_ASSERT_EQUAL(model::ColorType::RGB, rEffect.maColor.meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rEffect.maColor.mnComponent1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rEffect.maColor.mnComponent2);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rEffect.maColor.mnComponent3);

        CPPUNIT_ASSERT_EQUAL(size_t(1), rEffect.maColor.maTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::Alpha,
                             rEffect.maColor.maTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(6300), rEffect.maColor.maTransformations[0].mnValue);
    }
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testDrawPageThemeExistsDOCX)
{
    createSwDoc("ThemeColorInHeading.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    auto const& pTheme = pPage->getSdrPageProperties().GetTheme();
    CPPUNIT_ASSERT(pTheme);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Office Theme"), pTheme->GetName());

    model::ColorSet* pColorSet = pTheme->GetColorSet();
    CPPUNIT_ASSERT(pColorSet);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Orange"), pColorSet->getName());

    CPPUNIT_ASSERT_EQUAL(Color(0xE48312), pTheme->GetColor(model::ThemeColorType::Accent1));
    CPPUNIT_ASSERT_EQUAL(Color(0xBD582C), pTheme->GetColor(model::ThemeColorType::Accent2));
    CPPUNIT_ASSERT_EQUAL(Color(0x865640), pTheme->GetColor(model::ThemeColorType::Accent3));
    CPPUNIT_ASSERT_EQUAL(Color(0x9B8357), pTheme->GetColor(model::ThemeColorType::Accent4));
    CPPUNIT_ASSERT_EQUAL(Color(0xC2BC80), pTheme->GetColor(model::ThemeColorType::Accent5));
    CPPUNIT_ASSERT_EQUAL(Color(0x94A088), pTheme->GetColor(model::ThemeColorType::Accent6));
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), pTheme->GetColor(model::ThemeColorType::Dark1));
    CPPUNIT_ASSERT_EQUAL(Color(0x637052), pTheme->GetColor(model::ThemeColorType::Dark2));
    CPPUNIT_ASSERT_EQUAL(Color(0xFFFFFF), pTheme->GetColor(model::ThemeColorType::Light1));
    CPPUNIT_ASSERT_EQUAL(Color(0xCCDDEA), pTheme->GetColor(model::ThemeColorType::Light2));

    model::FontScheme const& rFontScheme = pTheme->getFontScheme();
    CPPUNIT_ASSERT_EQUAL(OUString(u"Calibri Light"), rFontScheme.getMajorLatin().maTypeface);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Calibri"), rFontScheme.getMinorLatin().maTypeface);
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMajorAsian().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMinorAsian().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMajorComplex().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(true, rFontScheme.getMinorComplex().maTypeface.isEmpty());
    CPPUNIT_ASSERT_EQUAL(size_t(47), rFontScheme.getMajorSupplementalFontList().size());
    CPPUNIT_ASSERT_EQUAL(size_t(47), rFontScheme.getMinorSupplementalFontList().size());
    CPPUNIT_ASSERT_EQUAL(OUString(u"Angsana New"),
                         rFontScheme.findMajorSupplementalTypeface(u"Thai"));
    CPPUNIT_ASSERT_EQUAL(OUString(u"Cordia New"),
                         rFontScheme.findMinorSupplementalTypeface(u"Thai"));

    model::FormatScheme const& rFormatScheme = pTheme->getFormatScheme();
    CPPUNIT_ASSERT_EQUAL(size_t(3), rFormatScheme.getFillStyleList().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), rFormatScheme.getLineStyleList().size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), rFormatScheme.getBackgroundFillStyleList().size());

    checkEffects(rFormatScheme.getEffectStyleList());

    // Fill style 1
    {
        model::FillStyle const& rFillStyle = rFormatScheme.getFillStyleList().at(0);
        CPPUNIT_ASSERT(rFillStyle.mpFill);
        CPPUNIT_ASSERT_EQUAL(model::FillType::Solid, rFillStyle.mpFill->meType);
        auto* pSolidFill = static_cast<model::SolidFill*>(rFillStyle.mpFill.get());
        CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, pSolidFill->maColorDefinition.meType);
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSolidFill->maColorDefinition.maTransformations.size());
    }

    // Fill style 2
    {
        model::FillStyle const& rFillStyle = rFormatScheme.getFillStyleList().at(1);
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
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.meType);
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.maTransformations.size());

            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(11000), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10500), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(6700), rTrasnsformation.mnValue);
            }
        }
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[1];
            CPPUNIT_ASSERT_EQUAL(0.5, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.meType);
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.maTransformations.size());

            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10500), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10300), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(7300), rTrasnsformation.mnValue);
            }
        }
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[2];
            CPPUNIT_ASSERT_EQUAL(1.0, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.meType);
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.maTransformations.size());

            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10500), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10900), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(8100), rTrasnsformation.mnValue);
            }
        }
    }

    // Fill style 3
    {
        model::FillStyle const& rFillStyle = rFormatScheme.getFillStyleList().at(2);
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
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.meType);
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.maTransformations.size());
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10300), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10200), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Tint, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(9400), rTrasnsformation.mnValue);
            }
        }
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[1];
            CPPUNIT_ASSERT_EQUAL(0.5, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.meType);
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.maTransformations.size());
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(11000), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10000), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Shade, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(10000), rTrasnsformation.mnValue);
            }
        }
        {
            auto const& rGradientStop = pGradientFill->maGradientStops[2];
            CPPUNIT_ASSERT_EQUAL(1.0, rGradientStop.mfPosition);
            CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, rGradientStop.maColor.meType);
            CPPUNIT_ASSERT_EQUAL(size_t(3), rGradientStop.maColor.maTransformations.size());
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[0];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(9900), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[1];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::SatMod, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(12000), rTrasnsformation.mnValue);
            }
            {
                auto const& rTrasnsformation = rGradientStop.maColor.maTransformations[2];
                CPPUNIT_ASSERT_EQUAL(model::TransformationType::Shade, rTrasnsformation.meType);
                CPPUNIT_ASSERT_EQUAL(sal_Int16(7800), rTrasnsformation.mnValue);
            }
        }
    }

    // Line style 1
    {
        model::LineStyle const& rLineStyle = rFormatScheme.getLineStyleList().at(0);
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
        CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, pSolidFill->maColorDefinition.meType);
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSolidFill->maColorDefinition.maTransformations.size());
    }

    // Line style 2
    {
        model::LineStyle const& rLineStyle = rFormatScheme.getLineStyleList().at(1);
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
        CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, pSolidFill->maColorDefinition.meType);
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSolidFill->maColorDefinition.maTransformations.size());
    }

    // Line style 3
    {
        model::LineStyle const& rLineStyle = rFormatScheme.getLineStyleList().at(2);
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
        CPPUNIT_ASSERT_EQUAL(model::ColorType::Placeholder, pSolidFill->maColorDefinition.meType);
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSolidFill->maColorDefinition.maTransformations.size());
    }
}

CPPUNIT_TEST_FIXTURE(SwCoreThemeTest, testDrawPageThemeExistsODT)
{
    createSwDoc("ThemeColorInHeading.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    auto const& pTheme = pPage->getSdrPageProperties().GetTheme();
    CPPUNIT_ASSERT(pTheme);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Office Theme"), pTheme->GetName());

    model::ColorSet* pColorSet = pTheme->GetColorSet();
    CPPUNIT_ASSERT(pColorSet);
    CPPUNIT_ASSERT_EQUAL(OUString(u"Orange"), pColorSet->getName());

    CPPUNIT_ASSERT_EQUAL(Color(0xE48312), pTheme->GetColor(model::ThemeColorType::Accent1));
    CPPUNIT_ASSERT_EQUAL(Color(0xBD582C), pTheme->GetColor(model::ThemeColorType::Accent2));
    CPPUNIT_ASSERT_EQUAL(Color(0x865640), pTheme->GetColor(model::ThemeColorType::Accent3));
    CPPUNIT_ASSERT_EQUAL(Color(0x9B8357), pTheme->GetColor(model::ThemeColorType::Accent4));
    CPPUNIT_ASSERT_EQUAL(Color(0xC2BC80), pTheme->GetColor(model::ThemeColorType::Accent5));
    CPPUNIT_ASSERT_EQUAL(Color(0x94A088), pTheme->GetColor(model::ThemeColorType::Accent6));
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), pTheme->GetColor(model::ThemeColorType::Dark1));
    CPPUNIT_ASSERT_EQUAL(Color(0x637052), pTheme->GetColor(model::ThemeColorType::Dark2));
    CPPUNIT_ASSERT_EQUAL(Color(0xFFFFFF), pTheme->GetColor(model::ThemeColorType::Light1));
    CPPUNIT_ASSERT_EQUAL(Color(0xCCDDEA), pTheme->GetColor(model::ThemeColorType::Light2));
}

} // end anonymous namnespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
