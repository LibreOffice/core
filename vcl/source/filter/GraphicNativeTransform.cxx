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
#include <vcl/graphicfilter.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>


#include "jpeg/Exif.hxx"
#include "jpeg/JpegTransform.hxx"

GraphicNativeTransform::GraphicNativeTransform(Graphic& rGraphic) :
    mrGraphic(rGraphic)
{}

GraphicNativeTransform::~GraphicNativeTransform()
{}

bool GraphicNativeTransform::canBeRotated()
{
    GfxLink aLink = mrGraphic.GetLink();

    // Don't allow rotation on animations for now
    if (mrGraphic.IsAnimated())
    {
        return false;
    }

    if (   aLink.GetType() == GFX_LINK_TYPE_NATIVE_JPG
        || aLink.GetType() == GFX_LINK_TYPE_NATIVE_PNG
        || aLink.GetType() == GFX_LINK_TYPE_NATIVE_GIF
        || aLink.GetType() == GFX_LINK_TYPE_NONE)
    {
        return true;
    }

    return false;
}

bool GraphicNativeTransform::rotate(sal_uInt16 aInputRotation)
{
    // Rotation can be between 0 and 3600
    sal_uInt16 aRotation = aInputRotation % 3600;

    if (aRotation == 0)
    {
        return true; // No rotation is needed
    }
    else if (aRotation != 900 && aRotation != 1800 && aRotation != 2700)
    {
        return false;
    }

    GfxLink aLink = mrGraphic.GetLink();
    if ( aLink.GetType() == GFX_LINK_TYPE_NATIVE_JPG )
    {
        return rotateJPEG(aRotation);
    }
    else if ( aLink.GetType() == GFX_LINK_TYPE_NATIVE_PNG )
    {
        return rotateGeneric(aRotation, OUString("png"));
    }
    else if ( aLink.GetType() == GFX_LINK_TYPE_NATIVE_GIF )
    {
        return rotateGeneric(aRotation, OUString("gif"));
    }
    else if ( aLink.GetType() == GFX_LINK_TYPE_NONE )
    {
        return rotateBitmapOnly(aRotation);
    }
    return false;
}

bool GraphicNativeTransform::rotateBitmapOnly(sal_uInt16 aRotation)
{
    if (mrGraphic.IsAnimated())
    {
        return false;
    }

    BitmapEx aBitmap = mrGraphic.GetBitmapEx();
    aBitmap.Rotate(aRotation, COL_BLACK);
    mrGraphic = aBitmap;

    return true;
}

bool GraphicNativeTransform::rotateGeneric(sal_uInt16 aRotation, OUString aType)
{
    // Can't rotate animations yet
    if (mrGraphic.IsAnimated())
    {
        return false;
    }

    SvMemoryStream aStream;

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    css::uno::Sequence< css::beans::PropertyValue > aFilterData( 3 );
    aFilterData[ 0 ].Name = "Interlaced";
    aFilterData[ 0 ].Value <<= (sal_Int32) 0;
    aFilterData[ 1 ].Name = "Compression";
    aFilterData[ 1 ].Value <<= (sal_Int32) 9;
    aFilterData[ 2 ].Name = "Quality";
    aFilterData[ 2 ].Value <<= (sal_Int32) 90;

    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( aType );

    BitmapEx aBitmap = mrGraphic.GetBitmapEx();
    aBitmap.Rotate(aRotation, COL_BLACK);
    rFilter.ExportGraphic( aBitmap, OUString( "none" ), aStream, nFilterFormat, &aFilterData );

    aStream.Seek( STREAM_SEEK_TO_BEGIN );

    Graphic aGraphic;
    rFilter.ImportGraphic( aGraphic, OUString("import"), aStream );

    mrGraphic = aGraphic;
    return true;
}

bool GraphicNativeTransform::rotateJPEG(sal_uInt16 aRotation)
{
    BitmapEx aBitmap = mrGraphic.GetBitmapEx();

    if (aBitmap.GetSizePixel().Width()  % 16 != 0 ||
        aBitmap.GetSizePixel().Height() % 16 != 0 )
    {
        rotateGeneric(aRotation, OUString("png"));
    }
    else
    {
        GfxLink aLink = mrGraphic.GetLink();

        SvMemoryStream aSourceStream;
        aSourceStream.Write(aLink.GetData(), aLink.GetDataSize());
        aSourceStream.Seek( STREAM_SEEK_TO_BEGIN );

        Orientation aOrientation = TOP_LEFT;

        Exif exif;
        if ( exif.read(aSourceStream) )
        {
            aOrientation = exif.getOrientation();
        }

        SvMemoryStream aTargetStream;
        JpegTransform tranform(aSourceStream, aTargetStream);
        tranform.setRotate(aRotation);
        tranform.perform();

        aTargetStream.Seek( STREAM_SEEK_TO_BEGIN );

        // Reset orientation in exif if needed
        if ( exif.hasExif() && aOrientation != TOP_LEFT)
        {
            exif.setOrientation(TOP_LEFT);
            exif.write(aTargetStream);
        }

        aTargetStream.Seek( STREAM_SEEK_TO_BEGIN );

        Graphic aGraphic;
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.ImportGraphic( aGraphic, OUString("import"), aTargetStream );
        mrGraphic = aGraphic;
    }

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
