/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <svsys.h>
#include <vcl/salinst.hxx>
#include <vcl/salgdi.hxx>
#include <vcl/salframe.hxx>
#include <vcl/salvd.hxx>
#include <tools/debug.hxx>
#include <vcl/svdata.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/outdev.h>
#include <vcl/virdev.hxx>

using namespace ::com::sun::star::uno;

// =======================================================================

void VirtualDevice::ImplInitVirDev( const OutputDevice* pOutDev,
                                    long nDX, long nDY, USHORT nBitCount, const SystemGraphicsData *pData )
{
    DBG_ASSERT( nBitCount <= 1,
                "VirtualDevice::VirtualDevice(): Only 0 or 1 is for BitCount allowed" );

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
        // do not abort but throw an exception, may be the current thread terminates anyway (plugin-scenario)
        throw ::com::sun::star::uno::RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Could not create system bitmap!" ) ),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );
        //GetpApp()->Exception( EXC_SYSOBJNOTCREATED );
    }

    mnBitCount      = ( nBitCount ? nBitCount : pOutDev->GetBitCount() );
    mnOutWidth      = nDX;
    mnOutHeight     = nDY;
    mbScreenComp    = TRUE;
    mnAlphaDepth    = -1;

    // #i59315# init vdev size from system object, when passed a
    // SystemGraphicsData. Otherwise, output size will always
    // incorrectly stay at (1,1)
    if( pData && mpVirDev )
        mpVirDev->GetSize(mnOutWidth,mnOutHeight);

    if( mnBitCount < 8 )
        SetAntialiasing( ANTIALIASING_DISABLE_TEXT );

    if ( pOutDev->GetOutDevType() == OUTDEV_PRINTER )
        mbScreenComp = FALSE;
    else if ( pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        mbScreenComp = ((VirtualDevice*)pOutDev)->mbScreenComp;

    meOutDevType    = OUTDEV_VIRDEV;
    mbDevOutput     = TRUE;
    mpFontList      = pSVData->maGDIData.mpScreenFontList;
    mpFontCache     = pSVData->maGDIData.mpScreenFontCache;
    mnDPIX          = pOutDev->mnDPIX;
    mnDPIY          = pOutDev->mnDPIY;
    maFont          = pOutDev->maFont;

    if( maTextColor != pOutDev->maTextColor )
    {
        maTextColor = pOutDev->maTextColor;
        mbInitTextColor = true;
    }

    // Virtuelle Devices haben defaultmaessig einen weissen Hintergrund
    SetBackground( Wallpaper( Color( COL_WHITE ) ) );

    // #i59283# don't erase user-provided surface
    if( !pData )
        Erase();

    // VirDev in Liste eintragen
    mpNext = pSVData->maGDIData.mpFirstVirDev;
    mpPrev = NULL;
    if ( mpNext )
        mpNext->mpPrev = this;
    else
        pSVData->maGDIData.mpLastVirDev = this;
    pSVData->maGDIData.mpFirstVirDev = this;
}

// -----------------------------------------------------------------------

VirtualDevice::VirtualDevice( USHORT nBitCount )
:   mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    OSL_TRACE( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( Application::GetDefaultDevice(), 1, 1, nBitCount );
}

// -----------------------------------------------------------------------

VirtualDevice::VirtualDevice( const OutputDevice& rCompDev, USHORT nBitCount )
    : mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    OSL_TRACE( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( &rCompDev, 1, 1, nBitCount );
}

// -----------------------------------------------------------------------

VirtualDevice::VirtualDevice( const OutputDevice& rCompDev, USHORT nBitCount, USHORT nAlphaBitCount )
    : mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    OSL_TRACE( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( &rCompDev, 1, 1, nBitCount );

    // #110958# Enable alpha channel
    mnAlphaDepth = sal::static_int_cast<sal_Int8>(nAlphaBitCount);
}

// -----------------------------------------------------------------------

VirtualDevice::VirtualDevice( const SystemGraphicsData *pData, USHORT nBitCount )
:   mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    OSL_TRACE( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( Application::GetDefaultDevice(), 1, 1, nBitCount, pData );
}

// -----------------------------------------------------------------------

VirtualDevice::~VirtualDevice()
{
    DBG_TRACE( "VirtualDevice::~VirtualDevice()" );

    ImplSVData* pSVData = ImplGetSVData();

    ImplReleaseGraphics();

    if ( mpVirDev )
        pSVData->mpDefInst->DestroyVirtualDevice( mpVirDev );

    // remove this VirtualDevice from the double-linked global list
    if( mpPrev )
        mpPrev->mpNext = mpNext;
    else
        pSVData->maGDIData.mpFirstVirDev = mpNext;

    if( mpNext )
        mpNext->mpPrev = mpPrev;
    else
        pSVData->maGDIData.mpLastVirDev = mpPrev;
}

// -----------------------------------------------------------------------

BOOL VirtualDevice::ImplSetOutputSizePixel( const Size& rNewSize, BOOL bErase )
{
    OSL_TRACE( "VirtualDevice::ImplSetOutputSizePixel( %ld, %ld, %d )", rNewSize.Width(), rNewSize.Height(), (int)bErase );

    if ( !mpVirDev )
        return FALSE;
    else if ( rNewSize == GetOutputSizePixel() )
    {
        if ( bErase )
            Erase();
        return TRUE;
    }

    BOOL bRet;
    long nNewWidth = rNewSize.Width(), nNewHeight = rNewSize.Height();

    if ( nNewWidth < 1 )
        nNewWidth = 1;

    if ( nNewHeight < 1 )
        nNewHeight = 1;

    if ( bErase )
    {
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

        // we need a graphics
        if ( !mpGraphics )
        {
            if ( !ImplGetGraphics() )
                return FALSE;
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

                pGraphics->CopyBits( &aPosAry, mpGraphics, this, this );
                pNewVirDev->ReleaseGraphics( pGraphics );
                ImplReleaseGraphics();
                pSVData->mpDefInst->DestroyVirtualDevice( mpVirDev );
                mpVirDev = pNewVirDev;
                mnOutWidth  = rNewSize.Width();
                mnOutHeight = rNewSize.Height();
                bRet = TRUE;
            }
            else
            {
                bRet = FALSE;
                pSVData->mpDefInst->DestroyVirtualDevice( pNewVirDev );
            }
        }
        else
            bRet = FALSE;
    }

    return bRet;
}

// -----------------------------------------------------------------------

// #i32109#: Fill opaque areas correctly (without relying on
// fill/linecolor state)
void VirtualDevice::ImplFillOpaqueRectangle( const Rectangle& rRect )
{
    // Set line and fill color to black (->opaque),
    // fill rect with that (linecolor, too, because of
    // those pesky missing pixel problems)
    Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
    SetLineColor( COL_BLACK );
    SetFillColor( COL_BLACK );
    DrawRect( rRect );
    Pop();
}

// -----------------------------------------------------------------------

BOOL VirtualDevice::SetOutputSizePixel( const Size& rNewSize, BOOL bErase )
{
    if( ImplSetOutputSizePixel(rNewSize, bErase) )
    {
        if( mnAlphaDepth != -1 )
        {
            // #110958# Setup alpha bitmap
            if(mpAlphaVDev && mpAlphaVDev->GetOutputSizePixel() != rNewSize)
            {
                delete mpAlphaVDev;
                mpAlphaVDev = 0L;
            }

            if( !mpAlphaVDev )
            {
                mpAlphaVDev = new VirtualDevice( *this, mnAlphaDepth );
                mpAlphaVDev->ImplSetOutputSizePixel(rNewSize, bErase);
            }

            // TODO: copy full outdev state to new one, here. Also needed in outdev2.cxx:DrawOutDev
            if( GetLineColor() != Color( COL_TRANSPARENT ) )
                mpAlphaVDev->SetLineColor( COL_BLACK );

            if( GetFillColor() != Color( COL_TRANSPARENT ) )
                mpAlphaVDev->SetFillColor( COL_BLACK );

            mpAlphaVDev->SetMapMode( GetMapMode() );
        }

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void VirtualDevice::SetReferenceDevice( RefDevMode i_eRefDevMode )
{
    sal_Int32 nDPIX = 600, nDPIY = 600;
    switch( i_eRefDevMode )
    {
    case REFDEV_NONE:
    default:
        DBG_ASSERT( FALSE, "VDev::SetRefDev illegal argument!" );
        break;
    case REFDEV_MODE06:
        nDPIX = nDPIY = 600;
        break;
    case REFDEV_MODE48:
        nDPIX = nDPIY = 4800;
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

    EnableOutput( FALSE );  // prevent output on reference device
    mbScreenComp = FALSE;

    // invalidate currently selected fonts
    mbInitFont = TRUE;
    mbNewFont = TRUE;

    // avoid adjusting font lists when already in refdev mode
    BYTE nOldRefDevMode = meRefDevMode;
    BYTE nOldCompatFlag = (BYTE)meRefDevMode & REFDEV_FORCE_ZERO_EXTLEAD;
    meRefDevMode = (BYTE)(i_eRefDevMode | nOldCompatFlag);
    if( (nOldRefDevMode ^ nOldCompatFlag) != REFDEV_NONE )
        return;

    // the reference device should have only scalable fonts
    // => clean up the original font lists before getting new ones
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

    // preserve global font lists
    ImplSVData* pSVData = ImplGetSVData();
    if( mpFontList && (mpFontList != pSVData->maGDIData.mpScreenFontList) )
        delete mpFontList;
    if( mpFontCache && (mpFontCache != pSVData->maGDIData.mpScreenFontCache) )
        delete mpFontCache;

    // get font list with scalable fonts only
    ImplGetGraphics();
    mpFontList = pSVData->maGDIData.mpScreenFontList->Clone( true, false );

    // prepare to use new font lists
    mpFontCache = new ImplFontCache( false );
}

// -----------------------------------------------------------------------

void VirtualDevice::Compat_ZeroExtleadBug()
{
    meRefDevMode = (BYTE)meRefDevMode | REFDEV_FORCE_ZERO_EXTLEAD;
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
