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

DBG_NAME(OutlinerView)


OutlinerView::OutlinerView( Outliner* pOut, Window* pWin )
{
    DBG_CTOR( OutlinerView, 0 );

    pOwner                      = pOut;
    bDDCursorVisible            = sal_False;
    bInDragMode                 = sal_False;
    nDDScrollLRBorderWidthWin   = 0;
    nDDScrollTBBorderWidthWin   = 0;
    pHorTabArrDoc               = 0;

    pEditView = new EditView( pOut->pEditEngine, pWin );
    pEditView->SetSelectionMode( EE_SELMODE_TXTONLY );
}

OutlinerView::~OutlinerView()
{
    DBG_DTOR(OutlinerView,0);
    delete pEditView;
}

void OutlinerView::Paint( const Rectangle& rRect, OutputDevice* pTargetDevice )
{
    DBG_CHKTHIS(OutlinerView,0);

    // For the first Paint/KeyInput/Drop an emty Outliner is turned into
    // an Outliner with exactly one paragraph.
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( String() );

    pEditView->Paint( rRect, pTargetDevice );
}

sal_Bool OutlinerView::PostKeyEvent( const KeyEvent& rKEvt, Window* pFrameWin )
{
    DBG_CHKTHIS( OutlinerView, 0 );

    // For the first Paint/KeyInput/Drop an emty Outliner is turned into
    // an Outliner with exactly one paragraph.
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( String() );


    sal_Bool bKeyProcessed = sal_False;
    ESelection aSel( pEditView->GetSelection() );
    sal_Bool bSelection = aSel.HasRange();
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    KeyFuncType eFunc = aKeyCode.GetFunction();
    sal_uInt16 nCode = aKeyCode.GetCode();
    sal_Bool bReadOnly = IsReadOnly();

    if( bSelection && ( nCode != KEY_TAB ) && EditEngine::DoesKeyChangeText( rKEvt ) )
    {
        if ( ImpCalcSelectedPages( sal_False ) && !pOwner->ImpCanDeleteSelectedPages( this ) )
            return sal_True;
    }

    if ( eFunc != KEYFUNC_DONTKNOW )
    {
        switch ( eFunc )
        {
            case KEYFUNC_CUT:
            {
                if ( !bReadOnly )
                {
                    Cut();
                    bKeyProcessed = sal_True;
                }
            }
            break;
            case KEYFUNC_COPY:
            {
                Copy();
                bKeyProcessed = sal_True;
            }
            break;
            case KEYFUNC_PASTE:
            {
                if ( !bReadOnly )
                {
                    PasteSpecial();
                    bKeyProcessed = sal_True;
                }
            }
            break;
            case KEYFUNC_DELETE:
            {
                if( !bReadOnly && !bSelection && ( pOwner->ImplGetOutlinerMode() != OUTLINERMODE_TEXTOBJECT ) )
                {
                    if( aSel.nEndPos == pOwner->pEditEngine->GetTextLen( aSel.nEndPara ) )
                    {
                        Paragraph* pNext = pOwner->pParaList->GetParagraph( aSel.nEndPara+1 );
                        if( pNext && pNext->HasFlag(PARAFLAG_ISPAGE) )
                        {
                            if( !pOwner->ImpCanDeleteSelectedPages( this, aSel.nEndPara, 1 ) )
                                return sal_False;
                        }
                    }
                }
            }
            break;
            default:    // is then possibly edited below.
                        eFunc = KEYFUNC_DONTKNOW;
        }
    }
    if ( eFunc == KEYFUNC_DONTKNOW )
    {
        switch ( nCode )
        {
            case KEY_TAB:
            {
                if ( !bReadOnly && !aKeyCode.IsMod1() && !aKeyCode.IsMod2() )
                {
                    if ( ( pOwner->ImplGetOutlinerMode() != OUTLINERMODE_TEXTOBJECT ) &&
                         ( pOwner->ImplGetOutlinerMode() != OUTLINERMODE_TITLEOBJECT ) &&
                         ( bSelection || !aSel.nStartPos ) )
                    {
                        Indent( aKeyCode.IsShift() ? (-1) : (+1) );
                        bKeyProcessed = sal_True;
                    }
                    else if ( ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) &&
                              !bSelection && !aSel.nEndPos && pOwner->ImplHasNumberFormat( aSel.nEndPara ) )
                    {
                        Indent( aKeyCode.IsShift() ? (-1) : (+1) );
                        bKeyProcessed = sal_True;
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
                        return sal_True;
                    if( !pPara->GetDepth() )
                    {
                        if(!pOwner->ImpCanDeleteSelectedPages(this, aSel.nEndPara , 1 ) )
                            return sal_True;
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
                                    pOwner->Insert( String(),nTemp,pPara->GetDepth());
                                    // Position the cursor
                                    ESelection aTmpSel(nTemp,0,nTemp,0);
                                    pEditView->SetSelection( aTmpSel );
                                }
                                pEditView->ShowCursor( sal_True, sal_True );
                                pOwner->UndoActionEnd( OLUNDO_INSERT );
                                bKeyProcessed = sal_True;
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
                        pOwner->Insert( String(), nTemp, pPara->GetDepth()+1 );

                        // Position the cursor
                        ESelection aTmpSel(nTemp,0,nTemp,0);
                        pEditView->SetSelection( aTmpSel );
                        pEditView->ShowCursor( sal_True, sal_True );
                        pOwner->UndoActionEnd( OLUNDO_INSERT );
                        bKeyProcessed = sal_True;
                    }
                }
            }
            break;
        }
    }

    return bKeyProcessed ? sal_True : pEditView->PostKeyEvent( rKEvt, pFrameWin );

}


sal_Int32 OutlinerView::ImpCheckMousePos(const Point& rPosPix, MouseTarget& reTarget)
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_Int32 nPara = EE_PARA_NOT_FOUND;

    Point aMousePosWin = pEditView->GetWindow()->PixelToLogic( rPosPix );
    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
    {
        reTarget = MouseOutside;
    }
    else
    {
        reTarget = MouseText;

        Point aPaperPos( aMousePosWin );
        Rectangle aOutArea = pEditView->GetOutputArea();
        Rectangle aVisArea = pEditView->GetVisArea();
        aPaperPos.X() -= aOutArea.Left();
        aPaperPos.X() += aVisArea.Left();
        aPaperPos.Y() -= aOutArea.Top();
        aPaperPos.Y() += aVisArea.Top();

        sal_Bool bBullet;
        if ( pOwner->IsTextPos( aPaperPos, 0, &bBullet ) )
        {
            Point aDocPos = pOwner->GetDocPos( aPaperPos );
            nPara = pOwner->pEditEngine->FindParagraph( aDocPos.Y() );

            if ( bBullet )
            {
                reTarget = MouseBullet;
            }
            else
            {
                // Check for hyperlink
                const SvxFieldItem* pFieldItem = pEditView->GetField( aMousePosWin );
                if ( pFieldItem && pFieldItem->GetField() && pFieldItem->GetField()->ISA( SvxURLField ) )
                    reTarget = MouseHypertext;
            }
        }
    }
    return nPara;
}

sal_Bool OutlinerView::MouseMove( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(OutlinerView,0);

    if( ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) || pEditView->GetEditEngine()->IsInSelectionMode())
        return pEditView->MouseMove( rMEvt );

    Point aMousePosWin( pEditView->GetWindow()->PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
        return sal_False;

    Pointer aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );
    return pEditView->MouseMove( rMEvt );
}


sal_Bool OutlinerView::MouseButtonDown( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) || pEditView->GetEditEngine()->IsInSelectionMode() )
        return pEditView->MouseButtonDown( rMEvt );

    Point aMousePosWin( pEditView->GetWindow()->PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
        return sal_False;

    Pointer aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );

    MouseTarget eTarget;
    sal_Int32 nPara = ImpCheckMousePos( rMEvt.GetPosPixel(), eTarget );
    if ( eTarget == MouseBullet )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        sal_Bool bHasChildren = (pPara && pOwner->pParaList->HasChildren(pPara));
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

        aDDStartPosPix = rMEvt.GetPosPixel();
        aDDStartPosRef=pEditView->GetWindow()->PixelToLogic( aDDStartPosPix,pOwner->GetRefMapMode());
        return sal_True;
    }

    // special case for outliner view in impress, check if double click hits the page icon for toggle
    if( (nPara == EE_PARA_NOT_FOUND) && (pOwner->ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEVIEW) && (eTarget == MouseText) && (rMEvt.GetClicks() == 2) )
    {
        ESelection aSel( pEditView->GetSelection() );
        nPara = aSel.nStartPara;
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        if( (pPara && pOwner->pParaList->HasChildren(pPara)) && pPara->HasFlag(PARAFLAG_ISPAGE) )
        {
            ImpToggleExpand( pPara );
        }
    }
    return pEditView->MouseButtonDown( rMEvt );
}


sal_Bool OutlinerView::MouseButtonUp( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) || pEditView->GetEditEngine()->IsInSelectionMode() )
        return pEditView->MouseButtonUp( rMEvt );

    Point aMousePosWin( pEditView->GetWindow()->PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
        return sal_False;

    Pointer aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );

    return pEditView->MouseButtonUp( rMEvt );
}

void OutlinerView::ImpToggleExpand( Paragraph* pPara )
{
    DBG_CHKTHIS(OutlinerView,0);

    sal_Int32 nPara = pOwner->pParaList->GetAbsPos( pPara );
    pEditView->SetSelection( ESelection( nPara, 0, nPara, 0 ) );
    ImplExpandOrCollaps( nPara, nPara, !pOwner->pParaList->HasVisibleChildren( pPara ) );
    pEditView->ShowCursor();
}

sal_Int32 OutlinerView::Select( Paragraph* pParagraph, sal_Bool bSelect,
    sal_Bool bWithChildren )
{
    DBG_CHKTHIS(OutlinerView,0);

    sal_Int32 nPara = pOwner->pParaList->GetAbsPos( pParagraph );
    sal_uInt16 nEnd = 0;
    if ( bSelect )
        nEnd = 0xffff;

    sal_Int32 nChildCount = 0;
    if ( bWithChildren )
        nChildCount = pOwner->pParaList->GetChildCount( pParagraph );

    ESelection aSel( nPara, 0,nPara+nChildCount, nEnd );
    pEditView->SetSelection( aSel );
    return nChildCount+1;
}


void OutlinerView::SetAttribs( const SfxItemSet& rAttrs )
{
    DBG_CHKTHIS(OutlinerView,0);

    sal_Bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( sal_False );

    if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
        pOwner->UndoActionStart( OLUNDO_ATTR );

    ParaRange aSel = ImpGetSelectedParagraphs( sal_False );

    pEditView->SetAttribs( rAttrs );

    // Update Bullet text
    for( sal_Int32 nPara= aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        pOwner->ImplCheckNumBulletItem( nPara );
        pOwner->ImplCalcBulletText( nPara, sal_False, sal_False );

        if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
            pOwner->InsertUndo( new OutlinerUndoCheckPara( pOwner, nPara ) );
    }

    if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
        pOwner->UndoActionEnd( OLUNDO_ATTR );

    pEditView->SetEditEngineUpdateMode( bUpdate );
}

ParaRange OutlinerView::ImpGetSelectedParagraphs( sal_Bool bIncludeHiddenChildren )
{
    DBG_CHKTHIS( OutlinerView, 0 );

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
    DBG_CHKTHIS( OutlinerView, 0 );

    if( !nDiff || ( ( nDiff > 0 ) && ImpCalcSelectedPages( sal_True ) && !pOwner->ImpCanIndentSelectedPages( this ) ) )
        return;

    const bool bOutlinerView = pOwner->pEditEngine->GetControlWord() & EE_CNTRL_OUTLINER;
    sal_Bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( sal_False );

    sal_Bool bUndo = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();

    if( bUndo )
        pOwner->UndoActionStart( OLUNDO_DEPTH );

    sal_Int16 nMinDepth = -1;   // Optimization: Not to recalculate to manny parargaphs when not really needed.

    ParaRange aSel = ImpGetSelectedParagraphs( sal_True );
    for ( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );

        sal_Int16 nOldDepth = pPara->GetDepth();
        sal_Int16 nNewDepth = nOldDepth + nDiff;

        if( bOutlinerView && nPara )
        {
            const bool bPage = pPara->HasFlag(PARAFLAG_ISPAGE);
            if( (bPage && (nDiff == +1)) || (!bPage && (nDiff == -1) && (nOldDepth <= 0))  )
            {
                            // Notify App
                pOwner->nDepthChangedHdlPrevDepth = nOldDepth;
                pOwner->mnDepthChangeHdlPrevFlags = pPara->nFlags;
                pOwner->pHdlParagraph = pPara;

                if( bPage )
                    pPara->RemoveFlag( PARAFLAG_ISPAGE );
                else
                    pPara->SetFlag( PARAFLAG_ISPAGE );

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
            if ( ( nPara == aSel.nStartPara ) && aSel.nStartPara && ( pOwner->ImplGetOutlinerMode() != OUTLINERMODE_TEXTOBJECT ))
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
                    pOwner->InvalidateBullet( pPrev, pOwner->pParaList->GetAbsPos( pPrev ) );
                }
            }

            pOwner->nDepthChangedHdlPrevDepth = nOldDepth;
            pOwner->mnDepthChangeHdlPrevFlags = pPara->nFlags;
            pOwner->pHdlParagraph = pPara;

            pOwner->ImplInitDepth( nPara, nNewDepth, sal_True, sal_False );
            pOwner->ImplCalcBulletText( nPara, sal_False, sal_False );

            if ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT )
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
        pOwner->ImplCalcBulletText( n, sal_False, sal_False );
    }

    if ( bUpdate )
    {
        pEditView->SetEditEngineUpdateMode( sal_True );
        pEditView->ShowCursor();
    }

    if( bUndo )
        pOwner->UndoActionEnd( OLUNDO_DEPTH );
}

sal_Bool OutlinerView::AdjustHeight( long nDY )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->MoveParagraphs( nDY );
    return sal_True;    // remove return value...
}

Rectangle OutlinerView::GetVisArea() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetVisArea();
}

void OutlinerView::Expand()
{
    DBG_CHKTHIS( OutlinerView, 0 );
    ParaRange aParas = ImpGetSelectedParagraphs( sal_False );
    ImplExpandOrCollaps( aParas.nStartPara, aParas.nEndPara, sal_True );
}


void OutlinerView::Collapse()
{
    DBG_CHKTHIS( OutlinerView, 0 );
    ParaRange aParas = ImpGetSelectedParagraphs( sal_False );
    ImplExpandOrCollaps( aParas.nStartPara, aParas.nEndPara, sal_False );
}


void OutlinerView::ExpandAll()
{
    DBG_CHKTHIS( OutlinerView, 0 );
    ImplExpandOrCollaps( 0, pOwner->pParaList->GetParagraphCount()-1, sal_True );
}


void OutlinerView::CollapseAll()
{
    DBG_CHKTHIS(OutlinerView,0);
    ImplExpandOrCollaps( 0, pOwner->pParaList->GetParagraphCount()-1, sal_False );
}

void OutlinerView::ImplExpandOrCollaps( sal_Int32 nStartPara, sal_Int32 nEndPara, sal_Bool bExpand )
{
    DBG_CHKTHIS( OutlinerView, 0 );

    sal_Bool bUpdate = pOwner->GetUpdateMode();
    pOwner->SetUpdateMode( sal_False );

    sal_Bool bUndo = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();
    if( bUndo )
        pOwner->UndoActionStart( bExpand ? OLUNDO_EXPAND : OLUNDO_COLLAPSE );

    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        sal_Bool bDone = bExpand ? pOwner->Expand( pPara ) : pOwner->Collapse( pPara );
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
        pOwner->SetUpdateMode( sal_True );
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

    DBG_CHKTHIS(OutlinerView,0);

    if ( ImpCalcSelectedPages( sal_False ) && !pOwner->ImpCanDeleteSelectedPages( this ) )
        return;

    pOwner->UndoActionStart( OLUNDO_INSERT );

    pOwner->pEditEngine->SetUpdateMode( sal_False );
    sal_Int32 nStart, nParaCount;
    nParaCount = pOwner->pEditEngine->GetParagraphCount();
    sal_uInt16 nSize = ImpInitPaste( nStart );
    pEditView->InsertText( rParaObj.GetTextObject() );
    ImpPasted( nStart, nParaCount, nSize);
    pEditView->SetEditEngineUpdateMode( sal_True );

    pOwner->UndoActionEnd( OLUNDO_INSERT );

    pEditView->ShowCursor( sal_True, sal_True );
}



void OutlinerView::Cut()
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( !ImpCalcSelectedPages( sal_False ) || pOwner->ImpCanDeleteSelectedPages( this ) )
        pEditView->Cut();
}

void OutlinerView::Paste()
{
    DBG_CHKTHIS(OutlinerView,0);
    PasteSpecial(); // HACK(SD does not call PasteSpecial)
}

void OutlinerView::PasteSpecial()
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( !ImpCalcSelectedPages( sal_False ) || pOwner->ImpCanDeleteSelectedPages( this ) )
    {
        pOwner->UndoActionStart( OLUNDO_INSERT );

        pOwner->pEditEngine->SetUpdateMode( sal_False );
        pOwner->bPasting = sal_True;
        pEditView->PasteSpecial();

        if ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT )
        {
            const sal_Int32 nParaCount = pOwner->pEditEngine->GetParagraphCount();

            for( sal_Int32 nPara = 0; nPara < nParaCount; nPara++ )
                pOwner->ImplSetLevelDependendStyleSheet( nPara );
        }

        pEditView->SetEditEngineUpdateMode( sal_True );
        pOwner->UndoActionEnd( OLUNDO_INSERT );
        pEditView->ShowCursor( sal_True, sal_True );
    }
}

void OutlinerView::CreateSelectionList (std::vector<Paragraph*> &aSelList)
{
    DBG_CHKTHIS( OutlinerView, 0 );

    ParaRange aParas = ImpGetSelectedParagraphs( sal_True );

    for ( sal_Int32 nPara = aParas.nStartPara; nPara <= aParas.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        aSelList.push_back(pPara);
    }
}

const SfxStyleSheet* OutlinerView::GetStyleSheet() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetStyleSheet();
}

SfxStyleSheet* OutlinerView::GetStyleSheet()
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetStyleSheet();
}

Pointer OutlinerView::GetPointer( const Point& rPosPixel )
{
    DBG_CHKTHIS(OutlinerView,0);

    MouseTarget eTarget;
    ImpCheckMousePos( rPosPixel, eTarget );

    PointerStyle ePointerStyle = POINTER_ARROW;
    if ( eTarget == MouseText )
    {
        ePointerStyle = GetOutliner()->IsVertical() ? POINTER_TEXT_VERTICAL : POINTER_TEXT;
    }
    else if ( eTarget == MouseHypertext )
    {
        ePointerStyle = POINTER_REFHAND;
    }
    else if ( eTarget == MouseBullet )
    {
        ePointerStyle = POINTER_MOVE;
    }

    return Pointer( ePointerStyle );
}


sal_Int32 OutlinerView::ImpInitPaste( sal_Int32& rStart )
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner->bPasting = sal_True;
    ESelection aSelection( pEditView->GetSelection() );
    aSelection.Adjust();
    rStart = aSelection.nStartPara;
    sal_Int32 nSize = aSelection.nEndPara - aSelection.nStartPara + 1;
    return nSize;
}


void OutlinerView::ImpPasted( sal_Int32 nStart, sal_Int32 nPrevParaCount, sal_Int32 nSize)
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner->bPasting = sal_False;
    sal_Int32 nCurParaCount = pOwner->pEditEngine->GetParagraphCount();
    if( nCurParaCount < nPrevParaCount )
        nSize = nSize - ( nPrevParaCount - nCurParaCount );
    else
        nSize = nSize + ( nCurParaCount - nPrevParaCount );
    pOwner->ImpTextPasted( nStart, nSize );
}


void OutlinerView::Command( const CommandEvent& rCEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->Command( rCEvt );
}


void OutlinerView::SelectRange( sal_Int32 nFirst, sal_Int32 nCount )
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_Int32 nLast = nFirst+nCount;
    nCount = pOwner->pParaList->GetParagraphCount();
    if( nLast <= nCount )
        nLast = nCount - 1;
    ESelection aSel( nFirst, 0, nLast, EE_TEXTPOS_ALL );
    pEditView->SetSelection( aSel );
}


sal_Int32 OutlinerView::ImpCalcSelectedPages( sal_Bool bIncludeFirstSelected )
{
    DBG_CHKTHIS(OutlinerView,0);

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
        if( pPara->HasFlag(PARAFLAG_ISPAGE) )
        {
            nPages++;
            if( nFirstPage == EE_PARA_MAX_COUNT )
                nFirstPage = nPara;
        }
    }

    if( nPages )
    {
        pOwner->nDepthChangedHdlPrevDepth = nPages;
        pOwner->pHdlParagraph = 0;
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
    pOwner->pEditEngine->SetUpdateMode( sal_False );

    sal_Int16 nNewDepth = -2;
    const SvxNumRule* pDefaultBulletNumRule = 0;

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
                    pDefaultBulletNumRule =  pNumBulletItem ? pNumBulletItem->GetNumRule() : 0;
                }
            }

            pOwner->SetDepth( pPara, nNewDepth );

            if( nNewDepth == -1 )
            {
                const SfxItemSet& rAttrs = pOwner->GetParaAttribs( nPara );
                if ( rAttrs.GetItemState( EE_PARA_BULLETSTATE ) == SFX_ITEM_SET )
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
                        aAttrs.Put( SvxNumBulletItem( aNewNumRule ), EE_PARA_NUMBULLET );
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
    pOwner->pEditEngine->SetUpdateMode( sal_False );

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
    const sal_Bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode(sal_False);

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
                        const SfxPoolItem* pPoolItem=NULL;
                        SfxItemState eState = rAttrs.GetItemState(EE_PARA_NUMBULLET, sal_False, &pPoolItem);
                        if (eState != SFX_ITEM_SET)
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

                    aAttrs.Put(SvxNumBulletItem(aNewRule), EE_PARA_NUMBULLET);
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
    pOwner->pEditEngine->SetUpdateMode( sal_False );

    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; ++nPara )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::SwitchOffBulletsNumbering(...), illegal paragraph index?");

        if( pPara )
        {
            pOwner->SetDepth( pPara, -1 );

            const SfxItemSet& rAttrs = pOwner->GetParaAttribs( nPara );
            if (rAttrs.GetItemState( EE_PARA_BULLETSTATE ) == SFX_ITEM_SET)
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


void OutlinerView::RemoveAttribsKeepLanguages( sal_Bool bRemoveParaAttribs )
{
    RemoveAttribs( bRemoveParaAttribs, 0, sal_True /*keep language attribs*/ );
}

void OutlinerView::RemoveAttribs( sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich, sal_Bool bKeepLanguages )
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_Bool bUpdate = pOwner->GetUpdateMode();
    pOwner->SetUpdateMode( sal_False );
    pOwner->UndoActionStart( OLUNDO_ATTR );
    if (bKeepLanguages)
        pEditView->RemoveAttribsKeepLanguages( bRemoveParaAttribs );
    else
        pEditView->RemoveAttribs( bRemoveParaAttribs, nWhich );
    if ( bRemoveParaAttribs )
    {
        // Loop through all paragraphs and set indentation and level
        ESelection aSel = pEditView->GetSelection();
        aSel.Adjust();
        for ( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
        {
            Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
            pOwner->ImplInitDepth( nPara, pPara->GetDepth(), sal_False, sal_False );
        }
    }
    pOwner->UndoActionEnd( OLUNDO_ATTR );
    pOwner->SetUpdateMode( bUpdate );
}



// ======================================================================
// ======================   Simple pass-through   =======================
// ======================================================================


void OutlinerView::InsertText( const OUString& rNew, sal_Bool bSelect )
{
    DBG_CHKTHIS(OutlinerView,0);
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( OUString() );
    pEditView->InsertText( rNew, bSelect );
}

void OutlinerView::SetVisArea( const Rectangle& rRect )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetVisArea( rRect );
}


void OutlinerView::SetSelection( const ESelection& rSel )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetSelection( rSel );
}

void OutlinerView::SetReadOnly( sal_Bool bReadOnly )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetReadOnly( bReadOnly );
}

sal_Bool OutlinerView::IsReadOnly() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->IsReadOnly();
}

sal_Bool OutlinerView::HasSelection() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->HasSelection();
}


void OutlinerView::ShowCursor( sal_Bool bGotoCursor )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->ShowCursor( bGotoCursor );
}


void OutlinerView::HideCursor()
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->HideCursor();
}


void OutlinerView::SetWindow( Window* pWin )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetWindow( pWin );
}


Window* OutlinerView::GetWindow() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetWindow();
}


void OutlinerView::SetOutputArea( const Rectangle& rRect )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetOutputArea( rRect );
}


Rectangle OutlinerView::GetOutputArea() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetOutputArea();
}


OUString OutlinerView::GetSelected() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetSelected();
}

EESpellState OutlinerView::StartSpeller( sal_Bool bMultiDoc )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->StartSpeller( bMultiDoc );
}

EESpellState OutlinerView::StartThesaurus()
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->StartThesaurus();
}


void OutlinerView::StartTextConversion(
    LanguageType nSrcLang, LanguageType nDestLang, const Font *pDestFont,
    sal_Int32 nOptions, sal_Bool bIsInteractive, sal_Bool bMultipleDoc )
{
    DBG_CHKTHIS(OutlinerView,0);
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


sal_uInt16 OutlinerView::StartSearchAndReplace( const SvxSearchItem& rSearchItem )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->StartSearchAndReplace( rSearchItem );
}

void OutlinerView::TransliterateText( sal_Int32 nTransliterationMode )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->TransliterateText( nTransliterationMode );
}



ESelection OutlinerView::GetSelection()
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetSelection();
}


void OutlinerView::Scroll( long nHorzScroll, long nVertScroll )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->Scroll( nHorzScroll, nVertScroll );
}


void OutlinerView::SetControlWord( sal_uLong nWord )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetControlWord( nWord );
}


sal_uLong OutlinerView::GetControlWord() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetControlWord();
}


void OutlinerView::SetAnchorMode( EVAnchorMode eMode )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetAnchorMode( eMode );
}


EVAnchorMode OutlinerView::GetAnchorMode() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetAnchorMode();
}

void OutlinerView::Copy()
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->Copy();
}


void OutlinerView::InsertField( const SvxFieldItem& rFld )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->InsertField( rFld );
}

const SvxFieldItem* OutlinerView::GetFieldUnderMousePointer() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetFieldUnderMousePointer();
}

const SvxFieldItem* OutlinerView::GetFieldAtSelection() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetFieldAtSelection();
}

void OutlinerView::SetInvalidateMore( sal_uInt16 nPixel )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetInvalidateMore( nPixel );
}


sal_uInt16 OutlinerView::GetInvalidateMore() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetInvalidateMore();
}


sal_Bool OutlinerView::IsCursorAtWrongSpelledWord( sal_Bool bMarkIfWrong )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->IsCursorAtWrongSpelledWord( bMarkIfWrong );
}


sal_Bool OutlinerView::IsWrongSpelledWordAtPos( const Point& rPosPixel, sal_Bool bMarkIfWrong )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->IsWrongSpelledWordAtPos( rPosPixel, bMarkIfWrong );
}

void OutlinerView::ExecuteSpellPopup( const Point& rPosPixel, Link* pStartDlg )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->ExecuteSpellPopup( rPosPixel, pStartDlg );
}

sal_uLong OutlinerView::Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, sal_Bool bSelect, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_Int32 nOldParaCount = pEditView->GetEditEngine()->GetParagraphCount();
    ESelection aOldSel = pEditView->GetSelection();
    aOldSel.Adjust();

    sal_uLong nRet = pEditView->Read( rInput, rBaseURL, eFormat, bSelect, pHTTPHeaderAttrs );

    long nParaDiff = pEditView->GetEditEngine()->GetParagraphCount() - nOldParaCount;
    sal_Int32 nChangesStart = aOldSel.nStartPara;
    sal_Int32 nChangesEnd = nChangesStart + nParaDiff + (aOldSel.nEndPara-aOldSel.nStartPara);

    for ( sal_Int32 n = nChangesStart; n <= nChangesEnd; n++ )
    {
        if ( eFormat == EE_FORMAT_BIN )
        {
            const SfxItemSet& rAttrs = pOwner->GetParaAttribs( n );
            const SfxInt16Item& rLevel = (const SfxInt16Item&) rAttrs.Get( EE_PARA_OUTLLEVEL );
            sal_uInt16 nDepth = rLevel.GetValue();
            pOwner->ImplInitDepth( n, nDepth, sal_False );
        }

        if ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT )
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
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetBackgroundColor( rColor );
}


Color OutlinerView::GetBackgroundColor()
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetBackgroundColor();
}

SfxItemSet OutlinerView::GetAttribs()
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetAttribs();
}

sal_uInt16 OutlinerView::GetSelectedScriptType() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetSelectedScriptType();
}

OUString OutlinerView::GetSurroundingText() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetSurroundingText();
}

Selection OutlinerView::GetSurroundingTextSelection() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetSurroundingTextSelection();
}


// ======================================================================
// ===== some code for thesaurus sub menu within context menu
// ======================================================================

// returns: true if a word for thesaurus look-up was found at the current cursor position.
// The status string will be word + iso language string (e.g. "light#en-US")
bool EDITENG_DLLPUBLIC GetStatusValueForThesaurusFromContext(
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
    LanguageType nLang = pEditEngine->GetLanguage( aTextSel.nStartPara, aTextSel.nStartPos );
    OUString aLangText( LanguageTag::convertToBcp47( nLang ) );

    // set word and locale to look up as status value
    rStatusVal  = aText + "#" + aLangText;
    rLang       = nLang;

    return aText.getLength() > 0;
}


void EDITENG_DLLPUBLIC ReplaceTextWithSynonym( EditView &rEditView, const OUString &rSynonmText )
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
    rEditView.ShowCursor( sal_True, sal_False );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
