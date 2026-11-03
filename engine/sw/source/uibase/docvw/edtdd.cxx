/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svx/svdview.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdobj.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sfx2/bindings.hxx>
#include <comphelper/kit.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/kit.hxx>
#include <osl/diagnose.h>

#include <sfx2/viewfrm.hxx>
#include <fmturl.hxx>
#include <frmfmt.hxx>
#include <wrtsh.hxx>
#include <viscrs.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <swdtflvr.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>

using namespace ::com::sun::star;

// no include "dbgoutsw.hxx" here!!!!!!

namespace
{
/// The data of the Writer drag that a drop into rWin takes, or null for a drag from outside
/// Writer. In a kit process the drop lands in the window that its drag started from, while
/// several views drag at the same time. Elsewhere one drag runs at a time, from the window of any
/// view.
SwTransferable* lcl_GetDroppedData(const SwEditWin& rWin)
{
    if (SwTransferable* pDraggedData = rWin.GetDraggedData())
        return pDraggedData;
    if (comphelper::COKit::isActive())
        return nullptr;
    for (SfxViewShell* pViewShell = SfxViewShell::GetFirst(false, checkSfxViewShell<SwView>);
         pViewShell;
         pViewShell = SfxViewShell::GetNext(*pViewShell, false, checkSfxViewShell<SwView>))
    {
        if (SwTransferable* pDraggedData
            = static_cast<SwView*>(pViewShell)->GetEditWin().GetDraggedData())
            return pDraggedData;
    }
    return nullptr;
}
}

void SwEditWin::StartDDTimer()
{
    m_aTimer.SetInvokeHandler(LINK(this, SwEditWin, DDHandler));
    m_aTimer.SetTimeout(480);
    m_aTimer.Start();
    m_bDragTimerStarted = true;
}

void SwEditWin::StopDDTimer(SwWrtShell *pSh, const Point &rPt)
{
    m_aTimer.Stop();
    m_bDragTimerStarted = false;
    if(!pSh->IsSelFrameMode())
        pSh->CallSetCursor(&rPt, false);
    m_aTimer.SetInvokeHandler(LINK(this,SwEditWin, TimerHandler));
}

void SwEditWin::StartDrag( sal_Int8 /*nAction*/, const Point& rPosPixel )
{
    if (m_rView.GetObjectShell()->isContentExtractionLocked()
        || !SwViewOption::IsAllowDragDropText())
        return;

    // In a kit process a read-only view starts no drag.
    if (comphelper::COKit::isActive() && m_rView.IsKitReadOnlyView())
        return;

    SwWrtShell &rSh = m_rView.GetWrtShell();
    if( rSh.GetDrawView() )
    {
        CommandEvent aDragEvent( rPosPixel, CommandEventId::StartDrag, true );
        if( rSh.GetDrawView()->Command( aDragEvent, this ) )
        {
            m_rView.GetViewFrame().GetBindings().InvalidateAll(false);
            return; // Event evaluated by SdrView
        }
    }

    if ( m_pApplyTempl || rSh.IsDrawCreate() || IsDrawAction())
        return;

    bool bStartDragging = false, bDelSelect = false;
    SdrObject *pObj = nullptr;
    Point aDocPos( PixelToLogic( rPosPixel ) );
    const bool bInSelect = rSh.IsInSelect();
    if (!bInSelect && rSh.TestCurrPam(aDocPos, true))
        //We are not selecting and aren't at a selection
        bStartDragging = true;
    // In a kit process only a press inside the text selection starts a drag. The client moves
    // frames and drawing objects itself.
    else if (comphelper::COKit::isActive())
        return;
    // Several selected fly frames move by the frame drag alone, as the drag and drop copy
    // handles one fly frame.
    else if ( !m_bFrameDrag && rSh.IsSelFrameMode() &&
                rSh.IsInsideSelectedObj( aDocPos ) &&
                nullptr == m_pAnchorMarker &&
                !rSh.IsMultipleFlyFramesSelected() )
    {
        //We are not dragging internally and are not at an
        //object (frame, draw object)

        // #i106131# *and* AnchorDrag is *not* active: When active,
        // entering global drag mode will destroy the AnchorHdl but
        // keep the now invalid ptr in place, next access will crash.
        // It is indeed wrong to enter drag mode when AnchorDrag is
        // already active
        bStartDragging = true;
    }
    else if( !m_bFrameDrag && m_rView.GetDocShell()->IsReadOnly() &&
            OBJCNT_NONE != rSh.GetObjCntType( aDocPos, pObj ))
    {
        rSh.LockPaint(LockPaintReason::StartDrag);
        if( rSh.SelectObj( aDocPos, 0, pObj ))
            bStartDragging = bDelSelect = true;
        else
            rSh.UnlockPaint();
    }
    else if (!bInSelect)// tdf#116384 only drag hyperlink if not currently setting the selection
    {
        // Prefer starting a selection (rather than a drag) when at the start/end of a hyperlink.

        // Use the cursor point, not the mouse point, to determine whether this is at the start/end.
        Point aCursorPoint = rSh.GetCharRect().Center();

        SwContentAtPos aSwContentAtPos( IsAttrAtPos::InetAttr );
        bStartDragging = rSh.GetContentAtPos(aCursorPoint, aSwContentAtPos);

        // If true (cursor is in hyperlink), perhaps the cursor is at the start of the hyperlink?
        if (bStartDragging)
        {
            aCursorPoint.AdjustX(-1);
            aSwContentAtPos = SwContentAtPos(IsAttrAtPos::InetAttr);
            bStartDragging = rSh.GetContentAtPos(aCursorPoint, aSwContentAtPos);
        }
    }

    if ( !bStartDragging || m_bIsInDrag )
        return;

    // If the add selection mode has been pushed in the MouseButtonDown handler it needs to be
    // popped or it will remain active and noticeable in the statusbar selection control until the
    // next MouseButtonUp event after the DnD, since a MouseButtonUp event is not received by the
    // edit window when DnD is done.
    if (m_bModePushed)
    {
        rSh.PopMode();
        m_bModePushed = false;
    }

    m_bMBPressed = false;
    ReleaseMouse();

    m_bFrameDrag = false;
    m_aDragStartPos = aDocPos;
    m_aMovePos = aDocPos;
    StartExecuteDrag();
    if( bDelSelect )
    {
        rSh.UnSelectFrame();
        rSh.UnlockPaint();
    }
}

void SwEditWin::StartExecuteDrag()
{
    if( m_bIsInDrag )
        return;

    // A kit process drags only a text selection. The client moves frames and drawing objects
    // itself.
    const bool bKit = comphelper::COKit::isActive();
    if (bKit && m_rView.GetWrtShell().IsSelFrameMode())
        return;

    m_bIsInDrag = true;

    // A kit process has no system drag-and-drop loop. The kit mouse handling turns the following
    // mouse moves into drag-over events and the button release into the drop.
    if (bKit)
        LocalStartDrag();

    // Each window keeps the data of its own drag, so every view can run a drag at the same
    // time.
    m_xDraggedData = new SwTransferable( m_rView.GetWrtShell() );

    m_xDraggedData->StartDrag( this, m_aMovePos );
}

void SwEditWin::CancelDrag()
{
    // The drag source finishes the dragged data, which restores the view the way the end of
    // any drag does.
    vcl::kit::cancelDrag(this);
    m_xDraggedData.clear();
}

void SwEditWin::DragFinished()
{
    DropCleanup();
    m_aTimer.SetInvokeHandler( LINK(this,SwEditWin, TimerHandler) );
    m_bIsInDrag = false;
    m_xDraggedData.clear();
}

void SwEditWin::DropCleanup()
{
    SwWrtShell &rSh =  m_rView.GetWrtShell();

    // reset statuses
    m_bMousePressRunning = false;
    if ( m_bOldIdleSet )
    {
        rSh.GetViewOptions()->SetIdle( m_bOldIdle );
        m_bOldIdleSet = false;
    }
    if ( m_pUserMarker )
        CleanupDropUserMarker();
    else
        rSh.UnSetVisibleCursor();

}

void SwEditWin::CleanupDropUserMarker()
{
    if ( m_pUserMarker )
    {
        m_pUserMarker.reset();
        m_pUserMarkerObj = nullptr;
    }
}

//exhibition hack (MA,MBA)
void SwView::SelectShellForDrop()
{
    if ( !GetCurShell() )
        SelectShell();
}

sal_Int8 SwEditWin::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    GetView().SelectShellForDrop();
    DropCleanup();
    sal_Int8 nRet = DND_ACTION_NONE;

    //A Drop to an open OutlinerView doesn't concern us (also see QueryDrop)
    SwWrtShell &rSh = m_rView.GetWrtShell();
    const Point aDocPt( PixelToLogic( rEvt.maPosPixel ));
    SdrObject *pObj = nullptr;
    OutlinerView* pOLV;
    rSh.GetObjCntType( aDocPt, pObj );

    if( pObj && nullptr != ( pOLV = rSh.GetDrawView()->GetTextEditOutlinerView() ))
    {
        tools::Rectangle aRect( pOLV->GetOutputArea() );
        aRect.Union( pObj->GetLogicRect() );
        const Point aPos = pOLV->GetWindow()->PixelToLogic(rEvt.maPosPixel);
        if ( aRect.Contains(aPos) )
        {
            rSh.StartAllAction();
            rSh.EndAllAction();
            return nRet;
        }
    }

    // There's a special treatment for file lists with a single
    // element, that depends on the actual content of the
    // Transferable to be accessible. Since the transferable
    // may only be accessed after the drop has been accepted
    // (according to KA due to Java D&D), we'll have to
    // reevaluate the drop action once more _with_ the
    // Transferable.
    sal_uInt8 nEventAction;
    sal_Int8 nUserOpt = rEvt.mbDefault ? EXCHG_IN_ACTION_DEFAULT
                                       : rEvt.mnAction;
    SotExchangeActionFlags nActionFlags;
    m_nDropAction = SotExchange::GetExchangeAction(
                                GetDataFlavorExVector(),
                                m_nDropDestination,
                                rEvt.mnAction,
                                nUserOpt, m_nDropFormat, nEventAction, SotClipboardFormatId::NONE,
                                &rEvt.maDropEvent.Transferable,
                                &nActionFlags );

    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );
    nRet = rEvt.mnAction;
    if( !SwTransferable::PasteData( aData, rSh, m_nDropAction, nActionFlags, m_nDropFormat,
                                m_nDropDestination, false, rEvt.mbDefault, &aDocPt, nRet))
        nRet = DND_ACTION_NONE;
    else if (SwTransferable* pDraggedData = lcl_GetDroppedData(*this))
        //Don't clean up anymore at internal D&D!
        pDraggedData->SetCleanUp(false);

    return nRet;
}

SotExchangeDest SwEditWin::GetDropDestination( const Point& rPixPnt, SdrObject ** ppObj )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    const Point aDocPt( PixelToLogic( rPixPnt ) );
    if (rSh.IsOverReadOnlyPos(aDocPt) || rSh.DocPtInsideInputField(aDocPt))
        return SotExchangeDest::NONE;

    SdrObject *pObj = nullptr;
    const ObjCntType eType = rSh.GetObjCntType( aDocPt, pObj );

    //Drop to OutlinerView (TextEdit in Drawing) should decide it on its own!
    if( pObj )
    {
        OutlinerView* pOLV = rSh.GetDrawView()->GetTextEditOutlinerView();
        if ( pOLV )
        {
            tools::Rectangle aRect( pOLV->GetOutputArea() );
            aRect.Union( pObj->GetLogicRect() );
            const Point aPos = pOLV->GetWindow()->PixelToLogic( rPixPnt );
            if( aRect.Contains( aPos ) )
                return SotExchangeDest::NONE;
        }
    }

    //What do we want to drop on now?
    SotExchangeDest nDropDestination = SotExchangeDest::NONE;

    //Did anything else arrive from the DrawingEngine?
    if( OBJCNT_NONE != eType )
    {
        switch ( eType )
        {
        case OBJCNT_GRF:
            {
                bool bLink,
                    bIMap = nullptr != rSh.GetFormatFromObj( aDocPt )->GetURL().GetMap();
                OUString aDummy;
                rSh.GetGrfAtPos( aDocPt, aDummy, bLink );
                if ( bLink && bIMap )
                    nDropDestination = SotExchangeDest::DOC_LNKD_GRAPH_W_IMAP;
                else if ( bLink )
                    nDropDestination = SotExchangeDest::DOC_LNKD_GRAPHOBJ;
                else if ( bIMap )
                    nDropDestination = SotExchangeDest::DOC_GRAPH_W_IMAP;
                else
                    nDropDestination = SotExchangeDest::DOC_GRAPHOBJ;
            }
            break;
        case OBJCNT_FLY:
            if( dynamic_cast< const SwWebDocShell *>( rSh.GetView().GetDocShell() ) != nullptr  )
                nDropDestination = SotExchangeDest::DOC_TEXTFRAME_WEB;
            else
                nDropDestination = SotExchangeDest::DOC_TEXTFRAME;
            break;
        case OBJCNT_OLE:        nDropDestination = SotExchangeDest::DOC_OLEOBJ; break;
        case OBJCNT_CONTROL:    /* no Action avail */
        case OBJCNT_SIMPLE:     nDropDestination = SotExchangeDest::DOC_DRAWOBJ; break;
        case OBJCNT_URLBUTTON:  nDropDestination = SotExchangeDest::DOC_URLBUTTON; break;
        case OBJCNT_GROUPOBJ:   nDropDestination = SotExchangeDest::DOC_GROUPOBJ;     break;

        default: OSL_ENSURE( false, "new ObjectType?" );
        }
    }
    if ( nDropDestination == SotExchangeDest::NONE )
    {
        if( dynamic_cast< const SwWebDocShell *>( rSh.GetView().GetDocShell() ) != nullptr  )
            nDropDestination = SotExchangeDest::SWDOC_FREE_AREA_WEB;
        else
            nDropDestination = SotExchangeDest::SWDOC_FREE_AREA;
    }
    if( ppObj )
        *ppObj = pObj;
    return nDropDestination;
}

sal_Int8 SwEditWin::AcceptDrop( const AcceptDropEvent& rEvt )
{
    if( rEvt.mbLeaving )
    {
        DropCleanup();
        return rEvt.mnAction;
    }

    if( m_rView.GetDocShell()->IsReadOnly() )
        return DND_ACTION_NONE;

    SwWrtShell &rSh = m_rView.GetWrtShell();

    Point aPixPt( rEvt.maPosPixel );

    // If the cursor is near the inner boundary
    // we attempt to scroll towards the desired direction.
    // A kit client scrolls its own view while a drag nears its edge.
    tools::Rectangle aWin(Point(), GetOutputSizePixel());
    const int nMargin = 10;
    aWin.AdjustLeft(nMargin );
    aWin.AdjustTop(nMargin );
    aWin.AdjustRight( -nMargin );
    aWin.AdjustBottom( -nMargin );
    if (!comphelper::COKit::isActive() && !aWin.Contains(aPixPt)) {
        static sal_uInt64 last_tick = 0;
        sal_uInt64 current_tick = tools::Time::GetSystemTicks();
        if((current_tick-last_tick) > 500) {
            last_tick = current_tick;
            if(!m_bOldIdleSet) {
                m_bOldIdle = rSh.GetViewOptions()->IsIdle();
                rSh.GetViewOptions()->SetIdle(false);
                m_bOldIdleSet = true;
            }
            CleanupDropUserMarker();
            if(aPixPt.X() > aWin.Right()) aPixPt.AdjustX(nMargin );
            if(aPixPt.X() < aWin.Left()) aPixPt.AdjustX( -nMargin );
            if(aPixPt.Y() > aWin.Bottom()) aPixPt.AdjustY(nMargin );
            if(aPixPt.Y() < aWin.Top()) aPixPt.AdjustY( -nMargin );
            Point aDocPt(PixelToLogic(aPixPt));
            SwRect rect(aDocPt,Size(1,1));
            rSh.MakeVisible(rect, ScrollSizeMode::ScrollSizeTimer2);
        }
    }

    if(m_bOldIdleSet) {
        rSh.GetViewOptions()->SetIdle( m_bOldIdle );
        m_bOldIdleSet = false;
    }

    SdrObject *pObj = nullptr;
    m_nDropDestination = GetDropDestination( aPixPt, &pObj );
    if( m_nDropDestination == SotExchangeDest::NONE )
        return DND_ACTION_NONE;

    sal_uInt8 nEventAction;
    sal_Int8 nUserOpt = rEvt.mbDefault ? EXCHG_IN_ACTION_DEFAULT
                                       : rEvt.mnAction;

    m_nDropAction = SotExchange::GetExchangeAction(
                                GetDataFlavorExVector(),
                                m_nDropDestination,
                                rEvt.mnAction,
                                nUserOpt, m_nDropFormat, nEventAction );

    if( EXCHG_INOUT_ACTION_NONE != m_nDropAction )
    {
        const Point aDocPt( PixelToLogic( aPixPt ) );

        //With the default action we still want to have a say.
        if (SwTransferable* pDraggedData = lcl_GetDroppedData(*this))
        {
            bool bCleanup = false;
            //Drawing objects in Headers/Footers are not allowed

            SwWrtShell *pSrcSh = pDraggedData->GetShell();
            if( (pSrcSh->GetSelFrameType() == FrameTypeFlags::DRAWOBJ) &&
                pSrcSh->IsSelContainsControl() &&
                 (rSh.GetFrameType( &aDocPt, false ) & (FrameTypeFlags::HEADER|FrameTypeFlags::FOOTER)) )
            {
                bCleanup = true;
            }
            // don't more position protected objects!
            else if( DND_ACTION_MOVE == rEvt.mnAction &&
                     pSrcSh->IsSelObjProtected( FlyProtectFlags::Pos ) != FlyProtectFlags::NONE )
            {
                bCleanup = true;
            }
            else if( rEvt.mbDefault )
            {
                // internal Drag&Drop: within same Doc a Move
                // otherwise a Copy - Task 54974
                nEventAction = pSrcSh->GetDoc() == rSh.GetDoc()
                                    ? DND_ACTION_MOVE
                                    : DND_ACTION_COPY;
            }
            if ( bCleanup )
            {
                CleanupDropUserMarker();
                rSh.UnSetVisibleCursor();
                return DND_ACTION_NONE;
            }
        }
        else
        {
            //D&D from outside of SW should be a Copy per default.
            if( EXCHG_IN_ACTION_DEFAULT == nEventAction &&
                DND_ACTION_MOVE == rEvt.mnAction )
                nEventAction = DND_ACTION_COPY;

            if( (SotClipboardFormatId::SBA_FIELDDATAEXCHANGE == m_nDropFormat &&
                 EXCHG_IN_ACTION_LINK == m_nDropAction) ||
                 SotClipboardFormatId::SBA_CTRLDATAEXCHANGE == m_nDropFormat  )
            {
                SdrMarkView* pMView = rSh.GetDrawView();
                if( pMView && !pMView->IsDesignMode() )
                    return DND_ACTION_NONE;
            }
        }

        if ( EXCHG_IN_ACTION_DEFAULT != nEventAction )
            nUserOpt = static_cast<sal_Int8>(nEventAction);

        // show DropCursor or UserMarker ?
        if( SotExchangeDest::SWDOC_FREE_AREA_WEB == m_nDropDestination ||
            SotExchangeDest::SWDOC_FREE_AREA == m_nDropDestination )
        {
            CleanupDropUserMarker();
            SwContentAtPos aCont( IsAttrAtPos::ContentCheck );
            if(rSh.GetContentAtPos(aDocPt, aCont))
            {
                rSh.GetVisibleCursor()->SetDropPosition(true);
                rSh.SwCursorShell::SetVisibleCursor( aDocPt, ScrollSizeMode::ScrollSizeMouseSelection );
            }
        }
        else
        {
            rSh.UnSetVisibleCursor();

            if ( m_pUserMarkerObj != pObj )
            {
                CleanupDropUserMarker();
                m_pUserMarkerObj = pObj;

                if(m_pUserMarkerObj)
                {
                    m_pUserMarker.reset(new SdrDropMarkerOverlay( *rSh.GetDrawView(), *m_pUserMarkerObj ));
                }
            }
        }
        return nUserOpt;
    }

    CleanupDropUserMarker();
    rSh.UnSetVisibleCursor();
    return DND_ACTION_NONE;
}

IMPL_LINK_NOARG(SwEditWin, DDHandler, Timer *, void)
{
    m_bDragTimerStarted = false;
    m_aTimer.Stop();
    m_aTimer.SetTimeout(240);
    m_bMBPressed = false;
    ReleaseMouse();
    m_bFrameDrag = false;
    StartExecuteDrag();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
