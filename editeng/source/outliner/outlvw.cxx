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

#include <memory>
#include <com/sun/star/i18n/WordType.hpp>

#include <svl/itempool.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/StripPortionsHelper.hxx>

#include <svl/style.hxx>
#include <svl/languageoptions.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <editeng/outliner.hxx>
#include <outleeng.hxx>
#include "paralist.hxx"
#include "outlundo.hxx"
#include <editeng/outlobj.hxx>
#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/numitem.hxx>
#include <vcl/window.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <editeng/editstat.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

using namespace ::com::sun::star;

OutlinerView::OutlinerView(Outliner& rOut, vcl::Window* pWin)
    : rOwner(rOut)
    , pEditView(new EditView(rOut.getOutlinerEditEng(), pWin))
{
}

OutlinerView::~OutlinerView()
{
}

void OutlinerView::DrawText_ToEditView( const tools::Rectangle& rRect, OutputDevice* pTargetDevice )
{
    // For the first Paint/KeyInput/Drop an empty Outliner is turned into
    // an Outliner with exactly one paragraph.
    if( rOwner.bFirstParaIsEmpty )
        rOwner.Insert( OUString() );

    // use TextHierarchyBreakupOutliner to get all text embedded to the
    // TextHierarchy.*Primitive2D groupings for better processing, plus
    // the correct paragtaph countings
    TextHierarchyBreakupOutliner aHelper(rOwner);

    // hand that Helper over to DrawText_ToEditView at the EditEngine
    // for usage
    pEditView->DrawText_ToEditView( aHelper, rRect, pTargetDevice );
}

bool OutlinerView::PostKeyEvent( const KeyEvent& rKEvt, vcl::Window const * pFrameWin )
{
    // For the first Paint/KeyInput/Drop an empty Outliner is turned into
    // an Outliner with exactly one paragraph.
    if( rOwner.bFirstParaIsEmpty )
        rOwner.Insert( OUString() );

    bool bKeyProcessed = false;
    ESelection aSel( pEditView->GetSelection() );
    bool bSelection = aSel.HasRange();
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    KeyFuncType eFunc = aKeyCode.GetFunction();
    sal_uInt16 nCode = aKeyCode.GetCode();
    bool bReadOnly = IsReadOnly();

    if( bSelection && ( nCode != KEY_TAB ) && EditEngine::DoesKeyChangeText( rKEvt ) )
    {
        if ( ImpCalcSelectedPages( false ) && !rOwner.ImpCanDeleteSelectedPages( this ) )
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
                if( !bReadOnly && !bSelection && ( rOwner.GetOutlinerMode() != OutlinerMode::TextObject ) )
                {
                    if (aSel.end.nIndex == rOwner.pEditEngine->GetTextLen(aSel.end.nPara))
                    {
                        Paragraph* pNext = rOwner.pParaList->GetParagraph(aSel.end.nPara + 1);
                        if( pNext && pNext->HasFlag(ParaFlag::ISPAGE) )
                        {
                            if (!rOwner.ImpCanDeleteSelectedPages(this, aSel.end.nPara, 1))
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
                    if ( ( rOwner.GetOutlinerMode() != OutlinerMode::TextObject ) &&
                         ( rOwner.GetOutlinerMode() != OutlinerMode::TitleObject ) &&
                         ( bSelection || !aSel.start.nIndex ) )
                    {
                        Indent( aKeyCode.IsShift() ? -1 : +1 );
                        bKeyProcessed = true;
                    }
                    else if ( ( rOwner.GetOutlinerMode() == OutlinerMode::TextObject ) &&
                              !bSelection && !aSel.end.nIndex && rOwner.ImplHasNumberFormat( aSel.end.nPara ) )
                    {
                        Indent( aKeyCode.IsShift() ? -1 : +1 );
                        bKeyProcessed = true;
                    }
                }
            }
            break;
            case KEY_BACKSPACE:
            {
                if (!bReadOnly && !bSelection && aSel.end.nPara && !aSel.end.nIndex)
                {
                    Paragraph* pPara = rOwner.pParaList->GetParagraph(aSel.end.nPara);
                    Paragraph* pPrev = rOwner.pParaList->GetParagraph(aSel.end.nPara - 1);
                    if( !pPrev->IsVisible()  )
                        return true;
                    if( !pPara->GetDepth() )
                    {
                        if (!rOwner.ImpCanDeleteSelectedPages(this, aSel.end.nPara, 1))
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
                    Paragraph* pPara = rOwner.pParaList->GetParagraph(aSel.end.nPara);

                    if( !aKeyCode.IsShift() )
                    {
                        // Don't let insert empty paragraph with numbering. Instead end numbering.
                        if (pPara->GetDepth() > -1 &&
                            rOwner.pEditEngine->GetTextLen( aSel.end.nPara ) == 0)
                        {
                            ToggleBullets();
                            return true;
                        }
                        // ImpGetCursor again???
                        if( !bSelection &&
                                aSel.end.nIndex == rOwner.pEditEngine->GetTextLen( aSel.end.nPara ) )
                        {
                            sal_Int32 nChildren = rOwner.pParaList->GetChildCount(pPara);
                            if( nChildren && !rOwner.pParaList->HasVisibleChildren(pPara))
                            {
                                rOwner.UndoActionStart( OLUNDO_INSERT );
                                sal_Int32 nTemp = aSel.end.nPara;
                                nTemp += nChildren;
                                nTemp++; // insert above next Non-Child
                                SAL_WARN_IF( nTemp < 0, "editeng", "OutlinerView::PostKeyEvent - overflow");
                                if (nTemp >= 0)
                                {
                                    rOwner.Insert( OUString(),nTemp,pPara->GetDepth());
                                    // Position the cursor
                                    ESelection aTmpSel(nTemp, 0);
                                    pEditView->SetSelection( aTmpSel );
                                }
                                pEditView->ShowCursor();
                                rOwner.UndoActionEnd();
                                bKeyProcessed = true;
                            }
                        }
                    }
                    if( !bKeyProcessed && !bSelection &&
                                !aKeyCode.IsShift() && aKeyCode.IsMod1() &&
                            ( aSel.end.nIndex == rOwner.pEditEngine->GetTextLen(aSel.end.nPara) ) )
                    {
                        rOwner.UndoActionStart( OLUNDO_INSERT );
                        sal_Int32 nTemp = aSel.end.nPara;
                        nTemp++;
                        rOwner.Insert( OUString(), nTemp, pPara->GetDepth()+1 );

                        // Position the cursor
                        ESelection aTmpSel(nTemp, 0);
                        pEditView->SetSelection( aTmpSel );
                        pEditView->ShowCursor();
                        rOwner.UndoActionEnd();
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
    sal_Int32 nPara = EE_PARA_MAX;

    Point aMousePosWin = pEditView->GetOutputDevice().PixelToLogic( rPosPix );
    if( !pEditView->GetOutputArea().Contains( aMousePosWin ) )
    {
        reTarget = MouseTarget::Outside;
    }
    else
    {
        reTarget = MouseTarget::Text;

        Point aPaperPos( aMousePosWin );
        tools::Rectangle aOutArea = pEditView->GetOutputArea();
        tools::Rectangle aVisArea = pEditView->GetVisArea();
        aPaperPos.AdjustX( -(aOutArea.Left()) );
        aPaperPos.AdjustX(aVisArea.Left() );
        aPaperPos.AdjustY( -(aOutArea.Top()) );
        aPaperPos.AdjustY(aVisArea.Top() );

        bool bBullet;
        if ( rOwner.IsTextPos( aPaperPos, 0, &bBullet ) )
        {
            Point aDocPos = rOwner.GetDocPos( aPaperPos );
            nPara = rOwner.pEditEngine->FindParagraph( aDocPos.Y() );

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
    if( ( rOwner.GetOutlinerMode() == OutlinerMode::TextObject ) || pEditView->getEditEngine().IsInSelectionMode())
        return pEditView->MouseMove( rMEvt );

    Point aMousePosWin( pEditView->GetOutputDevice().PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().Contains( aMousePosWin ) )
        return false;

    PointerStyle aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );
    return pEditView->MouseMove( rMEvt );
}


bool OutlinerView::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( ( rOwner.GetOutlinerMode() == OutlinerMode::TextObject ) || pEditView->getEditEngine().IsInSelectionMode() )
        return pEditView->MouseButtonDown( rMEvt );

    Point aMousePosWin( pEditView->GetOutputDevice().PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().Contains( aMousePosWin ) )
        return false;

    PointerStyle aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );

    MouseTarget eTarget;
    sal_Int32 nPara = ImpCheckMousePos( rMEvt.GetPosPixel(), eTarget );
    if ( eTarget == MouseTarget::Bullet )
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
        bool bHasChildren = (pPara && rOwner.pParaList->HasChildren(pPara));
        if( rMEvt.GetClicks() == 1 )
        {
            sal_Int32 nEndPara = nPara;
            if ( bHasChildren && rOwner.pParaList->HasVisibleChildren(pPara) )
                nEndPara += rOwner.pParaList->GetChildCount( pPara );
            // The selection is inverted, so that EditEngine does not scroll
            ESelection aSel(nEndPara, EE_TEXTPOS_MAX, nPara, 0);
            pEditView->SetSelection( aSel );
        }
        else if( rMEvt.GetClicks() == 2 && bHasChildren )
            ImpToggleExpand( pPara );

        return true;
    }

    // special case for outliner view in impress, check if double click hits the page icon for toggle
    if( (nPara == EE_PARA_MAX) && (rOwner.GetOutlinerMode() == OutlinerMode::OutlineView) && (eTarget == MouseTarget::Text) && (rMEvt.GetClicks() == 2) )
    {
        ESelection aSel( pEditView->GetSelection() );
        nPara = aSel.start.nPara;
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
        if( (pPara && rOwner.pParaList->HasChildren(pPara)) && pPara->HasFlag(ParaFlag::ISPAGE) )
        {
            ImpToggleExpand( pPara );
        }
    }
    return pEditView->MouseButtonDown( rMEvt );
}


bool OutlinerView::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( ( rOwner.GetOutlinerMode() == OutlinerMode::TextObject ) || pEditView->getEditEngine().IsInSelectionMode() )
        return pEditView->MouseButtonUp( rMEvt );

    Point aMousePosWin( pEditView->GetOutputDevice().PixelToLogic( rMEvt.GetPosPixel() ) );
    if( !pEditView->GetOutputArea().Contains( aMousePosWin ) )
        return false;

    PointerStyle aPointer = GetPointer( rMEvt.GetPosPixel() );
    pEditView->GetWindow()->SetPointer( aPointer );

    return pEditView->MouseButtonUp( rMEvt );
}

void OutlinerView::ReleaseMouse()
{
    pEditView->ReleaseMouse();
}

void OutlinerView::ImpToggleExpand( Paragraph const * pPara )
{
    sal_Int32 nPara = rOwner.pParaList->GetAbsPos( pPara );
    pEditView->SetSelection(ESelection(nPara, 0));
    ImplExpandOrCollaps( nPara, nPara, !rOwner.pParaList->HasVisibleChildren( pPara ) );
    pEditView->ShowCursor();
}

void OutlinerView::Select( Paragraph const * pParagraph, bool bSelect )
{
    sal_Int32 nPara = rOwner.pParaList->GetAbsPos( pParagraph );
    sal_Int32 nEnd = 0;
    if ( bSelect )
        nEnd = SAL_MAX_INT32;

    ESelection aSel( nPara, 0, nPara, nEnd );
    pEditView->SetSelection( aSel );
}

void OutlinerView::SetDepth(sal_Int32 nParagraph, sal_Int16 nDepth)
{
    Paragraph* pParagraph = rOwner.GetParagraph(nParagraph);
    rOwner.SetDepth(pParagraph, nDepth);
}

sal_Int16 OutlinerView::GetDepth() const
{
    ESelection aESelection = GetSelection();
    aESelection.Adjust();
    sal_Int16 nDepth = rOwner.GetDepth(aESelection.start.nPara);
    for (sal_Int32 nPara = aESelection.start.nPara + 1; nPara <= aESelection.end.nPara; ++nPara)
    {
        if (nDepth != rOwner.GetDepth(nPara))
            return -2;
    }
    return nDepth;
}

void OutlinerView::SetAttribs( const SfxItemSet& rAttrs )
{
    bool bUpdate = rOwner.pEditEngine->SetUpdateLayout( false );

    if( !rOwner.IsInUndo() && rOwner.IsUndoEnabled() )
        rOwner.UndoActionStart( OLUNDO_ATTR );

    ParaRange aSel = ImpGetSelectedParagraphs( false );

    pEditView->SetAttribs( rAttrs );

    // Update Bullet text
    for( sal_Int32 nPara= aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        rOwner.ImplCheckNumBulletItem( nPara );
        rOwner.ImplCalcBulletText( nPara, false, false );

        if( !rOwner.IsInUndo() && rOwner.IsUndoEnabled() )
            rOwner.InsertUndo( std::make_unique<OutlinerUndoCheckPara>( &rOwner, nPara ) );
    }

    if( !rOwner.IsInUndo() && rOwner.IsUndoEnabled() )
        rOwner.UndoActionEnd();

    pEditView->SetEditEngineUpdateLayout( bUpdate );
}

ParaRange OutlinerView::ImpGetSelectedParagraphs( bool bIncludeHiddenChildren )
{
    ESelection aSel = pEditView->GetSelection();
    ParaRange aParas(aSel.start.nPara, aSel.end.nPara);
    aParas.Adjust();

    // Record the  invisible Children of the last Parents in the selection
    if ( bIncludeHiddenChildren )
    {
        Paragraph* pLast = rOwner.pParaList->GetParagraph( aParas.nEndPara );
        if ( rOwner.pParaList->HasHiddenChildren( pLast ) )
            aParas.nEndPara = aParas.nEndPara + rOwner.pParaList->GetChildCount( pLast );
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
    if( !nDiff || ( ( nDiff > 0 ) && ImpCalcSelectedPages( true ) && !rOwner.ImpCanIndentSelectedPages( this ) ) )
        return;

    const bool bOutlinerView = bool(rOwner.pEditEngine->GetControlWord() & EEControlBits::OUTLINER);
    bool bUpdate = rOwner.pEditEngine->SetUpdateLayout( false );

    bool bUndo = !rOwner.IsInUndo() && rOwner.IsUndoEnabled();

    if( bUndo )
        rOwner.UndoActionStart( OLUNDO_DEPTH );

    sal_Int16 nMinDepth = -1;   // Optimization: avoid recalculate too many paragraphs if not really needed.

    ParaRange aSel = ImpGetSelectedParagraphs( true );
    for ( sal_Int32 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );

        sal_Int16 nOldDepth = pPara->GetDepth();
        sal_Int16 nNewDepth = nOldDepth + nDiff;

        if( bOutlinerView && nPara )
        {
            const bool bPage = pPara->HasFlag(ParaFlag::ISPAGE);
            if( (bPage && (nDiff == +1)) || (!bPage && (nDiff == -1) && (nOldDepth <= 0))  )
            {
                            // Notify App
                rOwner.nDepthChangedHdlPrevDepth = nOldDepth;
                ParaFlag nPrevFlags = pPara->nFlags;

                if( bPage )
                    pPara->RemoveFlag( ParaFlag::ISPAGE );
                else
                    pPara->SetFlag( ParaFlag::ISPAGE );

                rOwner.DepthChangedHdl(pPara, nPrevFlags);
                rOwner.pEditEngine->QuickMarkInvalid(ESelection(nPara, 0));

                if( bUndo )
                    rOwner.InsertUndo( std::make_unique<OutlinerUndoChangeParaFlags>( &rOwner, nPara, nPrevFlags, pPara->nFlags ) );

                continue;
            }
        }

        // do not switch off numeration with tab
        if( (nOldDepth == 0) && (nNewDepth == -1) )
            continue;

        // do not indent if there is no numeration enabled
        if( nOldDepth == -1 )
            continue;

        if ( nNewDepth < Outliner::gnMinDepth )
            nNewDepth = Outliner::gnMinDepth;
        if ( nNewDepth > rOwner.nMaxDepth )
            nNewDepth = rOwner.nMaxDepth;

        if( nOldDepth < nMinDepth )
            nMinDepth = nOldDepth;
        if( nNewDepth < nMinDepth )
            nMinDepth = nNewDepth;

        if( nOldDepth != nNewDepth )
        {
            if ( ( nPara == aSel.nStartPara ) && aSel.nStartPara && ( rOwner.GetOutlinerMode() != OutlinerMode::TextObject ))
            {
                // Special case: the predecessor of an indented paragraph is
                // invisible and is now on the same level as the visible
                // paragraph. In this case, the next visible paragraph is
                // searched for and fluffed.
#ifdef DBG_UTIL
                Paragraph* _pPara = rOwner.pParaList->GetParagraph( aSel.nStartPara );
                DBG_ASSERT(_pPara->IsVisible(),"Selected Paragraph invisible ?!");
#endif
                Paragraph* pPrev= rOwner.pParaList->GetParagraph( aSel.nStartPara-1 );

                if( !pPrev->IsVisible() && ( pPrev->GetDepth() == nNewDepth ) )
                {
                    // Predecessor is collapsed and is on the same level
                    // => find next visible paragraph and expand it
                    pPrev = rOwner.pParaList->GetParent( pPrev );
                    while( !pPrev->IsVisible() )
                        pPrev = rOwner.pParaList->GetParent( pPrev );

                    rOwner.Expand( pPrev );
                    rOwner.InvalidateBullet(rOwner.pParaList->GetAbsPos(pPrev));
                }
            }

            rOwner.nDepthChangedHdlPrevDepth = nOldDepth;
            ParaFlag nPrevFlags = pPara->nFlags;

            rOwner.ImplInitDepth( nPara, nNewDepth, true );
            rOwner.ImplCalcBulletText( nPara, false, false );

            if ( rOwner.GetOutlinerMode() == OutlinerMode::OutlineObject )
                rOwner.ImplSetLevelDependentStyleSheet( nPara );

            // Notify App
            rOwner.DepthChangedHdl(pPara, nPrevFlags);
        }
        else
        {
            // Needs at least a repaint...
            rOwner.pEditEngine->QuickMarkInvalid(ESelection(nPara, 0));
        }
    }

    sal_Int32 nParas = rOwner.pParaList->GetParagraphCount();
    for ( sal_Int32 n = aSel.nEndPara+1; n < nParas; n++ )
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( n );
        if ( pPara->GetDepth() < nMinDepth )
            break;
        rOwner.ImplCalcBulletText( n, false, false );
    }

    if ( bUpdate )
    {
        pEditView->SetEditEngineUpdateLayout( true );
        pEditView->ShowCursor();
    }

    if( bUndo )
        rOwner.UndoActionEnd();
}

void OutlinerView::AdjustHeight( tools::Long nDY )
{
    pEditView->MoveParagraphs( nDY );
}

tools::Rectangle OutlinerView::GetVisArea() const
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
    ImplExpandOrCollaps( 0, rOwner.pParaList->GetParagraphCount()-1, true );
}


void OutlinerView::CollapseAll()
{
    ImplExpandOrCollaps( 0, rOwner.pParaList->GetParagraphCount()-1, false );
}

void OutlinerView::ImplExpandOrCollaps( sal_Int32 nStartPara, sal_Int32 nEndPara, bool bExpand )
{
    bool bUpdate = rOwner.SetUpdateLayout( false );

    bool bUndo = !rOwner.IsInUndo() && rOwner.IsUndoEnabled();
    if( bUndo )
        rOwner.UndoActionStart( bExpand ? OLUNDO_EXPAND : OLUNDO_COLLAPSE );

    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
        bool bDone = bExpand ? rOwner.Expand( pPara ) : rOwner.Collapse( pPara );
        if( bDone )
        {
            // The line under the paragraph should disappear ...
            rOwner.pEditEngine->QuickMarkToBeRepainted( nPara );
        }
    }

    if( bUndo )
        rOwner.UndoActionEnd();

    if ( bUpdate )
    {
        rOwner.SetUpdateLayout( true );
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

    if ( ImpCalcSelectedPages( false ) && !rOwner.ImpCanDeleteSelectedPages( this ) )
        return;

    rOwner.UndoActionStart( OLUNDO_INSERT );

    const bool bPrevUpdateLayout = rOwner.pEditEngine->SetUpdateLayout( false );
    sal_Int32 nStart, nParaCount;
    nParaCount = rOwner.pEditEngine->GetParagraphCount();
    sal_uInt16 nSize = ImpInitPaste( nStart );
    pEditView->InsertText( rParaObj.GetTextObject() );
    ImpPasted( nStart, nParaCount, nSize);
    pEditView->SetEditEngineUpdateLayout( bPrevUpdateLayout );

    rOwner.UndoActionEnd();

    pEditView->ShowCursor();
}


void OutlinerView::Cut()
{
    if ( !ImpCalcSelectedPages( false ) || rOwner.ImpCanDeleteSelectedPages( this ) ) {
        pEditView->Cut();
        // Chaining handling
        aEndCutPasteLink.Call(nullptr);
    }
}

void OutlinerView::PasteSpecial(SotClipboardFormatId format)
{
    Paste( true, format );
}

void OutlinerView::Paste( bool bUseSpecial, SotClipboardFormatId format)
{
    if ( ImpCalcSelectedPages( false ) && !rOwner.ImpCanDeleteSelectedPages( this ) )
        return;

    rOwner.UndoActionStart( OLUNDO_INSERT );

    const bool bPrevUpdateLayout = rOwner.pEditEngine->SetUpdateLayout( false );
    rOwner.bPasting = true;

    if ( bUseSpecial )
        pEditView->PasteSpecial(format);
    else
        pEditView->Paste();

    if ( rOwner.GetOutlinerMode() == OutlinerMode::OutlineObject )
    {
        const sal_Int32 nParaCount = rOwner.pEditEngine->GetParagraphCount();

        for( sal_Int32 nPara = 0; nPara < nParaCount; nPara++ )
            rOwner.ImplSetLevelDependentStyleSheet( nPara );
    }

    pEditView->SetEditEngineUpdateLayout( bPrevUpdateLayout );
    rOwner.UndoActionEnd();
    pEditView->ShowCursor();

    // Chaining handling
    // NOTE: We need to do this last because it pEditView may be deleted if a switch of box occurs
    aEndCutPasteLink.Call(nullptr);
}

void OutlinerView::CreateSelectionList (std::vector<Paragraph*> &aSelList)
{
    ParaRange aParas = ImpGetSelectedParagraphs( true );

    for ( sal_Int32 nPara = aParas.nStartPara; nPara <= aParas.nEndPara; nPara++ )
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
        aSelList.push_back(pPara);
    }
}

void OutlinerView::SetStyleSheet(const OUString& rStyleName)
{
    ParaRange aParas = ImpGetSelectedParagraphs(false);

    auto pStyle = rOwner.GetStyleSheetPool()->Find(rStyleName, SfxStyleFamily::Para);
    if (!pStyle)
        return;

    for (sal_Int32 nPara = aParas.nStartPara; nPara <= aParas.nEndPara; nPara++)
        rOwner.SetStyleSheet(nPara, static_cast<SfxStyleSheet*>(pStyle));
}

const SfxStyleSheet* OutlinerView::GetStyleSheet() const
{
    return pEditView->GetStyleSheet();
}

SfxStyleSheet* OutlinerView::GetStyleSheet()
{
    return pEditView->GetStyleSheet();
}

PointerStyle OutlinerView::GetPointer( const Point& rPosPixel )
{
    MouseTarget eTarget;
    ImpCheckMousePos( rPosPixel, eTarget );

    PointerStyle ePointerStyle = PointerStyle::Arrow;
    if ( eTarget == MouseTarget::Text )
    {
        ePointerStyle = GetOutliner().IsVertical() ? PointerStyle::TextVertical : PointerStyle::Text;
    }
    else if ( eTarget == MouseTarget::Hypertext )
    {
        ePointerStyle = PointerStyle::RefHand;
    }
    else if ( eTarget == MouseTarget::Bullet )
    {
        ePointerStyle = PointerStyle::Move;
    }

    return ePointerStyle;
}


sal_Int32 OutlinerView::ImpInitPaste( sal_Int32& rStart )
{
    rOwner.bPasting = true;
    ESelection aSelection( pEditView->GetSelection() );
    aSelection.Adjust();
    rStart = aSelection.start.nPara;
    sal_Int32 nSize = aSelection.end.nPara - aSelection.start.nPara + 1;
    return nSize;
}


void OutlinerView::ImpPasted( sal_Int32 nStart, sal_Int32 nPrevParaCount, sal_Int32 nSize)
{
    rOwner.bPasting = false;
    sal_Int32 nCurParaCount = rOwner.pEditEngine->GetParagraphCount();
    if( nCurParaCount < nPrevParaCount )
        nSize = nSize - ( nPrevParaCount - nCurParaCount );
    else
        nSize = nSize + ( nCurParaCount - nPrevParaCount );
    rOwner.ImpTextPasted( nStart, nSize );
}

bool OutlinerView::Command(const CommandEvent& rCEvt)
{
    return pEditView->Command(rCEvt);
}

void OutlinerView::SelectRange( sal_Int32 nFirst, sal_Int32 nCount )
{
    ESelection aSel(nFirst, 0, nFirst + nCount, EE_TEXTPOS_MAX);
    pEditView->SetSelection( aSel );
}


sal_Int32 OutlinerView::ImpCalcSelectedPages( bool bIncludeFirstSelected )
{
    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    sal_Int32 nPages = 0;
    sal_Int32 nFirstPage = EE_PARA_MAX;
    sal_Int32 nStartPara = aSel.start.nPara;
    if ( !bIncludeFirstSelected )
        nStartPara++;   // All paragraphs after StartPara will be deleted
    for (sal_Int32 nPara = nStartPara; nPara <= aSel.end.nPara; nPara++)
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
        assert(pPara && "ImpCalcSelectedPages: invalid Selection?");
        if( pPara->HasFlag(ParaFlag::ISPAGE) )
        {
            nPages++;
            if (nFirstPage == EE_PARA_MAX)
                nFirstPage = nPara;
        }
    }

    if( nPages )
    {
        rOwner.nDepthChangedHdlPrevDepth = nPages;
        rOwner.mnFirstSelPage = nFirstPage;
    }

    return nPages;
}


void OutlinerView::ToggleBullets()
{
    rOwner.UndoActionStart( OLUNDO_DEPTH );

    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    const bool bUpdate = rOwner.pEditEngine->SetUpdateLayout( false );

    sal_Int16 nNewDepth = -2;
    const SvxNumRule* pDefaultBulletNumRule = nullptr;

    for (sal_Int32 nPara = aSel.start.nPara; nPara <= aSel.end.nPara; nPara++)
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::ToggleBullets(), illegal selection?");

        if( pPara )
        {
            if( nNewDepth == -2 )
            {
                nNewDepth = (rOwner.GetDepth(nPara) == -1) ? 0 : -1;
                if ( nNewDepth == 0 )
                {
                    // determine default numbering rule for bullets
                    const ESelection aSelection(nPara, 0);
                    const SfxItemSet aTmpSet(rOwner.pEditEngine->GetAttribs(aSelection));
                    const SfxPoolItem& rPoolItem = aTmpSet.GetPool()->GetUserOrPoolDefaultItem( EE_PARA_NUMBULLET );
                    const SvxNumBulletItem* pNumBulletItem = dynamic_cast< const SvxNumBulletItem* >(&rPoolItem);
                    pDefaultBulletNumRule =  pNumBulletItem ? &pNumBulletItem->GetNumRule() : nullptr;
                }
            }

            rOwner.SetDepth( pPara, nNewDepth );

            if( nNewDepth == -1 )
            {
                const SfxItemSet& rAttrs = rOwner.GetParaAttribs( nPara );
                if ( rAttrs.GetItemState( EE_PARA_BULLETSTATE ) == SfxItemState::SET )
                {
                    SfxItemSet aAttrs(rAttrs);
                    aAttrs.ClearItem( EE_PARA_BULLETSTATE );
                    rOwner.SetParaAttribs( nPara, aAttrs );
                }
            }
            else
            {
                if ( pDefaultBulletNumRule )
                {
                    const SvxNumberFormat* pFmt = rOwner.GetNumberFormat( nPara );
                    if ( !pFmt
                         || ( pFmt->GetNumberingType() != SVX_NUM_BITMAP
                              && pFmt->GetNumberingType() != SVX_NUM_CHAR_SPECIAL ) )
                    {
                        SfxItemSet aAttrs( rOwner.GetParaAttribs( nPara ) );
                        SvxNumRule aNewNumRule( *pDefaultBulletNumRule );
                        aAttrs.Put( SvxNumBulletItem( std::move(aNewNumRule), EE_PARA_NUMBULLET ) );
                        rOwner.SetParaAttribs( nPara, aAttrs );
                    }
                }
            }
        }
    }

    const sal_Int32 nParaCount = rOwner.pParaList->GetParagraphCount();
    rOwner.ImplCheckParagraphs(aSel.start.nPara, nParaCount);

    sal_Int32 nEndPara = (nParaCount > 0) ? nParaCount-1 : nParaCount;
    rOwner.pEditEngine->QuickMarkInvalid(ESelection(aSel.start.nPara, 0, nEndPara, 0));

    rOwner.pEditEngine->SetUpdateLayout( bUpdate );

    rOwner.UndoActionEnd();
}

bool OutlinerView::IsBulletOrNumbering(bool& bBullets, bool& bNumbering)
{
    //TODO: returns true if the same list is active in the selection,
    // sets bBullets/bNumbering if the related list type is found
    bool bBulletFound = false;
    bool bNumberingFound = false;

    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();
    for (sal_Int32 nPara = aSel.start.nPara; nPara <= aSel.end.nPara; nPara++)
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::IsBulletOrNumbering(), illegal selection?");

        if( pPara )
        {
            if (rOwner.GetDepth(nPara) < 0)
                return false;
            const SvxNumberFormat* pFmt = rOwner.GetNumberFormat(nPara);
            if (pFmt)
            {
                sal_Int16 nNumType = pFmt->GetNumberingType();
                if (nNumType != SVX_NUM_BITMAP && nNumType != SVX_NUM_CHAR_SPECIAL)
                    bNumberingFound = true;
                else
                    bBulletFound = true;
            }
        }
    }
    if (bNumberingFound)
    {
        if (bBulletFound)
            return false;
        bNumbering = true;
    }
    else
        bBullets = true;
    return true;
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
        const sal_Int16 nBulletNumberingStatus( rOwner.GetBulletsNumberingStatus( aSel.start.nPara, aSel.end.nPara ) );
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
}

void OutlinerView::EnsureNumberingIsOn()
{
    rOwner.UndoActionStart(OLUNDO_DEPTH);

    ESelection aSel(pEditView->GetSelection());
    aSel.Adjust();

    const bool bUpdate = rOwner.pEditEngine->IsUpdateLayout();
    rOwner.pEditEngine->SetUpdateLayout(false);

    for (sal_Int32 nPara = aSel.start.nPara; nPara <= aSel.end.nPara; nPara++)
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph(nPara);
        DBG_ASSERT(pPara, "OutlinerView::EnableBullets(), illegal selection?");

        if (pPara && rOwner.GetDepth(nPara) == -1)
            rOwner.SetDepth(pPara, 0);
    }

    sal_Int32 nParaCount = rOwner.pParaList->GetParagraphCount();
    rOwner.ImplCheckParagraphs(aSel.start.nPara, nParaCount);

    const sal_Int32 nEndPara = (nParaCount > 0) ? nParaCount-1 : nParaCount;
    rOwner.pEditEngine->QuickMarkInvalid(ESelection(aSel.start.nPara, 0, nEndPara, 0));

    rOwner.pEditEngine->SetUpdateLayout(bUpdate);

    rOwner.UndoActionEnd();
}

void OutlinerView::ApplyBulletsNumbering(
    const bool bHandleBullets,
    const SvxNumRule* pNewNumRule,
    const bool bCheckCurrentNumRuleBeforeApplyingNewNumRule,
    const bool bAtSelection )
{
    if (!rOwner.pEditEngine || !rOwner.pParaList)
        return;

    rOwner.UndoActionStart(OLUNDO_DEPTH);
    const bool bUpdate = rOwner.pEditEngine->SetUpdateLayout(false);

    sal_Int32 nStartPara = 0;
    sal_Int32 nEndPara = 0;
    if ( bAtSelection )
    {
        ESelection aSel( pEditView->GetSelection() );
        aSel.Adjust();
        nStartPara = aSel.start.nPara;
        nEndPara = aSel.end.nPara;
    }
    else
    {
        nStartPara = 0;
        nEndPara = rOwner.pParaList->GetParagraphCount() - 1;
    }

    for (sal_Int32 nPara = nStartPara; nPara <= nEndPara; ++nPara)
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph(nPara);
        DBG_ASSERT(pPara, "OutlinerView::ApplyBulletsNumbering(..), illegal selection?");

        if (pPara)
        {
            const sal_Int16 nDepth = rOwner.GetDepth(nPara);
            if ( nDepth == -1 )
            {
                rOwner.SetDepth( pPara, 0 );
            }

            const SfxItemSet& rAttrs = rOwner.GetParaAttribs(nPara);
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
                    const SvxNumberFormat* pFmt = rOwner.GetNumberFormat(nPara);
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
                        const SvxNumBulletItem* pNumBulletItem = rAttrs.GetItemIfSet(EE_PARA_NUMBULLET, false);
                        if (pNumBulletItem)
                        {
                            // Use default value when has not contain bullet item.
                            ESelection aSelection(nPara, 0);
                            SfxItemSet aTmpSet(rOwner.pEditEngine->GetAttribs(aSelection));
                            pNumBulletItem = aTmpSet.GetItem(EE_PARA_NUMBULLET);
                        }

                        if (pNumBulletItem)
                        {
                            const sal_uInt16 nLevelCnt = std::min(pNumBulletItem->GetNumRule().GetLevelCount(), aNewRule.GetLevelCount());
                            for ( sal_uInt16 nLevel = 0; nLevel < nLevelCnt; ++nLevel )
                            {
                                const SvxNumberFormat* pOldFmt = pNumBulletItem->GetNumRule().Get(nLevel);
                                const SvxNumberFormat* pNewFmt = aNewRule.Get(nLevel);
                                if (pOldFmt && pNewFmt && (pOldFmt->GetFirstLineOffset() != pNewFmt->GetFirstLineOffset() || pOldFmt->GetAbsLSpace() != pNewFmt->GetAbsLSpace()))
                                {
                                    SvxNumberFormat aNewFmtClone(*pNewFmt);
                                    aNewFmtClone.SetFirstLineOffset(pOldFmt->GetFirstLineOffset());
                                    aNewFmtClone.SetAbsLSpace(pOldFmt->GetAbsLSpace());
                                    aNewRule.SetLevel(nLevel, &aNewFmtClone);
                                }
                            }
                        }
                    }

                    aAttrs.Put(SvxNumBulletItem(std::move(aNewRule), EE_PARA_NUMBULLET));
                }
            }
            rOwner.SetParaAttribs(nPara, aAttrs);
        }
    }

    const sal_uInt16 nParaCount = static_cast<sal_uInt16>(rOwner.pParaList->GetParagraphCount());
    rOwner.ImplCheckParagraphs( nStartPara, nParaCount );
    rOwner.pEditEngine->QuickMarkInvalid( ESelection( nStartPara, 0, nParaCount, 0 ) );

    rOwner.pEditEngine->SetUpdateLayout( bUpdate );

    rOwner.UndoActionEnd();
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
        nStartPara = aSel.start.nPara;
        nEndPara = aSel.end.nPara;
    }
    else
    {
        nStartPara = 0;
        nEndPara = rOwner.pParaList->GetParagraphCount() - 1;
    }

    rOwner.UndoActionStart( OLUNDO_DEPTH );
    const bool bUpdate = rOwner.pEditEngine->SetUpdateLayout( false );

    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; ++nPara )
    {
        Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::SwitchOffBulletsNumbering(...), illegal paragraph index?");

        if( pPara )
        {
            rOwner.SetDepth( pPara, -1 );

            const SfxItemSet& rAttrs = rOwner.GetParaAttribs( nPara );
            if (rAttrs.GetItemState( EE_PARA_BULLETSTATE ) == SfxItemState::SET)
            {
                SfxItemSet aAttrs(rAttrs);
                aAttrs.ClearItem( EE_PARA_BULLETSTATE );
                rOwner.SetParaAttribs( nPara, aAttrs );
            }
        }
    }

    const sal_uInt16 nParaCount = static_cast<sal_uInt16>(rOwner.pParaList->GetParagraphCount());
    rOwner.ImplCheckParagraphs( nStartPara, nParaCount );
    rOwner.pEditEngine->QuickMarkInvalid( ESelection( nStartPara, 0, nParaCount, 0 ) );

    rOwner.pEditEngine->SetUpdateLayout( bUpdate );
    rOwner.UndoActionEnd();
}


void OutlinerView::RemoveAttribsKeepLanguages( bool bRemoveParaAttribs )
{
    RemoveAttribs( bRemoveParaAttribs, true /*keep language attribs*/ );
}

void OutlinerView::RemoveAttribs( bool bRemoveParaAttribs, bool bKeepLanguages )
{
    bool bUpdate = rOwner.SetUpdateLayout( false );
    rOwner.UndoActionStart( OLUNDO_ATTR );
    if (bKeepLanguages)
        pEditView->RemoveAttribsKeepLanguages( bRemoveParaAttribs );
    else
        pEditView->RemoveAttribs( bRemoveParaAttribs );
    if ( bRemoveParaAttribs )
    {
        // Loop through all paragraphs and set indentation and level
        ESelection aSel = pEditView->GetSelection();
        aSel.Adjust();
        for (sal_Int32 nPara = aSel.start.nPara; nPara <= aSel.end.nPara; nPara++)
        {
            Paragraph* pPara = rOwner.pParaList->GetParagraph( nPara );
            rOwner.ImplInitDepth( nPara, pPara->GetDepth(), false );
        }
    }
    rOwner.UndoActionEnd();
    rOwner.SetUpdateLayout( bUpdate );
}


// ======================   Simple pass-through   =======================


void OutlinerView::InsertText( const OUString& rNew, bool bSelect )
{
    if( rOwner.bFirstParaIsEmpty )
        rOwner.Insert( OUString() );
    pEditView->InsertText( rNew, bSelect );
}

void OutlinerView::SetVisArea( const tools::Rectangle& rRect )
{
    pEditView->SetVisArea( rRect );
}


void OutlinerView::SetSelection( const ESelection& rSel )
{
    pEditView->SetSelection( rSel );
}

void OutlinerView::GetSelectionRectangles(std::vector<tools::Rectangle>& rLogicRects) const
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

bool OutlinerView::IsCursorVisible() const { return pEditView->IsCursorVisible(); }

void OutlinerView::SetWindow( vcl::Window* pWin )
{
    pEditView->SetWindow( pWin );
}

vcl::Window* OutlinerView::GetWindow() const
{
    return pEditView->GetWindow();
}

void OutlinerView::SetOutputArea( const tools::Rectangle& rRect )
{
    pEditView->SetOutputArea( rRect );
}

tools::Rectangle const & OutlinerView::GetOutputArea() const
{
    return pEditView->GetOutputArea();
}

OUString OutlinerView::GetSelected() const
{
    return pEditView->GetSelected();
}

void OutlinerView::StartSpeller(weld::Widget* pDialogParent)
{
    pEditView->StartSpeller(pDialogParent);
}

EESpellState OutlinerView::StartThesaurus(weld::Widget* pDialogParent)
{
    return pEditView->StartThesaurus(pDialogParent);
}

void OutlinerView::StartTextConversion(weld::Widget* pDialogParent,
    LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont,
    sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc )
{
    if (
        (LANGUAGE_KOREAN == nSrcLang && LANGUAGE_KOREAN == nDestLang) ||
        (LANGUAGE_CHINESE_SIMPLIFIED  == nSrcLang && LANGUAGE_CHINESE_TRADITIONAL == nDestLang) ||
        (LANGUAGE_CHINESE_TRADITIONAL == nSrcLang && LANGUAGE_CHINESE_SIMPLIFIED  == nDestLang)
       )
    {
        pEditView->StartTextConversion(pDialogParent, nSrcLang, nDestLang, pDestFont, nOptions, bIsInteractive, bMultipleDoc);
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

void OutlinerView::TransliterateText( TransliterationFlags nTransliterationMode )
{
    pEditView->TransliterateText( nTransliterationMode );
}

ESelection OutlinerView::GetSelection() const
{
    return pEditView->GetSelection();
}


void OutlinerView::Scroll( tools::Long nHorzScroll, tools::Long nVertScroll )
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

void OutlinerView::SetAnchorMode( EEAnchorMode eMode )
{
    pEditView->SetAnchorMode( eMode );
}

EEAnchorMode OutlinerView::GetAnchorMode() const
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

const SvxFieldItem* OutlinerView::GetFieldAtSelection(bool bAlsoCheckBeforeCursor) const
{
    return pEditView->GetFieldAtSelection(bAlsoCheckBeforeCursor);
}

void OutlinerView::SelectFieldAtCursor()
{
    pEditView->SelectFieldAtCursor();
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


bool OutlinerView::IsWrongSpelledWordAtPos( const Point& rPosPixel )
{
    return pEditView->IsWrongSpelledWordAtPos( rPosPixel, /*bMarkIfWrong*/false );
}

void OutlinerView::ExecuteSpellPopup(const Point& rPosPixel, const Link<SpellCallbackInfo&,void>& rStartDlg)
{
    pEditView->ExecuteSpellPopup(rPosPixel, rStartDlg);
}

void OutlinerView::Read( SvStream& rInput, EETextFormat eFormat, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    sal_Int32 nOldParaCount = pEditView->getEditEngine().GetParagraphCount();
    ESelection aOldSel = pEditView->GetSelection();
    aOldSel.Adjust();

    pEditView->Read( rInput, eFormat, pHTTPHeaderAttrs );

    tools::Long nParaDiff = pEditView->getEditEngine().GetParagraphCount() - nOldParaCount;
    sal_Int32 nChangesStart = aOldSel.start.nPara;
    sal_Int32 nChangesEnd = nChangesStart + nParaDiff + (aOldSel.end.nPara-aOldSel.start.nPara);

    for ( sal_Int32 n = nChangesStart; n <= nChangesEnd; n++ )
    {
        if ( rOwner.GetOutlinerMode() == OutlinerMode::OutlineObject )
            rOwner.ImplSetLevelDependentStyleSheet( n );
    }

    rOwner.ImpFilterIndents( nChangesStart, nChangesEnd );
}

void OutlinerView::SetBackgroundColor( const Color& rColor )
{
    pEditView->SetBackgroundColor( rColor );
}

void OutlinerView::RegisterViewShell(OutlinerViewShell* pViewShell)
{
    pEditView->RegisterViewShell(pViewShell);
}

Color const & OutlinerView::GetBackgroundColor() const
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

bool OutlinerView::DeleteSurroundingText(const Selection& rSelection)
{
    return pEditView->DeleteSurroundingText(rSelection);
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
    EditEngine& rEditEngine = rEditView.getEditEngine();
    ESelection aTextSel( rEditView.GetSelection() );
    if (!aTextSel.HasRange())
        aTextSel = rEditEngine.GetWord( aTextSel, i18n::WordType::DICTIONARY_WORD );
    aText = rEditEngine.GetText( aTextSel );
    aTextSel.Adjust();

    if (!isSingleScriptType(rEditEngine.GetScriptType(aTextSel)))
        return false;

    LanguageType nLang = rEditEngine.GetLanguage(aTextSel.start.nPara, aTextSel.start.nIndex).nLang;
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
