/*************************************************************************
 *
 *  $RCSfile: salbmp.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: bmahbod $ $Date: 2001-02-14 19:39:49 $
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

#ifndef _SV_SALCONST_H
#include <salconst.h>
#endif

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

#ifndef _SV_SALCOLORUTILS_HXX
#include <salcolorutils.hxx>
#endif

#ifndef _SV_SALPIXMAPUTILS_HXX
#include <salpixmaputils.hxx>
#endif

#ifndef _SV_SALRECTANGLEUTILS_HXX
#include <salrectangleutils.hxx>
#endif

// =======================================================================

// =======================================================================

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
                        const BitmapPalette&  rBitmapPalette
                      )
{
    ImplSVData  *pSVData           = ImplGetSVData();
    BOOL         bSalBitmapCreated = FALSE;

    if ( pSVData != NULL )
    {
        const long nHeight = rSize.Height();
        const long nWidth  = rSize.Width();

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

                if ( pGraphics != NULL )
                {
                    const CGrafPtr  pCGraf = pGraphics->maGraphicsData.mpCGrafPort;

                    if ( pCGraf != NULL )
                    {
                        mhPixMap = GetCGrafPortPixMap( rSize,
                                                       nBitCount,
                                                       rBitmapPalette,
                                                       pCGraf
                                                     );

                        if ( ( mhPixMap != NULL ) && ( *mhPixMap != NULL ) )
                        {
                            mnBitCount = GetPixDepth( mhPixMap);
                            maSize     = rSize;

                            bSalBitmapCreated = TRUE;
                        } // if

                        // Release the SalGraphics so that others can get a
                        // handle to it in future GetGraphics() calls
                    } // if

                    ReleaseGraphics( pGraphics );
                } // if
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
    USHORT nSalBmpBitCount   = 0;
    BOOL   bSalBitmapCreated = FALSE;

    if ( pGraphics != NULL )
    {
        const CGrafPtr  pCGraf = pGraphics->maGraphicsData.mpCGrafPort;

        if ( pCGraf != NULL )
        {
            mhPixMap = GetCGrafPortPixMap( rSalBmp.GetSize(),
                                           rSalBmp.GetBitCount(),
                                           BitmapPalette(),
                                           pCGraf
                                         );

            if ( ( mhPixMap != NULL ) && ( *mhPixMap != NULL ) )
            {
                nSalBmpBitCount = GetPixDepth( mhPixMap );
            } // if
        } // if
    } // if

    if ( nSalBmpBitCount == 0 )
    {
        nSalBmpBitCount = rSalBmp.GetBitCount();
    } // if

    bSalBitmapCreated = Create( rSalBmp, nSalBmpBitCount );

    return  bSalBitmapCreated;
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
                SalTwoRect  aTwoRect;

                // Get size of graphics to copy from

                GetTwoRectFromSalBmp( rSalBmp, &aTwoRect );

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
        pBuffer = new BitmapBuffer();

        if (    (   pBuffer != NULL )
             && (  mhPixMap != NULL )
             && ( *mhPixMap != NULL )
           )
        {
            GWorldFlags  nPixMapFlags = noErr;

            nPixMapFlags = GetPixelsState( mhPixMap );

            if ( nPixMapFlags == noErr )
            {
                if ( LockPixels( mhPixMap ) )
                {
                    SetBitmapBufferColorFormat( mhPixMap, pBuffer );

                    if ( BMP_SCANLINE_FORMAT( pBuffer->mnFormat ) )
                    {
                        pBuffer->mnWidth        = mpVirDev->maVirDevData.mnWidth;
                        pBuffer->mnHeight       = mpVirDev->maVirDevData.mnHeight;
                        pBuffer->mnScanlineSize = GetPixRowBytes( mhPixMap );
                        pBuffer->mpBits         = (BYTE *)GetPixBaseAddr( mhPixMap );
                        pBuffer->mnBitCount     = GetPixDepth( mhPixMap);

                        // If the pixel depth is <= 8, we need to map QD's
                        // internal color table to the platform independent
                        // BitmapPalette color table

                        if ( pBuffer->mnBitCount <= kEightBitColor )
                        {
                            CTabHandle hCTab = (**mhPixMap).pmTable;

                            if ( ( hCTab != NULL ) && ( *hCTab != NULL ) )
                            {
                                SInt8 nCTabFlags  = noErr;

                                nCTabFlags = HGetState( (Handle)hCTab );

                                if ( nCTabFlags == noErr )
                                {
                                    USHORT          nCTabSize      = 0;
                                    USHORT          nCTabIndex     = 0;
                                    BitmapPalette  &rBitmapPalette = pBuffer->maPalette;

                                    HLock( (Handle)hCTab );

                                        // Map each color in the QuickDraw color
                                        // table to a BitmapColor

                                        nCTabSize = (**hCTab).ctSize + 1;

                                        rBitmapPalette.SetEntryCount( nCTabSize );

                                        for ( nCTabIndex = 0;
                                              nCTabIndex < nCTabSize;
                                              nCTabIndex++
                                            )
                                        {
                                            BitmapColor     &rBitmapColor = rBitmapPalette[nCTabIndex];
                                            const RGBColor   aRGBColor    = (**hCTab).ctTable[nCTabIndex].rgb;

                                            RGBColor2BitmapColor( &aRGBColor, rBitmapColor );
                                        } // for

                                    HSetState( (Handle)hCTab, nCTabFlags );
                                } // if
                            } // if
                        } // if
                    } // if
                    else
                    {
                        delete pBuffer;

                        pBuffer = NULL;
                    } // else

                    SetPixelsState( mhPixMap, nPixMapFlags );
                } // if
            } // if
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
            // handle to it in future calls to GetGraphics()

            ReleaseGraphics( pGraphics );

            pGraphics = NULL;
        } // if
    } // if

    if ( pBuffer != NULL )
    {
        if ( !bReadOnly )
        {
            if ( ( mhPixMap != NULL ) && (*mhPixMap != NULL ) )
            {
                GWorldFlags  nPixMapFlags = noErr;

                nPixMapFlags = GetPixelsState( mhPixMap );

                if ( nPixMapFlags == noErr )
                {
                    if ( LockPixels( mhPixMap ) )
                    {
                        if ( !!pBuffer->maPalette )
                        {
                            CTabHandle hCTab = (**mhPixMap).pmTable;

                            if ( ( hCTab != NULL ) && ( *hCTab != NULL ) )
                            {
                                SInt8 nCTabFlags = noErr;

                                nCTabFlags = HGetState( (Handle)hCTab );

                                if ( nCTabFlags == noErr )
                                {
                                    const BitmapPalette  &rBitmapPalette = pBuffer->maPalette;
                                    short                 nCTabSize      = 0;
                                    short                 nCTabMinSize   = 0;
                                    short                 nCTabIndex;

                                    HLock( (Handle)hCTab );

                                        nCTabSize    = (**hCTab).ctSize + 1;
                                        nCTabMinSize = GetMinColorCount( nCTabSize, rBitmapPalette );

                                        for( nCTabIndex = 0;
                                             nCTabIndex < nCTabMinSize;
                                             nCTabIndex++
                                           )
                                        {
                                            const BitmapColor &rBitmapPaletteColor = rBitmapPalette[nCTabIndex];

                                            (**hCTab).ctTable[nCTabIndex].value = nCTabIndex;

                                            (**hCTab).ctTable[nCTabIndex].rgb
                                                = BitmapColor2RGBColor( rBitmapPaletteColor );
                                        } // for

                                        CTabChanged( hCTab );

                                    HSetState( (Handle)hCTab, nCTabFlags );
                                } // if
                            } // if
                        } // if
                        else
                        {
                            GDPtr pGDevice = NULL;

                            pGDevice = *GetGDevice();

                            if ( pGDevice != NULL )
                            {
                                PixMapPtr  pPixMap = *pGDevice->gdPMap;

                                if ( pPixMap != NULL )
                                {
                                    CTabPtr  pCTable = *pPixMap->pmTable;

                                    if ( pCTable != NULL )
                                    {
                                        ITabPtr  pITable = *pGDevice->gdITable;

                                        if ( pITable != NULL )
                                        {
                                            // Is the inverse color table up-to-date?

                                            if ( pITable->iTabSeed != pCTable->ctSeed )
                                            {
                                                // Update our inverse color table

                                                MakeITable( pPixMap->pmTable,
                                                            pGDevice->gdITable,
                                                            pGDevice->gdResPref
                                                          );

                                                pGDevice = *GetGDevice();
                                                pCTable  = *pPixMap->pmTable;
                                            } // if
                                        } // if

                                        (**mhPixMap).pmTable = pPixMap->pmTable;
                                    } // if
                                    else
                                    {
                                        (**mhPixMap).pmTable = NULL;
                                    } // else
                                } // if
                            } // if
                        } // else

                        SetPixelsState( mhPixMap, nPixMapFlags );
                    } // if
                }  //if
            } // if
        } // if

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

// =======================================================================

// =======================================================================
