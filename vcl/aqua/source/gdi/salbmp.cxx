/*************************************************************************
 *
 *  $RCSfile: salbmp.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: bmahbod $ $Date: 2001-01-24 03:39:58 $
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

BOOL SalBitmap::Create( const Size&           rSize,
                        USHORT                nBitCount,
                        const BitmapPalette&  rPal
                      )
{
    ImplSVData  *pSVData           = ImplGetSVData();
    BOOL         bSalBitmapCreated = FALSE;

    if ( pSVData != NULL )
    {
        long nHeight = rSize.Height();
        long nWidth  = rSize.Width();

        Destroy();

        if ( ( nHeight > 0 ) && ( nWidth > 0 ) )
        {
            // Create a SalVirtualDevice

            mpVirDev
                = pSVData->mpDefInst->CreateVirtualDevice( NULL,
                                                           nWidth,
                                                           nHeight,
                                                           nBitCount
                                                         );

            if ( mpVirDev != NULL )
            {
                // Get the SalGraphics which contains the
                // GWorld we will draw to

                SalGraphics *pGraphics = GetGraphics();

                if (    ( pGraphics                             != NULL )
                     && ( pGraphics->maGraphicsData.mpCGrafPort != NULL )
                   )
                {
                    mhPixMap
                        = GetPortPixMap( pGraphics->maGraphicsData.mpCGrafPort );

                    if ( mhPixMap != NULL )
                    {
                        mnBitCount = GetPixDepth( mhPixMap);
                        maSize     = rSize;

                        bSalBitmapCreated = TRUE;
                    } // if
                } // if

                // Release the SalGraphics so that others can get a
                // handle to it in future GetGraphics() calls

                ReleaseGraphics( pGraphics );
            } // if
        } // if
    } // if

    return bSalBitmapCreated;
} // SalBitmap::Create

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp )
{
    return Create( rSalBmp, rSalBmp.GetBitCount() );
} // SalBitmap::Create

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp,
                        SalGraphics*     pGraphics
                      )
{
    USHORT nBitCount = 0;

    if (    ( pGraphics                             != NULL )
         && ( pGraphics->maGraphicsData.mpCGrafPort != NULL )
       )
    {
        mhPixMap = GetPortPixMap( pGraphics->maGraphicsData.mpCGrafPort );

        if ( mhPixMap != NULL )
        {
            nBitCount = GetPixDepth( mhPixMap );
        } // if
    } // if

    if ( nBitCount == 0 )
    {
        nBitCount = rSalBmp.GetBitCount();
    } // if

    return Create( rSalBmp, nBitCount );
} // SalBitmap::Create

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
{
    BOOL bSalBitmapCreated = FALSE;

    if ( Create( rSalBmp.maSize, nNewBitCount, BitmapPalette() ) )
    {
        // Copy pixels from rSalBmp.mpVirDev to mpVirDev

        SalGraphics *pDstGraphics = GetGraphics();

        if ( pDstGraphics != NULL )
        {
            SalGraphics *pSrcGraphics = rSalBmp.GetGraphics();

            if (    ( pSrcGraphics                             != NULL )
                 && ( pSrcGraphics->maGraphicsData.mpCGrafPort != NULL )
               )
            {
                long        nWidth  = rSalBmp.GetSize().Width();
                long        nHeight = rSalBmp.GetSize().Height();
                SalTwoRect  aTwoRect;

                // Get size of graphics to copy from

                aTwoRect.mnSrcX  = 0;
                aTwoRect.mnDestX = 0;
                aTwoRect.mnSrcY  = 0;
                aTwoRect.mnDestY = 0;

                aTwoRect.mnSrcWidth   = nWidth;
                aTwoRect.mnDestWidth  = nWidth;
                aTwoRect.mnSrcHeight  = nHeight;
                aTwoRect.mnDestHeight = nHeight;

                // Copy bits from source graphics

                pDstGraphics->CopyBits( &aTwoRect, pSrcGraphics );

                rSalBmp.ReleaseGraphics( pSrcGraphics );

                bSalBitmapCreated = TRUE;
            } // if

            ReleaseGraphics( pDstGraphics );
        } // if
    } // if

    return bSalBitmapCreated;
} // SalBitmap::Create

// ------------------------------------------------------------------

void SalBitmap::Destroy()
{
    ImplSVData *pSVData = ImplGetSVData();

    if ( pSVData != NULL )
    {
        if ( mpVirDev != NULL )
        {
            pSVData->mpDefInst->DestroyVirtualDevice( mpVirDev );
        } // if

        mhPixMap   = NULL;
        maSize     = Size();
        mnBitCount = 0;
    } // if
} // SalBitmap::Destroy

// ------------------------------------------------------------------

BitmapBuffer* SalBitmap::AcquireBuffer( BOOL bReadOnly )
{
    BitmapBuffer *pBuffer = NULL;

    if ( mpVirDev != NULL )
    {
        // Get the SalGraphics which contains the GWorld we will draw to

        SalGraphics *pGraphics = GetGraphics();

        if (    ( pGraphics                             != NULL )
             && ( pGraphics->maGraphicsData.mpCGrafPort != NULL )
           )
        {
            if ( mhPixMap != NULL )
            {
                // Lock the GWorld so that the calling functions
                // can write to this buffer

                pGraphics->maGraphicsData.mnOSStatus
                    = LockPortBits( pGraphics->maGraphicsData.mpCGrafPort );

                if ( pGraphics->maGraphicsData.mnOSStatus == noErr )
                {
                    ULONG nBMPScanlineFormat = 0;

                    pBuffer = new BitmapBuffer();

                    if ( pBuffer != NULL )
                    {
                        pBuffer->mnFormat   = BMP_FORMAT_TOP_DOWN;
                        pBuffer->mnBitCount = GetPixDepth( mhPixMap);

                        switch ( pBuffer->mnBitCount )
                        {
                            case kBlackAndWhite:
                                pBuffer->mnFormat |= BMP_FORMAT_1BIT_MSB_PAL;
                                break;
                            case kFourBitColor:
                                pBuffer->mnFormat |= BMP_FORMAT_4BIT_MSN_PAL;
                                break;
                            case kEightBitColor:
                                pBuffer->mnFormat |= BMP_FORMAT_8BIT_PAL;
                                break;
                            case kThousandsColor:
                                pBuffer->mnFormat    |= BMP_FORMAT_16BIT_TC_MASK;
                                pBuffer->maColorMask  = ColorMask( 0x7b00, 0x03e0, 0x001f);
                                break;
                            case kTrueColor:
                                pBuffer->mnFormat |= BMP_FORMAT_32BIT_TC_ARGB;
                                break;
                            default:
                                break;
                        } // switch

                        nBMPScanlineFormat = BMP_SCANLINE_FORMAT( pBuffer->mnFormat );

                        if ( nBMPScanlineFormat )
                        {
                            GWorldFlags  nGWorldFlags = noErr;

                            pBuffer->mnWidth  = mpVirDev->maVirDevData.mnWidth;
                            pBuffer->mnHeight = mpVirDev->maVirDevData.mnHeight;

                            nGWorldFlags = GetPixelsState( mhPixMap );

                            if ( nGWorldFlags == noErr )
                            {
                                LockPixels( mhPixMap );

                                pBuffer->mnScanlineSize = GetPixRowBytes( mhPixMap );
                                pBuffer->mpBits = (BYTE *)GetPixBaseAddr( mhPixMap );

                                // If the pixel depth is <= 8, we need to map QD's
                                // internal color table to the platform independent
                                // BitmapPalette color table

                                if ( pBuffer->mnBitCount <= 8 )
                                {
                                    CTabHandle hCTab = (**mhPixMap).pmTable;

                                    if ( ( hCTab != NULL ) && ( *hCTab != NULL ) )
                                    {
                                        USHORT          nColors    = (**hCTab).ctSize + 1;
                                        BitmapPalette  &rPal       = pBuffer->maPalette;
                                        SInt8           nCTabFlags = noErr;

                                        rPal.SetEntryCount( nColors );

                                        // Map each color in the QuickDraw color
                                        // table to a BitmapColor

                                        nCTabFlags = HGetState( (Handle)hCTab );

                                        if ( nCTabFlags == noErr )
                                        {
                                            USHORT  i;

                                            HLock( (Handle)hCTab );

                                            for ( i = 0; i < nColors; i++ )
                                            {
                                                BitmapColor &rCol     = rPal[i];
                                                ColorSpec    aColSpec = (**hCTab).ctTable[i];

                                                rCol.SetRed   ( (BYTE)( aColSpec.rgb.red   >> 8 ) );
                                                rCol.SetGreen ( (BYTE)( aColSpec.rgb.green >> 8 ) );
                                                rCol.SetBlue  ( (BYTE)( aColSpec.rgb.blue  >> 8 ) );
                                            } // for

                                            HSetState( (Handle)hCTab, nCTabFlags );
                                        } // if
                                    } // if
                                } // if

                                SetPixelsState( mhPixMap, nGWorldFlags );
                            } // if
                        } // if
                        else
                        {
                            UnlockPortBits( pGraphics->maGraphicsData.mpCGrafPort );

                            ReleaseBuffer( pBuffer, TRUE );
                        } // else
                    } // if

                    if ( nBMPScanlineFormat )
                    {
                        UnlockPortBits( pGraphics->maGraphicsData.mpCGrafPort );
                    } // if
                } // if
            } // if

            // Release the SalGraphics so that others can get a
            // handle to it in future GetGraphics() calls

            ReleaseGraphics( pGraphics );
        } // if
    } // if

    return pBuffer;
} // SalBitmap::AcquireBuffer

// ------------------------------------------------------------------

void SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
    if ( mpVirDev != NULL )
    {
        // Get the SalGraphics which contains the GWorld
        // we used as the buffer

        SalGraphics *pGraphics = GetGraphics();

        if (    ( pGraphics                             != NULL )
             && ( pGraphics->maGraphicsData.mpCGrafPort != NULL )
           )
        {
            // Release the SalGraphics so that others can get a
            // handle to it in future GetGraphics() calls

            ReleaseGraphics( pGraphics );

            pGraphics = NULL;
        } // if
    } // if

    if ( pBuffer != NULL )
    {
        delete pBuffer;

        pBuffer = NULL;
    } // if
} // SalBitmap::ReleaseBuffer

// ------------------------------------------------------------------

SalGraphics* SalBitmap::GetGraphics()
{
    if ( mpVirDev != NULL )
    {
        return mpVirDev->GetGraphics();
    } // if
    else
    {
        return NULL;
    } // else
} // SalBitmap::GetGraphics

// ------------------------------------------------------------------

void SalBitmap::ReleaseGraphics( SalGraphics* pGraphics )
{
    if ( mpVirDev != NULL )
    {
        mpVirDev->ReleaseGraphics( pGraphics );
    } // if
} // SalBitmap::ReleaseGraphics

