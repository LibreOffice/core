/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:01:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "salvd.h"
#include "salinst.h"
#include "salgdi.h"
#include "saldata.hxx"

#include "vcl/sysdata.hxx"

// -----------------------------------------------------------------------

SalVirtualDevice* AquaSalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
    long nDX, long nDY, USHORT nBitCount, const SystemGraphicsData *pData )
{
    return new AquaSalVirtualDevice( static_cast< AquaSalGraphics* >( pGraphics ), nDX, nDY, nBitCount, pData );
}

// -----------------------------------------------------------------------

void AquaSalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
    delete pDevice;
}

// =======================================================================

AquaSalVirtualDevice::AquaSalVirtualDevice( AquaSalGraphics* pGraphic, long nDX, long nDY, USHORT nBitCount, const SystemGraphicsData *pData )
: mbGraphicsUsed( false ),
  mxContext( 0 )
{
    if( pGraphic && pData && pData->rCGContext)
    {
        // Create virtual device based on existing SystemGraphicsData
        // We ignore nDx and nDY, as the desired size comes from the SystemGraphicsData
        mxContext = pData->rCGContext;
        mbForeignContext = true;        // the mxContext is from pData
        mpGraphics = pGraphic;
        mpGraphics->SetVirDevGraphics( mxContext, true );
    }
    else
    {
        // create empty new virtual device
        mbForeignContext = false;           // the mxContext is created within VCL
        mpGraphics = new AquaSalGraphics(); // never fails

        if( nDX && nDY )
            SetSize( nDX, nDY );

        // NOTE: if SetSize does not succeed, we just ignore the nDX and nDY
    }
}

// -----------------------------------------------------------------------

AquaSalVirtualDevice::~AquaSalVirtualDevice()
{
    if( mpGraphics )
    {
        mpGraphics->SetVirDevGraphics( 0, true );
        delete mpGraphics;
        mpGraphics = 0;
    }
    Destroy();
}

// -----------------------------------------------------------------------

void AquaSalVirtualDevice::Destroy()
{
    if( mbForeignContext ) {
        // Do not delete mxContext that we have received from outside VCL
        return;
    }

    if( mxContext )
    {
        if( mpGraphics )
            mpGraphics->SetVirDevGraphics( 0, true );
        void* pBuffer = CGBitmapContextGetData(mxContext);
        CFRelease( mxContext );
        mxContext = 0;
        if( pBuffer )
            free( pBuffer );
    }
}
// -----------------------------------------------------------------------

SalGraphics* AquaSalVirtualDevice::GetGraphics()
{
    if( mbGraphicsUsed || !mpGraphics )
        return 0;

    mbGraphicsUsed = true;
    return mpGraphics;
}

// -----------------------------------------------------------------------

void AquaSalVirtualDevice::ReleaseGraphics( SalGraphics *pGraphics )
{
    mbGraphicsUsed = false;
}

// -----------------------------------------------------------------------

BOOL AquaSalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( mbForeignContext )
    {
        // Do not delete/resize mxContext that we have received from outside VCL
        return true;
    }

    if( mxContext &&
        ( nDX == static_cast<long> (CGBitmapContextGetWidth( mxContext ) ) ) &&
        ( nDY == static_cast<long> (CGBitmapContextGetHeight( mxContext ) ) ) )
    {
        // Yay, we do not have to do anything :)
        return true;
    }

    Destroy();

    void* pData = malloc( nDX * 4 * nDY );
    if (pData)
    {
        mxContext = CGBitmapContextCreate( pData, nDX, nDY, 8, nDX * 4, GetSalData()->mxRGBSpace, kCGImageAlphaNoneSkipFirst );

        if( mxContext )
        {
            if( mpGraphics )
                mpGraphics->SetVirDevGraphics( mxContext, true );
        }
        else
        {
            free (pData);
            DBG_ERROR( "vcl::AquaSalVirtualDevice::SetSize(), could not create Bitmap Context" );
        }
    }
    else
    {
        DBG_ERROR( "vcl::AquaSalVirtualDevice::SetSize(), could not allocate bitmap data" );
    }

    return mxContext != 0;
}

// -----------------------------------------------------------------------

void AquaSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
    if( mxContext )
    {
        rWidth = CGBitmapContextGetWidth( mxContext );
        rHeight = CGBitmapContextGetHeight( mxContext );
    }
    else
    {
        rWidth = 0;
        rHeight = 0;
    }
}
