/*************************************************************************
 *
 *  $RCSfile: grfmgr2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-10-11 15:17:49 $
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

#include <vos/macros.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/poly.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/animate.hxx>
#include <vcl/alpha.hxx>
#include "grfcache.hxx"
#include "grfmgr.hxx"

// -----------
// - defines -
// -----------

#define MAX_PRINTER_EXT             1024
#define MAP( cVal0, cVal1, nFrac )  ((BYTE)((((long)(cVal0)<<20L)+nFrac*((long)(cVal1)-(cVal0)))>>20L))
#define WATERMARK_LUM_OFFSET        50
#define WATERMARK_CON_OFFSET        -70

// ------------------
// - GraphicManager -
// ------------------

GraphicManager::GraphicManager( ULONG nCacheSize, ULONG nMaxObjCacheSize ) :
        mpCache( new GraphicCache( *this, nCacheSize, nMaxObjCacheSize ) )
{
}

// -----------------------------------------------------------------------------

GraphicManager::~GraphicManager()
{
    for( void* pObj = maObjList.First(); pObj; pObj = maObjList.Next() )
        ( (GraphicObject*) pObj )->GraphicManagerDestroyed();

    delete mpCache;
}

// -----------------------------------------------------------------------------

void GraphicManager::SetMaxCacheSize( ULONG nNewCacheSize )
{
    mpCache->SetMaxDisplayCacheSize( nNewCacheSize );
}

// -----------------------------------------------------------------------------

ULONG GraphicManager::GetMaxCacheSize() const
{
    return mpCache->GetMaxDisplayCacheSize();
}

// -----------------------------------------------------------------------------

void GraphicManager::SetMaxObjCacheSize( ULONG nNewMaxObjSize, BOOL bDestroyGreaterCached )
{
    mpCache->SetMaxObjDisplayCacheSize( nNewMaxObjSize, bDestroyGreaterCached );
}

// -----------------------------------------------------------------------------

ULONG GraphicManager::GetMaxObjCacheSize() const
{
    return mpCache->GetMaxObjDisplayCacheSize();
}

// -----------------------------------------------------------------------------

ULONG GraphicManager::GetUsedCacheSize() const
{
    return mpCache->GetUsedDisplayCacheSize();
}

// -----------------------------------------------------------------------------

ULONG GraphicManager::GetFreeCacheSize() const
{
    return mpCache->GetFreeDisplayCacheSize();
}

// -----------------------------------------------------------------------------

void GraphicManager::ClearCache()
{
    mpCache->ClearDisplayCache();
}

// -----------------------------------------------------------------------------

void GraphicManager::ReleaseFromCache( const GraphicObject& rObj )
{
    // !!!
}

// -----------------------------------------------------------------------------

BOOL GraphicManager::IsInCache( OutputDevice* pOut, const Point& rPt,
                                    const Size& rSz, const GraphicObject& rObj,
                                    const GraphicAttr& rAttr ) const
{
    return mpCache->IsInDisplayCache( pOut, rPt, rSz, rObj, rAttr );
}

// -----------------------------------------------------------------------------

BOOL GraphicManager::DrawObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                              GraphicObject& rObj, const GraphicAttr& rAttr,
                              const ULONG nFlags, BOOL& rCached )
{
    GraphicAttr aAttr( rAttr );
    Point       aPt( rPt );
    Size        aSz( rSz );
    BOOL        bRet = FALSE;

    rCached = FALSE;

    // mirrored horizontically
    if( aSz.Width() < 0L )
    {
        aPt.X() += aSz.Width() + 1;
        aSz.Width() = -aSz.Width();
        aAttr.SetMirrorFlags( aAttr.GetMirrorFlags() ^ BMP_MIRROR_HORZ );
    }

    // mirrored vertically
    if( aSz.Height() < 0L )
    {
        aPt.Y() += aSz.Height() + 1;
        aSz.Height() = -aSz.Height();
        aAttr.SetMirrorFlags( aAttr.GetMirrorFlags() ^ BMP_MIRROR_VERT );
    }

    if( ( rObj.GetType() == GRAPHIC_BITMAP ) || ( rObj.GetType() == GRAPHIC_GDIMETAFILE ) )
    {
        const Size aOutSize( pOut->GetOutputSizePixel() );

        // metafile recording?
        if( ( pOut->GetOutDevType() == OUTDEV_PRINTER ) ||
            ( pOut->GetConnectMetaFile() && !pOut->IsOutputEnabled() &&
              ( aOutSize.Width() == 1 ) && ( aOutSize.Height() == 1 ) ) )
        {
            const Graphic aGraphic( rObj.GetTransformedGraphic( &aAttr ) );

            if( aGraphic.IsSupportedGraphic() )
                aGraphic.Draw( pOut, aPt, aSz );

            bRet = TRUE;
        }

        // cached/direct drawing
        if( !bRet )
        {
            if( !mpCache->DrawDisplayCacheObj( pOut, aPt, aSz, rObj, aAttr ) )
                bRet = ImplDraw( pOut, aPt, aSz, rObj, aAttr, rCached );
            else
                bRet = rCached = TRUE;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicManager::ImplRegisterObj( const GraphicObject& rObj, Graphic& rSubstitute, const ByteString* pID )
{
    maObjList.Insert( (void*) &rObj, LIST_APPEND );
    mpCache->AddGraphicObject( rObj, rSubstitute, pID );
}

// -----------------------------------------------------------------------------

void GraphicManager::ImplUnregisterObj( const GraphicObject& rObj )
{
    mpCache->ReleaseGraphicObject( rObj );
    maObjList.Remove( (void*) &rObj );
}

// -----------------------------------------------------------------------------

void GraphicManager::ImplGraphicObjectWasSwappedOut( const GraphicObject& rObj )
{
    mpCache->GraphicObjectWasSwappedOut( rObj );
}

// -----------------------------------------------------------------------------

ByteString GraphicManager::ImplGetUniqueID( const GraphicObject& rObj ) const
{
    return mpCache->GetUniqueID( rObj );
}

// -----------------------------------------------------------------------------

BOOL GraphicManager::ImplFillSwappedGraphicObject( const GraphicObject& rObj, Graphic& rSubstitute )
{
    return( mpCache->FillSwappedGraphicObject( rObj, rSubstitute ) );
}

// -----------------------------------------------------------------------------

void GraphicManager::ImplGraphicObjectWasSwappedIn( const GraphicObject& rObj )
{
    mpCache->GraphicObjectWasSwappedIn( rObj );
}

// -----------------------------------------------------------------------------

BOOL GraphicManager::ImplDraw( OutputDevice* pOut, const Point& rPt,
                               const Size& rSz, GraphicObject& rObj,
                               const GraphicAttr& rAttr, BOOL& rCached )
{
    const Graphic&  rGraphic = rObj.GetGraphic();
    BOOL            bRet = FALSE;

    if( rGraphic.IsSupportedGraphic() && !rGraphic.IsSwapOut() )
    {
        if( GRAPHIC_BITMAP == rGraphic.GetType() )
        {
            const BitmapEx aSrcBmpEx( rGraphic.GetBitmapEx() );

            if( mpCache->IsDisplayCacheable( pOut, rPt, rSz, rObj, rAttr ) )
            {
                BitmapEx aDstBmpEx;

                if( ImplCreateOutput( pOut, rPt, rSz, aSrcBmpEx, rAttr, &aDstBmpEx ) )
                {
                    rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstBmpEx );
                    bRet = TRUE;
                }
            }

            if( !bRet )
                bRet = ImplCreateOutput( pOut, rPt, rSz, aSrcBmpEx, rAttr );
        }
        else
        {
            const GDIMetaFile& rSrcMtf = rGraphic.GetGDIMetaFile();

            if( mpCache->IsDisplayCacheable( pOut, rPt, rSz, rObj, rAttr ) )
            {
                GDIMetaFile aDstMtf;

                if( ImplCreateOutput( pOut, rPt, rSz, rSrcMtf, rAttr, &aDstMtf ) )
                {
                    rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstMtf );
                    bRet = TRUE;
                }
            }

            if( !bRet )
            {
                const Graphic aGraphic( rObj.GetTransformedGraphic( &rAttr ) );

                if( aGraphic.IsSupportedGraphic() )
                {
                    aGraphic.Draw( pOut, rPt, rSz );
                    bRet = TRUE;
                }
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicManager::ImplCreateOutput( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                       const BitmapEx& rBmpEx, const GraphicAttr& rAttr,
                                       BitmapEx* pBmpEx )
{
    USHORT  nRot10 = rAttr.GetRotation() % 3600;
    Point   aOutPtPix;
    Size    aOutSzPix;
    Size    aUnrotatedSzPix( pOut->LogicToPixel( rSz ) );
    BOOL    bRet = FALSE;

    if( nRot10 )
    {
        Polygon aPoly( Rectangle( rPt, rSz ) );

        aPoly.Rotate( rPt, nRot10 );
        const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
        aOutPtPix = pOut->LogicToPixel( aRotBoundRect.TopLeft() );
        aOutSzPix = pOut->LogicToPixel( aRotBoundRect.GetSize() );
    }
    else
    {
        aOutPtPix = pOut->LogicToPixel( rPt );
        aOutSzPix = aUnrotatedSzPix;
    }

    if( aUnrotatedSzPix.Width() && aUnrotatedSzPix.Height() )
    {
        BitmapEx        aBmpEx( rBmpEx );
        BitmapEx        aOutBmpEx;
        Point           aOutPt;
        Size            aOutSz;
        const Size&     rBmpSzPix = rBmpEx.GetSizePixel();
        const long      nW = rBmpSzPix.Width();
        const long      nH = rBmpSzPix.Height();
        const long      nNewW = aUnrotatedSzPix.Width();
        const long      nNewH = aUnrotatedSzPix.Height();
        const double    fRevScaleX = ( nNewW > 1L ) ? ( (double) ( nW - 1L ) / ( nNewW - 1L ) ) : 0.0;
        const double    fRevScaleY = ( nNewH > 1L ) ? ( (double) ( nH - 1L ) / ( nNewH - 1L ) ) : 0.0;
        double          fTmp;
        long*           pMapIX = new long[ nNewW ];
        long*           pMapFX = new long[ nNewW ];
        long*           pMapIY = new long[ nNewH ];
        long*           pMapFY = new long[ nNewH ];
        long            nStartX, nStartY, nEndX, nEndY;
        long            nX, nY, nTmp, nTmpX, nTmpY;
        BOOL            bHMirr = ( rAttr.GetMirrorFlags() & BMP_MIRROR_HORZ ) != 0;
        BOOL            bVMirr = ( rAttr.GetMirrorFlags() & BMP_MIRROR_VERT ) != 0;

        // create horizontal mapping table
        for( nX = 0L, nTmpX = nW - 1L, nTmp = nW - 2L; nX < nNewW; nX++ )
        {
            fTmp = nX * fRevScaleX;

            if( bHMirr )
                fTmp = nTmpX - fTmp;

            pMapFX[ nX ] = (long) ( ( fTmp - ( pMapIX[ nX ] = MinMax( (long) fTmp, 0, nTmp ) ) ) * 1048576. );
        }

        // create vertical mapping table
        for( nY = 0L, nTmpY = nH - 1L, nTmp = nH - 2L; nY < nNewH; nY++ )
        {
            fTmp = nY * fRevScaleY;

            if( bVMirr )
                fTmp = nTmpY - fTmp;

            pMapFY[ nY ] = (long) ( ( fTmp - ( pMapIY[ nY ] = MinMax( (long) fTmp, 0, nTmp ) ) ) * 1048576. );
        }

        // calculate output sizes
        if( !pBmpEx )
        {
            Point       aPt;
            Rectangle   aOutRect( aPt, pOut->GetOutputSizePixel() );
            Rectangle   aBmpRect( aOutPtPix, aOutSzPix );

            if( pOut->GetOutDevType() == OUTDEV_WINDOW )
            {
                const Region aPaintRgn( ( (Window*) pOut )->GetPaintRegion() );
                if( !aPaintRgn.IsNull() )
                    aOutRect.Intersection( pOut->LogicToPixel( aPaintRgn.GetBoundRect() ) );
            }

            aOutRect.Intersection( aBmpRect );

            if( !aOutRect.IsEmpty() )
            {
                aOutPt = pOut->PixelToLogic( aOutRect.TopLeft() );
                aOutSz = pOut->PixelToLogic( aOutRect.GetSize() );
                nStartX = aOutRect.Left() - aBmpRect.Left();
                nStartY = aOutRect.Top() - aBmpRect.Top();
                nEndX = aOutRect.Right() - aBmpRect.Left();
                nEndY = aOutRect.Bottom() - aBmpRect.Top();
            }
            else
                nStartX = -1L; // invalid
        }
        else
        {
            aOutPt = pOut->PixelToLogic( aOutPtPix );
            aOutSz = pOut->PixelToLogic( aOutSzPix );
            nStartX = nStartY = 0;
            nEndX = aOutSzPix.Width() - 1L;
            nEndY = aOutSzPix.Height() - 1L;
        }

        // do transformation
        if( nStartX >= 0L )
        {
            const BOOL bSimple = ( 1 == nW || 1 == nH );

            if( nRot10 )
            {
                if( bSimple )
                {
                    bRet = ( aOutBmpEx = aBmpEx ).Scale( aUnrotatedSzPix );

                    if( bRet )
                        aOutBmpEx.Rotate( nRot10, COL_TRANSPARENT );
                }
                else
                {
                    bRet = ImplCreateRotatedScaled( aBmpEx,
                                                    nRot10, aOutSzPix, aUnrotatedSzPix,
                                                    pMapIX, pMapFX, pMapIY, pMapFY, nStartX, nEndX, nStartY, nEndY,
                                                    aOutBmpEx );
                }
            }
            else
            {
                if( bSimple )
                    bRet = ( aOutBmpEx = aBmpEx ).Scale( Size( nEndX - nStartX + 1, nEndY - nStartY + 1 ) );
                else
                {
                    bRet = ImplCreateScaled( aBmpEx,
                                             pMapIX, pMapFX, pMapIY, pMapFY,
                                             nStartX, nEndX, nStartY, nEndY,
                                             aOutBmpEx );
                }
            }

            if( bRet )
            {
                // attribute adjustment if neccessary
                if( rAttr.IsSpecialDrawMode() || rAttr.IsAdjusted() || rAttr.IsTransparent() )
                    ImplAdjust( aOutBmpEx, rAttr, ADJUSTMENT_DRAWMODE | ADJUSTMENT_COLORS | ADJUSTMENT_TRANSPARENCY );

                // OutDev adjustment if neccessary
                if( pOut->GetOutDevType() != OUTDEV_PRINTER && pOut->GetBitCount() <= 8 && aOutBmpEx.GetBitCount() >= 8 )
                    aOutBmpEx.Dither( BMP_DITHER_MATRIX );
            }
        }

        // delete lookup tables
        delete[] pMapIX;
        delete[] pMapFX;
        delete[] pMapIY;
        delete[] pMapFY;

        // create output
        if( bRet )
        {
            if( !pBmpEx )
                pOut->DrawBitmapEx( aOutPt, aOutSz, aOutBmpEx );
            else
            {
                if( !rAttr.IsTransparent() && !aOutBmpEx.IsAlpha() )
                    aOutBmpEx = BitmapEx( aOutBmpEx.GetBitmap().CreateDisplayBitmap( pOut ), aOutBmpEx.GetMask() );

                pOut->DrawBitmapEx( aOutPt, aOutSz, *pBmpEx = aOutBmpEx );
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicManager::ImplCreateOutput( OutputDevice* pOut,
                                       const Point& rPt, const Size& rSz,
                                       const GDIMetaFile& rMtf,
                                       const GraphicAttr& rAttr,
                                       GDIMetaFile* pMtf )
{
    if( !pMtf )
    {
        DBG_ERROR( "Missing..." );
    }
    else
    {
        *pMtf = rMtf;

        if( rAttr.IsSpecialDrawMode() || rAttr.IsAdjusted() || rAttr.IsRotated() || rAttr.IsTransparent() )
            ImplAdjust( *pMtf, rAttr, ADJUSTMENT_DRAWMODE | ADJUSTMENT_COLORS | ADJUSTMENT_ROTATE | ADJUSTMENT_TRANSPARENCY );

        ImplDraw( pOut, rPt, rSz, *pMtf, rAttr );
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL GraphicManager::ImplCreateScaled( const BitmapEx& rBmpEx,
                                       long* pMapIX, long* pMapFX, long* pMapIY, long* pMapFY,
                                       long nStartX, long nEndX, long nStartY, long nEndY,
                                       BitmapEx& rOutBmpEx )
{
    Bitmap              aBmp( rBmpEx.GetBitmap() );
    Bitmap              aOutBmp;
    BitmapReadAccess*   pAcc = aBmp.AcquireReadAccess();
    BitmapWriteAccess*  pWAcc;
    BitmapColor         aCol0, aCol1, aColRes;
    const long          nDstW = nEndX - nStartX + 1L;
    const long          nDstH = nEndY - nStartY + 1L;
    long                nX, nY, nTmpX, nTmpY, nTmpFX, nTmpFY;
    long                nXDst, nYDst;
    BYTE                cR0, cG0, cB0, cR1, cG1, cB1;
    BOOL                bRet = FALSE;

    if( pAcc )
    {
        aOutBmp = Bitmap( Size( nDstW, nDstH ), 24 );
        pWAcc = aOutBmp.AcquireWriteAccess();

        if( pWAcc )
        {
            if( pAcc->HasPalette() )
            {
                if( pAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                {
                    Scanline pLine0, pLine1;

                    for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
                    {
                        nTmpY = pMapIY[ nY ]; nTmpFY = pMapFY[ nY ];
                        pLine0 = pAcc->GetScanline( nTmpY );
                        pLine1 = pAcc->GetScanline( ++nTmpY );

                        for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
                        {
                            nTmpX = pMapIX[ nX ]; nTmpFX = pMapFX[ nX ];

                            const BitmapColor& rCol0 = pAcc->GetPaletteColor( pLine0[ nTmpX ] );
                            const BitmapColor& rCol2 = pAcc->GetPaletteColor( pLine1[ nTmpX ] );
                            const BitmapColor& rCol1 = pAcc->GetPaletteColor( pLine0[ ++nTmpX ] );
                            const BitmapColor& rCol3 = pAcc->GetPaletteColor( pLine1[ nTmpX ] );

                            cR0 = MAP( rCol0.GetRed(), rCol1.GetRed(), nTmpFX );
                            cG0 = MAP( rCol0.GetGreen(), rCol1.GetGreen(), nTmpFX );
                            cB0 = MAP( rCol0.GetBlue(), rCol1.GetBlue(), nTmpFX );

                            cR1 = MAP( rCol2.GetRed(), rCol3.GetRed(), nTmpFX );
                            cG1 = MAP( rCol2.GetGreen(), rCol3.GetGreen(), nTmpFX );
                            cB1 = MAP( rCol2.GetBlue(), rCol3.GetBlue(), nTmpFX );

                            aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                            aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                            aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
                        }
                    }
                }
                else
                {
                    for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
                    {
                        nTmpY = pMapIY[ nY ], nTmpFY = pMapFY[ nY ];

                        for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
                        {
                            nTmpX = pMapIX[ nX ]; nTmpFX = pMapFX[ nX ];

                            aCol0 = pAcc->GetPaletteColor( pAcc->GetPixel( nTmpY, nTmpX ) );
                            aCol1 = pAcc->GetPaletteColor( pAcc->GetPixel( nTmpY, ++nTmpX ) );
                            cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTmpFX );
                            cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTmpFX );
                            cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTmpFX );

                            aCol1 = pAcc->GetPaletteColor( pAcc->GetPixel( ++nTmpY, nTmpX ) );
                            aCol0 = pAcc->GetPaletteColor( pAcc->GetPixel( nTmpY--, --nTmpX ) );
                            cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTmpFX );
                            cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTmpFX );
                            cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTmpFX );

                            aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                            aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                            aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
                        }
                    }
                }
            }
            else
            {
                if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
                {
                    Scanline    pLine0, pLine1, pTmp0, pTmp1;
                    long        nOff;

                    for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
                    {
                        nTmpY = pMapIY[ nY ]; nTmpFY = pMapFY[ nY ];
                        pLine0 = pAcc->GetScanline( nTmpY );
                        pLine1 = pAcc->GetScanline( ++nTmpY );

                        for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
                        {
                            nOff = 3L * ( nTmpX = pMapIX[ nX ] );
                            nTmpFX = pMapFX[ nX ];

                            pTmp1 = ( pTmp0 = pLine0 + nOff ) + 3L;
                            cB0 = MAP( *pTmp0, *pTmp1, nTmpFX ); pTmp0++; pTmp1++;
                            cG0 = MAP( *pTmp0, *pTmp1, nTmpFX ); pTmp0++; pTmp1++;
                            cR0 = MAP( *pTmp0, *pTmp1, nTmpFX );

                            pTmp1 = ( pTmp0 = pLine1 + nOff ) + 3L;
                            cB1 = MAP( *pTmp0, *pTmp1, nTmpFX ); pTmp0++; pTmp1++;
                            cG1 = MAP( *pTmp0, *pTmp1, nTmpFX ); pTmp0++; pTmp1++;
                            cR1 = MAP( *pTmp0, *pTmp1, nTmpFX );

                            aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                            aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                            aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
                        }
                    }
                }
                else if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
                {
                    Scanline    pLine0, pLine1, pTmp0, pTmp1;
                    long        nOff;

                    for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
                    {
                        nTmpY = pMapIY[ nY ]; nTmpFY = pMapFY[ nY ];
                        pLine0 = pAcc->GetScanline( nTmpY );
                        pLine1 = pAcc->GetScanline( ++nTmpY );

                        for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
                        {
                            nOff = 3L * ( nTmpX = pMapIX[ nX ] );
                            nTmpFX = pMapFX[ nX ];

                            pTmp1 = ( pTmp0 = pLine0 + nOff ) + 3L;
                            cR0 = MAP( *pTmp0, *pTmp1, nTmpFX ); pTmp0++; pTmp1++;
                            cG0 = MAP( *pTmp0, *pTmp1, nTmpFX ); pTmp0++; pTmp1++;
                            cB0 = MAP( *pTmp0, *pTmp1, nTmpFX );

                            pTmp1 = ( pTmp0 = pLine1 + nOff ) + 3L;
                            cR1 = MAP( *pTmp0, *pTmp1, nTmpFX ); pTmp0++; pTmp1++;
                            cG1 = MAP( *pTmp0, *pTmp1, nTmpFX ); pTmp0++; pTmp1++;
                            cB1 = MAP( *pTmp0, *pTmp1, nTmpFX );

                            aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                            aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                            aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
                        }
                    }
                }
                else
                {
                    for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
                    {
                        nTmpY = pMapIY[ nY ]; nTmpFY = pMapFY[ nY ];

                        for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
                        {
                            nTmpX = pMapIX[ nX ]; nTmpFX = pMapFX[ nX ];

                            aCol0 = pAcc->GetPixel( nTmpY, nTmpX );
                            aCol1 = pAcc->GetPixel( nTmpY, ++nTmpX );
                            cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTmpFX );
                            cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTmpFX );
                            cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTmpFX );

                            aCol1 = pAcc->GetPixel( ++nTmpY, nTmpX );
                            aCol0 = pAcc->GetPixel( nTmpY--, --nTmpX );
                            cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTmpFX );
                            cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTmpFX );
                            cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTmpFX );

                            aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                            aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                            aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                            pWAcc->SetPixel( nYDst, nXDst++, aColRes );
                        }
                    }
                }
            }

            aOutBmp.ReleaseAccess( pWAcc );
            bRet = TRUE;
        }

        aBmp.ReleaseAccess( pAcc );
    }

    if( bRet && rBmpEx.IsTransparent() )
    {
        bRet = FALSE;

        if( rBmpEx.IsAlpha() )
        {
            AlphaMask   aAlpha( rBmpEx.GetAlpha() );
            AlphaMask   aOutAlpha;

            pAcc = aAlpha.AcquireReadAccess();

            if( pAcc )
            {
                aOutAlpha = AlphaMask( Size( nDstW, nDstH ) );
                pWAcc = aOutAlpha.AcquireWriteAccess();

                if( pWAcc )
                {
                    if( pAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL &&
                        pWAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                    {
                        Scanline pLine0, pLine1, pLineW;

                        for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
                        {
                            nTmpY = pMapIY[ nY ]; nTmpFY = pMapFY[ nY ];
                            pLine0 = pAcc->GetScanline( nTmpY );
                            pLine1 = pAcc->GetScanline( ++nTmpY );
                            pLineW = pWAcc->GetScanline( nYDst );

                            for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++, nXDst++ )
                            {
                                nTmpX = pMapIX[ nX ]; nTmpFX = pMapFX[ nX ];

                                const long  nAlpha0 = pLine0[ nTmpX ];
                                const long  nAlpha2 = pLine1[ nTmpX ];
                                const long  nAlpha1 = pLine0[ ++nTmpX ];
                                const long  nAlpha3 = pLine1[ nTmpX ];
                                const long  n0 = MAP( nAlpha0, nAlpha1, nTmpFX );
                                const long  n1 = MAP( nAlpha2, nAlpha3, nTmpFX );

                                *pLineW++ = MAP( n0, n1, nTmpFY );
                            }
                        }
                    }
                    else
                    {
                        BitmapColor aAlphaValue( 0 );

                        for( nY = nStartY, nYDst = 0L; nY <= nEndY; nY++, nYDst++ )
                        {
                            nTmpY = pMapIY[ nY ], nTmpFY = pMapFY[ nY ];

                            for( nX = nStartX, nXDst = 0L; nX <= nEndX; nX++ )
                            {
                                nTmpX = pMapIX[ nX ]; nTmpFX = pMapFX[ nX ];

                                long        nAlpha0 = pAcc->GetPixel( nTmpY, nTmpX ).GetIndex();
                                long        nAlpha1 = pAcc->GetPixel( nTmpY, ++nTmpX ).GetIndex();
                                const long  n0 = MAP( nAlpha0, nAlpha1, nTmpFX );

                                nAlpha1 = pAcc->GetPixel( ++nTmpY, nTmpX ).GetIndex();
                                nAlpha0 = pAcc->GetPixel( nTmpY--, --nTmpX ).GetIndex();
                                const long  n1 = MAP( nAlpha0, nAlpha1, nTmpFX );

                                aAlphaValue.SetIndex( MAP( n0, n1, nTmpFY ) );
                                pWAcc->SetPixel( nYDst, nXDst++, aAlphaValue );
                            }
                        }
                    }

                    aOutAlpha.ReleaseAccess( pWAcc );
                    bRet = TRUE;
                }

                aAlpha.ReleaseAccess( pAcc );

                if( bRet )
                    rOutBmpEx = BitmapEx( aOutBmp, aOutAlpha );
            }
        }
        else
        {
            Bitmap  aMsk( rBmpEx.GetMask() );
            Bitmap  aOutMsk;

            pAcc = aMsk.AcquireReadAccess();

            if( pAcc )
            {
                aOutMsk = Bitmap( Size( nDstW, nDstH ), 1 );
                pWAcc = aOutMsk.AcquireWriteAccess();

                if( pWAcc )
                {
                    long* pMapLX = new long[ nDstW ];
                    long* pMapLY = new long[ nDstH ];

                    // create new horizontal mapping table
                    for( nX = 0UL, nTmpX = nStartX; nX < nDstW; nTmpX++ )
                        pMapLX[ nX++ ] = FRound( (double) pMapIX[ nTmpX ] + pMapFX[ nTmpX ] / 1048576. );

                    // create new vertical mapping table
                    for( nY = 0UL, nTmpY = nStartY; nY < nDstH; nTmpY++ )
                        pMapLY[ nY++ ] = FRound( (double) pMapIY[ nTmpY ] + pMapFY[ nTmpY ] / 1048576. );

                    // do normal scaling
                    if( pAcc->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL &&
                        pWAcc->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL )
                    {
                        // optimized
                        for( nY = 0; nY < nDstH; nY++ )
                        {
                            Scanline pSrc = pAcc->GetScanline( pMapLY[ nY ] );
                            Scanline pDst = pWAcc->GetScanline( nY );

                            for( nX = 0L; nX < nDstW; nX++ )
                            {
                                const long nSrcX = pMapLX[ nX ];

                                if( pSrc[ nSrcX >> 3 ] & ( 1 << ( 7 - ( nSrcX & 7 ) ) ) )
                                    pDst[ nX >> 3 ] |= 1 << ( 7 - ( nX & 7 ) );
                                else
                                    pDst[ nX >> 3 ] &= ~( 1 << ( 7 - ( nX & 7 ) ) );
                            }
                        }
                    }
                    else
                    {
                        const BitmapColor aB( pAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
                        const BitmapColor aWB( pWAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
                        const BitmapColor aWW( pWAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

                        if( pAcc->HasPalette() )
                        {
                            for( nY = 0L; nY < nDstH; nY++ )
                            {
                                for( nX = 0L; nX < nDstW; nX++ )
                                {
                                    if( pAcc->GetPaletteColor( (BYTE) pAcc->GetPixel( pMapLY[ nY ], pMapLX[ nX ] ) ) == aB )
                                        pWAcc->SetPixel( nY, nX, aWB );
                                    else
                                        pWAcc->SetPixel( nY, nX, aWW );
                                }
                            }
                        }
                        else
                        {
                            for( nY = 0L; nY < nDstH; nY++ )
                            {
                                for( nX = 0L; nX < nDstW; nX++ )
                                {
                                    if( pAcc->GetPixel( pMapLY[ nY ], pMapLX[ nX ] ) == aB )
                                        pWAcc->SetPixel( nY, nX, aWB );
                                    else
                                        pWAcc->SetPixel( nY, nX, aWW );
                                }
                            }
                        }
                    }

                    delete[] pMapLX;
                    delete[] pMapLY;
                    aOutMsk.ReleaseAccess( pWAcc );
                    bRet = TRUE;
                }

                aMsk.ReleaseAccess( pAcc );

                if( bRet )
                    rOutBmpEx = BitmapEx( aOutBmp, aOutMsk );
            }
        }

        if( !bRet )
            rOutBmpEx = aOutBmp;
    }
    else
        rOutBmpEx = aOutBmp;

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GraphicManager::ImplCreateRotatedScaled( const BitmapEx& rBmpEx,
                                              USHORT nRot10, const Size& rOutSzPix, const Size& rUnrotatedSzPix,
                                              long* pMapIX, long* pMapFX, long* pMapIY, long* pMapFY,
                                              long nStartX, long nEndX, long nStartY, long nEndY,
                                              BitmapEx& rOutBmpEx )
{
    Point               aPt;
    Bitmap              aBmp( rBmpEx.GetBitmap() );
    Bitmap              aOutBmp;
    BitmapReadAccess*   pAcc = aBmp.AcquireReadAccess();
    BitmapWriteAccess*  pWAcc;
    Polygon             aPoly( Rectangle( aPt, rUnrotatedSzPix ) ); aPoly.Rotate( Point(), nRot10 );
    Rectangle           aNewBound( aPoly.GetBoundRect() );
    const double        fCosAngle = cos( nRot10 * F_PI1800 ), fSinAngle = sin( nRot10 * F_PI1800 );
    double              fTmp;
    const long          nDstW = nEndX - nStartX + 1L;
    const long          nDstH = nEndY - nStartY + 1L;
    const long          nUnRotW = rUnrotatedSzPix.Width();
    const long          nUnRotH = rUnrotatedSzPix.Height();
    long*               pCosX = new long[ nDstW ];
    long*               pSinX = new long[ nDstW ];
    long*               pCosY = new long[ nDstH ];
    long*               pSinY = new long[ nDstH ];
    long                nX, nY, nTmpX, nTmpY, nTmpFX, nTmpFY, nUnRotX, nUnRotY, nSinY, nCosY;
    BYTE                cR0, cG0, cB0, cR1, cG1, cB1;
    BOOL                bRet = FALSE;

    // create horizontal mapping table
    for( nX = 0L, nTmpX = aNewBound.Left() + nStartX; nX < nDstW; nX++ )
    {
        pCosX[ nX ] = FRound( fCosAngle * ( fTmp = nTmpX++ << 8 ) );
        pSinX[ nX ] = FRound( fSinAngle * fTmp );
    }

    // create vertical mapping table
    for( nY = 0L, nTmpY = aNewBound.Top() + nStartY; nY < nDstH; nY++ )
    {
        pCosY[ nY ] = FRound( fCosAngle * ( fTmp = nTmpY++ << 8 ) );
        pSinY[ nY ] = FRound( fSinAngle * fTmp );
    }

    if( pAcc )
    {
        aOutBmp = Bitmap( Size( nDstW, nDstH ), 24 );
        pWAcc = aOutBmp.AcquireWriteAccess();

        if( pWAcc )
        {
            BitmapColor aColRes;

            if( pAcc->HasPalette() )
            {
                for( nY = 0; nY < nDstH; nY++ )
                {
                    nSinY = pSinY[ nY ];
                    nCosY = pCosY[ nY ];

                    for( nX = 0; nX < nDstW; nX++ )
                    {
                        nUnRotX = ( pCosX[ nX ] - nSinY ) >> 8;
                        nUnRotY = ( pSinX[ nX ] + nCosY ) >> 8;

                        if( ( nUnRotX >= 0L ) && ( nUnRotX < nUnRotW ) &&
                            ( nUnRotY >= 0L ) && ( nUnRotY < nUnRotH ) )
                        {
                            nTmpX = pMapIX[ nUnRotX ]; nTmpFX = pMapFX[ nUnRotX ];
                            nTmpY = pMapIY[ nUnRotY ], nTmpFY = pMapFY[ nUnRotY ];

                            const BitmapColor& rCol0 = pAcc->GetPaletteColor( pAcc->GetPixel( nTmpY, nTmpX ) );
                            const BitmapColor& rCol1 = pAcc->GetPaletteColor( pAcc->GetPixel( nTmpY, ++nTmpX ) );
                            cR0 = MAP( rCol0.GetRed(), rCol1.GetRed(), nTmpFX );
                            cG0 = MAP( rCol0.GetGreen(), rCol1.GetGreen(), nTmpFX );
                            cB0 = MAP( rCol0.GetBlue(), rCol1.GetBlue(), nTmpFX );

                            const BitmapColor& rCol3 = pAcc->GetPaletteColor( pAcc->GetPixel( ++nTmpY, nTmpX ) );
                            const BitmapColor& rCol2 = pAcc->GetPaletteColor( pAcc->GetPixel( nTmpY, --nTmpX ) );
                            cR1 = MAP( rCol2.GetRed(), rCol3.GetRed(), nTmpFX );
                            cG1 = MAP( rCol2.GetGreen(), rCol3.GetGreen(), nTmpFX );
                            cB1 = MAP( rCol2.GetBlue(), rCol3.GetBlue(), nTmpFX );

                            aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                            aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                            aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                            pWAcc->SetPixel( nY, nX, aColRes );
                        }
                    }
                }
            }
            else
            {
                BitmapColor aCol0, aCol1;

                for( nY = 0; nY < nDstH; nY++ )
                {
                    nSinY = pSinY[ nY ];
                    nCosY = pCosY[ nY ];

                    for( nX = 0; nX < nDstW; nX++ )
                    {
                        nUnRotX = ( pCosX[ nX ] - nSinY ) >> 8;
                        nUnRotY = ( pSinX[ nX ] + nCosY ) >> 8;

                        if( ( nUnRotX >= 0L ) && ( nUnRotX < nUnRotW ) &&
                            ( nUnRotY >= 0L ) && ( nUnRotY < nUnRotH ) )
                        {
                            nTmpX = pMapIX[ nUnRotX ]; nTmpFX = pMapFX[ nUnRotX ];
                            nTmpY = pMapIY[ nUnRotY ], nTmpFY = pMapFY[ nUnRotY ];

                            aCol0 = pAcc->GetPixel( nTmpY, nTmpX );
                            aCol1 = pAcc->GetPixel( nTmpY, ++nTmpX );
                            cR0 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTmpFX );
                            cG0 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTmpFX );
                            cB0 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTmpFX );

                            aCol1 = pAcc->GetPixel( ++nTmpY, nTmpX );
                            aCol0 = pAcc->GetPixel( nTmpY, --nTmpX );
                            cR1 = MAP( aCol0.GetRed(), aCol1.GetRed(), nTmpFX );
                            cG1 = MAP( aCol0.GetGreen(), aCol1.GetGreen(), nTmpFX );
                            cB1 = MAP( aCol0.GetBlue(), aCol1.GetBlue(), nTmpFX );

                            aColRes.SetRed( MAP( cR0, cR1, nTmpFY ) );
                            aColRes.SetGreen( MAP( cG0, cG1, nTmpFY ) );
                            aColRes.SetBlue( MAP( cB0, cB1, nTmpFY ) );
                            pWAcc->SetPixel( nY, nX, aColRes );
                        }
                    }
                }
            }

            aOutBmp.ReleaseAccess( pWAcc );
            bRet = TRUE;
        }

        aBmp.ReleaseAccess( pAcc );
    }

    // mask processing
    if( bRet && ( rBmpEx.IsTransparent() || ( nRot10 != 900 && nRot10 != 1800 && nRot10 != 2700 ) ) )
    {
        bRet = FALSE;

        if( rBmpEx.IsAlpha() )
        {
            AlphaMask   aAlpha( rBmpEx.GetAlpha() );
            AlphaMask   aOutAlpha;

            pAcc = aAlpha.AcquireReadAccess();

            if( pAcc )
            {
                aOutAlpha = AlphaMask( Size( nDstW, nDstH ) );
                pWAcc = aOutAlpha.AcquireWriteAccess();

                if( pWAcc )
                {
                    if( pAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL &&
                        pWAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                    {
                        Scanline pLine0, pLine1, pLineW;

                        for( nY = 0; nY < nDstH; nY++ )
                        {
                            nSinY = pSinY[ nY ], nCosY = pCosY[ nY ];
                            pLineW = pWAcc->GetScanline( nY );

                            for( nX = 0; nX < nDstW; nX++ )
                            {
                                nUnRotX = ( pCosX[ nX ] - nSinY ) >> 8;
                                nUnRotY = ( pSinX[ nX ] + nCosY ) >> 8;

                                if( ( nUnRotX >= 0L ) && ( nUnRotX < nUnRotW ) &&
                                    ( nUnRotY >= 0L ) && ( nUnRotY < nUnRotH ) )
                                {
                                    nTmpX = pMapIX[ nUnRotX ], nTmpFX = pMapFX[ nUnRotX ];
                                    nTmpY = pMapIY[ nUnRotY ], nTmpFY = pMapFY[ nUnRotY ];

                                    pLine0 = pAcc->GetScanline( nTmpY++ );
                                    pLine1 = pAcc->GetScanline( nTmpY );

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
                    else
                    {
                        const BitmapColor   aTrans( pWAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
                        BitmapColor         aAlphaVal( 0 );

                        for( nY = 0; nY < nDstH; nY++ )
                        {
                            nSinY = pSinY[ nY ], nCosY = pCosY[ nY ];

                            for( nX = 0; nX < nDstW; nX++ )
                            {
                                nUnRotX = ( pCosX[ nX ] - nSinY ) >> 8;
                                nUnRotY = ( pSinX[ nX ] + nCosY ) >> 8;

                                if( ( nUnRotX >= 0L ) && ( nUnRotX < nUnRotW ) &&
                                    ( nUnRotY >= 0L ) && ( nUnRotY < nUnRotH ) )
                                {
                                    nTmpX = pMapIX[ nUnRotX ]; nTmpFX = pMapFX[ nUnRotX ];
                                    nTmpY = pMapIY[ nUnRotY ], nTmpFY = pMapFY[ nUnRotY ];

                                    const long  nAlpha0 = pAcc->GetPixel( nTmpY, nTmpX ).GetIndex();
                                    const long  nAlpha1 = pAcc->GetPixel( nTmpY, ++nTmpX ).GetIndex();
                                    const long  nAlpha3 = pAcc->GetPixel( ++nTmpY, nTmpX ).GetIndex();
                                    const long  nAlpha2 = pAcc->GetPixel( nTmpY, --nTmpX ).GetIndex();
                                    const long  n0 = MAP( nAlpha0, nAlpha1, nTmpFX );
                                    const long  n1 = MAP( nAlpha2, nAlpha3, nTmpFX );

                                    aAlphaVal.SetIndex( MAP( n0, n1, nTmpFY ) );
                                    pWAcc->SetPixel( nY, nX, aAlphaVal );
                                }
                                else
                                    pWAcc->SetPixel( nY, nX, aTrans );
                            }
                        }
                    }

                    aOutAlpha.ReleaseAccess( pWAcc );
                    bRet = TRUE;
                }

                aAlpha.ReleaseAccess( pAcc );
            }

            if( bRet )
                rOutBmpEx = BitmapEx( aOutBmp, aOutAlpha );
        }
        else
        {
            Bitmap aOutMsk( Size( nDstW, nDstH ), 1 );
            pWAcc = aOutMsk.AcquireWriteAccess();

            if( pWAcc )
            {
                Bitmap              aMsk( rBmpEx.GetMask() );
                const BitmapColor   aB( pWAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
                const BitmapColor   aW( pWAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
                BitmapReadAccess*   pMAcc = NULL;

                if( !aMsk || ( ( pMAcc = aMsk.AcquireReadAccess() ) != NULL ) )
                {
                    long*       pMapLX = new long[ nUnRotW ];
                    long*       pMapLY = new long[ nUnRotH ];
                    BitmapColor aTestB;

                    if( pMAcc )
                        aTestB = pMAcc->GetBestMatchingColor( Color( COL_BLACK ) );

                    // create new horizontal mapping table
                    for( nX = 0UL; nX < nUnRotW; nX++ )
                        pMapLX[ nX ] = FRound( (double) pMapIX[ nX ] + pMapFX[ nX ] / 1048576. );

                    // create new vertical mapping table
                    for( nY = 0UL; nY < nUnRotH; nY++ )
                        pMapLY[ nY ] = FRound( (double) pMapIY[ nY ] + pMapFY[ nY ] / 1048576. );

                    // do mask rotation
                    for( nY = 0; nY < nDstH; nY++ )
                    {
                        nSinY = pSinY[ nY ];
                        nCosY = pCosY[ nY ];

                        for( nX = 0; nX < nDstW; nX++ )
                        {
                            nUnRotX = ( pCosX[ nX ] - nSinY ) >> 8;
                            nUnRotY = ( pSinX[ nX ] + nCosY ) >> 8;

                            if( ( nUnRotX >= 0L ) && ( nUnRotX < nUnRotW ) &&
                                ( nUnRotY >= 0L ) && ( nUnRotY < nUnRotH ) )
                            {
                                if( pMAcc )
                                {
                                    if( pMAcc->GetPixel( pMapLY[ nUnRotY ], pMapLX[ nUnRotX ] ) == aTestB )
                                        pWAcc->SetPixel( nY, nX, aB );
                                    else
                                        pWAcc->SetPixel( nY, nX, aW );
                                }
                                else
                                    pWAcc->SetPixel( nY, nX, aB );
                            }
                            else
                                pWAcc->SetPixel( nY, nX, aW );
                        }
                    }

                    delete[] pMapLX;
                    delete[] pMapLY;

                    if( pMAcc )
                        aMsk.ReleaseAccess( pMAcc );

                    bRet = TRUE;
                }

                aOutMsk.ReleaseAccess( pWAcc );
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

    return bRet;
}

// -----------------------------------------------------------------------------

void GraphicManager::ImplAdjust( BitmapEx& rBmpEx, const GraphicAttr& rAttr, ULONG nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
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
        AlphaMask   aAlpha;
        BYTE        cTrans = aAttr.GetTransparency();

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
                ULONG       nTrans = cTrans, nNewTrans;
                const long  nWidth = pA->Width(), nHeight = pA->Height();

                if( pA->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                {
                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pAScan = pA->GetScanline( nY );

                        for( long nX = 0; nX < nWidth; nX++ )
                        {
                            nNewTrans = nTrans + *pAScan;
                            *pAScan++ = (BYTE) ( ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans );
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
                            aAlphaValue.SetIndex( (BYTE) ( ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans ) );
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

// -----------------------------------------------------------------------------

void GraphicManager::ImplAdjust( GDIMetaFile& rMtf, const GraphicAttr& rAttr, ULONG nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
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

    if( ( nAdjustmentFlags & ADJUSTMENT_COLORS ) && aAttr.IsAdjusted() )
    {
        rMtf.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                     aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                     aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_MIRROR ) && aAttr.IsMirrored() )
    {
        Size    aPrefSize( rMtf.GetPrefSize() );
        long    nMoveX, nMoveY;
        double  fScaleX, fScaleY;

        if( aAttr.GetMirrorFlags() & BMP_MIRROR_HORZ )
            nMoveX = VOS_ABS( aPrefSize.Width() ) - 1, fScaleX = -1.0;
        else
            nMoveX = 0, fScaleX = 1.0;

        if( aAttr.GetMirrorFlags() & BMP_MIRROR_VERT )
            nMoveY = VOS_ABS( aPrefSize.Height() ) - 1, fScaleY = -1.0;
        else
            nMoveY = 0, fScaleY = 1.0;

        rMtf.Scale( fScaleX, fScaleY );
        rMtf.Move( nMoveX, nMoveY );
        rMtf.SetPrefSize( aPrefSize );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        DBG_ERROR( "Missing implementation: Mtf-Rotation" );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        DBG_ERROR( "Missing implementation: Mtf-Transparency" );
    }
}

// -----------------------------------------------------------------------------

void GraphicManager::ImplAdjust( Animation& rAnimation, const GraphicAttr& rAttr, ULONG nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & ADJUSTMENT_DRAWMODE ) && aAttr.IsSpecialDrawMode() )
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

    if( ( nAdjustmentFlags & ADJUSTMENT_COLORS ) && aAttr.IsAdjusted() )
    {
        rAnimation.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                           aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                           aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_MIRROR ) && aAttr.IsMirrored() )
        rAnimation.Mirror( aAttr.GetMirrorFlags() );

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        DBG_ERROR( "Missing implementation: Animation-Rotation" );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        DBG_ERROR( "Missing implementation: Animation-Transparency" );
    }
}

// -----------------------------------------------------------------------------

void GraphicManager::ImplDraw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                               const GDIMetaFile& rMtf, const GraphicAttr& rAttr )
{
    Point   aPt( rPt );
    Size    aSz( rSz );

    if( rAttr.IsMirrored() )
    {
        if( rAttr.GetMirrorFlags() & BMP_MIRROR_HORZ )
        {
            aPt.X() += aSz.Width() - 1;
            aSz.Width() = -aSz.Width();
        }

        if( rAttr.GetMirrorFlags() & BMP_MIRROR_VERT )
        {
            aPt.Y() += aSz.Height() - 1;
            aSz.Height() = -aSz.Height();
        }
    }

    pOut->Push( PUSH_CLIPREGION );
    pOut->IntersectClipRegion( Rectangle( aPt, aSz ) );

    ( (GDIMetaFile&) rMtf ).WindStart();
    ( (GDIMetaFile&) rMtf ).Play( pOut, aPt, aSz );
    ( (GDIMetaFile&) rMtf ).WindStart();

    pOut->Pop();
}

