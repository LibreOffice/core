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
#include <vcl/skia/SkiaHelper.hxx>

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

void AquaGraphicsBackend::copyBits(const SalTwoRect &rPosAry, SalGraphics *pSrcGraphics)
{
    AquaSharedAttributes* pSrcShared = nullptr;

    if (pSrcGraphics)
    {
        AquaSalGraphics* pSrc = static_cast<AquaSalGraphics*>(pSrcGraphics);
        pSrcShared = &pSrc->getAquaGraphicsBackend()->GetShared();
    }
    else
        pSrcShared = &mrShared;

    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0 || rPosAry.mnDestHeight <= 0)
        return;
    if (!mrShared.maContextHolder.isSet())
        return;

    SAL_WARN_IF (!pSrcShared->maLayer.isSet(), "vcl.quartz", "AquaSalGraphics::copyBits() from non-layered graphics this=" << this);

    // Layered graphics are copied by AquaSalGraphics::copyScaledArea() which is able to consider the layer's scaling.

    if (pSrcShared->maLayer.isSet())
        copyScaledArea(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnSrcX, rPosAry.mnSrcY,
                       rPosAry.mnSrcWidth, rPosAry.mnSrcHeight, pSrcShared);
    else
    {
        mrShared.applyXorContext();
        pSrcShared->applyXorContext();
        std::shared_ptr<SalBitmap> pBitmap = pSrcGraphics->GetImpl()->getBitmap(rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                                        rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                                                                        /*bWithoutAlpha*/false);
        if (pBitmap)
        {
            SalTwoRect aPosAry(rPosAry);
            aPosAry.mnSrcX = 0;
            aPosAry.mnSrcY = 0;
            drawBitmap(aPosAry, *pBitmap);
        }
    }
}

void AquaGraphicsBackend::copyArea(tools::Long nDstX, tools::Long nDstY,tools::Long nSrcX, tools::Long nSrcY,
                               tools::Long nSrcWidth, tools::Long nSrcHeight, bool)
{
    if (!mrShared.maContextHolder.isSet())
        return;

    // Functionality is implemented in protected member function AquaSalGraphics::copyScaledArea() which requires an additional
    // parameter of type SalGraphics to be used in AquaSalGraphics::copyBits() too.

    copyScaledArea(nDstX, nDstY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, &mrShared);
}

void AquaGraphicsBackend::copyScaledArea(tools::Long nDstX, tools::Long nDstY,tools::Long nSrcX, tools::Long nSrcY,
                                     tools::Long nSrcWidth, tools::Long nSrcHeight, AquaSharedAttributes* pSrcShared)
{
    SAL_WARN_IF(!mrShared.maLayer.isSet(), "vcl.quartz",
                "AquaSalGraphics::copyScaledArea() without graphics context or for non-layered graphics this=" << this);

    if (!mrShared.maContextHolder.isSet() || !mrShared.maLayer.isSet())
        return;

    // Determine scaled geometry of source and target area assuming source and target area have the same scale

    float fScale = mrShared.maLayer.getScale();
    CGFloat nScaledSourceX = nSrcX * fScale;
    CGFloat nScaledSourceY = nSrcY * fScale;
    CGFloat nScaledTargetX = nDstX * fScale;
    CGFloat nScaledTargetY = nDstY * fScale;
    CGFloat nScaledSourceWidth = nSrcWidth * fScale;
    CGFloat nScaledSourceHeight = nSrcHeight * fScale;

    // Apply XOR context and get copy context from current graphics context or XOR context

    mrShared.applyXorContext();
    mrShared.maContextHolder.saveState();
    CGContextRef xCopyContext = mrShared.maContextHolder.get();
    if (mrShared.mpXorEmulation && mrShared.mpXorEmulation->IsEnabled())
        xCopyContext = mrShared.mpXorEmulation->GetTargetContext();

    // Set scale matrix of copy context to consider layer scaling

    CGContextScaleCTM(xCopyContext, 1 / fScale, 1 / fScale);

    // Creating an additional layer is required for drawing with the required scale and extent at the drawing destination
    // thereafter.

    CGLayerHolder aSourceLayerHolder(pSrcShared->maLayer);
    const CGSize aSourceSize = CGSizeMake(nScaledSourceWidth, nScaledSourceHeight);
    aSourceLayerHolder.set(CGLayerCreateWithContext(xCopyContext, aSourceSize, nullptr));
    const CGContextRef xSourceContext = CGLayerGetContext(aSourceLayerHolder.get());
    CGPoint aSrcPoint = CGPointMake(-nScaledSourceX, -nScaledSourceY);
    if (pSrcShared->isFlipped())
    {
        CGContextTranslateCTM(xSourceContext, 0, nScaledSourceHeight);
        CGContextScaleCTM(xSourceContext, 1, -1);
        aSrcPoint.y = nScaledSourceY + nScaledSourceHeight - mrShared.mnHeight * fScale;
    }
    CGContextSetBlendMode(xSourceContext, kCGBlendModeCopy);
    CGContextDrawLayerAtPoint(xSourceContext, aSrcPoint, pSrcShared->maLayer.get());

    // Copy source area from additional layer to target area

    const CGRect aTargetRect = CGRectMake(nScaledTargetX, nScaledTargetY, nScaledSourceWidth, nScaledSourceHeight);
    CGContextSetBlendMode(xCopyContext, kCGBlendModeCopy);
    CGContextDrawLayerInRect(xCopyContext, aTargetRect, aSourceLayerHolder.get());

    // Housekeeping on exit

    mrShared.maContextHolder.restoreState();
    if (aSourceLayerHolder.get() != mrShared.maLayer.get())
        CGLayerRelease(aSourceLayerHolder.get());

    mrShared.refreshRect(nDstX, nDstY, nSrcWidth, nSrcHeight);
}

void AquaSalGraphics::SetVirDevGraphics(SalVirtualDevice* pVirDev, CGLayerHolder const &rLayer, CGContextRef xContext, int nBitmapDepth)
{
    SAL_INFO("vcl.quartz", "SetVirDevGraphics() this=" << this << " layer=" << rLayer.get() << " context=" << xContext);

    // Set member variables

    InvalidateContext();
    maShared.mbWindow = false;
    maShared.mbPrinter = false;
    maShared.mbVirDev = true;
    maShared.maLayer = rLayer;
    maShared.mnBitmapDepth = nBitmapDepth;

    mpBackend->UpdateGeometryProvider(pVirDev);

    // Get size and scale from layer if set else from bitmap and sal::aqua::getWindowScaling(), which is used to determine
    // scaling for direct graphics output too

    CGSize aSize;
    float fScale;
    if (maShared.maLayer.isSet())
    {
        maShared.maContextHolder.set(CGLayerGetContext(maShared.maLayer.get()));
        aSize = CGLayerGetSize(maShared.maLayer.get());
        fScale = maShared.maLayer.getScale();
    }
    else
    {
        maShared.maContextHolder.set(xContext);
        if (!xContext)
            return;
        aSize.width = CGBitmapContextGetWidth(xContext);
        aSize.height = CGBitmapContextGetHeight(xContext);
        fScale = sal::aqua::getWindowScaling();
    }
    maShared.mnWidth = aSize.width / fScale;
    maShared.mnHeight = aSize.height / fScale;

    // Set color space for fill and stroke

    CGColorSpaceRef aColorSpace = GetSalData()->mxRGBSpace;
    CGContextSetFillColorSpace(maShared.maContextHolder.get(), aColorSpace);
    CGContextSetStrokeColorSpace(maShared.maContextHolder.get(), aColorSpace);

    // Apply scale matrix to virtual device graphics

    CGContextScaleCTM(maShared.maContextHolder.get(), fScale, fScale);

    // Apply XOR emulation if required

    if (maShared.mpXorEmulation)
    {
        maShared.mpXorEmulation->SetTarget(maShared.mnWidth, maShared.mnHeight, maShared.mnBitmapDepth, maShared.maContextHolder.get(), maShared.maLayer.get());
        if (maShared.mpXorEmulation->IsEnabled())
            maShared.maContextHolder.set(maShared.mpXorEmulation->GetMaskContext());
    }

    // Housekeeping on exit

    maShared.maContextHolder.saveState();
    maShared.setState();

    SAL_INFO("vcl.quartz", "SetVirDevGraphics() this=" << this <<
             " (" << maShared.mnWidth << "x" << maShared.mnHeight << ") fScale=" << fScale << " mnBitmapDepth=" << maShared.mnBitmapDepth);
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
    float fScale = sal::aqua::getWindowScaling();
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
        float fScale = sal::aqua::getWindowScaling();
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

    if (mbForeignContext)
    {
        // Do not delete mxContext that we have received from outside VCL
        maLayer.set(nullptr);
        return;
    }

    if (maLayer.isSet())
    {
        if( mpGraphics )
        {
            mpGraphics->SetVirDevGraphics(this, nullptr, nullptr);
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

bool AquaSalVirtualDevice::SetSize(tools::Long nDX, tools::Long nDY, bool bAlphaMaskTransparent)
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
    // sal::aqua::getWindowScaling(), which is used to determine scaling for direct graphics output too

    mnWidth = nDX;
    mnHeight = nDY;
    fScale = sal::aqua::getWindowScaling();
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

        nFlags = uint32_t(kCGImageAlphaPremultipliedFirst) | uint32_t(kCGBitmapByteOrder32Host);
    }

    if (SkiaHelper::isVCLSkiaEnabled())
    {
        mpGraphics->SetVirDevGraphics(this, maLayer, nullptr, mnBitmapDepth);
        return true;
    }

    // Allocate buffer for virtual device graphics as bitmap context to store graphics with highest required (scaled) resolution

    size_t nScaledWidth = mnWidth * fScale;
    size_t nScaledHeight = mnHeight * fScale;
    size_t nBytesPerRow = mnBitmapDepth * nScaledWidth / 8;
    maBitmapContext.set(CGBitmapContextCreate(nullptr, nScaledWidth, nScaledHeight, 8, nBytesPerRow, aColorSpace, nFlags));

    if (mnBitmapDepth == 32)
    {
        if (bAlphaMaskTransparent)
        {
            CGColorRef transparentCol = CGColorCreateGenericRGB(0, 0, 0, 0);
            CGContextSetFillColorWithColor(maBitmapContext.get(), transparentCol);
            CGContextFillRect(maBitmapContext.get(), CGRectMake(0, 0, nScaledWidth, nScaledHeight));
            CGColorRelease(transparentCol);
        }
        else
        {
            CGColorRef opaqueCol = CGColorCreateGenericRGB(0, 0, 0, 1.0);
            CGContextSetFillColorWithColor(maBitmapContext.get(), opaqueCol);
            CGContextFillRect(maBitmapContext.get(), CGRectMake(0, 0, nScaledWidth, nScaledHeight));
            CGColorRelease(opaqueCol);
        }
    }

    SAL_INFO("vcl.virdev", "AquaSalVirtualDevice::SetSize() this=" << this <<
             " fScale=" << fScale << " mnBitmapDepth=" << mnBitmapDepth);

    CGSize aLayerSize = { static_cast<CGFloat>(nScaledWidth), static_cast<CGFloat>(nScaledHeight) };
    maLayer.set(CGLayerCreateWithContext(maBitmapContext.get(), aLayerSize, nullptr));
    maLayer.setScale(fScale);
    mpGraphics->SetVirDevGraphics(this, maLayer, CGLayerGetContext(maLayer.get()), mnBitmapDepth);

    SAL_WARN_IF(!maBitmapContext.isSet(), "vcl.quartz", "No context");

    return maLayer.isSet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
