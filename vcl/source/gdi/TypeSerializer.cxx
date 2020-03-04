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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
