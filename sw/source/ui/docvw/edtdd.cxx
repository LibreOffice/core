/*************************************************************************
 *
 *  $RCSfile: edtdd.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:22:54 $
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

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _SVDVMARK_HXX //autogen
#include <svx/svdvmark.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif


#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif


extern BOOL bNoInterrupt;
extern BOOL bFrmDrag;
extern BOOL bDDTimerStarted;

BOOL bExecuteDrag = FALSE;

void SwEditWin::StartDDTimer()
{
    aTimer.SetTimeoutHdl(LINK(this, SwEditWin, DDHandler));
    aTimer.SetTimeout(480);
    aTimer.Start();
    bDDTimerStarted = TRUE;
}


void SwEditWin::StopDDTimer(SwWrtShell *pSh, const Point &rPt)
{
    aTimer.Stop();
    bDDTimerStarted = FALSE;
    if(!pSh->IsSelFrmMode())
        (pSh->*pSh->fnSetCrsr)(&rPt,FALSE);
    aTimer.SetTimeoutHdl(LINK(this,SwEditWin, TimerHandler));
}

void SwEditWin::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    if( rSh.GetDrawView() )
    {
        CommandEvent aDragEvent( rPosPixel, COMMAND_STARTDRAG, TRUE );
        if( rSh.GetDrawView()->Command( aDragEvent, this ) )
        {
            rView.GetViewFrame()->GetBindings().InvalidateAll(FALSE);
            return; // Event von der SdrView ausgewertet
        }
    }

    if ( !pApplyTempl && !rSh.IsDrawCreate() && !IsDrawAction())
    {
        BOOL bStart = FALSE, bDelSelect = FALSE;
        SdrObject *pObj = NULL;
        Point aDocPos( PixelToLogic( rPosPixel ) );
        if ( !rSh.IsInSelect() && rSh.ChgCurrPam( aDocPos, TRUE, TRUE))
            //Wir sind nicht beim Selektieren und stehen auf einer
            //Selektion
            bStart = TRUE;
        else if ( !bFrmDrag && rSh.IsSelFrmMode() &&
                    rSh.IsInsideSelectedObj( aDocPos ) )
        {
            //Wir sind nicht am internen Draggen und stehen auf
            //einem Objekt (Rahmen, Zeichenobjekt)

            bStart = TRUE;
        }
        else if( !bFrmDrag && rView.GetDocShell()->IsReadOnly() &&
                OBJCNT_NONE != rSh.GetObjCntType( aDocPos, pObj ))
        {
            rSh.LockPaint();
            if( rSh.SelectObj( aDocPos, 0, pObj ))
                bStart = bDelSelect = TRUE;
            else
                rSh.UnlockPaint();
        }
        else
        {
            SwContentAtPos aSwContentAtPos( SwContentAtPos::SW_INETATTR );
            bStart = rSh.GetContentAtPos( aDocPos,
                        aSwContentAtPos,
                        FALSE );
        }

        if ( bStart && !bIsInDrag )
        {
            bMBPressed = FALSE;
            ReleaseMouse();
            bFrmDrag = FALSE;
            bExecuteDrag = TRUE;
            SwEditWin::nDDStartPosY = aDocPos.Y();
            SwEditWin::nDDStartPosX = aDocPos.X();
            aMovePos = aDocPos;
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
    if( !bExecuteDrag || bIsInDrag )
        return;

    bIsInDrag = TRUE;

    SwTransferable* pTransfer = new SwTransferable( rView.GetWrtShell() );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::datatransfer::XTransferable > xRef( pTransfer );

    pTransfer->StartDrag( this, aMovePos );
}

void SwEditWin::DragFinished()
{
    DropCleanup();
    aTimer.SetTimeoutHdl( LINK(this,SwEditWin, TimerHandler) );
    bIsInDrag = FALSE;
}


void SwEditWin::DropCleanup()
{
    SwWrtShell &rSh =  rView.GetWrtShell();

    // Stati zuruecksetzen
    bNoInterrupt = FALSE;
    if ( bOldIdleSet )
    {
        ((SwViewOption*)rSh.GetViewOptions())->SetIdle( bOldIdle );
        bOldIdleSet = FALSE;
    }
    if ( pUserMarker )
        CleanupDropUserMarker();
    else
        rSh.UnSetVisCrsr();

}

void SwEditWin::CleanupDropUserMarker()
{
    if ( pUserMarker )
    {
        delete pUserMarker, pUserMarker = 0;
        pUserMarkerObj = 0;
    }
}


//Messehack (MA,MBA)
void lcl_SelectShellForDrop( SwView &rView )
{
    if ( !rView.GetCurShell() )
        rView.SelectShell();
}

sal_Int8 SwEditWin::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    ::lcl_SelectShellForDrop( GetView() );
    DropCleanup();
    sal_Int8 nRet = DND_ACTION_NONE;

    //Ein Drop auf eine offene OutlinerView geht uns nichts an (siehe auch QueryDrop)
    SwWrtShell &rSh = rView.GetWrtShell();
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
//!!            sal_Int8 nRet = DND_ACTION_NONE/*pOLV->ExecuteDrop( rEvt )*/;
            rSh.EndAllAction();
            return nRet;
        }
    }


    // dvo 2002-05-27, #99027#: There's a special treatment for file lists with a single
    //                          element, that depends on the actual content of the
    //                          Transferable to be accessible. Since the transferable
    //                          may only be accessed after the drop has been accepted
    //                          (according to KA due to Java D&D), we'll have to
    //                          reevaluate the drop action once more _with_ the
    //                          Transferable.
    USHORT nEventAction;
    sal_Int8 nUserOpt = rEvt.mbDefault ? EXCHG_IN_ACTION_DEFAULT
                                       : rEvt.mnAction;
    nDropAction = SotExchange::GetExchangeAction(
                                GetDataFlavorExVector(),
                                nDropDestination,
                                rEvt.mnAction,
//!!                                rEvt.GetSourceOptions(),
                                nUserOpt, nDropFormat, nEventAction, 0,
                                &rEvt.maDropEvent.Transferable );


    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );
    nRet = rEvt.mnAction;
    if( !SwTransferable::PasteData( aData, rSh, nDropAction, nDropFormat,
                                nDropDestination, FALSE, rEvt.mbDefault, &aDocPt, nRet))
//!!    nRet = OFF_APP()->ExecuteDrop( rEvt );
        nRet = DND_ACTION_NONE;
    else if ( SW_MOD()->pDragDrop )
        //Bei internem D&D nicht mehr aufraeumen!
        SW_MOD()->pDragDrop->SetCleanUp( FALSE );

    return nRet;
}


USHORT SwEditWin::GetDropDestination( const Point& rPixPnt, SdrObject ** ppObj )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    const Point aDocPt( PixelToLogic( rPixPnt ) );
    if( rSh.ChgCurrPam( aDocPt ) || rSh.IsOverReadOnlyPos( aDocPt ) )
        return 0;

    SdrObject *pObj = NULL;
    const ObjCntType eType = rSh.GetObjCntType( aDocPt, pObj );

    //Drop auf OutlinerView (TextEdit im Drawing) soll diese selbst entscheiden!
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

    //Auf was wollen wir denn gerade droppen?
    USHORT nDropDestination = 0;

    //Sonst etwas aus der DrawingEngine getroffen?
    if( OBJCNT_NONE != eType )
    {
        switch ( eType )
        {
        case OBJCNT_GRF:
            {
                BOOL bLink,
                    bIMap = 0 != rSh.GetFmtFromObj( aDocPt )->GetURL().GetMap();
                String aDummy;
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

        default: ASSERT( !this, "new ObjectType?" );
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

    if( rView.GetDocShell()->IsReadOnly() )
        return DND_ACTION_NONE;

    SwWrtShell &rSh = rView.GetWrtShell();

    //Ein bischen scrollen?
    Point aPixPt( rEvt.maPosPixel );
    Point aPoint;
    Rectangle aWin( aPoint, GetOutputSizePixel() );
    Rectangle aWin2( aWin );
    aWin.Left()  += 10;
    aWin.Top()   += 10;
    aWin.Right() -= 10;
    aWin.Bottom()-= 10;

    if ( !aWin.IsInside( aPixPt ) )
    {
        if ( !bOldIdleSet )
        {
            bOldIdle = rSh.GetViewOptions()->IsIdle();
            ((SwViewOption *)rSh.GetViewOptions())->SetIdle(FALSE);
            bOldIdleSet = TRUE;
        }
        CleanupDropUserMarker();

        aWin2.Left()  += 5;
        aWin2.Top()   += 5;
        aWin2.Right() -= 5;
        aWin2.Bottom()-= 5;

        long x = 0;
        if ( aPixPt.Y() < aWin2.Top() )
            x = aPixPt.Y() = -5;
        else if ( aPixPt.Y() > aWin2.Bottom() )
            x = aPixPt.Y() += 10;
        if ( aPixPt.X() < aWin2.Left() )
            x = aPixPt.X() = -5;
        else if ( aPixPt.X() > aWin2.Right() )
            x = aPixPt.X() += 10;
        Point aDocPt( PixelToLogic( aPixPt ) );
        aDocPt = rSh.GetCntntPos( aDocPt, x > 0 );
        rSh.SwCrsrShell::SetVisCrsr( aDocPt );
        return TRUE;
    }

    if ( bOldIdleSet )
    {
        ((SwViewOption *)rSh.GetViewOptions())->SetIdle( bOldIdle );
        bOldIdleSet = FALSE;
    }

    SdrObject *pObj = NULL;
    nDropDestination = GetDropDestination( aPixPt, &pObj );
    if( !nDropDestination )
        return DND_ACTION_NONE;

    USHORT nEventAction;
    sal_Int8 nUserOpt = rEvt.mbDefault ? EXCHG_IN_ACTION_DEFAULT
                                       : rEvt.mnAction;

    nDropAction = SotExchange::GetExchangeAction(
                                GetDataFlavorExVector(),
                                nDropDestination,
                                rEvt.mnAction,
//!!                                rEvt.GetSourceOptions(),
                                nUserOpt, nDropFormat, nEventAction );

    if( EXCHG_INOUT_ACTION_NONE != nDropAction )
    {
        const Point aDocPt( PixelToLogic( aPixPt ) );

        //Bei den default Aktionen wollen wir noch ein bischen mitreden.
        SwModule *pMod = SW_MOD();
        if( pMod->pDragDrop )
        {
            BOOL bCleanup = FALSE;
            //Zeichenobjekte in Kopf-/Fusszeilen sind nicht erlaubt

            SwWrtShell *pSrcSh = pMod->pDragDrop->GetShell();
            if( (pSrcSh->GetSelFrmType() == FRMTYPE_DRAWOBJ) &&
                 (rSh.GetFrmType( &aDocPt, FALSE ) & (FRMTYPE_HEADER|FRMTYPE_FOOTER)) )
            {
                bCleanup = TRUE;
            }
            // keine positionsgeschuetzten Objecte verschieben!
            else if( DND_ACTION_MOVE == rEvt.mnAction &&
                     pSrcSh->IsSelObjProtected( FLYPROTECT_POS ) )
            {
                bCleanup = TRUE;
            }
            else if( rEvt.mbDefault )
            {
                // JP 13.08.98: internes Drag&Drop: bei gleichem Doc ein Move
                //              ansonten ein Copy - Task 54974
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
            //D&D von ausserhalb des SW soll per default ein Copy sein.
            if( EXCHG_IN_ACTION_DEFAULT == nEventAction &&
                DND_ACTION_MOVE == rEvt.mnAction )
                nEventAction = DND_ACTION_COPY;

            if( (SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE == nDropFormat &&
                 EXCHG_IN_ACTION_LINK == nDropAction) ||
                 SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE == nDropFormat  )
            {
                SdrMarkView* pMView = PTR_CAST( SdrMarkView, rSh.GetDrawView() );
                if( pMView && !pMView->IsDesignMode() )
                    return DND_ACTION_NONE;
            }
            //controls cannot be created for the complete table
            if(EXCHG_IN_ACTION_LINK == nDropAction &&
                SOT_FORMATSTR_ID_SBA_DATAEXCHANGE == nDropFormat)
                return DND_ACTION_NONE;
        }

        if ( EXCHG_IN_ACTION_DEFAULT != nEventAction )
            nUserOpt = nEventAction;

        // show DropCursor or UserMarker ?
        if( EXCHG_DEST_SWDOC_FREE_AREA_WEB == nDropDestination ||
            EXCHG_DEST_SWDOC_FREE_AREA == nDropDestination )
        {
            CleanupDropUserMarker();
            rSh.SwCrsrShell::SetVisCrsr( aDocPt );
        }
        else
        {
            rSh.UnSetVisCrsr();
            if ( !pUserMarker )
                pUserMarker = new SdrViewUserMarker( rSh.GetDrawView() );
            if ( pUserMarkerObj != pObj )
            {
                pUserMarkerObj = pObj;
                pUserMarker->SetXPolyPolygon( pUserMarkerObj,
                    rSh.GetDrawView()->GetPageView(
                            rSh.GetDrawView()->GetModel()->GetPage(0)));
                pUserMarker->Show();
            }
        }
        return nUserOpt;
    }

    CleanupDropUserMarker();
    rSh.UnSetVisCrsr();
//!!    return OFF_APP()->AcceptDrop( rEvt );
    return DND_ACTION_NONE;
}


IMPL_LINK( SwEditWin, DDHandler, Timer *, EMPTYARG )
{
    bDDTimerStarted = FALSE;
    aTimer.Stop();
    aTimer.SetTimeout(240);
    bMBPressed = FALSE;
    ReleaseMouse();
    bFrmDrag = FALSE;

    if ( rView.GetViewFrame() && rView.GetViewFrame()->GetFrame()  &&
         !rView.GetViewFrame()->GetFrame()->TransferForReplaceInProgress() )
    {
        bExecuteDrag = TRUE;
        StartExecuteDrag();
    }
    return 0;
}



