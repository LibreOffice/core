/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/log.hxx>
#include <osl/thread.h>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <vcl/filter/SvmReader.hxx>
#include <vcl/TypeSerializer.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

#include "SvmConverter.hxx"

namespace
{
class DepthGuard
{
private:
    ImplMetaReadData& m_rData;
    rtl_TextEncoding m_eOrigCharSet;

public:
    DepthGuard(ImplMetaReadData& rData, SvStream const& rIStm)
        : m_rData(rData)
        , m_eOrigCharSet(m_rData.meActualCharSet)
    {
        ++m_rData.mnParseDepth;
        m_rData.meActualCharSet = rIStm.GetStreamCharSet();
    }
    bool TooDeep() const { return m_rData.mnParseDepth > 1024; }
    ~DepthGuard()
    {
        --m_rData.mnParseDepth;
        m_rData.meActualCharSet = m_eOrigCharSet;
    }
};
}

SvmReader::SvmReader(SvStream& rIStm)
    : mrStream(rIStm)
{
}

SvStream& SvmReader::Read(GDIMetaFile& rMetaFile, ImplMetaReadData* pData)
{
    if (mrStream.GetError())
    {
        SAL_WARN("vcl.gdi", "Stream error: " << mrStream.GetError());
        return mrStream;
    }

    sal_uInt64 nStmPos = mrStream.Tell();
    SvStreamEndian nOldFormat = mrStream.GetEndian();

    mrStream.SetEndian(SvStreamEndian::LITTLE);

    try
    {
        char aId[7];
        aId[0] = 0;
        aId[6] = 0;
        mrStream.ReadBytes(aId, 6);
        if (mrStream.good() && !strcmp(aId, "VCLMTF"))
        {
            // new format
            sal_uInt32 nStmCompressMode = 0;
            sal_uInt32 nCount = 0;
            std::unique_ptr<VersionCompatRead> pCompat(new VersionCompatRead(mrStream));

            mrStream.ReadUInt32(nStmCompressMode);
            TypeSerializer aSerializer(mrStream);
            MapMode aMapMode;
            aSerializer.readMapMode(aMapMode);
            rMetaFile.SetPrefMapMode(aMapMode);
            Size aSize;
            aSerializer.readSize(aSize);
            rMetaFile.SetPrefSize(aSize);
            mrStream.ReadUInt32(nCount);

            pCompat.reset(); // destructor writes stuff into the header

            std::unique_ptr<ImplMetaReadData> xReadData;
            if (!pData)
            {
                xReadData.reset(new ImplMetaReadData);
                pData = xReadData.get();
            }
            DepthGuard aDepthGuard(*pData, mrStream);

            if (aDepthGuard.TooDeep())
                throw std::runtime_error("too much recursion");

            for (sal_uInt32 nAction = 0; (nAction < nCount) && !mrStream.eof(); nAction++)
            {
                rtl::Reference<MetaAction> pAction = MetaActionHandler(pData);
                if (pAction)
                {
                    if (pAction->GetType() == MetaActionType::COMMENT)
                    {
                        MetaCommentAction* pCommentAct
                            = static_cast<MetaCommentAction*>(pAction.get());

                        if (pCommentAct->GetComment() == "EMF_PLUS")
                            rMetaFile.UseCanvas(true);
                    }
                    rMetaFile.AddAction(pAction);
                }
            }
        }
        else
        {
            mrStream.Seek(nStmPos);
            SVMConverter(mrStream, rMetaFile);
        }
    }
    catch (...)
    {
        SAL_WARN("vcl", "GDIMetaFile exception during load");
        mrStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
    };

    // check for errors
    if (mrStream.GetError())
    {
        rMetaFile.Clear();
        mrStream.Seek(nStmPos);
    }

    mrStream.SetEndian(nOldFormat);
    return mrStream;
}

rtl::Reference<MetaAction> SvmReader::MetaActionHandler(ImplMetaReadData* pData)
{
    rtl::Reference<MetaAction> pAction;
    sal_uInt16 nTmp = 0;
    mrStream.ReadUInt16(nTmp);
    MetaActionType nType = static_cast<MetaActionType>(nTmp);

    switch (nType)
    {
        case MetaActionType::NONE:
            return DefaultHandler();
            break;
        case MetaActionType::PIXEL:
            return PixelHandler();
            break;
        case MetaActionType::POINT:
            return PointHandler();
            break;
        case MetaActionType::LINE:
            return LineHandler();
            break;
        case MetaActionType::RECT:
            return RectHandler();
            break;
        case MetaActionType::ROUNDRECT:
            return RoundRectHandler();
            break;
        case MetaActionType::ELLIPSE:
            return EllipseHandler();
            break;
        case MetaActionType::ARC:
            return ArcHandler();
            break;
        case MetaActionType::PIE:
            return PieHandler();
            break;
        case MetaActionType::CHORD:
            return ChordHandler();
            break;
        case MetaActionType::POLYLINE:
            return PolyLineHandler();
            break;
        case MetaActionType::POLYGON:
            return PolygonHandler();
            break;
        case MetaActionType::POLYPOLYGON:
            return PolyPolygonHandler();
            break;
        case MetaActionType::TEXT:
            return TextHandler(pData);
            break;
        case MetaActionType::TEXTARRAY:
            return TextArrayHandler(pData);
            break;
        case MetaActionType::STRETCHTEXT:
            return StretchTextHandler(pData);
            break;
        case MetaActionType::TEXTRECT:
            return TextRectHandler(pData);
            break;
        case MetaActionType::TEXTLINE:
            return TextLineHandler();
            break;
        case MetaActionType::BMP:
            return BmpHandler();
            break;
        case MetaActionType::BMPSCALE:
            return BmpScaleHandler();
            break;
        case MetaActionType::BMPSCALEPART:
            return BmpScalePartHandler();
            break;
        case MetaActionType::BMPEX:
            return BmpExHandler();
            break;
        case MetaActionType::BMPEXSCALE:
            return BmpExScaleHandler();
            break;
        case MetaActionType::BMPEXSCALEPART:
            return BmpExScalePartHandler();
            break;
        case MetaActionType::MASK:
            return MaskHandler();
            break;
        case MetaActionType::MASKSCALE:
            return MaskScaleHandler();
            break;
        case MetaActionType::MASKSCALEPART:
            return MaskScalePartHandler();
            break;
        case MetaActionType::GRADIENT:
            return GradientHandler();
            break;
        case MetaActionType::GRADIENTEX:
            return GradientExHandler();
            break;
        case MetaActionType::HATCH:
            return HatchHandler();
            break;
        case MetaActionType::WALLPAPER:
            return WallpaperHandler();
            break;
        case MetaActionType::CLIPREGION:
            return ClipRegionHandler();
            break;
        case MetaActionType::ISECTRECTCLIPREGION:
            return ISectRectClipRegionHandler();
            break;
        case MetaActionType::ISECTREGIONCLIPREGION:
            return ISectRegionClipRegionHandler();
            break;
        case MetaActionType::MOVECLIPREGION:
            return MoveClipRegionHandler();
            break;
        case MetaActionType::LINECOLOR:
            return LineColorHandler();
            break;
        case MetaActionType::FILLCOLOR:
            return FillColorHandler();
            break;
        case MetaActionType::TEXTCOLOR:
            return TextColorHandler();
            break;
        case MetaActionType::TEXTFILLCOLOR:
            return TextFillColorHandler();
            break;
        case MetaActionType::TEXTLINECOLOR:
            return TextLineColorHandler();
            break;
        case MetaActionType::OVERLINECOLOR:
            return OverlineColorHandler();
            break;
        case MetaActionType::TEXTALIGN:
            return TextAlignHandler();
            break;
        case MetaActionType::MAPMODE:
            return MapModeHandler();
            break;
        case MetaActionType::FONT:
            return FontHandler(pData);
            break;
        case MetaActionType::PUSH:
            return PushHandler();
            break;
        case MetaActionType::POP:
            return PopHandler();
            break;
        case MetaActionType::RASTEROP:
            return RasterOpHandler();
            break;
        case MetaActionType::Transparent:
            return TransparentHandler();
            break;
        case MetaActionType::FLOATTRANSPARENT:
            return FloatTransparentHandler(pData);
            break;
        case MetaActionType::EPS:
            return EPSHandler();
            break;
        case MetaActionType::REFPOINT:
            return RefPointHandler();
            break;
        case MetaActionType::COMMENT:
            return CommentHandler();
            break;
        case MetaActionType::LAYOUTMODE:
            return LayoutModeHandler();
            break;
        case MetaActionType::TEXTLANGUAGE:
            return TextLanguageHandler();
            break;

        default:
        {
            VersionCompatRead aCompat(mrStream);
        }
        break;
    }

    return pAction;
}

void SvmReader::ReadColor(Color& rColor)
{
    sal_uInt32 nTmp(0);
    mrStream.ReadUInt32(nTmp);
    rColor = ::Color(ColorTransparency, nTmp);
}

rtl::Reference<MetaAction> SvmReader::LineColorHandler()
{
    rtl::Reference<MetaLineColorAction> pAction(new MetaLineColorAction);

    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);
    bool aBool(false);
    mrStream.ReadCharAsBool(aBool);

    pAction->SetSetting(aBool);
    pAction->SetColor(aColor);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::FillColorHandler()
{
    rtl::Reference<MetaFillColorAction> pAction(new MetaFillColorAction);

    VersionCompatRead aCompat(mrStream);

    Color aColor;
    ReadColor(aColor);
    bool aBool(false);
    mrStream.ReadCharAsBool(aBool);

    pAction->SetColor(aColor);
    pAction->SetSetting(aBool);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::RectHandler()
{
    rtl::Reference<MetaRectAction> pAction(new MetaRectAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    pAction->SetRect(aRectangle);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PointHandler()
{
    rtl::Reference<MetaPointAction> pAction(new MetaPointAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);
    pAction->SetPoint(aPoint);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PixelHandler()
{
    rtl::Reference<MetaPixelAction> pAction(new MetaPixelAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);
    Color aColor;
    ReadColor(aColor);

    pAction->SetPoint(aPoint);
    pAction->SetColor(aColor);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::LineHandler()
{
    rtl::Reference<MetaLineAction> pAction(new MetaLineAction);

    VersionCompatRead aCompat(mrStream);

    // Version 1
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    Point aEndPoint;
    aSerializer.readPoint(aPoint);
    aSerializer.readPoint(aEndPoint);

    pAction->SetStartPoint(aPoint);
    pAction->SetEndPoint(aEndPoint);

    // Version 2
    if (aCompat.GetVersion() >= 2)
    {
        LineInfo aLineInfo;
        ReadLineInfo(mrStream, aLineInfo);
        pAction->SetLineInfo(aLineInfo);
    }

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::RoundRectHandler()
{
    rtl::Reference<MetaRoundRectAction> pAction(new MetaRoundRectAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    sal_uInt32 HorzRound(0);
    sal_uInt32 VertRound(0);
    mrStream.ReadUInt32(HorzRound).ReadUInt32(VertRound);

    pAction->SetRect(aRectangle);
    pAction->SetHorzRound(HorzRound);
    pAction->SetVertRound(VertRound);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::EllipseHandler()
{
    rtl::Reference<MetaEllipseAction> pAction(new MetaEllipseAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);

    pAction->SetRect(aRectangle);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::ArcHandler()
{
    rtl::Reference<MetaArcAction> pAction(new MetaArcAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Point aEndPoint;
    aSerializer.readPoint(aEndPoint);

    pAction->SetRect(aRectangle);
    pAction->SetStartPoint(aPoint);
    pAction->SetEndPoint(aEndPoint);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PieHandler()
{
    rtl::Reference<MetaPieAction> pAction(new MetaPieAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Point aEndPoint;
    aSerializer.readPoint(aEndPoint);

    pAction->SetRect(aRectangle);
    pAction->SetStartPoint(aPoint);
    pAction->SetEndPoint(aEndPoint);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::ChordHandler()
{
    rtl::Reference<MetaChordAction> pAction(new MetaChordAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Point aEndPoint;
    aSerializer.readPoint(aEndPoint);

    pAction->SetRect(aRectangle);
    pAction->SetStartPoint(aPoint);
    pAction->SetEndPoint(aEndPoint);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PolyLineHandler()
{
    rtl::Reference<MetaPolyLineAction> pAction(new MetaPolyLineAction);

    VersionCompatRead aCompat(mrStream);

    // Version 1
    tools::Polygon aPolygon;
    ReadPolygon(mrStream, aPolygon);

    // Version 2
    if (aCompat.GetVersion() >= 2)
    {
        LineInfo aLineInfo;
        ReadLineInfo(mrStream, aLineInfo);
        pAction->SetLineInfo(aLineInfo);
    }
    if (aCompat.GetVersion() >= 3)
    {
        sal_uInt8 bHasPolyFlags(0);
        mrStream.ReadUChar(bHasPolyFlags);
        if (bHasPolyFlags)
            aPolygon.Read(mrStream);
    }
    pAction->SetPolygon(aPolygon);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PolygonHandler()
{
    rtl::Reference<MetaPolygonAction> pAction(new MetaPolygonAction);

    VersionCompatRead aCompat(mrStream);

    tools::Polygon aPolygon;
    ReadPolygon(mrStream, aPolygon); // Version 1

    if (aCompat.GetVersion() >= 2) // Version 2
    {
        sal_uInt8 bHasPolyFlags(0);
        mrStream.ReadUChar(bHasPolyFlags);
        if (bHasPolyFlags)
            aPolygon.Read(mrStream);
    }

    pAction->SetPolygon(aPolygon);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PolyPolygonHandler()
{
    rtl::Reference<MetaPolyPolygonAction> pAction(new MetaPolyPolygonAction);

    VersionCompatRead aCompat(mrStream);
    tools::PolyPolygon aPolyPolygon;
    ReadPolyPolygon(mrStream, aPolyPolygon); // Version 1

    if (aCompat.GetVersion() < 2) // Version 2
    {
        pAction->SetPolyPolygon(aPolyPolygon);
        return pAction;
    }

    sal_uInt16 nNumberOfComplexPolygons(0);
    mrStream.ReadUInt16(nNumberOfComplexPolygons);
    const size_t nMinRecordSize = sizeof(sal_uInt16);
    const size_t nMaxRecords = mrStream.remainingSize() / nMinRecordSize;
    if (nNumberOfComplexPolygons > nMaxRecords)
    {
        SAL_WARN("vcl.gdi", "Parsing error: " << nMaxRecords << " max possible entries, but "
                                              << nNumberOfComplexPolygons
                                              << " claimed, truncating");
        nNumberOfComplexPolygons = nMaxRecords;
    }
    for (sal_uInt16 i = 0; i < nNumberOfComplexPolygons; ++i)
    {
        sal_uInt16 nIndex(0);
        mrStream.ReadUInt16(nIndex);
        tools::Polygon aPoly;
        aPoly.Read(mrStream);
        if (nIndex >= aPolyPolygon.Count())
        {
            SAL_WARN("vcl.gdi", "svm contains polygon index " << nIndex
                                                              << " outside possible range "
                                                              << aPolyPolygon.Count());
            continue;
        }
        aPolyPolygon.Replace(aPoly, nIndex);
    }

    pAction->SetPolyPolygon(aPolyPolygon);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextHandler(const ImplMetaReadData* pData)
{
    rtl::Reference<MetaTextAction> pAction(new MetaTextAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);
    OUString aStr = mrStream.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt16 nTmpIndex(0);
    mrStream.ReadUInt16(nTmpIndex);
    sal_uInt16 nTmpLen(0);
    mrStream.ReadUInt16(nTmpLen);

    pAction->SetPoint(aPoint);

    if (aCompat.GetVersion() >= 2) // Version 2
        aStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(mrStream);

    if (nTmpIndex > aStr.getLength())
    {
        SAL_WARN("vcl.gdi", "inconsistent offset");
        nTmpIndex = aStr.getLength();
    }

    if (nTmpLen > aStr.getLength() - nTmpIndex)
    {
        SAL_WARN("vcl.gdi", "inconsistent len");
        nTmpLen = aStr.getLength() - nTmpIndex;
    }

    pAction->SetIndex(nTmpIndex);
    pAction->SetLen(nTmpLen);

    pAction->SetText(aStr);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextArrayHandler(const ImplMetaReadData* pData)
{
    rtl::Reference<MetaTextArrayAction> pAction(new MetaTextArrayAction);

    std::vector<sal_Int32> aArray;

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);
    pAction->SetPoint(aPoint);

    OUString aStr = mrStream.ReadUniOrByteString(pData->meActualCharSet);
    pAction->SetText(aStr);

    sal_uInt16 nTmpIndex(0);
    mrStream.ReadUInt16(nTmpIndex);

    sal_uInt16 nTmpLen(0);
    mrStream.ReadUInt16(nTmpLen);

    sal_Int32 nAryLen(0);
    mrStream.ReadInt32(nAryLen);

    if (nTmpLen > aStr.getLength() - nTmpIndex)
    {
        SAL_WARN("vcl.gdi", "inconsistent offset and len");
        pAction->SetIndex(0);
        pAction->SetLen(aStr.getLength());
        return pAction;
    }

    pAction->SetIndex(nTmpIndex);
    pAction->SetLen(nTmpLen);

    if (nAryLen)
    {
        // #i9762#, #106172# Ensure that DX array is at least mnLen entries long
        if (nTmpLen >= nAryLen)
        {
            try
            {
                aArray.resize(nTmpLen);
                sal_Int32 i;
                sal_Int32 val(0);
                for (i = 0; i < nAryLen; i++)
                {
                    mrStream.ReadInt32(val);
                    aArray[i] = val;
                }
                // #106172# setup remainder
                for (; i < nTmpLen; i++)
                    aArray[i] = 0;
            }
            catch (std::bad_alloc&)
            {
            }
        }
        else
        {
            return pAction;
        }
    }

    if (aCompat.GetVersion() >= 2) // Version 2
    {
        aStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(mrStream);
        pAction->SetText(aStr);

        if (nTmpLen > aStr.getLength() - nTmpIndex)
        {
            SAL_WARN("vcl.gdi", "inconsistent offset and len");
            pAction->SetIndex(0);
            pAction->SetLen(aStr.getLength());
            aArray.clear();
        }
    }

    if (!aArray.empty())
        pAction->SetDXArray(std::move(aArray));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::StretchTextHandler(const ImplMetaReadData* pData)
{
    rtl::Reference<MetaStretchTextAction> pAction(new MetaStretchTextAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);
    OUString aStr = mrStream.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt32 nTmpWidth(0);
    mrStream.ReadUInt32(nTmpWidth);
    sal_uInt16 nTmpIndex(0);
    mrStream.ReadUInt16(nTmpIndex);
    sal_uInt16 nTmpLen(0);
    mrStream.ReadUInt16(nTmpLen);

    pAction->SetPoint(aPoint);
    pAction->SetWidth(nTmpWidth);
    pAction->SetIndex(nTmpIndex);
    pAction->SetLen(nTmpLen);

    if (aCompat.GetVersion() >= 2) // Version 2
        aStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(mrStream);

    pAction->SetText(aStr);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextRectHandler(const ImplMetaReadData* pData)
{
    rtl::Reference<MetaTextRectAction> pAction(new MetaTextRectAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRect;
    aSerializer.readRectangle(aRect);
    OUString aStr = mrStream.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt16 nTmp(0);
    mrStream.ReadUInt16(nTmp);

    pAction->SetRect(aRect);
    pAction->SetStyle(static_cast<DrawTextFlags>(nTmp));

    if (aCompat.GetVersion() >= 2) // Version 2
        aStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(mrStream);

    pAction->SetText(aStr);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextLineHandler()
{
    rtl::Reference<MetaTextLineAction> pAction(new MetaTextLineAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPos;
    aSerializer.readPoint(aPos);
    sal_Int32 nTempWidth(0);
    mrStream.ReadInt32(nTempWidth);

    pAction->SetStartPoint(aPos);
    pAction->SetWidth(nTempWidth);

    sal_uInt32 nTempStrikeout(0);
    mrStream.ReadUInt32(nTempStrikeout);
    sal_uInt32 nTempUnderline(0);
    mrStream.ReadUInt32(nTempUnderline);

    pAction->SetStrikeout(static_cast<FontStrikeout>(nTempStrikeout & SAL_MAX_ENUM));
    pAction->SetUnderline(static_cast<FontLineStyle>(nTempUnderline & SAL_MAX_ENUM));

    if (aCompat.GetVersion() >= 2)
    {
        sal_uInt32 nTempOverline(0);
        mrStream.ReadUInt32(nTempOverline);
        pAction->SetOverline(static_cast<FontLineStyle>(nTempOverline & SAL_MAX_ENUM));
    }

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::BmpHandler()
{
    rtl::Reference<MetaBmpAction> pAction(new MetaBmpAction);

    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);

    pAction->SetBitmap(aBmp);
    pAction->SetPoint(aPoint);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::BmpScaleHandler()
{
    rtl::Reference<MetaBmpScaleAction> pAction(new MetaBmpScaleAction);

    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Size aSz;
    aSerializer.readSize(aSz);

    pAction->SetBitmap(aBmp);
    pAction->SetPoint(aPoint);
    pAction->SetSize(aSz);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::BmpScalePartHandler()
{
    rtl::Reference<MetaBmpScalePartAction> pAction(new MetaBmpScalePartAction);

    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aDestPoint;
    aSerializer.readPoint(aDestPoint);
    Size aDestSize;
    aSerializer.readSize(aDestSize);
    Point aSrcPoint;
    aSerializer.readPoint(aSrcPoint);
    Size aSrcSize;
    aSerializer.readSize(aSrcSize);

    pAction->SetBitmap(aBmp);
    pAction->SetDestPoint(aDestPoint);
    pAction->SetDestSize(aDestSize);
    pAction->SetSrcPoint(aSrcPoint);
    pAction->SetSrcSize(aSrcSize);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::BmpExHandler()
{
    rtl::Reference<MetaBmpExAction> pAction(new MetaBmpExAction);

    VersionCompatRead aCompat(mrStream);
    BitmapEx aBmpEx;
    ReadDIBBitmapEx(aBmpEx, mrStream);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);

    pAction->SetPoint(aPoint);
    pAction->SetBitmapEx(aBmpEx);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::BmpExScaleHandler()
{
    rtl::Reference<MetaBmpExScaleAction> pAction(new MetaBmpExScaleAction);

    VersionCompatRead aCompat(mrStream);
    BitmapEx aBmpEx;
    ReadDIBBitmapEx(aBmpEx, mrStream);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Size aSize;
    aSerializer.readSize(aSize);

    pAction->SetBitmapEx(aBmpEx);
    pAction->SetPoint(aPoint);
    pAction->SetSize(aSize);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::BmpExScalePartHandler()
{
    rtl::Reference<MetaBmpExScalePartAction> pAction(new MetaBmpExScalePartAction);

    VersionCompatRead aCompat(mrStream);
    BitmapEx aBmpEx;
    ReadDIBBitmapEx(aBmpEx, mrStream);
    TypeSerializer aSerializer(mrStream);
    Point aDstPoint;
    aSerializer.readPoint(aDstPoint);
    Size aDstSize;
    aSerializer.readSize(aDstSize);
    Point aSrcPoint;
    aSerializer.readPoint(aSrcPoint);
    Size aSrcSize;
    aSerializer.readSize(aSrcSize);

    pAction->SetBitmapEx(aBmpEx);
    pAction->SetDestPoint(aDstPoint);
    pAction->SetDestSize(aDstSize);
    pAction->SetSrcPoint(aSrcPoint);
    pAction->SetSrcSize(aSrcSize);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::MaskHandler()
{
    rtl::Reference<MetaMaskAction> pAction(new MetaMaskAction);

    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);

    pAction->SetBitmap(aBmp);
    pAction->SetPoint(aPoint);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::MaskScaleHandler()
{
    rtl::Reference<MetaMaskScaleAction> pAction(new MetaMaskScaleAction);

    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Size aSize;
    aSerializer.readSize(aSize);

    pAction->SetBitmap(aBmp);
    pAction->SetPoint(aPoint);
    pAction->SetSize(aSize);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::MaskScalePartHandler()
{
    rtl::Reference<MetaMaskScalePartAction> pAction(new MetaMaskScalePartAction);

    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    Color aColor;
    ReadColor(aColor);
    TypeSerializer aSerializer(mrStream);
    Point aDstPt;
    aSerializer.readPoint(aDstPt);
    Size aDstSz;
    aSerializer.readSize(aDstSz);
    Point aSrcPt;
    aSerializer.readPoint(aSrcPt);
    Size aSrcSz;
    aSerializer.readSize(aSrcSz);

    pAction->SetBitmap(aBmp);
    pAction->SetColor(aColor);
    pAction->SetDestPoint(aDstPt);
    pAction->SetDestSize(aDstSz);
    pAction->SetSrcPoint(aSrcPt);
    pAction->SetSrcSize(aSrcSz);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::GradientHandler()
{
    rtl::Reference<MetaGradientAction> pAction(new MetaGradientAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRect;
    aSerializer.readRectangle(aRect);
    Gradient aGradient;
    aSerializer.readGradient(aGradient);

    pAction->SetRect(aRect);
    pAction->SetGradient(aGradient);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::GradientExHandler()
{
    rtl::Reference<MetaGradientExAction> pAction(new MetaGradientExAction);

    VersionCompatRead aCompat(mrStream);
    tools::PolyPolygon aPolyPoly;
    ReadPolyPolygon(mrStream, aPolyPoly);
    TypeSerializer aSerializer(mrStream);
    Gradient aGradient;
    aSerializer.readGradient(aGradient);

    pAction->SetGradient(aGradient);
    pAction->SetPolyPolygon(aPolyPoly);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::HatchHandler()
{
    rtl::Reference<MetaHatchAction> pAction(new MetaHatchAction);

    VersionCompatRead aCompat(mrStream);
    tools::PolyPolygon aPolyPoly;
    ReadPolyPolygon(mrStream, aPolyPoly);
    Hatch aHatch;
    ReadHatch(mrStream, aHatch);

    pAction->SetPolyPolygon(aPolyPoly);
    pAction->SetHatch(aHatch);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::WallpaperHandler()
{
    rtl::Reference<MetaWallpaperAction> pAction(new MetaWallpaperAction);

    VersionCompatRead aCompat(mrStream);
    Wallpaper aWallpaper;
    ReadWallpaper(mrStream, aWallpaper);

    pAction->SetWallpaper(aWallpaper);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::ClipRegionHandler()
{
    rtl::Reference<MetaClipRegionAction> pAction(new MetaClipRegionAction);

    VersionCompatRead aCompat(mrStream);
    vcl::Region aRegion;
    ReadRegion(mrStream, aRegion);
    bool aClip(false);
    mrStream.ReadCharAsBool(aClip);

    pAction->SetRegion(aRegion);
    pAction->SetClipping(aClip);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::ISectRectClipRegionHandler()
{
    rtl::Reference<MetaISectRectClipRegionAction> pAction(new MetaISectRectClipRegionAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);
    tools::Rectangle aRect;
    aSerializer.readRectangle(aRect);

    pAction->SetRect(aRect);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::ISectRegionClipRegionHandler()
{
    rtl::Reference<MetaISectRegionClipRegionAction> pAction(new MetaISectRegionClipRegionAction);

    VersionCompatRead aCompat(mrStream);
    vcl::Region aRegion;
    ReadRegion(mrStream, aRegion);
    pAction->SetRegion(aRegion);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::MoveClipRegionHandler()
{
    rtl::Reference<MetaMoveClipRegionAction> pAction(new MetaMoveClipRegionAction);

    VersionCompatRead aCompat(mrStream);
    sal_Int32 nTmpHM(0), nTmpVM(0);
    mrStream.ReadInt32(nTmpHM).ReadInt32(nTmpVM);

    pAction->SetHorzMove(nTmpHM);
    pAction->SetVertMove(nTmpVM);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextColorHandler()
{
    rtl::Reference<MetaTextColorAction> pAction(new MetaTextColorAction);

    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);

    pAction->SetColor(aColor);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextFillColorHandler()
{
    rtl::Reference<MetaTextFillColorAction> pAction(new MetaTextFillColorAction);

    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);
    bool bSet(false);
    mrStream.ReadCharAsBool(bSet);

    pAction->SetColor(aColor);
    pAction->SetSetting(bSet);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextLineColorHandler()
{
    rtl::Reference<MetaTextLineColorAction> pAction(new MetaTextLineColorAction);

    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);
    bool bSet(false);
    mrStream.ReadCharAsBool(bSet);

    pAction->SetColor(aColor);
    pAction->SetSetting(bSet);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::OverlineColorHandler()
{
    rtl::Reference<MetaOverlineColorAction> pAction(new MetaOverlineColorAction);

    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);
    bool bSet(false);
    mrStream.ReadCharAsBool(bSet);

    pAction->SetColor(aColor);
    pAction->SetSetting(bSet);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextAlignHandler()
{
    rtl::Reference<MetaTextAlignAction> pAction(new MetaTextAlignAction);

    VersionCompatRead aCompat(mrStream);
    sal_uInt16 nTmp16(0);
    mrStream.ReadUInt16(nTmp16);

    pAction->SetTextAlign(static_cast<TextAlign>(nTmp16));

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::MapModeHandler()
{
    rtl::Reference<MetaMapModeAction> pAction(new MetaMapModeAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);
    MapMode aMapMode;
    aSerializer.readMapMode(aMapMode);

    pAction->SetMapMode(aMapMode);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::FontHandler(ImplMetaReadData* pData)
{
    rtl::Reference<MetaFontAction> pAction(new MetaFontAction);

    VersionCompatRead aCompat(mrStream);
    vcl::Font aFont;
    ReadFont(mrStream, aFont);
    pData->meActualCharSet = aFont.GetCharSet();
    if (pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW)
        pData->meActualCharSet = osl_getThreadTextEncoding();

    pAction->SetFont(aFont);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PushHandler()
{
    rtl::Reference<MetaPushAction> pAction(new MetaPushAction);

    VersionCompatRead aCompat(mrStream);
    sal_uInt16 nTmp(0);
    mrStream.ReadUInt16(nTmp);

    pAction->SetPushFlags(static_cast<vcl::PushFlags>(nTmp));

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PopHandler()
{
    rtl::Reference<MetaPopAction> pAction(new MetaPopAction);

    VersionCompatRead aCompat(mrStream);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::RasterOpHandler()
{
    rtl::Reference<MetaRasterOpAction> pAction(new MetaRasterOpAction);

    sal_uInt16 nTmp16(0);

    VersionCompatRead aCompat(mrStream);
    mrStream.ReadUInt16(nTmp16);

    pAction->SetRasterOp(static_cast<RasterOp>(nTmp16));

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TransparentHandler()
{
    rtl::Reference<MetaTransparentAction> pAction(new MetaTransparentAction);

    VersionCompatRead aCompat(mrStream);
    tools::PolyPolygon aPolyPoly;
    ReadPolyPolygon(mrStream, aPolyPoly);
    sal_uInt16 nTransPercent(0);
    mrStream.ReadUInt16(nTransPercent);

    pAction->SetPolyPolygon(aPolyPoly);
    pAction->SetTransparence(nTransPercent);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::FloatTransparentHandler(ImplMetaReadData* pData)
{
    rtl::Reference<MetaFloatTransparentAction> pAction(new MetaFloatTransparentAction);

    VersionCompatRead aCompat(mrStream);
    GDIMetaFile aMtf;
    SvmReader aReader(mrStream);
    aReader.Read(aMtf, pData);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Size aSize;
    aSerializer.readSize(aSize);
    Gradient aGradient;
    aSerializer.readGradient(aGradient);

    pAction->SetGDIMetaFile(aMtf);
    pAction->SetPoint(aPoint);
    pAction->SetSize(aSize);
    pAction->SetGradient(aGradient);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::EPSHandler()
{
    rtl::Reference<MetaEPSAction> pAction(new MetaEPSAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);
    GfxLink aGfxLink;
    aSerializer.readGfxLink(aGfxLink);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Size aSize;
    aSerializer.readSize(aSize);
    GDIMetaFile aSubst;
    Read(aSubst);

    pAction->SetLink(aGfxLink);
    pAction->SetPoint(aPoint);
    pAction->SetSize(aSize);
    pAction->SetSubstitute(aSubst);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::RefPointHandler()
{
    rtl::Reference<MetaRefPointAction> pAction(new MetaRefPointAction);

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aRefPoint;
    aSerializer.readPoint(aRefPoint);
    bool bSet(false);
    mrStream.ReadCharAsBool(bSet);

    pAction->SetRefPoint(aRefPoint);
    pAction->SetSetting(bSet);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::CommentHandler()
{
    rtl::Reference<MetaCommentAction> pAction(new MetaCommentAction);

    VersionCompatRead aCompat(mrStream);
    OString aComment;
    aComment = read_uInt16_lenPrefixed_uInt8s_ToOString(mrStream);
    sal_Int32 nValue(0);
    sal_uInt32 nDataSize(0);
    mrStream.ReadInt32(nValue).ReadUInt32(nDataSize);

    if (nDataSize > mrStream.remainingSize())
    {
        SAL_WARN("vcl.gdi", "Parsing error: " << mrStream.remainingSize() << " available data, but "
                                              << nDataSize << " claimed, truncating");
        nDataSize = mrStream.remainingSize();
    }

    SAL_INFO("vcl.gdi", "MetaCommentAction::Read " << aComment);

    std::unique_ptr<sal_uInt8[]> pData;
    pData.reset();

    if (nDataSize)
    {
        pData.reset(new sal_uInt8[nDataSize]);
        mrStream.ReadBytes(pData.get(), nDataSize);
    }

    pAction->SetComment(aComment);
    pAction->SetDataSize(nDataSize);
    pAction->SetValue(nValue);
    pAction->SetData(pData.get(), nDataSize);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::LayoutModeHandler()
{
    rtl::Reference<MetaLayoutModeAction> pAction(new MetaLayoutModeAction);

    VersionCompatRead aCompat(mrStream);
    sal_uInt32 tmp(0);
    mrStream.ReadUInt32(tmp);

    pAction->SetLayoutMode(static_cast<vcl::text::ComplexTextLayoutFlags>(tmp));

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextLanguageHandler()
{
    rtl::Reference<MetaTextLanguageAction> pAction(new MetaTextLanguageAction);

    VersionCompatRead aCompat(mrStream);
    sal_uInt16 nTmp = 0;
    mrStream.ReadUInt16(nTmp);

    pAction->SetTextLanguage(static_cast<LanguageType>(nTmp));

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::DefaultHandler()
{
    return rtl::Reference<MetaAction>(new MetaAction);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
