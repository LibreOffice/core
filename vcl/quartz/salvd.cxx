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

#include "vcl/svapp.hxx"
#include "vcl/sysdata.hxx"

#include "quartz/salvd.h"
#ifdef MACOSX
#include "osx/salinst.h"
#include "osx/saldata.hxx"
#include "osx/salframe.h"
#else
#include "headless/svpframe.hxx"
#include "headless/svpgdi.hxx"
#include "headless/svpinst.hxx"
#include "headless/svpvd.hxx"
#endif
#include "quartz/salgdi.h"



SalVirtualDevice* AquaSalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
    long nDX, long nDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData )
{
    // #i92075# can be called first in a thread
    SalData::ensureThreadAutoreleasePool();

#ifdef IOS
    if( pData )
        return new AquaSalVirtualDevice( static_cast< AquaSalGraphics* >( pGraphics ), nDX, nDY, nBitCount, pData );
    else
        return new SvpSalVirtualDevice( nBitCount );
#else
    return new AquaSalVirtualDevice( static_cast< AquaSalGraphics* >( pGraphics ), nDX, nDY, nBitCount, pData );
#endif
}



void AquaSalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
    delete pDevice;
}



AquaSalVirtualDevice::AquaSalVirtualDevice( AquaSalGraphics* pGraphic, long nDX, long nDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData )
:   mbGraphicsUsed( false )
,   mxBitmapContext( NULL )
,   mnBitmapDepth( 0 )
,   mxLayer( NULL )
{
    if( pGraphic && pData && pData->rCGContext )
    {
        // Create virtual device based on existing SystemGraphicsData
        // We ignore nDx and nDY, as the desired size comes from the SystemGraphicsData.
        // WTF does the above mean, SystemGraphicsData has no size field(s).
        mbForeignContext = true;        // the mxContext is from pData
        mpGraphics = new AquaSalGraphics( /*pGraphic*/ );
#ifdef IOS
        // Note: we should *not* create a CGLayer and assign it to
        // mxLayer here. Don't confuse CGLayer and CALayer. A CGLayer
        // is basically a fancy off-screen bitmap not related to
        // anything being displayed at all. The CGContext passed in
        // here refers to something actively part of the compositor
        // stack and being dislayed on the device, and *there*
        // CALayers are involved, sure. The use of mxLayer in this
        // code is for "traditional" LO virtual devices, off-screen
        // bitmaps. I think. On the other hand, the use of
        // VirtualDevice with a "foreign" CGContext for OS X is
        // actually dead code...
#endif
        mpGraphics->SetVirDevGraphics( mxLayer, pData->rCGContext );
    }
    else
    {
        // create empty new virtual device
        mbForeignContext = false;           // the mxContext is created within VCL
        mpGraphics = new AquaSalGraphics(); // never fails
        mnBitmapDepth = nBitCount;
#ifdef MACOSX
        // inherit resolution from reference device
        if( pGraphic )
        {
            AquaSalFrame* pFrame = pGraphic->getGraphicsFrame();
            if( pFrame && AquaSalFrame::isAlive( pFrame ) )
            {
                mpGraphics->setGraphicsFrame( pFrame );
                mpGraphics->copyResolution( *pGraphic );
            }
        }
#endif
        if( nDX && nDY )
            SetSize( nDX, nDY );

        // NOTE: if SetSize does not succeed, we just ignore the nDX and nDY
    }
}



AquaSalVirtualDevice::~AquaSalVirtualDevice()
{
    if( mpGraphics )
    {
        mpGraphics->SetVirDevGraphics( NULL, NULL );
        delete mpGraphics;
        mpGraphics = 0;
    }
    Destroy();
}



void AquaSalVirtualDevice::Destroy()
{
    if( mbForeignContext ) {
        // Do not delete mxContext that we have received from outside VCL
        mxLayer = NULL;
        return;
    }

    if( mxLayer )
    {
        if( mpGraphics )
            mpGraphics->SetVirDevGraphics( NULL, NULL );
        CGLayerRelease( mxLayer );
        mxLayer = NULL;
    }

    if( mxBitmapContext )
    {
        void* pRawData = CGBitmapContextGetData( mxBitmapContext );
        rtl_freeMemory( pRawData );
        CGContextRelease( mxBitmapContext );
        mxBitmapContext = NULL;
    }
}



SalGraphics* AquaSalVirtualDevice::GetGraphics()
{
    if( mbGraphicsUsed || !mpGraphics )
        return 0;

    mbGraphicsUsed = true;
    return mpGraphics;
}



void AquaSalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    mbGraphicsUsed = false;
}



bool AquaSalVirtualDevice::SetSize( long nDX, long nDY )
{
#ifdef IOS
    (void) nDX;
    (void) nDY;
    assert(mbForeignContext);
#endif

    if( mbForeignContext )
    {
        // Do not delete/resize mxContext that we have received from outside VCL
        return true;
    }

#ifdef IOS
    return false;
#else
    if( mxLayer )
    {
        const CGSize aSize = CGLayerGetSize( mxLayer );
        if( (nDX == aSize.width) && (nDY == aSize.height) )
        {
            // Yay, we do not have to do anything :)
            return true;
        }
    }

    Destroy();

    // create a Quartz layer matching to the intended virdev usage
    CGContextRef xCGContext = NULL;
    if( mnBitmapDepth && (mnBitmapDepth < 16) )
    {
        mnBitmapDepth = 8;  // TODO: are 1bit vdevs worth it?
        const CGColorSpaceRef aCGColorSpace = GetSalData()->mxGraySpace;
        const CGBitmapInfo aCGBmpInfo = kCGImageAlphaNone;
        const int nBytesPerRow = (mnBitmapDepth * nDX + 7) / 8;

        void* pRawData = rtl_allocateMemory( nBytesPerRow * nDY );
        mxBitmapContext = CGBitmapContextCreate( pRawData, nDX, nDY,
                                                 mnBitmapDepth, nBytesPerRow, aCGColorSpace, aCGBmpInfo );
        xCGContext = mxBitmapContext;
    }
    else
    {
        // default to a NSView target context
        AquaSalFrame* pSalFrame = mpGraphics->getGraphicsFrame();
        if( !pSalFrame || !AquaSalFrame::isAlive( pSalFrame ))
        {
            if( !GetSalData()->maFrames.empty() )
            {
                // get the first matching frame
                pSalFrame = *GetSalData()->maFrames.begin();
            }
            else
            {
                // ensure we don't reuse a dead AquaSalFrame on the very
                // unlikely case of no other frame to use
                pSalFrame = NULL;
            }
            // update the frame reference
            mpGraphics->setGraphicsFrame( pSalFrame );
        }
        if( pSalFrame )
        {
            // #i91990#
            NSWindow* pNSWindow = pSalFrame->getNSWindow();
            if ( pNSWindow )
            {
                NSGraphicsContext* pNSContext = [NSGraphicsContext graphicsContextWithWindow: pNSWindow];
                if( pNSContext )
                    xCGContext = reinterpret_cast<CGContextRef>([pNSContext graphicsPort]);
            }
            else
            {
                // fall back to a bitmap context
                mnBitmapDepth = 32;
                const CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
                const CGBitmapInfo aCGBmpInfo = kCGImageAlphaNoneSkipFirst;
                const int nBytesPerRow = (mnBitmapDepth * nDX) / 8;

                void* pRawData = rtl_allocateMemory( nBytesPerRow * nDY );
                mxBitmapContext = CGBitmapContextCreate( pRawData, nDX, nDY,
                                                         8, nBytesPerRow, aCGColorSpace, aCGBmpInfo );
                xCGContext = mxBitmapContext;
            }
        }
    }

    DBG_ASSERT( xCGContext, "no context" );

    const CGSize aNewSize = { static_cast<CGFloat>(nDX), static_cast<CGFloat>(nDY) };
    mxLayer = CGLayerCreateWithContext( xCGContext, aNewSize, NULL );

    if( mxLayer && mpGraphics )
    {
        // get the matching Quartz context
        CGContextRef xDrawContext = CGLayerGetContext( mxLayer );
        mpGraphics->SetVirDevGraphics( mxLayer, xDrawContext, mnBitmapDepth );
    }

    return (mxLayer != NULL);
#endif
}



void AquaSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
    if( mxLayer )
    {
        const CGSize aSize = CGLayerGetSize( mxLayer );
        rWidth = static_cast<long>(aSize.width);
        rHeight = static_cast<long>(aSize.height);
    }
    else
    {
        rWidth = 0;
        rHeight = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
