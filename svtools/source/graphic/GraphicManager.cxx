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

#include <sal/config.h>
#include <tools/solar.h>
#include <tools/helpers.hxx>
#include <svtools/GraphicManager.hxx>
#include <svtools/GraphicObject.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metric.hxx>
#include <vcl/window.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/bmpacc.hxx>
#include "grfcache.hxx"

#define WATERMARK_LUM_OFFSET        50
#define WATERMARK_CON_OFFSET        -70

GraphicManager* GraphicManager::mpGlobalManager = NULL;

void GraphicManager::InitGlobal()
{
    mpGlobalManager = new GraphicManager( (officecfg::Office::Common::Cache::GraphicManager::
                                           TotalCacheSize::get()),
                                          (officecfg::Office::Common::Cache::GraphicManager::
                                           ObjectCacheSize::get()));
    mpGlobalManager->SetCacheTimeout( officecfg::Office::Common::Cache::GraphicManager::
                                      ObjectReleaseTime::get());
}

GraphicManager::GraphicManager( size_t nCacheSize, size_t nMaxObjCacheSize )
    : mpCache( new GraphicCache( nCacheSize, nMaxObjCacheSize ) )
{
}

GraphicManager::~GraphicManager()
{
    delete mpCache;
}

void GraphicManager::SetMaxCacheSize( size_t nNewCacheSize )
{
    mpCache->SetMaxDisplayCacheSize( nNewCacheSize );
}

void GraphicManager::SetMaxObjCacheSize( size_t nNewMaxObjSize, bool bDestroyGreaterCached )
{
    mpCache->SetMaxObjDisplayCacheSize( nNewMaxObjSize, bDestroyGreaterCached );
}

void GraphicManager::SetCacheTimeout( int nTimeoutSeconds )
{
    mpCache->SetCacheTimeout( nTimeoutSeconds );
}

void GraphicManager::ReleaseFromCache( const GraphicObject& /*rGraphicObject*/ )
{
    // !!!
}

bool GraphicManager::IsInCache( OutputDevice* pOutDev, const Point& rPoint,
                                const Size& rSize, const rtl::Reference<GraphicObject>& rGraphicObject,
                                const GraphicAttr& rAttr ) const
{
    return mpCache->IsInDisplayCache( pOutDev, rPoint, rSize, rGraphicObject, rAttr );
}

bool GraphicManager::IsInCache( OutputDevice* pOutDev, const Point& rPoint,
                                const Size& rSize, const GraphicObject* pGraphicObject,
                                const GraphicAttr& rAttr ) const
{
    return mpCache->IsInDisplayCache( pOutDev, rPoint, rSize, pGraphicObject, rAttr );
}

bool GraphicManager::DrawObj( OutputDevice* pOutDev, const Point& rPoint, const Size& rSize,
                              GraphicObject* pGraphicObject, const GraphicAttr& rAttr,
                              const sal_uInt32 nFlags, bool& rCached )
{
    Point   aPoint( rPoint );
    Size    aSize( rSize );
    bool    bRet = false;

    rCached = false;

    if( ( pGraphicObject->GetType() == GRAPHIC_BITMAP ) ||
        ( pGraphicObject->GetType() == GRAPHIC_GDIMETAFILE ) )
    {
        // create output and fill cache

        if( pGraphicObject->IsAnimated() ||
            ( pOutDev->GetOutDevType() == OUTDEV_PRINTER ) ||
            ( !( nFlags & GRFMGR_DRAW_NO_SUBSTITUTE ) &&
              ( ( nFlags & GRFMGR_DRAW_SUBSTITUTE ) || !( nFlags & GRFMGR_DRAW_CACHED ) ||
                ( pOutDev->GetConnectMetaFile() && !pOutDev->IsOutputEnabled() ) ) ) )
        {
            // simple output of transformed graphic
            const Graphic aGraphic( pGraphicObject->GetTransformedGraphic( &rAttr ) );

            if( aGraphic.IsSupportedGraphic() )
            {
                const sal_uInt16 nRot10 = rAttr.GetRotation() % 3600;

                if( nRot10 )
                {
                    Polygon aPoly( Rectangle( aPoint, aSize ) );

                    aPoly.Rotate( aPoint, nRot10 );
                    const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
                    aPoint = aRotBoundRect.TopLeft();
                    aSize = aRotBoundRect.GetSize();
                }

                aGraphic.Draw( pOutDev, aPoint, aSize );
            }

            bRet = true;
        }

        if( !bRet )
        {
            // cached/direct drawing
            if( !mpCache->DrawDisplayCacheObj( pOutDev, aPoint, aSize, pGraphicObject, rAttr ) )
            {
                bRet = Draw( pOutDev, aPoint, aSize, pGraphicObject, rAttr, nFlags, rCached );
            }
            else
            {
                bRet = rCached = true;
            }
        }
    }

    return bRet;
}

void GraphicManager::RegisterObject( const GraphicObject* pGraphicObject, Graphic& rSubstituteGraphicObject,
                                     const OString* pID, const GraphicObject* pCopyGraphicObject )
{
    maObjectList.push_back( pGraphicObject );
    mpCache->AddGraphicObject( pGraphicObject, rSubstituteGraphicObject, pID, pCopyGraphicObject );
}

void GraphicManager::UnregisterObject( const GraphicObject* pGraphicObject )
{
    mpCache->ReleaseGraphicObject( pGraphicObject );
    for( GraphicObjectList::iterator it = maObjectList.begin(); it != maObjectList.end(); ++it )
    {
        if ( (*it) == pGraphicObject )
        {
            maObjectList.erase( it );
            break;
        }
    }
}

void GraphicManager::GraphicObjectWasSwappedOut( const rtl::Reference<GraphicObject>& rGraphicObject )
{
    mpCache->GraphicObjectWasSwappedOut( rGraphicObject );
}

OString GraphicManager::GetUniqueID( const rtl::Reference<GraphicObject>& rGraphicObject ) const
{
    return mpCache->GetUniqueID( rGraphicObject.get() );
}

OString GraphicManager::GetUniqueID( const GraphicObject* pGraphicObject ) const
{
    return mpCache->GetUniqueID( pGraphicObject );
}

bool GraphicManager::FillSwappedGraphicObject( const rtl::Reference<GraphicObject>& rGraphicObject,
                                               Graphic& rSubstituteGraphicObject )
{
    return( mpCache->FillSwappedGraphicObject( rGraphicObject, rSubstituteGraphicObject ) );
}

void GraphicManager::GraphicObjectWasSwappedIn( const rtl::Reference<GraphicObject>& rGraphicObject )
{
    mpCache->GraphicObjectWasSwappedIn( rGraphicObject );
}

#define MAP( cVal0, cVal1, nFrac )  ((sal_uInt8)((((long)(cVal0)<<20L)+nFrac*((long)(cVal1)-(cVal0)))>>20L))

bool ImplCreateRotatedScaled( const BitmapEx& rBmpEx, const GraphicAttr& rAttributes,
                              sal_uInt16 nRot10, const Size& rUnrotatedSzPix,
                              long nStartX, long nEndX, long nStartY, long nEndY,
                              BitmapEx& rOutBmpEx )
{
    const long  aUnrotatedWidth  = rUnrotatedSzPix.Width();
    const long  aUnrotatedHeight = rUnrotatedSzPix.Height();
    const long  aBitmapWidth  = rBmpEx.GetSizePixel().Width();
    const long  aBitmapHeight = rBmpEx.GetSizePixel().Height();

    long    nX, nY, nTmpX, nTmpY, nTmpFX, nTmpFY, nTmp;
    double  fTmp;

    bool    bHMirr = ( rAttributes.GetMirrorFlags() & BMP_MIRROR_HORZ ) != 0;
    bool    bVMirr = ( rAttributes.GetMirrorFlags() & BMP_MIRROR_VERT ) != 0;

    long*   pMapIX = new long[ aUnrotatedWidth ];
    long*   pMapFX = new long[ aUnrotatedWidth ];
    long*   pMapIY = new long[ aUnrotatedHeight ];
    long*   pMapFY = new long[ aUnrotatedHeight ];

    double fRevScaleX;
    double fRevScaleY;

    bool scaleByAveraging = false;
    int x,y;

    if(aBitmapWidth > 1 && aUnrotatedWidth > 1)
    {
        fRevScaleX = (double) ( aBitmapWidth  - 1 ) / (double)( aUnrotatedWidth  - 1 );
        // create horizontal mapping table
        for( x = 0, nTmpX = aBitmapWidth - 1L, nTmp = aBitmapWidth - 2L >= 0 ? aBitmapWidth -2L : 0L; x < aUnrotatedWidth; x++ )
        {
            fTmp = x * fRevScaleX;

            if( bHMirr )
                fTmp = nTmpX - fTmp;

            pMapIX[ x ] = MinMax( fTmp, 0, nTmp );
            pMapFX[ x ] = (long) ( ( fTmp - pMapIX[ x ] ) * 1048576.0 );
        }
        scaleByAveraging |= fRevScaleX > 5.0/3.0;
    }
    else
    {
        if(aBitmapWidth == 1)
        {
            fRevScaleX = 1.0 / (double)( aUnrotatedWidth );
            for ( x = 0; x < aUnrotatedWidth ; x++)
            {
                pMapIX[x] = 0;
                pMapFX[x] = 0;
            }
            scaleByAveraging = true;
        }
        else
        {
            fRevScaleX = (double) aBitmapWidth / (double)( aUnrotatedWidth);
            fTmp = (double)aBitmapWidth / 2.0;

            pMapIX[ 0 ] = (long)fTmp;
            pMapFX[ 0 ] = (long)( ( fTmp - pMapIX[ 0 ] ) * 1048576.0 );
            scaleByAveraging = true;
        }
    }
    if(aBitmapHeight > 1 && aUnrotatedHeight > 1)
    {
        fRevScaleY = (double) ( aBitmapHeight  - 1 ) / (double)( aUnrotatedHeight - 1 );
        // create vertical mapping table
        for( y = 0, nTmpY = aBitmapHeight - 1L, nTmp = aBitmapHeight - 2L >= 0 ? aBitmapHeight - 2L : 0L; y < aUnrotatedHeight; y++ )
        {
            fTmp = y * fRevScaleY;

            if( bVMirr )
                fTmp = nTmpY - fTmp;

            pMapIY[ y ] = MinMax( fTmp, 0, nTmp );
            pMapFY[ y ] = (long) ( ( fTmp - pMapIY[ y ] ) * 1048576.0 );
        }
        scaleByAveraging |= fRevScaleY > 5.0/3.0;
    }
    else
    {
        if(aBitmapHeight == 1)
        {
            fRevScaleY = 1.0 / (double)( aUnrotatedHeight);
            for (y = 0; y < aUnrotatedHeight; ++y)
            {
                pMapIY[y] = 0;
                pMapFY[y] = 0;
            }
            scaleByAveraging = true;
        }
        else
        {
            fRevScaleY = (double) aBitmapHeight / (double)( aUnrotatedHeight);
            fTmp = (double)aBitmapHeight / 2.0;

            pMapIY[ 0 ] = (long)fTmp;
            pMapFY[ 0 ] = (long)( ( fTmp - pMapIY[ 0 ] ) * 1048576.0 );
            scaleByAveraging = true;
        }
    }

    Bitmap              aBmp( rBmpEx.GetBitmap() );
    Bitmap              aOutBmp;
    BitmapReadAccess*   pReadAccess = aBmp.AcquireReadAccess();
    BitmapWriteAccess*  pWriteAccess;

    const double        fCosAngle = cos( nRot10 * F_PI1800 );
    const double        fSinAngle = sin( nRot10 * F_PI1800 );
    const long          aTargetWidth  = nEndX - nStartX + 1L;
    const long          aTargetHeight = nEndY - nStartY + 1L;
    long*               pCosX = new long[ aTargetWidth ];
    long*               pSinX = new long[ aTargetWidth ];
    long*               pCosY = new long[ aTargetHeight ];
    long*               pSinY = new long[ aTargetHeight ];
    long                nUnRotX, nUnRotY, nSinY, nCosY;
    sal_uInt8           cR0, cG0, cB0, cR1, cG1, cB1;
    bool                bRet = false;

    Polygon             aPoly( Rectangle( Point(), rUnrotatedSzPix ) );
    aPoly.Rotate( Point(), nRot10 );
    Rectangle           aNewBound( aPoly.GetBoundRect() );

    // create horizontal mapping table
    for( x = 0, nTmpX = aNewBound.Left() + nStartX; x < aTargetWidth; x++ )
    {
        pCosX[ x ] = FRound( fCosAngle * ( fTmp = nTmpX++ << 8 ) );
        pSinX[ x ] = FRound( fSinAngle * fTmp );
    }

    // create vertical mapping table
    for( y = 0, nTmpY = aNewBound.Top() + nStartY; y < aTargetHeight; y++ )
    {
        pCosY[ y ] = FRound( fCosAngle * ( fTmp = nTmpY++ << 8 ) );
        pSinY[ y ] = FRound( fSinAngle * fTmp );
    }

    if( pReadAccess )
    {
        aOutBmp = Bitmap( Size( aTargetWidth, aTargetHeight ), 24 );
        pWriteAccess = aOutBmp.AcquireWriteAccess();

        if( pWriteAccess )
        {
            BitmapColor aColRes;

            if ( !scaleByAveraging )
            {
                if( pReadAccess->HasPalette() )
                {
                    for( y = 0; y < aTargetHeight; y++ )
                    {
                        nSinY = pSinY[ y ];
                        nCosY = pCosY[ y ];

                        for( x = 0; x < aTargetWidth; x++ )
                        {
                            nUnRotX = ( pCosX[ x ] - nSinY ) >> 8;
                            nUnRotY = ( pSinX[ x ] + nCosY ) >> 8;

                            if( ( nUnRotX >= 0L ) && ( nUnRotX < aUnrotatedWidth ) &&
                                ( nUnRotY >= 0L ) && ( nUnRotY < aUnrotatedHeight ) )
                            {
                                nTmpX = pMapIX[ nUnRotX ]; nTmpFX = pMapFX[ nUnRotX ];
                                nTmpY = pMapIY[ nUnRotY ], nTmpFY = pMapFY[ nUnRotY ];

                                const BitmapColor& rCol0 = pReadAccess->GetPaletteColor( pReadAccess->GetPixelIndex( nTmpY, nTmpX ) );
                                const BitmapColor& rCol1 = pReadAccess->GetPaletteColor( pReadAccess->GetPixelIndex( nTmpY, ++nTmpX ) );
                                cR0 = MAP( rCol0.GetRed(), rCol1.GetRed(), nTmpFX );
                                cG0 = MAP( rCol0.GetGreen(), rCol1.GetGreen(), nTmpFX );
                                cB0 = MAP( rCol0.GetBlue(), rCol1.GetBlue(), nTmpFX );

                                const BitmapColor& rCol3 = pReadAccess->GetPaletteColor( pReadAccess->GetPixelIndex( ++nTmpY, nTmpX ) );
                                const BitmapColor& rCol2 = pReadAccess->GetPaletteColor( pReadAccess->GetPixelIndex( nTmpY, --nTmpX ) );
                                cR1 = MAP( rCol2.GetRed(), rCol3.GetRed(), nTmpFX );
                                cG1 = MAP( rCol2.GetGreen(), rCol3.GetGreen(), nTmpFX );
                                cB1 = MAP( rCol2.GetBlue(), rCol3.GetBlue(), nTmpFX );

                                aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                                aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                                aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                                pWriteAccess->SetPixel( y, x, aColRes );
                            }
                        }
                    }
                }
                else
                {
                    BitmapColor aCol0, aCol1;

                    for( y = 0; y < aTargetHeight; y++ )
                    {
                        nSinY = pSinY[ y ];
                        nCosY = pCosY[ y ];

                        for( x = 0; x < aTargetWidth; x++ )
                        {
                            nUnRotX = ( pCosX[ x ] - nSinY ) >> 8;
                            nUnRotY = ( pSinX[ x ] + nCosY ) >> 8;

                            if( ( nUnRotX >= 0L ) && ( nUnRotX < aUnrotatedWidth ) &&
                                ( nUnRotY >= 0L ) && ( nUnRotY < aUnrotatedHeight ) )
                            {
                                nTmpX = pMapIX[ nUnRotX ]; nTmpFX = pMapFX[ nUnRotX ];
                                nTmpY = pMapIY[ nUnRotY ], nTmpFY = pMapFY[ nUnRotY ];

                                aCol0 = pReadAccess->GetPixel( nTmpY, nTmpX );
                                aCol1 = pReadAccess->GetPixel( nTmpY, ++nTmpX );
                                cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTmpFX );
                                cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTmpFX );
                                cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTmpFX );

                                aCol1 = pReadAccess->GetPixel( ++nTmpY, nTmpX );
                                aCol0 = pReadAccess->GetPixel( nTmpY, --nTmpX );
                                cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTmpFX );
                                cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTmpFX );
                                cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTmpFX );

                                aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                                aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                                aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                                pWriteAccess->SetPixel( y, x, aColRes );
                            }
                        }
                    }
                }
            }
            else // scaleByAveraging
            {
                double aSumRed, aSumGreen, aSumBlue, aCount;
                BitmapColor aColor;
                BitmapColor aResultColor;

                for( y = 0; y < aTargetHeight; y++ )
                {
                    nSinY = pSinY[ y ];
                    nCosY = pCosY[ y ];

                    for( x = 0; x < aTargetWidth; x++ )
                    {
                        double aUnrotatedX = ( pCosX[ x ] - nSinY ) / 256.0;
                        double aUnrotatedY = ( pSinX[ x ] + nCosY ) / 256.0;

                        if ( bHMirr )
                            aUnrotatedX = aUnrotatedWidth - aUnrotatedX - 1;
                        if ( bVMirr )
                            aUnrotatedY = aUnrotatedHeight - aUnrotatedY - 1;

                        if( ( aUnrotatedX >= 0 ) && ( aUnrotatedX < aUnrotatedWidth ) &&
                            ( aUnrotatedY >= 0 ) && ( aUnrotatedY < aUnrotatedHeight ) )
                        {
                            double dYStart = ((aUnrotatedY + 0.5) * fRevScaleY) - 0.5;
                            double dYEnd   = ((aUnrotatedY + 1.5) * fRevScaleY) - 0.5;

                            int yStart = MinMax( dYStart, 0, aBitmapHeight - 1);
                            int yEnd   = MinMax( dYEnd,   0, aBitmapHeight - 1);

                            double dXStart = ((aUnrotatedX + 0.5) * fRevScaleX) - 0.5;
                            double dXEnd   = ((aUnrotatedX + 1.5) * fRevScaleX) - 0.5;

                            int xStart = MinMax( dXStart, 0, aBitmapWidth - 1);
                            int xEnd   = MinMax( dXEnd,   0, aBitmapWidth - 1);

                            aSumRed = aSumGreen = aSumBlue = 0.0;
                            aCount = 0;

                            for (int yIn = yStart; yIn <= yEnd; yIn++)
                            {
                                for (int xIn = xStart; xIn <= xEnd; xIn++)
                                {
                                    if( pReadAccess->HasPalette() )
                                        aColor = pReadAccess->GetPaletteColor( pReadAccess->GetPixelIndex( yIn, xIn ) );
                                    else
                                        aColor = pReadAccess->GetPixel( yIn, xIn );

                                    aSumRed   += aColor.GetRed();
                                    aSumGreen += aColor.GetGreen();
                                    aSumBlue  += aColor.GetBlue();

                                    aCount++;
                                }
                            }

                            aResultColor.SetRed(   MinMax( aSumRed   / aCount, 0, 255) );
                            aResultColor.SetGreen( MinMax( aSumGreen / aCount, 0, 255) );
                            aResultColor.SetBlue(  MinMax( aSumBlue  / aCount, 0, 255) );

                            pWriteAccess->SetPixel( y, x, aResultColor );
                        }
                    }
                }
            }

            aOutBmp.ReleaseAccess( pWriteAccess );
            bRet = true;
        }

        aBmp.ReleaseAccess( pReadAccess );
    }

    // mask processing
    if( bRet && ( rBmpEx.IsTransparent() || ( nRot10 != 0 && nRot10 != 900 && nRot10 != 1800 && nRot10 != 2700 ) ) )
    {
        bRet = false;

        if( rBmpEx.IsAlpha() )
        {
            AlphaMask   aAlpha( rBmpEx.GetAlpha() );
            AlphaMask   aOutAlpha;

            pReadAccess = aAlpha.AcquireReadAccess();

            if( pReadAccess )
            {
                aOutAlpha = AlphaMask( Size( aTargetWidth, aTargetHeight ) );
                pWriteAccess = aOutAlpha.AcquireWriteAccess();

                if( pWriteAccess )
                {
                    if( pReadAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL &&
                        pWriteAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                    {
                        if ( !scaleByAveraging )
                        {
                            Scanline pLine0, pLine1, pLineW;

                            for( nY = 0; nY < aTargetHeight; nY++ )
                            {
                                nSinY = pSinY[ nY ], nCosY = pCosY[ nY ];
                                pLineW = pWriteAccess->GetScanline( nY );

                                for( nX = 0; nX < aTargetWidth; nX++ )
                                {
                                    nUnRotX = ( pCosX[ nX ] - nSinY ) >> 8;
                                    nUnRotY = ( pSinX[ nX ] + nCosY ) >> 8;

                                    if( ( nUnRotX >= 0L ) && ( nUnRotX < aUnrotatedWidth ) &&
                                        ( nUnRotY >= 0L ) && ( nUnRotY < aUnrotatedHeight ) )
                                    {
                                        nTmpX = pMapIX[ nUnRotX ], nTmpFX = pMapFX[ nUnRotX ];
                                        nTmpY = pMapIY[ nUnRotY ], nTmpFY = pMapFY[ nUnRotY ];

                                        pLine0 = pReadAccess->GetScanline( nTmpY++ );
                                        pLine1 = pReadAccess->GetScanline( nTmpY );

                                        const long  nAlpha0 = pLine0[ nTmpX ];
                                        const long  nAlpha2 = pLine1[ nTmpX++ ];
                                        const long  nAlpha1 = pLine0[ nTmpX ];
                                        const long  nAlpha3 = pLine1[ nTmpX ];
                                        const long  n0 = MAP( nAlpha0, nAlpha1, nTmpFX );
                                        const long  n1 = MAP( nAlpha2, nAlpha3, nTmpFX );

                                        *pLineW++ = MAP( n0, n1, nTmpFY );
                                    }
                                    else
                                        *pLineW++ = 255;
                                }
                            }
                        }
                        else // scaleByAveraging
                        {
                            const BitmapColor   aTrans( pWriteAccess->GetBestMatchingColor( Color( COL_WHITE ) ) );
                            BitmapColor         aResultColor( 0 );
                            double aSum, aCount;

                            for( y = 0; y < aTargetHeight; y++ )
                            {
                                nSinY = pSinY[ y ];
                                nCosY = pCosY[ y ];

                                for( x = 0; x < aTargetWidth; x++ )
                                {

                                    double aUnrotatedX = ( pCosX[ x ] - nSinY ) / 256.0;
                                    double aUnrotatedY = ( pSinX[ x ] + nCosY ) / 256.0;

                                    if ( bHMirr )
                                        aUnrotatedX = aUnrotatedWidth - aUnrotatedX - 1;
                                    if ( bVMirr )
                                        aUnrotatedY = aUnrotatedHeight - aUnrotatedY - 1;

                                    if( ( aUnrotatedX >= 0 ) && ( aUnrotatedX < aUnrotatedWidth ) &&
                                        ( aUnrotatedY >= 0 ) && ( aUnrotatedY < aUnrotatedHeight ) )
                                    {
                                        double dYStart = ((aUnrotatedY + 0.5) * fRevScaleY) - 0.5;
                                        double dYEnd   = ((aUnrotatedY + 1.5) * fRevScaleY) - 0.5;

                                        int yStart = MinMax( dYStart, 0, aBitmapHeight - 1);
                                        int yEnd   = MinMax( dYEnd,   0, aBitmapHeight - 1);

                                        double dXStart = ((aUnrotatedX + 0.5) * fRevScaleX) - 0.5;
                                        double dXEnd   = ((aUnrotatedX + 1.5) * fRevScaleX) - 0.5;

                                        int xStart = MinMax( dXStart, 0, aBitmapWidth - 1);
                                        int xEnd   = MinMax( dXEnd,   0, aBitmapWidth - 1);

                                        aSum = 0.0;
                                        aCount = 0;

                                        for (int yIn = yStart; yIn <= yEnd; yIn++)
                                        {
                                            for (int xIn = xStart; xIn <= xEnd; xIn++)
                                            {
                                                aSum += pReadAccess->GetPixel( yIn, xIn ).GetIndex();
                                                aCount++;
                                            }
                                        }
                                        aResultColor.SetIndex( MinMax( aSum  / aCount, 0, 255) );
                                        pWriteAccess->SetPixel( y, x, aResultColor );
                                    }
                                    else
                                    {
                                        pWriteAccess->SetPixel( y, x, aTrans );
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        const BitmapColor   aTrans( pWriteAccess->GetBestMatchingColor( Color( COL_WHITE ) ) );
                        BitmapColor         aAlphaVal( 0 );

                        for( nY = 0; nY < aTargetHeight; nY++ )
                        {
                            nSinY = pSinY[ nY ], nCosY = pCosY[ nY ];

                            for( nX = 0; nX < aTargetWidth; nX++ )
                            {
                                nUnRotX = ( pCosX[ nX ] - nSinY ) >> 8;
                                nUnRotY = ( pSinX[ nX ] + nCosY ) >> 8;

                                if( ( nUnRotX >= 0L ) && ( nUnRotX < aUnrotatedWidth ) &&
                                    ( nUnRotY >= 0L ) && ( nUnRotY < aUnrotatedHeight ) )
                                {
                                    nTmpX = pMapIX[ nUnRotX ]; nTmpFX = pMapFX[ nUnRotX ];
                                    nTmpY = pMapIY[ nUnRotY ], nTmpFY = pMapFY[ nUnRotY ];

                                    const long  nAlpha0 = pReadAccess->GetPixel( nTmpY, nTmpX ).GetIndex();
                                    const long  nAlpha1 = pReadAccess->GetPixel( nTmpY, ++nTmpX ).GetIndex();
                                    const long  nAlpha3 = pReadAccess->GetPixel( ++nTmpY, nTmpX ).GetIndex();
                                    const long  nAlpha2 = pReadAccess->GetPixel( nTmpY, --nTmpX ).GetIndex();
                                    const long  n0 = MAP( nAlpha0, nAlpha1, nTmpFX );
                                    const long  n1 = MAP( nAlpha2, nAlpha3, nTmpFX );

                                    aAlphaVal.SetIndex( MAP( n0, n1, nTmpFY ) );
                                    pWriteAccess->SetPixel( nY, nX, aAlphaVal );
                                }
                                else
                                    pWriteAccess->SetPixel( nY, nX, aTrans );
                            }
                        }
                    }

                    aOutAlpha.ReleaseAccess( pWriteAccess );
                    bRet = sal_True;
                }

                aAlpha.ReleaseAccess( pReadAccess );
            }

            if( bRet )
                rOutBmpEx = BitmapEx( aOutBmp, aOutAlpha );
        }
        else
        {
            Bitmap aOutMsk( Size( aTargetWidth, aTargetHeight ), 1 );
            pWriteAccess = aOutMsk.AcquireWriteAccess();

            if( pWriteAccess )
            {
                Bitmap              aMsk( rBmpEx.GetMask() );
                const BitmapColor   aB( pWriteAccess->GetBestMatchingColor( Color( COL_BLACK ) ) );
                const BitmapColor   aW( pWriteAccess->GetBestMatchingColor( Color( COL_WHITE ) ) );
                BitmapReadAccess*   pMAcc = NULL;

                if( !aMsk || ( ( pMAcc = aMsk.AcquireReadAccess() ) != NULL ) )
                {
                    long*       pMapLX = new long[ aUnrotatedWidth ];
                    long*       pMapLY = new long[ aUnrotatedHeight ];
                    BitmapColor aTestB;

                    if( pMAcc )
                        aTestB = pMAcc->GetBestMatchingColor( Color( COL_BLACK ) );

                    // create new horizontal mapping table
                    for( nX = 0UL; nX < aUnrotatedWidth; nX++ )
                        pMapLX[ nX ] = FRound( (double) pMapIX[ nX ] + pMapFX[ nX ] / 1048576.0 );

                    // create new vertical mapping table
                    for( nY = 0UL; nY < aUnrotatedHeight; nY++ )
                        pMapLY[ nY ] = FRound( (double) pMapIY[ nY ] + pMapFY[ nY ] / 1048576.0 );

                    // do mask rotation
                    for( nY = 0; nY < aTargetHeight; nY++ )
                    {
                        nSinY = pSinY[ nY ];
                        nCosY = pCosY[ nY ];

                        for( nX = 0; nX < aTargetWidth; nX++ )
                        {
                            nUnRotX = ( pCosX[ nX ] - nSinY ) >> 8;
                            nUnRotY = ( pSinX[ nX ] + nCosY ) >> 8;

                            if( ( nUnRotX >= 0L ) && ( nUnRotX < aUnrotatedWidth ) &&
                                ( nUnRotY >= 0L ) && ( nUnRotY < aUnrotatedHeight ) )
                            {
                                if( pMAcc )
                                {
                                    if( pMAcc->GetPixel( pMapLY[ nUnRotY ], pMapLX[ nUnRotX ] ) == aTestB )
                                        pWriteAccess->SetPixel( nY, nX, aB );
                                    else
                                        pWriteAccess->SetPixel( nY, nX, aW );
                                }
                                else
                                    pWriteAccess->SetPixel( nY, nX, aB );
                            }
                            else
                                pWriteAccess->SetPixel( nY, nX, aW );
                        }
                    }

                    delete[] pMapLX;
                    delete[] pMapLY;

                    if( pMAcc )
                        aMsk.ReleaseAccess( pMAcc );

                    bRet = sal_True;
                }

                aOutMsk.ReleaseAccess( pWriteAccess );
            }

            if( bRet )
                rOutBmpEx = BitmapEx( aOutBmp, aOutMsk );
        }

        if( !bRet )
            rOutBmpEx = aOutBmp;
    }
    else
        rOutBmpEx = aOutBmp;

    delete[] pSinX;
    delete[] pCosX;
    delete[] pSinY;
    delete[] pCosY;

    delete[] pMapIX;
    delete[] pMapFX;
    delete[] pMapIY;
    delete[] pMapFY;

    return bRet;
}

bool GraphicManager::Draw( OutputDevice* pOutDev, const Point& rPoint,
                           const Size& rSize, GraphicObject* pGraphicObject,
                           const GraphicAttr& rAttr,
                           const sal_uInt32 nFlags, bool& rCached )
{
    const Graphic&  rGraphic = pGraphicObject->GetGraphic();
    bool bRet = false;

    if( rGraphic.IsSupportedGraphic() && !rGraphic.IsSwapOut() )
    {
        if( GRAPHIC_BITMAP == rGraphic.GetType() )
        {
            const BitmapEx aSrcBmpEx( rGraphic.GetBitmapEx() );

            // #i46805# No point in caching a bitmap that is rendered
            // via RectFill on the OutDev
            if( !(pOutDev->GetDrawMode() & ( DRAWMODE_BLACKBITMAP | DRAWMODE_WHITEBITMAP )) &&
                mpCache->IsDisplayCacheable( pOutDev, rPoint, rSize, pGraphicObject, rAttr ) )
            {
                BitmapEx aDstBmpEx;

                if( ImplCreateOutput( pOutDev, rPoint, rSize, aSrcBmpEx, rAttr, nFlags, &aDstBmpEx ) )
                {
                    rCached = mpCache->CreateDisplayCacheObj( pOutDev, rPoint, rSize, pGraphicObject, rAttr, aDstBmpEx );
                    bRet = true;
                }
            }

            if( !bRet )
            {
                bRet = ImplCreateOutput( pOutDev, rPoint, rSize, aSrcBmpEx, rAttr, nFlags );
            }
        }
        else
        {
            const GDIMetaFile& rSrcMtf = rGraphic.GetGDIMetaFile();

            if( mpCache->IsDisplayCacheable( pOutDev, rPoint, rSize, pGraphicObject, rAttr ) )
            {
                GDIMetaFile aDstMtf;
                BitmapEx    aContainedBmpEx;

                if( ImplCreateOutput( pOutDev, rPoint, rSize, rSrcMtf, rAttr, nFlags, aDstMtf, aContainedBmpEx ) )
                {
                    if( !!aContainedBmpEx )
                    {
                        // Use bitmap output method, if metafile basically contains only a single
                        // bitmap (allows caching the resulting pixmap).
                        BitmapEx aDstBmpEx;

                        if( ImplCreateOutput( pOutDev, rPoint, rSize, aContainedBmpEx, rAttr, nFlags, &aDstBmpEx ) )
                        {
                            rCached = mpCache->CreateDisplayCacheObj( pOutDev, rPoint, rSize, pGraphicObject, rAttr, aDstBmpEx );
                            bRet = true;
                        }
                    }
                    else
                    {
                        rCached = mpCache->CreateDisplayCacheObj( pOutDev, rPoint, rSize, pGraphicObject, rAttr, aDstMtf );
                        bRet = true;
                    }
                }
            }

            if( !bRet )
            {
                const Graphic aGraphic( pGraphicObject->GetTransformedGraphic( &rAttr ) );

                if( aGraphic.IsSupportedGraphic() )
                {
                    aGraphic.Draw( pOutDev, rPoint, rSize );
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}

// This function checks whether the bitmap is usable for skipping
// mtf rendering by using just this one bitmap (i.e. in case the metafile
// contains just this one pixmap that covers the entire metafile area).
static BitmapEx checkMetadataBitmap( const BitmapEx& rBmpEx,
                                     Point    rSrcPoint,
                                     Size     rSrcSize,
                                     const Point&    rDestPoint,
                                     const Size&     rDestSize,
                                     const Size&     rRefSize,
                                     bool&           o_rbNonBitmapActionEncountered )
{
// NOTE: If you do changes in this function, change checkMetadataBitmap() in grfcache.cxx too.
    BitmapEx aBmpEx;
    if( rSrcSize == Size())
        rSrcSize = rBmpEx.GetSizePixel();

    if( rDestPoint != Point( 0, 0 ))
    {   // The pixmap in the metafile has an offset (and so would not cover)
        // the entire result -> fall back to mtf rendering.
        o_rbNonBitmapActionEncountered = true;
        return aBmpEx;
    }
    if( rDestSize != rRefSize )
    {   // The pixmap is not fullscale (does not cover the entire metafile area).
        // HACK: The code here should refuse to use the bitmap directly
        // and fall back to mtf rendering, but there seem to be metafiles
        // that do not specify exactly their area (the Windows API requires apps
        // the specify it manually, the rectangle is specified as topleft/bottomright
        // rather than topleft/size [which may be confusing], and the docs
        // on the exact meaning are somewhat confusing as well), so if it turns
        // out this metafile really contains just one bitmap and no other painting,
        // and if the sizes almost match, just use the pixmap (which will be scaled
        // to fit exactly the requested size, so there should not be any actual problem
        // caused by this small difference). This will allow caching of the resulting
        // (scaled) pixmap, which can make a noticeable performance difference.
        if( rBmpEx.GetSizePixel().Width() > 100 && rBmpEx.GetSizePixel().Height() > 100
            && abs( rDestSize.Width() - rRefSize.Width()) < 5
            && abs( rDestSize.Height() - rRefSize.Height()) < 5 )
            ; // ok, assume it's close enough
        else
        {  // fall back to mtf rendering
            o_rbNonBitmapActionEncountered = true;
            return aBmpEx;
        }
    }

    aBmpEx = rBmpEx;

    if( (rSrcPoint.X() != 0 && rSrcPoint.Y() != 0) ||
        rSrcSize != rBmpEx.GetSizePixel() )
    {
        // crop bitmap to given source rectangle (no
        // need to copy and convert the whole bitmap)
        const Rectangle aCropRect( rSrcPoint,
                                   rSrcSize );
        aBmpEx.Crop( aCropRect );
    }

    return aBmpEx;
}

bool GraphicManager::ImplCreateOutput( OutputDevice* pOutputDevice,
                                       const Point& rPoint, const Size& rSize,
                                       const BitmapEx& rBitmapEx, const GraphicAttr& rAttributes,
                                       const sal_uLong /*nFlags*/, BitmapEx* pBmpEx )
{
    sal_uInt16  nRot10 = rAttributes.GetRotation() % 3600;

    Point   aOutputPointPix;
    Size    aOutputSizePix;
    Point   aUnrotatedPointPix( pOutputDevice->LogicToPixel( rPoint ) );
    Size    aUnrotatedSizePix(  pOutputDevice->LogicToPixel( rSize ) );

    bool    bRet = false;

    if( nRot10 )
    {
        Polygon aPoly( Rectangle( rPoint, rSize ) );
        aPoly.Rotate( rPoint, nRot10 );
        const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
        aOutputPointPix = pOutputDevice->LogicToPixel( aRotBoundRect.TopLeft() );
        aOutputSizePix  = pOutputDevice->LogicToPixel( aRotBoundRect.GetSize() );
    }
    else
    {
        aOutputPointPix = aUnrotatedPointPix;
        aOutputSizePix  = aUnrotatedSizePix;
    }

    if( aUnrotatedSizePix.Width() && aUnrotatedSizePix.Height() )
    {
        BitmapEx        aBmpEx( rBitmapEx );
        BitmapEx        aOutBmpEx;
        Point           aOutPoint;
        Size            aOutSize;
        const Size&     rBmpSzPix = rBitmapEx.GetSizePixel();
        const long      nW = rBmpSzPix.Width();
        const long      nH = rBmpSzPix.Height();
        long            nStartX = -1, nStartY = -1, nEndX = -1, nEndY = -1;
        bool            bHMirr = ( rAttributes.GetMirrorFlags() & BMP_MIRROR_HORZ ) != 0;
        bool            bVMirr = ( rAttributes.GetMirrorFlags() & BMP_MIRROR_VERT ) != 0;

        // calculate output sizes
        if( !pBmpEx )
        {
            Point       aPt;
            Rectangle   aOutRect( aPt, pOutputDevice->GetOutputSizePixel() );
            Rectangle   aBmpRect( aOutputPointPix, aOutputSizePix );

            if( pOutputDevice->GetOutDevType() == OUTDEV_WINDOW )
            {
                const Region aPaintRgn( ( (Window*) pOutputDevice )->GetPaintRegion() );
                if( !aPaintRgn.IsNull() )
                    aOutRect.Intersection( pOutputDevice->LogicToPixel( aPaintRgn.GetBoundRect() ) );
            }

            aOutRect.Intersection( aBmpRect );

            if( !aOutRect.IsEmpty() )
            {
                aOutPoint = pOutputDevice->PixelToLogic( aOutRect.TopLeft() );
                aOutSize = pOutputDevice->PixelToLogic( aOutRect.GetSize() );
                nStartX = aOutRect.Left() - aBmpRect.Left();
                nStartY = aOutRect.Top() - aBmpRect.Top();
                nEndX = aOutRect.Right() - aBmpRect.Left();
                nEndY = aOutRect.Bottom() - aBmpRect.Top();
            }
            else
            {
                nStartX = -1L; // invalid
            }
        }
        else
        {
            aOutPoint = pOutputDevice->PixelToLogic( aOutputPointPix );
            aOutSize = pOutputDevice->PixelToLogic( aOutputSizePix );
            nStartX = nStartY = 0;
            nEndX = aOutputSizePix.Width() - 1L;
            nEndY = aOutputSizePix.Height() - 1L;
        }

        // do transformation
        if( nStartX >= 0L )
        {
            const bool bSimple = ( 1 == nW || 1 == nH );

            if( nRot10 )
            {
                if( bSimple )
                {
                    bRet = ( aOutBmpEx = aBmpEx ).Scale( aUnrotatedSizePix );

                    if( bRet )
                        aOutBmpEx.Rotate( nRot10, COL_TRANSPARENT );
                }
                else
                {
                    bRet = ImplCreateRotatedScaled( aBmpEx, rAttributes,
                                                    nRot10, aUnrotatedSizePix,
                                                    nStartX, nEndX, nStartY, nEndY,
                                                    aOutBmpEx );
                }
            }
            else
            {
                if( !bHMirr && !bVMirr && aOutputSizePix == rBmpSzPix )
                {
                    aOutPoint = pOutputDevice->PixelToLogic( aOutputPointPix );
                    aOutSize  = pOutputDevice->PixelToLogic( aOutputSizePix );
                    aOutBmpEx = aBmpEx;
                    bRet      = true;
                }
                else
                {
                    if( bSimple )
                    {
                        bRet = ( aOutBmpEx = aBmpEx ).Scale( Size( nEndX - nStartX + 1, nEndY - nStartY + 1 ) );
                    }
                    else
                    {
                        bRet = ImplCreateRotatedScaled( aBmpEx, rAttributes,
                                                    nRot10, aUnrotatedSizePix,
                                                    nStartX, nEndX, nStartY, nEndY,
                                                    aOutBmpEx );
                    }
                }
            }

            if( bRet )
            {
                // Attribute adjustment if necessary
                if( rAttributes.IsSpecialDrawMode() || rAttributes.IsAdjusted() || rAttributes.IsTransparent() )
                {
                    Adjust( aOutBmpEx, rAttributes, ADJUSTMENT_DRAWMODE | ADJUSTMENT_COLORS | ADJUSTMENT_TRANSPARENCY );
                }
                // OutDev adjustment if necessary
                if( pOutputDevice->GetOutDevType() != OUTDEV_PRINTER && pOutputDevice->GetBitCount() <= 8 && aOutBmpEx.GetBitCount() >= 8 )
                {
                    aOutBmpEx.Dither( BMP_DITHER_MATRIX );
                }
            }
        }

        // Create output
        if( bRet )
        {
            if( !pBmpEx )
                pOutputDevice->DrawBitmapEx( aOutPoint, aOutSize, aOutBmpEx );
            else
            {
                if( !rAttributes.IsTransparent() && !aOutBmpEx.IsAlpha() )
                    aOutBmpEx = BitmapEx( aOutBmpEx.GetBitmap().CreateDisplayBitmap( pOutputDevice ), aOutBmpEx.GetMask() );

                pOutputDevice->DrawBitmapEx( aOutPoint, aOutSize, *pBmpEx = aOutBmpEx );
            }
        }
    }

    return bRet;
}

bool GraphicManager::ImplCreateOutput( OutputDevice* pOutDev,
                                       const Point& rPoint, const Size& rSize,
                                       const GDIMetaFile& rMtf, const GraphicAttr& rAttr,
                                       const sal_uInt32 /*nFlags*/, GDIMetaFile& rOutMtf, BitmapEx& rOutBmpEx )
{
    const Size aNewSize( rMtf.GetPrefSize() );

    rOutMtf = rMtf;

    // Count bitmap actions, and flag actions that paint, but
    // are no bitmaps.
    sal_Int32   nNumBitmaps(0);
    bool        bNonBitmapActionEncountered(false);
    if( aNewSize.Width() && aNewSize.Height() && rSize.Width() && rSize.Height() )
    {
        const double fGrfWH = (double) aNewSize.Width() / aNewSize.Height();
        const double fOutWH = (double) rSize.Width() / rSize.Height();

        const double fScaleX = fOutWH / fGrfWH;
        const double fScaleY = 1.0;

        const MapMode rPrefMapMode( rMtf.GetPrefMapMode() );
        const Size rSizePix( pOutDev->LogicToPixel( aNewSize, rPrefMapMode ) );

// NOTE: If you do changes in this function, check GraphicDisplayCacheEntry::IsCacheableAsBitmap
// in grfcache.cxx too.

        // Determine whether the metafile basically displays
        // a single bitmap (in which case that bitmap is simply used directly
        // instead of playing the metafile). Note that
        // the solution, as implemented here, is quite suboptimal (the
        // cases where a mtf consisting basically of a single bitmap,
        // that fail to pass the test below, are probably frequent). A
        // better solution would involve FSAA, but that's currently
        // expensive, and might trigger bugs on display drivers, if
        // VDevs get bigger than the actual screen.
        sal_uInt32  nCurPos;
        MetaAction* pAct;
        for( nCurPos = 0, pAct = (MetaAction*)rOutMtf.FirstAction(); pAct;
             pAct = (MetaAction*)rOutMtf.NextAction(), nCurPos++ )
        {
            MetaAction* pModAct = NULL;
            switch( pAct->GetType() )
            {
            case META_FONT_ACTION:
                {
                    // taking care of font width default if scaling metafile.
                    MetaFontAction* pA = (MetaFontAction*)pAct;
                    Font aFont( pA->GetFont() );
                    if ( !aFont.GetWidth() )
                    {
                        FontMetric aFontMetric( pOutDev->GetFontMetric( aFont ) );
                        aFont.SetWidth( aFontMetric.GetWidth() );
                        pModAct = new MetaFontAction( aFont );
                    }
                }
                // FALLTHROUGH intended
            case META_NULL_ACTION:
            case META_LINECOLOR_ACTION:
            case META_FILLCOLOR_ACTION:
            case META_TEXTCOLOR_ACTION:
            case META_TEXTFILLCOLOR_ACTION:
            case META_TEXTALIGN_ACTION:
            case META_TEXTLINECOLOR_ACTION:
            case META_TEXTLINE_ACTION:
            case META_PUSH_ACTION:
            case META_POP_ACTION:
            case META_LAYOUTMODE_ACTION:
            case META_TEXTLANGUAGE_ACTION:
            case META_COMMENT_ACTION:
                break;

            // bitmap output methods
            case META_BMP_ACTION:
                if( !nNumBitmaps && !bNonBitmapActionEncountered )
                {
                    MetaBmpAction* pAction = (MetaBmpAction*)pAct;

                    rOutBmpEx = checkMetadataBitmap( BitmapEx( pAction->GetBitmap()),
                                                     Point(), Size(),
                                                     pOutDev->LogicToPixel( pAction->GetPoint(),
                                                                            rPrefMapMode ),
                                                     pAction->GetBitmap().GetSizePixel(),
                                                     rSizePix,
                                                     bNonBitmapActionEncountered );
                }
                ++nNumBitmaps;
                break;

            case META_BMPSCALE_ACTION:
                if( !nNumBitmaps && !bNonBitmapActionEncountered )
                {
                    MetaBmpScaleAction* pAction = (MetaBmpScaleAction*)pAct;

                    rOutBmpEx = checkMetadataBitmap( BitmapEx( pAction->GetBitmap()),
                                                     Point(), Size(),
                                                     pOutDev->LogicToPixel( pAction->GetPoint(),
                                                                            rPrefMapMode ),
                                                     pOutDev->LogicToPixel( pAction->GetSize(),
                                                                            rPrefMapMode ),
                                                     rSizePix,
                                                     bNonBitmapActionEncountered );
                }
                ++nNumBitmaps;
                break;

            case META_BMPSCALEPART_ACTION:
                if( !nNumBitmaps && !bNonBitmapActionEncountered )
                {
                    MetaBmpScalePartAction* pAction = (MetaBmpScalePartAction*)pAct;

                    rOutBmpEx = checkMetadataBitmap( BitmapEx( pAction->GetBitmap() ),
                                                     pAction->GetSrcPoint(),
                                                     pAction->GetSrcSize(),
                                                     pOutDev->LogicToPixel( pAction->GetDestPoint(),
                                                                            rPrefMapMode ),
                                                     pOutDev->LogicToPixel( pAction->GetDestSize(),
                                                                            rPrefMapMode ),
                                                     rSizePix,
                                                     bNonBitmapActionEncountered );
                }
                ++nNumBitmaps;
                break;

            case META_BMPEX_ACTION:
                if( !nNumBitmaps && !bNonBitmapActionEncountered )
                {
                    MetaBmpExAction* pAction = (MetaBmpExAction*)pAct;

                    rOutBmpEx = checkMetadataBitmap( pAction->GetBitmapEx(),
                                                     Point(), Size(),
                                                     pOutDev->LogicToPixel( pAction->GetPoint(),
                                                                            rPrefMapMode ),
                                                     pAction->GetBitmapEx().GetSizePixel(),
                                                     rSizePix,
                                                     bNonBitmapActionEncountered );
                }
                ++nNumBitmaps;
                break;

            case META_BMPEXSCALE_ACTION:
                if( !nNumBitmaps && !bNonBitmapActionEncountered )
                {
                    MetaBmpExScaleAction* pAction = (MetaBmpExScaleAction*)pAct;

                    rOutBmpEx = checkMetadataBitmap( pAction->GetBitmapEx(),
                                                     Point(), Size(),
                                                     pOutDev->LogicToPixel( pAction->GetPoint(),
                                                                            rPrefMapMode ),
                                                     pOutDev->LogicToPixel( pAction->GetSize(),
                                                                            rPrefMapMode ),
                                                     rSizePix,
                                                     bNonBitmapActionEncountered );
                }
                ++nNumBitmaps;
                break;

            case META_BMPEXSCALEPART_ACTION:
                if( !nNumBitmaps && !bNonBitmapActionEncountered )
                {
                    MetaBmpExScalePartAction* pAction = (MetaBmpExScalePartAction*)pAct;

                    rOutBmpEx = checkMetadataBitmap( pAction->GetBitmapEx(),
                                                     pAction->GetSrcPoint(),
                                                     pAction->GetSrcSize(),
                                                     pOutDev->LogicToPixel( pAction->GetDestPoint(),
                                                                            rPrefMapMode ),
                                                     pOutDev->LogicToPixel( pAction->GetDestSize(),
                                                                            rPrefMapMode ),
                                                     rSizePix,
                                                     bNonBitmapActionEncountered );
                }
                ++nNumBitmaps;
                break;

            // these actions actually output something (that's
            // different from a bitmap)
            case META_RASTEROP_ACTION:
                if( ((MetaRasterOpAction*)pAct)->GetRasterOp() == ROP_OVERPAINT )
                    break;
                // FALLTHROUGH intended
            case META_PIXEL_ACTION:
            case META_POINT_ACTION:
            case META_LINE_ACTION:
            case META_RECT_ACTION:
            case META_ROUNDRECT_ACTION:
            case META_ELLIPSE_ACTION:
            case META_ARC_ACTION:
            case META_PIE_ACTION:
            case META_CHORD_ACTION:
            case META_POLYLINE_ACTION:
            case META_POLYGON_ACTION:
            case META_POLYPOLYGON_ACTION:
            case META_TEXT_ACTION:
            case META_TEXTARRAY_ACTION:
            case META_STRETCHTEXT_ACTION:
            case META_TEXTRECT_ACTION:
            case META_MASK_ACTION:
            case META_MASKSCALE_ACTION:
            case META_MASKSCALEPART_ACTION:
            case META_GRADIENT_ACTION:
            case META_HATCH_ACTION:
            case META_WALLPAPER_ACTION:
            case META_TRANSPARENT_ACTION:
            case META_EPS_ACTION:
            case META_FLOATTRANSPARENT_ACTION:
            case META_GRADIENTEX_ACTION:
            case META_CLIPREGION_ACTION:
            case META_ISECTRECTCLIPREGION_ACTION:
            case META_ISECTREGIONCLIPREGION_ACTION:
            case META_MOVECLIPREGION_ACTION:
            case META_MAPMODE_ACTION:
            case META_REFPOINT_ACTION:
            default:
                bNonBitmapActionEncountered = true;
                break;
            }
            if ( pModAct )
            {
                MetaAction* pDeleteAction = rOutMtf.ReplaceAction( pModAct, nCurPos );
                assert(pDeleteAction);
                pDeleteAction->Delete();
            }
            else
            {
                if( pAct->GetRefCount() > 1 )
                {
                    MetaAction* pDeleteAction = rOutMtf.ReplaceAction( pModAct = pAct->Clone(), nCurPos );
                    assert(pDeleteAction);
                    pDeleteAction->Delete();
                }
                else
                {
                    pModAct = pAct;
                }
            }
            pModAct->Scale( fScaleX, fScaleY );
        }
        rOutMtf.SetPrefSize( Size( FRound( aNewSize.Width() * fScaleX ),
                                   FRound( aNewSize.Height() * fScaleY ) ) );
    }

    if( nNumBitmaps != 1 || bNonBitmapActionEncountered )
    {
        if( rAttr.IsSpecialDrawMode() || rAttr.IsAdjusted() || rAttr.IsMirrored() || rAttr.IsRotated() || rAttr.IsTransparent() )
        {
            Adjust( rOutMtf, rAttr, ADJUSTMENT_ALL );
        }
        Draw( pOutDev, rPoint, rSize, rOutMtf, rAttr );
        rOutBmpEx = BitmapEx();
    }

    return true;
}

void GraphicManager::Adjust( BitmapEx& rBmpEx, const GraphicAttr& rAttr, sal_uInt32 nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
        case GRAPHICDRAWMODE_MONO:
            rBmpEx.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            break;

        case GRAPHICDRAWMODE_GREYS:
            rBmpEx.Convert( BMP_CONVERSION_8BIT_GREYS );
            break;

        case GRAPHICDRAWMODE_WATERMARK:
            aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
            aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            break;

        default:
            break;
        }
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_COLORS ) && aAttr.IsAdjusted() )
    {
        rBmpEx.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                       aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                       aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_MIRROR ) && aAttr.IsMirrored() )
    {
        rBmpEx.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        rBmpEx.Rotate( aAttr.GetRotation(), Color( COL_TRANSPARENT ) );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        AlphaMask aAlpha;
        sal_uInt8 cTrans = aAttr.GetTransparency();

        if( !rBmpEx.IsTransparent() )
        {
            aAlpha = AlphaMask( rBmpEx.GetSizePixel(), &cTrans );
        }
        else if( !rBmpEx.IsAlpha() )
        {
            aAlpha = rBmpEx.GetMask();
            aAlpha.Replace( 0, cTrans );
        }
        else
        {
            aAlpha = rBmpEx.GetAlpha();
            BitmapWriteAccess* pA = aAlpha.AcquireWriteAccess();

            if( pA )
            {
                sal_uLong nTrans = cTrans, nNewTrans;
                const long  nWidth = pA->Width(), nHeight = pA->Height();

                if( pA->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                {
                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pAScan = pA->GetScanline( nY );

                        for( long nX = 0; nX < nWidth; nX++ )
                        {
                            nNewTrans = nTrans + *pAScan;
                            *pAScan++ = (sal_uInt8) ( ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans );
                        }
                    }
                }
                else
                {
                    BitmapColor aAlphaValue( 0 );

                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        for( long nX = 0; nX < nWidth; nX++ )
                        {
                            nNewTrans = nTrans + pA->GetPixel( nY, nX ).GetIndex();
                            aAlphaValue.SetIndex( (sal_uInt8) ( ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans ) );
                            pA->SetPixel( nY, nX, aAlphaValue );
                        }
                    }
                }

                aAlpha.ReleaseAccess( pA );
            }
        }

        rBmpEx = BitmapEx( rBmpEx.GetBitmap(), aAlpha );
    }
}

void GraphicManager::Adjust( GDIMetaFile& rMtf, const GraphicAttr& rAttr, sal_uInt32 nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
        case GRAPHICDRAWMODE_MONO:
            rMtf.Convert( MTF_CONVERSION_1BIT_THRESHOLD );
            break;

        case GRAPHICDRAWMODE_GREYS:
            rMtf.Convert( MTF_CONVERSION_8BIT_GREYS );
            break;

        case GRAPHICDRAWMODE_WATERMARK:
            aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
            aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            break;

        default:
            break;
        }
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_COLORS ) && aAttr.IsAdjusted() )
    {
        rMtf.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                     aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                     aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_MIRROR ) && aAttr.IsMirrored() )
    {
        rMtf.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        rMtf.Rotate( aAttr.GetRotation() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        OSL_FAIL( "Missing implementation: Mtf-Transparency" );
    }
}

void GraphicManager::Adjust( Animation& rAnimation, const GraphicAttr& rAttr, sal_uInt32 nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
        case GRAPHICDRAWMODE_MONO:
            rAnimation.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            break;

        case GRAPHICDRAWMODE_GREYS :
            rAnimation.Convert( BMP_CONVERSION_8BIT_GREYS );
            break;

        case GRAPHICDRAWMODE_WATERMARK:
            aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
            aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            break;

        default:
            break;
        }
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_COLORS ) && aAttr.IsAdjusted() )
    {
        rAnimation.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                           aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                           aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_MIRROR ) && aAttr.IsMirrored() )
    {
        rAnimation.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        OSL_FAIL( "Missing implementation: Animation-Rotation" );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        OSL_FAIL( "Missing implementation: Animation-Transparency" );
    }
}

void GraphicManager::Draw( OutputDevice* pOutDev, const Point& rPoint, const Size& rSize,
                           GDIMetaFile& rMtf, const GraphicAttr& rAttr )
{
    sal_uInt16   nRot10 = rAttr.GetRotation() % 3600;
    Point   aOutPoint( rPoint );
    Size    aOutSize( rSize );

    if( nRot10 )
    {
        Polygon aPoly( Rectangle( aOutPoint, aOutSize ) );

        aPoly.Rotate( aOutPoint, nRot10 );
        const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
        aOutPoint = aRotBoundRect.TopLeft();
        aOutSize = aRotBoundRect.GetSize();
    }

    pOutDev->Push( PUSH_CLIPREGION );
    pOutDev->IntersectClipRegion( Rectangle( aOutPoint, aOutSize ) );

    rMtf.WindStart();
    rMtf.Play( pOutDev, aOutPoint, aOutSize );
    rMtf.WindStart();

    pOutDev->Pop();
}
