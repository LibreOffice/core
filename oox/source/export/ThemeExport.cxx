/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/export/ThemeExport.hxx>

#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/utils.hxx>
#include <docmodel/theme/Theme.hxx>
#include <docmodel/theme/FormatScheme.hxx>
#include <sax/fshelper.hxx>
#include <sax/fastattribs.hxx>
#include <unordered_map>
#include <oox/export/drawingml.hxx>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

namespace oox
{
namespace
{
void writeRelativeRectangle(sax_fastparser::FSHelperPtr pFS, sal_Int32 nToken,
                            model::RelativeRectangle const& rRelativeRectangle)
{
    pFS->singleElementNS(XML_a, nToken, XML_l, OString::number(rRelativeRectangle.mnLeft), XML_t,
                         OString::number(rRelativeRectangle.mnTop), XML_r,
                         OString::number(rRelativeRectangle.mnRight), XML_b,
                         OString::number(rRelativeRectangle.mnBottom));
}
} // end anonymous namespace

ThemeExport::ThemeExport(oox::core::XmlFilterBase* pFilterBase,
                         oox::drawingml::DocumentType eDocumentType)
    : mpFilterBase(pFilterBase)
    , meDocumentType(eDocumentType)
{
}

void ThemeExport::write(OUString const& rPath, model::Theme const& rTheme)
{
    mpFS = mpFilterBase->openFragmentStreamWithSerializer(
        rPath, u"application/vnd.openxmlformats-officedocument.theme+xml"_ustr);

    OUString aThemeName = rTheme.GetName();

    mpFS->startElementNS(XML_a, XML_theme, FSNS(XML_xmlns, XML_a),
                         mpFilterBase->getNamespaceURL(OOX_NS(dml)), FSNS(XML_xmlns, XML_r),
                         mpFilterBase->getNamespaceURL(OOX_NS(officeRel)), XML_name, aThemeName);

    mpFS->startElementNS(XML_a, XML_themeElements);

    const auto pColorSet = rTheme.getColorSet();

    mpFS->startElementNS(XML_a, XML_clrScheme, XML_name, pColorSet->getName());
    writeColorSet(rTheme);
    mpFS->endElementNS(XML_a, XML_clrScheme);

    model::FontScheme const& rFontScheme = rTheme.getFontScheme();
    mpFS->startElementNS(XML_a, XML_fontScheme, XML_name, rFontScheme.getName());
    writeFontScheme(rFontScheme);
    mpFS->endElementNS(XML_a, XML_fontScheme);

    model::FormatScheme const& rFormatScheme = rTheme.getFormatScheme();
    mpFS->startElementNS(XML_a, XML_fmtScheme);
    writeFormatScheme(rFormatScheme);
    mpFS->endElementNS(XML_a, XML_fmtScheme);

    mpFS->endElementNS(XML_a, XML_themeElements);
    mpFS->endElementNS(XML_a, XML_theme);

    mpFS->endDocument();
}

namespace
{
void fillAttrList(rtl::Reference<sax_fastparser::FastAttributeList> const& pAttrList,
                  model::ThemeFont const& rThemeFont)
{
    if (rThemeFont.maTypeface.isEmpty())
    {
        pAttrList->add(XML_typeface, ""); // 'typeface' attribute is mandatory
        return;
    }

    pAttrList->add(XML_typeface, rThemeFont.maTypeface);

    if (!rThemeFont.maPanose.isEmpty())
        pAttrList->add(XML_panose, rThemeFont.maPanose);

    pAttrList->add(XML_pitchFamily, OString::number(rThemeFont.getPitchFamily()));
    pAttrList->add(XML_charset, OString::number(rThemeFont.maCharset));
}

} // end anonymous ns

bool ThemeExport::writeFontScheme(model::FontScheme const& rFontScheme)
{
    mpFS->startElementNS(XML_a, XML_majorFont);

    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorLatin());
        mpFS->singleElementNS(XML_a, XML_latin, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorAsian());
        mpFS->singleElementNS(XML_a, XML_ea, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorComplex());
        mpFS->singleElementNS(XML_a, XML_cs, aAttrList);
    }

    mpFS->endElementNS(XML_a, XML_majorFont);

    mpFS->startElementNS(XML_a, XML_minorFont);

    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorLatin());
        mpFS->singleElementNS(XML_a, XML_latin, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorAsian());
        mpFS->singleElementNS(XML_a, XML_ea, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorComplex());
        mpFS->singleElementNS(XML_a, XML_cs, aAttrList);
    }

    mpFS->endElementNS(XML_a, XML_minorFont);

    return true;
}

namespace
{
constexpr frozen::unordered_map<model::TransformationType, sal_Int32, 4> constTransformTypeTokenMap{
    { model::TransformationType::Tint, XML_tint },
    { model::TransformationType::Shade, XML_shade },
    { model::TransformationType::LumMod, XML_lumMod },
    { model::TransformationType::LumOff, XML_lumOff },
};

constexpr frozen::unordered_map<model::ThemeColorType, const char*, 12> constThemeColorTypeTokenMap{
    { model::ThemeColorType::Dark1, "dk1" },
    { model::ThemeColorType::Light1, "lt1" },
    { model::ThemeColorType::Dark2, "dk2" },
    { model::ThemeColorType::Light2, "lt2" },
    { model::ThemeColorType::Accent1, "accent1" },
    { model::ThemeColorType::Accent2, "accent2" },
    { model::ThemeColorType::Accent3, "accent3" },
    { model::ThemeColorType::Accent4, "accent4" },
    { model::ThemeColorType::Accent5, "accent5" },
    { model::ThemeColorType::Accent6, "accent6" },
    { model::ThemeColorType::Hyperlink, "hlink" },
    { model::ThemeColorType::FollowedHyperlink, "folHlink" }
};

constexpr frozen::unordered_map<model::SystemColorType, const char*, 30>
    constSystemColorTypeTokenMap{
        { model::SystemColorType::DarkShadow3D, "3dDkShadow" },
        { model::SystemColorType::Light3D, "3dLight" },
        { model::SystemColorType::ActiveBorder, "activeBorder" },
        { model::SystemColorType::ActiveCaption, "activeCaption" },
        { model::SystemColorType::AppWorkspace, "appWorkspace" },
        { model::SystemColorType::Background, "background" },
        { model::SystemColorType::ButtonFace, "btnFace" },
        { model::SystemColorType::ButtonHighlight, "btnHighlight" },
        { model::SystemColorType::ButtonShadow, "btnShadow" },
        { model::SystemColorType::ButtonText, "btnText" },
        { model::SystemColorType::CaptionText, "captionText" },
        { model::SystemColorType::GradientActiveCaption, "gradientActiveCaption" },
        { model::SystemColorType::GradientInactiveCaption, "gradientInactiveCaption" },
        { model::SystemColorType::GrayText, "grayText" },
        { model::SystemColorType::Highlight, "highlight" },
        { model::SystemColorType::HighlightText, "highlightText" },
        { model::SystemColorType::HotLight, "hotLight" },
        { model::SystemColorType::InactiveBorder, "inactiveBorder" },
        { model::SystemColorType::InactiveCaption, "inactiveCaption" },
        { model::SystemColorType::InactiveCaptionText, "inactiveCaptionText" },
        { model::SystemColorType::InfoBack, "infoBk" },
        { model::SystemColorType::InfoText, "infoText" },
        { model::SystemColorType::Menu, "menu" },
        { model::SystemColorType::MenuBar, "menuBar" },
        { model::SystemColorType::MenuHighlight, "menuHighlight" },
        { model::SystemColorType::MenuText, "menuText" },
        { model::SystemColorType::ScrollBar, "scrollBar" },
        { model::SystemColorType::Window, "window" },
        { model::SystemColorType::WindowFrame, "windowFrame" },
        { model::SystemColorType::WindowText, "windowText" }
    };

constexpr frozen::unordered_map<sal_Int32, model::ThemeColorType, 12> constTokenMap{
    { XML_dk1, model::ThemeColorType::Dark1 },
    { XML_lt1, model::ThemeColorType::Light1 },
    { XML_dk2, model::ThemeColorType::Dark2 },
    { XML_lt2, model::ThemeColorType::Light2 },
    { XML_accent1, model::ThemeColorType::Accent1 },
    { XML_accent2, model::ThemeColorType::Accent2 },
    { XML_accent3, model::ThemeColorType::Accent3 },
    { XML_accent4, model::ThemeColorType::Accent4 },
    { XML_accent5, model::ThemeColorType::Accent5 },
    { XML_accent6, model::ThemeColorType::Accent6 },
    { XML_hlink, model::ThemeColorType::Hyperlink },
    { XML_folHlink, model::ThemeColorType::FollowedHyperlink }
};

} // end anonymous ns

void ThemeExport::writeColorTransformations(
    std::vector<model::Transformation> const& rTransformations)
{
    for (model::Transformation const& rTransformation : rTransformations)
    {
        auto iterator = constTransformTypeTokenMap.find(rTransformation.meType);
        if (iterator != constTransformTypeTokenMap.end())
        {
            sal_Int32 nToken = iterator->second;
            mpFS->singleElementNS(XML_a, nToken, XML_val,
                                  OString::number(rTransformation.mnValue * 10));
        }
    }
}

void ThemeExport::writeColorRGB(model::ComplexColor const& rComplexColor)
{
    auto aColor = rComplexColor.getRGBColor();
    mpFS->startElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(sal_Int32(aColor)));
    mpFS->endElementNS(XML_a, XML_srgbClr);
}

void ThemeExport::writeColorCRGB(model::ComplexColor const& rComplexColor)
{
    mpFS->startElementNS(XML_a, XML_scrgbClr, XML_r,
                         OString::number(sal_Int32(rComplexColor.getRed())), XML_g,
                         OString::number(sal_Int32(rComplexColor.getGreen())), XML_b,
                         OString::number(sal_Int32(rComplexColor.getBlue())));
    writeColorTransformations(rComplexColor.getTransformations());
    mpFS->endElementNS(XML_a, XML_scrgbClr);
}

void ThemeExport::writeColorHSL(model::ComplexColor const& rComplexColor)
{
    mpFS->startElementNS(XML_a, XML_hslClr, XML_hue,
                         OString::number(sal_Int32(rComplexColor.getRed())), XML_sat,
                         OString::number(sal_Int32(rComplexColor.getGreen())), XML_lum,
                         OString::number(sal_Int32(rComplexColor.getBlue())));
    writeColorTransformations(rComplexColor.getTransformations());
    mpFS->endElementNS(XML_a, XML_hslClr);
}

void ThemeExport::writeColorTheme(model::ComplexColor const& rComplexColor)
{
    auto iterator = constThemeColorTypeTokenMap.find(rComplexColor.getThemeColorType());
    if (iterator != constThemeColorTypeTokenMap.end())
    {
        const char* sValue = iterator->second;
        mpFS->startElementNS(XML_a, XML_schemeClr, XML_val, sValue);
        writeColorTransformations(rComplexColor.getTransformations());
        mpFS->endElementNS(XML_a, XML_schemeClr);
    }
}

void ThemeExport::writeColorSystem(model::ComplexColor const& rComplexColor)
{
    auto iterator = constSystemColorTypeTokenMap.find(rComplexColor.getSystemColorType());
    if (iterator != constSystemColorTypeTokenMap.end())
    {
        const char* sValue = iterator->second;
        mpFS->startElementNS(XML_a, XML_sysClr, XML_val, sValue);
        //XML_lastClr
        writeColorTransformations(rComplexColor.getTransformations());
        mpFS->endElementNS(XML_a, XML_schemeClr);
    }
}

void ThemeExport::writeColorPlaceholder(model::ComplexColor const& rComplexColor)
{
    mpFS->startElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
    writeColorTransformations(rComplexColor.getTransformations());
    mpFS->endElementNS(XML_a, XML_schemeClr);
}

void ThemeExport::writeComplexColor(model::ComplexColor const& rComplexColor)
{
    switch (rComplexColor.getType())
    {
        case model::ColorType::Unused:
            break;
        case model::ColorType::RGB:
            writeColorRGB(rComplexColor);
            break;
        case model::ColorType::CRGB:
            writeColorCRGB(rComplexColor);
            break;
        case model::ColorType::HSL:
            writeColorHSL(rComplexColor);
            break;
        case model::ColorType::Theme:
            writeColorTheme(rComplexColor);
            break;
        case model::ColorType::Palette:
            break;
        case model::ColorType::System:
            writeColorSystem(rComplexColor);
            break;
        case model::ColorType::Placeholder:
            writeColorPlaceholder(rComplexColor);
            break;
    }
}

void ThemeExport::writeSolidFill(model::SolidFill const& rSolidFill)
{
    mpFS->startElementNS(XML_a, XML_solidFill);
    writeComplexColor(rSolidFill.maColor);
    mpFS->endElementNS(XML_a, XML_solidFill);
}

void ThemeExport::writeGradientFill(model::GradientFill const& rGradientFill)
{
    mpFS->startElementNS(XML_a, XML_gradFill);
    mpFS->startElementNS(XML_a, XML_gsLst);
    for (auto const& rStop : rGradientFill.maGradientStops)
    {
        mpFS->startElementNS(XML_a, XML_gs, XML_pos,
                             OString::number(sal_Int32(rStop.mfPosition * 100000.0)));
        writeComplexColor(rStop.maColor);
        mpFS->endElementNS(XML_a, XML_gs);
    }
    mpFS->endElementNS(XML_a, XML_gsLst);

    if (rGradientFill.meGradientType == model::GradientType::Linear)
    {
        mpFS->singleElementNS(XML_a, XML_lin, XML_ang,
                              OString::number(rGradientFill.maLinearGradient.mnAngle), XML_scaled,
                              rGradientFill.maLinearGradient.mbScaled ? "1" : "0");
    }
    else
    {
        OString sPathType;
        switch (rGradientFill.meGradientType)
        {
            case model::GradientType::Circle:
                sPathType = "circle"_ostr;
                break;
            case model::GradientType::Rectangle:
                sPathType = "rect"_ostr;
                break;
            case model::GradientType::Shape:
                sPathType = "shape"_ostr;
                break;
            default:
                break;
        }

        if (!sPathType.isEmpty())
        {
            mpFS->startElementNS(XML_a, XML_path, XML_path, sPathType);
            writeRelativeRectangle(mpFS, XML_fillToRect, rGradientFill.maFillToRectangle);
            mpFS->endElementNS(XML_a, XML_path);
        }
    }
    writeRelativeRectangle(mpFS, XML_tileRect, rGradientFill.maTileRectangle);
    mpFS->endElementNS(XML_a, XML_gradFill);
}

void ThemeExport::writePatternFill(model::PatternFill const& rPatternFill)
{
    OString sPresetType;
    switch (rPatternFill.mePatternPreset)
    {
        case model::PatternPreset::Percent_5:
            sPresetType = "pct5"_ostr;
            break;
        case model::PatternPreset::Percent_10:
            sPresetType = "pct10"_ostr;
            break;
        case model::PatternPreset::Percent_20:
            sPresetType = "pct20"_ostr;
            break;
        case model::PatternPreset::Percent_25:
            sPresetType = "pct25"_ostr;
            break;
        case model::PatternPreset::Percent_30:
            sPresetType = "pct30"_ostr;
            break;
        case model::PatternPreset::Percent_40:
            sPresetType = "pct40"_ostr;
            break;
        case model::PatternPreset::Percent_50:
            sPresetType = "pct50"_ostr;
            break;
        case model::PatternPreset::Percent_60:
            sPresetType = "pct60"_ostr;
            break;
        case model::PatternPreset::Percent_70:
            sPresetType = "pct70"_ostr;
            break;
        case model::PatternPreset::Percent_75:
            sPresetType = "pct75"_ostr;
            break;
        case model::PatternPreset::Percent_80:
            sPresetType = "pct80"_ostr;
            break;
        case model::PatternPreset::Percent_90:
            sPresetType = "pct90"_ostr;
            break;
        case model::PatternPreset::Horizontal:
            sPresetType = "horz"_ostr;
            break;
        case model::PatternPreset::Vertical:
            sPresetType = "vert"_ostr;
            break;
        case model::PatternPreset::LightHorizontal:
            sPresetType = "ltHorz"_ostr;
            break;
        case model::PatternPreset::LightVertical:
            sPresetType = "ltVert"_ostr;
            break;
        case model::PatternPreset::DarkHorizontal:
            sPresetType = "dkHorz"_ostr;
            break;
        case model::PatternPreset::DarkVertical:
            sPresetType = "dkVert"_ostr;
            break;
        case model::PatternPreset::NarrowHorizontal:
            sPresetType = "narHorz"_ostr;
            break;
        case model::PatternPreset::NarrowVertical:
            sPresetType = "narVert"_ostr;
            break;
        case model::PatternPreset::DashedHorizontal:
            sPresetType = "dashHorz"_ostr;
            break;
        case model::PatternPreset::DashedVertical:
            sPresetType = "dashVert"_ostr;
            break;
        case model::PatternPreset::Cross:
            sPresetType = "cross"_ostr;
            break;
        case model::PatternPreset::DownwardDiagonal:
            sPresetType = "dnDiag"_ostr;
            break;
        case model::PatternPreset::UpwardDiagonal:
            sPresetType = "upDiag"_ostr;
            break;
        case model::PatternPreset::LightDownwardDiagonal:
            sPresetType = "ltDnDiag"_ostr;
            break;
        case model::PatternPreset::LightUpwardDiagonal:
            sPresetType = "ltUpDiag"_ostr;
            break;
        case model::PatternPreset::DarkDownwardDiagonal:
            sPresetType = "dkDnDiag"_ostr;
            break;
        case model::PatternPreset::DarkUpwardDiagonal:
            sPresetType = "dkUpDiag"_ostr;
            break;
        case model::PatternPreset::WideDownwardDiagonal:
            sPresetType = "wdDnDiag"_ostr;
            break;
        case model::PatternPreset::WideUpwardDiagonal:
            sPresetType = "wdUpDiag"_ostr;
            break;
        case model::PatternPreset::DashedDownwardDiagonal:
            sPresetType = "dashDnDiag"_ostr;
            break;
        case model::PatternPreset::DashedUpwardDiagonal:
            sPresetType = "dashUpDiag"_ostr;
            break;
        case model::PatternPreset::DiagonalCross:
            sPresetType = "diagCross"_ostr;
            break;
        case model::PatternPreset::SmallCheckerBoard:
            sPresetType = "smCheck"_ostr;
            break;
        case model::PatternPreset::LargeCheckerBoard:
            sPresetType = "lgCheck"_ostr;
            break;
        case model::PatternPreset::SmallGrid:
            sPresetType = "smGrid"_ostr;
            break;
        case model::PatternPreset::LargeGrid:
            sPresetType = "lgGrid"_ostr;
            break;
        case model::PatternPreset::DottedGrid:
            sPresetType = "dotGrid"_ostr;
            break;
        case model::PatternPreset::SmallConfetti:
            sPresetType = "smConfetti"_ostr;
            break;
        case model::PatternPreset::LargeConfetti:
            sPresetType = "lgConfetti"_ostr;
            break;
        case model::PatternPreset::HorizontalBrick:
            sPresetType = "horzBrick"_ostr;
            break;
        case model::PatternPreset::DiagonalBrick:
            sPresetType = "diagBrick"_ostr;
            break;
        case model::PatternPreset::SolidDiamond:
            sPresetType = "solidDmnd"_ostr;
            break;
        case model::PatternPreset::OpenDiamond:
            sPresetType = "openDmnd"_ostr;
            break;
        case model::PatternPreset::DottedDiamond:
            sPresetType = "dotDmnd"_ostr;
            break;
        case model::PatternPreset::Plaid:
            sPresetType = "plaid"_ostr;
            break;
        case model::PatternPreset::Sphere:
            sPresetType = "sphere"_ostr;
            break;
        case model::PatternPreset::Weave:
            sPresetType = "weave"_ostr;
            break;
        case model::PatternPreset::Divot:
            sPresetType = "divot"_ostr;
            break;
        case model::PatternPreset::Shingle:
            sPresetType = "shingle"_ostr;
            break;
        case model::PatternPreset::Wave:
            sPresetType = "wave"_ostr;
            break;
        case model::PatternPreset::Trellis:
            sPresetType = "trellis"_ostr;
            break;
        case model::PatternPreset::ZigZag:
            sPresetType = "zigZag"_ostr;
            break;
        default:
            break;
    }

    if (!sPresetType.isEmpty())
    {
        mpFS->startElementNS(XML_a, XML_pattFill, XML_prst, sPresetType);

        mpFS->startElementNS(XML_a, XML_fgClr);
        writeComplexColor(rPatternFill.maForegroundColor);
        mpFS->endElementNS(XML_a, XML_fgClr);

        mpFS->startElementNS(XML_a, XML_bgClr);
        writeComplexColor(rPatternFill.maBackgroundColor);
        mpFS->endElementNS(XML_a, XML_bgClr);

        mpFS->endElementNS(XML_a, XML_pattFill);
    }
}

namespace
{
OString convertFlipMode(model::FlipMode eFlipMode)
{
    switch (eFlipMode)
    {
        case model::FlipMode::X:
            return "x"_ostr;
        case model::FlipMode::Y:
            return "y"_ostr;
        case model::FlipMode::XY:
            return "xy"_ostr;
        case model::FlipMode::None:
            return "none"_ostr;
    }
    return "none"_ostr;
}

OString convertRectangleAlignment(model::RectangleAlignment eFlipMode)
{
    switch (eFlipMode)
    {
        case model::RectangleAlignment::TopLeft:
            return "tl"_ostr;
        case model::RectangleAlignment::Top:
            return "t"_ostr;
        case model::RectangleAlignment::TopRight:
            return "tr"_ostr;
        case model::RectangleAlignment::Left:
            return "l"_ostr;
        case model::RectangleAlignment::Center:
            return "ctr"_ostr;
        case model::RectangleAlignment::Right:
            return "r"_ostr;
        case model::RectangleAlignment::BottomLeft:
            return "bl"_ostr;
        case model::RectangleAlignment::Bottom:
            return "b"_ostr;
        case model::RectangleAlignment::BottomRight:
            return "br"_ostr;
        case model::RectangleAlignment::Unset:
            break;
    }
    return {};
}
} // end anonymous ns

void ThemeExport::writeBlip(model::BlipFill const& rBlipFill)
{
    if (!rBlipFill.mxGraphic.is())
        return;
    oox::drawingml::GraphicExport aExporter(mpFS, mpFilterBase, meDocumentType);
    Graphic aGraphic(rBlipFill.mxGraphic);
    aExporter.writeBlip(aGraphic, rBlipFill.maBlipEffects);
}

void ThemeExport::writeBlipFill(model::BlipFill const& rBlipFill)
{
    mpFS->startElementNS(XML_a, XML_blipFill, XML_rotWithShape,
                         rBlipFill.mbRotateWithShape ? "1" : "0"
                         /*XML_dpi*/);

    writeBlip(rBlipFill);

    writeRelativeRectangle(mpFS, XML_srcRect, rBlipFill.maClipRectangle);

    if (rBlipFill.meMode == model::BitmapMode::Tile)
    {
        OString aFlipMode = convertFlipMode(rBlipFill.meTileFlipMode);
        OString aAlignment = convertRectangleAlignment(rBlipFill.meTileAlignment);

        mpFS->startElementNS(XML_a, XML_tile, XML_tx, OString::number(rBlipFill.mnTileOffsetX),
                             XML_ty, OString::number(rBlipFill.mnTileOffsetY), XML_sx,
                             OString::number(rBlipFill.mnTileScaleX), XML_sy,
                             OString::number(rBlipFill.mnTileScaleY), XML_flip, aFlipMode, XML_algn,
                             aAlignment);
        mpFS->endElementNS(XML_a, XML_tile);
    }
    else if (rBlipFill.meMode == model::BitmapMode::Stretch)
    {
        mpFS->startElementNS(XML_a, XML_stretch);
        writeRelativeRectangle(mpFS, XML_fillRect, rBlipFill.maFillRectangle);
        mpFS->endElementNS(XML_a, XML_stretch);
    }

    mpFS->endElementNS(XML_a, XML_blipFill);
}

void ThemeExport::writeFillStyle(model::FillStyle const& rFillStyle)
{
    switch (rFillStyle.mpFill->meType)
    {
        case model::FillType::None:
        case model::FillType::Solid:
        {
            auto* pSolidFill = static_cast<model::SolidFill*>(rFillStyle.mpFill.get());
            writeSolidFill(*pSolidFill);
        }
        break;
        case model::FillType::Gradient:
        {
            auto* pGradientFill = static_cast<model::GradientFill*>(rFillStyle.mpFill.get());
            writeGradientFill(*pGradientFill);
        }
        break;
        case model::FillType::Pattern:
        {
            auto* pPatternFill = static_cast<model::PatternFill*>(rFillStyle.mpFill.get());
            writePatternFill(*pPatternFill);
        }
        break;
        case model::FillType::Blip:
        {
            auto* pBlipFill = static_cast<model::BlipFill*>(rFillStyle.mpFill.get());
            writeBlipFill(*pBlipFill);
        }
        break;
    }
}

void ThemeExport::writeBackgroundFillStyle(model::FillStyle const& rFillStyle)
{
    writeFillStyle(rFillStyle);
}

void ThemeExport::writeLineStyle(model::LineStyle const& rLineStyle)
{
    OString sCap;
    switch (rLineStyle.meCapType)
    {
        case model::CapType::Flat:
            sCap = "flat"_ostr;
            break;
        case model::CapType::Round:
            sCap = "rnd"_ostr;
            break;
        case model::CapType::Square:
            sCap = "sq"_ostr;
            break;
        case model::CapType::Unset:
            break;
    }

    OString sPenAlign;
    switch (rLineStyle.mePenAlignment)
    {
        case model::PenAlignmentType::Center:
            sPenAlign = "ctr"_ostr;
            break;
        case model::PenAlignmentType::Inset:
            sPenAlign = "in"_ostr;
            break;
        case model::PenAlignmentType::Unset:
            break;
    }

    OString sCompoundLine;
    switch (rLineStyle.meCompoundLineType)
    {
        case model::CompoundLineType::Single:
            sCompoundLine = "sng"_ostr;
            break;
        case model::CompoundLineType::Double:
            sCompoundLine = "dbl"_ostr;
            break;
        case model::CompoundLineType::ThickThin_Double:
            sCompoundLine = "thickThin"_ostr;
            break;
        case model::CompoundLineType::ThinThick_Double:
            sCompoundLine = "thinThick"_ostr;
            break;
        case model::CompoundLineType::Triple:
            sCompoundLine = "tri"_ostr;
            break;
        case model::CompoundLineType::Unset:
            break;
    }

    mpFS->startElementNS(XML_a, XML_ln, XML_w, OString::number(rLineStyle.mnWidth), XML_cap,
                         sax_fastparser::UseIf(sCap, !sCap.isEmpty()), XML_cmpd,
                         sax_fastparser::UseIf(sCompoundLine, !sCompoundLine.isEmpty()), XML_algn,
                         sax_fastparser::UseIf(sPenAlign, !sPenAlign.isEmpty()));

    if (rLineStyle.maLineDash.mePresetType != model::PresetDashType::Unset)
    {
        OString sPresetType;
        switch (rLineStyle.maLineDash.mePresetType)
        {
            case model::PresetDashType::Dot:
                sPresetType = "dot"_ostr;
                break;
            case model::PresetDashType::Dash:
                sPresetType = "dash"_ostr;
                break;
            case model::PresetDashType::LargeDash:
                sPresetType = "lgDash"_ostr;
                break;
            case model::PresetDashType::DashDot:
                sPresetType = "dashDot"_ostr;
                break;
            case model::PresetDashType::LargeDashDot:
                sPresetType = "lgDashDot"_ostr;
                break;
            case model::PresetDashType::LargeDashDotDot:
                sPresetType = "lgDashDotDot"_ostr;
                break;
            case model::PresetDashType::Solid:
                sPresetType = "solid"_ostr;
                break;
            case model::PresetDashType::SystemDash:
                sPresetType = "sysDash"_ostr;
                break;
            case model::PresetDashType::SystemDot:
                sPresetType = "sysDot"_ostr;
                break;
            case model::PresetDashType::SystemDashDot:
                sPresetType = "sysDashDot"_ostr;
                break;
            case model::PresetDashType::SystemDashDotDot:
                sPresetType = "sysDashDotDot"_ostr;
                break;
            case model::PresetDashType::Unset:
                break;
        }
        mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, sPresetType);
    }

    if (rLineStyle.maLineJoin.meType != model::LineJoinType::Unset)
    {
        switch (rLineStyle.maLineJoin.meType)
        {
            case model::LineJoinType::Round:
                mpFS->singleElementNS(XML_a, XML_round);
                break;
            case model::LineJoinType::Bevel:
                mpFS->singleElementNS(XML_a, XML_bevel);
                break;
            case model::LineJoinType::Miter:
            {
                sal_Int32 nMiterLimit = rLineStyle.maLineJoin.mnMiterLimit;
                mpFS->singleElementNS(
                    XML_a, XML_miter, XML_lim,
                    sax_fastparser::UseIf(OString::number(nMiterLimit), nMiterLimit > 0));
            }
            break;
            case model::LineJoinType::Unset:
                break;
        }
    }

    mpFS->endElementNS(XML_a, XML_ln);
}

void ThemeExport::writeEffectStyle(model::EffectStyle const& /*rEffectStyle*/)
{
    mpFS->startElementNS(XML_a, XML_effectStyle);
    mpFS->singleElementNS(XML_a, XML_effectLst);
    mpFS->endElementNS(XML_a, XML_effectStyle);
}

bool ThemeExport::writeFormatScheme(model::FormatScheme const& rFormatScheme)
{
    // Format Scheme: 3 or more per list but only 3 will be used currently

    // Fill Style List
    rFormatScheme.ensureFillStyleList();
    mpFS->startElementNS(XML_a, XML_fillStyleLst);
    for (auto const& rFillStyle : rFormatScheme.getFillStyleList())
    {
        writeFillStyle(rFillStyle);
    }
    mpFS->endElementNS(XML_a, XML_fillStyleLst);

    // Line Style List
    rFormatScheme.ensureLineStyleList();
    mpFS->startElementNS(XML_a, XML_lnStyleLst);
    for (auto const& rLineStyle : rFormatScheme.getLineStyleList())
    {
        writeLineStyle(rLineStyle);
    }
    mpFS->endElementNS(XML_a, XML_lnStyleLst);

    // Effect Style List
    rFormatScheme.ensureEffectStyleList();
    mpFS->startElementNS(XML_a, XML_effectStyleLst);
    {
        for (auto const& rEffectStyle : rFormatScheme.getEffectStyleList())
        {
            writeEffectStyle(rEffectStyle);
        }
    }
    mpFS->endElementNS(XML_a, XML_effectStyleLst);

    // Background Fill Style List
    rFormatScheme.ensureBackgroundFillStyleList();
    mpFS->startElementNS(XML_a, XML_bgFillStyleLst);
    for (auto const& rFillStyle : rFormatScheme.getBackgroundFillStyleList())
    {
        writeBackgroundFillStyle(rFillStyle);
    }
    mpFS->endElementNS(XML_a, XML_bgFillStyleLst);

    return true;
}

bool ThemeExport::writeColorSet(model::Theme const& rTheme)
{
    static const constexpr std::array<sal_Int32, 12> constTokenArray
        = { XML_dk1,     XML_lt1,     XML_dk2,     XML_lt2,     XML_accent1, XML_accent2,
            XML_accent3, XML_accent4, XML_accent5, XML_accent6, XML_hlink,   XML_folHlink };

    const auto pColorSet = rTheme.getColorSet();
    if (!pColorSet)
        return false;

    for (auto nToken : constTokenArray)
    {
        auto iterator = constTokenMap.find(nToken);
        if (iterator != constTokenMap.end())
        {
            model::ThemeColorType eColorType = iterator->second;
            Color aColor = pColorSet->getColor(eColorType);
            mpFS->startElementNS(XML_a, nToken);
            mpFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(sal_Int32(aColor)));
            mpFS->endElementNS(XML_a, nToken);
        }
    }

    return true;
}

} // end namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
