/*************************************************************************
 *
 *  $RCSfile: scrrect.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:29 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SCRRECT_HXX
#include <scrrect.hxx>      // SwScrollRect, SwScrollRects
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _CURSOR_HXX //autogen
#include <vcl/cursor.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#include "crsrsh.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "doc.hxx"

DBG_NAME(RefreshTimer);

//TODO: Why BRUSH_SIZE?
#define SW_SV_BRUSH_SIZE 8

SV_IMPL_VARARR(SwStripeArr,SwStripe);
SV_IMPL_OP_PTRARR_SORT(SwScrollStripes, SwStripesPtr);
SV_IMPL_OP_PTRARR_SORT(SScrAreas, SwScrollAreaPtr);

/*****************************************************************************
|*
|*  ViewShell::AddScrollRect()
|*
|*  Creation            MA 07. Mar. 94
|*  Last change         AMA 20. July 00
|*
|*  Description
|*  ViewShell::AddScrollRect(..) passes a registration from a scrolling frame or
|*  rectangle to all ViewShells and SwViewImps respectively.
|*
******************************************************************************/

void ViewShell::AddScrollRect( const SwFrm *pFrm, const SwRect &rRect,
    long nOfs )
{
    ASSERT( pFrm, "Where is my friend, the frame?" );
    BOOL bScrollOn = TRUE;

#ifdef NOSCROLL
    //Auch im Product per speziellem Compilat abschaltbar.
    bScrollOn = FALSE;
#endif

    if( bScrollOn && Imp()->IsScroll() && nOfs <= SHRT_MAX && nOfs >= SHRT_MIN )
    {
        ViewShell *pSh = this;
        do
        {
            pSh->Imp()->AddScrollRect( pFrm, rRect, nOfs );
            pSh = (ViewShell*)pSh->GetNext();
        } while ( pSh != this );
    }
    else
        AddPaintRect( rRect );
}

/******************************************************************************
|*
|*  ViewShell::Scroll()
|*
|*  Ersterstellung      MA 07. Mar. 94
|*  Last change         AMA 21. July 00
|*
|*  Description
|*  ViewShell::Scroll() scrolls all rectangles in the pScrollRects-list and
|*  transfers the critical lines by calling SwViewImp::MoveScrollArea(..).
|*
******************************************************************************/

void ViewShell::Scroll()
{
    SwScrollAreas *pScrollRects = Imp()->GetScrollRects();
    if ( pScrollRects )
    {
        bPaintWorks = FALSE;
        ASSERT( pScrollRects->Count(), "ScrollRects ohne ScrollRects." );

        //Abgleichen der Region mit den Scroll-Bereichen!!!
        //Wenn eines der Scroll-Rechtecke ungueltig wird, so muss
        //der PaintMode wahrscheinlich auf PAINT_BACKGROUND umgeschaltet
        //werden.

        //Auf die Richtung kommt es an:
        //- Bei einem pos. Ofst muss von hinten nach vorn gescrollt werden.
        //- Bei einem neg. Ofst muss von vorn nach hinten gescrollt werden.
        const BOOL bPositive = (*pScrollRects)[0]->GetOffs() > 0;

        int i = bPositive ? pScrollRects->Count()-1 : 0;

        for ( ; bPositive ? i >= 0 : i < (int)pScrollRects->Count();
                bPositive ? --i : ++i )
        {
            const SwScrollArea &rScroll = *(*pScrollRects)[i];
            if( rScroll.Count() )
            {
                int j = bPositive ? rScroll.Count()-1 : 0;
                for ( ; bPositive ? j >= 0 : j < (int)rScroll.Count();
                    bPositive ? --j : ++j )
                {
                    const SwStripes& rStripes = *rScroll[j];
                      Rectangle aRectangle( rStripes.GetMin(),
                          rStripes.GetY() - rScroll.GetOffs(),
                          rStripes.GetRight(),
                          rStripes.GetBottom() - rScroll.GetOffs() );
                      GetWin()->Scroll( 0, rScroll.GetOffs(), aRectangle,
                          SCROLL_CHILDREN);
                      SwRect aRect( aRectangle );
                      Imp()->ScrolledRect( aRect, rScroll.GetOffs() );
                      if ( bPositive )
                          aRect.Bottom( aRect.Top() + rScroll.GetOffs()-1 );
                      else
                          aRect.Top( aRect.Bottom() + rScroll.GetOffs() );
                      Imp()->AddPaintRect( aRect );
                }
            }
        }
        if ( !Imp()->IsScrolled() )
            Imp()->SetScrolled();

        Imp()->MoveScrollArea();
        bPaintWorks = TRUE;
    }
}

/******************************************************************************
|*
|*  ViewShell::SetNoNextScroll()
|*
|*  Ersterstellung      MA 08. Mar. 94
|*  Letzte Aenderung    MA 08. Mar. 94
|*
******************************************************************************/

void ViewShell::SetNoNextScroll()
{
    ViewShell *pSh = this;
    do
    {   pSh->Imp()->ResetNextScroll();
        pSh = (ViewShell*)pSh->GetNext();

    } while ( pSh != this );
}

/******************************************************************************
|*
|*  SwViewImp::AddScrollRect()
|*
|*  Ersterstellung      MA 07. Mar. 94
|*  Last change         AMA 21. July 00
|*
|*  Adds a scrollable rectangle and his critical lines to the list.
|*
******************************************************************************/

void SwViewImp::AddScrollRect( const SwFrm *pFrm, const SwRect &rRect,
    long nOffs )
{
    ASSERT( nOffs != 0, "Scrollen ohne Ofst." );
    SwRect aRect( rRect );
    aRect.Pos().Y() -= nOffs;
    if( aRect.IsOver( pSh->VisArea() ) )
    {
        ASSERT( pSh->GetWin(), "Scrolling without outputdevice" );
        aRect._Intersection( pSh->VisArea() );
        aRect.Pos().Y() += nOffs;
        SwStripes *pStripes = new SwStripes( aRect );
        if( pFrm->IsTxtFrm() )
            ((SwTxtFrm*)pFrm)->CriticalLines(*pSh->GetWin(), *pStripes, nOffs);
        else
            pStripes->Insert( SwStripe( aRect.Top(), aRect.Height() ), 0 );
        if ( !pScrollRects )
            pScrollRects = new SwScrollAreas;
        pScrollRects->InsertCol( SwScrollColumn(pFrm->Frm(),nOffs), pStripes );
    }
    else
        AddPaintRect( rRect );
}

/******************************************************************************
|*
|*  SwViewImp::MoveScrollArea()
|*
|*  Creation            AMA 10. July 00
|*  Last change         AMA 21. July 00
|*
|*  Transfers the areas after scrolling to the scrolled list, but only those
|*  parts with critical lines.
|*
******************************************************************************/

void SwViewImp::MoveScrollArea()
{
    if( !pScrolledArea )
        pScrolledArea = new SwScrollAreas;
    for( long nIdx = 0; nIdx < pScrollRects->Count(); ++nIdx )
    {
        SwScrollArea *pScr = (*pScrollRects)[ nIdx ];
        if( pScr->Compress() )
            delete pScr;
        else
        {
            USHORT nIdx;
            if( pScrolledArea->Seek_Entry( pScr, &nIdx ) )
                pScrolledArea->GetObject(nIdx)->Add( pScr );
            else
                pScrolledArea->Insert( pScr );
        }
    }
    delete pScrollRects;
    pScrollRects = NULL;
}

/******************************************************************************
|*
|*  SwViewImp::FlushScrolledArea()
|*
|*  Creation            AMA 10. July 00
|*  Last change         AMA 21. July 00
|*
|*  Flushes the scrolled critical lines, that is transfer them to AddPaintRect()
|*  and remove them from the list.
|*
******************************************************************************/

void SwViewImp::FlushScrolledArea()
{
    long nCount = pScrolledArea->Count();
    while( nCount )
    {
        SwScrollArea* pScroll = (*pScrolledArea)[--nCount];
        long nCnt = pScroll->Count();
        while( nCnt )
        {
            SwStripes* pStripes = (*pScroll)[--nCnt];
            SwRect aRect( pStripes->GetMin(), 0, pStripes->GetWidth(), 0 );
            for( long i = 0; i < pStripes->Count(); ++i )
            {
                aRect.Top( (*pStripes)[i].GetY() );
                aRect.Height( (*pStripes)[i].GetHeight() );
                AddPaintRect( aRect );
            }
            pScroll->Remove( nCnt );
            delete pStripes;
        }
        pScrolledArea->Remove( nCount );
        delete pScroll;
    }
    delete pScrolledArea;
    pScrolledArea = NULL;
}

/******************************************************************************
|*
|*  SwViewImp::_FlushScrolledArea(..)
|*
|*  Creation            AMA 10. July 00
|*  Last change         AMA 21. July 00
|*
|*  The critical lines, which overlaps with the given rectangle, will be united
|*  with the rectangle and removed from the list.
|*
******************************************************************************/

BOOL SwViewImp::_FlushScrolledArea( SwRect& rRect )
{
    long nCount = pScrolledArea->Count();
    BOOL bRet = FALSE;
    for( long i = pScrolledArea->Count(); i; )
    {
        SwScrollArea* pScroll = (*pScrolledArea)[--i];
        for( long j = pScroll->Count(); j; )
        {
            SwStripes* pStripes = (*pScroll)[--j];
            if( pStripes->Count() )
            {
                SwRect aRect( pStripes->GetMin(), pStripes->GetY(),
                    pStripes->GetWidth(), pStripes->GetHeight() );
                if( rRect.IsOver( aRect ) )
                {
                    for( long nI = pStripes->Count(); nI; )
                    {
                        aRect.Top( (*pStripes)[--nI].GetY() );
                        aRect.Height( (*pStripes)[nI].GetHeight() );
                        if( rRect.IsOver( aRect ) )
                        {
                            rRect.Union( aRect );
                            bRet = TRUE;
                            pStripes->Remove( nI );
                            nI = pStripes->Count();
                        }
                    }
                }
            }
            if( !pStripes->Count() )
            {
                pScroll->Remove( j );
                delete pStripes;
            }
        }
        if( !pScroll->Count() )
        {
            pScrolledArea->Remove( pScroll );
            delete pScroll;
        }
    }
    if( !pScrolledArea->Count() )
    {
        DELETEZ( pScrolledArea );
        SetNextScroll();
    }
    return bRet;
}

/******************************************************************************
|*
|*  SwViewImp::RefreshScrolledHdl(..)
|*
|*  Creation            MA 06. Oct. 94
|*  Last change         AMA 21. July 00
|*
|*  Every timerstop one of the critical lines will be painted.
|*
******************************************************************************/

IMPL_LINK( SwViewImp, RefreshScrolledHdl, Timer *, EMPTYARG )
{
    DBG_PROFSTART( RefreshTimer );

    if ( !IsScrolled() )
    {   DBG_PROFSTOP( RefreshTimer );
        return 0;
    }

    SET_CURR_SHELL( GetShell() );

    //Kein Refresh wenn eine Selektion besteht.
    if ( GetShell()->IsA( TYPE(SwCrsrShell) ) &&
         (((SwCrsrShell*)GetShell())->HasSelection() ||
          ((SwCrsrShell*)GetShell())->GetCrsrCnt() > 1))
    {
        DBG_PROFSTOP( RefreshTimer );
        return 0;
    }

    if( pScrolledArea )
    {
        BOOL bFound = FALSE;
        const SwRect aRect( GetShell()->VisArea() );
        BOOL bNoRefresh = GetShell()->IsA( TYPE(SwCrsrShell) ) &&
            ( ((SwCrsrShell*)GetShell())->HasSelection() ||
              ((SwCrsrShell*)GetShell())->GetCrsrCnt() > 1 );
        if( pScrolledArea->Count() )
        {
            SwScrollArea* pScroll = pScrolledArea->GetObject(0);
            ASSERT( pScroll->Count(), "Empty scrollarea" );
            SwStripes* pStripes = pScroll->GetObject(0);
            ASSERT( pStripes->Count() > 1, "Empty scrollstripes" );
            const SwStripe &rStripe = pStripes->GetObject(1);
            SwRect aTmpRect( pScroll->GetX(), rStripe.GetY(),
                             pScroll->GetWidth(), rStripe.GetHeight() );
            if( aTmpRect.IsOver( aRect ) )
            {
                SwSaveHdl aSaveHdl( this );
                 if( !bNoRefresh )
                    _RefreshScrolledArea( aTmpRect );
            }
            pStripes->Remove( 1 );
            if( pStripes->Count() < 2 )
            {
                pScroll->Remove( USHORT(0) );
                delete pStripes;
            }
            if( !pScroll->Count() )
            {
                pScrolledArea->Remove( pScroll );
                delete pScroll;
            }
        }
        if( !pScrolledArea->Count() )
        {
            delete pScrolledArea;
            pScrolledArea = 0;
        }
    }

    //All done?
    if( !pScrolledArea || !pScrolledArea->Count()
        )
    {
        ResetScrolled();
        SetNextScroll();
        aScrollTimer.Stop();
    }

    DBG_PROFSTOP( RefreshTimer );
    return 0;
}

/******************************************************************************
|*
|*  SwViewImp::_ScrolledRect(..)
|*
|*  Creation            AMA 20. July 00
|*  Last change         AMA 21. July 00
|*
|*  handles the problem of scrolled criticals lines, when they are a part of
|*  a scrolling area again. In this case, their rectangle has to move to the
|*  right position.
|*
******************************************************************************/

void SwViewImp::_ScrolledRect( const SwRect& rRect, long nOffs )
{
    for( long i = pScrolledArea->Count(); i; )
    {
        SwScrollArea* pScroll = (*pScrolledArea)[--i];
        ASSERT( pScroll->Count() == 1, "Missing scrollarea compression 1" );
        SwStripes* pStripes = (*pScroll)[0];
        if( pStripes->Count() )
        {
            SwRect aRect( pStripes->GetMin(), pStripes->GetY(),
                pStripes->GetWidth(), pStripes->GetHeight() );
            if( rRect.IsOver( aRect ) )
            {
                BOOL bRecalc = FALSE;
                for( long nI = pStripes->Count(); nI; )
                {
                    aRect.Top( (*pStripes)[--nI].GetY() );
                    aRect.Height( (*pStripes)[nI].GetHeight() );
                    if( rRect.IsInside( aRect ) )
                    {
                        (*pStripes)[nI].Y() += nOffs;
                        bRecalc = TRUE;
                    }
                }
                if( bRecalc )
                    pStripes->Recalc();
            }
        }
    }
}

/******************************************************************************
|*
|*  SwViewImp::_RefreshScrolledArea()
|*
******************************************************************************/

//Berechnen der Hoehe fuer das virtuelle Device, Breite und maximaler
//Speicherbedarf sind vorgegeben.
#define MAXKB 180L

void lcl_CalcVirtHeight( OutputDevice *pOut, Size &rSz )
{
    char nBytes;
    const ULONG nColorCount = pOut->GetColorCount();
    if( 256 >= nColorCount )                // 2^8
        nBytes = 1;
    else
        if( USHRT_MAX >= nColorCount )      // 2^16
            nBytes = 2;
        else
            if( 16777216 >= nColorCount )   // 2^24
                nBytes = 3;
            else
                nBytes = 4;                 // 2^n

    rSz = pOut->LogicToPixel( rSz );

    long nKB = MAXKB * 1000;
    nKB /= nBytes;
    rSz.Height() = nKB / rSz.Width();

    rSz = pOut->PixelToLogic( rSz );
}

void SwViewImp::_RefreshScrolledArea( const SwRect &rRect )
{
    SwRect aScRect( rRect );
    aScRect.Intersection( GetShell()->VisArea() );

    if( aScRect.IsEmpty() )
        return;

    BOOL bShowCrsr = FALSE;
    Window *pWin = GetShell()->GetWin();
    if ( pWin && pWin->GetCursor() && pWin->GetCursor()->IsVisible() )
    {
        bShowCrsr = TRUE;
        pWin->GetCursor()->Hide();
    }

    //Virtuelles Device erzeugen und einstellen.
    OutputDevice *pOld = GetShell()->GetOut();
    VirtualDevice *pVout = new VirtualDevice( *pOld );
    MapMode aMapMode( pOld->GetMapMode() );
    pVout->SetMapMode( aMapMode );
    Size aSize( aScRect.Width(), 0 );
    lcl_CalcVirtHeight( pOld, aSize );
    if ( aSize.Height() > aScRect.Height() )
        aSize.Height() = aScRect.Height() + 50;

    //unten in der Schleife lassen wir die Rechtecke ein wenig ueberlappen,
    //das muss auch bei der Groesse beruecksichtigt werden.
    aSize = pOld->LogicToPixel( aSize );
    aSize.Width() += SW_SV_BRUSH_SIZE + 4; aSize.Height() += SW_SV_BRUSH_SIZE + 4;
    aSize = pOld->PixelToLogic( aSize );

    const SwRootFrm* pLayout = GetShell()->GetLayout();

    if( pVout->SetOutputSize( aSize ) )
    {
        pVout->SetLineColor( pOld->GetLineColor() );
        pVout->SetFillColor( pOld->GetFillColor() );

        //Virtuelles Device in die ViewShell 'selektieren'
        GetShell()->pOut = pVout;

        const SwFrm *pPg = GetFirstVisPage();
        do
        {
            SwRect aRect( pPg->Frm() );
            if ( aRect.IsOver( aScRect ) )
            {
                aRect._Intersection( aScRect );
                do
                {   Rectangle aTmp( aRect.SVRect() );
                    long nTmp = aTmp.Top() + aSize.Height();
                    if ( aTmp.Bottom() > nTmp )
                        aTmp.Bottom() = nTmp;

                    aTmp = pOld->LogicToPixel( aTmp );
                    aTmp.Top()  -= 2; aTmp.Bottom() += 2;
                    aTmp.Left() -= 2; aTmp.Right()  += 2;
                    aTmp.Left() -= aTmp.Left() % SW_SV_BRUSH_SIZE;
                    aTmp.Top()  -= aTmp.Top()  % SW_SV_BRUSH_SIZE;
                    aTmp = pOld->PixelToLogic( aTmp );
                    SwRect aTmp2( aTmp );

                    Point aOrigin( aTmp2.Pos() );
                    aOrigin.X() = -aOrigin.X(); aOrigin.Y() = -aOrigin.Y();
                    aMapMode.SetOrigin( aOrigin );
                    pVout->SetMapMode( aMapMode );

                    pLayout->Paint( aTmp2 );
                    pOld->DrawOutDev( aTmp2.Pos(), aTmp2.SSize(),
                                      aTmp2.Pos(), aTmp2.SSize(), *pVout );

                    aRect.Top( aRect.Top() + aSize.Height() );
                    aScRect.Top( aRect.Top() );

                } while ( aRect.Height() > 0 );
            }
            pPg = pPg->GetNext();

        } while ( pPg && pPg->Frm().IsOver( GetShell()->VisArea() ) );

        GetShell()->pOut = pOld;
        delete pVout;
        if( GetShell()->GetViewOptions()->IsControl() )
            PaintLayer( GetShell()->GetDoc()->GetControlsId(), aScRect );
    }
    else
    {
        delete pVout;
        pLayout->Paint( aScRect );
    }

    if ( bShowCrsr )
        pWin->GetCursor()->Show();
}

/******************************************************************************
|*
|*  SwViewImp::RefreshScrolledArea()
|*
|*  Ersterstellung      MA 06. Oct. 94
|*  Letzte Aenderung    MA 19. Apr. 95
|*
******************************************************************************/

void SwViewImp::RefreshScrolledArea( SwRect &rRect )
{
    //1. Wird auch von der CrsrShell gerufen, um ggf. den Bereich, in den der
    //Crsr gesetzt wird (Absatz, ganze Zeile bei einer Tabelle) aufzufrischen.
    //Allerdings kann es dann natuerlich sein, dass das Rechteck ueberhaupt
    //nicht mit aufzufrischenden Bereichen ueberlappt.
    //2. Kein Refresh wenn eine Selektion besteht.
    if( (GetShell()->IsA( TYPE(SwCrsrShell) ) &&
          (((SwCrsrShell*)GetShell())->HasSelection() ||
           ((SwCrsrShell*)GetShell())->GetCrsrCnt() > 1)))
    {
        return;
    }

    if( pScrolledArea && pScrolledArea->Count() &&
        !( ( GetShell()->IsA( TYPE(SwCrsrShell) ) &&
        ( ((SwCrsrShell*)GetShell())->HasSelection() ||
          ((SwCrsrShell*)GetShell())->GetCrsrCnt() > 1) ) ) )
    {
        for( long i = pScrolledArea->Count(); i; )
        {
            SwScrollArea* pScroll = (*pScrolledArea)[--i];
            for( long j = pScroll->Count(); j; )
            {
                SwStripes* pStripes = (*pScroll)[--j];
                if( pStripes->Count() )
                {
                    SwRect aRect( pStripes->GetMin(), pStripes->GetY(),
                        pStripes->GetWidth(), pStripes->GetHeight() );
                    if( rRect.IsOver( aRect ) )
                    {
                        for( long nI = pStripes->Count(); nI; )
                        {
                            aRect.Top( (*pStripes)[--nI].GetY() );
                            aRect.Height( (*pStripes)[nI].GetHeight() );
                            if( rRect.IsOver( aRect ) )
                            {
                                pStripes->Remove( nI );
                                _RefreshScrolledArea( aRect );
                            }
                        }
                    }
                }
                if( !pStripes->Count() )
                {
                    pScroll->Remove( j );
                    delete pStripes;
                }
            }
            if( !pScroll->Count() )
            {
                pScrolledArea->Remove( pScroll );
                delete pScroll;
            }
            //Ist da jemand ungeduldig?
            //Nur Mouse und Keyboard, weil wir sonst von jeder billigen
            //Uhr unterbrochen werden.
            if( GetpApp()->AnyInput( INPUT_MOUSEANDKEYBOARD ) )
                break;
        }
    }

    //All done?
    if( !pScrolledArea || !pScrolledArea->Count() )
    {
        ResetScrolled();
        SetNextScroll();
        aScrollTimer.Stop();
    }
}

SwStripes& SwStripes::operator+=( const SwStripes& rOther )
{
    if( !Count() )
    {
        Insert( &rOther, 0 );
        SetMin( rOther.GetMin() );
        SetMax( rOther.GetMax() );
        return *this;
    }

    long nCnt = rOther.Count();
    if( nCnt )
    {
        ChkMin( rOther.GetMin() );
        ChkMax( rOther.GetMax() );
        USHORT nStart = 0;
        for( long nIdx = 0; nIdx < nCnt; ++nIdx )
        {
            const SwStripe& rAdd = rOther[ nIdx ];
            long nBottom = rAdd.GetY() + rAdd.GetHeight();
            USHORT nCount = Count();
            USHORT nY = nStart;
            while( nY < nCount )
            {
                SwStripe& rChk = GetObject( nY );
                if( rChk.GetY() + rChk.GetHeight() > rAdd.GetY() )
                    break;
                else
                    ++nY;
            }
            USHORT nB = nY;
            while( nB < nCount )
            {
                const SwStripe& rChk = GetObject( nB );
                if( rChk.GetY() >= nBottom )
                    break;
                else
                    ++nB;
            }
            nStart = nY;
            if( nY == nB )
                Insert( rAdd, nY );
            else
            {
                long nChkBottom = rAdd.GetY() + rAdd.GetHeight();;
                const SwStripe& rChkB = GetObject( nB - 1 );
                long nTmp = rChkB.GetY() + rChkB.GetHeight();
                if( nTmp > nChkBottom )
                    nChkBottom = nTmp;
                SwStripe& rChk = GetObject( nY );
                if( rAdd.GetY() < rChk.GetY() )
                    rChk.Y() = rAdd.GetY();
                rChk.Height() = nChkBottom - rChk.GetY();
                nChkBottom = nB - nY - 1;
                if( nChkBottom )
                    Remove( nY + 1, nChkBottom );
            }
        }
    }
    return *this;
}

BOOL SwStripes::Recalc()
{
    if( !Count() )
        return TRUE;
    Y() = GetObject(0).GetY();
    long nTmpMax = GetObject(0).GetY() + GetObject(0).Height();
    for( long nIdx = 1; nIdx < Count(); )
    {
        const SwStripe& rStr = GetObject(nIdx++);
        if( GetY() > rStr.GetY() )
            Y() = rStr.GetY();
        if( nTmpMax < rStr.GetY() + rStr.GetHeight() )
            nTmpMax = rStr.GetY() + rStr.GetHeight();
    }
    Height() = nTmpMax - GetY();
    return FALSE;
}

BOOL SwScrollArea::Compress()
{
    if( !Count() )
        return TRUE;
    for( long nIdx = Count() - 1; nIdx > 0; --nIdx )
    {
        *GetObject(0) += *GetObject(nIdx);
        delete GetObject( nIdx );
        Remove( nIdx, 1 );
    }
    ClrOffs();
    return GetObject(0)->Recalc();
}

void SwScrollArea::Add( SwScrollArea *pScroll )
{
    ASSERT( pScroll->Count() == 1, "Missing scrollarea compression 2" );
    ASSERT( Count() == 1, "Missing scrollarea compression 3" );
    *GetObject(0) += *pScroll->GetObject(0);
    GetObject(0)->Recalc();
    delete pScroll->GetObject( 0 );
    pScroll->Remove( (USHORT)0, 1 );
    delete pScroll;
}

/******************************************************************************
|*
|*  SwScrollAreas::Insert(..)
|*
******************************************************************************/

void SwScrollAreas::InsertCol( const SwScrollColumn &rCol,
                            SwStripes *pStripes )
{
    SwScrollArea *pTmp = new SwScrollArea( rCol, pStripes );
    USHORT nIdx;
    if( Seek_Entry( pTmp, &nIdx ) )
        GetObject( nIdx )->SmartInsert( pStripes );
    else
        Insert( pTmp );
}


void SwScrollArea::SmartInsert( SwStripes* pStripes )
{
    ASSERT( pStripes, "Insert empty scrollstripe?" );
    BOOL bNotInserted = TRUE;
    for( long nIdx = 0; nIdx < Count() && bNotInserted; )
    {
        SwStripes* pTmp = GetObject( nIdx++ );
        if( pTmp->GetY() + pTmp->GetHeight() == pStripes->GetY() )
        {
            pTmp->Height() += pStripes->GetHeight();
            pTmp->ChkMin( pStripes->GetMin() );
            pTmp->ChkMax( pStripes->GetMax() );
            if( pStripes->Count() )
                pTmp->Insert( (SwStripeArr*)pStripes, pTmp->Count(), 0 );
            bNotInserted = FALSE;
        }
        else if( pTmp->GetY() == pStripes->GetY() + pStripes->GetHeight() )
        {
            pTmp->Height() += pStripes->GetHeight();
            pTmp->Y() = pStripes->GetY();
            pTmp->ChkMin( pStripes->GetMin() );
            pTmp->ChkMax( pStripes->GetMax() );
            if( pStripes->Count() )
                pTmp->Insert( (SwStripeArr*)pStripes, 0, 0 );
            bNotInserted = FALSE;
        }
    }
    if( !bNotInserted || !Insert( pStripes ) )
        delete pStripes;
}

/************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.10  2000/09/18 16:04:37  willem.vandorp
      OpenOffice header added.

      Revision 1.9  2000/09/05 08:07:52  ama
      Fix: Wrong insert position

      Revision 1.8  2000/09/04 13:58:17  ama
      Fix: some paint problems with negative indents

      Revision 1.7  2000/09/01 15:02:04  ama
      Opt.: Scrolling, paints and negative indents

      Revision 1.6  2000/08/17 11:26:05  ama
      Fix #77608#: Scrolling bugs

      Revision 1.5  2000/07/21 11:47:53  ama
      Opt: some comment may be helpful

      Revision 1.4  2000/07/20 15:09:13  ama
      Fix #77064#: Wrong paintarea

      Revision 1.3  2000/07/18 15:17:13  ama
      Fix: InsertCol instead insert

      Revision 1.2  2000/07/18 14:37:51  ama
      Fix: Scrolling of scrolled critical areas

      Revision 1.1  2000/07/17 10:32:09  ama
      Opt: Smarter scrolling for RVP


*************************************************************************/

