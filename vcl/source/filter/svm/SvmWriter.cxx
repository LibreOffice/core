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
#include <tools/vcompat.hxx>

#include <vcl/filter/SvmWriter.hxx>
#include <vcl/TypeSerializer.hxx>
#include <vcl/dibtools.hxx>

SvmWriter::SvmWriter(SvStream& rIStm)
    : mrStream(rIStm)
{
}

void SvmWriter::WriteColor(::Color aColor)
{
    mrStream.WriteUInt32(static_cast<sal_uInt32>(aColor));
}

SvStream& SvmWriter::Write(const GDIMetaFile& rMetaFile)
{
    const SvStreamCompressFlags nStmCompressMode = mrStream.GetCompressMode();
    SvStreamEndian nOldFormat = mrStream.GetEndian();

    mrStream.SetEndian(SvStreamEndian::LITTLE);
    mrStream.WriteBytes("VCLMTF", 6);

    {
        VersionCompatWrite aCompat(mrStream, 1);

        mrStream.WriteUInt32(static_cast<sal_uInt32>(nStmCompressMode));
        TypeSerializer aSerializer(mrStream);
        aSerializer.writeMapMode(rMetaFile.GetPrefMapMode());
        aSerializer.writeSize(rMetaFile.GetPrefSize());
        mrStream.WriteUInt32(rMetaFile.GetActionSize());
    } // VersionCompatWrite dtor writes stuff into the header

    ImplMetaWriteData aWriteData;

    aWriteData.meActualCharSet = mrStream.GetStreamCharSet();

    MetaAction* pAct = const_cast<GDIMetaFile&>(rMetaFile).FirstAction();
    while (pAct)
    {
        MetaActionHandler(pAct, &aWriteData);
        pAct = const_cast<GDIMetaFile&>(rMetaFile).NextAction();
    }

    mrStream.SetEndian(nOldFormat);

    return mrStream;
}

BitmapChecksum SvmWriter::GetChecksum(const GDIMetaFile& rMetaFile)
{
    SvMemoryStream aMemStm(65535, 65535);
    ImplMetaWriteData aWriteData;
    SVBT16 aBT16;
    SVBT32 aBT32;
    BitmapChecksumOctetArray aBCOA;
    BitmapChecksum nCrc = 0;

    aWriteData.meActualCharSet = aMemStm.GetStreamCharSet();

    for (size_t i = 0, nObjCount = rMetaFile.GetActionSize(); i < nObjCount; i++)
    {
        MetaAction* pAction = rMetaFile.GetAction(i);

        switch (pAction->GetType())
        {
            case MetaActionType::BMP:
            {
                MetaBmpAction* pAct = static_cast<MetaBmpAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmap().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                Int32ToSVBT32(pAct->GetPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmap().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                Int32ToSVBT32(pAct->GetPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmap().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                Int32ToSVBT32(pAct->GetDestPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::BMPEX:
            {
                MetaBmpExAction* pAct = static_cast<MetaBmpExAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmapEx().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                Int32ToSVBT32(pAct->GetPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                MetaBmpExScaleAction* pAct = static_cast<MetaBmpExScaleAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmapEx().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                Int32ToSVBT32(pAct->GetPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::BMPEXSCALEPART:
            {
                MetaBmpExScalePartAction* pAct = static_cast<MetaBmpExScalePartAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmapEx().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                Int32ToSVBT32(pAct->GetDestPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::MASK:
            {
                MetaMaskAction* pAct = static_cast<MetaMaskAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmap().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                UInt32ToSVBT32(sal_uInt32(pAct->GetColor()), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::MASKSCALE:
            {
                MetaMaskScaleAction* pAct = static_cast<MetaMaskScaleAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmap().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                UInt32ToSVBT32(sal_uInt32(pAct->GetColor()), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::MASKSCALEPART:
            {
                MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pAction);

                ShortToSVBT16(static_cast<sal_uInt16>(pAct->GetType()), aBT16);
                nCrc = vcl_get_checksum(nCrc, aBT16, 2);

                BCToBCOA(pAct->GetBitmap().GetChecksum(), aBCOA);
                nCrc = vcl_get_checksum(nCrc, aBCOA, BITMAP_CHECKSUM_SIZE);

                UInt32ToSVBT32(sal_uInt32(pAct->GetColor()), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetDestSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcPoint().X(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcPoint().Y(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcSize().Width(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);

                Int32ToSVBT32(pAct->GetSrcSize().Height(), aBT32);
                nCrc = vcl_get_checksum(nCrc, aBT32, 4);
            }
            break;

            case MetaActionType::EPS:
            {
                MetaEPSAction* pAct = static_cast<MetaEPSAction*>(pAction);
                nCrc = vcl_get_checksum(nCrc, pAct->GetLink().GetData(),
                                        pAct->GetLink().GetDataSize());
            }
            break;

            case MetaActionType::CLIPREGION:
            {
                MetaClipRegionAction& rAct = static_cast<MetaClipRegionAction&>(*pAction);
                const vcl::Region& rRegion = rAct.GetRegion();

                if (rRegion.HasPolyPolygonOrB2DPolyPolygon())
                {
                    // It has shown that this is a possible bottleneck for checksum calculation.
                    // In worst case a very expensive RegionHandle representation gets created.
                    // In this case it's cheaper to use the PolyPolygon
                    const basegfx::B2DPolyPolygon aPolyPolygon(rRegion.GetAsB2DPolyPolygon());
                    SVBT64 aSVBT64;

                    for (auto const& rPolygon : aPolyPolygon)
                    {
                        const sal_uInt32 nPointCount(rPolygon.count());
                        const bool bControl(rPolygon.areControlPointsUsed());

                        for (sal_uInt32 b(0); b < nPointCount; b++)
                        {
                            const basegfx::B2DPoint aPoint(rPolygon.getB2DPoint(b));

                            DoubleToSVBT64(aPoint.getX(), aSVBT64);
                            nCrc = vcl_get_checksum(nCrc, aSVBT64, 8);
                            DoubleToSVBT64(aPoint.getY(), aSVBT64);
                            nCrc = vcl_get_checksum(nCrc, aSVBT64, 8);

                            if (bControl)
                            {
                                if (rPolygon.isPrevControlPointUsed(b))
                                {
                                    const basegfx::B2DPoint aCtrl(rPolygon.getPrevControlPoint(b));

                                    DoubleToSVBT64(aCtrl.getX(), aSVBT64);
                                    nCrc = vcl_get_checksum(nCrc, aSVBT64, 8);
                                    DoubleToSVBT64(aCtrl.getY(), aSVBT64);
                                    nCrc = vcl_get_checksum(nCrc, aSVBT64, 8);
                                }

                                if (rPolygon.isNextControlPointUsed(b))
                                {
                                    const basegfx::B2DPoint aCtrl(rPolygon.getNextControlPoint(b));

                                    DoubleToSVBT64(aCtrl.getX(), aSVBT64);
                                    nCrc = vcl_get_checksum(nCrc, aSVBT64, 8);
                                    DoubleToSVBT64(aCtrl.getY(), aSVBT64);
                                    nCrc = vcl_get_checksum(nCrc, aSVBT64, 8);
                                }
                            }
                        }
                    }

                    sal_uInt8 tmp = static_cast<sal_uInt8>(rAct.IsClipping());
                    nCrc = vcl_get_checksum(nCrc, &tmp, 1);
                }
                else
                {
                    SvmWriter aWriter(aMemStm);
                    aWriter.MetaActionHandler(pAction, &aWriteData);
                    nCrc = vcl_get_checksum(nCrc, aMemStm.GetData(), aMemStm.Tell());
                    aMemStm.Seek(0);
                }
            }
            break;

            default:
            {
                SvmWriter aWriter(aMemStm);
                aWriter.MetaActionHandler(pAction, &aWriteData);
                nCrc = vcl_get_checksum(nCrc, aMemStm.GetData(), aMemStm.Tell());
                aMemStm.Seek(0);
            }
            break;
        }
    }

    return nCrc;
}

void SvmWriter::MetaActionHandler(MetaAction* pAction, ImplMetaWriteData* pData)
{
    MetaActionType nType = pAction->GetType();

    switch (nType)
    {
        case MetaActionType::NONE:
        {
            ActionHandler(pAction);
        }
        break;

        case MetaActionType::PIXEL:
        {
            auto* pMetaAction = static_cast<MetaPixelAction*>(pAction);
            PixelHandler(pMetaAction);
        }
        break;

        case MetaActionType::POINT:
        {
            auto pMetaAction = static_cast<MetaPointAction*>(pAction);
            PointHandler(pMetaAction);
        }
        break;

        case MetaActionType::LINE:
        {
            auto* pMetaAction = static_cast<MetaLineAction*>(pAction);
            LineHandler(pMetaAction);
        }
        break;

        case MetaActionType::RECT:
        {
            auto* pMetaAction = static_cast<MetaRectAction*>(pAction);
            RectHandler(pMetaAction);
        }
        break;

        case MetaActionType::ROUNDRECT:
        {
            auto* pMetaAction = static_cast<MetaRoundRectAction*>(pAction);
            RoundRectHandler(pMetaAction);
        }
        break;

        case MetaActionType::ELLIPSE:
        {
            auto* pMetaAction = static_cast<MetaEllipseAction*>(pAction);
            EllipseHandler(pMetaAction);
        }
        break;

        case MetaActionType::ARC:
        {
            auto* pMetaAction = static_cast<MetaArcAction*>(pAction);
            ArcHandler(pMetaAction);
        }
        break;

        case MetaActionType::PIE:
        {
            auto* pMetaAction = static_cast<MetaPieAction*>(pAction);
            PieHandler(pMetaAction);
        }
        break;

        case MetaActionType::CHORD:
        {
            auto* pMetaAction = static_cast<MetaChordAction*>(pAction);
            ChordHandler(pMetaAction);
        }
        break;

        case MetaActionType::POLYLINE:
        {
            auto* pMetaAction = static_cast<MetaPolyLineAction*>(pAction);
            PolyLineHandler(pMetaAction);
        }
        break;

        case MetaActionType::POLYGON:
        {
            auto* pMetaAction = static_cast<MetaPolygonAction*>(pAction);
            PolygonHandler(pMetaAction);
        }
        break;

        case MetaActionType::POLYPOLYGON:
        {
            auto* pMetaAction = static_cast<MetaPolyPolygonAction*>(pAction);
            PolyPolygonHandler(pMetaAction);
        }
        break;

        case MetaActionType::TEXT:
        {
            auto* pMetaAction = static_cast<MetaTextAction*>(pAction);
            TextHandler(pMetaAction, pData);
        }
        break;

        case MetaActionType::TEXTARRAY:
        {
            auto* pMetaAction = static_cast<MetaTextArrayAction*>(pAction);
            TextArrayHandler(pMetaAction, pData);
        }
        break;

        case MetaActionType::STRETCHTEXT:
        {
            auto* pMetaAction = static_cast<MetaStretchTextAction*>(pAction);
            StretchTextHandler(pMetaAction, pData);
        }
        break;

        case MetaActionType::TEXTRECT:
        {
            auto* pMetaAction = static_cast<MetaTextRectAction*>(pAction);
            TextRectHandler(pMetaAction, pData);
        }
        break;

        case MetaActionType::TEXTLINE:
        {
            auto* pMetaAction = static_cast<MetaTextLineAction*>(pAction);
            TextLineHandler(pMetaAction);
        }
        break;

        case MetaActionType::BMP:
        {
            auto* pMetaAction = static_cast<MetaBmpAction*>(pAction);
            BmpHandler(pMetaAction);
        }
        break;

        case MetaActionType::BMPSCALE:
        {
            auto* pMetaAction = static_cast<MetaBmpScaleAction*>(pAction);
            BmpScaleHandler(pMetaAction);
        }
        break;

        case MetaActionType::BMPSCALEPART:
        {
            auto* pMetaAction = static_cast<MetaBmpScalePartAction*>(pAction);
            BmpScalePartHandler(pMetaAction);
        }
        break;

        case MetaActionType::BMPEX:
        {
            auto* pMetaAction = static_cast<MetaBmpExAction*>(pAction);
            BmpExHandler(pMetaAction);
        }
        break;

        case MetaActionType::BMPEXSCALE:
        {
            auto* pMetaAction = static_cast<MetaBmpExScaleAction*>(pAction);
            BmpExScaleHandler(pMetaAction);
        }
        break;

        case MetaActionType::BMPEXSCALEPART:
        {
            auto* pMetaAction = static_cast<MetaBmpExScalePartAction*>(pAction);
            BmpExScalePartHandler(pMetaAction);
        }
        break;

        case MetaActionType::MASK:
        {
            auto* pMetaAction = static_cast<MetaMaskAction*>(pAction);
            MaskHandler(pMetaAction);
        }
        break;

        case MetaActionType::MASKSCALE:
        {
            auto* pMetaAction = static_cast<MetaMaskScaleAction*>(pAction);
            MaskScaleHandler(pMetaAction);
        }
        break;

        case MetaActionType::MASKSCALEPART:
        {
            auto* pMetaAction = static_cast<MetaMaskScalePartAction*>(pAction);
            MaskScalePartHandler(pMetaAction);
        }
        break;

        case MetaActionType::GRADIENT:
        {
            auto* pMetaAction = static_cast<MetaGradientAction*>(pAction);
            GradientHandler(pMetaAction);
        }
        break;

        case MetaActionType::GRADIENTEX:
        {
            auto* pMetaAction = static_cast<MetaGradientExAction*>(pAction);
            GradientExHandler(pMetaAction);
        }
        break;

        case MetaActionType::HATCH:
        {
            auto* pMetaAction = static_cast<MetaHatchAction*>(pAction);
            HatchHandler(pMetaAction);
        }
        break;

        case MetaActionType::WALLPAPER:
        {
            auto* pMetaAction = static_cast<MetaWallpaperAction*>(pAction);
            WallpaperHandler(pMetaAction);
        }
        break;

        case MetaActionType::CLIPREGION:
        {
            auto* pMetaAction = static_cast<MetaClipRegionAction*>(pAction);
            ClipRegionHandler(pMetaAction);
        }
        break;

        case MetaActionType::ISECTRECTCLIPREGION:
        {
            auto* pMetaAction = static_cast<MetaISectRectClipRegionAction*>(pAction);
            ISectRectClipRegionHandler(pMetaAction);
        }
        break;

        case MetaActionType::ISECTREGIONCLIPREGION:
        {
            auto* pMetaAction = static_cast<MetaISectRegionClipRegionAction*>(pAction);
            ISectRegionClipRegionHandler(pMetaAction);
        }
        break;

        case MetaActionType::MOVECLIPREGION:
        {
            auto* pMetaAction = static_cast<MetaMoveClipRegionAction*>(pAction);
            MoveClipRegionHandler(pMetaAction);
        }
        break;

        case MetaActionType::LINECOLOR:
        {
            auto* pMetaAction = static_cast<MetaLineColorAction*>(pAction);
            LineColorHandler(pMetaAction);
        }
        break;

        case MetaActionType::FILLCOLOR:
        {
            auto* pMetaAction = static_cast<MetaFillColorAction*>(pAction);
            FillColorHandler(pMetaAction);
        }
        break;

        case MetaActionType::TEXTCOLOR:
        {
            auto* pMetaAction = static_cast<MetaTextColorAction*>(pAction);
            TextColorHandler(pMetaAction);
        }
        break;

        case MetaActionType::TEXTFILLCOLOR:
        {
            auto* pMetaAction = static_cast<MetaTextFillColorAction*>(pAction);
            TextFillColorHandler(pMetaAction);
        }
        break;

        case MetaActionType::TEXTLINECOLOR:
        {
            auto* pMetaAction = static_cast<MetaTextLineColorAction*>(pAction);
            TextLineColorHandler(pMetaAction);
        }
        break;

        case MetaActionType::OVERLINECOLOR:
        {
            auto* pMetaAction = static_cast<MetaOverlineColorAction*>(pAction);
            OverlineColorHandler(pMetaAction);
        }
        break;

        case MetaActionType::TEXTALIGN:
        {
            auto* pMetaAction = static_cast<MetaTextAlignAction*>(pAction);
            TextAlignHandler(pMetaAction);
        }
        break;

        case MetaActionType::MAPMODE:
        {
            auto* pMetaAction = static_cast<MetaMapModeAction*>(pAction);
            MapModeHandler(pMetaAction);
        }
        break;

        case MetaActionType::FONT:
        {
            auto* pMetaAction = static_cast<MetaFontAction*>(pAction);
            FontHandler(pMetaAction, pData);
        }
        break;

        case MetaActionType::PUSH:
        {
            auto* pMetaAction = static_cast<MetaPushAction*>(pAction);
            PushHandler(pMetaAction);
        }
        break;

        case MetaActionType::POP:
        {
            auto* pMetaAction = static_cast<MetaPopAction*>(pAction);
            PopHandler(pMetaAction);
        }
        break;

        case MetaActionType::RASTEROP:
        {
            auto* pMetaAction = static_cast<MetaRasterOpAction*>(pAction);
            RasterOpHandler(pMetaAction);
        }
        break;

        case MetaActionType::Transparent:
        {
            auto* pMetaAction = static_cast<MetaTransparentAction*>(pAction);
            TransparentHandler(pMetaAction);
        }
        break;

        case MetaActionType::FLOATTRANSPARENT:
        {
            auto* pMetaAction = static_cast<MetaFloatTransparentAction*>(pAction);
            FloatTransparentHandler(pMetaAction);
        }
        break;

        case MetaActionType::EPS:
        {
            auto* pMetaAction = static_cast<MetaEPSAction*>(pAction);
            EPSHandler(pMetaAction);
        }
        break;

        case MetaActionType::REFPOINT:
        {
            auto* pMetaAction = static_cast<MetaRefPointAction*>(pAction);
            RefPointHandler(pMetaAction);
        }
        break;

        case MetaActionType::COMMENT:
        {
            auto* pMetaAction = static_cast<MetaCommentAction*>(pAction);
            CommentHandler(pMetaAction);
        }
        break;

        case MetaActionType::LAYOUTMODE:
        {
            auto* pMetaAction = static_cast<MetaLayoutModeAction*>(pAction);
            LayoutModeHandler(pMetaAction);
        }
        break;

        case MetaActionType::TEXTLANGUAGE:
        {
            auto* pMetaAction = static_cast<MetaTextLanguageAction*>(pAction);
            TextLanguageHandler(pMetaAction);
        }
        break;

        case MetaActionType::LINEARGRADIENT:
        {
            auto* pMetaAction = static_cast<MetaLinearGradientAction*>(pAction);
            LinearGradientHandler(pMetaAction);
        }
    }
}

void SvmWriter::ActionHandler(const MetaAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
}

void SvmWriter::PixelHandler(const MetaPixelAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetPoint());
    WriteColor(pAction->GetColor());
}

void SvmWriter::PointHandler(const MetaPointAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetPoint());
}

void SvmWriter::LineHandler(const MetaLineAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);

    // Version 1
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetStartPoint());
    aSerializer.writePoint(pAction->GetEndPoint());
    // Version 2
    WriteLineInfo(mrStream, pAction->GetLineInfo());
}

void SvmWriter::RectHandler(const MetaRectAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
}

void SvmWriter::RoundRectHandler(const MetaRoundRectAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    mrStream.WriteUInt32(pAction->GetHorzRound()).WriteUInt32(pAction->GetVertRound());
}

void SvmWriter::EllipseHandler(const MetaEllipseAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
}

void SvmWriter::ArcHandler(const MetaArcAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    aSerializer.writePoint(pAction->GetStartPoint());
    aSerializer.writePoint(pAction->GetEndPoint());
}

void SvmWriter::PieHandler(const MetaPieAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    aSerializer.writePoint(pAction->GetStartPoint());
    aSerializer.writePoint(pAction->GetEndPoint());
}

void SvmWriter::ChordHandler(const MetaChordAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    aSerializer.writePoint(pAction->GetStartPoint());
    aSerializer.writePoint(pAction->GetEndPoint());
}

void SvmWriter::PolyLineHandler(const MetaPolyLineAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 3);

    tools::Polygon aSimplePoly;
    pAction->GetPolygon().AdaptiveSubdivide(aSimplePoly);

    WritePolygon(mrStream, aSimplePoly); // Version 1
    WriteLineInfo(mrStream, pAction->GetLineInfo()); // Version 2

    bool bHasPolyFlags = pAction->GetPolygon().HasFlags(); // Version 3
    mrStream.WriteBool(bHasPolyFlags);
    if (bHasPolyFlags)
        pAction->GetPolygon().Write(mrStream);
}

void SvmWriter::PolygonHandler(const MetaPolygonAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);

    tools::Polygon aSimplePoly; // Version 1
    pAction->GetPolygon().AdaptiveSubdivide(aSimplePoly);
    WritePolygon(mrStream, aSimplePoly);

    bool bHasPolyFlags = pAction->GetPolygon().HasFlags(); // Version 2
    mrStream.WriteBool(bHasPolyFlags);
    if (bHasPolyFlags)
        pAction->GetPolygon().Write(mrStream);
}

void SvmWriter::PolyPolygonHandler(const MetaPolyPolygonAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);

    sal_uInt16 nNumberOfComplexPolygons = 0;
    sal_uInt16 i, nPolyCount = pAction->GetPolyPolygon().Count();

    tools::Polygon aSimplePoly; // Version 1
    mrStream.WriteUInt16(nPolyCount);
    for (i = 0; i < nPolyCount; i++)
    {
        const tools::Polygon& rPoly = pAction->GetPolyPolygon().GetObject(i);
        if (rPoly.HasFlags())
            nNumberOfComplexPolygons++;
        rPoly.AdaptiveSubdivide(aSimplePoly);
        WritePolygon(mrStream, aSimplePoly);
    }

    mrStream.WriteUInt16(nNumberOfComplexPolygons); // Version 2
    for (i = 0; nNumberOfComplexPolygons && (i < nPolyCount); i++)
    {
        const tools::Polygon& rPoly = pAction->GetPolyPolygon().GetObject(i);
        if (rPoly.HasFlags())
        {
            mrStream.WriteUInt16(i);
            rPoly.Write(mrStream);

            nNumberOfComplexPolygons--;
        }
    }
}

void SvmWriter::TextHandler(const MetaTextAction* pAction, const ImplMetaWriteData* pData)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetPoint());
    mrStream.WriteUniOrByteString(pAction->GetText(), pData->meActualCharSet);
    mrStream.WriteUInt16(pAction->GetIndex());
    mrStream.WriteUInt16(pAction->GetLen());

    write_uInt16_lenPrefixed_uInt16s_FromOUString(mrStream, pAction->GetText()); // version 2
}

void SvmWriter::TextArrayHandler(const MetaTextArrayAction* pAction, const ImplMetaWriteData* pData)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    const std::vector<sal_Int32>& rDXArray = pAction->GetDXArray();

    const sal_Int32 nAryLen = !rDXArray.empty() ? pAction->GetLen() : 0;

    VersionCompatWrite aCompat(mrStream, 2);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetPoint());
    mrStream.WriteUniOrByteString(pAction->GetText(), pData->meActualCharSet);
    mrStream.WriteUInt16(pAction->GetIndex());
    mrStream.WriteUInt16(pAction->GetLen());
    mrStream.WriteInt32(nAryLen);

    for (sal_Int32 i = 0; i < nAryLen; ++i)
        mrStream.WriteInt32(rDXArray[i]);

    write_uInt16_lenPrefixed_uInt16s_FromOUString(mrStream, pAction->GetText()); // version 2
}

void SvmWriter::StretchTextHandler(const MetaStretchTextAction* pAction,
                                   const ImplMetaWriteData* pData)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetPoint());
    mrStream.WriteUniOrByteString(pAction->GetText(), pData->meActualCharSet);
    mrStream.WriteUInt32(pAction->GetWidth());
    mrStream.WriteUInt16(pAction->GetIndex());
    mrStream.WriteUInt16(pAction->GetLen());

    write_uInt16_lenPrefixed_uInt16s_FromOUString(mrStream, pAction->GetText()); // version 2
}

void SvmWriter::TextRectHandler(const MetaTextRectAction* pAction, const ImplMetaWriteData* pData)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    mrStream.WriteUniOrByteString(pAction->GetText(), pData->meActualCharSet);
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetStyle()));

    write_uInt16_lenPrefixed_uInt16s_FromOUString(mrStream, pAction->GetText()); // version 2
}

void SvmWriter::TextLineHandler(const MetaTextLineAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));

    VersionCompatWrite aCompat(mrStream, 2);

    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetStartPoint());

    mrStream.WriteInt32(pAction->GetWidth());
    mrStream.WriteUInt32(pAction->GetStrikeout());
    mrStream.WriteUInt32(pAction->GetUnderline());
    // new in version 2
    mrStream.WriteUInt32(pAction->GetOverline());
}

void SvmWriter::BmpHandler(const MetaBmpAction* pAction)
{
    if (!pAction->GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIB(pAction->GetBitmap(), mrStream, false, true);
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetPoint());
    }
}

void SvmWriter::BmpScaleHandler(const MetaBmpScaleAction* pAction)
{
    if (!pAction->GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIB(pAction->GetBitmap(), mrStream, false, true);
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetPoint());
        aSerializer.writeSize(pAction->GetSize());
    }
}

void SvmWriter::BmpScalePartHandler(const MetaBmpScalePartAction* pAction)
{
    if (!pAction->GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIB(pAction->GetBitmap(), mrStream, false, true);
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetDestPoint());
        aSerializer.writeSize(pAction->GetDestSize());
        aSerializer.writePoint(pAction->GetSrcPoint());
        aSerializer.writeSize(pAction->GetSrcSize());
    }
}

void SvmWriter::BmpExHandler(const MetaBmpExAction* pAction)
{
    if (!pAction->GetBitmapEx().GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIBBitmapEx(pAction->GetBitmapEx(), mrStream);
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetPoint());
    }
}

void SvmWriter::BmpExScaleHandler(const MetaBmpExScaleAction* pAction)
{
    if (!pAction->GetBitmapEx().GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIBBitmapEx(pAction->GetBitmapEx(), mrStream);
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetPoint());
        aSerializer.writeSize(pAction->GetSize());
    }
}

void SvmWriter::BmpExScalePartHandler(const MetaBmpExScalePartAction* pAction)
{
    if (!pAction->GetBitmapEx().GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIBBitmapEx(pAction->GetBitmapEx(), mrStream);
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetDestPoint());
        aSerializer.writeSize(pAction->GetDestSize());
        aSerializer.writePoint(pAction->GetSrcPoint());
        aSerializer.writeSize(pAction->GetSrcSize());
    }
}

void SvmWriter::MaskHandler(const MetaMaskAction* pAction)
{
    if (!pAction->GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIB(pAction->GetBitmap(), mrStream, false, true);
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetPoint());
    }
}

void SvmWriter::MaskScaleHandler(const MetaMaskScaleAction* pAction)
{
    if (!pAction->GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIB(pAction->GetBitmap(), mrStream, false, true);
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetPoint());
        aSerializer.writeSize(pAction->GetSize());
    }
}

void SvmWriter::MaskScalePartHandler(const MetaMaskScalePartAction* pAction)
{
    if (!pAction->GetBitmap().IsEmpty())
    {
        mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
        VersionCompatWrite aCompat(mrStream, 1);
        WriteDIB(pAction->GetBitmap(), mrStream, false, true);
        WriteColor(pAction->GetColor());
        TypeSerializer aSerializer(mrStream);
        aSerializer.writePoint(pAction->GetDestPoint());
        aSerializer.writeSize(pAction->GetDestSize());
        aSerializer.writePoint(pAction->GetSrcPoint());
        aSerializer.writeSize(pAction->GetSrcSize());
    }
}

void SvmWriter::GradientHandler(const MetaGradientAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
    aSerializer.writeGradient(pAction->GetGradient());
}

void SvmWriter::GradientExHandler(const MetaGradientExAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);

    // #i105373# see comment at MetaTransparentAction::Write
    tools::PolyPolygon aNoCurvePolyPolygon;
    pAction->GetPolyPolygon().AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon(mrStream, aNoCurvePolyPolygon);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeGradient(pAction->GetGradient());
}

void SvmWriter::HatchHandler(const MetaHatchAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);

    // #i105373# see comment at MetaTransparentAction::Write
    tools::PolyPolygon aNoCurvePolyPolygon;
    pAction->GetPolyPolygon().AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon(mrStream, aNoCurvePolyPolygon);
    WriteHatch(mrStream, pAction->GetHatch());
}

void SvmWriter::WallpaperHandler(const MetaWallpaperAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);

    WriteWallpaper(mrStream, pAction->GetWallpaper());
}

void SvmWriter::ClipRegionHandler(const MetaClipRegionAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteRegion(mrStream, pAction->GetRegion());
    mrStream.WriteBool(pAction->IsClipping());
}

void SvmWriter::ISectRectClipRegionHandler(const MetaISectRectClipRegionAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeRectangle(pAction->GetRect());
}

void SvmWriter::ISectRegionClipRegionHandler(const MetaISectRegionClipRegionAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteRegion(mrStream, pAction->GetRegion());
}

void SvmWriter::MoveClipRegionHandler(const MetaMoveClipRegionAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    mrStream.WriteInt32(pAction->GetHorzMove()).WriteInt32(pAction->GetVertMove());
}

void SvmWriter::LineColorHandler(const MetaLineColorAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteColor(pAction->GetColor());
    mrStream.WriteBool(pAction->IsSetting());
}

void SvmWriter::FillColorHandler(const MetaFillColorAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteColor(pAction->GetColor());
    mrStream.WriteBool(pAction->IsSetting());
}

void SvmWriter::TextColorHandler(const MetaTextColorAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteColor(pAction->GetColor());
}

void SvmWriter::TextFillColorHandler(const MetaTextFillColorAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteColor(pAction->GetColor());
    mrStream.WriteBool(pAction->IsSetting());
}

void SvmWriter::TextLineColorHandler(const MetaTextLineColorAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteColor(pAction->GetColor());
    mrStream.WriteBool(pAction->IsSetting());
}

void SvmWriter::OverlineColorHandler(const MetaOverlineColorAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteColor(pAction->GetColor());
    mrStream.WriteBool(pAction->IsSetting());
}

void SvmWriter::TextAlignHandler(const MetaTextAlignAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetTextAlign()));
}

void SvmWriter::MapModeHandler(const MetaMapModeAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writeMapMode(pAction->GetMapMode());
}

void SvmWriter::FontHandler(const MetaFontAction* pAction, ImplMetaWriteData* pData)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    WriteFont(mrStream, pAction->GetFont());
    pData->meActualCharSet = pAction->GetFont().GetCharSet();
    if (pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW)
        pData->meActualCharSet = osl_getThreadTextEncoding();
}

void SvmWriter::PushHandler(const MetaPushAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetFlags()));
}

void SvmWriter::PopHandler(const MetaPopAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
}

void SvmWriter::RasterOpHandler(const MetaRasterOpAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetRasterOp()));
}

void SvmWriter::TransparentHandler(const MetaTransparentAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);

    // #i105373# The tools::PolyPolygon in this action may be a curve; this
    // was ignored until now what is an error. To make older office
    // versions work with MetaFiles, i opt for applying AdaptiveSubdivide
    // to the PolyPolygon.
    // The alternative would be to really write the curve information
    // like in MetaPolyPolygonAction::Write (where someone extended it
    // correctly, but not here :-( ).
    // The golden solution would be to combine both, but i think it's
    // not necessary; a good subdivision will be sufficient.
    tools::PolyPolygon aNoCurvePolyPolygon;
    pAction->GetPolyPolygon().AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon(mrStream, aNoCurvePolyPolygon);
    mrStream.WriteUInt16(pAction->GetTransparence());
}

void SvmWriter::FloatTransparentHandler(const MetaFloatTransparentAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);

    SvmWriter aWriter(mrStream);
    GDIMetaFile aMtf = pAction->GetGDIMetaFile();
    aWriter.Write(aMtf);
    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetPoint());
    aSerializer.writeSize(pAction->GetSize());
    aSerializer.writeGradient(pAction->GetGradient());
}

void SvmWriter::EPSHandler(const MetaEPSAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);

    TypeSerializer aSerializer(mrStream);
    aSerializer.writeGfxLink(pAction->GetLink());
    aSerializer.writePoint(pAction->GetPoint());
    aSerializer.writeSize(pAction->GetSize());

    SvmWriter aWriter(mrStream);
    GDIMetaFile aMtf = pAction->GetSubstitute();
    aWriter.Write(aMtf);
}

void SvmWriter::RefPointHandler(const MetaRefPointAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);

    TypeSerializer aSerializer(mrStream);
    aSerializer.writePoint(pAction->GetRefPoint());
    mrStream.WriteBool(pAction->IsSetting());
}

void SvmWriter::CommentHandler(const MetaCommentAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    write_uInt16_lenPrefixed_uInt8s_FromOString(mrStream, pAction->GetComment());
    mrStream.WriteInt32(pAction->GetValue()).WriteUInt32(pAction->GetDataSize());

    if (pAction->GetDataSize())
        mrStream.WriteBytes(pAction->GetData(), pAction->GetDataSize());
}

void SvmWriter::LayoutModeHandler(const MetaLayoutModeAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    mrStream.WriteUInt32(static_cast<sal_uInt32>(pAction->GetLayoutMode()));
}

void SvmWriter::TextLanguageHandler(const MetaTextLanguageAction* pAction)
{
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetType()));
    VersionCompatWrite aCompat(mrStream, 1);
    mrStream.WriteUInt16(static_cast<sal_uInt16>(pAction->GetTextLanguage()));
}

void SvmWriter::LinearGradientHandler(const MetaLinearGradientAction* pActionConst)
{
    MetaLinearGradientAction* pAction = const_cast<MetaLinearGradientAction*>(pActionConst);

    for (auto* pLinearAction : *pAction)
    {
        MetaActionType nType = pLinearAction->GetType();

        switch (nType)
        {
            case MetaActionType::COMMENT:
            {
                auto* pMetaAction = static_cast<MetaCommentAction*>(pLinearAction);
                CommentHandler(pMetaAction);
            }
            break;

            case MetaActionType::FILLCOLOR:
            {
                auto* pMetaAction = static_cast<MetaFillColorAction*>(pLinearAction);
                FillColorHandler(pMetaAction);
            }
            break;

            case MetaActionType::LINECOLOR:
            {
                auto* pMetaAction = static_cast<MetaLineColorAction*>(pLinearAction);
                LineColorHandler(pMetaAction);
            }
            break;

            case MetaActionType::POLYGON:
            {
                auto* pMetaAction = static_cast<MetaPolygonAction*>(pLinearAction);
                PolygonHandler(pMetaAction);
            }
            break;

            case MetaActionType::POLYPOLYGON:
            {
                auto* pMetaAction = static_cast<MetaPolyPolygonAction*>(pLinearAction);
                PolyPolygonHandler(pMetaAction);
            }
            break;

            case MetaActionType::GRADIENT:
            {
                auto* pMetaAction = static_cast<MetaGradientAction*>(pLinearAction);
                GradientHandler(pMetaAction);
            }
            break;

            case MetaActionType::GRADIENTEX:
            {
                auto* pMetaAction = static_cast<MetaGradientExAction*>(pLinearAction);
                GradientExHandler(pMetaAction);
            }
            break;

            default:
                SAL_INFO("vcl.gdi", "SVM gradient action not handled: " << (sal_uInt32)nType);
                break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
