/*************************************************************************
 *
 *  $RCSfile: outlvw.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-24 11:30:26 $
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

#include <outl_pch.hxx>

#pragma hdrstop

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#define _OUTLINER_CXX
#include <outliner.hxx>
#include <outleeng.hxx>
#include <paralist.hxx>
#include <outlundo.hxx>
#include <outlobj.hxx>
#include <flditem.hxx>
#include <flditem.hxx>
#include <eeitem.hxx>
#include <numitem.hxx>

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#ifndef _EDITSTAT_HXX //autogen
#include <editstat.hxx>
#endif


// Breite der Randzonen innerhalb derer beim D&D gescrollt wird
#define OL_SCROLL_LRBORDERWIDTHPIX  10
#define OL_SCROLL_TBBORDERWIDTHPIX  10

// Wert, um den Fensterinhalt beim D&D gescrollt wird
#define OL_SCROLL_HOROFFSET         20  /* in % von VisibleSize.Width */
#define OL_SCROLL_VEROFFSET         20  /* in % von VisibleSize.Height */

DBG_NAME(OutlinerView);


OutlinerView::OutlinerView( Outliner* pOut, Window* pWin )
{
    DBG_CTOR( OutlinerView, 0 );

    pOwner                      = pOut;
    ePrevMouseTarget            = MouseDontKnow;
    bBeginDragAtMove            = FALSE;
    bDDCursorVisible            = FALSE;
    bInDragMode                 = FALSE;
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

BOOL OutlinerView::PostKeyEvent( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS( OutlinerView, 0 );

    // beim ersten Paint/KeyInput/Drop wird aus einem leeren Outliner ein
    // Outliner mit genau einem Absatz
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( String() );


    BOOL bKeyProcessed = FALSE;
    ESelection aSel( pEditView->GetSelection() );
    BOOL bSelection = aSel.HasRange();
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    KeyFuncType eFunc = aKeyCode.GetFunction();
    USHORT nCode = aKeyCode.GetCode();
    BOOL bReadOnly = IsReadOnly();

    if( bSelection && EditEngine::DoesKeyChangeText( rKEvt ) )
    {
        if ( ImpCalcSelectedPages( FALSE ) && !pOwner->ImpCanDeleteSelectedPages( this ) )
            return TRUE;
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
                    bKeyProcessed = TRUE;
                }
            }
            break;
            case KEYFUNC_COPY:
            {
                Copy();
                bKeyProcessed = TRUE;
            }
            break;
            case KEYFUNC_PASTE:
            {
                if ( !bReadOnly )
                {
                    PasteSpecial();
                    bKeyProcessed = TRUE;
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
                        if( pNext && pNext->GetDepth() == 0 )
                        {
                            if( !pOwner->ImpCanDeleteSelectedPages( this, aSel.nEndPara, 1 ) )
                                return FALSE;
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
                        if ( aKeyCode.IsShift() )
                            AdjustDepth( -1 );
                        else
                            AdjustDepth( 1 );
                        bKeyProcessed = TRUE;
                    }
                    else if ( ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) &&
                              !bSelection && !aSel.nEndPos && pOwner->ImplHasBullet( aSel.nEndPara ) )
                    {
                        // Ein-Ausruecken...
                        Paragraph* pPara = pOwner->pParaList->GetParagraph( aSel.nEndPara );
                        USHORT nDepth = pPara->GetDepth();
                        if ( nDepth || !aKeyCode.IsShift() )
                        {
                            if ( aKeyCode.IsShift() )
                                nDepth--;
                            else
                                nDepth++;
                            pOwner->ImplInitDepth( aSel.nEndPara, nDepth, TRUE, TRUE );
                            bKeyProcessed = TRUE;
                        }
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
                        return TRUE;
                    if( !pPara->GetDepth() )
                    {
                        if(!pOwner->ImpCanDeleteSelectedPages(this, aSel.nEndPara , 1 ) )
                            return TRUE;
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
                            ULONG nChilds = pOwner->pParaList->GetChildCount(pPara);
                            if( nChilds && !pOwner->pParaList->HasVisibleChilds(pPara))
                            {
                                pOwner->UndoActionStart( OLUNDO_INSERT );
                                ULONG nTemp = aSel.nEndPara;
                                nTemp += nChilds;
                                nTemp++; // einfuegen ueber naechstem Non-Child
                                pOwner->Insert( String(),nTemp,pPara->GetDepth());
                                // Cursor positionieren
                                ESelection aTmpSel((USHORT)nTemp,0,(USHORT)nTemp,0);
                                pEditView->SetSelection( aTmpSel );
                                pEditView->ShowCursor( TRUE, TRUE );
                                pOwner->UndoActionEnd( OLUNDO_INSERT );
                                bKeyProcessed = TRUE;
                            }
                        }
                    }
                    if( !bKeyProcessed && !bSelection &&
                                !aKeyCode.IsShift() && aKeyCode.IsMod1() &&
                            ( aSel.nEndPos == pOwner->pEditEngine->GetTextLen(aSel.nEndPara) ) )
                    {
                        pOwner->UndoActionStart( OLUNDO_INSERT );
                        ULONG nTemp = aSel.nEndPara;
                        nTemp++;
                        pOwner->Insert( String(), nTemp, pPara->GetDepth()+1 );

                        // Cursor positionieren
                        ESelection aTmpSel((USHORT)nTemp,0,(USHORT)nTemp,0);
                        pEditView->SetSelection( aTmpSel );
                        pEditView->ShowCursor( TRUE, TRUE );
                        pOwner->UndoActionEnd( OLUNDO_INSERT );
                        bKeyProcessed = TRUE;
                    }
                }
            }
            break;
        }
    }

    return bKeyProcessed ? TRUE : pEditView->PostKeyEvent( rKEvt );
}


ULONG OutlinerView::ImpCheckMousePos(const Point& rPosPix,MouseTarget& reTarget)
{
    DBG_CHKTHIS(OutlinerView,0);
    ULONG nPara = EE_PARA_NOT_FOUND;

    Point aMousePosWin( rPosPix );
    aMousePosWin = pEditView->GetWindow()->PixelToLogic( rPosPix );

    if( !pEditView->GetOutputArea().IsInside( aMousePosWin ) )
        reTarget = MouseOutside;
    else
    {
        reTarget = MouseText;

        // MT 04/00: ImpEditView::GetDocPos => An EditView anbieten!
        Point aDocPos( aMousePosWin );
        Rectangle aOutArea = pEditView->GetOutputArea();
        Rectangle aVisArea = pEditView->GetVisArea();
        aDocPos.Y() -= aOutArea.Top();
        aDocPos.X() -= aOutArea.Left();
        aDocPos.Y() += aVisArea.Top();
        aDocPos.X() += aVisArea.Left();
        // MT: Dieser Code ist doppelt (Outliner::IsTextPos() )
        // => Mal eine Methode anbieten, jetzt muesste ich dafür aber branchen => spaeter
        // Dann wird wahrscheinlich EditView::GetDocPosTopLeft ueberfluessig
        // Bullet?
        nPara = pOwner->pEditEngine->FindParagraph( aDocPos.Y() );
        if ( ( nPara != EE_PARA_NOT_FOUND ) && pOwner->ImplHasBullet( nPara ) )
        {
            Rectangle aBulArea = pOwner->ImpCalcBulletArea( nPara, TRUE );
            Point aParaXY = pOwner->pEditEngine->GetDocPosTopLeft( nPara );
            aBulArea.Top() += aParaXY.Y();
            aBulArea.Bottom() += aParaXY.Y();
            if ( aBulArea.IsInside( aDocPos ) )
            {
                reTarget = MouseBullet;
            }
        }
    }
    return nPara;
}


Pointer OutlinerView::ImpGetMousePointer( MouseTarget eTarget )
{
    DBG_CHKTHIS(OutlinerView,0);
    // Mapping MouseTarget -> PointerStyle
    static const PointerStyle pStyles[] =
    {
        POINTER_TEXT,       // MouseText
        POINTER_MOVE,       // MouseBullet
        POINTER_ARROW,      // MouseNodeButton
        POINTER_ARROW       // MouseOutside
    };
    return Pointer( pStyles[eTarget] );
}


void OutlinerView::ImpSetMousePointer( MouseTarget eTarget )
{
    DBG_CHKTHIS(OutlinerView,0);
    if( eTarget == MouseOutside )
        ePrevMouseTarget = MouseOutside;
    else if ( eTarget != ePrevMouseTarget )
    {
        ePrevMouseTarget = eTarget;
        pEditView->GetWindow()->SetPointer( ImpGetMousePointer( eTarget ) );
    }
}


BOOL __EXPORT OutlinerView::MouseMove( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    if( ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) || pEditView->GetEditEngine()->IsInSelectionMode())
        return pEditView->MouseMove( rMEvt );

    if ( bBeginDragAtMove )
        return TRUE;
    else
    {
        MouseTarget eTarget;
        ImpCheckMousePos( rMEvt.GetPosPixel(), eTarget );
        ImpSetMousePointer( eTarget );
        if ( eTarget != MouseOutside )
            return pEditView->MouseMove( rMEvt );
    }
    return FALSE;
}


BOOL __EXPORT OutlinerView::MouseButtonDown( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) || pEditView->GetEditEngine()->IsInSelectionMode() )
        return pEditView->MouseButtonDown( rMEvt );

    MouseTarget eTarget;
    ULONG nPara = ImpCheckMousePos( rMEvt.GetPosPixel(), eTarget );
    if ( eTarget == MouseOutside )
        return FALSE;
    if ( nPara == EE_PARA_NOT_FOUND )
        return TRUE;
    ImpSetMousePointer( eTarget );
    if ( eTarget == MouseBullet )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        BOOL bHasChilds = (pPara && pOwner->pParaList->HasChilds(pPara));
        if( rMEvt.GetClicks() == 1 )
        {
            ULONG nEndPara = nPara;
            if ( bHasChilds && pOwner->pParaList->HasVisibleChilds(pPara) )
                nEndPara += pOwner->pParaList->GetChildCount( pPara );
            // umgekehrt rum selektieren, damit EditEngine nicht scrollt
            ESelection aSel((USHORT)nEndPara, 0xffff,(USHORT)nPara, 0 );
            pEditView->SetSelection( aSel );
        }
        else if( rMEvt.GetClicks() == 2 && bHasChilds )
            ImpToggleExpand( pPara );

        bBeginDragAtMove = TRUE;
        aDDStartPosPix = rMEvt.GetPosPixel();
        aDDStartPosRef=pEditView->GetWindow()->PixelToLogic( aDDStartPosPix,pOwner->GetRefMapMode());
        return TRUE;
    }
    return pEditView->MouseButtonDown( rMEvt );
}


BOOL __EXPORT OutlinerView::MouseButtonUp( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) || pEditView->GetEditEngine()->IsInSelectionMode() )
        return pEditView->MouseButtonUp( rMEvt );

    MouseTarget eTarget;
    bBeginDragAtMove = FALSE;
    ImpCheckMousePos( rMEvt.GetPosPixel(), eTarget );
    if ( eTarget == MouseOutside )
        return FALSE;
    ImpSetMousePointer( eTarget );
    return pEditView->MouseButtonUp( rMEvt );
}


void OutlinerView::ImpDrag( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    PointerStyle ePtrStyle;

    bBeginDragAtMove        = FALSE;
    bInDragMode             = TRUE;
    ParagraphList* pPList   = pOwner->pParaList;
    Window* pWindow         = pEditView->GetWindow();

    Point aCurPosRef= pWindow->PixelToLogic(aDDStartPosPix,pOwner->GetRefMapMode());
    nDDCurPara      = pEditView->GetParagraph( aDDStartPosPix );
    nDDStartPara    = nDDCurPara;

    Paragraph* pPara = pPList->GetParagraph( nDDStartPara );
    DBG_ASSERT(pPara,"Drag:No StartPara")
    nDDStartParaVisChildCount = 0;
    if ( pPList->HasVisibleChilds( pPara ) )
        nDDStartParaVisChildCount += pPList->GetChildCount( pPara );

    nDDStartDepth = pPList->GetParagraph(nDDCurPara)->GetDepth();

    // Scroll-Borders in Win-Koordinaten umrechnen
    Point aTempPtWin = Point(OL_SCROLL_LRBORDERWIDTHPIX, OL_SCROLL_TBBORDERWIDTHPIX);
    aTempPtWin = pWindow->PixelToLogic( aTempPtWin );
    nDDScrollLRBorderWidthWin = aTempPtWin.X();
    nDDScrollTBBorderWidthWin = aTempPtWin.Y();

    Point aDocPos = ImpGetDocPos( rMEvt.GetPosPixel() );

    // Feststellen, ob Einruecktiefe oder Hoehe geaendert werden soll
    Point aPosPix( rMEvt.GetPosPixel() );
    long nDX = aPosPix.X() - aDDStartPosPix.X();
    if ( nDX < 0 )
        nDX *= -1;
    long nDY = aPosPix.Y() - aDDStartPosPix.Y();
    if ( nDY < 0 )
        nDY *= -1;

    // MT 07/00: Old implementation was never OK!
    // Change like in PPT: Only with one Paragraph valid
    // => possible positions see SvxNumBulletItem
//  if ( nDX > nDY )
//  {
//      bDDChangingDepth = TRUE;
//      ImpSetHorTabArr();
//      nDDCurDepth = ImpGetDepthArrIdx( aDocPos.X() );
//      ePtrStyle = POINTER_ESIZE;
//  }
//  else
    {
        bDDChangingDepth = FALSE;
        ePtrStyle = POINTER_SSIZE;
    }

    aDDStartPosRef = aCurPosRef;
    pEditView->HideCursor();
    ImpShowDDCursor();
    Region aReg;    // fuer MAC
    Pointer aPtr( ePtrStyle );
    DragServer::Clear();
    // !!!HACK selektierten Text in DragServer stellen, solange
    // Malte noch kein "PutInDragServer" o.ae. zur Verfuegung stellt
    DragServer::CopyString(pEditView->GetSelected());
    USHORT nDragOptions = DRAG_ALL;
    if ( IsReadOnly() )
        nDragOptions &= ~DRAG_MOVEABLE;
    pWindow->ExecuteDrag( aPtr, aPtr, nDragOptions, &aReg );
    ImpHideDDCursor();
    pEditView->ShowCursor( TRUE );
    delete pHorTabArrDoc;
    pHorTabArrDoc = 0;
    bInDragMode = FALSE;
}

void OutlinerView::ImpHideDDCursor()
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( bDDCursorVisible )
    {
        bDDCursorVisible = FALSE;
        ImpPaintDDCursor();
    }
}

void OutlinerView::ImpShowDDCursor()
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( !bDDCursorVisible )
    {
        bDDCursorVisible = TRUE;
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
        ULONG nPara = nDDCurPara;
        if ( nDDCurPara == LIST_APPEND )
        {
            Paragraph* pTemp = pOwner->pParaList->LastVisible();
            nPara = pOwner->pParaList->GetAbsPos( pTemp );
        }
        aStartPointWin = pEditView->GetWindowPosTopLeft((USHORT) nPara );
        if ( nDDCurPara == LIST_APPEND )
        {
            long nHeight = pOwner->pEditEngine->GetTextHeight((USHORT)nPara );
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

ULONG OutlinerView::ImpGetInsertionPara( const Point& rPosPixel  )
{
    DBG_CHKTHIS(OutlinerView,0);
    ULONG nCurPara = pEditView->GetParagraph( rPosPixel );
    ParagraphList* pParaList = pOwner->pParaList;

    if ( nCurPara == EE_PARA_NOT_FOUND )
        nCurPara = LIST_APPEND;
    else
    {
        Point aPosWin = pEditView->GetWindow()->PixelToLogic( rPosPixel );
        Point aParaPosWin = pEditView->GetWindowPosTopLeft((USHORT)nCurPara);
        long nHeightRef = pOwner->pEditEngine->GetTextHeight((USHORT)nCurPara);
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

    USHORT nPara = (USHORT) pOwner->pParaList->GetAbsPos( pPara );
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


ULONG OutlinerView::Select( Paragraph* pParagraph, BOOL bSelect,
    BOOL bWithChilds )
{
    DBG_CHKTHIS(OutlinerView,0);

    ULONG nPara = pOwner->pParaList->GetAbsPos( pParagraph );
    USHORT nEnd = 0;
    if ( bSelect )
        nEnd = 0xffff;

    ULONG nChildCount = 0;
    if ( bWithChilds )
        nChildCount = pOwner->pParaList->GetChildCount( pParagraph );

    ESelection aSel( (USHORT)nPara, 0,(USHORT)(nPara+nChildCount), nEnd );
    pEditView->SetSelection( aSel );
    return nChildCount+1;
}


void OutlinerView::SetAttribs( const SfxItemSet& rAttrs )
{
    DBG_CHKTHIS(OutlinerView,0);

    BOOL bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( FALSE );

    if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
        pOwner->UndoActionStart( OLUNDO_ATTR );

    ParaRange aSel = ImpGetSelectedParagraphs( FALSE );

    if ( rAttrs.GetItemState( EE_PARA_LRSPACE) == SFX_ITEM_ON )
    {
        // Erstmal ohne LRSpace einstellen, damit Konvertierung ins
        // NumBulletItem nur dann, wenn geaendert.
        SfxItemSet aSet( rAttrs );
        aSet.ClearItem( EE_PARA_LRSPACE );
        pEditView->SetAttribs( aSet );

        // Jetzt ggf. LRSpace in NumBulletItem mergen...
        EditEngine* pEditEng = pOwner->pEditEngine;
        const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&) rAttrs.Get( EE_PARA_LRSPACE );
        for ( USHORT n = aSel.nStartPara; n <= aSel.nEndPara; n++ )
        {
            SfxItemSet aAttribs = pEditEng->GetParaAttribs( n );
            if ( !( rLR == pEditEng->GetParaAttrib( (USHORT)n, EE_PARA_LRSPACE ) ) )
            {
                const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&)pEditEng->GetParaAttrib( (USHORT)n, EE_PARA_NUMBULLET );
                Paragraph* pPara = pOwner->pParaList->GetParagraph( n );
                if ( rNumBullet.GetNumRule()->GetLevelCount() > pPara->GetDepth() )
                {
                    SvxNumBulletItem* pNewNumBullet = (SvxNumBulletItem*) rNumBullet.Clone();
                    EditEngine::ImportBulletItem( *pNewNumBullet, pPara->GetDepth(), NULL, &rLR );
                    aAttribs.Put( *pNewNumBullet );
                    delete pNewNumBullet;
                }
            }
            aAttribs.Put( rLR );
            pEditEng->SetParaAttribs( (USHORT)n, aAttribs );
        }
    }
    else
    {
        pEditView->SetAttribs( rAttrs );
    }

    // Bullet-Texte aktualisieren
    for( USHORT nPara= aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        pOwner->ImplCheckNumBulletItem( nPara );
        pOwner->ImplCalcBulletText( nPara, FALSE, FALSE );

        if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
            pOwner->InsertUndo( new OutlinerUndoCheckPara( pOwner, nPara ) );
    }

    if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
        pOwner->UndoActionEnd( OLUNDO_ATTR );

    pEditView->SetEditEngineUpdateMode( bUpdate );
}

ParaRange OutlinerView::ImpGetSelectedParagraphs( BOOL bIncludeHiddenChilds )
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
            aParas.nEndPara += (USHORT) pOwner->pParaList->GetChildCount( pLast );
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

    if( !nDiff || ( ( nDiff > 0 ) && ImpCalcSelectedPages( TRUE ) && !pOwner->ImpCanIndentSelectedPages( this ) ) )
        return;

    BOOL bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( FALSE );

    BOOL bUndo = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();

    if( bUndo )
        pOwner->UndoActionStart( OLUNDO_DEPTH );

    USHORT nMinDepth = 0xFFFF;  // Optimierung: Nicht unnoetig viele Absatze neu berechnen

    ParaRange aSel = ImpGetSelectedParagraphs( TRUE );
    for ( USHORT nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        if ( !nPara && !pPara->GetDepth() )
        {
            // Seite 0 nicht einruecken.
            // Absatz muss neu gepaintet werden (wg. doppeltem Highlight beim Painten der Selektion )
            pOwner->pEditEngine->QuickMarkInvalid( ESelection( 0, 0, 0, 0 ) );
            continue;
        }

        USHORT nOldDepth = pPara->GetDepth();
        USHORT nNewDepth = nOldDepth + nDiff;
        if ( ( nDiff < 0 ) && ( nOldDepth < (-nDiff) ) )
            nNewDepth = 0;

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
            if ( ( nPara == aSel.nStartPara ) && aSel.nStartPara )
            {
                // Sonderfall: Der Vorgaenger eines eingerueckten Absatzes ist
                // unsichtbar und steht jetzt auf der gleichen Ebene wie der
                // sichtbare Absatz. In diesem Fall wird der naechste sichtbare
                // Absatz gesucht und aufgeplustert.
                Paragraph* pPara = pOwner->pParaList->GetParagraph( aSel.nStartPara );
                DBG_ASSERT(pPara->IsVisible(),"Selected Paragraph invisible ?!")

                Paragraph* pPrev= pOwner->pParaList->GetParagraph( aSel.nStartPara-1 );

                if( !pPrev->IsVisible() && ( pPrev->GetDepth() == nNewDepth ) )
                {
                    // Vorgaenger ist eingeklappt und steht auf gleicher Ebene
                    // => naechsten sichtbaren Absatz suchen und expandieren
                    USHORT nDummy;
                    pPrev = pOwner->pParaList->GetParent( pPrev, nDummy );
                    while( !pPrev->IsVisible() )
                        pPrev = pOwner->pParaList->GetParent( pPrev, nDummy );

                    pOwner->Expand( pPrev );
                    pOwner->InvalidateBullet( pPrev, pOwner->pParaList->GetAbsPos( pPrev ) );
                }
            }

            pOwner->ImplInitDepth( nPara, nNewDepth, TRUE, FALSE );
            pOwner->ImplCalcBulletText( nPara, FALSE, FALSE );

            if ( pOwner->ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT )
                pOwner->ImplSetLevelDependendStyleSheet( nPara );

            // App benachrichtigen
            pOwner->nDepthChangedHdlPrevDepth = (USHORT)nOldDepth;
            pOwner->pHdlParagraph = pPara;
            pOwner->DepthChangedHdl();
        }
    }

    // MT 19.08.99: War mal fuer Optimierung (outliner.cxx#1.193),
    // hat aber zu zuviel Wartungsaufwand / doppelten Funktionen gefuehrt
    // und zu wenig gebracht:
    // pOwner->ImpSetBulletTextsFrom( aSel.nStartPara+1, nMinDepth );
    // Wird jetzt direkt in Schleife mit ImplCalcBulletText() erledigt.
    // Jetzt fehlen nur noch die folgenden Ansaetze, die davon betroffen sind.
    USHORT nParas = (USHORT)pOwner->pParaList->GetParagraphCount();
    for ( USHORT n = aSel.nEndPara+1; n < nParas; n++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( n );
        if ( pPara->GetDepth() < nMinDepth )
            break;
        pOwner->ImplCalcBulletText( n, FALSE, FALSE );
    }

    if ( bUpdate )
    {
        pEditView->SetEditEngineUpdateMode( TRUE );
        pEditView->ShowCursor();
    }

    if( bUndo )
        pOwner->UndoActionEnd( OLUNDO_DEPTH );
}

/* Soll Ersatz fuer AdjustHeight werden
void OutlinerView::MoveParagraph( USHORT nPara, short nDiff, BOOL bWithChilds )
{
    if ( !nDiff || ( ( nDiff < 0 ) && ( (-nDiff) > nPara ) ) )
        return;

    BOOL bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( FALSE );

    BOOL bUndo = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();
    if ( bUndo )
    {
        pOwner->UndoActionStart( OLUNDO_MOVEPARAGRAPHS );
        OutlinerUndoMoveParagraphs* pUndo = new OutlinerUndoMoveParagraphs( nPara, nEndPara, nDiff );
    }

    pOwner->GetBeginMovingHdl().Call( pOwner );

}
*/

BOOL OutlinerView::AdjustHeight( long nDY )
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( !nDY )
        return TRUE;

    BOOL bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( FALSE );

    OLUndoHeight* pUndo = NULL;
    if( !pOwner->IsInUndo() && pOwner->IsUndoEnabled() )
    {
        pOwner->UndoActionStart( OLUNDO_HEIGHT );
        pUndo = new OLUndoHeight( pOwner );
        pUndo->nAbsCount = (USHORT)(pOwner->pParaList->GetParagraphCount());
        pUndo->ppBulletTexts = pOwner->ImpCreateBulletArray();
        pUndo->pDepths = pOwner->ImpCreateDepthArray();
        pOwner->InsertUndo( pUndo );
    }

    pOwner->GetBeginMovingHdl().Call( pOwner );

    ParaRange aSel = ImpGetSelectedParagraphs( TRUE );

    if ( nDY > 0 )
    {
        Paragraph* pLastPara = pOwner->pParaList->GetParagraph( aSel.nEndPara );
        Paragraph* pInsertionPara = pLastPara;
        nDY++;  // Para finden, _vor_ dem wir einfuegen muessen
        long nDYTemp = nDY;
        while( nDYTemp && pInsertionPara )
        {
            pInsertionPara = pOwner->pParaList->NextVisible( pInsertionPara );
            nDYTemp--;
        }
        ULONG nInsertionPos = LIST_APPEND;
        if ( pInsertionPara )
            nInsertionPos = pOwner->pParaList->GetAbsPos( pInsertionPara );
        if ( aSel.nStartPara != nInsertionPos )
        {
            pOwner->pParaList->MoveParagraphs( aSel.nStartPara, nInsertionPos, aSel.Len() );
            pEditView->MoveParagraphs( Range( aSel.nStartPara, aSel.nEndPara ), (USHORT)nInsertionPos );
            // pOwner->ImpSetBulletTextsFrom( nChangesStart, 0 );
            USHORT nChangesStart = Min( (USHORT)aSel.nStartPara, (USHORT)nInsertionPos );
            USHORT nParas = (USHORT)pOwner->pParaList->GetParagraphCount();
            for ( USHORT n = nChangesStart; n < nParas; n++ )
                pOwner->ImplCalcBulletText( n, FALSE, FALSE );
        }
    }
    else
    {
        Paragraph* pInsertionPara = pOwner->pParaList->GetParagraph( aSel.nStartPara );
        long nDYTemp = -1 * nDY;
        while( nDYTemp && pInsertionPara )
        {
            pInsertionPara = pOwner->pParaList->PrevVisible( pInsertionPara );
            nDYTemp--;
        }
        ULONG nInsertionPos = 0;
        if ( pInsertionPara )
            nInsertionPos = pOwner->pParaList->GetAbsPos( pInsertionPara );
        if ( aSel.nStartPara != nInsertionPos )
        {
            pOwner->pParaList->MoveParagraphs( aSel.nStartPara, nInsertionPos, aSel.Len() );
            pEditView->MoveParagraphs( Range( aSel.nStartPara, aSel.nEndPara ), (USHORT)nInsertionPos);
            // pOwner->ImpSetBulletTextsFrom( nChangesStart, 0 );
            USHORT nChangesStart = Min( (USHORT)aSel.nStartPara, (USHORT)nInsertionPos );
            USHORT nParas = (USHORT)pOwner->pParaList->GetParagraphCount();
            for ( USHORT n = nChangesStart; n < nParas; n++ )
                pOwner->ImplCalcBulletText( n, FALSE, FALSE );
        }

        // ersten Absatz immer auf Ebene 0 stellen
        Paragraph* pStartPara = pOwner->pParaList->GetParagraph( 0 );
        if( pStartPara->GetDepth() != pOwner->GetMinDepth() )
            pOwner->SetDepth( pStartPara, pOwner->GetMinDepth() );
    }

    pEditView->SetEditEngineUpdateMode( bUpdate );
    pEditView->ShowCursor();

    pOwner->aEndMovingHdl.Call( pOwner );

    if ( pUndo )
        pOwner->UndoActionEnd( OLUNDO_HEIGHT );

    return TRUE;
}

void OutlinerView::AdjustDepth( Paragraph* pPara, short nDX, BOOL bWithChilds)
{
    DBG_CHKTHIS(OutlinerView,0);
    ULONG nStartPara = pOwner->pParaList->GetAbsPos( pPara );
    ULONG nEndPara = nStartPara;
    if ( bWithChilds )
        nEndPara += pOwner->pParaList->GetChildCount( pPara );
    ESelection aSel((USHORT)nStartPara, 0,(USHORT)nEndPara, 0xffff );
    pEditView->SetSelection( aSel );
    AdjustDepth( nDX );
}


void OutlinerView::AdjustHeight( Paragraph* pPara, long nDY, BOOL bWithChilds )
{
    DBG_CHKTHIS(OutlinerView,0);
    ULONG nStartPara = pOwner->pParaList->GetAbsPos( pPara );
    ULONG nEndPara = nStartPara;
    if ( bWithChilds )
        nEndPara += pOwner->pParaList->GetChildCount( pPara );
    ESelection aSel( (USHORT)nStartPara, 0, (USHORT)nEndPara, 0xffff );
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


BOOL __EXPORT OutlinerView::QueryDrop( DropEvent& rDEvt )
{
    DBG_CHKTHIS(OutlinerView,0);
    BOOL bRetVal = TRUE;

    if ( IsReadOnly() )
        return FALSE;

    if ( !bInDragMode )
        return pEditView->QueryDrop( rDEvt );

    if( rDEvt.IsLeaveWindow() )
    {
        ImpHideDDCursor();
        return FALSE;
    }

    Point aCurPosDoc = ImpGetDocPos( rDEvt.GetPosPixel() );

    ImpDragScroll( rDEvt.GetPosPixel() );

    if ( bDDChangingDepth )
    {
        // NIY
    }
    else
    {
        ULONG nNewPara = ImpGetInsertionPara( rDEvt.GetPosPixel() );
        ULONG nMaxPara = nDDStartPara+nDDStartParaVisChildCount+1;
        Range aRange( nDDStartPara, nMaxPara );
        if ( aRange.IsInside( nNewPara ) ||
                (nMaxPara==pOwner->pParaList->GetParagraphCount() &&
                 nNewPara == LIST_APPEND ))
            bRetVal = FALSE;
        if ( !bDDCursorVisible || nNewPara != nDDCurPara )
        {
            ImpHideDDCursor();
            nDDCurPara = nNewPara;
            ImpShowDDCursor();
        }
    }
    return bRetVal;
}


BOOL __EXPORT OutlinerView::Drop( const DropEvent& rDEvt )
{
    DBG_CHKTHIS(OutlinerView,0);

    // beim ersten Paint/KeyInput/Drop wird aus einem leeren Outliner ein
    // Outliner mit genau einem Absatz
    if( pOwner->bFirstParaIsEmpty )
        pOwner->Insert( String() );

    if ( !bInDragMode )
    {
        //  if( !ImpIsRemovingPages() )
        if ( ImpCalcSelectedPages( FALSE ) && !pOwner->ImpCanDeleteSelectedPages( this ) )
            return FALSE;
        // UpdateMode auf False ist im Drop ist verboten
        pOwner->pEditEngine->SetUpdateMode( FALSE );
        pOwner->bPasting = TRUE;
        ESelection aSel = pEditView->GetDropPos();
        Paragraph* pPara = pOwner->pParaList->GetParagraph(aSel.nStartPara);
        DBG_ASSERT(pPara,"DropTarget not found");
        USHORT nOldFlags = pPara->nFlags;
        pPara->nFlags |= PARAFLAG_DROPTARGET;
        // leere Absaetze nach dem Drop voll attributieren, da die
        // EditEngine in diesem Fall die Para-Attribs der Drop-Source
        // uebernimmt
        if( pOwner->pEditEngine->GetTextLen( aSel.nStartPara ) == 0 )
            pPara->nFlags |= PARAFLAG_DROPTARGET_EMPTY;
        BOOL bDone = pEditView->Drop( rDEvt );
        pOwner->bPasting = FALSE;
        if ( !bDone )
        {
            pPara->nFlags = nOldFlags;
            pOwner->pEditEngine->SetUpdateMode( TRUE );
            return FALSE;
        }
        pOwner->ImpDropped( this );
        pOwner->pEditEngine->SetUpdateMode( TRUE );
        return TRUE;
    }
    ImpHideDDCursor();

    long nStart, nEnd, nDiff;

    if ( bDDChangingDepth )
    {
        nEnd = (long)nDDCurDepth;
        nStart = (long)nDDStartDepth;
        nDiff = nEnd - nStart;
        AdjustDepth( (short)nDiff );
    }
    else
    {
        Paragraph* pTemp = pOwner->pParaList->GetParagraph( nDDStartPara );
        nStart = (long)(pOwner->pParaList->GetVisPos( pTemp ));
        if ( nDDCurPara != LIST_APPEND )
        {
            pTemp = pOwner->pParaList->GetParagraph( nDDCurPara );
            nEnd = (long)(pOwner->pParaList->GetVisPos( pTemp ));
        }
        else
        {
            nEnd = (long)(pOwner->pParaList->GetParagraphCount());
            nEnd++;
        }
        if ( nEnd > nStart )
        {
            // Berechnen, um wieviel sichtbare Absaetze der
            // _Block_ nach unten verschoben werden muss
            nStart += (long)nDDStartParaVisChildCount;
            nDiff = nEnd - nStart;
            nDiff--;
        }
        else
            nDiff = nEnd - nStart;

        AdjustHeight( nDiff );
    }
    return TRUE;
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
    ParaRange aParas = ImpGetSelectedParagraphs( FALSE );
    ImplExpandOrCollaps( aParas.nStartPara, aParas.nEndPara, TRUE );
}


void OutlinerView::Collapse()
{
    DBG_CHKTHIS( OutlinerView, 0 );
    ParaRange aParas = ImpGetSelectedParagraphs( FALSE );
    ImplExpandOrCollaps( aParas.nStartPara, aParas.nEndPara, FALSE );
}


void OutlinerView::ExpandAll()
{
    DBG_CHKTHIS( OutlinerView, 0 );
    ImplExpandOrCollaps( 0, (USHORT)(pOwner->pParaList->GetParagraphCount()-1), TRUE );
}


void OutlinerView::CollapseAll()
{
    DBG_CHKTHIS(OutlinerView,0);
    ImplExpandOrCollaps( 0, (USHORT)(pOwner->pParaList->GetParagraphCount()-1), FALSE );
}

void OutlinerView::ImplExpandOrCollaps( USHORT nStartPara, USHORT nEndPara, BOOL bExpand )
{
    DBG_CHKTHIS( OutlinerView, 0 );

    BOOL bUpdate = pOwner->GetUpdateMode();
    pOwner->SetUpdateMode( FALSE );

    BOOL bUndo = !pOwner->IsInUndo() && pOwner->IsUndoEnabled();
    if( bUndo )
        pOwner->UndoActionStart( bExpand ? OLUNDO_EXPAND : OLUNDO_COLLAPSE );

    for ( USHORT nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        BOOL bDone = bExpand ? pOwner->Expand( pPara ) : pOwner->Collapse( pPara );
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
        pOwner->SetUpdateMode( TRUE );
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
    // Dann wird im Inserted gleich ermittelt, was fr eine Einrueckebene
    // Moegliche Struktur:
    // pImportInfo mit DestPara, DestPos, nFormat, pParaObj...
    // Evtl. Problematisch:
    // EditEngine, RTF => Absplittung des Bereichs, spaeter
    // zusammenfuehrung

    DBG_CHKTHIS(OutlinerView,0);

    if ( ImpCalcSelectedPages( FALSE ) && !pOwner->ImpCanDeleteSelectedPages( this ) )
        return;

    pOwner->UndoActionStart( OLUNDO_INSERT );

    pOwner->pEditEngine->SetUpdateMode( FALSE );
    ULONG nStart, nParaCount;
    nParaCount = pOwner->pEditEngine->GetParagraphCount();
    USHORT nSize = ImpInitPaste( nStart );
    pEditView->InsertText( rParaObj.GetTextObject() );
    ImpPasted( nStart, nParaCount, nSize);
    pEditView->SetEditEngineUpdateMode( TRUE );

    pOwner->UndoActionEnd( OLUNDO_INSERT );

    pEditView->ShowCursor( TRUE, TRUE );
}



void OutlinerView::Cut()
{
    DBG_CHKTHIS(OutlinerView,0);
    if ( !ImpCalcSelectedPages( FALSE ) || pOwner->ImpCanDeleteSelectedPages( this ) )
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
    if ( !ImpCalcSelectedPages( FALSE ) || pOwner->ImpCanDeleteSelectedPages( this ) )
    {
        pOwner->UndoActionStart( OLUNDO_INSERT );

        pOwner->pEditEngine->SetUpdateMode( FALSE );
        ULONG nStart, nParaCount;
        nParaCount = pOwner->pEditEngine->GetParagraphCount();
        USHORT nSize = ImpInitPaste( nStart );
        pEditView->PasteSpecial();
        ImpPasted( nStart, nParaCount, nSize);
        pEditView->SetEditEngineUpdateMode( TRUE );

        pOwner->UndoActionEnd( OLUNDO_INSERT );

        pEditView->ShowCursor( TRUE, TRUE );
    }
}

List* OutlinerView::CreateSelectionList()
{
    DBG_CHKTHIS( OutlinerView, 0 );

    ParaRange aParas = ImpGetSelectedParagraphs( TRUE );
    List* pSelList = new List;
    for ( USHORT nPara = aParas.nStartPara; nPara <= aParas.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        pSelList->Insert( pPara, LIST_APPEND );
    }
    return pSelList;
}

/*
XubString OutlinerView::GetStyleSheet()
{
    DBG_CHKTHIS(OutlinerView,0);
    SfxStyleFamily aFamily;
    XubString aName;
    pEditView->GetStyleSheet( aName, aFamily );
    return aName;
}
*/

SfxStyleSheet* OutlinerView::GetStyleSheet() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetStyleSheet();
}

void OutlinerView::SetStyleSheet( SfxStyleSheet* pStyle )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetStyleSheet( pStyle );

    ParaRange aSel = ImpGetSelectedParagraphs( TRUE );
    for( USHORT nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        pOwner->ImplCheckNumBulletItem( nPara );
        pOwner->ImplCalcBulletText( nPara, FALSE, FALSE );
    }
}

Pointer OutlinerView::GetPointer( const Point& rPosPixel )
{
    DBG_CHKTHIS(OutlinerView,0);
    MouseTarget eTarget;
    ImpCheckMousePos( rPosPixel, eTarget );
    Pointer aPointer = ImpGetMousePointer( eTarget );
    Point aPos = rPosPixel;
    aPos = pEditView->GetWindow()->PixelToLogic( aPos );
    const SvxFieldItem* pField = pEditView->GetField( aPos );
    if ( pField && pField->GetField() && pField->GetField()->ISA( SvxURLField ) )
        aPointer = Pointer( POINTER_REFHAND );
    return aPointer;
}


USHORT OutlinerView::ImpInitPaste( ULONG& rStart )
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner->bPasting = TRUE;
    ESelection aSelection( pEditView->GetSelection() );
    aSelection.Adjust();
    rStart = aSelection.nStartPara;
    USHORT nSize = aSelection.nEndPara - aSelection.nStartPara + 1;
    return nSize;
}


void OutlinerView::ImpPasted( ULONG nStart, ULONG nPrevParaCount, USHORT nSize)
{
    DBG_CHKTHIS(OutlinerView,0);
    pOwner->bPasting = FALSE;
    ULONG nCurParaCount = (ULONG)pOwner->pEditEngine->GetParagraphCount();
    if( nCurParaCount < nPrevParaCount )
        nSize -= (USHORT)( nPrevParaCount - nCurParaCount );
    else
        nSize += (USHORT)( nCurParaCount - nPrevParaCount );
    pOwner->ImpTextPasted( nStart, nSize );
}


void OutlinerView::Command( const CommandEvent& rCEvt )
{
    DBG_CHKTHIS(OutlinerView,0);

    if( ( rCEvt.GetCommand() == COMMAND_STARTDRAG ) && bBeginDragAtMove )
    {
        // ImpDrag braucht Abstand von MouseButtonDown-Position
        Point aPos( pEditView->GetWindow()->GetPointerPosPixel() );
        aPos = pEditView->GetWindow()->ScreenToOutputPixel( aPos );
        MouseEvent aEvt( aPos );
        ImpDrag( aEvt );
    }
    else
        pEditView->Command( rCEvt );
}


void OutlinerView::SelectRange( ULONG nFirst, USHORT nCount )
{
    DBG_CHKTHIS(OutlinerView,0);
    ULONG nLast = nFirst+nCount;
    nCount = (USHORT)pOwner->pParaList->GetParagraphCount();
    if( nLast <= nCount )
        nLast = nCount - 1;
    ESelection aSel( (USHORT)nFirst, 0, (USHORT)nLast, 0xffff );
    pEditView->SetSelection( aSel );
}


USHORT OutlinerView::ImpCalcSelectedPages( BOOL bIncludeFirstSelected )
{
    DBG_CHKTHIS(OutlinerView,0);

    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    USHORT nPages = 0;
    USHORT nFirstPage = 0xFFFF;
    USHORT nStartPara = aSel.nStartPara;
    if ( !bIncludeFirstSelected )
        nStartPara++;   // alle nach StartPara kommenden Absaetze werden geloescht
    for ( USHORT nPara = nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "ImpCalcSelectedPages: ungueltige Selection? ");
        if( pPara->GetDepth() == 0 )
        {
            nPages++;
            if( nFirstPage == 0xFFFF )
                nFirstPage = nPara;
        }
    }

    if( nPages )
    {
        pOwner->nDepthChangedHdlPrevDepth = nPages;
        pOwner->pHdlParagraph = (Paragraph*)nFirstPage;
    }

    return nPages;
}


void OutlinerView::ShowBullets( BOOL bShow, BOOL bAffectLevel0 )
{
    pOwner->UndoActionStart( OLUNDO_ATTR );

    ESelection aSel( pEditView->GetSelection() );
    aSel.Adjust();

    BOOL bUpdate = pOwner->pEditEngine->GetUpdateMode();
    pOwner->pEditEngine->SetUpdateMode( FALSE );

    for ( USHORT nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
    {
        Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
        DBG_ASSERT(pPara, "ShowBullets: ungueltige Selection? ");
        if( pPara && ( bAffectLevel0 || pPara->GetDepth() ) )
        {
            SfxItemSet aAttribs( pOwner->pEditEngine->GetParaAttribs( nPara ) );
            BOOL bVis = ((const SfxUInt16Item&)aAttribs.Get( EE_PARA_BULLETSTATE )).
                                GetValue() ? TRUE : FALSE;
            if ( bVis != bShow )
            {
                aAttribs.Put( SfxUInt16Item( EE_PARA_BULLETSTATE, bShow ? 1 : 0 ) );
                pOwner->pEditEngine->SetParaAttribs( nPara, aAttribs );
            }
        }
    }
    pOwner->pEditEngine->SetUpdateMode( bUpdate );

    pOwner->UndoActionEnd( OLUNDO_ATTR );
}

void OutlinerView::RemoveAttribs( BOOL bRemoveParaAttribs, USHORT nWhich )
{
    DBG_CHKTHIS(OutlinerView,0);
    BOOL bUpdate = pOwner->GetUpdateMode();
    pOwner->SetUpdateMode( FALSE );
    pOwner->UndoActionStart( OLUNDO_ATTR );
    pEditView->RemoveAttribs( bRemoveParaAttribs, nWhich );
    if ( bRemoveParaAttribs )
    {
        // Ueber alle Absaetze, und Einrueckung und Level einstellen
        ESelection aSel = pEditView->GetSelection();
        aSel.Adjust();
        for ( USHORT nPara = aSel.nStartPara; nPara <= aSel.nEndPara; nPara++ )
        {
            Paragraph* pPara = pOwner->pParaList->GetParagraph( nPara );
            pPara->Invalidate();

            SfxItemSet aAttrs( pOwner->pEditEngine->GetParaAttribs( nPara ) );
            aAttrs.Put( SfxUInt16Item( EE_PARA_OUTLLEVEL, pPara->GetDepth() ) );
            pOwner->pEditEngine->SetParaAttribs( nPara, aAttrs );

            pOwner->ImplCheckNumBulletItem( (USHORT)nPara );
            pOwner->ImplCalcBulletText( (USHORT)nPara, FALSE, FALSE );
        }
    }
    pOwner->UndoActionEnd( OLUNDO_ATTR );
    pOwner->SetUpdateMode( bUpdate );
}



// =====================================================================
// ======================   Einfache Durchreicher =======================
// ======================================================================


void OutlinerView::InsertText( const XubString& rNew, BOOL bSelect )
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

void OutlinerView::SetReadOnly( BOOL bReadOnly )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetReadOnly( bReadOnly );
}

BOOL OutlinerView::IsReadOnly() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->IsReadOnly();
}

BOOL OutlinerView::HasSelection() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->HasSelection();
}


void OutlinerView::ShowCursor( BOOL bGotoCursor )
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


void OutlinerView::RemoveCharAttribs( ULONG nPara, USHORT nWhich)
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->RemoveCharAttribs( (USHORT)nPara, nWhich);
}


void OutlinerView::CompleteAutoCorrect()
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->CompleteAutoCorrect();
}


EESpellState OutlinerView::StartSpeller( LanguageType eLang, BOOL bMultiDoc )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->StartSpeller( eLang, bMultiDoc );
}


EESpellState OutlinerView::StartThesaurus( LanguageType eLang )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->StartThesaurus( eLang );
}


USHORT OutlinerView::StartSearchAndReplace( const SvxSearchItem& rSearchItem )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->StartSearchAndReplace( rSearchItem );
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


void OutlinerView::SetControlWord( ULONG nWord )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetControlWord( nWord );
}


ULONG OutlinerView::GetControlWord() const
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


void OutlinerView::EnablePaste( BOOL bEnable )
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


const SvxFieldItem* OutlinerView::GetFieldUnderMousePointer( USHORT& nPara, USHORT& nPos ) const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetFieldUnderMousePointer( nPara, nPos );
}


const SvxFieldItem* OutlinerView::GetFieldAtSelection() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetFieldAtSelection();
}

void OutlinerView::SetInvalidateMore( USHORT nPixel )
{
    DBG_CHKTHIS(OutlinerView,0);
    pEditView->SetInvalidateMore( nPixel );
}


USHORT OutlinerView::GetInvalidateMore() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetInvalidateMore();
}


BOOL OutlinerView::IsCursorAtWrongSpelledWord( BOOL bMarkIfWrong )
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->IsCursorAtWrongSpelledWord( bMarkIfWrong );
}


BOOL OutlinerView::IsWrongSpelledWordAtPos( const Point& rPosPixel, BOOL bMarkIfWrong )
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

ULONG OutlinerView::Read( SvStream& rInput, EETextFormat eFormat, BOOL bSelect, SvKeyValueIterator* pHTTPHeaderAttrs )
{
    DBG_CHKTHIS(OutlinerView,0);
    USHORT nOldParaCount = pEditView->GetEditEngine()->GetParagraphCount();
    ESelection aOldSel = pEditView->GetSelection();
    aOldSel.Adjust();

    ULONG nRet = pEditView->Read( rInput, eFormat, bSelect, pHTTPHeaderAttrs );

    // MT 08/00: Hier sollte eigentlich das gleiche wie in PasteSpecial passieren!
    // Mal anpassen, wenn dieses ImplInitPaste und ImpPasted-Geraffel ueberarbeitet ist.

    long nParaDiff = pEditView->GetEditEngine()->GetParagraphCount() - nOldParaCount;
    USHORT nChangesStart = aOldSel.nStartPara;
    USHORT nChangesEnd = nChangesStart + nParaDiff + (aOldSel.nEndPara-aOldSel.nStartPara);

    for ( USHORT n = nChangesStart; n <= nChangesEnd; n++ )
    {
        if ( eFormat == EE_FORMAT_BIN )
        {
            USHORT nDepth = 0;
            const SfxItemSet& rAttrs = pOwner->GetParaAttribs( n );
            const SfxUInt16Item& rLevel = (const SfxUInt16Item&) rAttrs.Get( EE_PARA_OUTLLEVEL );
            nDepth = rLevel.GetValue();
            pOwner->ImplInitDepth( n, nDepth, FALSE );
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

ULONG OutlinerView::Write( SvStream& rOutput, EETextFormat eFormat )
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

USHORT OutlinerView::GetSelectedScriptType() const
{
    DBG_CHKTHIS(OutlinerView,0);
    return pEditView->GetSelectedScriptType();
}

