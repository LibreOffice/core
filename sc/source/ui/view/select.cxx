/*************************************************************************
 *
 *  $RCSfile: select.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-30 19:14:44 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <tools/urlobj.hxx>
#include <vcl/sound.hxx>
#include <sfx2/docfile.hxx>

#include "select.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "scmod.hxx"
#include "document.hxx"
//#include "dataobj.hxx"
#include "transobj.hxx"
#include "docsh.hxx"

extern USHORT nScFillModeMouseModifier;             // global.cxx

using namespace com::sun::star;

// STATIC DATA -----------------------------------------------------------

static Point aSwitchPos;                //! Member
static BOOL bDidSwitch = FALSE;

// -----------------------------------------------------------------------

//
//                  View (Gridwin / Tastatur)
//

ScViewFunctionSet::ScViewFunctionSet( ScViewData* pNewViewData ) :
        pViewData( pNewViewData ),
        pEngine( NULL ),
        bAnchor( FALSE ),
        bStarted( FALSE )
{
    DBG_ASSERT(pViewData, "ViewData==0 bei FunctionSet");
}

ScSplitPos ScViewFunctionSet::GetWhich()
{
    if (pEngine)
        return pEngine->GetWhich();
    else
        return pViewData->GetActivePart();
}

void ScViewFunctionSet::SetSelectionEngine( ScViewSelectionEngine* pSelEngine )
{
    pEngine = pSelEngine;
}

//      Drag & Drop

void __EXPORT ScViewFunctionSet::BeginDrag()
{
    USHORT nTab = pViewData->GetTabNo();

    short nPosX;
    short nPosY;
    if (pEngine)
    {
        Point aMPos = pEngine->GetMousePosPixel();
        pViewData->GetPosFromPixel( aMPos.X(), aMPos.Y(), GetWhich(), nPosX, nPosY );
    }
    else
    {
        nPosX = pViewData->GetCurX();
        nPosY = pViewData->GetCurY();
    }

    ScModule* pScMod = SC_MOD();
    BOOL bRefMode = pScMod->IsFormulaMode();
    if (!bRefMode)
    {
        pViewData->GetView()->FakeButtonUp( GetWhich() );   // ButtonUp wird verschluckt

        ScMarkData& rMark = pViewData->GetMarkData();
//      rMark.SetMarking(FALSE);                        // es fehlt ein ButtonUp
        rMark.MarkToSimple();
        if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
        {
            ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
            pViewData->GetView()->CopyToClip( pClipDoc, FALSE );

            ScDocShell* pDocSh = pViewData->GetDocShell();
            TransferableObjectDescriptor aObjDesc;
            pDocSh->FillTransferableObjectDescriptor( aObjDesc );
            aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
            // maSize is set in ScTransferObj ctor

            ScTransferObj* pTransferObj = new ScTransferObj( pClipDoc, aObjDesc );
            uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

            // set position of dragged cell within range
            ScRange aMarkRange = pTransferObj->GetRange();
            USHORT nStartX = aMarkRange.aStart.Col();
            USHORT nStartY = aMarkRange.aStart.Row();
            USHORT nHandleX = (nPosX >= (short) nStartX) ? nPosX - nStartX : 0;
            USHORT nHandleY = (nPosY >= (short) nStartY) ? nPosY - nStartY : 0;
            pTransferObj->SetDragHandlePos( nHandleX, nHandleY );

            pTransferObj->SetDragSource( pDocSh, rMark );

            Window* pWindow = pViewData->GetActiveWin();
            SC_MOD()->SetDragObject( pTransferObj, NULL );      // for internal D&D
            pTransferObj->StartDrag( pWindow, DND_ACTION_COPYMOVE | DND_ACTION_LINK );

#if 0
            // old api:

            ScDocument* pDoc = pViewData->GetDocument();
            ScRange aMarkRange;
            rMark.GetMarkArea( aMarkRange );
            USHORT nStartX = aMarkRange.aStart.Col();       // Block
            USHORT nStartY = aMarkRange.aStart.Row();
            USHORT nEndX = aMarkRange.aEnd.Col();
            USHORT nEndY = aMarkRange.aEnd.Row();
            USHORT nHandleX = (nPosX >= (short) nStartX) ? nPosX - nStartX : 0;
            USHORT nHandleY = (nPosY >= (short) nStartY) ? nPosY - nStartY : 0;

            pScMod->SetDragObject( rMark,ScRange( nStartX,nStartY,nTab,nEndX,nEndY,nTab ),
                                    nHandleX, nHandleY, pDoc, 0 );

            Region aRegion = pDoc->GetMMRect( nStartX,nStartY, nEndX,nEndY, nTab );

            SvDataObjectRef pDragServer = new ScDataObject( pDoc, FALSE,
                                pViewData->GetDocShell(), NULL, &aMarkRange );
            DropAction eAction = pDragServer->ExecuteDrag(pViewData->GetActiveWin(),
                                        POINTER_MOVEDATA, POINTER_COPYDATA, POINTER_LINKDATA,
                                        DRAG_ALL, &aRegion );
            BOOL bIntern = pScMod->GetDragIntern();

            pScMod->ResetDragObject();

            switch (eAction)
            {
                case DROP_MOVE:
                case DROP_DISCARD:
                    if (!bIntern)
                    {
                        ScViewFunc* pView = pViewData->GetView();
                        pView->DoneBlockMode();
                        pView->MoveCursorAbs( nStartX, nStartY, SC_FOLLOW_JUMP, FALSE, FALSE );
                        pView->InitOwnBlockMode();
                        rMark.SetMarkArea( ScRange( nStartX,nStartY,nTab, nEndX,nEndY,nTab ) );
                        pView->DeleteContents( IDF_ALL );
                    }
                    break;
                default:
                    break;
            }
#endif

            return;         // Dragging passiert
        }
    }

    Sound::Beep();          // kein Dragging
}

//      Selektion

void __EXPORT ScViewFunctionSet::CreateAnchor()
{
    if (bAnchor) return;

    BOOL bRefMode = SC_MOD()->IsFormulaMode();
    if (bRefMode)
        SetAnchor( pViewData->GetRefStartX(), pViewData->GetRefStartY() );
    else
        SetAnchor( pViewData->GetCurX(), pViewData->GetCurY() );
}

void ScViewFunctionSet::SetAnchor( USHORT nPosX, USHORT nPosY )
{
    BOOL bRefMode = SC_MOD()->IsFormulaMode();
    ScTabView* pView = pViewData->GetView();
    USHORT nTab = pViewData->GetTabNo();

    if (bRefMode)
    {
        pView->DoneRefMode( FALSE );
        aAnchorPos.Put( nPosX, nPosY, nTab );
        pView->InitRefMode( aAnchorPos.GetCol(), aAnchorPos.GetRow(), aAnchorPos.GetTab(),
                            SC_REFTYPE_REF );
        bStarted = TRUE;
    }
    else if (pViewData->IsAnyFillMode())
    {
        aAnchorPos.Put( nPosX, nPosY, nTab );
        bStarted = TRUE;
    }
    else
    {
        // nicht weg und gleich wieder hin
        if ( bStarted && pView->IsMarking( nPosX, nPosY, nTab ) )
        {
            // nix
        }
        else
        {
            pView->DoneBlockMode( TRUE );
            aAnchorPos.Put( nPosX, nPosY, nTab );
            ScMarkData& rMark = pViewData->GetMarkData();
            if ( rMark.IsMarked() || rMark.IsMultiMarked() )
            {
                pView->InitBlockMode( aAnchorPos.GetCol(), aAnchorPos.GetRow(),
                                        aAnchorPos.GetTab(), TRUE );
                bStarted = TRUE;
            }
            else
                bStarted = FALSE;
        }
    }
    bAnchor = TRUE;
}

void __EXPORT ScViewFunctionSet::DestroyAnchor()
{
    BOOL bRefMode = SC_MOD()->IsFormulaMode();
    if (bRefMode)
        pViewData->GetView()->DoneRefMode( TRUE );
    else
        pViewData->GetView()->DoneBlockMode( TRUE );

    bAnchor = FALSE;
}

void ScViewFunctionSet::SetAnchorFlag( BOOL bSet )
{
    bAnchor = bSet;
}

BOOL __EXPORT ScViewFunctionSet::SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor )
{
    if ( bDidSwitch )
    {
        if ( rPointPixel == aSwitchPos )
            return FALSE;                   // nicht auf falschem Fenster scrollen
        else
            bDidSwitch = FALSE;
    }
    aSwitchPos = rPointPixel;       // nur wichtig, wenn bDidSwitch

    //  Scrolling

    Size aWinSize = pEngine->GetWindow()->GetOutputSizePixel();
    BOOL bRightScroll  = ( rPointPixel.X() >= aWinSize.Width() );
    BOOL bBottomScroll = ( rPointPixel.Y() >= aWinSize.Height() );
    BOOL bNegScroll    = ( rPointPixel.X() < 0 || rPointPixel.Y() < 0 );
    BOOL bScroll = bRightScroll || bBottomScroll || bNegScroll;

    short   nPosX;
    short   nPosY;
    pViewData->GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), GetWhich(),
                                nPosX, nPosY, TRUE, TRUE );     // mit Repair

    //  fuer AutoFill in der Mitte der Zelle umschalten
    //  dabei aber nicht das Scrolling nach rechts/unten verhindern
    if ( pViewData->IsFillMode() || pViewData->GetFillMode() == SC_FILL_MATRIX )
    {
        BOOL bLeft, bTop;
        pViewData->GetMouseQuadrant( rPointPixel, GetWhich(), nPosX, nPosY, bLeft, bTop );
        ScDocument* pDoc = pViewData->GetDocument();
        USHORT nTab = pViewData->GetTabNo();
        if ( bLeft && !bRightScroll )
            do --nPosX; while ( nPosX>=0 && ( pDoc->GetColFlags( nPosX, nTab ) & CR_HIDDEN ) );
        if ( bTop && !bBottomScroll )
            do --nPosY; while ( nPosY>=0 && ( pDoc->GetRowFlags( nPosY, nTab ) & CR_HIDDEN ) );
        //  negativ ist erlaubt
    }

    //  ueber Fixier-Grenze bewegt?

    ScSplitPos eWhich = GetWhich();
    if ( eWhich == pViewData->GetActivePart() )
    {
        if ( pViewData->GetHSplitMode() == SC_SPLIT_FIX )
            if ( rPointPixel.X() >= aWinSize.Width() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_TOPRIGHT ), bScroll = FALSE, bDidSwitch = TRUE;
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT ), bScroll = FALSE, bDidSwitch = TRUE;
            }

        if ( pViewData->GetVSplitMode() == SC_SPLIT_FIX )
            if ( rPointPixel.Y() >= aWinSize.Height() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT ), bScroll = FALSE, bDidSwitch = TRUE;
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT ), bScroll = FALSE, bDidSwitch = TRUE;
            }
    }

    pViewData->ResetOldCursor();
    return SetCursorAtCell( nPosX, nPosY, bScroll );
}

BOOL ScViewFunctionSet::SetCursorAtCell( short nPosX, short nPosY, BOOL bScroll )
{
    ScTabView* pView = pViewData->GetView();
    USHORT nTab = pViewData->GetTabNo();
    BOOL bRefMode = SC_MOD()->IsFormulaMode();

    BOOL bHide = !bRefMode && !pViewData->IsAnyFillMode() &&
            ( nPosX != (short) pViewData->GetCurX() || nPosY != (short) pViewData->GetCurY() );

    if (bHide)
        pView->HideAllCursors();

    if (bScroll)
    {
        if (bRefMode)
        {
            ScSplitPos eWhich = GetWhich();
            pView->AlignToCursor( nPosX, nPosY, SC_FOLLOW_LINE, &eWhich );
        }
        else
            pView->AlignToCursor( nPosX, nPosY, SC_FOLLOW_LINE );
    }

    if (bRefMode)
    {
        if (!bAnchor)
        {
            pView->DoneRefMode( TRUE );
            pView->InitRefMode( nPosX, nPosY, pViewData->GetTabNo(), SC_REFTYPE_REF );
        }

        pView->UpdateRef( nPosX, nPosY, pViewData->GetTabNo() );
    }
    else if (pViewData->IsFillMode() ||
            (pViewData->GetFillMode() == SC_FILL_MATRIX && (nScFillModeMouseModifier & KEY_MOD1) ))
    {
        //  Wenn eine Matrix angefasst wurde, kann mit Ctrl auf AutoFill zurueckgeschaltet werden

        USHORT nStartX, nStartY, nEndX, nEndY;      // Block
        USHORT nDummy;
        pViewData->GetSimpleArea( nStartX, nStartY, nDummy, nEndX, nEndY, nDummy, FALSE );

        if (pViewData->GetRefType() != SC_REFTYPE_FILL)
        {
            pView->InitRefMode( nStartX, nStartY, nTab, SC_REFTYPE_FILL );
            CreateAnchor();
        }

        ScRange aDelRange;
        BOOL bOldDelMark = pViewData->GetDelMark( aDelRange );
        ScDocument* pDoc = pViewData->GetDocument();

        if ( nPosX+1 >= (short) nStartX && nPosX <= (short) nEndX &&
             nPosY+1 >= (short) nStartY && nPosY <= (short) nEndY &&
             ( nPosX != nEndX || nPosY != nEndY ) )                     // verkleinern ?
        {
            //  Richtung (links oder oben)

            short i;
            long nSizeX = 0;
            for (i=nPosX+1; i<=nEndX; i++)
                nSizeX += pDoc->GetColWidth( i, nTab );
            long nSizeY = 0;
            for (i=nPosY+1; i<=nEndY; i++)
                nSizeY += pDoc->GetRowHeight( i, nTab );

            USHORT nDelStartX = nStartX;
            USHORT nDelStartY = nStartY;
            if ( nSizeX > nSizeY )
                nDelStartX = nPosX + 1;
            else
                nDelStartY = nPosY + 1;
            // 0 braucht nicht mehr getrennt abgefragt zu werden, weil nPosX/Y auch negativ wird

            if ( nDelStartX < nStartX )
                nDelStartX = nStartX;
            if ( nDelStartY < nStartY )
                nDelStartY = nStartY;

            //  Bereich setzen

            pViewData->SetDelMark( ScRange( nDelStartX,nDelStartY,nTab,
                                            nEndX,nEndY,nTab ) );

            if ( bOldDelMark )
            {
                ScUpdateRect aRect( aDelRange.aStart.Col(), aDelRange.aStart.Row(),
                                    aDelRange.aEnd.Col(), aDelRange.aEnd.Row() );
                aRect.SetNew( nDelStartX,nDelStartY, nEndX,nEndY );
                USHORT nPaintStartX;
                USHORT nPaintStartY;
                USHORT nPaintEndX;
                USHORT nPaintEndY;
                if (aRect.GetDiff( nPaintStartX, nPaintStartY, nPaintEndX, nPaintEndY ))
                    pViewData->GetView()->
                        PaintArea( nPaintStartX, nPaintStartY,
                                    nPaintEndX, nPaintEndY, SC_UPDATE_MARKS );
            }
            else
                pViewData->GetView()->
                    PaintArea( nStartX,nDelStartY, nEndX,nEndY, SC_UPDATE_MARKS );

            nPosX = nEndX;      // roten Rahmen um ganzen Bereich lassen
            nPosY = nEndY;

            //  Referenz wieder richtigherum, falls unten umgedreht
            if ( nStartX != pViewData->GetRefStartX() || nStartY != pViewData->GetRefStartY() )
            {
                pViewData->GetView()->DoneRefMode();
                pViewData->GetView()->InitRefMode( nStartX, nStartY, nTab, SC_REFTYPE_FILL );
            }
        }
        else
        {
            if ( bOldDelMark )
            {
                pViewData->ResetDelMark();
                pViewData->GetView()->
                    PaintArea( aDelRange.aStart.Col(), aDelRange.aStart.Row(),
                               aDelRange.aEnd.Col(), aDelRange.aEnd.Row(), SC_UPDATE_MARKS );
            }

            BOOL bNegX = ( nPosX < (short) nStartX );
            BOOL bNegY = ( nPosY < (short) nStartY );
            short i;

            long nSizeX = 0;
            if ( bNegX )
            {
                ++nPosX;
                for (i=nPosX; i<nStartX; i++)
                    nSizeX += pDoc->GetColWidth( i, nTab );
            }
            else
                for (i=nEndX+1; i<=nPosX; i++)
                    nSizeX += pDoc->GetColWidth( i, nTab );

            long nSizeY = 0;
            if ( bNegY )
            {
                ++nPosY;
                for (i=nPosY; i<nStartY; i++)
                    nSizeY += pDoc->GetRowHeight( i, nTab );
            }
            else
                for (i=nEndY+1; i<=nPosY; i++)
                    nSizeY += pDoc->GetRowHeight( i, nTab );

            if ( nSizeX > nSizeY )          // Fill immer nur in einer Richtung
            {
                nPosY = nEndY;
                bNegY = FALSE;
            }
            else
            {
                nPosX = nEndX;
                bNegX = FALSE;
            }

            USHORT nRefStX = bNegX ? nEndX : nStartX;
            USHORT nRefStY = bNegY ? nEndY : nStartY;
            if ( nRefStX != pViewData->GetRefStartX() || nRefStY != pViewData->GetRefStartY() )
            {
                pViewData->GetView()->DoneRefMode();
                pViewData->GetView()->InitRefMode( nRefStX, nRefStY, nTab, SC_REFTYPE_FILL );
            }
        }

        pView->UpdateRef( nPosX, nPosY, nTab );
    }
    else if (pViewData->IsAnyFillMode())
    {
        BYTE nMode = pViewData->GetFillMode();
        if ( nMode == SC_FILL_EMBED_LT || nMode == SC_FILL_EMBED_RB )
        {
            ScDocument* pDoc = pViewData->GetDocument();
            DBG_ASSERT( pDoc->IsEmbedded(), "!pDoc->IsEmbedded()" );
            ScTripel aStart;
            ScTripel aEnd;
            pDoc->GetEmbedded( aStart, aEnd );
            ScRefType eRefMode = (nMode == SC_FILL_EMBED_LT) ? SC_REFTYPE_EMBED_LT : SC_REFTYPE_EMBED_RB;
            if (pViewData->GetRefType() != eRefMode)
            {
                if ( nMode == SC_FILL_EMBED_LT )
                    pView->InitRefMode( aEnd.GetCol(), aEnd.GetRow(), nTab, eRefMode );
                else
                    pView->InitRefMode( aStart.GetCol(), aStart.GetRow(), nTab, eRefMode );
                CreateAnchor();
            }

            pView->UpdateRef( nPosX, nPosY, nTab );
        }
        else if ( nMode == SC_FILL_MATRIX )
        {
            USHORT nStartX, nStartY, nEndX, nEndY;      // Block
            USHORT nDummy;
            pViewData->GetSimpleArea( nStartX, nStartY, nDummy, nEndX, nEndY, nDummy, FALSE );

            if (pViewData->GetRefType() != SC_REFTYPE_FILL)
            {
                pView->InitRefMode( nStartX, nStartY, nTab, SC_REFTYPE_FILL );
                CreateAnchor();
            }

            if ( nPosX < nStartX ) nPosX = nStartX;
            if ( nPosY < nStartY ) nPosY = nStartY;

            pView->UpdateRef( nPosX, nPosY, nTab );
        }
        // else neue Modi
    }
    else                    // normales Markieren
    {
        BOOL bHideCur = bAnchor && ( (USHORT)nPosX != pViewData->GetCurX() ||
                                     (USHORT)nPosY != pViewData->GetCurY() );
        if (bHideCur)
            pView->HideAllCursors();            // sonst zweimal: Block und SetCursor

        if (bAnchor)
        {
            if (!bStarted)
            {
                BOOL bMove = ( nPosX != (short) aAnchorPos.GetCol() ||
                                nPosY != (short) aAnchorPos.GetRow() );
                if ( bMove || ( pEngine && pEngine->GetMouseEvent().IsShift() ) )
                {
                    pView->InitBlockMode( aAnchorPos.GetCol(), aAnchorPos.GetRow(),
                                            aAnchorPos.GetTab(), TRUE );
                    bStarted = TRUE;
                }
            }
            if (bStarted)
                pView->MarkCursor( (USHORT) nPosX, (USHORT) nPosY, nTab );
        }
        else
        {
            ScMarkData& rMark = pViewData->GetMarkData();
            if (rMark.IsMarked() || rMark.IsMultiMarked())
            {
                pView->DoneBlockMode(TRUE);
                pView->InitBlockMode( nPosX, nPosY, nTab, TRUE );
                pView->MarkCursor( (USHORT) nPosX, (USHORT) nPosY, nTab );

                aAnchorPos.Put( nPosX, nPosY, nTab );
                bStarted = TRUE;
            }
        }

        pView->SetCursor( (USHORT) nPosX, (USHORT) nPosY );
        pViewData->SetRefStart( nPosX, nPosY, nTab );
        if (bHideCur)
            pView->ShowAllCursors();
    }

    if (bHide)
        pView->ShowAllCursors();

    return TRUE;
}

BOOL __EXPORT ScViewFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    BOOL bRefMode = SC_MOD()->IsFormulaMode();
    if (bRefMode)
        return FALSE;

    if (pViewData->IsAnyFillMode())
        return FALSE;

    ScMarkData& rMark = pViewData->GetMarkData();
    if (bAnchor || !rMark.IsMultiMarked())
    {
        short   nPosX;
        short   nPosY;
        pViewData->GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), GetWhich(), nPosX, nPosY );
        return pViewData->GetMarkData().IsCellMarked( (USHORT) nPosX, (USHORT) nPosY );
    }

    return FALSE;
}

void __EXPORT ScViewFunctionSet::DeselectAtPoint( const Point& rPointPixel )
{
    //  gibt's nicht
}

void __EXPORT ScViewFunctionSet::DeselectAll()
{
    if (pViewData->IsAnyFillMode())
        return;

    BOOL bRefMode = SC_MOD()->IsFormulaMode();
    if (bRefMode)
    {
        pViewData->GetView()->DoneRefMode( FALSE );
    }
    else
    {
        pViewData->GetView()->DoneBlockMode( FALSE );
        pViewData->GetViewShell()->UpdateInputHandler();
    }

    bAnchor = FALSE;
}

//------------------------------------------------------------------------

ScViewSelectionEngine::ScViewSelectionEngine( Window* pWindow, ScTabView* pView,
                                                ScSplitPos eSplitPos ) :
        SelectionEngine( pWindow, pView->GetFunctionSet() ),
        eWhich( eSplitPos )
{
    //  Parameter einstellen
    SetSelectionMode( MULTIPLE_SELECTION );
    EnableDrag( TRUE );
}


//------------------------------------------------------------------------

//
//                  Spalten- / Zeilenheader
//

ScHeaderFunctionSet::ScHeaderFunctionSet( ScViewData* pNewViewData ) :
        pViewData( pNewViewData ),
        bColumn( FALSE ),
        eWhich( SC_SPLIT_TOPLEFT ),
        bAnchor( FALSE ),
        nCursorPos( 0 )
{
    DBG_ASSERT(pViewData, "ViewData==0 bei FunctionSet");
}

void ScHeaderFunctionSet::SetColumn( BOOL bSet )
{
    bColumn = bSet;
}

void ScHeaderFunctionSet::SetWhich( ScSplitPos eNew )
{
    eWhich = eNew;
}

void __EXPORT ScHeaderFunctionSet::BeginDrag()
{
    // gippsnich
}

void __EXPORT ScHeaderFunctionSet::CreateAnchor()
{
    if (bAnchor)
        return;

    ScTabView* pView = pViewData->GetView();
    pView->DoneBlockMode( TRUE );
    if (bColumn)
    {
        pView->InitBlockMode( nCursorPos, 0, pViewData->GetTabNo(), TRUE, TRUE, FALSE );
        pView->MarkCursor( nCursorPos, MAXROW, pViewData->GetTabNo() );
    }
    else
    {
        pView->InitBlockMode( 0, nCursorPos, pViewData->GetTabNo(), TRUE, FALSE, TRUE );
        pView->MarkCursor( MAXCOL, nCursorPos, pViewData->GetTabNo() );
    }
    bAnchor = TRUE;
}

void __EXPORT ScHeaderFunctionSet::DestroyAnchor()
{
    pViewData->GetView()->DoneBlockMode( TRUE );
    bAnchor = FALSE;
}

BOOL __EXPORT ScHeaderFunctionSet::SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor )
{
    if ( bDidSwitch )
    {
        //  die naechste gueltige Position muss vom anderen Fenster kommen
        if ( rPointPixel == aSwitchPos )
            return FALSE;                   // nicht auf falschem Fenster scrollen
        else
            bDidSwitch = FALSE;
    }

    //  Scrolling

    Size aWinSize = pViewData->GetActiveWin()->GetOutputSizePixel();
    BOOL bScroll;
    if (bColumn)
        bScroll = ( rPointPixel.X() < 0 || rPointPixel.X() >= aWinSize.Width() );
    else
        bScroll = ( rPointPixel.Y() < 0 || rPointPixel.Y() >= aWinSize.Height() );

    //  ueber Fixier-Grenze bewegt?

    BOOL bSwitched = FALSE;
    if ( bColumn )
    {
        if ( pViewData->GetHSplitMode() == SC_SPLIT_FIX )
        {
            if ( rPointPixel.X() > aWinSize.Width() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_TOPRIGHT ), bSwitched = TRUE;
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT ), bSwitched = TRUE;
            }
        }
    }
    else                // Zeilenkoepfe
    {
        if ( pViewData->GetVSplitMode() == SC_SPLIT_FIX )
        {
            if ( rPointPixel.Y() > aWinSize.Height() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT ), bSwitched = TRUE;
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT ), bSwitched = TRUE;
            }
        }
    }
    if (bSwitched)
    {
        aSwitchPos = rPointPixel;
        bDidSwitch = TRUE;
        return FALSE;               // nicht mit falschen Positionen rechnen
    }

    //

    short   nPosX;
    short   nPosY;
    pViewData->GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), pViewData->GetActivePart(),
                                nPosX, nPosY, FALSE );
    if (bColumn)
    {
        nCursorPos = nPosX;
        nPosY = pViewData->GetPosY(WhichV(pViewData->GetActivePart()));
    }
    else
    {
        nCursorPos = nPosY;
        nPosX = pViewData->GetPosX(WhichH(pViewData->GetActivePart()));
    }

    ScTabView* pView = pViewData->GetView();
    BOOL bHide = pViewData->GetCurX() != nPosX ||
                 pViewData->GetCurY() != nPosY;
    if (bHide)
        pView->HideAllCursors();

    if (bScroll)
        pView->AlignToCursor( nPosX, nPosY, SC_FOLLOW_LINE );
    pView->SetCursor( nPosX, nPosY );

    if (!bAnchor)
    {
        pView->DoneBlockMode( TRUE );
        pViewData->GetMarkData().MarkToMulti();         //! wer verstellt das ???
        pView->InitBlockMode( nPosX, nPosY, pViewData->GetTabNo(), TRUE, bColumn, !bColumn );

        bAnchor = TRUE;
    }

    pView->MarkCursor( nPosX, nPosY, pViewData->GetTabNo(), bColumn, !bColumn );

    //  SelectionChanged innerhalb von HideCursor wegen UpdateAutoFillMark
    pView->SelectionChanged();

    if (bHide)
        pView->ShowAllCursors();

    return TRUE;
}

BOOL __EXPORT ScHeaderFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    short   nPosX;
    short   nPosY;
    pViewData->GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), pViewData->GetActivePart(),
                                nPosX, nPosY, FALSE );

    ScMarkData& rMark = pViewData->GetMarkData();
    if (bColumn)
        return rMark.IsColumnMarked( nPosX );
    else
        return rMark.IsRowMarked( nPosY );
}

void __EXPORT ScHeaderFunctionSet::DeselectAtPoint( const Point& rPointPixel )
{
}

void __EXPORT ScHeaderFunctionSet::DeselectAll()
{
    pViewData->GetView()->DoneBlockMode( FALSE );
    bAnchor = FALSE;
}

//------------------------------------------------------------------------

ScHeaderSelectionEngine::ScHeaderSelectionEngine( Window* pWindow, ScHeaderFunctionSet* pFuncSet ) :
        SelectionEngine( pWindow, pFuncSet )
{
    //  Parameter einstellen
    SetSelectionMode( MULTIPLE_SELECTION );
    EnableDrag( FALSE );
}





