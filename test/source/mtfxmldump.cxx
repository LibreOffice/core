/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/mtfxmldump.hxx>
#include <test/xmlwriter.hxx>

#include <vcl/metaact.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>

namespace
{

int lclWriteCallback(void* pContext, const char* sBuffer, int nLen)
{
    SvStream* pStream = static_cast<SvStream*>(pContext);
    pStream->Write(sBuffer, nLen);
    return nLen;
}

int lclCloseCallback(void* pContext)
{
    SvStream* pStream = static_cast<SvStream*>(pContext);
    pStream->WriteChar(0);
    return 0;
}

OUString flagToString(sal_uInt16 nFlag)
{
    if (nFlag & PUSH_LINECOLOR)
        return OUString("PushLineColor");
    else if (nFlag & PUSH_FILLCOLOR)
        return OUString("PushFillColor");
    else if (nFlag & PUSH_FONT)
        return OUString("PushFont");
    else if (nFlag & PUSH_TEXTCOLOR)
        return OUString("PushTextColor");
    else if (nFlag & PUSH_MAPMODE)
        return OUString("PushMapMode");
    else if (nFlag & PUSH_CLIPREGION)
        return OUString("PushClipRegion");
    else if (nFlag & PUSH_RASTEROP)
        return OUString("PushRasterOp");
    else if (nFlag & PUSH_TEXTFILLCOLOR)
        return OUString("PushTextFillColor");
    else if (nFlag & PUSH_TEXTALIGN)
        return OUString("PushTextAlign");
    else if (nFlag & PUSH_REFPOINT)
        return OUString("PushRefPoint");
    else if (nFlag & PUSH_TEXTLINECOLOR)
        return OUString("PushTextLineColor");
    else if (nFlag & PUSH_TEXTLAYOUTMODE)
        return OUString("PushTextLayoutMode");
    else if (nFlag & PUSH_TEXTLANGUAGE)
        return OUString("PushTextLanguage");
    else if (nFlag & PUSH_OVERLINECOLOR)
        return OUString("PushOverlineColor");

    return OUString();
}

OUString collectPushFlags(sal_uInt16 nFlags)
{
    if ((nFlags & PUSH_ALL) == nFlags)
        return OUString("PushAll");
    else if ((nFlags & PUSH_ALLFONT) == nFlags)
        return OUString("PushAllFont");
    else if ((nFlags & PUSH_ALLTEXT) == nFlags)
        return OUString("PushAllText");

    OUString sFlags;

    for (sal_uInt16 nFlag = 1; nFlag > 0; nFlag <<= 1)
    {
        OUString sFlag = flagToString(nFlag);
        if (!sFlag.isEmpty())
        {
            if (!sFlags.isEmpty())
            {
                sFlags += ",";
            }
            sFlags += flagToString(nFlag);
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

} // anonymous namespace

MetafileXmlDump::MetafileXmlDump(SvStream& rStream) :
    maFilter(512, false),
    mrStream(rStream)
{}

MetafileXmlDump::~MetafileXmlDump()
{}

void MetafileXmlDump::filterActionType(const sal_uInt16 nActionType, bool bShouldFilter)
{
    maFilter[nActionType] = bShouldFilter;
}

void MetafileXmlDump::filterAllActionTypes()
{
    maFilter.assign(512, true);
}

void MetafileXmlDump::filterNoneActionTypes()
{
    maFilter.assign(512, false);
}

void MetafileXmlDump::dump(GDIMetaFile& rMetaFile)
{
    xmlOutputBufferPtr xmlOutBuffer = xmlOutputBufferCreateIO(lclWriteCallback, lclCloseCallback, &mrStream, NULL);
    xmlTextWriterPtr xmlWriter = xmlNewTextWriter( xmlOutBuffer );
    xmlTextWriterSetIndent( xmlWriter, 1 );

    XmlWriter aWriter(xmlWriter);
    aWriter.startDocument();
    aWriter.startElement("metafile");

    for(MetaAction* pAction = rMetaFile.FirstAction(); pAction != NULL; pAction = rMetaFile.NextAction())
    {
        const sal_uInt16 nActionType = pAction->GetType();
        if (maFilter[nActionType])
            continue;

        switch (nActionType)
        {
            case META_LINE_ACTION:
            {
                MetaLineAction* pMetaLineAction = static_cast<MetaLineAction*>(pAction);

                aWriter.startElement("line");
                aWriter.attribute("startx", pMetaLineAction->GetStartPoint().X());
                aWriter.attribute("starty", pMetaLineAction->GetStartPoint().Y());
                aWriter.attribute("endx", pMetaLineAction->GetEndPoint().X());
                aWriter.attribute("endy", pMetaLineAction->GetEndPoint().Y());

                LineInfo aLineInfo = pMetaLineAction->GetLineInfo();
                aWriter.attribute("style", convertLineStyleToString(aLineInfo.GetStyle()));
                aWriter.attribute("width", aLineInfo.GetWidth());
                aWriter.attribute("dashlen", aLineInfo.GetDashLen());
                aWriter.attribute("dotlen", aLineInfo.GetDotLen());
                aWriter.attribute("distance", aLineInfo.GetDistance());

                aWriter.endElement();
            }
            break;

            case META_PUSH_ACTION:
            {
                MetaPushAction* pMetaPushAction = static_cast<MetaPushAction*>(pAction);

                aWriter.startElement("push");
                aWriter.attribute("flags", collectPushFlags(pMetaPushAction->GetFlags()));
            }
            break;

            case META_POP_ACTION:
            {
                aWriter.endElement();
            }
            break;

            case META_RASTEROP_ACTION:
            {
                MetaRasterOpAction* pMetaRasterOpAction = static_cast<MetaRasterOpAction*>(pAction);

                aWriter.startElement("rop");
                if (pMetaRasterOpAction->GetRasterOp() != ROP_OVERPAINT)
                {
                    aWriter.attribute("operation", convertRopToString(pMetaRasterOpAction->GetRasterOp()));
                }
                aWriter.endElement();
            }
            break;

            case META_TEXTFILLCOLOR_ACTION:
            {
                MetaTextFillColorAction* pMetaTextFillColorAction = static_cast<MetaTextFillColorAction*>(pAction);

                aWriter.startElement("textfillcolor");

                aWriter.attribute("color", convertColorToString(pMetaTextFillColorAction->GetColor()));

                if (pMetaTextFillColorAction->IsSetting())
                    aWriter.attribute("setting", OUString("true"));

                aWriter.endElement();
            }
            break;

            case META_FONT_ACTION:
            {
                MetaFontAction* pMetaFontAction = static_cast<MetaFontAction*>(pAction);
                aWriter.startElement("font");

                Font aFont = pMetaFontAction->GetFont();

                aWriter.attribute("color", convertColorToString(aFont.GetColor()));
                aWriter.attribute("fillcolor", convertColorToString(aFont.GetFillColor()));
                aWriter.attribute("name", aFont.GetName());
                aWriter.attribute("stylename", aFont.GetStyleName());
                aWriter.attribute("width", aFont.GetSize().Width());
                aWriter.attribute("height", aFont.GetSize().Height());

                aWriter.endElement();
            }
            break;

            case META_TEXTALIGN_ACTION:
            {
                MetaTextAlignAction* pMetaTextAlignAction = static_cast<MetaTextAlignAction*>(pAction);
                aWriter.startElement("textalign");
                OUString sAlign = convertTextAlignToString(pMetaTextAlignAction->GetTextAlign());
                if (!sAlign.isEmpty())
                    aWriter.attribute("align", sAlign);
                aWriter.endElement();
            }
            break;

            case META_TEXTCOLOR_ACTION:
            {
                MetaTextColorAction* pMetaTextColorAction = static_cast<MetaTextColorAction*>(pAction);

                aWriter.startElement("textcolor");
                aWriter.attribute("color", convertColorToString(pMetaTextColorAction->GetColor()));
                aWriter.endElement();
            }
            break;

            case META_TEXTARRAY_ACTION:
            {
                MetaTextArrayAction* pMetaTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);

                aWriter.startElement("textarray");
                sal_Int32 aIndex = pMetaTextArrayAction->GetIndex();
                sal_Int32 aLength = pMetaTextArrayAction->GetLen();

                aWriter.attribute("x", pMetaTextArrayAction->GetPoint().X());
                aWriter.attribute("y", pMetaTextArrayAction->GetPoint().Y());
                aWriter.attribute("index", aIndex);
                aWriter.attribute("length", aLength);

                aWriter.startElement("dxarray");
                OUString sDxLengthString;
                for (sal_Int32 i = 0; i < aLength; ++i)
                {
                    sDxLengthString += OUString::number(pMetaTextArrayAction->GetDXArray()[aIndex+i]);
                    sDxLengthString += " ";
                }
                aWriter.content(sDxLengthString);
                aWriter.endElement();

                aWriter.startElement("text");
                aWriter.content(pMetaTextArrayAction->GetText());
                aWriter.endElement();

                aWriter.endElement();
            }
            break;

            case META_LINECOLOR_ACTION:
            {
                MetaTextLineColorAction* pMetaTextLineColorAction = static_cast<MetaTextLineColorAction*>(pAction);

                aWriter.startElement("linecolor");
                aWriter.attribute("color", convertColorToString(pMetaTextLineColorAction->GetColor()));
                aWriter.endElement();
            }
            break;

            case META_MAPMODE_ACTION:
            {
                aWriter.startElement("mapmode");
                aWriter.endElement();
            }
            break;

            case META_ISECTRECTCLIPREGION_ACTION:
            {
                MetaISectRectClipRegionAction* pMetaISectRectClipRegionAction = static_cast<MetaISectRectClipRegionAction*>(pAction);

                aWriter.startElement("sectrectclipregion");
                Rectangle aRectangle = pMetaISectRectClipRegionAction->GetRect();
                aWriter.attribute("top",    aRectangle.Top());
                aWriter.attribute("left",   aRectangle.Left());
                aWriter.attribute("bottom", aRectangle.Bottom());
                aWriter.attribute("right",  aRectangle.Right());

                aWriter.endElement();
            }
            break;

            case META_POLYLINE_ACTION:
            {
                MetaPolyLineAction* pMetaPolyLineAction = static_cast<MetaPolyLineAction*>(pAction);
                aWriter.startElement("polyline");

                Polygon aPolygon = pMetaPolyLineAction->GetPolygon();
                for (sal_uInt16 i = 0; i < aPolygon.GetSize(); i++)
                {
                    aWriter.startElement("point");
                    aWriter.attribute("x", aPolygon[i].X());
                    aWriter.attribute("y", aPolygon[i].Y());
                    aWriter.endElement();
                }

                LineInfo aLineInfo = pMetaPolyLineAction->GetLineInfo();
                aWriter.attribute("style", convertLineStyleToString(aLineInfo.GetStyle()));
                aWriter.attribute("width", aLineInfo.GetWidth());
                aWriter.attribute("dashlen", aLineInfo.GetDashLen());
                aWriter.attribute("dotlen", aLineInfo.GetDotLen());
                aWriter.attribute("distance", aLineInfo.GetDistance());

                aWriter.endElement();
            }
            break;

            case META_POLYGON_ACTION:
            {
                MetaPolygonAction* pMetaPolygonAction = static_cast<MetaPolygonAction*>(pAction);
                aWriter.startElement("polygon");

                Polygon aPolygon = pMetaPolygonAction->GetPolygon();
                for (sal_uInt16 i = 0; i < aPolygon.GetSize(); i++)
                {
                    aWriter.startElement("point");
                    aWriter.attribute("x", aPolygon[i].X());
                    aWriter.attribute("y", aPolygon[i].Y());
                    aWriter.endElement();
                }

                aWriter.endElement();
            }
            break;

        }
    }

    aWriter.endElement();
    aWriter.endDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
