/*************************************************************************
 *
 *  $RCSfile: impedit.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-20 11:53:50 $
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

#include <eeng_pch.hxx>

#pragma hdrstop


#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>

#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYENTRY_HPP_
#include <com/sun/star/linguistic2/XDictionaryEntry.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_DICTIONARYTYPE_HPP_
#include <com/sun/star/linguistic2/DictionaryType.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_DICTIONARYEVENT_HPP_
#include <com/sun/star/linguistic2/DictionaryEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XDictionaryEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_DICTIONARYEVENTFLAGS_HPP_
#include <com/sun/star/linguistic2/DictionaryEventFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARY_HPP_
#include <com/sun/star/linguistic2/XDictionary.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARY1_HPP_
#include <com/sun/star/linguistic2/XDictionary1.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;


inline void lcl_AllignToPixel( Point& rPoint, OutputDevice* pOutDev, short nDiffX, short nDiffY )
{
    rPoint = pOutDev->LogicToPixel( rPoint );

    if ( nDiffX )
        rPoint.X() += nDiffX;
    if ( nDiffY )
        rPoint.Y() += nDiffY;

    rPoint = pOutDev->PixelToLogic( rPoint );
}

// ----------------------------------------------------------------------
//  class ImpEditView
//  ----------------------------------------------------------------------
ImpEditView::ImpEditView( EditEngine* pEng, Window* pWindow ) :
    aOutArea( Point(), pEng->GetPaperSize() )
{
    pEditEngine         = pEng;
    pOutWin             = pWindow;
    pPointer            = NULL;
    pBackgroundColor    = NULL;
    nScrollDiffX        = 0;
    pCursor             = NULL;
    bReadOnly           = sal_False;
    eSelectionMode      = EE_SELMODE_TXTONLY;
    eAnchorMode         = ANCHOR_TOP_LEFT;
    nInvMore            = 1;
    nTravelXPos         = TRAVEL_X_DONTKNOW;
    nControl            = EV_CNTRL_AUTOSCROLL | EV_CNTRL_ENABLEPASTE;
    aEditSelection.Min() = pEng->pImpEditEngine->GetEditDoc().GetStartPaM();
    aEditSelection.Max() = pEng->pImpEditEngine->GetEditDoc().GetEndPaM();
}

ImpEditView::~ImpEditView()
{
    if ( pOutWin && ( pOutWin->GetCursor() == pCursor ) )
        pOutWin->SetCursor( NULL );

    delete pCursor;
    delete pBackgroundColor;
    delete pPointer;
}

void ImpEditView::SetBackgroundColor( const Color& rColor )
{
    delete pBackgroundColor;
    pBackgroundColor = new Color( rColor );
}

void ImpEditView::DrawSelection( EditSelection aTmpSel, Region* pRegion )
{
    // Vor dem Zeichnen der Selektion muss sichergestellt werden,
    // das der Fensterinhalt komplett gueltig ist!
    // Muss hier stehen, damit auf jeden Fall weg wenn lerr, nicht spaeter
    // zwei Paint-Events!
    // 19.10: Muss sogar vor Abfrage von bUpdate, falls nach Invalidate
    // noch Paints in der Queue, aber jemand schaltet den UpdateMode um!

    // pRegion: Wenn nicht NULL, dann nur Region berechnen.
    PolyPolygon* pPolyPoly = NULL;
    if ( pRegion )
        pPolyPoly = new PolyPolygon;

    sal_Bool bInvertSelection = sal_False;
    sal_Bool bPixelMode = pOutWin->GetMapMode() == MAP_PIXEL;
    sal_Bool bClipRegion = pOutWin->IsClipRegion();
    Region aOldRegion = pOutWin->GetClipRegion();
    // aTmpOutArea: Falls OutputArea > Papierbreite und
    // Text > Papierbreite ( uebergrosse Felder )
    Rectangle aTmpOutArea( aOutArea );
    if ( aTmpOutArea.GetWidth() > pEditEngine->pImpEditEngine->GetPaperSize().Width() )
        aTmpOutArea.Right() = aTmpOutArea.Left() + pEditEngine->pImpEditEngine->GetPaperSize().Width();

    if ( !pRegion )
    {
        if ( pEditEngine->pImpEditEngine->GetUpdateMode() == sal_False )
            return;
        if ( pEditEngine->pImpEditEngine->IsInUndo() )
            return;

        if ( !aTmpSel.HasRange() )
            return;

        pOutWin->IntersectClipRegion( aTmpOutArea );

#ifdef MAC
        bInvertSelection = ( GetBackground().GetColor() != COL_WHITE );
#endif

        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Hide();
    }

    DBG_ASSERT( !pEditEngine->pImpEditEngine->aIdleFormatter.IsActive(), "DrawSelection: Not formatted!" );
    aTmpSel.Adjust( pEditEngine->pImpEditEngine->GetEditDoc() );

    // Alte Selektion EE_SELMODE_STD siehe Rev 1.101 (schneller!)
    ContentNode* pStartNode = aTmpSel.Min().GetNode();
    ContentNode* pEndNode = aTmpSel.Max().GetNode();
    sal_uInt16 nStartPara = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( pStartNode );
    sal_uInt16 nEndPara = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( pEndNode );
    // ueber die Absaetze iterieren....
    for ( sal_uInt16 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        ParaPortion* pTmpPortion = pEditEngine->pImpEditEngine->GetParaPortions().SaveGetObject( nPara );
        DBG_ASSERT( pTmpPortion, "Portion in Selektion nicht gefunden!" );
        DBG_ASSERT( !pTmpPortion->IsInvalid(), "Portion in Selektion nicht formatiert!" );

        if ( !pTmpPortion->IsVisible() || pTmpPortion->IsInvalid() )
            continue;

        long nParaStart = pEditEngine->pImpEditEngine->GetParaPortions().GetYOffset( pTmpPortion );
        if ( ( nParaStart + pTmpPortion->GetHeight() ) < GetVisDocTop() )
            continue;
        if ( nParaStart > GetVisDocBottom() )
            break;

        sal_uInt16 nStartLine = 0;
        sal_uInt16 nEndLine = pTmpPortion->GetLines().Count() -1;
        if ( nPara == nStartPara )
            nStartLine = pTmpPortion->GetLines().FindLine( aTmpSel.Min().GetIndex(), sal_False );
        if ( nPara == nEndPara )
            nEndLine = pTmpPortion->GetLines().FindLine( aTmpSel.Max().GetIndex(), sal_True );

        // ueber die Zeilen iterieren....
        for ( sal_uInt16 nLine = nStartLine; nLine <= nEndLine; nLine++ )
        {
            EditLine* pLine = pTmpPortion->GetLines().GetObject( nLine );
            DBG_ASSERT( pLine, "Zeile nicht gefunden: DrawSelection()" );

            sal_uInt16 nStartIndex = pLine->GetStart();
            sal_uInt16 nEndIndex = pLine->GetEnd();
            if ( ( nPara == nStartPara ) && ( nLine == nStartLine ) )
                nStartIndex = aTmpSel.Min().GetIndex();
            if ( ( nPara == nEndPara ) && ( nLine == nEndLine ) )
                nEndIndex = aTmpSel.Max().GetIndex();

            // Kann passieren, wenn am Anfang einer umgebrochenen Zeile.
            if ( nEndIndex < nStartIndex )
                nEndIndex = nStartIndex;

            Rectangle aTmpRec( pEditEngine->pImpEditEngine->GetEditCursor( pTmpPortion, nStartIndex ) );
            aTmpRec.Top() += nParaStart;
            aTmpRec.Bottom() += nParaStart;

            Point aTopLeft( aTmpRec.TopLeft() );
            Point aBottomRight( aTmpRec.BottomRight() );
            aBottomRight.X() = pTmpPortion->GetXPos( pLine, nEndIndex );

            // Nur Painten, wenn im sichtbaren Bereich...
            if ( aTopLeft.Y() > GetVisDocBottom() )
                break;

            if ( ( aTopLeft.X() != aBottomRight.X() ) && ( aBottomRight.Y() >= GetVisDocTop() ) )
            {
                Point aPnt1( GetWindowPos( aTopLeft ) );
                Point aPnt2( GetWindowPos( aBottomRight ) );

                if ( eSelectionMode == EE_SELMODE_STD )
                {
                    // Bekannter Bug:
                    // Wenn in diesem Modus am Anfang einer nicht-ersten-Zeile
                    // in einem Absatz nach unten selektiert wird,
                    // findet FindLine die Zeile davor, lediglich der Cursor
                    // weiter unten angezeigt wird
                    // => rechter Rand der Selektion wird nicht richtig eingestellt!
                    if ( !( ( nPara == nStartPara ) && ( nLine == nStartLine ) ) )
                        aPnt1.X() = aTmpOutArea.Left();

                    if ( !( ( nPara == nEndPara ) && ( nLine == nEndLine ) ) )
                        aPnt2.X() = aTmpOutArea.Right()-1;
                }

                lcl_AllignToPixel( aPnt1, pOutWin, +1, 0 );
                lcl_AllignToPixel( aPnt2, pOutWin, 0, ( bPixelMode ? 0 : -1 ) );

                // Damit man selektierte leere Zeile erkennt, aber leider Paintprobleme
//              if ( pLine->IsEmpty() && ( aPnt2.X() <= aPnt1.X() ) )
//                  aPnt2.X() = aPnt1.X() + 3*pEditEngine->pImpEditEngine->nOnePixelInRef;

                Rectangle aRect( aPnt1, aPnt2 );
                if ( pRegion )
                {
                    Polygon aTmpPoly( 4 );
                    aTmpPoly[0] = aRect.TopLeft();
                    aTmpPoly[1] = aRect.TopRight();
                    aTmpPoly[2] = aRect.BottomRight();
                    aTmpPoly[3] = aRect.BottomLeft();
                    pPolyPoly->Insert( aTmpPoly );
                }
                else
                {
                    if ( bInvertSelection )
                        pOutWin->InvertRect( aRect );
                    else
                        pOutWin->HighlightRect( aRect );
                }
            }
        }
    }

    if ( pRegion )
    {
        *pRegion = Region( *pPolyPoly );
        delete pPolyPoly;
    }
    else
    {
        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Show();

        if ( bClipRegion )
            pOutWin->SetClipRegion( aOldRegion );
        else
            pOutWin->SetClipRegion();
    }

}

BOOL ImpEditView::IsVertical() const
{
    return pEditEngine->pImpEditEngine->IsVertical();
}

Point ImpEditView::GetDocPos( const Point& rWindowPos ) const
{
    // Fensterposition => Dokumentposition
    Point aPoint;

    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aPoint.X() = rWindowPos.X() - aOutArea.Left() + GetVisDocLeft();
        aPoint.Y() = rWindowPos.Y() - aOutArea.Top() + GetVisDocTop();
    }
    else
    {
        aPoint.X() = rWindowPos.Y() - aOutArea.Top() + GetVisDocLeft();
        aPoint.Y() = aOutArea.Right() - rWindowPos.X() + GetVisDocTop();
    }

    return aPoint;
}

Point ImpEditView::GetWindowPos( const Point& rDocPos ) const
{
    // Dokumentposition => Fensterposition
    Point aPoint;

    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aPoint.X() = rDocPos.X() + aOutArea.Left() - GetVisDocLeft();
        aPoint.Y() = rDocPos.Y() + aOutArea.Top() - GetVisDocTop();
    }
    else
    {
        aPoint.X() = aOutArea.Right() - rDocPos.Y() + GetVisDocTop();
        aPoint.Y() = rDocPos.X() + aOutArea.Top() - GetVisDocLeft();
    }

    return aPoint;
}

Rectangle ImpEditView::GetWindowPos( const Rectangle& rDocRect ) const
{
    // Dokumentposition => Fensterposition
    Point aPos( GetWindowPos( rDocRect.TopLeft() ) );
    Size aSz = rDocRect.GetSize();
    Rectangle aRect;
    if ( !pEditEngine->pImpEditEngine->IsVertical() )
    {
        aRect = Rectangle( aPos, aSz );
    }
    else
    {
        Point aNewPos( aPos.X()-aSz.Height(), aPos.Y() );
        aRect = Rectangle( aNewPos, Size( aSz.Height(), aSz.Width() ) );
    }
    return aRect;
}


Region* ImpEditView::CalcSelectedRegion()
{
    Region* pRegion = new Region;
    DrawSelection( GetEditSelection(), pRegion );
    return pRegion;
}

void ImpEditView::SetSelectionMode( EESelectionMode eNewMode )
{
    if ( eSelectionMode != eNewMode )
    {
        DrawSelection();    // 'Wegmalen' ...
        eSelectionMode = eNewMode;
        DrawSelection();    // und neu zeichnen.
    }
}

void ImpEditView::SetOutputArea( const Rectangle& rRec )
{
    // sollte besser auf Pixel allignt sein!
    Rectangle aNewRec( pOutWin->LogicToPixel( rRec ) );
    aNewRec = pOutWin->PixelToLogic( aNewRec );
    aOutArea = aNewRec;
    if ( aOutArea.Right() < aOutArea.Left() )
        aOutArea.Right() = aOutArea.Left();
    if ( aOutArea.Bottom() < aOutArea.Top() )
        aOutArea.Bottom() = aOutArea.Top();

    if ( DoBigScroll() )
        SetScrollDiffX( (sal_uInt16)aOutArea.GetWidth() * 3 / 10 );
    else
        SetScrollDiffX( (sal_uInt16)aOutArea.GetWidth() * 2 / 10 );
}

void ImpEditView::ResetOutputArea( const Rectangle& rRec )
{
    Rectangle aCurArea( aOutArea );
    SetOutputArea( rRec );
    // Umliegende Bereiche invalidieren, wenn UpdateMode der Engine auf sal_True
    if ( !aCurArea.IsEmpty() && pEditEngine->pImpEditEngine->GetUpdateMode() )
    {
        long nMore = 0;
        if ( DoInvalidateMore() )
            nMore = GetWindow()->PixelToLogic( Size( nInvMore, 0 ) ).Width();
        if ( aCurArea.Left() < aOutArea.Left() )
        {
            Rectangle aRect( aCurArea.TopLeft(),
                Size( aOutArea.Left()-aCurArea.Left(), aCurArea.GetHeight() ) );
            if ( nMore )
            {
                aRect.Left() -= nMore;
                aRect.Top() -= nMore;
                aRect.Bottom() += nMore;
            }
            GetWindow()->Invalidate( aRect );
        }
        if ( aCurArea.Right() > aOutArea.Right() )
        {
            long nW = aCurArea.Right() - aOutArea.Right();
            Point aPos( aCurArea.TopRight() );
            aPos.X() -= nW;
            Rectangle aRect( aPos, Size( nW, aCurArea.GetHeight() ) );
            if ( nMore )
            {
                aRect.Right() += nMore;
                aRect.Top() -= nMore;
                aRect.Bottom() += nMore;
            }
            GetWindow()->Invalidate( aRect );
        }
        if ( aCurArea.Top() < aOutArea.Top() )
        {
            Rectangle aRect( aCurArea.TopLeft(), Size( aCurArea.GetWidth(), aOutArea.Top() - aCurArea.Top() ) );
            if ( nMore )
            {
                aRect.Top() -= nMore;
                aRect.Left() -= nMore;
                aRect.Right() += nMore;
            }
            GetWindow()->Invalidate( aRect );
        }
        if ( aCurArea.Bottom() > aOutArea.Bottom() )
        {
            long nH = aCurArea.Bottom() - aOutArea.Bottom();
            Point aPos( aCurArea.BottomLeft() );
            aPos.Y() -= nH;
            Rectangle aRect( aPos, Size( aCurArea.GetWidth(), nH ) );
            if ( nMore )
            {
                aRect.Bottom() += nMore;
                aRect.Left() -= nMore;
                aRect.Right() += nMore;
            }

            GetWindow()->Invalidate( aRect );
        }
    }
}

void ImpEditView::RecalcOutputArea()
{
    Rectangle aOldArea( aOutArea );
    Point aNewTopLeft( aOutArea.TopLeft() );
    Size aNewSz( aOutArea.GetSize() );

    // X:
    if ( DoAutoWidth() )
    {
        if ( pEditEngine->pImpEditEngine->GetStatus().AutoPageWidth() )
            aNewSz.Width() = pEditEngine->pImpEditEngine->GetPaperSize().Width();
        switch ( eAnchorMode )
        {
            case ANCHOR_TOP_LEFT:
            case ANCHOR_VCENTER_LEFT:
            case ANCHOR_BOTTOM_LEFT:
            {
                aNewTopLeft.X() = aAnchorPoint.X();
            }
            break;
            case ANCHOR_TOP_HCENTER:
            case ANCHOR_VCENTER_HCENTER:
            case ANCHOR_BOTTOM_HCENTER:
            {
                aNewTopLeft.X() = aAnchorPoint.X() - aNewSz.Width() / 2;
            }
            break;
            case ANCHOR_TOP_RIGHT:
            case ANCHOR_VCENTER_RIGHT:
            case ANCHOR_BOTTOM_RIGHT:
            {
                aNewTopLeft.X() = aAnchorPoint.X() - aNewSz.Width() - 1;
            }
            break;
        }
    }

    // Y:
    if ( DoAutoHeight() )
    {
        if ( pEditEngine->pImpEditEngine->GetStatus().AutoPageHeight() )
            aNewSz.Height() = pEditEngine->pImpEditEngine->GetPaperSize().Height();
        switch ( eAnchorMode )
        {
            case ANCHOR_TOP_LEFT:
            case ANCHOR_TOP_HCENTER:
            case ANCHOR_TOP_RIGHT:
            {
                aNewTopLeft.Y() = aAnchorPoint.Y();
            }
            break;
            case ANCHOR_VCENTER_LEFT:
            case ANCHOR_VCENTER_HCENTER:
            case ANCHOR_VCENTER_RIGHT:
            {
                aNewTopLeft.Y() = aAnchorPoint.Y() - aNewSz.Height() / 2;
            }
            break;
            case ANCHOR_BOTTOM_LEFT:
            case ANCHOR_BOTTOM_HCENTER:
            case ANCHOR_BOTTOM_RIGHT:
            {
                aNewTopLeft.Y() = aAnchorPoint.Y() - aNewSz.Height() - 1;
            }
            break;
        }
    }
    ResetOutputArea( Rectangle( aNewTopLeft, aNewSz ) );
}

void ImpEditView::SetAnchorMode( EVAnchorMode eMode )
{
    eAnchorMode = eMode;
    CalcAnchorPoint();
}

void ImpEditView::CalcAnchorPoint()
{
    // GetHeight() und GetWidth() -1, da Rectangle-Berechnung nicht erwuenscht.

    // X:
    switch ( eAnchorMode )
    {
        case ANCHOR_TOP_LEFT:
        case ANCHOR_VCENTER_LEFT:
        case ANCHOR_BOTTOM_LEFT:
        {
            aAnchorPoint.X() = aOutArea.Left();
        }
        break;
        case ANCHOR_TOP_HCENTER:
        case ANCHOR_VCENTER_HCENTER:
        case ANCHOR_BOTTOM_HCENTER:
        {
            aAnchorPoint.X() = aOutArea.Left() + (aOutArea.GetWidth()-1) / 2;
        }
        break;
        case ANCHOR_TOP_RIGHT:
        case ANCHOR_VCENTER_RIGHT:
        case ANCHOR_BOTTOM_RIGHT:
        {
            aAnchorPoint.X() = aOutArea.Right();
        }
        break;
    }

    // Y:
    switch ( eAnchorMode )
    {
        case ANCHOR_TOP_LEFT:
        case ANCHOR_TOP_HCENTER:
        case ANCHOR_TOP_RIGHT:
        {
            aAnchorPoint.Y() = aOutArea.Top();
        }
        break;
        case ANCHOR_VCENTER_LEFT:
        case ANCHOR_VCENTER_HCENTER:
        case ANCHOR_VCENTER_RIGHT:
        {
            aAnchorPoint.Y() = aOutArea.Top() + (aOutArea.GetHeight()-1) / 2;
        }
        break;
        case ANCHOR_BOTTOM_LEFT:
        case ANCHOR_BOTTOM_HCENTER:
        case ANCHOR_BOTTOM_RIGHT:
        {
            aAnchorPoint.Y() = aOutArea.Bottom() - 1;
        }
        break;
    }
}

void ImpEditView::ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, sal_Bool bSpecial )
{
    // Kein ShowCursor bei einer leeren View...
    if ( ( aOutArea.Left() >= aOutArea.Right() ) && ( aOutArea.Top() >= aOutArea.Bottom() ) )
        return;

    pEditEngine->pImpEditEngine->CheckIdleFormatter();
    if ( !pEditEngine->pImpEditEngine->IsFormatted() )
        pEditEngine->pImpEditEngine->FormatDoc();

    // Aus irgendwelchen Gruenden lande ich waehrend der Formatierung hier,
    // wenn sich der Outiner im Paint initialisiert, weil kein SetPool();
    if ( pEditEngine->pImpEditEngine->IsFormatting() )
        return;
    if ( pEditEngine->pImpEditEngine->GetUpdateMode() == sal_False )
        return;
    if ( pEditEngine->pImpEditEngine->IsInUndo() )
        return;

    if ( pOutWin->GetCursor() != GetCursor() )
        pOutWin->SetCursor( GetCursor() );

    sal_uInt16 nFlags = GETCRSR_TXTONLY;
    if ( bSpecial )
        nFlags |= GETCRSR_ENDOFLINE;

    EditPaM aPaM( aEditSelection.Max() );
    Rectangle aEditCursor = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, nFlags );
    if ( !IsInsertMode() && !aEditSelection.HasRange() )
    {
        // Bei Overwrite die Breite korrigieren
        // Umstellen auf TextArray !!!
        // MT 12.12.98: Einfach mal aEditCursor.Right() = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM+1, sal_False ), aber nur wenn nicht Zeilenende
        if ( aPaM.GetNode()->Len() && ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) )
        {
            SvxFont aFont;
            pEditEngine->pImpEditEngine->SeekCursor( aPaM.GetNode(), aPaM.GetIndex()+1, aFont );
            Font aOldFont( pOutWin->GetFont() );
            aFont.SetPhysFont( pOutWin );
            aEditCursor.Right() += aFont.GetPhysTxtSize( pOutWin, *aPaM.GetNode(), aPaM.GetIndex(), 1 ).Width();
            if ( pEditEngine->pImpEditEngine->GetStatus().DoRestoreFont() )
                pOutWin->SetFont( aOldFont );
        }
    }
    long nMaxHeight = !IsVertical() ? aOutArea.GetHeight() : aOutArea.GetWidth();
    if ( aEditCursor.GetHeight() > nMaxHeight )
    {
        aEditCursor.Bottom() = aEditCursor.Top() + nMaxHeight - 1;
    }
    if ( bGotoCursor  ) // && (!pEditEngine->pImpEditEngine->GetStatus().AutoPageSize() ) )
    {
        // pruefen, ob scrollen notwendig...
        // wenn scrollen, dann Update() und Scroll() !
        long nDocDiffX = 0;
        long nDocDiffY = 0;

        Rectangle aTmpVisArea( GetVisDocArea() );
        // aTmpOutArea: Falls OutputArea > Papierbreite und
        // Text > Papierbreite ( uebergrosse Felder )
        long nMaxTextWidth = !IsVertical() ? pEditEngine->pImpEditEngine->GetPaperSize().Width() : pEditEngine->pImpEditEngine->GetPaperSize().Height();
        if ( aTmpVisArea.GetWidth() > nMaxTextWidth )
            aTmpVisArea.Right() = aTmpVisArea.Left() + nMaxTextWidth;

        if ( aEditCursor.Bottom() > aTmpVisArea.Bottom() )
        {   // hochscrollen, hier positiv
            nDocDiffY = aEditCursor.Bottom() - aTmpVisArea.Bottom();
        }
        else if ( aEditCursor.Top() < aTmpVisArea.Top() )
        {   // runterscrollen, negativ
            nDocDiffY = aEditCursor.Top() - aTmpVisArea.Top();
        }

        if ( aEditCursor.Right() > aTmpVisArea.Right() )
        {
            // linksscrollen, positiv
            nDocDiffX = aEditCursor.Right() - aTmpVisArea.Right();
            // Darfs ein bischen mehr sein?
            if ( aEditCursor.Right() < ( nMaxTextWidth - GetScrollDiffX() ) )
                nDocDiffX += GetScrollDiffX();
            else
            {
                long n = nMaxTextWidth - aEditCursor.Right();
                // Bei einem MapMode != RefMapMode kann der EditCursor auch mal ueber
                // die Papierbreite Wandern!
                nDocDiffX += ( n > 0 ? n : -n );
            }
        }
        else if ( aEditCursor.Left() < aTmpVisArea.Left() )
        {   // rechtsscrollen
            // negativ:
            nDocDiffX = aEditCursor.Left() - aTmpVisArea.Left();
            // Darfs ein bischen mehr sein?
            if ( aEditCursor.Left() > ( - (long)GetScrollDiffX() ) )
                nDocDiffX -= GetScrollDiffX();
            else
                nDocDiffX -= aEditCursor.Left();
        }
        if ( aPaM.GetIndex() == 0 )     // braucht Olli fuer den Outliner
        {
            // Aber sicherstellen, dass dadurch der Cursor nicht den
            // sichtbaren bereich verlaesst!
            if ( aEditCursor.Left() < aTmpVisArea.GetWidth() )
            {
                nDocDiffX = -aTmpVisArea.Left();
            }
        }

        if ( nDocDiffX | nDocDiffY )
        {
            long nDiffX = !IsVertical() ? nDocDiffX : -nDocDiffY;
            long nDiffY = !IsVertical() ? nDocDiffY : nDocDiffX;

            // Negativ: Zum Anfang bzw. linken Rand
            if ( ( Abs( nDiffY ) > pEditEngine->pImpEditEngine->nOnePixelInRef ) && DoBigScroll() )
            {
                long nH = aOutArea.GetHeight() / 4;
                if ( ( nH > aEditCursor.GetHeight() ) && ( Abs( nDiffY ) < nH ) )
                {
                    if ( nDiffY < 0 )
                        nDiffY -= nH;
                    else
                        nDiffY += nH;
                }
            }

            if ( ( Abs( nDiffX ) > pEditEngine->pImpEditEngine->nOnePixelInRef ) && DoBigScroll() )
            {
                long nW = aOutArea.GetWidth() / 4;
                if ( Abs( nDiffX ) < nW )
                {
                    if ( nDiffY < 0 )
                        nDiffY -= nW;
                    else
                        nDiffY += nW;
                }
            }

            if ( nDiffX )
                pEditEngine->pImpEditEngine->aStatus.GetStatusWord() = pEditEngine->pImpEditEngine->aStatus.GetStatusWord() | EE_STAT_HSCROLL;
            if ( nDiffY )
                pEditEngine->pImpEditEngine->aStatus.GetStatusWord() = pEditEngine->pImpEditEngine->aStatus.GetStatusWord() | EE_STAT_VSCROLL;
            Scroll( -nDiffX, -nDiffY );
            pEditEngine->pImpEditEngine->DelayedCallStatusHdl();
        }
    }

    // Cursor evtl. etwas stutzen...
    if ( ( aEditCursor.Bottom() > GetVisDocTop() ) &&
         ( aEditCursor.Top() < GetVisDocBottom() ) )
    {
        if ( aEditCursor.Bottom() > GetVisDocBottom() )
            aEditCursor.Bottom() = GetVisDocBottom();
        if ( aEditCursor.Top() < GetVisDocTop() )
            aEditCursor.Top() = GetVisDocTop();
    }

    long nOnePixel = pOutWin->PixelToLogic( Size( 1, 0 ) ).Width();

    if ( /* pEditEngine->pImpEditEngine->GetStatus().AutoPageSize() || */
         ( ( aEditCursor.Top() + nOnePixel >= GetVisDocTop() ) &&
         ( aEditCursor.Bottom() - nOnePixel <= GetVisDocBottom() ) &&
         ( aEditCursor.Left() + nOnePixel >= GetVisDocLeft() ) &&
         ( aEditCursor.Right() - nOnePixel <= GetVisDocRight() ) ) )
    {
        Rectangle aCursorRect = GetWindowPos( aEditCursor );
        GetCursor()->SetPos( aCursorRect.TopLeft() );
        Size aCursorSz( aCursorRect.GetSize() );
        // Rectangle is inclusive
        aCursorSz.Width()--;
        aCursorSz.Height()--;
        if ( !aCursorSz.Width() || !aCursorSz.Height() )
        {
            long nCursorSz = pOutWin->GetSettings().GetStyleSettings().GetCursorSize();
            nCursorSz = pOutWin->PixelToLogic( Size( nCursorSz, 0 ) ).Width();
            if ( !aCursorSz.Width() )
                aCursorSz.Width() = nCursorSz;
            if ( !aCursorSz.Height() )
                aCursorSz.Height() = nCursorSz;
        }
        GetCursor()->SetSize( aCursorSz );
        if ( bForceVisCursor )
            GetCursor()->Show();

        if ( !pEditEngine->pImpEditEngine->mpIMEInfos )
        {
            SvxFont aFont;
            pEditEngine->pImpEditEngine->SeekCursor( aPaM.GetNode(), aPaM.GetIndex()+1, aFont );
            ULONG nContextFlags = INPUTCONTEXT_TEXT|INPUTCONTEXT_EXTTEXTINPUT;
            GetWindow()->SetInputContext( InputContext( aFont, nContextFlags ) );
        }
    }
    else
    {
        pEditEngine->pImpEditEngine->GetStatus().GetStatusWord() = pEditEngine->pImpEditEngine->GetStatus().GetStatusWord() | EE_STAT_CURSOROUT;
        GetCursor()->Hide();
        GetCursor()->SetPos( Point( -1, -1 ) );
        GetCursor()->SetSize( Size( 0, 0 ) );
    }
}

Pair ImpEditView::Scroll( long ndX, long ndY, BYTE nRangeCheck )
{
    DBG_ASSERT( pEditEngine->pImpEditEngine->IsFormatted(), "Scroll: Nicht formatiert!" );
    if ( !ndX && !ndY )
        return Range( 0, 0 );

#ifdef DBG_UTIL
    Rectangle aR( aOutArea );
    aR = pOutWin->LogicToPixel( aR );
    aR = pOutWin->PixelToLogic( aR );
    DBG_ASSERT( aR == aOutArea, "OutArea vor Scroll nicht aligned" );
#endif

    Rectangle aNewVisArea( GetVisDocArea() );
    Size aPaperSz( pEditEngine->pImpEditEngine->GetPaperSize() );

    // Vertical:
    if ( !IsVertical() )
    {
        aNewVisArea.Top() -= ndY;
        aNewVisArea.Bottom() -= ndY;
    }
    else
    {
        aNewVisArea.Top() += ndX;
        aNewVisArea.Bottom() += ndX;
    }
    if ( ( nRangeCheck == RGCHK_PAPERSZ1 ) && ( aNewVisArea.Bottom() > (long)pEditEngine->pImpEditEngine->GetTextHeight() ) )
    {
        // GetTextHeight noch optimieren!
        long nDiff = pEditEngine->pImpEditEngine->GetTextHeight() - aNewVisArea.Bottom(); // negativ
        aNewVisArea.Move( 0, nDiff );   // koennte im neg. Bereich landen...
    }
    if ( ( aNewVisArea.Top() < 0 ) && ( nRangeCheck != RGCHK_NONE ) )
        aNewVisArea.Move( 0, -aNewVisArea.Top() );

    // Horizontal:
    if ( !IsVertical() )
    {
        aNewVisArea.Left() -= ndX;
        aNewVisArea.Right() -= ndX;
    }
    else
    {
        aNewVisArea.Left() -= ndY;
        aNewVisArea.Right() -= ndY;
    }
    if ( ( nRangeCheck == RGCHK_PAPERSZ1 ) && ( aNewVisArea.Right() > pEditEngine->pImpEditEngine->CalcTextWidth() ) )
    {
        long nDiff = pEditEngine->pImpEditEngine->CalcTextWidth() - aNewVisArea.Right();    // negativ
        aNewVisArea.Move( nDiff, 0 );   // koennte im neg. Bereich landen...
    }
    if ( ( aNewVisArea.Left() < 0 ) && ( nRangeCheck != RGCHK_NONE ) )
        aNewVisArea.Move( -aNewVisArea.Left(), 0 );

    // Die Differenz muss auf Pixel alignt sein (wegen Scroll!)
    long nDiffX = !IsVertical() ? ( GetVisDocLeft() - aNewVisArea.Left() ) : -( GetVisDocTop() - aNewVisArea.Top() );
    long nDiffY = !IsVertical() ? ( GetVisDocTop() - aNewVisArea.Top() ) : ( GetVisDocLeft() - aNewVisArea.Left() );

    Size aDiffs( nDiffX, nDiffY );
    aDiffs = pOutWin->LogicToPixel( aDiffs );
    aDiffs = pOutWin->PixelToLogic( aDiffs );

    long nRealDiffX = aDiffs.Width();
    long nRealDiffY = aDiffs.Height();


    if ( nRealDiffX || nRealDiffY )
    {
        Cursor* pCrsr = GetCursor();
        sal_Bool bVisCursor = pCrsr->IsVisible();
        pCrsr->Hide();
        pOutWin->Update();
        if ( !IsVertical() )
            aVisDocStartPos.Move( -nRealDiffX, -nRealDiffY );
        else
            aVisDocStartPos.Move( -nRealDiffY, nRealDiffX );
        // Das Move um den allignten Wert ergibt nicht unbedingt ein
        // alligntes Rechteck...
        // MT 11/00: Align VisArea???
        aVisDocStartPos = pOutWin->LogicToPixel( aVisDocStartPos );
        aVisDocStartPos = pOutWin->PixelToLogic( aVisDocStartPos );
        Rectangle aRec( aOutArea );
        pOutWin->Scroll( nRealDiffX, nRealDiffY, aRec, sal_True );
        pOutWin->Update();
        pCrsr->SetPos( pCrsr->GetPos() + Point( nRealDiffX, nRealDiffY ) );
        if ( bVisCursor )
        {
            Rectangle aCursorRec( pCrsr->GetPos(), pCrsr->GetSize() );
            if ( aOutArea.IsInside( aCursorRec ) )
                pCrsr->Show();
        }
    }

    return Pair( nRealDiffX, nRealDiffY );
}

void ImpEditView::SetInsertMode( sal_Bool bInsert )
{
    if ( bInsert != IsInsertMode() )
    {
        SetFlags( nControl, EV_CNTRL_OVERWRITE, !bInsert );
        ShowCursor( DoAutoScroll(), sal_False );
    }
}

sal_Bool ImpEditView::IsWrongSpelledWord( const EditPaM& rPaM, sal_Bool bMarkIfWrong )
{
    sal_Bool bIsWrong = sal_False;
#ifndef SVX_LIGHT
    if ( rPaM.GetNode()->GetWrongList() )
    {
        EditSelection aSel = pEditEngine->pImpEditEngine->SelectWord( rPaM );
        bIsWrong = rPaM.GetNode()->GetWrongList()->HasWrong( aSel.Min().GetIndex(), aSel.Max().GetIndex() );
        if ( bIsWrong && bMarkIfWrong )
        {
            DrawSelection();    // alte Selektion 'weg-zeichnen'
            SetEditSelection( aSel );
            DrawSelection();
        }
    }
#endif // !SVX_LIGHT
    return bIsWrong;
}

String ImpEditView::SpellIgnoreOrAddWord( sal_Bool bAdd )
{
    String aWord;
#ifndef SVX_LIGHT
    if ( pEditEngine->pImpEditEngine->GetSpeller().is() )
    {
        EditPaM aPaM = GetEditSelection().Max();
        if ( !HasSelection() )
        {
            EditSelection aSel = pEditEngine->pImpEditEngine->SelectWord( aPaM );
            aWord = pEditEngine->pImpEditEngine->GetSelected( aSel );
        }
        else
        {
            aWord = pEditEngine->pImpEditEngine->GetSelected( GetEditSelection() );
            // Und deselektieren
            DrawSelection();    // alte Selektion 'weg-zeichnen'
            SetEditSelection( EditSelection( aPaM, aPaM ) );
            DrawSelection();
        }

        if ( aWord.Len() )
        {
            if ( bAdd )
            {
                DBG_ERROR( "Sorry, AddWord not implemented" );
            }
            else // Ignore
            {
                Reference< XDictionary1 >  xDic( SvxGetIgnoreAllList(), UNO_QUERY );
                if (xDic.is())
                    xDic->add( aWord, sal_False, String() );
            }
            const EditDoc& rDoc = pEditEngine->pImpEditEngine->GetEditDoc();
            sal_uInt16 nNodes = rDoc.Count();
            for ( sal_uInt16 n = 0; n < nNodes; n++ )
            {
                ContentNode* pNode = rDoc.GetObject( n );
                pNode->GetWrongList()->MarkWrongsInvalid();
            }
            pEditEngine->pImpEditEngine->DoOnlineSpelling( aPaM.GetNode() );
            pEditEngine->pImpEditEngine->StartOnlineSpellTimer();
        }
    }
#endif // !SVX_LIGHT
    return aWord;
}

