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

#include <vcl/GraphicNativeTransform.hxx>

#include <vcl/gfxlink.hxx>
#include <vcl/cvtgrf.hxx>


#include "jpeg/Exif.hxx"
#include "jpeg/JpegTransform.hxx"

GraphicNativeTransform::GraphicNativeTransform(Graphic& rGraphic) :
    mrGraphic(rGraphic)
{}

GraphicNativeTransform::~GraphicNativeTransform()
{}

bool GraphicNativeTransform::rotate(sal_uInt16 aRotation)
{
    if (aRotation == 0)
        return true;

    if (aRotation != 900 && aRotation != 1800 && aRotation != 2700)
        return false;

    GfxLink aLink = mrGraphic.GetLink();
    if ( aLink.GetType() == GFX_LINK_TYPE_NATIVE_JPG )
    {
        return rotateJPEG(aRotation);
    }
    else if ( aLink.GetType() == GFX_LINK_TYPE_NATIVE_SVG )
    {
        return rotateSVG(aRotation);
    }
    else if ( aLink.GetType() == GFX_LINK_TYPE_NATIVE_PNG )
    {
        return rotateGeneric(aRotation, CVT_PNG);
    }
    else if ( aLink.GetType() == GFX_LINK_TYPE_NATIVE_GIF )
    {
        return rotateGeneric(aRotation, CVT_PNG);
    }
    return false;
}

bool GraphicNativeTransform::rotateGeneric(sal_uInt16 aRotation, sal_uInt32 aType)
{
    BitmapEx aBitmap = mrGraphic.GetBitmapEx();
    aBitmap.Rotate(aRotation, COL_BLACK);

    SvMemoryStream aStream;
    GraphicConverter::Export(aStream, aBitmap, aType);
    aStream.Seek( STREAM_SEEK_TO_BEGIN );

    Graphic aNewGraphic;
    GraphicConverter::Import(aStream, aNewGraphic, aType);

    mrGraphic = aNewGraphic;
    return true;
}

bool GraphicNativeTransform::rotateSVG(sal_uInt16 aRotation)
{
    GDIMetaFile aGDIMetafile = mrGraphic.GetGDIMetaFile();
    //aGDIMetafile.Rotate(aRotation);

    SvMemoryStream aStream;
    GraphicConverter::Export(aStream, aGDIMetafile, CVT_SVG);
    aStream.Seek( STREAM_SEEK_TO_BEGIN );

    Graphic aNewGraphic;
    GraphicConverter::Import(aStream, aNewGraphic, CVT_SVG);

    mrGraphic = aNewGraphic;
    return true;
}

bool GraphicNativeTransform::rotateJPEG(sal_uInt16 aRotation)
{
    GfxLink aLink = mrGraphic.GetLink();

    sal_uInt32 aDataSize = aLink.GetDataSize();
    sal_uInt8* aInputBuffer = new sal_uInt8[aDataSize];

    memcpy(aInputBuffer, aLink.GetData(), aDataSize);
    SvMemoryStream aSourceStream(aInputBuffer, aDataSize, STREAM_READ);

    SvMemoryStream aTargetStream;
    JpegTransform tranform(aSourceStream, aTargetStream);
    tranform.setRotate(aRotation);
    tranform.perform();

    aTargetStream.Seek( STREAM_SEEK_TO_BEGIN );

    Exif exif;
    exif.setOrientation(Orientation::TOP_LEFT);
    exif.write(aTargetStream);

    aTargetStream.Seek( STREAM_SEEK_TO_END );
    sal_uInt32 aBufferSize = aTargetStream.Tell();
    sal_uInt8* pBuffer = new sal_uInt8[ aBufferSize ];

    aTargetStream.Seek( STREAM_SEEK_TO_BEGIN );
    aTargetStream.Read( pBuffer, aBufferSize );

    BitmapEx aBitmap = mrGraphic.GetBitmapEx();
    aBitmap.Rotate(aRotation, COL_BLACK);
    mrGraphic = aBitmap;
    mrGraphic.SetLink( GfxLink( pBuffer, aBufferSize, aLink.GetType(), sal_True ) );
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
