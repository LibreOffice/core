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

// TODO: Scale will be set to 2.0f as default after implementation of full scaled display support . This will allow moving of
// windows between non retina and retina displays without blurry text and graphics. Static variables have to be removed thereafter.

// Currently scaled display support is not implemented for bitmaps. This will cause a slight performance degradation on displays
// with single precision. To preserve performance for now, window scaling is only activated if at least one display with double
// precision is present. Moving windows between displays is then possible without blurry text and graphics too. Adapting window
// scaling when displays are added while application is running is not supported.

static bool  bWindowScaling = false;
static float fWindowScale = 1.0f;

namespace sal::aqua
{
float getWindowScaling()
{
    if (!bWindowScaling)
    {
        NSArray *aScreens = [NSScreen screens];
        if (aScreens != nullptr)
        {
            int nScreens = [aScreens count];
            for (int i = 0; i < nScreens; i++)
            {
                float fScale = [[aScreens objectAtIndex:i] backingScaleFactor];
                if (fScale > fWindowScale)
                  fWindowScale = fScale;
            }
            bWindowScaling = true;
        }
    }
    return fWindowScale;
}
} // end aqua

void AquaSalGraphics::SetWindowGraphics( AquaSalFrame* pFrame )
{
    maShared.mpFrame = pFrame;
    maShared.mbWindow = true;
    maShared.mbPrinter = false;
    maShared.mbVirDev = false;
}

void AquaSalGraphics::SetPrinterGraphics( CGContextRef xContext, sal_Int32 nDPIX, sal_Int32 nDPIY )
{
    maShared.mbWindow = false;
    maShared.mbPrinter = true;
    maShared.mbVirDev = false;

    maShared.maContextHolder.set(xContext);
    mnRealDPIX = nDPIX;
    mnRealDPIY = nDPIY;

    // a previously set clip path is now invalid
    maShared.unsetClipPath();

    if (maShared.maContextHolder.isSet())
    {
        CGContextSetFillColorSpace( maShared.maContextHolder.get(), GetSalData()->mxRGBSpace );
        CGContextSetStrokeColorSpace( maShared.maContextHolder.get(), GetSalData()->mxRGBSpace );
        CGContextSaveGState( maShared.maContextHolder.get() );
        maShared.setState();
    }
}

void AquaSalGraphics::InvalidateContext()
{
    UnsetState();

    CGContextRelease(maShared.maContextHolder.get());
    CGContextRelease(maShared.maBGContextHolder.get());
    CGContextRelease(maShared.maCSContextHolder.get());

    maShared.maContextHolder.set(nullptr);
    maShared.maCSContextHolder.set(nullptr);
    maShared.maBGContextHolder.set(nullptr);
}

void AquaSalGraphics::UnsetState()
{
    if (maShared.maBGContextHolder.isSet())
    {
        CGContextRelease(maShared.maBGContextHolder.get());
        maShared.maBGContextHolder.set(nullptr);
    }
    if (maShared.maCSContextHolder.isSet())
    {
        CGContextRelease(maShared.maCSContextHolder.get());
        maShared.maBGContextHolder.set(nullptr);
    }
    if (maShared.maContextHolder.isSet())
    {
        maShared.maContextHolder.restoreState();
        maShared.maContextHolder.set(nullptr);
    }
    maShared.unsetState();
}

/**
 * (re-)create the off-screen maLayer we render everything to if
 * necessary: eg. not initialized yet, or it has an incorrect size.
 */
bool AquaSharedAttributes::checkContext()
{
    if (mbWindow && mpFrame && (mpFrame->getNSWindow() || Application::IsBitmapRendering()))
    {
        const unsigned int nWidth = mpFrame->maGeometry.nWidth;
        const unsigned int nHeight = mpFrame->maGeometry.nHeight;
        const float fScale = sal::aqua::getWindowScaling();
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
                setState();

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
    if (!maShared.mpFrame)
    {
        return;
    }

    NSGraphicsContext* pContext = [NSGraphicsContext currentContext];
    if (maShared.maLayer.isSet() && pContext != nullptr)
    {
        CGContextHolder rCGContextHolder([pContext CGContext]);

        rCGContextHolder.saveState();

        CGMutablePathRef rClip = maShared.mpFrame->getClipPath();
        if (rClip)
        {
            CGContextBeginPath(rCGContextHolder.get());
            CGContextAddPath(rCGContextHolder.get(), rClip );
            CGContextClip(rCGContextHolder.get());
        }

        maShared.applyXorContext();

        const CGSize aSize = maShared.maLayer.getSizePoints();
        const CGRect aRect = CGRectMake(0, 0, aSize.width,  aSize.height);
        const CGRect aRectPoints = { CGPointZero, maShared.maLayer.getSizePixels() };
        CGContextSetBlendMode(maShared.maCSContextHolder.get(), kCGBlendModeCopy);
        CGContextDrawLayerInRect(maShared.maCSContextHolder.get(), aRectPoints, maShared.maLayer.get());

        CGImageRef img = CGBitmapContextCreateImage(maShared.maCSContextHolder.get());
        CGImageRef displayColorSpaceImage = CGImageCreateCopyWithColorSpace(img, [[maShared.mpFrame->getNSWindow() colorSpace] CGColorSpace]);
        CGContextSetBlendMode(rCGContextHolder.get(), kCGBlendModeCopy);
        CGContextDrawImage(rCGContextHolder.get(), aRect, displayColorSpaceImage);

        CGImageRelease(img);
        CGImageRelease(displayColorSpaceImage);

        rCGContextHolder.restoreState();
    }
    else
    {
        SAL_WARN_IF(!maShared.mpFrame->mbInitShow, "vcl", "UpdateWindow called on uneligible graphics");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
