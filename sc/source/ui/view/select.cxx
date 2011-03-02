/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <tools/urlobj.hxx>
#include <vcl/sound.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/docfile.hxx>

#include "select.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "scmod.hxx"
#include "document.hxx"
#include "transobj.hxx"
#include "docsh.hxx"
#include "tabprotection.hxx"

#define SC_SELENG_REFMODE_UPDATE_INTERVAL_MIN 65

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

ULONG ScViewFunctionSet::CalcUpdateInterval( const Size& rWinSize, const Point& rEffPos,
                                             bool bLeftScroll, bool bTopScroll, bool bRightScroll, bool bBottomScroll )
{
    ULONG nUpdateInterval = SELENG_AUTOREPEAT_INTERVAL_MAX;
    Window* pWin = pEngine->GetWindow();
    Rectangle aScrRect = pWin->GetDesktopRectPixel();
    Point aRootPos = pWin->OutputToAbsoluteScreenPixel(Point(0,0));
    if (bRightScroll)
    {
        double nWinRight = rWinSize.getWidth() + aRootPos.getX();
        double nMarginRight = aScrRect.GetWidth() - nWinRight;
        double nHOffset = rEffPos.X() - rWinSize.Width();
        double nHAccelRate = nHOffset / nMarginRight;

        if (nHAccelRate > 1.0)
            nHAccelRate = 1.0;

        nUpdateInterval = static_cast<ULONG>(SELENG_AUTOREPEAT_INTERVAL_MAX*(1.0 - nHAccelRate));
    }

    if (bLeftScroll)
    {
        double nMarginLeft = aRootPos.getX();
        double nHOffset = -rEffPos.X();
        double nHAccelRate = nHOffset / nMarginLeft;

        if (nHAccelRate > 1.0)
            nHAccelRate = 1.0;

        ULONG nTmp = static_cast<ULONG>(SELENG_AUTOREPEAT_INTERVAL_MAX*(1.0 - nHAccelRate));
        if (nUpdateInterval > nTmp)
            nUpdateInterval = nTmp;
    }

    if (bBottomScroll)
    {
        double nWinBottom = rWinSize.getHeight() + aRootPos.getY();
        double nMarginBottom = aScrRect.GetHeight() - nWinBottom;
        double nVOffset = rEffPos.Y() - rWinSize.Height();
        double nVAccelRate = nVOffset / nMarginBottom;

        if (nVAccelRate > 1.0)
            nVAccelRate = 1.0;

        ULONG nTmp = static_cast<ULONG>(SELENG_AUTOREPEAT_INTERVAL_MAX*(1.0 - nVAccelRate));
        if (nUpdateInterval > nTmp)
            nUpdateInterval = nTmp;
    }

    if (bTopScroll)
    {
        double nMarginTop = aRootPos.getY();
        double nVOffset = -rEffPos.Y();
        double nVAccelRate = nVOffset / nMarginTop;

        if (nVAccelRate > 1.0)
            nVAccelRate = 1.0;

        ULONG nTmp = static_cast<ULONG>(SELENG_AUTOREPEAT_INTERVAL_MAX*(1.0 - nVAccelRate));
        if (nUpdateInterval > nTmp)
            nUpdateInterval = nTmp;
    }

#ifdef WNT
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    bool bRefMode = pViewShell && pViewShell->IsRefInputMode();
    if (bRefMode && nUpdateInterval < SC_SELENG_REFMODE_UPDATE_INTERVAL_MIN)
        // Lower the update interval during ref mode, because re-draw can be
        // expensive on Windows.  Making this interval too small would queue up
        // the scroll/paint requests which would cause semi-infinite
        // scrolls even after the mouse cursor is released.  We don't have
        // this problem on Linux.
        nUpdateInterval = SC_SELENG_REFMODE_UPDATE_INTERVAL_MIN;
#endif
    return nUpdateInterval;
}

void ScViewFunctionSet::SetSelectionEngine( ScViewSelectionEngine* pSelEngine )
{
    pEngine = pSelEngine;
}

//      Drag & Drop

void ScViewFunctionSet::BeginDrag()
{
    SCTAB nTab = pViewData->GetTabNo();

    SCsCOL nPosX;
    SCsROW nPosY;
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
        pViewData->GetView()->FakeButtonUp( GetWhich() );   // ButtonUp is swallowed

        ScMarkData& rMark = pViewData->GetMarkData();
        rMark.MarkToSimple();
        if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
        {
            ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
            // bApi = TRUE -> no error messages
            BOOL bCopied = pViewData->GetView()->CopyToClip( pClipDoc, FALSE, TRUE );
            if ( bCopied )
            {
                sal_Int8 nDragActions = pViewData->GetView()->SelectionEditable() ?
                                        ( DND_ACTION_COPYMOVE | DND_ACTION_LINK ) :
                                        ( DND_ACTION_COPY | DND_ACTION_LINK );

                ScDocShell* pDocSh = pViewData->GetDocShell();
                TransferableObjectDescriptor aObjDesc;
                pDocSh->FillTransferableObjectDescriptor( aObjDesc );
                aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
                // maSize is set in ScTransferObj ctor

                ScTransferObj* pTransferObj = new ScTransferObj( pClipDoc, aObjDesc );
                uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

                // set position of dragged cell within range
                ScRange aMarkRange = pTransferObj->GetRange();
                SCCOL nStartX = aMarkRange.aStart.Col();
                SCROW nStartY = aMarkRange.aStart.Row();
                SCCOL nHandleX = (nPosX >= (SCsCOL) nStartX) ? nPosX - nStartX : 0;
                SCROW nHandleY = (nPosY >= (SCsROW) nStartY) ? nPosY - nStartY : 0;
                pTransferObj->SetDragHandlePos( nHandleX, nHandleY );
                pTransferObj->SetVisibleTab( nTab );

                pTransferObj->SetDragSource( pDocSh, rMark );

                Window* pWindow = pViewData->GetActiveWin();
                if ( pWindow->IsTracking() )
                    pWindow->EndTracking( ENDTRACK_CANCEL );    // abort selecting

                SC_MOD()->SetDragObject( pTransferObj, NULL );      // for internal D&D
                pTransferObj->StartDrag( pWindow, nDragActions );

                return;         // dragging started
            }
            else
                delete pClipDoc;
        }
    }

    Sound::Beep();          // can't drag
}

//      Selection

void ScViewFunctionSet::CreateAnchor()
{
    if (bAnchor) return;

    BOOL bRefMode = SC_MOD()->IsFormulaMode();
    if (bRefMode)
        SetAnchor( pViewData->GetRefStartX(), pViewData->GetRefStartY() );
    else
        SetAnchor( pViewData->GetCurX(), pViewData->GetCurY() );
}

void ScViewFunctionSet::SetAnchor( SCCOL nPosX, SCROW nPosY )
{
    BOOL bRefMode = SC_MOD()->IsFormulaMode();
    ScTabView* pView = pViewData->GetView();
    SCTAB nTab = pViewData->GetTabNo();

    if (bRefMode)
    {
        pView->DoneRefMode( FALSE );
        aAnchorPos.Set( nPosX, nPosY, nTab );
        pView->InitRefMode( aAnchorPos.Col(), aAnchorPos.Row(), aAnchorPos.Tab(),
                            SC_REFTYPE_REF );
        bStarted = TRUE;
    }
    else if (pViewData->IsAnyFillMode())
    {
        aAnchorPos.Set( nPosX, nPosY, nTab );
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
            aAnchorPos.Set( nPosX, nPosY, nTab );
            ScMarkData& rMark = pViewData->GetMarkData();
            if ( rMark.IsMarked() || rMark.IsMultiMarked() )
            {
                pView->InitBlockMode( aAnchorPos.Col(), aAnchorPos.Row(),
                                        aAnchorPos.Tab(), TRUE );
                bStarted = TRUE;
            }
            else
                bStarted = FALSE;
        }
    }
    bAnchor = TRUE;
}

void ScViewFunctionSet::DestroyAnchor()
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

BOOL ScViewFunctionSet::SetCursorAtPoint( const Point& rPointPixel, BOOL /* bDontSelectAtCursor */ )
{
    if ( bDidSwitch )
    {
        if ( rPointPixel == aSwitchPos )
            return FALSE;                   // nicht auf falschem Fenster scrollen
        else
            bDidSwitch = FALSE;
    }
    aSwitchPos = rPointPixel;       // nur wichtig, wenn bDidSwitch

    //  treat position 0 as -1, so scrolling is always possible
    //  (with full screen and hidden headers, the top left border may be at 0)
    //  (moved from ScViewData::GetPosFromPixel)

    Point aEffPos = rPointPixel;
    if ( aEffPos.X() == 0 )
        aEffPos.X() = -1;
    if ( aEffPos.Y() == 0 )
        aEffPos.Y() = -1;

    //  Scrolling

    Size aWinSize = pEngine->GetWindow()->GetOutputSizePixel();
    bool bRightScroll  = ( aEffPos.X() >= aWinSize.Width() );
    bool bLeftScroll  = ( aEffPos.X() < 0 );
    bool bBottomScroll = ( aEffPos.Y() >= aWinSize.Height() );
    bool bTopScroll = ( aEffPos.Y() < 0 );
    bool bScroll = bRightScroll || bBottomScroll || bLeftScroll || bTopScroll;

    SCsCOL  nPosX;
    SCsROW  nPosY;
    pViewData->GetPosFromPixel( aEffPos.X(), aEffPos.Y(), GetWhich(),
                                nPosX, nPosY, TRUE, TRUE );     // mit Repair

    //  fuer AutoFill in der Mitte der Zelle umschalten
    //  dabei aber nicht das Scrolling nach rechts/unten verhindern
    if ( pViewData->IsFillMode() || pViewData->GetFillMode() == SC_FILL_MATRIX )
    {
        BOOL bLeft, bTop;
        pViewData->GetMouseQuadrant( aEffPos, GetWhich(), nPosX, nPosY, bLeft, bTop );
        ScDocument* pDoc = pViewData->GetDocument();
        SCTAB nTab = pViewData->GetTabNo();
        if ( bLeft && !bRightScroll )
            do --nPosX; while ( nPosX>=0 && pDoc->ColHidden( nPosX, nTab ) );
        if ( bTop && !bBottomScroll )
        {
            if (--nPosY >= 0)
            {
                nPosY = pDoc->LastVisibleRow(0, nPosY, nTab);
                if (!ValidRow(nPosY))
                    nPosY = -1;
            }
        }
        //  negativ ist erlaubt
    }

    //  ueber Fixier-Grenze bewegt?

    ScSplitPos eWhich = GetWhich();
    if ( eWhich == pViewData->GetActivePart() )
    {
        if ( pViewData->GetHSplitMode() == SC_SPLIT_FIX )
            if ( aEffPos.X() >= aWinSize.Width() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_TOPRIGHT ), bScroll = FALSE, bDidSwitch = TRUE;
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT ), bScroll = FALSE, bDidSwitch = TRUE;
            }

        if ( pViewData->GetVSplitMode() == SC_SPLIT_FIX )
            if ( aEffPos.Y() >= aWinSize.Height() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT ), bScroll = FALSE, bDidSwitch = TRUE;
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT ), bScroll = FALSE, bDidSwitch = TRUE;
            }
    }

    if (bScroll)
    {
        // Adjust update interval based on how far the mouse pointer is from the edge.
        ULONG nUpdateInterval = CalcUpdateInterval(
            aWinSize, aEffPos, bLeftScroll, bTopScroll, bRightScroll, bBottomScroll);
        pEngine->SetUpdateInterval(nUpdateInterval);
    }
    else
    {
        // Don't forget to reset the interval when not scrolling!
        pEngine->SetUpdateInterval(SELENG_AUTOREPEAT_INTERVAL);
    }

    pViewData->ResetOldCursor();
    return SetCursorAtCell( nPosX, nPosY, bScroll );
}

BOOL ScViewFunctionSet::SetCursorAtCell( SCsCOL nPosX, SCsROW nPosY, BOOL bScroll )
{
    ScTabView* pView = pViewData->GetView();
    SCTAB nTab = pViewData->GetTabNo();
    ScDocument* pDoc = pViewData->GetDocument();

    if ( pDoc->IsTabProtected(nTab) )
    {
        if (nPosX < 0 || nPosY < 0)
            return false;

        ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
        bool bSkipProtected   = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bool bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);

        if ( bSkipProtected && bSkipUnprotected )
            return FALSE;

        bool bCellProtected = pDoc->HasAttrib(nPosX, nPosY, nTab, nPosX, nPosY, nTab, HASATTR_PROTECTED);
        if ( (bCellProtected && bSkipProtected) || (!bCellProtected && bSkipUnprotected) )
            // Don't select this cell!
            return FALSE;
    }

    ScModule* pScMod = SC_MOD();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    bool bRefMode = ( pViewShell ? pViewShell->IsRefInputMode() : false );

    BOOL bHide = !bRefMode && !pViewData->IsAnyFillMode() &&
            ( nPosX != (SCsCOL) pViewData->GetCurX() || nPosY != (SCsROW) pViewData->GetCurY() );

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
        // if no input is possible from this doc, don't move the reference cursor around
        if ( !pScMod->IsModalMode(pViewData->GetSfxDocShell()) )
        {
            if (!bAnchor)
            {
                pView->DoneRefMode( TRUE );
                pView->InitRefMode( nPosX, nPosY, pViewData->GetTabNo(), SC_REFTYPE_REF );
            }

            pView->UpdateRef( nPosX, nPosY, pViewData->GetTabNo() );
        }
    }
    else if (pViewData->IsFillMode() ||
            (pViewData->GetFillMode() == SC_FILL_MATRIX && (nScFillModeMouseModifier & KEY_MOD1) ))
    {
        //  Wenn eine Matrix angefasst wurde, kann mit Ctrl auf AutoFill zurueckgeschaltet werden

        SCCOL nStartX, nEndX;
        SCROW nStartY, nEndY; // Block
        SCTAB nDummy;
        pViewData->GetSimpleArea( nStartX, nStartY, nDummy, nEndX, nEndY, nDummy );

        if (pViewData->GetRefType() != SC_REFTYPE_FILL)
        {
            pView->InitRefMode( nStartX, nStartY, nTab, SC_REFTYPE_FILL );
            CreateAnchor();
        }

        ScRange aDelRange;
        BOOL bOldDelMark = pViewData->GetDelMark( aDelRange );

        if ( nPosX+1 >= (SCsCOL) nStartX && nPosX <= (SCsCOL) nEndX &&
             nPosY+1 >= (SCsROW) nStartY && nPosY <= (SCsROW) nEndY &&
             ( nPosX != nEndX || nPosY != nEndY ) )                     // verkleinern ?
        {
            //  Richtung (links oder oben)

            long nSizeX = 0;
            for (SCCOL i=nPosX+1; i<=nEndX; i++)
                nSizeX += pDoc->GetColWidth( i, nTab );
            long nSizeY = (long) pDoc->GetRowHeight( nPosY+1, nEndY, nTab );

            SCCOL nDelStartX = nStartX;
            SCROW nDelStartY = nStartY;
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
            pViewData->GetView()->UpdateShrinkOverlay();

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
                pViewData->GetView()->UpdateShrinkOverlay();
            }

            BOOL bNegX = ( nPosX < (SCsCOL) nStartX );
            BOOL bNegY = ( nPosY < (SCsROW) nStartY );

            long nSizeX = 0;
            if ( bNegX )
            {
                //  in SetCursorAtPoint hidden columns are skipped.
                //  They must be skipped here too, or the result will always be the first hidden column.
                do ++nPosX; while ( nPosX<nStartX && pDoc->ColHidden(nPosX, nTab) );
                for (SCCOL i=nPosX; i<nStartX; i++)
                    nSizeX += pDoc->GetColWidth( i, nTab );
            }
            else
                for (SCCOL i=nEndX+1; i<=nPosX; i++)
                    nSizeX += pDoc->GetColWidth( i, nTab );

            long nSizeY = 0;
            if ( bNegY )
            {
                //  in SetCursorAtPoint hidden rows are skipped.
                //  They must be skipped here too, or the result will always be the first hidden row.
                if (++nPosY < nStartY)
                {
                    nPosY = pDoc->FirstVisibleRow(nPosY, nStartY-1, nTab);
                    if (!ValidRow(nPosY))
                        nPosY = nStartY;
                }
                nSizeY += pDoc->GetRowHeight( nPosY, nStartY-1, nTab );
            }
            else
                nSizeY += pDoc->GetRowHeight( nEndY+1, nPosY, nTab );

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

            SCCOL nRefStX = bNegX ? nEndX : nStartX;
            SCROW nRefStY = bNegY ? nEndY : nStartY;
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
            DBG_ASSERT( pDoc->IsEmbedded(), "!pDoc->IsEmbedded()" );
            ScRange aRange;
            pDoc->GetEmbedded( aRange);
            ScRefType eRefMode = (nMode == SC_FILL_EMBED_LT) ? SC_REFTYPE_EMBED_LT : SC_REFTYPE_EMBED_RB;
            if (pViewData->GetRefType() != eRefMode)
            {
                if ( nMode == SC_FILL_EMBED_LT )
                    pView->InitRefMode( aRange.aEnd.Col(), aRange.aEnd.Row(), nTab, eRefMode );
                else
                    pView->InitRefMode( aRange.aStart.Col(), aRange.aStart.Row(), nTab, eRefMode );
                CreateAnchor();
            }

            pView->UpdateRef( nPosX, nPosY, nTab );
        }
        else if ( nMode == SC_FILL_MATRIX )
        {
            SCCOL nStartX, nEndX;
            SCROW nStartY, nEndY; // Block
            SCTAB nDummy;
            pViewData->GetSimpleArea( nStartX, nStartY, nDummy, nEndX, nEndY, nDummy );

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
        BOOL bHideCur = bAnchor && ( (SCCOL)nPosX != pViewData->GetCurX() ||
                                     (SCROW)nPosY != pViewData->GetCurY() );
        if (bHideCur)
            pView->HideAllCursors();            // sonst zweimal: Block und SetCursor

        if (bAnchor)
        {
            if (!bStarted)
            {
                BOOL bMove = ( nPosX != (SCsCOL) aAnchorPos.Col() ||
                                nPosY != (SCsROW) aAnchorPos.Row() );
                if ( bMove || ( pEngine && pEngine->GetMouseEvent().IsShift() ) )
                {
                    pView->InitBlockMode( aAnchorPos.Col(), aAnchorPos.Row(),
                                            aAnchorPos.Tab(), TRUE );
                    bStarted = TRUE;
                }
            }
            if (bStarted)
                // If the selection is already started, don't set the cursor.
                pView->MarkCursor( (SCCOL) nPosX, (SCROW) nPosY, nTab, FALSE, FALSE, TRUE );
            else
                pView->SetCursor( (SCCOL) nPosX, (SCROW) nPosY );
        }
        else
        {
            ScMarkData& rMark = pViewData->GetMarkData();
            if (rMark.IsMarked() || rMark.IsMultiMarked())
            {
                pView->DoneBlockMode(TRUE);
                pView->InitBlockMode( nPosX, nPosY, nTab, TRUE );
                pView->MarkCursor( (SCCOL) nPosX, (SCROW) nPosY, nTab );

                aAnchorPos.Set( nPosX, nPosY, nTab );
                bStarted = TRUE;
            }
            // #i3875# *Hack* When a new cell is Ctrl-clicked with no pre-selected cells,
            // it highlights that new cell as well as the old cell where the cursor is
            // positioned prior to the click.  A selection mode via Shift-F8 should also
            // follow the same behavior.
            else if ( pViewData->IsSelCtrlMouseClick() )
            {
                SCCOL nOldX = pViewData->GetCurX();
                SCROW nOldY = pViewData->GetCurY();

                pView->InitBlockMode( nOldX, nOldY, nTab, TRUE );
                pView->MarkCursor( (SCCOL) nOldX, (SCROW) nOldY, nTab );

                if ( nOldX != nPosX || nOldY != nPosY )
                {
                    pView->DoneBlockMode( TRUE );
                    pView->InitBlockMode( nPosX, nPosY, nTab, TRUE );
                    pView->MarkCursor( (SCCOL) nPosX, (SCROW) nPosY, nTab );
                    aAnchorPos.Set( nPosX, nPosY, nTab );
                }

                bStarted = TRUE;
            }
            pView->SetCursor( (SCCOL) nPosX, (SCROW) nPosY );
        }

        pViewData->SetRefStart( nPosX, nPosY, nTab );
        if (bHideCur)
            pView->ShowAllCursors();
    }

    if (bHide)
        pView->ShowAllCursors();

    return TRUE;
}

BOOL ScViewFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    BOOL bRefMode = SC_MOD()->IsFormulaMode();
    if (bRefMode)
        return FALSE;

    if (pViewData->IsAnyFillMode())
        return FALSE;

    ScMarkData& rMark = pViewData->GetMarkData();
    if (bAnchor || !rMark.IsMultiMarked())
    {
        SCsCOL  nPosX;
        SCsROW  nPosY;
        pViewData->GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), GetWhich(), nPosX, nPosY );
        return pViewData->GetMarkData().IsCellMarked( (SCCOL) nPosX, (SCROW) nPosY );
    }

    return FALSE;
}

void ScViewFunctionSet::DeselectAtPoint( const Point& /* rPointPixel */ )
{
    //  gibt's nicht
}

void ScViewFunctionSet::DeselectAll()
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

void ScHeaderFunctionSet::BeginDrag()
{
    // gippsnich
}

void ScHeaderFunctionSet::CreateAnchor()
{
    if (bAnchor)
        return;

    ScTabView* pView = pViewData->GetView();
    pView->DoneBlockMode( TRUE );
    if (bColumn)
    {
        pView->InitBlockMode( static_cast<SCCOL>(nCursorPos), 0, pViewData->GetTabNo(), TRUE, TRUE, FALSE );
        pView->MarkCursor( static_cast<SCCOL>(nCursorPos), MAXROW, pViewData->GetTabNo() );
    }
    else
    {
        pView->InitBlockMode( 0, nCursorPos, pViewData->GetTabNo(), TRUE, FALSE, TRUE );
        pView->MarkCursor( MAXCOL, nCursorPos, pViewData->GetTabNo() );
    }
    bAnchor = TRUE;
}

void ScHeaderFunctionSet::DestroyAnchor()
{
    pViewData->GetView()->DoneBlockMode( TRUE );
    bAnchor = FALSE;
}

BOOL ScHeaderFunctionSet::SetCursorAtPoint( const Point& rPointPixel, BOOL /* bDontSelectAtCursor */ )
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

    SCsCOL  nPosX;
    SCsROW  nPosY;
    pViewData->GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), pViewData->GetActivePart(),
                                nPosX, nPosY, FALSE );
    if (bColumn)
    {
        nCursorPos = static_cast<SCCOLROW>(nPosX);
        nPosY = pViewData->GetPosY(WhichV(pViewData->GetActivePart()));
    }
    else
    {
        nCursorPos = static_cast<SCCOLROW>(nPosY);
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

    if ( !bAnchor || !pView->IsBlockMode() )
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

BOOL ScHeaderFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    SCsCOL  nPosX;
    SCsROW  nPosY;
    pViewData->GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), pViewData->GetActivePart(),
                                nPosX, nPosY, FALSE );

    ScMarkData& rMark = pViewData->GetMarkData();
    if (bColumn)
        return rMark.IsColumnMarked( nPosX );
    else
        return rMark.IsRowMarked( nPosY );
}

void ScHeaderFunctionSet::DeselectAtPoint( const Point& /* rPointPixel */ )
{
}

void ScHeaderFunctionSet::DeselectAll()
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





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
