/*************************************************************************
 *
 *  $RCSfile: salbmp.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pluby $ $Date: 2001-01-05 21:18:14 $
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

#define _SV_SALBMP_CXX

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

// ==================================================================

SalBitmap::SalBitmap() :
        mpVirDev( 0 ),
        mnBitCount( 0 )
{
}

// ------------------------------------------------------------------

SalBitmap::~SalBitmap()
{
    Destroy();
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
{
    ImplSVData* pSVData = ImplGetSVData();
    BOOL bRet = FALSE;

    // Stub code: we can only handle 32 bit pixel depth's right now
    nBitCount = 32;

    if ( rSize.Width() && rSize.Height() )
    {
        // Create a SalVirtualDevice
        mpVirDev = pSVData->mpDefInst->CreateVirtualDevice( NULL,
            rSize.Width(), rSize.Height(), nBitCount );

        if ( mpVirDev )
        {
            mnBitCount = nBitCount;
            maSize = rSize;
            bRet = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp )
{
    BOOL bRet = FALSE;

    if ( Create( rSalBmp, rSalBmp.mnBitCount ) )
        bRet = TRUE;

    return bRet;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    return Create( rSalBmp );
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
{
    BOOL bRet = FALSE;

    if ( Create( rSalBmp.maSize, nNewBitCount, BitmapPalette() ) )
    {
        // Not yet implemented: Copy pixels from rSalBmp.mpVirDev to mpVirDev

        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------

void SalBitmap::Destroy()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( mpVirDev )
        pSVData->mpDefInst->DestroyVirtualDevice( mpVirDev );

    maSize = Size();
    mnBitCount = 0;
}

// ------------------------------------------------------------------

BitmapBuffer* SalBitmap::AcquireBuffer( BOOL bReadOnly )
{
    BitmapBuffer *pBuffer = NULL;

    if ( mpVirDev )
    {
        // Get the SalGraphics which contains the GWorld we will draw to
        SalGraphics *pGraphics = GetGraphics();

        if ( pGraphics && pGraphics->maGraphicsData.mpCGrafPort )
        {
            PixMapHandle hPixMap = NULL;

            hPixMap = GetPortPixMap( pGraphics->maGraphicsData.mpCGrafPort );

            if ( hPixMap )
            {
                pBuffer = new BitmapBuffer();

                // Lock the GWorld so that the calling functions can write to
                // this buffer
                LockPortBits( pGraphics->maGraphicsData.mpCGrafPort );

                pBuffer->mnBitCount = (**hPixMap).pixelSize;
                pBuffer->mnWidth = mpVirDev->maVirDevData.mnWidth;
                pBuffer->mnHeight = mpVirDev->maVirDevData.mnHeight;
                pBuffer->mnBitCount = (**hPixMap).pixelSize;
                pBuffer->mnScanlineSize = GetPixRowBytes( hPixMap );
                pBuffer->mpBits = (BYTE *)GetPixBaseAddr( hPixMap );
                pBuffer->mnFormat = BMP_FORMAT_TOP_DOWN | BMP_FORMAT_32BIT_TC_MASK;
            }

            // Release the SalGraphics so that others can get a handle to it
            // in future GetGraphics() calls
            ReleaseGraphics( pGraphics );
        }
    }

    return pBuffer;
}

// ------------------------------------------------------------------

void SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
    SalGraphics *pGraphics = NULL;

    if ( mpVirDev )
    {
        // Get the SalGraphics which contains the GWorld we used as the buffer
        SalGraphics *pGraphics = GetGraphics();

        if ( pGraphics && pGraphics->maGraphicsData.mpCGrafPort )
        {
            // Unlock the GWorld so that this GWorld can be reused
            UnlockPortBits( pGraphics->maGraphicsData.mpCGrafPort );

            // Release the SalGraphics so that others can get a handle to it
            // in future GetGraphics() calls
            ReleaseGraphics( pGraphics );
        }
    }

    if ( pBuffer )
        delete pBuffer;
}

// ------------------------------------------------------------------

SalGraphics* SalBitmap::GetGraphics()
{
    if ( mpVirDev )
        return mpVirDev->GetGraphics();
    else
        return NULL;
}

// ------------------------------------------------------------------

void SalBitmap::ReleaseGraphics( SalGraphics* pGraphics )
{
    if ( mpVirDev )
        mpVirDev->ReleaseGraphics( pGraphics );
}

