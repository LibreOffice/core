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

#include "scitems.hxx"
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include "globstr.hrc"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "docsh.hxx"
#include "reffact.hxx"
#include "uiitems.hxx"
#include "autoform.hxx"
#include "autofmt.hxx"
#include "cellsh.hxx"
#include "inputhdl.hxx"
#include "editable.hxx"
#include "markdata.hxx"
#include "scabstdlg.hxx"

#include <config_telepathy.h>

#if ENABLE_TELEPATHY
#include "sccollaboration.hxx"
#endif

#include <memory>

#define IS_EDITMODE() GetViewData()->HasEditView( GetViewData()->GetActivePart() )

using sc::HMMToTwips;
using sc::TwipsToEvenHMM;

void ScCellShell::Execute( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    SfxBindings&        rBindings   = pTabViewShell->GetViewFrame()->GetBindings();
    ScModule*           pScMod      = SC_MOD();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();

    if (nSlot != SID_CURRENTCELL)       // this comes with MouseButtonUp
        pTabViewShell->HideListBox();   // Autofilter-DropDown-Listbox

    if ( IS_EDITMODE() )
    {
        switch ( nSlot )
        {
            //  when opening a reference-dialog the subshell may not be switched
            //  (on closing the dialog StopEditShell is called)
            case SID_OPENDLG_FUNCTION:
                    //  inplace leads to trouble with EditShell ...
                    //! cannot always be switched ????
                    if (!pTabViewShell->GetViewFrame()->GetFrame().IsInPlace())
                        pTabViewShell->SetDontSwitch(true);         // do not switch off EditShell
                    SAL_FALLTHROUGH;

            case FID_CELL_FORMAT:
            case SID_ENABLE_HYPHENATION:
            case SID_DATA_SELECT:
            case SID_OPENDLG_CONSOLIDATE:
            case SID_OPENDLG_SOLVE:
            case SID_OPENDLG_OPTSOLVER:

                    pScMod->InputEnterHandler();
                    pTabViewShell->UpdateInputHandler();

                    pTabViewShell->SetDontSwitch(false);

                    break;

            default:
                    break;
        }
    }

    switch ( nSlot )
    {
        case SID_COLLABORATION:
#if ENABLE_TELEPATHY
            GetViewData()->GetDocShell()->GetCollaboration()->DisplayContacts();
#endif
            break;

        case SID_STATUS_SELMODE:
            if ( pReqArgs )
            {
                /* 0: STD   Click cancels selection
                 * 1: ER    Click extends selection
                 * 2: ERG   Click defines further selection
                 */
                sal_uInt16 nMode = static_cast<const SfxUInt16Item&>(pReqArgs->Get( nSlot )).GetValue();

                switch ( nMode )
                {
                    case 1: nMode = KEY_SHIFT;  break;
                    case 2: nMode = KEY_MOD1;   break; // control-key
                    case 0:
                    default:
                        nMode = 0;
                }

                pTabViewShell->LockModifiers( nMode );
            }
            else
            {
                //  no arguments (also executed by double click on the status bar controller):
                //  advance to next selection mode

                sal_uInt16 nModifiers = pTabViewShell->GetLockedModifiers();
                switch ( nModifiers )
                {
                    case KEY_SHIFT: nModifiers = KEY_MOD1;  break;      // EXT -> ADD
                    case KEY_MOD1:  nModifiers = 0;         break;      // ADD -> STD
                    default:        nModifiers = KEY_SHIFT; break;      // STD -> EXT
                }
                pTabViewShell->LockModifiers( nModifiers );
            }

            rBindings.Invalidate( SID_STATUS_SELMODE );
            rReq.Done();
            break;

        //  SID_STATUS_SELMODE_NORM is not used ???

        case SID_STATUS_SELMODE_NORM:
            pTabViewShell->LockModifiers( 0 );
            rBindings.Invalidate( SID_STATUS_SELMODE );
            break;

        //  SID_STATUS_SELMODE_ERG / SID_STATUS_SELMODE_ERW as toggles:

        case SID_STATUS_SELMODE_ERG:
            if ( pTabViewShell->GetLockedModifiers() & KEY_MOD1 )
                pTabViewShell->LockModifiers( 0 );
            else
                pTabViewShell->LockModifiers( KEY_MOD1 );
            rBindings.Invalidate( SID_STATUS_SELMODE );
            break;

        case SID_STATUS_SELMODE_ERW:
            if ( pTabViewShell->GetLockedModifiers() & KEY_SHIFT )
                pTabViewShell->LockModifiers( 0 );
            else
                pTabViewShell->LockModifiers( KEY_SHIFT );
            rBindings.Invalidate( SID_STATUS_SELMODE );
            break;

        case SID_ENTER_STRING:
            {
                if ( pReqArgs )
                {
                    OUString aStr( static_cast<const SfxStringItem&>(pReqArgs->
                                    Get( SID_ENTER_STRING )).GetValue() );
                    const SfxPoolItem* pDontCommitItem;
                    bool bCommit = true;
                    if (pReqArgs->HasItem(FN_PARAM_1, &pDontCommitItem))
                        bCommit = !(static_cast<const SfxBoolItem*>(pDontCommitItem)->GetValue());

                    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( pTabViewShell );
                    if (bCommit)
                    {
                        pTabViewShell->EnterData( GetViewData()->GetCurX(),
                                                  GetViewData()->GetCurY(),
                                                  GetViewData()->GetTabNo(),
                                                  aStr );
                    }
                    else
                    {
                        SC_MOD()->SetInputMode(SC_INPUT_TABLE);

                        EditView* pTableView = pHdl->GetActiveView();
                        pHdl->DataChanging();
                        if (pTableView)
                            pTableView->GetEditEngine()->SetText(aStr);
                        pHdl->DataChanged();

                        SC_MOD()->SetInputMode(SC_INPUT_NONE);
                    }

                    if ( !pHdl || !pHdl->IsInEnterHandler() )
                    {
                        //  UpdateInputHandler is needed after the cell content
                        //  has changed, but if called from EnterHandler, UpdateInputHandler
                        //  will be called later when moving the cursor.
                        pTabViewShell->UpdateInputHandler();
                    }

                    rReq.Done();

                    //  no GrabFocus here, as otherwise on a Mac the tab jumps before the
                    //  sideview, when the input was not finished
                    //  (GrabFocus is called in KillEditView)
                }
            }
            break;

        case SID_INSERT_MATRIX:
            {
                if ( pReqArgs )
                {
                    OUString aStr = static_cast<const SfxStringItem&>(pReqArgs->
                                    Get( SID_INSERT_MATRIX )).GetValue();
                    ScDocument* pDoc = GetViewData()->GetDocument();
                    pTabViewShell->EnterMatrix( aStr, pDoc->GetGrammar() );
                    rReq.Done();
                }
            }
            break;

        case FID_INPUTLINE_ENTER:
        case FID_INPUTLINE_BLOCK:
        case FID_INPUTLINE_MATRIX:
            {
                if( pReqArgs == nullptr ) //XXX temporary HACK to avoid GPF
                    break;

                const ScInputStatusItem* pStatusItem
                    = static_cast<const ScInputStatusItem*>(&pReqArgs->
                            Get( FID_INPUTLINE_STATUS ));

                ScAddress aCursorPos = pStatusItem->GetPos();
                OUString aString = pStatusItem->GetString();
                const EditTextObject* pData = pStatusItem->GetEditData();

                if (pData)
                {
                    if (nSlot == FID_INPUTLINE_BLOCK)
                    {
                        pTabViewShell->EnterBlock( aString, pData );
                    }
                    else if ( !aString.isEmpty() && ( aString[0] == '=' || aString[0] == '+' || aString[0] == '-' ) )
                    {
                        pTabViewShell->EnterData( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), aString, pData );
                    }
                    else
                    {
                        pTabViewShell->EnterData(aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(), *pData);
                    }
                }
                else
                {
                    if (nSlot == FID_INPUTLINE_ENTER)
                    {
                        if (
                            aCursorPos.Col() == GetViewData()->GetCurX() &&
                            aCursorPos.Row() == GetViewData()->GetCurY() &&
                            aCursorPos.Tab() == GetViewData()->GetTabNo()
                            )
                        {
                            SfxStringItem   aItem( SID_ENTER_STRING, aString );

                            const SfxPoolItem* aArgs[2];
                            aArgs[0] = &aItem;
                            aArgs[1] = nullptr;
                            rBindings.Execute( SID_ENTER_STRING, aArgs );
                        }
                        else
                        {
                            pTabViewShell->EnterData( aCursorPos.Col(),
                                                    aCursorPos.Row(),
                                                    aCursorPos.Tab(),
                                                    aString );
                            rReq.Done();
                        }
                    }
                    else if (nSlot == FID_INPUTLINE_BLOCK)
                    {
                        pTabViewShell->EnterBlock( aString, nullptr );
                        rReq.Done();
                    }
                    else
                    {
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        pTabViewShell->EnterMatrix( aString, pDoc->GetGrammar() );
                        rReq.Done();
                    }
                }

                pTabViewShell->SetAutoSpellData(
                    aCursorPos.Col(), aCursorPos.Row(), pStatusItem->GetMisspellRanges());

                //  no GrabFocus here, as otherwise on a Mac the tab jumps before the
                //  sideview, when the input was not finished
                //  (GrabFocus is called in KillEditView)
            }
            break;

        case SID_OPENDLG_FUNCTION:
            {
                sal_uInt16 nId = SID_OPENDLG_FUNCTION;
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
                rReq.Ignore();
            }
            break;

        case SID_OPENDLG_CONSOLIDATE:
            {
                sal_uInt16          nId  = ScConsolidateDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case FID_CELL_FORMAT:
            {
                if ( pReqArgs != nullptr )
                {

                    // set cell attribute without dialog:

                    std::unique_ptr<SfxItemSet> pEmptySet(
                                        new SfxItemSet( *pReqArgs->GetPool(),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END ));

                    std::unique_ptr<SfxItemSet> pNewSet(
                                        new SfxItemSet( *pReqArgs->GetPool(),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END ));

                    const SfxPoolItem*  pAttr = nullptr;
                    sal_uInt16              nWhich = 0;

                    for ( nWhich=ATTR_PATTERN_START; nWhich<=ATTR_PATTERN_END; nWhich++ )
                        if ( pReqArgs->GetItemState( nWhich, true, &pAttr ) == SfxItemState::SET )
                            pNewSet->Put( *pAttr );

                    pTabViewShell->ApplyAttributes( pNewSet.get(), pEmptySet.get() );

                    pNewSet.reset();
                    pEmptySet.reset();

                    rReq.Done();
                }
                else
                {
                    pTabViewShell->ExecuteCellFormatDlg( rReq );
                }
            }
            break;

        case SID_ENABLE_HYPHENATION:
            pTabViewShell->ExecuteCellFormatDlg(rReq, "alignment");
            break;

        case SID_PROPERTY_PANEL_CELLTEXT_DLG:
            pTabViewShell->ExecuteCellFormatDlg( rReq, "font" );
            break;

        case SID_CELL_FORMAT_BORDER:
            pTabViewShell->ExecuteCellFormatDlg( rReq, "borders" );
            break;

        case SID_CHAR_DLG_EFFECT:
            pTabViewShell->ExecuteCellFormatDlg( rReq, "fonteffects" );
            break;

        case SID_OPENDLG_SOLVE:
            {
                sal_uInt16          nId  = ScSolverDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_OPENDLG_OPTSOLVER:
            {
                sal_uInt16 nId = ScOptSolverDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_OPENDLG_TABOP:
            {
                sal_uInt16          nId  = ScTabOpDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_SCENARIOS:
            {
                ScDocument* pDoc = GetViewData()->GetDocument();
                ScMarkData& rMark = GetViewData()->GetMarkData();
                SCTAB nTab = GetViewData()->GetTabNo();

                if ( pDoc->IsScenario(nTab) )
                {
                    rMark.MarkToMulti();
                    if ( rMark.IsMultiMarked() )
                    {
                        if (   rReq.IsAPI()
                            || RET_YES ==
                               ScopedVclPtr<QueryBox>::Create( pTabViewShell->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                         ScGlobal::GetRscString(STR_UPDATE_SCENARIO) )->
                                        Execute() )
                        {
                            pTabViewShell->ExtendScenario();
                            rReq.Done();
                        }
                    }
                    else if( ! rReq.IsAPI() )
                    {
                        ScopedVclPtrInstance<MessageDialog> aErrorBox(pTabViewShell->GetDialogParent(),
                                            ScGlobal::GetRscString(STR_NOAREASELECTED));
                        aErrorBox->Execute();
                    }
                }
                else
                {
                    rMark.MarkToMulti();
                    if ( rMark.IsMultiMarked() )
                    {
                        SCTAB i=1;
                        OUString aBaseName;
                        OUString aName;
                        OUString aComment;
                        Color  aColor;
                        ScScenarioFlags nFlags;

                        OUString aTmp;
                        pDoc->GetName(nTab, aTmp);
                        aBaseName = aTmp;
                        aBaseName += "_";
                        aBaseName += ScGlobal::GetRscString(STR_SCENARIO);
                        aBaseName += "_";

                        //  first test, if the prefix is recognised as valid,
                        //  else avoid only doubles
                        bool bPrefix = ScDocument::ValidTabName( aBaseName );
                        OSL_ENSURE(bPrefix, "invalid sheet name");

                        while ( pDoc->IsScenario(nTab+i) )
                            i++;

                        bool bValid;
                        SCTAB nDummy;
                        do
                        {
                            aName = aBaseName + OUString::number( i );
                            if (bPrefix)
                                bValid = pDoc->ValidNewTabName( aName );
                            else
                                bValid = !pDoc->GetTable( aName, nDummy );
                            ++i;
                        }
                        while ( !bValid && i <= MAXTAB + 2 );

                        if ( pReqArgs != nullptr )
                        {
                            OUString aArgName;
                            OUString aArgComment;
                            const SfxPoolItem* pItem;
                            if ( pReqArgs->GetItemState( SID_SCENARIOS, true, &pItem ) == SfxItemState::SET )
                                aArgName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                            if ( pReqArgs->GetItemState( SID_NEW_TABLENAME, true, &pItem ) == SfxItemState::SET )
                                aArgComment = static_cast<const SfxStringItem*>(pItem)->GetValue();

                            aColor = Color( COL_LIGHTGRAY );        // Default
                            nFlags = ScScenarioFlags::NONE;         // not TwoWay

                            pTabViewShell->MakeScenario( aArgName, aArgComment, aColor, nFlags );
                            if( ! rReq.IsAPI() )
                                rReq.Done();
                        }
                        else
                        {
                            bool bSheetProtected = pDoc->IsTabProtected(nTab);
                            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                            OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                            std::unique_ptr<AbstractScNewScenarioDlg> pNewDlg(pFact->CreateScNewScenarioDlg(pTabViewShell->GetDialogParent(), aName, false, bSheetProtected));
                            OSL_ENSURE(pNewDlg, "Dialog create fail!");
                            if ( pNewDlg->Execute() == RET_OK )
                            {
                                pNewDlg->GetScenarioData( aName, aComment, aColor, nFlags );
                                pTabViewShell->MakeScenario( aName, aComment, aColor, nFlags );

                                rReq.AppendItem( SfxStringItem( SID_SCENARIOS, aName ) );
                                rReq.AppendItem( SfxStringItem( SID_NEW_TABLENAME, aComment ) );
                                rReq.Done();
                            }
                        }
                    }
                    else if( ! rReq.IsAPI() )
                    {
                        pTabViewShell->ErrorMessage(STR_ERR_NEWSCENARIO);
                    }
                }
            }
            break;

        case SID_SELECTALL:
            {
                pTabViewShell->SelectAll();
                rReq.Done();
            }
            break;

        case FID_ROW_HEIGHT:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = static_cast<const SfxUInt16Item&>(pReqArgs->Get( FID_ROW_HEIGHT ));

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_DIRECT,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    ScViewData* pData      = GetViewData();
                    FieldUnit   eMetric    = SC_MOD()->GetAppOptions().GetAppMetric();
                    sal_uInt16      nCurHeight = pData->GetDocument()->
                                                GetRowHeight( pData->GetCurY(),
                                                              pData->GetTabNo() );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    assert(pFact); //ScAbstractFactory create fail!

                    std::unique_ptr<AbstractScMetricInputDlg> pDlg(pFact->CreateScMetricInputDlg(
                        pTabViewShell->GetDialogParent(), "RowHeightDialog",
                        nCurHeight, ScGlobal::nStdRowHeight,
                        eMetric, 2, MAX_ROW_HEIGHT));
                    assert(pDlg); //Dialog create fail

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_DIRECT, (sal_uInt16)nVal );

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_ROW_HEIGHT, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();

                    }
                }
            }
            break;

        case FID_ROW_OPT_HEIGHT:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = static_cast<const SfxUInt16Item&>(pReqArgs->Get( FID_ROW_OPT_HEIGHT ));

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_OPTIMAL,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
                    ScGlobal::nLastRowHeightExtra = rUInt16Item.GetValue();

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit eMetric = SC_MOD()->GetAppOptions().GetAppMetric();

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    assert(pFact); //ScAbstractFactory create fail!

                    std::unique_ptr<AbstractScMetricInputDlg> pDlg(pFact->CreateScMetricInputDlg(
                        pTabViewShell->GetDialogParent(), "OptimalRowHeightDialog",
                        ScGlobal::nLastRowHeightExtra, 0, eMetric, 1, MAX_EXTRA_HEIGHT));
                    assert(pDlg); //Dialog create fail!

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_OPTIMAL, (sal_uInt16)nVal );
                        ScGlobal::nLastRowHeightExtra = nVal;

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_ROW_OPT_HEIGHT, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();

                    }
                }
            }
            break;

        case FID_COL_WIDTH:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = static_cast<const SfxUInt16Item&>(pReqArgs->Get( FID_COL_WIDTH ));

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_DIRECT,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit   eMetric    = SC_MOD()->GetAppOptions().GetAppMetric();
                    ScViewData* pData      = GetViewData();
                    sal_uInt16      nCurHeight = pData->GetDocument()->
                                                GetColWidth( pData->GetCurX(),
                                                             pData->GetTabNo() );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    assert(pFact); //ScAbstractFactory create fail!

                    std::unique_ptr<AbstractScMetricInputDlg> pDlg(pFact->CreateScMetricInputDlg(
                        pTabViewShell->GetDialogParent(), "ColWidthDialog", nCurHeight,
                        STD_COL_WIDTH, eMetric, 2, MAX_COL_WIDTH));
                    assert(pDlg); //Dialog create fail!

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_DIRECT, (sal_uInt16)nVal );

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_COL_WIDTH, (sal_uInt16)TwipsToEvenHMM(nVal)) );
                        rReq.Done();

                    }
                }
            }
            break;

        case FID_COL_OPT_WIDTH:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = static_cast<const SfxUInt16Item&>(pReqArgs->Get( FID_COL_OPT_WIDTH ));

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_OPTIMAL,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
                    ScGlobal::nLastColWidthExtra = rUInt16Item.GetValue();

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit eMetric = SC_MOD()->GetAppOptions().GetAppMetric();

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    assert(pFact); //ScAbstractFactory create fail!

                    std::unique_ptr<AbstractScMetricInputDlg> pDlg(pFact->CreateScMetricInputDlg(
                        pTabViewShell->GetDialogParent(), "OptimalColWidthDialog",
                        ScGlobal::nLastColWidthExtra, STD_EXTRA_WIDTH, eMetric, 1, MAX_EXTRA_WIDTH));
                    assert(pDlg); //Dialog create fail!
                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_OPTIMAL, (sal_uInt16)nVal );
                        ScGlobal::nLastColWidthExtra = nVal;

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_COL_OPT_WIDTH, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();
                    }
                }
            }
            break;

        case FID_COL_OPT_DIRECT:
            pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH );
            rReq.Done();
            break;

        case FID_ROW_HIDE:
            pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_DIRECT, 0 );
            rReq.Done();
            break;
        case FID_ROW_SHOW:
            pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_SHOW, 0 );
            rReq.Done();
            break;
        case FID_COL_HIDE:
            pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_DIRECT, 0 );
            rReq.Done();
            break;
        case FID_COL_SHOW:
            pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_SHOW, 0 );
            rReq.Done();
            break;

        case SID_CELL_FORMAT_RESET:
            {
                pTabViewShell->DeleteContents( InsertDeleteFlags::HARDATTR | InsertDeleteFlags::EDITATTR );
                rReq.Done();
            }
            break;

        case FID_MERGE_ON:
        case FID_MERGE_OFF:
        case FID_MERGE_TOGGLE:
        {
            if ( !GetViewData()->GetDocument()->GetChangeTrack() )
            {
                // test whether to merge or to split
                bool bMerge = false;
                bool bCenter = false;
                switch( nSlot )
                {
                    case FID_MERGE_ON:
                        bMerge = true;
                    break;
                    case FID_MERGE_OFF:
                        bMerge = false;
                    break;
                    case FID_MERGE_TOGGLE:
                    {
                        bCenter = true;
                        std::unique_ptr<SfxPoolItem> pItem;
                        if( rBindings.QueryState( nSlot, pItem ) >= SfxItemState::DEFAULT )
                            bMerge = !static_cast< SfxBoolItem* >( pItem.get() )->GetValue();
                    }
                    break;
                }

                if( bMerge )
                {
                    // merge - check if to move contents of covered cells
                    bool bMoveContents = false;
                    bool bApi = rReq.IsAPI();
                    const SfxPoolItem* pItem;
                    if ( pReqArgs &&
                        pReqArgs->GetItemState(nSlot, true, &pItem) == SfxItemState::SET )
                    {
                        OSL_ENSURE(pItem && dynamic_cast<const SfxBoolItem*>( pItem) !=  nullptr, "wrong item");
                        bMoveContents = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    }

                    if (pTabViewShell->MergeCells( bApi, bMoveContents, bCenter ))
                    {
                        if (!bApi && bMoveContents)             // "yes" clicked in dialog
                            rReq.AppendItem( SfxBoolItem( nSlot, bMoveContents ) );
                        rBindings.Invalidate( nSlot );
                        rReq.Done();
                    }
                }
                else
                {
                    // split cells
                    if (pTabViewShell->RemoveMerge())
                    {
                        rBindings.Invalidate( nSlot );
                        rReq.Done();
                    }
                }
                break;
            }
        }
        break;

        case SID_AUTOFORMAT:
            {
                vcl::Window* pDlgParent = pTabViewShell->GetDialogParent();
                SCCOL nStartCol;
                SCROW nStartRow;
                SCTAB nStartTab;
                SCCOL nEndCol;
                SCROW nEndRow;
                SCTAB nEndTab;

                const ScMarkData& rMark = GetViewData()->GetMarkData();
                if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
                    pTabViewShell->MarkDataArea();

                GetViewData()->GetSimpleArea( nStartCol,nStartRow,nStartTab,
                                              nEndCol,nEndRow,nEndTab );

                if (   ( std::abs((SCsCOL)nEndCol-(SCsCOL)nStartCol) > 1 )
                    && ( std::abs((SCsROW)nEndRow-(SCsROW)nStartRow) > 1 ) )
                {
                    if ( pReqArgs )
                    {
                        const SfxStringItem& rNameItem = static_cast<const SfxStringItem&>(pReqArgs->Get( SID_AUTOFORMAT ));
                        ScAutoFormat* pFormat = ScGlobal::GetOrCreateAutoFormat();
                        ScAutoFormat::const_iterator it = pFormat->find(rNameItem.GetValue());
                        ScAutoFormat::const_iterator itBeg = pFormat->begin();
                        size_t nIndex = std::distance(itBeg, it);

                        pTabViewShell->AutoFormat( nIndex );

                        if( ! rReq.IsAPI() )
                            rReq.Done();
                    }
                    else
                    {
                        ScGlobal::ClearAutoFormat();
                        std::unique_ptr<ScAutoFormatData> pNewEntry(pTabViewShell->CreateAutoFormatData());
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        std::unique_ptr<AbstractScAutoFormatDlg> pDlg(pFact->CreateScAutoFormatDlg(pDlgParent, ScGlobal::GetOrCreateAutoFormat(), pNewEntry.get(), GetViewData()));
                        OSL_ENSURE(pDlg, "Dialog create fail!");

                        if ( pDlg->Execute() == RET_OK )
                        {
                            ScEditableTester aTester( pTabViewShell );
                            if ( !aTester.IsEditable() )
                            {
                                pTabViewShell->ErrorMessage(aTester.GetMessageId());
                            }
                            else
                            {
                                pTabViewShell->AutoFormat( pDlg->GetIndex() );

                                rReq.AppendItem( SfxStringItem( SID_AUTOFORMAT, pDlg->GetCurrFormatName() ) );
                                rReq.Done();
                            }
                        }
                    }
                }
                else
                    ScopedVclPtrInstance<MessageDialog>(pDlgParent,
                              ScGlobal::GetRscString(STR_INVALID_AFAREA) )->Execute();
            }
            break;

        case SID_CANCEL:
            {
                if (GetViewData()->HasEditView(GetViewData()->GetActivePart()))
                    pScMod->InputCancelHandler();
                else if (pTabViewShell->HasPaintBrush())
                    pTabViewShell->ResetBrushDocument();            // abort format paint brush
                else if (pTabViewShell->HasHintWindow())
                    pTabViewShell->RemoveHintWindow();
                else if( ScViewUtil::IsFullScreen( *pTabViewShell ) )
                    ScViewUtil::SetFullScreen( *pTabViewShell, false );
                else
                {
                    // TODO/LATER: when is this code executed?
                    pTabViewShell->Escape();
                }
            }
            break;

        case SID_DATA_SELECT:
            pTabViewShell->StartDataSelect();
            break;

        case SID_DETECTIVE_FILLMODE:
            {
                bool bOldMode = pTabViewShell->IsAuditShell();
                pTabViewShell->SetAuditShell( !bOldMode );
                pTabViewShell->Invalidate( nSlot );
            }
            break;

        case FID_INPUTLINE_STATUS:
            OSL_FAIL("Execute from InputLine status");
            break;

        case SID_STATUS_DOCPOS:
            // Launch navigator.
            GetViewData()->GetDispatcher().Execute(
                SID_NAVIGATOR, SfxCallMode::SYNCHRON|SfxCallMode::RECORD );
            break;

        case SID_MARKAREA:
            // called from Basic at the hidden view to select a range in the visible view
            OSL_FAIL("old slot SID_MARKAREA");
            break;

        default:
            OSL_FAIL("ScCellShell::Execute: unknown slot");
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
