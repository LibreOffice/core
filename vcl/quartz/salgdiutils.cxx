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

#include "basebmp/scanlineformats.hxx"
#include "basebmp/color.hxx"

#include "basegfx/range/b2drectangle.hxx"
#include "basegfx/range/b2irange.hxx"
#include "basegfx/vector/b2ivector.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"

#include "vcl/svapp.hxx"

#include "quartz/salgdi.h"
#include "osx/salframe.h"
#include "osx/saldata.hxx"

void AquaSalGraphics::SetWindowGraphics( AquaSalFrame* pFrame )
{
    mpFrame     = pFrame;

    mbWindow    = true;
    mbPrinter   = false;
    mbVirDev    = false;
}

void AquaSalGraphics::SetPrinterGraphics( CGContextRef xContext, long nDPIX, long nDPIY )
{
    mbWindow    = false;
    mbPrinter   = true;
    mbVirDev    = false;

    mrContext   = xContext;
    mnRealDPIX  = nDPIX;
    mnRealDPIY  = nDPIY;

    // a previously set clip path is now invalid
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = NULL;
    }

    if( mrContext )
    {
        CGContextSetFillColorSpace( mrContext, GetSalData()->mxRGBSpace );
        CGContextSetStrokeColorSpace( mrContext, GetSalData()->mxRGBSpace );
        CGContextSaveGState( mrContext );
        SetState();
    }
}

void AquaSalGraphics::InvalidateContext()
{
    UnsetState();
    mrContext = 0;
}

void AquaSalGraphics::UnsetState()
{
    if( mrContext )
    {
        CGContextRestoreGState( mrContext );
        mrContext = 0;
    }
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = NULL;
    }
}

bool AquaSalGraphics::CheckContext()
{
    if( mbWindow && mpFrame && mpFrame->getNSWindow() )
    {
        const unsigned int nWidth = mpFrame->maGeometry.nWidth;
        const unsigned int nHeight = mpFrame->maGeometry.nHeight;

        CGContextRef rReleaseContext = 0;
        CGLayerRef   rReleaseLayer = NULL;

        // check if a new drawing context is needed (e.g. after a resize)
        if( (unsigned(mnWidth) != nWidth) || (unsigned(mnHeight) != nHeight) )
        {
            mnWidth = nWidth;
            mnHeight = nHeight;
            // prepare to release the corresponding resources
            rReleaseContext = mrContext;
            rReleaseLayer   = mxLayer;
            mrContext = NULL;
            mxLayer = NULL;
        }

        if( !mrContext )
        {
            const CGSize aLayerSize = { static_cast<CGFloat>(nWidth), static_cast<CGFloat>(nHeight) };
            NSGraphicsContext* pNSGContext = [NSGraphicsContext graphicsContextWithWindow: mpFrame->getNSWindow()];
            CGContextRef xCGContext = reinterpret_cast<CGContextRef>([pNSGContext graphicsPort]);
            mxLayer = CGLayerCreateWithContext( xCGContext, aLayerSize, NULL );
            if( mxLayer )
                mrContext = CGLayerGetContext( mxLayer );

            if( mrContext )
            {
                // copy original layer to resized layer
                if( rReleaseLayer )
                    CGContextDrawLayerAtPoint( mrContext, CGPointZero, rReleaseLayer );

                CGContextTranslateCTM( mrContext, 0, nHeight );
                CGContextScaleCTM( mrContext, 1.0, -1.0 );
                CGContextSetFillColorSpace( mrContext, GetSalData()->mxRGBSpace );
                CGContextSetStrokeColorSpace( mrContext, GetSalData()->mxRGBSpace );
                CGContextSaveGState( mrContext );
                SetState();

                // re-enable XOR emulation for the new context
                if( mpXorEmulation )
                    mpXorEmulation->SetTarget( mnWidth, mnHeight, mnBitmapDepth, mrContext, mxLayer );
            }
        }

        if( rReleaseLayer )
            CGLayerRelease( rReleaseLayer );
        else if( rReleaseContext )
            CGContextRelease( rReleaseContext );
    }

    DBG_ASSERT( mrContext || mbPrinter, "<<<WARNING>>> AquaSalGraphics::CheckContext() FAILED!!!!\n" );
    return (mrContext != NULL);
}

CGContextRef AquaSalGraphics::GetContext()
{
    if(!mrContext)
    {
        CheckContext();
    }
    return mrContext;
}

void AquaSalGraphics::RefreshRect(float lX, float lY, float lWidth, float lHeight)
{
    if( ! mbWindow ) // view only on Window graphics
        return;

    if( mpFrame )
    {
        // update a little more around the designated rectangle
        // this helps with antialiased rendering
        const Rectangle aVclRect(Point(static_cast<long int>(lX-1),
                    static_cast<long int>(lY-1) ),
                 Size(  static_cast<long int>(lWidth+2),
                    static_cast<long int>(lHeight+2) ) );
        mpFrame->maInvalidRect.Union( aVclRect );
    }
}

void AquaSalGraphics::UpdateWindow( NSRect& )
{
    if( !mpFrame )
        return;
    NSGraphicsContext* pContext = [NSGraphicsContext currentContext];
    if( (mxLayer != NULL) && (pContext != NULL) )
    {
        CGContextRef rCGContext = reinterpret_cast<CGContextRef>([pContext graphicsPort]);

        CGMutablePathRef rClip = mpFrame->getClipPath();
        if( rClip )
        {
            CGContextSaveGState( rCGContext );
            CGContextBeginPath( rCGContext );
            CGContextAddPath( rCGContext, rClip );
            CGContextClip( rCGContext );
        }

        ApplyXorContext();
        CGContextDrawLayerAtPoint( rCGContext, CGPointZero, mxLayer );
        if( rClip ) // cleanup clipping
            CGContextRestoreGState( rCGContext );
    }
    else
        DBG_ASSERT( mpFrame->mbInitShow, "UpdateWindow called on uneligible graphics" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
