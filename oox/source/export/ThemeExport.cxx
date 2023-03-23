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

namespace oox
{
ThemeExport::ThemeExport(oox::core::XmlFilterBase* pFilterBase)
    : mpFilterBase(pFilterBase)

{
}

void ThemeExport::write(OUString const& rPath, model::Theme const& rTheme)
{
    sax_fastparser::FSHelperPtr pFS = mpFilterBase->openFragmentStreamWithSerializer(
        rPath, "application/vnd.openxmlformats-officedocument.theme+xml");

    OUString aThemeName = rTheme.GetName();

    pFS->startElementNS(XML_a, XML_theme, FSNS(XML_xmlns, XML_a),
                        mpFilterBase->getNamespaceURL(OOX_NS(dml)), XML_name, aThemeName);

    pFS->startElementNS(XML_a, XML_themeElements);

    const model::ColorSet* pColorSet = rTheme.GetColorSet();

    pFS->startElementNS(XML_a, XML_clrScheme, XML_name, pColorSet->getName());
    writeColorSet(pFS, rTheme);
    pFS->endElementNS(XML_a, XML_clrScheme);

    model::FontScheme const& rFontScheme = rTheme.getFontScheme();
    pFS->startElementNS(XML_a, XML_fontScheme, XML_name, rFontScheme.getName());
    writeFontScheme(pFS, rFontScheme);
    pFS->endElementNS(XML_a, XML_fontScheme);

    model::FormatScheme const& rFormatScheme = rTheme.getFormatScheme();
    pFS->startElementNS(XML_a, XML_fmtScheme);
    writeFormatScheme(pFS, rFormatScheme);
    pFS->endElementNS(XML_a, XML_fmtScheme);

    pFS->endElementNS(XML_a, XML_themeElements);
    pFS->endElementNS(XML_a, XML_theme);

    pFS->endDocument();
}

namespace
{
void fillAttrList(rtl::Reference<sax_fastparser::FastAttributeList> const& pAttrList,
                  model::ThemeFont const& rThemeFont)
{
    if (rThemeFont.maTypeface.isEmpty())
        return;

    pAttrList->add(XML_typeface, rThemeFont.maTypeface);

    if (!rThemeFont.maPanose.isEmpty())
        pAttrList->add(XML_panose, rThemeFont.maPanose);

    pAttrList->add(XML_pitchFamily, OString::number(rThemeFont.getPitchFamily()));
    pAttrList->add(XML_charset, OString::number(rThemeFont.maCharset));
}

} // end anonymous ns

bool ThemeExport::writeFontScheme(sax_fastparser::FSHelperPtr pFS,
                                  model::FontScheme const& rFontScheme)
{
    pFS->startElementNS(XML_a, XML_majorFont);

    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorLatin());
        pFS->singleElementNS(XML_a, XML_latin, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorAsian());
        pFS->singleElementNS(XML_a, XML_ea, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMajorComplex());
        pFS->singleElementNS(XML_a, XML_cs, aAttrList);
    }

    pFS->endElementNS(XML_a, XML_majorFont);

    pFS->startElementNS(XML_a, XML_minorFont);

    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorLatin());
        pFS->singleElementNS(XML_a, XML_latin, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorAsian());
        pFS->singleElementNS(XML_a, XML_ea, aAttrList);
    }
    {
        auto aAttrList = sax_fastparser::FastSerializerHelper::createAttrList();
        fillAttrList(aAttrList, rFontScheme.getMinorComplex());
        pFS->singleElementNS(XML_a, XML_cs, aAttrList);
    }

    pFS->endElementNS(XML_a, XML_minorFont);

    return true;
}

namespace
{
void writeColorTransformations(sax_fastparser::FSHelperPtr pFS,
                               std::vector<model::Transformation> const& rTransformations)
{
    static std::unordered_map<model::TransformationType, sal_Int32> constTransformationTypeTokenMap
        = {
              { model::TransformationType::Tint, XML_tint },
              { model::TransformationType::Shade, XML_shade },
              { model::TransformationType::LumMod, XML_lumMod },
              { model::TransformationType::LumOff, XML_lumOff },
          };

    for (model::Transformation const& rTransformation : rTransformations)
    {
        auto iterator = constTransformationTypeTokenMap.find(rTransformation.meType);
        if (iterator != constTransformationTypeTokenMap.end())
        {
            sal_Int32 nToken = iterator->second;
            pFS->singleElementNS(XML_a, nToken, XML_val,
                                 OString::number(rTransformation.mnValue * 10));
        }
    }
}

void writeColorRGB(sax_fastparser::FSHelperPtr pFS, model::ColorDefinition const& rColorDefinition)
{
    auto aColor = rColorDefinition.getRGBColor();
    pFS->startElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(sal_Int32(aColor)));
    pFS->endElementNS(XML_a, XML_srgbClr);
}

void writeColorCRGB(sax_fastparser::FSHelperPtr pFS, model::ColorDefinition const& rColorDefinition)
{
    pFS->startElementNS(XML_a, XML_scrgbClr, XML_r, OString::number(rColorDefinition.mnComponent1),
                        XML_g, OString::number(rColorDefinition.mnComponent2), XML_b,
                        OString::number(rColorDefinition.mnComponent3));
    writeColorTransformations(pFS, rColorDefinition.maTransformations);
    pFS->endElementNS(XML_a, XML_scrgbClr);
}

void writeColorHSL(sax_fastparser::FSHelperPtr pFS, model::ColorDefinition const& rColorDefinition)
{
    pFS->startElementNS(XML_a, XML_hslClr, XML_hue, OString::number(rColorDefinition.mnComponent1),
                        XML_sat, OString::number(rColorDefinition.mnComponent2), XML_lum,
                        OString::number(rColorDefinition.mnComponent3));
    writeColorTransformations(pFS, rColorDefinition.maTransformations);
    pFS->endElementNS(XML_a, XML_hslClr);
}

void writeColorScheme(sax_fastparser::FSHelperPtr pFS,
                      model::ColorDefinition const& rColorDefinition)
{
    static std::unordered_map<model::ThemeColorType, const char*> constThemeColorTypeTokenMap
        = { { model::ThemeColorType::Dark1, "dk1" },
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
            { model::ThemeColorType::FollowedHyperlink, "folHlink" } };
    auto iterator = constThemeColorTypeTokenMap.find(rColorDefinition.meSchemeType);
    if (iterator != constThemeColorTypeTokenMap.end())
    {
        const char* sValue = iterator->second;
        pFS->startElementNS(XML_a, XML_schemeClr, XML_val, sValue);
        writeColorTransformations(pFS, rColorDefinition.maTransformations);
        pFS->endElementNS(XML_a, XML_schemeClr);
    }
}

void writeColorSystem(sax_fastparser::FSHelperPtr pFS,
                      model::ColorDefinition const& rColorDefinition)
{
    static std::unordered_map<model::SystemColorType, const char*> constThemeColorTypeTokenMap = {
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
        { model::SystemColorType::WindowText, "windowText" },
    };
    auto iterator = constThemeColorTypeTokenMap.find(rColorDefinition.meSystemColorType);
    if (iterator != constThemeColorTypeTokenMap.end())
    {
        const char* sValue = iterator->second;
        pFS->startElementNS(XML_a, XML_sysClr, XML_val, sValue);
        //XML_lastClr
        writeColorTransformations(pFS, rColorDefinition.maTransformations);
        pFS->endElementNS(XML_a, XML_schemeClr);
    }
}

void writeColorPlaceholder(sax_fastparser::FSHelperPtr pFS,
                           model::ColorDefinition const& rColorDefinition)
{
    pFS->startElementNS(XML_a, XML_schemeClr, XML_val, "phClr");
    writeColorTransformations(pFS, rColorDefinition.maTransformations);
    pFS->endElementNS(XML_a, XML_schemeClr);
}

void writeColorDefinition(sax_fastparser::FSHelperPtr pFS,
                          model::ColorDefinition const& rColorDefinition)
{
    switch (rColorDefinition.meType)
    {
        case model::ColorType::Unused:
            break;
        case model::ColorType::RGB:
            writeColorRGB(pFS, rColorDefinition);
            break;
        case model::ColorType::CRGB:
            writeColorCRGB(pFS, rColorDefinition);
            break;
        case model::ColorType::HSL:
            writeColorHSL(pFS, rColorDefinition);
            break;
        case model::ColorType::Scheme:
            writeColorScheme(pFS, rColorDefinition);
            break;
        case model::ColorType::Palette:
            break;
        case model::ColorType::System:
            writeColorSystem(pFS, rColorDefinition);
            break;
        case model::ColorType::Placeholder:
            writeColorPlaceholder(pFS, rColorDefinition);
            break;
    }
}

void writeSolidFill(sax_fastparser::FSHelperPtr pFS, model::SolidFill const& rSolidFill)
{
    pFS->startElementNS(XML_a, XML_solidFill);
    writeColorDefinition(pFS, rSolidFill.maColorDefinition);
    pFS->endElementNS(XML_a, XML_solidFill);
}

void writeRelativeRectangle(sax_fastparser::FSHelperPtr pFS, sal_Int32 nToken,
                            model::RelativeRectangle const& rRelativeRectangle)
{
    pFS->singleElementNS(XML_a, nToken, XML_l, OString::number(rRelativeRectangle.mnLeft), XML_t,
                         OString::number(rRelativeRectangle.mnTop), XML_r,
                         OString::number(rRelativeRectangle.mnRight), XML_b,
                         OString::number(rRelativeRectangle.mnBottom));
}

void writeGradientFill(sax_fastparser::FSHelperPtr pFS, model::GradientFill const& rGradientFill)
{
    pFS->startElementNS(XML_a, XML_gradFill);
    pFS->startElementNS(XML_a, XML_gsLst);
    for (auto const& rStop : rGradientFill.maGradientStops)
    {
        pFS->startElementNS(XML_a, XML_gs, XML_pos,
                            OString::number(sal_Int32(rStop.mfPosition * 100000.0)));
        writeColorDefinition(pFS, rStop.maColor);
        pFS->endElementNS(XML_a, XML_gs);
    }
    pFS->endElementNS(XML_a, XML_gsLst);

    if (rGradientFill.meGradientType == model::GradientType::Linear)
    {
        pFS->singleElementNS(XML_a, XML_lin, XML_ang,
                             OString::number(rGradientFill.maLinearGradient.mnAngle), XML_scaled,
                             rGradientFill.maLinearGradient.mbScaled ? "1" : "0");
    }
    else
    {
        OString sPathType;
        switch (rGradientFill.meGradientType)
        {
            case model::GradientType::Circle:
                sPathType = "circle";
                break;
            case model::GradientType::Rectangle:
                sPathType = "rect";
                break;
            case model::GradientType::Shape:
                sPathType = "shape";
                break;
            default:
                break;
        }

        if (!sPathType.isEmpty())
        {
            pFS->startElementNS(XML_a, XML_path, XML_path, sPathType);
            writeRelativeRectangle(pFS, XML_fillToRect, rGradientFill.maFillToRectangle);
            pFS->endElementNS(XML_a, XML_path);
        }
    }
    writeRelativeRectangle(pFS, XML_tileRect, rGradientFill.maTileRectangle);
    pFS->endElementNS(XML_a, XML_gradFill);
}

void writePatternFill(sax_fastparser::FSHelperPtr pFS, model::PatternFill const& rPatternFill)
{
    OString sPresetType;
    switch (rPatternFill.mePatternPreset)
    {
        case model::PatternPreset::Percent_5:
            sPresetType = "pct5";
            break;
        case model::PatternPreset::Percent_10:
            sPresetType = "pct10";
            break;
        case model::PatternPreset::Percent_20:
            sPresetType = "pct20";
            break;
        case model::PatternPreset::Percent_25:
            sPresetType = "pct25";
            break;
        case model::PatternPreset::Percent_30:
            sPresetType = "pct30";
            break;
        case model::PatternPreset::Percent_40:
            sPresetType = "pct40";
            break;
        case model::PatternPreset::Percent_50:
            sPresetType = "pct50";
            break;
        case model::PatternPreset::Percent_60:
            sPresetType = "pct60";
            break;
        case model::PatternPreset::Percent_70:
            sPresetType = "pct70";
            break;
        case model::PatternPreset::Percent_75:
            sPresetType = "pct75";
            break;
        case model::PatternPreset::Percent_80:
            sPresetType = "pct80";
            break;
        case model::PatternPreset::Percent_90:
            sPresetType = "pct90";
            break;
        case model::PatternPreset::Horizontal:
            sPresetType = "horz";
            break;
        case model::PatternPreset::Vertical:
            sPresetType = "vert";
            break;
        case model::PatternPreset::LightHorizontal:
            sPresetType = "ltHorz";
            break;
        case model::PatternPreset::LightVertical:
            sPresetType = "ltVert";
            break;
        case model::PatternPreset::DarkHorizontal:
            sPresetType = "dkHorz";
            break;
        case model::PatternPreset::DarkVertical:
            sPresetType = "dkVert";
            break;
        case model::PatternPreset::NarrowHorizontal:
            sPresetType = "narHorz";
            break;
        case model::PatternPreset::NarrowVertical:
            sPresetType = "narVert";
            break;
        case model::PatternPreset::DashedHorizontal:
            sPresetType = "dashHorz";
            break;
        case model::PatternPreset::DashedVertical:
            sPresetType = "dashVert";
            break;
        case model::PatternPreset::Cross:
            sPresetType = "cross";
            break;
        case model::PatternPreset::DownwardDiagonal:
            sPresetType = "dnDiag";
            break;
        case model::PatternPreset::UpwardDiagonal:
            sPresetType = "upDiag";
            break;
        case model::PatternPreset::LightDownwardDiagonal:
            sPresetType = "ltDnDiag";
            break;
        case model::PatternPreset::LightUpwardDiagonal:
            sPresetType = "ltUpDiag";
            break;
        case model::PatternPreset::DarkDownwardDiagonal:
            sPresetType = "dkDnDiag";
            break;
        case model::PatternPreset::DarkUpwardDiagonal:
            sPresetType = "dkUpDiag";
            break;
        case model::PatternPreset::WideDownwardDiagonal:
            sPresetType = "wdDnDiag";
            break;
        case model::PatternPreset::WideUpwardDiagonal:
            sPresetType = "wdUpDiag";
            break;
        case model::PatternPreset::DashedDownwardDiagonal:
            sPresetType = "dashDnDiag";
            break;
        case model::PatternPreset::DashedUpwardDiagonal:
            sPresetType = "dashUpDiag";
            break;
        case model::PatternPreset::DiagonalCross:
            sPresetType = "diagCross";
            break;
        case model::PatternPreset::SmallCheckerBoard:
            sPresetType = "smCheck";
            break;
        case model::PatternPreset::LargeCheckerBoard:
            sPresetType = "lgCheck";
            break;
        case model::PatternPreset::SmallGrid:
            sPresetType = "smGrid";
            break;
        case model::PatternPreset::LargeGrid:
            sPresetType = "lgGrid";
            break;
        case model::PatternPreset::DottedGrid:
            sPresetType = "dotGrid";
            break;
        case model::PatternPreset::SmallConfetti:
            sPresetType = "smConfetti";
            break;
        case model::PatternPreset::LargeConfetti:
            sPresetType = "lgConfetti";
            break;
        case model::PatternPreset::HorizontalBrick:
            sPresetType = "horzBrick";
            break;
        case model::PatternPreset::DiagonalBrick:
            sPresetType = "diagBrick";
            break;
        case model::PatternPreset::SolidDiamond:
            sPresetType = "solidDmnd";
            break;
        case model::PatternPreset::OpenDiamond:
            sPresetType = "openDmnd";
            break;
        case model::PatternPreset::DottedDiamond:
            sPresetType = "dotDmnd";
            break;
        case model::PatternPreset::Plaid:
            sPresetType = "plaid";
            break;
        case model::PatternPreset::Sphere:
            sPresetType = "sphere";
            break;
        case model::PatternPreset::Weave:
            sPresetType = "weave";
            break;
        case model::PatternPreset::Divot:
            sPresetType = "divot";
            break;
        case model::PatternPreset::Shingle:
            sPresetType = "shingle";
            break;
        case model::PatternPreset::Wave:
            sPresetType = "wave";
            break;
        case model::PatternPreset::Trellis:
            sPresetType = "trellis";
            break;
        case model::PatternPreset::ZigZag:
            sPresetType = "zigZag";
            break;
        default:
            break;
    }

    if (!sPresetType.isEmpty())
    {
        pFS->startElementNS(XML_a, XML_pattFill, XML_prst, sPresetType);

        pFS->startElementNS(XML_a, XML_fgClr);
        writeColorDefinition(pFS, rPatternFill.maForegroundColor);
        pFS->endElementNS(XML_a, XML_fgClr);

        pFS->startElementNS(XML_a, XML_bgClr);
        writeColorDefinition(pFS, rPatternFill.maBackgroundColor);
        pFS->endElementNS(XML_a, XML_bgClr);

        pFS->endElementNS(XML_a, XML_pattFill);
    }
}

OString convertFlipMode(model::FlipMode eFlipMode)
{
    switch (eFlipMode)
    {
        case model::FlipMode::X:
            return "x";
        case model::FlipMode::Y:
            return "y";
        case model::FlipMode::XY:
            return "xy";
        case model::FlipMode::None:
            return "none";
    }
    return "none";
}

OString convertRectangleAlignment(model::RectangleAlignment eFlipMode)
{
    switch (eFlipMode)
    {
        case model::RectangleAlignment::TopLeft:
            return "tl";
        case model::RectangleAlignment::Top:
            return "t";
        case model::RectangleAlignment::TopRight:
            return "tr";
        case model::RectangleAlignment::Left:
            return "l";
        case model::RectangleAlignment::Center:
            return "ctr";
        case model::RectangleAlignment::Right:
            return "r";
        case model::RectangleAlignment::BottomLeft:
            return "bl";
        case model::RectangleAlignment::Bottom:
            return "b";
        case model::RectangleAlignment::BottomRight:
            return "br";
        case model::RectangleAlignment::Unset:
            break;
    }
    return {};
}

void writeBlip(sax_fastparser::FSHelperPtr pFS, model::BlipFill const& /*rBlipFill*/)
{
    // TODO - reuse WriteXGraphicBlip
    pFS->startElementNS(XML_a, XML_blip);
    pFS->endElementNS(XML_a, XML_blip);
}

void writeBlipFill(sax_fastparser::FSHelperPtr pFS, model::BlipFill const& rBlipFill)
{
    pFS->startElementNS(XML_a, XML_blipFill, XML_rotWithShape,
                        rBlipFill.mbRotateWithShape ? "1" : "0"
                        /*XML_dpi*/);

    writeBlip(pFS, rBlipFill);

    writeRelativeRectangle(pFS, XML_srcRect, rBlipFill.maClipRectangle);

    if (rBlipFill.meMode == model::BitmapMode::Tile)
    {
        OString aFlipMode = convertFlipMode(rBlipFill.meTileFlipMode);
        OString aAlignment = convertRectangleAlignment(rBlipFill.meTileAlignment);

        pFS->startElementNS(XML_a, XML_tile, XML_tx, OString::number(rBlipFill.mnTileOffsetX),
                            XML_ty, OString::number(rBlipFill.mnTileOffsetY), XML_sx,
                            OString::number(rBlipFill.mnTileScaleX), XML_sy,
                            OString::number(rBlipFill.mnTileScaleY), XML_flip, aFlipMode, XML_algn,
                            aAlignment);
        pFS->endElementNS(XML_a, XML_tile);
    }
    else if (rBlipFill.meMode == model::BitmapMode::Stretch)
    {
        pFS->startElementNS(XML_a, XML_stretch);
        writeRelativeRectangle(pFS, XML_fillRect, rBlipFill.maFillRectangle);
        pFS->endElementNS(XML_a, XML_stretch);
    }

    pFS->endElementNS(XML_a, XML_blipFill);
}

void writeFillStyle(sax_fastparser::FSHelperPtr pFS, model::FillStyle const& rFillStyle)
{
    switch (rFillStyle.mpFill->meType)
    {
        case model::FillType::None:
        case model::FillType::Solid:
        {
            auto* pSolidFill = static_cast<model::SolidFill*>(rFillStyle.mpFill.get());
            writeSolidFill(pFS, *pSolidFill);
        }
        break;
        case model::FillType::Gradient:
        {
            auto* pGradientFill = static_cast<model::GradientFill*>(rFillStyle.mpFill.get());
            writeGradientFill(pFS, *pGradientFill);
        }
        break;
        case model::FillType::Pattern:
        {
            auto* pPatternFill = static_cast<model::PatternFill*>(rFillStyle.mpFill.get());
            writePatternFill(pFS, *pPatternFill);
        }
        break;
        case model::FillType::Blip:
        {
            auto* pBlipFill = static_cast<model::BlipFill*>(rFillStyle.mpFill.get());
            writeBlipFill(pFS, *pBlipFill);
        }
        break;
    }
}

void writeBackgroundFillStyle(sax_fastparser::FSHelperPtr pFS, model::FillStyle const& rFillStyle)
{
    writeFillStyle(pFS, rFillStyle);
}

void writeLineStyle(sax_fastparser::FSHelperPtr pFS, model::LineStyle const& rLineStyle)
{
    OString sCap;
    switch (rLineStyle.meCapType)
    {
        case model::CapType::Flat:
            sCap = "flat";
            break;
        case model::CapType::Round:
            sCap = "rnd";
            break;
        case model::CapType::Square:
            sCap = "sq";
            break;
        case model::CapType::Unset:
            break;
    }

    OString sPenAlign;
    switch (rLineStyle.mePenAlignment)
    {
        case model::PenAlignmentType::Center:
            sPenAlign = "ctr";
            break;
        case model::PenAlignmentType::Inset:
            sPenAlign = "in";
            break;
        case model::PenAlignmentType::Unset:
            break;
    }

    OString sCompoundLine;
    switch (rLineStyle.meCompoundLineType)
    {
        case model::CompoundLineType::Single:
            sCompoundLine = "sng";
            break;
        case model::CompoundLineType::Double:
            sCompoundLine = "dbl";
            break;
        case model::CompoundLineType::ThickThin_Double:
            sCompoundLine = "thickThin";
            break;
        case model::CompoundLineType::ThinThick_Double:
            sCompoundLine = "thinThick";
            break;
        case model::CompoundLineType::Triple:
            sCompoundLine = "tri";
            break;
        case model::CompoundLineType::Unset:
            break;
    }

    pFS->startElementNS(XML_a, XML_ln, XML_w, OString::number(rLineStyle.mnWidth), XML_cap,
                        sax_fastparser::UseIf(sCap, !sCap.isEmpty()), XML_cmpd,
                        sax_fastparser::UseIf(sCompoundLine, !sCompoundLine.isEmpty()), XML_algn,
                        sax_fastparser::UseIf(sPenAlign, !sPenAlign.isEmpty()));

    if (rLineStyle.maLineDash.mePresetType != model::PresetDashType::Unset)
    {
        OString sPresetType;
        switch (rLineStyle.maLineDash.mePresetType)
        {
            case model::PresetDashType::Dot:
                sPresetType = "dot";
                break;
            case model::PresetDashType::Dash:
                sPresetType = "dash";
                break;
            case model::PresetDashType::LargeDash:
                sPresetType = "lgDash";
                break;
            case model::PresetDashType::DashDot:
                sPresetType = "dashDot";
                break;
            case model::PresetDashType::LargeDashDot:
                sPresetType = "lgDashDot";
                break;
            case model::PresetDashType::LargeDashDotDot:
                sPresetType = "lgDashDotDot";
                break;
            case model::PresetDashType::Solid:
                sPresetType = "solid";
                break;
            case model::PresetDashType::SystemDash:
                sPresetType = "sysDash";
                break;
            case model::PresetDashType::SystemDot:
                sPresetType = "sysDot";
                break;
            case model::PresetDashType::SystemDashDot:
                sPresetType = "sysDashDot";
                break;
            case model::PresetDashType::SystemDashDotDot:
                sPresetType = "sysDashDotDot";
                break;
            case model::PresetDashType::Unset:
                break;
        }
        pFS->singleElementNS(XML_a, XML_prstDash, XML_val, sPresetType);
    }

    if (rLineStyle.maLineJoin.meType != model::LineJoinType::Unset)
    {
        switch (rLineStyle.maLineJoin.meType)
        {
            case model::LineJoinType::Round:
                pFS->singleElementNS(XML_a, XML_round);
                break;
            case model::LineJoinType::Bevel:
                pFS->singleElementNS(XML_a, XML_bevel);
                break;
            case model::LineJoinType::Miter:
            {
                sal_Int32 nMiterLimit = rLineStyle.maLineJoin.mnMiterLimit;
                pFS->singleElementNS(
                    XML_a, XML_miter, XML_lim,
                    sax_fastparser::UseIf(OString::number(nMiterLimit), nMiterLimit > 0));
            }
            break;
            case model::LineJoinType::Unset:
                break;
        }
    }

    pFS->endElementNS(XML_a, XML_ln);
}

void writeEffectStyle(sax_fastparser::FSHelperPtr pFS, model::EffectStyle const& /*rEffectStyle*/)
{
    pFS->startElementNS(XML_a, XML_effectStyle);
    pFS->singleElementNS(XML_a, XML_effectLst);
    pFS->endElementNS(XML_a, XML_effectStyle);
}

} // end anonymous ns

bool ThemeExport::writeFormatScheme(sax_fastparser::FSHelperPtr pFS,
                                    model::FormatScheme const& rFormatScheme)
{
    // Format Scheme: 3 or more per list but only 3 will be used currently

    // Fill Style List
    rFormatScheme.ensureFillStyleList();
    pFS->startElementNS(XML_a, XML_fillStyleLst);
    for (auto const& rFillStyle : rFormatScheme.getFillStyleList())
    {
        writeFillStyle(pFS, rFillStyle);
    }
    pFS->endElementNS(XML_a, XML_fillStyleLst);

    // Line Style List
    rFormatScheme.ensureLineStyleList();
    pFS->startElementNS(XML_a, XML_lnStyleLst);
    for (auto const& rLineStyle : rFormatScheme.getLineStyleList())
    {
        writeLineStyle(pFS, rLineStyle);
    }
    pFS->endElementNS(XML_a, XML_lnStyleLst);

    // Effect Style List
    rFormatScheme.ensureEffectStyleList();
    pFS->startElementNS(XML_a, XML_effectStyleLst);
    {
        for (auto const& rEffectStyle : rFormatScheme.getEffectStyleList())
        {
            writeEffectStyle(pFS, rEffectStyle);
        }
    }
    pFS->endElementNS(XML_a, XML_effectStyleLst);

    // Background Fill Style List
    rFormatScheme.ensureBackgroundFillStyleList();
    pFS->startElementNS(XML_a, XML_bgFillStyleLst);
    for (auto const& rFillStyle : rFormatScheme.getBackgroundFillStyleList())
    {
        writeBackgroundFillStyle(pFS, rFillStyle);
    }
    pFS->endElementNS(XML_a, XML_bgFillStyleLst);

    return true;
}

bool ThemeExport::writeColorSet(sax_fastparser::FSHelperPtr pFS, model::Theme const& rTheme)
{
    static std::unordered_map<sal_Int32, model::ThemeColorType> constTokenMap
        = { { XML_dk1, model::ThemeColorType::Dark1 },
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
            { XML_folHlink, model::ThemeColorType::FollowedHyperlink } };

    static std::array<sal_Int32, 12> constTokenArray
        = { XML_dk1,     XML_lt1,     XML_dk2,     XML_lt2,     XML_accent1, XML_accent2,
            XML_accent3, XML_accent4, XML_accent5, XML_accent6, XML_hlink,   XML_folHlink };

    const model::ColorSet* pColorSet = rTheme.GetColorSet();
    if (!pColorSet)
        return false;

    for (auto nToken : constTokenArray)
    {
        model::ThemeColorType eColorType = constTokenMap[nToken];
        Color aColor = pColorSet->getColor(eColorType);
        pFS->startElementNS(XML_a, nToken);
        pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(sal_Int32(aColor)));
        pFS->endElementNS(XML_a, nToken);
    }

    return true;
}

} // end namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
