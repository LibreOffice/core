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

// This file contains the iOS-specific versions of the functions which were touched in the commit to
// fix tdf#138122. The funtions are here (for now) as they were before that commit. The
// macOS-specific versions of these functions are in vcl/osx/salmacos.cxx.

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

#include <headless/svpframe.hxx>
#include <headless/svpinst.hxx>
#include <headless/svpvd.hxx>

#include <quartz/salbmp.h>
#include <quartz/salgdi.h>
#include <quartz/salvd.h>
#include <quartz/utils.h>

#include "saldatabasic.hxx"

// From salbmp.cxx

bool QuartzSalBitmap::Create(CGLayerHolder const & rLayerHolder, int nBitmapBits, int nX, int nY, int nWidth, int nHeight, bool bFlipped)
{
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

    const CGSize aLayerSize = CGLayerGetSize(rLayerHolder.get());

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
    const CGPoint aSrcPoint = { static_cast<CGFloat>(-nX), static_cast<CGFloat>(-nY) };
    if (maGraphicContext.isSet()) // remove warning
    {
        if( bFlipped )
        {
            CGContextTranslateCTM( maGraphicContext.get(), 0, +mnHeight );

            CGContextScaleCTM( maGraphicContext.get(), +1, -1 );
        }

        CGContextDrawLayerAtPoint(maGraphicContext.get(), aSrcPoint, rLayerHolder.get());
    }
    return true;
}

// From salgdicommon.cxx

void AquaSalGraphics::copyBits( const SalTwoRect& rPosAry, SalGraphics *pSrcGraphics )
{

    if( !pSrcGraphics )
    {
        pSrcGraphics = this;
    }
    //from unix salgdi2.cxx
    //[FIXME] find a better way to prevent calc from crashing when width and height are negative
    if( rPosAry.mnSrcWidth <= 0 ||
        rPosAry.mnSrcHeight <= 0 ||
        rPosAry.mnDestWidth <= 0 ||
        rPosAry.mnDestHeight <= 0 )
    {
        return;
    }

    // If called from idle layout, maContextHolder.get() is NULL, no idea what to do
    if (!maContextHolder.isSet())
        return;

    // accelerate trivial operations
    /*const*/ AquaSalGraphics* pSrc = static_cast<AquaSalGraphics*>(pSrcGraphics);
    const bool bSameGraphics = (this == pSrc);

    if( bSameGraphics &&
        (rPosAry.mnSrcWidth == rPosAry.mnDestWidth) &&
        (rPosAry.mnSrcHeight == rPosAry.mnDestHeight))
    {
        // short circuit if there is nothing to do
        if( (rPosAry.mnSrcX == rPosAry.mnDestX) &&
            (rPosAry.mnSrcY == rPosAry.mnDestY))
        {
            return;
        }
        // use copyArea() if source and destination context are identical
        copyArea( rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnSrcX, rPosAry.mnSrcY,
                  rPosAry.mnSrcWidth, rPosAry.mnSrcHeight, false/*bWindowInvalidate*/ );
        return;
    }

    ApplyXorContext();
    pSrc->ApplyXorContext();

    SAL_WARN_IF (!pSrc->maLayer.isSet(), "vcl.quartz",
                 "AquaSalGraphics::copyBits() from non-layered graphics this=" << this);

    const CGPoint aDstPoint = CGPointMake(+rPosAry.mnDestX - rPosAry.mnSrcX, rPosAry.mnDestY - rPosAry.mnSrcY);
    if ((rPosAry.mnSrcWidth == rPosAry.mnDestWidth &&
         rPosAry.mnSrcHeight == rPosAry.mnDestHeight) &&
        (!mnBitmapDepth || (aDstPoint.x + pSrc->mnWidth) <= mnWidth)
        && pSrc->maLayer.isSet()) // workaround for a Quartz crash
    {
        // in XOR mode the drawing context is redirected to the XOR mask
        // if source and target are identical then copyBits() paints onto the target context though
        CGContextHolder aCopyContext = maContextHolder;
        if( mpXorEmulation && mpXorEmulation->IsEnabled() )
        {
            if( pSrcGraphics == this )
            {
                aCopyContext.set(mpXorEmulation->GetTargetContext());
            }
        }
        aCopyContext.saveState();

        const CGRect aDstRect = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
        CGContextClipToRect(aCopyContext.get(), aDstRect);

        // draw at new destination
        // NOTE: flipped drawing gets disabled for this, else the subimage would be drawn upside down
        if( pSrc->IsFlipped() )
        {
            CGContextTranslateCTM( aCopyContext.get(), 0, +mnHeight );
            CGContextScaleCTM( aCopyContext.get(), +1, -1 );
        }

        // TODO: pSrc->size() != this->size()
        CGContextDrawLayerAtPoint(aCopyContext.get(), aDstPoint, pSrc->maLayer.get());

        aCopyContext.restoreState();
        // mark the destination rectangle as updated
        RefreshRect( aDstRect );
    }
    else
    {
        std::shared_ptr<SalBitmap> pBitmap = pSrc->getBitmap( rPosAry.mnSrcX, rPosAry.mnSrcY,
                                              rPosAry.mnSrcWidth, rPosAry.mnSrcHeight );
        if( pBitmap )
        {
            SalTwoRect aPosAry( rPosAry );
            aPosAry.mnSrcX = 0;
            aPosAry.mnSrcY = 0;
            drawBitmap( aPosAry, *pBitmap );
        }
    }
}

void AquaSalGraphics::copyArea(tools::Long nDstX, tools::Long nDstY,tools::Long nSrcX, tools::Long nSrcY,
                               tools::Long nSrcWidth, tools::Long nSrcHeight, bool /*bWindowInvalidate*/)
{
    SAL_WARN_IF (!maLayer.isSet(), "vcl.quartz",
                 "AquaSalGraphics::copyArea() for non-layered graphics this=" << this);

    if (!maLayer.isSet())
        return;

    float fScale = maLayer.getScale();

    tools::Long nScaledSourceX = nSrcX * fScale;
    tools::Long nScaledSourceY = nSrcY * fScale;

    tools::Long nScaledTargetX = nDstX * fScale;
    tools::Long nScaledTargetY = nDstY * fScale;

    tools::Long nScaledSourceWidth = nSrcWidth * fScale;
    tools::Long nScaledSourceHeight = nSrcHeight * fScale;

    ApplyXorContext();

    maContextHolder.saveState();

    // in XOR mode the drawing context is redirected to the XOR mask
    // copyArea() always works on the target context though
    CGContextRef xCopyContext = maContextHolder.get();

    if( mpXorEmulation && mpXorEmulation->IsEnabled() )
    {
        xCopyContext = mpXorEmulation->GetTargetContext();
    }

    // If we have a scaled layer, we need to revert the scaling or else
    // it will interfere with the coordinate calculation
    CGContextScaleCTM(xCopyContext, 1.0 / fScale, 1.0 / fScale);

    // drawing a layer onto its own context causes trouble on OSX => copy it first
    // TODO: is it possible to get rid of this unneeded copy more often?
    //       e.g. on OSX>=10.5 only this situation causes problems:
    //          mnBitmapDepth && (aDstPoint.x + pSrc->mnWidth) > mnWidth

    CGLayerHolder sSourceLayerHolder(maLayer);
    {
        const CGSize aSrcSize = CGSizeMake(nScaledSourceWidth, nScaledSourceHeight);
        sSourceLayerHolder.set(CGLayerCreateWithContext(xCopyContext, aSrcSize, nullptr));

        const CGContextRef xSrcContext = CGLayerGetContext(sSourceLayerHolder.get());

        CGPoint aSrcPoint = CGPointMake(-nScaledSourceX, -nScaledSourceY);
        if( IsFlipped() )
        {
            CGContextTranslateCTM( xSrcContext, 0, +nScaledSourceHeight );
            CGContextScaleCTM( xSrcContext, +1, -1 );
            aSrcPoint.y = (nScaledSourceY + nScaledSourceHeight) - (mnHeight * fScale);
        }
        CGContextSetBlendMode(xSrcContext, kCGBlendModeCopy);

        CGContextDrawLayerAtPoint(xSrcContext, aSrcPoint, maLayer.get());
    }

    // draw at new destination
    const CGRect aTargetRect = CGRectMake(nScaledTargetX, nScaledTargetY, nScaledSourceWidth, nScaledSourceHeight);
    CGContextSetBlendMode(xCopyContext, kCGBlendModeCopy);
    CGContextDrawLayerInRect(xCopyContext, aTargetRect, sSourceLayerHolder.get());

    maContextHolder.restoreState();

    // cleanup
    if (sSourceLayerHolder.get() != maLayer.get())
    {
        CGLayerRelease(sSourceLayerHolder.get());
    }
    // mark the destination rectangle as updated
    RefreshRect( nDstX, nDstY, nSrcWidth, nSrcHeight );
}

void AquaSalGraphics::SetVirDevGraphics(CGLayerHolder const & rLayer, CGContextRef xContext,
                                        int nBitmapDepth)
{
    SAL_INFO( "vcl.quartz", "SetVirDevGraphics() this=" << this << " layer=" << rLayer.get() << " context=" << xContext );

    mbPrinter = false;
    mbVirDev = true;

    // set graphics properties
    maLayer = rLayer;
    maContextHolder.set(xContext);

    mnBitmapDepth = nBitmapDepth;

    mbForeignContext = xContext != NULL;

    // return early if the virdev is being destroyed
    if( !xContext )
        return;

    // get new graphics properties
    if (!maLayer.isSet())
    {
        mnWidth = CGBitmapContextGetWidth( maContextHolder.get() );
        mnHeight = CGBitmapContextGetHeight( maContextHolder.get() );
    }
    else
    {
        const CGSize aSize = CGLayerGetSize(maLayer.get());
        mnWidth = static_cast<int>(aSize.width);
        mnHeight = static_cast<int>(aSize.height);
    }

    // prepare graphics for drawing
    const CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
    CGContextSetFillColorSpace( maContextHolder.get(), aCGColorSpace );
    CGContextSetStrokeColorSpace( maContextHolder.get(), aCGColorSpace );

    // re-enable XorEmulation for the new context
    if( mpXorEmulation )
    {
        mpXorEmulation->SetTarget(mnWidth, mnHeight, mnBitmapDepth, maContextHolder.get(), maLayer.get());
        if( mpXorEmulation->IsEnabled() )
        {
            maContextHolder.set(mpXorEmulation->GetMaskContext());
        }
    }

    // initialize stack of CGContext states
    maContextHolder.saveState();
    SetState();
}

void XorEmulation::SetTarget( int nWidth, int nHeight, int nTargetDepth,
                              CGContextRef xTargetContext, CGLayerRef xTargetLayer )
{
    SAL_INFO( "vcl.quartz", "XorEmulation::SetTarget() this=" << this <<
              " (" << nWidth << "x" << nHeight << ") depth=" << nTargetDepth <<
              " context=" << xTargetContext << " layer=" << xTargetLayer );

    // prepare to replace old mask+temp context
    if( m_xMaskContext )
    {
        // cleanup the mask context
        CGContextRelease( m_xMaskContext );
        delete[] m_pMaskBuffer;
        m_xMaskContext = nullptr;
        m_pMaskBuffer = nullptr;

        // cleanup the temp context if needed
        if( m_xTempContext )
        {
            CGContextRelease( m_xTempContext );
            delete[] m_pTempBuffer;
            m_xTempContext = nullptr;
            m_pTempBuffer = nullptr;
        }
    }

    // return early if there is nothing more to do
    if( !xTargetContext )
    {
        return;
    }
    // retarget drawing operations to the XOR mask
    m_xTargetLayer = xTargetLayer;
    m_xTargetContext = xTargetContext;

    // prepare creation of matching CGBitmaps
    CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
    CGBitmapInfo aCGBmpInfo = kCGImageAlphaNoneSkipFirst;
    int nBitDepth = nTargetDepth;
    if( !nBitDepth )
    {
        nBitDepth = 32;
    }
    int nBytesPerRow = 4;
    const size_t nBitsPerComponent = (nBitDepth == 16) ? 5 : 8;
    if( nBitDepth <= 8 )
    {
        aCGColorSpace = GetSalData()->mxGraySpace;
        aCGBmpInfo = kCGImageAlphaNone;
        nBytesPerRow = 1;
    }
    nBytesPerRow *= nWidth;
    m_nBufferLongs = (nHeight * nBytesPerRow + sizeof(sal_uLong)-1) / sizeof(sal_uLong);

    // create a XorMask context
    m_pMaskBuffer = new sal_uLong[ m_nBufferLongs ];
    m_xMaskContext = CGBitmapContextCreate( m_pMaskBuffer,
                                            nWidth, nHeight,
                                            nBitsPerComponent, nBytesPerRow,
                                            aCGColorSpace, aCGBmpInfo );
    SAL_WARN_IF( !m_xMaskContext, "vcl.quartz", "mask context creation failed" );

    // reset the XOR mask to black
    memset( m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong) );

    // a bitmap context will be needed for manual XORing
    // create one unless the target context is a bitmap context
    if( nTargetDepth )
    {
        m_pTempBuffer = static_cast<sal_uLong*>(CGBitmapContextGetData( m_xTargetContext ));
    }
    if( !m_pTempBuffer )
    {
        // create a bitmap context matching to the target context
        m_pTempBuffer = new sal_uLong[ m_nBufferLongs ];
        m_xTempContext = CGBitmapContextCreate( m_pTempBuffer,
                                                nWidth, nHeight,
                                                nBitsPerComponent, nBytesPerRow,
                                                aCGColorSpace, aCGBmpInfo );
        SAL_WARN_IF( !m_xTempContext, "vcl.quartz", "temp context creation failed" );
    }

    // initialize XOR mask context for drawing
    CGContextSetFillColorSpace( m_xMaskContext, aCGColorSpace );
    CGContextSetStrokeColorSpace( m_xMaskContext, aCGColorSpace );
    CGContextSetShouldAntialias( m_xMaskContext, false );

    // improve the XorMask's XOR emulation a little
    // NOTE: currently only enabled for monochrome contexts
    if( aCGColorSpace == GetSalData()->mxGraySpace )
    {
        CGContextSetBlendMode( m_xMaskContext, kCGBlendModeDifference );
    }
    // initialize the transformation matrix to the drawing target
    const CGAffineTransform aCTM = CGContextGetCTM( xTargetContext );
    CGContextConcatCTM( m_xMaskContext, aCTM );
    if( m_xTempContext )
    {
        CGContextConcatCTM( m_xTempContext, aCTM );
    }
    // initialize the default XorMask graphics state
    CGContextSaveGState( m_xMaskContext );
}

bool XorEmulation::UpdateTarget()
{
    SAL_INFO( "vcl.quartz", "XorEmulation::UpdateTarget() this=" << this );

    if( !IsEnabled() )
    {
        return false;
    }
    // update the temp bitmap buffer if needed
    if( m_xTempContext )
    {
        SAL_WARN_IF( m_xTargetContext == nullptr, "vcl.quartz", "Target layer is NULL");
        CGContextDrawLayerAtPoint( m_xTempContext, CGPointZero, m_xTargetLayer );
    }
    // do a manual XOR with the XorMask
    // this approach suffices for simple color manipulations
    // and also the complex-clipping-XOR-trick used in metafiles
    const sal_uLong* pSrc = m_pMaskBuffer;
    sal_uLong* pDst = m_pTempBuffer;
    for( int i = m_nBufferLongs; --i >= 0;)
    {
        *(pDst++) ^= *(pSrc++);
    }
    // write back the XOR results to the target context
    if( m_xTempContext )
    {
        CGImageRef xXorImage = CGBitmapContextCreateImage( m_xTempContext );
        const int nWidth  = static_cast<int>(CGImageGetWidth( xXorImage ));
        const int nHeight = static_cast<int>(CGImageGetHeight( xXorImage ));
        // TODO: update minimal changerect
        const CGRect aFullRect = CGRectMake(0, 0, nWidth, nHeight);
        CGContextDrawImage( m_xTargetContext, aFullRect, xXorImage );
        CGImageRelease( xXorImage );
    }

    // reset the XorMask to black again
    // TODO: not needed for last update
    memset( m_pMaskBuffer, 0, m_nBufferLongs * sizeof(sal_uLong) );

    // TODO: return FALSE if target was not changed
    return true;
}

/// From salvd.cxx

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
        void* pRawData = CGBitmapContextGetData(maBitmapContext.get());
        std::free(pRawData);
        CGContextRelease(maBitmapContext.get());
        maBitmapContext.set(nullptr);
    }
}

bool AquaSalVirtualDevice::SetSize( tools::Long nDX, tools::Long nDY )
{
    SAL_INFO( "vcl.virdev", "AquaSalVirtualDevice::SetSize() this=" << this <<
              " (" << nDX << "x" << nDY << ") mbForeignContext=" << (mbForeignContext ? "YES" : "NO"));

    if( mbForeignContext )
    {
        // Do not delete/resize mxContext that we have received from outside VCL
        return true;
    }

    if (maLayer.isSet())
    {
        const CGSize aSize = CGLayerGetSize(maLayer.get());
        if( (nDX == aSize.width) && (nDY == aSize.height) )
        {
            // Yay, we do not have to do anything :)
            return true;
        }
    }

    Destroy();

    mnWidth = nDX;
    mnHeight = nDY;

    // create a CGLayer matching to the intended virdev usage
    CGContextHolder xCGContextHolder;
    if( mnBitmapDepth && (mnBitmapDepth < 16) )
    {
        mnBitmapDepth = 8;  // TODO: are 1bit vdevs worth it?
        const int nBytesPerRow = (mnBitmapDepth * nDX + 7) / 8;

        void* pRawData = std::malloc( nBytesPerRow * nDY );
        maBitmapContext.set(CGBitmapContextCreate( pRawData, nDX, nDY,
                                                 mnBitmapDepth, nBytesPerRow,
                                                 GetSalData()->mxGraySpace, kCGImageAlphaNone));
        xCGContextHolder = maBitmapContext;
    }
    else
    {
        if (!xCGContextHolder.isSet())
        {
            // assert(Application::IsBitmapRendering());
            mnBitmapDepth = 32;

            const int nBytesPerRow = (mnBitmapDepth * nDX) / 8;
            void* pRawData = std::malloc( nBytesPerRow * nDY );
            const int nFlags = kCGImageAlphaNoneSkipFirst | kCGImageByteOrder32Little;
            maBitmapContext.set(CGBitmapContextCreate(pRawData, nDX, nDY, 8, nBytesPerRow,
                                                      GetSalData()->mxRGBSpace, nFlags));
            xCGContextHolder = maBitmapContext;
        }
    }

    SAL_WARN_IF(!xCGContextHolder.isSet(), "vcl.quartz", "No context");

    const CGSize aNewSize = { static_cast<CGFloat>(nDX), static_cast<CGFloat>(nDY) };
    maLayer.set(CGLayerCreateWithContext(xCGContextHolder.get(), aNewSize, nullptr));

    if (maLayer.isSet() && mpGraphics)
    {
        // get the matching Quartz context
        CGContextRef xDrawContext = CGLayerGetContext( maLayer.get() );

        // Here we pass the CGLayerRef that the CGLayerHolder maLayer holds as the first parameter
        // to SetVirDevGraphics(). That parameter is of type CGLayerHolder, so what we actually pass
        // is an implicitly constructed *separate* CGLayerHolder. Is that what we want? No idea.
        // Possibly we could pass just maLayer as such? But doing that does not fix tdf#138122.
        mpGraphics->SetVirDevGraphics(maLayer.get(), xDrawContext, mnBitmapDepth);
    }

    return maLayer.isSet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
