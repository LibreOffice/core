/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove

#include "precompiled_editeng.hxx"

#include <com/sun/star/i18n/WordType.hpp>

#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>

#include <svl/style.hxx>
#include <i18npool/mslangid.hxx>

#define _OUTLINER_CXX
#include <editeng/outliner.hxx>
#include <outleeng.hxx>
#include <paralist.hxx>
#include <outlundo.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/flditem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/numitem.hxx>
#include <vcl/window.hxx>
#include <svl/itemset.hxx>
#include <editeng/editstat.hxx>


// Breite der Randzonen innerhalb derer beim D&D gescrollt wird
#define OL_SCROLL_LRBORDERWIDTHPIX  10
#define OL_SCROLL_TBBORDERWIDTHPIX  10

// Wert, um den Fensterinhalt beim D&D gescrollt wird
#define OL_SCROLL_HOROFFSET         20  /* in % von VisibleSize.Width */
#define OL_SCROLL_VEROFFSET         20  /* in % von VisibleSize.Height */

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

void OutlinerView::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS(OutlinerView,0);

    // beim ersten Paint/KeyInput/Drop wird aus einem leeren Outliner ein
    // Outliner mit genau einem Absatz
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( String() );

    pEditView->Paint( rRect );
}

sal_Bool OutlinerView::PostKeyEvent( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS( OutlinerView, 0 );

    // beim ersten Paint/KeyInput/Drop wird aus einem leeren Outliner ein
    // Outliner mit genau einem Absatz
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
            default:    // wird dann evtl. unten bearbeitet.
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
                              !bSelection && !aSel.nEndPos && pOwner->ImplHasBullet( aSel.nEndPara ) )
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
                    // Sonderbehandlung: Hartes Return am Ende eines Absatzes,
                    // der eingeklappte Unterabsaetze besitzt
                    Paragraph* pPara = pOwner->pParaList->GetParagraph( aSel.nEndPara );

                    if( !aKeyCode.IsShift() )
                    {
                        // Nochmal ImpGetCursor ???
                        if( !bSelection &&
                                aSel.nEndPos == pOwner->pEditEngine->GetTextLen( aSel.nEndPara ) )
                        {
                            sal_uLong nChilds = pOwner->pParaList->GetChildCount(pPara);
                            if( nChilds && !pOwner->pParaList->HasVisibleChilds(pPara))
                            {
                                pOwner->UndoActionStart( OLUNDO_INSERT );
                                sal_uLong nTemp = aSel.nEndPara;
                                nTemp += nChilds;
                                nTemp++; // einfuegen ueber naechstem Non-Child
                                pOwner->Insert( String(),nTemp,pPara->GetDepth());
                                // Cursor positionieren
                                ESelection aTmpSel((sal_uInt16)nTemp,0,(sal_uInt16)nTemp,0);
                                pEditView->SetSelection( aTmpSel );
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
                        sal_uLong nTemp = aSel.nEndPara;
                        nTemp++;
                        pOwner->Insert( String(), nTemp, pPara->GetDepth()+1 );

                        // Cursor positionieren
                        ESelection aTmpSel((sal_uInt16)nTemp,0,(sal_uInt16)nTemp,0);
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

    return bKeyProcessed ? sal_True : pEditView->PostKeyEvent( rKEvt );
}


sal_uLong OutlinerView::ImpCheckMousePos(const Point& rPosPix, MouseTarget& reTarget)
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_uLong nPara = EE_PARA_NOT_FOUND;

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

sal_Bool __EXPORT OutlinerView::MouseMove( const MouseEvent& rMEvt )
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


sal_Bool __EXPORT OutlinerView::MouseButtonDown( const MouseEvent& rMEvt )
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
    sal_uLong nPara = ImpCheckMousePos( rMEvt.GetPosPixel(), eTarget );
    if ( eTarget == MouseBullet )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        sal_Bool bHasChilds = (pPara && pOwner->pParaList->HasChilds(pPara));
        if( rMEvt.GetClicks() == 1 )
        {
            sal_uLong nEndPara = nPara;
            if ( bHasChilds && pOwner->pParaList->HasVisibleChilds(pPara) )
                nEndPara += pOwner->pParaList->GetChildCount( pPara );
            // umgekehrt rum selektieren, damit EditEngine nicht scrollt
            ESelection aSel((sal_uInt16)nEndPara, 0xffff,(sal_uInt16)nPara, 0 );
            pEditView->SetSelection( aSel );
        }
        else if( rMEvt.GetClicks() == 2 && bHasChilds )
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
        if( (pPara && pOwner->pParaList->HasChilds(pPara)) && pPara->HasFlag(PARAFLAG_ISPAGE) )
        {
            ImpToggleExpand( pPara );
        }
    }
    return pEditView->MouseButtonDown( rMEvt );
}


sal_Bool __EXPORT OutlinerView::MouseButtonUp( const MouseEvent& rMEvt )
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

void OutlinerView::ImpHideDDCursor()
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( bDDCursorVisible )
    {
        bDDCursorVisible = sal_False;
        ImpPaintDDCursor();
    }
}

void OutlinerView::ImpShowDDCursor()
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( !bDDCursorVisible )
    {
        bDDCursorVisible = sal_True;
        ImpPaintDDCursor();
    }
}

void OutlinerView::ImpPaintDDCursor()
{
    DBG_CHKTHIS(OutlinerView,0);

    Window* pWindow = pEditView->GetWindow();
    RasterOp eOldOp = pWindow->GetRasterOp();
    pWindow->SetRasterOp( ROP_INVERT );

    const Color& rOldLineColor = pWindow->GetLineColor();
    pWindow->SetLineColor( Color( COL_BLACK ) );

    Point aStartPointWin, aEndPointWin;
    Rectangle aOutputArWin = pEditView->GetOutputArea();
    Rectangle aVisAreaRef = pEditView->GetVisArea();

    if( bDDChangingDepth )
    {
        aStartPointWin.X() = pHorTabArrDoc[ nDDCurDepth ];
        aStartPointWin.X() += aOutputArWin.Left();
        aStartPointWin.Y() = aOutputArWin.Top();
        aEndPointWin.X() = aStartPointWin.X();
        aEndPointWin.Y() = aOutputArWin.Bottom();
    }
    else
    {
        sal_uLong nPara = nDDCurPara;
        if ( nDDCurPara == LIST_APPEND )
        {
            Paragraph* pTemp = pOwner->pParaList->LastVisible();
            nPara = pOwner->pParaList->GetAbsPos( pTemp );
        }
        aStartPointWin = pEditView->GetWindowPosTopLeft((sal_uInt16) nPara );
        if ( nDDCurPara == LIST_APPEND )
        {
            long nHeight = pOwner->pEditEngine->GetTextHeight((sal_uInt16)nPara );
            aStartPointWin.Y() += nHeight;
        }
        aStartPointWin.X() = aOutputArWin.Left();
        aEndPointWin.Y() = aStartPointWin.Y();
        aEndPointWin.X() = aOutputArWin.Right();
    }

    pWindow->DrawLine( aStartPointWin, aEndPointWin );
    pWindow->SetLineColor( rOldLineColor );
    pWindow->SetRasterOp( eOldOp );
}

// Berechnet, ueber welchem Absatz eingefuegt werden muss

sal_uLong OutlinerView::ImpGetInsertionPara( const Point& rPosPixel  )
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_uLong nCurPara = pEditView->GetParagraph( rPosPixel );
    ParagraphList* pParaList = pOwner->pParaList;

    if ( nCurPara == EE_PARA_NOT_FOUND )
        nCurPara = LIST_APPEND;
    else
    {
        Point aPosWin = pEditView->GetWindow()->PixelToLogic( rPosPixel );
        Point aParaPosWin = pEditView->GetWindowPosTopLeft((sal_uInt16)nCurPara);
        long nHeightRef = pOwner->pEditEngine->GetTextHeight((sal_uInt16)nCurPara);
        long nParaYOffs = aPosWin.Y() - aParaPosWin.Y();

        if ( nParaYOffs > nHeightRef / 2  )
        {
            Paragraph* p = pParaList->GetParagraph( nCurPara );
            p = pParaList->NextVisible( p );
            nCurPara = p ? pParaList->GetAbsPos( p ) : LIST_APPEND;
        }
    }
    return nCurPara;
}


void OutlinerView::ImpToggleExpand( Paragraph* pPara )
{
    DBG_CHKTHIS(OutlinerView,0);

    sal_uInt16 nPara = (sal_uInt16) pOwner->pParaList->GetAbsPos( pPara );
    pEditView->SetSelection( ESelection( nPara, 0, nPara, 0 ) );
    ImplExpandOrCollaps( nPara, nPara, !pOwner->pParaList->HasVisibleChilds( pPara ) );
    pEditView->ShowCursor();
}


void OutlinerView::SetOutliner( Outliner* pOutliner )
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner = pOutliner;
    pEditView->SetEditEngine( pOutliner->pEditEngine );
}



sal_uLong OutlinerView::Select( Paragraph* pParagraph, sal_Bool bSelect,
    sal_Bool bWithChilds )
{
    DBG_CHKTHIS(OutlinerView,0);

    sal_uLong nPara = pOwner->pParaList->GetAbsPos( pParagraph );
    sal_uInt16 nEnd = 0;
    if ( bSelect )
        nEnd = 0xffff;

    sal_uLong nChildCount = 0;
    if ( bWithChilds )
        nChildCount = pOwner->pParaList->GetChildCount( pParagraph );

    ESelection aSel( (sal_uInt16)nPara, 0,(sal_uInt16)(nPara+nChildCount), nEnd );
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

    // Bullet-Texte aktualisieren
    for( sal_uInt16 nPara= aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
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

ParaRange OutlinerView::ImpGetSelectedParagraphs( sal_Bool bIncludeHiddenChilds )
{
    DBG_CHKTHIS( OutlinerView, 0 );

    ESelection aSel = pEditView->GetSelection();
    ParaRange aParas( aSel.nStartPara, aSel.nEndPara );
    aParas.Adjust();

    // unsichtbare Childs des letzten Parents in Selektion mit aufnehmen
    if ( bIncludeHiddenChilds )
    {
        Paragraph* pLast = pOwner->pParaList->GetParagraph( aParas.nEndPara );
        if ( pOwner->pParaList->HasHiddenChilds( pLast ) )
            aParas.nEndPara =
                sal::static_int_cast< sal_uInt16 >(
                    aParas.nEndPara +
                    pOwner->pParaList->GetChildCount( pLast ) );
    }
    return aParas;
}

// MT: Name sollte mal geaendert werden!
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

    sal_Int16 nMinDepth = -1;   // Optimierung: Nicht unnoetig viele Absatze neu berechnen

    ParaRange aSel = ImpGetSelectedParagraphs( sal_True );
    for ( sal_uInt16 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );

        sal_Int16 nOldDepth = pPara->GetDepth();
        sal_Int16 nNewDepth = nOldDepth + nDiff;

        if( bOutlinerView && nPara )
        {
            const bool bPage = pPara->HasFlag(PARAFLAG_ISPAGE);
            if( (bPage && (nDiff == +1)) || (!bPage && (nDiff == -1) && (nOldDepth <= 0))  )
            {
                // App benachrichtigen
                pOwner->nDepthChangedHdlPrevDepth = (sal_Int16)nOldDepth;
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
                // Sonderfall: Der Vorgaenger eines eingerueckten Absatzes ist
                // unsichtbar und steht jetzt auf der gleichen Ebene wie der
                // sichtbare Absatz. In diesem Fall wird der naechste sichtbare
                // Absatz gesucht und aufgeplustert.
#ifdef DBG_UTIL
                Paragraph* _pPara = pOwner->pParaList->GetParagraph( aSel.nStartPara );
                DBG_ASSERT(_pPara->IsVisible(),"Selected Paragraph invisible ?!");
#endif
                Paragraph* pPrev= pOwner->pParaList->GetParagraph( aSel.nStartPara-1 );

                if( !pPrev->IsVisible() && ( pPrev->GetDepth() == nNewDepth ) )
                {
                    // Vorgaenger ist eingeklappt und steht auf gleicher Ebene
                    // => naechsten sichtbaren Absatz suchen und expandieren
                    pPrev = pOwner->pParaList->GetParent( pPrev );
                    while( !pPrev->IsVisible() )
                        pPrev = pOwner->pParaList->GetParent( pPrev );

                    pOwner->Expand( pPrev );
                    pOwner->InvalidateBullet( pPrev, pOwner->pParaList->GetAbsPos( pPrev ) );
                }
            }

            pOwner->nDepthChangedHdlPrevDepth = (sal_Int16)nOldDepth;
            pOwner->mnDepthChangeHdlPrevFlags = pPara->nFlags;
            pOwner->pHdlParagraph = pPara;

            pOwner->ImplInitDepth( nPara, nNewDepth, sal_True, sal_False );
            pOwner->ImplCalcBulletText( nPara, sal_False, sal_False );

            if ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT )
                pOwner->ImplSetLevelDependendStyleSheet( nPara );

            // App benachrichtigen
            pOwner->DepthChangedHdl();
        }
        else
        {
            // Needs at least a repaint...
            pOwner->pEditEngine->QuickMarkInvalid( ESelection( nPara, 0, nPara, 0 ) );
        }
    }

    // MT 19.08.99: War mal fuer Optimierung (outliner.cxx#1.193),
    // hat aber zu zuviel Wartungsaufwand / doppelten Funktionen gefuehrt
    // und zu wenig gebracht:
    // pOwner->ImpSetBulletTextsFrom( aSel.nStartPara+1, nMinDepth );
    // Wird jetzt direkt in Schleife mit ImplCalcBulletText() erledigt.
    // Jetzt fehlen nur noch die folgenden Ansaetze, die davon betroffen sind.
    sal_uInt16 nParas = (sal_uInt16)pOwner->pParaList->GetParagraphCount();
    for ( sal_uInt16 n = aSel.nEndPara+1; n < nParas; n++ )
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

void OutlinerView::AdjustDepth( Paragraph* pPara, short nDX, sal_Bool bWithChilds)
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_uLong nStartPara = pOwner->pParaList->GetAbsPos( pPara );
    sal_uLong nEndPara = nStartPara;
    if ( bWithChilds )
        nEndPara += pOwner->pParaList->GetChildCount( pPara );
    ESelection aSel((sal_uInt16)nStartPara, 0,(sal_uInt16)nEndPara, 0xffff );
    pEditView->SetSelection( aSel );
    AdjustDepth( nDX );
}

void OutlinerView::AdjustHeight( Paragraph* pPara, long nDY, sal_Bool bWithChilds )
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_uLong nStartPara = pOwner->pParaList->GetAbsPos( pPara );
    sal_uLong nEndPara = nStartPara;
    if ( bWithChilds )
        nEndPara += pOwner->pParaList->GetChildCount( pPara );
    ESelection aSel( (sal_uInt16)nStartPara, 0, (sal_uInt16)nEndPara, 0xffff );
    pEditView->SetSelection( aSel );
    AdjustHeight( nDY );
}


Rectangle OutlinerView::GetVisArea() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetVisArea();
}


Point OutlinerView::ImpGetDocPos( const Point& rPosPixel )
{
    DBG_CHKTHIS(OutlinerView,0);
    Rectangle aOutArWin = GetOutputArea();
    // Position in der OutputArea berechnen
    Point aCurPosDoc( rPosPixel );
    aCurPosDoc = pEditView->GetWindow()->PixelToLogic( aCurPosDoc );
    aCurPosDoc -= aOutArWin.TopLeft();
    aCurPosDoc += pEditView->GetVisArea().TopLeft();
    return aCurPosDoc;
}

// MT 05/00: Wofuer dies ImpXXXScroll, sollte das nicht die EditEngine machen???

void OutlinerView::ImpDragScroll( const Point& rPosPix )
{
    DBG_CHKTHIS(OutlinerView,0);
    Point aPosWin = pEditView->GetWindow()->PixelToLogic( rPosPix );
    Rectangle aOutputArWin = pEditView->GetOutputArea();
    if ( aPosWin.X() <= aOutputArWin.Left() + nDDScrollLRBorderWidthWin)
        ImpScrollLeft();
    else if( aPosWin.X() >= aOutputArWin.Right()- nDDScrollLRBorderWidthWin)
        ImpScrollRight();
    else if( aPosWin.Y() <= aOutputArWin.Top() + nDDScrollTBBorderWidthWin)
        ImpScrollUp();
    else if(aPosWin.Y() >= aOutputArWin.Bottom() - nDDScrollTBBorderWidthWin)
        ImpScrollDown();
}


void OutlinerView::ImpScrollLeft()
{
    DBG_CHKTHIS(OutlinerView,0);
    Rectangle aVisArea( pEditView->GetVisArea() );
    long nMaxScrollOffs = aVisArea.Left();
    if ( !nMaxScrollOffs )
        return;
    long nScrollOffsRef = (aVisArea.GetWidth() * OL_SCROLL_HOROFFSET) / 100;
    if ( !nScrollOffsRef )
        nScrollOffsRef = 1;
    if ( nScrollOffsRef > nMaxScrollOffs )
        nScrollOffsRef = nMaxScrollOffs;

    ImpHideDDCursor();
    Scroll( -nScrollOffsRef, 0 );

    EditStatus aScrollStat;
    aScrollStat.GetStatusWord() = EE_STAT_HSCROLL;
    pOwner->pEditEngine->GetStatusEventHdl().Call( &aScrollStat );
}


void OutlinerView::ImpScrollRight()
{
    DBG_CHKTHIS(OutlinerView,0);
    Rectangle aVisArea( pEditView->GetVisArea() );
    long nMaxScrollOffs = pOwner->pEditEngine->GetPaperSize().Width() -
                          aVisArea.Right();
    if ( !nMaxScrollOffs )
        return;
    long nScrollOffsRef = (aVisArea.GetWidth() * OL_SCROLL_HOROFFSET) / 100;
    if ( !nScrollOffsRef )
        nScrollOffsRef = 1;
    if ( nScrollOffsRef > nMaxScrollOffs )
        nScrollOffsRef = nMaxScrollOffs;

    ImpHideDDCursor();
    Scroll( nScrollOffsRef, 0 );

    EditStatus aScrollStat;
    aScrollStat.GetStatusWord() = EE_STAT_HSCROLL;
    pOwner->pEditEngine->GetStatusEventHdl().Call( &aScrollStat );
}


void OutlinerView::ImpScrollDown()
{
    DBG_CHKTHIS(OutlinerView,0);
    Rectangle aVisArea( pEditView->GetVisArea() );
    Size aDocSize( 0, (long)pOwner->pEditEngine->GetTextHeight() );

    long nMaxScrollOffs = aDocSize.Height();
    nMaxScrollOffs -= aVisArea.Top();
    nMaxScrollOffs -= aVisArea.GetHeight();
    if ( !nMaxScrollOffs )
        return;

    long nScrollOffsRef = (aVisArea.GetHeight() * OL_SCROLL_VEROFFSET) / 100;

    if ( nScrollOffsRef > nMaxScrollOffs )
        nScrollOffsRef = nMaxScrollOffs;
    if ( !nScrollOffsRef )
        nScrollOffsRef = 1;

    ImpHideDDCursor();
    Scroll( 0, -nScrollOffsRef );

    EditStatus aScrollStat;
    aScrollStat.GetStatusWord() = EE_STAT_VSCROLL;
    pOwner->pEditEngine->GetStatusEventHdl().Call( &aScrollStat );
}


void OutlinerView::ImpScrollUp()
{
    DBG_CHKTHIS(OutlinerView,0);
    Rectangle aVisArea( pEditView->GetVisArea() );
    long nMaxScrollOffs = aVisArea.Top();
    if ( !nMaxScrollOffs )
        return;
    long nScrollOffsRef = (aVisArea.GetHeight() * OL_SCROLL_VEROFFSET) / 100;


    if ( nScrollOffsRef > nMaxScrollOffs )
        nScrollOffsRef = nMaxScrollOffs;
    if ( !nScrollOffsRef )
        nScrollOffsRef = 1;

    ImpHideDDCursor();
    Scroll( 0, nScrollOffsRef );

    EditStatus aScrollStat;
    aScrollStat.GetStatusWord() = EE_STAT_VSCROLL;
    pOwner->pEditEngine->GetStatusEventHdl().Call( &aScrollStat );
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
    ImplExpandOrCollaps( 0, (sal_uInt16)(pOwner->pParaList->GetParagraphCount()-1), sal_True );
}


void OutlinerView::CollapseAll()
{
    DBG_CHKTHIS(OutlinerView,0);
    ImplExpandOrCollaps( 0, (sal_uInt16)(pOwner->pParaList->GetParagraphCount()-1), sal_False );
}

void OutlinerView::ImplExpandOrCollaps( sal_uInt16 nStartPara, sal_uInt16 nEndPara, sal_Bool bExpand )
{
    DBG_CHKTHIS( OutlinerView, 0 );

    sal_Bool bUpdate = pOwner->GetUpdateMode();
    pOwner->SetUpdateMode( sal_False );

    sal_Bool bUndo = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();
    if( bUndo )
        pOwner->UndoActionStart( bExpand ? OLUNDO_EXPAND : OLUNDO_COLLAPSE );

    for ( sal_uInt16 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        sal_Bool bDone = bExpand ? pOwner->Expand( pPara ) : pOwner->Collapse( pPara );
        if( bDone )
        {
            // Der Strich unter dem Absatz muss verschwinden...
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


void OutlinerView::Expand( Paragraph* pPara)
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner->Expand( pPara );
}


void OutlinerView::Collapse( Paragraph* pPara)
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner->Collapse( pPara );
}

void OutlinerView::InsertText( const OutlinerParaObject& rParaObj )
{
    // MT: Wie Paste, nur EditView::Insert, statt EditView::Paste.
    // Eigentlich nicht ganz richtig, das evtl. Einrueckungen
    // korrigiert werden muessen, aber das kommt spaeter durch ein
    // allgemeingueltiges Import.
    // Dann wird im Inserted gleich ermittelt, was f�r eine Einrueckebene
    // Moegliche Struktur:
    // pImportInfo mit DestPara, DestPos, nFormat, pParaObj...
    // Evtl. Problematisch:
    // EditEngine, RTF => Absplittung des Bereichs, spaeter
    // zusammenfuehrung

    DBG_CHKTHIS(OutlinerView,0);

    if ( ImpCalcSelectedPages( sal_False ) && !pOwner->ImpCanDeleteSelectedPages( this ) )
        return;

    pOwner->UndoActionStart( OLUNDO_INSERT );

    pOwner->pEditEngine->SetUpdateMode( sal_False );
    sal_uLong nStart, nParaCount;
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
    PasteSpecial(); // HACK(SD ruft nicht PasteSpecial auf)
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
            const sal_uInt16 nParaCount = pOwner->pEditEngine->GetParagraphCount();

            for( sal_uInt16 nPara = 0; nPara < nParaCount; nPara++ )
                pOwner->ImplSetLevelDependendStyleSheet( nPara );
        }

        pEditView->SetEditEngineUpdateMode( sal_True );
        pOwner->UndoActionEnd( OLUNDO_INSERT );
        pEditView->ShowCursor( sal_True, sal_True );
    }
}

List* OutlinerView::CreateSelectionList()
{
    DBG_CHKTHIS( OutlinerView, 0 );

    ParaRange aParas = ImpGetSelectedParagraphs( sal_True );
    List* pSelList = new List;
    for ( sal_uInt16 nPara = aParas.nStartPara; nPara <= aParas.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        pSelList->Insert( pPara, LIST_APPEND );
    }
    return pSelList;
}

SfxStyleSheet* OutlinerView::GetStyleSheet() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetStyleSheet();
}

void OutlinerView::SetStyleSheet( SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetStyleSheet( pStyle );

    ParaRange aSel = ImpGetSelectedParagraphs( sal_True );
    for( sal_uInt16 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        pOwner->ImplCheckNumBulletItem( nPara );
        pOwner->ImplCalcBulletText( nPara, sal_False, sal_False );
    }
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


sal_uInt16 OutlinerView::ImpInitPaste( sal_uLong& rStart )
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner->bPasting = sal_True;
    ESelection aSelection( pEditView->GetSelection() );
    aSelection.Adjust();
    rStart = aSelection.nStartPara;
    sal_uInt16 nSize = aSelection.nEndPara - aSelection.nStartPara + 1;
    return nSize;
}


void OutlinerView::ImpPasted( sal_uLong nStart, sal_uLong nPrevParaCount, sal_uInt16 nSize)
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner->bPasting = sal_False;
    sal_uLong nCurParaCount = (sal_uLong)pOwner->pEditEngine->GetParagraphCount();
    if( nCurParaCount < nPrevParaCount )
        nSize = sal::static_int_cast< sal_uInt16 >(
            nSize - ( nPrevParaCount - nCurParaCount ) );
    else
        nSize = sal::static_int_cast< sal_uInt16 >(
            nSize + ( nCurParaCount - nPrevParaCount ) );
    pOwner->ImpTextPasted( nStart, nSize );
}


void OutlinerView::Command( const CommandEvent& rCEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->Command( rCEvt );
}


void OutlinerView::SelectRange( sal_uLong nFirst, sal_uInt16 nCount )
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_uLong nLast = nFirst+nCount;
    nCount = (sal_uInt16)pOwner->pParaList->GetParagraphCount();
    if( nLast <= nCount )
        nLast = nCount - 1;
    ESelection aSel( (sal_uInt16)nFirst, 0, (sal_uInt16)nLast, 0xffff );
    pEditView->SetSelection( aSel );
}


sal_uInt16 OutlinerView::ImpCalcSelectedPages( sal_Bool bIncludeFirstSelected )
{
    DBG_CHKTHIS(OutlinerView,0);

    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    sal_uInt16 nPages = 0;
    sal_uInt16 nFirstPage = 0xFFFF;
    sal_uInt16 nStartPara = aSel.nStartPara;
    if ( !bIncludeFirstSelected )
        nStartPara++;   // alle nach StartPara kommenden Absaetze werden geloescht
    for ( sal_uInt16 nPara = nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "ImpCalcSelectedPages: ungueltige Selection? ");
        if( pPara->HasFlag(PARAFLAG_ISPAGE) )
        {
            nPages++;
            if( nFirstPage == 0xFFFF )
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

    sal_Int16 nDepth = -2;

    for ( sal_uInt16 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::ToggleBullets(), illegal selection?");

        if( pPara )
        {
            if( nDepth == -2 )
                nDepth = (pOwner->GetDepth(nPara) == -1) ? 0 : -1;

            pOwner->SetDepth( pPara, nDepth );

            if( nDepth == -1 )
            {
                const SfxItemSet& rAttrs = pOwner->GetParaAttribs( nPara );
                if(rAttrs.GetItemState( EE_PARA_BULLETSTATE ) == SFX_ITEM_SET)
                {
                    SfxItemSet aAttrs(rAttrs);
                    aAttrs.ClearItem( EE_PARA_BULLETSTATE );
                    pOwner->SetParaAttribs( nPara, aAttrs );
                }
            }
        }
    }

    // --> OD 2009-03-10 #i100014#
    // It is not a good idea to substract 1 from a count and cast the result
    // to sal_uInt16 without check, if the count is 0.
    sal_uInt16 nParaCount = (sal_uInt16) (pOwner->pParaList->GetParagraphCount());
    // <--
    pOwner->ImplCheckParagraphs( aSel.nStartPara, nParaCount );
    pOwner->pEditEngine->QuickMarkInvalid( ESelection( aSel.nStartPara, 0, nParaCount, 0 ) );

    pOwner->pEditEngine->SetUpdateMode( bUpdate );

    pOwner->UndoActionEnd( OLUNDO_DEPTH );
}

void OutlinerView::EnableBullets()
{
    pOwner->UndoActionStart( OLUNDO_DEPTH );

    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    const bool bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( sal_False );

    for ( sal_uInt16 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "OutlinerView::ToggleBullets(), illegal selection?");

        if( pPara && (pOwner->GetDepth(nPara) == -1) )
        {
            pOwner->SetDepth( pPara, 0 );
        }
    }

    // --> OD 2009-03-10 #i100014#
    // It is not a good idea to substract 1 from a count and cast the result
    // to sal_uInt16 without check, if the count is 0.
    sal_uInt16 nParaCount = (sal_uInt16) (pOwner->pParaList->GetParagraphCount());
    // <--
    pOwner->ImplCheckParagraphs( aSel.nStartPara, nParaCount );
    pOwner->pEditEngine->QuickMarkInvalid( ESelection( aSel.nStartPara, 0, nParaCount, 0 ) );

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
        // Ueber alle Absaetze, und Einrueckung und Level einstellen
        ESelection aSel = pEditView->GetSelection();
        aSel.Adjust();
        for ( sal_uInt16 nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
        {
            Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
            pOwner->ImplInitDepth( nPara, pPara->GetDepth(), sal_False, sal_False );
        }
    }
    pOwner->UndoActionEnd( OLUNDO_ATTR );
    pOwner->SetUpdateMode( bUpdate );
}



// =====================================================================
// ======================   Einfache Durchreicher =======================
// ======================================================================


void OutlinerView::InsertText( const XubString& rNew, sal_Bool bSelect )
{
    DBG_CHKTHIS(OutlinerView,0);
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( String() );
    pEditView->InsertText( rNew, bSelect );
}

void OutlinerView::SetVisArea( const Rectangle& rRec )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetVisArea( rRec );
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


XubString OutlinerView::GetSelected() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetSelected();
}


void OutlinerView::RemoveCharAttribs( sal_uLong nPara, sal_uInt16 nWhich)
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->RemoveCharAttribs( (sal_uInt16)nPara, nWhich);
}


void OutlinerView::CompleteAutoCorrect()
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->CompleteAutoCorrect();
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
        DBG_ERROR( "unexpected language" );
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


void OutlinerView::Undo()
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->Undo();
}


void OutlinerView::Redo()
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->Redo();
}


void OutlinerView::EnablePaste( sal_Bool bEnable )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->EnablePaste( bEnable );
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


const SvxFieldItem* OutlinerView::GetFieldUnderMousePointer( sal_uInt16& nPara, sal_uInt16& nPos ) const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetFieldUnderMousePointer( nPara, nPos );
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


void OutlinerView::SpellIgnoreWord()
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SpellIgnoreWord();
}


void OutlinerView::ExecuteSpellPopup( const Point& rPosPixel, Link* pStartDlg )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->ExecuteSpellPopup( rPosPixel, pStartDlg );
}

sal_uLong OutlinerView::Read( SvStream& rInput,  const String& rBaseURL, EETextFormat eFormat, sal_Bool bSelect, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    DBG_CHKTHIS(OutlinerView,0);
    sal_uInt16 nOldParaCount = pEditView->GetEditEngine()->GetParagraphCount();
    ESelection aOldSel = pEditView->GetSelection();
    aOldSel.Adjust();

    sal_uLong nRet = pEditView->Read( rInput, rBaseURL, eFormat, bSelect, pHTTPHeaderAttrs );

    // MT 08/00: Hier sollte eigentlich das gleiche wie in PasteSpecial passieren!
    // Mal anpassen, wenn dieses ImplInitPaste und ImpPasted-Geraffel ueberarbeitet ist.

    long nParaDiff = pEditView->GetEditEngine()->GetParagraphCount() - nOldParaCount;
    sal_uInt16 nChangesStart = aOldSel.nStartPara;
    sal_uInt16 nChangesEnd = sal::static_int_cast< sal_uInt16 >(nChangesStart + nParaDiff + (aOldSel.nEndPara-aOldSel.nStartPara));

    for ( sal_uInt16 n = nChangesStart; n <= nChangesEnd; n++ )
    {
        if ( eFormat == EE_FORMAT_BIN )
        {
            sal_uInt16 nDepth = 0;
            const SfxItemSet& rAttrs = pOwner->GetParaAttribs( n );
            const SfxInt16Item& rLevel = (const SfxInt16Item&) rAttrs.Get( EE_PARA_OUTLLEVEL );
            nDepth = rLevel.GetValue();
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

sal_uLong OutlinerView::Write( SvStream& rOutput, EETextFormat eFormat )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->Write( rOutput, eFormat );
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

String OutlinerView::GetSurroundingText() const
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
    String &rStatusVal,
    LanguageType &rLang,
    const EditView &rEditView )
{
    // get text and locale for thesaurus look up
    String aText;
    EditEngine *pEditEngine = rEditView.GetEditEngine();
    ESelection aTextSel( rEditView.GetSelection() );
    if (!aTextSel.HasRange())
        aTextSel = pEditEngine->GetWord( aTextSel, i18n::WordType::DICTIONARY_WORD );
    aText = pEditEngine->GetText( aTextSel );
    aTextSel.Adjust();
    LanguageType nLang = pEditEngine->GetLanguage( aTextSel.nStartPara, aTextSel.nStartPos );
    String aLangText( MsLangId::convertLanguageToIsoString( nLang ) );

    // set word and locale to look up as status value
    String aStatusVal( aText );
    aStatusVal.AppendAscii( "#" );
    aStatusVal += aLangText;

    rStatusVal  = aStatusVal;
    rLang       = nLang;

    return aText.Len() > 0;
}


void EDITENG_DLLPUBLIC ReplaceTextWithSynonym( EditView &rEditView, const String &rSynonmText )
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


