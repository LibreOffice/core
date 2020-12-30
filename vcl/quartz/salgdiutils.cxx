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

#include <cstdint>

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

void AquaSalGraphics::SetPrinterGraphics( CGContextRef xContext, sal_Int32 nDPIX, sal_Int32 nDPIY )
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

    CGContextRelease(maContextHolder.get());
    CGContextRelease(maBGContextHolder.get());
    CGContextRelease(maCSContextHolder.get());

    maContextHolder.set(nullptr);
    maCSContextHolder.set(nullptr);
    maBGContextHolder.set(nullptr);
}

void AquaSalGraphics::UnsetState()
{
    if (maBGContextHolder.isSet())
    {
        CGContextRelease(maBGContextHolder.get());
        maBGContextHolder.set(nullptr);
    }
    if (maCSContextHolder.isSet())
    {
        CGContextRelease(maCSContextHolder.get());
        maBGContextHolder.set(nullptr);
    }
    if (maContextHolder.isSet())
    {
        maContextHolder.restoreState();
        maContextHolder.set(nullptr);
    }
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = nullptr;
    }
}

/**
 * (re-)create the off-screen maLayer we render everything to if
 * necessary: eg. not initialized yet, or it has an incorrect size.
 */
bool AquaSalGraphics::CheckContext()
{
    if (mbWindow && mpFrame && (mpFrame->getNSWindow() || Application::IsBitmapRendering()))
    {
        const unsigned int nWidth = mpFrame->maGeometry.nWidth;
        const unsigned int nHeight = mpFrame->maGeometry.nHeight;

        // Let's get the window scaling factor if possible, or use 1.0
        // as the scaling factor.
        float fScale = 1.0f;
        if (mpFrame->getNSWindow())
            fScale = [mpFrame->getNSWindow() backingScaleFactor];

        CGLayerRef rReleaseLayer = nullptr;

        // check if a new drawing context is needed (e.g. after a resize)
        if( (unsigned(mnWidth) != nWidth) || (unsigned(mnHeight) != nHeight) )
        {
            mnWidth = nWidth;
            mnHeight = nHeight;
            // prepare to release the corresponding resources
            if (maLayer.isSet())
            {
                rReleaseLayer = maLayer.get();
            }
            else if (maContextHolder.isSet())
            {
                CGContextRelease(maContextHolder.get());
            }
            CGContextRelease(maBGContextHolder.get());
            CGContextRelease(maCSContextHolder.get());

            maContextHolder.set(nullptr);
            maBGContextHolder.set(nullptr);
            maCSContextHolder.set(nullptr);
            maLayer.set(nullptr);
        }

        if (!maContextHolder.isSet())
        {
            const int nBitmapDepth = 32;

            float nScaledWidth = mnWidth * fScale;
            float nScaledHeight = mnHeight * fScale;

            const CGSize aLayerSize = { static_cast<CGFloat>(nScaledWidth), static_cast<CGFloat>(nScaledHeight) };

            const int nBytesPerRow = (nBitmapDepth * nScaledWidth) / 8;
            std::uint32_t nFlags = std::uint32_t(kCGImageAlphaNoneSkipFirst)
                | std::uint32_t(kCGBitmapByteOrder32Host);
            maBGContextHolder.set(CGBitmapContextCreate(
                nullptr, nScaledWidth, nScaledHeight, 8, nBytesPerRow, GetSalData()->mxRGBSpace, nFlags));

            maLayer.set(CGLayerCreateWithContext(maBGContextHolder.get(), aLayerSize, nullptr));
            maLayer.setScale(fScale);

            nFlags = std::uint32_t(kCGImageAlphaPremultipliedFirst)
                | std::uint32_t(kCGBitmapByteOrder32Host);
            maCSContextHolder.set(CGBitmapContextCreate(
                nullptr, nScaledWidth, nScaledHeight, 8, nBytesPerRow, GetSalData()->mxRGBSpace, nFlags));

            CGContextRef xDrawContext = CGLayerGetContext(maLayer.get());
            maContextHolder = xDrawContext;

            if (rReleaseLayer)
            {
                // copy original layer to resized layer
                if (maContextHolder.isSet())
                {
                    CGContextDrawLayerAtPoint(maContextHolder.get(), CGPointZero, rReleaseLayer);
                }
                CGLayerRelease(rReleaseLayer);
            }

            if (maContextHolder.isSet())
            {
                CGContextTranslateCTM(maContextHolder.get(), 0, nScaledHeight);
                CGContextScaleCTM(maContextHolder.get(), 1.0, -1.0);
                CGContextSetFillColorSpace(maContextHolder.get(), GetSalData()->mxRGBSpace);
                CGContextSetStrokeColorSpace(maContextHolder.get(), GetSalData()->mxRGBSpace);
                // apply a scale matrix so everything is auto-magically scaled
                CGContextScaleCTM(maContextHolder.get(), fScale, fScale);
                maContextHolder.saveState();
                SetState();

                // re-enable XOR emulation for the new context
                if (mpXorEmulation)
                    mpXorEmulation->SetTarget(mnWidth, mnHeight, mnBitmapDepth, maContextHolder.get(), maLayer.get());
            }
        }
    }

    SAL_WARN_IF(!maContextHolder.isSet() && !mbPrinter, "vcl", "<<<WARNING>>> AquaSalGraphics::CheckContext() FAILED!!!!");

    return maContextHolder.isSet();
}

/**
 * Blit the contents of our internal maLayer state to the
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
    if (maLayer.isSet() && pContext != nullptr)
    {
        CGContextHolder rCGContextHolder([pContext CGContext]);

        rCGContextHolder.saveState();

        CGMutablePathRef rClip = mpFrame->getClipPath();
        if (rClip)
        {
            CGContextBeginPath(rCGContextHolder.get());
            CGContextAddPath(rCGContextHolder.get(), rClip );
            CGContextClip(rCGContextHolder.get());
        }

        ApplyXorContext();

        const CGSize aSize = maLayer.getSizePoints();
        const CGRect aRect = CGRectMake(0, 0, aSize.width,  aSize.height);
        const CGRect aRectPoints = { CGPointZero, maLayer.getSizePixels() };
        CGContextSetBlendMode(maCSContextHolder.get(), kCGBlendModeCopy);
        CGContextDrawLayerInRect(maCSContextHolder.get(), aRectPoints, maLayer.get());

        CGImageRef img = CGBitmapContextCreateImage(maCSContextHolder.get());
        CGImageRef displayColorSpaceImage = CGImageCreateCopyWithColorSpace(img, [[mpFrame->getNSWindow() colorSpace] CGColorSpace]);
        CGContextSetBlendMode(rCGContextHolder.get(), kCGBlendModeCopy);
        CGContextDrawImage(rCGContextHolder.get(), aRect, displayColorSpaceImage);

        CGImageRelease(img);
        CGImageRelease(displayColorSpaceImage);

        rCGContextHolder.restoreState();
    }
    else
    {
        SAL_WARN_IF( !mpFrame->mbInitShow, "vcl", "UpdateWindow called on uneligible graphics" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
