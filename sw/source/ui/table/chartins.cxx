/*************************************************************************
 *
 *  $RCSfile: chartins.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:47 $
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

#define _CHARTINS_CXX

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SCHDLL0_HXX
#include <sch/schdll0.hxx>
#endif
#ifndef _SCH_DLL_HXX //autogen
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _CHARTINS_HXX
#include <chartins.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _CHARTINS_HRC
#include <chartins.hrc>
#endif




SFX_IMPL_MODELESSDIALOG( SwInsertChartChild, SID_INSERT_DIAGRAM )


/*------------------------------------------------------------------------
    Beschreibung: AutoPilot fuer StarChart
------------------------------------------------------------------------*/


SwInsertChartDlg::SwInsertChartDlg( SfxBindings* pBindings,
                                    SfxChildWindow* pChild,
                                    Window *pParent,
                                    SwWrtShell* pSh ) :
    SfxModelessDialog( pBindings, pChild, pParent, SW_RES(DLG_INSERT_CHART) ),
    aTextFt     (this, SW_RES(FT_TEXT    )),
    aRangeEd    (this, SW_RES(ED_RANGE   )),
    aRangeFt    (this, SW_RES(FT_RANGE    )),
    aFinish     (this, SW_RES(BT_FINISH   )),
    aHelp       (this, SW_RES(BT_HELP     )),
    aCancel     (this, SW_RES(BT_CANCEL   )),
    aHLine      (this, SW_RES(FT_HORZLINE )),
    aPrev       (this, SW_RES(BT_PREV    )),
    aNext       (this, SW_RES(BT_NEXT    )),
    aFirstRow   (this, SW_RES(CB_FIRST_ROW)),
    aFirstCol   (this, SW_RES(CB_FIRST_COL)),
    aGroup1     (this, SW_RES(GB_1       )),
    pChartDlg(0),
    pWrtShell(pSh),
    pInItemSet(0),
    pOutItemSet(0),
    pChartData(0),
    bUpdateChartData(TRUE),
    bChartInserted(FALSE)
{
    FreeResource();
    pSh->Push();

    aRangeEd.   SetModifyHdl(LINK(this, SwInsertChartDlg, ModifyHdl));
    aNext.      SetClickHdl(LINK(this,  SwInsertChartDlg, NextHdl));
    aFinish.    SetClickHdl(LINK(this,  SwInsertChartDlg, FinishHdl));
    aCancel.    SetClickHdl(LINK(this,  SwInsertChartDlg, CloseHdl));
    aFirstRow.  SetClickHdl(LINK(this,  SwInsertChartDlg, ClickHdl));
    aFirstCol.  SetClickHdl(LINK(this,  SwInsertChartDlg, ClickHdl));

    if( pWrtShell->IsCrsrInTbl() )
    {
        SwFrmFmt* pTblFmt = pWrtShell->GetTableFmt();
        aAktTableName = pTblFmt->GetName();
        if( !pWrtShell->IsTableMode() )
        {
            pWrtShell->GetView().GetViewFrame()->GetDispatcher()->
                    Execute(FN_TABLE_SELECT_ALL, SFX_CALLMODE_SYNCHRON);
        }

        pWrtShell->UpdateChartData( aAktTableName, pChartData );

        String sText( String::CreateFromAscii("<.>") );
        sText.Insert( pWrtShell->GetBoxNms(), 2);
        sText.Insert( aAktTableName, 1 );
        if(sText.GetTokenCount(':') == 2)
            aRangeEd.SetText(sText);

        ModifyHdl( &aRangeEd );
    }
    pInItemSet =  new SfxItemSet( pWrtShell->GetAttrPool(),
                                            CHATTR_START, CHATTR_END, 0 );
    pOutItemSet =  new SfxItemSet( pWrtShell->GetAttrPool(),
                                            CHATTR_START, CHATTR_END, 0 );
    SFX_APP()->LockDispatcher(TRUE);
    pWrtShell->SelTblCells( LINK( this, SwInsertChartDlg,
                                            SelTblCellsNotify) );
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


__EXPORT SwInsertChartDlg::~SwInsertChartDlg()
{
    SfxApplication* pSfxApp = SFX_APP();
    if(pSfxApp->IsDispatcherLocked())
        pSfxApp->LockDispatcher(FALSE);
    pWrtShell->EndSelTblCells();
    pWrtShell->Pop(bChartInserted);
    delete pInItemSet;
    delete pOutItemSet;
    delete pChartDlg;
    delete pChartData;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


BOOL SwInsertChartDlg::Close()
{
    SFX_APP()->LockDispatcher(FALSE);
    return SfxModelessDialog::Close();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

//OS: WNTMSCI4 optimiert sonst SetPosPixel und Hide weg!
#pragma optimize("",off)


IMPL_LINK( SwInsertChartDlg, NextHdl, Button *, pBtn )
{

    if ( bUpdateChartData )
        UpdateData();

    if(!pChartDlg)
    {
        pChartDlg = SchDLL::CreateAutoPilotDlg( this,   pChartData,
                                     *pInItemSet, *pOutItemSet, TRUE);
    }
    else if(bUpdateChartData)
    {
        SchDLL::ChangeChartData(pChartDlg,
                            pChartData);
    }
    bUpdateChartData = FALSE;
    pChartDlg->SetPosPixel(GetPosPixel());
    SfxViewFrame* pVFrame = pWrtShell->GetView().GetViewFrame();
    pVFrame->ShowChildWindow(SID_INSERT_DIAGRAM, FALSE);
    USHORT nResult = pChartDlg->Execute();
    switch( nResult )
    {
        case RET_OK:
            FinishHdl( &aFinish );
        break;
        case RET_CANCEL:
            SetPosPixel(pChartDlg->GetPosPixel());
            Close();
        break;
        default:
            SetPosPixel(pChartDlg->GetPosPixel());
            pVFrame->ShowChildWindow(SID_INSERT_DIAGRAM, TRUE);
        break;
    }
    return 0;
}

#pragma optimize("",on)

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK( SwInsertChartDlg, FinishHdl, Button *, EMPTYARG )
{
    pWrtShell->GotoTable(aAktTableName);
    pWrtShell->GetView().GetViewFrame()->ToTop();
    SwWrtShell* pShell = pWrtShell; // Member auf den Stack wg. Close()
    SfxItemSet* pOutSet = pOutItemSet;
    pOutItemSet = 0;
    bChartInserted = TRUE;
    BOOL bCrsrInTbl = pShell->IsCrsrInTbl() != 0;
    BOOL bTblCplx = pShell->IsTblComplexForChart();
    if( bCrsrInTbl && !bTblCplx && bUpdateChartData )
        UpdateData();
    SchMemChart *pChData = pChartData; // Member auf den Stack wg. Close() -> aber erst nach UpdateData()
    pChartData = 0;

    Close();

    if( bCrsrInTbl && !bTblCplx )
    {
        SwTableFUNC( pShell, FALSE ).InsertChart( *pChData, pOutSet );
    }
    else
    {
        pShell->Insert( 0, SCH_MOD()->pSchChartDocShellFactory, FALSE );
    }
    delete pOutSet;
    delete pChData;
    return 0;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


IMPL_LINK( SwInsertChartDlg, ClickHdl, CheckBox *, EMPTYARG )
{
    bUpdateChartData = TRUE;
    return 0;
}

/*------------------------------------------------------------------------
    Beschreibung: Handler fuer die Tabellenselektion
------------------------------------------------------------------------*/

IMPL_LINK( SwInsertChartDlg, SelTblCellsNotify, SwWrtShell *, pCaller )
{
    SwFrmFmt* pTblFmt = pCaller->GetTableFmt();
    SwTable* pTbl = 0;
    if(pTblFmt)
    {
        SwClientIter aIter(*pTblFmt);
        pTbl = (SwTable*)aIter.First(TYPE(SwTable));
        DBG_ASSERT(pTbl, "keine Tabelle gefunden")
    }
    if( pTbl && !pCaller->IsTblComplexForChart() )
    {
        String sCommand = String::CreateFromAscii("<.>");
        sCommand.Insert(pCaller->GetBoxNms(),2);
        aAktTableName = pTblFmt->GetName();
        sCommand.Insert(pTblFmt->GetName(), 1);
        aRangeEd.SetText(sCommand);
    }
    else
    {
        aRangeEd.SetText(aEmptyStr);
    }
    ModifyHdl( &aRangeEd );
    return 0;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwInsertChartDlg, CloseHdl, Button*, EMPTYARG )
{
    Close();
    return 0;
}

/*------------------------------------------------------------------------
    Beschreibung: Handler fuer Edit
------------------------------------------------------------------------*/

IMPL_LINK( SwInsertChartDlg, ModifyHdl, Edit*, pEdit )
{
//  hier muss getestet werden, ob mit dem aktuellen Eintrag eine
//  gueltige Selektion einer Tabelle aufgebaut werden kann

    BOOL bCorrect = FALSE;
    BOOL bFinish = FALSE;
    BOOL bChkFirstRow = TRUE, bChkFirstCol = TRUE;

    String sContent = pEdit->GetText();
    if( !sContent.Len() )
    {
        bCorrect = pWrtShell->IsCrsrInTbl() != 0;
        aAktTableName = sContent;
        bFinish = TRUE;
    }
    else if( sContent.GetChar( 0 ) == '<'   &&
        sContent.GetTokenCount(':') == 2    &&
        sContent.GetChar( sContent.Len() - 1 ) == '>')
    {
        USHORT nFndPos = sContent.Search( '.' );
        String aTable( sContent.Copy( 1, nFndPos - 1 ));
        SwFrmFmt* pFmt = pWrtShell->GetTableFmt();
        if( ( pFmt && pFmt->GetName() == aTable ) ||
            pWrtShell->GotoTable( aTable ) )
        {
            aAktTableName = aTable;

            sContent.Erase( 0, nFndPos + 1 );
            sContent.Erase( sContent.Len() - 1 );

            SwTable* pTable = SwTable::FindTable( pWrtShell->GetTableFmt() );
            SwChartLines aLines;
            if( !pTable->IsTblComplexForChart( sContent, &aLines ))
            {
                bChkFirstCol = 1 < aLines[ 0 ]->Count();
                bChkFirstRow = 1 < aLines.Count();

                bFinish = bCorrect = TRUE;
            }
        }
    }

    aNext.Enable( bCorrect );
    aFinish.Enable( bFinish );

    if( bChkFirstRow != aFirstRow.IsEnabled() )
    {
        if( bChkFirstRow )
            aFirstRow.Check( aFirstRow.GetSavedValue() );
        else
        {
            aFirstRow.SaveValue();
            aFirstRow.Check( FALSE );
        }

        aFirstRow.Enable( bChkFirstRow );
    }

    if( bChkFirstCol != aFirstCol.IsEnabled() )
    {
        if( bChkFirstCol )
            aFirstCol.Check( aFirstCol.GetSavedValue() );
        else
        {
            aFirstCol.SaveValue();
            aFirstCol.Check( FALSE );
        }

        aFirstCol.Enable( bChkFirstCol );
    }

    bUpdateChartData = TRUE;
    return 0;
}


void SwInsertChartDlg::UpdateData()
{
    if( !pChartData )
        pWrtShell->UpdateChartData( aAktTableName, pChartData );

    if( pChartData )
    {
        String aData = aFirstRow.IsChecked() ? '1' : '0';
        aData += aFirstCol.IsChecked() ? '1': '0';
        pChartData->SomeData2() = aData;
        aData = aRangeEd.GetText();
        aData.Erase(1, aAktTableName.Len() +1 );
        pChartData->SomeData1() = aData;
        pWrtShell->UpdateChartData( aAktTableName, pChartData );
    }
}


/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/


SwInsertChartChild::SwInsertChartChild(Window* pParent,
                        USHORT nId,
                        SfxBindings* pBindings,
                        SfxChildWinInfo* pInfo ) :
                        SfxChildWindow( pParent, nId )
{

    SwView *pView = ::GetActiveView();
    SwWrtShell &rSh = pView->GetWrtShell();

    pWindow = new SwInsertChartDlg( pBindings, this, pParent, &rSh );
    pWindow->SetPosPixel(pInfo->aPos);
    pWindow->Show();

}


void __EXPORT AutoEdit::KeyInput( const KeyEvent& rEvt )
{
    USHORT nCode = rEvt.GetKeyCode().GetCode();
    if( nCode != KEY_SPACE )
        Edit::KeyInput( rEvt );
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.50  2000/09/18 16:06:07  willem.vandorp
    OpenOffice header added.

    Revision 1.49  2000/09/11 06:52:45  os
    Get/Set/Has/ToggleChildWindow SfxApplication -> SfxViewFrame

    Revision 1.48  2000/09/07 15:59:31  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.47  2000/04/26 14:59:03  os
    GetName() returns const String&

    Revision 1.46  2000/04/19 11:22:11  os
    UNICODE

    Revision 1.45  2000/03/09 21:54:45  jp
    Changes: SchMemChart in new headerfile

    Revision 1.44  2000/02/11 14:58:21  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.43  1999/02/09 13:31:16  JP
    Task #61632#: Charts auch in complexen Tabellen zulassen


      Rev 1.42   09 Feb 1999 14:31:16   JP
   Task #61632#: Charts auch in complexen Tabellen zulassen

      Rev 1.41   29 Oct 1998 11:35:34   JP
   Bug #58509#: im ModifyHdl die FirstRow/-Col Buttons ggfs. disablen

      Rev 1.40   28 Oct 1998 19:34:34   JP
   Bug #58509#: Button FirstRow/-Col ggfs. disablen

      Rev 1.39   16 Mar 1998 13:55:08   MA
   #48288# ohne Data leben

      Rev 1.38   29 Nov 1997 14:28:12   MA
   includes

      Rev 1.37   24 Nov 1997 15:52:20   MA
   includes

      Rev 1.36   09 Sep 1997 11:39:16   OS
   define fuer .hxx

      Rev 1.35   01 Sep 1997 13:17:26   OS
   DLL-Umstellung

      Rev 1.34   08 Aug 1997 17:37:36   OM
   Headerfile-Umstellung

      Rev 1.33   09 Jul 1997 17:39:18   HJS
   includes

      Rev 1.32   08 Apr 1997 10:32:30   MA
   includes

      Rev 1.31   07 Apr 1997 16:54:14   OS
   FinishHdl: pChartData erst nach UpdateData umkopieren

      Rev 1.30   24 Nov 1996 14:59:48   WP
   SvGlobalNames beseitigt

      Rev 1.29   11 Nov 1996 11:20:06   MA
   ResMgr

      Rev 1.28   05 Nov 1996 13:48:20   OS
   im FinishHdl wird der ViewFrame 'getoppt'

      Rev 1.27   24 Oct 1996 14:17:12   JP
   AutoEdit:KeyInput aus AutoCorr.cxx hierher verschoben

      Rev 1.26   24 Oct 1996 13:36:42   JP
   String Umstellung: [] -> GetChar()

      Rev 1.25   02 Oct 1996 19:06:08   MA
   Umstellung Enable/Disable

      Rev 1.24   28 Aug 1996 14:33:14   OS
   includes

      Rev 1.23   23 Aug 1996 14:37:14   OS
   FinishHdl: nach dem Close keine Member benutzen

      Rev 1.22   22 Aug 1996 12:40:44   OS
   Close im FinishHdl vor dem Einfuegen des Charts rufen

      Rev 1.21   20 Aug 1996 11:34:00   OS
   Dialog nicht mit Show/Hide, sondern mit ShowChildWindow anzeigen/verstecken

      Rev 1.20   29 Jul 1996 13:07:04   OS
   doch wieder mit Close()

      Rev 1.19   25 Jul 1996 15:36:12   OS
   kein Close()

      Rev 1.18   26 Jun 1996 15:25:20   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.17   11 Jun 1996 13:42:32   OS
   alten Cursor nur zurueckholen, wenn Chart nicht eingefuegt wurde

      Rev 1.16   10 Jun 1996 20:40:10   MA
   fix: IsTblComplexForChart

      Rev 1.15   06 Jun 1996 15:53:18   OS
   keine Tabellenselektion in komplexen Tabellen

      Rev 1.14   30 Apr 1996 16:29:44   MA
   fix: ggf. ein Update beim Fertigstellen

------------------------------------------------------------------------*/




