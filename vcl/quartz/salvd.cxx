/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
        
        
        
        mbForeignContext = true;        
        mpGraphics = new AquaSalGraphics( /*pGraphic*/ );
#ifdef IOS
        
        
        
        
        
        
        
        
        
        
        
#endif
        mpGraphics->SetVirDevGraphics( mxLayer, pData->rCGContext );
    }
    else
    {
        
        mbForeignContext = false;           
        mpGraphics = new AquaSalGraphics(); 
        mnBitmapDepth = nBitCount;
#ifdef MACOSX
        
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
            
            return true;
        }
    }

    Destroy();

    
    CGContextRef xCGContext = NULL;
    if( mnBitmapDepth && (mnBitmapDepth < 16) )
    {
        mnBitmapDepth = 8;  
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
        
        AquaSalFrame* pSalFrame = mpGraphics->getGraphicsFrame();
        if( !pSalFrame || !AquaSalFrame::isAlive( pSalFrame ))
        {
            if( !GetSalData()->maFrames.empty() )
            {
                
                pSalFrame = *GetSalData()->maFrames.begin();
            }
            else
            {
                
                
                pSalFrame = NULL;
            }
            
            mpGraphics->setGraphicsFrame( pSalFrame );
        }
        if( pSalFrame )
        {
            
            NSWindow* pNSWindow = pSalFrame->getNSWindow();
            if ( pNSWindow )
            {
                NSGraphicsContext* pNSContext = [NSGraphicsContext graphicsContextWithWindow: pNSWindow];
                if( pNSContext )
                    xCGContext = reinterpret_cast<CGContextRef>([pNSContext graphicsPort]);
            }
            else
            {
                
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
