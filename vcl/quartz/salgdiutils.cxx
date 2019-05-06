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
#include <sal/log.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <vcl/svapp.hxx>

#include <quartz/salgdi.h>
#include <quartz/utils.h>
#include <osx/salframe.h>
#include <osx/saldata.hxx>

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

    maContextHolder.set(xContext);
    mnRealDPIX = nDPIX;
    mnRealDPIY = nDPIY;

    // a previously set clip path is now invalid
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = nullptr;
    }

    if (maContextHolder.isSet())
    {
        CGContextSetFillColorSpace( maContextHolder.get(), GetSalData()->mxRGBSpace );
        CGContextSetStrokeColorSpace( maContextHolder.get(), GetSalData()->mxRGBSpace );
        CGContextSaveGState( maContextHolder.get() );
        SetState();
    }
}

void AquaSalGraphics::InvalidateContext()
{
    UnsetState();
    maContextHolder.set(nullptr);
}

void AquaSalGraphics::UnsetState()
{
    if (maContextHolder.isSet())
    {
        SAL_INFO( "vcl.cg", "CGContextRestoreGState(" << maContextHolder.get() << ")" );
        CGContextRestoreGState( maContextHolder.get() );
        maContextHolder.set(nullptr);
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
    if (mbWindow && mpFrame && (mpFrame->getNSWindow() || Application::IsBitmapRendering()))
    {
        const unsigned int nWidth = mpFrame->maGeometry.nWidth;
        const unsigned int nHeight = mpFrame->maGeometry.nHeight;

        CGLayerRef rReleaseLayer = nullptr;

        // check if a new drawing context is needed (e.g. after a resize)
        if( (unsigned(mnWidth) != nWidth) || (unsigned(mnHeight) != nHeight) )
        {
            mnWidth = nWidth;
            mnHeight = nHeight;
            // prepare to release the corresponding resources
            if (mxLayer)
                rReleaseLayer = mxLayer;
            else if (maContextHolder.isSet())
            {
                SAL_INFO("vcl.cg", "CGContextRelease(" << maContextHolder.get() << ")");
                CGContextRelease(maContextHolder.get());
            }
            maContextHolder.set(nullptr);
            mxLayer = nullptr;
        }

        if (!maContextHolder.isSet())
        {
            if (mpFrame->getNSWindow())
            {
                const CGSize aLayerSize = { static_cast<CGFloat>(nWidth), static_cast<CGFloat>(nHeight) };
                NSGraphicsContext* pNSGContext = [NSGraphicsContext graphicsContextWithWindow: mpFrame->getNSWindow()];
                CGContextRef xCGContext = [pNSGContext CGContext];
                mxLayer = CGLayerCreateWithContext(xCGContext, aLayerSize, nullptr);
                SAL_INFO("vcl.cg", "CGLayerCreateWithContext(" << xCGContext << "," << aLayerSize << ",NULL) = " << mxLayer);
                if (mxLayer)
                {
                    maContextHolder.set(CGLayerGetContext(mxLayer));
                    SAL_INFO( "vcl.cg", "CGLayerGetContext(" << mxLayer << ") = " << maContextHolder.get() );
                }

                if (rReleaseLayer)
                {
                    // copy original layer to resized layer
                    if (maContextHolder.isSet())
                    {
                        SAL_INFO("vcl.cg", "CGContextDrawLayerAtPoint(" << maContextHolder.get() << "," << CGPointZero << "," << rReleaseLayer << ")");
                        CGContextDrawLayerAtPoint(maContextHolder.get(), CGPointZero, rReleaseLayer);
                    }
                    SAL_INFO("vcl.cg", "CGLayerRelease(" << rReleaseLayer << ")");
                    CGLayerRelease(rReleaseLayer);
                }
            }
            else
            {
                assert(Application::IsBitmapRendering());
                const int nBitmapDepth = 32;
                const int nBytesPerRow = (nBitmapDepth * mnWidth) / 8;
                void* pRawData = std::malloc(nBytesPerRow * mnHeight);
                const int nFlags = kCGImageAlphaNoneSkipFirst;
                maContextHolder.set(CGBitmapContextCreate(pRawData, mnWidth, mnHeight, 8, nBytesPerRow,
                                                  GetSalData()->mxRGBSpace, nFlags));
                SAL_INFO("vcl.cg", "CGBitmapContextCreate(" << mnWidth << "x" << mnHeight
                                   << "x" << nBitmapDepth << ") = " << maContextHolder.get());
            }

            if (maContextHolder.isSet())
            {
                CGContextTranslateCTM(maContextHolder.get(), 0, nHeight);
                CGContextScaleCTM(maContextHolder.get(), 1.0, -1.0);
                CGContextSetFillColorSpace(maContextHolder.get(), GetSalData()->mxRGBSpace);
                CGContextSetStrokeColorSpace(maContextHolder.get(), GetSalData()->mxRGBSpace);
                SAL_INFO("vcl.cg", "CGContextSaveGState(" << maContextHolder.get() << ") " << ++mnContextStackDepth);
                CGContextSaveGState(maContextHolder.get());
                SetState();

                // re-enable XOR emulation for the new context
                if (mpXorEmulation)
                    mpXorEmulation->SetTarget(mnWidth, mnHeight, mnBitmapDepth, maContextHolder.get(), mxLayer);
            }
        }
    }

    SAL_WARN_IF( !maContextHolder.get() && !mbPrinter, "vcl", "<<<WARNING>>> AquaSalGraphics::CheckContext() FAILED!!!!" );
    return maContextHolder.isSet();
}

CGContextRef AquaSalGraphics::GetContext()
{
    if (!maContextHolder.isSet())
    {
        CheckContext();
    }
    return maContextHolder.get();
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
        CGContextRef rCGContext = [pContext CGContext];
        SAL_INFO( "vcl.cg", "[[NSGraphicsContext currentContext] CGContext] = " << rCGContext );

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
        SAL_WARN_IF( !mpFrame->mbInitShow, "vcl", "UpdateWindow called on uneligible graphics" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
