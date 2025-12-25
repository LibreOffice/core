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
#include <comphelper/configuration.hxx>

#include <vcl/filter/SvmReader.hxx>
#include <vcl/rendercontext/DrawTextFlags.hxx>
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
        case MetaActionType::PIXEL:
            return PixelHandler();
        case MetaActionType::POINT:
            return PointHandler();
        case MetaActionType::LINE:
            return LineHandler();
        case MetaActionType::RECT:
            return RectHandler();
        case MetaActionType::ROUNDRECT:
            return RoundRectHandler();
        case MetaActionType::ELLIPSE:
            return EllipseHandler();
        case MetaActionType::ARC:
            return ArcHandler();
        case MetaActionType::PIE:
            return PieHandler();
        case MetaActionType::CHORD:
            return ChordHandler();
        case MetaActionType::POLYLINE:
            return PolyLineHandler();
        case MetaActionType::POLYGON:
            return PolygonHandler();
        case MetaActionType::POLYPOLYGON:
            return PolyPolygonHandler();
        case MetaActionType::TEXT:
            return TextHandler(pData);
        case MetaActionType::TEXTARRAY:
            return TextArrayHandler(pData);
        case MetaActionType::STRETCHTEXT:
            return StretchTextHandler(pData);
        case MetaActionType::TEXTRECT:
            return TextRectHandler(pData);
        case MetaActionType::TEXTLINE:
            return TextLineHandler();
        case MetaActionType::BMP:
            return BmpHandler();
        case MetaActionType::BMPSCALE:
            return BmpScaleHandler();
        case MetaActionType::BMPSCALEPART:
            return BmpScalePartHandler();
        case MetaActionType::BMPEX:
            return BmpExHandler();
        case MetaActionType::BMPEXSCALE:
            return BmpExScaleHandler();
        case MetaActionType::BMPEXSCALEPART:
            return BmpExScalePartHandler();
        case MetaActionType::MASK:
            return MaskHandler();
        case MetaActionType::MASKSCALE:
            return MaskScaleHandler();
        case MetaActionType::MASKSCALEPART:
            return MaskScalePartHandler();
        case MetaActionType::GRADIENT:
            return GradientHandler();
        case MetaActionType::GRADIENTEX:
            return GradientExHandler();
        case MetaActionType::HATCH:
            return HatchHandler();
        case MetaActionType::WALLPAPER:
            return WallpaperHandler();
        case MetaActionType::CLIPREGION:
            return ClipRegionHandler();
        case MetaActionType::ISECTRECTCLIPREGION:
            return ISectRectClipRegionHandler();
        case MetaActionType::ISECTREGIONCLIPREGION:
            return ISectRegionClipRegionHandler();
        case MetaActionType::MOVECLIPREGION:
            return MoveClipRegionHandler();
        case MetaActionType::LINECOLOR:
            return LineColorHandler();
        case MetaActionType::FILLCOLOR:
            return FillColorHandler();
        case MetaActionType::TEXTCOLOR:
            return TextColorHandler();
        case MetaActionType::TEXTFILLCOLOR:
            return TextFillColorHandler();
        case MetaActionType::TEXTLINECOLOR:
            return TextLineColorHandler();
        case MetaActionType::OVERLINECOLOR:
            return OverlineColorHandler();
        case MetaActionType::TEXTALIGN:
            return TextAlignHandler();
        case MetaActionType::MAPMODE:
            return MapModeHandler();
        case MetaActionType::FONT:
            return FontHandler(pData);
        case MetaActionType::PUSH:
            return PushHandler();
        case MetaActionType::POP:
            return PopHandler();
        case MetaActionType::RASTEROP:
            return RasterOpHandler();
        case MetaActionType::Transparent:
            return TransparentHandler();
        case MetaActionType::FLOATTRANSPARENT:
            return FloatTransparentHandler(pData);
        case MetaActionType::EPS:
            return EPSHandler();
        case MetaActionType::REFPOINT:
            return RefPointHandler();
        case MetaActionType::COMMENT:
            return CommentHandler();
        case MetaActionType::LAYOUTMODE:
            return LayoutModeHandler();
        case MetaActionType::TEXTLANGUAGE:
            return TextLanguageHandler();

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
    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);
    bool aBool(false);
    mrStream.ReadCharAsBool(aBool);

    rtl::Reference<MetaLineColorAction> pAction(new MetaLineColorAction(aColor, aBool));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::FillColorHandler()
{
    VersionCompatRead aCompat(mrStream);

    Color aColor;
    ReadColor(aColor);
    bool aBool(false);
    mrStream.ReadCharAsBool(aBool);

    rtl::Reference<MetaFillColorAction> pAction(new MetaFillColorAction(aColor, aBool));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::RectHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);

    rtl::Reference<MetaRectAction> pAction(new MetaRectAction(aRectangle));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PointHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);

    rtl::Reference<MetaPointAction> pAction(new MetaPointAction(aPoint));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PixelHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);
    Color aColor;
    ReadColor(aColor);

    rtl::Reference<MetaPixelAction> pAction(new MetaPixelAction(aPoint, aColor));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::LineHandler()
{
    VersionCompatRead aCompat(mrStream);

    // Version 1
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    Point aEndPoint;
    aSerializer.readPoint(aPoint);
    aSerializer.readPoint(aEndPoint);

    // Version 2
    if (aCompat.GetVersion() >= 2)
    {
        LineInfo aLineInfo;
        ReadLineInfo(mrStream, aLineInfo);
        return new MetaLineAction(aPoint, aEndPoint, aLineInfo);
    }
    else
        return new MetaLineAction(aPoint, aEndPoint);
}

rtl::Reference<MetaAction> SvmReader::RoundRectHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    sal_uInt32 HorzRound(0);
    sal_uInt32 VertRound(0);
    mrStream.ReadUInt32(HorzRound).ReadUInt32(VertRound);

    rtl::Reference<MetaRoundRectAction> pAction(
        new MetaRoundRectAction(aRectangle, HorzRound, VertRound));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::EllipseHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);

    rtl::Reference<MetaEllipseAction> pAction(new MetaEllipseAction(aRectangle));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::ArcHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Point aEndPoint;
    aSerializer.readPoint(aEndPoint);

    rtl::Reference<MetaArcAction> pAction(new MetaArcAction(aRectangle, aPoint, aEndPoint));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PieHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Point aEndPoint;
    aSerializer.readPoint(aEndPoint);

    rtl::Reference<MetaPieAction> pAction(new MetaPieAction(aRectangle, aPoint, aEndPoint));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::ChordHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRectangle;
    aSerializer.readRectangle(aRectangle);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Point aEndPoint;
    aSerializer.readPoint(aEndPoint);

    rtl::Reference<MetaChordAction> pAction(new MetaChordAction(aRectangle, aPoint, aEndPoint));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::PolyLineHandler()
{
    VersionCompatRead aCompat(mrStream);

    // Version 1
    tools::Polygon aPolygon;
    ReadPolygon(mrStream, aPolygon);

    // Version 2
    LineInfo aLineInfo;
    if (aCompat.GetVersion() >= 2)
    {
        ReadLineInfo(mrStream, aLineInfo);
    }
    if (aCompat.GetVersion() >= 3)
    {
        sal_uInt8 bHasPolyFlags(0);
        mrStream.ReadUChar(bHasPolyFlags);
        if (bHasPolyFlags)
            aPolygon.Read(mrStream);
    }

    if (aCompat.GetVersion() < 2)
        return new MetaPolyLineAction(aPolygon);
    else
        return new MetaPolyLineAction(aPolygon, aLineInfo);
}

rtl::Reference<MetaAction> SvmReader::PolygonHandler()
{
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

    return new MetaPolygonAction(aPolygon);
}

rtl::Reference<MetaAction> SvmReader::PolyPolygonHandler()
{
    VersionCompatRead aCompat(mrStream);
    tools::PolyPolygon aPolyPolygon;
    ReadPolyPolygon(mrStream, aPolyPolygon); // Version 1

    if (aCompat.GetVersion() < 2) // Version 2
    {
        return new MetaPolyPolygonAction(aPolyPolygon);
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

    rtl::Reference<MetaPolyPolygonAction> pAction(new MetaPolyPolygonAction(aPolyPolygon));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextHandler(const ImplMetaReadData* pData)
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);
    OUString aStr = mrStream.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt16 nTmpIndex(0);
    mrStream.ReadUInt16(nTmpIndex);
    sal_uInt16 nTmpLen(0);
    mrStream.ReadUInt16(nTmpLen);

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

    rtl::Reference<MetaTextAction> pAction(new MetaTextAction(aPoint, aStr, nTmpIndex, nTmpLen));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextArrayHandler(const ImplMetaReadData* pData)
{
    KernArray aArray;

    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPoint;
    aSerializer.readPoint(aPoint);

    OUString aStr = mrStream.ReadUniOrByteString(pData->meActualCharSet);

    sal_uInt16 nStrIndex(0);
    mrStream.ReadUInt16(nStrIndex);

    sal_uInt16 nStrLen(0);
    mrStream.ReadUInt16(nStrLen);

    sal_Int32 nAryLen(0);
    mrStream.ReadInt32(nAryLen);

    if (nStrLen > aStr.getLength() - nStrIndex)
    {
        SAL_WARN("vcl.gdi", "inconsistent offset and len");
        return new MetaTextArrayAction(aPoint, aStr, KernArray(), std::vector<sal_Bool>(), 0,
                                       aStr.getLength());
    }

    if (nAryLen)
    {
        // #i9762#, #106172# Ensure that DX array is at least mnLen entries long
        if (nStrLen >= nAryLen)
        {
            try
            {
                sal_Int32 i;
                sal_Int32 val(0);
                for (i = 0; i < nAryLen; i++)
                {
                    mrStream.ReadInt32(val);
                    aArray.push_back(val);
                }
                // #106172# setup remainder
                for (; i < nStrLen; i++)
                    aArray.push_back(0);
            }
            catch (std::bad_alloc&)
            {
            }
        }
        else
        {
            return new MetaTextArrayAction(aPoint, aStr, KernArray(), std::vector<sal_Bool>(),
                                           nStrIndex, nStrLen);
        }
    }

    if (aCompat.GetVersion() >= 2) // Version 2
    {
        aStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(mrStream);

        if (nStrLen > aStr.getLength() - nStrIndex)
        {
            SAL_WARN("vcl.gdi", "inconsistent offset and len");
            nStrIndex = 0;
            nStrLen = aStr.getLength();
            aArray.clear();
        }
    }

    std::vector<sal_Bool> aKashidaArray;
    if (aCompat.GetVersion() >= 3) // Version 3
    {
        sal_uInt32 nKashidaAryLen(0);
        mrStream.ReadUInt32(nKashidaAryLen);
        sal_uInt32 nTmpLen = std::min(nKashidaAryLen, static_cast<sal_uInt32>(aArray.size()));
        if (nTmpLen)
        {
            // aKashidaArray, if not empty, must be the same size as aArray
            aKashidaArray.resize(aArray.size(), 0);

            for (sal_uInt32 i = 0; i < nTmpLen; i++)
            {
                mrStream.ReadUChar(aKashidaArray[i]);
            }
        }
    }

    sal_Int32 nLayoutContextIndex = -1;
    sal_Int32 nLayoutContextLen = -1;
    if (aCompat.GetVersion() >= 4) // Version 4
    {
        bool bTmpHasContext = false;
        mrStream.ReadCharAsBool(bTmpHasContext);

        if (bTmpHasContext)
        {
            sal_uInt16 nTmp = 0;
            mrStream.ReadUInt16(nTmp);
            nLayoutContextIndex = nTmp;
            mrStream.ReadUInt16(nTmp);
            nLayoutContextLen = nTmp;

            sal_uInt16 nTmpEnd = nStrIndex + nStrLen;
            sal_uInt16 nTmpContextEnd = nLayoutContextIndex + nLayoutContextLen;
            if ((nTmpContextEnd <= aStr.getLength()) && (nLayoutContextIndex <= nStrIndex)
                && (nTmpContextEnd >= nTmpEnd))
            {
            }
            else
            {
                nLayoutContextIndex = -1;
                nLayoutContextLen = -1;
                SAL_WARN("vcl.gdi", "inconsistent layout context offset and len");
            }
        }
    }

    return new MetaTextArrayAction(aPoint, aStr, std::move(aArray), std::move(aKashidaArray),
                                   nStrIndex, nStrLen, nLayoutContextIndex, nLayoutContextLen);
}

rtl::Reference<MetaAction> SvmReader::StretchTextHandler(const ImplMetaReadData* pData)
{
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

    rtl::Reference<MetaStretchTextAction> pAction(
        new MetaStretchTextAction(aPoint, nTmpWidth, aStr, nTmpIndex, nTmpLen));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextRectHandler(const ImplMetaReadData* pData)
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRect;
    aSerializer.readRectangle(aRect);
    OUString aStr = mrStream.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt16 nTmp(0);
    mrStream.ReadUInt16(nTmp);

    DrawTextFlags nFlags(static_cast<DrawTextFlags>(nTmp));
    const static bool bFuzzing = comphelper::IsFuzzing();
    if (bFuzzing)
        nFlags = nFlags & ~DrawTextFlags::MultiLine;

    if (aCompat.GetVersion() >= 2) // Version 2
        aStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(mrStream);

    rtl::Reference<MetaTextRectAction> pAction(new MetaTextRectAction(aRect, aStr, nFlags));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::TextLineHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aPos;
    aSerializer.readPoint(aPos);
    sal_Int32 nTempWidth(0);
    mrStream.ReadInt32(nTempWidth);

    if (nTempWidth < 0)
    {
        SAL_WARN("vcl.gdi", "negative width");
        nTempWidth = 0;
    }

    sal_uInt32 nTempStrikeout(0);
    mrStream.ReadUInt32(nTempStrikeout);
    sal_uInt32 nTempUnderline(0);
    mrStream.ReadUInt32(nTempUnderline);

    sal_uInt32 nTempOverline(0);
    if (aCompat.GetVersion() >= 2)
    {
        mrStream.ReadUInt32(nTempOverline);
    }

    return new MetaTextLineAction(aPos, nTempWidth,
                                  static_cast<FontStrikeout>(nTempStrikeout & SAL_MAX_ENUM),
                                  static_cast<FontLineStyle>(nTempUnderline & SAL_MAX_ENUM),
                                  static_cast<FontLineStyle>(nTempOverline & SAL_MAX_ENUM));
}

rtl::Reference<MetaAction> SvmReader::BmpHandler()
{
    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);

    rtl::Reference<MetaBmpAction> pAction(new MetaBmpAction(aPoint, aBmp));
    return pAction;
}

rtl::Reference<MetaAction> SvmReader::BmpScaleHandler()
{
    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);

    Size aSz;
    aSerializer.readSize(aSz);

    rtl::Reference<MetaBmpScaleAction> pAction(new MetaBmpScaleAction(aPoint, aSz, aBmp));

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::BmpScalePartHandler()
{
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

    return new MetaBmpScalePartAction(aDestPoint, aDestSize, aSrcPoint, aSrcSize, aBmp);
}

rtl::Reference<MetaAction> SvmReader::BmpExHandler()
{
    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIBBitmapEx(aBmp, mrStream);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);

    return new MetaBmpExAction(aPoint, aBmp);
}

rtl::Reference<MetaAction> SvmReader::BmpExScaleHandler()
{
    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIBBitmapEx(aBmp, mrStream);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);

    Size aSize;
    aSerializer.readSize(aSize);

    return new MetaBmpExScaleAction(aPoint, aSize, aBmp);
}

rtl::Reference<MetaAction> SvmReader::BmpExScalePartHandler()
{
    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIBBitmapEx(aBmp, mrStream);
    TypeSerializer aSerializer(mrStream);
    Point aDstPoint;
    aSerializer.readPoint(aDstPoint);
    Size aDstSize;
    aSerializer.readSize(aDstSize);
    Point aSrcPoint;
    aSerializer.readPoint(aSrcPoint);
    Size aSrcSize;
    aSerializer.readSize(aSrcSize);

    return new MetaBmpExScalePartAction(aDstPoint, aDstSize, aSrcPoint, aSrcSize, aBmp);
}

rtl::Reference<MetaAction> SvmReader::MaskHandler()
{
    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);

    return new MetaMaskAction(aPoint, aBmp, Color());
}

rtl::Reference<MetaAction> SvmReader::MaskScaleHandler()
{
    VersionCompatRead aCompat(mrStream);
    Bitmap aBmp;
    ReadDIB(aBmp, mrStream, true);
    TypeSerializer aSerializer(mrStream);
    Point aPoint;
    aSerializer.readPoint(aPoint);
    Size aSize;
    aSerializer.readSize(aSize);

    return new MetaMaskScaleAction(aPoint, aSize, aBmp, Color());
}

rtl::Reference<MetaAction> SvmReader::MaskScalePartHandler()
{
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

    return new MetaMaskScalePartAction(aDstPt, aDstSz, aSrcPt, aSrcSz, aBmp, aColor);
}

rtl::Reference<MetaAction> SvmReader::GradientHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    tools::Rectangle aRect;
    aSerializer.readRectangle(aRect);
    Gradient aGradient;
    aSerializer.readGradient(aGradient);

    return new MetaGradientAction(aRect, aGradient);
}

rtl::Reference<MetaAction> SvmReader::GradientExHandler()
{
    VersionCompatRead aCompat(mrStream);
    tools::PolyPolygon aPolyPoly;
    ReadPolyPolygon(mrStream, aPolyPoly);
    TypeSerializer aSerializer(mrStream);
    Gradient aGradient;
    aSerializer.readGradient(aGradient);

    return new MetaGradientExAction(aPolyPoly, aGradient);
}

rtl::Reference<MetaAction> SvmReader::HatchHandler()
{
    VersionCompatRead aCompat(mrStream);
    tools::PolyPolygon aPolyPoly;
    ReadPolyPolygon(mrStream, aPolyPoly);
    Hatch aHatch;
    ReadHatch(mrStream, aHatch);

    return new MetaHatchAction(aPolyPoly, aHatch);
}

rtl::Reference<MetaAction> SvmReader::WallpaperHandler()
{
    VersionCompatRead aCompat(mrStream);
    Wallpaper aWallpaper;
    ReadWallpaper(mrStream, aWallpaper);

    return new MetaWallpaperAction(tools::Rectangle(), aWallpaper);
}

rtl::Reference<MetaAction> SvmReader::ClipRegionHandler()
{
    VersionCompatRead aCompat(mrStream);
    vcl::Region aRegion;
    ReadRegion(mrStream, aRegion);
    bool bClip(false);
    mrStream.ReadCharAsBool(bClip);

    return new MetaClipRegionAction(std::move(aRegion), bClip);
}

rtl::Reference<MetaAction> SvmReader::ISectRectClipRegionHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);
    tools::Rectangle aRect;
    aSerializer.readRectangle(aRect);

    return new MetaISectRectClipRegionAction(aRect);
}

rtl::Reference<MetaAction> SvmReader::ISectRegionClipRegionHandler()
{
    VersionCompatRead aCompat(mrStream);
    vcl::Region aRegion;
    ReadRegion(mrStream, aRegion);

    return new MetaISectRegionClipRegionAction(std::move(aRegion));
}

rtl::Reference<MetaAction> SvmReader::MoveClipRegionHandler()
{
    VersionCompatRead aCompat(mrStream);
    sal_Int32 nTmpHM(0), nTmpVM(0);
    mrStream.ReadInt32(nTmpHM).ReadInt32(nTmpVM);

    return new MetaMoveClipRegionAction(nTmpHM, nTmpVM);
}

rtl::Reference<MetaAction> SvmReader::TextColorHandler()
{
    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);

    return new MetaTextColorAction(aColor);
}

rtl::Reference<MetaAction> SvmReader::TextFillColorHandler()
{
    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);
    bool bSet(false);
    mrStream.ReadCharAsBool(bSet);

    return new MetaTextFillColorAction(aColor, bSet);
}

rtl::Reference<MetaAction> SvmReader::TextLineColorHandler()
{
    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);
    bool bSet(false);
    mrStream.ReadCharAsBool(bSet);

    return new MetaTextLineColorAction(aColor, bSet);
}

rtl::Reference<MetaAction> SvmReader::OverlineColorHandler()
{
    VersionCompatRead aCompat(mrStream);
    Color aColor;
    ReadColor(aColor);
    bool bSet(false);
    mrStream.ReadCharAsBool(bSet);

    return new MetaOverlineColorAction(aColor, bSet);
}

rtl::Reference<MetaAction> SvmReader::TextAlignHandler()
{
    VersionCompatRead aCompat(mrStream);
    sal_uInt16 nTmp16(0);
    mrStream.ReadUInt16(nTmp16);

    return new MetaTextAlignAction(static_cast<TextAlign>(nTmp16));
}

rtl::Reference<MetaAction> SvmReader::MapModeHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    MapMode aMapMode;
    const bool bSuccess = aSerializer.readMapMode(aMapMode);
    if (!bSuccess)
        return nullptr;

    return new MetaMapModeAction(aMapMode);
}

rtl::Reference<MetaAction> SvmReader::FontHandler(ImplMetaReadData* pData)
{
    VersionCompatRead aCompat(mrStream);
    vcl::Font aFont;
    ReadFont(mrStream, aFont);
    pData->meActualCharSet = aFont.GetCharSet();
    if (pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW)
        pData->meActualCharSet = osl_getThreadTextEncoding();

    return new MetaFontAction(aFont);
}

rtl::Reference<MetaAction> SvmReader::PushHandler()
{
    VersionCompatRead aCompat(mrStream);
    sal_uInt16 nTmp(0);
    mrStream.ReadUInt16(nTmp);

    return new MetaPushAction(static_cast<vcl::PushFlags>(nTmp));
}

rtl::Reference<MetaAction> SvmReader::PopHandler()
{
    rtl::Reference<MetaPopAction> pAction(new MetaPopAction);

    VersionCompatRead aCompat(mrStream);

    return pAction;
}

rtl::Reference<MetaAction> SvmReader::RasterOpHandler()
{
    sal_uInt16 nTmp16(0);

    VersionCompatRead aCompat(mrStream);
    mrStream.ReadUInt16(nTmp16);

    return new MetaRasterOpAction(static_cast<RasterOp>(nTmp16));
}

rtl::Reference<MetaAction> SvmReader::TransparentHandler()
{
    VersionCompatRead aCompat(mrStream);
    tools::PolyPolygon aPolyPoly;
    ReadPolyPolygon(mrStream, aPolyPoly);
    sal_uInt16 nTransPercent(0);
    mrStream.ReadUInt16(nTransPercent);

    return new MetaTransparentAction(aPolyPoly, nTransPercent);
}

rtl::Reference<MetaAction> SvmReader::FloatTransparentHandler(ImplMetaReadData* pData)
{
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

    // tdf#155479 add support for MCGR and SVG export
    basegfx::BColorStops aColorStops;
    if (aCompat.GetVersion() > 1)
    {
        sal_uInt16 nTmp(0);
        double fOff, fR, fG, fB;
        mrStream.ReadUInt16(nTmp);

        const size_t nMaxPossibleEntries = mrStream.remainingSize() / 4 * sizeof(double);
        if (nTmp > nMaxPossibleEntries)
        {
            SAL_WARN("vcl.gdi", "gradient record claims to have: " << nTmp << " entries, but only "
                                                                   << nMaxPossibleEntries
                                                                   << " possible, clamping");
            nTmp = nMaxPossibleEntries;
        }

        for (sal_uInt16 a(0); a < nTmp; a++)
        {
            mrStream.ReadDouble(fOff);
            mrStream.ReadDouble(fR);
            mrStream.ReadDouble(fG);
            mrStream.ReadDouble(fB);

            aColorStops.addStop(fOff, basegfx::BColor(fR, fG, fB));
        }
    }

    return new MetaFloatTransparentAction(aMtf, aPoint, aSize, aGradient, aColorStops);
}

rtl::Reference<MetaAction> SvmReader::EPSHandler()
{
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

    return new MetaEPSAction(aPoint, aSize, std::move(aGfxLink), aSubst);
}

rtl::Reference<MetaAction> SvmReader::RefPointHandler()
{
    VersionCompatRead aCompat(mrStream);
    TypeSerializer aSerializer(mrStream);

    Point aRefPoint;
    aSerializer.readPoint(aRefPoint);
    bool bSet(false);
    mrStream.ReadCharAsBool(bSet);

    return new MetaRefPointAction(aRefPoint, bSet);
}

rtl::Reference<MetaAction> SvmReader::CommentHandler()
{
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

    return new MetaCommentAction(aComment, nValue, pData.get(), nDataSize);
}

rtl::Reference<MetaAction> SvmReader::LayoutModeHandler()
{
    VersionCompatRead aCompat(mrStream);
    sal_uInt32 tmp(0);
    mrStream.ReadUInt32(tmp);

    return new MetaLayoutModeAction(static_cast<vcl::text::ComplexTextLayoutFlags>(tmp));
}

rtl::Reference<MetaAction> SvmReader::TextLanguageHandler()
{
    VersionCompatRead aCompat(mrStream);
    sal_uInt16 nTmp = 0;
    mrStream.ReadUInt16(nTmp);

    return new MetaTextLanguageAction(static_cast<LanguageType>(nTmp));
}

rtl::Reference<MetaAction> SvmReader::DefaultHandler()
{
    return rtl::Reference<MetaAction>(new MetaAction);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
