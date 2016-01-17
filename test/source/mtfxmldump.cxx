/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/mtfxmldump.hxx>
#include <test/xmltesttools.hxx>
#include <test/xmlwriter.hxx>

#include <vcl/metaact.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

#include <memory>

namespace
{

OUString flagToString(PushFlags nFlag)
{
    if (nFlag & PushFlags::LINECOLOR)
        return OUString("PushLineColor");
    else if (nFlag & PushFlags::FILLCOLOR)
        return OUString("PushFillColor");
    else if (nFlag & PushFlags::FONT)
        return OUString("PushFont");
    else if (nFlag & PushFlags::TEXTCOLOR)
        return OUString("PushTextColor");
    else if (nFlag & PushFlags::MAPMODE)
        return OUString("PushMapMode");
    else if (nFlag & PushFlags::CLIPREGION)
        return OUString("PushClipRegion");
    else if (nFlag & PushFlags::RASTEROP)
        return OUString("PushRasterOp");
    else if (nFlag & PushFlags::TEXTFILLCOLOR)
        return OUString("PushTextFillColor");
    else if (nFlag & PushFlags::TEXTALIGN)
        return OUString("PushTextAlign");
    else if (nFlag & PushFlags::REFPOINT)
        return OUString("PushRefPoint");
    else if (nFlag & PushFlags::TEXTLINECOLOR)
        return OUString("PushTextLineColor");
    else if (nFlag & PushFlags::TEXTLAYOUTMODE)
        return OUString("PushTextLayoutMode");
    else if (nFlag & PushFlags::TEXTLANGUAGE)
        return OUString("PushTextLanguage");
    else if (nFlag & PushFlags::OVERLINECOLOR)
        return OUString("PushOverlineColor");

    return OUString();
}

OUString collectPushFlags(PushFlags nFlags)
{
    if ((nFlags & PushFlags::ALL) == nFlags)
        return OUString("PushAll");
    else if ((nFlags & PUSH_ALLFONT) == nFlags)
        return OUString("PushAllFont");
    else if ((nFlags & PUSH_ALLTEXT) == nFlags)
        return OUString("PushAllText");

    OUString sFlags;

    for (sal_uInt16 nFlag = 1; nFlag > 0; nFlag <<= 1)
    {
        OUString sFlag = flagToString(static_cast<PushFlags>(nFlag));
        if (!sFlag.isEmpty())
        {
            if (!sFlags.isEmpty())
            {
                sFlags += ",";
            }
            sFlags += flagToString(static_cast<PushFlags>(nFlag));
        }
    }

    return sFlags;
}

OUString convertRopToString(RasterOp eRop)
{
    switch (eRop)
    {
        case ROP_OVERPAINT: return OUString("overpaint");
        case ROP_XOR:       return OUString("xor");
        case ROP_0:         return OUString("0");
        case ROP_1:         return OUString("1");
        case ROP_INVERT:    return OUString("invert");
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
        case LINE_NONE:    return OUString("none");
        case LINE_SOLID:   return OUString("solid");
        case LINE_DASH:    return OUString("dash");
        case LineStyle_FORCE_EQUAL_SIZE: return OUString("equalsize");
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

OUString convertFontWeigthToString(FontWeight eFontWeight)
{
    enum FontWeight { WEIGHT_DONTKNOW, WEIGHT_THIN, WEIGHT_ULTRALIGHT,
                  WEIGHT_LIGHT, WEIGHT_SEMILIGHT, WEIGHT_NORMAL,
                  WEIGHT_MEDIUM, WEIGHT_SEMIBOLD, WEIGHT_BOLD,
                  WEIGHT_ULTRABOLD, WEIGHT_BLACK, FontWeight_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };
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
        case FontWeight_FORCE_EQUAL_SIZE:    return OUString("equalsize");
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

xmlDocPtr MetafileXmlDump::dumpAndParse(const GDIMetaFile& rMetaFile, const OUString& rTempStreamName)
{
    std::unique_ptr<SvStream> pStream;

    if (rTempStreamName.isEmpty())
        pStream.reset(new SvMemoryStream());
    else
        pStream.reset(new SvFileStream(rTempStreamName, STREAM_STD_READWRITE | StreamMode::TRUNC));

    XmlWriter aWriter(pStream.get());
    aWriter.startDocument();
    aWriter.startElement("metafile");

    writeXml(rMetaFile, aWriter);

    aWriter.endElement();
    aWriter.endDocument();

    pStream->Seek(STREAM_SEEK_TO_BEGIN);

    xmlDocPtr pDoc = XmlTestTools::parseXmlStream(pStream.get());

    return pDoc;
}

void MetafileXmlDump::writeXml(const GDIMetaFile& rMetaFile, XmlWriter& rWriter)
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
            case MetaActionType::LINE:
            {
                MetaLineAction* pMetaLineAction = static_cast<MetaLineAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                rWriter.attribute("startx", pMetaLineAction->GetStartPoint().X());
                rWriter.attribute("starty", pMetaLineAction->GetStartPoint().Y());
                rWriter.attribute("endx", pMetaLineAction->GetEndPoint().X());
                rWriter.attribute("endy", pMetaLineAction->GetEndPoint().Y());

                LineInfo aLineInfo = pMetaLineAction->GetLineInfo();
                rWriter.attribute("style", convertLineStyleToString(aLineInfo.GetStyle()));
                rWriter.attribute("width", aLineInfo.GetWidth());
                rWriter.attribute("dashlen", aLineInfo.GetDashLen());
                rWriter.attribute("dashcount", aLineInfo.GetDashCount());
                rWriter.attribute("dotlen", aLineInfo.GetDotLen());
                rWriter.attribute("dotcount", aLineInfo.GetDotCount());
                rWriter.attribute("distance", aLineInfo.GetDistance());
                rWriter.attribute("join", convertLineJoinToString(aLineInfo.GetLineJoin()));
                rWriter.attribute("cap", convertLineCapToString(aLineInfo.GetLineCap()));
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

                if (pMetaRasterOpAction->GetRasterOp() != ROP_OVERPAINT)
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
                rWriter.attribute("width", aFont.GetSize().Width());
                rWriter.attribute("height", aFont.GetSize().Height());
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

            case MetaActionType::TEXTARRAY:
            {
                MetaTextArrayAction* pMetaTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                sal_Int32 aIndex = pMetaTextArrayAction->GetIndex();
                sal_Int32 aLength = pMetaTextArrayAction->GetLen();

                rWriter.attribute("x", pMetaTextArrayAction->GetPoint().X());
                rWriter.attribute("y", pMetaTextArrayAction->GetPoint().Y());
                rWriter.attribute("index", aIndex);
                rWriter.attribute("length", aLength);

                rWriter.startElement("dxarray");
                OUString sDxLengthString;
                for (sal_Int32 i = 0; i < aLength; ++i)
                {
                    sDxLengthString += OUString::number(pMetaTextArrayAction->GetDXArray()[aIndex+i]);
                    sDxLengthString += " ";
                }
                rWriter.content(sDxLengthString);
                rWriter.endElement();

                rWriter.startElement("text");
                rWriter.content(pMetaTextArrayAction->GetText());
                rWriter.endElement();

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
                Rectangle aRectangle = pA->GetRegion().GetBoundRect();
                rWriter.attribute("top",    aRectangle.Top());
                rWriter.attribute("left",   aRectangle.Left());
                rWriter.attribute("bottom", aRectangle.Bottom());
                rWriter.attribute("right",  aRectangle.Right());

                rWriter.endElement();
            }
            break;

            case MetaActionType::ISECTRECTCLIPREGION:
            {
                MetaISectRectClipRegionAction* pMetaISectRectClipRegionAction = static_cast<MetaISectRectClipRegionAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                Rectangle aRectangle = pMetaISectRectClipRegionAction->GetRect();
                rWriter.attribute("top",    aRectangle.Top());
                rWriter.attribute("left",   aRectangle.Left());
                rWriter.attribute("bottom", aRectangle.Bottom());
                rWriter.attribute("right",  aRectangle.Right());

                rWriter.endElement();
            }
            break;

            case MetaActionType::POLYLINE:
            {
                MetaPolyLineAction* pMetaPolyLineAction = static_cast<MetaPolyLineAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                tools::Polygon aPolygon = pMetaPolyLineAction->GetPolygon();
                for (sal_uInt16 i = 0; i < aPolygon.GetSize(); i++)
                {
                    rWriter.startElement("point");
                    rWriter.attribute("x", aPolygon[i].X());
                    rWriter.attribute("y", aPolygon[i].Y());
                    rWriter.endElement();
                }

                LineInfo aLineInfo = pMetaPolyLineAction->GetLineInfo();
                rWriter.attribute("style", convertLineStyleToString(aLineInfo.GetStyle()));
                rWriter.attribute("width", aLineInfo.GetWidth());
                rWriter.attribute("dashlen", aLineInfo.GetDashLen());
                rWriter.attribute("dotlen", aLineInfo.GetDotLen());
                rWriter.attribute("distance", aLineInfo.GetDistance());

                rWriter.endElement();
            }
            break;

            case MetaActionType::POLYGON:
            {
                MetaPolygonAction* pMetaPolygonAction = static_cast<MetaPolygonAction*>(pAction);
                rWriter.startElement(sCurrentElementTag);

                tools::Polygon aPolygon = pMetaPolygonAction->GetPolygon();
                for (sal_uInt16 i = 0; i < aPolygon.GetSize(); i++)
                {
                    rWriter.startElement("point");
                    rWriter.attribute("x", aPolygon[i].X());
                    rWriter.attribute("y", aPolygon[i].Y());
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

            default:
            {
                rWriter.element(sCurrentElementTag);
            }
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
