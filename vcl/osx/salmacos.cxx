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

#include <cstddef>
#include <limits>

#include <o3tl/make_shared.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <tools/color.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/BitmapAccessMode.hxx>
#include <vcl/BitmapBuffer.hxx>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapPalette.hxx>
#include <vcl/ColorMask.hxx>
#include <vcl/Scanline.hxx>

#include <quartz/salbmp.h>
#include <quartz/salgdi.h>
#include <quartz/salvd.h>
#include <quartz/utils.h>

#include <osx/saldata.hxx>
#include <osx/salframe.h>
#include <osx/salinst.h>

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

    // Copy source area from additional layer to traget area

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

void XorEmulation::SetTarget(int nWidth, int nHeight, int nTargetDepth, CGContextRef xTargetContext, CGLayerRef xTargetLayer)
{
    SAL_INFO("vcl.quartz", "XorEmulation::SetTarget() this=" << this <<
             " (" << nWidth << "x" << nHeight << ") depth=" << nTargetDepth <<
             " context=" << xTargetContext << " layer=" << xTargetLayer);

    // Prepare to replace old mask and temporary context

    if (m_xMaskContext)
    {
        CGContextRelease(m_xMaskContext);
        delete[] m_pMaskBuffer;
        m_xMaskContext = nullptr;
        m_pMaskBuffer = nullptr;
        if (m_xTempContext)
        {
            CGContextRelease(m_xTempContext);
            delete[] m_pTempBuffer;
            m_xTempContext = nullptr;
            m_pTempBuffer = nullptr;
        }
    }

    // Return early if there is nothing more to do

    if (!xTargetContext)
        return;

    // Retarget drawing operations to the XOR mask

    m_xTargetLayer = xTargetLayer;
    m_xTargetContext = xTargetContext;

    // Prepare creation of matching bitmaps

    CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
    CGBitmapInfo aCGBmpInfo = kCGImageAlphaNoneSkipFirst;
    int nBitDepth = nTargetDepth;
    if (!nBitDepth)
        nBitDepth = 32;
    int nBytesPerRow = 4;
    const size_t nBitsPerComponent = (nBitDepth == 16) ? 5 : 8;
    if (nBitDepth <= 8)
    {
        aCGColorSpace = GetSalData()->mxGraySpace;
        aCGBmpInfo = kCGImageAlphaNone;
        nBytesPerRow = 1;
    }
    float fScale = AquaSalGraphics::GetWindowScaling();
    size_t nScaledWidth = nWidth * fScale;
    size_t nScaledHeight = nHeight * fScale;
    nBytesPerRow *= nScaledWidth;
    m_nBufferLongs = (nScaledHeight * nBytesPerRow + sizeof(sal_uLong) - 1) / sizeof(sal_uLong);

    // Create XOR mask context

    m_pMaskBuffer = new sal_uLong[m_nBufferLongs];
    m_xMaskContext = CGBitmapContextCreate(m_pMaskBuffer, nScaledWidth, nScaledHeight,
                                           nBitsPerComponent, nBytesPerRow, aCGColorSpace, aCGBmpInfo);
    SAL_WARN_IF(!m_xMaskContext, "vcl.quartz", "mask context creation failed");

    // Reset XOR mask to black

    memset(m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong));

    // Create bitmap context for manual XOR unless target context is a bitmap context

    if (nTargetDepth)
        m_pTempBuffer = static_cast<sal_uLong*>(CGBitmapContextGetData(m_xTargetContext));
    if (!m_pTempBuffer)
    {
        m_pTempBuffer = new sal_uLong[m_nBufferLongs];
        m_xTempContext = CGBitmapContextCreate(m_pTempBuffer, nScaledWidth, nScaledHeight,
                                               nBitsPerComponent, nBytesPerRow, aCGColorSpace, aCGBmpInfo);
        SAL_WARN_IF(!m_xTempContext, "vcl.quartz", "temp context creation failed");
    }

    // Initialize XOR mask context for drawing

    CGContextSetFillColorSpace(m_xMaskContext, aCGColorSpace);
    CGContextSetStrokeColorSpace(m_xMaskContext, aCGColorSpace);
    CGContextSetShouldAntialias(m_xMaskContext, false);

    // Improve XOR emulation for monochrome contexts

    if (aCGColorSpace == GetSalData()->mxGraySpace)
        CGContextSetBlendMode(m_xMaskContext, kCGBlendModeDifference);

    // Initialize XOR mask transformation matrix and apply scale matrix to consider layer scaling

    const CGAffineTransform aCTM = CGContextGetCTM(xTargetContext);
    CGContextConcatCTM(m_xMaskContext, aCTM);
    if (m_xTempContext)
    {
        CGContextConcatCTM( m_xTempContext, aCTM );
        CGContextScaleCTM(m_xTempContext, 1 / fScale, 1 / fScale);
    }
    CGContextSaveGState(m_xMaskContext);
}

bool XorEmulation::UpdateTarget()
{
    SAL_INFO("vcl.quartz", "XorEmulation::UpdateTarget() this=" << this);

    if (!IsEnabled())
        return false;

    // Update temporary bitmap buffer

    if (m_xTempContext)
    {
        SAL_WARN_IF(m_xTargetContext == nullptr, "vcl.quartz", "Target layer is NULL");
        CGContextDrawLayerAtPoint(m_xTempContext, CGPointZero, m_xTargetLayer);
    }

    // XOR using XOR mask (sufficient for simple color manipulations as well as for complex XOR clipping used in metafiles)

    const sal_uLong *pSrc = m_pMaskBuffer;
    sal_uLong *pDst = m_pTempBuffer;
    for (int i = m_nBufferLongs; --i >= 0;)
        *(pDst++) ^= *(pSrc++);

    // Write back XOR results to target context

    if (m_xTempContext)
    {
        CGImageRef xXorImage = CGBitmapContextCreateImage(m_xTempContext);
        size_t nWidth = CGImageGetWidth(xXorImage);
        size_t nHeight = CGImageGetHeight(xXorImage);

        // Set scale matrix of target context to consider layer scaling and update target context
        // TODO: Update minimal change rectangle

        const CGRect aFullRect = CGRectMake(0, 0, nWidth, nHeight);
        CGContextSaveGState(m_xTargetContext);
        float fScale = AquaSalGraphics::GetWindowScaling();
        CGContextScaleCTM(m_xTargetContext, 1 / fScale, 1 / fScale);
        CGContextDrawImage(m_xTargetContext, aFullRect, xXorImage);
        CGContextRestoreGState(m_xTargetContext);
        CGImageRelease(xXorImage);
    }

    // Reset XOR mask to black again
    // TODO: Not needed for last update

    memset(m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong));

    // TODO: Return FALSE if target was not changed

    return true;
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

        nFlags = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Host;
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
