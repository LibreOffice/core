/*************************************************************************
 *
 *  $RCSfile: colex.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-09 15:15:33 $
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
        }
        else
            SetFooter( FALSE );
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
//      SetPen( Pen( PEN_DOT ) );

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

        SetFillColor( Color( COL_WHITE ) );
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

}

/*-----------------25.10.96 09.16-------------------

--------------------------------------------------*/


void SwColumnOnlyExample::Paint( const Rectangle& rRect )
{
//      Pen aSolidPen(PEN_SOLID);
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
//      SetPen(Pen(PEN_DOT));
        if( !nColCount)
            DrawRect(aRect);
        else
        {
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
//          SetPen( aSolidPen );
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

/*-----------------25.02.94 21:22-------------------
   $Log: not supported by cvs2svn $
   Revision 1.2  2000/11/07 12:08:55  hjs
   use min/max from stl

   Revision 1.1.1.1  2000/09/18 17:14:37  hr
   initial import

   Revision 1.26  2000/09/18 16:05:32  willem.vandorp
   OpenOffice header added.

   Revision 1.25  2000/03/03 15:17:01  os
   StarView remainders removed

   Revision 1.24  1998/04/28 09:28:32  OS
   Abstand nur fuer existierende Spalten erfragen


      Rev 1.23   28 Apr 1998 11:28:32   OS
   Abstand nur fuer existierende Spalten erfragen

      Rev 1.22   16 Apr 1998 13:09:28   OS
   Spaltenabstand fuer Seitenbeispiel richtig #49516#

      Rev 1.21   28 Nov 1997 19:40:38   MA
   includes

      Rev 1.20   24 Nov 1997 17:40:12   MA
   include

      Rev 1.19   03 Nov 1997 13:19:44   MA
   precomp entfernt

      Rev 1.18   15 Aug 1997 12:15:24   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.17   12 Aug 1997 15:58:08   OS
   frmitems/textitem/paraitem aufgeteilt

      Rev 1.16   07 Aug 1997 14:59:08   OM
   Headerfile-Umstellung

      Rev 1.15   23 Apr 1997 11:08:48   OS
   ResId const

      Rev 1.14   21 Nov 1996 11:53:34   OS
   Umrandungsabstand mit anzeigen

      Rev 1.13   08 Nov 1996 14:07:54   OS
   ohne Spalten mit richtigem Hintergrund painten

      Rev 1.12   25 Oct 1996 14:56:56   OS
   neues Spaltenbeispiel ohne Seite

      Rev 1.11   28 Aug 1996 11:52:54   OS
   includes

      Rev 1.10   27 Mar 1996 16:02:56   OS
   Hoehe Kopf-/Fusszeilen richtig einstellen

      Rev 1.9   22 Mar 1996 14:20:58   MH
   add: include pageitem.hxx

      Rev 1.8   24 Nov 1995 16:58:10   OM
   PCH->PRECOMPILED

      Rev 1.7   21 Nov 1995 08:16:04   OS
   +pragma

      Rev 1.6   21 Nov 1995 08:00:48   OS
   Itemsate auf _SET pruefen, Seitengroesse init.

      Rev 1.5   17 Oct 1995 15:00:08   MA
   fix: SEXPORT'iert

      Rev 1.4   22 Aug 1995 09:10:02   MA
   svxitems-header entfernt

      Rev 1.3   07 Aug 1995 18:51:24   OS
   Trennlinien mit einzeichnen

      Rev 1.2   02 Aug 1995 17:47:50   OS
   Seitenspaltenbeispiel implementiert

      Rev 1.1   11 Jul 1995 15:42:06   OS
   Beispielfenster jetzt vom SvxPageWindow abgeleitet

      Rev 1.0   20 Apr 1995 17:38:02   OS
   Initial revision.

--------------------------------------------------*/


