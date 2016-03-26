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

#include <vcl/GraphicNativeMetadata.hxx>

#include <vcl/gfxlink.hxx>

#include "jpeg/Exif.hxx"
#include <memory>

GraphicNativeMetadata::GraphicNativeMetadata() :
    mRotation(0)
{}

GraphicNativeMetadata::~GraphicNativeMetadata()
{}


bool GraphicNativeMetadata::read(Graphic& rGraphic)
{
    GfxLink aLink = rGraphic.GetLink();
    if ( aLink.GetType() != GfxLinkType::NativeJpeg )
        return false;

    sal_uInt32 aDataSize = aLink.GetDataSize();
    if (!aDataSize)
        return false;

    std::unique_ptr<sal_uInt8[]> aBuffer(new sal_uInt8[aDataSize]);

    memcpy(aBuffer.get(), aLink.GetData(), aDataSize);
    SvMemoryStream aMemoryStream(aBuffer.get(), aDataSize, StreamMode::READ);

    Exif aExif;
    aExif.read(aMemoryStream);
    mRotation = aExif.getRotation();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
