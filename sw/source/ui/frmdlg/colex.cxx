/*************************************************************************
 *
 *  $RCSfile: colex.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dr $ $Date: 2001-06-22 16:14:15 $
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


#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "cmdid.h"
#include "hintids.hxx"
#include "uiparam.hxx"
#include <algorithm>


#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX //autogen
#include <svx/pageitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#include "colex.hxx"
#include "colmgr.hxx"


/*-----------------------------------------------------------------------
    Beschreibung: Uebernahme der aktualisierten Werte aus dem Set
 -----------------------------------------------------------------------*/


void SwColExample::UpdateExample( const SfxItemSet& rSet, SwColMgr* pMgr )
{
    pColMgr = pMgr;
    const SvxPageItem* pPage = 0;
    SfxItemPool* pPool = rSet.GetPool();
    USHORT nWhich = pPool->GetWhich( SID_ATTR_PAGE );

    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
    {
        // Ausrichtung
        pPage = (const SvxPageItem*)&rSet.Get( nWhich );

        if ( pPage )
            SetUsage( pPage->GetPageUsage() );
    }

    nWhich = pPool->GetWhich( SID_ATTR_PAGE_SIZE );

    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
    {
        // Orientation und Size aus dem PageItem
        const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get( nWhich );
        Size aSize( rSize.GetSize() );
        SetSize( aSize );
    }
    nWhich = RES_LR_SPACE;
    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
    {
        // linken und rechten Rand einstellen
        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)rSet.Get( nWhich );

        SetLeft( rLRSpace.GetLeft() );
        SetRight( rLRSpace.GetRight() );
    }
    else
    {
        SetLeft( 0 );
        SetRight( 0 );
    }

    nWhich = RES_UL_SPACE;

    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
    {
        // oberen und unteren Rand einstellen
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)rSet.Get( nWhich );

        SetTop( rULSpace.GetUpper() );
        SetBottom( rULSpace.GetLower() );
    }
    else
    {
        SetTop( 0 );
        SetBottom( 0 );
    }


    // Kopfzeilen-Attribute auswerten
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( pPool->GetWhich( SID_ATTR_PAGE_HEADERSET),
            FALSE, &pItem ) )
    {
        const SfxItemSet& rHeaderSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get( pPool->GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize =
                (const SvxSizeItem&)rHeaderSet.Get(pPool->GetWhich(SID_ATTR_PAGE_SIZE));

            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)rHeaderSet.Get(
                                        pPool->GetWhich(SID_ATTR_ULSPACE));
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)rHeaderSet.Get(
                                        pPool->GetWhich(SID_ATTR_LRSPACE));

            SetHdHeight( rSize.GetSize().Height() - rUL.GetLower());
            SetHdDist( rUL.GetLower() );
            SetHdLeft( rLR.GetLeft() );
            SetHdRight( rLR.GetRight() );
            SetHeader( TRUE );
            if ( rHeaderSet.GetItemState( RES_BACKGROUND ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem =
                    (const SvxBrushItem&)rHeaderSet.Get( RES_BACKGROUND );
                SetHdColor( rItem.GetColor() );
            }
            if ( rHeaderSet.GetItemState( RES_BOX ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rHeaderSet.Get( RES_BOX );
                SetHdBorder( rItem );
            }
        }
        else
            SetHeader( FALSE );
    }

    if( SFX_ITEM_SET == rSet.GetItemState( pPool->GetWhich( SID_ATTR_PAGE_FOOTERSET),
            FALSE, &pItem ) )
    {
        const SfxItemSet& rFooterSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rFooterOn =
            (const SfxBoolItem&)rFooterSet.Get( SID_ATTR_PAGE_ON );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize =
                (const SvxSizeItem&)rFooterSet.Get( pPool->GetWhich( SID_ATTR_PAGE_SIZE ) );

            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)rFooterSet.Get(
                                        pPool->GetWhich( SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)rFooterSet.Get(
                                        pPool->GetWhich( SID_ATTR_LRSPACE ) );

            SetFtHeight( rSize.GetSize().Height() - rUL.GetUpper());
            SetFtDist( rUL.GetUpper() );
            SetFtLeft( rLR.GetLeft() );
            SetFtRight( rLR.GetRight() );
            SetFooter( TRUE );
            if( rFooterSet.GetItemState( RES_BACKGROUND ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem =
                    (const SvxBrushItem&)rFooterSet.Get( RES_BACKGROUND );
                SetFtColor( rItem.GetColor() );
            }
            if( rFooterSet.GetItemState( RES_BOX ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rFooterSet.Get( RES_BOX );
                SetFtBorder( rItem );
            }
        }
        else
            SetFooter( FALSE );
    }
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BACKGROUND,
            FALSE, &pItem ) )
    {
        SetColor( ( (const SvxBrushItem*)pItem )->GetColor() );
        const Graphic* pGrf = ( (const SvxBrushItem*)pItem )->GetGraphic();

        if ( pGrf )
        {
            Bitmap aBitmap = pGrf->GetBitmap();
            SetBitmap( &aBitmap );
        }
        else
            SetBitmap( NULL );
    }

    Invalidate();
}
/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/


void SwColExample::DrawPage( const Point& rOrg,
                            const BOOL bSecond,
                            const BOOL bEnabled )
{
    SvxPageWindow::DrawPage( rOrg, bSecond, bEnabled );
    if( pColMgr && pColMgr->GetCount() >1 )
    {
        long nL = GetLeft();
        long nR = GetRight();

        if ( GetUsage() == SVX_PAGE_MIRROR && !bSecond )
        {
            // fuer gespiegelt drehen
            nL = GetRight();
            nR = GetLeft();
        }

        SetFillColor( Color( COL_LIGHTGRAY ) );
        Rectangle aRect;
        aRect.Right() = rOrg.X() + GetSize().Width() - nR;
        aRect.Left()  = rOrg.X() + nL;
        aRect.Top()   = rOrg.Y() + GetTop()
                        + GetHdHeight() + GetHdDist();
        aRect.Bottom()= rOrg.Y() + GetSize().Height() - GetBottom()
                        - GetFtHeight() - GetFtDist();
        DrawRect(aRect);

        SetFillColor( GetColor() );
        USHORT nColumnCount = pColMgr->GetCount();
        for(USHORT i = 0; i < nColumnCount; i++)
        {
            aRect.Right() = aRect.Left() + pColMgr->GetColWidth( i );
            DrawRect(aRect);
            if(i < nColumnCount - 1)
                aRect.Left() = aRect.Right() + pColMgr->GetGutterWidth(i);
        }
        if(pColMgr->HasLine())
        {
//          SetPen( Pen ( PEN_SOLID ) );
            Point aUp( rOrg.X() + nL, rOrg.Y() + GetTop() );
            Point aDown( rOrg.X() + nL, rOrg.Y() + GetSize().Height()
                        - GetBottom() - GetFtHeight() - GetFtDist() );

            if( pColMgr->GetLineHeightPercent() != 100 )
            {
                long nLength = aDown.Y() - aUp.Y();
                nLength -= nLength * pColMgr->GetLineHeightPercent() / 100;
                switch(pColMgr->GetAdjust())
                {
                    case COLADJ_BOTTOM: aUp.Y() += nLength; break;
                    case COLADJ_TOP: aDown.Y() -= nLength; break;
                    case COLADJ_CENTER:
                          aUp.Y() += nLength / 2;
                          aDown.Y() -= nLength / 2;
                    break;
                }
            }

            int nDist;
            for( i = 0; i < nColumnCount -  1; i++)
            {
                int nGutter = pColMgr->GetGutterWidth(i);
                nDist = pColMgr->GetColWidth( i ) + nGutter;
                nDist -= (i == 0) ?
                    nGutter/2 :
                        0;
                aUp.X() += nDist;
                aDown.X() += nDist;
                DrawLine( aUp, aDown );

            }
        }
    }
}



SwColExample::SwColExample(Window* pPar, const ResId& rResId ) :
                                SvxPageWindow(pPar, rResId )
{
    SetSize(Size(11907, 16433));// DIN A4
}

/*-----------------25.10.96 09.15-------------------

--------------------------------------------------*/


SwColumnOnlyExample::SwColumnOnlyExample( Window* pParent, const ResId& rResId) :
    Window(pParent, rResId),
    aFrmSize(1,1),
    nDistance(0)
{
    SetMapMode( MapMode( MAP_TWIP ) );
    aWinSize = GetOutputSizePixel();
    aWinSize.Height() -= 4;
    aWinSize.Width() -= 4;

    aWinSize = PixelToLogic( aWinSize );

    SetBorderStyle( WINDOW_BORDER_MONO );
}

/*-----------------25.10.96 09.16-------------------

--------------------------------------------------*/


void SwColumnOnlyExample::Paint( const Rectangle& rRect )
{
    long nWidth = aFrmSize.Width();
    long nHeight = aFrmSize.Height();
    Fraction aXScale( aWinSize.Width(), std::max( (long)(nWidth + nWidth / 8), (long) 1 ) );
    Fraction aYScale( aWinSize.Height(), std::max( nHeight, (long) 1 ) );
    MapMode aMapMode( GetMapMode() );
    aMapMode.SetScaleX( aXScale );
    aMapMode.SetScaleY( aYScale );
    SetMapMode( aMapMode );

    Size aLogSize(PixelToLogic(GetOutputSizePixel()));
    Point aTL(  (aLogSize.Width() - aFrmSize.Width()) / 2,
                (aLogSize.Height() - aFrmSize.Height()) / 2);
    Rectangle aRect(aTL, aFrmSize);

    SetFillColor( Color( COL_WHITE ) );
//      SetPen(aSolidPen);
    DrawRect(aRect);

    Size aInside(aFrmSize.Width() - nDistance, aFrmSize.Height() - nDistance);
    long nDist2 = nDistance / 2;
    aTL.X() += nDist2;
    aTL.Y() += nDist2;
    Rectangle aInsRect(aTL, aInside);
    DrawRect(aInsRect);

    SetFillColor(Color( COL_LIGHTGRAY ) );

    //Spaltentrenner?
    long nLength = aLogSize.Height() - 2 * aTL.Y();
    Point aUp( aTL );
    Point aDown( aTL.X(), nLength );
    BOOL bLines = FALSE;
    if(aCols.GetLineAdj() != COLADJ_NONE)
    {
        bLines = TRUE;

        USHORT nPercent = aCols.GetLineHeight();
        if( nPercent != 100 )
        {
            nLength -= nLength * nPercent / 100;
            switch(aCols.GetLineAdj())
            {
                case COLADJ_BOTTOM: aUp.Y() += nLength; break;
                case COLADJ_TOP: aDown.Y() -= nLength; break;
                case COLADJ_CENTER:
                        aUp.Y() += nLength / 2;
                        aDown.Y() -= nLength / 2;
                break;
            }
        }

    }

    const SwColumns& rCols = aCols.GetColumns();
    USHORT nColCount = rCols.Count();
    if( nColCount )
    {
        DrawRect(aRect);
        SetFillColor( Color( COL_WHITE ) );
        Rectangle aFrmRect(aTL, aInside);
        long nSum = aTL.X();
        for(USHORT i = 0; i < nColCount; i++)
        {
            SwColumn* pCol = rCols[i];
            aFrmRect.Left()    = nSum + pCol->GetLeft();//nSum + pCol->GetLeft() + aTL.X();
            nSum              += pCol->GetWishWidth();
            aFrmRect.Right()   = nSum - pCol->GetRight();
            DrawRect(aFrmRect);
        }
        if(bLines )
        {
            nSum = aTL.X();
            for(USHORT i = 0; i < nColCount - 1; i++)
            {
                nSum += rCols[i]->GetWishWidth();
                aUp.X() = nSum;
                aDown.X() = nSum;
                DrawLine(aUp, aDown);
            }
        }
    }
}

/*-----------------25.10.96 12.05-------------------

--------------------------------------------------*/


void SwColumnOnlyExample::SetFrameSize(const Size& rS, long nDist)
{
    aFrmSize  = rS;
    nDistance = 2 * nDist;
    ::FitToActualSize(aCols, (USHORT)aFrmSize.Width());
}

