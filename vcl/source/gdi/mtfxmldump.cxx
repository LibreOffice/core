/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/mtfxmldump.hxx>
#include <tools/XmlWriter.hxx>
#include <tools/fract.hxx>

#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>

#include <sstream>

namespace
{

OUString collectPushFlags(vcl::PushFlags nFlags)
{
    if ((nFlags & vcl::PushFlags::ALL) == vcl::PushFlags::ALL)
        return "PushAll";
    else if ((nFlags & PUSH_ALLFONT) == PUSH_ALLFONT)
        return "PushAllFont";

    std::vector<OUString> aStrings;

    if (nFlags & vcl::PushFlags::LINECOLOR)
        aStrings.emplace_back("PushLineColor");
    if (nFlags & vcl::PushFlags::FILLCOLOR)
        aStrings.emplace_back("PushFillColor");
    if (nFlags & vcl::PushFlags::FONT)
        aStrings.emplace_back("PushFont");
    if (nFlags & vcl::PushFlags::TEXTCOLOR)
        aStrings.emplace_back("PushTextColor");
    if (nFlags & vcl::PushFlags::MAPMODE)
        aStrings.emplace_back("PushMapMode");
    if (nFlags & vcl::PushFlags::CLIPREGION)
        aStrings.emplace_back("PushClipRegion");
    if (nFlags & vcl::PushFlags::RASTEROP)
        aStrings.emplace_back("PushRasterOp");
    if (nFlags & vcl::PushFlags::TEXTFILLCOLOR)
        aStrings.emplace_back("PushTextFillColor");
    if (nFlags & vcl::PushFlags::TEXTALIGN)
        aStrings.emplace_back("PushTextAlign");
    if (nFlags & vcl::PushFlags::REFPOINT)
        aStrings.emplace_back("PushRefPoint");
    if (nFlags & vcl::PushFlags::TEXTLINECOLOR)
        aStrings.emplace_back("PushTextLineColor");
    if (nFlags & vcl::PushFlags::TEXTLAYOUTMODE)
        aStrings.emplace_back("PushTextLayoutMode");
    if (nFlags & vcl::PushFlags::TEXTLANGUAGE)
        aStrings.emplace_back("PushTextLanguage");
    if (nFlags & vcl::PushFlags::OVERLINECOLOR)
        aStrings.emplace_back("PushOverlineColor");

    OUString aString;

    if (aStrings.empty())
        return aString;

    aString = aStrings[0];
    for (size_t i = 1; i < aStrings.size(); ++i)
    {
        aString += ", " + aStrings[i];
    }
    return aString;
}

OUString convertDrawTextFlagsToString(DrawTextFlags eDrawTextFlags)
{
    std::vector<OUString> aStrings;
    if (eDrawTextFlags & DrawTextFlags::Disable)
        aStrings.emplace_back("Disable");
    if (eDrawTextFlags & DrawTextFlags::Mnemonic)
        aStrings.emplace_back("Mnemonic");
    if (eDrawTextFlags & DrawTextFlags::Mono)
        aStrings.emplace_back("Mono");
    if (eDrawTextFlags & DrawTextFlags::Clip)
        aStrings.emplace_back("Clip");
    if (eDrawTextFlags & DrawTextFlags::Left)
        aStrings.emplace_back("Left");
    if (eDrawTextFlags & DrawTextFlags::Center)
        aStrings.emplace_back("Center");
    if (eDrawTextFlags & DrawTextFlags::Right)
        aStrings.emplace_back("Right");
    if (eDrawTextFlags & DrawTextFlags::Top)
        aStrings.emplace_back("Top");
    if (eDrawTextFlags & DrawTextFlags::VCenter)
        aStrings.emplace_back("VCenter");
    if (eDrawTextFlags & DrawTextFlags::Bottom)
        aStrings.emplace_back("Bottom");
    if (eDrawTextFlags & DrawTextFlags::EndEllipsis)
        aStrings.emplace_back("EndEllipsis");
    if (eDrawTextFlags & DrawTextFlags::PathEllipsis)
        aStrings.emplace_back("PathEllipsis");
    if (eDrawTextFlags & DrawTextFlags::MultiLine)
        aStrings.emplace_back("MultiLine");
    if (eDrawTextFlags & DrawTextFlags::WordBreak)
        aStrings.emplace_back("WordBreak");
    if (eDrawTextFlags & DrawTextFlags::NewsEllipsis)
        aStrings.emplace_back("NewsEllipsis");
    if (eDrawTextFlags & DrawTextFlags::WordBreakHyphenation)
        aStrings.emplace_back("WordBreakHyphenation");
    if (eDrawTextFlags & DrawTextFlags::CenterEllipsis)
        aStrings.emplace_back("CenterEllipsis");
    if (eDrawTextFlags & DrawTextFlags::HideMnemonic)
        aStrings.emplace_back("HideMnemonic");

    OUString aString;

    if (aStrings.empty())
        return "None";

    aString = aStrings[0];
    for (size_t i = 1; i < aStrings.size(); ++i)
    {
        aString += " " + aStrings[i];
    }
    return aString;
};

OUString convertRopToString(RasterOp eRop)
{
    switch (eRop)
    {
        case RasterOp::OverPaint: return "overpaint";
        case RasterOp::Xor:       return "xor";
        case RasterOp::N0:        return "0";
        case RasterOp::N1:        return "1";
        case RasterOp::Invert:    return "invert";
    }
    return OUString();
}

OUString convertTextAlignToString(TextAlign eAlign)
{
    switch (eAlign)
    {
        case ALIGN_BASELINE:             return "baseline";
        case ALIGN_BOTTOM:               return "bottom";
        case ALIGN_TOP:                  return "top";
        case TextAlign_FORCE_EQUAL_SIZE: return "equalsize";
    }
    return OUString();
}

OUString convertColorToString(Color aColor)
{
    OUString aRGBString = aColor.AsRGBHexString();
    return "#" + aRGBString;
}

OUString convertLineStyleToString(LineStyle eAlign)
{
    switch (eAlign)
    {
        case LineStyle::NONE:    return "none";
        case LineStyle::Solid:   return "solid";
        case LineStyle::Dash:    return "dash";
        default: break;
    }
    return OUString();
}

OUString convertLineJoinToString(basegfx::B2DLineJoin eJoin)
{
    switch (eJoin)
    {
        default:
        case basegfx::B2DLineJoin::NONE:    return "none";
        case basegfx::B2DLineJoin::Bevel:   return "bevel";
        case basegfx::B2DLineJoin::Miter:   return "miter";
        case basegfx::B2DLineJoin::Round:   return "round";
    }
}

OUString convertLineCapToString(css::drawing::LineCap eCap)
{
    switch (eCap)
    {
        default:
        case css::drawing::LineCap_BUTT:   return "butt";
        case css::drawing::LineCap_ROUND:  return "round";
        case css::drawing::LineCap_SQUARE: return "square";
    }
}

OUString convertPolygonFlags(PolyFlags eFlags)
{
    switch (eFlags)
    {
        default:
        case PolyFlags::Normal:    return "normal";
        case PolyFlags::Control:   return "control";
        case PolyFlags::Smooth:    return "smooth";
        case PolyFlags::Symmetric: return "symmetric";
    }
}

OUString convertFontWeightToString(FontWeight eFontWeight)
{
    switch (eFontWeight)
    {
        case WEIGHT_DONTKNOW:   return "unknown";
        case WEIGHT_THIN:       return "thin";
        case WEIGHT_ULTRALIGHT: return "ultralight";
        case WEIGHT_LIGHT:      return "light";
        case WEIGHT_SEMILIGHT:  return "semilight";
        case WEIGHT_NORMAL:     return "normal";
        case WEIGHT_MEDIUM:     return "medium";
        case WEIGHT_SEMIBOLD:   return "semibold";
        case WEIGHT_BOLD:       return "bold";
        case WEIGHT_ULTRABOLD:  return "ultrabold";
        case WEIGHT_BLACK:      return "black";
        case FontWeight_FORCE_EQUAL_SIZE: return "equalsize";
    }
    return OUString();
}

OUString convertFontStrikeoutToString(FontStrikeout eFontStrikeout)
{
    switch (eFontStrikeout)
    {
        case STRIKEOUT_NONE:     return "none";
        case STRIKEOUT_SINGLE:   return "single";
        case STRIKEOUT_DOUBLE:   return "double";
        case STRIKEOUT_DONTKNOW: return "dontknow";
        case STRIKEOUT_BOLD:     return "bold";
        case STRIKEOUT_SLASH:    return "slash";
        case STRIKEOUT_X:        return "x";
        case FontStrikeout_FORCE_EQUAL_SIZE: return "equalsize";
    }
    return OUString();
}

OUString convertFontLineStyleToString(FontLineStyle eFontLineStyle)
{
    switch (eFontLineStyle)
    {
        case LINESTYLE_NONE:            return "none";
        case LINESTYLE_SINGLE:          return "single";
        case LINESTYLE_DOUBLE:          return "double";
        case LINESTYLE_DOTTED:          return "dotted";
        case LINESTYLE_DONTKNOW:        return "dontknow";
        case LINESTYLE_DASH:            return "dash";
        case LINESTYLE_LONGDASH:        return "longdash";
        case LINESTYLE_DASHDOT:         return "dashdot";
        case LINESTYLE_DASHDOTDOT:      return "dashdotdot";
        case LINESTYLE_SMALLWAVE:       return "smallwave";
        case LINESTYLE_WAVE:            return "wave";
        case LINESTYLE_DOUBLEWAVE:      return "doublewave";
        case LINESTYLE_BOLD:            return "bold";
        case LINESTYLE_BOLDDOTTED:      return "bolddotted";
        case LINESTYLE_BOLDDASH:        return "bolddash";
        case LINESTYLE_BOLDLONGDASH:    return "boldlongdash";
        case LINESTYLE_BOLDDASHDOT:     return "bolddashdot";
        case LINESTYLE_BOLDDASHDOTDOT:  return "bolddashdotdot";
        case LINESTYLE_BOLDWAVE:        return "boldwave";
        case FontLineStyle_FORCE_EQUAL_SIZE: return "equalsize";
    }
    return OUString();
}

OString convertLineStyleToString(const MetaActionType nActionType)
{
    switch (nActionType)
    {
        case MetaActionType::NONE:                  return "null";
        case MetaActionType::PIXEL:                 return "pixel";
        case MetaActionType::POINT:                 return "point";
        case MetaActionType::LINE:                  return "line";
        case MetaActionType::RECT:                  return "rect";
        case MetaActionType::ROUNDRECT:             return "roundrect";
        case MetaActionType::ELLIPSE:               return "ellipse";
        case MetaActionType::ARC:                   return "arc";
        case MetaActionType::PIE:                   return "pie";
        case MetaActionType::CHORD:                 return "chord";
        case MetaActionType::POLYLINE:              return "polyline";
        case MetaActionType::POLYGON:               return "polygon";
        case MetaActionType::POLYPOLYGON:           return "polypolygon";
        case MetaActionType::TEXT:                  return "text";
        case MetaActionType::TEXTARRAY:             return "textarray";
        case MetaActionType::STRETCHTEXT:           return "stretchtext";
        case MetaActionType::TEXTRECT:              return "textrect";
        case MetaActionType::TEXTLINE:              return "textline";
        case MetaActionType::BMP:                   return "bmp";
        case MetaActionType::BMPSCALE:              return "bmpscale";
        case MetaActionType::BMPSCALEPART:          return "bmpscalepart";
        case MetaActionType::BMPEX:                 return "bmpex";
        case MetaActionType::BMPEXSCALE:            return "bmpexscale";
        case MetaActionType::BMPEXSCALEPART:        return "bmpexscalepart";
        case MetaActionType::MASK:                  return "mask";
        case MetaActionType::MASKSCALE:             return "maskscale";
        case MetaActionType::MASKSCALEPART:         return "maskscalepart";
        case MetaActionType::GRADIENT:              return "gradient";
        case MetaActionType::GRADIENTEX:            return "gradientex";
        case MetaActionType::HATCH:                 return "hatch";
        case MetaActionType::WALLPAPER:             return "wallpaper";
        case MetaActionType::CLIPREGION:            return "clipregion";
        case MetaActionType::ISECTRECTCLIPREGION:   return "sectrectclipregion";
        case MetaActionType::ISECTREGIONCLIPREGION: return "sectregionclipregion";
        case MetaActionType::MOVECLIPREGION:        return "moveclipregion";
        case MetaActionType::LINECOLOR:             return "linecolor";
        case MetaActionType::FILLCOLOR:             return "fillcolor";
        case MetaActionType::TEXTCOLOR:             return "textcolor";
        case MetaActionType::TEXTFILLCOLOR:         return "textfillcolor";
        case MetaActionType::TEXTLINECOLOR:         return "textlinecolor";
        case MetaActionType::OVERLINECOLOR:         return "overlinecolor";
        case MetaActionType::TEXTALIGN:             return "textalign";
        case MetaActionType::MAPMODE:               return "mapmode";
        case MetaActionType::FONT:                  return "font";
        case MetaActionType::PUSH:                  return "push";
        case MetaActionType::POP:                   return "pop";
        case MetaActionType::RASTEROP:              return "rasterop";
        case MetaActionType::Transparent:           return "transparent";
        case MetaActionType::FLOATTRANSPARENT:      return "floattransparent";
        case MetaActionType::EPS:                   return "eps";
        case MetaActionType::REFPOINT:              return "refpoint";
        case MetaActionType::COMMENT:               return "comment";
        case MetaActionType::LAYOUTMODE:            return "layoutmode";
        case MetaActionType::TEXTLANGUAGE:          return "textlanguage";
        case MetaActionType::FILLMODE:             return "fillmode";
    }
    return "";
}

OUString convertBitmapExTransparentType(BitmapEx const & rBitmapEx)
{
    if (rBitmapEx.IsAlpha())
        return "bitmap";
    else
        return "none";
}

OUString convertMapUnitToString(MapUnit eUnit)
{
    switch (eUnit)
    {
        default:
        case MapUnit::LASTENUMDUMMY:    return "LASTENUMDUMMY";
        case MapUnit::Map1000thInch:    return "Map1000thInch";
        case MapUnit::Map100thInch:     return "Map100thInch";
        case MapUnit::Map100thMM:       return "Map100thMM";
        case MapUnit::Map10thInch:      return "Map10thInch";
        case MapUnit::Map10thMM:        return "Map10thMM";
        case MapUnit::MapAppFont:       return "MapAppFont";
        case MapUnit::MapCM:            return "MapCM";
        case MapUnit::MapInch:          return "MapInch";
        case MapUnit::MapMM:            return "MapMM";
        case MapUnit::MapPixel:         return "MapPixel";
        case MapUnit::MapPoint:         return "MapPoint";
        case MapUnit::MapRelative:      return "MapRelative";
        case MapUnit::MapSysFont:       return "MapSysFont";
        case MapUnit::MapTwip:          return "MapTwip";
    }
}

OUString convertFractionToString(const Fraction& aFraction)
{
    std::stringstream ss;

    ss << aFraction;

    return OUString::createFromAscii(ss.str().c_str());
}

OUString convertGradientStyle(GradientStyle eStyle)
{
    switch (eStyle)
    {
        case GradientStyle::Linear:     return "Linear";
        case GradientStyle::Axial:      return "Axial";
        case GradientStyle::Radial:     return "Radial";
        case GradientStyle::Elliptical: return "Elliptical";
        case GradientStyle::Square:     return "Square";
        case GradientStyle::Rect:       return "Rect";
        case GradientStyle::FORCE_EQUAL_SIZE: return "ForceEqualSize";
    }
    return OUString();
}

OUString convertHatchStyle(HatchStyle eStyle)
{
    switch (eStyle)
    {
        case HatchStyle::Single: return "Single";
        case HatchStyle::Double: return "Double";
        case HatchStyle::Triple: return "Triple";
        case HatchStyle::FORCE_EQUAL_SIZE: return "ForceEqualSize";
    }
    return OUString();
}

OUString convertLanguageTypeToString(LanguageType rLanguageType)
{
    std::stringstream ss;
    ss << std::hex << std::setfill ('0') << std::setw(4) << rLanguageType.get();
    return "#" + OUString::createFromAscii(ss.str().c_str());
}

OUString convertWallpaperStyleToString(WallpaperStyle eWallpaperStyle)
{
    switch (eWallpaperStyle)
    {
        case WallpaperStyle::NONE: return "NONE";
        case WallpaperStyle::Tile: return "Tile";
        case WallpaperStyle::Center: return "Center";
        case WallpaperStyle::Scale: return "Scale";
        case WallpaperStyle::TopLeft: return "TopLeft";
        case WallpaperStyle::Top: return "Top";
        case WallpaperStyle::TopRight: return "TopRight";
        case WallpaperStyle::Left: return "Left";
        case WallpaperStyle::Right: return "Right";
        case WallpaperStyle::BottomLeft: return "BottomLeft";
        case WallpaperStyle::Bottom: return "Bottom";
        case WallpaperStyle::BottomRight: return "BottomRight";
        case WallpaperStyle::ApplicationGradient: return "ApplicationGradient";
    }
    return OUString();
}

OUString convertPixelFormatToString(vcl::PixelFormat ePixelFormat)
{
    switch (ePixelFormat)
    {
        case vcl::PixelFormat::INVALID: return "INVALID";
        case vcl::PixelFormat::N1_BPP: return "1BPP";
        case vcl::PixelFormat::N8_BPP: return "8BPP";
        case vcl::PixelFormat::N24_BPP: return "24BPP";
        case vcl::PixelFormat::N32_BPP: return "32BPP";
    }
    return OUString();
}

OUString convertComplexTestLayoutFlags(vcl::text::ComplexTextLayoutFlags eComplexTestLayoutFlags)
{
    switch(eComplexTestLayoutFlags)
    {
        default:
        case vcl::text::ComplexTextLayoutFlags::Default: return "#0000";
        case vcl::text::ComplexTextLayoutFlags::BiDiRtl: return "#0001";
        case vcl::text::ComplexTextLayoutFlags::BiDiStrong: return "#0002";
        case vcl::text::ComplexTextLayoutFlags::TextOriginLeft: return "#0004";
        case vcl::text::ComplexTextLayoutFlags::TextOriginRight: return "#0008";
    }
}

OUString convertGfxLinkTypeToString(GfxLinkType eGfxLinkType)
{
    switch(eGfxLinkType)
    {
        case GfxLinkType::EpsBuffer: return "EpsBuffer";
        case GfxLinkType::NativeBmp: return "NativeBmp";
        case GfxLinkType::NativeGif: return "NativeGif";
        case GfxLinkType::NativeJpg: return "NativeJpg";
        case GfxLinkType::NativeMet: return "NativeMet";
        case GfxLinkType::NativeMov: return "NativeMov";
        case GfxLinkType::NativePct: return "NativePct";
        case GfxLinkType::NativePdf: return "NativePdf";
        case GfxLinkType::NativePng: return "NativePng";
        case GfxLinkType::NativeSvg: return "NativeSvg";
        case GfxLinkType::NativeTif: return "NativeTif";
        case GfxLinkType::NativeWmf: return "NativeWmf";
        case GfxLinkType::NONE: return "None";
    }
    return OUString();
}

OUString hex32(sal_uInt32 nNumber)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << nNumber;
    return OUString::createFromAscii(ss.str().c_str());
}

OUString toHexString(const sal_uInt8* nData, sal_uInt32 nDataSize){

    std::stringstream aStrm;
    for (sal_uInt32 i = 0; i < nDataSize; i++)
    {
        aStrm << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(nData[i]);
    }

    return OUString::createFromAscii(aStrm.str().c_str());
}

void writePoint(tools::XmlWriter& rWriter, Point const& rPoint)
{
    rWriter.attribute("x", rPoint.X());
    rWriter.attribute("y", rPoint.Y());
}

void writeStartPoint(tools::XmlWriter& rWriter, Point const& rPoint)
{
    rWriter.attribute("startx", rPoint.X());
    rWriter.attribute("starty", rPoint.Y());
}

void writeEndPoint(tools::XmlWriter& rWriter, Point const& rPoint)
{
    rWriter.attribute("endx", rPoint.X());
    rWriter.attribute("endy", rPoint.Y());
}

void writeSize(tools::XmlWriter& rWriter, Size const& rSize)
{
    rWriter.attribute("width", rSize.Width());
    rWriter.attribute("height", rSize.Height());
}

void writeRectangle(tools::XmlWriter& rWriter, tools::Rectangle const& rRectangle)
{
    rWriter.attribute("left", rRectangle.Left());
    rWriter.attribute("top", rRectangle.Top());
    if (rRectangle.IsWidthEmpty())
        rWriter.attribute("right", OString("empty"));
    else
        rWriter.attribute("right", rRectangle.Right());
    if (rRectangle.IsHeightEmpty())
        rWriter.attribute("bottom", OString("empty"));
    else
        rWriter.attribute("bottom", rRectangle.Bottom());
}

void writeMapMode(tools::XmlWriter& rWriter, MapMode const& rMapMode)
{
    rWriter.attribute("mapunit", convertMapUnitToString( rMapMode.GetMapUnit() ));
    writePoint(rWriter, rMapMode.GetOrigin());
    rWriter.attribute("scalex", convertFractionToString(rMapMode.GetScaleX()));
    rWriter.attribute("scaley", convertFractionToString(rMapMode.GetScaleY()));
}

void writeLineInfo(tools::XmlWriter& rWriter, LineInfo const& rLineInfo)
{
    rWriter.attribute("style", convertLineStyleToString(rLineInfo.GetStyle()));
    rWriter.attribute("width", rLineInfo.GetWidth());
    rWriter.attribute("dashlen", rLineInfo.GetDashLen());
    rWriter.attribute("dashcount", rLineInfo.GetDashCount());
    rWriter.attribute("dotlen", rLineInfo.GetDotLen());
    rWriter.attribute("dotcount", rLineInfo.GetDotCount());
    rWriter.attribute("distance", rLineInfo.GetDistance());
    rWriter.attribute("join", convertLineJoinToString(rLineInfo.GetLineJoin()));
    rWriter.attribute("cap", convertLineCapToString(rLineInfo.GetLineCap()));
}

void writeGradient(tools::XmlWriter& rWriter, Gradient const& rGradient)
{
    rWriter.attribute("style", convertGradientStyle(rGradient.GetStyle()));
    rWriter.attribute("startcolor", convertColorToString(rGradient.GetStartColor()));
    rWriter.attribute("endcolor", convertColorToString(rGradient.GetEndColor()));
    rWriter.attribute("angle", rGradient.GetAngle().get());
    rWriter.attribute("border", rGradient.GetBorder());
    rWriter.attribute("offsetx", rGradient.GetOfsX());
    rWriter.attribute("offsety", rGradient.GetOfsY());
    rWriter.attribute("startintensity", rGradient.GetStartIntensity());
    rWriter.attribute("endintensity", rGradient.GetEndIntensity());
    rWriter.attribute("steps", rGradient.GetSteps());
}

} // anonymous namespace

MetafileXmlDump::MetafileXmlDump()
{
    maFilter.fill(false);
}

MetafileXmlDump::~MetafileXmlDump()
{}

void MetafileXmlDump::filterActionType(const MetaActionType nActionType, bool bShouldFilter)
{
    maFilter[nActionType] = bShouldFilter;
}

void MetafileXmlDump::filterAllActionTypes()
{
    maFilter.fill(true);
}

void MetafileXmlDump::dump(const GDIMetaFile& rMetaFile, SvStream& rStream)
{
    tools::XmlWriter aWriter(&rStream);
    aWriter.startDocument();
    aWriter.startElement("metafile");

    writeXml(rMetaFile, aWriter);

    aWriter.endElement();
    aWriter.endDocument();
}

void MetafileXmlDump::writeXml(const GDIMetaFile& rMetaFile, tools::XmlWriter& rWriter)
{
    MapMode aMtfMapMode = rMetaFile.GetPrefMapMode();
    rWriter.attribute("mapunit", convertMapUnitToString(aMtfMapMode.GetMapUnit()));
    writePoint(rWriter, aMtfMapMode.GetOrigin());
    rWriter.attribute("scalex", convertFractionToString(aMtfMapMode.GetScaleX()));
    rWriter.attribute("scaley", convertFractionToString(aMtfMapMode.GetScaleY()));

    Size aMtfSize = rMetaFile.GetPrefSize();
    writeSize(rWriter, aMtfSize);

    for(size_t nAction = 0; nAction < rMetaFile.GetActionSize(); ++nAction)
    {
        MetaAction* pAction = rMetaFile.GetAction(nAction);
        const MetaActionType nActionType = pAction->GetType();
        if (maFilter[nActionType])
            continue;

        OString sCurrentElementTag = convertLineStyleToString(nActionType);

        switch (nActionType)
        {
            case MetaActionType::NONE:
            {
                rWriter.startElement(sCurrentElementTag);
                rWriter.endElement();
            }
            break;

            case MetaActionType::PIXEL:
            {
                auto* pMetaAction = static_cast<MetaPixelAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMetaAction->GetPoint());
                rWriter.attribute("color", convertColorToString(pMetaAction->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::POINT:
            {
                auto* pMetaAction = static_cast<MetaPointAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMetaAction->GetPoint());
                rWriter.endElement();
            }
            break;

            case MetaActionType::LINE:
            {
                MetaLineAction* pMetaLineAction = static_cast<MetaLineAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writeStartPoint(rWriter, pMetaLineAction->GetStartPoint());
                writeEndPoint(rWriter, pMetaLineAction->GetEndPoint());

                writeLineInfo(rWriter, pMetaLineAction->GetLineInfo());
                rWriter.endElement();
            }
            break;

            case MetaActionType::RECT:
            {
                MetaRectAction* pMetaAction = static_cast<MetaRectAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writeRectangle(rWriter, pMetaAction->GetRect());
                rWriter.endElement();
            }
            break;

            case MetaActionType::ROUNDRECT:
            {
                auto pMetaAction = static_cast<MetaRoundRectAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writeRectangle(rWriter, pMetaAction->GetRect());
                rWriter.attribute("horizontalround", pMetaAction->GetHorzRound());
                rWriter.attribute("verticalround", pMetaAction->GetVertRound());
                rWriter.endElement();
            }
            break;

            case MetaActionType::ELLIPSE:
            {
                auto pMetaAction = static_cast<MetaEllipseAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writeRectangle(rWriter, pMetaAction->GetRect());
                rWriter.endElement();
            }
            break;

            case MetaActionType::ARC:
            {
                auto pMetaAction = static_cast<MetaArcAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writeRectangle(rWriter, pMetaAction->GetRect());
                writeStartPoint(rWriter, pMetaAction->GetStartPoint());
                writeEndPoint(rWriter, pMetaAction->GetEndPoint());
                rWriter.endElement();
            }
            break;

            case MetaActionType::PIE:
            {
                auto pMetaAction = static_cast<MetaPieAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writeRectangle(rWriter, pMetaAction->GetRect());
                writeStartPoint(rWriter, pMetaAction->GetStartPoint());
                writeEndPoint(rWriter, pMetaAction->GetEndPoint());
                rWriter.endElement();
            }
            break;

            case MetaActionType::CHORD:
            {
                auto pMetaAction = static_cast<MetaChordAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writeRectangle(rWriter, pMetaAction->GetRect());
                writeStartPoint(rWriter, pMetaAction->GetStartPoint());
                writeEndPoint(rWriter, pMetaAction->GetEndPoint());
                rWriter.endElement();
            }
            break;

            case MetaActionType::POLYLINE:
            {
                MetaPolyLineAction* pMetaPolyLineAction = static_cast<MetaPolyLineAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                writeLineInfo(rWriter, pMetaPolyLineAction->GetLineInfo());

                tools::Polygon aPolygon = pMetaPolyLineAction->GetPolygon();
                bool bFlags = aPolygon.HasFlags();
                for (sal_uInt16 i = 0; i < aPolygon.GetSize(); i++)
                {
                    rWriter.startElement("point");
                    writePoint(rWriter, aPolygon[i]);
                    if (bFlags)
                        rWriter.attribute("flags", convertPolygonFlags(aPolygon.GetFlags(i)));
                    rWriter.endElement();
                }

                rWriter.endElement();
            }
            break;

            case MetaActionType::POLYGON:
            {
                MetaPolygonAction* pMetaPolygonAction = static_cast<MetaPolygonAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                tools::Polygon aPolygon = pMetaPolygonAction->GetPolygon();
                bool bFlags = aPolygon.HasFlags();
                for (sal_uInt16 i = 0; i < aPolygon.GetSize(); i++)
                {
                    rWriter.startElement("point");
                    writePoint(rWriter, aPolygon[i]);
                    if (bFlags)
                        rWriter.attribute("flags", convertPolygonFlags(aPolygon.GetFlags(i)));
                    rWriter.endElement();
                }

                rWriter.endElement();
            }
            break;

            case MetaActionType::POLYPOLYGON:
            {
                MetaPolyPolygonAction *const pMetaPolyPolygonAction = static_cast<MetaPolyPolygonAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                tools::PolyPolygon const& rPolyPolygon(pMetaPolyPolygonAction->GetPolyPolygon());

                for (sal_uInt16 j = 0; j < rPolyPolygon.Count(); ++j)
                {
                    rWriter.startElement("polygon");
                    tools::Polygon const& rPolygon = rPolyPolygon[j];
                    bool bFlags = rPolygon.HasFlags();
                    for (sal_uInt16 i = 0; i < rPolygon.GetSize(); ++i)
                    {
                        rWriter.startElement("point");
                        writePoint(rWriter, rPolygon[i]);
                        if (bFlags)
                            rWriter.attribute("flags", convertPolygonFlags(rPolygon.GetFlags(i)));
                        rWriter.endElement();
                    }
                    rWriter.endElement();
                }
                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXT:
            {
                auto* pMeta = static_cast<MetaTextAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetPoint());
                rWriter.attribute("index", pMeta->GetIndex());
                rWriter.attribute("length", pMeta->GetLen());
                rWriter.startElement("textcontent");
                rWriter.content(pMeta->GetText());
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTARRAY:
            {
                MetaTextArrayAction* pMetaTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                sal_Int32 aIndex = pMetaTextArrayAction->GetIndex();
                sal_Int32 aLength = pMetaTextArrayAction->GetLen();

                writePoint(rWriter, pMetaTextArrayAction->GetPoint());
                rWriter.attribute("index", aIndex);
                rWriter.attribute("length", aLength);

                if (!pMetaTextArrayAction->GetDXArray().empty())
                {
                    rWriter.startElement("dxarray");
                    OUStringBuffer sDxLengthString;
                    for (sal_Int32 i = 0; i < aLength - aIndex; ++i)
                    {
                        sDxLengthString.append(pMetaTextArrayAction->GetDXArray()[aIndex + i]);
                        sDxLengthString.append(" ");
                    }
                    rWriter.content(sDxLengthString.makeStringAndClear());
                    rWriter.endElement();
                }

                rWriter.startElement("text");
                rWriter.content(pMetaTextArrayAction->GetText());
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::STRETCHTEXT:
            {
                auto* pMeta = static_cast<MetaStretchTextAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                writePoint(rWriter, pMeta->GetPoint());
                rWriter.attribute("index", pMeta->GetIndex());
                rWriter.attribute("length", pMeta->GetLen());
                rWriter.attribute("width", pMeta->GetWidth());

                rWriter.startElement("textcontent");
                rWriter.content(pMeta->GetText());
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTRECT:
            {
                auto* pMeta = static_cast<MetaTextRectAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writeRectangle(rWriter, pMeta->GetRect());
                rWriter.startElement("textcontent");
                rWriter.content(pMeta->GetText());
                rWriter.endElement();

                rWriter.startElement("style");
                rWriter.content(convertDrawTextFlagsToString(pMeta->GetStyle()));
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::BMP:
            {
                auto pMeta = static_cast<MetaBmpAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetPoint());
                rWriter.attribute("crc", hex32(pMeta->GetBitmap().GetChecksum()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                auto pMeta = static_cast<MetaBmpScaleAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetPoint());
                writeSize(rWriter, pMeta->GetSize());
                rWriter.attribute("crc", hex32(pMeta->GetBitmap().GetChecksum()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                auto pMeta = static_cast<MetaBmpScalePartAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("destx", pMeta->GetDestPoint().X());
                rWriter.attribute("desty", pMeta->GetDestPoint().Y());
                rWriter.attribute("destwidth", pMeta->GetDestSize().Width());
                rWriter.attribute("destheight", pMeta->GetDestSize().Height());
                rWriter.attribute("srcx", pMeta->GetSrcPoint().X());
                rWriter.attribute("srcy", pMeta->GetSrcPoint().Y());
                rWriter.attribute("srcwidth", pMeta->GetSrcSize().Width());
                rWriter.attribute("srcheight", pMeta->GetSrcSize().Height());
                rWriter.attribute("crc", hex32(pMeta->GetBitmap().GetChecksum()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::BMPEX:
            {
                auto pMeta = static_cast<MetaBmpExAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetPoint());
                rWriter.attribute("crc", hex32(pMeta->GetBitmapEx().GetBitmap().GetChecksum()));
                rWriter.attribute("transparenttype", convertBitmapExTransparentType(pMeta->GetBitmapEx()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                auto pMeta = static_cast<MetaBmpExScaleAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetPoint());
                writeSize(rWriter, pMeta->GetSize());
                rWriter.attribute("crc", hex32(pMeta->GetBitmapEx().GetBitmap().GetChecksum()));
                rWriter.attribute("transparenttype", convertBitmapExTransparentType(pMeta->GetBitmapEx()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::BMPEXSCALEPART:
            {
                auto pMeta = static_cast<MetaBmpExScalePartAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("destx", pMeta->GetDestPoint().X());
                rWriter.attribute("desty", pMeta->GetDestPoint().Y());
                rWriter.attribute("destwidth", pMeta->GetDestSize().Width());
                rWriter.attribute("destheight", pMeta->GetDestSize().Height());
                rWriter.attribute("srcx", pMeta->GetSrcPoint().X());
                rWriter.attribute("srcy", pMeta->GetSrcPoint().Y());
                rWriter.attribute("srcwidth", pMeta->GetSrcSize().Width());
                rWriter.attribute("srcheight", pMeta->GetSrcSize().Height());
                rWriter.attribute("crc", hex32(pMeta->GetBitmapEx().GetBitmap().GetChecksum()));
                rWriter.attribute("transparenttype", convertBitmapExTransparentType(pMeta->GetBitmapEx()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::MASK:
            {
                auto pMeta = static_cast<MetaMaskAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetPoint());
                rWriter.attribute("crc", hex32(pMeta->GetBitmap().GetChecksum()));
                rWriter.attribute("color", convertColorToString(pMeta->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::MASKSCALE:
            {
                auto pMeta = static_cast<MetaMaskScaleAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetPoint());
                writeSize(rWriter, pMeta->GetSize());
                rWriter.attribute("crc", hex32(pMeta->GetBitmap().GetChecksum()));
                rWriter.attribute("color", convertColorToString(pMeta->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::MASKSCALEPART:
            {
                auto pMeta = static_cast<MetaMaskScalePartAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("destx", pMeta->GetDestPoint().X());
                rWriter.attribute("desty", pMeta->GetDestPoint().Y());
                rWriter.attribute("destwidth", pMeta->GetDestSize().Width());
                rWriter.attribute("destheight", pMeta->GetDestSize().Height());
                rWriter.attribute("srcx", pMeta->GetSrcPoint().X());
                rWriter.attribute("srcy", pMeta->GetSrcPoint().Y());
                rWriter.attribute("srcwidth", pMeta->GetSrcSize().Width());
                rWriter.attribute("srcheight", pMeta->GetSrcSize().Height());
                rWriter.attribute("crc", hex32(pMeta->GetBitmap().GetChecksum()));
                rWriter.attribute("color", convertColorToString(pMeta->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::GRADIENT:
            {
                const MetaGradientAction* pMeta = static_cast<MetaGradientAction*>(pAction);

                rWriter.startElement(sCurrentElementTag);
                writeGradient(rWriter, pMeta->GetGradient());

                rWriter.startElement("rectangle");
                writeRectangle(rWriter, pMeta->GetRect());
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::HATCH:
            {
                auto* const pMetaHatchAction = static_cast<MetaHatchAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                tools::PolyPolygon const& rPolyPolygon(pMetaHatchAction->GetPolyPolygon());

                for (sal_uInt16 j = 0; j < rPolyPolygon.Count(); ++j)
                {
                    rWriter.startElement("polygon");
                    tools::Polygon const& rPolygon = rPolyPolygon[j];
                    bool bFlags = rPolygon.HasFlags();
                    for (sal_uInt16 i = 0; i < rPolygon.GetSize(); ++i)
                    {
                        rWriter.startElement("point");
                        writePoint(rWriter, rPolygon[i]);
                        if (bFlags)
                            rWriter.attribute("flags", convertPolygonFlags(rPolygon.GetFlags(i)));
                        rWriter.endElement();
                    }
                    rWriter.endElement();
                }

                rWriter.startElement("hatch");
                const auto& rHatch = pMetaHatchAction->GetHatch();
                rWriter.attribute("style", convertHatchStyle(rHatch.GetStyle()));
                rWriter.attribute("color", convertColorToString(rHatch.GetColor()));
                rWriter.attribute("distance", sal_Int32(rHatch.GetDistance()));
                rWriter.attribute("angle", sal_Int32(rHatch.GetAngle().get()));
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::WALLPAPER:
            {
                const auto* pMetaAction = static_cast<const MetaWallpaperAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                writeRectangle(rWriter, pMetaAction->GetRect());

                rWriter.startElement("wallpaper");
                const auto& rWallpaper = pMetaAction->GetWallpaper();

                rWriter.attribute("color", convertColorToString(rWallpaper.GetColor()));

                WallpaperStyle eStyle = rWallpaper.GetStyle();
                rWriter.attribute("style", convertWallpaperStyleToString(eStyle));

                if (rWallpaper.IsBitmap())
                {
                    rWriter.startElement("bitmap");
                    BitmapEx const & rBitmapEx = rWallpaper.GetBitmap();
                    rWriter.attribute("crc", hex32(rBitmapEx.GetChecksum()));
                    rWriter.attribute("transparenttype", convertBitmapExTransparentType(rBitmapEx));
                    rWriter.attribute("pixelformat", convertPixelFormatToString(rBitmapEx.GetBitmap().getPixelFormat()));
                    rWriter.attribute("width", hex32(rBitmapEx.GetSizePixel().Width()));
                    rWriter.attribute("height", hex32(rBitmapEx.GetSizePixel().Height()));
                    rWriter.endElement();
                }

                if (rWallpaper.IsGradient())
                {
                    rWriter.startElement("gradient");
                    Gradient aGradient = rWallpaper.GetGradient();
                    writeGradient(rWriter, aGradient);
                    rWriter.endElement();
                }

                if (rWallpaper.IsRect())
                {
                    tools::Rectangle aRect = rWallpaper.GetRect();
                    rWriter.startElement("rectangle");
                    writeRectangle(rWriter, aRect);
                    rWriter.endElement();
                }

                rWriter.attribute("fixed", rWallpaper.IsFixed() ? "true" : "false");
                rWriter.attribute("scrollable", rWallpaper.IsScrollable() ? "true" : "false");

                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::CLIPREGION:
            {
                const auto* pMetaClipRegionAction = static_cast<const MetaClipRegionAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                // FIXME for now we dump only the bounding box; this is
                // enough for the tests we have, but may need extending to
                // dumping the real polypolygon in the future
                tools::Rectangle aRectangle = pMetaClipRegionAction->GetRegion().GetBoundRect();
                writeRectangle(rWriter, aRectangle);

                vcl::Region aRegion = pMetaClipRegionAction->GetRegion();

                if (aRegion.HasPolyPolygonOrB2DPolyPolygon())
                {
                    tools::PolyPolygon aPolyPolygon = aRegion.GetAsPolyPolygon();

                    for (sal_uInt16 j = 0; j < aPolyPolygon.Count(); ++j)
                    {
                        rWriter.startElement("polygon");
                        tools::Polygon const& rPolygon = aPolyPolygon[j];
                        bool bFlags = rPolygon.HasFlags();
                        for (sal_uInt16 i = 0; i < rPolygon.GetSize(); ++i)
                        {
                            rWriter.startElement("point");
                            writePoint(rWriter, rPolygon[i]);
                            if (bFlags)
                                rWriter.attribute("flags", convertPolygonFlags(rPolygon.GetFlags(i)));
                            rWriter.endElement();
                        }
                        rWriter.endElement();
                    }
                }

                rWriter.endElement();
            }
            break;

            case MetaActionType::ISECTRECTCLIPREGION:
            {
                MetaISectRectClipRegionAction* pMetaISectRectClipRegionAction = static_cast<MetaISectRectClipRegionAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                tools::Rectangle aRectangle = pMetaISectRectClipRegionAction->GetRect();
                writeRectangle(rWriter, aRectangle);
                rWriter.endElement();
            }
            break;

            case MetaActionType::ISECTREGIONCLIPREGION:
            {
                MetaISectRegionClipRegionAction* pMetaISectRegionClipRegionAction = static_cast<MetaISectRegionClipRegionAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                // FIXME for now we dump only the bounding box; this is
                // enough for the tests we have, but may need extending to
                // dumping the real polypolygon in the future
                tools::Rectangle aRectangle = pMetaISectRegionClipRegionAction->GetRegion().GetBoundRect();
                writeRectangle(rWriter, aRectangle);
                rWriter.endElement();
            }
            break;

            case MetaActionType::MOVECLIPREGION:
            {
                const auto* pMetaMoveClipRegionAction = static_cast<MetaMoveClipRegionAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("horzmove", pMetaMoveClipRegionAction->GetHorzMove());
                rWriter.attribute("vertmove", pMetaMoveClipRegionAction->GetVertMove());
                rWriter.endElement();
            }
            break;

            case MetaActionType::LINECOLOR:
            {
                MetaLineColorAction* pMetaLineColorAction = static_cast<MetaLineColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("color", convertColorToString(pMetaLineColorAction->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::FILLCOLOR:
            {
                MetaFillColorAction* pMetaFillColorAction = static_cast<MetaFillColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("color", convertColorToString(pMetaFillColorAction->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTCOLOR:
            {
                MetaTextColorAction* pMetaTextColorAction = static_cast<MetaTextColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("color", convertColorToString(pMetaTextColorAction->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTFILLCOLOR:
            {
                MetaTextFillColorAction* pMetaTextFillColorAction = static_cast<MetaTextFillColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("color", convertColorToString(pMetaTextFillColorAction->GetColor()));

                if (pMetaTextFillColorAction->IsSetting())
                    rWriter.attribute("setting", u"true");

                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTALIGN:
            {
                MetaTextAlignAction* pMetaTextAlignAction = static_cast<MetaTextAlignAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                OUString sAlign = convertTextAlignToString(pMetaTextAlignAction->GetTextAlign());
                if (!sAlign.isEmpty())
                    rWriter.attribute("align", sAlign);
                rWriter.endElement();
            }
            break;

            case MetaActionType::MAPMODE:
            {
                const MetaMapModeAction* pMeta = static_cast<MetaMapModeAction*>(pAction);
                MapMode aMapMode = pMeta->GetMapMode();
                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("mapunit", convertMapUnitToString( aMapMode.GetMapUnit() ));
                writePoint(rWriter, aMapMode.GetOrigin());
                rWriter.attribute("scalex", convertFractionToString(aMapMode.GetScaleX()));
                rWriter.attribute("scaley", convertFractionToString(aMapMode.GetScaleY()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::FONT:
            {
                MetaFontAction* pMetaFontAction = static_cast<MetaFontAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                vcl::Font aFont = pMetaFontAction->GetFont();

                rWriter.attribute("color", convertColorToString(aFont.GetColor()));
                rWriter.attribute("fillcolor", convertColorToString(aFont.GetFillColor()));
                rWriter.attribute("name", aFont.GetFamilyName());
                rWriter.attribute("stylename", aFont.GetStyleName());
                rWriter.attribute("width", aFont.GetFontSize().Width());
                rWriter.attribute("height", aFont.GetFontSize().Height());
                rWriter.attribute("orientation", aFont.GetOrientation().get());
                rWriter.attribute("weight", convertFontWeightToString(aFont.GetWeight()));
                rWriter.attribute("vertical", aFont.IsVertical() ? "true" : "false");

                rWriter.endElement();
            }
            break;

            case MetaActionType::PUSH:
            {
                MetaPushAction* pMetaPushAction = static_cast<MetaPushAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("flags", collectPushFlags(pMetaPushAction->GetFlags()));
            }
            break;

            case MetaActionType::POP:
            {
                rWriter.endElement();
            }
            break;

            case MetaActionType::RASTEROP:
            {
                MetaRasterOpAction* pMetaRasterOpAction = static_cast<MetaRasterOpAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                if (pMetaRasterOpAction->GetRasterOp() != RasterOp::OverPaint)
                {
                    rWriter.attribute("operation", convertRopToString(pMetaRasterOpAction->GetRasterOp()));
                }
                rWriter.endElement();
            }
            break;

            case MetaActionType::Transparent:
            {
                const MetaTransparentAction* pMeta = static_cast<MetaTransparentAction*>(pAction);

                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("transparence", pMeta->GetTransparence());

                tools::PolyPolygon const& rPolyPolygon(pMeta->GetPolyPolygon());

                for (sal_uInt16 j = 0; j < rPolyPolygon.Count(); ++j)
                {
                    rWriter.startElement("polygon");
                    tools::Polygon const& rPolygon = rPolyPolygon[j];
                    bool bFlags = rPolygon.HasFlags();
                    for (sal_uInt16 i = 0; i < rPolygon.GetSize(); ++i)
                    {
                        rWriter.startElement("point");
                        writePoint(rWriter, rPolygon[i]);
                        if (bFlags)
                            rWriter.attribute("flags", convertPolygonFlags(rPolygon.GetFlags(i)));
                        rWriter.endElement();
                    }
                    rWriter.endElement();
                }

                rWriter.endElement();
            }
            break;

            case MetaActionType::EPS:
            {
                MetaEPSAction* pMetaEPSAction = static_cast<MetaEPSAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                writePoint(rWriter, pMetaEPSAction->GetPoint());
                writeSize(rWriter, pMetaEPSAction->GetSize());

                rWriter.startElement("gfxlink");
                writeSize(rWriter, pMetaEPSAction->GetLink().GetPrefSize());
                rWriter.attribute("type", convertGfxLinkTypeToString(pMetaEPSAction->GetLink().GetType()));
                rWriter.attribute("userid", pMetaEPSAction->GetLink().GetUserId());
                rWriter.attribute("datasize", pMetaEPSAction->GetLink().GetDataSize());
                rWriter.attribute("data", toHexString(pMetaEPSAction->GetLink().GetData(), pMetaEPSAction->GetLink().GetDataSize()));
                rWriter.attribute("native", pMetaEPSAction->GetLink().IsNative() ? "true" : "false");
                rWriter.attribute("emf", pMetaEPSAction->GetLink().IsEMF() ? "true" : "false");
                rWriter.attribute("validmapmode", pMetaEPSAction->GetLink().IsPrefMapModeValid() ? "true" : "false");
                rWriter.startElement("prefmapmode");
                writeMapMode(rWriter, pMetaEPSAction->GetLink().GetPrefMapMode());
                rWriter.endElement();
                rWriter.endElement();

                rWriter.startElement("metafile");
                writeXml(pMetaEPSAction->GetSubstitute(), rWriter);
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::REFPOINT:
            {
                auto* pMeta = static_cast<MetaRefPointAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetRefPoint());
                rWriter.attribute("set", pMeta->IsSetting() ? "true" : "false");
                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTLINECOLOR:
            {
                auto* pMeta = static_cast<MetaTextLineColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("color", convertColorToString(pMeta->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTLINE:
            {
                auto* pMeta = static_cast<MetaTextLineAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetStartPoint());
                rWriter.attribute("width", pMeta->GetWidth());
                rWriter.attribute("strikeout", convertFontStrikeoutToString(pMeta->GetStrikeout()));
                rWriter.attribute("underline", convertFontLineStyleToString(pMeta->GetUnderline()));
                rWriter.attribute("overline", convertFontLineStyleToString(pMeta->GetOverline()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::FLOATTRANSPARENT:
            {
                const auto* pMeta = static_cast<MetaFloatTransparentAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                writePoint(rWriter, pMeta->GetPoint());
                writeSize(rWriter, pMeta->GetSize());
                rWriter.attribute("transparent", pMeta->IsTransparent() ? "true" : "false");

                rWriter.startElement("gradient");
                writeGradient(rWriter, pMeta->GetGradient());
                rWriter.endElement();

                rWriter.startElement("metafile");
                writeXml(pMeta->GetGDIMetaFile(), rWriter);
                rWriter.endElement();

                rWriter.endElement();
            }
            break;

            case MetaActionType::GRADIENTEX:
            {
                const MetaGradientExAction* pMetaGradientExAction = static_cast<MetaGradientExAction*>(pAction);

                rWriter.startElement(sCurrentElementTag);
                writeGradient(rWriter, pMetaGradientExAction->GetGradient());

                tools::PolyPolygon const& rPolyPolygon(pMetaGradientExAction->GetPolyPolygon());
                for (sal_uInt16 j = 0; j < rPolyPolygon.Count(); ++j)
                {
                    rWriter.startElement("polygon");
                    tools::Polygon const& rPolygon = rPolyPolygon[j];
                    bool bFlags = rPolygon.HasFlags();
                    for (sal_uInt16 i = 0; i < rPolygon.GetSize(); ++i)
                    {
                        rWriter.startElement("point");
                        writePoint(rWriter, rPolygon[i]);
                        if (bFlags)
                            rWriter.attribute("flags", convertPolygonFlags(rPolygon.GetFlags(i)));
                        rWriter.endElement();
                    }
                    rWriter.endElement();
                }

                rWriter.endElement();
            }
            break;

            case MetaActionType::LAYOUTMODE:
            {
                const MetaLayoutModeAction* pMetaLayoutModeAction = static_cast<MetaLayoutModeAction*>(pAction);

                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("textlayout", convertComplexTestLayoutFlags(pMetaLayoutModeAction->GetLayoutMode()));

                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTLANGUAGE:
            {
                const MetaTextLanguageAction* pMetaTextLanguageAction = static_cast<MetaTextLanguageAction*>(pAction);

                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("language", convertLanguageTypeToString(pMetaTextLanguageAction->GetTextLanguage()));

                rWriter.endElement();
            }
            break;

            case MetaActionType::OVERLINECOLOR:
            {
                const auto* pMetaAction = static_cast<MetaOverlineColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("color", convertColorToString(pMetaAction->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::COMMENT:
            {
                MetaCommentAction* pMetaCommentAction = static_cast<MetaCommentAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                if (pMetaCommentAction->GetDataSize() > 0)
                {
                    rWriter.attribute("datasize", pMetaCommentAction->GetDataSize());
                    rWriter.attribute("data", toHexString(pMetaCommentAction->GetData(), pMetaCommentAction->GetDataSize()));
                }
                rWriter.attribute("value", pMetaCommentAction->GetValue());

                if (!pMetaCommentAction->GetComment().isEmpty())
                {
                    rWriter.startElement("comment");
                    rWriter.content(pMetaCommentAction->GetComment());
                    rWriter.endElement();
                }
                rWriter.endElement();
            }
            break;

            default:
            {
                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("note", OString("not implemented in xml dump"));
                rWriter.endElement();
            }
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
