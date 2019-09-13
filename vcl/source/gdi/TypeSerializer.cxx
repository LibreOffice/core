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

void TypeSerializer::writeGradient(Gradient& rGradient)
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
