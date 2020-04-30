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

#include <TypeSerializer.hxx>
#include <tools/vcompat.hxx>
#include <sal/log.hxx>
#include <comphelper/fileformat.h>
#include <vcl/gdimtf.hxx>

TypeSerializer::TypeSerializer(SvStream& rStream)
    : GenericTypeSerializer(rStream)
{
}

void TypeSerializer::readGradient(Gradient& rGradient)
{
    VersionCompat aCompat(mrStream, StreamMode::READ);

    sal_uInt16 nStyle;
    Color aStartColor;
    Color aEndColor;
    sal_uInt16 nAngle;
    sal_uInt16 nBorder;
    sal_uInt16 nOffsetX;
    sal_uInt16 nOffsetY;
    sal_uInt16 nIntensityStart;
    sal_uInt16 nIntensityEnd;
    sal_uInt16 nStepCount;

    mrStream.ReadUInt16(nStyle);
    readColor(aStartColor);
    readColor(aEndColor);
    mrStream.ReadUInt16(nAngle);
    mrStream.ReadUInt16(nBorder);
    mrStream.ReadUInt16(nOffsetX);
    mrStream.ReadUInt16(nOffsetY);
    mrStream.ReadUInt16(nIntensityStart);
    mrStream.ReadUInt16(nIntensityEnd);
    mrStream.ReadUInt16(nStepCount);

    rGradient.SetStyle(static_cast<GradientStyle>(nStyle));
    rGradient.SetStartColor(aStartColor);
    rGradient.SetEndColor(aEndColor);
    rGradient.SetAngle(nAngle);
    rGradient.SetBorder(nBorder);
    rGradient.SetOfsX(nOffsetX);
    rGradient.SetOfsY(nOffsetY);
    rGradient.SetStartIntensity(nIntensityStart);
    rGradient.SetEndIntensity(nIntensityEnd);
    rGradient.SetSteps(nStepCount);
}

void TypeSerializer::writeGradient(const Gradient& rGradient)
{
    VersionCompat aCompat(mrStream, StreamMode::WRITE, 1);

    mrStream.WriteUInt16(static_cast<sal_uInt16>(rGradient.GetStyle()));
    writeColor(rGradient.GetStartColor());
    writeColor(rGradient.GetEndColor());
    mrStream.WriteUInt16(rGradient.GetAngle());
    mrStream.WriteUInt16(rGradient.GetBorder());
    mrStream.WriteUInt16(rGradient.GetOfsX());
    mrStream.WriteUInt16(rGradient.GetOfsY());
    mrStream.WriteUInt16(rGradient.GetStartIntensity());
    mrStream.WriteUInt16(rGradient.GetEndIntensity());
    mrStream.WriteUInt16(rGradient.GetSteps());
}

void TypeSerializer::readGfxLink(GfxLink& rGfxLink)
{
    sal_uInt16 nType = 0;
    sal_uInt32 nDataSize = 0;
    sal_uInt32 nUserId = 0;

    Size aSize;
    MapMode aMapMode;
    bool bMapAndSizeValid = false;

    {
        VersionCompat aCompat(mrStream, StreamMode::READ);

        // Version 1
        mrStream.ReadUInt16(nType);
        mrStream.ReadUInt32(nDataSize);
        mrStream.ReadUInt32(nUserId);

        if (aCompat.GetVersion() >= 2)
        {
            readSize(aSize);
            ReadMapMode(mrStream, aMapMode);
            bMapAndSizeValid = true;
        }
    }

    auto nRemainingData = mrStream.remainingSize();
    if (nDataSize > nRemainingData)
    {
        SAL_WARN("vcl", "graphic link stream is smaller than requested size");
        nDataSize = nRemainingData;
    }

    std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nDataSize]);
    mrStream.ReadBytes(pBuffer.get(), nDataSize);

    rGfxLink = GfxLink(std::move(pBuffer), nDataSize, static_cast<GfxLinkType>(nType));
    rGfxLink.SetUserId(nUserId);

    if (bMapAndSizeValid)
    {
        rGfxLink.SetPrefSize(aSize);
        rGfxLink.SetPrefMapMode(aMapMode);
    }
}

void TypeSerializer::writeGfxLink(const GfxLink& rGfxLink)
{
    {
        VersionCompat aCompat(mrStream, StreamMode::WRITE, 2);

        // Version 1
        mrStream.WriteUInt16(sal_uInt16(rGfxLink.GetType()));
        mrStream.WriteUInt32(rGfxLink.GetDataSize());
        mrStream.WriteUInt32(rGfxLink.GetUserId());

        // Version 2
        writeSize(rGfxLink.GetPrefSize());
        WriteMapMode(mrStream, rGfxLink.GetPrefMapMode());
    }

    if (rGfxLink.GetDataSize())
    {
        if (rGfxLink.GetData())
            mrStream.WriteBytes(rGfxLink.GetData(), rGfxLink.GetDataSize());
    }
}

namespace
{
constexpr sal_uInt32 constSvgMagic((sal_uInt32('s') << 24) | (sal_uInt32('v') << 16)
                                   | (sal_uInt32('g') << 8) | sal_uInt32('0'));
constexpr sal_uInt32 constWmfMagic((sal_uInt32('w') << 24) | (sal_uInt32('m') << 16)
                                   | (sal_uInt32('f') << 8) | sal_uInt32('0'));
constexpr sal_uInt32 constEmfMagic((sal_uInt32('e') << 24) | (sal_uInt32('m') << 16)
                                   | (sal_uInt32('f') << 8) | sal_uInt32('0'));
constexpr sal_uInt32 constPdfMagic((sal_uInt32('s') << 24) | (sal_uInt32('v') << 16)
                                   | (sal_uInt32('g') << 8) | sal_uInt32('0'));

#define NATIVE_FORMAT_50 COMPAT_FORMAT('N', 'A', 'T', '5')

} // end anonymous namespace

void TypeSerializer::readGraphic(Graphic& /*rGraphic*/) {}

void TypeSerializer::writeGraphic(const Graphic& rGraphic)
{
    Graphic aGraphic(rGraphic);

    if (!aGraphic.makeAvailable())
        return;

    auto pGfxLink = aGraphic.GetSharedGfxLink();

    if (mrStream.GetVersion() >= SOFFICE_FILEFORMAT_50
        && (mrStream.GetCompressMode() & SvStreamCompressFlags::NATIVE) && pGfxLink
        && pGfxLink->IsNative())
    {
        // native format
        mrStream.WriteUInt32(NATIVE_FORMAT_50);

        // write compat info, destructor writes stuff into the header
        {
            VersionCompat aCompat(mrStream, StreamMode::WRITE, 1);
        }
        pGfxLink->SetPrefMapMode(aGraphic.GetPrefMapMode());
        pGfxLink->SetPrefSize(aGraphic.GetPrefSize());
        writeGfxLink(*pGfxLink);
    }
    else
    {
        // own format
        const SvStreamEndian nOldFormat = mrStream.GetEndian();
        mrStream.SetEndian(SvStreamEndian::LITTLE);

        switch (aGraphic.GetType())
        {
            case GraphicType::NONE:
            case GraphicType::Default:
                break;

            case GraphicType::Bitmap:
            {
                auto pVectorGraphicData = aGraphic.getVectorGraphicData();
                if (pVectorGraphicData)
                {
                    // stream out Vector Graphic defining data (length, byte array and evtl. path)
                    // this is used e.g. in swapping out graphic data and in transporting it over UNO API
                    // as sequence of bytes, but AFAIK not written anywhere to any kind of file, so it should be
                    // no problem to extend it; only used at runtime
                    switch (pVectorGraphicData->getVectorGraphicDataType())
                    {
                        case VectorGraphicDataType::Wmf:
                        {
                            mrStream.WriteUInt32(constWmfMagic);
                            break;
                        }
                        case VectorGraphicDataType::Emf:
                        {
                            mrStream.WriteUInt32(constEmfMagic);
                            break;
                        }
                        case VectorGraphicDataType::Svg:
                        {
                            mrStream.WriteUInt32(constSvgMagic);
                            break;
                        }
                        case VectorGraphicDataType::Pdf:
                        {
                            mrStream.WriteUInt32(constPdfMagic);
                            break;
                        }
                    }

                    sal_uInt32 nSize = pVectorGraphicData->getVectorGraphicDataArrayLength();
                    mrStream.WriteUInt32(nSize);
                    mrStream.WriteBytes(
                        pVectorGraphicData->getVectorGraphicDataArray().getConstArray(), nSize);
                    mrStream.WriteUniOrByteString(pVectorGraphicData->getPath(),
                                                  mrStream.GetStreamCharSet());
                }
                else if (aGraphic.IsAnimated())
                {
                    WriteAnimation(mrStream, aGraphic.GetAnimation());
                }
                else
                {
                    WriteDIBBitmapEx(aGraphic.GetBitmapEx(), mrStream);
                }
            }
            break;

            default:
            {
                if (aGraphic.IsSupportedGraphic())
                    WriteGDIMetaFile(mrStream, rGraphic.GetGDIMetaFile());
            }
            break;
        }
        mrStream.SetEndian(nOldFormat);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
