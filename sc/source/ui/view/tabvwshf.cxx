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

#include <memory>

#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <basic/sbstar.hxx>
#include <basic/sberrors.hxx>
#include <svl/languageoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/objface.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/colritem.hxx>

#include <tabvwsh.hxx>
#include <sc.hrc>
#include <helpids.h>
#include <docsh.hxx>
#include <document.hxx>
#include <scresid.hxx>
#include <globstr.hrc>
#include <strings.hrc>
#include <docfunc.hxx>
#include <eventuno.hxx>
#include <dpobject.hxx>
#include <dpshttab.hxx>

#include <scabstdlg.hxx>

#include <tabbgcolor.hxx>
#include <markdata.hxx>

#include <vector>

using std::unique_ptr;
using namespace com::sun::star;

void ScTabViewShell::ExecuteTable( SfxRequest& rReq )
{
    ScViewData& rViewData   = GetViewData();
    ScDocument& rDoc        = rViewData.GetDocument();

    SCTAB       nCurrentTab = rViewData.GetTabNo();
    SCTAB       nTabCount   = rDoc.GetTableCount();
    sal_uInt16  nSlot       = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    HideListBox();                  // Autofilter-DropDown-Listbox

    switch ( nSlot )
    {
        case FID_TABLE_VISIBLE:
            {
                OUString aName;
                rDoc.GetName( nCurrentTab, aName );

                bool bVisible=true;
                if( pReqArgs != nullptr )
                {
                    const SfxPoolItem* pItem;
                    if( pReqArgs->HasItem( FID_TABLE_VISIBLE, &pItem ) )
                        bVisible = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                }

                if( ! bVisible )            // fade out
                {
                    if ( rDoc.IsDocEditable() )
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
                if ( rDoc.IsDocEditable() )
                {
                    ScMarkData& rMark = rViewData.GetMarkData();
                    SCTAB nActiveTab = -1;
                    // For the cases when user right clicks on a non-active tab and hides it. This case is possible for Online.
                    if (pReqArgs)
                    {
                        const SfxPoolItem *pItem;
                        if( pReqArgs->HasItem( FID_TABLE_HIDE, &pItem ) )
                        {
                            SCTAB nTabNumber = static_cast<const SfxInt16Item*>(pItem)->GetValue();
                            // Does selected sheets (tabs) list include the sheet to be hidden?
                            std::set<SCTAB>::iterator it = rMark.GetSelectedTabs().find(nTabNumber);
                            if (it == rMark.GetSelectedTabs().end())
                            {
                                // No it doesn't, so we won't shift the selected tab. Let's remember its position.
                                nActiveTab = GetViewData().GetTabNo();
                            }
                            rMark.SelectOneTable(nTabNumber);
                        }
                    }
                    HideTable( rMark, nActiveTab );
                }
            }
            break;

        case FID_TABLE_SHOW:
            {
                std::vector<OUString> rNames;
                if ( pReqArgs )
                {
                    const SfxPoolItem* pItem;
                    if( pReqArgs->HasItem( FID_TABLE_SHOW, &pItem ) )
                    {
                        OUString aName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        rNames.push_back(aName);
                        ShowTable( rNames );

                        if( ! rReq.IsAPI() )
                            rReq.Done();
                    }
                }
                else
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                    VclPtr<AbstractScShowTabDlg> pDlg(pFact->CreateScShowTabDlg(GetFrameWeld()));

                    OUString aTabName;
                    bool bFirst = true;
                    for ( SCTAB i=0; i != nTabCount; i++ )
                    {
                        if (!rDoc.IsVisible(i))
                        {
                            rDoc.GetName( i, aTabName );
                            pDlg->Insert( aTabName, bFirst );
                            bFirst = false;
                        }
                    }

                    std::shared_ptr<SfxRequest> pReq = std::make_shared<SfxRequest>(rReq);
                    pDlg->StartExecuteAsync([this, pDlg, pReq](sal_Int32 nResult){
                        std::vector<OUString> sTables;
                        if (RET_OK == nResult)
                        {
                            std::vector<sal_Int32> aSelectedRows = pDlg->GetSelectedRows();
                            for (auto a : aSelectedRows)
                            {
                                OUString sTable = pDlg->GetEntry(a);
                                pReq->AppendItem( SfxStringItem( FID_TABLE_SHOW, sTable ) );
                                sTables.push_back(sTable);
                            }
                            ShowTable( sTables );
                            pReq->Done();
                        }
                        pDlg->disposeOnce();
                    });
                    rReq.Ignore();
                }
            }
            break;

        case FID_INS_TABLE:
        case FID_INS_TABLE_EXT:
            {
                ScMarkData& rMark    = rViewData.GetMarkData();
                SCTAB   nTabSelCount = rMark.GetSelectCount();
                SCTAB   nTabNr       = nCurrentTab;

                if ( !rDoc.IsDocEditable() )
                    break;                          // locked

                if ( pReqArgs != nullptr )             // from basic
                {
                    bool bOk = false;
                    const SfxPoolItem*  pTabItem;
                    const SfxPoolItem*  pNameItem;

                    if ( pReqArgs->HasItem( FN_PARAM_1, &pTabItem ) &&
                         pReqArgs->HasItem( nSlot, &pNameItem ) )
                    {
                        OUString aName = static_cast<const SfxStringItem*>(pNameItem)->GetValue();
                        rDoc.CreateValidTabName(aName);

                        // sheet number from basic: 1-based
                        // 0 is special, means adding at the end
                        nTabNr = static_cast<const SfxUInt16Item*>(pTabItem)->GetValue();
                        if (nTabNr == 0)
                            nTabNr = nTabCount;
                        else
                            --nTabNr;

                        if (nTabNr > nTabCount)
                            nTabNr = nTabCount;

                        bOk = InsertTable(aName, nTabNr);
                    }

                    if (bOk)
                        rReq.Done( *pReqArgs );
                    //! else set error
                }
                else                                // dialog
                {
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                    ScopedVclPtr<AbstractScInsertTableDlg> pDlg(pFact->CreateScInsertTableDlg(GetFrameWeld(), rViewData,
                        nTabSelCount, nSlot == FID_INS_TABLE_EXT));
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
                                    ImportTables( pDlg->GetDocShellTables(), nTabs.size(), nTabs.data(),
                                                bLink,nTabNr );
                                }
                                else
                                {
                                    SCTAB   nTabAfter    = nTabNr+1;

                                    for(SCTAB j=nCurrentTab+1;j<nTabCount;j++)
                                    {
                                        if(!rDoc.IsScenario(j))
                                        {
                                            nTabAfter=j;
                                            break;
                                        }
                                    }

                                    ImportTables( pDlg->GetDocShellTables(), nTabs.size(), nTabs.data(),
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
                                    if(!rDoc.IsScenario(j))
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

                if ( !rDoc.IsDocEditable() )
                    break; // everything locked

                if ( nSlot != FID_TAB_APPEND &&
                        ( rDoc.IsTabProtected( nTabNr ) || nTabSelCount > 1 ) )
                    break; // no rename

                if( pReqArgs != nullptr )
                {
                    bool        bDone   = false;
                    const SfxPoolItem* pItem;
                    OUString      aName;

                    if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                    {
                        nTabNr = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

                        // inserting is 1-based, let's be consistent
                        if (nTabNr > 0)
                            --nTabNr;
                    }

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
                    OUString      aErrMsg ( ScResId( STR_INVALIDTABNAME ) );
                    OUString aName;
                    OUString      aDlgTitle;
                    const char* pHelpId = nullptr;

                    switch ( nSlot )
                    {
                        case FID_TAB_APPEND:
                            aDlgTitle = ScResId(SCSTR_APDTABLE);
                            rDoc.CreateValidTabName( aName );
                            pHelpId = HID_SC_APPEND_NAME;
                            break;

                        case FID_TAB_RENAME:
                            aDlgTitle = ScResId(SCSTR_RENAMETAB);
                            rDoc.GetName( rViewData.GetTabNo(), aName );
                            pHelpId = HID_SC_RENAME_NAME;
                            break;
                    }

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                    ScopedVclPtr<AbstractScStringInputDlg> pDlg(pFact->CreateScStringInputDlg(
                        GetFrameWeld(), aDlgTitle, ScResId(SCSTR_NAME),
                        aName, GetStaticInterface()->GetSlot(nSlot)->GetCommand(),
                        pHelpId));


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
                                    StarBASIC::Error( ERRCODE_BASIC_SETPROP_FAILED ); // XXX error handling???
#endif
                                }
                                else
                                {
                                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                              VclMessageType::Warning, VclButtonsType::Ok, aErrMsg));
                                    nRet = xBox->run();
                                }
                            }
                        }
                    }
                }
            }
            break;

        case FID_TAB_MOVE:
            {
                if ( rDoc.GetChangeTrack() != nullptr )
                    break;    // if ChangeTracking is active, then no TabMove

                bool   bDoIt = false;
                sal_uInt16 nDoc = 0;
                SCTAB nTab = rViewData.GetTabNo();
                bool   bCpy = false;
                OUString aDocName;
                OUString aTabName;

                if( pReqArgs != nullptr )
                {
                    SCTAB nTableCount = rDoc.GetTableCount();
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
                        ScDocShell*     pScSh   = nullptr;
                        sal_uInt16          i=0;

                        while ( pSh )
                        {
                            pScSh = dynamic_cast<ScDocShell*>( pSh  );

                            if( pScSh )
                            {
                                pScSh->GetTitle();

                                if (aDocName == pScSh->GetTitle())
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
                    rDoc.GetName( rViewData.GetTabNo(), aDefaultName );

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                    ScopedVclPtr<AbstractScMoveTableDlg> pDlg(pFact->CreateScMoveTableDlg(GetFrameWeld(),
                        aDefaultName));

                    SCTAB nTableCount = rDoc.GetTableCount();
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
                bool bHasIndex = (pReqArgs != nullptr);

                // allow removing via the Index/FID_DELETE_TABLE parameter
                SCTAB nTabNr = nCurrentTab;
                if (bHasIndex)
                {
                    const SfxPoolItem* pItem;
                    if (pReqArgs->HasItem(FID_DELETE_TABLE, &pItem))
                    {
                        nTabNr = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

                        // inserting is 1-based, let's be consistent
                        if (nTabNr > 0)
                            --nTabNr;
                    }
                }

                bool bDoIt = bHasIndex;
                if (!bDoIt)
                {
                    bool bTabWithPivotTable = false;
                    if (rDoc.HasPivotTable())
                    {
                        const ScDPCollection* pDPs = rDoc.GetDPCollection();
                        if (pDPs)
                        {
                            const ScMarkData::MarkedTabsType& rSelectedTabs = rViewData.GetMarkData().GetSelectedTabs();
                            for (const SCTAB nSelTab : rSelectedTabs)
                            {
                                const size_t nCount = pDPs->GetCount();
                                for (size_t i = 0; i < nCount; ++i)
                                {
                                    const ScDPObject& rDPObj = (*pDPs)[i];
                                    const ScSheetSourceDesc* pSheetSourceDesc = rDPObj.GetSheetDesc();
                                    if (pSheetSourceDesc && pSheetSourceDesc->GetSourceRange().aStart.Tab() == nSelTab)
                                        bTabWithPivotTable = true;
                                }
                                if (bTabWithPivotTable)
                                    break;
                            }
                        }
                    }

                    if (bTabWithPivotTable)
                    {
                        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                                       ScResId(STR_QUERY_PIVOTTABLE_DELTAB)));
                        xQueryBox->set_default_response(RET_NO);

                        // Hard warning as there is potential of data loss on deletion
                        bDoIt = (RET_YES == xQueryBox->run());
                    }
                    else
                    {
                        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                                       ScResId(STR_QUERY_DELTAB)));
                        xQueryBox->set_default_response(RET_YES);

                        // no parameter given, ask for confirmation
                        bDoIt = (RET_YES == xQueryBox->run());
                    }
                }

                if (bDoIt)
                {
                    SCTAB nNewTab = nCurrentTab;
                    std::vector<SCTAB> TheTabs;

                    if (bHasIndex)
                    {
                        // sheet no. provided by the parameter
                        TheTabs.push_back(nTabNr);
                        if (nNewTab > nTabNr && nNewTab > 0)
                            --nNewTab;
                    }
                    else
                    {
                        SCTAB nFirstTab = 0;
                        bool bTabFlag = false;
                        ScMarkData& rMark = rViewData.GetMarkData();
                        for (SCTAB i = 0; i < nTabCount; i++)
                        {
                            if (rMark.GetTableSelect(i) && !rDoc.IsTabProtected(i))
                            {
                                TheTabs.push_back(i);
                                bTabFlag = true;
                                if (nNewTab == i && i+1 < nTabCount)
                                    nNewTab++;
                            }
                            if (!bTabFlag)
                                nFirstTab = i;
                        }
                        if (nNewTab >= nTabCount - static_cast<SCTAB>(TheTabs.size()))
                            nNewTab = nFirstTab;
                    }

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
                bool bSet = !rDoc.IsLayoutRTL( nCurrentTab );

                const ScMarkData& rMark = rViewData.GetMarkData();
                if ( rMark.GetSelectCount() != 0 )
                {
                    //  handle several sheets

                    SfxUndoManager* pUndoManager = pDocSh->GetUndoManager();
                    OUString aUndo = ScResId( STR_UNDO_TAB_RTL );
                    pUndoManager->EnterListAction( aUndo, aUndo, 0, rViewData.GetViewShell()->GetViewShellId() );

                    for (const auto& rTab : rMark)
                        rFunc.SetLayoutRTL( rTab, bSet );

                    pUndoManager->LeaveListAction();
                }
                else
                    rFunc.SetLayoutRTL( nCurrentTab, bSet );
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
                if ( !rDoc.IsDocEditable() )
                    break;

                if ( rDoc.IsTabProtected( nTabNr ) ) // ||nTabSelCount > 1
                    break;

                if( pReqArgs != nullptr )
                {
                    bool                bDone = false;
                    const SfxPoolItem*  pItem;
                    Color               aColor;

                    if( pReqArgs->HasItem( nSlot, &pItem ) )
                        aColor = static_cast<const SvxColorItem*>(pItem)->GetValue();

                    if ( nTabSelCount > 1 )
                    {
                        std::unique_ptr<ScUndoTabColorInfo::List>
                            pTabColorList(new ScUndoTabColorInfo::List);
                        for (const auto& rTab : rMark)
                        {
                            if ( !rDoc.IsTabProtected(rTab) )
                            {
                                ScUndoTabColorInfo aTabColorInfo(rTab);
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

                    Color aTabBgColor = rDoc.GetTabBgColor( nCurrentTab );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    ScopedVclPtr<AbstractScTabBgColorDlg> pDlg(pFact->CreateScTabBgColorDlg(
                                                                GetFrameWeld(),
                                                                ScResId(SCSTR_SET_TAB_BG_COLOR),
                                                                ScResId(SCSTR_NO_TAB_BG_COLOR),
                                                                aTabBgColor));
                    while ( !bDone && nRet == RET_OK )
                    {
                        nRet = pDlg->Execute();
                        if( nRet == RET_OK )
                        {
                            Color aSelectedColor;
                            pDlg->GetSelectedColor(aSelectedColor);
                            std::unique_ptr<ScUndoTabColorInfo::List>
                                pTabColorList(new ScUndoTabColorInfo::List);
                            if ( nTabSelCount > 1 )
                            {
                                for (const auto& rTab : rMark)
                                {
                                    if ( !rDoc.IsTabProtected(rTab) )
                                    {
                                        ScUndoTabColorInfo aTabColorInfo(rTab);
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
                                    StarBASIC::Error( ERRCODE_BASIC_SETPROP_FAILED );
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
                    ScopedVclPtr<VclAbstractDialog> pDialog( pDlgFactory->CreateSvxMacroAssignDlg(
                        GetFrameWeld(), xFrame, false, xEvents, 0 ) );
                    if ( pDialog->Execute() == RET_OK )
                    {
                            // the dialog modifies the settings directly
                    }
                }
                break;

        default:
                OSL_FAIL("unknown message for ViewShell");
                break;
    }
}

void ScTabViewShell::GetStateTable( SfxItemSet& rSet )
{
    ScViewData& rViewData   = GetViewData();
    ScDocument& rDoc        = rViewData.GetDocument();
    ScDocShell* pDocShell   = rViewData.GetDocShell();
    ScMarkData& rMark       = GetViewData().GetMarkData();
    SCTAB       nTab        = rViewData.GetTabNo();

    SCTAB nTabCount = rDoc.GetTableCount();
    SCTAB nTabSelCount = rMark.GetSelectCount();

    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {

            case FID_TABLE_VISIBLE:
                rSet.Put( SfxBoolItem( nWhich, rDoc.IsVisible(nTab) ));
                break;

            case FID_TABLE_HIDE:
                {
                    sal_uInt16 nVis = 0;
                    // enable menu : check to make sure we won't hide all sheets. we need at least one visible at all times.
                    for ( SCTAB i=0; i < nTabCount && nVis<nTabSelCount + 1; i++ )
                        if (rDoc.IsVisible(i))
                            ++nVis;
                    if ( nVis<=nTabSelCount || !rDoc.IsDocEditable() )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_TABLE_SHOW:
                {
                    bool bHasHidden = false;
                    for ( SCTAB i=0; i < nTabCount && !bHasHidden; i++ )
                        if (!rDoc.IsVisible(i))
                            bHasHidden = true;
                    if ( !bHasHidden || rDoc.IsDocProtected() || nTabSelCount > 1 )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_DELETE_TABLE:
                {
                    if ( rDoc.GetChangeTrack() )
                        rSet.DisableItem( nWhich );
                    else
                    {
                        sal_uInt16 nVis = 0;
                        for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
                            if (rDoc.IsVisible(i))
                                ++nVis;
                        if (   rDoc.IsTabProtected(nTab)
                            || !rDoc.IsDocEditable()
                            || nVis < 2
                            || nTabSelCount == nTabCount)
                            rSet.DisableItem( nWhich );
                    }
                }
                break;

            case FID_INS_TABLE:
            case FID_INS_TABLE_EXT:
            case FID_TAB_APPEND:
                if ( !rDoc.IsDocEditable() ||
                     nTabCount > MAXTAB ||
                     ( nWhich == FID_INS_TABLE_EXT && pDocShell && pDocShell->IsDocShared() ) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_TAB_MOVE:
                if (   !rDoc.IsDocEditable()
                    || rDoc.GetChangeTrack() != nullptr
                    || nTabCount > MAXTAB)
                    rSet.DisableItem( nWhich );
                break;

            //  FID_TAB_MENU_RENAME - "rename" from Menu
            //  FID_TAB_RENAME      - "name"-property for Basic

            case FID_TAB_MENU_RENAME:
                if ( !rDoc.IsDocEditable() ||
                     rDoc.IsTabProtected(nTab) ||nTabSelCount > 1 ||
                     ( pDocShell && pDocShell->IsDocShared() ) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_TAB_RENAME:
                {
                    OUString aTabName;
                    rDoc.GetName( nTab, aTabName );

                    rSet.Put( SfxStringItem( nWhich, aTabName ));

                }
                break;

            case FID_TAB_RTL:
                {
                    SvtLanguageOptions aLangOpt;
                    if ( !aLangOpt.IsCTLFontEnabled() )
                        rSet.DisableItem( nWhich );
                    else
                        rSet.Put( SfxBoolItem( nWhich, rDoc.IsLayoutRTL( nTab ) ) );
                }
                break;

            case FID_TAB_MENU_SET_TAB_BG_COLOR:
                {
                    if ( !rDoc.IsDocEditable()
                        || ( pDocShell && pDocShell->IsDocShared() )
                        || rDoc.IsTabProtected(nTab) )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_TAB_SET_TAB_BG_COLOR:
                {
                    Color aColor = rDoc.GetTabBgColor( nTab );
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
