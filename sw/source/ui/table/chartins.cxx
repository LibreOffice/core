/*************************************************************************
 *
 *  $RCSfile: chartins.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:47:35 $
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


#include <sot/clsids.hxx>

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
    aHLine      (this, SW_RES(FL_HLINE   )),
    aPrev       (this, SW_RES(BT_PREV    )),
    aNext       (this, SW_RES(BT_NEXT    )),
    aFirstRow   (this, SW_RES(CB_FIRST_ROW)),
    aFirstCol   (this, SW_RES(CB_FIRST_COL)),
    aFL1     (this, SW_RES(FL_1       )),
    pChartDlg(0),
    pWrtShell(pSh),
    pInItemSet(0),
    pOutItemSet(0),
    pChartData(0),
    bUpdateChartData(TRUE),
    bChartInserted(FALSE),
    bChildOpen(FALSE)
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


void SwInsertChartDlg::Activate()
{
    SfxModelessDialog::Activate();

    if ( bChildOpen && pChartDlg )
    {
        //  #107337# The ChildWindow's "hidden" state is reset if the view is activated,
        //  so it is hidden again on activating if the child dialog is open.

        SfxViewFrame* pVFrame = pWrtShell->GetView().GetViewFrame();
        pVFrame->ShowChildWindow(SID_INSERT_DIAGRAM, FALSE);

        pChartDlg->GrabFocus();     // child dialog should have focus
    }
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
        pChartDlg = SchDLL::CreateAutoPilotDlg( GetParent(),    pChartData,
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
    bChildOpen = TRUE;
    USHORT nResult = pChartDlg->Execute();
    bChildOpen = FALSE;
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
        pShell->Insert( 0, &SvGlobalName( SO3_SCH_CLASSID ), FALSE );
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




