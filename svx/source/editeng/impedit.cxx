/*************************************************************************
 *
 *  $RCSfile: impedit.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-27 16:50:40 $
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

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGGESTURERECOGNIZER_HPP_
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XFLUSHABLECLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#endif

#include <vos/mutex.hxx>

#include <flditem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/transfer.hxx>

#include <sot/exchange.hxx>
#include <sot/formats.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

#define SCRLRANGE   20      // 1/20 der Breite/Hoehe scrollen, wenn im QueryDrop

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
ImpEditView::ImpEditView( EditView* pView, EditEngine* pEng, Window* pWindow ) :
    aOutArea( Point(), pEng->GetPaperSize() )
{
    pEditView           = pView;
    pEditEngine         = pEng;
    pOutWin             = pWindow;
    pPointer            = NULL;
    pBackgroundColor    = NULL;
    nScrollDiffX        = 0;
    nExtraCursorFlags   = 0;
    nCursorBidiLevel    = CURSOR_BIDILEVEL_DONTKNOW;
    pCursor             = NULL;
       pDragAndDropInfo = NULL;
    bReadOnly           = sal_False;
    bClickedInSelection = sal_False;
    eSelectionMode      = EE_SELMODE_TXTONLY;
    eAnchorMode         = ANCHOR_TOP_LEFT;
    nInvMore            = 1;
    nTravelXPos         = TRAVEL_X_DONTKNOW;
    nControl            = EV_CNTRL_AUTOSCROLL | EV_CNTRL_ENABLEPASTE;
    bActiveDragAndDropListener = FALSE;

    aEditSelection.Min() = pEng->pImpEditEngine->GetEditDoc().GetStartPaM();
    aEditSelection.Max() = pEng->pImpEditEngine->GetEditDoc().GetEndPaM();
}

ImpEditView::~ImpEditView()
{
    RemoveDragAndDropListeners();

    if ( pOutWin && ( pOutWin->GetCursor() == pCursor ) )
        pOutWin->SetCursor( NULL );

    delete pCursor;
    delete pBackgroundColor;
    delete pPointer;
    delete pDragAndDropInfo;
}

void ImpEditView::SetBackgroundColor( const Color& rColor )
{
    delete pBackgroundColor;
    pBackgroundColor = new Color( rColor );
}

void ImpEditView::SetEditSelection( const EditSelection& rEditSelection )
{
    // #100856# set state before notification
    aEditSelection = rEditSelection;

    if ( pEditEngine->pImpEditEngine->GetNotifyHdl().IsSet() )
    {
        EENotify aNotify( EE_NOTIFY_TEXTVIEWSELECTIONCHANGED );
        aNotify.pEditEngine = pEditEngine;
        aNotify.pEditView = GetEditViewPtr();
        pEditEngine->pImpEditEngine->CallNotify( aNotify );
    }
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

    sal_Bool bClipRegion = pOutWin->IsClipRegion();
    Region aOldRegion = pOutWin->GetClipRegion();

    if ( !pRegion )
    {
        if ( pEditEngine->pImpEditEngine->GetUpdateMode() == sal_False )
            return;
        if ( pEditEngine->pImpEditEngine->IsInUndo() )
            return;

        if ( !aTmpSel.HasRange() )
            return;

        // aTmpOutArea: Falls OutputArea > Papierbreite und
        // Text > Papierbreite ( uebergrosse Felder )
        Rectangle aTmpOutArea( aOutArea );
        if ( aTmpOutArea.GetWidth() > pEditEngine->pImpEditEngine->GetPaperSize().Width() )
            aTmpOutArea.Right() = aTmpOutArea.Left() + pEditEngine->pImpEditEngine->GetPaperSize().Width();
        pOutWin->IntersectClipRegion( aTmpOutArea );

        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Hide();
    }

    DBG_ASSERT( !pEditEngine->pImpEditEngine->aIdleFormatter.IsActive(), "DrawSelection: Not formatted!" );
    aTmpSel.Adjust( pEditEngine->pImpEditEngine->GetEditDoc() );

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

            BOOL bPartOfLine = FALSE;
            sal_uInt16 nStartIndex = pLine->GetStart();
            sal_uInt16 nEndIndex = pLine->GetEnd();
            if ( ( nPara == nStartPara ) && ( nLine == nStartLine ) && ( nStartIndex != aTmpSel.Min().GetIndex() ) )
            {
                nStartIndex = aTmpSel.Min().GetIndex();
                bPartOfLine = TRUE;
            }
            if ( ( nPara == nEndPara ) && ( nLine == nEndLine ) && ( nEndIndex != aTmpSel.Max().GetIndex() ) )
            {
                nEndIndex = aTmpSel.Max().GetIndex();
                bPartOfLine = TRUE;
            }

            // Kann passieren, wenn am Anfang einer umgebrochenen Zeile.
            if ( nEndIndex < nStartIndex )
                nEndIndex = nStartIndex;

            Rectangle aTmpRec( pEditEngine->pImpEditEngine->GetEditCursor( pTmpPortion, nStartIndex ) );
            Point aTopLeft( aTmpRec.TopLeft() );
            Point aBottomRight( aTmpRec.BottomRight() );

            aTopLeft.Y() += nParaStart;
            aBottomRight.Y() += nParaStart;

            // Nur Painten, wenn im sichtbaren Bereich...
            if ( aTopLeft.Y() > GetVisDocBottom() )
                break;

            if ( aBottomRight.Y() < GetVisDocTop() )
                continue;

            // Now that we have Bidi, the first/last index doesn't have to be the 'most outside' postion
            if ( !bPartOfLine )
            {
                Range aLineXPosStartEnd = pEditEngine->pImpEditEngine->GetLineXPosStartEnd( pTmpPortion, pLine );
                aTopLeft.X() = aLineXPosStartEnd.Min();
                aBottomRight.X() = aLineXPosStartEnd.Max();
                ImplDrawHighlightRect( pOutWin, aTopLeft, aBottomRight, pPolyPoly );
            }
            else
            {
                USHORT nTmpStartIndex = nStartIndex;
                USHORT nWritingDirStart, nTmpEndIndex;

                while ( nTmpStartIndex < nEndIndex )
                {
                    pEditEngine->pImpEditEngine->GetRightToLeft( nPara, nTmpStartIndex+1, &nWritingDirStart, &nTmpEndIndex );
                    if ( nTmpEndIndex > nEndIndex )
                        nTmpEndIndex = nEndIndex;

                    DBG_ASSERT( nTmpEndIndex > nTmpStartIndex, "DrawSelection, Start >= End?" );

                    long nX1 = pEditEngine->pImpEditEngine->GetXPos( pTmpPortion, pLine, nTmpStartIndex, TRUE );
                    long nX2 = pEditEngine->pImpEditEngine->GetXPos( pTmpPortion, pLine, nTmpEndIndex );

                    Point aPt1( Min( nX1, nX2 ), aTopLeft.Y() );
                    Point aPt2( Max( nX1, nX2 ), aBottomRight.Y() );

                    ImplDrawHighlightRect( pOutWin, aPt1, aPt2, pPolyPoly );

                    nTmpStartIndex = nTmpEndIndex;
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

void ImpEditView::ImplDrawHighlightRect( Window* pOutWin, const Point& rDocPosTopLeft, const Point& rDocPosBottomRight, PolyPolygon* pPolyPoly )
{
    if ( rDocPosTopLeft.X() != rDocPosBottomRight.X() )
    {
        sal_Bool bPixelMode = pOutWin->GetMapMode() == MAP_PIXEL;

        Point aPnt1( GetWindowPos( rDocPosTopLeft ) );
        Point aPnt2( GetWindowPos( rDocPosBottomRight ) );

        if ( !IsVertical() )
        {
            lcl_AllignToPixel( aPnt1, pOutWin, +1, 0 );
            lcl_AllignToPixel( aPnt2, pOutWin, 0, ( bPixelMode ? 0 : -1 ) );
        }
        else
        {
            lcl_AllignToPixel( aPnt1, pOutWin, 0, +1 );
            lcl_AllignToPixel( aPnt2, pOutWin, ( bPixelMode ? 0 : +1 ), 0 );
        }

        Rectangle aRect( aPnt1, aPnt2 );
        if ( pPolyPoly )
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
            pOutWin->HighlightRect( aRect );
        }
    }
}


BOOL ImpEditView::IsVertical() const
{
    return pEditEngine->pImpEditEngine->IsVertical();
}

Rectangle ImpEditView::GetVisDocArea() const
{
    return Rectangle( GetVisDocLeft(), GetVisDocTop(), GetVisDocRight(), GetVisDocBottom() );
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

void ImpEditView::ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor, USHORT nShowCursorFlags )
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

    EditPaM aPaM( aEditSelection.Max() );

    USHORT nTextPortionStart = 0;
    USHORT nPara = pEditEngine->pImpEditEngine->aEditDoc.GetPos( aPaM.GetNode() );
    ParaPortion* pParaPortion = pEditEngine->pImpEditEngine->GetParaPortions().GetObject( nPara );

    nShowCursorFlags |= nExtraCursorFlags;

    nShowCursorFlags |= GETCRSR_TXTONLY;

    // Use CursorBidiLevel 0/1 in meaning of
    // 0: prefer portion end, normal mode
    // 1: prefer portion start

    if ( ( GetCursorBidiLevel() != CURSOR_BIDILEVEL_DONTKNOW ) && GetCursorBidiLevel() )
    {
        nShowCursorFlags |= GETCRSR_PREFERPORTIONSTART;
    }

    Rectangle aEditCursor = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, nShowCursorFlags );
    if ( !IsInsertMode() && !aEditSelection.HasRange() )
    {
        if ( aPaM.GetNode()->Len() && ( aPaM.GetIndex() < aPaM.GetNode()->Len() ) )
        {
            // If we are behind a portion, and the next portion has other direction, we must change position...
            aEditCursor.Left() = aEditCursor.Right() = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM, GETCRSR_TXTONLY|GETCRSR_PREFERPORTIONSTART ).Left();

            USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, TRUE );
            TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
            if ( pTextPortion->GetKind() == PORTIONKIND_TAB )
            {
                aEditCursor.Right() += pTextPortion->GetSize().Width();
            }
            else
            {
                EditPaM aNext = pEditEngine->pImpEditEngine->CursorRight( aPaM, (USHORT)i18n::CharacterIteratorMode::SKIPCELL );
                Rectangle aTmpRect = pEditEngine->pImpEditEngine->PaMtoEditCursor( aNext, GETCRSR_TXTONLY );
                if ( aTmpRect.Top() != aEditCursor.Top() )
                    aTmpRect = pEditEngine->pImpEditEngine->PaMtoEditCursor( aNext, GETCRSR_TXTONLY|GETCRSR_ENDOFLINE );
                aEditCursor.Right() = aTmpRect.Left();
            }
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
        // #111036# Let VCL do orientation for cursor, otherwise problem when cursor has direction flag
        if ( IsVertical() )
        {
            Size aOldSz( aCursorSz );
            aCursorSz.Width() = aOldSz.Height();
            aCursorSz.Height() = aOldSz.Width();
            GetCursor()->SetPos( aCursorRect.TopRight() );
            GetCursor()->SetOrientation( 2700 );
        }
        GetCursor()->SetSize( aCursorSz );

        unsigned char nCursorDir = CURSOR_DIRECTION_NONE;
        if ( IsInsertMode() && !aEditSelection.HasRange() && ( pEditEngine->pImpEditEngine->HasDifferentRTLLevels( aPaM.GetNode() ) ) )
        {
            USHORT nTextPortion = pParaPortion->GetTextPortions().FindPortion( aPaM.GetIndex(), nTextPortionStart, nShowCursorFlags & GETCRSR_PREFERPORTIONSTART ? TRUE : FALSE );
            TextPortion* pTextPortion = pParaPortion->GetTextPortions().GetObject( nTextPortion );
            USHORT nRTLLevel = pTextPortion->GetRightToLeft();
            if ( nRTLLevel%2 )
                nCursorDir = CURSOR_DIRECTION_RTL;
            else
                nCursorDir = CURSOR_DIRECTION_LTR;

        }
        GetCursor()->SetDirection( nCursorDir );

        if ( bForceVisCursor )
            GetCursor()->Show();

        // #102936# Call SetInputContext every time, otherwise we may have the wrong font
        // if ( !pEditEngine->pImpEditEngine->mpIMEInfos )
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
    DBG_ASSERTWARNING( aR == aOutArea, "OutArea vor Scroll nicht aligned" );
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
    if ( ( nRangeCheck == RGCHK_PAPERSZ1 ) && ( aNewVisArea.Right() > (long)pEditEngine->pImpEditEngine->CalcTextWidth( FALSE ) ) )
    {
        long nDiff = pEditEngine->pImpEditEngine->CalcTextWidth( FALSE ) - aNewVisArea.Right();     // negativ
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

        if ( pEditEngine->pImpEditEngine->GetNotifyHdl().IsSet() )
        {
            EENotify aNotify( EE_NOTIFY_TEXTVIEWSCROLLED );
            aNotify.pEditEngine = pEditEngine;
            aNotify.pEditView = GetEditViewPtr();
            pEditEngine->pImpEditEngine->CallNotify( aNotify );
        }
    }

    return Pair( nRealDiffX, nRealDiffY );
}

sal_Bool ImpEditView::PostKeyEvent( const KeyEvent& rKeyEvent )
{
    BOOL bDone = FALSE;

    KeyFuncType eFunc = rKeyEvent.GetKeyCode().GetFunction();
    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !bReadOnly )
                {
                    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                    CutCopy( aClipBoard, sal_True );
                    bDone = sal_True;
                }
            }
            break;
            case KEYFUNC_COPY:
            {
                Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                CutCopy( aClipBoard, sal_False );
                bDone = TRUE;
            }
            break;
            case KEYFUNC_PASTE:
            {
                if ( !bReadOnly && IsPasteEnabled() )
                {
                    pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_PASTE );
                    Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetClipboard());
                    Paste( aClipBoard, pEditEngine->pImpEditEngine->GetStatus().AllowPasteSpecial() );
                    pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_PASTE );
                    bDone = sal_True;
                }
            }
            break;
        }
    }

    if( !bDone )
        bDone = pEditEngine->PostKeyEvent( rKeyEvent, GetEditViewPtr() );

    return bDone;
}

sal_Bool ImpEditView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    if ( pEditEngine->pImpEditEngine->aStatus.NotifyCursorMovements() )
    {
        if ( pEditEngine->pImpEditEngine->aStatus.GetPrevParagraph() != pEditEngine->pImpEditEngine->GetEditDoc().GetPos( GetEditSelection().Max().GetNode() ) )
        {
            pEditEngine->pImpEditEngine->aStatus.GetStatusWord() = pEditEngine->pImpEditEngine->aStatus.GetStatusWord() | EE_STAT_CRSRLEFTPARA;
            pEditEngine->pImpEditEngine->CallStatusHdl();
        }
    }
    nTravelXPos = TRAVEL_X_DONTKNOW;
    nCursorBidiLevel = CURSOR_BIDILEVEL_DONTKNOW;
    nExtraCursorFlags = 0;
    bClickedInSelection = sal_False;

    if ( rMouseEvent.IsMiddle() && !bReadOnly &&
         ( GetWindow()->GetSettings().GetMouseSettings().GetMiddleButtonAction() == MOUSE_MIDDLE_PASTESELECTION ) )
    {
        Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetSelection());
        Paste( aClipBoard );
    }
    else if ( rMouseEvent.IsLeft() && GetEditSelection().HasRange() )
    {
        Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipBoard(GetWindow()->GetSelection());
        CutCopy( aClipBoard, FALSE );
    }

    return pEditEngine->pImpEditEngine->MouseButtonUp( rMouseEvent, GetEditViewPtr() );
}

sal_Bool ImpEditView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    pEditEngine->pImpEditEngine->CheckIdleFormatter();  // Falls schnelles Tippen und MouseButtonDown
    if ( pEditEngine->pImpEditEngine->aStatus.NotifyCursorMovements() )
        pEditEngine->pImpEditEngine->aStatus.GetPrevParagraph() = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( GetEditSelection().Max().GetNode() );
    nTravelXPos = TRAVEL_X_DONTKNOW;
    nExtraCursorFlags = 0;
    nCursorBidiLevel    = CURSOR_BIDILEVEL_DONTKNOW;
    bClickedInSelection = IsSelectionAtPoint( rMouseEvent.GetPosPixel() );
    return pEditEngine->pImpEditEngine->MouseButtonDown( rMouseEvent, GetEditViewPtr() );
}

sal_Bool ImpEditView::MouseMove( const MouseEvent& rMouseEvent )
{
    return pEditEngine->pImpEditEngine->MouseMove( rMouseEvent, GetEditViewPtr() );
}

void ImpEditView::Command( const CommandEvent& rCEvt )
{
    pEditEngine->pImpEditEngine->CheckIdleFormatter();  // Falls schnelles Tippen und MouseButtonDown
    pEditEngine->pImpEditEngine->Command( rCEvt, GetEditViewPtr() );
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
        EditSelection aSel = pEditEngine->pImpEditEngine->SelectWord( rPaM, ::com::sun::star::i18n::WordType::DICTIONARY_WORD );
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

void ImpEditView::DeleteSelected()
{
    DrawSelection();

    pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_DELETE );

    EditPaM aPaM = pEditEngine->pImpEditEngine->DeleteSelected( GetEditSelection() );

    pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_DELETE );

    SetEditSelection( EditSelection( aPaM, aPaM ) );
    pEditEngine->pImpEditEngine->FormatAndUpdate( GetEditViewPtr() );
    ShowCursor( DoAutoScroll(), TRUE );
}

const SvxFieldItem* ImpEditView::GetField( const Point& rPos, sal_uInt16* pPara, sal_uInt16* pPos ) const
{
    if( !GetOutputArea().IsInside( rPos ) )
        return 0;

    Point aDocPos( GetDocPos( rPos ) );
    EditPaM aPaM = pEditEngine->pImpEditEngine->GetPaM( aDocPos, sal_False );

    if ( aPaM.GetIndex() == aPaM.GetNode()->Len() )
    {
        // Sonst immer, wenn Feld ganz am Schluss und Mouse unter Text
        return 0;
    }

    const CharAttribArray& rAttrs = aPaM.GetNode()->GetCharAttribs().GetAttribs();
    sal_uInt16 nXPos = aPaM.GetIndex();
    for ( sal_uInt16 nAttr = rAttrs.Count(); nAttr; )
    {
        EditCharAttrib* pAttr = rAttrs[--nAttr];
        if ( pAttr->GetStart() == nXPos )
            if ( pAttr->Which() == EE_FEATURE_FIELD )
            {
                DBG_ASSERT( pAttr->GetItem()->ISA( SvxFieldItem ), "Kein FeldItem..." );
                if ( pPara )
                    *pPara = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
                if ( pPos )
                    *pPos = pAttr->GetStart();
                return (const SvxFieldItem*)pAttr->GetItem();
            }
    }
    return NULL;
}

BOOL ImpEditView::IsBulletArea( const Point& rPos, sal_uInt16* pPara )
{
    if ( pPara )
        *pPara = 0xFFFF;

    if( !GetOutputArea().IsInside( rPos ) )
        return FALSE;

    Point aDocPos( GetDocPos( rPos ) );
    EditPaM aPaM = pEditEngine->pImpEditEngine->GetPaM( aDocPos, sal_False );

    if ( aPaM.GetIndex() == 0 )
    {
        USHORT nPara = pEditEngine->pImpEditEngine->aEditDoc.GetPos( aPaM.GetNode() );
        Rectangle aBulletArea = pEditEngine->GetBulletArea( nPara );
        long nY = pEditEngine->GetDocPosTopLeft( nPara ).Y();
        ParaPortion* pParaPortion = pEditEngine->pImpEditEngine->GetParaPortions().GetObject( nPara );
        nY += pParaPortion->GetFirstLineOffset();
        if ( ( aDocPos.Y() > ( nY + aBulletArea.Top() ) ) &&
             ( aDocPos.Y() < ( nY + aBulletArea.Bottom() ) ) &&
             ( aDocPos.X() > ( aBulletArea.Left() ) ) &&
             ( aDocPos.X() < ( aBulletArea.Right() ) ) )
        {
            if ( pPara )
                *pPara = nPara;
            return TRUE;
        }
    }

    return FALSE;
}

void ImpEditView::CutCopy( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard, BOOL bCut )
{
    if ( rxClipboard.is() && GetEditSelection().HasRange() )
    {
        uno::Reference< datatransfer::XTransferable > xData = pEditEngine->pImpEditEngine->CreateTransferable( GetEditSelection() );

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            rxClipboard->setContents( xData, NULL );

            // #87756# FlushClipboard, but it would be better to become a TerminateListener to the Desktop and flush on demand...
            uno::Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( rxClipboard, uno::UNO_QUERY );
            if( xFlushableClipboard.is() )
                xFlushableClipboard->flushClipboard();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );

        if ( bCut )
        {
            pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_CUT );
            DeleteSelected();
            pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_CUT );

        }
    }
}

void ImpEditView::Paste( ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& rxClipboard, BOOL bUseSpecial )
{
    if ( rxClipboard.is() )
    {
        uno::Reference< datatransfer::XTransferable > xDataObj;

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            xDataObj = rxClipboard->getContents();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );

        if ( xDataObj.is() && EditEngine::HasValidData( xDataObj ) )
        {
            pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_PASTE );

            EditSelection aSel( GetEditSelection() );
            if ( aSel.HasRange() )
            {
                DrawSelection();
                aSel = pEditEngine->pImpEditEngine->ImpDeleteSelection( aSel );
            }

            PasteOrDropInfos aPasteOrDropInfos;
            aPasteOrDropInfos.nAction = EE_ACTION_PASTE;
            aPasteOrDropInfos.nStartPara = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( aSel.Min().GetNode() );
            pEditEngine->pImpEditEngine->aBeginPasteOrDropHdl.Call( &aPasteOrDropInfos );

            if ( DoSingleLinePaste() )
            {
                datatransfer::DataFlavor aFlavor;
                SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
                if ( xDataObj->isDataFlavorSupported( aFlavor ) )
                {
                    try
                    {
                        uno::Any aData = xDataObj->getTransferData( aFlavor );
                        ::rtl::OUString aTmpText;
                        aData >>= aTmpText;
                        String aText( aTmpText );
                        aText.ConvertLineEnd( LINEEND_LF );
                        aText.SearchAndReplaceAll( LINE_SEP, ' ' );
                        aSel = pEditEngine->pImpEditEngine->ImpInsertText( aSel, aText );
                    }
                    catch( ... )
                    {
                        ; // #i9286# can happen, even if isDataFlavorSupported returns true...
                    }
                }
            }
            else
            {
                aSel = pEditEngine->pImpEditEngine->InsertText( xDataObj, aSel.Min(), bUseSpecial && pEditEngine->pImpEditEngine->GetStatus().AllowPasteSpecial() );
            }

            aPasteOrDropInfos.nEndPara = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( aSel.Max().GetNode() );
            pEditEngine->pImpEditEngine->aEndPasteOrDropHdl.Call( &aPasteOrDropInfos );

            pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_PASTE );
            SetEditSelection( aSel );
            pEditEngine->pImpEditEngine->UpdateSelections();
            pEditEngine->pImpEditEngine->FormatAndUpdate( GetEditViewPtr() );
            ShowCursor( DoAutoScroll(), TRUE );
        }
    }
}


BOOL ImpEditView::IsInSelection( const EditPaM& rPaM )
{
    EditSelection aSel = GetEditSelection();
    if ( !aSel.HasRange() )
        return FALSE;

    aSel.Adjust( pEditEngine->pImpEditEngine->GetEditDoc() );

    USHORT nStartNode = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( aSel.Min().GetNode() );
    USHORT nEndNode = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( aSel.Max().GetNode() );
    USHORT nCurNode = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( rPaM.GetNode() );

    if ( ( nCurNode > nStartNode ) && ( nCurNode < nEndNode ) )
        return TRUE;

    if ( nStartNode == nEndNode )
    {
        if ( nCurNode == nStartNode )
            if ( ( rPaM.GetIndex() >= aSel.Min().GetIndex() ) && ( rPaM.GetIndex() < aSel.Max().GetIndex() ) )
                return TRUE;
    }
    else if ( ( nCurNode == nStartNode ) && ( rPaM.GetIndex() >= aSel.Min().GetIndex() ) )
        return TRUE;
    else if ( ( nCurNode == nEndNode ) && ( rPaM.GetIndex() < aSel.Max().GetIndex() ) )
        return TRUE;

    return FALSE;
}

void ImpEditView::CreateAnchor()
{
    pEditEngine->pImpEditEngine->bInSelection = TRUE;
    GetEditSelection().Min() = GetEditSelection().Max();
}

void ImpEditView::DeselectAll()
{
    pEditEngine->pImpEditEngine->bInSelection = FALSE;
    DrawSelection();
    GetEditSelection().Min() = GetEditSelection().Max();
}

BOOL ImpEditView::IsSelectionAtPoint( const Point& rPosPixel )
{
    if ( pDragAndDropInfo && pDragAndDropInfo->pField )
        return TRUE;

    Point aMousePos( rPosPixel );

    // Logische Einheiten...
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

    if ( ( !GetOutputArea().IsInside( aMousePos ) ) && !pEditEngine->pImpEditEngine->IsInSelectionMode() )
    {
        return FALSE;
    }

    Point aDocPos( GetDocPos( aMousePos ) );
    EditPaM aPaM = pEditEngine->pImpEditEngine->GetPaM( aDocPos, FALSE );
    return IsInSelection( aPaM );
}

BOOL ImpEditView::SetCursorAtPoint( const Point& rPointPixel )
{
    pEditEngine->pImpEditEngine->CheckIdleFormatter();

    Point aMousePos( rPointPixel );

    // Logische Einheiten...
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

    if ( ( !GetOutputArea().IsInside( aMousePos ) ) && !pEditEngine->pImpEditEngine->IsInSelectionMode() )
    {
        return FALSE;
    }

    Point aDocPos( GetDocPos( aMousePos ) );

    // Kann optimiert werden: Erst innerhalb eines Absatzes die Zeilen
    // fuer den PaM durchwuehlen, dann nochmal mit dem PaM fuer das Rect,
    // obwohl die Zeile schon bekannt ist....
    // Das muss doch nicht sein !

    EditPaM aPaM = pEditEngine->pImpEditEngine->GetPaM( aDocPos );
    BOOL bGotoCursor = DoAutoScroll();

    // aTmpNewSel: Diff zwischen alt und neu, nicht die neue Selektion
    EditSelection aTmpNewSel( GetEditSelection().Max(), aPaM );

    GetEditSelection().Max() = aPaM;
    if ( !pEditEngine->pImpEditEngine->aSelEngine.HasAnchor() )
    {
        if ( GetEditSelection().Min() != aPaM )
            pEditEngine->pImpEditEngine->CursorMoved( GetEditSelection().Min().GetNode() );
        GetEditSelection().Min() = aPaM;
    }
    else
    {
        DrawSelection( aTmpNewSel );
    }

    BOOL bForceCursor = ( pDragAndDropInfo ? FALSE : TRUE ) && !pEditEngine->pImpEditEngine->IsInSelectionMode();
    ShowCursor( bGotoCursor, bForceCursor );
    return TRUE;
}


void ImpEditView::HideDDCursor()
{
    if ( pDragAndDropInfo && pDragAndDropInfo->bVisCursor )
    {
        GetWindow()->DrawOutDev( pDragAndDropInfo->aCurSavedCursor.TopLeft(), pDragAndDropInfo->aCurSavedCursor.GetSize(),
                            Point(0,0), pDragAndDropInfo->aCurSavedCursor.GetSize(),*pDragAndDropInfo->pBackground );
        pDragAndDropInfo->bVisCursor = sal_False;
    }
}

void ImpEditView::ShowDDCursor( const Rectangle& rRect )
{
    if ( !pDragAndDropInfo->bVisCursor )
    {
        if ( pOutWin->GetCursor() )
            pOutWin->GetCursor()->Hide();

        Brush aOldBrush = GetWindow()->GetFillInBrush( );
        GetWindow()->SetFillInBrush( Brush( Color( COL_GRAY), BRUSH_50 ) );

        // Hintergrund sichern...
        Rectangle aSaveRec( GetWindow()->LogicToPixel( rRect ) );
        // lieber etwas mehr sichern...
        aSaveRec.Right() += 1;
        aSaveRec.Bottom() += 1;

        Size aNewSzPx( aSaveRec.GetSize() );
        if ( !pDragAndDropInfo->pBackground )
        {
            pDragAndDropInfo->pBackground = new VirtualDevice( *GetWindow() );
            MapMode aMapMode( GetWindow()->GetMapMode() );
            aMapMode.SetOrigin( Point( 0, 0 ) );
            pDragAndDropInfo->pBackground->SetMapMode( aMapMode );

        }
        Size aCurSzPx( pDragAndDropInfo->pBackground->GetOutputSizePixel() );
        if ( ( aCurSzPx.Width() < aNewSzPx.Width() ) ||( aCurSzPx.Height() < aNewSzPx.Height() ) )
        {
            sal_Bool bDone = pDragAndDropInfo->pBackground->SetOutputSizePixel( aNewSzPx );
            DBG_ASSERT( bDone, "Virtuelles Device kaputt?" );
        }


        aSaveRec = GetWindow()->PixelToLogic( aSaveRec );

        pDragAndDropInfo->pBackground->DrawOutDev( Point(0,0), aSaveRec.GetSize(),
                                    aSaveRec.TopLeft(), aSaveRec.GetSize(), *GetWindow() );
        pDragAndDropInfo->aCurSavedCursor = aSaveRec;

        // Cursor malen...
        GetWindow()->DrawRect( rRect );

        pDragAndDropInfo->bVisCursor = sal_True;
        pDragAndDropInfo->aCurCursor = rRect;

        GetWindow()->SetFillInBrush( aOldBrush );
    }
}

void ImpEditView::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException)
{
    DBG_ASSERT( !pDragAndDropInfo, "dragGestureRecognized - DragAndDropInfo exist!" );

    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    Point aMousePosPixel( rDGE.DragOriginX, rDGE.DragOriginY );

    EditSelection aCopySel( GetEditSelection() );
    aCopySel.Adjust( pEditEngine->pImpEditEngine->GetEditDoc() );

    if ( GetEditSelection().HasRange() && bClickedInSelection )
    {
        pDragAndDropInfo = new DragAndDropInfo();
    }
    else
    {
        // Field?!
        USHORT nPara, nPos;
        Point aMousePos = GetWindow()->PixelToLogic( aMousePosPixel );
        const SvxFieldItem* pField = GetField( aMousePos, &nPara, &nPos );
        if ( pField )
        {
            pDragAndDropInfo = new DragAndDropInfo();
            pDragAndDropInfo->pField = pField;
            ContentNode* pNode = pEditEngine->pImpEditEngine->GetEditDoc().GetObject( nPara );
            aCopySel = EditSelection( EditPaM( pNode, nPos ), EditPaM( pNode, nPos+1 ) );
            GetEditSelection() = aCopySel;
            DrawSelection();
            BOOL bGotoCursor = DoAutoScroll();
            BOOL bForceCursor = ( pDragAndDropInfo ? FALSE : TRUE ) && !pEditEngine->pImpEditEngine->IsInSelectionMode();
            ShowCursor( bGotoCursor, bForceCursor );
        }
        else if ( IsBulletArea( aMousePos, &nPara ) )
        {
            pDragAndDropInfo = new DragAndDropInfo();
            pDragAndDropInfo->bOutlinerMode = TRUE;
            EditPaM aStartPaM( pEditEngine->pImpEditEngine->GetEditDoc().GetObject( nPara ), 0 );
            EditPaM aEndPaM( aStartPaM );
            const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
            for ( USHORT n = nPara +1; n < pEditEngine->pImpEditEngine->GetEditDoc().Count(); n++ )
            {
                const SfxUInt16Item& rL = (const SfxUInt16Item&) pEditEngine->GetParaAttrib( n, EE_PARA_OUTLLEVEL );
                if ( rL.GetValue() > rLevel.GetValue() )
                {
                    aEndPaM.SetNode( pEditEngine->pImpEditEngine->GetEditDoc().GetObject( n ) );
                }
                else
                {
                    break;
                }
            }
            aEndPaM.GetIndex() = aEndPaM.GetNode()->Len();
            SetEditSelection( EditSelection( aStartPaM, aEndPaM ) );
        }
    }

    if ( pDragAndDropInfo )
    {

        pDragAndDropInfo->bStarterOfDD = sal_True;

        // Sensibler Bereich, wo gescrollt werden soll.
        Size aSz( 5, 0 );
        aSz = GetWindow()->PixelToLogic( aSz );
        pDragAndDropInfo->nSensibleRange = (sal_uInt16) aSz.Width();
        pDragAndDropInfo->nCursorWidth = (sal_uInt16) aSz.Width() / 2;
        pDragAndDropInfo->aBeginDragSel = pEditEngine->pImpEditEngine->CreateESel( aCopySel );

        uno::Reference< datatransfer::XTransferable > xData = pEditEngine->pImpEditEngine->CreateTransferable( aCopySel );

        sal_Int8 nActions = bReadOnly ? datatransfer::dnd::DNDConstants::ACTION_COPY : datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE;

        rDGE.DragSource->startDrag( rDGE, nActions, 0 /*cursor*/, 0 /*image*/, xData, mxDnDListener );
        // Falls Drag&Move in einer Engine, muessen Copy&Del geklammert sein!
        GetCursor()->Hide();

    }
}

void ImpEditView::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    if ( !bReadOnly && rDSDE.DropSuccess && !pDragAndDropInfo->bOutlinerMode && ( rDSDE.DropAction & datatransfer::dnd::DNDConstants::ACTION_MOVE ) )
    {
        if ( pDragAndDropInfo->bStarterOfDD && pDragAndDropInfo->bDroppedInMe )
        {
            // DropPos: Wohin wurde gedroppt, unabhaengig von laenge.
            ESelection aDropPos( pDragAndDropInfo->aDropSel.nStartPara, pDragAndDropInfo->aDropSel.nStartPos, pDragAndDropInfo->aDropSel.nStartPara, pDragAndDropInfo->aDropSel.nStartPos );
            ESelection aToBeDelSel = pDragAndDropInfo->aBeginDragSel;
            ESelection aNewSel( pDragAndDropInfo->aDropSel.nEndPara, pDragAndDropInfo->aDropSel.nEndPos,
                                pDragAndDropInfo->aDropSel.nEndPara, pDragAndDropInfo->aDropSel.nEndPos );
            sal_Bool bBeforeSelection = aDropPos.IsLess( pDragAndDropInfo->aBeginDragSel );
            sal_uInt16 nParaDiff = pDragAndDropInfo->aBeginDragSel.nEndPara - pDragAndDropInfo->aBeginDragSel.nStartPara;
            if ( bBeforeSelection )
            {
                // aToBeDelSel anpassen.
                DBG_ASSERT( pDragAndDropInfo->aBeginDragSel.nStartPara >= pDragAndDropInfo->aDropSel.nStartPara, "Doch nicht davor?" );
                aToBeDelSel.nStartPara += nParaDiff;
                aToBeDelSel.nEndPara += nParaDiff;
                // Zeichen korrigieren?
                if ( aToBeDelSel.nStartPara == pDragAndDropInfo->aDropSel.nEndPara )
                {
                    sal_uInt16 nMoreChars;
                    if ( pDragAndDropInfo->aDropSel.nStartPara == pDragAndDropInfo->aDropSel.nEndPara )
                        nMoreChars = pDragAndDropInfo->aDropSel.nEndPos - pDragAndDropInfo->aDropSel.nStartPos;
                    else
                        nMoreChars = pDragAndDropInfo->aDropSel.nEndPos;
                    aToBeDelSel.nStartPos += nMoreChars;
                    if ( aToBeDelSel.nStartPara == aToBeDelSel.nEndPara )
                        aToBeDelSel.nEndPos += nMoreChars;
                }
            }
            else
            {
                // aToBeDelSel ist ok, aber Selektion der View
                // muss angepasst werden, wenn davor geloescht wird!
                DBG_ASSERT( pDragAndDropInfo->aBeginDragSel.nStartPara <= pDragAndDropInfo->aDropSel.nStartPara, "Doch nicht davor?" );
                aNewSel.nStartPara -= nParaDiff;
                aNewSel.nEndPara -= nParaDiff;
                // Zeichen korrigieren?
                if ( pDragAndDropInfo->aBeginDragSel.nEndPara == pDragAndDropInfo->aDropSel.nStartPara )
                {
                    sal_uInt16 nLessChars;
                    if ( pDragAndDropInfo->aBeginDragSel.nStartPara == pDragAndDropInfo->aBeginDragSel.nEndPara )
                        nLessChars = pDragAndDropInfo->aBeginDragSel.nEndPos - pDragAndDropInfo->aBeginDragSel.nStartPos;
                    else
                        nLessChars = pDragAndDropInfo->aBeginDragSel.nEndPos;
                    aNewSel.nStartPos -= nLessChars;
                    if ( aNewSel.nStartPara == aNewSel.nEndPara )
                        aNewSel.nEndPos -= nLessChars;
                }
            }

            DrawSelection();
            EditSelection aDelSel( pEditEngine->pImpEditEngine->CreateSel( aToBeDelSel ) );
            DBG_ASSERT( !aDelSel.DbgIsBuggy( pEditEngine->pImpEditEngine->aEditDoc ), "ToBeDel ist buggy!" );
            pEditEngine->pImpEditEngine->ImpDeleteSelection( aDelSel );
            if ( !bBeforeSelection )
            {
                DBG_ASSERT( !pEditEngine->pImpEditEngine->CreateSel( aNewSel ).DbgIsBuggy(pEditEngine->pImpEditEngine->aEditDoc), "Bad" );
                SetEditSelection( pEditEngine->pImpEditEngine->CreateSel( aNewSel ) );
            }
            pEditEngine->pImpEditEngine->FormatAndUpdate( pEditEngine->pImpEditEngine->GetActiveView() );
            DrawSelection();
        }
        else
        {
            // andere EditEngine...
            if ( pEditEngine->pImpEditEngine->ImplHasText() )   // #88630# SC ist removing the content when switching the task
                DeleteSelected();
        }
    }

    if ( pDragAndDropInfo && pDragAndDropInfo->bUndoAction )
        pEditEngine->pImpEditEngine->UndoActionEnd( EDITUNDO_DRAGANDDROP );

    HideDDCursor();
    ShowCursor( DoAutoScroll(), TRUE );
    delete pDragAndDropInfo;
    pDragAndDropInfo = NULL;
}

void ImpEditView::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    DBG_ASSERT( pDragAndDropInfo, "Drop - No Drag&Drop info?!" );

    if ( pDragAndDropInfo->bDragAccepted )
    {
        BOOL bChanges = FALSE;

        HideDDCursor();

        if ( pDragAndDropInfo->bStarterOfDD )
        {
            pEditEngine->pImpEditEngine->UndoActionStart( EDITUNDO_DRAGANDDROP );
            pDragAndDropInfo->bUndoAction = TRUE;
        }

        if ( pDragAndDropInfo->bOutlinerMode )
        {
            bChanges = TRUE;
            GetEditViewPtr()->MoveParagraphs( Range( pDragAndDropInfo->aBeginDragSel.nStartPara, pDragAndDropInfo->aBeginDragSel.nEndPara ), pDragAndDropInfo->nOutlinerDropDest );
        }
        else
        {
            uno::Reference< datatransfer::XTransferable > xDataObj = rDTDE.Transferable;
            if ( xDataObj.is() )
            {
                bChanges = TRUE;
                // Selektion wegmalen...
                DrawSelection();
                EditPaM aPaM( pDragAndDropInfo->aDropDest );

                PasteOrDropInfos aPasteOrDropInfos;
                aPasteOrDropInfos.nAction = EE_ACTION_DROP;
                aPasteOrDropInfos.nStartPara = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( aPaM.GetNode() );
                pEditEngine->pImpEditEngine->aBeginPasteOrDropHdl.Call( &aPasteOrDropInfos );

                EditSelection aNewSel = pEditEngine->pImpEditEngine->InsertText( xDataObj, aPaM, pEditEngine->pImpEditEngine->GetStatus().AllowPasteSpecial() );

                aPasteOrDropInfos.nEndPara = pEditEngine->pImpEditEngine->GetEditDoc().GetPos( aNewSel.Max().GetNode() );
                pEditEngine->pImpEditEngine->aEndPasteOrDropHdl.Call( &aPasteOrDropInfos );

                SetEditSelection( aNewSel );
                pEditEngine->pImpEditEngine->FormatAndUpdate( pEditEngine->pImpEditEngine->GetActiveView() );
                if ( pDragAndDropInfo->bStarterOfDD )
                {
                    // Nur dann setzen, wenn in gleicher Engine!
                    pDragAndDropInfo->aDropSel.nStartPara = pEditEngine->pImpEditEngine->aEditDoc.GetPos( aPaM.GetNode() );
                    pDragAndDropInfo->aDropSel.nStartPos = aPaM.GetIndex();
                    pDragAndDropInfo->aDropSel.nEndPara = pEditEngine->pImpEditEngine->aEditDoc.GetPos( aNewSel.Max().GetNode() );
                    pDragAndDropInfo->aDropSel.nEndPos = aNewSel.Max().GetIndex();
                    pDragAndDropInfo->bDroppedInMe = sal_True;
                }
            }
        }

        if ( bChanges )
        {
            rDTDE.Context->acceptDrop( rDTDE.DropAction );
        }

        if ( !pDragAndDropInfo->bStarterOfDD )
        {
            delete pDragAndDropInfo;
            pDragAndDropInfo = NULL;
        }

        rDTDE.Context->dropComplete( bChanges );
    }
}

void ImpEditView::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    if ( !pDragAndDropInfo )
        pDragAndDropInfo = new DragAndDropInfo( );

    pDragAndDropInfo->bHasValidData = sal_False;

    // Check for supported format...
    // Only check for text, will also be there if bin or rtf
    datatransfer::DataFlavor aTextFlavor;
    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aTextFlavor );
    const ::com::sun::star::datatransfer::DataFlavor* pFlavors = rDTDEE.SupportedDataFlavors.getConstArray();
    int nFlavors = rDTDEE.SupportedDataFlavors.getLength();
    for ( int n = 0; n < nFlavors; n++ )
    {
        if( TransferableDataHelper::IsEqual( pFlavors[n], aTextFlavor ) )
        {
            pDragAndDropInfo->bHasValidData = sal_True;
            break;
        }
    }

    dragOver( rDTDEE );
}

void ImpEditView::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    HideDDCursor();

    if ( !pDragAndDropInfo->bStarterOfDD )
    {
        delete pDragAndDropInfo;
        pDragAndDropInfo = NULL;
    }
}

void ImpEditView::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    Point aMousePos( rDTDE.LocationX, rDTDE.LocationY );
    aMousePos = GetWindow()->PixelToLogic( aMousePos );

    sal_Bool bAccept = sal_False;

    if ( GetOutputArea().IsInside( aMousePos ) && !bReadOnly )
    {
        sal_Int8 nSupportedActions = bReadOnly ? datatransfer::dnd::DNDConstants::ACTION_COPY : datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE;

        if ( pDragAndDropInfo->bHasValidData /* && ( nSupportedActions & rDTDE.DropAction ) MT: Default = 0x80 ?! */ )
        {
            bAccept = sal_True;

            sal_Bool bAllowScroll = DoAutoScroll();
            if ( bAllowScroll )
            {
                long nScrollX = 0;
                long nScrollY = 0;
                // pruefen, ob im sensitiven Bereich
                if ( ( (aMousePos.X()-pDragAndDropInfo->nSensibleRange) < GetOutputArea().Left() ) && ( ( aMousePos.X() + pDragAndDropInfo->nSensibleRange ) > GetOutputArea().Left() ) )
                        nScrollX = GetOutputArea().GetWidth() / SCRLRANGE;
                else if ( ( (aMousePos.X()+pDragAndDropInfo->nSensibleRange) > GetOutputArea().Right() ) && ( ( aMousePos.X() - pDragAndDropInfo->nSensibleRange ) < GetOutputArea().Right() ) )
                        nScrollX = -( GetOutputArea().GetWidth() / SCRLRANGE );

                if ( ( (aMousePos.Y()-pDragAndDropInfo->nSensibleRange) < GetOutputArea().Top() ) && ( ( aMousePos.Y() + pDragAndDropInfo->nSensibleRange ) > GetOutputArea().Top() ) )
                        nScrollY = GetOutputArea().GetHeight() / SCRLRANGE;
                else if ( ( (aMousePos.Y()+pDragAndDropInfo->nSensibleRange) > GetOutputArea().Bottom() ) && ( ( aMousePos.Y() - pDragAndDropInfo->nSensibleRange ) < GetOutputArea().Bottom() ) )
                        nScrollY = -( GetOutputArea().GetHeight() / SCRLRANGE );

                if ( nScrollX || nScrollY )
                {
                    HideDDCursor();
                    Scroll( nScrollX, nScrollY, RGCHK_PAPERSZ1 );
                }
            }

            Point aDocPos( GetDocPos( aMousePos ) );
            EditPaM aPaM = pEditEngine->pImpEditEngine->GetPaM( aDocPos );
            pDragAndDropInfo->aDropDest = aPaM;
            if ( pDragAndDropInfo->bOutlinerMode )
            {
                USHORT nPara = pEditEngine->pImpEditEngine->aEditDoc.GetPos( aPaM.GetNode() );
                ParaPortion* pPPortion = pEditEngine->pImpEditEngine->GetParaPortions().SaveGetObject( nPara );
                long nDestParaStartY = pEditEngine->pImpEditEngine->GetParaPortions().GetYOffset( pPPortion );
                long nRel = aDocPos.Y() - nDestParaStartY;
                if ( nRel < ( pPPortion->GetHeight() / 2 ) )
                {
                    pDragAndDropInfo->nOutlinerDropDest = nPara;
                }
                else
                {
                    pDragAndDropInfo->nOutlinerDropDest = nPara+1;
                }

                if( ( pDragAndDropInfo->nOutlinerDropDest >= pDragAndDropInfo->aBeginDragSel.nStartPara ) &&
                    ( pDragAndDropInfo->nOutlinerDropDest <= (pDragAndDropInfo->aBeginDragSel.nEndPara+1) ) )
                {
                    bAccept = FALSE;
                }
            }
            else if ( HasSelection() )
            {
                // es darf nicht in eine Selektion gedroppt werden
                EPaM aP = pEditEngine->pImpEditEngine->CreateEPaM( aPaM );
                ESelection aDestSel( aP.nPara, aP.nIndex, aP.nPara, aP.nIndex);
                ESelection aCurSel = pEditEngine->pImpEditEngine->CreateESel( GetEditSelection() );
                aCurSel.Adjust();
                if ( !aDestSel.IsLess( aCurSel ) && !aDestSel.IsGreater( aCurSel ) )
                {
                    bAccept = FALSE;
                }
            }
            if ( bAccept )
            {
                Rectangle aEditCursor;
                if ( pDragAndDropInfo->bOutlinerMode )
                {
                    long nDDYPos;
                    if ( pDragAndDropInfo->nOutlinerDropDest < pEditEngine->pImpEditEngine->GetEditDoc().Count() )
                    {
                        ParaPortion* pPPortion = pEditEngine->pImpEditEngine->GetParaPortions().SaveGetObject( pDragAndDropInfo->nOutlinerDropDest );
                        nDDYPos = pEditEngine->pImpEditEngine->GetParaPortions().GetYOffset( pPPortion );
                    }
                    else
                    {
                        nDDYPos = pEditEngine->pImpEditEngine->GetTextHeight();
                    }
                    Point aStartPos( 0, nDDYPos );
                    aStartPos = GetWindowPos( aStartPos );
                    Point aEndPos( GetOutputArea().GetWidth(), nDDYPos );
                    aEndPos = GetWindowPos( aEndPos );
                    aEditCursor = GetWindow()->LogicToPixel( Rectangle( aStartPos, aEndPos ) );
                    if ( !pEditEngine->IsVertical() )
                    {
                        aEditCursor.Top()--;
                        aEditCursor.Bottom()++;
                    }
                    else
                    {
                        aEditCursor.Left()--;
                        aEditCursor.Right()++;
                    }
                    aEditCursor = GetWindow()->PixelToLogic( aEditCursor );
                }
                else
                {
                    aEditCursor = pEditEngine->pImpEditEngine->PaMtoEditCursor( aPaM );
                    Point aTopLeft( GetWindowPos( aEditCursor.TopLeft() ) );
                    aEditCursor.SetPos( aTopLeft );
                    aEditCursor.Right() = aEditCursor.Left() + pDragAndDropInfo->nCursorWidth;
                    aEditCursor = GetWindow()->LogicToPixel( aEditCursor );
                    aEditCursor = GetWindow()->PixelToLogic( aEditCursor );
                }

                sal_Bool bCursorChanged = !pDragAndDropInfo->bVisCursor || ( pDragAndDropInfo->aCurCursor != aEditCursor );
                if ( bCursorChanged )
                {
                    HideDDCursor();
                    ShowDDCursor(aEditCursor );
                }
                pDragAndDropInfo->bDragAccepted = TRUE;
                rDTDE.Context->acceptDrag( rDTDE.DropAction );
            }
        }
    }

    if ( !bAccept )
    {
        HideDDCursor();
        pDragAndDropInfo->bDragAccepted = FALSE;
        rDTDE.Context->rejectDrag();
    }
}

void ImpEditView::AddDragAndDropListeners()
{
    Window* pWindow = GetWindow();
    if ( !bActiveDragAndDropListener && pWindow && pWindow->GetDragGestureRecognizer().is() )
    {
        vcl::unohelper::DragAndDropWrapper* pDnDWrapper = new vcl::unohelper::DragAndDropWrapper( this );
        mxDnDListener = pDnDWrapper;

        uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
        pWindow->GetDragGestureRecognizer()->addDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( xDGL, uno::UNO_QUERY );
        pWindow->GetDropTarget()->addDropTargetListener( xDTL );
        pWindow->GetDropTarget()->setActive( sal_True );
        pWindow->GetDropTarget()->setDefaultActions( datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE );

        bActiveDragAndDropListener = TRUE;
    }
}

void ImpEditView::RemoveDragAndDropListeners()
{
    if ( bActiveDragAndDropListener && GetWindow() && GetWindow()->GetDragGestureRecognizer().is() )
    {
        uno::Reference< datatransfer::dnd::XDragGestureListener> xDGL( mxDnDListener, uno::UNO_QUERY );
        GetWindow()->GetDragGestureRecognizer()->removeDragGestureListener( xDGL );
        uno::Reference< datatransfer::dnd::XDropTargetListener> xDTL( xDGL, uno::UNO_QUERY );
        GetWindow()->GetDropTarget()->removeDropTargetListener( xDTL );

        if ( mxDnDListener.is() )
        {
            uno::Reference< lang::XEventListener> xEL( mxDnDListener, uno::UNO_QUERY );
            xEL->disposing( lang::EventObject() );  // #95154# Empty Source means it's the Client
            mxDnDListener.clear();
        }

        bActiveDragAndDropListener = FALSE;
    }
}