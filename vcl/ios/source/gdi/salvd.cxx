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

#include "ios/salvd.h"
#include "ios/salinst.h"
#include "coretext/salgdi.h"
#include "ios/saldata.hxx"
#include "ios/salframe.h"

// -----------------------------------------------------------------------

SalVirtualDevice* IosSalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
    long nDX, long nDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData )
{
    // #i92075# can be called first in a thread
    SalData::ensureThreadAutoreleasePool();

    return new IosSalVirtualDevice( static_cast< QuartzSalGraphics* >( pGraphics ), nDX, nDY, nBitCount, pData );
}

// -----------------------------------------------------------------------

void IosSalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
    delete pDevice;
}

// =======================================================================

IosSalVirtualDevice::IosSalVirtualDevice( QuartzSalGraphics* pGraphic, long nDX, long nDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData )
:   mbGraphicsUsed( false )
,   mxBitmapContext( NULL )
,   mnBitmapDepth( 0 )
,   mxLayer( NULL )
{
    if( pGraphic && pData && pData->rCGContext )
    {
        // Create virtual device based on existing SystemGraphicsData
        // We ignore nDx and nDY, as the desired size comes from the SystemGraphicsData
        mbForeignContext = true;        // the mxContext is from pData
        mpGraphics = new QuartzSalGraphics( /*pGraphic*/ );
        mpGraphics->SetVirDevGraphics( mxLayer, pData->rCGContext );
    }
    else
    {
        // create empty new virtual device
        mbForeignContext = false;           // the mxContext is created within VCL
        mpGraphics = new QuartzSalGraphics(); // never fails
        mnBitmapDepth = nBitCount;

        // inherit resolution from reference device
        if( pGraphic )
        {
            IosSalFrame* pFrame = pGraphic->getGraphicsFrame();
            if( pFrame && IosSalFrame::isAlive( pFrame ) )
            {
                mpGraphics->setGraphicsFrame( pFrame );
                mpGraphics->copyResolution( *pGraphic );
            }
        }

        if( nDX && nDY )
            SetSize( nDX, nDY );

        // NOTE: if SetSize does not succeed, we just ignore the nDX and nDY
    }
}

// -----------------------------------------------------------------------

IosSalVirtualDevice::~IosSalVirtualDevice()
{
    if( mpGraphics )
    {
        mpGraphics->SetVirDevGraphics( NULL, NULL );
        delete mpGraphics;
        mpGraphics = 0;
    }
    Destroy();
}

// -----------------------------------------------------------------------

void IosSalVirtualDevice::Destroy()
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

// -----------------------------------------------------------------------

SalGraphics* IosSalVirtualDevice::GetGraphics()
{
    if( mbGraphicsUsed || !mpGraphics )
        return 0;

    mbGraphicsUsed = true;
    return mpGraphics;
}

// -----------------------------------------------------------------------

void IosSalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    mbGraphicsUsed = false;
}

// -----------------------------------------------------------------------

sal_Bool IosSalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( mbForeignContext )
    {
        // Do not delete/resize mxContext that we have received from outside VCL
        return true;
    }

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
        mxBitmapContext = ::CGBitmapContextCreate( pRawData, nDX, nDY,
            mnBitmapDepth, nBytesPerRow, aCGColorSpace, aCGBmpInfo );
        xCGContext = mxBitmapContext;
    }
    else
    {
        // default to a UIView target context
        IosSalFrame* pSalFrame = mpGraphics->getGraphicsFrame();
        if( !pSalFrame && !GetSalData()->maFrames.empty() )
            pSalFrame = *GetSalData()->maFrames.begin();
        if( pSalFrame )
        {
#if 0 // No idea...
            // #i91990#
            UIWindow* pWindow = pSalFrame->getWindow();
            if ( pWindow )
            {
                UIGraphicsContext* pUIContext = [UIGraphicsContext graphicsContextWithWindow: pWindow];
                if( pUIContext )
                    xCGContext = reinterpret_cast<CGContextRef>([pUIContext graphicsPort]);
            }
            else
#endif
            {
                // fall back to a bitmap context
                mnBitmapDepth = 32;
                const CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
                const CGBitmapInfo aCGBmpInfo = kCGImageAlphaNoneSkipFirst;
                const int nBytesPerRow = (mnBitmapDepth * nDX) / 8;

                void* pRawData = rtl_allocateMemory( nBytesPerRow * nDY );
                mxBitmapContext = ::CGBitmapContextCreate( pRawData, nDX, nDY,
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
}

// -----------------------------------------------------------------------

void IosSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
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
