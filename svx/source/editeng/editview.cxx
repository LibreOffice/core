/*************************************************************************
 *
 *  $RCSfile: editview.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-07 18:25:29 $
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

#define __PRIVATE 1

#include <srchitem.hxx>

#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>
#include <flditem.hxx>
#include <svxacorr.hxx>
#include <eerdll.hxx>
#include <eerdll2.hxx>
#include <editeng.hrc>
#include <tools/isolang.hxx>
#include <vcl/menu.hxx>

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARY1_HPP_
#include <com/sun/star/linguistic2/XDictionary1.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HDL_
#include <com/sun/star/beans/PropertyValues.hdl>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::linguistic2;


DBG_NAME( EditView );

void SetSearchFlags( SvxSearchItem& rSearchItem, sal_uInt16 nSearchFlags )
{
    rSearchItem.SetWordOnly( nSearchFlags &  EE_SEARCH_WORDONLY ? sal_True : sal_False );
    rSearchItem.SetExact( nSearchFlags &  EE_SEARCH_EXACT ? sal_True : sal_False );
    rSearchItem.SetBackward( nSearchFlags &  EE_SEARCH_BACKWARD ? sal_True : sal_False );
    rSearchItem.SetSelection( nSearchFlags &  EE_SEARCH_INSELECTION ? sal_True : sal_False );
    rSearchItem.SetRegExp( nSearchFlags &  EE_SEARCH_REGEXPR ? sal_True : sal_False );
    rSearchItem.SetPattern( nSearchFlags &  EE_SEARCH_PATTERN ? sal_True : sal_False );
}


// ----------------------------------------------------------------------
// class EditView
// ----------------------------------------------------------------------
EditView::EditView( EditEngine* pEng, Window* pWindow )
{
    DBG_CTOR( EditView, 0 );
    pImpEditView = new ImpEditView( pEng, pWindow );
}

EditView::~EditView()
{
    DBG_DTOR( EditView, 0 );
    delete pImpEditView;
}

ImpEditEngine* EditView::GetImpEditEngine() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->pEditEngine->pImpEditEngine;
}

EditEngine* EditView::GetEditEngine() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->pEditEngine;
}

void EditView::Invalidate()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !pImpEditView->DoInvalidateMore() )
        pImpEditView->GetWindow()->Invalidate( pImpEditView->aOutArea );
    else
    {
        Rectangle aRect( pImpEditView->aOutArea );
        long nMore = pImpEditView->GetWindow()->PixelToLogic( Size( pImpEditView->GetInvalidateMore(), 0 ) ).Width();
        aRect.Left() -= nMore;
        aRect.Right() += nMore;
        aRect.Top() -= nMore;
        aRect.Bottom() += nMore;
        pImpEditView->GetWindow()->Invalidate( aRect );
    }
}

void EditView::SetReadOnly( sal_Bool bReadOnly )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->bReadOnly = bReadOnly;
}

sal_Bool EditView::IsReadOnly() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->bReadOnly;
}

void EditView::SetSelection( const ESelection& rESel )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    // Falls jemand gerade ein leeres Attribut hinterlassen hat,
    // und dann der Outliner die Selektion manipulitert:
    if ( !pImpEditView->GetEditSelection().HasRange() )
    {
        ContentNode* pNode = pImpEditView->GetEditSelection().Max().GetNode();
        PIMPEE->CursorMoved( pNode );
    }
    EditSelection aNewSelection( PIMPEE->ConvertSelection( rESel.nStartPara, rESel.nStartPos, rESel.nEndPara, rESel.nEndPos ) );

    // Wenn nach einem KeyInput die Selection manipuliert wird:
    PIMPEE->CheckIdleFormatter();

    // Selektion darf nicht bei einem unsichtbaren Absatz Starten/Enden:
    ParaPortion* pPortion = PIMPEE->FindParaPortion( aNewSelection.Min().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = PIMPEE->GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : PIMPEE->GetEditDoc().GetObject( 0 );
        aNewSelection.Min() = EditPaM( pNode, pNode->Len() );
    }
    pPortion = PIMPEE->FindParaPortion( aNewSelection.Max().GetNode() );
    if ( !pPortion->IsVisible() )
    {
        pPortion = PIMPEE->GetPrevVisPortion( pPortion );
        ContentNode* pNode = pPortion ? pPortion->GetNode() : PIMPEE->GetEditDoc().GetObject( 0 );
        aNewSelection.Max() = EditPaM( pNode, pNode->Len() );
    }

    pImpEditView->DrawSelection();  // alte Selektion 'weg-zeichnen'
    pImpEditView->SetEditSelection( aNewSelection );
    pImpEditView->DrawSelection();
    sal_Bool bGotoCursor = pImpEditView->DoAutoScroll();
    ShowCursor( bGotoCursor );
}

ESelection EditView::GetSelection() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    ESelection aSelection;

    aSelection.nStartPara = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Min().GetNode() );
    aSelection.nEndPara = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Max().GetNode() );

    aSelection.nStartPos = pImpEditView->GetEditSelection().Min().GetIndex();
    aSelection.nEndPos = pImpEditView->GetEditSelection().Max().GetIndex();

    return aSelection;
}

sal_Bool EditView::HasSelection() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->HasSelection();
}

void EditView::DeleteSelected()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->DrawSelection();

    PIMPEE->UndoActionStart( EDITUNDO_DELETE );

    EditPaM aPaM = PIMPEE->DeleteSelected( pImpEditView->GetEditSelection() );

    PIMPEE->UndoActionEnd( EDITUNDO_DELETE );

    pImpEditView->SetEditSelection( EditSelection( aPaM, aPaM ) );
    PIMPEE->FormatAndUpdate( this );
    ShowCursor();
}

void EditView::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Paint( pImpEditView, rRect );
}

void EditView::SetEditEngine( EditEngine* pEditEng )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->pEditEngine = pEditEng;
    EditSelection aStartSel;
    aStartSel = PIMPEE->GetEditDoc().GetStartPaM();
    pImpEditView->SetEditSelection( aStartSel );
}

void EditView::SetWindow( Window* pWin )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->pOutWin = pWin;
    PIMPEE->GetSelEngine().Reset();
}

Window* EditView::GetWindow() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->pOutWin;
}

void EditView::SetVisArea( const Rectangle& rRec )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetVisArea( rRec );
}

const Rectangle& EditView::GetVisArea() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetVisArea();
}

void EditView::SetOutputArea( const Rectangle& rRec )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetOutputArea( rRec );

    // Rest nur hier, wenn API-Aufruf:
    pImpEditView->CalcAnchorPoint();
    if ( PIMPEE->GetStatus().AutoPageSize() )
        pImpEditView->RecalcOutputArea();
    pImpEditView->ShowCursor( sal_False, sal_False );
}

const Rectangle& EditView::GetOutputArea() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetOutputArea();
}

void EditView::SetPointer( const Pointer& rPointer )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetPointer( rPointer );
}

const Pointer& EditView::GetPointer() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetPointer();
}

void EditView::SetCursor( const Cursor& rCursor )
{
    DBG_CHKTHIS( EditView, 0 );
    delete pImpEditView->pCursor;
    pImpEditView->pCursor = new Cursor( rCursor );
}

Cursor* EditView::GetCursor() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->pCursor;
}

sal_uInt16 EditView::SearchAndReplace( const XubString& rBefore, const XubString& rAfter, EditSearchMode eSearchMode )
{
    // eSearchMode auswerten!
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->UndoActionStart( EDITUNDO_SRCHANDREPL );
    sal_uInt16 n = PIMPEE->SearchAndReplace( rBefore, rAfter, this );
    PIMPEE->UndoActionEnd( EDITUNDO_SRCHANDREPL );
    return n;
}

sal_Bool EditView::Search( const XubString& rText, sal_uInt16 nFlags )
{
#ifndef SVX_LIGHT
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    SvxSearchItem aSrchItem( SID_SEARCH_ITEM );
    SetSearchFlags( aSrchItem, nFlags );
    aSrchItem.SetSearchString( rText );
    return PIMPEE->Search( aSrchItem, this );
#else
    return sal_False;
#endif
}

void EditView::InsertText( const XubString& rStr, sal_Bool bSelect )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    ImpEditEngine* pImpEE = PIMPEE;
    pImpEditView->DrawSelection();

    EditPaM aPaM1;
    if ( bSelect )
    {
        EditSelection aTmpSel( pImpEditView->GetEditSelection() );
        aTmpSel.Adjust( pImpEE->GetEditDoc() );
        aPaM1 = aTmpSel.Min();
    }

    pImpEE->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM2( pImpEE->InsertText( pImpEditView->GetEditSelection(), rStr ) );
    pImpEE->UndoActionEnd( EDITUNDO_INSERT );

    if ( bSelect )
    {
        DBG_ASSERT( !aPaM1.DbgIsBuggy( pImpEE->GetEditDoc() ), "Insert: PaM kaputt" );
        pImpEditView->SetEditSelection( EditSelection( aPaM1, aPaM2 ) );
    }
    else
        pImpEditView->SetEditSelection( EditSelection( aPaM2, aPaM2 ) );

    pImpEE->FormatAndUpdate( this );
}

sal_Bool EditView::PostKeyEvent( const KeyEvent& rKeyEvent )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->pEditEngine->PostKeyEvent( rKeyEvent, this );
}

sal_Bool EditView::MouseButtonUp( const MouseEvent& rMouseEvent )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( PIMPEE->aStatus.NotifyCursorMovements() )
    {
        if ( PIMPEE->aStatus.GetPrevParagraph() != PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Max().GetNode() ) )
        {
            PIMPEE->aStatus.GetStatusWord() = PIMPEE->aStatus.GetStatusWord() | EE_STAT_CRSRLEFTPARA;
            PIMPEE->CallStatusHdl();
        }
    }
    pImpEditView->nTravelXPos = TRAVEL_X_DONTKNOW;
    return PIMPEE->MouseButtonUp( rMouseEvent, this );
}

sal_Bool EditView::MouseButtonDown( const MouseEvent& rMouseEvent )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->CheckIdleFormatter();   // Falls schnelles Tippen und MouseButtonDown
    if ( PIMPEE->aStatus.NotifyCursorMovements() )
        PIMPEE->aStatus.GetPrevParagraph() = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Max().GetNode() );
    pImpEditView->nTravelXPos = TRAVEL_X_DONTKNOW;
    return PIMPEE->MouseButtonDown( rMouseEvent, this );
}

sal_Bool EditView::MouseMove( const MouseEvent& rMouseEvent )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->nTravelXPos = TRAVEL_X_DONTKNOW;
    return PIMPEE->MouseMove( rMouseEvent, this );
}

void EditView::Command( const CommandEvent& rCEvt )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->CheckIdleFormatter();   // Falls schnelles Tippen und MouseButtonDown
    PIMPEE->Command( rCEvt, this );
}

void EditView::ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

// Draw vertraegt die Assertion nicht, spaeter mal aktivieren
//  DBG_ASSERT( pImpEditView->pEditEngine->HasView( this ), "ShowCursor - View nicht angemeldet!" );

    if ( pImpEditView->pEditEngine->HasView( this ) )
    {
        // Das ControlWord hat mehr Gewicht:
        if ( !pImpEditView->DoAutoScroll() )
            bGotoCursor = sal_False;
        pImpEditView->ShowCursor( bGotoCursor, bForceVisCursor );
    }
}

void EditView::HideCursor()
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->GetCursor()->Hide();
}

Pair EditView::Scroll( long ndX, long ndY, BYTE nRangeCheck )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->Scroll( ndX, ndY, nRangeCheck );
}

const SfxItemSet& EditView::GetEmptyItemSet()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->GetEmptyItemSet();
}

void EditView::SetAttribs( const SfxItemSet& rSet )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blinde Selection in ...." );

    // Kein Undo-Kappseln noetig...
    pImpEditView->DrawSelection();
    PIMPEE->SetAttribs( pImpEditView->GetEditSelection(), rSet, ATTRSPECIAL_WHOLEWORD );
    PIMPEE->FormatAndUpdate( this );
}

void EditView::SetParaAttribs( const SfxItemSet& rSet, sal_uInt16 nPara )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    // Kein Undo-Kappseln noetig...
    PIMPEE->SetParaAttribs( nPara, rSet );
    // Beim Aendern von Absatzattributen muss immer formatiert werden...
    PIMPEE->FormatAndUpdate( this );
}

void EditView::RemoveAttribs( sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    pImpEditView->DrawSelection();
    PIMPEE->UndoActionStart( EDITUNDO_RESETATTRIBS );
    PIMPEE->RemoveCharAttribs( pImpEditView->GetEditSelection(), bRemoveParaAttribs, nWhich  );
    PIMPEE->UndoActionEnd( EDITUNDO_RESETATTRIBS );
    PIMPEE->FormatAndUpdate( this );
}

void EditView::RemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    // Undo ?????????????
    PIMPEE->RemoveCharAttribs( nPara, nWhich );
    PIMPEE->FormatAndUpdate( this );
}

SfxItemSet EditView::GetAttribs()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    DBG_ASSERT( !pImpEditView->aEditSelection.IsInvalid(), "Blinde Selection in ...." );
    return PIMPEE->GetAttribs( pImpEditView->GetEditSelection() );
}

void EditView::Undo()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Undo( this );
}

void EditView::Redo()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Redo( this );
}

sal_uInt32 EditView::Read( SvStream& rInput, EETextFormat eFormat, sal_Bool bSelect, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    EditSelection aOldSel( pImpEditView->GetEditSelection() );
    pImpEditView->DrawSelection();
    PIMPEE->UndoActionStart( EDITUNDO_READ );
    EditPaM aEndPaM = PIMPEE->Read( rInput, eFormat, aOldSel, pHTTPHeaderAttrs );
    PIMPEE->UndoActionEnd( EDITUNDO_READ );
    EditSelection aNewSel( aEndPaM, aEndPaM );
    if ( bSelect )
    {
        aOldSel.Adjust( PIMPEE->GetEditDoc() );
        aNewSel.Min() = aOldSel.Min();
    }

    pImpEditView->SetEditSelection( aNewSel );
    sal_Bool bGotoCursor = pImpEditView->DoAutoScroll();
    ShowCursor( bGotoCursor );

    return rInput.GetError();
}

sal_uInt32 EditView::Write( SvStream& rOutput, EETextFormat eFormat )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->Write( rOutput, eFormat, pImpEditView->GetEditSelection() );
    ShowCursor();
    return rOutput.GetError();
}

void EditView::Cut()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPEE->UndoActionStart( EDITUNDO_CUT );
    EditSelection aCurSel ( PIMPEE->CutCopy( this, sal_True ) );
    PIMPEE->UndoActionEnd( EDITUNDO_CUT );

    pImpEditView->SetEditSelection( aCurSel );
    PIMPEE->UpdateSelections();
    PIMPEE->FormatAndUpdate( this );
}

void EditView::Copy()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->CutCopy( this, sal_False );
}

void EditView::Paste()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPEE->UndoActionStart( EDITUNDO_PASTE );
    EditSelection aCurSel( PIMPEE->Paste( this, sal_False ) );
    PIMPEE->UndoActionEnd( EDITUNDO_PASTE );

    pImpEditView->SetEditSelection( aCurSel );
    PIMPEE->UpdateSelections();
    PIMPEE->FormatAndUpdate( this );
}

void EditView::PasteSpecial()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPEE->UndoActionStart( EDITUNDO_PASTE );
    EditSelection aCurSel ( PIMPEE->Paste( this, sal_True ) );
    PIMPEE->UndoActionEnd( EDITUNDO_PASTE );

    pImpEditView->SetEditSelection( aCurSel );
    PIMPEE->UpdateSelections();
    PIMPEE->FormatAndUpdate( this );
}

void EditView::EnablePaste( sal_Bool bEnable )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->EnablePaste( bEnable );
}

sal_Bool EditView::IsPasteEnabled() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->IsPasteEnabled();
}

Point EditView::GetWindowPosTopLeft( sal_uInt16 nParagraph )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aDocPos( pImpEditView->pEditEngine->GetDocPosTopLeft( nParagraph ) );
    return pImpEditView->GetWindowPos( aDocPos );
}

sal_uInt16 EditView::GetParagraph( const Point& rMousePosPixel )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aMousePos( rMousePosPixel );
    aMousePos = GetWindow()->PixelToLogic( aMousePos );
    Point aDocPos( pImpEditView->GetDocPos( aMousePos ) );
    sal_uInt16 nParagraph = PIMPEE->GetParaPortions().FindParagraph( aDocPos.Y() );
    return nParagraph;
}

void EditView::IndentBlock()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPEE->IndentBlock( this, sal_True );
}

void EditView::UnindentBlock()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    PIMPEE->IndentBlock( this, sal_False );
}

EESelectionMode EditView::GetSelectionMode() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetSelectionMode();
}

void EditView::SetSelectionMode( EESelectionMode eMode )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetSelectionMode( eMode );
}

XubString EditView::GetSelected()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->GetSelected( pImpEditView->GetEditSelection() );
}

void EditView::MoveParagraphs( Range aParagraphs, sal_uInt16 nNewPos )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->UndoActionStart( EDITUNDO_MOVEPARAS );
    PIMPEE->MoveParagraphs( aParagraphs, nNewPos, this );
    PIMPEE->UndoActionEnd( EDITUNDO_MOVEPARAS );
}

void EditView::SetBackgroundColor( const Color& rColor )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->SetBackgroundColor( rColor );
}

Color EditView::GetBackgroundColor() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return pImpEditView->GetBackgroundColor();
}

void EditView::SetControlWord( sal_uInt32 nWord )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->nControl = nWord;
}

sal_uInt32 EditView::GetControlWord() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->nControl;
}

EditTextObject* EditView::CreateTextObject()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->CreateTextObject( pImpEditView->GetEditSelection() );
}

void EditView::InsertText( const EditTextObject& rTextObject )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->DrawSelection();

    PIMPEE->UndoActionStart( EDITUNDO_INSERT );
    EditSelection aTextSel( PIMPEE->InsertText( rTextObject, pImpEditView->GetEditSelection() ) );
    PIMPEE->UndoActionEnd( EDITUNDO_INSERT );

    aTextSel.Min() = aTextSel.Max();    // Selektion nicht behalten.
    pImpEditView->SetEditSelection( aTextSel );
    PIMPEE->FormatAndUpdate( this );
}

sal_Bool EditView::Drop( const DropEvent& rEvt )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    // Undo-Kappselung nur intern, falls nicht noetig....
    return PIMPEE->Drop( rEvt, this );
}

ESelection EditView::GetDropPos()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->GetDropPos();
}

sal_Bool EditView::QueryDrop( DropEvent& rEvt )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return IsReadOnly() ? sal_False : PIMPEE->QueryDrop( rEvt, this );
}

void EditView::SetEditEngineUpdateMode( sal_Bool bUpdate )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->SetUpdateMode( bUpdate, this );
}

void EditView::ForceUpdate()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    PIMPEE->SetUpdateMode( sal_True, this, sal_True );
}

void EditView::SetStyleSheet( SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    EditSelection aSel( pImpEditView->GetEditSelection() );
    PIMPEE->UndoActionStart( EDITUNDO_STYLESHEET );
    PIMPEE->SetStyleSheet( aSel, pStyle );
    PIMPEE->UndoActionEnd( EDITUNDO_STYLESHEET );
}

SfxStyleSheet* EditView::GetStyleSheet() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );


    EditSelection aSel( pImpEditView->GetEditSelection() );
    aSel.Adjust( PIMPEE->GetEditDoc() );
    sal_uInt16 nStartPara = PIMPEE->GetEditDoc().GetPos( aSel.Min().GetNode() );
    sal_uInt16 nEndPara = PIMPEE->GetEditDoc().GetPos( aSel.Max().GetNode() );

    SfxStyleSheet* pStyle = NULL;
    for ( sal_uInt16 n = nStartPara; n <= nEndPara; n++ )
    {
        SfxStyleSheet* pTmpStyle = PIMPEE->GetStyleSheet( n );
        if ( ( n != nStartPara ) && ( pStyle != pTmpStyle ) )
            return NULL;    // Nicht eindeutig.
        pStyle = pTmpStyle;
    }
    return pStyle;
}

sal_Bool EditView::IsInsertMode() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->IsInsertMode();
}

void EditView::SetInsertMode( sal_Bool bInsert )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetInsertMode( bInsert );
}

void EditView::SetAnchorMode( EVAnchorMode eMode )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetAnchorMode( eMode );
}

EVAnchorMode EditView::GetAnchorMode() const
{
    DBG_CHKTHIS( EditView, 0 );
    return pImpEditView->GetAnchorMode();
}

sal_Bool EditView::MatchGroup()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    EditSelection aNewSel( PIMPEE->MatchGroup( pImpEditView->GetEditSelection() ) );
    if ( aNewSel.HasRange() )
    {
        pImpEditView->DrawSelection();
        pImpEditView->SetEditSelection( aNewSel );
        pImpEditView->DrawSelection();
        ShowCursor();
        return sal_True;
    }
    return sal_False;
}

void EditView::CompleteAutoCorrect()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !pImpEditView->HasSelection() && PIMPEE->GetStatus().DoAutoCorrect() )
    {
        pImpEditView->DrawSelection();
        EditSelection aSel = pImpEditView->GetEditSelection();
        aSel = PIMPEE->EndOfWord( aSel.Max() );
        // MT 06/00: Why pass EditSelection to AutoCorrect, not EditPaM?!
        aSel = PIMPEE->AutoCorrect( aSel, 0, !IsInsertMode() );
        pImpEditView->SetEditSelection( aSel );
        if ( PIMPEE->IsModified() )
            PIMPEE->FormatAndUpdate( this );
    }
}

EESpellState EditView::StartSpeller( LanguageType eLang, sal_Bool bMultipleDoc )
{
#ifdef SVX_LIGHT
    return EE_SPELL_NOSPELLER;
#else
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !PIMPEE->GetSpeller().is() )
        return EE_SPELL_NOSPELLER;
    PIMPEE->eDefaultLanguage = eLang;
    SvxSpellWrapper::CheckSpellLang( PIMPEE->GetSpeller(), eLang );
    return PIMPEE->Spell( this, bMultipleDoc );
#endif
}

EESpellState EditView::StartThesaurus( LanguageType eLang )
{
#ifdef SVX_LIGHT
    return EE_SPELL_NOSPELLER;
#else
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    if ( !PIMPEE->GetSpeller().is() )
        return EE_SPELL_NOSPELLER;
    PIMPEE->eDefaultLanguage = eLang;
    return PIMPEE->StartThesaurus( this );
#endif
}

sal_uInt16 EditView::StartSearchAndReplace( const SvxSearchItem& rSearchItem )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    return PIMPEE->StartSearchAndReplace( this, rSearchItem );
}

sal_Bool EditView::IsCursorAtWrongSpelledWord( sal_Bool bMarkIfWrong )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    sal_Bool bIsWrong = sal_False;
    if ( !HasSelection() )
    {
        EditPaM aPaM = pImpEditView->GetEditSelection().Max();
        bIsWrong = pImpEditView->IsWrongSpelledWord( aPaM, bMarkIfWrong );
    }
    return bIsWrong;
}

sal_Bool EditView::IsWrongSpelledWordAtPos( const Point& rPosPixel, sal_Bool bMarkIfWrong )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aPos ( pImpEditView->GetWindow()->PixelToLogic( rPosPixel ) );
    aPos = pImpEditView->GetDocPos( aPos );
    EditPaM aPaM = pImpEditView->pEditEngine->pImpEditEngine->GetPaM( aPos, sal_False );
    return pImpEditView->IsWrongSpelledWord( aPaM , bMarkIfWrong );
}

void EditView::ExecuteSpellPopup( const Point& rPosPixel, Link* pCallBack )
{
#ifndef SVX_LIGHT
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    Point aPos ( pImpEditView->GetWindow()->PixelToLogic( rPosPixel ) );
    aPos = pImpEditView->GetDocPos( aPos );
    EditPaM aPaM = pImpEditView->pEditEngine->pImpEditEngine->GetPaM( aPos, sal_False );
    Reference< XSpellChecker1 >  xSpeller( PIMPEE->GetSpeller() );
    if ( xSpeller.is() && pImpEditView->IsWrongSpelledWord( aPaM, sal_True ) )
    {
        PopupMenu aPopupMenu( EditResId( RID_MENU_SPELL ) );
        PopupMenu *pAutoMenu = aPopupMenu.GetPopupMenu( MN_AUTOCORR );
        PopupMenu *pInsertMenu = aPopupMenu.GetPopupMenu( MN_INSERT );

        // Gibt es Replace-Vorschlaege?
        String aSelected( GetSelected() );
        Reference< XSpellAlternatives >  xSpellAlt =
                xSpeller->spell( aSelected, PIMPEE->GetLanguage(),
                                 Sequence< PropertyValue >() );
        Sequence< OUString > aAlt;
        if (xSpellAlt.is())
            aAlt = xSpellAlt->getAlternatives();
        const OUString *pAlt = aAlt.getConstArray();
        sal_uInt16 nWords = aAlt.getLength();
        if ( nWords )
        {
            for ( sal_uInt16 nW = 0; nW < nWords; nW++ )
            {
                String aAlternate( pAlt[nW] );
                aPopupMenu.InsertItem( MN_ALTSTART+nW, aAlternate, 0, nW );
                pAutoMenu->InsertItem( MN_AUTOSTART+nW, aAlternate, 0, nW );
            }
            aPopupMenu.InsertSeparator( nWords );
        }
        else
            aPopupMenu.RemoveItem( MN_AUTOCORR );   // Loeschen?

        Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );

        Sequence< Reference< XDictionary >  > aDics;
        if (xDicList.is())
            aDics = xDicList->getDictionaries();
        const Reference< XDictionary >  *pDic = aDics.getConstArray();
        sal_uInt16 nLanguage = PIMPEE->eDefaultLanguage;
        sal_uInt16 nDicCount = aDics.getLength();
        for ( sal_uInt16 i = 0; i < nDicCount; i++ )
        {
            Reference< XDictionary1 >  xDic( pDic[i], UNO_QUERY );
            if (xDic.is())
            {
                sal_uInt16 nActLanguage = xDic->getLanguage();
                if( xDic->isActive() &&
                    xDic->getDictionaryType() == DictionaryType_POSITIVE &&
                    (nLanguage == nActLanguage || LANGUAGE_NONE == nActLanguage ) )
                {
                    pInsertMenu->InsertItem( MN_DICTSTART + i, xDic->getName() );
                }
            }
        }

        if ( !pInsertMenu->GetItemCount() )
            aPopupMenu.EnableItem( MN_INSERT, sal_False );

        aPopupMenu.RemoveDisabledEntries( sal_True, sal_True );

        Point aPos( pImpEditView->GetWindow()->OutputToScreenPixel( rPosPixel ) );
        sal_uInt16 nId = aPopupMenu.Execute( pImpEditView->GetWindow(), aPos );
        if ( nId == MN_IGNORE )
        {
            String aWord = pImpEditView->SpellIgnoreOrAddWord( sal_False );
            if ( pCallBack )
            {
                SpellCallbackInfo aInf( SPELLCMD_IGNOREWORD, aWord );
                pCallBack->Call( &aInf );
            }
        }
        else if ( nId == MN_SPELLING )
        {
            if ( !pCallBack )
            {
                // Cursor vor das Wort setzen...
                EditPaM aCursor = pImpEditView->GetEditSelection().Min();
                pImpEditView->DrawSelection();  // alte Selektion 'weg-zeichnen'
                pImpEditView->SetEditSelection( EditSelection( aCursor, aCursor ) );
                pImpEditView->DrawSelection();
                // Stuerzt ab, wenn keine SfxApp
                PIMPEE->Spell( this, sal_False );
            }
            else
            {
                SpellCallbackInfo aInf( SPELLCMD_STARTSPELLDLG, String() );
                pCallBack->Call( &aInf );
            }
        }
        else if ( nId >= MN_DICTSTART )
        {
            Reference< XDictionary1 >  xDic( pDic[nId - MN_DICTSTART], UNO_QUERY );
            if (xDic.is())
                xDic->add( aSelected, sal_False, String() );
            aPaM.GetNode()->GetWrongList()->GetInvalidStart() = 0;
            PIMPEE->StartOnlineSpellTimer();
        }
        else if ( nId >= MN_AUTOSTART )
        {
            DBG_ASSERT(nId - MN_AUTOSTART < aAlt.getLength(), "index out of range");
            String aWord = pAlt[nId - MN_AUTOSTART];
            SvxAutoCorrect* pAutoCorrect = EE_DLL()->GetGlobalData()->GetAutoCorrect();
            if ( pAutoCorrect )
                pAutoCorrect->PutText( aSelected, aWord, PIMPEE->GetLanguage() );
            InsertText( aWord );
        }
        else if ( nId >= MN_ALTSTART )  // Replace
        {
            DBG_ASSERT(nId - MN_ALTSTART < aAlt.getLength(), "index out of range");
            String aWord = pAlt[nId - MN_ALTSTART];
            InsertText( aWord );
        }
    }
#endif
}

void EditView::SpellIgnoreWord()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    pImpEditView->SpellIgnoreOrAddWord( sal_False );
}

sal_Bool EditView::SelectCurrentWord()
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    EditSelection aCurSel( pImpEditView->GetEditSelection() );
    pImpEditView->DrawSelection();
    aCurSel = PIMPEE->SelectWord( aCurSel.Max() );
    pImpEditView->SetEditSelection( aCurSel );
    pImpEditView->DrawSelection();
    ShowCursor( sal_True, sal_False );
    return aCurSel.HasRange() ? sal_True : sal_False;
}

void EditView::InsertField( const SvxFieldItem& rFld )
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    ImpEditEngine* pImpEE = PIMPEE;
    pImpEditView->DrawSelection();
    pImpEE->UndoActionStart( EDITUNDO_INSERT );
    EditPaM aPaM( pImpEE->InsertField( pImpEditView->GetEditSelection(), rFld ) );
    pImpEE->UndoActionEnd( EDITUNDO_INSERT );
    pImpEditView->SetEditSelection( EditSelection( aPaM, aPaM ) );
    pImpEE->UpdateFields();
    pImpEE->FormatAndUpdate( this );
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer() const
{
    DBG_CHKTHIS( EditView, 0 );
    sal_uInt16 nPara, nPos;
    return GetFieldUnderMousePointer( nPara, nPos );
}

const SvxFieldItem* EditView::GetField( const Point& rPos, sal_uInt16* pPara, sal_uInt16* pPos ) const
{
    if( !GetOutputArea().IsInside( rPos ) )
        return 0;

    Point aDocPos( pImpEditView->GetDocPos( rPos ) );
    EditPaM aPaM = pImpEditView->pEditEngine->pImpEditEngine->GetPaM( aDocPos, sal_False );

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
                    *pPara = PIMPEE->GetEditDoc().GetPos( aPaM.GetNode() );
                if ( pPos )
                    *pPos = pAttr->GetStart();
                return (const SvxFieldItem*)pAttr->GetItem();
            }
    }
    return 0;
}

const SvxFieldItem* EditView::GetFieldUnderMousePointer( sal_uInt16& nPara, sal_uInt16& nPos ) const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
    Point aPos = pImpEditView->GetWindow()->GetPointerPosPixel();
    aPos = pImpEditView->GetWindow()->PixelToLogic( aPos );
    return GetField( aPos, &nPara, &nPos );
}

const SvxFieldItem* EditView::GetFieldAtSelection() const
{
    EditSelection aSel( pImpEditView->GetEditSelection() );
    aSel.Adjust( pImpEditView->pEditEngine->pImpEditEngine->GetEditDoc() );
    // Nur wenn Cursor vor Feld, keine Selektion, oder nur Feld selektiert
    if ( ( aSel.Min().GetNode() == aSel.Max().GetNode() ) &&
         ( ( aSel.Max().GetIndex() == aSel.Min().GetIndex() ) ||
           ( aSel.Max().GetIndex() == aSel.Min().GetIndex()+1 ) ) )
    {
        EditPaM aPaM = aSel.Min();
        const CharAttribArray& rAttrs = aPaM.GetNode()->GetCharAttribs().GetAttribs();
        sal_uInt16 nXPos = aPaM.GetIndex();
        for ( sal_uInt16 nAttr = rAttrs.Count(); nAttr; )
        {
            EditCharAttrib* pAttr = rAttrs[--nAttr];
            if ( pAttr->GetStart() == nXPos )
                if ( pAttr->Which() == EE_FEATURE_FIELD )
                {
                    DBG_ASSERT( pAttr->GetItem()->ISA( SvxFieldItem ), "Kein FeldItem..." );
                    return (const SvxFieldItem*)pAttr->GetItem();
                }
        }
    }
    return 0;
}

XubString EditView::GetWordUnderMousePointer() const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    Rectangle aRect;
    return GetWordUnderMousePointer( aRect );
}

XubString EditView::GetWordUnderMousePointer( Rectangle& rWordRect ) const
{
    DBG_CHKTHIS( EditView, 0 );
    DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );

    Point aPos = pImpEditView->GetWindow()->GetPointerPosPixel();
    aPos = pImpEditView->GetWindow()->PixelToLogic( aPos );

    XubString aWord;

    if( GetOutputArea().IsInside( aPos ) )
    {
        ImpEditEngine* pImpEE = pImpEditView->pEditEngine->pImpEditEngine;
        Point aDocPos( pImpEditView->GetDocPos( aPos ) );
        EditPaM aPaM = pImpEE->GetPaM( aDocPos, sal_False );
        EditSelection aWordSel = pImpEE->SelectWord( aPaM );

        Rectangle aTopLeftRec( pImpEE->PaMtoEditCursor( aWordSel.Min() ) );
        Rectangle aBottomRightRec( pImpEE->PaMtoEditCursor( aWordSel.Max() ) );

#ifdef DEBUG
        DBG_ASSERT( aTopLeftRec.Top() == aBottomRightRec.Top(), "Top() in einer Zeile unterschiedlich?" );
#endif

        Point aPnt1( pImpEditView->GetWindowPos( aTopLeftRec.TopLeft() ) );
        Point aPnt2( pImpEditView->GetWindowPos( aBottomRightRec.BottomRight()) );
        rWordRect = Rectangle( aPnt1, aPnt2 );
        aWord = pImpEE->GetSelected( aWordSel );
    }

    return aWord;
}

void EditView::SetInvalidateMore( sal_uInt16 nPixel )
{
    DBG_CHKTHIS( EditView, 0 );
    pImpEditView->SetInvalidateMore( nPixel );
}

sal_uInt16 EditView::GetInvalidateMore() const
{
    DBG_CHKTHIS( EditView, 0 );
    return (sal_uInt16)pImpEditView->GetInvalidateMore();
}


