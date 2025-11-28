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

#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <osl/diagnose.h>

#include <select.hxx>
#include <tabvwsh.hxx>
#include <scmod.hxx>
#include <document.hxx>
#include <transobj.hxx>
#include <docsh.hxx>
#include <tabprotection.hxx>
#include <markdata.hxx>
#include <gridwin.hxx>
#include <sfx2/lokhelper.hxx>
#include <comphelper/lok.hxx>

#if defined(_WIN32)
#define SC_SELENG_REFMODE_UPDATE_INTERVAL_MIN 65
#endif

using namespace com::sun::star;

static Point aSwitchPos;                //! Member
static bool bDidSwitch = false;

// View (Gridwin / keyboard)
ScViewFunctionSet::ScViewFunctionSet( ScViewData& rViewData ) :
        m_rViewData( rViewData ),
        m_pEngine( nullptr ),
        m_bAnchor( false ),
        m_bStarted( false )
{
}

ScSplitPos ScViewFunctionSet::GetWhich() const
{
    if (m_pEngine)
        return m_pEngine->GetWhich();
    else
        return m_rViewData.GetActivePart();
}

sal_uInt64 ScViewFunctionSet::CalcUpdateInterval( const Size& rWinSize, const Point& rEffPos,
                                             bool bLeftScroll, bool bTopScroll, bool bRightScroll, bool bBottomScroll )
{
    sal_uInt64 nUpdateInterval = SELENG_AUTOREPEAT_INTERVAL_MAX;
    vcl::Window* pWin = m_pEngine->GetWindow();
    AbsoluteScreenPixelRectangle aScrRect = pWin->GetDesktopRectPixel();
    AbsoluteScreenPixelPoint aRootPos = pWin->OutputToAbsoluteScreenPixel(Point(0,0));
    if (bRightScroll)
    {
        double nWinRight = rWinSize.getWidth() + aRootPos.getX();
        double nMarginRight = aScrRect.GetWidth() - nWinRight;
        double nHOffset = rEffPos.X() - rWinSize.Width();
        double nHAccelRate = nHOffset / nMarginRight;

        if (nHAccelRate > 1.0)
            nHAccelRate = 1.0;

        nUpdateInterval = SELENG_AUTOREPEAT_INTERVAL_MAX*(1.0 - nHAccelRate);
    }

    if (bLeftScroll)
    {
        double nMarginLeft = aRootPos.getX();
        double nHOffset = -rEffPos.X();
        double nHAccelRate = nHOffset / nMarginLeft;

        if (nHAccelRate > 1.0)
            nHAccelRate = 1.0;

        sal_uLong nTmp = static_cast<sal_uLong>(SELENG_AUTOREPEAT_INTERVAL_MAX*(1.0 - nHAccelRate));
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

        sal_uInt64 nTmp = SELENG_AUTOREPEAT_INTERVAL_MAX*(1.0 - nVAccelRate);
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

        sal_uInt64 nTmp = SELENG_AUTOREPEAT_INTERVAL_MAX*(1.0 - nVAccelRate);
        if (nUpdateInterval > nTmp)
            nUpdateInterval = nTmp;
    }

#ifdef _WIN32
    ScTabViewShell* pViewShell = m_rViewData.GetViewShell();
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
    m_pEngine = pSelEngine;
}

// Drag & Drop
void ScViewFunctionSet::BeginDrag()
{
    if (m_rViewData.GetViewShell()->IsLokReadOnlyView())
        return;

    SCTAB nTab = m_rViewData.GetTabNo();

    SCCOL nPosX;
    SCROW nPosY;
    if (m_pEngine)
    {
        Point aMPos = m_pEngine->GetMousePosPixel();
        m_rViewData.GetPosFromPixel( aMPos.X(), aMPos.Y(), GetWhich(), nPosX, nPosY );
    }
    else
    {
        nPosX = m_rViewData.GetCurX();
        nPosY = m_rViewData.GetCurY();
    }

    ScModule* pScMod = ScModule::get();
    bool bRefMode = pScMod->IsFormulaMode();
    if (bRefMode)
        return;

    m_rViewData.GetView()->FakeButtonUp( GetWhich() );   // ButtonUp is swallowed

    ScMarkData& rMark = m_rViewData.GetMarkData();
    rMark.MarkToSimple();
    if ( !rMark.IsMarked() || rMark.IsMultiMarked() )
        return;

    ScDocumentUniquePtr pClipDoc(new ScDocument( SCDOCMODE_CLIP ));
    // bApi = TRUE -> no error messages
    bool bCopied = m_rViewData.GetView()->CopyToClip( pClipDoc.get(), false, true );
    if ( !bCopied )
        return;

    sal_Int8 nDragActions = m_rViewData.GetView()->SelectionEditable() ?
                            ( DND_ACTION_COPYMOVE | DND_ACTION_LINK ) :
                            ( DND_ACTION_COPY | DND_ACTION_LINK );

    ScDocShell* pDocSh = m_rViewData.GetDocShell();
    TransferableObjectDescriptor aObjDesc;
    pDocSh->FillTransferableObjectDescriptor( aObjDesc );
    aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
    // maSize is set in ScTransferObj ctor

    rtl::Reference<ScTransferObj> pTransferObj = new ScTransferObj( std::move(pClipDoc), std::move(aObjDesc) );

    // set position of dragged cell within range
    ScRange aMarkRange = pTransferObj->GetRange();
    SCCOL nStartX = aMarkRange.aStart.Col();
    SCROW nStartY = aMarkRange.aStart.Row();
    SCCOL nHandleX = (nPosX >= nStartX) ? nPosX - nStartX : 0;
    SCROW nHandleY = (nPosY >= nStartY) ? nPosY - nStartY : 0;
    pTransferObj->SetDragHandlePos( nHandleX, nHandleY );
    pTransferObj->SetSourceCursorPos( m_rViewData.GetCurX(), m_rViewData.GetCurY() );
    pTransferObj->SetVisibleTab( nTab );

    pTransferObj->SetDragSource( pDocSh, rMark );

    vcl::Window* pWindow = m_rViewData.GetActiveWin();
    if ( pWindow->IsTracking() )
        pWindow->EndTracking( TrackingEventFlags::Cancel );    // abort selecting

    if (comphelper::LibreOfficeKit::isActive())
        pWindow->LocalStartDrag();

    pScMod->SetDragObject( pTransferObj.get(), nullptr );      // for internal D&D
    pTransferObj->StartDrag( pWindow, nDragActions );

    return;         // dragging started

}

// Selection
void ScViewFunctionSet::CreateAnchor()
{
    if (m_bAnchor) return;

    bool bRefMode = ScModule::get()->IsFormulaMode();
    if (bRefMode)
        SetAnchor( m_rViewData.GetRefStartX(), m_rViewData.GetRefStartY() );
    else
        SetAnchor( m_rViewData.GetCurX(), m_rViewData.GetCurY() );
}

void ScViewFunctionSet::SetAnchor( SCCOL nPosX, SCROW nPosY )
{
    bool bRefMode = ScModule::get()->IsFormulaMode();
    ScTabView* pView = m_rViewData.GetView();
    SCTAB nTab = m_rViewData.GetTabNo();

    if (bRefMode)
    {
        pView->DoneRefMode();
        m_aAnchorPos.Set( nPosX, nPosY, nTab );
        pView->InitRefMode( m_aAnchorPos.Col(), m_aAnchorPos.Row(), m_aAnchorPos.Tab(),
                            SC_REFTYPE_REF );
        m_bStarted = true;
    }
    else if (m_rViewData.IsAnyFillMode())
    {
        m_aAnchorPos.Set( nPosX, nPosY, nTab );
        m_bStarted = true;
    }
    else
    {
        // don't go there and back again
        if ( m_bStarted && pView->IsMarking( nPosX, nPosY, nTab ) )
        {
            // don't do anything
        }
        else
        {
            pView->DoneBlockMode( true );
            m_aAnchorPos.Set( nPosX, nPosY, nTab );
            ScMarkData& rMark = m_rViewData.GetMarkData();
            if ( rMark.IsMarked() || rMark.IsMultiMarked() )
            {
                pView->InitBlockMode( m_aAnchorPos.Col(), m_aAnchorPos.Row(),
                                      m_aAnchorPos.Tab(), true );
                m_bStarted = true;
            }
            else
                m_bStarted = false;
        }
    }
    m_bAnchor = true;
}

void ScViewFunctionSet::DestroyAnchor()
{
    if (m_rViewData.IsAnyFillMode())
        return;

    bool bRefMode = ScModule::get()->IsFormulaMode();
    if (bRefMode)
        m_rViewData.GetView()->DoneRefMode( true );
    else
        m_rViewData.GetView()->DoneBlockMode( true );

    m_bAnchor = false;
}

void ScViewFunctionSet::SetAnchorFlag( bool bSet )
{
    m_bAnchor = bSet;
}

void ScViewFunctionSet::SetCursorAtPoint( const Point& rPointPixel, bool /* bDontSelectAtCursor */ )
{
    if ( bDidSwitch )
    {
        if ( rPointPixel == aSwitchPos )
            return;                   // don't scroll in wrong window
        else
            bDidSwitch = false;
    }
    aSwitchPos = rPointPixel;       // only important, if bDidSwitch

    //  treat position 0 as -1, so scrolling is always possible
    //  (with full screen and hidden headers, the top left border may be at 0)
    //  (moved from ScViewData::GetPosFromPixel)

    Point aEffPos = rPointPixel;
    if ( aEffPos.X() == 0 )
        aEffPos.setX( -1 );
    if ( aEffPos.Y() == 0 )
        aEffPos.setY( -1 );

    //  Scrolling
    Size aWinSize = m_pEngine->GetWindow()->GetOutputSizePixel();
    bool bLeftScroll  = ( aEffPos.X() < 0 );
    bool bTopScroll = ( aEffPos.Y() < 0 );

    SCCOL  nPosX;
    SCROW  nPosY;
    m_rViewData.GetPosFromPixel( aEffPos.X(), aEffPos.Y(), GetWhich(),
                                  nPosX, nPosY, true, true );     // with Repair

    tools::Rectangle aEditArea = m_rViewData.GetEditArea(GetWhich(), nPosX, nPosY,
                                                          m_pEngine->GetWindow(),
                                                          nullptr, false);

    bool bFillingSelection = m_rViewData.IsFillMode() || m_rViewData.GetFillMode() == ScFillMode::MATRIX;
    bool bBottomScroll;
    bool bRightScroll;
    // for Autofill don't yet assume we want to auto-scroll to the cell under the mouse
    // because the autofill handle extends into a cells neighbours so initial click is usually
    // above a neighbour cell
    if (bFillingSelection)
    {
        bBottomScroll = aEffPos.Y() >= aWinSize.Height();
        bRightScroll  = aEffPos.X() >= aWinSize.Width();
    }
    else
    {
        //in the normal case make the full selected cell visible
        bBottomScroll = aEditArea.Bottom() >= aWinSize.Height();
        bRightScroll  = aEditArea.Right() >= aWinSize.Width();
    }

    bool bScroll = bRightScroll || bBottomScroll || bLeftScroll || bTopScroll;

    // for Autofill switch in the center of cell thereby don't prevent scrolling to bottom/right
    if (bFillingSelection)
    {
        bool bLeft, bTop;
        m_rViewData.GetMouseQuadrant( aEffPos, GetWhich(), nPosX, nPosY, bLeft, bTop );
        ScDocument& rDoc = m_rViewData.GetDocument();
        SCTAB nTab = m_rViewData.GetTabNo();
        if ( bLeft && !bRightScroll )
            do --nPosX; while ( nPosX>=0 && rDoc.ColHidden( nPosX, nTab ) );
        if ( bTop && !bBottomScroll )
        {
            if (--nPosY >= 0)
            {
                nPosY = rDoc.LastVisibleRow(0, nPosY, nTab);
                if (!rDoc.ValidRow(nPosY))
                    nPosY = -1;
            }
        }
        // negative value is allowed
    }

    // moved out of fix limit?
    ScSplitPos eWhich = GetWhich();
    if ( eWhich == m_rViewData.GetActivePart() )
    {
        if ( m_rViewData.GetHSplitMode() == SC_SPLIT_FIX )
            if ( aEffPos.X() >= aWinSize.Width() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                {
                    m_rViewData.GetView()->ActivatePart( SC_SPLIT_TOPRIGHT );
                    bScroll = false;
                    bDidSwitch = true;
                }
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                {
                    m_rViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
                    bScroll = false;
                    bDidSwitch = true;
                }
            }

        if ( m_rViewData.GetVSplitMode() == SC_SPLIT_FIX )
            if ( aEffPos.Y() >= aWinSize.Height() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                {
                    m_rViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT );
                    bScroll = false;
                    bDidSwitch = true;
                }
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                {
                    m_rViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
                    bScroll = false;
                    bDidSwitch = true;
                }
            }
    }

    if (bScroll)
    {
        // Adjust update interval based on how far the mouse pointer is from the edge.
        sal_uInt64 nUpdateInterval = CalcUpdateInterval(
            aWinSize, aEffPos, bLeftScroll, bTopScroll, bRightScroll, bBottomScroll);
        m_pEngine->SetUpdateInterval(nUpdateInterval);
    }
    else
    {
        // Don't forget to reset the interval when not scrolling!
        m_pEngine->SetUpdateInterval(SELENG_AUTOREPEAT_INTERVAL);
    }

    m_rViewData.ResetOldCursor();
    SetCursorAtCell( nPosX, nPosY, bScroll );
}

bool ScViewFunctionSet::CheckRefBounds(SCCOL nPosX, SCROW nPosY)
{
    SCCOL startX = m_rViewData.GetRefStartX();
    SCROW startY = m_rViewData.GetRefStartY();

    SCCOL endX = m_rViewData.GetRefEndX();
    SCROW endY = m_rViewData.GetRefEndY();

    return nPosX >= startX && nPosX <= endX && nPosY >= startY && nPosY <= endY;
}

bool ScViewFunctionSet::SetCursorAtCell( SCCOL nPosX, SCROW nPosY, bool bScroll )
{
    ScTabView* pView = m_rViewData.GetView();
    SCTAB nTab = m_rViewData.GetTabNo();
    ScDocument& rDoc = m_rViewData.GetDocument();

    if ( rDoc.IsTabProtected(nTab) )
    {
        if (nPosX < 0 || nPosY < 0)
            return false;

        const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
        if (!pProtect)
            return false;

        bool bSkipProtected   = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bool bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);

        if ( bSkipProtected && bSkipUnprotected )
            return false;

        bool bCellProtected = rDoc.HasAttrib(nPosX, nPosY, nTab, nPosX, nPosY, nTab, HasAttrFlags::Protected);
        if ( (bCellProtected && bSkipProtected) || (!bCellProtected && bSkipUnprotected) )
            // Don't select this cell!
            return false;
    }

    ScTabViewShell* pViewShell = m_rViewData.GetViewShell();
    bool bRefMode = pViewShell && pViewShell->IsRefInputMode();

    bool bHide = !bRefMode && !m_rViewData.IsAnyFillMode() &&
            ( nPosX != m_rViewData.GetCurX() || nPosY != m_rViewData.GetCurY() );

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
        if ( !ScModule::get()->IsModalMode(m_rViewData.GetSfxDocShell()) && (!CheckRefBounds(nPosX, nPosY) || SfxLokHelper::getDeviceFormFactor() != LOKDeviceFormFactor::MOBILE))
        {
            if (!m_bAnchor)
            {
                pView->DoneRefMode( true );
                pView->InitRefMode( nPosX, nPosY, m_rViewData.GetTabNo(), SC_REFTYPE_REF );
            }

            if(SfxLokHelper::getDeviceFormFactor() != LOKDeviceFormFactor::MOBILE)
                pView->UpdateRef( nPosX, nPosY, m_rViewData.GetTabNo() );

            pView->SelectionChanged();
        }
    }
    else if (m_rViewData.IsFillMode() ||
            (m_rViewData.GetFillMode() == ScFillMode::MATRIX && (nScFillModeMouseModifier & KEY_MOD1) ))
    {
        // If a matrix got touched, switch back to Autofill is possible with Ctrl

        SCCOL nStartX, nEndX;
        SCROW nStartY, nEndY; // Block
        SCTAB nDummy;
        m_rViewData.GetSimpleArea( nStartX, nStartY, nDummy, nEndX, nEndY, nDummy );

        if (m_rViewData.GetRefType() != SC_REFTYPE_FILL)
        {
            pView->InitRefMode( nStartX, nStartY, nTab, SC_REFTYPE_FILL );
            CreateAnchor();
        }

        ScRange aDelRange;
        bool bOldDelMark = m_rViewData.GetDelMark( aDelRange );

        if ( nPosX+1 >= nStartX && nPosX <= nEndX &&
             nPosY+1 >= nStartY && nPosY <= nEndY &&
             ( nPosX != nEndX || nPosY != nEndY ) )                     // minimize?
        {
            // direction (left or top)

            tools::Long nSizeX = 0;
            for (SCCOL i=nPosX+1; i<=nEndX; i++)
                nSizeX += rDoc.GetColWidth( i, nTab );
            tools::Long nSizeY = rDoc.GetRowHeight( nPosY+1, nEndY, nTab );

            SCCOL nDelStartX = nStartX;
            SCROW nDelStartY = nStartY;
            if ( nSizeX > nSizeY )
                nDelStartX = nPosX + 1;
            else
                nDelStartY = nPosY + 1;
            // there is no need to check for zero, because nPosX/Y is also negative

            if ( nDelStartX < nStartX )
                nDelStartX = nStartX;
            if ( nDelStartY < nStartY )
                nDelStartY = nStartY;

            // set range

            m_rViewData.SetDelMark( ScRange( nDelStartX,nDelStartY,nTab,
                                              nEndX,nEndY,nTab ) );
            m_rViewData.GetView()->UpdateShrinkOverlay();

            m_rViewData.GetView()->
                PaintArea( nStartX,nDelStartY, nEndX,nEndY, ScUpdateMode::Marks );

            nPosX = nEndX;      // keep red border around range
            nPosY = nEndY;

            // reference the right way up, if it's upside down below
            if ( nStartX != m_rViewData.GetRefStartX() || nStartY != m_rViewData.GetRefStartY() )
            {
                m_rViewData.GetView()->DoneRefMode();
                m_rViewData.GetView()->InitRefMode( nStartX, nStartY, nTab, SC_REFTYPE_FILL );
            }
        }
        else
        {
            if ( bOldDelMark )
            {
                m_rViewData.ResetDelMark();
                m_rViewData.GetView()->UpdateShrinkOverlay();
            }

            bool bNegX = ( nPosX < nStartX );
            bool bNegY = ( nPosY < nStartY );

            tools::Long nSizeX = 0;
            if ( bNegX )
            {
                //  in SetCursorAtPoint hidden columns are skipped.
                //  They must be skipped here too, or the result will always be the first hidden column.
                do ++nPosX; while ( nPosX<nStartX && rDoc.ColHidden(nPosX, nTab) );
                for (SCCOL i=nPosX; i<nStartX; i++)
                    nSizeX += rDoc.GetColWidth( i, nTab );
            }
            else
                for (SCCOL i=nEndX+1; i<=nPosX; i++)
                    nSizeX += rDoc.GetColWidth( i, nTab );

            tools::Long nSizeY = 0;
            if ( bNegY )
            {
                //  in SetCursorAtPoint hidden rows are skipped.
                //  They must be skipped here too, or the result will always be the first hidden row.
                if (++nPosY < nStartY)
                {
                    nPosY = rDoc.FirstVisibleRow(nPosY, nStartY-1, nTab);
                    if (!rDoc.ValidRow(nPosY))
                        nPosY = nStartY;
                }
                nSizeY += rDoc.GetRowHeight( nPosY, nStartY-1, nTab );
            }
            else
                nSizeY += rDoc.GetRowHeight( nEndY+1, nPosY, nTab );

            if ( nSizeX > nSizeY )          // Fill only ever in one direction
            {
                nPosY = nEndY;
                bNegY = false;
            }
            else
            {
                nPosX = nEndX;
                bNegX = false;
            }

            SCCOL nRefStX = bNegX ? nEndX : nStartX;
            SCROW nRefStY = bNegY ? nEndY : nStartY;
            if ( nRefStX != m_rViewData.GetRefStartX() || nRefStY != m_rViewData.GetRefStartY() )
            {
                m_rViewData.GetView()->DoneRefMode();
                m_rViewData.GetView()->InitRefMode( nRefStX, nRefStY, nTab, SC_REFTYPE_FILL );
            }
        }

        pView->UpdateRef( nPosX, nPosY, nTab );
    }
    else if (m_rViewData.IsAnyFillMode())
    {
        ScFillMode nMode = m_rViewData.GetFillMode();
        if ( nMode == ScFillMode::EMBED_LT || nMode == ScFillMode::EMBED_RB )
        {
            OSL_ENSURE( rDoc.IsEmbedded(), "!rDoc.IsEmbedded()" );
            ScRange aRange;
            rDoc.GetEmbedded( aRange);
            ScRefType eRefMode = (nMode == ScFillMode::EMBED_LT) ? SC_REFTYPE_EMBED_LT : SC_REFTYPE_EMBED_RB;
            if (m_rViewData.GetRefType() != eRefMode)
            {
                if ( nMode == ScFillMode::EMBED_LT )
                    pView->InitRefMode( aRange.aEnd.Col(), aRange.aEnd.Row(), nTab, eRefMode );
                else
                    pView->InitRefMode( aRange.aStart.Col(), aRange.aStart.Row(), nTab, eRefMode );
                CreateAnchor();
            }

            pView->UpdateRef( nPosX, nPosY, nTab );
        }
        else if ( nMode == ScFillMode::MATRIX )
        {
            SCCOL nStartX, nEndX;
            SCROW nStartY, nEndY; // Block
            SCTAB nDummy;
            m_rViewData.GetSimpleArea( nStartX, nStartY, nDummy, nEndX, nEndY, nDummy );

            if (m_rViewData.GetRefType() != SC_REFTYPE_FILL)
            {
                pView->InitRefMode( nStartX, nStartY, nTab, SC_REFTYPE_FILL );
                CreateAnchor();
            }

            if ( nPosX < nStartX ) nPosX = nStartX;
            if ( nPosY < nStartY ) nPosY = nStartY;

            pView->UpdateRef( nPosX, nPosY, nTab );
        }
        // else new modes
    }
    else                    // regular selection
    {
        bool bHideCur = m_bAnchor && ( nPosX != m_rViewData.GetCurX() ||
                                       nPosY != m_rViewData.GetCurY() );
        if (bHideCur)
            pView->HideAllCursors();            // otherwise twice: Block and SetCursor

        if (m_bAnchor)
        {
            if (!m_bStarted)
            {
                bool bMove = ( nPosX != m_aAnchorPos.Col() ||
                                nPosY != m_aAnchorPos.Row() );
                if ( bMove || ( m_pEngine && m_pEngine->GetMouseEvent().IsShift() ) )
                {
                    pView->InitBlockMode( m_aAnchorPos.Col(), m_aAnchorPos.Row(),
                                          m_aAnchorPos.Tab(), true );
                    m_bStarted = true;
                }
            }
            if (m_bStarted)
                // If the selection is already started, don't set the cursor.
                pView->MarkCursor( nPosX, nPosY, nTab, false, false, true );
            else
                pView->SetCursor( nPosX, nPosY );
        }
        else
        {
            ScMarkData& rMark = m_rViewData.GetMarkData();
            if (rMark.IsMarked() || rMark.IsMultiMarked())
            {
                pView->DoneBlockMode(true);
                pView->InitBlockMode( nPosX, nPosY, nTab, true );
                pView->MarkCursor( nPosX, nPosY, nTab );

                m_aAnchorPos.Set( nPosX, nPosY, nTab );
                m_bStarted = true;
            }
            // #i3875# *Hack* When a new cell is Ctrl-clicked with no pre-selected cells,
            // it highlights that new cell as well as the old cell where the cursor is
            // positioned prior to the click.  A selection mode via Shift-F8 should also
            // follow the same behavior.
            else if ( m_rViewData.IsSelCtrlMouseClick() )
            {
                SCCOL nOldX = m_rViewData.GetCurX();
                SCROW nOldY = m_rViewData.GetCurY();

                pView->InitBlockMode( nOldX, nOldY, nTab, true );
                pView->MarkCursor( nOldX, nOldY, nTab );

                if ( nOldX != nPosX || nOldY != nPosY )
                {
                    pView->DoneBlockMode( true );
                    pView->InitBlockMode( nPosX, nPosY, nTab, true );
                    pView->MarkCursor( nPosX, nPosY, nTab );
                    m_aAnchorPos.Set( nPosX, nPosY, nTab );
                }

                m_bStarted = true;
            }
            pView->SetCursor( nPosX, nPosY );
        }

        m_rViewData.SetRefStart( nPosX, nPosY, nTab );
        if (bHideCur)
            pView->ShowAllCursors();
    }

    if (bHide)
        pView->ShowAllCursors();

    return true;
}

bool ScViewFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    bool bRefMode = ScModule::get()->IsFormulaMode();
    if (bRefMode)
        return false;

    if (m_rViewData.IsAnyFillMode())
        return false;

    ScMarkData& rMark = m_rViewData.GetMarkData();
    if (m_bAnchor || !rMark.IsMultiMarked())
    {
        SCCOL  nPosX;
        SCROW  nPosY;
        m_rViewData.GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), GetWhich(), nPosX, nPosY );
        return m_rViewData.GetMarkData().IsCellMarked( nPosX, nPosY );
    }

    return false;
}

void ScViewFunctionSet::DeselectAtPoint( const Point& /* rPointPixel */ )
{
    // doesn't exist
}

void ScViewFunctionSet::DeselectAll()
{
    if (m_rViewData.IsAnyFillMode())
        return;

    bool bRefMode = ScModule::get()->IsFormulaMode();
    if (bRefMode)
    {
        m_rViewData.GetView()->DoneRefMode();
    }
    else
    {
        m_rViewData.GetView()->DoneBlockMode();
        m_rViewData.GetViewShell()->UpdateInputHandler();
    }

    m_bAnchor = false;
}

ScViewSelectionEngine::ScViewSelectionEngine( vcl::Window* pWindow, ScTabView* pView,
                                                ScSplitPos eSplitPos ) :
        SelectionEngine( pWindow, &pView->GetFunctionSet() ),
        eWhich( eSplitPos )
{
    SetSelectionMode( SelectionMode::Multiple );
    EnableDrag( true );
}

// column and row headers
ScHeaderFunctionSet::ScHeaderFunctionSet( ScViewData& rData ) :
        rViewData( rData ),
        bColumn( false ),
        eWhich( SC_SPLIT_TOPLEFT ),
        bAnchor( false ),
        nCursorPos( 0 )
{
}

void ScHeaderFunctionSet::SetColumn( bool bSet )
{
    bColumn = bSet;
}

void ScHeaderFunctionSet::SetWhich( ScSplitPos eNew )
{
    eWhich = eNew;
}

void ScHeaderFunctionSet::BeginDrag()
{
    // doesn't exist
}

void ScHeaderFunctionSet::CreateAnchor()
{
    if (bAnchor)
        return;

    ScTabView* pView = rViewData.GetView();
    pView->DoneBlockMode( true );
    if (bColumn)
    {
        pView->InitBlockMode( static_cast<SCCOL>(nCursorPos), 0, rViewData.GetTabNo(), true, true );
        pView->MarkCursor( static_cast<SCCOL>(nCursorPos), rViewData.MaxRow(), rViewData.GetTabNo() );
    }
    else
    {
        pView->InitBlockMode( 0, nCursorPos, rViewData.GetTabNo(), true, false, true );
        pView->MarkCursor( rViewData.MaxCol(), nCursorPos, rViewData.GetTabNo() );
    }
    bAnchor = true;
}

void ScHeaderFunctionSet::DestroyAnchor()
{
    rViewData.GetView()->DoneBlockMode( true );
    bAnchor = false;
}

void ScHeaderFunctionSet::SetCursorAtPoint( const Point& rPointPixel, bool /* bDontSelectAtCursor */ )
{
    if ( bDidSwitch )
    {
        // next valid position has to be originated from another window
        if ( rPointPixel == aSwitchPos )
            return;                   // don't scroll in the wrong window
        else
            bDidSwitch = false;
    }

    //  Scrolling
    Size aWinSize = rViewData.GetActiveWin()->GetOutputSizePixel();
    bool bScroll;
    if (bColumn)
        bScroll = ( rPointPixel.X() < 0 || rPointPixel.X() >= aWinSize.Width() );
    else
        bScroll = ( rPointPixel.Y() < 0 || rPointPixel.Y() >= aWinSize.Height() );

    // moved out of fix limit?
    bool bSwitched = false;
    if ( bColumn )
    {
        if ( rViewData.GetHSplitMode() == SC_SPLIT_FIX )
        {
            if ( rPointPixel.X() > aWinSize.Width() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                {
                    rViewData.GetView()->ActivatePart( SC_SPLIT_TOPRIGHT );
                    bSwitched = true;
                }
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                {
                    rViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
                    bSwitched = true;
                }
            }
        }
    }
    else                // column headers
    {
        if ( rViewData.GetVSplitMode() == SC_SPLIT_FIX )
        {
            if ( rPointPixel.Y() > aWinSize.Height() )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                {
                    rViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT );
                    bSwitched = true;
                }
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                {
                    rViewData.GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
                    bSwitched = true;
                }
            }
        }
    }
    if (bSwitched)
    {
        aSwitchPos = rPointPixel;
        bDidSwitch = true;
        return;               // do not crunch with wrong positions
    }

    SCCOL  nPosX;
    SCROW  nPosY;
    rViewData.GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), rViewData.GetActivePart(),
                                nPosX, nPosY, false );
    if (bColumn)
    {
        nCursorPos = static_cast<SCCOLROW>(nPosX);
        nPosY = rViewData.GetPosY(WhichV(rViewData.GetActivePart()));
    }
    else
    {
        nCursorPos = static_cast<SCCOLROW>(nPosY);
        nPosX = rViewData.GetPosX(WhichH(rViewData.GetActivePart()));
    }

    ScTabView* pView = rViewData.GetView();
    bool bHide = rViewData.GetCurX() != nPosX ||
                 rViewData.GetCurY() != nPosY;
    if (bHide)
        pView->HideAllCursors();

    if (bScroll)
        pView->AlignToCursor( nPosX, nPosY, SC_FOLLOW_LINE );
    pView->SetCursor( nPosX, nPosY );

    if ( !bAnchor || !pView->IsBlockMode() )
    {
        pView->DoneBlockMode( true );
        rViewData.GetMarkData().MarkToMulti();         //! who changes this?
        pView->InitBlockMode( nPosX, nPosY, rViewData.GetTabNo(), true, bColumn, !bColumn );

        bAnchor = true;
    }

    pView->MarkCursor( nPosX, nPosY, rViewData.GetTabNo(), bColumn, !bColumn );

    // SelectionChanged inside of HideCursor because of UpdateAutoFillMark
    pView->SelectionChanged();

    if (bHide)
        pView->ShowAllCursors();
}

bool ScHeaderFunctionSet::IsSelectionAtPoint( const Point& rPointPixel )
{
    SCCOL  nPosX;
    SCROW  nPosY;
    rViewData.GetPosFromPixel( rPointPixel.X(), rPointPixel.Y(), rViewData.GetActivePart(),
                                nPosX, nPosY, false );

    ScMarkData& rMark = rViewData.GetMarkData();
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
    rViewData.GetView()->DoneBlockMode();
    bAnchor = false;
}

ScHeaderSelectionEngine::ScHeaderSelectionEngine( vcl::Window* pWindow, ScHeaderFunctionSet* pFuncSet ) :
        SelectionEngine( pWindow, pFuncSet )
{
    SetSelectionMode( SelectionMode::Multiple );
    EnableDrag( false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
