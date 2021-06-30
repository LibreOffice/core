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

#include <vcl/filter/SvmReader.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/TypeSerializer.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>

#include <svmconverter.hxx>

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

    sal_uLong nStmPos = mrStream.Tell();
    SvStreamEndian nOldFormat = mrStream.GetEndian();

    mrStream.SetEndian(SvStreamEndian::LITTLE);

    try
    {
        char aId[7];
        aId[0] = 0;
        aId[6] = 0;
        mrStream.ReadBytes(aId, 6);
        if (!strcmp(aId, "VCLMTF"))
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
            pAction = new MetaAction;
            break;
        case MetaActionType::PIXEL:
            pAction = new MetaPixelAction;
            break;
        case MetaActionType::POINT:
            pAction = new MetaPointAction;
            break;
        case MetaActionType::LINE:
            pAction = new MetaLineAction;
            break;
        case MetaActionType::RECT:
            pAction = new MetaRectAction;
            break;
        case MetaActionType::ROUNDRECT:
            pAction = new MetaRoundRectAction;
            break;
        case MetaActionType::ELLIPSE:
            pAction = new MetaEllipseAction;
            break;
        case MetaActionType::ARC:
            pAction = new MetaArcAction;
            break;
        case MetaActionType::PIE:
            pAction = new MetaPieAction;
            break;
        case MetaActionType::CHORD:
            pAction = new MetaChordAction;
            break;
        case MetaActionType::POLYLINE:
            pAction = new MetaPolyLineAction;
            break;
        case MetaActionType::POLYGON:
            pAction = new MetaPolygonAction;
            break;
        case MetaActionType::POLYPOLYGON:
            pAction = new MetaPolyPolygonAction;
            break;
        case MetaActionType::TEXT:
            pAction = new MetaTextAction;
            break;
        case MetaActionType::TEXTARRAY:
            pAction = new MetaTextArrayAction;
            break;
        case MetaActionType::STRETCHTEXT:
            pAction = new MetaStretchTextAction;
            break;
        case MetaActionType::TEXTRECT:
            pAction = new MetaTextRectAction;
            break;
        case MetaActionType::TEXTLINE:
            pAction = new MetaTextLineAction;
            break;
        case MetaActionType::BMP:
            pAction = new MetaBmpAction;
            break;
        case MetaActionType::BMPSCALE:
            pAction = new MetaBmpScaleAction;
            break;
        case MetaActionType::BMPSCALEPART:
            pAction = new MetaBmpScalePartAction;
            break;
        case MetaActionType::BMPEX:
            pAction = new MetaBmpExAction;
            break;
        case MetaActionType::BMPEXSCALE:
            pAction = new MetaBmpExScaleAction;
            break;
        case MetaActionType::BMPEXSCALEPART:
            pAction = new MetaBmpExScalePartAction;
            break;
        case MetaActionType::MASK:
            pAction = new MetaMaskAction;
            break;
        case MetaActionType::MASKSCALE:
            pAction = new MetaMaskScaleAction;
            break;
        case MetaActionType::MASKSCALEPART:
            pAction = new MetaMaskScalePartAction;
            break;
        case MetaActionType::GRADIENT:
            pAction = new MetaGradientAction;
            break;
        case MetaActionType::GRADIENTEX:
            pAction = new MetaGradientExAction;
            break;
        case MetaActionType::HATCH:
            pAction = new MetaHatchAction;
            break;
        case MetaActionType::WALLPAPER:
            pAction = new MetaWallpaperAction;
            break;
        case MetaActionType::CLIPREGION:
            pAction = new MetaClipRegionAction;
            break;
        case MetaActionType::ISECTRECTCLIPREGION:
            pAction = new MetaISectRectClipRegionAction;
            break;
        case MetaActionType::ISECTREGIONCLIPREGION:
            pAction = new MetaISectRegionClipRegionAction;
            break;
        case MetaActionType::MOVECLIPREGION:
            pAction = new MetaMoveClipRegionAction;
            break;
        case MetaActionType::LINECOLOR:
            pAction = new MetaLineColorAction;
            break;
        case MetaActionType::FILLCOLOR:
            pAction = new MetaFillColorAction;
            break;
        case MetaActionType::TEXTCOLOR:
            pAction = new MetaTextColorAction;
            break;
        case MetaActionType::TEXTFILLCOLOR:
            pAction = new MetaTextFillColorAction;
            break;
        case MetaActionType::TEXTLINECOLOR:
            pAction = new MetaTextLineColorAction;
            break;
        case MetaActionType::OVERLINECOLOR:
            pAction = new MetaOverlineColorAction;
            break;
        case MetaActionType::TEXTALIGN:
            pAction = new MetaTextAlignAction;
            break;
        case MetaActionType::MAPMODE:
            pAction = new MetaMapModeAction;
            break;
        case MetaActionType::FONT:
            pAction = new MetaFontAction;
            break;
        case MetaActionType::PUSH:
            pAction = new MetaPushAction;
            break;
        case MetaActionType::POP:
            pAction = new MetaPopAction;
            break;
        case MetaActionType::RASTEROP:
            pAction = new MetaRasterOpAction;
            break;
        case MetaActionType::Transparent:
            pAction = new MetaTransparentAction;
            break;
        case MetaActionType::FLOATTRANSPARENT:
            pAction = new MetaFloatTransparentAction;
            break;
        case MetaActionType::EPS:
            pAction = new MetaEPSAction;
            break;
        case MetaActionType::REFPOINT:
            pAction = new MetaRefPointAction;
            break;
        case MetaActionType::COMMENT:
            pAction = new MetaCommentAction;
            break;
        case MetaActionType::LAYOUTMODE:
            pAction = new MetaLayoutModeAction;
            break;
        case MetaActionType::TEXTLANGUAGE:
            pAction = new MetaTextLanguageAction;
            break;

        default:
        {
            VersionCompatRead aCompat(mrStream);
        }
        break;
    }

    if (pAction)
        pAction->Read(mrStream, pData);

    return pAction;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
