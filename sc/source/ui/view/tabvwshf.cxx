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

#include <config_features.h>

#include <boost/scoped_ptr.hpp>

#include "scitems.hxx"
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <basic/sbstar.hxx>
#include <svl/languageoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/objface.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/colritem.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "docsh.hxx"
#include "document.hxx"
#include "shtabdlg.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "docfunc.hxx"
#include "eventuno.hxx"

#include "scabstdlg.hxx"

#include "tabbgcolor.hxx"
#include "tabbgcolordlg.hxx"
#include "sccommands.h"
#include "markdata.hxx"

#include <vector>

using ::boost::scoped_ptr;
using namespace com::sun::star;

void ScTabViewShell::ExecuteTable( SfxRequest& rReq )
{
    ScViewData& rViewData   = GetViewData();
    ScDocument* pDoc        = rViewData.GetDocument();

    SCTAB       nCurrentTab = rViewData.GetTabNo();
    SCTAB       nTabCount   = pDoc->GetTableCount();
    sal_uInt16  nSlot       = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    HideListBox();                  // Autofilter-DropDown-Listbox

    switch ( nSlot )
    {
        case FID_TABLE_VISIBLE:
            {
                OUString aName;
                pDoc->GetName( nCurrentTab, aName );

                bool bVisible=true;
                if( pReqArgs != NULL )
                {
                    const SfxPoolItem* pItem;
                    if( pReqArgs->HasItem( FID_TABLE_VISIBLE, &pItem ) )
                        bVisible = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                }

                if( ! bVisible )            // fade out
                {
                    if ( pDoc->IsDocEditable() )
                    {
                        ScMarkData& rMark = rViewData.GetMarkData();
                        HideTable( rMark );
                    }
                }
                else                        // fade in
                {
                    std::vector<OUString> rNames;
                    rNames.push_back(aName);
                    ShowTable( rNames );
                }
            }
            break;

        case FID_TABLE_HIDE:
            {
                if ( pDoc->IsDocEditable() )
                {
                    ScMarkData& rMark = rViewData.GetMarkData();
                    HideTable( rMark );
                }
            }
            break;

        case FID_TABLE_SHOW:
            {
                OUString aName;
                std::vector<OUString> rNames;
                if ( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    if( pReqArgs->HasItem( FID_TABLE_SHOW, &pItem ) )
                    {
                        aName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        rNames.push_back(aName);
                        ShowTable( rNames );

                        if( ! rReq.IsAPI() )
                            rReq.Done();
                    }
                }
                else
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    boost::scoped_ptr<AbstractScShowTabDlg> pDlg(pFact->CreateScShowTabDlg(GetDialogParent()));
                    OSL_ENSURE(pDlg, "Dialog create fail!");

                    OUString aTabName;
                    bool bFirst = true;
                    for ( SCTAB i=0; i != nTabCount; i++ )
                    {
                        if (!pDoc->IsVisible(i))
                        {
                            pDoc->GetName( i, aTabName );
                            pDlg->Insert( aTabName, bFirst );
                            bFirst = false;
                        }
                    }

                    if ( pDlg->Execute() == RET_OK )
                    {
                        sal_uInt16 nCount = pDlg->GetSelectEntryCount();
                        for (sal_uInt16 nPos=0; nPos<nCount; nPos++)
                        {
                            aName = pDlg->GetSelectEntry(nPos);
                            rReq.AppendItem( SfxStringItem( FID_TABLE_SHOW, aName ) );
                            rNames.push_back(aName);
                        }
                        ShowTable( rNames );
                        rReq.Done();
                    }
                }
            }
            break;

        case FID_INS_TABLE:
        case FID_INS_TABLE_EXT:
            {
                ScMarkData& rMark    = rViewData.GetMarkData();
                SCTAB   nTabSelCount = rMark.GetSelectCount();
                SCTAB   nTabNr       = nCurrentTab;

                if ( !pDoc->IsDocEditable() )
                    break;                          // locked

                if ( pReqArgs != NULL )             // from basic
                {
                    bool bOk = false;
                    const SfxPoolItem*  pTabItem;
                    const SfxPoolItem*  pNameItem;
                    OUString            aName;

                    if ( pReqArgs->HasItem( FN_PARAM_1, &pTabItem ) &&
                         pReqArgs->HasItem( nSlot, &pNameItem ) )
                    {
                        //  tablenumber from basic: 1-based

                        aName = static_cast<const SfxStringItem*>(pNameItem)->GetValue();
                        nTabNr = static_cast<const SfxUInt16Item*>(pTabItem)->GetValue() - 1;
                        if ( nTabNr < nTabCount )
                            bOk = InsertTable( aName, nTabNr );
                    }

                    if (bOk)
                        rReq.Done( *pReqArgs );
                    //! else set error
                }
                else                                // dialog
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    boost::scoped_ptr<AbstractScInsertTableDlg> pDlg(pFact->CreateScInsertTableDlg(GetDialogParent(), rViewData,
                        nTabSelCount, nSlot == FID_INS_TABLE_EXT));
                    OSL_ENSURE(pDlg, "Dialog create fail!");
                    if ( RET_OK == pDlg->Execute() )
                    {
                        if (pDlg->GetTablesFromFile())
                        {
                            std::vector<SCTAB> nTabs;
                            sal_uInt16 n = 0;
                            const OUString* pStr = pDlg->GetFirstTable( &n );
                            while ( pStr )
                            {
                                nTabs.push_back( static_cast<SCTAB>(n) );
                                pStr = pDlg->GetNextTable( &n );
                            }
                            bool bLink = pDlg->GetTablesAsLink();
                            if (!nTabs.empty())
                            {
                                if(pDlg->IsTableBefore())
                                {
                                    ImportTables( pDlg->GetDocShellTables(), nTabs.size(), &nTabs[0],
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

                                    ImportTables( pDlg->GetDocShellTables(), nTabs.size(), &nTabs[0],
                                                bLink,nTabAfter );
                                }
                            }
                        }
                        else
                        {
                            SCTAB nCount=pDlg->GetTableCount();
                            if(pDlg->IsTableBefore())
                            {
                                if(nCount==1 && !pDlg->GetFirstTable()->isEmpty())
                                {
                                    rReq.AppendItem( SfxStringItem( FID_INS_TABLE, *pDlg->GetFirstTable() ) );
                                    rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nTabNr) + 1 ) );        // 1-based
                                    rReq.Done();

                                    InsertTable( *pDlg->GetFirstTable(), nTabNr );
                                }
                                else
                                {
                                    std::vector<OUString> aNames(0);
                                    InsertTables( aNames, nTabNr,nCount );
                                }
                            }
                            else
                            {
                                SCTAB   nTabAfter    = nTabNr+1;
                                SCTAB nSelHigh = rMark.GetLastSelected();

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

                                if(nCount==1 && !pDlg->GetFirstTable()->isEmpty())
                                {
                                    rReq.AppendItem( SfxStringItem( FID_INS_TABLE, *pDlg->GetFirstTable() ) );
                                    rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nTabAfter) + 1 ) );     // 1-based
                                    rReq.Done();

                                    InsertTable( *pDlg->GetFirstTable(), nTabAfter);
                                }
                                else
                                {
                                    std::vector<OUString> aNames(0);
                                    InsertTables( aNames, nTabAfter,nCount);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case FID_TAB_APPEND:
        case FID_TAB_RENAME:
        case FID_TAB_MENU_RENAME:
            {
                //  FID_TAB_MENU_RENAME - "rename" in menu
                //  FID_TAB_RENAME      - "name"-property for basic
                //  equal execute, but MENU_RENAME may be disabled inside GetState

                if ( nSlot == FID_TAB_MENU_RENAME )
                    nSlot = FID_TAB_RENAME;             // equal execute

                SCTAB nTabNr = rViewData.GetTabNo();
                ScMarkData& rMark = rViewData.GetMarkData();
                SCTAB nTabSelCount = rMark.GetSelectCount();

                if ( !pDoc->IsDocEditable() )
                    break; // evrything locked

                if ( nSlot != FID_TAB_APPEND &&
                        ( pDoc->IsTabProtected( nTabNr ) || nTabSelCount > 1 ) )
                    break; // no rename

                if( pReqArgs != NULL )
                {
                    bool        bDone   = false;
                    const SfxPoolItem* pItem;
                    OUString      aName;

                    if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                        nTabNr = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

                    if( pReqArgs->HasItem( nSlot, &pItem ) )
                        aName = static_cast<const SfxStringItem*>(pItem)->GetValue();

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
                    sal_uInt16      nRet    = RET_OK;
                    bool        bDone   = false;
                    OUString      aErrMsg ( ScGlobal::GetRscString( STR_INVALIDTABNAME ) );
                    OUString aName;
                    OUString      aDlgTitle;
                    const sal_Char* pHelpId = 0;

                    switch ( nSlot )
                    {
                        case FID_TAB_APPEND:
                            aDlgTitle = OUString(ScResId(SCSTR_APDTABLE));
                            pDoc->CreateValidTabName( aName );
                            pHelpId = HID_SC_APPEND_NAME;
                            break;

                        case FID_TAB_RENAME:
                            aDlgTitle = OUString(ScResId(SCSTR_RENAMETAB));
                            pDoc->GetName( rViewData.GetTabNo(), aName );
                            pHelpId = HID_SC_RENAME_NAME;
                            break;
                    }

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    boost::scoped_ptr<AbstractScStringInputDlg> pDlg(pFact->CreateScStringInputDlg(
                        GetDialogParent(), aDlgTitle, OUString(ScResId(SCSTR_NAME)),
                        aName, GetStaticInterface()->GetSlot(nSlot)->GetCommand(),
                        pHelpId));

                    OSL_ENSURE(pDlg, "Dialog create fail!");

                    while ( !bDone && nRet == RET_OK )
                    {
                        nRet = pDlg->Execute();

                        if ( nRet == RET_OK )
                        {
                            aName = pDlg->GetInputString();

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
#if HAVE_FEATURE_SCRIPTING
                                    StarBASIC::Error( SbERR_SETPROP_FAILED ); // XXX error handling???
#endif
                                }
                                else
                                {
                                    nRet = MessageDialog(GetDialogParent(),
                                                     aErrMsg
                                                   ).Execute();
                                }
                            }
                        }
                    }
                }
            }
            break;

        case FID_TAB_MOVE:
            {
                if ( pDoc->GetChangeTrack() != NULL )
                    break;    // if ChangeTracking is active, then no TabMove

                bool   bDoIt = false;
                sal_uInt16 nDoc = 0;
                SCTAB nTab = rViewData.GetTabNo();
                bool   bCpy = false;
                OUString aDocName;
                OUString aTabName;

                if( pReqArgs != NULL )
                {
                    SCTAB nTableCount = pDoc->GetTableCount();
                    const SfxPoolItem* pItem;

                    if( pReqArgs->HasItem( FID_TAB_MOVE, &pItem ) )
                        aDocName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                    {
                        //  table is 1-based
                        nTab = static_cast<const SfxUInt16Item*>(pItem)->GetValue() - 1;
                        if ( nTab >= nTableCount )
                            nTab = SC_TAB_APPEND;
                    }
                    if( pReqArgs->HasItem( FN_PARAM_2, &pItem ) )
                        bCpy = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                    if (!aDocName.isEmpty())
                    {
                        SfxObjectShell* pSh     = SfxObjectShell::GetFirst();
                        ScDocShell*     pScSh   = NULL;
                        sal_uInt16          i=0;

                        while ( pSh )
                        {
                            pScSh = PTR_CAST( ScDocShell, pSh );

                            if( pScSh )
                            {
                                pScSh->GetTitle();

                                if (aDocName.equals(pScSh->GetTitle()))
                                {
                                    nDoc = i;
                                    ScDocument& rDestDoc = pScSh->GetDocument();
                                    nTableCount = rDestDoc.GetTableCount();
                                    bDoIt = rDestDoc.IsDocEditable();
                                    break;
                                }

                                i++;        // only count ScDocShell
                            }
                            pSh = SfxObjectShell::GetNext( *pSh );
                        }
                    }
                    else // no doc-name -> new doc
                    {
                        nDoc = SC_DOC_NEW;
                        bDoIt = true;
                    }

                    if ( bDoIt && nTab >= nTableCount )     // if necessary append
                        nTab = SC_TAB_APPEND;
                }
                else
                {
                    OUString aDefaultName;
                    pDoc->GetName( rViewData.GetTabNo(), aDefaultName );

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    boost::scoped_ptr<AbstractScMoveTableDlg> pDlg(pFact->CreateScMoveTableDlg(GetDialogParent(),
                        aDefaultName));
                    OSL_ENSURE(pDlg, "Dialog create fail!");

                    SCTAB nTableCount = pDoc->GetTableCount();
                    ScMarkData& rMark       = GetViewData().GetMarkData();
                    SCTAB       nTabSelCount = rMark.GetSelectCount();

                    if(nTableCount==nTabSelCount)
                    {
                        pDlg->SetForceCopyTable();
                    }

                    // We support direct renaming of sheet only when one sheet
                    // is selected.
                    pDlg->EnableRenameTable(nTabSelCount == 1);

                    if ( pDlg->Execute() == RET_OK )
                    {
                        nDoc = pDlg->GetSelectedDocument();
                        nTab = pDlg->GetSelectedTable();
                        bCpy = pDlg->GetCopyTable();
                        bool bRna = pDlg->GetRenameTable();
                        // Leave aTabName string empty, when Rename is FALSE.
                        if( bRna )
                        {
                           pDlg->GetTabNameString( aTabName );
                        }
                        bDoIt = true;

                        OUString aFoundDocName;
                        if ( nDoc != SC_DOC_NEW )
                        {
                            ScDocShell* pSh = ScDocShell::GetShellByNum( nDoc );
                            if (pSh)
                            {
                                aFoundDocName = pSh->GetTitle();
                                if ( !pSh->GetDocument().IsDocEditable() )
                                {
                                    ErrorMessage(STR_READONLYERR);
                                    bDoIt = false;
                                }
                            }
                        }
                        rReq.AppendItem( SfxStringItem( FID_TAB_MOVE, aFoundDocName ) );
                        // 1-based table, if not APPEND
                        SCTAB nBasicTab = ( nTab <= MAXTAB ) ? (nTab+1) : nTab;
                        rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nBasicTab) ) );
                        rReq.AppendItem( SfxBoolItem( FN_PARAM_2, bCpy ) );
                    }
                }

                if( bDoIt )
                {
                    rReq.Done();        // record, while doc is active

                    MoveTable( nDoc, nTab, bCpy, &aTabName );
                }
            }
            break;

        case FID_DELETE_TABLE:
            {
                //  unnecessary parameter ->  method depends on table

                bool bDoIt = rReq.IsAPI();
                if( !bDoIt )
                {
                    //  source isn't basic -> ask again

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
                    bool   bTabFlag=false;
                    ScMarkData& rMark = rViewData.GetMarkData();
                    std::vector<SCTAB> TheTabs;
                    for(SCTAB i=0;i<nTabCount;i++)
                    {
                        if(rMark.GetTableSelect(i) &&!pDoc->IsTabProtected(i))
                        {
                            TheTabs.push_back(i);
                            bTabFlag=true;
                            if(nNewTab==i) nNewTab++;
                        }
                        if(!bTabFlag) nFirstTab=i;
                    }
                    if(nNewTab>=nTabCount) nNewTab=nFirstTab;

                    rViewData.SetTabNo(nNewTab);
                    DeleteTables(TheTabs);
                    TheTabs.clear();
                    rReq.Done();
                }
            }
            break;

        case FID_TAB_RTL:
            {
                ScDocShell* pDocSh = rViewData.GetDocShell();
                ScDocFunc &rFunc = pDocSh->GetDocFunc();
                bool bSet = !pDoc->IsLayoutRTL( nCurrentTab );

                const ScMarkData& rMark = rViewData.GetMarkData();
                if ( rMark.GetSelectCount() != 0 )
                {
                    //  handle several sheets

                    ::svl::IUndoManager* pUndoManager = pDocSh->GetUndoManager();
                    OUString aUndo = ScGlobal::GetRscString( STR_UNDO_TAB_RTL );
                    pUndoManager->EnterListAction( aUndo, aUndo );

                    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
                    for (; itr != itrEnd; ++itr)
                        rFunc.SetLayoutRTL( *itr, bSet, false );

                    pUndoManager->LeaveListAction();
                }
                else
                    rFunc.SetLayoutRTL( nCurrentTab, bSet, false );
            }
            break;

        case FID_TAB_TOGGLE_GRID:
            {
                bool bShowGrid = rViewData.GetShowGrid();
                rViewData.SetShowGrid(!bShowGrid);
                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( FID_TAB_TOGGLE_GRID );
                PaintGrid();
                rReq.Done();
            }
            break;

        case FID_TAB_SET_TAB_BG_COLOR:
        case FID_TAB_MENU_SET_TAB_BG_COLOR:
            {
                if ( nSlot == FID_TAB_MENU_SET_TAB_BG_COLOR )
                    nSlot = FID_TAB_SET_TAB_BG_COLOR;
                SCTAB nTabNr = rViewData.GetTabNo();
                ScMarkData& rMark = rViewData.GetMarkData();
                SCTAB nTabSelCount = rMark.GetSelectCount();
                if ( !pDoc->IsDocEditable() )
                    break;

                if ( pDoc->IsTabProtected( nTabNr ) ) // ||nTabSelCount > 1
                    break;

                if( pReqArgs != NULL )
                {
                    bool                bDone = false;
                    const SfxPoolItem*  pItem;
                    Color               aColor;
                    if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                        nTabNr = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

                    if( pReqArgs->HasItem( nSlot, &pItem ) )
                        aColor = static_cast<const SvxColorItem*>(pItem)->GetValue();

                    if ( nTabSelCount > 1 )
                    {
                        scoped_ptr<ScUndoTabColorInfo::List>
                            pTabColorList(new ScUndoTabColorInfo::List);
                        ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
                        for (; itr != itrEnd; ++itr)
                        {
                            if ( !pDoc->IsTabProtected(*itr) )
                            {
                                ScUndoTabColorInfo aTabColorInfo(*itr);
                                aTabColorInfo.maNewTabBgColor = aColor;
                                pTabColorList->push_back(aTabColorInfo);
                            }
                        }
                        bDone = SetTabBgColor( *pTabColorList );
                    }
                    else
                    {
                        bDone = SetTabBgColor( aColor, nCurrentTab ); //ScViewFunc.SetTabBgColor
                    }
                    if( bDone )
                    {
                        rReq.Done( *pReqArgs );
                    }
                }
                else
                {
                    sal_uInt16      nRet    = RET_OK; /// temp
                    bool        bDone   = false; /// temp
                    Color       aTabBgColor;

                    aTabBgColor = pDoc->GetTabBgColor( nCurrentTab );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");
                    boost::scoped_ptr<AbstractScTabBgColorDlg> pDlg(pFact->CreateScTabBgColorDlg(
                                                                GetDialogParent(),
                                                                OUString(ScResId(SCSTR_SET_TAB_BG_COLOR)),
                                                                OUString(ScResId(SCSTR_NO_TAB_BG_COLOR)),
                                                                aTabBgColor,
                                                                CMD_FID_TAB_SET_TAB_BG_COLOR));
                    while ( !bDone && nRet == RET_OK )
                    {
                        nRet = pDlg->Execute();
                        if( nRet == RET_OK )
                        {
                            Color aSelectedColor;
                            pDlg->GetSelectedColor(aSelectedColor);
                            scoped_ptr<ScUndoTabColorInfo::List>
                                pTabColorList(new ScUndoTabColorInfo::List);
                            if ( nTabSelCount > 1 )
                            {
                                ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
                                for (; itr != itrEnd; ++itr)
                                    {
                                        if ( !pDoc->IsTabProtected(*itr) )
                                        {
                                            ScUndoTabColorInfo aTabColorInfo(*itr);
                                            aTabColorInfo.maNewTabBgColor = aSelectedColor;
                                            pTabColorList->push_back(aTabColorInfo);
                                        }
                                    }
                                    bDone = SetTabBgColor( *pTabColorList );
                                }
                                else
                                {
                                    bDone = SetTabBgColor( aSelectedColor, nCurrentTab ); //ScViewFunc.SetTabBgColor
                                }
                                if ( bDone )
                                {
                                    rReq.AppendItem( SvxColorItem( aTabBgColor, nSlot ) );
                                    rReq.Done();
                                }
                                else
                                {
                                    if( rReq.IsAPI() )
                                    {
#if HAVE_FEATURE_SCRIPTING
                                        StarBASIC::Error( SbERR_SETPROP_FAILED );
#endif
                                    }
                                }
                            }
                        }
                    }
                }
                break;

        case FID_TAB_EVENTS:
                {
                    ScDocShell* pDocSh = rViewData.GetDocShell();
                    uno::Reference<container::XNameReplace> xEvents( new ScSheetEventsObj( pDocSh, nCurrentTab ) );
                    uno::Reference<frame::XFrame> xFrame = GetViewFrame()->GetFrame().GetFrameInterface();
                    SvxAbstractDialogFactory* pDlgFactory = SvxAbstractDialogFactory::Create();
                    if (pDlgFactory)
                    {
                        boost::scoped_ptr<VclAbstractDialog> pDialog( pDlgFactory->CreateSvxMacroAssignDlg(
                            GetDialogParent(), xFrame, false, xEvents, 0 ) );
                        if ( pDialog.get() && pDialog->Execute() == RET_OK )
                        {
                            // the dialog modifies the settings directly
                        }
                    }
                }
                break;

        default:
                OSL_FAIL("Unbekannte Message bei ViewShell");
                break;
    }
}

void ScTabViewShell::GetStateTable( SfxItemSet& rSet )
{
    ScViewData& rViewData   = GetViewData();
    ScDocument* pDoc        = rViewData.GetDocument();
    ScDocShell* pDocShell   = rViewData.GetDocShell();
    ScMarkData& rMark       = GetViewData().GetMarkData();
    SCTAB       nTab        = rViewData.GetTabNo();

    SCTAB nTabCount = pDoc->GetTableCount();
    SCTAB nTabSelCount = rMark.GetSelectCount();

    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {

            case FID_TABLE_VISIBLE:
                rSet.Put( SfxBoolItem( nWhich, pDoc->IsVisible(nTab) ));
                break;

            case FID_TABLE_HIDE:
                {
                    sal_uInt16 nVis = 0;
                    // enable menu : check to make sure we won't hide all sheets. we need at least one visible at all times.
                    for ( SCTAB i=0; i < nTabCount && nVis<nTabSelCount + 1; i++ )
                        if (pDoc->IsVisible(i))
                            ++nVis;
                    if ( nVis<=nTabSelCount || !pDoc->IsDocEditable() )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_TABLE_SHOW:
                {
                    bool bHasHidden = false;
                    for ( SCTAB i=0; i < nTabCount && !bHasHidden; i++ )
                        if (!pDoc->IsVisible(i))
                            bHasHidden = true;
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
                        sal_uInt16 nVis = 0;
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
                if ( !pDoc->IsDocEditable() ||
                     nTabCount > MAXTAB ||
                     ( nWhich == FID_INS_TABLE_EXT && pDocShell && pDocShell->IsDocShared() ) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_TAB_MOVE:
                if (   !pDoc->IsDocEditable()
                    || pDoc->GetChangeTrack() != NULL
                    || nTabCount > MAXTAB)
                    rSet.DisableItem( nWhich );
                break;

            //  FID_TAB_MENU_RENAME - "rename" from Menu
            //  FID_TAB_RENAME      - "name"-property for Basic

            case FID_TAB_MENU_RENAME:
                if ( !pDoc->IsDocEditable() ||
                     pDoc->IsTabProtected(nTab) ||nTabSelCount > 1 ||
                     ( pDocShell && pDocShell->IsDocShared() ) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_TAB_RENAME:
                {
                    OUString aTabName;
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

            case FID_TAB_MENU_SET_TAB_BG_COLOR:
                {
                    if ( !pDoc->IsDocEditable()
                        || ( pDocShell && pDocShell->IsDocShared() )
                        || pDoc->IsTabProtected(nTab) )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_TAB_SET_TAB_BG_COLOR:
                {
                    Color aColor;
                    aColor = pDoc->GetTabBgColor( nTab );
                    rSet.Put( SvxColorItem( aColor, nWhich ) );
                }
                break;

            case FID_TAB_TOGGLE_GRID:
                rSet.Put( SfxBoolItem(nWhich, rViewData.GetShowGrid()) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
