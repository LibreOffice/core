/*************************************************************************
 *
 *  $RCSfile: print2.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ka $ $Date: 2001-07-04 11:38:56 $
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

#define _SV_PRINT_CXX
#define _SPOOLPRINTER_EXT

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <print.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

// -----------
// - Defines -
// -----------

#define MAX_TILE_WIDTH  1024
#define MAX_TILE_HEIGHT 1024

// -----------------
// - ImplCheckRect -
// -----------------

struct ImplCheckRect
{
    Rectangle*      mpRect;
    MetaAction*     mpAct;
    ImplCheckRect*  mpNext;
    BOOL            mbSpecialOutput;

                    ImplCheckRect() { }
                    ~ImplCheckRect() { delete mpRect; }
    void            ImplCreate( MetaAction* pAct, OutputDevice* pOut, BOOL bSpecial );
    BOOL            Intersects( const ImplCheckRect& rRect )
                    {
                        return( mpRect && rRect.mpRect ) ? mpRect->IsOver( *rRect.mpRect ) : FALSE;
                    }
};

// -----------------------------------------------------------------------------

void ImplCheckRect::ImplCreate( MetaAction* pAct, OutputDevice* pOut, BOOL bSpecial )
{
    const long bLineTransparency = ( pOut->GetLineColor().GetTransparency() == 255 );
    const long bFillTransparency = ( pOut->GetFillColor().GetTransparency() == 255 );

    mpAct = pAct;
    mpNext = NULL;
    mpRect = NULL;

    switch( mpAct->GetType() )
    {
        case( META_PIXEL_ACTION ):
            mpRect = new Rectangle( ( (MetaPixelAction*) mpAct )->GetPoint(), Size( 1, 1 ) );
        break;

        case( META_POINT_ACTION ):
        {
            if( !bLineTransparency )
                mpRect = new Rectangle( ( (MetaPointAction*) mpAct )->GetPoint(), Size( 1, 1 ) );
        }
        break;

        case( META_LINE_ACTION ):
        {
            if( !bLineTransparency )
            {
                MetaLineAction* pA = (MetaLineAction*) mpAct;
                mpRect = new Rectangle( pA->GetStartPoint(), pA->GetEndPoint() );
            }
        }
        break;

        case( META_RECT_ACTION ):
        {
            if( !bLineTransparency || !bFillTransparency )
                mpRect = new Rectangle( ( (MetaRectAction*) mpAct )->GetRect() );
        }
        break;

        case( META_ROUNDRECT_ACTION ):
        {
            if( !bLineTransparency || !bFillTransparency )
            {
                MetaRoundRectAction* pA = (MetaRoundRectAction*) mpAct;
                mpRect = new Rectangle( Polygon( pA->GetRect(),
                                                 pA->GetHorzRound(),
                                                 pA->GetVertRound() ).GetBoundRect() );
            }
        }
        break;

        case( META_ELLIPSE_ACTION ):
        {
            if( !bLineTransparency || !bFillTransparency )
            {
                MetaEllipseAction*  pA = (MetaEllipseAction*) mpAct;
                const Rectangle&    rRect = pA->GetRect();
                mpRect = new Rectangle( Polygon( rRect.Center(),
                                                 rRect.GetWidth() >> 1,
                                                 rRect.GetHeight() >> 1 ).GetBoundRect() );
            }
        }
        break;

        case( META_ARC_ACTION ):
        {
            if( !bLineTransparency || !bFillTransparency )
            {
                MetaArcAction* pA = (MetaArcAction*) mpAct;
                mpRect = new Rectangle( Polygon( pA->GetRect(),
                                                 pA->GetStartPoint(),
                                                 pA->GetEndPoint(), POLY_ARC ).GetBoundRect() );
            }
        }
        break;

        case( META_PIE_ACTION ):
        {
            if( !bLineTransparency || !bFillTransparency )
            {
                MetaPieAction* pA = (MetaPieAction*) mpAct;
                mpRect = new Rectangle( Polygon( pA->GetRect(),
                                                 pA->GetStartPoint(),
                                                 pA->GetEndPoint(), POLY_PIE ).GetBoundRect() );
            }
        }
        break;

        case( META_CHORD_ACTION ):
        {
            if( !bLineTransparency || !bFillTransparency )
            {
                MetaChordAction* pA = (MetaChordAction*) mpAct;
                mpRect = new Rectangle( Polygon( pA->GetRect(),
                                                 pA->GetStartPoint(),
                                                 pA->GetEndPoint(), POLY_CHORD ).GetBoundRect() );
            }
        }
        break;

        case( META_POLYLINE_ACTION ):
        {
            if( !bLineTransparency )
            {
                MetaPolyLineAction* pA = (MetaPolyLineAction*) mpAct;
                mpRect = new Rectangle( pA->GetPolygon().GetBoundRect() );
            }
        }
        break;

        case( META_POLYGON_ACTION ):
        {
            if( !bLineTransparency || !bFillTransparency )
            {
                MetaPolygonAction* pA = (MetaPolygonAction*) mpAct;
                mpRect = new Rectangle( pA->GetPolygon().GetBoundRect() );
            }
        }
        break;

        case( META_POLYPOLYGON_ACTION ):
        {
            if( !bLineTransparency || !bFillTransparency )
            {
                MetaPolyPolygonAction* pA = (MetaPolyPolygonAction*) mpAct;
                mpRect = new Rectangle( pA->GetPolyPolygon().GetBoundRect() );
            }
        }
        break;

        case( META_BMP_ACTION ):
        {
            MetaBmpAction* pA = (MetaBmpAction*) mpAct;
            mpRect = new Rectangle( pA->GetPoint(), pOut->PixelToLogic( pA->GetBitmap().GetSizePixel() ) );
        }
        break;

        case( META_BMPSCALE_ACTION ):
        {
            MetaBmpScaleAction* pA = (MetaBmpScaleAction*) mpAct;
            mpRect = new Rectangle( pA->GetPoint(), pA->GetSize() );
        }
        break;

        case( META_BMPSCALEPART_ACTION ):
        {
            MetaBmpScalePartAction* pA = (MetaBmpScalePartAction*) mpAct;
            mpRect = new Rectangle( pA->GetDestPoint(), pA->GetDestSize() );
        }
        break;

        case( META_BMPEX_ACTION ):
        {
            MetaBmpExAction* pA = (MetaBmpExAction*) mpAct;
            mpRect = new Rectangle( pA->GetPoint(), pOut->PixelToLogic( pA->GetBitmapEx().GetSizePixel() ) );
        }
        break;

        case( META_BMPEXSCALE_ACTION ):
        {
            MetaBmpExScaleAction* pA = (MetaBmpExScaleAction*) mpAct;
            mpRect = new Rectangle( pA->GetPoint(), pA->GetSize() );
        }
        break;

        case( META_BMPEXSCALEPART_ACTION ):
        {
            MetaBmpExScalePartAction* pA = (MetaBmpExScalePartAction*) mpAct;
            mpRect = new Rectangle( pA->GetDestPoint(), pA->GetDestSize() );
        }
        break;

        case( META_MASK_ACTION ):
        {
            MetaMaskAction* pA = (MetaMaskAction*) mpAct;
            mpRect = new Rectangle( pA->GetPoint(), pOut->PixelToLogic( pA->GetBitmap().GetSizePixel() ) );
        }
        break;

        case( META_MASKSCALE_ACTION ):
        {
            MetaMaskScaleAction* pA = (MetaMaskScaleAction*) mpAct;
            mpRect = new Rectangle( pA->GetPoint(), pA->GetSize() );
        }
        break;

        case( META_MASKSCALEPART_ACTION ):
        {
            MetaMaskScalePartAction* pA = (MetaMaskScalePartAction*) mpAct;
            mpRect = new Rectangle( pA->GetDestPoint(), pA->GetDestSize() );
        }
        break;

        case( META_GRADIENT_ACTION ):
            mpRect = new Rectangle( ( (MetaGradientAction*) mpAct )->GetRect() );
        break;

        case( META_GRADIENTEX_ACTION ):
            mpRect = new Rectangle( ( (MetaGradientExAction*) mpAct )->GetPolyPolygon().GetBoundRect() );
        break;

        case( META_HATCH_ACTION ):
            mpRect = new Rectangle( ( (MetaHatchAction*) mpAct )->GetPolyPolygon().GetBoundRect() );
        break;

        case( META_WALLPAPER_ACTION ):
            mpRect = new Rectangle( ( (MetaWallpaperAction*) mpAct )->GetRect() );
        break;

        case( META_TRANSPARENT_ACTION ):
            mpRect = new Rectangle( ( (MetaTransparentAction*) mpAct )->GetPolyPolygon().GetBoundRect() );
        break;

        case( META_FLOATTRANSPARENT_ACTION ):
        {
            MetaFloatTransparentAction* pA = (MetaFloatTransparentAction*) mpAct;
            mpRect = new Rectangle( pA->GetPoint(), pA->GetSize() );
        }
        break;

        case( META_EPS_ACTION ):
        {
            MetaEPSAction* pA = (MetaEPSAction*) mpAct;
            mpRect = new Rectangle( pA->GetPoint(), pA->GetSize() );
        }
        break;

        case( META_TEXT_ACTION ):
        {
            MetaTextAction* pA = (MetaTextAction*) mpAct;
            const Point     aPt( pOut->LogicToPixel( pA->GetPoint() ) );
            const XubString aString( pA->GetText(), pA->GetIndex(), pA->GetLen() );
            mpRect = new Rectangle( pOut->PixelToLogic( pOut->ImplGetTextBoundRect( aPt.X(), aPt.Y(), aString.GetBuffer(), aString.Len(), NULL ) ) );
        }
        break;

        case( META_TEXTARRAY_ACTION ):
        {
            MetaTextArrayAction*    pA = (MetaTextArrayAction*) mpAct;
            const XubString         aString( pA->GetText(), pA->GetIndex(), pA->GetLen() );
            const long              nLen = aString.Len();

            if( nLen )
            {
                const Point aPtLog( pA->GetPoint() );
                const Point aPtPix( pOut->LogicToPixel( aPtLog ) );
                long*       pPixDX = pA->GetDXArray() ? ( new long[ nLen ] ) : NULL;

                if( pPixDX )
                {
                    for ( long i = 0; i < ( nLen - 1 ); i++ )
                    {
                        const Point aNextPt( aPtLog.X() + pA->GetDXArray()[ i ], 0 );
                        pPixDX[ i ] = pOut->LogicToPixel( aNextPt ).X() - aPtPix.X();
                    }
                }

                mpRect = new Rectangle( pOut->PixelToLogic( pOut->ImplGetTextBoundRect( aPtPix.X(), aPtPix.Y(), aString.GetBuffer(), aString.Len(), pPixDX ) ) );
                delete[] pPixDX;
            }
        }
        break;

        case( META_TEXTRECT_ACTION ):
        {
            MetaTextRectAction* pA = (MetaTextRectAction*) mpAct;
            mpRect = new Rectangle( pA->GetRect() );
        }
        break;

        case( META_STRETCHTEXT_ACTION ):
        case( META_TEXTLINE_ACTION ):
            mpRect = NULL;
            // !!! DBG_ERROR( "Missing" );
        break;

        default:
        break;
    }

    if( mpRect )
    {
        *mpRect = pOut->LogicToPixel( *mpRect );
        mbSpecialOutput = bSpecial;
    }
    else
        mbSpecialOutput = FALSE;
}

// -----------
// - Printer -
// -----------

void Printer::GetPreparedMetaFile( const GDIMetaFile& rInMtf, GDIMetaFile& rOutMtf,
                                   long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    const PrinterOptions&   rPrinterOptions = GetPrinterOptions();
    MetaAction*             pAct;
    ULONG                   i;
    BOOL                    bTransparent = FALSE;

    rOutMtf.Clear();

    if( !rPrinterOptions.IsReduceTransparency() ||
        ( PRINTER_TRANSPARENCY_AUTO == rPrinterOptions.GetReducedTransparencyMode() ) )
    {
        // watch for transparent drawing actions
        for( pAct = ( (GDIMetaFile&) rInMtf ).FirstAction(); pAct && !bTransparent; pAct = ( (GDIMetaFile&) rInMtf ).NextAction() )
        {
            if( ( META_TRANSPARENT_ACTION == pAct->GetType() ) ||
                ( META_FLOATTRANSPARENT_ACTION == pAct->GetType() ) )
            {
                bTransparent = TRUE;
            }
        }
    }

    // separate actions which are special actions or which are affected by special actions
    if( bTransparent )
    {
        Rectangle       aBoundRect;
        const ULONG     nCount = rInMtf.GetActionCount();
        VirtualDevice   aPaintVDev;
        ImplCheckRect*  pRects = new ImplCheckRect[ nCount ];
        ImplCheckRect*  pIListFirst = NULL;
        ImplCheckRect*  pIListPrev = NULL;
        ImplCheckRect*  pIListAct = NULL;
        ImplCheckRect*  pOListFirst = NULL;
        ImplCheckRect*  pOListLast = NULL;
        ImplCheckRect*  pOListAct = NULL;
        ImplCheckRect*  pO;

        aPaintVDev.mnDPIX = mnDPIX;
        aPaintVDev.mnDPIY = mnDPIY;
        aPaintVDev.mbOutput = FALSE;

        // create list of special action rects
        for( i = 0, pO = pRects, pAct = ( (GDIMetaFile&) rInMtf ).FirstAction(); pAct;
             pAct = ( (GDIMetaFile&) rInMtf ).NextAction(), i++, pO++ )
        {
            // is it a special action
            bTransparent = ( ( META_TRANSPARENT_ACTION == pAct->GetType() ) ||
                             ( META_FLOATTRANSPARENT_ACTION == pAct->GetType() ) );

            // execute action to get correct MapMode's etc.
            pAct->Execute( &aPaintVDev );

            // create (bounding) rect object
            pO->ImplCreate( pAct, &aPaintVDev, bTransparent );

            // add object to one of the lists
            if( pO->mbSpecialOutput )
            {
                if( !pOListFirst )
                    pOListFirst = pOListAct = pOListLast = pO;
                else
                    pOListAct = pOListAct->mpNext = pOListLast = pO;
            }
            else
            {
                if( !pIListFirst )
                    pIListFirst = pIListAct = pO;
                else
                    pIListAct = pIListAct->mpNext = pO;
            }
        }

        // find all intersections and create list of (bitmap)
        for( pO = pOListFirst; pO; pO = pO->mpNext )
        {
            pIListPrev = NULL, pIListAct = pIListFirst;

            while( pIListAct )
            {
                // move act object from in list to out list?
                if( pIListAct->Intersects( *pO ) )
                {
                    // mark it as special object
                    pIListAct->mbSpecialOutput = TRUE;

                    if( pIListPrev )
                        pIListPrev->mpNext = pIListAct->mpNext;
                    else
                        pIListFirst = pIListAct->mpNext;

                    pOListLast = pOListLast->mpNext = pIListAct;
                }
                else
                    pIListPrev = pIListAct;

                pIListAct = pIListAct->mpNext;
                pOListLast->mpNext = NULL;
            }
        }

        // calculate bounding rectangle of special actions
        for( pO = pOListFirst; pO; pO = pO->mpNext )
            aBoundRect.Union( *pO->mpRect );

        Point aPoint;
        const Rectangle aOutputRect( aPoint, GetOutputSizePixel() );

        aBoundRect.Intersection( aOutputRect );

        const double    fBmpArea = (double) aBoundRect.GetWidth() * aBoundRect.GetHeight();
        const double    fOutArea = (double) aOutputRect.GetWidth() * aOutputRect.GetHeight();

        // check if output doesn't exceed given size
        if( rPrinterOptions.IsReduceTransparency() &&
            ( PRINTER_TRANSPARENCY_AUTO == rPrinterOptions.GetReducedTransparencyMode() ) &&
            ( fBmpArea > ( 0.25 * fOutArea ) ) )
        {
            rOutMtf = rInMtf;
        }
        else
        {
            // create new bitmap action first
            if( aBoundRect.GetWidth() && aBoundRect.GetHeight() )
            {
                Point           aDstPtPix( aBoundRect.TopLeft() );
                Size            aDstSzPix;
                VirtualDevice   aMapVDev;

                rOutMtf.AddAction( new MetaPushAction( PUSH_MAPMODE ) );
                rOutMtf.AddAction( new MetaMapModeAction() );

                aPaintVDev.mbOutput = TRUE;
                aPaintVDev.SetDrawMode( GetDrawMode() );

                aMapVDev.mbOutput = FALSE;

                while( aDstPtPix.Y() <= aBoundRect.Bottom() )
                {
                    aDstPtPix.X() = aBoundRect.Left();
                    aDstSzPix = Size( MAX_TILE_WIDTH, MAX_TILE_HEIGHT );

                    if( ( aDstPtPix.Y() + aDstSzPix.Height() - 1L ) > aBoundRect.Bottom() )
                        aDstSzPix.Height() = aBoundRect.Bottom() - aDstPtPix.Y() + 1L;

                    while( aDstPtPix.X() <= aBoundRect.Right() )
                    {
                        if( ( aDstPtPix.X() + aDstSzPix.Width() - 1L ) > aBoundRect.Right() )
                            aDstSzPix.Width() = aBoundRect.Right() - aDstPtPix.X() + 1L;

                        if( !Rectangle( aDstPtPix, aDstSzPix ).Intersection( aBoundRect ).IsEmpty() &&
                            aPaintVDev.SetOutputSizePixel( aDstSzPix ) )
                        {
                            aPaintVDev.Push();
                            aMapVDev.Push();

                            aMapVDev.mnDPIX = aPaintVDev.mnDPIX = mnDPIX;
                            aMapVDev.mnDPIY = aPaintVDev.mnDPIY = mnDPIY;

                            for( i = 0, pO = pRects; i < nCount; i++, pO++ )
                            {
                                MetaAction*     pAction = pO->mpAct;
                                const USHORT    nType = pAction->GetType();

                                if( META_MAPMODE_ACTION == nType )
                                {
                                    pAction->Execute( &aMapVDev );

                                    MapMode     aMtfMap( aMapVDev.GetMapMode() );
                                    const Point aNewOrg( aMapVDev.PixelToLogic( aDstPtPix ) );

                                    aMtfMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                                    aPaintVDev.SetMapMode( aMtfMap );
                                }
                                else if( ( META_PUSH_ACTION == nType ) || ( META_POP_ACTION ) == nType )
                                {
                                    pAction->Execute( &aMapVDev );
                                    pAction->Execute( &aPaintVDev );
                                }
                                else if( META_GRADIENT_ACTION == nType )
                                {
                                    MetaGradientAction* pGradientAction = (MetaGradientAction*) pAction;
                                    DrawGradientEx( &aPaintVDev, pGradientAction->GetRect(), pGradientAction->GetGradient() );
                                }
                                else
                                    pAction->Execute( &aPaintVDev );

                                if( !( i % 4 ) )
                                    Application::Reschedule();
                            }

                            const BOOL bOldMap = mbMap;
                            mbMap = aPaintVDev.mbMap = FALSE;

                            Bitmap aBandBmp( aPaintVDev.GetBitmap( Point(), aDstSzPix ) );

                            // scale down bitmap, if requested
                            if( rPrinterOptions.IsReduceBitmaps() && rPrinterOptions.IsReducedBitmapIncludesTransparency() )
                            {
                                aBandBmp = GetPreparedBitmap( aDstPtPix, aDstSzPix,
                                                              Point(), aBandBmp.GetSizePixel(),
                                                              aBandBmp, nMaxBmpDPIX, nMaxBmpDPIY );
                            }

#ifdef DEBUG
                            // aBandBmp.Invert();
#endif

                            rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_BEGIN" ) );
                            rOutMtf.AddAction( new MetaBmpScaleAction( aDstPtPix, aDstSzPix, aBandBmp ) );
                            rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_END" ) );

                            aPaintVDev.mbMap = TRUE;
                            mbMap = bOldMap;
                            aMapVDev.Pop();
                            aPaintVDev.Pop();
                        }

                        // overlapping bands to avoid missing lines (e.g. PostScript)
                        aDstPtPix.X() += aDstSzPix.Width();
                    }

                    // overlapping bands to avoid missing lines (e.g. PostScript)
                    aDstPtPix.Y() += aDstSzPix.Height();
                }

                rOutMtf.AddAction( new MetaPopAction() );
            }

            // add normal actions
            for( i = 0, pO = pRects; i < nCount; i++, pO++ )
                if( !pO->mbSpecialOutput )
                    rOutMtf.AddAction( ( pO->mpAct->Duplicate(), pO->mpAct ) );

            rOutMtf.SetPrefMapMode( rInMtf.GetPrefMapMode() );
            rOutMtf.SetPrefSize( rInMtf.GetPrefSize() );
        }

        delete[] pRects;
    }
    else
        rOutMtf = rInMtf;
}

// -----------------------------------------------------------------------------

Bitmap Printer::GetPreparedBitmap( const Point& rDstPt, const Size& rDstSz,
                                   const Point& rSrcPt, const Size& rSrcSz,
                                   const Bitmap& rBmp, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    Bitmap aBmp( rBmp );

    if( !aBmp.IsEmpty() )
    {
        Point           aPoint;
        const Rectangle aBmpRect( aPoint, aBmp.GetSizePixel() );
        Rectangle       aSrcRect( rSrcPt, rSrcSz );

        // do cropping if neccessary
        if( aSrcRect.Intersection( aBmpRect ) != aBmpRect )
        {
            if( !aSrcRect.IsEmpty() )
                aBmp.Crop( aSrcRect );
            else
                aBmp.SetEmpty();
        }

        if( !aBmp.IsEmpty() )
        {
            // do downsampling if neccessary
            const Size      aDstSizeTwip( PixelToLogic( LogicToPixel( rDstSz ), MAP_TWIP ) );
            const Size      aBmpSize( aBmp.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * nMaxBmpDPIX / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * nMaxBmpDPIY / 1440.0;

            // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
            if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
                  ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
                ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
            {
                // do scaling
                Size            aNewBmpSize;
                const double    fBmpWH = fBmpPixelX / fBmpPixelY;
                const double    fMaxWH = fMaxPixelX / fMaxPixelY;

                if( fBmpWH < fMaxWH )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelY * fBmpWH );
                    aNewBmpSize.Height() = FRound( fMaxPixelY );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelX );
                    aNewBmpSize.Height() = FRound( fMaxPixelX / fBmpWH);
                }

                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                    aBmp.Scale( aNewBmpSize );
                else
                    aBmp.SetEmpty();
            }
        }
    }

    return aBmp;
}

// -----------------------------------------------------------------------------

BitmapEx Printer::GetPreparedBitmapEx( const Point& rDstPt, const Size& rDstSz,
                                       const Point& rSrcPt, const Size& rSrcSz,
                                       const BitmapEx& rBmpEx, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    BitmapEx aBmpEx( rBmpEx );

    if( !aBmpEx.IsEmpty() )
    {
        Point           aPoint;
        const Rectangle aBmpRect( aPoint, aBmpEx.GetSizePixel() );
        Rectangle       aSrcRect( rSrcPt, rSrcSz );

        // do cropping if neccessary
        if( aSrcRect.Intersection( aBmpRect ) != aBmpRect )
        {
            if( !aSrcRect.IsEmpty() )
                aBmpEx.Crop( aSrcRect );
            else
                aBmpEx.SetEmpty();
        }

        if( !aBmpEx.IsEmpty() )
        {
            // do downsampling if neccessary
            const Size      aDstSizeTwip( PixelToLogic( LogicToPixel( rDstSz ), MAP_TWIP ) );
            const Size      aBmpSize( aBmpEx.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * nMaxBmpDPIX / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * nMaxBmpDPIY / 1440.0;

            // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
            if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
                  ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
                ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
            {
                // do scaling
                Size            aNewBmpSize;
                const double    fBmpWH = fBmpPixelX / fBmpPixelY;
                const double    fMaxWH = fMaxPixelX / fMaxPixelY;

                if( fBmpWH < fMaxWH )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelY * fBmpWH );
                    aNewBmpSize.Height() = FRound( fMaxPixelY );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelX );
                    aNewBmpSize.Height() = FRound( fMaxPixelX / fBmpWH);
                }

                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                    aBmpEx.Scale( aNewBmpSize );
                else
                    aBmpEx.SetEmpty();
            }
        }
    }

    return aBmpEx;
}

// -----------------------------------------------------------------------------

void Printer::DrawGradientEx( OutputDevice* pOut, const Rectangle& rRect, const Gradient& rGradient )
{
    const PrinterOptions& rPrinterOptions = GetPrinterOptions();

    if( rPrinterOptions.IsReduceGradients() )
    {
        if( PRINTER_GRADIENT_STRIPES == rPrinterOptions.GetReducedGradientMode() )
        {
            if( !rGradient.GetSteps() || ( rGradient.GetSteps() > rPrinterOptions.GetReducedGradientStepCount() ) )
            {
                Gradient aNewGradient( rGradient );

                aNewGradient.SetSteps( rPrinterOptions.GetReducedGradientStepCount() );
                pOut->DrawGradient( rRect, aNewGradient );
            }
            else
                pOut->DrawGradient( rRect, rGradient );
        }
        else
        {
            const Color&    rStartColor = rGradient.GetStartColor();
            const Color&    rEndColor = rGradient.GetEndColor();
            const long      nR = ( ( (long) rStartColor.GetRed() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetRed() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nG = ( ( (long) rStartColor.GetGreen() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetGreen() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nB = ( ( (long) rStartColor.GetBlue() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetBlue() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const Color     aColor( (BYTE) nR, (BYTE) nG, (BYTE) nB );

            pOut->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            pOut->SetLineColor( aColor );
            pOut->SetFillColor( aColor );
            pOut->DrawRect( rRect );
            pOut->Pop();
        }
    }
    else
        pOut->DrawGradient( rRect, rGradient );
}

// -----------------------------------------------------------------------------

void Printer::DrawGradientEx( OutputDevice* pOut, const PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    const PrinterOptions& rPrinterOptions = GetPrinterOptions();

    if( rPrinterOptions.IsReduceGradients() )
    {
        if( PRINTER_GRADIENT_STRIPES == rPrinterOptions.GetReducedGradientMode() )
        {
            if( !rGradient.GetSteps() || ( rGradient.GetSteps() > rPrinterOptions.GetReducedGradientStepCount() ) )
            {
                Gradient aNewGradient( rGradient );

                aNewGradient.SetSteps( rPrinterOptions.GetReducedGradientStepCount() );
                pOut->DrawGradient( rPolyPoly, aNewGradient );
            }
            else
                pOut->DrawGradient( rPolyPoly, rGradient );
        }
        else
        {
            const Color&    rStartColor = rGradient.GetStartColor();
            const Color&    rEndColor = rGradient.GetEndColor();
            const long      nR = ( ( (long) rStartColor.GetRed() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetRed() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nG = ( ( (long) rStartColor.GetGreen() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetGreen() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nB = ( ( (long) rStartColor.GetBlue() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetBlue() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const Color     aColor( (BYTE) nR, (BYTE) nG, (BYTE) nB );

            pOut->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            pOut->SetLineColor( aColor );
            pOut->SetFillColor( aColor );
            pOut->DrawPolyPolygon( rPolyPoly );
            pOut->Pop();
        }
    }
    else
        pOut->DrawGradient( rPolyPoly, rGradient );
}
