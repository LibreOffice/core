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
#include <sal/types.h>

#include <boost/scoped_array.hpp>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include "outdata.hxx"
#include "salgdi.hxx"

extern const sal_uLong nVCLRLut[ 6 ];
extern const sal_uLong nVCLGLut[ 6 ];
extern const sal_uLong nVCLBLut[ 6 ];
extern const sal_uLong nVCLDitherLut[ 256 ];
extern const sal_uLong nVCLLut[ 256 ];

Color OutputDevice::GetPixel( const Point& rPt ) const
{

    Color aColor;

    if ( mpGraphics || AcquireGraphics() )
    {
        if ( mbInitClipRegion )
            const_cast<OutputDevice*>(this)->InitClipRegion();

        if ( !mbOutputClipped )
        {
            const long nX = ImplLogicXToDevicePixel( rPt.X() );
            const long nY = ImplLogicYToDevicePixel( rPt.Y() );
            const SalColor aSalCol = mpGraphics->GetPixel( nX, nY, this );
            aColor.SetRed( SALCOLOR_RED( aSalCol ) );
            aColor.SetGreen( SALCOLOR_GREEN( aSalCol ) );
            aColor.SetBlue( SALCOLOR_BLUE( aSalCol ) );
        }
    }
    return aColor;
}

void OutputDevice::DrawPixel( const Point& rPt )
{
    assert(!dynamic_cast<vcl::Window*>(this) || !dynamic_cast<vcl::Window*>(this)->SupportsDoubleBuffering());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPointAction( rPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ImplIsRecordLayout() )
        return;

    Point aPt = ImplLogicToDevicePixel( rPt );

    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPt );
}

void OutputDevice::DrawPixel( const Point& rPt, const Color& rColor )
{
    assert(!dynamic_cast<vcl::Window*>(this) || !dynamic_cast<vcl::Window*>(this)->SupportsDoubleBuffering());

    Color aColor = ImplDrawModeToColor( rColor );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPixelAction( rPt, aColor ) );

    if ( !IsDeviceOutputNecessary() || ImplIsColorTransparent( aColor ) || ImplIsRecordLayout() )
        return;

    Point aPt = ImplLogicToDevicePixel( rPt );

    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), ImplColorToSal( aColor ), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPt );
}

void OutputDevice::DrawPixel( const Polygon& rPts, const Color* pColors )
{
    assert(!dynamic_cast<vcl::Window*>(this) || !dynamic_cast<vcl::Window*>(this)->SupportsDoubleBuffering());

    if ( !pColors )
    {
        DrawPixel( rPts, GetLineColor() );
    }
    else
    {
        DBG_ASSERT( pColors, "OutputDevice::DrawPixel: No color array specified" );

        const sal_uInt16 nSize = rPts.GetSize();

        if ( nSize )
        {
            if ( mpMetaFile )
            {
                for ( sal_uInt16 i = 0; i < nSize; i++ )
                {
                    mpMetaFile->AddAction( new MetaPixelAction( rPts[ i ], pColors[ i ] ) );
                }
            }
            if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
                return;

            if ( mpGraphics || AcquireGraphics() )
            {
                if ( mbInitClipRegion )
                    InitClipRegion();

                if ( mbOutputClipped )
                    return;

                for ( sal_uInt16 i = 0; i < nSize; i++ )
                {
                    const Point aPt( ImplLogicToDevicePixel( rPts[ i ] ) );
                    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), ImplColorToSal( pColors[ i ] ), this );
                }
            }
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPts, pColors );
}

void OutputDevice::DrawPixel( const Polygon& rPts, const Color& rColor )
{
    assert(!dynamic_cast<vcl::Window*>(this) || !dynamic_cast<vcl::Window*>(this)->SupportsDoubleBuffering());

    if( rColor != COL_TRANSPARENT && ! ImplIsRecordLayout() )
    {
        const sal_uInt16 nSize = rPts.GetSize();
        boost::scoped_array<Color> pColArray(new Color[ nSize ]);

        for( sal_uInt16 i = 0; i < nSize; i++ )
        {
            pColArray[ i ] = rColor;
        }
        DrawPixel( rPts, pColArray.get() );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPts, rColor );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
