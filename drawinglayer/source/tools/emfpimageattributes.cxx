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

#include "emfpimageattributes.hxx"

using namespace ::com::sun::star;
using namespace ::basegfx;

namespace emfplushelper
{
EMFPImageAttributes::EMFPImageAttributes()
    : EMFPObject()
    , wrapMode(0)
    , clampColor(Color())
    , objectClamp(0)
{
}

EMFPImageAttributes::~EMFPImageAttributes() {}

void EMFPImageAttributes::Read(SvStream& s)
{
    sal_uInt32 graphicsVersion, reserved1, reserved2, tempClampColor;
    sal_uInt8 clampColorBlue, clampColorGreen, clampColorRed, clampColorAlpha;

    s.ReadUInt32(graphicsVersion)
        .ReadUInt32(reserved1)
        .ReadUInt32(wrapMode)
        .ReadUInt32(tempClampColor)
        .ReadUInt32(objectClamp)
        .ReadUInt32(reserved2);

    clampColorBlue = tempClampColor >> 24;
    clampColorGreen = (tempClampColor & 0x00FFFFFF) >> 16;
    clampColorRed = (tempClampColor & 0x0000FFFF) >> 8;
    clampColorAlpha = tempClampColor & 0x000000FF;

    clampColor.SetRed(clampColorRed);
    clampColor.SetGreen(clampColorGreen);
    clampColor.SetBlue(clampColorBlue);
    clampColor.SetAlpha(255 - clampColorAlpha);

    SAL_INFO("drawinglayer", "EMF+\timage attributes");
    SAL_WARN_IF((reserved1 != 0) || (reserved2 != 0), "drawinglayer",
                "Reserved field(s) not zero - reserved1: " << std::hex << reserved1
                                                           << " reserved2: " << reserved2);
    SAL_WARN_IF((objectClamp != EmpPlusRectClamp) && (objectClamp != EmpPlusBitmapClamp),
                "drawinglayer", "Invalid object clamp - set to" << std::hex << objectClamp);
    SAL_INFO("drawinglayer", "EMF+\t image graphics version: 0x"
                                 << std::hex << graphicsVersion << " wrap mode: " << wrapMode
                                 << " clamp color: " << clampColor
                                 << " object clamp: " << objectClamp);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
