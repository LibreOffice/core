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

#include <hintids.hxx>


#include <svx/svdview.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdobj.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sfx2/bindings.hxx>

#include <sfx2/viewfrm.hxx>
#include <fmturl.hxx>
#include <frmfmt.hxx>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <swdtflvr.hxx>
#include <swmodule.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <swundo.hxx>

using namespace ::com::sun::star;

// no include "dbgoutsw.hxx" here!!!!!!

extern bool bNoInterrupt;
extern bool bFrmDrag;
extern bool bDDTimerStarted;

bool bExecuteDrag = false;

void SwEditWin::StartDDTimer()
{
    m_aTimer.SetTimeoutHdl(LINK(this, SwEditWin, DDHandler));
    m_aTimer.SetTimeout(480);
    m_aTimer.Start();
    bDDTimerStarted = true;
}


void SwEditWin::StopDDTimer(SwWrtShell *pSh, const Point &rPt)
{
    m_aTimer.Stop();
    bDDTimerStarted = false;
    if(!pSh->IsSelFrmMode())
        pSh->SetCursor(&rPt, false);
    m_aTimer.SetTimeoutHdl(LINK(this,SwEditWin, TimerHandler));
}

void SwEditWin::StartDrag( sal_Int8 /*nAction*/, const Point& rPosPixel )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    if( rSh.GetDrawView() )
    {
        CommandEvent aDragEvent( rPosPixel, COMMAND_STARTDRAG, sal_True );
        if( rSh.GetDrawView()->Command( aDragEvent, this ) )
        {
            m_rView.GetViewFrame()->GetBindings().InvalidateAll(sal_False);
            return; // Event evaluated by SdrView
        }
    }

    if ( !m_pApplyTempl && !rSh.IsDrawCreate() && !IsDrawAction())
    {
        sal_Bool bStart = sal_False, bDelSelect = sal_False;
        SdrObject *pObj = NULL;
        Point aDocPos( PixelToLogic( rPosPixel ) );
        if ( !rSh.IsInSelect() && rSh.ChgCurrPam( aDocPos, sal_True, sal_True))
            //We are not selecting and aren't at a selection
            bStart = sal_True;
        else if ( !bFrmDrag && rSh.IsSelFrmMode() &&
                    rSh.IsInsideSelectedObj( aDocPos ) )
        {
            //We are not dragging internally and are not at an
            //object (frame, draw object)

            bStart = sal_True;
        }
        else if( !bFrmDrag && m_rView.GetDocShell()->IsReadOnly() &&
                OBJCNT_NONE != rSh.GetObjCntType( aDocPos, pObj ))
        {
            rSh.LockPaint();
            if( rSh.SelectObj( aDocPos, 0, pObj ))
                bStart = bDelSelect = sal_True;
            else
                rSh.UnlockPaint();
        }
        else
        {
            SwContentAtPos aSwContentAtPos( SwContentAtPos::SW_INETATTR );
            bStart = rSh.GetContentAtPos( aDocPos,
                        aSwContentAtPos,
                        sal_False );
        }

        if ( bStart && !m_bIsInDrag )
        {
            m_bMBPressed = sal_False;
            ReleaseMouse();
            bFrmDrag = false;
            bExecuteDrag = true;
            SwEditWin::m_nDDStartPosY = aDocPos.Y();
            SwEditWin::m_nDDStartPosX = aDocPos.X();
            m_aMovePos = aDocPos;
            StartExecuteDrag();
            if( bDelSelect )
            {
                rSh.UnSelectFrm();
                rSh.UnlockPaint();
            }
        }
    }
}

void SwEditWin::StartExecuteDrag()
{
    if( !bExecuteDrag || m_bIsInDrag )
        return;

    m_bIsInDrag = sal_True;

    SwTransferable* pTransfer = new SwTransferable( m_rView.GetWrtShell() );
    uno::Reference<
        datatransfer::XTransferable > xRef( pTransfer );

    pTransfer->StartDrag( this, m_aMovePos );
}

void SwEditWin::DragFinished()
{
    DropCleanup();
    m_aTimer.SetTimeoutHdl( LINK(this,SwEditWin, TimerHandler) );
    m_bIsInDrag = sal_False;
}


void SwEditWin::DropCleanup()
{
    SwWrtShell &rSh =  m_rView.GetWrtShell();

    // reset statuses
    bNoInterrupt = false;
    if ( m_bOldIdleSet )
    {
        ((SwViewOption*)rSh.GetViewOptions())->SetIdle( m_bOldIdle );
        m_bOldIdleSet = sal_False;
    }
    if ( m_pUserMarker )
        CleanupDropUserMarker();
    else
        rSh.UnSetVisCrsr();

}

void SwEditWin::CleanupDropUserMarker()
{
    if ( m_pUserMarker )
    {
        delete m_pUserMarker;
        m_pUserMarker = 0;
        m_pUserMarkerObj = 0;
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
    SdrObject *pObj = 0;
    OutlinerView* pOLV;
    rSh.GetObjCntType( aDocPt, pObj );

    if( pObj && 0 != ( pOLV = rSh.GetDrawView()->GetTextEditOutlinerView() ))
    {
        Rectangle aRect( pOLV->GetOutputArea() );
        aRect.Union( pObj->GetLogicRect() );
        const Point aPos = pOLV->GetWindow()->PixelToLogic(rEvt.maPosPixel);
        if ( aRect.IsInside(aPos) )
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
    sal_uInt16 nEventAction;
    sal_Int8 nUserOpt = rEvt.mbDefault ? EXCHG_IN_ACTION_DEFAULT
                                       : rEvt.mnAction;
    m_nDropAction = SotExchange::GetExchangeAction(
                                GetDataFlavorExVector(),
                                m_nDropDestination,
                                rEvt.mnAction,
                                nUserOpt, m_nDropFormat, nEventAction, 0,
                                &rEvt.maDropEvent.Transferable );


    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );
    nRet = rEvt.mnAction;
    if( !SwTransferable::PasteData( aData, rSh, m_nDropAction, m_nDropFormat,
                                m_nDropDestination, sal_False, rEvt.mbDefault, &aDocPt, nRet))
        nRet = DND_ACTION_NONE;
    else if ( SW_MOD()->pDragDrop )
        //Don't clean up anymore at internal D&D!
        SW_MOD()->pDragDrop->SetCleanUp( sal_False );

    return nRet;
}


sal_uInt16 SwEditWin::GetDropDestination( const Point& rPixPnt, SdrObject ** ppObj )
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    const Point aDocPt( PixelToLogic( rPixPnt ) );
    if( rSh.ChgCurrPam( aDocPt ) || rSh.IsOverReadOnlyPos( aDocPt ) )
        return 0;

    SdrObject *pObj = NULL;
    const ObjCntType eType = rSh.GetObjCntType( aDocPt, pObj );

    //Drop to OutlinerView (TextEdit in Drawing) should decide it on its own!
    if( pObj )
    {
        OutlinerView* pOLV = rSh.GetDrawView()->GetTextEditOutlinerView();
        if ( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );
            aRect.Union( pObj->GetLogicRect() );
            const Point aPos = pOLV->GetWindow()->PixelToLogic( rPixPnt );
            if( aRect.IsInside( aPos ) )
                return 0;
        }
    }

    //What do we want to drop on now?
    sal_uInt16 nDropDestination = 0;

    //Did anything else arrive from the DrawingEngine?
    if( OBJCNT_NONE != eType )
    {
        switch ( eType )
        {
        case OBJCNT_GRF:
            {
                sal_Bool bLink,
                    bIMap = 0 != rSh.GetFmtFromObj( aDocPt )->GetURL().GetMap();
                OUString aDummy;
                rSh.GetGrfAtPos( aDocPt, aDummy, bLink );
                if ( bLink && bIMap )
                    nDropDestination = EXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP;
                else if ( bLink )
                    nDropDestination = EXCHG_DEST_DOC_LNKD_GRAPHOBJ;
                else if ( bIMap )
                    nDropDestination = EXCHG_DEST_DOC_GRAPH_W_IMAP;
                else
                    nDropDestination = EXCHG_DEST_DOC_GRAPHOBJ;
            }
            break;
        case OBJCNT_FLY:
            if( rSh.GetView().GetDocShell()->ISA(SwWebDocShell) )
                nDropDestination = EXCHG_DEST_DOC_TEXTFRAME_WEB;
            else
                nDropDestination = EXCHG_DEST_DOC_TEXTFRAME;
            break;
        case OBJCNT_OLE:        nDropDestination = EXCHG_DEST_DOC_OLEOBJ; break;
        case OBJCNT_CONTROL:    /* no Action avail */
        case OBJCNT_SIMPLE:     nDropDestination = EXCHG_DEST_DOC_DRAWOBJ; break;
        case OBJCNT_URLBUTTON:  nDropDestination = EXCHG_DEST_DOC_URLBUTTON; break;
        case OBJCNT_GROUPOBJ:   nDropDestination = EXCHG_DEST_DOC_GROUPOBJ;     break;

        default: OSL_ENSURE( !this, "new ObjectType?" );
        }
    }
    if ( !nDropDestination )
    {
        if( rSh.GetView().GetDocShell()->ISA(SwWebDocShell) )
            nDropDestination = EXCHG_DEST_SWDOC_FREE_AREA_WEB;
        else
            nDropDestination = EXCHG_DEST_SWDOC_FREE_AREA;
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
    Point aPoint;
    Rectangle aWin(aPoint,GetOutputSizePixel());
    const int nMargin = 10;
    aWin.Left() += nMargin;
    aWin.Top() += nMargin;
    aWin.Right() -= nMargin;
    aWin.Bottom() -= nMargin;
    if(!aWin.IsInside(aPixPt)) {
        static sal_uLong last_tick = 0;
        sal_uLong current_tick = Time::GetSystemTicks();
        if((current_tick-last_tick) > 500) {
            last_tick = current_tick;
            if(!m_bOldIdleSet) {
                m_bOldIdle = rSh.GetViewOptions()->IsIdle();
                ((SwViewOption *)rSh.GetViewOptions())->SetIdle(sal_False);
                m_bOldIdleSet = sal_True;
            }
            CleanupDropUserMarker();
            if(aPixPt.X() > aWin.Right()) aPixPt.X() += nMargin;
            if(aPixPt.X() < aWin.Left()) aPixPt.X() -= nMargin;
            if(aPixPt.Y() > aWin.Bottom()) aPixPt.Y() += nMargin;
            if(aPixPt.Y() < aWin.Top()) aPixPt.Y() -= nMargin;
            Point aDocPt(PixelToLogic(aPixPt));
            SwRect rect(aDocPt,Size(1,1));
            rSh.MakeVisible(rect);
        }
    }

    if(m_bOldIdleSet) {
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( m_bOldIdle );
        m_bOldIdleSet = sal_False;
    }

    SdrObject *pObj = NULL;
    m_nDropDestination = GetDropDestination( aPixPt, &pObj );
    if( !m_nDropDestination )
        return DND_ACTION_NONE;

    sal_uInt16 nEventAction;
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
        SwModule *pMod = SW_MOD();
        if( pMod->pDragDrop )
        {
            sal_Bool bCleanup = sal_False;
            //Drawing objects in Headers/Footers are not allowed

            SwWrtShell *pSrcSh = pMod->pDragDrop->GetShell();
            if( (pSrcSh->GetSelFrmType() == FRMTYPE_DRAWOBJ) &&
                pSrcSh->IsSelContainsControl() &&
                 (rSh.GetFrmType( &aDocPt, sal_False ) & (FRMTYPE_HEADER|FRMTYPE_FOOTER)) )
            {
                bCleanup = sal_True;
            }
            // don't more position protected objects!
            else if( DND_ACTION_MOVE == rEvt.mnAction &&
                     pSrcSh->IsSelObjProtected( FLYPROTECT_POS ) )
            {
                bCleanup = sal_True;
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
                rSh.UnSetVisCrsr();
                return DND_ACTION_NONE;
            }
        }
        else
        {
            //D&D from outside of SW should be a Copy per default.
            if( EXCHG_IN_ACTION_DEFAULT == nEventAction &&
                DND_ACTION_MOVE == rEvt.mnAction )
                nEventAction = DND_ACTION_COPY;

            if( (SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE == m_nDropFormat &&
                 EXCHG_IN_ACTION_LINK == m_nDropAction) ||
                 SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE == m_nDropFormat  )
            {
                SdrMarkView* pMView = PTR_CAST( SdrMarkView, rSh.GetDrawView() );
                if( pMView && !pMView->IsDesignMode() )
                    return DND_ACTION_NONE;
            }
        }

        if ( EXCHG_IN_ACTION_DEFAULT != nEventAction )
            nUserOpt = (sal_Int8)nEventAction;

        // show DropCursor or UserMarker ?
        if( EXCHG_DEST_SWDOC_FREE_AREA_WEB == m_nDropDestination ||
            EXCHG_DEST_SWDOC_FREE_AREA == m_nDropDestination )
        {
            CleanupDropUserMarker();
            SwContentAtPos aCont( SwContentAtPos::SW_CONTENT_CHECK );
            if(rSh.GetContentAtPos(aDocPt, aCont))
                rSh.SwCrsrShell::SetVisCrsr( aDocPt );
        }
        else
        {
            rSh.UnSetVisCrsr();

            if ( m_pUserMarkerObj != pObj )
            {
                CleanupDropUserMarker();
                m_pUserMarkerObj = pObj;

                if(m_pUserMarkerObj)
                {
                    m_pUserMarker = new SdrDropMarkerOverlay( *rSh.GetDrawView(), *m_pUserMarkerObj );
                }
            }
        }
        return nUserOpt;
    }

    CleanupDropUserMarker();
    rSh.UnSetVisCrsr();
    return DND_ACTION_NONE;
}


IMPL_LINK_NOARG(SwEditWin, DDHandler)
{
    bDDTimerStarted = false;
    m_aTimer.Stop();
    m_aTimer.SetTimeout(240);
    m_bMBPressed = sal_False;
    ReleaseMouse();
    bFrmDrag = false;

    if ( m_rView.GetViewFrame() )
    {
        bExecuteDrag = true;
        StartExecuteDrag();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
