/*************************************************************************
 *
 *  $RCSfile: grfmgr2.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 15:37:47 $
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
#include <vcl/metaact.hxx>
#include <vcl/metric.hxx>
#include <vcl/animate.hxx>
#include <vcl/alpha.hxx>
#include <vcl/virdev.hxx>
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

void GraphicManager::SetCacheTimeout( ULONG nTimeoutSeconds )
{
    mpCache->SetCacheTimeout( nTimeoutSeconds );
}

// -----------------------------------------------------------------------------

ULONG GraphicManager::GetCacheTimeout() const
{
    return mpCache->GetCacheTimeout();
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
    Point   aPt( rPt );
    Size    aSz( rSz );
    BOOL    bRet = FALSE;

    rCached = FALSE;

    if( ( rObj.GetType() == GRAPHIC_BITMAP ) || ( rObj.GetType() == GRAPHIC_GDIMETAFILE ) )
    {
        // create output and fill cache
        const Size aOutSize( pOut->GetOutputSizePixel() );

        if( rObj.IsAnimated() || ( pOut->GetOutDevType() == OUTDEV_PRINTER ) ||
            ( !( nFlags & GRFMGR_DRAW_NO_SUBSTITUTE ) &&
              ( ( nFlags & GRFMGR_DRAW_SUBSTITUTE ) ||
                !( nFlags & GRFMGR_DRAW_CACHED ) ||
                ( pOut->GetConnectMetaFile() && !pOut->IsOutputEnabled() ) ) ) )
        {
            // simple output of transformed graphic
            const Graphic aGraphic( rObj.GetTransformedGraphic( &rAttr ) );

            if( aGraphic.IsSupportedGraphic() )
            {
                const USHORT nRot10 = rAttr.GetRotation() % 3600;

                if( nRot10 )
                {
                    Polygon aPoly( Rectangle( aPt, aSz ) );

                    aPoly.Rotate( aPt, nRot10 );
                    const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
                    aPt = aRotBoundRect.TopLeft();
                    aSz = aRotBoundRect.GetSize();
                }

                aGraphic.Draw( pOut, aPt, aSz );
            }

            bRet = TRUE;
        }

        if( !bRet )
        {
            // cached/direct drawing
            if( !mpCache->DrawDisplayCacheObj( pOut, aPt, aSz, rObj, rAttr ) )
                bRet = ImplDraw( pOut, aPt, aSz, rObj, rAttr, nFlags, rCached );
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
                               const GraphicAttr& rAttr,
                               const ULONG nFlags, BOOL& rCached )
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

                if( ImplCreateOutput( pOut, rPt, rSz, aSrcBmpEx, rAttr, nFlags, &aDstBmpEx ) )
                {
                    rCached = mpCache->CreateDisplayCacheObj( pOut, rPt, rSz, rObj, rAttr, aDstBmpEx );
                    bRet = TRUE;
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

                if( ImplCreateOutput( pOut, rPt, rSz, rSrcMtf, rAttr, nFlags, &aDstMtf ) )
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

BOOL GraphicManager::ImplCreateOutput( OutputDevice* pOut,
                                       const Point& rPt, const Size& rSz,
                                       const BitmapEx& rBmpEx, const GraphicAttr& rAttr,
                                       const ULONG nFlags, BitmapEx* pBmpEx )
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
        double          fTmp;
        long*           pMapIX = new long[ nNewW ];
        long*           pMapFX = new long[ nNewW ];
        long*           pMapIY = new long[ nNewH ];
        long*           pMapFY = new long[ nNewH ];
        long            nStartX, nStartY, nEndX, nEndY;
        long            nX, nY, nTmp, nTmpX, nTmpY;
        BOOL            bHMirr = ( rAttr.GetMirrorFlags() & BMP_MIRROR_HORZ ) != 0;
        BOOL            bVMirr = ( rAttr.GetMirrorFlags() & BMP_MIRROR_VERT ) != 0;

        if( nFlags & GRFMGR_DRAW_BILINEAR )
        {
            const double    fRevScaleX = ( nNewW > 1L ) ? ( (double) ( nW - 1L ) / ( nNewW - 1L ) ) : 0.0;
            const double    fRevScaleY = ( nNewH > 1L ) ? ( (double) ( nH - 1L ) / ( nNewH - 1L ) ) : 0.0;

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
        }
        else
        {
            // #98290# Use a different mapping for non-interpolating mode, to avoid missing rows/columns
            const double    fRevScaleX = ( nNewW > 1L ) ? ( (double) nW / nNewW ) : 0.0;
            const double    fRevScaleY = ( nNewH > 1L ) ? ( (double) nH / nNewH ) : 0.0;

            // create horizontal mapping table
            for( nX = 0L, nTmpX = nW - 1L, nTmp = nW - 2L; nX < nNewW; nX++ )
            {
                fTmp = nX * fRevScaleX;

                if( bHMirr )
                    fTmp = nTmpX - fTmp;

                // #98290# Do not use round to zero, otherwise last column will be missing
                pMapIX[ nX ] = MinMax( (long) fTmp, 0, nTmp );
                pMapFX[ nX ] = fTmp >= nTmp+1 ? 1048576 : 0;
            }

            // create vertical mapping table
            for( nY = 0L, nTmpY = nH - 1L, nTmp = nH - 2L; nY < nNewH; nY++ )
            {
                fTmp = nY * fRevScaleY;

                if( bVMirr )
                    fTmp = nTmpY - fTmp;

                // #98290# Do not use round to zero, otherwise last row will be missing
                pMapIY[ nY ] = MinMax( (long) fTmp, 0, nTmp );
                pMapFY[ nY ] = fTmp >= nTmp+1 ? 1048576 : 0;
            }
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
                // #105229# Don't scale if output size equals bitmap size
                // #107226# Copy through only if we're not mirroring
                if( !bHMirr && !bVMirr && aOutSzPix == rBmpSzPix )
                {
                    // #107226# Use original dimensions when just copying through
                    aOutPt = pOut->PixelToLogic( aOutPtPix );
                    aOutSz = pOut->PixelToLogic( aOutSzPix );
                    aOutBmpEx = aBmpEx;
                    bRet = TRUE;
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
                                       const GDIMetaFile& rMtf, const GraphicAttr& rAttr,
                                       const ULONG nFlags, GDIMetaFile* pMtf )
{
    if( !pMtf )
    {
        DBG_ERROR( "Missing..." );
    }
    else
    {
        Size aNewSize( rMtf.GetPrefSize() );

        *pMtf = rMtf;

        if( aNewSize.Width() && aNewSize.Height() && rSz.Width() && rSz.Height() )
        {
            const double fGrfWH = (double) aNewSize.Width() / aNewSize.Height();
            const double fOutWH = (double) rSz.Width() / rSz.Height();

            double fScaleX = fOutWH / fGrfWH;
            double fScaleY = 1.0;

            // taking care of font width default if scaling metafile:
            sal_uInt32 nCurPos;
            MetaAction* pAct;
            for( nCurPos = 0, pAct = (MetaAction*)pMtf->FirstAction(); pAct;
                    pAct = (MetaAction*)pMtf->NextAction(), nCurPos++ )
            {
                MetaAction* pModAct = NULL;
                switch( pAct->GetType() )
                {
                    case META_FONT_ACTION :
                    {
                        MetaFontAction* pA = (MetaFontAction*)pAct;
                        Font aFont( pA->GetFont() );
                        if ( !aFont.GetWidth() )
                        {
                            FontMetric aFontMetric( pOut->GetFontMetric( aFont ) );
                            aFont.SetWidth( aFontMetric.GetWidth() );
                            pModAct = new MetaFontAction( aFont );
                        }
                    }
                }
                if ( pModAct )
                {
                    pMtf->ReplaceAction( pModAct, nCurPos );
                    pAct->Delete();
                }
                else
                {
                    if( pAct->GetRefCount() > 1 )
                    {
                        pMtf->ReplaceAction( pModAct = pAct->Clone(), nCurPos );
                        pAct->Delete();
                    }
                    else
                        pModAct = pAct;
                }
                pModAct->Scale( fScaleX, fScaleY );
            }
            pMtf->SetPrefSize( Size( FRound( aNewSize.Width() * fScaleX ),
                                     FRound( aNewSize.Height() * fScaleY ) ) );

/*
            if( fGrfWH < fWinWH )
                aNewSize.Width() = (long) ( ( aNewSize.Height() = rSz.Height.Height() ) * fGrfWH );
            else
                aNewSize.Height()= (long) ( ( aNewSize.Width() = rSz.Width() ) / fGrfWH );
*/
        }

        if( rAttr.IsSpecialDrawMode() || rAttr.IsAdjusted() || rAttr.IsMirrored() || rAttr.IsRotated() || rAttr.IsTransparent() )
            ImplAdjust( *pMtf, rAttr, ADJUSTMENT_ALL );

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
        rMtf.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & ADJUSTMENT_ROTATE ) && aAttr.IsRotated() )
    {
        rMtf.Rotate( aAttr.GetRotation() );
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
    {
        rAnimation.Mirror( aAttr.GetMirrorFlags() );
    }

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
       USHORT   nRot10 = rAttr.GetRotation() % 3600;
    Point   aOutPt( rPt );
    Size    aOutSz( rSz );

    if( nRot10 )
    {
        Polygon aPoly( Rectangle( aOutPt, aOutSz ) );

        aPoly.Rotate( aOutPt, nRot10 );
        const Rectangle aRotBoundRect( aPoly.GetBoundRect() );
        aOutPt = aRotBoundRect.TopLeft();
        aOutSz = aRotBoundRect.GetSize();
    }

    pOut->Push( PUSH_CLIPREGION );
    pOut->IntersectClipRegion( Rectangle( aOutPt, aOutSz ) );

    ( (GDIMetaFile&) rMtf ).WindStart();
    ( (GDIMetaFile&) rMtf ).Play( pOut, aOutPt, aOutSz );
    ( (GDIMetaFile&) rMtf ).WindStart();

    pOut->Pop();
}

// -----------------------------------------------------------------------------

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
                                        const GraphicAttr* pAttr, ULONG nFlags )
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
    BOOL bOldMap( rVDev.IsMapModeEnabled() );
    rVDev.EnableMapMode( FALSE );

    bool bRet( ImplRenderTileRecursive( rVDev, nExponent, nMSBFactor, nNumTilesX, nNumTilesY,
                                        nNumTilesX, nNumTilesY, rTileSizePixel, pAttr, nFlags, aTileInfo ) );

    rVDev.EnableMapMode( bOldMap );

    return bRet;
}

// -----------------------------------------------------------------------------

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
                                             ULONG nFlags, ImplTileInfo& rTileInfo )
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
        //
        //  x0000
        //  0
        //  0
        //  0
        //  0
        //
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

// -----------------------------------------------------------------------------

bool GraphicObject::ImplDrawTiled( OutputDevice* pOut, const Rectangle& rArea, const Size& rSizePixel,
                                   const Size& rOffset, const GraphicAttr* pAttr, ULONG nFlags, int nTileCacheSize1D )
{
    // how many tiles to generate per recursion step
    enum{ SubdivisionExponent=2 };

    const MapMode   aOutMapMode( pOut->GetMapMode() );
    const MapMode   aMapMode( aOutMapMode.GetMapUnit(), Point(), aOutMapMode.GetScaleX(), aOutMapMode.GetScaleY() );
    bool            bRet( false );

    if( GetGraphic().GetType() == GRAPHIC_BITMAP &&
        rSizePixel.Width() * rSizePixel.Height() < nTileCacheSize1D*nTileCacheSize1D )
    {
        // First combine very small bitmaps into a larger tile
        // ===================================================

        VirtualDevice   aVDev;
        const int       nNumTilesInCacheX( (nTileCacheSize1D + rSizePixel.Width()-1) / rSizePixel.Width() );
        const int       nNumTilesInCacheY( (nTileCacheSize1D + rSizePixel.Height()-1) / rSizePixel.Height() );

        aVDev.SetOutputSizePixel( Size( nNumTilesInCacheX*rSizePixel.Width(), nNumTilesInCacheY*rSizePixel.Height() ) );
        aVDev.SetMapMode( aMapMode );

        // draw bitmap content
        if( ImplRenderTempTile( aVDev, SubdivisionExponent, nNumTilesInCacheX,
                                nNumTilesInCacheY, rSizePixel, pAttr, nFlags ) )
        {
            BitmapEx aTileBitmap( aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ) );

            // draw alpha content, if any
            if( IsTransparent() )
            {
                GraphicObject aAlphaGraphic;

                if( GetGraphic().IsAlpha() )
                    aAlphaGraphic.SetGraphic( GetGraphic().GetBitmapEx().GetAlpha().GetBitmap() );
                else
                    aAlphaGraphic.SetGraphic( GetGraphic().GetBitmapEx().GetMask() );

                if( aAlphaGraphic.ImplRenderTempTile( aVDev, SubdivisionExponent, nNumTilesInCacheX,
                                                      nNumTilesInCacheY, rSizePixel, pAttr, nFlags ) )
                {
                    // Combine bitmap and alpha/mask
                    if( GetGraphic().IsAlpha() )
                        aTileBitmap = BitmapEx( aTileBitmap.GetBitmap(),
                                                AlphaMask( aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ) ) );
                    else
                        aTileBitmap = BitmapEx( aTileBitmap.GetBitmap(),
                                                aVDev.GetBitmap( Point(0,0), aVDev.GetOutputSize() ).CreateMask( Color(COL_WHITE) ) );
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

        pOut->Push( PUSH_CLIPREGION );
        pOut->IntersectClipRegion( rArea );

        // Paint all tiles
        // ===============

        bRet = ImplDrawTiled( *pOut, aOutStart,
                              (aOutArea.GetWidth() + aOutArea.Left() - aOutStart.X() + rSizePixel.Width() - 1) / rSizePixel.Width(),
                              (aOutArea.GetHeight() + aOutArea.Top() - aOutStart.Y() + rSizePixel.Height() - 1) / rSizePixel.Height(),
                              rSizePixel, pAttr, nFlags );

        pOut->Pop();
    }

    return bRet;
}

// -----------------------------------------------------------------------------

bool GraphicObject::ImplDrawTiled( OutputDevice& rOut, const Point& rPosPixel,
                                   int nNumTilesX, int nNumTilesY,
                                   const Size& rTileSizePixel, const GraphicAttr* pAttr, ULONG nFlags )
{
    Point   aCurrPos( rPosPixel );
    Size    aTileSizeLogic( rOut.PixelToLogic( rTileSizePixel ) );
    int     nX, nY;

    // #107607# Use logical coordinates for metafile playing, too
    bool    bDrawInPixel( rOut.GetConnectMetaFile() == NULL && GRAPHIC_BITMAP == GetType() );

    // #105229# Switch off mapping (converting to logic and back to
    // pixel might cause roundoff errors)
    BOOL bOldMap( rOut.IsMapModeEnabled() );

    if( bDrawInPixel )
        rOut.EnableMapMode( FALSE );

    for( nY=0; nY < nNumTilesY; ++nY )
    {
        aCurrPos.X() = rPosPixel.X();

        for( nX=0; nX < nNumTilesX; ++nX )
        {
            // #105229# work with pixel coordinates here, mapping is disabled!
            // #104004# don't disable mapping for metafile recordings
            if( !Draw( &rOut,
                       bDrawInPixel ? aCurrPos : rOut.PixelToLogic( aCurrPos ),
                       bDrawInPixel ? rTileSizePixel : aTileSizeLogic,
                       pAttr, nFlags ) )
            {
                rOut.EnableMapMode( bOldMap );
                return FALSE;
            }

            aCurrPos.X() += rTileSizePixel.Width();
        }

        aCurrPos.Y() += rTileSizePixel.Height();
    }

    if( bDrawInPixel )
        rOut.EnableMapMode( bOldMap );

    return TRUE;
}
