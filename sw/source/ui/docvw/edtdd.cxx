/*************************************************************************
 *
 *  $RCSfile: edtdd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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
#ifndef _DATAEX_HXX
#include <dataex.hxx>
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

static BOOL bDroped = FALSE;
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


void SwEditWin::StartExecuteDrag()
{
    if( !bExecuteDrag || bIsInDrag )
        return;

    bDroped = FALSE;
    bIsInDrag = TRUE;

    SwWrtShell *pSh = &rView.GetWrtShell();

    BOOL bOldIdle = pSh->GetViewOptions()->IsIdle();
    ((SwViewOption *)pSh->GetViewOptions())->SetIdle(FALSE);

    if( pSh->IsSelFrmMode() )
        pSh->ShowCrsr();

    DropAction eAction;
#ifdef MAC
    const Region aRegion( pSh->GetCrsrRegion() );
#endif

    //Object bereits hier auf den Stack legen, weil es sich im DTor aus dem
    //SwView austragen wurde und deshalb hinter dem else-Scope nicht mehr
    //erreichbar waere.
    SwDataExchangeRef aRef( new SwDataExchange( rView.GetWrtShell() ) );
    SW_MOD()->pDragDrop = aRef;
    aRef->Copy( TRUE );

    DBG_TRACE("SW: Vor ExecuteDrag" );

    // JP 02.10.96: aus ReadOnly-Docs darf nichts gemovt werden!
    // JP 19.01.99: aus ReadOnly-Docs darf eigentlich nur kopiert werden!
    USHORT nDragOptions = DRAG_ALL;
    if( ( rView.GetDocShell() && rView.GetDocShell()->IsReadOnly() ) ||
        rView.GetWrtShell().HasReadonlySel() )
        nDragOptions = DRAG_COPYABLE;

    eAction = aRef->ExecuteDrag( this, Pointer( POINTER_MOVEDATA ),
                                       Pointer( POINTER_COPYDATA ),
                                       nDragOptions
#ifdef MAC
                                        , &aRegion
#endif
                                        );

    DBG_TRACE("SW: Nach ExecuteDrag" );

    //Und noch die letzten Nacharbeiten damit alle Stati stimmen.
    if ( eAction == DROP_MOVE || eAction == DROP_DISCARD )
    {
        if ( !bDroped )
        {
            //Es wurde auserhalb des Writers gedroped. Wir muessen noch
            //loeschen.

            pSh->StartAllAction();
            pSh->StartUndo( UIUNDO_DRAG_AND_MOVE );
            if ( pSh->IsTableMode() )
                pSh->DeleteTblSel();
            else
            {
                if ( !(pSh->IsSelFrmMode() || pSh->IsObjSelected()) )
                    //SmartCut, eines der Blanks mitnehmen.
                    pSh->IntelligentCut( pSh->GetSelectionType(), TRUE );
                pSh->DelRight();
            }
            pSh->EndUndo( UIUNDO_DRAG_AND_MOVE );
            pSh->EndAllAction();
        }
        else
        {
            const int nSelection = pSh->GetSelectionType();
            if( ( SwWrtShell::SEL_FRM | SwWrtShell::SEL_GRF |
                 SwWrtShell::SEL_OLE | SwWrtShell::SEL_DRW ) & nSelection )
            {
                pSh->EnterSelFrmMode();
            }
        }
    }
    if ( pSh->IsSelFrmMode() )
        pSh->HideCrsr();
    else if ( DROP_NONE != eAction )
        pSh->ShowCrsr();
    else
    {
        //Muss wohl sein weil gescrollt wurde und ?...?
        pSh->StartAction();
        pSh->EndAction();
    }
    aTimer.SetTimeoutHdl( LINK(this,SwEditWin, TimerHandler) );
    ((SwViewOption *)pSh->GetViewOptions())->SetIdle(bOldIdle);
    bIsInDrag = FALSE;

    DBG_TRACE("SW: Fertig mit D&D" );
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

BOOL SwEditWin::Drop( const DropEvent& rEvt )
{
    ::lcl_SelectShellForDrop( GetView() );
    DropCleanup();

    //Ein Drop auf eine offene OutlinerView geht uns nichts an (siehe auch QueryDrop)
    SwWrtShell &rSh = rView.GetWrtShell();
    SdrObject *pObj = 0;
    const Point aDocPt( PixelToLogic( rEvt.GetPosPixel() ) );
    rSh.GetObjCntType( aDocPt, pObj );
    if( pObj )
    {
        OutlinerView* pOLV = rSh.GetDrawView()->GetTextEditOutlinerView();
        if ( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );
            aRect.Union( pObj->GetLogicRect() );
            const Point aPos = pOLV->GetWindow()->PixelToLogic(rEvt.GetPosPixel());
            if ( aRect.IsInside(aPos) )
            {
                rSh.StartAllAction();
                BOOL bRet = pOLV->Drop( rEvt );
                rSh.EndAllAction();
                return bRet;
            }
        }
    }

    BOOL bRet = rEvt.GetData() &&
                0 != SwDataExchange::PasteData( *rEvt.GetData(), rSh,
                                                nDropAction, nDropFormat,
                                                nDropDestination, &rEvt );

    if ( bRet )
    {
        if ( SW_MOD()->pDragDrop )
            bDroped = TRUE;         //Bei internem D&D nicht mehr aufraeumen!
    }
    else
        bRet = OFF_APP()->Drop( (DropEvent&)rEvt );
    return bRet;
}


BOOL SwEditWin::QueryDrop( DropEvent& rEvt )
{
    if ( rEvt.IsLeaveWindow() )
    {
        DropCleanup();
        return TRUE;
    }

    if(rView.GetDocShell()->IsReadOnly())
        return FALSE;

    SwWrtShell &rSh = rView.GetWrtShell();

    //Ein bischen scrollen?
    Point aPixPt( rEvt.GetPosPixel() );
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

    //Drop auf Selektion ist nicht statthaft.
    //JP 19.01.99: Drop in geschuetzte Bereiche ist nicht statthaft
    const Point aDocPt( PixelToLogic( aPixPt ) );
    if( rSh.ChgCurrPam( aDocPt ) || rSh.IsOverReadOnlyPos( aDocPt ) )
        return FALSE;

    //Auf was wollen wir denn gerade droppen?
    nDropDestination = 0;

    BOOL bDropCursor = TRUE;    //UserMarker oder DropCursor anzeigen.

    SdrObject *pObj = NULL;
    const ObjCntType eType = rSh.GetObjCntType( aDocPt, pObj );

    //Drop auf OutlinerView (TextEdit im Drawing) soll diese selbst entscheiden!
    if ( pObj )
    {
        OutlinerView* pOLV = rSh.GetDrawView()->GetTextEditOutlinerView();
        if ( pOLV )
        {
            Rectangle aRect( pOLV->GetOutputArea() );
            aRect.Union( pObj->GetLogicRect() );
            const Point aPos = pOLV->GetWindow()->PixelToLogic(aPixPt);
            if ( aRect.IsInside(aPos) )
                return pOLV->QueryDrop( rEvt );
        }
    }

    //Sonst etwas aus der DrawingEngine getroffen?
    if ( OBJCNT_NONE != eType )
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
/*
JP 13.07.98: Bug 52637: es wird ein URL-Feld erkannt also werden nur die
                        Inhalte zugelassen. Das ist aber bestimmt nicht das
                        gewollte.
        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
        if ( rSh.GetContentAtPos( aDocPt, aCntntAtPos, FALSE ) )
            nDropDestination = EXCHG_DEST_DOC_URLFIELD;
        else
*/
        if( rSh.GetView().GetDocShell()->ISA(SwWebDocShell) )
            nDropDestination = EXCHG_DEST_SWDOC_FREE_AREA_WEB;
        else
            nDropDestination = EXCHG_DEST_SWDOC_FREE_AREA;

    }
    else
        bDropCursor = FALSE;

    USHORT nEventAction, nUserOpt = EXCHG_IN_ACTION_DEFAULT;
    if( !rEvt.IsDefaultAction() )
        nUserOpt = rEvt.GetAction();

    nDropAction = SotExchange::GetExchangeAction( *rEvt.GetData(),
                                nDropDestination, rEvt.GetSourceOptions(),
                                nUserOpt, nDropFormat, nEventAction );

    BOOL bRet = TRUE;
    if( EXCHG_INOUT_ACTION_NONE != nDropAction )
    {
        //Bei den default Aktionen wollen wir noch ein bischen mitreden.
        SwModule *pMod = SW_MOD();
        if ( pMod->pDragDrop )
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
            else if( DROP_MOVE == rEvt.GetAction() &&
                     pSrcSh->IsSelObjProtected( FLYPROTECT_POS ) )
            {
                bCleanup = TRUE;
            }
            else if( rEvt.IsDefaultAction() )
            {
                // JP 13.08.98: internes Drag&Drop: bei gleichem Doc ein Move
                //              ansonten ein Copy - Task 54974
                nEventAction = pSrcSh->GetDoc() == rSh.GetDoc()
                                    ? DROP_MOVE
                                    : DROP_COPY;
            }
            if ( bCleanup )
            {
                CleanupDropUserMarker();
                rSh.UnSetVisCrsr();
                return FALSE;
            }
        }
        else
        {
            //D&D von ausserhalb des SW soll per default ein Copy sein.
            if( EXCHG_IN_ACTION_DEFAULT == nEventAction &&
                DROP_MOVE == rEvt.GetAction() )
                nEventAction = DROP_COPY;

            if( (SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE == nDropFormat &&
                 EXCHG_IN_ACTION_LINK != nDropAction) ||
                 SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE == nDropFormat )
            {
                SdrMarkView* pMView = PTR_CAST( SdrMarkView, rSh.GetDrawView() );
                if( pMView && !pMView->IsDesignMode() )
                    return FALSE;
            }

        }

        if ( EXCHG_IN_ACTION_DEFAULT != nEventAction )
            rEvt.SetAction( (DropAction)nEventAction );

        if ( bDropCursor )
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
        return TRUE;
    }

    CleanupDropUserMarker();
    rSh.UnSetVisCrsr();
    return OFF_APP()->QueryDrop( rEvt );
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


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.174  2000/09/18 16:05:23  willem.vandorp
    OpenOffice header added.

    Revision 1.173  1999/05/11 19:50:40  JP
    Task #66127#: Methoden rund ums Lineal verbessert und Schnittstellen veraendert/erweitert


      Rev 1.172   11 May 1999 21:50:40   JP
   Task #66127#: Methoden rund ums Lineal verbessert und Schnittstellen veraendert/erweitert

      Rev 1.171   25 Jan 1999 16:11:42   JP
   Bug #61109#: QueryDrop - DropModifier nicht ueberbuegeln

      Rev 1.170   19 Jan 1999 22:57:28   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.169   11 Dec 1998 11:46:02   JP
   Task #58217#: WebDocShell als D&D Ziel erkennen

      Rev 1.168   27 Nov 1998 14:50:42   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.167   17 Nov 1998 22:17:06   JP
   Task #59398#: ClipboardId Umstellungen

      Rev 1.166   12 Nov 1998 13:31:08   JP
   Task #59398# Umstellung von SotFormatStringId

      Rev 1.165   02 Sep 1998 12:10:46   MA
   #55895# ctrldataexchange

      Rev 1.164   13 Aug 1998 14:20:14   JP
   Bug #54974# QueryDrop - bei DefaultAction per internem Format im eigenen Doc immer move, sonst copy

      Rev 1.163   16 Jul 1998 19:21:22   JP
   Bug #53113# DatenbankControls nur im DesignModus einfuegen

      Rev 1.162   08 Jul 1998 14:17:32   JP
   das richtige define fuer supd benutzen

      Rev 1.161   08 Jul 1998 12:24:36   JP
   neues Drag&Drop ab Version 5.0

      Rev 1.160   04 Jul 1998 16:06:24   JP
   Umstellung auf Tabelle weiergemacht

      Rev 1.159   27 Jun 1998 18:44:00   JP
   Umstellung auf Tabelle weitergemacht

      Rev 1.158   25 Jun 1998 14:12:50   JP
   SvDataObject -> SotObject

      Rev 1.157   16 Jun 1998 17:14:42   OM
   #51085# Draw-Objekte nicht auf die Wiese droppen

      Rev 1.156   16 Jun 1998 16:35:44   OM
   #51085# Draw-Objekte nicht auf die Wiese droppen

      Rev 1.155   09 Jun 1998 15:31:32   OM
   VC-Controls entfernt

      Rev 1.154   27 Apr 1998 21:15:26   JP
   Bug #49842#: Drop - bei Grafiken den FileNamen in eine URL wandeln

      Rev 1.153   31 Mar 1998 17:30:22   MA
   erstmal abgebrochen, neues DD

      Rev 1.152   26 Mar 1998 18:02:54   MA
   ein paar vorb. D&D

      Rev 1.151   20 Feb 1998 16:18:20   MA
   headerfiles gewandert

      Rev 1.150   19 Feb 1998 08:53:20   OK
   NEW: include mieclip.hxx

      Rev 1.149   07 Jan 1998 18:46:26   MIB
   5.0 Fileformat

      Rev 1.148   28 Nov 1997 18:02:26   MA
   includes

      Rev 1.147   25 Nov 1997 10:33:00   MA
   includes

      Rev 1.146   22 Oct 1997 16:04:52   OM
   Farbendrop

      Rev 1.145   17 Oct 1997 13:24:02   JP
   neu: MSE40-HTML-Format erkennen/lesen

      Rev 1.144   15 Oct 1997 14:03:44   OS
   lcl_SelectShellForDrop - SelectShell() nur bei Bedarf rufen #44690#

      Rev 1.143   17 Sep 1997 11:26:04   MA
   #43801# precedence nicht beachtet

      Rev 1.142   17 Sep 1997 09:57:22   OS
   Nummer 3: pSdrView pruefen!

      Rev 1.141   10 Sep 1997 10:50:08   JP
   neu: Undo fuers kopieren von Tabellen & in Tabellen

      Rev 1.140   08 Sep 1997 11:02:36   MA
   includes

      Rev 1.139   05 Sep 1997 13:10:48   OM
   #42027# D&D auf DrawText-Objekte

      Rev 1.138   02 Sep 1997 13:20:48   OS
   includes

      Rev 1.137   01 Sep 1997 13:15:38   OS
   DLL-Umstellung

      Rev 1.136   28 Aug 1997 10:29:56   JP
   Bug #42430#: Drop - bei INetAttr. den SourceCursor auf das Attribut setzen

      Rev 1.135   15 Aug 1997 12:09:54   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.134   11 Aug 1997 20:06:16   HJS
   includes

      Rev 1.133   11 Aug 1997 10:08:40   MH
   chg: header

      Rev 1.132   17 Jul 1997 19:06:32   MA
   #41801# File zur absoluten URL verarbeiten

      Rev 1.131   19 Jun 1997 11:38:52   MA
   #40833# Drop_Link, sdw nicht einfuegen bei sw/web

      Rev 1.130   09 Jun 1997 20:08:48   JP
   SwFilterNms/SwFilterNm entfernt

      Rev 1.129   29 May 1997 14:31:14   MA
   fix: Grafik nur verknuepft fuer WebDocShell

      Rev 1.128   16 May 1997 15:07:42   MH
   add: defines wg. CLOOKS

      Rev 1.127   25 Apr 1997 13:00:46   MA
   #39306# ggf. den UserMarker loeschen

      Rev 1.126   14 Apr 1997 17:25:00   MH
   add: header

      Rev 1.125   10 Apr 1997 16:05:18   TRI
   includes

      Rev 1.124   08 Apr 1997 10:31:46   OM
   Fehlende Includes

      Rev 1.123   08 Apr 1997 09:55:34   NF
   includes...

      Rev 1.122   21 Mar 1997 13:53:18   MA
   #36900# D&D offset beseitigt

      Rev 1.121   16 Mar 1997 13:44:16   MA
   fix: Kein D&D wenn der Frame schon klinisch tot ist

      Rev 1.120   07 Mar 1997 09:47:24   MA
   Neue D&D Moeglichkeiten, Handling verbessert

      Rev 1.119   23 Feb 1997 22:03:10   MA
   fix: Drop von Grafik auf Grafik reanimiert

      Rev 1.118   07 Feb 1997 12:05:44   OS
   Navigator benutzt eigenes Drag-Format

      Rev 1.117   05 Feb 1997 19:13:18   JP
   ExcuteDrag: Modus der WrtShell setzen, wenn Flys selektiert sind

      Rev 1.116   19 Dec 1996 16:12:18   MA
   fix: Crsr wieder umschalten

      Rev 1.115   28 Nov 1996 14:20:06   MA
   fix: Drop per Link auch bei einem File

      Rev 1.114   20 Nov 1996 16:58:42   MA
   #33444# QueryDrop, Pos fuer Objekte anpassen

      Rev 1.113   18 Oct 1996 16:57:56   MA
   Messehack: eine hoffentlich kurzlebige Freundschaft

      Rev 1.112   15 Oct 1996 12:14:20   MA
   new: Drawing per Clipboard

      Rev 1.111   02 Oct 1996 13:50:52   JP
   StartExecuteDrag: aus ReadOnly-Docs darf nur kopiert werden!

      Rev 1.110   16 Sep 1996 14:00:58   JP
   Drop: INetBookmark am URL-Button setzen

-------------------------------------------------------------------------*/

