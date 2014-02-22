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


#include <tools/debug.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>

#include <salinst.hxx>
#include <salgdi.hxx>
#include <salframe.hxx>
#include <salvd.hxx>
#include <outdev.h>
#include <svdata.hxx>

using namespace ::com::sun::star::uno;



void VirtualDevice::ImplInitVirDev( const OutputDevice* pOutDev,
                                    long nDX, long nDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData )
{
    SAL_INFO( "vcl.virdev", "ImplInitVirDev(" << nDX << "," << nDY << "," << nBitCount << ")" );

    if ( nDX < 1 )
        nDX = 1;

    if ( nDY < 1 )
        nDY = 1;

    ImplSVData* pSVData = ImplGetSVData();

    if ( !pOutDev )
        pOutDev = ImplGetDefaultWindow();
    if( !pOutDev )
        return;

    SalGraphics* pGraphics;
    if ( !pOutDev->mpGraphics )
        ((OutputDevice*)pOutDev)->ImplGetGraphics();
    pGraphics = pOutDev->mpGraphics;
    if ( pGraphics )
        mpVirDev = pSVData->mpDefInst->CreateVirtualDevice( pGraphics, nDX, nDY, nBitCount, pData );
    else
        mpVirDev = NULL;
    if ( !mpVirDev )
    {
        
        throw ::com::sun::star::uno::RuntimeException(
            OUString( "Could not create system bitmap!" ),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );
    }

    mnBitCount      = ( nBitCount ? nBitCount : pOutDev->GetBitCount() );
    mnOutWidth      = nDX;
    mnOutHeight     = nDY;
    mbScreenComp    = true;
    mnAlphaDepth    = -1;

    
    
    
    if( pData && mpVirDev )
        mpVirDev->GetSize(mnOutWidth,mnOutHeight);

    if( mnBitCount < 8 )
        SetAntialiasing( ANTIALIASING_DISABLE_TEXT );

    if ( pOutDev->GetOutDevType() == OUTDEV_PRINTER )
        mbScreenComp = false;
    else if ( pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        mbScreenComp = ((VirtualDevice*)pOutDev)->mbScreenComp;

    meOutDevType    = OUTDEV_VIRDEV;
    mbDevOutput     = true;
    mpFontList      = pSVData->maGDIData.mpScreenFontList;
    mpFontCache     = pSVData->maGDIData.mpScreenFontCache;
    mnDPIX          = pOutDev->mnDPIX;
    mnDPIY          = pOutDev->mnDPIY;
    mnDPIScaleFactor = pOutDev->mnDPIScaleFactor;
    maFont          = pOutDev->maFont;

    if( maTextColor != pOutDev->maTextColor )
    {
        maTextColor = pOutDev->maTextColor;
        mbInitTextColor = true;
    }

    
    SetBackground( Wallpaper( Color( COL_WHITE ) ) );

    
    if( !pData )
        Erase();

    
    mpNext = pSVData->maGDIData.mpFirstVirDev;
    mpPrev = NULL;
    if ( mpNext )
        mpNext->mpPrev = this;
    else
        pSVData->maGDIData.mpLastVirDev = this;
    pSVData->maGDIData.mpFirstVirDev = this;
}



VirtualDevice::VirtualDevice( sal_uInt16 nBitCount )
:   mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    SAL_WARN_IF( (nBitCount > 1 && nBitCount != 8), "vcl.gdi",
                "VirtualDevice::VirtualDevice(): Only 0, 1 or 8 allowed for BitCount" );
    SAL_INFO( "vcl.gdi", "VirtualDevice::VirtualDevice( " << nBitCount << " )" );

    ImplInitVirDev( Application::GetDefaultDevice(), 1, 1, nBitCount );
}



VirtualDevice::VirtualDevice( const OutputDevice& rCompDev, sal_uInt16 nBitCount )
    : mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    SAL_WARN_IF( nBitCount > 1, "vcl.gdi",
                "VirtualDevice::VirtualDevice(): Only 0 or 1 is for BitCount allowed" );
    SAL_INFO( "vcl.gdi", "VirtualDevice::VirtualDevice( " << nBitCount << " )" );

    ImplInitVirDev( &rCompDev, 1, 1, nBitCount );
}



VirtualDevice::VirtualDevice( const OutputDevice& rCompDev, sal_uInt16 nBitCount, sal_uInt16 nAlphaBitCount )
    : mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    SAL_WARN_IF( nBitCount > 1, "vcl.gdi",
                "VirtualDevice::VirtualDevice(): Only 0 or 1 is for BitCount allowed" );
    SAL_INFO( "vcl.gdi",
            "VirtualDevice::VirtualDevice( " << nBitCount << ", " << nAlphaBitCount << " )" );

    ImplInitVirDev( &rCompDev, 1, 1, nBitCount );

    
    mnAlphaDepth = sal::static_int_cast<sal_Int8>(nAlphaBitCount);
}



VirtualDevice::VirtualDevice( const SystemGraphicsData *pData, sal_uInt16 nBitCount )
:   mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    SAL_INFO( "vcl.gdi", "VirtualDevice::VirtualDevice( " << nBitCount << " )" );

    ImplInitVirDev( Application::GetDefaultDevice(), 1, 1, nBitCount, pData );
}



VirtualDevice::~VirtualDevice()
{
    SAL_INFO( "vcl.gdi", "VirtualDevice::~VirtualDevice()" );

    ImplSVData* pSVData = ImplGetSVData();

    ImplReleaseGraphics();

    if ( mpVirDev )
        pSVData->mpDefInst->DestroyVirtualDevice( mpVirDev );

    
    if( mpPrev )
        mpPrev->mpNext = mpNext;
    else
        pSVData->maGDIData.mpFirstVirDev = mpNext;

    if( mpNext )
        mpNext->mpPrev = mpPrev;
    else
        pSVData->maGDIData.mpLastVirDev = mpPrev;
}



bool VirtualDevice::InnerImplSetOutputSizePixel( const Size& rNewSize, bool bErase, const basebmp::RawMemorySharedArray &pBuffer )
{
    SAL_INFO( "vcl.gdi",
              "VirtualDevice::InnerImplSetOutputSizePixel( " << rNewSize.Width() << ", "
              << rNewSize.Height() << ", " << int(bErase) << " )" );

    if ( !mpVirDev )
        return false;
    else if ( rNewSize == GetOutputSizePixel() )
    {
        if ( bErase )
            Erase();
        
        
        return true;
    }

    bool bRet;
    long nNewWidth = rNewSize.Width(), nNewHeight = rNewSize.Height();

    if ( nNewWidth < 1 )
        nNewWidth = 1;

    if ( nNewHeight < 1 )
        nNewHeight = 1;

    if ( bErase )
    {
        if ( pBuffer )
            bRet = mpVirDev->SetSizeUsingBuffer( nNewWidth, nNewHeight, pBuffer );
        else
            bRet = mpVirDev->SetSize( nNewWidth, nNewHeight );

        if ( bRet )
        {
            mnOutWidth  = rNewSize.Width();
            mnOutHeight = rNewSize.Height();
            Erase();
        }
    }
    else
    {
        SalVirtualDevice*   pNewVirDev;
        ImplSVData*         pSVData = ImplGetSVData();

        
        if ( !mpGraphics )
        {
            if ( !ImplGetGraphics() )
                return false;
        }

        pNewVirDev = pSVData->mpDefInst->CreateVirtualDevice( mpGraphics, nNewWidth, nNewHeight, mnBitCount );
        if ( pNewVirDev )
        {
            SalGraphics* pGraphics = pNewVirDev->GetGraphics();
            if ( pGraphics )
            {
                SalTwoRect aPosAry;
                long nWidth;
                long nHeight;
                if ( mnOutWidth < nNewWidth )
                    nWidth = mnOutWidth;
                else
                    nWidth = nNewWidth;
                if ( mnOutHeight < nNewHeight )
                    nHeight = mnOutHeight;
                else
                    nHeight = nNewHeight;
                aPosAry.mnSrcX       = 0;
                aPosAry.mnSrcY       = 0;
                aPosAry.mnSrcWidth   = nWidth;
                aPosAry.mnSrcHeight  = nHeight;
                aPosAry.mnDestX      = 0;
                aPosAry.mnDestY      = 0;
                aPosAry.mnDestWidth  = nWidth;
                aPosAry.mnDestHeight = nHeight;

                pGraphics->CopyBits( aPosAry, mpGraphics, this, this );
                pNewVirDev->ReleaseGraphics( pGraphics );
                ImplReleaseGraphics();
                pSVData->mpDefInst->DestroyVirtualDevice( mpVirDev );
                mpVirDev = pNewVirDev;
                mnOutWidth  = rNewSize.Width();
                mnOutHeight = rNewSize.Height();
                bRet = true;
            }
            else
            {
                bRet = false;
                pSVData->mpDefInst->DestroyVirtualDevice( pNewVirDev );
            }
        }
        else
            bRet = false;
    }

    return bRet;
}





void VirtualDevice::ImplFillOpaqueRectangle( const Rectangle& rRect )
{
    
    
    
    Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
    SetLineColor( COL_BLACK );
    SetFillColor( COL_BLACK );
    DrawRect( rRect );
    Pop();
}



bool VirtualDevice::ImplSetOutputSizePixel( const Size& rNewSize, bool bErase, const basebmp::RawMemorySharedArray &pBuffer )
{
    if( InnerImplSetOutputSizePixel(rNewSize, bErase, pBuffer) )
    {
        if( mnAlphaDepth != -1 )
        {
            
            if(mpAlphaVDev && mpAlphaVDev->GetOutputSizePixel() != rNewSize)
            {
                delete mpAlphaVDev;
                mpAlphaVDev = 0L;
            }

            if( !mpAlphaVDev )
            {
                mpAlphaVDev = new VirtualDevice( *this, mnAlphaDepth );
                mpAlphaVDev->InnerImplSetOutputSizePixel(rNewSize, bErase, basebmp::RawMemorySharedArray() );
            }

            
            if( GetLineColor() != Color( COL_TRANSPARENT ) )
                mpAlphaVDev->SetLineColor( COL_BLACK );

            if( GetFillColor() != Color( COL_TRANSPARENT ) )
                mpAlphaVDev->SetFillColor( COL_BLACK );

            mpAlphaVDev->SetMapMode( GetMapMode() );
        }

        return true;
    }

    return false;
}

bool VirtualDevice::SetOutputSizePixel( const Size& rNewSize, bool bErase )
{
    return ImplSetOutputSizePixel( rNewSize, bErase, basebmp::RawMemorySharedArray() );
}

bool VirtualDevice::SetOutputSizePixelScaleOffsetAndBuffer( const Size& rNewSize, const Fraction& rScale, const Point& rNewOffset, const basebmp::RawMemorySharedArray &pBuffer )
{
    if (pBuffer) {
        MapMode mm = GetMapMode();
        mm.SetOrigin( rNewOffset );
        mm.SetScaleX( rScale );
        mm.SetScaleY( rScale );
        SetMapMode( mm );
    }
    return ImplSetOutputSizePixel( rNewSize, true, pBuffer);
}



void VirtualDevice::SetReferenceDevice( RefDevMode i_eRefDevMode )
{
    sal_Int32 nDPIX = 600, nDPIY = 600;
    switch( i_eRefDevMode )
    {
    case REFDEV_NONE:
    default:
        DBG_ASSERT( false, "VDev::SetRefDev illegal argument!" );
        break;
    case REFDEV_MODE06:
        nDPIX = nDPIY = 600;
        break;
    case REFDEV_MODE_MSO1:
        nDPIX = nDPIY = 6*1440;
        break;
    case REFDEV_MODE_PDF1:
        nDPIX = nDPIY = 720;
        break;
    }
    ImplSetReferenceDevice( i_eRefDevMode, nDPIX, nDPIY );
}

void VirtualDevice::SetReferenceDevice( sal_Int32 i_nDPIX, sal_Int32 i_nDPIY )
{
    ImplSetReferenceDevice( REFDEV_CUSTOM, i_nDPIX, i_nDPIY );
}

void VirtualDevice::ImplSetReferenceDevice( RefDevMode i_eRefDevMode, sal_Int32 i_nDPIX, sal_Int32 i_nDPIY )
{
    mnDPIX = i_nDPIX;
    mnDPIY = i_nDPIY;
    mnDPIScaleFactor = 1;

    EnableOutput( false );  
    mbScreenComp = false;

    
    mbInitFont = true;
    mbNewFont = true;

    
    sal_uInt8 nOldRefDevMode = meRefDevMode;
    sal_uInt8 nOldCompatFlag = (sal_uInt8)meRefDevMode & REFDEV_FORCE_ZERO_EXTLEAD;
    meRefDevMode = (sal_uInt8)(i_eRefDevMode | nOldCompatFlag);
    if( (nOldRefDevMode ^ nOldCompatFlag) != REFDEV_NONE )
        return;

    
    
    if ( mpFontEntry )
    {
        mpFontCache->Release( mpFontEntry );
        mpFontEntry = NULL;
    }
    if ( mpGetDevFontList )
    {
        delete mpGetDevFontList;
        mpGetDevFontList = NULL;
    }
    if ( mpGetDevSizeList )
    {
        delete mpGetDevSizeList;
        mpGetDevSizeList = NULL;
    }

    
    ImplSVData* pSVData = ImplGetSVData();
    if( mpFontList && (mpFontList != pSVData->maGDIData.mpScreenFontList) )
        delete mpFontList;
    if( mpFontCache && (mpFontCache != pSVData->maGDIData.mpScreenFontCache) )
        delete mpFontCache;

    
    ImplGetGraphics();
    mpFontList = pSVData->maGDIData.mpScreenFontList->Clone( true, false );

    
    mpFontCache = new ImplFontCache();
}



void VirtualDevice::Compat_ZeroExtleadBug()
{
    meRefDevMode = (sal_uInt8)meRefDevMode | REFDEV_FORCE_ZERO_EXTLEAD;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
