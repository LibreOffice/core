/*************************************************************************
 *
 *  $RCSfile: virdev.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cd $ $Date: 2000-11-17 13:30:08 $
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

#define _SV_VIRDEV_CXX

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif

#ifndef REMOTE_APPSERVER
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

// appserver
#ifdef REMOTE_APPSERVER
#ifndef _SV_RMOUTDEV_HXX
#include <rmoutdev.hxx>
#endif
#ifndef _SV_RMVIRDEV_HXX
#include <rmvirdev.hxx>
#endif
#ifndef _VCL_RMCACHE_HXX_
#include <rmcache.hxx>
#endif
#endif

// =======================================================================

// interface cache
#ifdef REMOTE_APPSERVER

static ::vcl::InterfacePairCache< ::com::sun::star::portal::client::XRmVirtualDevice, ::com::sun::star::portal::client::XRmOutputDevice >* pRemoteVirdevCache = NULL;

typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::portal::client::XRmVirtualDevice >, ::com::sun::star::uno::Reference< ::com::sun::star::portal::client::XRmOutputDevice > > virdevInterfacePair;


void createRemoteVirdevCache( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aInterfaceSeq )
{
    if( ! pRemoteVirdevCache )
    {
        ImplSVData* pSVData = ImplGetSVData();
        pRemoteVirdevCache = new ::vcl::InterfacePairCache< ::com::sun::star::portal::client::XRmVirtualDevice, ::com::sun::star::portal::client::XRmOutputDevice >(
            pSVData->mxMultiFactory,
            aInterfaceSeq,
            ::rtl::OUString::createFromAscii( "OfficeVirtualDevice.stardiv.de" ), 10, 40 );
    }
}

void eraseRemoteVirdevCache()
{
    if( pRemoteVirdevCache )
    {
        delete pRemoteVirdevCache;
        pRemoteVirdevCache = NULL;
    }
}

#endif

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

#ifndef REMOTE_APPSERVER
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
#else

    if( pOutDev->GetOutDevType() == OUTDEV_PRINTER || ! mpVirDev )
    {
        virdevInterfacePair aPair = pRemoteVirdevCache->takeInterface();
        if( aPair.first.is() && aPair.second.is() )
        {
            if( ! mpVirDev )
                mpVirDev = new RmVirtualDevice;
            mpVirDev->SetInterface( aPair.first );
            mpVirDev->Create( (ULONG) pOutDev, nDX, nDY, nBitCount );

            if( ! mpGraphics )
                mpGraphics = new ImplServerGraphics();
            mpGraphics->SetInterface( aPair.second );
        }
    }
    else
    {
        // this was done in ImpGetServerGraphics before
        // and is now here because of interface caching
        if( mpGraphics && mpGraphics->GetInterface().is() )
        {
            try
            {
                CHECK_FOR_RVPSYNC_NORMAL();
                mpGraphics->GetInterface()->SetFillColor( mpGraphics->maFillColor.GetColor() );
            }
            catch (...)
            {
                if( mpGraphics )
                    delete mpGraphics, mpGraphics = NULL;

                if( mpVirDev )
                {
                    virdevInterfacePair aPair = pRemoteVirdevCache->takeInterface();
                    if( aPair.first.is() && aPair.second.is() )
                    {
                        mpVirDev->SetInterface( aPair.first );
                        mpVirDev->Create( (ULONG)NULL, mnOutWidth, mnOutHeight, mnBitCount );
                        mpGraphics = new ImplServerGraphics();
                        mpGraphics->SetInterface( aPair.second );
                    }
                    else
                        mpVirDev->SetInterface( REF( NMSP_CLIENT::XRmVirtualDevice )() );
                }
            }
        }
        ImplGetServerGraphics( TRUE );
    }
#endif

    mnBitCount      = ( nBitCount ? nBitCount : pOutDev->GetBitCount() );
    mnOutWidth      = nDX;
    mnOutHeight     = nDY;
    mbScreenComp    = TRUE;

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
    : mpVirDev( NULL )
{
    DBG_TRACE1( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( Application::GetDefaultDevice(), 1, 1, nBitCount );
}

// -----------------------------------------------------------------------

VirtualDevice::VirtualDevice( const OutputDevice& rCompDev, USHORT nBitCount )
    : mpVirDev( NULL )
{
    DBG_TRACE1( "VirtualDevice::VirtualDevice( %hu )", nBitCount );

    ImplInitVirDev( &rCompDev, 1, 1, nBitCount );
}

// -----------------------------------------------------------------------

VirtualDevice::~VirtualDevice()
{
    DBG_TRACE( "VirtualDevice::~VirtualDevice()" );

   ImplSVData* pSVData = ImplGetSVData();

#ifndef REMOTE_APPSERVER
    ImplReleaseGraphics();

    if ( mpVirDev )
        pSVData->mpDefInst->DestroyVirtualDevice( mpVirDev );
#else
    if ( pRemoteVirdevCache && mpVirDev && mpGraphics )
    {
        virdevInterfacePair aPair( mpVirDev->GetInterface(), mpGraphics->GetInterface() );
        CHECK_FOR_RVPSYNC_NORMAL();
        aPair.first->Create( 0, 0, 0, 0 );
        pRemoteVirdevCache->putInterface( aPair );
    }

    mpGraphics->SetInterface( REF( NMSP_CLIENT::XRmOutputDevice )() );
    ImplReleaseServerGraphics();
    delete mpVirDev;
    delete mpGraphics;
#endif

    // VirDev aus der Liste eintragen
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

BOOL VirtualDevice::SetOutputSizePixel( const Size& rNewSize, BOOL bErase )
{
    DBG_TRACE3( "VirtualDevice::SetOutputSizePixel( %ld, %ld, %d )", rNewSize.Width(), rNewSize.Height(), (int)bErase );

    if ( !mpVirDev )
        return FALSE;
    else if ( rNewSize == GetOutputSizePixel() )
    {
        if ( bErase )
            Erase();
        return TRUE;
    }

#ifdef REMOTE_APPSERVER
    long nOldWidth = mnOutWidth, nOldHeight = mnOutHeight;

    try
    {
        mnOutWidth  = rNewSize.Width();
        mnOutHeight = rNewSize.Height();
        mpVirDev->ResizeOutputSizePixel( mnOutWidth, mnOutHeight );
    }
    catch (...)
    {
        delete mpVirDev, mpVirDev = NULL;
        ImplInitVirDev( NULL, mnOutWidth, mnOutHeight, mnBitCount );
    }

    if( bErase )
        Erase();
    else
    {
        if ( nOldWidth < mnOutWidth )
            Erase( Rectangle( Point( nOldWidth, 0 ), Size( mnOutWidth-nOldWidth, Max( nOldHeight, mnOutHeight ) ) ) );
        if ( nOldHeight< mnOutHeight )
            Erase( Rectangle( Point( 0, nOldHeight ), Size( Max( nOldWidth, mnOutWidth ), mnOutHeight-nOldHeight ) ) );
    }

    return TRUE;
#else
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

                pGraphics->CopyBits( &aPosAry, mpGraphics );
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
#endif
}

