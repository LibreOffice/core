/*************************************************************************
 *
 *  $RCSfile: print2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-20 16:52:07 $
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
#ifndef _SV_PRINT_HXX
#include <print.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

// -----------
// - Defines -
// -----------

#define MAX_BANDWIDTH 1000000

// -----------
// - statics -
// -----------

static USHORT aSpecialPrintActions[] =
{
    META_TRANSPARENT_ACTION,
    META_FLOATTRANSPARENT_ACTION
};

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
    mpAct = pAct;
    mpNext = NULL;

    switch( mpAct->GetType() )
    {
        case( META_PIXEL_ACTION ):
            mpRect = new Rectangle( ( (MetaPixelAction*) mpAct )->GetPoint(), Size( 1, 1 ) );
        break;

        case( META_POINT_ACTION ):
            mpRect = new Rectangle( ( (MetaPointAction*) mpAct )->GetPoint(), Size( 1, 1 ) );
        break;

        case( META_LINE_ACTION ):
        {
            MetaLineAction* pA = (MetaLineAction*) mpAct;
            mpRect = new Rectangle( pA->GetStartPoint(), pA->GetEndPoint() );
        }
        break;

        case( META_RECT_ACTION ):
            mpRect = new Rectangle( ( (MetaRectAction*) mpAct )->GetRect() );
        break;

        case( META_ROUNDRECT_ACTION ):
        {
            MetaRoundRectAction* pA = (MetaRoundRectAction*) mpAct;
            mpRect = new Rectangle( Polygon( pA->GetRect(),
                                             pA->GetHorzRound(),
                                             pA->GetVertRound() ).GetBoundRect() );
        }
        break;

        case( META_ELLIPSE_ACTION ):
        {
            MetaEllipseAction*  pA = (MetaEllipseAction*) mpAct;
            const Rectangle&    rRect = pA->GetRect();
            mpRect = new Rectangle( Polygon( rRect.Center(),
                                             rRect.GetWidth() >> 1,
                                             rRect.GetHeight() >> 1 ).GetBoundRect() );
        }
        break;

        case( META_ARC_ACTION ):
        {
            MetaArcAction* pA = (MetaArcAction*) mpAct;
            mpRect = new Rectangle( Polygon( pA->GetRect(),
                                             pA->GetStartPoint(),
                                             pA->GetEndPoint(), POLY_ARC ).GetBoundRect() );
        }
        break;

        case( META_PIE_ACTION ):
        {
            MetaPieAction* pA = (MetaPieAction*) mpAct;
            mpRect = new Rectangle( Polygon( pA->GetRect(),
                                             pA->GetStartPoint(),
                                             pA->GetEndPoint(), POLY_PIE ).GetBoundRect() );
        }
        break;

        case( META_CHORD_ACTION ):
        {
            MetaChordAction* pA = (MetaChordAction*) mpAct;
            mpRect = new Rectangle( Polygon( pA->GetRect(),
                                             pA->GetStartPoint(),
                                             pA->GetEndPoint(), POLY_CHORD ).GetBoundRect() );
        }
        break;

        case( META_POLYLINE_ACTION ):
        {
            MetaPolyLineAction* pA = (MetaPolyLineAction*) mpAct;
            mpRect = new Rectangle( pA->GetPolygon().GetBoundRect() );
        }
        break;

        case( META_POLYGON_ACTION ):
        {
            MetaPolygonAction* pA = (MetaPolygonAction*) mpAct;
            mpRect = new Rectangle( pA->GetPolygon().GetBoundRect() );
        }
        break;

        case( META_POLYPOLYGON_ACTION ):
        {
            MetaPolyPolygonAction* pA = (MetaPolyPolygonAction*) mpAct;
            mpRect = new Rectangle( pA->GetPolyPolygon().GetBoundRect() );
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
            const Point             aPt( pOut->LogicToPixel( pA->GetPoint() ) );
            const XubString         aString( pA->GetText(), pA->GetIndex(), pA->GetLen() );
            mpRect = new Rectangle( pOut->PixelToLogic( pOut->ImplGetTextBoundRect( aPt.X(), aPt.Y(), aString.GetBuffer(), aString.Len(), pA->GetDXArray() ) ) );
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
            mpRect = NULL;
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

void Printer::GetPreparedMetaFile( const GDIMetaFile& rInMtf, GDIMetaFile& rOutMtf, ULONG nFlags )
{
    const ULONG nSpecialCount = sizeof( aSpecialPrintActions ) / sizeof( aSpecialPrintActions[ 0 ] );
    MetaAction* pAct;
    ULONG       i, j;
    BOOL        bSpecial = FALSE;

    rOutMtf.Clear();

    // look, if we've got special actions
    for( pAct = ( (GDIMetaFile&) rInMtf ).FirstAction(); pAct && !bSpecial; pAct = ( (GDIMetaFile&) rInMtf ).NextAction() )
        for( i = 0; i < nSpecialCount; i++ )
            if( pAct->GetType() == aSpecialPrintActions[ i ] )
                bSpecial = TRUE;

    // separate actions which are special actions or which are affected by special actions
    if( bSpecial )
    {
        Rectangle       aBoundPixel;
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
            for( j = 0, bSpecial = FALSE; j < nSpecialCount && !bSpecial; j++ )
                if( pAct->GetType() == aSpecialPrintActions[ j ] )
                    bSpecial = TRUE;

            // execute action to get correct MapMode's etc.
            pAct->Execute( &aPaintVDev );

            // create (bounding) rect object
            pO->ImplCreate( pAct, &aPaintVDev, bSpecial );

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
            aBoundPixel.Union( *pO->mpRect );

        const Size aSzPix( aBoundPixel.GetSize() );

        // create new bitmap action first
        if( aSzPix.Width() && aSzPix.Height() )
        {
            Point           aDstPtPix( aBoundPixel.TopLeft() );
            Size            aDstSzPix( aSzPix.Width(), Max( MAX_BANDWIDTH / aSzPix.Width(), 2L ) );
            const long      nLastY = aDstPtPix.Y() + aSzPix.Height() - 1L;
            VirtualDevice   aMapVDev;

            rOutMtf.AddAction( new MetaPushAction( PUSH_MAPMODE ) );
            rOutMtf.AddAction( new MetaMapModeAction() );

            aPaintVDev.mbOutput = TRUE;
            aMapVDev.mbOutput = FALSE;

            while( aDstPtPix.Y() <= nLastY )
            {
                if( ( aDstPtPix.Y() + aDstSzPix.Height() - 1L ) > nLastY )
                    aDstSzPix.Height() = nLastY - aDstPtPix.Y() + 1L;

                if( aPaintVDev.SetOutputSizePixel( aDstSzPix ) )
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
                        else
                            pAction->Execute( &aPaintVDev );

                        if( !( i % 4 ) )
                            Application::Reschedule();
                    }

                    aPaintVDev.mbMap = FALSE;
                    Bitmap aBandBmp( aPaintVDev.GetBitmap( Point(), aDstSzPix ) );
#ifdef DEBUG
                    aBandBmp.Invert();
#endif
                    rOutMtf.AddAction( new MetaBmpScaleAction( aDstPtPix, aDstSzPix, aBandBmp ) );
                    aPaintVDev.mbMap = TRUE;

                    aMapVDev.Pop();
                    aPaintVDev.Pop();
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
        delete[] pRects;
    }
    else
        rOutMtf = rInMtf;
}
