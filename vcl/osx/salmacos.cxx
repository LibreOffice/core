/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

// This file contains the macOS-specific versions of the functions which were touched in the commit
// to fix tdf#138122. The iOS-specific versions of these functions are kept (for now, when this
// comment is written) as they were before that commit in vcl/ios/salios.cxx.

#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <vcl/bitmap.hxx>

#include <quartz/salbmp.h>
#include <quartz/salgdi.h>
#include <quartz/salvd.h>
#include <quartz/utils.h>

#include <osx/saldata.hxx>

// From salbmp.cxx

bool QuartzSalBitmap::Create(CGLayerHolder const & rLayerHolder, int nBitmapBits, int nX, int nY, int nWidth, int nHeight, bool bFlipped)
{

    // TODO: Bitmaps from scaled layers are reverted to single precision. This is a workaround only unless bitmaps with precision of
    // source layer are implemented.

    SAL_WARN_IF(!rLayerHolder.isSet(), "vcl", "QuartzSalBitmap::Create() from non-layered context");

    // sanitize input parameters
    if( nX < 0 ) {
        nWidth += nX;
        nX = 0;
    }

    if( nY < 0 ) {
        nHeight += nY;
        nY = 0;
    }

    CGSize aLayerSize = CGLayerGetSize(rLayerHolder.get());
    const float fScale = rLayerHolder.getScale();
    aLayerSize.width /= fScale;
    aLayerSize.height /= fScale;

    if( nWidth >= static_cast<int>(aLayerSize.width) - nX )
        nWidth = static_cast<int>(aLayerSize.width) - nX;

    if( nHeight >= static_cast<int>(aLayerSize.height) - nY )
        nHeight = static_cast<int>(aLayerSize.height) - nY;

    if( (nWidth < 0) || (nHeight < 0) )
        nWidth = nHeight = 0;

    // initialize properties
    mnWidth  = nWidth;
    mnHeight = nHeight;
    mnBits   = nBitmapBits ? nBitmapBits : 32;

    // initialize drawing context
    CreateContext();

    // copy layer content into the bitmap buffer
    const CGPoint aSrcPoint = { static_cast<CGFloat>(-nX * fScale), static_cast<CGFloat>(-nY * fScale) };
    if (maGraphicContext.isSet())
    {
        if( bFlipped )
        {
            CGContextTranslateCTM(maGraphicContext.get(), 0, +mnHeight);
            CGContextScaleCTM(maGraphicContext.get(), +1, -1);
        }
        maGraphicContext.saveState();
        CGContextScaleCTM(maGraphicContext.get(), 1 / fScale, 1 / fScale);
        CGContextDrawLayerAtPoint(maGraphicContext.get(), aSrcPoint, rLayerHolder.get());
        maGraphicContext.restoreState();
    }
    return true;
}

// From salgdicommon.cxx

void AquaSalGraphics::copyBits(const SalTwoRect &rPosAry, SalGraphics *pSrcGraphics)
{
    if (!pSrcGraphics)
        pSrcGraphics = this;
    AquaSalGraphics *pSrc = static_cast<AquaSalGraphics*>(pSrcGraphics);
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0 || rPosAry.mnDestHeight <= 0)
        return;
    if (!maContextHolder.isSet())
        return;

    SAL_WARN_IF (!pSrc->maLayer.isSet(), "vcl.quartz", "AquaSalGraphics::copyBits() from non-layered graphics this=" << this);

    // Layered graphics are copied by AquaSalGraphics::copyScaledArea() which is able to consider the layer's scaling.

    if (pSrc->maLayer.isSet())
        copyScaledArea(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnSrcX, rPosAry.mnSrcY,
                       rPosAry.mnSrcWidth, rPosAry.mnSrcHeight, pSrcGraphics);
    else
    {
        ApplyXorContext();
        pSrc->ApplyXorContext();
        std::shared_ptr<SalBitmap> pBitmap = pSrc->getBitmap(rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                             rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
        if (pBitmap)
        {
            SalTwoRect aPosAry(rPosAry);
            aPosAry.mnSrcX = 0;
            aPosAry.mnSrcY = 0;
            drawBitmap(aPosAry, *pBitmap);
        }
    }
}

void AquaSalGraphics::copyArea(tools::Long nDstX, tools::Long nDstY,tools::Long nSrcX, tools::Long nSrcY,
                               tools::Long nSrcWidth, tools::Long nSrcHeight, bool)
{
    if (!maContextHolder.isSet())
        return;

    // Functionality is implemented in protected member function AquaSalGraphics::copyScaledArea() which requires an additional
    // parameter of type SalGraphics to be used in AquaSalGraphics::copyBits() too.

    copyScaledArea(nDstX, nDstY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, this);
}

void AquaSalGraphics::copyScaledArea(tools::Long nDstX, tools::Long nDstY,tools::Long nSrcX, tools::Long nSrcY,
                                     tools::Long nSrcWidth, tools::Long nSrcHeight, SalGraphics *pSrcGraphics)
{
    if (!pSrcGraphics)
        pSrcGraphics = this;
    AquaSalGraphics *pSrc = static_cast<AquaSalGraphics*>(pSrcGraphics);

    SAL_WARN_IF(!maLayer.isSet(), "vcl.quartz",
                "AquaSalGraphics::copyScaledArea() without graphics context or for non-layered graphics this=" << this);

    if (!maContextHolder.isSet() || !maLayer.isSet())
        return;

    // Determine scaled geometry of source and target area assuming source and target area have the same scale

    float fScale = maLayer.getScale();
    CGFloat nScaledSourceX = nSrcX * fScale;
    CGFloat nScaledSourceY = nSrcY * fScale;
    CGFloat nScaledTargetX = nDstX * fScale;
    CGFloat nScaledTargetY = nDstY * fScale;
    CGFloat nScaledSourceWidth = nSrcWidth * fScale;
    CGFloat nScaledSourceHeight = nSrcHeight * fScale;

    // Apply XOR context and get copy context from current graphics context or XOR context

    ApplyXorContext();
    maContextHolder.saveState();
    CGContextRef xCopyContext = maContextHolder.get();
    if (mpXorEmulation && mpXorEmulation->IsEnabled())
        xCopyContext = mpXorEmulation->GetTargetContext();

    // Set scale matrix of copy context to consider layer scaling

    CGContextScaleCTM(xCopyContext, 1 / fScale, 1 / fScale);

    // Creating an additional layer is required for drawing with the required scale and extent at the drawing destination
    // thereafter.

    CGLayerHolder aSourceLayerHolder(pSrc->maLayer);
    const CGSize aSourceSize = CGSizeMake(nScaledSourceWidth, nScaledSourceHeight);
    aSourceLayerHolder.set(CGLayerCreateWithContext(xCopyContext, aSourceSize, nullptr));
    const CGContextRef xSourceContext = CGLayerGetContext(aSourceLayerHolder.get());
    CGPoint aSrcPoint = CGPointMake(-nScaledSourceX, -nScaledSourceY);
    if (pSrc->IsFlipped())
    {
        CGContextTranslateCTM(xSourceContext, 0, nScaledSourceHeight);
        CGContextScaleCTM(xSourceContext, 1, -1);
        aSrcPoint.y = nScaledSourceY + nScaledSourceHeight - mnHeight * fScale;
    }
    CGContextSetBlendMode(xSourceContext, kCGBlendModeCopy);
    CGContextDrawLayerAtPoint(xSourceContext, aSrcPoint, pSrc->maLayer.get());

    // Copy source area from additional layer to target area

    const CGRect aTargetRect = CGRectMake(nScaledTargetX, nScaledTargetY, nScaledSourceWidth, nScaledSourceHeight);
    CGContextSetBlendMode(xCopyContext, kCGBlendModeCopy);
    CGContextDrawLayerInRect(xCopyContext, aTargetRect, aSourceLayerHolder.get());

    // Housekeeping on exit

    maContextHolder.restoreState();
    if (aSourceLayerHolder.get() != maLayer.get())
        CGLayerRelease(aSourceLayerHolder.get());
    RefreshRect(nDstX, nDstY, nSrcWidth, nSrcHeight);
}

void AquaSalGraphics::SetVirDevGraphics(CGLayerHolder const &rLayer, CGContextRef xContext, int nBitmapDepth)
{
    SAL_INFO("vcl.quartz", "SetVirDevGraphics() this=" << this << " layer=" << rLayer.get() << " context=" << xContext);

    // Set member variables

    InvalidateContext();
    mbWindow = false;
    mbPrinter = false;
    mbVirDev = true;
    maLayer = rLayer;
    mnBitmapDepth = nBitmapDepth;

    // Get size and scale from layer if set else from bitmap and AquaSalGraphics::GetWindowScaling(), which is used to determine
    // scaling for direct graphics output too

    CGSize aSize;
    float fScale;
    if (maLayer.isSet())
    {
        maContextHolder.set(CGLayerGetContext(maLayer.get()));
        aSize = CGLayerGetSize(maLayer.get());
        fScale = maLayer.getScale();
    }
    else
    {
        maContextHolder.set(xContext);
        if (!xContext)
            return;
        aSize.width = CGBitmapContextGetWidth(xContext);
        aSize.height = CGBitmapContextGetHeight(xContext);
        fScale = GetWindowScaling();
    }
    mnWidth = aSize.width / fScale;
    mnHeight = aSize.height / fScale;

    // Set color space for fill and stroke

    CGColorSpaceRef aColorSpace = GetSalData()->mxRGBSpace;
    CGContextSetFillColorSpace(maContextHolder.get(), aColorSpace);
    CGContextSetStrokeColorSpace(maContextHolder.get(), aColorSpace);

    // Apply scale matrix to virtual device graphics

    CGContextScaleCTM(maContextHolder.get(), fScale, fScale);

    // Apply XOR emulation if required

    if (mpXorEmulation)
    {
        mpXorEmulation->SetTarget(mnWidth, mnHeight, mnBitmapDepth, maContextHolder.get(), maLayer.get());
        if (mpXorEmulation->IsEnabled())
            maContextHolder.set(mpXorEmulation->GetMaskContext());
    }

    // Housekeeping on exit

    maContextHolder.saveState();
    SetState();

    SAL_INFO("vcl.quartz", "SetVirDevGraphics() this=" << this <<
             " (" << mnWidth << "x" << mnHeight << ") fScale=" << fScale << " mnBitmapDepth=" << mnBitmapDepth);
}

// From salvd.cxx

void AquaSalVirtualDevice::Destroy()
{
    SAL_INFO( "vcl.virdev", "AquaSalVirtualDevice::Destroy() this=" << this << " mbForeignContext=" << mbForeignContext );

    if( mbForeignContext )
    {
        // Do not delete mxContext that we have received from outside VCL
        maLayer.set(nullptr);
        return;
    }

    if (maLayer.isSet())
    {
        if( mpGraphics )
        {
            mpGraphics->SetVirDevGraphics(nullptr, nullptr);
        }
        CGLayerRelease(maLayer.get());
        maLayer.set(nullptr);
    }

    if (maBitmapContext.isSet())
    {
        CGContextRelease(maBitmapContext.get());
        maBitmapContext.set(nullptr);
    }
}

bool AquaSalVirtualDevice::SetSize(tools::Long nDX, tools::Long nDY)
{
    SAL_INFO("vcl.virdev", "AquaSalVirtualDevice::SetSize() this=" << this <<
             " (" << nDX << "x" << nDY << ") mbForeignContext=" << (mbForeignContext ? "YES" : "NO"));

    // Do not delete/resize graphics context if it has been received from outside VCL

    if (mbForeignContext)
        return true;

    // Do not delete/resize graphics context if no change of geometry has been requested

    float fScale;
    if (maLayer.isSet())
    {
        fScale = maLayer.getScale();
        const CGSize aSize = CGLayerGetSize(maLayer.get());
        if ((nDX == aSize.width / fScale) && (nDY  == aSize.height / fScale))
            return true;
    }

    // Destroy graphics context if change of geometry has been requested

    Destroy();

    // Prepare new graphics context for initialization, use scaling independent of prior graphics context calculated by
    // AquaSalGraphics::GetWindowScaling(), which is used to determine scaling for direct graphics output too

    mnWidth = nDX;
    mnHeight = nDY;
    fScale = AquaSalGraphics::GetWindowScaling();
    CGColorSpaceRef aColorSpace;
    uint32_t nFlags;
    if (mnBitmapDepth && (mnBitmapDepth < 16))
    {
        mnBitmapDepth = 8;
        aColorSpace = GetSalData()->mxGraySpace;
        nFlags = kCGImageAlphaNone;
    }
    else
    {
        mnBitmapDepth = 32;
        aColorSpace = GetSalData()->mxRGBSpace;

        nFlags = uint32_t(kCGImageAlphaNoneSkipFirst) | uint32_t(kCGBitmapByteOrder32Host);
    }

    // Allocate buffer for virtual device graphics as bitmap context to store graphics with highest required (scaled) resolution

    size_t nScaledWidth = mnWidth * fScale;
    size_t nScaledHeight = mnHeight * fScale;
    size_t nBytesPerRow = mnBitmapDepth * nScaledWidth / 8;
    maBitmapContext.set(CGBitmapContextCreate(nullptr, nScaledWidth, nScaledHeight, 8, nBytesPerRow, aColorSpace, nFlags));

    SAL_INFO("vcl.virdev", "AquaSalVirtualDevice::SetSize() this=" << this <<
             " fScale=" << fScale << " mnBitmapDepth=" << mnBitmapDepth);

    CGSize aLayerSize = { static_cast<CGFloat>(nScaledWidth), static_cast<CGFloat>(nScaledHeight) };
    maLayer.set(CGLayerCreateWithContext(maBitmapContext.get(), aLayerSize, nullptr));
    maLayer.setScale(fScale);
    mpGraphics->SetVirDevGraphics(maLayer, CGLayerGetContext(maLayer.get()), mnBitmapDepth);

    SAL_WARN_IF(!maBitmapContext.isSet(), "vcl.quartz", "No context");

    return maLayer.isSet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
