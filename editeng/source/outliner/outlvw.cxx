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

#include <com/sun/star/i18n/WordType.hpp>

#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>

#include <svl/style.hxx>
#include <svl/languageoptions.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <editeng/outliner.hxx>
#include <outleeng.hxx>
#include <paralist.hxx>
#include <outlundo.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/numitem.hxx>
#include <vcl/window.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <editeng/editstat.hxx>

using namespace ::com::sun::star;


OutlinerView::OutlinerView( Outliner* pOut, vcl::Window* pWin )
{
    pOwner                      = pOut;

    pEditView = new EditView( pOut->pEditEngine, pWin );
    pEditView->SetSelectionMode( EE_SELMODE_TXTONLY );
}

OutlinerView::~OutlinerView()
{
    delete pEditView;
}

void OutlinerView::Paint( const Rectangle& rRect, OutputDevice* pTargetDevice )
{
    // For the first Paint/KeyInput/Drop an empty Outliner is turned into
    // an Outliner with exactly one paragraph.
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( OUString() );

    pEditView->Paint( rRect, pTargetDevice );
}

bool OutlinerView::PostKeyEvent( const KeyEvent& rKEvt, vcl::Window* pFrameWin )
{
    // For the first Paint/KeyInput/Drop an empty Outliner is turned into
    // an Outliner with exactly one paragraph.
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( OUString() );

    bool bKeyProcessed = false;
    ESelection aSel( pEditView->GetSelection() );
    bool bSelection = aSel.HasRange();
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    KeyFuncType eFunc = aKeyCode.GetFunction();
    sal_uInt16 nCode = aKeyCode.GetCode();
    bool bReadOnly = IsReadOnly();

    if( bSelection && ( nCode != KEY_TAB ) && EditEngine::DoesKeyChangeText( rKEvt ) )
    {
        if ( ImpCalcSelectedPages( false ) && !pOwner->ImpCanDeleteSelectedPages( this ) )
            return true;
    }

    if ( eFunc != KeyFuncType::DONTKNOW )
    {
        switch ( eFunc )
        {
            case KeyFuncType::CUT:
            {
                if ( !bReadOnly )
                {
                    Cut();
                    bKeyProcessed = true;
                }
            }
            break;
            case KeyFuncType::COPY:
            {
                Copy();
                bKeyProcessed = true;
            }
            break;
            case KeyFuncType::PASTE:
            {
                if ( !bReadOnly )
                {
                    PasteSpecial();
                    bKeyProcessed = true;
                }
            }
            break;
            case KeyFuncType::DELETE:
            {
                if( !bReadOnly && !bSelection && ( pOwner->ImplGetOutlinerMode() != OutlinerMode::TextObject ) )
                {
                    if( aSel.nEndPos == pOwner->pEditEngine->GetTextLen( aSel.nEndPara ) )
                    {
                        Paragraph* pNext = pOwner->pParaList->GetParagraph( aSel.nEndPara+1 );
                        if( pNext && pNext->HasFlag(ParaFlag::ISPAGE) )
                        {
                            if( !pOwner->ImpCanDeleteSelectedPages( this, aSel.nEndPara, 1 ) )
                                return false;
                        }
                    }
                }
            }
            break;
            default:    // is then possibly edited below.
                        eFunc = KeyFuncType::DONTKNOW;
        }
    }
    if ( eFunc == KeyFuncType::DONTKNOW )
    {
        switch ( nCode )
        {
            case KEY_TAB:
            {
                if ( !bReadOnly && !aKeyCode.IsMod1() && !aKeyCode.IsMod2() )
                {
                    if ( ( pOwner->ImplGetOutlinerMode() != OutlinerMode::TextObject ) &&
                         ( pOwner->ImplGetOutlinerMode() != OutlinerMode::TitleObject ) &&
                         ( bSelection || !aSel.nStartPos ) )
                    {
                        Indent( aKeyCode.IsShift() ? (-1) : (+1) );
                        bKeyProcessed = true;
                    }
                    else if ( ( pOwner->ImplGetOutlinerMode() == OutlinerMode::TextObject ) &&
                              !bSelection && !aSel.nEndPos && pOwner->ImplHasNumberFormat( aSel.nEndPara ) )
                    {
                        Indent( aKeyCode.IsShift() ? (-1) : (+1) );
                        bKeyProcessed = true;
                    }
                }
            }
            break;
            case KEY_BACKSPACE:
            {
                if( !bReadOnly && !bSelection && aSel.nEndPara && !aSel.nEndPos )
                {
                    Paragraph* pPara = pOwner->pParaList->GetParagraph( aSel.nEndPara );
                    Paragraph* pPrev = pOwner->pParaList->GetParagraph( aSel.nEndPara-1 );
                    if( !pPrev->IsVisible()  )
                        return true;
                    if( !pPara->GetDepth() )
                    {
                        if(!pOwner->ImpCanDeleteSelectedPages(this, aSel.nEndPara , 1 ) )
                            return true;
                    }
                }
            }
            break;
            case KEY_RETURN:
            {
                if ( !bReadOnly )
                {
                    // Special treatment: hard return at the end of a paragraph,
                    // which has collapsed subparagraphs.
                    Paragraph* pPara = pOwner->pParaList->GetParagraph( aSel.nEndPara );

                    if( !aKeyCode.IsShift() )
                    {
                        // ImpGetCursor again???
                        if( !bSelection &&
                                aSel.nEndPos == pOwner->pEditEngine->GetTextLen( aSel.nEndPara ) )
                        {
                            sal_Int32 nChildren = pOwner->pParaList->GetChildCount(pPara);
                            if( nChildren && !pOwner->pParaList->HasVisibleChildren(pPara))
                            {
                                pOwner->UndoActionStart( OLUNDO_INSERT );
                                sal_Int32 nTemp = aSel.nEndPara;
                                nTemp += nChildren;
                                nTemp++; // insert above next Non-Child
                                SAL_WARN_IF( nTemp < 0, "editeng", "OutlinerView::PostKeyEvent - overflow");
                                if (nTemp >= 0)
                                {
                                    pOwner->Insert( OUString(),nTemp,pPara->GetDepth());
                                    // Position the cursor
                                    ESelection aTmpSel(nTemp,0,nTemp,0);
                                    pEditView->SetSelection( aTmpSel );
                                }
                                pEditView->ShowCursor();
                                pOwner->UndoActionEnd( OLUNDO_INSERT );
                                bKeyProcessed = true;
                            }
                        }
                    }
                    if( !bKeyProcessed && !bSelection &&
                                !aKeyCode.IsShift() && aKeyCode.IsMod1() &&
                            ( aSel.nEndPos == pOwner->pEditEngine->GetTextLen(aSel.nEndPara) ) )
                    {
                        pOwner->UndoActionStart( OLUNDO_INSERT );
                        sal_Int32 nTemp = aSel.nEndPara;
                        nTemp++;
                        pOwner->Insert( OUString(), nTemp, pPara->GetDepth()+1 );

                        // Position the cursor
                        ESelection aTmpSel(nTemp,0,nTemp,0);
                        pEditView->SetSelection( aTmpSel );
                        pEditView->ShowCursor();
                        pOwner->UndoActionEnd( OLUNDO_INSERT );
                        bKeyProcessed = true;
                    }
                }
            }
            break;
        }
    }

    return bKeyProcessed || pEditView->PostKeyEvent( rKEvt, pFrameWin );
}

sal_Int32 OutlinerView::ImpCheckMousePos(const Point& rPosPix, MouseTarget& reTarget)
{
    sal_Int32 nPara = EE_PARA_NOT_FOUND;

    Point aMousePosWin = pEditView->GetWindow()->PixelToLogic( rPosPix );
    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
    {
        reTarget = MouseTarget::Outside;
    }
    else
    {
        reTarget = MouseTarget::Text;

        Point aPaperPos( aMousePosWin );
        Rectangle aOutArea = pEditView->GetOutputArea();
        Rectangle aVisArea = pEditView->GetVisArea();
        aPaperPos.X() -= aOutArea.Left();
        aPaperPos.X() += aVisArea.Left();
        aPaperPos.Y() -= aOutArea.Top();
        aPaperPos.Y() += aVisArea.Top();

        bool bBullet;
        if ( pOwner->IsTextPos( aPaperPos, 0, &bBullet ) )
        {
            Point aDocPos = pOwner->GetDocPos( aPaperPos );
            nPara = pOwner->pEditEngine->FindParagraph( aDocPos.Y() );

            if ( bBullet )
            {
                reTarget = MouseTarget::Bullet;
            }
            else
            {
                // Check for hyperlink
                const SvxFieldItem* pFieldItem = pEditView->GetField( aMousePosWin );
                if ( pFieldItem && pFieldItem->GetField() && dynamic_cast< const SvxURLField* >(pFieldItem->GetField()) != nullptr )
                    reTarget = MouseTarget::Hypertext;
            }
        }
    }
    return nPara;
}

bool OutlinerView::MouseMove( const MouseEvent& rMEvt )
{
    if( ( pOwner->ImplGetOutlinerMode() == OutlinerMode::TextObject ) || pEditView->GetEditEngine()->IsInSelectionMode())
        return pEditView->MouseMove( rMEvt );

    Point aMousePosWin( pEditView->GetWindow()->PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
        return false;

    Pointer aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );
    return pEditView->MouseMove( rMEvt );
}


bool OutlinerView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( ( pOwner->ImplGetOutlinerMode() == OutlinerMode::TextObject ) || pEditView->GetEditEngine()->IsInSelectionMode() )
        return pEditView->MouseButtonDown( rMEvt );

    Point aMousePosWin( pEditView->GetWindow()->PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
        return false;

    Pointer aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );

    MouseTarget eTarget;
    sal_Int32 nPara = ImpCheckMousePos( rMEvt.GetPosPixel(), eTarget );
    if ( eTarget == MouseTarget::Bullet )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        bool bHasChildren = (pPara && pOwner->pParaList->HasChildren(pPara));
        if( rMEvt.GetClicks() == 1 )
        {
            sal_Int32 nEndPara = nPara;
            if ( bHasChildren && pOwner->pParaList->HasVisibleChildren(pPara) )
                nEndPara += pOwner->pParaList->GetChildCount( pPara );
            // The selection is inverted, so that EditEngine does not scroll
            ESelection aSel(nEndPara, EE_TEXTPOS_ALL, nPara, 0 );
            pEditView->SetSelection( aSel );
        }
        else if( rMEvt.GetClicks() == 2 && bHasChildren )
            ImpToggleExpand( pPara );

        return true;
    }

    // special case for outliner view in impress, check if double click hits the page icon for toggle
    if( (nPara == EE_PARA_NOT_FOUND) && (pOwner->ImplGetOutlinerMode() == OutlinerMode::OutlineView) && (eTarget == MouseTarget::Text) && (rMEvt.GetClicks() == 2) )
    {
        ESelection aSel( pEditView->GetSelection() );
        nPara = aSel.nStartPara;
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        if( (pPara && pOwner->pParaList->HasChildren(pPara)) && pPara->HasFlag(ParaFlag::ISPAGE) )
        {
            ImpToggleExpand( pPara );
        }
    }
    return pEditView->MouseButtonDown( rMEvt );
}


bool OutlinerView::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( ( pOwner->ImplGetOutlinerMode() == OutlinerMode::TextObject ) || pEditView->GetEditEngine()->IsInSelectionMode() )
        return pEditView->MouseButtonUp( rMEvt );

    Point aMousePosWin( pEditView->GetWindow()->PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
        return false;

    Pointer aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );

    return pEditView->MouseButtonUp( rMEvt );
}

void OutlinerView::ReleaseMouse()
{
    pEditView->ReleaseMouse();
}

void OutlinerView::ImpToggleExpand( Paragraph* pPara )
{
    sal_Int32 nPara = pOwner->pParaList->GetAbsPos( pPara );
    pEditView->SetSelection( ESelection( nPara, 0, nPara, 0 ) );
    ImplExpandOrCollaps( nPara, nPara, !pOwner->pParaList->HasVisibleChildren( pPara ) );
    pEditView->ShowCursor();
}

sal_Int32 OutlinerView::Select( Paragraph* pParagraph, bool bSelect )
{
    sal_Int32 nPara = pOwner->pParaList->GetAbsPos( pParagraph );
    sal_Int32 nEnd = 0;
    if ( bSelect )
        nEnd = SAL_MAX_INT32;

    ESelection aSel( nPara, 0, nPara, nEnd );
    pEditView->SetSelection( aSel );
    return 1;
}


void OutlinerView::SetAttribs( const SfxItemSet& rAttrs )
{
    bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( false );

    if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
        pOwner->UndoActionStart( OLUNDO_ATTR );

    ParaRange aSel = ImpGetSelectedParagraphs( false );

    pEditView->SetAttribs( rAttrs );

    // Update Bullet text
    for( sal_Int32 nPara= aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        pOwner->ImplCheckNumBulletItem( nPara );
        pOwner->ImplCalcBulletText( nPara, false, false );

        if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
            pOwner->InsertUndo( new OutlinerUndoCheckPara( pOwner, nPara ) );
    }

    if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
        pOwner->UndoActionEnd( OLUNDO_ATTR );

    pEditView->SetEditEngineUpdateMode( bUpdate );
}

ParaRange OutlinerView::ImpGetSelectedParagraphs( bool bIncludeHiddenChildren )
{
    ESelection aSel = pEditView->GetSelection();
    ParaRange aParas( aSel.nStartPara, aSel.nEndPara );
    aParas.Adjust();

    // Record the  invisible Children of the last Parents in the selection
    if ( bIncludeHiddenChildren )
    {
        Paragraph* pLast = pOwner->pParaList->GetParagraph( aParas.nEndPara );
        if ( pOwner->pParaList->HasHiddenChildren( pLast ) )
            aParas.nEndPara = aParas.nEndPara + pOwner->pParaList->GetChildCount( pLast );
    }
    return aParas;
}

// TODO: Name should be changed!
void OutlinerView::AdjustDepth( short nDX )
{
    Indent( nDX );
}

void OutlinerView::Indent( short nDiff )
{
    if( !nDiff || ( ( nDiff > 0 ) && ImpCalcSelectedPages( true ) && !pOwner->ImpCanIndentSelectedPages( this ) ) )
        return;

    const bool bOutlinerView = bool(pOwner->pEditEngine->GetControlWord() & EEControlBits::OUTLINER);
    bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( false );

    bool bUndo = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();

    if( bUndo )
        pOwner->UndoActionStart( OLUNDO_DEPTH );

    sal_Int16 nMinDepth = -1;   // Optimization: avoid recalculate too many paragraphs if not really needed.

    ParaRange aSel = ImpGetSelectedParagraphs( true );
    for ( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );

        sal_Int16 nOldDepth = pPara->GetDepth();
        sal_Int16 nNewDepth = nOldDepth + nDiff;

        if( bOutlinerView && nPara )
        {
            const bool bPage = pPara->HasFlag(ParaFlag::ISPAGE);
            if( (bPage && (nDiff == +1)) || (!bPage && (nDiff == -1) && (nOldDepth <= 0))  )
            {
                            // Notify App
                pOwner->nDepthChangedHdlPrevDepth = nOldDepth;
                pOwner->mnDepthChangeHdlPrevFlags = pPara->nFlags;
                pOwner->pHdlParagraph = pPara;

                if( bPage )
                    pPara->RemoveFlag( ParaFlag::ISPAGE );
                else
                    pPara->SetFlag( ParaFlag::ISPAGE );

                pOwner->DepthChangedHdl();
                pOwner->pEditEngine->QuickMarkInvalid( ESelection( nPara, 0, nPara, 0 ) );

                if( bUndo )
                    pOwner->InsertUndo( new OutlinerUndoChangeParaFlags( pOwner, nPara, pOwner->mnDepthChangeHdlPrevFlags, pPara->nFlags ) );

                continue;
            }
        }

        // do not switch off numeration with tab
        if( (nOldDepth == 0) && (nNewDepth == -1) )
            continue;

        // do not indent if there is no numeration enabled
        if( nOldDepth == -1 )
            continue;

        if ( nNewDepth < pOwner->nMinDepth )
            nNewDepth = pOwner->nMinDepth;
        if ( nNewDepth > pOwner->nMaxDepth )
            nNewDepth = pOwner->nMaxDepth;

        if( nOldDepth < nMinDepth )
            nMinDepth = nOldDepth;
        if( nNewDepth < nMinDepth )
            nMinDepth = nNewDepth;

        if( nOldDepth != nNewDepth )
        {
            if ( ( nPara == aSel.nStartPara ) && aSel.nStartPara && ( pOwner->ImplGetOutlinerMode() != OutlinerMode::TextObject ))
            {
                // Special case: the predecessor of an indented paragraph is
                // invisible and is now on the same level as the visible
                // paragraph. In this case, the next visible paragraph is
                // searched for and fluffed.
#ifdef DBG_UTIL
                Paragraph* _pPara = pOwner->pParaList->GetParagraph( aSel.nStartPara );
                DBG_ASSERT(_pPara->IsVisible(),"Selected Paragraph invisible ?!");
#endif
                Paragraph* pPrev= pOwner->pParaList->GetParagraph( aSel.nStartPara-1 );

                if( !pPrev->IsVisible() && ( pPrev->GetDepth() == nNewDepth ) )
                {
                    // Predecessor is collapsed and is on the same level
                    // => find next visible paragraph and expand it
                    pPrev = pOwner->pParaList->GetParent( pPrev );
                    while( !pPrev->IsVisible() )
                        pPrev = pOwner->pParaList->GetParent( pPrev );

                    pOwner->Expand( pPrev );
                    pOwner->InvalidateBullet(pOwner->pParaList->GetAbsPos(pPrev));
                }
            }

            pOwner->nDepthChangedHdlPrevDepth = nOldDepth;
            pOwner->mnDepthChangeHdlPrevFlags = pPara->nFlags;
            pOwner->pHdlParagraph = pPara;

            pOwner->ImplInitDepth( nPara, nNewDepth, true );
            pOwner->ImplCalcBulletText( nPara, false, false );

            if ( pOwner->ImplGetOutlinerMode() == OutlinerMode::OutlineObject )
                pOwner->ImplSetLevelDependendStyleSheet( nPara );

            // Notify App
            pOwner->DepthChangedHdl();
        }
        else
        {
            // Needs at least a repaint...
            pOwner->pEditEngine->QuickMarkInvalid( ESelection( nPara, 0, nPara, 0 ) );
        }
    }

    sal_Int32 nParas = pOwner->pParaList->GetParagraphCount();
    for ( sal_Int32 n = aSel.nEndPara+1; n < nParas; n++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( n );
        if ( pPara->GetDepth() < nMinDepth )
            break;
        pOwner->ImplCalcBulletText( n, false, false );
    }

    if ( bUpdate )
    {
        pEditView->SetEditEngineUpdateMode( true );
        pEditView->ShowCursor();
    }

    if( bUndo )
        pOwner->UndoActionEnd( OLUNDO_DEPTH );
}

void OutlinerView::AdjustHeight( long nDY )
{
    pEditView->MoveParagraphs( nDY );
}

Rectangle OutlinerView::GetVisArea() const
{
    return pEditView->GetVisArea();
}

void OutlinerView::Expand()
{
    ParaRange aParas = ImpGetSelectedParagraphs( false );
    ImplExpandOrCollaps( aParas.nStartPara, aParas.nEndPara, true );
}


void OutlinerView::Collapse()
{
    ParaRange aParas = ImpGetSelectedParagraphs( false );
    ImplExpandOrCollaps( aParas.nStartPara, aParas.nEndPara, false );
}


void OutlinerView::ExpandAll()
{
    ImplExpandOrCollaps( 0, pOwner->pParaList->GetParagraphCount()-1, true );
}


void OutlinerView::CollapseAll()
{
    ImplExpandOrCollaps( 0, pOwner->pParaList->GetParagraphCount()-1, false );
}

void OutlinerView::ImplExpandOrCollaps( sal_Int32 nStartPara, sal_Int32 nEndPara, bool bExpand )
{
    bool bUpdate = pOwner->GetUpdateMode();
    pOwner->SetUpdateMode( false );

    bool bUndo = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();
    if( bUndo )
        pOwner->UndoActionStart( bExpand ? OLUNDO_EXPAND : OLUNDO_COLLAPSE );

    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        bool bDone = bExpand ? pOwner->Expand( pPara ) : pOwner->Collapse( pPara );
        if( bDone )
        {
            // The line under the paragraph should disappear ...
            pOwner->pEditEngine->QuickMarkToBeRepainted( nPara );
        }
    }

    if( bUndo )
        pOwner->UndoActionEnd( bExpand ? OLUNDO_EXPAND : OLUNDO_COLLAPSE );

    if ( bUpdate )
    {
        pOwner->SetUpdateMode( true );
        pEditView->ShowCursor();
    }
}

void OutlinerView::InsertText( const OutlinerParaObject& rParaObj )
{
    // Like Paste, only EditView::Insert, instead of EditView::Paste.
    // Actually not quite true that possible indentations must be corrected,
    // but that comes later by a universal import. The indentation level is
    // then determined right in the Inserted method.
    // Possible structure:
    // pImportInfo with DestPara, DestPos, nFormat, pParaObj...
    // Possibly problematic:
    // EditEngine, RTF => Splitting the area, later join together.

    if ( ImpCalcSelectedPages( false ) && !pOwner->ImpCanDeleteSelectedPages( this ) )
        return;

    pOwner->UndoActionStart( OLUNDO_INSERT );

    pOwner->pEditEngine->SetUpdateMode( false );
    sal_Int32 nStart, nParaCount;
    nParaCount = pOwner->pEditEngine->GetParagraphCount();
    sal_uInt16 nSize = ImpInitPaste( nStart );
    pEditView->InsertText( rParaObj.GetTextObject() );
    ImpPasted( nStart, nParaCount, nSize);
    pEditView->SetEditEngineUpdateMode( true );

    pOwner->UndoActionEnd( OLUNDO_INSERT );

    pEditView->ShowCursor();
}


void OutlinerView::Cut()
{
    if ( !ImpCalcSelectedPages( false ) || pOwner->ImpCanDeleteSelectedPages( this ) ) {
        pEditView->Cut();
        // Chaining handling
        aEndCutPasteLink.Call(nullptr);
    }
}

void OutlinerView::Paste()
{
    PasteSpecial(); // HACK(SD does not call PasteSpecial)
}

void OutlinerView::PasteSpecial()
{
    if ( !ImpCalcSelectedPages( false ) || pOwner->ImpCanDeleteSelectedPages( this ) )
    {
        pOwner->UndoActionStart( OLUNDO_INSERT );

        pOwner->pEditEngine->SetUpdateMode( false );
        pOwner->bPasting = true;
        pEditView->PasteSpecial();

        if ( pOwner->ImplGetOutlinerMode() == OutlinerMode::OutlineObject )
        {
            const sal_Int32 nParaCount = pOwner->pEditEngine->GetParagraphCount();

            for( sal_Int32 nPara = 0; nPara < nParaCount; nPara++ )
                pOwner->ImplSetLevelDependendStyleSheet( nPara );
        }

        pEditView->SetEditEngineUpdateMode( true );
        pOwner->UndoActionEnd( OLUNDO_INSERT );
        pEditView->ShowCursor();

        // Chaining handling
        // NOTE: We need to do this last because it pEditView may be deleted if a switch of box occurs
        aEndCutPasteLink.Call(nullptr);
    }
}

void OutlinerView::CreateSelectionList (std::vector<Paragraph*> &aSelList)
{
    ParaRange aParas = ImpGetSelectedParagraphs( true );

    for ( sal_Int32 nPara = aParas.nStartPara; nPara <= aParas.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        aSelList.push_back(pPara);
    }
}

const SfxStyleSheet* OutlinerView::GetStyleSheet() const
{
    return pEditView->GetStyleSheet();
}

SfxStyleSheet* OutlinerView::GetStyleSheet()
{
    return pEditView->GetStyleSheet();
}

Pointer OutlinerView::GetPointer( const Point& rPosPixel )
{
    MouseTarget eTarget;
    ImpCheckMousePos( rPosPixel, eTarget );

    PointerStyle ePointerStyle = PointerStyle::Arrow;
    if ( eTarget == MouseTarget::Text )
    {
        ePointerStyle = GetOutliner()->IsVertical() ? PointerStyle::TextVertical : PointerStyle::Text;
    }
    else if ( eTarget == MouseTarget::Hypertext )
    {
        ePointerStyle = PointerStyle::RefHand;
    }
    else if ( eTarget == MouseTarget::Bullet )
    {
        ePointerStyle = PointerStyle::Move;
    }

    return Pointer( ePointerStyle );
}


sal_Int32 OutlinerView::ImpInitPaste( sal_Int32& rStart )
{
    pOwner->bPasting = true;
    ESelection aSelection( pEditView->GetSelection() );
    aSelection.Adjust();
    rStart = aSelection.nStartPara;
    sal_Int32 nSize = aSelection.nEndPara - aSelection.nStartPara + 1;
    return nSize;
}


void OutlinerView::ImpPasted( sal_Int32 nStart, sal_Int32 nPrevParaCount, sal_Int32 nSize)
{
    pOwner->bPasting = false;
    sal_Int32 nCurParaCount = pOwner->pEditEngine->GetParagraphCount();
    if( nCurParaCount < nPrevParaCount )
        nSize = nSize - ( nPrevParaCount - nCurParaCount );
    else
        nSize = nSize + ( nCurParaCount - nPrevParaCount );
    pOwner->ImpTextPasted( nStart, nSize );
}


void OutlinerView::Command( const CommandEvent& rCEvt )
{
    pEditView->Command( rCEvt );
}


void OutlinerView::SelectRange( sal_Int32 nFirst, sal_Int32 nCount )
{
    sal_Int32 nLast = nFirst+nCount;
    nCount = pOwner->pParaList->GetParagraphCount();
    if( nLast <= nCount )
        nLast = nCount - 1;
    ESelection aSel( nFirst, 0, nLast, EE_TEXTPOS_ALL );
    pEditView->SetSelection( aSel );
}


sal_Int32 OutlinerView::ImpCalcSelectedPages( bool bIncludeFirstSelected )
{
    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    sal_Int32 nPages = 0;
    sal_Int32 nFirstPage = EE_PARA_MAX_COUNT;
    sal_Int32 nStartPara = aSel.nStartPara;
    if ( !bIncludeFirstSelected )
        nStartPara++;   // All paragraphs after StartPara will be deleted
    for ( sal_Int32 nPara = nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "ImpCalcSelectedPages: invalid Selection? ");
        if( pPara->HasFlag(ParaFlag::ISPAGE) )
        {
            nPages++;
            if( nFirstPage == EE_PARA_MAX_COUNT )
                nFirstPage = nPara;
        }
    }

    if( nPages )
    {
        pOwner->nDepthChangedHdlPrevDepth = nPages;
        pOwner->pHdlParagraph = nullptr;
        pOwner->mnFirstSelPage = nFirstPage;
    }

    return nPages;
}


void OutlinerView::ToggleBullets()
{
    pOwner->UndoActionStart( OLUNDO_DEPTH );

    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    const bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( false );

    sal_Int16 nNewDepth = -2;
    const SvxNumRule* pDefaultBulletNumRule = nullptr;

    for ( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::ToggleBullets(), illegal selection?");

        if( pPara )
        {
            if( nNewDepth == -2 )
            {
                nNewDepth = (pOwner->GetDepth(nPara) == -1) ? 0 : -1;
                if ( nNewDepth == 0 )
                {
                    // determine default numbering rule for bullets
                    const ESelection aSelection(nPara, 0);
                    const SfxItemSet aTmpSet(pOwner->pEditEngine->GetAttribs(aSelection));
                    const SfxPoolItem& rPoolItem = aTmpSet.GetPool()->GetDefaultItem( EE_PARA_NUMBULLET );
                    const SvxNumBulletItem* pNumBulletItem = dynamic_cast< const SvxNumBulletItem* >(&rPoolItem);
                    pDefaultBulletNumRule =  pNumBulletItem ? pNumBulletItem->GetNumRule() : nullptr;
                }
            }

            pOwner->SetDepth( pPara, nNewDepth );

            if( nNewDepth == -1 )
            {
                const SfxItemSet& rAttrs = pOwner->GetParaAttribs( nPara );
                if ( rAttrs.GetItemState( EE_PARA_BULLETSTATE ) == SfxItemState::SET )
                {
                    SfxItemSet aAttrs(rAttrs);
                    aAttrs.ClearItem( EE_PARA_BULLETSTATE );
                    pOwner->SetParaAttribs( nPara, aAttrs );
                }
            }
            else
            {
                if ( pDefaultBulletNumRule )
                {
                    const SvxNumberFormat* pFmt = pOwner ->GetNumberFormat( nPara );
                    if ( !pFmt
                         || ( pFmt->GetNumberingType() != SVX_NUM_BITMAP
                              && pFmt->GetNumberingType() != SVX_NUM_CHAR_SPECIAL ) )
                    {
                        SfxItemSet aAttrs( pOwner->GetParaAttribs( nPara ) );
                        SvxNumRule aNewNumRule( *pDefaultBulletNumRule );
                        aAttrs.Put( SvxNumBulletItem( aNewNumRule, EE_PARA_NUMBULLET ) );
                        pOwner->SetParaAttribs( nPara, aAttrs );
                    }
                }
            }
        }
    }

    const sal_Int32 nParaCount = pOwner->pParaList->GetParagraphCount();
    pOwner->ImplCheckParagraphs( aSel.nStartPara, nParaCount );

    sal_Int32 nEndPara = (nParaCount > 0) ? nParaCount-1 : nParaCount;
    pOwner->pEditEngine->QuickMarkInvalid( ESelection( aSel.nStartPara, 0, nEndPara, 0 ) );

    pOwner->pEditEngine->SetUpdateMode( bUpdate );

    pOwner->UndoActionEnd( OLUNDO_DEPTH );
}


void OutlinerView::ToggleBulletsNumbering(
    const bool bToggle,
    const bool bHandleBullets,
    const SvxNumRule* pNumRule )
{
    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    bool bToggleOn = true;
    if ( bToggle )
    {
        bToggleOn = false;
        const sal_Int16 nBulletNumberingStatus( pOwner->GetBulletsNumberingStatus( aSel.nStartPara, aSel.nEndPara ) );
        if ( nBulletNumberingStatus != 0 && bHandleBullets )
        {
            // not all paragraphs have bullets and method called to toggle bullets --> bullets on
            bToggleOn = true;
        }
        else if ( nBulletNumberingStatus != 1 && !bHandleBullets )
        {
            // not all paragraphs have numbering and method called to toggle numberings --> numberings on
            bToggleOn = true;
        }
    }
    if ( bToggleOn )
    {
        // apply bullets/numbering for selected paragraphs
        ApplyBulletsNumbering( bHandleBullets, pNumRule, bToggle, true );
    }
    else
    {
        // switch off bullets/numbering for selected paragraphs
        SwitchOffBulletsNumbering( true );
    }

    return;
}


void OutlinerView::EnableBullets()
{
    pOwner->UndoActionStart( OLUNDO_DEPTH );

    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    const bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( false );

    for ( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::EnableBullets(), illegal selection?");

        if( pPara && (pOwner->GetDepth(nPara) == -1) )
        {
            pOwner->SetDepth( pPara, 0 );
        }
    }

    sal_Int32 nParaCount = pOwner->pParaList->GetParagraphCount();
    pOwner->ImplCheckParagraphs( aSel.nStartPara, nParaCount );

    sal_Int32 nEndPara = (nParaCount > 0) ? nParaCount-1 : nParaCount;
    pOwner->pEditEngine->QuickMarkInvalid( ESelection( aSel.nStartPara, 0, nEndPara, 0 ) );

    pOwner->pEditEngine->SetUpdateMode( bUpdate );

    pOwner->UndoActionEnd( OLUNDO_DEPTH );
}


void OutlinerView::ApplyBulletsNumbering(
    const bool bHandleBullets,
    const SvxNumRule* pNewNumRule,
    const bool bCheckCurrentNumRuleBeforeApplyingNewNumRule,
    const bool bAtSelection )
{
    if (!pOwner || !pOwner->pEditEngine || !pOwner->pParaList)
    {
        return;
    }

    pOwner->UndoActionStart(OLUNDO_DEPTH);
    const bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode(false);

    sal_Int32 nStartPara = 0;
    sal_Int32 nEndPara = 0;
    if ( bAtSelection )
    {
        ESelection aSel( pEditView->GetSelection() );
        aSel.Adjust();
        nStartPara = aSel.nStartPara;
        nEndPara = aSel.nEndPara;
    }
    else
    {
        nStartPara = 0;
        nEndPara = pOwner->pParaList->GetParagraphCount() - 1;
    }

    for (sal_Int32 nPara = nStartPara; nPara <= nEndPara; ++nPara)
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph(nPara);
        DBG_ASSERT(pPara, "OutlinerView::ApplyBulletsNumbering(..), illegal selection?");

        if (pPara)
        {
            const sal_Int16 nDepth = pOwner->GetDepth(nPara);
            if ( nDepth == -1 )
            {
                pOwner->SetDepth( pPara, 0 );
            }

            const SfxItemSet& rAttrs = pOwner->GetParaAttribs(nPara);
            SfxItemSet aAttrs(rAttrs);
            aAttrs.Put(SfxBoolItem(EE_PARA_BULLETSTATE, true));

            // apply new numbering rule
            if ( pNewNumRule )
            {
                bool bApplyNumRule = false;
                if ( !bCheckCurrentNumRuleBeforeApplyingNewNumRule )
                {
                    bApplyNumRule = true;
                }
                else
                {
                    const SvxNumberFormat* pFmt = pOwner ->GetNumberFormat(nPara);
                    if (!pFmt)
                    {
                        bApplyNumRule = true;
                    }
                    else
                    {
                        sal_Int16 nNumType = pFmt->GetNumberingType();
                        if ( bHandleBullets
                             && nNumType != SVX_NUM_BITMAP && nNumType != SVX_NUM_CHAR_SPECIAL)
                        {
                            // Set to Normal bullet, old bullet type is Numbering bullet.
                            bApplyNumRule = true;
                        }
                        else if ( !bHandleBullets
                                  && (nNumType == SVX_NUM_BITMAP || nNumType == SVX_NUM_CHAR_SPECIAL))
                        {
                            // Set to Numbering bullet, old bullet type is Normal bullet.
                            bApplyNumRule = true;
                        }
                    }
                }

                if ( bApplyNumRule )
                {
                    SvxNumRule aNewRule(*pNewNumRule);

                    // Get old bullet space.
                    {
                        const SfxPoolItem* pPoolItem=nullptr;
                        SfxItemState eState = rAttrs.GetItemState(EE_PARA_NUMBULLET, false, &pPoolItem);
                        if (eState != SfxItemState::SET)
                        {
                            // Use default value when has not contain bullet item.
                            ESelection aSelection(nPara, 0);
                            SfxItemSet aTmpSet(pOwner->pEditEngine->GetAttribs(aSelection));
                            pPoolItem = aTmpSet.GetItem(EE_PARA_NUMBULLET);
                        }

                        const SvxNumBulletItem* pNumBulletItem = dynamic_cast< const SvxNumBulletItem* >(pPoolItem);
                        if (pNumBulletItem)
                        {
                            const sal_uInt16 nLevelCnt = std::min(pNumBulletItem->GetNumRule()->GetLevelCount(), aNewRule.GetLevelCount());
                            for ( sal_uInt16 nLevel = 0; nLevel < nLevelCnt; ++nLevel )
                            {
                                const SvxNumberFormat* pOldFmt = pNumBulletItem->GetNumRule()->Get(nLevel);
                                const SvxNumberFormat* pNewFmt = aNewRule.Get(nLevel);
                                if (pOldFmt && pNewFmt && (pOldFmt->GetFirstLineOffset() != pNewFmt->GetFirstLineOffset() || pOldFmt->GetAbsLSpace() != pNewFmt->GetAbsLSpace()))
                                {
                                    SvxNumberFormat* pNewFmtClone = new SvxNumberFormat(*pNewFmt);
                                    pNewFmtClone->SetFirstLineOffset(pOldFmt->GetFirstLineOffset());
                                    pNewFmtClone->SetAbsLSpace(pOldFmt->GetAbsLSpace());
                                    aNewRule.SetLevel(nLevel, pNewFmtClone);
                                    delete pNewFmtClone;
                                }
                            }
                        }
                    }

                    aAttrs.Put(SvxNumBulletItem(aNewRule, EE_PARA_NUMBULLET));
                }
            }
            pOwner->SetParaAttribs(nPara, aAttrs);
        }
    }

    const sal_uInt16 nParaCount = (sal_uInt16) (pOwner->pParaList->GetParagraphCount());
    pOwner->ImplCheckParagraphs( nStartPara, nParaCount );
    pOwner->pEditEngine->QuickMarkInvalid( ESelection( nStartPara, 0, nParaCount, 0 ) );

    pOwner->pEditEngine->SetUpdateMode( bUpdate );

    pOwner->UndoActionEnd( OLUNDO_DEPTH );

    return;
}


void OutlinerView::SwitchOffBulletsNumbering(
    const bool bAtSelection )
{
    sal_Int32 nStartPara = 0;
    sal_Int32 nEndPara = 0;
    if ( bAtSelection )
    {
        ESelection aSel( pEditView->GetSelection() );
        aSel.Adjust();
        nStartPara = aSel.nStartPara;
        nEndPara = aSel.nEndPara;
    }
    else
    {
        nStartPara = 0;
        nEndPara = pOwner->pParaList->GetParagraphCount() - 1;
    }

    pOwner->UndoActionStart( OLUNDO_DEPTH );
    const bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( false );

    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; ++nPara )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::SwitchOffBulletsNumbering(...), illegal paragraph index?");

        if( pPara )
        {
            pOwner->SetDepth( pPara, -1 );

            const SfxItemSet& rAttrs = pOwner->GetParaAttribs( nPara );
            if (rAttrs.GetItemState( EE_PARA_BULLETSTATE ) == SfxItemState::SET)
            {
                SfxItemSet aAttrs(rAttrs);
                aAttrs.ClearItem( EE_PARA_BULLETSTATE );
                pOwner->SetParaAttribs( nPara, aAttrs );
            }
        }
    }

    const sal_uInt16 nParaCount = (sal_uInt16) (pOwner->pParaList->GetParagraphCount());
    pOwner->ImplCheckParagraphs( nStartPara, nParaCount );
    pOwner->pEditEngine->QuickMarkInvalid( ESelection( nStartPara, 0, nParaCount, 0 ) );

    pOwner->pEditEngine->SetUpdateMode( bUpdate );
    pOwner->UndoActionEnd( OLUNDO_DEPTH );
}


void OutlinerView::RemoveAttribsKeepLanguages( bool bRemoveParaAttribs )
{
    RemoveAttribs( bRemoveParaAttribs, true /*keep language attribs*/ );
}

void OutlinerView::RemoveAttribs( bool bRemoveParaAttribs, bool bKeepLanguages )
{
    bool bUpdate = pOwner->GetUpdateMode();
    pOwner->SetUpdateMode( false );
    pOwner->UndoActionStart( OLUNDO_ATTR );
    if (bKeepLanguages)
        pEditView->RemoveAttribsKeepLanguages( bRemoveParaAttribs );
    else
        pEditView->RemoveAttribs( bRemoveParaAttribs );
    if ( bRemoveParaAttribs )
    {
        // Loop through all paragraphs and set indentation and level
        ESelection aSel = pEditView->GetSelection();
        aSel.Adjust();
        for ( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
        {
            Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
            pOwner->ImplInitDepth( nPara, pPara->GetDepth(), false );
        }
    }
    pOwner->UndoActionEnd( OLUNDO_ATTR );
    pOwner->SetUpdateMode( bUpdate );
}


// ======================   Simple pass-through   =======================


void OutlinerView::InsertText( const OUString& rNew, bool bSelect )
{
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( OUString() );
    pEditView->InsertText( rNew, bSelect );
}

void OutlinerView::SetVisArea( const Rectangle& rRect )
{
    pEditView->SetVisArea( rRect );
}


void OutlinerView::SetSelection( const ESelection& rSel )
{
    pEditView->SetSelection( rSel );
}

void OutlinerView::GetSelectionRectangles(std::vector<Rectangle>& rLogicRects) const
{
    pEditView->GetSelectionRectangles(rLogicRects);
}

void OutlinerView::SetReadOnly( bool bReadOnly )
{
    pEditView->SetReadOnly( bReadOnly );
}

bool OutlinerView::IsReadOnly() const
{
    return pEditView->IsReadOnly();
}

bool OutlinerView::HasSelection() const
{
    return pEditView->HasSelection();
}

void OutlinerView::ShowCursor( bool bGotoCursor, bool bActivate )
{
    pEditView->ShowCursor( bGotoCursor, /*bForceVisCursor=*/true, bActivate );
}

void OutlinerView::HideCursor(bool bDeactivate)
{
    pEditView->HideCursor(bDeactivate);
}

void OutlinerView::SetWindow( vcl::Window* pWin )
{
    pEditView->SetWindow( pWin );
}

vcl::Window* OutlinerView::GetWindow() const
{
    return pEditView->GetWindow();
}

void OutlinerView::SetOutputArea( const Rectangle& rRect )
{
    pEditView->SetOutputArea( rRect );
}

Rectangle OutlinerView::GetOutputArea() const
{
    return pEditView->GetOutputArea();
}

OUString OutlinerView::GetSelected() const
{
    return pEditView->GetSelected();
}

void OutlinerView::StartSpeller()
{
    pEditView->StartSpeller();
}

EESpellState OutlinerView::StartThesaurus()
{
    return pEditView->StartThesaurus();
}

void OutlinerView::StartTextConversion(
    LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont,
    sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc )
{
    if (
        (LANGUAGE_KOREAN == nSrcLang && LANGUAGE_KOREAN == nDestLang) ||
        (LANGUAGE_CHINESE_SIMPLIFIED  == nSrcLang && LANGUAGE_CHINESE_TRADITIONAL == nDestLang) ||
        (LANGUAGE_CHINESE_TRADITIONAL == nSrcLang && LANGUAGE_CHINESE_SIMPLIFIED  == nDestLang)
       )
    {
        pEditView->StartTextConversion( nSrcLang, nDestLang, pDestFont, nOptions, bIsInteractive, bMultipleDoc );
    }
    else
    {
        OSL_FAIL( "unexpected language" );
    }
}


sal_Int32 OutlinerView::StartSearchAndReplace( const SvxSearchItem& rSearchItem )
{
    return pEditView->StartSearchAndReplace( rSearchItem );
}

void OutlinerView::TransliterateText( sal_Int32 nTransliterationMode )
{
    pEditView->TransliterateText( nTransliterationMode );
}

ESelection OutlinerView::GetSelection()
{
    return pEditView->GetSelection();
}


void OutlinerView::Scroll( long nHorzScroll, long nVertScroll )
{
    pEditView->Scroll( nHorzScroll, nVertScroll );
}

void OutlinerView::SetControlWord( EVControlBits nWord )
{
    pEditView->SetControlWord( nWord );
}

EVControlBits OutlinerView::GetControlWord() const
{
    return pEditView->GetControlWord();
}

void OutlinerView::SetAnchorMode( EVAnchorMode eMode )
{
    pEditView->SetAnchorMode( eMode );
}

EVAnchorMode OutlinerView::GetAnchorMode() const
{
    return pEditView->GetAnchorMode();
}

void OutlinerView::Copy()
{
    pEditView->Copy();
}

void OutlinerView::InsertField( const SvxFieldItem& rFld )
{
    pEditView->InsertField( rFld );
}

const SvxFieldItem* OutlinerView::GetFieldUnderMousePointer() const
{
    return pEditView->GetFieldUnderMousePointer();
}

const SvxFieldItem* OutlinerView::GetFieldAtSelection() const
{
    return pEditView->GetFieldAtSelection();
}

void OutlinerView::SetInvalidateMore( sal_uInt16 nPixel )
{
    pEditView->SetInvalidateMore( nPixel );
}


sal_uInt16 OutlinerView::GetInvalidateMore() const
{
    return pEditView->GetInvalidateMore();
}


bool OutlinerView::IsCursorAtWrongSpelledWord()
{
    return pEditView->IsCursorAtWrongSpelledWord();
}


bool OutlinerView::IsWrongSpelledWordAtPos( const Point& rPosPixel, bool bMarkIfWrong )
{
    return pEditView->IsWrongSpelledWordAtPos( rPosPixel, bMarkIfWrong );
}

void OutlinerView::ExecuteSpellPopup( const Point& rPosPixel, Link<SpellCallbackInfo&,void>* pStartDlg )
{
    pEditView->ExecuteSpellPopup( rPosPixel, pStartDlg );
}

sal_uLong OutlinerView::Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    sal_Int32 nOldParaCount = pEditView->GetEditEngine()->GetParagraphCount();
    ESelection aOldSel = pEditView->GetSelection();
    aOldSel.Adjust();

    sal_uLong nRet = pEditView->Read( rInput, rBaseURL, eFormat, pHTTPHeaderAttrs );

    long nParaDiff = pEditView->GetEditEngine()->GetParagraphCount() - nOldParaCount;
    sal_Int32 nChangesStart = aOldSel.nStartPara;
    sal_Int32 nChangesEnd = nChangesStart + nParaDiff + (aOldSel.nEndPara-aOldSel.nStartPara);

    for ( sal_Int32 n = nChangesStart; n <= nChangesEnd; n++ )
    {
        if ( eFormat == EE_FORMAT_BIN )
        {
            const SfxItemSet& rAttrs = pOwner->GetParaAttribs( n );
            const SfxInt16Item& rLevel = static_cast<const SfxInt16Item&>( rAttrs.Get( EE_PARA_OUTLLEVEL ) );
            sal_uInt16 nDepth = rLevel.GetValue();
            pOwner->ImplInitDepth( n, nDepth, false );
        }

        if ( pOwner->ImplGetOutlinerMode() == OutlinerMode::OutlineObject )
            pOwner->ImplSetLevelDependendStyleSheet( n );
    }

    if ( eFormat != EE_FORMAT_BIN )
    {
        pOwner->ImpFilterIndents( nChangesStart, nChangesEnd );
    }

    return nRet;
}

void OutlinerView::SetBackgroundColor( const Color& rColor )
{
    pEditView->SetBackgroundColor( rColor );
}

void OutlinerView::registerLibreOfficeKitViewCallback(OutlinerViewCallable* pCallable)
{
    pEditView->registerLibreOfficeKitViewCallback(pCallable);
}

Color OutlinerView::GetBackgroundColor()
{
    return pEditView->GetBackgroundColor();
}

SfxItemSet OutlinerView::GetAttribs()
{
    return pEditView->GetAttribs();
}

SvtScriptType OutlinerView::GetSelectedScriptType() const
{
    return pEditView->GetSelectedScriptType();
}

OUString OutlinerView::GetSurroundingText() const
{
    return pEditView->GetSurroundingText();
}

Selection OutlinerView::GetSurroundingTextSelection() const
{
    return pEditView->GetSurroundingTextSelection();
}

// ===== some code for thesaurus sub menu within context menu


namespace {

bool isSingleScriptType( SvtScriptType nScriptType )
{
    sal_uInt8 nScriptCount = 0;

    if (nScriptType & SvtScriptType::LATIN)
        ++nScriptCount;
    if (nScriptType & SvtScriptType::ASIAN)
        ++nScriptCount;
    if (nScriptType & SvtScriptType::COMPLEX)
        ++nScriptCount;

    return nScriptCount == 1;
}

}

// returns: true if a word for thesaurus look-up was found at the current cursor position.
// The status string will be word + iso language string (e.g. "light#en-US")
bool GetStatusValueForThesaurusFromContext(
    OUString &rStatusVal,
    LanguageType &rLang,
    const EditView &rEditView )
{
    // get text and locale for thesaurus look up
    OUString aText;
    EditEngine *pEditEngine = rEditView.GetEditEngine();
    ESelection aTextSel( rEditView.GetSelection() );
    if (!aTextSel.HasRange())
        aTextSel = pEditEngine->GetWord( aTextSel, i18n::WordType::DICTIONARY_WORD );
    aText = pEditEngine->GetText( aTextSel );
    aTextSel.Adjust();

    if (!isSingleScriptType(pEditEngine->GetScriptType(aTextSel)))
        return false;

    LanguageType nLang = pEditEngine->GetLanguage( aTextSel.nStartPara, aTextSel.nStartPos );
    OUString aLangText( LanguageTag::convertToBcp47( nLang ) );

    // set word and locale to look up as status value
    rStatusVal  = aText + "#" + aLangText;
    rLang       = nLang;

    return aText.getLength() > 0;
}


void ReplaceTextWithSynonym( EditView &rEditView, const OUString &rSynonmText )
{
    // get selection to use
    ESelection aCurSel( rEditView.GetSelection() );
    if (!rEditView.HasSelection())
    {
        // select the same word that was used in GetStatusValueForThesaurusFromContext by calling GetWord.
        // (In the end both functions will call ImpEditEngine::SelectWord)
        rEditView.SelectCurrentWord( i18n::WordType::DICTIONARY_WORD );
        aCurSel = rEditView.GetSelection();
    }

    // replace word ...
    rEditView.InsertText( rSynonmText );
    rEditView.ShowCursor( true, false );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
