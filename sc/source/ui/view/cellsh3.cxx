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

#include <scitems.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <formula/formulahelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scmod.hxx>
#include <appoptio.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <sc.hrc>
#include <reffact.hxx>
#include <uiitems.hxx>
#include <autoform.hxx>
#include <cellsh.hxx>
#include <inputhdl.hxx>
#include <inputopt.hxx>
#include <editable.hxx>
#include <funcdesc.hxx>
#include <markdata.hxx>
#include <scabstdlg.hxx>
#include <columnspanset.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <inputwin.hxx>

#include <memory>

using sc::TwipsToEvenHMM;

namespace
{
/// Rid ourselves of unwanted " quoted json characters.
OString escapeJSON(const OUString &aStr)
{
    OUString aEscaped = aStr;
    aEscaped = aEscaped.replaceAll("\n", " ");
    aEscaped = aEscaped.replaceAll("\"", "'");
    return OUStringToOString(aEscaped, RTL_TEXTENCODING_UTF8);
}

void lcl_lokGetWholeFunctionList()
{
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    if (!(comphelper::LibreOfficeKit::isActive()
        && pViewShell && pViewShell->isLOKMobilePhone()))
        return;

    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    sal_uInt32 nListCount = pFuncList->GetCount();
    std::set<OUString> aFuncNameOrderedSet;
    for(sal_uInt32 i = 0; i < nListCount; ++i)
    {
        const ScFuncDesc* pDesc = pFuncList->GetFunction( i );
        if ( pDesc->mxFuncName )
        {
            aFuncNameOrderedSet.insert(*pDesc->mxFuncName);
        }
    }
    ScFunctionMgr* pFuncManager = ScGlobal::GetStarCalcFunctionMgr();
    if (!(pFuncManager && aFuncNameOrderedSet.size()))
        return;

    OStringBuffer aPayload(
        "{ \"wholeList\": true, "
        "\"categories\": [ ");

    formula::FormulaHelper aHelper(pFuncManager);
    sal_uInt32 nCategoryCount = pFuncManager->getCount();
    for (sal_uInt32 i = 0; i < nCategoryCount; ++i)
    {
        OUString sCategoryName = ScFunctionMgr::GetCategoryName(i);
        aPayload.append("{"
            "\"name\": \""
            + escapeJSON(sCategoryName)
            + "\"}, ");
    }
    sal_Int32 nLen = aPayload.getLength();
    aPayload[nLen - 2] = ' ';
    aPayload[nLen - 1] = ']';
    aPayload.append(", ");

    OUString aDescFuncNameStr;
    aPayload.append("\"functions\": [ ");
    sal_uInt32 nCurIndex = 0;
    for (const OUString& aFuncNameStr : aFuncNameOrderedSet)
    {
        aDescFuncNameStr = aFuncNameStr + "()";
        sal_Int32 nNextFStart = 0;
        const formula::IFunctionDescription* ppFDesc;
        ::std::vector< OUString > aArgs;
        OUString eqPlusFuncName = "=" + aDescFuncNameStr;
        if ( aHelper.GetNextFunc( eqPlusFuncName, false, nNextFStart, nullptr, &ppFDesc, &aArgs ) )
        {
            if ( ppFDesc && !ppFDesc->getFunctionName().isEmpty() )
            {
                if (ppFDesc->getCategory())
                {
                    aPayload.append("{"
                        "\"index\": "
                        + OString::number(static_cast<sal_Int64>(nCurIndex))
                        + ", "
                        "\"category\": "
                        + OString::number(static_cast<sal_Int64>(ppFDesc->getCategory()->getNumber()))
                        + ", "
                        "\"signature\": \""
                        + escapeJSON(ppFDesc->getSignature())
                        + "\", "
                        "\"description\": \""
                        + escapeJSON(ppFDesc->getDescription())
                        + "\"}, ");
                }
            }
        }
        ++nCurIndex;
    }
    nLen = aPayload.getLength();
    aPayload[nLen - 2] = ' ';
    aPayload[nLen - 1] = ']';
    aPayload.append(" }");

    OString s = aPayload.makeStringAndClear();
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CALC_FUNCTION_LIST, s);
}

} // end namespace

void ScCellShell::Execute( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    SfxBindings&        rBindings   = pTabViewShell->GetViewFrame().GetBindings();
    ScModule*           pScMod      = SC_MOD();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    sal_uInt16              nSlot       = rReq.GetSlot();

    if (nSlot != SID_CURRENTCELL)       // this comes with MouseButtonUp
        pTabViewShell->HideListBox();   // Autofilter-DropDown-Listbox

    if ( GetViewData().HasEditView( GetViewData().GetActivePart() ) )
    {
        switch ( nSlot )
        {
            //  when opening a reference-dialog the subshell may not be switched
            //  (on closing the dialog StopEditShell is called)
            case SID_OPENDLG_FUNCTION:
                    //  inplace leads to trouble with EditShell ...
                    //! cannot always be switched ????
                    if (!pTabViewShell->GetViewFrame().GetFrame().IsInPlace())
                        pTabViewShell->SetDontSwitch(true);         // do not switch off EditShell
                    [[fallthrough]];

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
                    // In the LOK case, we want to set the document modified state
                    // right away at the start of the edit, so that the content is
                    // saved even when the user leaves the document before hitting
                    // Enter
                    // NOTE: This also means we want to set the modified state
                    // regardless of the DontCommit parameter's value.
                    if (comphelper::LibreOfficeKit::isActive() && !GetViewData().GetDocShell()->IsModified())
                    {
                        GetViewData().GetDocShell()->SetModified();
                        rBindings.Invalidate(SID_SAVEDOC);
                        rBindings.Invalidate(SID_DOC_MODIFIED);
                    }

                    OUString aStr( pReqArgs->Get( SID_ENTER_STRING ).GetValue() );
                    const SfxPoolItem* pDontCommitItem;
                    bool bCommit = true;
                    if (pReqArgs->HasItem(FN_PARAM_1, &pDontCommitItem))
                        bCommit = !(static_cast<const SfxBoolItem*>(pDontCommitItem)->GetValue());

                    ScInputHandler* pHdl = SC_MOD()->GetInputHdl( pTabViewShell );
                    if (bCommit)
                    {
                        pTabViewShell->EnterData( GetViewData().GetCurX(),
                                                  GetViewData().GetCurY(),
                                                  GetViewData().GetTabNo(),
                                                  aStr, nullptr,
                                                  true /*bMatrixExpand*/);
                    }
                    else if (pHdl)
                    {
                        SC_MOD()->SetInputMode(SC_INPUT_TABLE);

                        EditView* pTableView = pHdl->GetActiveView();
                        pHdl->DataChanging();
                        if (pTableView)
                            pTableView->getEditEngine().SetText(aStr);
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
                    ScDocument& rDoc = GetViewData().GetDocument();
                    pTabViewShell->EnterMatrix( aStr, rDoc.GetGrammar() );
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

                const ScAddress& aCursorPos = pStatusItem->GetPos();
                const OUString& aString = pStatusItem->GetString();
                const EditTextObject* pData = pStatusItem->GetEditData();

                if (pData)
                {
                    if (nSlot == FID_INPUTLINE_BLOCK)
                    {
                        pTabViewShell->EnterBlock( aString, pData );
                    }
                    else if ( !aString.isEmpty() && ( aString[0] == '=' || aString[0] == '+' || aString[0] == '-' ) )
                    {
                        pTabViewShell->EnterData( aCursorPos.Col(), aCursorPos.Row(), aCursorPos.Tab(),
                                aString, pData, true /*bMatrixExpand*/);
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
                            aCursorPos.Col() == GetViewData().GetCurX() &&
                            aCursorPos.Row() == GetViewData().GetCurY() &&
                            aCursorPos.Tab() == GetViewData().GetTabNo()
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
                                                    aString, nullptr,
                                                    true /*bMatrixExpand*/);
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
                        ScDocument& rDoc = GetViewData().GetDocument();
                        pTabViewShell->EnterMatrix( aString, rDoc.GetGrammar() );
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
                const SfxViewShell* pViewShell = SfxViewShell::Current();
                if (comphelper::LibreOfficeKit::isActive()
                    && pViewShell && pViewShell->isLOKMobilePhone())
                {
                    // not set the dialog id in the mobile case or we would
                    // not be able to get cell address pasted in the edit view
                    // by just tapping on them
                    lcl_lokGetWholeFunctionList();
                }
                else
                {
                    sal_uInt16 nId = SID_OPENDLG_FUNCTION;
                    SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );
                    bool bVis = comphelper::LibreOfficeKit::isActive() || pWnd == nullptr;
                    pScMod->SetRefDialog( nId, bVis );
                }
                rReq.Ignore();
            }
            break;

        case SID_OPENDLG_CONSOLIDATE:
            {
                sal_uInt16          nId  = ScConsolidateDlgWrapper::GetChildWindowId();
                SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_EASY_CONDITIONAL_FORMAT_DIALOG:
            {
                if (pReqArgs != nullptr)
                {
                    const SfxPoolItem* pFormat;
                    if (pReqArgs->HasItem( FN_PARAM_1, &pFormat))
                    {
                        sal_Int16 nFormat = static_cast<const SfxInt16Item*>(pFormat)->GetValue();
                        sal_uInt16 nId = sc::ConditionalFormatEasyDialogWrapper::GetChildWindowId();
                        SfxViewFrame& rViewFrame = pTabViewShell->GetViewFrame();
                        SfxChildWindow* pWindow = rViewFrame.GetChildWindow( nId );
                        GetViewData().GetDocument().SetEasyConditionalFormatDialogData(std::make_unique<ScConditionMode>(static_cast<ScConditionMode>(nFormat)));

                        pScMod->SetRefDialog( nId, pWindow == nullptr );
                    }
                }
            }
            break;

        case FID_CELL_FORMAT:
            {
                if ( pReqArgs != nullptr )
                {

                    // set cell attribute without dialog:

                    SfxItemSetFixed<ATTR_PATTERN_START, ATTR_PATTERN_END>  aEmptySet( *pReqArgs->GetPool() );

                    SfxItemSetFixed<ATTR_PATTERN_START, ATTR_PATTERN_END>  aNewSet( *pReqArgs->GetPool() );

                    const SfxPoolItem*  pAttr = nullptr;
                    sal_uInt16              nWhich = 0;

                    for ( nWhich=ATTR_PATTERN_START; nWhich<=ATTR_PATTERN_END; nWhich++ )
                        if ( pReqArgs->GetItemState( nWhich, true, &pAttr ) == SfxItemState::SET )
                            aNewSet.Put( *pAttr );

                    pTabViewShell->ApplyAttributes( aNewSet, aEmptySet );

                    rReq.Done();
                }
                else
                {
                    pTabViewShell->ExecuteCellFormatDlg( rReq, u""_ustr );
                }
            }
            break;

        case SID_ENABLE_HYPHENATION:
            pTabViewShell->ExecuteCellFormatDlg(rReq, u"alignment"_ustr);
            break;

        case SID_PROPERTY_PANEL_CELLTEXT_DLG:
            pTabViewShell->ExecuteCellFormatDlg( rReq, u"font"_ustr );
            break;

        case SID_CELL_FORMAT_BORDER:
            pTabViewShell->ExecuteCellFormatDlg( rReq, u"borders"_ustr );
            break;

        case SID_CHAR_DLG_EFFECT:
            pTabViewShell->ExecuteCellFormatDlg( rReq, u"fonteffects"_ustr );
            break;

        case SID_OPENDLG_SOLVE:
            {
                sal_uInt16          nId  = ScSolverDlgWrapper::GetChildWindowId();
                SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_OPENDLG_OPTSOLVER:
            {
                sal_uInt16 nId = ScOptSolverDlgWrapper::GetChildWindowId();
                SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_OPENDLG_TABOP:
            {
                sal_uInt16          nId  = ScTabOpDlgWrapper::GetChildWindowId();
                SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );
            }
            break;

        case SID_SCENARIOS:
            {
                ScDocument& rDoc = GetViewData().GetDocument();
                ScMarkData& rMark = GetViewData().GetMarkData();
                SCTAB nTab = GetViewData().GetTabNo();

                if ( rDoc.IsScenario(nTab) )
                {
                    rMark.MarkToMulti();
                    if ( rMark.IsMultiMarked() )
                    {

                        bool bExtend = rReq.IsAPI();
                        if (!bExtend)
                        {
                            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pTabViewShell->GetFrameWeld(),
                                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                                           ScResId(STR_UPDATE_SCENARIO)));
                            xQueryBox->set_default_response(RET_YES);
                            bExtend = xQueryBox->run() == RET_YES;
                        }

                        if (bExtend)
                        {
                            pTabViewShell->ExtendScenario();
                            rReq.Done();
                        }
                    }
                    else if( ! rReq.IsAPI() )
                    {
                        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pTabViewShell->GetFrameWeld(),
                                                                       VclMessageType::Warning, VclButtonsType::Ok,
                                                                       ScResId(STR_NOAREASELECTED)));
                        xErrorBox->run();
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
                        Color  aColor;
                        ScScenarioFlags nFlags;

                        OUString aTmp;
                        rDoc.GetName(nTab, aTmp);
                        aBaseName = aTmp + "_" + ScResId(STR_SCENARIO) + "_";

                        //  first test, if the prefix is recognised as valid,
                        //  else avoid only doubles
                        bool bPrefix = ScDocument::ValidTabName( aBaseName );
                        OSL_ENSURE(bPrefix, "invalid sheet name");

                        while ( rDoc.IsScenario(nTab+i) )
                            i++;

                        bool bValid;
                        SCTAB nDummy;
                        do
                        {
                            aName = aBaseName + OUString::number( i );
                            if (bPrefix)
                                bValid = rDoc.ValidNewTabName( aName );
                            else
                                bValid = !rDoc.GetTable( aName, nDummy );
                            ++i;
                        }
                        while ( !bValid && i <= MAXTAB + 2 );

                        if ( pReqArgs != nullptr )
                        {
                            OUString aArgName;
                            OUString aArgComment;
                            if ( const SfxStringItem* pItem = pReqArgs->GetItemIfSet( SID_SCENARIOS ) )
                                aArgName = pItem->GetValue();
                            if ( const SfxStringItem* pItem = pReqArgs->GetItemIfSet( SID_NEW_TABLENAME ) )
                                aArgComment = pItem->GetValue();

                            aColor = COL_LIGHTGRAY;        // Default
                            nFlags = ScScenarioFlags::NONE;         // not TwoWay

                            pTabViewShell->MakeScenario( aArgName, aArgComment, aColor, nFlags );
                            if( ! rReq.IsAPI() )
                                rReq.Done();
                        }
                        else
                        {
                            bool bSheetProtected = rDoc.IsTabProtected(nTab);
                            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                            ScopedVclPtr<AbstractScNewScenarioDlg> pNewDlg(pFact->CreateScNewScenarioDlg(pTabViewShell->GetFrameWeld(), aName, false, bSheetProtected));
                            if ( pNewDlg->Execute() == RET_OK )
                            {
                                OUString aComment;
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
                const SfxPoolItem* pRow;
                const SfxUInt16Item* pHeight;
                sal_uInt16 nHeight;

                if ( pReqArgs && (pHeight = pReqArgs->GetItemIfSet( FID_ROW_HEIGHT )) &&
                                 pReqArgs->HasItem( FN_PARAM_1, &pRow ) )
                {
                    std::vector<sc::ColRowSpan> aRanges;
                    SCCOLROW nRow = static_cast<const SfxInt32Item*>(pRow)->GetValue() - 1;
                    nHeight = pHeight->GetValue();
                    ScMarkData& rMark = GetViewData().GetMarkData();

                    if ( rMark.IsRowMarked( static_cast<SCROW>(nRow) ) )
                    {
                        aRanges = rMark.GetMarkedRowSpans();
                    }
                    else
                    {
                        aRanges.emplace_back(nRow, nRow);
                    }

                    pTabViewShell->SetWidthOrHeight(false, aRanges, SC_SIZE_DIRECT, o3tl::toTwips(nHeight, o3tl::Length::mm100));
                }
                else if ( pReqArgs && (pHeight = pReqArgs->GetItemIfSet( FID_ROW_HEIGHT )) )
                {
                    nHeight = pHeight->GetValue();

                    // #101390#; the value of the macro is in HMM so use convertMm100ToTwip to convert
                    pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_DIRECT,
                        o3tl::toTwips(nHeight, o3tl::Length::mm100));
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    ScViewData& rData      = GetViewData();
                    FieldUnit   eMetric    = SC_MOD()->GetAppOptions().GetAppMetric();
                    sal_uInt16      nCurHeight = rData.GetDocument().
                                                GetRowHeight( rData.GetCurY(),
                                                              rData.GetTabNo() );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    VclPtr<AbstractScMetricInputDlg> pDlg(pFact->CreateScMetricInputDlg(
                        pTabViewShell->GetFrameWeld(), u"RowHeightDialog"_ustr, nCurHeight,
                        rData.GetDocument().GetSheetOptimalMinRowHeight(rData.GetTabNo()),
                        eMetric, 2, MAX_ROW_HEIGHT));

                    pDlg->StartExecuteAsync([pDlg, pTabViewShell](sal_Int32 nResult){
                        if (nResult == RET_OK)
                        {
                            SfxRequest pRequest(pTabViewShell->GetViewFrame(), FID_ROW_HEIGHT);
                            tools::Long nVal = pDlg->GetInputValue();
                            pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_DIRECT, static_cast<sal_uInt16>(nVal) );

                            // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                            pRequest.AppendItem( SfxUInt16Item( FID_ROW_HEIGHT, static_cast<sal_uInt16>(TwipsToEvenHMM(nVal)) ) );
                            pRequest.Done();
                        }
                        pDlg->disposeOnce();
                    });
                }
            }
            break;

        case FID_ROW_OPT_HEIGHT:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = pReqArgs->Get( FID_ROW_OPT_HEIGHT );

                    // #101390#; the value of the macro is in HMM so use convertMm100ToTwip to convert
                    pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_OPTIMAL,
                                    o3tl::toTwips(rUInt16Item.GetValue(), o3tl::Length::mm100) );
                    ScGlobal::nLastRowHeightExtra = rUInt16Item.GetValue();

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit eMetric = SC_MOD()->GetAppOptions().GetAppMetric();

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    VclPtr<AbstractScMetricInputDlg> pDlg(pFact->CreateScMetricInputDlg(
                        pTabViewShell->GetFrameWeld(), u"OptimalRowHeightDialog"_ustr,
                        ScGlobal::nLastRowHeightExtra, 0, eMetric, 2, MAX_EXTRA_HEIGHT));

                    pDlg->StartExecuteAsync([pDlg, pTabViewShell](sal_Int32 nResult){
                        if ( nResult == RET_OK )
                        {
                            SfxRequest pRequest(pTabViewShell->GetViewFrame(), FID_ROW_OPT_HEIGHT);
                            tools::Long nVal = pDlg->GetInputValue();
                            pTabViewShell->SetMarkedWidthOrHeight( false, SC_SIZE_OPTIMAL, static_cast<sal_uInt16>(nVal) );
                            ScGlobal::nLastRowHeightExtra = nVal;

                            // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                            pRequest.AppendItem( SfxUInt16Item( FID_ROW_OPT_HEIGHT, static_cast<sal_uInt16>(TwipsToEvenHMM(nVal)) ) );
                            pRequest.Done();
                        }
                        pDlg->disposeOnce();
                    });
                }
            }
            break;

        case FID_COL_WIDTH:
            {
                const SfxPoolItem* pColumn;
                const SfxUInt16Item* pWidth;
                sal_uInt16 nWidth;

                if ( pReqArgs && (pWidth = pReqArgs->GetItemIfSet( FID_COL_WIDTH )) &&
                                 pReqArgs->HasItem( FN_PARAM_1, &pColumn ) )
                {
                    std::vector<sc::ColRowSpan> aRanges;
                    SCCOLROW nColumn = static_cast<const SfxUInt16Item*>(pColumn)->GetValue() - 1;
                    nWidth = pWidth->GetValue();
                    ScMarkData& rMark = GetViewData().GetMarkData();

                    if ( rMark.IsColumnMarked( static_cast<SCCOL>(nColumn) ) )
                    {
                        aRanges = rMark.GetMarkedColSpans();
                    }
                    else
                    {
                        aRanges.emplace_back(nColumn, nColumn);
                    }

                    pTabViewShell->SetWidthOrHeight(true, aRanges, SC_SIZE_DIRECT, o3tl::toTwips(nWidth, o3tl::Length::mm100));
                }
                else if ( pReqArgs && (pWidth = pReqArgs->GetItemIfSet( FID_COL_WIDTH )) )
                {
                    nWidth = pWidth->GetValue();

                    // #101390#; the value of the macro is in HMM so use convertMm100ToTwip to convert
                    pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_DIRECT,
                        o3tl::toTwips(nWidth, o3tl::Length::mm100));
                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit   eMetric    = SC_MOD()->GetAppOptions().GetAppMetric();
                    ScViewData& rData      = GetViewData();
                    sal_uInt16      nCurHeight = rData.GetDocument().
                                                GetColWidth( rData.GetCurX(),
                                                             rData.GetTabNo() );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    VclPtr<AbstractScMetricInputDlg> pDlg(pFact->CreateScMetricInputDlg(
                        pTabViewShell->GetFrameWeld(), u"ColWidthDialog"_ustr, nCurHeight,
                        STD_COL_WIDTH, eMetric, 2, MAX_COL_WIDTH));

                    pDlg->StartExecuteAsync([pDlg, pTabViewShell](sal_Int32 nResult){
                        if ( nResult == RET_OK )
                        {
                            SfxRequest pRequest(pTabViewShell->GetViewFrame(), FID_COL_WIDTH);
                            tools::Long nVal = pDlg->GetInputValue();
                            pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_DIRECT, static_cast<sal_uInt16>(nVal) );

                            // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                            pRequest.AppendItem( SfxUInt16Item( FID_COL_WIDTH, static_cast<sal_uInt16>(TwipsToEvenHMM(nVal))) );
                            pRequest.Done();
                        }
                        pDlg->disposeOnce();
                    });
                }
            }
            break;

        case FID_COL_OPT_WIDTH:
            {
                if ( pReqArgs )
                {
                    const SfxUInt16Item&  rUInt16Item = pReqArgs->Get( FID_COL_OPT_WIDTH );

                    // #101390#; the value of the macro is in HMM so use convertMm100ToTwip to convert
                    pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_OPTIMAL,
                                    o3tl::toTwips(rUInt16Item.GetValue(), o3tl::Length::mm100) );
                    ScGlobal::nLastColWidthExtra = rUInt16Item.GetValue();

                    if( ! rReq.IsAPI() )
                        rReq.Done();
                }
                else
                {
                    FieldUnit eMetric = SC_MOD()->GetAppOptions().GetAppMetric();

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    VclPtr<AbstractScMetricInputDlg> pDlg(pFact->CreateScMetricInputDlg(
                        pTabViewShell->GetFrameWeld(), u"OptimalColWidthDialog"_ustr,
                        ScGlobal::nLastColWidthExtra, STD_EXTRA_WIDTH, eMetric, 2, MAX_EXTRA_WIDTH));

                    pDlg->StartExecuteAsync([pDlg, pTabViewShell](sal_Int32 nResult){
                        SfxRequest pRequest(pTabViewShell->GetViewFrame(), FID_COL_OPT_WIDTH);
                        if ( nResult == RET_OK )
                        {
                            tools::Long nVal = pDlg->GetInputValue();
                            pTabViewShell->SetMarkedWidthOrHeight( true, SC_SIZE_OPTIMAL, static_cast<sal_uInt16>(nVal) );
                            ScGlobal::nLastColWidthExtra = nVal;

                            // #101390#; the value of the macro should be in HMM so use TwipsToEvenHMM to convert
                            pRequest.AppendItem( SfxUInt16Item( FID_COL_OPT_WIDTH, static_cast<sal_uInt16>(TwipsToEvenHMM(nVal)) ) );
                            pRequest.Done();
                        }
                        pDlg->disposeOnce();
                    });
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
            if ( !GetViewData().GetDocument().GetChangeTrack() )
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
                        assert(dynamic_cast<const SfxBoolItem*>( pItem) && "wrong item");
                        bMoveContents = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    }

                    pTabViewShell->MergeCells( bApi, bMoveContents, bCenter, nSlot );
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
                weld::Window* pDlgParent = pTabViewShell->GetFrameWeld();
                SCCOL nStartCol;
                SCROW nStartRow;
                SCTAB nStartTab;
                SCCOL nEndCol;
                SCROW nEndRow;
                SCTAB nEndTab;

                const ScMarkData& rMark = GetViewData().GetMarkData();
                if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
                    pTabViewShell->MarkDataArea();

                GetViewData().GetSimpleArea( nStartCol,nStartRow,nStartTab,
                                              nEndCol,nEndRow,nEndTab );

                if (   ( std::abs(nEndCol-nStartCol) > 1 )
                    && ( std::abs(nEndRow-nStartRow) > 1 ) )
                {
                    if ( pReqArgs )
                    {
                        const SfxStringItem& rNameItem = pReqArgs->Get( SID_AUTOFORMAT );
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

                        ScopedVclPtr<AbstractScAutoFormatDlg> pDlg(pFact->CreateScAutoFormatDlg(pDlgParent, ScGlobal::GetOrCreateAutoFormat(), pNewEntry.get(), GetViewData()));

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
                {
                    std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(pDlgParent,
                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                   ScResId(STR_INVALID_AFAREA)));
                    xErrorBox->run();
                }
            }
            break;

        case SID_CANCEL:
            {
                if (GetViewData().HasEditView(GetViewData().GetActivePart()))
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

        case SID_ACCEPT_FORMULA:
            {
                if (GetViewData().HasEditView(GetViewData().GetActivePart()))
                    pScMod->InputEnterHandler();
            }
            break;

        case SID_START_FORMULA:
            {
                ScInputHandler* pInputHandler = pScMod->GetInputHdl();
                if (pInputHandler && pInputHandler->GetInputWindow())
                    pInputHandler->GetInputWindow()->StartFormula();
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
            GetViewData().GetDispatcher().Execute(
                SID_NAVIGATOR, SfxCallMode::SYNCHRON|SfxCallMode::RECORD );
            break;

        case SID_MARKAREA:
            // called from Basic at the hidden view to select a range in the visible view
            OSL_FAIL("old slot SID_MARKAREA");
            break;

        case FID_MOVE_KEEP_INSERT_MODE:
        {
            const SfxBoolItem* pEnabledArg = rReq.GetArg<SfxBoolItem>(FID_MOVE_KEEP_INSERT_MODE);
            if (!pEnabledArg) {
                SAL_WARN("sfx.appl", "FID_MOVE_KEEP_INSERT_MODE: must specify if you would like this to be enabled");
                break;
            }

            ScInputOptions aInputOptions = pScMod->GetInputOptions();

            aInputOptions.SetMoveKeepEdit(pEnabledArg->GetValue());
            pScMod->SetInputOptions(aInputOptions);

            if (comphelper::LibreOfficeKit::isActive())
                pTabViewShell->SetMoveKeepEdit(pEnabledArg->GetValue());

            break;
        }

        default:
            OSL_FAIL("ScCellShell::Execute: unknown slot");
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
