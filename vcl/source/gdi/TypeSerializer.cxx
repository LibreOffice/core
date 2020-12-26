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

#include <vcl/TypeSerializer.hxx>
#include <tools/vcompat.hxx>
#include <sal/log.hxx>
#include <comphelper/fileformat.h>
#include <vcl/gdimtf.hxx>
#include <vcl/dibtools.hxx>

TypeSerializer::TypeSerializer(SvStream& rStream)
    : GenericTypeSerializer(rStream)
{
}

void TypeSerializer::readGradient(Gradient& rGradient)
{
    VersionCompat aCompat(mrStream, StreamMode::READ);

    sal_uInt16 nStyle = 0;
    Color aStartColor;
    Color aEndColor;
    sal_uInt16 nAngle = 0;
    sal_uInt16 nBorder = 0;
    sal_uInt16 nOffsetX = 0;
    sal_uInt16 nOffsetY = 0;
    sal_uInt16 nIntensityStart = 0;
    sal_uInt16 nIntensityEnd = 0;
    sal_uInt16 nStepCount = 0;

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
    if (nAngle > 3600)
    {
        SAL_WARN("vcl", "angle out of range " << nAngle);
        nAngle = 0;
    }
    rGradient.SetAngle(Degree10(nAngle));
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
    mrStream.WriteUInt16(rGradient.GetAngle().get());
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
#define NATIVE_FORMAT_50 COMPAT_FORMAT('N', 'A', 'T', '5')

} // end anonymous namespace

void TypeSerializer::readGraphic(Graphic& rGraphic)
{
    if (mrStream.GetError())
        return;

    const sal_uLong nInitialStreamPosition = mrStream.Tell();
    sal_uInt32 nType;

    // if there is no more data, avoid further expensive
    // reading which will create VDevs and other stuff, just to
    // read nothing.
    if (mrStream.remainingSize() < 4)
        return;

    // read Id
    mrStream.ReadUInt32(nType);

    if (NATIVE_FORMAT_50 == nType)
    {
        Graphic aGraphic;
        GfxLink aLink;

        // read compat info, destructor writes stuff into the header
        {
            VersionCompat aCompat(mrStream, StreamMode::READ);
        }

        readGfxLink(aLink);

        if (!mrStream.GetError() && aLink.LoadNative(aGraphic))
        {
            if (aLink.IsPrefMapModeValid())
                aGraphic.SetPrefMapMode(aLink.GetPrefMapMode());

            if (aLink.IsPrefSizeValid())
                aGraphic.SetPrefSize(aLink.GetPrefSize());
        }
        else
        {
            mrStream.Seek(nInitialStreamPosition);
            mrStream.SetError(ERRCODE_IO_WRONGFORMAT);
        }
        rGraphic = aGraphic;
    }
    else
    {
        BitmapEx aBitmapEx;
        const SvStreamEndian nOldFormat = mrStream.GetEndian();

        mrStream.SeekRel(-4);
        mrStream.SetEndian(SvStreamEndian::LITTLE);
        ReadDIBBitmapEx(aBitmapEx, mrStream);

        if (!mrStream.GetError())
        {
            sal_uInt32 nMagic1 = 0;
            sal_uInt32 nMagic2 = 0;
            if (mrStream.remainingSize() >= 8)
            {
                sal_uInt64 nBeginPosition = mrStream.Tell();
                mrStream.ReadUInt32(nMagic1);
                mrStream.ReadUInt32(nMagic2);
                mrStream.Seek(nBeginPosition);
            }
            if (!mrStream.GetError())
            {
                if (nMagic1 == 0x5344414e && nMagic2 == 0x494d4931)
                {
                    Animation aAnimation;
                    ReadAnimation(mrStream, aAnimation);

                    // #108077# manually set loaded BmpEx to Animation
                    // (which skips loading its BmpEx if already done)
                    aAnimation.SetBitmapEx(aBitmapEx);
                    rGraphic = Graphic(aAnimation);
                }
                else
                {
                    rGraphic = Graphic(aBitmapEx);
                }
            }
            else
            {
                mrStream.ResetError();
            }
        }
        else
        {
            GDIMetaFile aMetaFile;

            mrStream.Seek(nInitialStreamPosition);
            mrStream.ResetError();
            ReadGDIMetaFile(mrStream, aMetaFile);

            if (!mrStream.GetError())
            {
                rGraphic = Graphic(aMetaFile);
            }
            else
            {
                ErrCode nOriginalError = mrStream.GetErrorCode();
                // try to stream in Svg defining data (length, byte array and evtl. path)
                // See below (operator<<) for more information
                sal_uInt32 nMagic;
                mrStream.Seek(nInitialStreamPosition);
                mrStream.ResetError();
                mrStream.ReadUInt32(nMagic);

                if (constSvgMagic == nMagic || constWmfMagic == nMagic || constEmfMagic == nMagic
                    || constPdfMagic == nMagic)
                {
                    sal_uInt32 nLength = 0;
                    mrStream.ReadUInt32(nLength);

                    if (nLength)
                    {
                        VectorGraphicDataArray aData(nLength);

                        mrStream.ReadBytes(aData.getArray(), nLength);

                        if (!mrStream.GetError())
                        {
                            VectorGraphicDataType aDataType(VectorGraphicDataType::Svg);

                            switch (nMagic)
                            {
                                case constWmfMagic:
                                    aDataType = VectorGraphicDataType::Wmf;
                                    break;
                                case constEmfMagic:
                                    aDataType = VectorGraphicDataType::Emf;
                                    break;
                                case constPdfMagic:
                                    aDataType = VectorGraphicDataType::Pdf;
                                    break;
                            }

                            auto aVectorGraphicDataPtr
                                = std::make_shared<VectorGraphicData>(aData, aDataType);
                            rGraphic = Graphic(aVectorGraphicDataPtr);
                        }
                    }
                }
                else
                {
                    mrStream.SetError(nOriginalError);
                }

                mrStream.Seek(nInitialStreamPosition);
            }
        }
        mrStream.SetEndian(nOldFormat);
    }
}

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
                    // For backwards compatibility, used to serialize path
                    mrStream.WriteUniOrByteString(OUString(), mrStream.GetStreamCharSet());
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
