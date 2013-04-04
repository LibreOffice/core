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

#include "hintids.hxx"
#include <vcl/help.hxx>
#include <svx/ruler.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/lrspitem.hxx>
#include <sfx2/bindings.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <viewopt.hxx>
#include <frmatr.hxx>
#include <docsh.hxx>
#include <cmdid.h>
#include <edtwin.hxx>
#include <scroll.hxx>
#include <wview.hxx>
#include <usrpref.hxx>
#include <pagedesc.hxx>
#include <workctrl.hxx>
#include <crsskip.hxx>

#include <PostItMgr.hxx>

#include <IDocumentSettingAccess.hxx>

#include <basegfx/tools/zoomtools.hxx>

//Das SetVisArea der DocShell darf nicht vom InnerResizePixel gerufen werden.
//Unsere Einstellungen muessen aber stattfinden.
#ifndef WB_RIGHT_ALIGNED
#define WB_RIGHT_ALIGNED    ((WinBits)0x00008000)
#endif

static bool bProtectDocShellVisArea = false;

static sal_uInt16 nPgNum = 0;

bool SwView::IsDocumentBorder()
{
    return GetDocShell()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
           m_pWrtShell->GetViewOptions()->getBrowseMode() ||
           SVX_ZOOM_PAGEWIDTH_NOBORDER == (SvxZoomType)m_pWrtShell->GetViewOptions()->GetZoomType();
}

inline long GetLeftMargin( SwView &rView )
{
    SvxZoomType eType = (SvxZoomType)rView.GetWrtShell().GetViewOptions()->GetZoomType();
    long lRet = rView.GetWrtShell().GetAnyCurRect(RECT_PAGE_PRT).Left();
    return eType == SVX_ZOOM_PERCENT   ? lRet + DOCUMENTBORDER :
           eType == SVX_ZOOM_PAGEWIDTH || eType == SVX_ZOOM_PAGEWIDTH_NOBORDER ? 0 :
                                         lRet + DOCUMENTBORDER + nLeftOfst;
}

static void lcl_GetPos(SwView* pView,
                Point& rPos,
                SwScrollbar* pScrollbar,
                bool bBorder)
{
    SwWrtShell &rSh = pView->GetWrtShell();
    const Size m_aDocSz( rSh.GetDocSize() );

    const long lBorder = bBorder ? DOCUMENTBORDER : DOCUMENTBORDER * 2;
    const bool bHori = pScrollbar->IsHoriScroll();

    const long lPos = pScrollbar->GetThumbPos() + (bBorder ? DOCUMENTBORDER : 0);

    long lDelta = lPos - (bHori ? rSh.VisArea().Pos().X() : rSh.VisArea().Pos().Y());

    const long lSize = (bHori ? m_aDocSz.A() : m_aDocSz.B()) + lBorder;
    // sollte rechts oder unten zuviel Wiese sein, dann muss
    // diese von der VisArea herausgerechnet werden!
    long nTmp = pView->GetVisArea().Right()+lDelta;
    if ( bHori && nTmp > lSize )
        lDelta -= nTmp - lSize;
    nTmp = pView->GetVisArea().Bottom()+lDelta;
    if ( !bHori && nTmp > lSize )
        lDelta -= nTmp - lSize;

    // use a reference to access/moodify the correct coordinate
    // returned by accessors to non-const object
    long & rCoord = bHori ? rPos.X() : rPos.Y();
    rCoord += lDelta;
    if ( bBorder && rCoord < DOCUMENTBORDER )
        rCoord = DOCUMENTBORDER;
}

/*--------------------------------------------------------------------
    Beschreibung:   Nullpunkt Lineal setzen
 --------------------------------------------------------------------*/
void SwView::InvalidateRulerPos()
{
    static sal_uInt16 aInval[] =
    {
        SID_ATTR_PARA_LRSPACE, SID_RULER_BORDERS, SID_RULER_PAGE_POS,
        SID_RULER_LR_MIN_MAX, SID_ATTR_LONG_ULSPACE, SID_ATTR_LONG_LRSPACE,
        SID_RULER_BORDER_DISTANCE,
        SID_ATTR_PARA_LRSPACE_VERTICAL, SID_RULER_BORDERS_VERTICAL,
        SID_RULER_TEXT_RIGHT_TO_LEFT,
        SID_RULER_ROWS, SID_RULER_ROWS_VERTICAL, FN_STAT_PAGE,
        0
    };

    GetViewFrame()->GetBindings().Invalidate(aInval);

    OSL_ENSURE(m_pHRuler, "warum ist das Lineal nicht da?");
    m_pHRuler->ForceUpdate();
    m_pVRuler->ForceUpdate();
}

/*--------------------------------------------------------------------
    Beschreibung:   begrenzt das Scrollen soweit, dass jeweils nur einen
                    viertel Bildschirm bis vor das Ende des Dokumentes
                    gescrollt werden kann.
 --------------------------------------------------------------------*/
long SwView::SetHScrollMax( long lMax )
{
    const long lBorder = IsDocumentBorder() ? DOCUMENTBORDER : DOCUMENTBORDER * 2;
    const long lSize = GetDocSz().Width() + lBorder - m_aVisArea.GetWidth();

    // bei negativen Werten ist das Dokument vollstaendig sichtbar;
    // in diesem Fall kein Scrollen
    return Max( Min( lMax, lSize ), 0L );
}

long SwView::SetVScrollMax( long lMax )
{
    const long lBorder = IsDocumentBorder() ? DOCUMENTBORDER : DOCUMENTBORDER * 2;
    long lSize = GetDocSz().Height() + lBorder - m_aVisArea.GetHeight();
    return Max( Min( lMax, lSize), 0L );        // siehe horz.
}

Point SwView::AlignToPixel(const Point &rPt) const
{
    return GetEditWin().PixelToLogic( GetEditWin().LogicToPixel( rPt ) );
}

/*--------------------------------------------------------------------
    Beschreibung:   Dokumentgroesse hat sich geaendert
 --------------------------------------------------------------------*/
void SwView::DocSzChgd(const Size &rSz)
{

extern int bDocSzUpdated;


m_aDocSz = rSz;

    if( !m_pWrtShell || m_aVisArea.IsEmpty() )      // keine Shell -> keine Aenderung
    {
        bDocSzUpdated = sal_False;
        return;
    }

    //Wenn Text geloescht worden ist, kann es sein, dass die VisArea hinter
    //den sichtbaren Bereich verweist
    Rectangle aNewVisArea( m_aVisArea );
    bool bModified = false;
    SwTwips lGreenOffset = IsDocumentBorder() ? DOCUMENTBORDER : DOCUMENTBORDER * 2;
    SwTwips lTmp = m_aDocSz.Width() + lGreenOffset;

    if ( aNewVisArea.Right() >= lTmp  )
    {
        lTmp = aNewVisArea.Right() - lTmp;
        aNewVisArea.Right() -= lTmp;
        aNewVisArea.Left() -= lTmp;
        bModified = true;
    }

    lTmp = m_aDocSz.Height() + lGreenOffset;
    if ( aNewVisArea.Bottom() >= lTmp )
    {
        lTmp = aNewVisArea.Bottom() - lTmp;
        aNewVisArea.Bottom() -= lTmp;
        aNewVisArea.Top() -= lTmp;
        bModified = true;
    }

    if ( bModified )
        SetVisArea( aNewVisArea, sal_False );

    if ( UpdateScrollbars() && !m_bInOuterResizePixel && !m_bInInnerResizePixel &&
            !GetViewFrame()->GetFrame().IsInPlace())
        OuterResizePixel( Point(),
                          GetViewFrame()->GetWindow().GetOutputSizePixel() );
}

/*--------------------------------------------------------------------
    Beschreibung:   Visarea neu setzen
 --------------------------------------------------------------------*/
void SwView::SetVisArea( const Rectangle &rRect, sal_Bool bUpdateScrollbar )
{
    const Size aOldSz( m_aVisArea.GetSize() );

    const Point aTopLeft(     AlignToPixel( rRect.TopLeft() ));
    const Point aBottomRight( AlignToPixel( rRect.BottomRight() ));
    Rectangle aLR( aTopLeft, aBottomRight );

    if( aLR == m_aVisArea )
        return;

    const SwTwips lMin = IsDocumentBorder() ? DOCUMENTBORDER : 0;

    // keine negative Position, keine neg. Groesse
    if( aLR.Top() < lMin )
    {
        aLR.Bottom() += lMin - aLR.Top();
        aLR.Top() = lMin;
    }
    if( aLR.Left() < lMin )
    {
        aLR.Right() += lMin - aLR.Left();
        aLR.Left() = lMin;
    }
    if( aLR.Right() < 0 )
        aLR.Right() = 0;
    if( aLR.Bottom() < 0 )
        aLR.Bottom() = 0;

    if( aLR == m_aVisArea )
        return;

    const Size aSize( aLR.GetSize() );
    if( aSize.Width() < 0 || aSize.Height() < 0 )
        return;

    //Bevor die Daten veraendert werden ggf. ein Update rufen. Dadurch wird
    //sichergestellt, da? anliegende Paints korrekt in Dokumentkoordinaten
    //umgerechnet werden.
    //Vorsichtshalber tun wir das nur wenn an der Shell eine Action laeuft,
    //denn dann wir nicht wirklich gepaintet sondern die Rechtecke werden
    //lediglich (in Dokumentkoordinaten) vorgemerkt.
    if ( m_pWrtShell && m_pWrtShell->ActionPend() )
        m_pWrtShell->GetWin()->Update();

    m_aVisArea = aLR;

    const sal_Bool bOuterResize = bUpdateScrollbar && UpdateScrollbars();

    if ( m_pWrtShell )
    {
        m_pWrtShell->VisPortChgd( m_aVisArea );
        if ( aOldSz != m_pWrtShell->VisArea().SSize() &&
             ( Abs(aOldSz.Width() - m_pWrtShell->VisArea().Width()) > 2 ||
                Abs(aOldSz.Height() - m_pWrtShell->VisArea().Height()) > 2 ) )
            m_pWrtShell->CheckBrowseView( sal_False );
    }

    if ( !bProtectDocShellVisArea )
    {
        //Wenn die Groesse der VisArea unveraendert ist, reichen wir die
        //Groesse der VisArea vom InternalObject weiter. Damit soll der
        //Transport von Fehlern vermieden werden.
        Rectangle aVis( m_aVisArea );
        if ( aVis.GetSize() == aOldSz )
            aVis.SetSize( GetDocShell()->SfxObjectShell::GetVisArea(ASPECT_CONTENT).GetSize() );
                    // TODO/LATER: why casting?!
                    //GetDocShell()->SfxInPlaceObject::GetVisArea().GetSize() );

        //Bei embedded immer mit Modify...
        // TODO/LATER: why casting?!
        GetDocShell()->SfxObjectShell::SetVisArea( aVis );
        /*
        if ( GetDocShell()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
            GetDocShell()->SfxInPlaceObject::SetVisArea( aVis );
        else
            GetDocShell()->SvEmbeddedObject::SetVisArea( aVis );*/
    }

    SfxViewShell::VisAreaChanged( m_aVisArea );

    InvalidateRulerPos();

    if ( bOuterResize && !m_bInOuterResizePixel && !m_bInInnerResizePixel)
            OuterResizePixel( Point(),
                GetViewFrame()->GetWindow().GetOutputSizePixel() );
}

/*--------------------------------------------------------------------
    Beschreibung:   Pos VisArea setzen
 --------------------------------------------------------------------*/
void SwView::SetVisArea( const Point &rPt, sal_Bool bUpdateScrollbar )
{
    //einmal alignen, damit Brushes korrekt angesetzt werden.
    //MA 31. May. 96: Das geht in der BrowseView schief, weil evlt.
    //nicht das ganze Dokument sichtbar wird. Da der Inhalt in Frames
    //passgenau ist, kann nicht aligned werden (bessere Idee?!?!)
    //MA 29. Oct. 96 (fix: Bild.de, 200%) ganz ohne Alignment geht es nicht
    //mal sehen wie weit wir mit der halben BrushSize kommen.
    Point aPt( rPt );
//  const long nTmp = GetWrtShell().IsFrameView() ? BRUSH_SIZE/2 : BRUSH_SIZE;
    const long nTmp = GetWrtShell().IsFrameView() ? 4 : 8;
    aPt = GetEditWin().LogicToPixel( aPt );
    aPt.X() -= aPt.X() % nTmp;
    aPt.Y() -= aPt.Y() % nTmp;
    aPt = GetEditWin().PixelToLogic( aPt );

    if ( aPt == m_aVisArea.TopLeft() )
        return;

    const long lXDiff = m_aVisArea.Left() - aPt.X();
    const long lYDiff = m_aVisArea.Top()  - aPt.Y();
    SetVisArea( Rectangle( aPt,
            Point( m_aVisArea.Right() - lXDiff, m_aVisArea.Bottom() - lYDiff ) ),
            bUpdateScrollbar);
}

void SwView::CheckVisArea()
{
    m_pHScrollbar->SetAuto( m_pWrtShell->GetViewOptions()->getBrowseMode() &&
                              !GetViewFrame()->GetFrame().IsInPlace() );
    if ( IsDocumentBorder() )
    {
        if ( m_aVisArea.Left() != DOCUMENTBORDER ||
             m_aVisArea.Top()  != DOCUMENTBORDER )
        {
            Rectangle aNewVisArea( m_aVisArea );
            aNewVisArea.Move( DOCUMENTBORDER - m_aVisArea.Left(),
                              DOCUMENTBORDER - m_aVisArea.Top() );
            SetVisArea( aNewVisArea, sal_True );
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Sichtbaren Bereich berechnen

    OUT Point *pPt:             neue Position des sichtbaren
                                Bereiches
    IN  Rectangle &rRect:       Rechteck, das sich innerhalb des neuen
                                sichtbaren Bereiches befinden soll
        sal_uInt16 nRange           optional exakte Angabe des Bereiches,
                                um den ggfs. gescrollt werden soll
 --------------------------------------------------------------------*/
void SwView::CalcPt( Point *pPt, const Rectangle &rRect,
                     sal_uInt16 nRangeX, sal_uInt16 nRangeY)
{

    const SwTwips lMin = IsDocumentBorder() ? DOCUMENTBORDER : 0;

    long nYScroll = GetYScroll();
    long nDesHeight = rRect.GetHeight();
    long nCurHeight = m_aVisArea.GetHeight();
    nYScroll = Min(nYScroll, nCurHeight - nDesHeight); // wird es knapp, dann nicht zuviel scrollen
    if(nDesHeight > nCurHeight) // die Hoehe reicht nicht aus, dann interessiert nYScroll nicht mehr
    {
        pPt->Y() = rRect.Top();
        pPt->Y() = Max( lMin, pPt->Y() );
    }
    else if ( rRect.Top() < m_aVisArea.Top() )                //Verschiebung nach oben
    {
        pPt->Y() = rRect.Top() - (nRangeY != USHRT_MAX ? nRangeY : nYScroll);
        pPt->Y() = Max( lMin, pPt->Y() );
    }
    else if( rRect.Bottom() > m_aVisArea.Bottom() )   //Verschiebung nach unten
    {
        pPt->Y() = rRect.Bottom() -
                    (m_aVisArea.GetHeight()) + ( nRangeY != USHRT_MAX ?
            nRangeY : nYScroll );
        pPt->Y() = SetVScrollMax( pPt->Y() );
    }
    long nXScroll = GetXScroll();
    if ( rRect.Right() > m_aVisArea.Right() )         //Verschiebung nach rechts
    {
        pPt->X() = rRect.Right()  -
                    (m_aVisArea.GetWidth()) +
                    (nRangeX != USHRT_MAX ? nRangeX : nXScroll);
        pPt->X() = SetHScrollMax( pPt->X() );
    }
    else if ( rRect.Left() < m_aVisArea.Left() )      //Verschiebung nach links
    {
        pPt->X() = rRect.Left() - (nRangeX != USHRT_MAX ? nRangeX : nXScroll);
        pPt->X() = Max( ::GetLeftMargin( *this ) + nLeftOfst, pPt->X() );
        pPt->X() = Min( rRect.Left() - nScrollX, pPt->X() );
        pPt->X() = Max( 0L, pPt->X() );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Scrolling
 --------------------------------------------------------------------*/
sal_Bool SwView::IsScroll( const Rectangle &rRect ) const
{
    return m_bCenterCrsr || m_bTopCrsr || !m_aVisArea.IsInside(rRect);
}

void SwView::Scroll( const Rectangle &rRect, sal_uInt16 nRangeX, sal_uInt16 nRangeY )
{
    if ( m_aVisArea.IsEmpty() )
        return;

    Rectangle aOldVisArea( m_aVisArea );
    long nDiffY = 0;

    Window* pCareWn = ViewShell::GetCareWin(GetWrtShell());
    if ( pCareWn )
    {
        Rectangle aDlgRect( GetEditWin().PixelToLogic(
                pCareWn->GetWindowExtentsRelative( &GetEditWin() ) ) );
        // Nur, wenn der Dialog nicht rechts oder links der VisArea liegt:
        if ( aDlgRect.Left() < m_aVisArea.Right() &&
             aDlgRect.Right() > m_aVisArea.Left() )
        {
            // Falls wir nicht zentriert werden sollen, in der VisArea liegen
            // und nicht vom Dialog ueberdeckt werden ...
            if ( !m_bCenterCrsr && aOldVisArea.IsInside( rRect )
                 && ( rRect.Left() > aDlgRect.Right()
                      || rRect.Right() < aDlgRect.Left()
                      || rRect.Top() > aDlgRect.Bottom()
                      || rRect.Bottom() < aDlgRect.Top() ) )
                return;

            // Ist oberhalb oder unterhalb der Dialogs mehr Platz?
            long nTopDiff = aDlgRect.Top() - m_aVisArea.Top();
            long nBottomDiff = m_aVisArea.Bottom() - aDlgRect.Bottom();
            if ( nTopDiff < nBottomDiff )
            {
                if ( nBottomDiff > 0 ) // Ist unterhalb ueberhaupt Platz?
                {   // dann verschieben wir die Oberkante und merken uns dies
                    nDiffY = aDlgRect.Bottom() - m_aVisArea.Top();
                    m_aVisArea.Top() += nDiffY;
                }
            }
            else
            {
                if ( nTopDiff > 0 ) // Ist oberhalb ueberhaupt Platz?
                    m_aVisArea.Bottom() = aDlgRect.Top(); // Unterkante aendern
            }
        }
    }

    //s.o. !IsScroll()
    if( !(m_bCenterCrsr || m_bTopCrsr) && m_aVisArea.IsInside( rRect ) )
    {
        m_aVisArea = aOldVisArea;
        return;
    }
    //falls das Rechteck groesser als der sichtbare Bereich -->
    //obere linke Ecke
    Size aSize( rRect.GetSize() );
    const Size aVisSize( m_aVisArea.GetSize() );
    if( !m_aVisArea.IsEmpty() && (
        aSize.Width() + GetXScroll() > aVisSize.Width() ||
        aSize.Height()+ GetYScroll() > aVisSize.Height() ))
    {
        Point aPt( m_aVisArea.TopLeft() );
        aSize.Width() = Min( aSize.Width(), aVisSize.Width() );
        aSize.Height()= Min( aSize.Height(),aVisSize.Height());

        CalcPt( &aPt, Rectangle( rRect.TopLeft(), aSize ),
                static_cast< sal_uInt16 >((aVisSize.Width() - aSize.Width()) / 2),
                static_cast< sal_uInt16 >((aVisSize.Height()- aSize.Height())/ 2) );

        if( m_bTopCrsr )
        {
            const long nBorder = IsDocumentBorder() ? DOCUMENTBORDER : 0;
            aPt.Y() = Min( Max( nBorder, rRect.Top() ),
                                m_aDocSz.Height() + nBorder -
                                    m_aVisArea.GetHeight() );
        }
        aPt.Y() -= nDiffY;
        m_aVisArea = aOldVisArea;
        SetVisArea( aPt );
        return;
    }
    if( !m_bCenterCrsr )
    {
        Point aPt( m_aVisArea.TopLeft() );
        CalcPt( &aPt, rRect, nRangeX, nRangeY );

        if( m_bTopCrsr )
        {
            const long nBorder = IsDocumentBorder() ? DOCUMENTBORDER : 0;
            aPt.Y() = Min( Max( nBorder, rRect.Top() ),
                                m_aDocSz.Height() + nBorder -
                                    m_aVisArea.GetHeight() );
        }

        aPt.Y() -= nDiffY;
        m_aVisArea = aOldVisArea;
        SetVisArea( aPt );
        return;
    }

    //Cursor zentrieren
    Point aPnt( m_aVisArea.TopLeft() );
    // ... in Y-Richtung auf jeden Fall
    aPnt.Y() += ( rRect.Top() + rRect.Bottom()
                  - m_aVisArea.Top() - m_aVisArea.Bottom() ) / 2 - nDiffY;
    // ... in X-Richtung nur, wenn das Rechteck rechts oder links aus der
    //     VisArea hinausragt.
    if ( rRect.Right() > m_aVisArea.Right() || rRect.Left() < m_aVisArea.Left() )
    {
        aPnt.X() += ( rRect.Left() + rRect.Right()
                  - m_aVisArea.Left() - m_aVisArea.Right() ) / 2;
        aPnt.X() = SetHScrollMax( aPnt.X() );
        const SwTwips lMin = IsDocumentBorder() ? DOCUMENTBORDER : 0;
        aPnt.X() = Max( (GetLeftMargin( *this ) - lMin) + nLeftOfst, aPnt.X() );
    }
    m_aVisArea = aOldVisArea;
    if( pCareWn )
    {   // Wenn wir nur einem Dialog ausweichen wollen, wollen wir nicht ueber
        // das Ende des Dokument hinausgehen.
        aPnt.Y() = SetVScrollMax( aPnt.Y() );
    }
    SetVisArea( aPnt );
}

/*--------------------------------------------------------------------
    Beschreibung:   Seitenweises Scrollen
    Liefern den Wert, um den bei PageUp / -Down gescrollt werden soll
 --------------------------------------------------------------------*/
sal_Bool SwView::GetPageScrollUpOffset( SwTwips &rOff ) const
{
    if ( !m_aVisArea.Top() || !m_aVisArea.GetHeight() )
        return sal_False;
    long nYScrl = GetYScroll() / 2;
    rOff = -(m_aVisArea.GetHeight() - nYScrl);
    //nicht vor den Dokumentanfang scrollen
    if( m_aVisArea.Top() - rOff < 0 )
        rOff = rOff - m_aVisArea.Top();
    else if( GetWrtShell().GetCharRect().Top() < (m_aVisArea.Top() + nYScrl))
        rOff += nYScrl;
    return sal_True;
}

sal_Bool SwView::GetPageScrollDownOffset( SwTwips &rOff ) const
{
    if ( !m_aVisArea.GetHeight() ||
         (m_aVisArea.GetHeight() > m_aDocSz.Height()) )
        return sal_False;
    long nYScrl = GetYScroll() / 2;
    rOff = m_aVisArea.GetHeight() - nYScrl;
    //nicht hinter das Dokumentende scrollen
    if ( m_aVisArea.Top() + rOff > m_aDocSz.Height() )
        rOff = m_aDocSz.Height() - m_aVisArea.Bottom();
    else if( GetWrtShell().GetCharRect().Bottom() >
                                            ( m_aVisArea.Bottom() - nYScrl ))
        rOff -= nYScrl;
    return rOff > 0;
}

// Seitenweises Blaettern
long SwView::PageUp()
{
    if (!m_aVisArea.GetHeight())
        return 0;

    Point aPos(m_aVisArea.TopLeft());
    aPos.Y() -= m_aVisArea.GetHeight() - (GetYScroll() / 2);
    aPos.Y() = Max(0L, aPos.Y());
    SetVisArea( aPos );
    return 1;
}

long SwView::PageDown()
{
    if ( !m_aVisArea.GetHeight() )
        return 0;
    Point aPos( m_aVisArea.TopLeft() );
    aPos.Y() += m_aVisArea.GetHeight() - (GetYScroll() / 2);
    aPos.Y() = SetVScrollMax( aPos.Y() );
    SetVisArea( aPos );
    return 1;
}

long SwView::PhyPageUp()
{
    //aktuell sichtbare Seite erfragen, nicht formatieren
    sal_uInt16 nActPage = m_pWrtShell->GetNextPrevPageNum( sal_False );

    if( USHRT_MAX != nActPage )
    {
        const Point aPt( m_aVisArea.Left(),
                         m_pWrtShell->GetPagePos( nActPage ).Y() );
        Point aAlPt( AlignToPixel( aPt ) );
        // falls ein Unterschied besteht, wurde abgeschnitten --> dann
        // einen Pixel addieren, damit kein Rest der Vorgaengerseite
        // sichtbar ist
        if( aPt.Y() != aAlPt.Y() )
            aAlPt.Y() += 3 * GetEditWin().PixelToLogic( Size( 0, 1 ) ).Height();
        SetVisArea( aAlPt );
    }
    return 1;
}

long SwView::PhyPageDown()
{
    //aktuell sichtbare Seite erfragen, nicht formatieren
    sal_uInt16 nActPage = m_pWrtShell->GetNextPrevPageNum( sal_True );
    // falls die letzte Dokumentseite sichtbar ist, nichts tun
    if( USHRT_MAX != nActPage )
    {
        const Point aPt( m_aVisArea.Left(),
                         m_pWrtShell->GetPagePos( nActPage ).Y() );
        Point aAlPt( AlignToPixel( aPt ) );
        // falls ein Unterschied besteht, wurde abgeschnitten --> dann
        // einen Pixel addieren, damit kein Rest der Vorgaengerseite sichtbar ist
        if( aPt.Y() != aAlPt.Y() )
            aAlPt.Y() += 3 * GetEditWin().PixelToLogic( Size( 0, 1 ) ).Height();
        SetVisArea( aAlPt );
    }
    return 1;
}

long SwView::PageUpCrsr( sal_Bool bSelect )
{
    if ( !bSelect )
    {
        const sal_uInt16 eType = m_pWrtShell->GetFrmType(0,sal_True);
        if ( eType & FRMTYPE_FOOTNOTE )
        {
            m_pWrtShell->MoveCrsr();
            m_pWrtShell->GotoFtnAnchor();
            m_pWrtShell->Right(CRSR_SKIP_CHARS, sal_False, 1, sal_False );
            return 1;
        }
    }

    SwTwips lOff = 0;
    if ( GetPageScrollUpOffset( lOff ) &&
         (m_pWrtShell->IsCrsrReadonly() ||
          !m_pWrtShell->PageCrsr( lOff, bSelect )) &&
         PageUp() )
    {
        m_pWrtShell->ResetCursorStack();
        return sal_True;
    }
    return sal_False;
}

long SwView::PageDownCrsr(sal_Bool bSelect)
{
    SwTwips lOff = 0;
    if ( GetPageScrollDownOffset( lOff ) &&
         (m_pWrtShell->IsCrsrReadonly() ||
          !m_pWrtShell->PageCrsr( lOff, bSelect )) &&
         PageDown() )
    {
        m_pWrtShell->ResetCursorStack();
        return sal_True;
    }
    return sal_False;
}

/*------------------------------------------------------------------------
 Beschreibung:  Handler der Scrollbars
------------------------------------------------------------------------*/
IMPL_LINK( SwView, ScrollHdl, SwScrollbar *, pScrollbar )
{
    if ( GetWrtShell().ActionPend() )
        return 0;

    if ( pScrollbar->GetType() == SCROLL_DRAG )
        m_pWrtShell->EnableSmooth( sal_False );

    if(!m_pWrtShell->GetViewOptions()->getBrowseMode() &&
        pScrollbar->GetType() == SCROLL_DRAG)
    {
        //Hier wieder auskommentieren wenn das mitscrollen nicht gewuenscht ist.
        // the end scrollhandler invalidate the FN_STAT_PAGE,
        // so we dont must do it agin.
        EndScrollHdl(pScrollbar);

        Point aPos( m_aVisArea.TopLeft() );
        lcl_GetPos(this, aPos, pScrollbar, IsDocumentBorder());

        sal_uInt16 nPhNum = 1;
        sal_uInt16 nVirtNum = 1;

        String sDisplay;
        if(m_pWrtShell->GetPageNumber( aPos.Y(), sal_False, nPhNum, nVirtNum, sDisplay ))
        {

            //QuickHelp:
            if( !m_bWheelScrollInProgress && m_pWrtShell->GetPageCnt() > 1 && Help::IsQuickHelpEnabled() )
            {
                if( !nPgNum || nPgNum != nPhNum )
                {
                    Rectangle aRect;
                    aRect.Left() = pScrollbar->GetParent()->OutputToScreenPixel(
                                        pScrollbar->GetPosPixel() ).X() -8;
                    aRect.Top() = pScrollbar->OutputToScreenPixel(
                                    pScrollbar->GetPointerPosPixel() ).Y();
                    aRect.Right()   = aRect.Left();
                    aRect.Bottom()  = aRect.Top();

                    String sPageStr( GetPageStr( nPhNum, nVirtNum, sDisplay ));
                    SwContentAtPos aCnt( SwContentAtPos::SW_OUTLINE );
                    m_pWrtShell->GetContentAtPos( aPos, aCnt );
                    if( aCnt.sStr.Len() )
                    {
                        sPageStr += rtl::OUString("  - ");
                        sPageStr.Insert( aCnt.sStr, 0, 80 );
                        sPageStr.SearchAndReplaceAll( '\t', ' ' );
                        sPageStr.SearchAndReplaceAll( 0x0a, ' ' );
                    }

                    Help::ShowQuickHelp( pScrollbar, aRect, sPageStr,
                                    QUICKHELP_RIGHT|QUICKHELP_VCENTER);
                }
                nPgNum = nPhNum;
            }
        }
    }
    else
        EndScrollHdl(pScrollbar);

    if ( pScrollbar->GetType() == SCROLL_DRAG )
        m_pWrtShell->EnableSmooth( sal_True );

    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:  Handler der Scrollbars
------------------------------------------------------------------------*/
IMPL_LINK( SwView, EndScrollHdl, SwScrollbar *, pScrollbar )
{
    if ( !GetWrtShell().ActionPend() )
    {
        if(nPgNum)
        {
            nPgNum = 0;
            Help::ShowQuickHelp(pScrollbar, Rectangle(), aEmptyStr, 0);
        }
        Point aPos( m_aVisArea.TopLeft() );
        bool bBorder = IsDocumentBorder();
        lcl_GetPos(this, aPos, pScrollbar, bBorder);
        if ( bBorder && aPos == m_aVisArea.TopLeft() )
            UpdateScrollbars();
        else
            SetVisArea( aPos, sal_False );

        GetViewFrame()->GetBindings().Update(FN_STAT_PAGE);
    }
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
        berechnet die Groesse von m_aVisArea abhaengig von der Groesse
        des EditWin auf dem Schirm.
 --------------------------------------------------------------------*/
void SwView::CalcVisArea( const Size &rOutPixel )
{
    Point aTopLeft;
    Rectangle aRect( aTopLeft, rOutPixel );
    aTopLeft = GetEditWin().PixelToLogic( aTopLeft );
    Point aBottomRight( GetEditWin().PixelToLogic( aRect.BottomRight() ) );

    aRect.Left() = aTopLeft.X();
    aRect.Top() = aTopLeft.Y();
    aRect.Right() = aBottomRight.X();
    aRect.Bottom() = aBottomRight.Y();

    //Die Verschiebungen nach rechts und/oder unten koennen jetzt falsch
    //sein (z.B. Zoom aendern, Viewgroesse aendern.
    const long lBorder = IsDocumentBorder() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
    if ( aRect.Left() )
    {
        const long lWidth = GetWrtShell().GetDocSize().Width() + lBorder;
        if ( aRect.Right() > lWidth )
        {
            long lDelta    = aRect.Right() - lWidth;
            aRect.Left()  -= lDelta;
            aRect.Right() -= lDelta;
        }
    }
    if ( aRect.Top() )
    {
        const long lHeight = GetWrtShell().GetDocSize().Height() + lBorder;
        if ( aRect.Bottom() > lHeight )
        {
            long lDelta     = aRect.Bottom() - lHeight;
            aRect.Top()    -= lDelta;
            aRect.Bottom() -= lDelta;
        }
    }
    SetVisArea( aRect );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER ); // for snapping points
}

/*--------------------------------------------------------------------
    Beschreibung:   Bedienelemente neu anordnen
 --------------------------------------------------------------------*/
void SwView::CalcAndSetBorderPixel( SvBorder &rToFill, sal_Bool /*bInner*/ )
{
    sal_Bool bRightVRuler = m_pWrtShell->GetViewOptions()->IsVRulerRight();
    if ( m_pVRuler->IsVisible() )
    {
        long nWidth = m_pVRuler->GetSizePixel().Width();
        if(bRightVRuler)
            rToFill.Right() = nWidth;
        else
            rToFill.Left() = nWidth;
    }

    OSL_ENSURE(m_pHRuler, "warum ist das Lineal nicht da?");
    if ( m_pHRuler->IsVisible() )
        rToFill.Top() = m_pHRuler->GetSizePixel().Height();

    const StyleSettings &rSet = GetEditWin().GetSettings().GetStyleSettings();
    const long nTmp = rSet.GetScrollBarSize();
    if( m_pVScrollbar->IsVisible(sal_True) )
    {
        if(bRightVRuler)
            rToFill.Left() = nTmp;
        else
            rToFill.Right()  = nTmp;
    }
    if ( m_pHScrollbar->IsVisible(sal_True) )
        rToFill.Bottom() = nTmp;

    SetBorderPixel( rToFill );
}

void ViewResizePixel( const Window &rRef,
                    const Point &rOfst,
                    const Size &rSize,
                    const Size &rEditSz,
                    const sal_Bool /*bInner*/,
                    SwScrollbar& rVScrollbar,
                    SwScrollbar& rHScrollbar,
                    ImageButton* pPageUpBtn,
                    ImageButton* pPageDownBtn,
                    ImageButton* pNaviBtn,
                    Window& rScrollBarBox,
                    SvxRuler* pVRuler,
                    SvxRuler* pHRuler,
                    sal_Bool bWebView,
                    sal_Bool bVRulerRight )
{
// ViewResizePixel wird auch von der PreView benutzt!!!

    const sal_Bool bHRuler = pHRuler && pHRuler->IsVisible();
    const long nHLinSzHeight = bHRuler ?
                        pHRuler->GetSizePixel().Height() : 0;
    const sal_Bool bVRuler = pVRuler && pVRuler->IsVisible();
    const long nVLinSzWidth = bVRuler ?
                        pVRuler->GetSizePixel().Width() : 0;

    long nScrollBarSize = rRef.GetSettings().GetStyleSettings().GetScrollBarSize();
    long nHBSzHeight = rHScrollbar.IsVisible(true) ? nScrollBarSize : 0;
    long nVBSzWidth = rVScrollbar.IsVisible(true) ? nScrollBarSize : 0;

    if(pVRuler)
    {
        WinBits nStyle = pVRuler->GetStyle()&~WB_RIGHT_ALIGNED;
        Point aPos( rOfst.X(), rOfst.Y()+nHLinSzHeight );
        if(bVRulerRight)
        {
            aPos.X() += rSize.Width() - nVLinSzWidth;
            nStyle |= WB_RIGHT_ALIGNED;
        }
        Size  aSize( nVLinSzWidth, rEditSz.Height() );
        if(!aSize.Width())
            aSize.Width() = pVRuler->GetSizePixel().Width();
        pVRuler->SetStyle(nStyle);
        pVRuler->SetPosSizePixel( aPos, aSize );
        if(!pVRuler->IsVisible())
            pVRuler->Resize();
    }
//  Lineal braucht ein Resize, sonst funktioniert es nicht im unischtbaren Zustand
    if(pHRuler)
    {
        Size aSize( rSize.Width(), nHLinSzHeight );
        if ( nVBSzWidth && !bVRulerRight)
            aSize.Width() -= nVBSzWidth;
        if(!aSize.Height())
            aSize.Height() = pHRuler->GetSizePixel().Height();
        pHRuler->SetPosSizePixel( rOfst, aSize );
//      VCL ruft an unsichtbaren Fenstern kein Resize
//      fuer das Lineal ist das aber keine gute Idee
        if(!pHRuler->IsVisible())
            pHRuler->Resize();
    }

    // Scrollbars und SizeBox anordnen
    Point aScrollFillPos;
    {
        Point aPos( rOfst.X(),
                    rOfst.Y()+rSize.Height()-nHBSzHeight );
        if(bVRulerRight)
        {
            aPos.X() += nVBSzWidth;
        }

        Size  aSize( rSize.Width(), nHBSzHeight );
        if ( nVBSzWidth )
            aSize.Width() -= nVBSzWidth;
        rHScrollbar.SetPosSizePixel( aPos, aSize );
        aScrollFillPos.Y() = aPos.Y();
    }
    {
        Point aPos( rOfst.X()+rSize.Width()-nVBSzWidth,
                    rOfst.Y() );
        Size  aSize( nVBSzWidth, rSize.Height() );
        if(bVRulerRight)
        {
            aPos.X() = rOfst.X();
            if(bHRuler)
            {
                aPos.Y() += nHLinSzHeight;
                aSize.Height() -= nHLinSzHeight;
            }
        }

        Size  aImgSz( nVBSzWidth, nVBSzWidth );

        // wenn der Platz fuer Scrollbar und Page-Buttons zu klein wird, dann
        // werden die Buttons versteckt
        sal_uInt16 nCnt = pNaviBtn ? 3 : 2;
        long nSubSize = (aImgSz.Width() * nCnt );
        //
        sal_Bool bHidePageButtons = aSize.Height() < ((bWebView ? 3 : 2) * nSubSize);
        if(!bHidePageButtons)
            aSize.Height() -= nSubSize;
        else
            aImgSz.Width() = 0; // kein Hide, weil das im Update Scrollbar missverstanden wird

        if ( nHBSzHeight )
            aSize.Height() -= nHBSzHeight;
        rVScrollbar.SetPosSizePixel( aPos, aSize );

        aPos.Y() += aSize.Height();
        pPageUpBtn->SetPosSizePixel( aPos, aImgSz );
        if(pNaviBtn)
        {
            aPos.Y() += aImgSz.Height();
            pNaviBtn->SetPosSizePixel(aPos, aImgSz);
        }

        aPos.Y() += aImgSz.Height();
        pPageDownBtn->SetPosSizePixel( aPos, aImgSz );

        aScrollFillPos.X() = aPos.X();
    }

    rScrollBarBox.SetPosSizePixel( aScrollFillPos, Size( nHBSzHeight, nVBSzWidth) );
}

void SwView::ShowAtResize()
{
    m_bShowAtResize = sal_False;
    if ( m_pWrtShell->GetViewOptions()->IsViewHRuler() )
        m_pHRuler->Show();
}

void SwView::InnerResizePixel( const Point &rOfst, const Size &rSize )
{
    Size aObjSize = GetObjectShell()->GetVisArea().GetSize();
    if ( aObjSize.Width() > 0 && aObjSize.Height() > 0 )
    {
        SvBorder aBorder( GetBorderPixel() );
        Size aSize( rSize );
        aSize.Width() -= (aBorder.Left() + aBorder.Right());
        aSize.Height() -= (aBorder.Top() + aBorder.Bottom());
        Size aObjSizePixel = GetWindow()->LogicToPixel( aObjSize, MAP_TWIP );
        SfxViewShell::SetZoomFactor( Fraction( aSize.Width(), aObjSizePixel.Width() ),
                        Fraction( aSize.Height(), aObjSizePixel.Height() ) );
    }

    m_bInInnerResizePixel = sal_True;
    const sal_Bool bHScrollVisible = m_pHScrollbar->IsVisible(sal_True);
    const sal_Bool bVScrollVisible = m_pVScrollbar->IsVisible(sal_True);
    sal_Bool bRepeat = sal_False;
    do
    {
        Size aSz( rSize );
        SvBorder aBorder;
        CalcAndSetBorderPixel( aBorder, sal_True );
        if ( GetViewFrame()->GetFrame().IsInPlace() )
        {
            Size aViewSize( aSz );
            Point aViewPos( rOfst );
            aViewSize.Height() -= (aBorder.Top() + aBorder.Bottom());
            aViewSize.Width()  -= (aBorder.Left() + aBorder.Right());
            aViewPos.X() += aBorder.Left();
            aViewPos.Y() += aBorder.Top();
            GetEditWin().SetPosSizePixel( aViewPos, aViewSize );
        }
        else
        {
            aSz.Height() += aBorder.Top()  + aBorder.Bottom();
            aSz.Width()  += aBorder.Left() + aBorder.Right();
        }

        Size aEditSz( GetEditWin().GetOutputSizePixel() );
        ViewResizePixel( GetEditWin(), rOfst, aSz, aEditSz, sal_True, *m_pVScrollbar,
                            *m_pHScrollbar, m_pPageUpBtn, m_pPageDownBtn,
                            m_pNaviBtn,
                            *m_pScrollFill, m_pVRuler, m_pHRuler,
                            0 != PTR_CAST(SwWebView, this),
                            m_pWrtShell->GetViewOptions()->IsVRulerRight());
        if ( m_bShowAtResize )
            ShowAtResize();

        if( m_pHRuler->IsVisible() || m_pVRuler->IsVisible() )
        {
            const Fraction& rFrac = GetEditWin().GetMapMode().GetScaleX();
            sal_uInt16 nZoom = 100;
            if (0 != rFrac.GetDenominator())
                nZoom = sal_uInt16(rFrac.GetNumerator() * 100L / rFrac.GetDenominator());

            const Fraction aFrac( nZoom, 100 );
            m_pVRuler->SetZoom( aFrac );
            m_pHRuler->SetZoom( aFrac );
            InvalidateRulerPos();   //Inhalt invalidieren.
        }
        //CursorStack zuruecksetzen, da die Cursorpositionen fuer PageUp/-Down
        //nicht mehr zum aktuell sichtbaren Bereich passen
        m_pWrtShell->ResetCursorStack();

        //EditWin niemals einstellen!

        //VisArea einstellen, aber dort nicht das SetVisArea der DocShell rufen!
        bProtectDocShellVisArea = true;
        CalcVisArea( aEditSz );
        //visibility changes of the automatic horizontal scrollbar
        //require to repeat the ViewResizePixel() call - but only once!
        if(bRepeat)
            bRepeat = sal_False;
        else if(bHScrollVisible != m_pHScrollbar->IsVisible(sal_True) ||
                bVScrollVisible != m_pVScrollbar->IsVisible(sal_True))
            bRepeat = sal_True;
    }while( bRepeat );
    bProtectDocShellVisArea = false;
    m_bInInnerResizePixel = sal_False;
}

void SwView::OuterResizePixel( const Point &rOfst, const Size &rSize )
{
    // #i16909# return, if no size (caused by minimize window).
    if ( m_bInOuterResizePixel || ( !rSize.Width() && !rSize.Height() ) )
        return;
    m_bInOuterResizePixel = sal_True;

// feststellen, ob Scrollbars angezeigt werden duerfen
    sal_Bool bShowH = sal_True,
         bShowV = sal_True,
         bAuto  = sal_True,
         bHAuto = sal_True;

    const SwViewOption *pVOpt = m_pWrtShell->GetViewOptions();
    if ( !pVOpt->IsReadonly() || pVOpt->IsStarOneSetting() )
    {
        bShowH = pVOpt->IsViewHScrollBar();
        bShowV = pVOpt->IsViewVScrollBar();
    }

    if (!m_bHScrollbarEnabled)
    {
        bHAuto = bShowH = false;
    }
    if (!m_bVScrollbarEnabled)
    {
        bAuto = bShowV = false;
    }

    SwDocShell* pDocSh = GetDocShell();
    sal_Bool bIsPreview = pDocSh->IsPreview();
    if( bIsPreview )
    {
        bShowH = bShowV = bHAuto = bAuto = sal_False;
    }
    if(m_pHScrollbar->IsVisible(sal_False) != bShowH && !bHAuto)
        ShowHScrollbar(bShowH);
    m_pHScrollbar->SetAuto( bHAuto );
    if(m_pVScrollbar->IsVisible(sal_False) != bShowV && !bAuto)
        ShowVScrollbar(bShowV);
    m_pVScrollbar->SetAuto(bAuto);

    SET_CURR_SHELL( m_pWrtShell );
    sal_Bool bRepeat = sal_False;
    long nCnt = 0;

    sal_Bool bUnLockView = !m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( sal_True );
    m_pWrtShell->LockPaint();

    do {
        ++nCnt;
        const sal_Bool bScroll1 = m_pVScrollbar->IsVisible(sal_True);
        const sal_Bool bScroll2 = m_pHScrollbar->IsVisible(sal_True);
        SvBorder aBorder;
        CalcAndSetBorderPixel( aBorder, sal_False );
        const Size aEditSz( GetEditWin().GetOutputSizePixel() );
        ViewResizePixel( GetEditWin(), rOfst, rSize, aEditSz, sal_False, *m_pVScrollbar,
                                *m_pHScrollbar, m_pPageUpBtn, m_pPageDownBtn,
                                m_pNaviBtn,
                                *m_pScrollFill, m_pVRuler, m_pHRuler,
                                0 != PTR_CAST(SwWebView, this),
                                m_pWrtShell->GetViewOptions()->IsVRulerRight() );
        if ( m_bShowAtResize )
            ShowAtResize();

        if( m_pHRuler->IsVisible() || m_pVRuler->IsVisible() )
            InvalidateRulerPos();   //Inhalt invalidieren.

        //CursorStack zuruecksetzen, da die Cursorpositionen fuer PageUp/-Down
        //nicht mehr zum aktuell sichtbaren Bereich passen
        m_pWrtShell->ResetCursorStack();

        OSL_ENSURE( !GetEditWin().IsVisible() ||
                    (( aEditSz.Width() > 0 && aEditSz.Height() > 0 )
                        || !m_aVisArea.IsEmpty()), "Small world, isn't it?" );

        //EditWin niemals einstellen!

        //Die VisArea muss aber natuerlich eingestellt werden.
        //jetzt ist auch der richtige Zeitpunkt den Zoom neu zu berechnen wenn
        //es kein einfacher Faktor ist.
        m_pWrtShell->StartAction();
        CalcVisArea( aEditSz );

        //Damit auch beim outplace editing die Seitenbreite sofort
        //angepasst wird.
        //TODO/LATER: is that still necessary?!
        /*
        if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
            pDocSh->SetVisArea(
                            pDocSh->SfxInPlaceObject::GetVisArea() );*/
        if ( m_pWrtShell->GetViewOptions()->GetZoomType() != SVX_ZOOM_PERCENT &&
             !m_pWrtShell->GetViewOptions()->getBrowseMode() )
            _SetZoom( aEditSz, (SvxZoomType)m_pWrtShell->GetViewOptions()->GetZoomType(), 100, sal_True );
        m_pWrtShell->EndAction();

        bRepeat = bScroll1 != m_pVScrollbar->IsVisible(sal_True);
        if ( !bRepeat )
            bRepeat = bScroll2 != m_pHScrollbar->IsVisible(sal_True);

        //Nicht endlosschleifen. Moeglichst dann stoppen wenn die
        //(Auto-)Scrollbars sichtbar sind.
        if ( bRepeat &&
             ( nCnt > 10 || ( nCnt > 3 && bHAuto && bAuto ) )
           )
        {
            bRepeat = sal_False;
        }

    }while ( bRepeat );

    if( m_pVScrollbar->IsVisible(sal_False) || m_pVScrollbar->IsAuto())
    {
        sal_Bool bShowButtons = m_pVScrollbar->IsVisible(sal_True);
        if(m_pPageUpBtn && m_pPageUpBtn->IsVisible() != bShowButtons)
        {
            m_pPageUpBtn->Show(bShowButtons);
            if(m_pPageDownBtn)
                m_pPageDownBtn->Show(bShowButtons);
            if(m_pNaviBtn)
                m_pNaviBtn->Show(bShowButtons);
        }
    }

    m_pWrtShell->UnlockPaint();
    if( bUnLockView )
        m_pWrtShell->LockView( sal_False );

    m_bInOuterResizePixel = sal_False;

    if ( m_pPostItMgr )
    {
        m_pPostItMgr->CalcRects();
        m_pPostItMgr->LayoutPostIts();
    }
}

void SwView::SetZoomFactor( const Fraction &rX, const Fraction &rY )
{
    const Fraction &rFrac = rX < rY ? rX : rY;
    SetZoom( SVX_ZOOM_PERCENT, (short) long(rFrac * Fraction( 100, 1 )) );

    //Um Rundungsfehler zu minimieren lassen wir von der Basisklasse ggf.
    //auch die krummen Werte einstellen
    SfxViewShell::SetZoomFactor( rX, rY );
}

sal_Bool SwView::UpdateScrollbars()
{
    sal_Bool bRet = sal_False;
    if ( !m_aVisArea.IsEmpty() )
    {
        const bool bBorder = IsDocumentBorder();
        Rectangle aTmpRect( m_aVisArea );
        if ( bBorder )
        {
            Point aPt( DOCUMENTBORDER, DOCUMENTBORDER );
            aPt = AlignToPixel( aPt );
            aTmpRect.Move( -aPt.X(), -aPt.Y() );
        }

        Size aTmpSz( m_aDocSz );
        const long lOfst = bBorder ? 0 : DOCUMENTBORDER * 2L;
        aTmpSz.Width() += lOfst; aTmpSz.Height() += lOfst;

        {
            const sal_Bool bVScrollVisible = m_pVScrollbar->IsVisible(sal_True);
            m_pVScrollbar->DocSzChgd( aTmpSz );
            m_pVScrollbar->ViewPortChgd( aTmpRect );

            sal_Bool bShowButtons = m_pVScrollbar->IsVisible(sal_True);
            if(m_pPageUpBtn && m_pPageUpBtn->IsVisible() != bShowButtons)
            {
                m_pPageUpBtn->Show(bShowButtons);
                if(m_pPageDownBtn)
                    m_pPageDownBtn->Show(bShowButtons);
                if(m_pNaviBtn)
                    m_pNaviBtn->Show(bShowButtons);
            }

            if ( bVScrollVisible != m_pVScrollbar->IsVisible(sal_True) )
                bRet = sal_True;
        }
        {
            const sal_Bool bHScrollVisible = m_pHScrollbar->IsVisible(sal_True);
            m_pHScrollbar->DocSzChgd( aTmpSz );
            m_pHScrollbar->ViewPortChgd( aTmpRect );
            if ( bHScrollVisible != m_pHScrollbar->IsVisible(sal_True) )
                bRet = sal_True;
            m_pScrollFill->Show(m_pHScrollbar->IsVisible(sal_True) && m_pVScrollbar->IsVisible(sal_True) );
        }
    }
    return bRet;
}

void SwView::Move()
{
    if ( GetWrtShell().IsInSelect() )
        GetWrtShell().EndSelect();
    SfxViewShell::Move();
}

sal_Bool SwView::HandleWheelCommands( const CommandEvent& rCEvt )
{
    sal_Bool bOk = sal_False;
    const CommandWheelData* pWData = rCEvt.GetWheelData();
    if( pWData && COMMAND_WHEEL_ZOOM == pWData->GetMode() )
    {
        long nFact = m_pWrtShell->GetViewOptions()->GetZoom();
        if( 0L > pWData->GetDelta() )
            nFact = Max( (long) 20, basegfx::zoomtools::zoomOut( nFact ));
        else
            nFact = Min( (long) 600, basegfx::zoomtools::zoomIn( nFact ));

        SetZoom( SVX_ZOOM_PERCENT, nFact );
        bOk = sal_True;
    }
    else if( pWData && COMMAND_WHEEL_ZOOM_SCALE == pWData->GetMode() )
    {
        int newZoom = 100 * (m_pWrtShell->GetViewOptions()->GetZoom() / 100.0) * (pWData->GetDelta() / 100.0);
        SetZoom( SVX_ZOOM_PERCENT, Max( 20, Min( 600, newZoom ) ) );
        bOk = sal_True;
    }
    else
    {
        if(pWData->GetMode()==COMMAND_WHEEL_SCROLL)
        {
            // This influences whether quick help is shown
            m_bWheelScrollInProgress=true;
        }

        if (pWData && (COMMAND_WHEEL_SCROLL==pWData->GetMode()) && (((sal_uLong)0xFFFFFFFF) == pWData->GetScrollLines()))
            {
                        if (pWData->GetDelta()<0)
                                PhyPageDown();
                        else
                                PhyPageUp();
                        bOk = sal_True;
                }
        else
            bOk = m_pEditWin->HandleScrollCommand( rCEvt,
                            m_pHScrollbar, m_pVScrollbar);

        // Restore default state for case when scroll command comes from dragging scrollbar handle
        m_bWheelScrollInProgress=false;
    }
    return bOk;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
