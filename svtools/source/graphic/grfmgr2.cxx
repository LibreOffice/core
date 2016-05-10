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

#include <cstdlib>

#include <vcl/bitmapaccess.hxx>
#include <tools/poly.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/metric.hxx>
#include <vcl/animate.hxx>
#include <vcl/alpha.hxx>
#include <vcl/virdev.hxx>
#include "grfcache.hxx"
#include <svtools/grfmgr.hxx>
#include <memory>

// - defines -

#define WATERMARK_LUM_OFFSET        50
#define WATERMARK_CON_OFFSET        -70
#define MAP( cVal0, cVal1, nFrac )  ((sal_uInt8)((((long)(cVal0)<<20L)+nFrac*((long)(cVal1)-(cVal0)))>>20L))


// - GraphicManager -


GraphicManager::GraphicManager( sal_uLong nCacheSize, sal_uLong nMaxObjCacheSize ) :
    mnUsedSize(0),
    mpCache( new GraphicCache( nCacheSize, nMaxObjCacheSize ) )
{
}

GraphicManager::~GraphicManager()
{
    for( size_t i = 0, n = maObjList.size(); i < n; ++i )
        maObjList[ i ]->GraphicManagerDestroyed();

    delete mpCache;
}

void GraphicManager::SetMaxCacheSize( sal_uLong nNewCacheSize )
{
    mpCache->SetMaxDisplayCacheSize( nNewCacheSize );
}

sal_uLong GraphicManager::GetMaxCacheSize() const
{
    return mpCache->GetMaxDisplayCacheSize();
}

void GraphicManager::SetMaxObjCacheSize( sal_uLong nNewMaxObjSize, bool bDestroyGreaterCached )
{
    mpCache->SetMaxObjDisplayCacheSize( nNewMaxObjSize, bDestroyGreaterCached );
}

void GraphicManager::SetCacheTimeout( sal_uLong nTimeoutSeconds )
{
    mpCache->SetCacheTimeout( nTimeoutSeconds );
}

bool GraphicManager::IsInCache( OutputDevice* pOut, const Point& rPt,
                                    const Size& rSz, const GraphicObject& rObj,
                                    const GraphicAttr& rAttr ) const
{
    return mpCache->IsInDisplayCache( pOut, rPt, rSz, rObj, rAttr );
}

bool GraphicManager::DrawObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                              GraphicObject& rObj, const GraphicAttr& rAttr,
                              const GraphicManagerDrawFlags nFlags, bool& rCached )
{
    Point   aPt( rPt );
    Size    aSz( rSz );
    bool    bRet = false;

    rCached = false;

    if( ( rObj.GetType() == GRAPHIC_BITMAP ) || ( rObj.GetType() == GRAPHIC_GDIMETAFILE ) )
    {
        // create output and fill cache

        if( rObj.IsAnimated() || ( pOut->GetOutDevType() == OUTDEV_PRINTER ) ||
            ( !( nFlags & GraphicManagerDrawFlags::NO_SUBSTITUTE ) &&
              ( ( nFlags & GraphicManagerDrawFlags::SUBSTITUTE ) ||
                !( nFlags & GraphicManagerDrawFlags::CACHED ) ||
                ( pOut->GetConnectMetaFile() && !pOut->IsOutputEnabled() ) ) ) )
        {
            // simple output of transformed graphic
            const Graphic aGraphic( rObj.GetTransformedGraphic( &rAttr ) );

            if( aGraphic.IsSupportedGraphic() )
            {
                const sal_uInt16 nRot10 = rAttr.GetRotation() % 3600;

                if( nRot10 )
                {
                    tools::Polygon aPoly( Rectangle( aPt, aSz ) );

                    aPoly.Rotate( aPt, nRot10 );
                    const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
                    aPt = aRotBoundRect.TopLeft();
                    aSz = aRotBoundRect.GetSize();
                }

                aGraphic.Draw( pOut, aPt, aSz );
            }

            bRet = true;
        }

        if( !bRet )
        {
            // cached/direct drawing
            if( !mpCache->DrawDisplayCacheObj( pOut, aPt, aSz, rObj, rAttr ) )
                bRet = ImplDraw( pOut, aPt, aSz, rObj, rAttr, nFlags, rCached );
            else
                bRet = rCached = true;
        }
    }

    return bRet;
}

void GraphicManager::ImplRegisterObj( const GraphicObject& rObj, Graphic& rSubstitute,
                                      const OString* pID, const GraphicObject* pCopyObj )
{
    assert(std::find(maObjList.begin(), maObjList.end(),
               const_cast<GraphicObject*>(&rObj)) == maObjList.end());

    maObjList.push_back( const_cast<GraphicObject*>(&rObj) );
    mpCache->AddGraphicObject( rObj, rSubstitute, pID, pCopyObj );
    if( !rObj.IsSwappedOut() )
        mnUsedSize += rObj.maGraphic.GetSizeBytes();
}

void GraphicManager::ImplUnregisterObj( const GraphicObject& rObj )
{
    mpCache->ReleaseGraphicObject( rObj );
    if( !rObj.IsSwappedOut() )
    {
        assert(mnUsedSize >= rObj.maGraphic.GetSizeBytes());
        mnUsedSize -= rObj.maGraphic.GetSizeBytes();
    }
    for( GraphicObjectList_impl::iterator it = maObjList.begin(); it != maObjList.end(); ++it )
    {
        if ( *it == &rObj ) {
            maObjList.erase( it );
            return;
        }
    }
    assert(false); // surely it should have been registered?
}

void GraphicManager::ImplGraphicObjectWasSwappedOut( const GraphicObject& rObj )
{
    mpCache->GraphicObjectWasSwappedOut( rObj );
    assert(mnUsedSize >= rObj.GetSizeBytes());
    mnUsedSize -= rObj.GetSizeBytes();
}

OString GraphicManager::ImplGetUniqueID( const GraphicObject& rObj ) const
{
    return mpCache->GetUniqueID( rObj );
}

namespace
{
    struct simpleSortByDataChangeTimeStamp
    {
        bool operator() (GraphicObject* p1, GraphicObject* p2) const
        {
            return p1->GetDataChangeTimeStamp() < p2->GetDataChangeTimeStamp();
        }
    };
} // end of anonymous namespace

void GraphicManager::ImplCheckSizeOfSwappedInGraphics(const GraphicObject* pGraphicToIgnore)
{
    // detect maximum allowed memory footprint. Use the user-settings of MaxCacheSize (defaulted
    // to 200MB).
    const sal_uLong nMaxCacheSize(GetMaxCacheSize());

    if(mnUsedSize > nMaxCacheSize)
    {
        // Copy the object list for now, because maObjList can change in the meantime unexpectedly.
        std::vector< GraphicObject* > aCandidates(maObjList.begin(), maObjList.end());
        // if we use more currently, sort by last DataChangeTimeStamp
        // sort by DataChangeTimeStamp so that the oldest get removed first
        ::std::sort(aCandidates.begin(), aCandidates.end(), simpleSortByDataChangeTimeStamp());

        for(sal_uInt32 a(0); mnUsedSize >= nMaxCacheSize && a < aCandidates.size(); a++)
        {
            // swap out until we have no more or the goal to use less than nMaxCacheSize
            // is reached
            GraphicObject* pObj = aCandidates[a];
            if( pObj == pGraphicToIgnore )
            {
                continue;
            }
            if (std::find(maObjList.begin(), maObjList.end(), pObj) == maObjList.end())
            {
                // object has been deleted when swapping out another one
                continue;
            }

            // do not swap out when we have less than 16KB data objects
            if(pObj->GetSizeBytes() >= (16 * 1024))
            {
                pObj->FireSwapOutRequest();
            }
        }
    }
}

void GraphicManager::ImplGraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    mpCache->GraphicObjectWasSwappedIn( rObj );
    mnUsedSize += rObj.maGraphic.GetSizeBytes();
}

bool GraphicManager::ImplDraw( OutputDevice* pOut, const Point& rPt,
                               const Size& rSz, GraphicObject& rObj,
                               const GraphicAttr& rAttr,
                               const GraphicManagerDrawFlags nFlags, bool& rCached )
{
    const Graphic&  rGraphic = rObj.GetGraphic();
    bool            bRet = false;

    if( rGraphic.IsSupportedGraphic() && !rObj.IsSwappedOut() )
    {
        if( GRAPHIC_BITMAP == rGraphic.GetType() )
        {
            const BitmapEx aSrcBmpEx( rGraphic.GetBitmapEx() );

            // #i46805# No point in caching a bitmap that is rendered
            // via RectFill on the OutDev
            if( !(pOut->GetDrawMode() & ( DrawModeFlags::BlackBitmap | DrawModeFlags::WhiteBitmap )) &&
                mpCache->IsDisplayCacheable( pOut, rPt, rSz, rObj, rAttr ) )
            {
                BitmapEx aDstBmpEx;

                if( ImplCreateOutput( pOut, rPt, rSz, aSrcBmpEx, rAttr, nFlags, &aDstBmpEx ) )
                {
                    rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstBmpEx );
                    bRet = true;
                }
            }

            if( !bRet )
                bRet = ImplCreateOutput( pOut, rPt, rSz, aSrcBmpEx, rAttr, nFlags );
        }
        else
        {
            const GDIMetaFile& rSrcMtf = rGraphic.GetGDIMetaFile();

            if( mpCache->IsDisplayCacheable( pOut, rPt, rSz, rObj, rAttr ) )
            {
                GDIMetaFile aDstMtf;
                BitmapEx    aContainedBmpEx;

                if( ImplCreateOutput( pOut, rPt, rSz, rSrcMtf, rAttr, nFlags, aDstMtf, aContainedBmpEx ) )
                {
                    if( !!aContainedBmpEx )
                    {
                        // Use bitmap output method, if metafile basically contains only a single
                        // bitmap (allows caching the resulting pixmap).
                        BitmapEx aDstBmpEx;

                        if( ImplCreateOutput( pOut, rPt, rSz, aContainedBmpEx, rAttr, nFlags, &aDstBmpEx ) )
                        {
                            rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstBmpEx );
                            bRet = true;
                        }
                    }
                    else
                    {
                        rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstMtf );
                        bRet = true;
                    }
                }
            }

            if( !bRet )
            {
                const Graphic aGraphic( rObj.GetTransformedGraphic( &rAttr ) );

                if( aGraphic.IsSupportedGraphic() )
                {
                    aGraphic.Draw( pOut, rPt, rSz );
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}

bool ImplCreateRotatedScaled( const BitmapEx& rBmpEx, const GraphicAttr& rAttributes,
                                sal_uInt16 nRot10, const Size& rUnrotatedSzPix,
                                long nStartX, long nEndX, long nStartY, long nEndY,
                                BitmapEx& rOutBmpEx )
{
    const long  aUnrotatedWidth  = rUnrotatedSzPix.Width();
    const long  aUnrotatedHeight = rUnrotatedSzPix.Height();
    const long  aBitmapWidth  = rBmpEx.GetSizePixel().Width();
    const long  aBitmapHeight = rBmpEx.GetSizePixel().Height();

    long    nTmpX, nTmpY, nTmpFX, nTmpFY, nTmp;
    double  fTmp;

    bool    bHMirr( rAttributes.GetMirrorFlags() & BmpMirrorFlags::Horizontal );
    bool    bVMirr( rAttributes.GetMirrorFlags() & BmpMirrorFlags::Vertical );

    std::unique_ptr<long[]> pMapIX(new long[ aUnrotatedWidth ]);
    std::unique_ptr<long[]> pMapFX(new long[ aUnrotatedWidth ]);
    std::unique_ptr<long[]> pMapIY(new long[ aUnrotatedHeight ]);
    std::unique_ptr<long[]> pMapFY(new long[ aUnrotatedHeight ]);

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
    std::unique_ptr<long[]> pCosX(new long[ aTargetWidth ]);
    std::unique_ptr<long[]> pSinX(new long[ aTargetWidth ]);
    std::unique_ptr<long[]> pCosY(new long[ aTargetHeight ]);
    std::unique_ptr<long[]> pSinY(new long[ aTargetHeight ]);
    long                nUnRotX, nUnRotY, nSinY, nCosY;
    sal_uInt8           cR0, cG0, cB0, cR1, cG1, cB1;
    bool                bRet = false;

    tools::Polygon aPoly( Rectangle( Point(), rUnrotatedSzPix ) );
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

            Bitmap::ReleaseAccess( pWriteAccess );
            bRet = true;
        }

        Bitmap::ReleaseAccess( pReadAccess );
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

                            for( long nY = 0; nY < aTargetHeight; nY++ )
                            {
                                nSinY = pSinY[ nY ], nCosY = pCosY[ nY ];
                                pLineW = pWriteAccess->GetScanline( nY );

                                for( long nX = 0; nX < aTargetWidth; nX++ )
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

                        for( long nY = 0; nY < aTargetHeight; nY++ )
                        {
                            nSinY = pSinY[ nY ], nCosY = pCosY[ nY ];

                            for( long nX = 0; nX < aTargetWidth; nX++ )
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
                    bRet = true;
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
                BitmapReadAccess*   pMAcc = nullptr;

                if( !aMsk || ( ( pMAcc = aMsk.AcquireReadAccess() ) != nullptr ) )
                {
                    std::unique_ptr<long[]> pMapLX(new long[ aUnrotatedWidth ]);
                    std::unique_ptr<long[]> pMapLY(new long[ aUnrotatedHeight ]);
                    BitmapColor aTestB;

                    if( pMAcc )
                        aTestB = pMAcc->GetBestMatchingColor( Color( COL_BLACK ) );

                    // create new horizontal mapping table
                    for( long nX = 0UL; nX < aUnrotatedWidth; nX++ )
                        pMapLX[ nX ] = FRound( (double) pMapIX[ nX ] + pMapFX[ nX ] / 1048576.0 );

                    // create new vertical mapping table
                    for( long nY = 0UL; nY < aUnrotatedHeight; nY++ )
                        pMapLY[ nY ] = FRound( (double) pMapIY[ nY ] + pMapFY[ nY ] / 1048576.0 );

                    // do mask rotation
                    for( long nY = 0; nY < aTargetHeight; nY++ )
                    {
                        nSinY = pSinY[ nY ];
                        nCosY = pCosY[ nY ];

                        for( long nX = 0; nX < aTargetWidth; nX++ )
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

                    pMapLX.reset();
                    pMapLY.reset();

                    if( pMAcc )
                        Bitmap::ReleaseAccess( pMAcc );

                    bRet = true;
                }

                Bitmap::ReleaseAccess( pWriteAccess );
            }

            if( bRet )
                rOutBmpEx = BitmapEx( aOutBmp, aOutMsk );
        }

        if( !bRet )
            rOutBmpEx = aOutBmp;
    }
    else
        rOutBmpEx = aOutBmp;

    return bRet;
}

bool GraphicManager::ImplCreateOutput( OutputDevice* pOutputDevice,
                                       const Point& rPoint, const Size& rSize,
                                       const BitmapEx& rBitmapEx, const GraphicAttr& rAttributes,
                                       const GraphicManagerDrawFlags /*nFlags*/, BitmapEx* pBmpEx )
{
    sal_uInt16  nRot10 = rAttributes.GetRotation() % 3600;

    Point   aOutputPointPix;
    Size    aOutputSizePix;
    Point   aUnrotatedPointPix( pOutputDevice->LogicToPixel( rPoint ) );
    Size    aUnrotatedSizePix(  pOutputDevice->LogicToPixel( rSize ) );

    bool    bRet = false;

    if( nRot10 )
    {
        tools::Polygon aPoly( Rectangle( rPoint, rSize ) );
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
        bool            bHMirr( rAttributes.GetMirrorFlags() & BmpMirrorFlags::Horizontal );
        bool            bVMirr( rAttributes.GetMirrorFlags() & BmpMirrorFlags::Vertical );

        // calculate output sizes
        if( !pBmpEx )
        {
            Point       aPt;
            Rectangle   aOutRect( aPt, pOutputDevice->GetOutputSizePixel() );
            Rectangle   aBmpRect( aOutputPointPix, aOutputSizePix );

            if( pOutputDevice->GetOutDevType() == OUTDEV_WINDOW )
            {
                const vcl::Region aPaintRgn( static_cast<vcl::Window*>( pOutputDevice )->GetPaintRegion() );
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
                    ImplAdjust( aOutBmpEx, rAttributes, GraphicAdjustmentFlags::DRAWMODE | GraphicAdjustmentFlags::COLORS | GraphicAdjustmentFlags::TRANSPARENCY );

                // OutDev adjustment if necessary
                if( pOutputDevice->GetOutDevType() != OUTDEV_PRINTER && pOutputDevice->GetBitCount() <= 8 && aOutBmpEx.GetBitCount() >= 8 )
                    aOutBmpEx.Dither();
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
            && std::abs( rDestSize.Width() - rRefSize.Width()) < 5
            && std::abs( rDestSize.Height() - rRefSize.Height()) < 5 )
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

bool GraphicManager::ImplCreateOutput( OutputDevice* pOut,
                                       const Point& rPt, const Size& rSz,
                                       const GDIMetaFile& rMtf, const GraphicAttr& rAttr,
                                       const GraphicManagerDrawFlags /*nFlags*/, GDIMetaFile& rOutMtf, BitmapEx& rOutBmpEx )
{
    const Size aNewSize( rMtf.GetPrefSize() );

    rOutMtf = rMtf;

    // Count bitmap actions, and flag actions that paint, but
    // are no bitmaps.
    sal_Int32   nNumBitmaps(0);
    bool        bNonBitmapActionEncountered(false);
    if( aNewSize.Width() && aNewSize.Height() && rSz.Width() && rSz.Height() )
    {
        const double fGrfWH = (double) aNewSize.Width() / aNewSize.Height();
        const double fOutWH = (double) rSz.Width() / rSz.Height();

        const double fScaleX = fOutWH / fGrfWH;
        const double fScaleY = 1.0;

        const MapMode rPrefMapMode( rMtf.GetPrefMapMode() );
        const Size rSizePix( pOut->LogicToPixel( aNewSize, rPrefMapMode ) );

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
        for( nCurPos = 0, pAct = rOutMtf.FirstAction(); pAct;
             pAct = rOutMtf.NextAction(), nCurPos++ )
        {
            MetaAction* pModAct = nullptr;
            switch( pAct->GetType() )
            {
                case MetaActionType::FONT:
                {
                    // taking care of font width default if scaling metafile.
                    MetaFontAction* pA = static_cast<MetaFontAction*>(pAct);
                    vcl::Font aFont( pA->GetFont() );
                    if ( !aFont.GetWidth() )
                    {
                        FontMetric aFontMetric( pOut->GetFontMetric( aFont ) );
                        aFont.SetWidth( aFontMetric.GetWidth() );
                        pModAct = new MetaFontAction( aFont );
                    }
                    SAL_FALLTHROUGH;
                }
                case MetaActionType::NONE:
                    SAL_FALLTHROUGH;

                    // OutDev state changes (which don't affect bitmap
                    // output)
                case MetaActionType::LINECOLOR:
                    SAL_FALLTHROUGH;
                case MetaActionType::FILLCOLOR:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTCOLOR:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTFILLCOLOR:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTALIGN:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTLINECOLOR:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTLINE:
                    SAL_FALLTHROUGH;
                case MetaActionType::PUSH:
                    SAL_FALLTHROUGH;
                case MetaActionType::POP:
                    SAL_FALLTHROUGH;
                case MetaActionType::LAYOUTMODE:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTLANGUAGE:
                    SAL_FALLTHROUGH;
                case MetaActionType::COMMENT:
                    break;

                    // bitmap output methods
                case MetaActionType::BMP:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpAction* pAction = static_cast<MetaBmpAction*>(pAct);

                        rOutBmpEx = checkMetadataBitmap(
                            BitmapEx( pAction->GetBitmap()),
                            Point(), Size(),
                            pOut->LogicToPixel( pAction->GetPoint(),
                                                rPrefMapMode ),
                            pAction->GetBitmap().GetSizePixel(),
                            rSizePix,
                            bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case MetaActionType::BMPSCALE:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpScaleAction* pAction = static_cast<MetaBmpScaleAction*>(pAct);

                        rOutBmpEx = checkMetadataBitmap(
                            BitmapEx( pAction->GetBitmap()),
                            Point(), Size(),
                            pOut->LogicToPixel( pAction->GetPoint(),
                                                rPrefMapMode ),
                            pOut->LogicToPixel( pAction->GetSize(),
                                                rPrefMapMode ),
                            rSizePix,
                            bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case MetaActionType::BMPSCALEPART:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpScalePartAction* pAction = static_cast<MetaBmpScalePartAction*>(pAct);

                        rOutBmpEx = checkMetadataBitmap(
                                                    BitmapEx( pAction->GetBitmap() ),
                                                    pAction->GetSrcPoint(),
                                                    pAction->GetSrcSize(),
                                                    pOut->LogicToPixel( pAction->GetDestPoint(),
                                                                        rPrefMapMode ),
                                                    pOut->LogicToPixel( pAction->GetDestSize(),
                                                                        rPrefMapMode ),
                                                    rSizePix,
                                                    bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case MetaActionType::BMPEX:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExAction* pAction = static_cast<MetaBmpExAction*>(pAct);

                        rOutBmpEx = checkMetadataBitmap(
                            pAction->GetBitmapEx(),
                            Point(), Size(),
                            pOut->LogicToPixel( pAction->GetPoint(),
                                                rPrefMapMode ),
                            pAction->GetBitmapEx().GetSizePixel(),
                            rSizePix,
                            bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case MetaActionType::BMPEXSCALE:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExScaleAction* pAction = static_cast<MetaBmpExScaleAction*>(pAct);

                        rOutBmpEx = checkMetadataBitmap(
                            pAction->GetBitmapEx(),
                            Point(), Size(),
                            pOut->LogicToPixel( pAction->GetPoint(),
                                                rPrefMapMode ),
                            pOut->LogicToPixel( pAction->GetSize(),
                                                rPrefMapMode ),
                            rSizePix,
                            bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                case MetaActionType::BMPEXSCALEPART:
                    if( !nNumBitmaps && !bNonBitmapActionEncountered )
                    {
                        MetaBmpExScalePartAction* pAction = static_cast<MetaBmpExScalePartAction*>(pAct);

                        rOutBmpEx = checkMetadataBitmap( pAction->GetBitmapEx(),
                                                    pAction->GetSrcPoint(),
                                                    pAction->GetSrcSize(),
                                                    pOut->LogicToPixel( pAction->GetDestPoint(),
                                                                        rPrefMapMode ),
                                                    pOut->LogicToPixel( pAction->GetDestSize(),
                                                                        rPrefMapMode ),
                                                    rSizePix,
                                                    bNonBitmapActionEncountered );
                    }
                    ++nNumBitmaps;
                    break;

                    // these actions actually output something (that's
                    // different from a bitmap)
                case MetaActionType::RASTEROP:
                    if( static_cast<MetaRasterOpAction*>(pAct)->GetRasterOp() == ROP_OVERPAINT )
                        break;
                    SAL_FALLTHROUGH;
                case MetaActionType::PIXEL:
                    SAL_FALLTHROUGH;
                case MetaActionType::POINT:
                    SAL_FALLTHROUGH;
                case MetaActionType::LINE:
                    SAL_FALLTHROUGH;
                case MetaActionType::RECT:
                    SAL_FALLTHROUGH;
                case MetaActionType::ROUNDRECT:
                    SAL_FALLTHROUGH;
                case MetaActionType::ELLIPSE:
                    SAL_FALLTHROUGH;
                case MetaActionType::ARC:
                    SAL_FALLTHROUGH;
                case MetaActionType::PIE:
                    SAL_FALLTHROUGH;
                case MetaActionType::CHORD:
                    SAL_FALLTHROUGH;
                case MetaActionType::POLYLINE:
                    SAL_FALLTHROUGH;
                case MetaActionType::POLYGON:
                    SAL_FALLTHROUGH;
                case MetaActionType::POLYPOLYGON:
                    SAL_FALLTHROUGH;

                case MetaActionType::TEXT:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTARRAY:
                    SAL_FALLTHROUGH;
                case MetaActionType::STRETCHTEXT:
                    SAL_FALLTHROUGH;
                case MetaActionType::TEXTRECT:
                    SAL_FALLTHROUGH;

                case MetaActionType::MASK:
                    SAL_FALLTHROUGH;
                case MetaActionType::MASKSCALE:
                    SAL_FALLTHROUGH;
                case MetaActionType::MASKSCALEPART:
                    SAL_FALLTHROUGH;

                case MetaActionType::GRADIENT:
                    SAL_FALLTHROUGH;
                case MetaActionType::HATCH:
                    SAL_FALLTHROUGH;
                case MetaActionType::WALLPAPER:
                    SAL_FALLTHROUGH;

                case MetaActionType::Transparent:
                    SAL_FALLTHROUGH;
                case MetaActionType::EPS:
                    SAL_FALLTHROUGH;
                case MetaActionType::FLOATTRANSPARENT:
                    SAL_FALLTHROUGH;
                case MetaActionType::GRADIENTEX:
                    SAL_FALLTHROUGH;

                    // OutDev state changes that _do_ affect bitmap
                    // output
                case MetaActionType::CLIPREGION:
                    SAL_FALLTHROUGH;
                case MetaActionType::ISECTRECTCLIPREGION:
                    SAL_FALLTHROUGH;
                case MetaActionType::ISECTREGIONCLIPREGION:
                    SAL_FALLTHROUGH;
                case MetaActionType::MOVECLIPREGION:
                    SAL_FALLTHROUGH;

                case MetaActionType::MAPMODE:
                    SAL_FALLTHROUGH;
                case MetaActionType::REFPOINT:
                    SAL_FALLTHROUGH;
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
                    pModAct = pAct;
            }
            pModAct->Scale( fScaleX, fScaleY );
        }
        rOutMtf.SetPrefSize( Size( FRound( aNewSize.Width() * fScaleX ),
                                   FRound( aNewSize.Height() * fScaleY ) ) );
    }

    if( nNumBitmaps != 1 || bNonBitmapActionEncountered )
    {
        if( rAttr.IsSpecialDrawMode() || rAttr.IsAdjusted() || rAttr.IsMirrored() || rAttr.IsRotated() || rAttr.IsTransparent() )
            ImplAdjust( rOutMtf, rAttr, GraphicAdjustmentFlags::ALL );

        ImplDraw( pOut, rPt, rSz, rOutMtf, rAttr );
        rOutBmpEx = BitmapEx();
    }

    return true;
}

void GraphicManager::ImplAdjust( BitmapEx& rBmpEx, const GraphicAttr& rAttr, GraphicAdjustmentFlags nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case( GRAPHICDRAWMODE_MONO ):
                rBmpEx.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            break;

            case( GRAPHICDRAWMODE_GREYS ):
                rBmpEx.Convert( BMP_CONVERSION_8BIT_GREYS );
            break;

            case( GRAPHICDRAWMODE_WATERMARK ):
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::COLORS ) && aAttr.IsAdjusted() )
    {
        rBmpEx.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                       aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                       aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::MIRROR ) && aAttr.IsMirrored() )
    {
        rBmpEx.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::ROTATE ) && aAttr.IsRotated() )
    {
        rBmpEx.Rotate( aAttr.GetRotation(), Color( COL_TRANSPARENT ) );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        AlphaMask   aAlpha;
        sal_uInt8       cTrans = aAttr.GetTransparency();

        if( !rBmpEx.IsTransparent() )
            aAlpha = AlphaMask( rBmpEx.GetSizePixel(), &cTrans );
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
                sal_uLong       nTrans = cTrans, nNewTrans;
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

void GraphicManager::ImplAdjust( GDIMetaFile& rMtf, const GraphicAttr& rAttr, GraphicAdjustmentFlags nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case( GRAPHICDRAWMODE_MONO ):
                rMtf.Convert( MTF_CONVERSION_1BIT_THRESHOLD );
            break;

            case( GRAPHICDRAWMODE_GREYS ):
                rMtf.Convert( MTF_CONVERSION_8BIT_GREYS );
            break;

            case( GRAPHICDRAWMODE_WATERMARK ):
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::COLORS ) && aAttr.IsAdjusted() )
    {
        rMtf.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                     aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                     aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::MIRROR ) && aAttr.IsMirrored() )
    {
        rMtf.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::ROTATE ) && aAttr.IsRotated() )
    {
        rMtf.Rotate( aAttr.GetRotation() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        OSL_FAIL( "Missing implementation: Mtf-Transparency" );
    }
}

void GraphicManager::ImplAdjust( Animation& rAnimation, const GraphicAttr& rAttr, GraphicAdjustmentFlags nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case( GRAPHICDRAWMODE_MONO ):
                rAnimation.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            break;

            case( GRAPHICDRAWMODE_GREYS ):
                rAnimation.Convert( BMP_CONVERSION_8BIT_GREYS );
            break;

            case( GRAPHICDRAWMODE_WATERMARK ):
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::COLORS ) && aAttr.IsAdjusted() )
    {
        rAnimation.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                           aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                           aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::MIRROR ) && aAttr.IsMirrored() )
    {
        rAnimation.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::ROTATE ) && aAttr.IsRotated() )
    {
        OSL_FAIL( "Missing implementation: Animation-Rotation" );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        OSL_FAIL( "Missing implementation: Animation-Transparency" );
    }
}

void GraphicManager::ImplDraw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                               const GDIMetaFile& rMtf, const GraphicAttr& rAttr )
{
       sal_uInt16   nRot10 = rAttr.GetRotation() % 3600;
    Point   aOutPt( rPt );
    Size    aOutSz( rSz );

    if( nRot10 )
    {
        tools::Polygon aPoly( Rectangle( aOutPt, aOutSz ) );

        aPoly.Rotate( aOutPt, nRot10 );
        const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
        aOutPt = aRotBoundRect.TopLeft();
        aOutSz = aRotBoundRect.GetSize();
    }

    pOut->Push( PushFlags::CLIPREGION );
    pOut->IntersectClipRegion( Rectangle( aOutPt, aOutSz ) );

    ( (GDIMetaFile&) rMtf ).WindStart();
    ( (GDIMetaFile&) rMtf ).Play( pOut, aOutPt, aOutSz );
    ( (GDIMetaFile&) rMtf ).WindStart();

    pOut->Pop();
}

struct ImplTileInfo
{
    ImplTileInfo() : aTileTopLeft(), aNextTileTopLeft(), aTileSizePixel(), nTilesEmptyX(0), nTilesEmptyY(0) {}

    Point aTileTopLeft;     // top, left position of the rendered tile
    Point aNextTileTopLeft; // top, left position for next recursion
                            // level's tile
    Size  aTileSizePixel;   // size of the generated tile (might
                            // differ from
                            // aNextTileTopLeft-aTileTopLeft, because
                            // this is nExponent*prevTileSize. The
                            // generated tile is always nExponent
                            // times the previous tile, such that it
                            // can be used in the next stage. The
                            // required area coverage is often
                            // less. The extraneous area covered is
                            // later overwritten by the next stage)
    int   nTilesEmptyX;     // number of original tiles empty right of
                            // this tile. This counts from
                            // aNextTileTopLeft, i.e. the additional
                            // area covered by aTileSizePixel is not
                            // considered here. This is for
                            // unification purposes, as the iterative
                            // calculation of the next level's empty
                            // tiles has to be based on this value.
    int   nTilesEmptyY;     // as above, for Y
};


bool GraphicObject::ImplRenderTempTile( VirtualDevice& rVDev, int nExponent,
                                        int nNumTilesX, int nNumTilesY,
                                        const Size& rTileSizePixel,
                                        const GraphicAttr* pAttr, GraphicManagerDrawFlags nFlags )
{
    if( nExponent <= 1 )
        return false;

    // determine MSB factor
    int nMSBFactor( 1 );
    while( nNumTilesX / nMSBFactor != 0 ||
           nNumTilesY / nMSBFactor != 0 )
    {
        nMSBFactor *= nExponent;
    }

    // one less
    nMSBFactor /= nExponent;

    ImplTileInfo aTileInfo;

    // #105229# Switch off mapping (converting to logic and back to
    // pixel might cause roundoff errors)
    bool bOldMap( rVDev.IsMapModeEnabled() );
    rVDev.EnableMapMode( false );

    bool bRet( ImplRenderTileRecursive( rVDev, nExponent, nMSBFactor, nNumTilesX, nNumTilesY,
                                        nNumTilesX, nNumTilesY, rTileSizePixel, pAttr, nFlags, aTileInfo ) );

    rVDev.EnableMapMode( bOldMap );

    return bRet;
}

// define for debug drawings
//#define DBG_TEST

// see header comment. this works similar to base conversion of a
// number, i.e. if the exponent is 10, then the number for every tile
// size is given by the decimal place of the corresponding decimal
// representation.
bool GraphicObject::ImplRenderTileRecursive( VirtualDevice& rVDev, int nExponent, int nMSBFactor,
                                             int nNumOrigTilesX, int nNumOrigTilesY,
                                             int nRemainderTilesX, int nRemainderTilesY,
                                             const Size& rTileSizePixel, const GraphicAttr* pAttr,
                                             GraphicManagerDrawFlags nFlags, ImplTileInfo& rTileInfo )
{
    // gets loaded with our tile bitmap
    GraphicObject aTmpGraphic;

    // stores a flag that renders the zero'th tile position
    // (i.e. (0,0)+rCurrPos) only if we're at the bottom of the
    // recursion stack. All other position already have that tile
    // rendered, because the lower levels painted their generated tile
    // there.
    bool bNoFirstTileDraw( false );

    // what's left when we're done with our tile size
    const int nNewRemainderX( nRemainderTilesX % nMSBFactor );
    const int nNewRemainderY( nRemainderTilesY % nMSBFactor );

    // gets filled out from the recursive call with info of what's
    // been generated
    ImplTileInfo aTileInfo;

    // current output position while drawing
    Point aCurrPos;
    int nX, nY;

    // check for recursion's end condition: LSB place reached?
    if( nMSBFactor == 1 )
    {
        aTmpGraphic = *this;

        // set initial tile size -> orig size
        aTileInfo.aTileSizePixel = rTileSizePixel;
        aTileInfo.nTilesEmptyX = nNumOrigTilesX;
        aTileInfo.nTilesEmptyY = nNumOrigTilesY;
    }
    else if( ImplRenderTileRecursive( rVDev, nExponent, nMSBFactor/nExponent,
                                      nNumOrigTilesX, nNumOrigTilesY,
                                      nNewRemainderX, nNewRemainderY,
                                      rTileSizePixel, pAttr, nFlags, aTileInfo ) )
    {
        // extract generated tile -> see comment on the first loop below
        BitmapEx aTileBitmap( rVDev.GetBitmap( aTileInfo.aTileTopLeft, aTileInfo.aTileSizePixel ) );

        aTmpGraphic = GraphicObject( aTileBitmap );

        // fill stripes left over from upstream levels:

        //  x0000
        //  0
        //  0
        //  0
        //  0

        // where x denotes the place filled by our recursive predecessors

        // check whether we have to fill stripes here. Although not
        // obvious, there is one case where we can skip this step: if
        // the previous recursion level (the one who filled our
        // aTileInfo) had zero area to fill, then there are no white
        // stripes left, naturally. This happens if the digit
        // associated to that level has a zero, and can be checked via
        // aTileTopLeft==aNextTileTopLeft.
        if( aTileInfo.aTileTopLeft != aTileInfo.aNextTileTopLeft )
        {
            // now fill one row from aTileInfo.aNextTileTopLeft.X() all
            // the way to the right
            aCurrPos.X() = aTileInfo.aNextTileTopLeft.X();
            aCurrPos.Y() = aTileInfo.aTileTopLeft.Y();
            for( nX=0; nX < aTileInfo.nTilesEmptyX; nX += nMSBFactor )
            {
                if( !aTmpGraphic.Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                    return false;

                aCurrPos.X() += aTileInfo.aTileSizePixel.Width();
            }

#ifdef DBG_TEST
//          rVDev.SetFillColor( COL_WHITE );
            rVDev.SetFillColor();
            rVDev.SetLineColor( Color( 255 * nExponent / nMSBFactor, 255 - 255 * nExponent / nMSBFactor, 128 - 255 * nExponent / nMSBFactor ) );
            rVDev.DrawEllipse( Rectangle(aTileInfo.aNextTileTopLeft.X(), aTileInfo.aTileTopLeft.Y(),
                                         aTileInfo.aNextTileTopLeft.X() - 1 + (aTileInfo.nTilesEmptyX/nMSBFactor)*aTileInfo.aTileSizePixel.Width(),
                                         aTileInfo.aTileTopLeft.Y() + aTileInfo.aTileSizePixel.Height() - 1) );
#endif

            // now fill one column from aTileInfo.aNextTileTopLeft.Y() all
            // the way to the bottom
            aCurrPos.X() = aTileInfo.aTileTopLeft.X();
            aCurrPos.Y() = aTileInfo.aNextTileTopLeft.Y();
            for( nY=0; nY < aTileInfo.nTilesEmptyY; nY += nMSBFactor )
            {
                if( !aTmpGraphic.Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                    return false;

                aCurrPos.Y() += aTileInfo.aTileSizePixel.Height();
            }

#ifdef DBG_TEST
            rVDev.DrawEllipse( Rectangle(aTileInfo.aTileTopLeft.X(), aTileInfo.aNextTileTopLeft.Y(),
                                         aTileInfo.aTileTopLeft.X() + aTileInfo.aTileSizePixel.Width() - 1,
                                         aTileInfo.aNextTileTopLeft.Y() - 1 + (aTileInfo.nTilesEmptyY/nMSBFactor)*aTileInfo.aTileSizePixel.Height()) );
#endif
        }
        else
        {
            // Thought that aTileInfo.aNextTileTopLeft tile has always
            // been drawn already, but that's wrong: typically,
            // _parts_ of that tile have been drawn, since the
            // previous level generated the tile there. But when
            // aTileInfo.aNextTileTopLeft!=aTileInfo.aTileTopLeft, the
            // difference between these two values is missing in the
            // lower right corner of this first tile. So, can do that
            // only here.
            bNoFirstTileDraw = true;
        }
    }
    else
    {
        return false;
    }

    // calc number of original tiles in our drawing area without
    // remainder
    nRemainderTilesX -= nNewRemainderX;
    nRemainderTilesY -= nNewRemainderY;

    // fill tile info for calling method
    rTileInfo.aTileTopLeft     = aTileInfo.aNextTileTopLeft;
    rTileInfo.aNextTileTopLeft = Point( rTileInfo.aTileTopLeft.X() + rTileSizePixel.Width()*nRemainderTilesX,
                                        rTileInfo.aTileTopLeft.Y() + rTileSizePixel.Height()*nRemainderTilesY );
    rTileInfo.aTileSizePixel   = Size( rTileSizePixel.Width()*nMSBFactor*nExponent,
                                       rTileSizePixel.Height()*nMSBFactor*nExponent );
    rTileInfo.nTilesEmptyX     = aTileInfo.nTilesEmptyX - nRemainderTilesX;
    rTileInfo.nTilesEmptyY     = aTileInfo.nTilesEmptyY - nRemainderTilesY;

    // init output position
    aCurrPos = aTileInfo.aNextTileTopLeft;

    // fill our drawing area. Fill possibly more, to create the next
    // bigger tile size -> see bitmap extraction above. This does no
    // harm, since everything right or below our actual area is
    // overdrawn by our caller. Just in case we're in the last level,
    // we don't draw beyond the right or bottom border.
    for( nY=0; nY < aTileInfo.nTilesEmptyY && nY < nExponent*nMSBFactor; nY += nMSBFactor )
    {
        aCurrPos.X() = aTileInfo.aNextTileTopLeft.X();

        for( nX=0; nX < aTileInfo.nTilesEmptyX && nX < nExponent*nMSBFactor; nX += nMSBFactor )
        {
            if( bNoFirstTileDraw )
                bNoFirstTileDraw = false; // don't draw first tile position
            else if( !aTmpGraphic.Draw( &rVDev, aCurrPos, aTileInfo.aTileSizePixel, pAttr, nFlags ) )
                return false;

            aCurrPos.X() += aTileInfo.aTileSizePixel.Width();
        }

        aCurrPos.Y() += aTileInfo.aTileSizePixel.Height();
    }

#ifdef DBG_TEST
//  rVDev.SetFillColor( COL_WHITE );
    rVDev.SetFillColor();
    rVDev.SetLineColor( Color( 255 * nExponent / nMSBFactor, 255 - 255 * nExponent / nMSBFactor, 128 - 255 * nExponent / nMSBFactor ) );
    rVDev.DrawRect( Rectangle((rTileInfo.aTileTopLeft.X())*rTileSizePixel.Width(),
                              (rTileInfo.aTileTopLeft.Y())*rTileSizePixel.Height(),
                              (rTileInfo.aNextTileTopLeft.X())*rTileSizePixel.Width()-1,
                              (rTileInfo.aNextTileTopLeft.Y())*rTileSizePixel.Height()-1) );
#endif

    return true;
}

bool GraphicObject::ImplDrawTiled( OutputDevice* pOut, const Rectangle& rArea, const Size& rSizePixel,
                                   const Size& rOffset, const GraphicAttr* pAttr, GraphicManagerDrawFlags nFlags, int nTileCacheSize1D )
{
    // how many tiles to generate per recursion step
    enum{ SubdivisionExponent=2 };

    const MapMode   aOutMapMode( pOut->GetMapMode() );
    const MapMode   aMapMode( aOutMapMode.GetMapUnit(), Point(), aOutMapMode.GetScaleX(), aOutMapMode.GetScaleY() );
    bool            bRet( false );

    // #i42643# Casting to Int64, to avoid integer overflow for
    // huge-DPI output devices
    if( GetGraphic().GetType() == GRAPHIC_BITMAP &&
        static_cast<sal_Int64>(rSizePixel.Width()) * rSizePixel.Height() <
        static_cast<sal_Int64>(nTileCacheSize1D)*nTileCacheSize1D )
    {
        // First combine very small bitmaps into a larger tile


        ScopedVclPtrInstance< VirtualDevice > aVDev;
        const int       nNumTilesInCacheX( (nTileCacheSize1D + rSizePixel.Width()-1) / rSizePixel.Width() );
        const int       nNumTilesInCacheY( (nTileCacheSize1D + rSizePixel.Height()-1) / rSizePixel.Height() );

        aVDev->SetOutputSizePixel( Size( nNumTilesInCacheX*rSizePixel.Width(),
                                        nNumTilesInCacheY*rSizePixel.Height() ) );
        aVDev->SetMapMode( aMapMode );

        // draw bitmap content
        if( ImplRenderTempTile( *aVDev.get(), SubdivisionExponent, nNumTilesInCacheX,
                                nNumTilesInCacheY, rSizePixel, pAttr, nFlags ) )
        {
            BitmapEx aTileBitmap( aVDev->GetBitmap( Point(0,0), aVDev->GetOutputSize() ) );

            // draw alpha content, if any
            if( IsTransparent() )
            {
                GraphicObject aAlphaGraphic;

                if( GetGraphic().IsAlpha() )
                    aAlphaGraphic.SetGraphic( GetGraphic().GetBitmapEx().GetAlpha().GetBitmap() );
                else
                    aAlphaGraphic.SetGraphic( GetGraphic().GetBitmapEx().GetMask() );

                if( aAlphaGraphic.ImplRenderTempTile( *aVDev.get(), SubdivisionExponent, nNumTilesInCacheX,
                                                      nNumTilesInCacheY, rSizePixel, pAttr, nFlags ) )
                {
                    // Combine bitmap and alpha/mask
                    if( GetGraphic().IsAlpha() )
                        aTileBitmap = BitmapEx( aTileBitmap.GetBitmap(),
                                                AlphaMask( aVDev->GetBitmap( Point(0,0), aVDev->GetOutputSize() ) ) );
                    else
                        aTileBitmap = BitmapEx( aTileBitmap.GetBitmap(),
                                                aVDev->GetBitmap( Point(0,0), aVDev->GetOutputSize() ).CreateMask( Color(COL_WHITE) ) );
                }
            }

            // paint generated tile
            GraphicObject aTmpGraphic( aTileBitmap );
            bRet = aTmpGraphic.ImplDrawTiled( pOut, rArea,
                                              aTileBitmap.GetSizePixel(),
                                              rOffset, pAttr, nFlags, nTileCacheSize1D );
        }
    }
    else
    {
        const Size      aOutOffset( pOut->LogicToPixel( rOffset, aOutMapMode ) );
        const Rectangle aOutArea( pOut->LogicToPixel( rArea, aOutMapMode ) );

        // number of invisible (because out-of-area) tiles
        int nInvisibleTilesX;
        int nInvisibleTilesY;

        // round towards -infty for negative offset
        if( aOutOffset.Width() < 0 )
            nInvisibleTilesX = (aOutOffset.Width() - rSizePixel.Width() + 1) / rSizePixel.Width();
        else
            nInvisibleTilesX = aOutOffset.Width() / rSizePixel.Width();

        // round towards -infty for negative offset
        if( aOutOffset.Height() < 0 )
            nInvisibleTilesY = (aOutOffset.Height() - rSizePixel.Height() + 1) / rSizePixel.Height();
        else
            nInvisibleTilesY = aOutOffset.Height() / rSizePixel.Height();

        // origin from where to 'virtually' start drawing in pixel
        const Point aOutOrigin( pOut->LogicToPixel( Point( rArea.Left() - rOffset.Width(),
                                                           rArea.Top() - rOffset.Height() ) ) );
        // position in pixel from where to really start output
        const Point aOutStart( aOutOrigin.X() + nInvisibleTilesX*rSizePixel.Width(),
                               aOutOrigin.Y() + nInvisibleTilesY*rSizePixel.Height() );

        pOut->Push( PushFlags::CLIPREGION );
        pOut->IntersectClipRegion( rArea );

        // Paint all tiles


        bRet = ImplDrawTiled( *pOut, aOutStart,
                              (aOutArea.GetWidth() + aOutArea.Left() - aOutStart.X() + rSizePixel.Width() - 1) / rSizePixel.Width(),
                              (aOutArea.GetHeight() + aOutArea.Top() - aOutStart.Y() + rSizePixel.Height() - 1) / rSizePixel.Height(),
                              rSizePixel, pAttr, nFlags );

        pOut->Pop();
    }

    return bRet;
}

bool GraphicObject::ImplDrawTiled( OutputDevice& rOut, const Point& rPosPixel,
                                   int nNumTilesX, int nNumTilesY,
                                   const Size& rTileSizePixel, const GraphicAttr* pAttr, GraphicManagerDrawFlags nFlags )
{
    Point   aCurrPos( rPosPixel );
    Size    aTileSizeLogic( rOut.PixelToLogic( rTileSizePixel ) );
    int     nX, nY;

    // #107607# Use logical coordinates for metafile playing, too
    bool    bDrawInPixel( rOut.GetConnectMetaFile() == nullptr && GRAPHIC_BITMAP == GetType() );
    bool    bRet = false;

    // #105229# Switch off mapping (converting to logic and back to
    // pixel might cause roundoff errors)
    bool bOldMap( rOut.IsMapModeEnabled() );

    if( bDrawInPixel )
        rOut.EnableMapMode( false );

    for( nY=0; nY < nNumTilesY; ++nY )
    {
        aCurrPos.X() = rPosPixel.X();

        for( nX=0; nX < nNumTilesX; ++nX )
        {
            // #105229# work with pixel coordinates here, mapping is disabled!
            // #104004# don't disable mapping for metafile recordings
            // #108412# don't quit the loop if one draw fails

            // update return value. This method should return true, if
            // at least one of the looped Draws succeeded.
            bRet |= Draw( &rOut,
                          bDrawInPixel ? aCurrPos : rOut.PixelToLogic( aCurrPos ),
                          bDrawInPixel ? rTileSizePixel : aTileSizeLogic,
                          pAttr, nFlags );

            aCurrPos.X() += rTileSizePixel.Width();
        }

        aCurrPos.Y() += rTileSizePixel.Height();
    }

    if( bDrawInPixel )
        rOut.EnableMapMode( bOldMap );

    return bRet;
}

void GraphicObject::ImplTransformBitmap( BitmapEx&          rBmpEx,
                                         const GraphicAttr& rAttr,
                                         const Size&        rCropLeftTop,
                                         const Size&        rCropRightBottom,
                                         const Rectangle&   rCropRect,
                                         const Size&        rDstSize,
                                         bool               bEnlarge ) const
{
    // #107947# Extracted from svdograf.cxx

    // #104115# Crop the bitmap
    if( rAttr.IsCropped() )
    {
        rBmpEx.Crop( rCropRect );

        // #104115# Negative crop sizes mean: enlarge bitmap and pad
        if( bEnlarge && (
            rCropLeftTop.Width() < 0 ||
            rCropLeftTop.Height() < 0 ||
            rCropRightBottom.Width() < 0 ||
            rCropRightBottom.Height() < 0 ) )
        {
            Size aBmpSize( rBmpEx.GetSizePixel() );
            sal_Int32 nPadLeft( rCropLeftTop.Width() < 0 ? -rCropLeftTop.Width() : 0 );
            sal_Int32 nPadTop( rCropLeftTop.Height() < 0 ? -rCropLeftTop.Height() : 0 );
            sal_Int32 nPadTotalWidth( aBmpSize.Width() + nPadLeft + (rCropRightBottom.Width() < 0 ? -rCropRightBottom.Width() : 0) );
            sal_Int32 nPadTotalHeight( aBmpSize.Height() + nPadTop + (rCropRightBottom.Height() < 0 ? -rCropRightBottom.Height() : 0) );

            BitmapEx aBmpEx2;

            if( rBmpEx.IsTransparent() )
            {
                if( rBmpEx.IsAlpha() )
                    aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), rBmpEx.GetAlpha() );
                else
                    aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), rBmpEx.GetMask() );
            }
            else
            {
                // #104115# Generate mask bitmap and init to zero
                Bitmap aMask( aBmpSize, 1 );
                aMask.Erase( Color(0,0,0) );

                // #104115# Always generate transparent bitmap, we need the border transparent
                aBmpEx2 = BitmapEx( rBmpEx.GetBitmap(), aMask );

                // #104115# Add opaque mask to source bitmap, otherwise the destination remains transparent
                rBmpEx = aBmpEx2;
            }

            aBmpEx2.SetSizePixel( Size(nPadTotalWidth, nPadTotalHeight) );
            aBmpEx2.Erase( Color(0xFF,0,0,0) );
            aBmpEx2.CopyPixel( Rectangle( Point(nPadLeft, nPadTop), aBmpSize ), Rectangle( Point(0, 0), aBmpSize ), &rBmpEx );
            rBmpEx = aBmpEx2;
        }
    }

    const Size  aSizePixel( rBmpEx.GetSizePixel() );

    if( rAttr.GetRotation() != 0 && !IsAnimated() )
    {
        if( aSizePixel.Width() && aSizePixel.Height() && rDstSize.Width() && rDstSize.Height() )
        {
            double fSrcWH = (double) aSizePixel.Width() / aSizePixel.Height();
            double fDstWH = (double) rDstSize.Width() / rDstSize.Height();
            double fScaleX = 1.0, fScaleY = 1.0;

            // always choose scaling to shrink bitmap
            if( fSrcWH < fDstWH )
                fScaleY = aSizePixel.Width() / ( fDstWH * aSizePixel.Height() );
            else
                fScaleX = fDstWH * aSizePixel.Height() / aSizePixel.Width();

            rBmpEx.Scale( fScaleX, fScaleY );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
