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

#include <tools/debug.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/window.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/outdev.hxx>
#include <vcl/image.hxx>
#include <bmpfast.hxx>
#include <salbmp.hxx>
#include <salgdi.hxx>
#include <impbmp.hxx>
#include <sallayout.hxx>
#include <image.h>
#include <outdev.h>
#include <window.h>
#include <outdata.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <boost/scoped_array.hpp>

namespace
{
    // Co = Cs + Cd*(1-As) premultiplied alpha -or-
    // Co = (AsCs + AdCd*(1-As)) / Ao
    inline sal_uInt8 lcl_calcColor( const sal_uInt8 nSourceColor, const sal_uInt8 nSourceAlpha,
                                    const sal_uInt8 nDstAlpha, const sal_uInt8 nResAlpha, const sal_uInt8 nDestColor )
    {
        int c = nResAlpha ? ( (int)nSourceAlpha*nSourceColor + (int)nDstAlpha*nDestColor -
                              (int)nDstAlpha*nDestColor*nSourceAlpha/255 ) / (int)nResAlpha : 0;
        return sal_uInt8( c );
    }

inline BitmapColor lcl_AlphaBlend( int nX,               int nY,
                                   const long            nMapX,
                                   const long            nMapY,
                                   BitmapReadAccess*     pP,
                                   BitmapReadAccess*     pA,
                                   BitmapReadAccess*     pB,
                                   BitmapWriteAccess*    pAlphaW,
                                   sal_uInt8&            nResAlpha )
{
    BitmapColor aDstCol,aSrcCol;
    aSrcCol = pP->GetColor( nMapY, nMapX );
    aDstCol = pB->GetColor( nY, nX );

    // vcl stores transparency, not alpha - invert it
    const sal_uInt8 nSrcAlpha = 255 - pA->GetPixelIndex( nMapY, nMapX );
    const sal_uInt8 nDstAlpha = 255 - pAlphaW->GetPixelIndex( nY, nX );

    // Perform porter-duff compositing 'over' operation

    // Co = Cs + Cd*(1-As)
    // Ad = As + Ad*(1-As)
    nResAlpha = (int)nSrcAlpha + (int)nDstAlpha - (int)nDstAlpha*nSrcAlpha/255;

    aDstCol.SetRed( lcl_calcColor( aSrcCol.GetRed(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetRed() ) );
    aDstCol.SetBlue( lcl_calcColor( aSrcCol.GetBlue(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetBlue() ) );
    aDstCol.SetGreen( lcl_calcColor( aSrcCol.GetGreen(), nSrcAlpha, nDstAlpha, nResAlpha, aDstCol.GetGreen() ) );

    return aDstCol;
}
}

Bitmap OutputDevice::ImplBlendWithAlpha( Bitmap              aBmp,
                                         BitmapReadAccess*   pP,
                                         BitmapReadAccess*   pA,
                                         const Rectangle&    aDstRect,
                                         const sal_Int32     nOffY,
                                         const sal_Int32     nDstHeight,
                                         const sal_Int32     nOffX,
                                         const sal_Int32     nDstWidth,
                                         const long*         pMapX,
                                         const long*         pMapY )
{
    BitmapColor aDstCol;
    Bitmap      res;
    int         nX, nY;
    sal_uInt8   nResAlpha;

    OSL_ENSURE(mpAlphaVDev,
               "ImplBlendWithAlpha(): call me only with valid alpha VDev!" );

    bool bOldMapMode( mpAlphaVDev->IsMapModeEnabled() );
    mpAlphaVDev->EnableMapMode(false);

    Bitmap aAlphaBitmap( mpAlphaVDev->GetBitmap( aDstRect.TopLeft(), aDstRect.GetSize() ) );
    BitmapWriteAccess*  pAlphaW = aAlphaBitmap.AcquireWriteAccess();

    if( GetBitCount() <= 8 )
    {
        Bitmap              aDither( aBmp.GetSizePixel(), 8 );
        BitmapColor         aIndex( 0 );
        BitmapReadAccess*   pB = aBmp.AcquireReadAccess();
        BitmapWriteAccess*  pW = aDither.AcquireWriteAccess();

        if( pB && pP && pA && pW && pAlphaW )
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                const long nMapY = pMapY[ nY ];
                const long nModY = ( nOutY & 0x0FL ) << 4L;
                int nOutX;

                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    const long  nMapX = pMapX[ nX ];
                    const sal_uLong nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                    aDstCol = lcl_AlphaBlend( nX, nY, nMapX, nMapY, pP, pA, pB, pAlphaW, nResAlpha );

                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16UL ] +
                                              nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16UL ] +
                                              nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16UL ] ) );
                    pW->SetPixel( nY, nX, aIndex );

                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] +
                                                   nVCLGLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] +
                                                   nVCLBLut[ ( nVCLLut[ 255-nResAlpha ] + nD ) >> 16UL ] ) );
                    pAlphaW->SetPixel( nY, nX, aIndex );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        aDither.ReleaseAccess( pW );
        res = aDither;
    }
    else
    {
        BitmapWriteAccess*  pB = aBmp.AcquireWriteAccess();
        if( pP && pA && pB )
        {
            for( nY = 0; nY < nDstHeight; nY++ )
            {
                const long  nMapY = pMapY[ nY ];

                for( nX = 0; nX < nDstWidth; nX++ )
                {
                    const long nMapX = pMapX[ nX ];
                    aDstCol = lcl_AlphaBlend( nX, nY, nMapX, nMapY, pP, pA, pB, pAlphaW, nResAlpha );

                    pB->SetPixel( nY, nX, aDstCol );
                    pAlphaW->SetPixel( nY, nX, Color(255L-nResAlpha, 255L-nResAlpha, 255L-nResAlpha) );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        res = aBmp;
    }

    aAlphaBitmap.ReleaseAccess( pAlphaW );
    mpAlphaVDev->DrawBitmap( aDstRect.TopLeft(), aAlphaBitmap );
    mpAlphaVDev->EnableMapMode( bOldMapMode );

    return res;
}

Bitmap OutputDevice::ImplBlend( Bitmap              aBmp,
                                BitmapReadAccess*   pP,
                                BitmapReadAccess*   pA,
                                const sal_Int32     nOffY,
                                const sal_Int32     nDstHeight,
                                const sal_Int32     nOffX,
                                const sal_Int32     nDstWidth,
                                const Rectangle&    aBmpRect,
                                const Size&         aOutSz,
                                const bool          bHMirr,
                                const bool          bVMirr,
                                const long*         pMapX,
                                const long*         pMapY )
{
    BitmapColor aDstCol;
    Bitmap      res;
    int         nX, nY;

    if( GetBitCount() <= 8 )
    {
        Bitmap              aDither( aBmp.GetSizePixel(), 8 );
        BitmapColor         aIndex( 0 );
        BitmapReadAccess*   pB = aBmp.AcquireReadAccess();
        BitmapWriteAccess*  pW = aDither.AcquireWriteAccess();

        if( pB && pP && pA && pW )
        {
            int nOutY;

            for( nY = 0, nOutY = nOffY; nY < nDstHeight; nY++, nOutY++ )
            {
                const long nMapY = pMapY[ nY ];
                const long nModY = ( nOutY & 0x0FL ) << 4L;
                int nOutX;

                for( nX = 0, nOutX = nOffX; nX < nDstWidth; nX++, nOutX++ )
                {
                    const long  nMapX = pMapX[ nX ];
                    const sal_uLong nD = nVCLDitherLut[ nModY | ( nOutX & 0x0FL ) ];

                    aDstCol = pB->GetColor( nY, nX );
                    aDstCol.Merge( pP->GetColor( nMapY, nMapX ), pA->GetPixelIndex( nMapY, nMapX ) );
                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ ( nVCLLut[ aDstCol.GetRed() ] + nD ) >> 16UL ] +
                                              nVCLGLut[ ( nVCLLut[ aDstCol.GetGreen() ] + nD ) >> 16UL ] +
                                              nVCLBLut[ ( nVCLLut[ aDstCol.GetBlue() ] + nD ) >> 16UL ] ) );
                    pW->SetPixel( nY, nX, aIndex );
                }
            }
        }

        aBmp.ReleaseAccess( pB );
        aDither.ReleaseAccess( pW );
        res = aDither;
    }
    else
    {
        BitmapWriteAccess*  pB = aBmp.AcquireWriteAccess();

        bool bFastBlend = false;
        if( pP && pA && pB )
        {
            SalTwoRect aTR;
            aTR.mnSrcX      = aBmpRect.Left();
            aTR.mnSrcY      = aBmpRect.Top();
            aTR.mnSrcWidth  = aBmpRect.GetWidth();
            aTR.mnSrcHeight = aBmpRect.GetHeight();
            aTR.mnDestX     = nOffX;
            aTR.mnDestY     = nOffY;
            aTR.mnDestWidth = aOutSz.Width();
            aTR.mnDestHeight= aOutSz.Height();

            if( !bHMirr && !bVMirr )
                bFastBlend = ImplFastBitmapBlending( *pB,*pP,*pA, aTR );
        }

        if( pP && pA && pB && !bFastBlend )
        {
            switch( pP->GetScanlineFormat() )
            {
                case( BMP_FORMAT_8BIT_PAL ):
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            const long  nMapY = pMapY[ nY ];
                            Scanline    pPScan = pP->GetScanline( nMapY );
                            Scanline    pAScan = pA->GetScanline( nMapY );

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                const long nMapX = pMapX[ nX ];
                                aDstCol = pB->GetPixel( nY, nX );
                                pB->SetPixel( nY, nX, aDstCol.Merge( pP->GetPaletteColor( pPScan[ nMapX ] ),
                                                                     pAScan[ nMapX ] ) );
                            }
                        }
                    }
                    break;

                case( BMP_FORMAT_24BIT_TC_BGR ):
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            const long  nMapY = pMapY[ nY ];
                            Scanline    pPScan = pP->GetScanline( nMapY );
                            Scanline    pAScan = pA->GetScanline( nMapY );

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                const long  nMapX = pMapX[ nX ];
                                Scanline    pTmp = pPScan + nMapX * 3;

                                aDstCol = pB->GetPixel( nY, nX );
                                pB->SetPixel( nY, nX, aDstCol.Merge( pTmp[ 2 ], pTmp[ 1 ], pTmp[ 0 ],
                                                                     pAScan[ nMapX ] ) );
                            }
                        }
                    }
                    break;

                case( BMP_FORMAT_24BIT_TC_RGB ):
                    {
                        for( nY = 0; nY < nDstHeight; nY++ )
                        {
                            const long  nMapY = pMapY[ nY ];
                            Scanline    pPScan = pP->GetScanline( nMapY );
                            Scanline    pAScan = pA->GetScanline( nMapY );

                            for( nX = 0; nX < nDstWidth; nX++ )
                            {
                                const long  nMapX = pMapX[ nX ];
                                Scanline    pTmp = pPScan + nMapX * 3;

                                aDstCol = pB->GetPixel( nY, nX );
                                pB->SetPixel( nY, nX, aDstCol.Merge( pTmp[ 0 ], pTmp[ 1 ], pTmp[ 2 ],
                                                                     pAScan[ nMapX ] ) );
                            }
                        }
                    }
                    break;

                default:
                {
                    for( nY = 0; nY < nDstHeight; nY++ )
                    {
                        const long  nMapY = pMapY[ nY ];
                        Scanline    pAScan = pA->GetScanline( nMapY );

                        for( nX = 0; nX < nDstWidth; nX++ )
                        {
                            const long nMapX = pMapX[ nX ];
                            aDstCol = pB->GetPixel( nY, nX );
                            pB->SetPixel( nY, nX, aDstCol.Merge( pP->GetColor( nMapY, nMapX ),
                                                                 pAScan[ nMapX ] ) );
                        }
                    }
                }
                break;
            }
        }

        aBmp.ReleaseAccess( pB );
        res = aBmp;
    }

    return res;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
