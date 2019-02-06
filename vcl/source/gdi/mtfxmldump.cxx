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
#include <rtl/strbuf.hxx>

#include <memory>
#include <numeric>
#include <sstream>

namespace
{

OUString collectPushFlags(PushFlags nFlags)
{
    if ((nFlags & PushFlags::ALL) == PushFlags::ALL)
        return OUString("PushAll");
    else if ((nFlags & PUSH_ALLFONT) == PUSH_ALLFONT)
        return OUString("PushAllFont");
    else if ((nFlags & PUSH_ALLTEXT) == PUSH_ALLTEXT)
        return OUString("PushAllText");

    std::vector<OUString> aStrings;

    if (nFlags & PushFlags::LINECOLOR)
        aStrings.emplace_back("PushLineColor");
    if (nFlags & PushFlags::FILLCOLOR)
        aStrings.emplace_back("PushFillColor");
    if (nFlags & PushFlags::FONT)
        aStrings.emplace_back("PushFont");
    if (nFlags & PushFlags::TEXTCOLOR)
        aStrings.emplace_back("PushTextColor");
    if (nFlags & PushFlags::MAPMODE)
        aStrings.emplace_back("PushMapMode");
    if (nFlags & PushFlags::CLIPREGION)
        aStrings.emplace_back("PushClipRegion");
    if (nFlags & PushFlags::RASTEROP)
        aStrings.emplace_back("PushRasterOp");
    if (nFlags & PushFlags::TEXTFILLCOLOR)
        aStrings.emplace_back("PushTextFillColor");
    if (nFlags & PushFlags::TEXTALIGN)
        aStrings.emplace_back("PushTextAlign");
    if (nFlags & PushFlags::REFPOINT)
        aStrings.emplace_back("PushRefPoint");
    if (nFlags & PushFlags::TEXTLINECOLOR)
        aStrings.emplace_back("PushTextLineColor");
    if (nFlags & PushFlags::TEXTLAYOUTMODE)
        aStrings.emplace_back("PushTextLayoutMode");
    if (nFlags & PushFlags::TEXTLANGUAGE)
        aStrings.emplace_back("PushTextLanguage");
    if (nFlags & PushFlags::OVERLINECOLOR)
        aStrings.emplace_back("PushOverlineColor");

    OUString aString;

    if (aStrings.empty())
        return aString;

    aString = aStrings[0];
    for (size_t i = 1; i < aStrings.size(); ++i)
    {
        aString = aString + ", " + aStrings[i];
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
        return OUString("None");

    aString = aStrings[0];
    for (size_t i = 1; i < aStrings.size(); ++i)
    {
        aString = aString + " " + aStrings[i];
    }
    return aString;
};

OUString convertRopToString(RasterOp eRop)
{
    switch (eRop)
    {
        case RasterOp::OverPaint: return OUString("overpaint");
        case RasterOp::Xor:       return OUString("xor");
        case RasterOp::N0:        return OUString("0");
        case RasterOp::N1:        return OUString("1");
        case RasterOp::Invert:    return OUString("invert");
    }
    return OUString();
}

OUString convertTextAlignToString(TextAlign eAlign)
{
    switch (eAlign)
    {
        case ALIGN_BASELINE:             return OUString("baseline");
        case ALIGN_BOTTOM:               return OUString("bottom");
        case ALIGN_TOP:                  return OUString("top");
        case TextAlign_FORCE_EQUAL_SIZE: return OUString("equalsize");
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
        case LineStyle::NONE:    return OUString("none");
        case LineStyle::Solid:   return OUString("solid");
        case LineStyle::Dash:    return OUString("dash");
        default: break;
    }
    return OUString();
}

OUString convertLineJoinToString(basegfx::B2DLineJoin eJoin)
{
    switch (eJoin)
    {
        default:
        case basegfx::B2DLineJoin::NONE:    return OUString("none");
        case basegfx::B2DLineJoin::Bevel:   return OUString("bevel");
        case basegfx::B2DLineJoin::Miter:   return OUString("miter");
        case basegfx::B2DLineJoin::Round:   return OUString("round");
    }
}

OUString convertLineCapToString(css::drawing::LineCap eCap)
{
    switch (eCap)
    {
        default:
        case css::drawing::LineCap_BUTT:   return OUString("butt");
        case css::drawing::LineCap_ROUND:  return OUString("round");
        case css::drawing::LineCap_SQUARE: return OUString("square");
    }
}

OUString convertPolygonFlags(PolyFlags eFlags)
{
    switch (eFlags)
    {
        default:
        case PolyFlags::Normal:    return OUString("normal");
        case PolyFlags::Control:   return OUString("control");
        case PolyFlags::Smooth:    return OUString("smooth");
        case PolyFlags::Symmetric: return OUString("symmetric");
    }
}

OUString convertFontWeigthToString(FontWeight eFontWeight)
{
    switch (eFontWeight)
    {
        case WEIGHT_DONTKNOW:   return OUString("unknown");
        case WEIGHT_THIN:       return OUString("thin");
        case WEIGHT_ULTRALIGHT: return OUString("ultralight");
        case WEIGHT_LIGHT:      return OUString("light");
        case WEIGHT_SEMILIGHT:  return OUString("semilight");
        case WEIGHT_NORMAL:     return OUString("normal");
        case WEIGHT_MEDIUM:     return OUString("medium");
        case WEIGHT_SEMIBOLD:   return OUString("semibold");
        case WEIGHT_BOLD:       return OUString("bold");
        case WEIGHT_ULTRABOLD:  return OUString("ultrabold");
        case WEIGHT_BLACK:      return OUString("black");
        case FontWeight_FORCE_EQUAL_SIZE: return OUString("equalsize");
    }
    return OUString();
}

OUString convertFontStrikeoutToString(FontStrikeout eFontStrikeout)
{
    switch (eFontStrikeout)
    {
        case STRIKEOUT_NONE:     return OUString("none");
        case STRIKEOUT_SINGLE:   return OUString("single");
        case STRIKEOUT_DOUBLE:   return OUString("double");
        case STRIKEOUT_DONTKNOW: return OUString("dontknow");
        case STRIKEOUT_BOLD:     return OUString("bold");
        case STRIKEOUT_SLASH:    return OUString("slash");
        case STRIKEOUT_X:        return OUString("x");
        case FontStrikeout_FORCE_EQUAL_SIZE: return OUString("equalsize");
    }
    return OUString();
}

OUString convertFontLineStyleToString(FontLineStyle eFontLineStyle)
{
    switch (eFontLineStyle)
    {
        case LINESTYLE_NONE:            return OUString("none");
        case LINESTYLE_SINGLE:          return OUString("single");
        case LINESTYLE_DOUBLE:          return OUString("double");
        case LINESTYLE_DOTTED:          return OUString("dotted");
        case LINESTYLE_DONTKNOW:        return OUString("dontknow");
        case LINESTYLE_DASH:            return OUString("dash");
        case LINESTYLE_LONGDASH:        return OUString("longdash");
        case LINESTYLE_DASHDOT:         return OUString("dashdot");
        case LINESTYLE_DASHDOTDOT:      return OUString("dashdotdot");
        case LINESTYLE_SMALLWAVE:       return OUString("smallwave");
        case LINESTYLE_WAVE:            return OUString("wave");
        case LINESTYLE_DOUBLEWAVE:      return OUString("doublewave");
        case LINESTYLE_BOLD:            return OUString("bold");
        case LINESTYLE_BOLDDOTTED:      return OUString("bolddotted");
        case LINESTYLE_BOLDDASH:        return OUString("bolddash");
        case LINESTYLE_BOLDLONGDASH:    return OUString("boldlongdash");
        case LINESTYLE_BOLDDASHDOT:     return OUString("bolddashdot");
        case LINESTYLE_BOLDDASHDOTDOT:  return OUString("bolddashdotdot");
        case LINESTYLE_BOLDWAVE:        return OUString("boldwave");
        case FontLineStyle_FORCE_EQUAL_SIZE: return OUString("equalsize");
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
    }
    return "";
}

OUString convertBitmapExTransparentType(TransparentType eType)
{
    switch (eType)
    {
        default:
        case TransparentType::NONE:   return OUString("none");
        case TransparentType::Bitmap: return OUString("bitmap");
        case TransparentType::Color:  return OUString("color");
    }
}

OUString convertMapUnitToString(MapUnit eUnit)
{
    switch (eUnit)
    {
        default:
        case MapUnit::LASTENUMDUMMY:    return OUString("LASTENUMDUMMY");
        case MapUnit::Map1000thInch:    return OUString("Map1000thInch");
        case MapUnit::Map100thInch:     return OUString("Map100thInch");
        case MapUnit::Map100thMM:       return OUString("Map100thMM");
        case MapUnit::Map10thInch:      return OUString("Map10thInch");
        case MapUnit::Map10thMM:        return OUString("Map10thMM");
        case MapUnit::MapAppFont:       return OUString("MapAppFont");
        case MapUnit::MapCM:            return OUString("MapCM");
        case MapUnit::MapInch:          return OUString("MapInch");
        case MapUnit::MapMM:            return OUString("MapMM");
        case MapUnit::MapPixel:         return OUString("MapPixel");
        case MapUnit::MapPoint:         return OUString("MapPoint");
        case MapUnit::MapRelative:      return OUString("MapRelative");
        case MapUnit::MapSysFont:       return OUString("MapSysFont");
        case MapUnit::MapTwip:          return OUString("MapTwip");
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
        case GradientStyle::Linear:     return OUString("Linear");
        case GradientStyle::Axial:      return OUString("Axial");
        case GradientStyle::Radial:     return OUString("Radial");
        case GradientStyle::Elliptical: return OUString("Elliptical");
        case GradientStyle::Square:     return OUString("Square");
        case GradientStyle::Rect:       return OUString("Rect");
        case GradientStyle::FORCE_EQUAL_SIZE: return OUString("ForceEqualSize");
    }
    return OUString();
}

OUString hex32(sal_uInt32 nNumber)
{
    std::stringstream ss;
    ss << std::hex << std::setfill ('0') << std::setw(8) << nNumber;
    return OUString::createFromAscii(ss.str().c_str());
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
    rWriter.attribute("right", rRectangle.Right());
    rWriter.attribute("bottom", rRectangle.Bottom());
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
    for(size_t nAction = 0; nAction < rMetaFile.GetActionSize(); ++nAction)
    {
        MetaAction* pAction = rMetaFile.GetAction(nAction);
        const MetaActionType nActionType = pAction->GetType();
        if (maFilter[nActionType])
            continue;

        OString sCurrentElementTag = convertLineStyleToString(nActionType);

        switch (nActionType)
        {
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

            case MetaActionType::TEXTLINECOLOR:
            {
                MetaTextLineColorAction* pMetaTextLineColorAction = static_cast<MetaTextLineColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("color", convertColorToString(pMetaTextLineColorAction->GetColor()));
                rWriter.endElement();
            }
            break;

            case MetaActionType::TEXTFILLCOLOR:
            {
                MetaTextFillColorAction* pMetaTextFillColorAction = static_cast<MetaTextFillColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("color", convertColorToString(pMetaTextFillColorAction->GetColor()));

                if (pMetaTextFillColorAction->IsSetting())
                    rWriter.attribute("setting", OUString("true"));

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
                rWriter.attribute("orientation", aFont.GetOrientation());
                rWriter.attribute("weight", convertFontWeigthToString(aFont.GetWeight()));

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

            case MetaActionType::TEXTCOLOR:
            {
                MetaTextColorAction* pMetaTextColorAction = static_cast<MetaTextColorAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("color", convertColorToString(pMetaTextColorAction->GetColor()));
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

                if (pMetaTextArrayAction->GetDXArray())
                {
                    rWriter.startElement("dxarray");
                    OUStringBuffer sDxLengthString;
                    for (sal_Int32 i = 0; i < aLength - aIndex; ++i)
                    {
                        sDxLengthString.append(OUString::number(pMetaTextArrayAction->GetDXArray()[aIndex + i]));
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

            case MetaActionType::CLIPREGION:
            {
                const MetaClipRegionAction* pA = static_cast< const MetaClipRegionAction* >(pAction);
                rWriter.startElement(sCurrentElementTag);

                // FIXME for now we dump only the bounding box; this is
                // enough for the tests we have, but may need extending to
                // dumping the real polypolygon in the future
                tools::Rectangle aRectangle = pA->GetRegion().GetBoundRect();
                writeRectangle(rWriter, aRectangle);
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

            case MetaActionType::COMMENT:
            {
                MetaCommentAction* pMetaCommentAction = static_cast<MetaCommentAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                if (pMetaCommentAction->GetDataSize() > 0)
                {
                    rWriter.attribute("datasize", pMetaCommentAction->GetDataSize());
                }
                if (!pMetaCommentAction->GetComment().isEmpty())
                {
                    rWriter.startElement("comment");
                    rWriter.content(pMetaCommentAction->GetComment());
                    rWriter.endElement();
                }

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
                rWriter.attribute("transparenttype", convertBitmapExTransparentType(pMeta->GetBitmapEx().GetTransparentType()));
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
                rWriter.attribute("transparenttype", convertBitmapExTransparentType(pMeta->GetBitmapEx().GetTransparentType()));
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
                rWriter.attribute("transparenttype", convertBitmapExTransparentType(pMeta->GetBitmapEx().GetTransparentType()));
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

            case MetaActionType::GRADIENT:
            {
                const MetaGradientAction* pMeta = static_cast<MetaGradientAction*>(pAction);
                tools::Rectangle aRectangle = pMeta->GetRect();
                Gradient aGradient = pMeta->GetGradient();

                rWriter.startElement(sCurrentElementTag);
                rWriter.attribute("style", convertGradientStyle(aGradient.GetStyle()));
                rWriter.attribute("startcolor", convertColorToString(aGradient.GetStartColor()));
                rWriter.attribute("endcolor", convertColorToString(aGradient.GetEndColor()));
                rWriter.attribute("angle", aGradient.GetAngle());
                rWriter.attribute("border", aGradient.GetBorder());
                rWriter.attribute("offsetx", aGradient.GetOfsX());
                rWriter.attribute("offsety", aGradient.GetOfsY());
                rWriter.attribute("startintensity", aGradient.GetStartIntensity());
                rWriter.attribute("endintensity", aGradient.GetEndIntensity());
                rWriter.attribute("steps", aGradient.GetSteps());

                rWriter.startElement("rectangle");
                writeRectangle(rWriter, aRectangle);
                rWriter.endElement();

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
