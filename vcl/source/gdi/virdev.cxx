/*************************************************************************
 *
 *  $RCSfile: virdev.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:29:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif

using namespace ::com::sun::star::uno;

// =======================================================================

void VirtualDevice::ImplInitVirDev( const OutputDevice* pOutDev,
                                    long nDX, long nDY, USHORT nBitCount )
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

    SalGraphics* pGraphics;
    if ( !pOutDev->mpGraphics )
        ((OutputDevice*)pOutDev)->ImplGetGraphics();
    pGraphics = pOutDev->mpGraphics;
    if ( pGraphics )
        mpVirDev = pSVData->mpDefInst->CreateVirtualDevice( pGraphics, nDX, nDY, nBitCount );
    else
        mpVirDev = NULL;
    if ( !mpVirDev )
        GetpApp()->Exception( EXC_SYSOBJNOTCREATED );

    mnBitCount      = ( nBitCount ? nBitCount : pOutDev->GetBitCount() );
    mnOutWidth      = nDX;
    mnOutHeight     = nDY;
    mbScreenComp    = TRUE;
    mbScreenComp    = FALSE;
    mnAlphaDepth    = -1;

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

    // Virtuelle Devices haben defaultmaessig einen weissen Hintergrund
    SetBackground( Wallpaper( Color( COL_WHITE ) ) );
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
    DBG_TRACE1( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( Application::GetDefaultDevice(), 1, 1, nBitCount );
}

// -----------------------------------------------------------------------

VirtualDevice::VirtualDevice( const OutputDevice& rCompDev, USHORT nBitCount )
    : mpVirDev( NULL ),
    meRefDevMode( REFDEV_NONE )
{
    DBG_TRACE1( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( &rCompDev, 1, 1, nBitCount );
}

// -----------------------------------------------------------------------

VirtualDevice::VirtualDevice( const OutputDevice& rCompDev, USHORT nBitCount, USHORT nAlphaBitCount )
    : mpVirDev( NULL )
{
    DBG_TRACE1( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( &rCompDev, 1, 1, nBitCount );

    // #110958# Enable alpha channel
    mnAlphaDepth = nAlphaBitCount;
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
    DBG_TRACE3( "VirtualDevice::ImplSetOutputSizePixel( %ld, %ld, %d )", rNewSize.Width(), rNewSize.Height(), (int)bErase );

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

void VirtualDevice::SetReferenceDevice( RefDevMode eRefDevMode )
{
    switch( eRefDevMode )
    {
    case REFDEV_NONE:
    default:
        DBG_ASSERT( FALSE, "VDev::SetRefDev illegal argument!" );
        // fall through
    case REFDEV_MODE06:
        mnDPIX = mnDPIY = 600;
        break;
    case REFDEV_MODE48:
        mnDPIX = mnDPIY = 4800;
        break;
    case REFDEV_MODE96:
        mnDPIX = mnDPIY = 1440; // as agreed with FME and AMA
        break;
    }

    EnableOutput( FALSE );  // prevent output on reference device
    mbScreenComp = FALSE;

    // invalidate currently selected fonts
    mbInitFont = TRUE;
    mbNewFont = TRUE;

    // avoid adjusting font lists when already in refdev mode
    BYTE nOldRefDevMode = meRefDevMode;
    meRefDevMode = (BYTE)eRefDevMode;
    if( nOldRefDevMode != REFDEV_NONE )
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
