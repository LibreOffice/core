/*************************************************************************
 *
 *  $RCSfile: tabvwshf.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 11:38:39 $
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

#include "scitems.hxx"
#include <sfx2/request.hxx>
#include <basic/sbstar.hxx>
#include <svtools/languageoptions.hxx>
#include <svtools/stritem.hxx>
#include <svtools/whiter.hxx>
#include <vcl/msgbox.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "docsh.hxx"
#include "document.hxx"
#include "shtabdlg.hxx"
#include "scresid.hxx"
//CHINA001 #include "instbdlg.hxx"
#include "globstr.hrc"
//CHINA001 #include "strindlg.hxx"
//CHINA001 #include "mvtabdlg.hxx"
#include "docfunc.hxx"


#include "scabstdlg.hxx" //CHINA001

#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)

//------------------------------------------------------------------

void ScTabViewShell::ExecuteTable( SfxRequest& rReq )
{
    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();

    SCTAB       nCurrentTab = pViewData->GetTabNo();
    SCTAB       nTabCount   = pDoc->GetTableCount();
    USHORT      nSlot       = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    HideListBox();                  // Autofilter-DropDown-Listbox

    switch ( nSlot )
    {
        case FID_TABLE_VISIBLE:
            {
                SCTAB nTabNr = pViewData->GetTabNo();
                String aName;
                pDoc->GetName( nCurrentTab, aName );

                BOOL bVisible=TRUE;
                if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;
                    if( IS_AVAILABLE( FID_TABLE_VISIBLE, &pItem ) )
                        bVisible = ((const SfxBoolItem*)pItem)->GetValue();
                }

                if( ! bVisible )            // ausblenden
                {
                    ScMarkData& rMark = pViewData->GetMarkData();
                    SCTAB nTabSelCount = rMark.GetSelectCount();
                    USHORT nVis = 0;
                    for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
                        if (pDoc->IsVisible(i))
                            ++nVis;
                    if ( nVis<2 || !pDoc->IsDocEditable() || nTabSelCount > 1 )
                        break;

                    SCTAB nHideTab;
                    if (pDoc->GetTable( aName, nHideTab ))
                        HideTable( nHideTab );
                }
                else                        // einblenden
                {
                    ShowTable( aName );
                }
            }
            break;

        case FID_TABLE_HIDE:
            {
                ScMarkData& rMark = pViewData->GetMarkData();
                SCTAB nTabSelCount = rMark.GetSelectCount();
                USHORT nVis = 0;
                for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
                    if (pDoc->IsVisible(i))
                        ++nVis;
                if ( nVis<2 || !pDoc->IsDocEditable() || nTabSelCount > 1 )
                    break;


                String aName;
                if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;
                    if( IS_AVAILABLE( FID_TABLE_HIDE, &pItem ) )
                        aName = ((const SfxStringItem*)pItem)->GetValue();
                }

                if (!aName.Len())
                {
                    pDoc->GetName( nCurrentTab, aName );        // aktuelle Tabelle
                    rReq.AppendItem( SfxStringItem( FID_TABLE_HIDE, aName ) );
                }

                SCTAB nHideTab;
                if (pDoc->GetTable( aName, nHideTab ))
                    HideTable( nHideTab );

                if( ! rReq.IsAPI() )
                    rReq.Done();
            }
            break;

        case FID_TABLE_SHOW:
            {
                String aName;
                if ( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    if( IS_AVAILABLE( FID_TABLE_SHOW, &pItem ) )
                    {
                        aName = ((const SfxStringItem*)pItem)->GetValue();

                        ShowTable( aName );

                        if( ! rReq.IsAPI() )
                            rReq.Done();
                    }
                }
                else
                {
                    //CHINA001 ScShowTabDlg* pDlg = new ScShowTabDlg( GetDialogParent() );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                    AbstractScShowTabDlg* pDlg = pFact->CreateScShowTabDlg( GetDialogParent(), ResId(RID_SCDLG_SHOW_TAB));
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    String aTabName;
                    BOOL bFirst = TRUE;
                    for ( SCTAB i=0; i != nTabCount; i++ )
                    {
                        if (!pDoc->IsVisible(i))
                        {
                            pDoc->GetName( i, aTabName );
                            pDlg->Insert( aTabName, bFirst );
                            bFirst = FALSE;
                        }
                    }

                    if ( pDlg->Execute() == RET_OK )
                    {
                        USHORT nCount = pDlg->GetSelectEntryCount();
                        for (USHORT nPos=0; nPos<nCount; nPos++)
                        {
                            aName = pDlg->GetSelectEntry(nPos);
                            ShowTable( aName );
                        }
                        rReq.AppendItem( SfxStringItem( FID_TABLE_SHOW, aName ) );
                        rReq.Done();
                    }
                    delete pDlg;
                }
            }
            break;

        case FID_INS_TABLE:
        case FID_INS_TABLE_EXT:
            {
                ScMarkData& rMark    = pViewData->GetMarkData();
                SCTAB   nTabSelCount = rMark.GetSelectCount();
                SCTAB   nTabNr       = nCurrentTab;

                if ( !pDoc->IsDocEditable() )
                    break;                          // gesperrt

                if ( pReqArgs != NULL )             // von Basic
                {
                    BOOL bOk = FALSE;
                    const SfxPoolItem*  pTabItem;
                    const SfxPoolItem*  pNameItem;
                    String              aName;

                    if ( IS_AVAILABLE( FN_PARAM_1, &pTabItem ) &&
                         IS_AVAILABLE( nSlot, &pNameItem ) )
                    {
                        //  Tabellennr. von Basic: 1-basiert

                        aName = ((const SfxStringItem*)pNameItem)->GetValue();
                        nTabNr = ((const SfxUInt16Item*)pTabItem)->GetValue() - 1;
                        if ( nTabNr < nTabCount )
                            bOk = InsertTable( aName, nTabNr );
                    }

                    if (bOk)
                        rReq.Done( *pReqArgs );
                    //! sonst Fehler setzen
                }
                else                                // Dialog
                {
//CHINA001                  ScInsertTableDlg* pDlg = new ScInsertTableDlg(
//CHINA001                  GetDialogParent(),
//CHINA001                  *pViewData,nTabSelCount);
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                    AbstractScInsertTableDlg* pDlg = pFact->CreateScInsertTableDlg( GetDialogParent(), *pViewData,
                                                                                    nTabSelCount, nSlot == FID_INS_TABLE_EXT,
                                                                                    ResId(RID_SCDLG_INSERT_TABLE));
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
                    if ( RET_OK == pDlg->Execute() )
                    {
                        if (pDlg->GetTablesFromFile())
                        {
                            SCTAB nTabs[MAXTABCOUNT];
                            SCTAB nCount = 0;
                            USHORT n = 0;
                            const String* pStr = pDlg->GetFirstTable( &n );
                            while ( pStr )
                            {
                                nTabs[nCount++] = static_cast<SCTAB>(n);
                                pStr = pDlg->GetNextTable( &n );
                            }
                            BOOL bLink = pDlg->GetTablesAsLink();
                            if (nCount != 0)
                            {
                                if(pDlg->IsTableBefore())
                                {
                                    ImportTables( pDlg->GetDocShellTables(), nCount, nTabs,
                                                bLink,nTabNr );
                                }
                                else
                                {
                                    SCTAB   nTabAfter    = nTabNr+1;

                                    for(SCTAB j=nCurrentTab+1;j<nTabCount;j++)
                                    {
                                        if(!pDoc->IsScenario(j))
                                        {
                                            nTabAfter=j;
                                            break;
                                        }
                                    }

                                    ImportTables( pDlg->GetDocShellTables(), nCount, nTabs,
                                                bLink,nTabAfter );
                                }
                            }
                        }
                        else
                        {
                            SCTAB nCount=pDlg->GetTableCount();
                            if(pDlg->IsTableBefore())
                            {
                                if(nCount==1 && pDlg->GetFirstTable()->Len()>0)
                                {
                                    rReq.AppendItem( SfxStringItem( FID_INS_TABLE, *pDlg->GetFirstTable() ) );
                                    rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nTabNr) + 1 ) );        // 1-based
                                    rReq.Done();

                                    InsertTable( *pDlg->GetFirstTable(), nTabNr );
                                }
                                else
                                    InsertTables( NULL, nTabNr,nCount );
                            }
                            else
                            {
                                SCTAB   nTabAfter    = nTabNr+1;
                                SCTAB nSelHigh=0;

                                for(SCTAB i=0;i<nTabCount;i++)
                                {
                                    if(rMark.GetTableSelect(i))
                                    {
                                        nSelHigh=i;
                                    }
                                }

                                for(SCTAB j=nSelHigh+1;j<nTabCount;j++)
                                {
                                    if(!pDoc->IsScenario(j))
                                    {
                                        nTabAfter=j;
                                        break;
                                    }
                                    else // #101672#; increase nTabAfter, because it is possible that the scenario tables are the last
                                        nTabAfter = j + 1;
                                }

                                if(nCount==1 && pDlg->GetFirstTable()->Len()>0)
                                {
                                    rReq.AppendItem( SfxStringItem( FID_INS_TABLE, *pDlg->GetFirstTable() ) );
                                    rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nTabAfter) + 1 ) );     // 1-based
                                    rReq.Done();

                                    InsertTable( *pDlg->GetFirstTable(), nTabAfter);
                                }
                                else
                                {
                                    InsertTables( NULL, nTabAfter,nCount);
                                }
                            }
                        }
                    }

                    delete pDlg;
                }
            }
            break;

        case FID_TAB_APPEND:
        case FID_TAB_RENAME:
        case FID_TAB_MENU_RENAME:
            {
                //  FID_TAB_MENU_RENAME - "umbenennen" im Menu
                //  FID_TAB_RENAME      - "Name"-Property fuer Basic
                //  Execute ist gleich, aber im GetState wird MENU_RENAME evtl. disabled

                if ( nSlot == FID_TAB_MENU_RENAME )
                    nSlot = FID_TAB_RENAME;             // Execute ist gleich

                SCTAB nTabNr = pViewData->GetTabNo();
                ScMarkData& rMark = pViewData->GetMarkData();
                SCTAB nTabSelCount = rMark.GetSelectCount();

                if ( !pDoc->IsDocEditable() )
                    break; // alles gesperrt

                if ( nSlot != FID_TAB_APPEND &&
                        ( pDoc->IsTabProtected( nTabNr ) || nTabSelCount > 1 ) )
                    break; // kein Rename

#if 0
                //  ScSbxObject wird nicht mehr benutzt, stattdessen aus dem
                //  ScSbxTable::Notify die richtige Tabelle an der Basic-View eingestellt
                if( rReq.IsAPI() )
                {
                    SbxObject* pObj = GetScSbxObject();
                    ScSbxTable* pSbxTab = PTR_CAST( ScSbxTable, pObj );
                    DBG_ASSERT( pSbxTab, "pSbxTab???" );

                    if( pSbxTab )
                        nTabNr = pSbxTab->GetTableNr();
                }
#endif

                if( pReqArgs != NULL )
                {
                    BOOL        bDone   = FALSE;
                    const SfxPoolItem* pItem;
                    String      aName;

                    if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
                        nTabNr = ((const SfxUInt16Item*)pItem)->GetValue();

                    if( IS_AVAILABLE( nSlot, &pItem ) )
                        aName = ((const SfxStringItem*)pItem)->GetValue();

                    switch ( nSlot )
                    {
                        case FID_TAB_APPEND:
                            bDone = AppendTable( aName );
                            break;
                        case FID_TAB_RENAME:
                            bDone = RenameTable( aName, nTabNr );
                            break;
                    }

                    if( bDone )
                    {
                        rReq.Done( *pReqArgs );
                    }
                }
                else
                {
                    USHORT      nRet    = RET_OK;
                    BOOL        bDone   = FALSE;
                    String      aErrMsg ( ScGlobal::GetRscString( STR_INVALIDTABNAME ) );
                    String      aName;
                    String      aDlgTitle;

                    switch ( nSlot )
                    {
                        case FID_TAB_APPEND:
                            aDlgTitle = String(ScResId(SCSTR_APDTABLE));
                            pDoc->CreateValidTabName( aName );
                            break;

                        case FID_TAB_RENAME:
                            aDlgTitle = String(ScResId(SCSTR_RENAMETAB));
                            pDoc->GetName( pViewData->GetTabNo(), aName );
                            break;
                    }

//CHINA001                  ScStringInputDlg* pDlg =
//CHINA001                  new ScStringInputDlg( GetDialogParent(),
//CHINA001                  aDlgTitle,
//CHINA001                  String(ScResId(SCSTR_NAME)),
//CHINA001                  aName,
//CHINA001                  nSlot );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                    AbstractScStringInputDlg* pDlg = pFact->CreateScStringInputDlg( GetDialogParent(),
                                                                                    aDlgTitle,
                                                                                    String(ScResId(SCSTR_NAME)),
                                                                                    aName,
                                                                                    nSlot,ResId(RID_SCDLG_STRINPUT));
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    while ( !bDone && nRet == RET_OK )
                    {
                        nRet = pDlg->Execute();

                        if ( nRet == RET_OK )
                        {
                            pDlg->GetInputString( aName );


                            switch ( nSlot )
                            {
                                case FID_TAB_APPEND:
                                    bDone = AppendTable( aName );
                                    break;
                                case FID_TAB_RENAME:
                                    bDone = RenameTable( aName, nTabNr );
                                    break;
                            }

                            if ( bDone )
                            {
                                rReq.AppendItem( SfxStringItem( nSlot, aName ) );
                                rReq.Done();
                            }
                            else
                            {
                                if( rReq.IsAPI() )
                                {
                                    StarBASIC::Error( SbERR_SETPROP_FAILED ); // XXX Fehlerbehandlung???
                                }
                                else
                                {
                                    nRet = ErrorBox( GetDialogParent(),
                                                     WinBits( WB_OK | WB_DEF_OK ),
                                                     aErrMsg
                                                   ).Execute();
                                }
                            }
                        }
                    }
                    delete pDlg;
                }
            }
            break;

        case FID_TAB_MOVE:
            {
                if ( pDoc->GetChangeTrack() != NULL )
                    break;      // bei aktiviertem ChangeTracking kein TabMove

                BOOL   bDoIt = FALSE;
                USHORT nDoc = 0;
                SCTAB nTab = pViewData->GetTabNo();
                BOOL   bCpy = FALSE;
                String aDocName;

                if( pReqArgs != NULL )
                {
                    SCTAB nTableCount = pDoc->GetTableCount();
                    const SfxPoolItem* pItem;

                    if( IS_AVAILABLE( FID_TAB_MOVE, &pItem ) )
                        aDocName = ((const SfxStringItem*)pItem)->GetValue();
                    if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
                    {
                        //  Tabelle ist 1-basiert
                        nTab = ((const SfxUInt16Item*)pItem)->GetValue() - 1;
                        if ( nTab >= nTableCount )
                            nTab = SC_TAB_APPEND;
                    }
                    if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
                        bCpy = ((const SfxBoolItem*)pItem)->GetValue();

                    if( aDocName.Len() )
                    {
                        SfxObjectShell* pSh     = SfxObjectShell::GetFirst();
                        ScDocShell*     pScSh   = NULL;
                        USHORT          i=0;

                        while ( pSh )
                        {
                            pScSh = PTR_CAST( ScDocShell, pSh );

                            if( pScSh )
                            {
                                pScSh->GetTitle();

                                if( pScSh->GetTitle() == aDocName )
                                {
                                    nDoc = i;
                                    ScDocument* pDestDoc = pScSh->GetDocument();
                                    nTableCount = pDestDoc->GetTableCount();
                                    bDoIt = pDestDoc->IsDocEditable();
                                    break;
                                }

                                i++;        // nur die ScDocShell's zaehlen
                            }
                            pSh = SfxObjectShell::GetNext( *pSh );
                        }
                    }
                    else // Kein Dokumentname -> neues Dokument
                    {
                        nDoc = SC_DOC_NEW;
                        bDoIt = TRUE;
                    }

                    if ( bDoIt && nTab >= nTableCount )     // ggf. anhaengen
                        nTab = SC_TAB_APPEND;
                }
                else
                {
                    //CHINA001 ScMoveTableDlg* pDlg = new ScMoveTableDlg( GetDialogParent() );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                    AbstractScMoveTableDlg* pDlg = pFact->CreateScMoveTableDlg( GetDialogParent(), ResId(RID_SCDLG_MOVETAB) );
                    DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    SCTAB nTableCount = pDoc->GetTableCount();
                    ScMarkData& rMark       = GetViewData()->GetMarkData();
                    SCTAB       nTabSelCount = rMark.GetSelectCount();


                    if(nTableCount==nTabSelCount)
                    {
                        pDlg->SetCopyTable();
                        pDlg->EnableCopyTable(FALSE);
                    }
                    if ( pDlg->Execute() == RET_OK )
                    {
                        nDoc = pDlg->GetSelectedDocument();
                        nTab = pDlg->GetSelectedTable();
                        bCpy = pDlg->GetCopyTable();
                        bDoIt = TRUE;

                        String aDocName;
                        if ( nDoc != SC_DOC_NEW )
                        {
                            ScDocShell* pSh = ScDocShell::GetShellByNum( nDoc );
                            if (pSh)
                            {
                                aDocName = pSh->GetTitle();
                                if ( !pSh->GetDocument()->IsDocEditable() )
                                {
                                    ErrorMessage(STR_READONLYERR);
                                    bDoIt = FALSE;
                                }
                            }
                        }
                        rReq.AppendItem( SfxStringItem( FID_TAB_MOVE, aDocName ) );
                        //  Tabelle ist 1-basiert, wenn nicht APPEND
                        SCTAB nBasicTab = ( nTab <= MAXTAB ) ? (nTab+1) : nTab;
                        rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nBasicTab) ) );
                        rReq.AppendItem( SfxBoolItem( FN_PARAM_2, bCpy ) );
                    }
                    delete pDlg;
                }

                if( bDoIt )
                {
                    rReq.Done();        // aufzeichnen, solange das Dokument noch aktiv ist

                    MoveTable( nDoc, nTab, bCpy );
                }
            }
            break;

        case FID_DELETE_TABLE:
            {
                //  Parameter war ueberfluessig, weil die Methode an der Table haengt

                BOOL bDoIt = rReq.IsAPI();
                if( !bDoIt )
                {
                    //  wenn's nicht von Basic kommt, nochmal nachfragen:

                    bDoIt = ( RET_YES ==
                              QueryBox( GetDialogParent(),
                                        WinBits( WB_YES_NO | WB_DEF_YES ),
                                        ScGlobal::GetRscString(STR_QUERY_DELTAB)
                                       ).Execute() );
                }
                if( bDoIt )
                {
                    SCTAB nNewTab   = nCurrentTab;
                    SCTAB nFirstTab=0;
                    BOOL   bTabFlag=FALSE;
                    ScMarkData& rMark = pViewData->GetMarkData();
                    SvShorts TheTabs;
                    for(SCTAB i=0;i<nTabCount;i++)
                    {
                        if(rMark.GetTableSelect(i) &&!pDoc->IsTabProtected(i))
                        {
                            TheTabs.Insert(i,TheTabs.Count());
                            bTabFlag=TRUE;
                            if(nNewTab==i) nNewTab++;
                        }
                        if(!bTabFlag) nFirstTab=i;
                    }
                    if(nNewTab>=nTabCount) nNewTab=nFirstTab;

                    pViewData->SetTabNo(nNewTab);
                    DeleteTables(TheTabs);
                    TheTabs.Remove(0,TheTabs.Count());
                    rReq.Done();
                }
            }
            break;

        case FID_TAB_RTL:
            {
                ScDocShell* pDocSh = pViewData->GetDocShell();
                ScDocFunc aFunc(*pDocSh);
                BOOL bSet = !pDoc->IsLayoutRTL( nCurrentTab );

                const ScMarkData& rMark = pViewData->GetMarkData();
                if ( rMark.GetSelectCount() != 0 )
                {
                    //  handle several sheets

                    SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
                    String aUndo = ScGlobal::GetRscString( STR_UNDO_TAB_RTL );
                    pUndoMgr->EnterListAction( aUndo, aUndo );

                    SCTAB nTabCount = pDoc->GetTableCount();
                    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                        if ( rMark.GetTableSelect(nTab) )
                            aFunc.SetLayoutRTL( nTab, bSet, FALSE );

                    pUndoMgr->LeaveListAction();
                }
                else
                    aFunc.SetLayoutRTL( nCurrentTab, bSet, FALSE );
            }
            break;

        default:
            DBG_ERROR("Unbekannte Message bei ViewShell");
            break;
    }
}

//------------------------------------------------------------------

void ScTabViewShell::GetStateTable( SfxItemSet& rSet )
{
    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScMarkData& rMark       = GetViewData()->GetMarkData();
    SCCOL       nPosX       = pViewData->GetCurX();
    SCROW       nPosY       = pViewData->GetCurY();
    SCTAB       nTab        = pViewData->GetTabNo();

    BOOL bOle = GetViewData()->GetDocShell()->IsOle();

    SCTAB nTabCount = pDoc->GetTableCount();
    SCTAB nTabSelCount = rMark.GetSelectCount();

    SfxWhichIter    aIter(rSet);
    USHORT          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {

            case FID_TABLE_VISIBLE:
                rSet.Put( SfxBoolItem( nWhich, pDoc->IsVisible(nTab) ));
                break;

            case FID_TABLE_HIDE:
                {
                    USHORT nVis = 0;
                    for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
                        if (pDoc->IsVisible(i))
                            ++nVis;

                    if ( nVis<2 || !pDoc->IsDocEditable() || nTabSelCount > 1 )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_TABLE_SHOW:
                {
                    BOOL bHasHidden = FALSE;
                    for ( SCTAB i=0; i < nTabCount && !bHasHidden; i++ )
                        if (!pDoc->IsVisible(i))
                            bHasHidden = TRUE;
                    if ( !bHasHidden || pDoc->IsDocProtected() || nTabSelCount > 1 )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_DELETE_TABLE:
                {
                    if ( pDoc->GetChangeTrack() )
                        rSet.DisableItem( nWhich );
                    else
                    {
                        USHORT nVis = 0;
                        for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
                            if (pDoc->IsVisible(i))
                                ++nVis;
                        if (   pDoc->IsTabProtected(nTab)
                            || !pDoc->IsDocEditable()
                            || nVis < 2
                            || nTabSelCount == nTabCount)
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case FID_INS_TABLE:
            case FID_INS_TABLE_EXT:
            case FID_TAB_APPEND:
                if (   !pDoc->IsDocEditable()
                    || nTabCount > MAXTAB)
                    rSet.DisableItem( nWhich );
                break;

            case FID_TAB_MOVE:
                if (   !pDoc->IsDocEditable()
                    || pDoc->GetChangeTrack() != NULL
                    || nTabCount > MAXTAB)
                    rSet.DisableItem( nWhich );
                break;

            //  FID_TAB_MENU_RENAME - "umbenennen" im Menu
            //  FID_TAB_RENAME      - "Name"-Property fuer Basic

            case FID_TAB_MENU_RENAME:
                if ( !pDoc->IsDocEditable() ||
                     pDoc->IsTabProtected(nTab) ||nTabSelCount > 1 )
                    rSet.DisableItem( nWhich );
                break;

            case FID_TAB_RENAME:
                {
                    String aTabName;
                    pDoc->GetName( nTab, aTabName );

                    rSet.Put( SfxStringItem( nWhich, aTabName ));

                }
                break;

            case FID_TAB_RTL:
                {
                    SvtLanguageOptions aLangOpt;
                    if ( !aLangOpt.IsCTLFontEnabled() )
                        rSet.DisableItem( nWhich );
                    else
                        rSet.Put( SfxBoolItem( nWhich, pDoc->IsLayoutRTL( nTab ) ) );
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}




