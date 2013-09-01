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

#define IS_EDITMODE() GetViewData()->HasEditView( GetViewData()->GetActivePart() )

using sc::HMMToTwips;
using sc::TwipsToHMM;
using sc::TwipsToEvenHMM;

//------------------------------------------------------------------

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
                    //! cannot always be switched werden ????
                    if (!pTabViewShell->GetViewFrame()->GetFrame().IsInPlace())
                        pTabViewShell->SetDontSwitch(sal_True);         // do not switch off EditShell
                    // no break

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
        case SID_ATTR_SIZE://XXX ???
            break;

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
                sal_uInt16 nMode = ((const SfxUInt16Item&)pReqArgs->Get( nSlot )).GetValue();

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
                    String aStr( ((const SfxStringItem&)pReqArgs->
                                    Get( SID_ENTER_STRING )).GetValue() );

                    pTabViewShell->EnterData( GetViewData()->GetCurX(),
                                               GetViewData()->GetCurY(),
                                               GetViewData()->GetTabNo(),
                                               aStr );

                    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( pTabViewShell );
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
                    String aStr = ((const SfxStringItem&)pReqArgs->
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
                if( pReqArgs == 0 ) //XXX temporary HACK to avoid GPF
                    break;

                const ScInputStatusItem* pStatusItem
                    = (const ScInputStatusItem*)&pReqArgs->
                            Get( FID_INPUTLINE_STATUS );

                ScAddress aCursorPos = pStatusItem->GetPos();
                String aString = pStatusItem->GetString();
                const EditTextObject* pData = pStatusItem->GetEditData();

                if (pData)
                {
                    if (nSlot == FID_INPUTLINE_BLOCK)
                    {
                        pTabViewShell->EnterBlock( aString, pData );
                    }
                    else if ( aString.Len() > 0 && ( aString.GetChar(0) == '=' || aString.GetChar(0) == '+' || aString.GetChar(0) == '-' ) )
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

                            // SfxBindings& rBindings = pTabViewShell->GetViewFrame()->GetBindings();
                            const SfxPoolItem* aArgs[2];
                            aArgs[0] = &aItem;
                            aArgs[1] = NULL;
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
                        pTabViewShell->EnterBlock( aString, NULL );
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

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
                rReq.Ignore();
            }
            break;

        case SID_OPENDLG_CONSOLIDATE:
            {
                sal_uInt16          nId  = ScConsolidateDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;

        case FID_CELL_FORMAT:
            {
                if ( pReqArgs != NULL )
                {
                    //----------------------------------
                    // set cell attribute without dialog:
                    //----------------------------------
                    SfxItemSet*     pEmptySet =
                                        new SfxItemSet( *pReqArgs->GetPool(),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END );

                    SfxItemSet*     pNewSet =
                                        new SfxItemSet( *pReqArgs->GetPool(),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END );

                    const SfxPoolItem*  pAttr = NULL;
                    sal_uInt16              nWhich = 0;

                    for ( nWhich=ATTR_PATTERN_START; nWhich<=ATTR_PATTERN_END; nWhich++ )
                        if ( pReqArgs->GetItemState( nWhich, sal_True, &pAttr ) == SFX_ITEM_SET )
                            pNewSet->Put( *pAttr );

                    pTabViewShell->ApplyAttributes( pNewSet, pEmptySet );

                    delete pNewSet;
                    delete pEmptySet;

                    rReq.Done();
                }
                else if ( pReqArgs == NULL )
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

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;

        case SID_OPENDLG_OPTSOLVER:
            {
                sal_uInt16 nId = ScOptSolverDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
            }
            break;

        case SID_OPENDLG_TABOP:
            {
                sal_uInt16          nId  = ScTabOpDlgWrapper::GetChildWindowId();
                SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd ? false : sal_True );
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
                               QueryBox( pTabViewShell->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                         ScGlobal::GetRscString(STR_UPDATE_SCENARIO) ).
                                        Execute() )
                        {
                            pTabViewShell->ExtendScenario();
                            rReq.Done();
                        }
                    }
                    else if( ! rReq.IsAPI() )
                    {
                        ErrorBox aErrorBox( pTabViewShell->GetDialogParent(), WinBits(WB_OK | WB_DEF_OK),
                                            ScGlobal::GetRscString(STR_NOAREASELECTED) );
                        aErrorBox.Execute();
                    }
                }
                else
                {
                    rMark.MarkToMulti();
                    if ( rMark.IsMultiMarked() )
                    {
                        SCTAB i=1;
                        String aBaseName;
                        OUString aName;
                        OUString aComment;
                        Color  aColor;
                        sal_uInt16 nFlags;

                        OUString aTmp;
                        pDoc->GetName(nTab, aTmp);
                        aBaseName = aTmp;
                        aBaseName += '_';
                        aBaseName += ScGlobal::GetRscString(STR_SCENARIO);
                        aBaseName += '_';

                        //  first test, if the prefix is recognised as valid,
                        //  else avoid only doubles
                        sal_Bool bPrefix = pDoc->ValidTabName( aBaseName );
                        OSL_ENSURE(bPrefix, "ungueltiger Tabellenname");

                        while ( pDoc->IsScenario(nTab+i) )
                            i++;

                        sal_Bool bValid;
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

                        if ( pReqArgs != NULL )
                        {
                            String aArgName;
                            String aArgComment;
                            const SfxPoolItem* pItem;
                            if ( pReqArgs->GetItemState( SID_SCENARIOS, sal_True, &pItem ) == SFX_ITEM_SET )
                                aArgName = ((const SfxStringItem*)pItem)->GetValue();
                            if ( pReqArgs->GetItemState( SID_NEW_TABLENAME, sal_True, &pItem ) == SFX_ITEM_SET )
                                aArgComment = ((const SfxStringItem*)pItem)->GetValue();

                            aColor = Color( COL_LIGHTGRAY );        // Default
                            nFlags = 0;                             // not-TwoWay

                            pTabViewShell->MakeScenario( aArgName, aArgComment, aColor, nFlags );
                            if( ! rReq.IsAPI() )
                                rReq.Done();
                        }
                        else
                        {
                            sal_Bool bSheetProtected = pDoc->IsTabProtected(nTab);
                            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                            OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                            AbstractScNewScenarioDlg* pNewDlg = pFact->CreateScNewScenarioDlg( pTabViewShell->GetDialogParent(), aName, RID_SCDLG_NEWSCENARIO, false,bSheetProtected);
                            OSL_ENSURE(pNewDlg, "Dialog create fail!");
                            if ( pNewDlg->Execute() == RET_OK )
                            {
                                pNewDlg->GetScenarioData( aName, aComment, aColor, nFlags );
                                pTabViewShell->MakeScenario( aName, aComment, aColor, nFlags );

                                rReq.AppendItem( SfxStringItem( SID_SCENARIOS, aName ) );
                                rReq.AppendItem( SfxStringItem( SID_NEW_TABLENAME, aComment ) );
                                rReq.Done();
                            }
                            delete pNewDlg;
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

        //----------------------------------------------------------------

        case FID_ROW_HEIGHT:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = (const SfxUInt16Item&)pReqArgs->Get( FID_ROW_HEIGHT );

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
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    AbstractScMetricInputDlg* pDlg = pFact->CreateScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_ROW_MAN,
                                                                                    nCurHeight,
                                                                                    ScGlobal::nStdRowHeight,
                                                                                    RID_SCDLG_ROW_MAN,
                                                                                    eMetric,
                                                                                    2,
                                                                                    MAX_ROW_HEIGHT);
                    OSL_ENSURE(pDlg, "Dialog create fail!");

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_DIRECT, (sal_uInt16)nVal );

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_ROW_HEIGHT, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();

                    }
                    delete pDlg;
                }
            }
            break;

        case FID_ROW_OPT_HEIGHT:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = (const SfxUInt16Item&)pReqArgs->Get( FID_ROW_OPT_HEIGHT );

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
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    AbstractScMetricInputDlg* pDlg = pFact->CreateScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_ROW_OPT,
                                                                                    ScGlobal::nLastRowHeightExtra,
                                                                                    0,
                                                                                    RID_SCDLG_ROW_OPT,
                                                                                    eMetric,
                                                                                    1,
                                                                                    MAX_EXTRA_HEIGHT);
                    OSL_ENSURE(pDlg, "Dialog create fail!");

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_OPTIMAL, (sal_uInt16)nVal );
                        ScGlobal::nLastRowHeightExtra = nVal;

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_ROW_OPT_HEIGHT, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();

                    }
                    delete pDlg;
                }
            }
            break;

        case FID_COL_WIDTH:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = (const SfxUInt16Item&)pReqArgs->Get( FID_COL_WIDTH );

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_DIRECT,
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
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    AbstractScMetricInputDlg* pDlg = pFact->CreateScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_COL_MAN,
                                                                                    nCurHeight,
                                                                                    STD_COL_WIDTH,
                                                                                    RID_SCDLG_COL_MAN,
                                                                                    eMetric,
                                                                                    2,
                                                                                    MAX_COL_WIDTH);
                    OSL_ENSURE(pDlg, "Dialog create fail!");

                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_DIRECT, (sal_uInt16)nVal );

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_COL_WIDTH, (sal_uInt16)TwipsToEvenHMM(nVal)) );
                        rReq.Done();

                    }
                    delete pDlg;
                }
            }
            break;

        case FID_COL_OPT_WIDTH:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = (const SfxUInt16Item&)pReqArgs->Get( FID_COL_OPT_WIDTH );

                    // #101390#; the value of the macro is in HMM so use HMMToTwips to convert
                    pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_OPTIMAL,
                                    sal::static_int_cast<sal_uInt16>( HMMToTwips(rUInt16Item.GetValue()) ) );
                    ScGlobal::nLastColWidthExtra = rUInt16Item.GetValue();

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit eMetric = SC_MOD()->GetAppOptions().GetAppMetric();

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                    AbstractScMetricInputDlg* pDlg = pFact->CreateScMetricInputDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_COL_OPT,
                                                                                    ScGlobal::nLastColWidthExtra,
                                                                                    STD_EXTRA_WIDTH,
                                                                                    RID_SCDLG_COL_OPT,
                                                                                    eMetric,
                                                                                    1,
                                                                                    MAX_EXTRA_WIDTH);
                    OSL_ENSURE(pDlg, "Dialog create fail!");
                    if ( pDlg->Execute() == RET_OK )
                    {
                        long nVal = pDlg->GetInputValue();
                        pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_OPTIMAL, (sal_uInt16)nVal );
                        ScGlobal::nLastColWidthExtra = nVal;

                        // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                        rReq.AppendItem( SfxUInt16Item( FID_COL_OPT_WIDTH, (sal_uInt16)TwipsToEvenHMM(nVal) ) );
                        rReq.Done();
                    }
                    delete pDlg;
                }
            }
            break;

        case FID_COL_OPT_DIRECT:
            pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH );
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
            pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_DIRECT, 0 );
            rReq.Done();
            break;
        case FID_COL_SHOW:
            pTabViewShell->SetMarkedWidthOrHeight( sal_True, SC_SIZE_SHOW, 0 );
            rReq.Done();
            break;

        //----------------------------------------------------------------


        case SID_CELL_FORMAT_RESET:
            {
                pTabViewShell->DeleteContents( IDF_HARDATTR | IDF_EDITATTR );
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
                sal_Bool bCenter = false;
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
                        SfxPoolItem* pItem = 0;
                        if( rBindings.QueryState( nSlot, pItem ) >= SFX_ITEM_DEFAULT )
                            bMerge = !static_cast< SfxBoolItem* >( pItem )->GetValue();

                        delete pItem;
                    }
                    break;
                }

                if( bMerge )
                {
                    // merge - check if to move contents of covered cells
                    sal_Bool bMoveContents = false;
                    sal_Bool bApi = rReq.IsAPI();
                    const SfxPoolItem* pItem;
                    if ( pReqArgs &&
                        pReqArgs->GetItemState(nSlot, sal_True, &pItem) == SFX_ITEM_SET )
                    {
                        OSL_ENSURE(pItem && pItem->ISA(SfxBoolItem), "falsches Item");
                        bMoveContents = ((const SfxBoolItem*)pItem)->GetValue();
                    }

                    if (pTabViewShell->MergeCells( bApi, bMoveContents, true, bCenter ))
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
                Window* pDlgParent = pTabViewShell->GetDialogParent();
                SCCOL nStartCol;
                SCROW nStartRow;
                SCTAB nStartTab;
                SCCOL nEndCol;
                SCROW nEndRow;
                SCTAB nEndTab;

                const ScMarkData& rMark = GetViewData()->GetMarkData();
                if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
                    pTabViewShell->MarkDataArea( sal_True );

                GetViewData()->GetSimpleArea( nStartCol,nStartRow,nStartTab,
                                              nEndCol,nEndRow,nEndTab );

                if (   ( std::abs((SCsCOL)nEndCol-(SCsCOL)nStartCol) > 1 )
                    && ( std::abs((SCsROW)nEndRow-(SCsROW)nStartRow) > 1 ) )
                {
                    if ( pReqArgs )
                    {
                        const SfxStringItem& rNameItem = (const SfxStringItem&)pReqArgs->Get( SID_AUTOFORMAT );
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
                        ScAutoFormatData* pNewEntry = pTabViewShell->CreateAutoFormatData();
                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                        AbstractScAutoFormatDlg* pDlg = pFact->CreateScAutoFormatDlg( pDlgParent, ScGlobal::GetOrCreateAutoFormat(), pNewEntry,GetViewData()->GetDocument(), RID_SCDLG_AUTOFORMAT );
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
                        delete pDlg;
                        delete pNewEntry;
                    }
                }
                else
                    ErrorBox( pDlgParent, WinBits( WB_OK | WB_DEF_OK ),
                              ScGlobal::GetRscString(STR_INVALID_AFAREA) ).Execute();
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
                sal_Bool bOldMode = pTabViewShell->IsAuditShell();
                pTabViewShell->SetAuditShell( !bOldMode );
                pTabViewShell->Invalidate( nSlot );
            }
            break;

        //  ----------------------------------------------------------------

        case FID_INPUTLINE_STATUS:
            OSL_FAIL("Execute von InputLine-Status");
            break;

        case SID_STATUS_DOCPOS:
            // Launch navigator.
            GetViewData()->GetDispatcher().Execute(
                SID_NAVIGATOR, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
            break;

        case SID_MARKAREA:
            // called from Basic at the hidden view to select a range in the visible view
            OSL_FAIL("old slot SID_MARKAREA");
            break;

        default:
            OSL_FAIL("Unbekannter Slot bei ScCellShell::Execute");
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
