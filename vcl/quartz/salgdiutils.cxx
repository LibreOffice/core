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

#include <sal/config.h>

#include <basebmp/color.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <vcl/svapp.hxx>

#include "quartz/salgdi.h"
#include "quartz/utils.h"
#include "osx/salframe.h"
#include "osx/saldata.hxx"

void AquaSalGraphics::SetWindowGraphics( AquaSalFrame* pFrame )
{
    mpFrame = pFrame;
    mbWindow = true;
    mbPrinter = false;
    mbVirDev = false;
}

void AquaSalGraphics::SetPrinterGraphics( CGContextRef xContext, long nDPIX, long nDPIY )
{
    mbWindow = false;
    mbPrinter = true;
    mbVirDev = false;

    mrContext = xContext;
    mnRealDPIX = nDPIX;
    mnRealDPIY = nDPIY;

    // a previously set clip path is now invalid
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = nullptr;
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
    mrContext = nullptr;
}

void AquaSalGraphics::UnsetState()
{
    if( mrContext )
    {
        SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << mrContext << ")" );
        CGContextRestoreGState( mrContext );
        mrContext = nullptr;
    }
    if( mxClipPath )
    {
        SAL_INFO( "vcl.cg", "CGPathRelease(" << mxClipPath << ")" );
        CGPathRelease( mxClipPath );
        mxClipPath = nullptr;
    }
}

/**
 * (re-)create the off-screen mxLayer we render everything to if
 * necessary: eg. not initialized yet, or it has an incorrect size.
 */
bool AquaSalGraphics::CheckContext()
{
    if( mbWindow && mpFrame && mpFrame->getNSWindow() )
    {
        const unsigned int nWidth = mpFrame->maGeometry.nWidth;
        const unsigned int nHeight = mpFrame->maGeometry.nHeight;

        CGContextRef rReleaseContext = nullptr;
        CGLayerRef   rReleaseLayer = nullptr;

        // check if a new drawing context is needed (e.g. after a resize)
        if( (unsigned(mnWidth) != nWidth) || (unsigned(mnHeight) != nHeight) )
        {
            mnWidth = nWidth;
            mnHeight = nHeight;
            // prepare to release the corresponding resources
            rReleaseContext = mrContext;
            rReleaseLayer   = mxLayer;
            mrContext = nullptr;
            mxLayer = nullptr;
        }

        if( !mrContext )
        {
            const CGSize aLayerSize = { static_cast<CGFloat>(nWidth), static_cast<CGFloat>(nHeight) };
            NSGraphicsContext* pNSGContext = [NSGraphicsContext graphicsContextWithWindow: mpFrame->getNSWindow()];
            CGContextRef xCGContext = static_cast<CGContextRef>([pNSGContext graphicsPort]);
            mxLayer = CGLayerCreateWithContext( xCGContext, aLayerSize, nullptr );
            SAL_INFO( "vcl.cg", "CGLayerCreateWithContext(" << xCGContext << "," << aLayerSize << ",NULL) = " << mxLayer );
            if( mxLayer )
            {
                mrContext = CGLayerGetContext( mxLayer );
                SAL_INFO( "vcl.cg", "CGLayerGetContext(" << mxLayer << ") = " << mrContext );
            }

            if( mrContext )
            {
                // copy original layer to resized layer
                if( rReleaseLayer )
                {
                    SAL_INFO( "vcl.cg", "CGContextDrawLayerAtPoint(" << mrContext << "," << CGPointZero << "," << rReleaseLayer << ")" );
                    CGContextDrawLayerAtPoint( mrContext, CGPointZero, rReleaseLayer );
                }

                CGContextTranslateCTM( mrContext, 0, nHeight );
                CGContextScaleCTM( mrContext, 1.0, -1.0 );
                CGContextSetFillColorSpace( mrContext, GetSalData()->mxRGBSpace );
                CGContextSetStrokeColorSpace( mrContext, GetSalData()->mxRGBSpace );
                SAL_INFO( "vcl.cg", "CGContextSaveGState(" << mrContext << ") " << ++mnContextStackDepth );
                CGContextSaveGState( mrContext );
                SetState();

                // re-enable XOR emulation for the new context
                if( mpXorEmulation )
                {
                    mpXorEmulation->SetTarget( mnWidth, mnHeight, mnBitmapDepth, mrContext, mxLayer );
                }
            }
        }

        if( rReleaseLayer )
        {
            SAL_INFO( "vcl.cg", "CGLayerRelease(" << rReleaseLayer << ")" );
            CGLayerRelease( rReleaseLayer );
        }
        else if( rReleaseContext )
        {
            SAL_INFO( "vcl.cg", "CGContextRelease(" << rReleaseContext << ")" );
            CGContextRelease( rReleaseContext );
        }
    }

    DBG_ASSERT( mrContext || mbPrinter, "<<<WARNING>>> AquaSalGraphics::CheckContext() FAILED!!!!\n" );
    return (mrContext != nullptr);
}

CGContextRef AquaSalGraphics::GetContext()
{
    if(!mrContext)
    {
        CheckContext();
    }
    return mrContext;
}

/**
 * Blit the contents of our internal mxLayer state to the
 * associated window, if any; cf. drawRect event handling
 * on the frame.
 */
void AquaSalGraphics::UpdateWindow( NSRect& )
{
    if( !mpFrame )
    {
        return;
    }

    NSGraphicsContext* pContext = [NSGraphicsContext currentContext];
    if( (mxLayer != nullptr) && (pContext != nullptr) )
    {
        CGContextRef rCGContext = static_cast<CGContextRef>([pContext graphicsPort]);
        SAL_INFO( "vcl.cg", "[[NSGraphicsContext currentContext] graphicsPort] = " << rCGContext );

        CGMutablePathRef rClip = mpFrame->getClipPath();
        if( rClip )
        {
            CGContextSaveGState( rCGContext );
            SAL_INFO( "vcl.cg", "CGContextBeginPath(" << rCGContext << ")" );
            CGContextBeginPath( rCGContext );
            SAL_INFO( "vcl.cg", "CGContextAddPath(" << rCGContext << "," << rClip << ")" );
            CGContextAddPath( rCGContext, rClip );
            SAL_INFO( "vcl.cg", "CGContextClip(" << rCGContext << ")" );
            CGContextClip( rCGContext );
        }

        ApplyXorContext();
        SAL_INFO( "vcl.cg", "CGContextDrawLayerAtPoint(" << rCGContext << "," << CGPointZero << "," << mxLayer << ")" );
        CGContextDrawLayerAtPoint( rCGContext, CGPointZero, mxLayer );
        if( rClip ) // cleanup clipping
        {
            CGContextRestoreGState( rCGContext );
        }
    }
    else
    {
        DBG_ASSERT( mpFrame->mbInitShow, "UpdateWindow called on uneligible graphics" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
