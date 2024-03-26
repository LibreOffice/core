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

#include <basic/sberrors.hxx>
#include <scitems.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <basic/sbxcore.hxx>
#include <svl/whiter.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <unotools/moduleoptions.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>

#include <cellsh.hxx>
#include <dbdata.hxx>
#include <queryparam.hxx>
#include <tabvwsh.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <global.hxx>
#include <scmod.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <uiitems.hxx>
#include <dbdocfun.hxx>
#include <reffact.hxx>
#include <utility>
#include <validat.hxx>
#include <validate.hxx>
#include <datamapper.hxx>
#include <datafdlg.hxx>

#include <scui_def.hxx>
#include <scabstdlg.hxx>
#include <impex.hxx>
#include <asciiopt.hxx>
#include <datastream.hxx>
#include <datastreamdlg.hxx>
#include <dataproviderdlg.hxx>
#include <queryentry.hxx>
#include <markdata.hxx>
#include <documentlinkmgr.hxx>
#include <officecfg/Office/Common.hxx>

#include <o3tl/make_shared.hxx>
#include <memory>

using namespace com::sun::star;

static bool lcl_GetTextToColumnsRange( const ScViewData& rData, ScRange& rRange, bool bDoEmptyCheckOnly )
{
    bool bRet = false;
    const ScMarkData& rMark = rData.GetMarkData();

    if ( rMark.IsMarked() )
    {
        if ( !rMark.IsMultiMarked() )
        {
            rRange = rMark.GetMarkArea();
            if ( rRange.aStart.Col() == rRange.aEnd.Col() )
            {
                bRet = true;
            }
        }
    }
    else
    {
        const SCCOL nCol = rData.GetCurX();
        const SCROW nRow = rData.GetCurY();
        const SCTAB nTab = rData.GetTabNo();
        rRange = ScRange( nCol, nRow, nTab, nCol, nRow, nTab );
        bRet = true;
    }

    const ScDocument& rDoc = rData.GetDocument();

    if ( bDoEmptyCheckOnly )
    {
        if ( bRet && rDoc.IsBlockEmpty( rRange.aStart.Col(), rRange.aStart.Row(),
                                        rRange.aEnd.Col(),  rRange.aEnd.Row(),
                                        rRange.aStart.Tab() ) )
        {
            bRet = false;
        }
    }
    else if ( bRet )
    {
        rRange.PutInOrder();
        SCCOL nStartCol = rRange.aStart.Col(), nEndCol = rRange.aEnd.Col();
        SCROW nStartRow = rRange.aStart.Row(), nEndRow = rRange.aEnd.Row();
        bool bShrunk = false;
        rDoc.ShrinkToUsedDataArea( bShrunk, rRange.aStart.Tab(), nStartCol, nStartRow,
                                   nEndCol, nEndRow, false, false, true );
        if ( bShrunk )
        {
            rRange.aStart.SetRow( nStartRow );
            rRange.aEnd.SetRow( nEndRow );
        }
    }

    return bRet;
}

static bool lcl_GetSortParam( const ScViewData& rData, const ScSortParam& rSortParam )
{
    ScTabViewShell* pTabViewShell   = rData.GetViewShell();
    ScDBData*   pDBData             = pTabViewShell->GetDBData();
    ScDocument& rDoc                = rData.GetDocument();
    SCTAB nTab                      = rData.GetTabNo();
    ScDirection eFillDir            = DIR_TOP;
    bool  bSort                     = true;
    ScRange aExternalRange;

    if( rSortParam.nCol1 != rSortParam.nCol2 )
        eFillDir = DIR_LEFT;
    if( rSortParam.nRow1 != rSortParam.nRow2 )
        eFillDir = DIR_TOP;

    if( rSortParam.nRow2 == rDoc.MaxRow() )
    {
        // Assume that user selected entire column(s), but cater for the
        // possibility that the start row is not the first row.
        SCSIZE nCount = rDoc.GetEmptyLinesInBlock( rSortParam.nCol1, rSortParam.nRow1, nTab,
                                                    rSortParam.nCol2, rSortParam.nRow2, nTab, eFillDir );
        aExternalRange = ScRange( rSortParam.nCol1,
                ::std::min( rSortParam.nRow1 + sal::static_int_cast<SCROW>( nCount ), rDoc.MaxRow()), nTab,
                rSortParam.nCol2, rSortParam.nRow2, nTab);
        aExternalRange.PutInOrder();
    }
    else if (rSortParam.nCol1 != rSortParam.nCol2 || rSortParam.nRow1 != rSortParam.nRow2)
    {
        // Preserve a preselected area.
        aExternalRange = ScRange( rSortParam.nCol1, rSortParam.nRow1, nTab, rSortParam.nCol2, rSortParam.nRow2, nTab);
        aExternalRange.PutInOrder();
    }
    else
        aExternalRange = ScRange( rData.GetCurX(), rData.GetCurY(), nTab );

    SCROW nStartRow = aExternalRange.aStart.Row();
    SCCOL nStartCol = aExternalRange.aStart.Col();
    SCROW nEndRow   = aExternalRange.aEnd.Row();
    SCCOL nEndCol   = aExternalRange.aEnd.Col();
    rDoc.GetDataArea( aExternalRange.aStart.Tab(), nStartCol, nStartRow, nEndCol, nEndRow, false, false );
    aExternalRange.aStart.SetRow( nStartRow );
    aExternalRange.aStart.SetCol( nStartCol );
    aExternalRange.aEnd.SetRow( nEndRow );
    aExternalRange.aEnd.SetCol( nEndCol );

    // with LibreOfficeKit, don't try to interact with the user
    if (!comphelper::LibreOfficeKit::isActive() &&
        ((rSortParam.nCol1 == rSortParam.nCol2 && aExternalRange.aStart.Col() != aExternalRange.aEnd.Col()) ||
         (rSortParam.nRow1 == rSortParam.nRow2 && aExternalRange.aStart.Row() != aExternalRange.aEnd.Row())))
    {
        pTabViewShell->AddHighlightRange( aExternalRange,COL_LIGHTBLUE );
        OUString aExtendStr( aExternalRange.Format(rDoc, ScRefFlags::VALID));

        ScRange aCurrentRange( rSortParam.nCol1, rSortParam.nRow1, nTab, rSortParam.nCol2, rSortParam.nRow2, nTab );
        OUString aCurrentStr(aCurrentRange.Format(rDoc, ScRefFlags::VALID));

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

        ScopedVclPtr<AbstractScSortWarningDlg> pWarningDlg(pFact->CreateScSortWarningDlg(pTabViewShell->GetFrameWeld(), aExtendStr, aCurrentStr));
        short bResult = pWarningDlg->Execute();
        if( bResult == BTN_EXTEND_RANGE || bResult == BTN_CURRENT_SELECTION )
        {
            if( bResult == BTN_EXTEND_RANGE )
            {
                pTabViewShell->MarkRange( aExternalRange, false );
                pDBData->SetArea( nTab, aExternalRange.aStart.Col(), aExternalRange.aStart.Row(), aExternalRange.aEnd.Col(), aExternalRange.aEnd.Row() );
            }
        }
        else
        {
            bSort = false;
            rData.GetDocShell()->CancelAutoDBRange();
        }

        pTabViewShell->ClearHighlightRanges();
    }
    return bSort;
}

namespace
{
    // this registers the dialog which Find1RefWindow search for
    class ScValidationRegisteredDlg
    {
        std::shared_ptr<SfxDialogController> m_xDlg;
    public:
        ScValidationRegisteredDlg(weld::Window* pParent, std::shared_ptr<SfxDialogController> xDlg)
            : m_xDlg(std::move(xDlg))
        {
            SC_MOD()->RegisterRefController(static_cast<sal_uInt16>(ScValidationDlg::SLOTID), m_xDlg, pParent);
        }
        ~ScValidationRegisteredDlg()
        {
            m_xDlg->Close();
            SC_MOD()->UnregisterRefController(static_cast<sal_uInt16>(ScValidationDlg::SLOTID), m_xDlg);
        }
    };
}

void ScCellShell::ExecuteDB( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    sal_uInt16 nSlotId = rReq.GetSlot();
    const SfxItemSet*   pReqArgs    = rReq.GetArgs();
    ScModule*           pScMod      = SC_MOD();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    if ( GetViewData().HasEditView( GetViewData().GetActivePart() ) )
    {
        pScMod->InputEnterHandler();
        pTabViewShell->UpdateInputHandler();
    }

    switch ( nSlotId )
    {
        case SID_VIEW_DATA_SOURCE_BROWSER:
            {
                //  check if database beamer is open

                SfxViewFrame& rViewFrame = pTabViewShell->GetViewFrame();
                bool bWasOpen = false;
                {
                    uno::Reference<frame::XFrame> xFrame = rViewFrame.GetFrame().GetFrameInterface();
                    uno::Reference<frame::XFrame> xBeamerFrame = xFrame->findFrame(
                                                        "_beamer",
                                                        frame::FrameSearchFlag::CHILDREN);
                    if ( xBeamerFrame.is() )
                        bWasOpen = true;
                }

                if ( bWasOpen )
                {
                    //  close database beamer: just forward to SfxViewFrame

                    rViewFrame.ExecuteSlot( rReq );
                }
                else
                {
                    //  show database beamer: SfxViewFrame call must be synchronous

                    rViewFrame.ExecuteSlot( rReq, false );      // false = synchronous

                    //  select current database in database beamer

                    ScImportParam aImportParam;
                    ScDBData* pDBData = pTabViewShell->GetDBData(true,SC_DB_OLD);       // don't create if none found
                    if (pDBData)
                        pDBData->GetImportParam( aImportParam );

                    ScDBDocFunc::ShowInBeamer( aImportParam, &pTabViewShell->GetViewFrame() );
                }
                rReq.Done();        // needed because it's a toggle slot
            }
            break;

        case SID_REIMPORT_DATA:
            {
                bool bOk = false;
                ScDBData* pDBData = pTabViewShell->GetDBData(true,SC_DB_OLD);
                if (pDBData)
                {
                    ScImportParam aImportParam;
                    pDBData->GetImportParam( aImportParam );
                    if (aImportParam.bImport && !pDBData->HasImportSelection())
                    {
                        pTabViewShell->ImportData( aImportParam );
                        pDBData->SetImportParam( aImportParam );    //! Undo ??
                        bOk = true;
                    }
                }

                if (!bOk && ! rReq.IsAPI() )
                    pTabViewShell->ErrorMessage(STR_REIMPORT_EMPTY);

                if( bOk )
                    rReq.Done();
            }
            break;

        case SID_REFRESH_DBAREA:
            {
                ScDBData* pDBData = pTabViewShell->GetDBData(true,SC_DB_OLD);
                if (pDBData)
                {
                    //  repeat import like SID_REIMPORT_DATA

                    bool bContinue = true;
                    ScImportParam aImportParam;
                    pDBData->GetImportParam( aImportParam );
                    if (aImportParam.bImport && !pDBData->HasImportSelection())
                    {
                        bContinue = pTabViewShell->ImportData( aImportParam );
                        pDBData->SetImportParam( aImportParam );    //! Undo ??

                        //  mark (size may have been changed)
                        ScRange aNewRange;
                        pDBData->GetArea(aNewRange);
                        pTabViewShell->MarkRange(aNewRange);
                    }

                    if ( bContinue )        // fail at import -> break
                    {
                        //  internal operations, when any stored

                        if ( pDBData->HasQueryParam() || pDBData->HasSortParam() ||
                                                          pDBData->HasSubTotalParam() )
                            pTabViewShell->RepeatDB();

                        //  pivot tables that have the range as data source

                        ScRange aRange;
                        pDBData->GetArea(aRange);
                        GetViewData().GetDocShell()->RefreshPivotTables(aRange);
                    }
                }
                rReq.Done();
            }
            break;

        case SID_SBA_BRW_INSERT:
            {
                OSL_FAIL( "Deprecated Slot" );
            }
            break;

        case SID_DATA_FORM:
            {
                ScViewData& rData = GetViewData();
                ScRange aRange;
                rData.GetSimpleArea( aRange );
                ScAddress aStart = aRange.aStart;
                ScAddress aEnd = aRange.aEnd;

                if((aEnd.Col() - aStart.Col()) >= MAX_DATAFORM_COLS)
                {
                    rData.GetDocShell()->ErrorMessage(STR_TOO_MANY_COLUMNS_DATA_FORM);
                    break;
                }

                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                ScopedVclPtr<AbstractScDataFormDlg> pDlg(pFact->CreateScDataFormDlg(
                    pTabViewShell->GetFrameWeld(), pTabViewShell));

                pDlg->Execute();

                rReq.Done();
            }
            break;

        case SID_SUBTOTALS:
            ExecuteSubtotals(rReq);
            break;

        case SID_SORT_DESCENDING:
        case SID_SORT_ASCENDING:
            {
                //#i60401 ux-ctest: Calc does not support all users' strategies regarding sorting data
                //the patch comes from maoyg
                ScSortParam aSortParam;
                ScDBData*   pDBData = pTabViewShell->GetDBData();
                ScViewData& rData   = GetViewData();

                pDBData->GetSortParam( aSortParam );

                if( lcl_GetSortParam( rData, aSortParam ) )
                {
                    SCCOL nCol  = GetViewData().GetCurX();
                    SCCOL nTab  = GetViewData().GetTabNo();
                    ScDocument& rDoc = GetViewData().GetDocument();

                    pDBData->GetSortParam( aSortParam );
                    bool bHasHeader = rDoc.HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, nTab );

                    if( nCol < aSortParam.nCol1 )
                        nCol = aSortParam.nCol1;
                    else if( nCol > aSortParam.nCol2 )
                        nCol = aSortParam.nCol2;

                    aSortParam.bHasHeader       = bHasHeader;
                    aSortParam.bByRow           = true;
                    aSortParam.bCaseSens        = false;
                    aSortParam.bNaturalSort     = false;
                    aSortParam.aDataAreaExtras.mbCellNotes = false;
                    aSortParam.aDataAreaExtras.mbCellDrawObjects = true;
                    aSortParam.aDataAreaExtras.mbCellFormats = true;
                    aSortParam.bInplace         = true;
                    aSortParam.maKeyState[0].bDoSort = true;
                    aSortParam.maKeyState[0].nField = nCol;
                    aSortParam.maKeyState[0].bAscending = ( nSlotId == SID_SORT_ASCENDING );

                    for ( sal_uInt16 i=1; i<aSortParam.GetSortKeyCount(); i++ )
                        aSortParam.maKeyState[i].bDoSort = false;

                    pTabViewShell->UISort( aSortParam );        // subtotal when needed new

                    rReq.Done();
                }
            }
            break;

        case SID_SORT:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();

                //#i60401 ux-ctest: Calc does not support all users' strategies regarding sorting data
                //the patch comes from maoyg

                if ( pArgs )        // Basic
                {
                    ScSortParam aSortParam;
                    ScDBData*   pDBData = pTabViewShell->GetDBData();
                    ScViewData& rData   = GetViewData();

                    pDBData->GetSortParam( aSortParam );

                    if( lcl_GetSortParam( rData, aSortParam ) )
                    {
                        ScDocument& rDoc = GetViewData().GetDocument();

                        pDBData->GetSortParam( aSortParam );
                        bool bHasHeader = rDoc.HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, rData.GetTabNo() );
                        if( bHasHeader )
                            aSortParam.bHasHeader = bHasHeader;

                        aSortParam.bInplace = true;             // from Basic always

                        if ( const SfxBoolItem* pItem = pArgs->GetItemIfSet( SID_SORT_BYROW ) )
                            aSortParam.bByRow = pItem->GetValue();
                        if ( const SfxBoolItem* pItem = pArgs->GetItemIfSet( SID_SORT_HASHEADER ) )
                            aSortParam.bHasHeader = pItem->GetValue();
                        if ( const SfxBoolItem* pItem = pArgs->GetItemIfSet( SID_SORT_CASESENS ) )
                            aSortParam.bCaseSens = pItem->GetValue();
                        if ( const SfxBoolItem* pItem = pArgs->GetItemIfSet( SID_SORT_NATURALSORT ) )
                            aSortParam.bNaturalSort = pItem->GetValue();
                        if ( const SfxBoolItem* pItem = pArgs->GetItemIfSet( SID_SORT_INCCOMMENTS ) )
                            aSortParam.aDataAreaExtras.mbCellNotes = pItem->GetValue();
                        if ( const SfxBoolItem* pItem = pArgs->GetItemIfSet( SID_SORT_INCIMAGES ) )
                            aSortParam.aDataAreaExtras.mbCellDrawObjects = pItem->GetValue();
                        if ( const SfxBoolItem* pItem = pArgs->GetItemIfSet( SID_SORT_ATTRIBS ) )
                            aSortParam.aDataAreaExtras.mbCellFormats = pItem->GetValue();
                        if ( const SfxUInt16Item* pItem = pArgs->GetItemIfSet( SID_SORT_USERDEF ) )
                        {
                            sal_uInt16 nUserIndex = pItem->GetValue();
                            aSortParam.bUserDef = ( nUserIndex != 0 );
                            if ( nUserIndex )
                                aSortParam.nUserIndex = nUserIndex - 1;     // Basic: 1-based
                        }

                        SCCOLROW nField0 = 0;
                        const SfxPoolItem* pItem = nullptr;
                        if ( pArgs->GetItemState( FN_PARAM_1, true, &pItem ) == SfxItemState::SET )
                            nField0 = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                        aSortParam.maKeyState[0].bDoSort = ( nField0 != 0 );
                        aSortParam.maKeyState[0].nField = nField0 > 0 ? (nField0-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_2, true, &pItem ) == SfxItemState::SET )
                            aSortParam.maKeyState[0].bAscending = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        SCCOLROW nField1 = 0;
                        if ( pArgs->GetItemState( FN_PARAM_3, true, &pItem ) == SfxItemState::SET )
                            nField1 = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                        aSortParam.maKeyState[1].bDoSort = ( nField1 != 0 );
                        aSortParam.maKeyState[1].nField = nField1 > 0 ? (nField1-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_4, true, &pItem ) == SfxItemState::SET )
                            aSortParam.maKeyState[1].bAscending = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                        SCCOLROW nField2 = 0;
                        if ( pArgs->GetItemState( FN_PARAM_5, true, &pItem ) == SfxItemState::SET )
                            nField2 = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                        aSortParam.maKeyState[2].bDoSort = ( nField2 != 0 );
                        aSortParam.maKeyState[2].nField = nField2 > 0 ? (nField2-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_6, true, &pItem ) == SfxItemState::SET )
                            aSortParam.maKeyState[2].bAscending = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                        // subtotal when needed new
                        pTabViewShell->UISort( aSortParam );
                        rReq.Done();
                    }
                }
                else
                {
                    ScSortParam aSortParam;
                    ScDBData*   pDBData = pTabViewShell->GetDBData();
                    ScViewData& rData   = GetViewData();

                    pDBData->GetSortParam( aSortParam );

                    if( lcl_GetSortParam( rData, aSortParam ) )
                    {
                        ScDocument& rDoc = GetViewData().GetDocument();
                        SfxItemSetFixed<SCITEM_SORTDATA, SCITEM_SORTDATA>  aArgSet( GetPool() );

                        pDBData->GetSortParam( aSortParam );
                        bool bHasHeader = rDoc.HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, rData.GetTabNo() );
                        if( bHasHeader )
                            aSortParam.bHasHeader = bHasHeader;

                        aArgSet.Put( ScSortItem( SCITEM_SORTDATA, &GetViewData(), &aSortParam ) );

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        std::shared_ptr<ScAsyncTabController> pDlg(pFact->CreateScSortDlg(pTabViewShell->GetFrameWeld(),  &aArgSet));
                        pDlg->SetCurPageId("criteria");  // 1=sort field tab  2=sort options tab

                        VclAbstractDialog::AsyncContext aContext;
                        aContext.maEndDialogFn = [pDlg, &rData, pTabViewShell](sal_Int32 nResult)
                            {
                                if ( nResult == RET_OK )
                                {
                                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                                    const ScSortParam& rOutParam =
                                        pOutSet->Get( SCITEM_SORTDATA ).GetSortData();

                                    // subtotal when needed new

                                    pTabViewShell->UISort( rOutParam );

                                    SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                                    SfxRequest aRequest(rViewFrm, SID_SORT);

                                    if ( rOutParam.bInplace )
                                    {
                                        aRequest.AppendItem( SfxBoolItem( SID_SORT_BYROW,
                                            rOutParam.bByRow ) );
                                        aRequest.AppendItem( SfxBoolItem( SID_SORT_HASHEADER,
                                            rOutParam.bHasHeader ) );
                                        aRequest.AppendItem( SfxBoolItem( SID_SORT_CASESENS,
                                            rOutParam.bCaseSens ) );
                                        aRequest.AppendItem( SfxBoolItem( SID_SORT_NATURALSORT,
                                                    rOutParam.bNaturalSort ) );
                                        aRequest.AppendItem( SfxBoolItem( SID_SORT_INCCOMMENTS,
                                                    rOutParam.aDataAreaExtras.mbCellNotes ) );
                                        aRequest.AppendItem( SfxBoolItem( SID_SORT_INCIMAGES,
                                                    rOutParam.aDataAreaExtras.mbCellDrawObjects ) );
                                        aRequest.AppendItem( SfxBoolItem( SID_SORT_ATTRIBS,
                                                    rOutParam.aDataAreaExtras.mbCellFormats ) );
                                        sal_uInt16 nUser = rOutParam.bUserDef ? ( rOutParam.nUserIndex + 1 ) : 0;
                                        aRequest.AppendItem( SfxUInt16Item( SID_SORT_USERDEF, nUser ) );
                                        if ( rOutParam.maKeyState[0].bDoSort )
                                        {
                                            aRequest.AppendItem( SfxInt32Item( TypedWhichId<SfxInt32Item>(FN_PARAM_1),
                                                rOutParam.maKeyState[0].nField + 1 ) );
                                            aRequest.AppendItem( SfxBoolItem( FN_PARAM_2,
                                                rOutParam.maKeyState[0].bAscending ) );
                                        }
                                        if ( rOutParam.maKeyState[1].bDoSort )
                                        {
                                            aRequest.AppendItem( SfxInt32Item( TypedWhichId<SfxInt32Item>(FN_PARAM_3),
                                                rOutParam.maKeyState[1].nField + 1 ) );
                                            aRequest.AppendItem( SfxBoolItem( FN_PARAM_4,
                                                rOutParam.maKeyState[1].bAscending ) );
                                        }
                                        if ( rOutParam.maKeyState[2].bDoSort )
                                        {
                                            aRequest.AppendItem( SfxInt32Item( TypedWhichId<SfxInt32Item>(FN_PARAM_5),
                                                rOutParam.maKeyState[2].nField + 1 ) );
                                            aRequest.AppendItem( SfxBoolItem( FN_PARAM_6,
                                                rOutParam.maKeyState[2].bAscending ) );
                                        }
                                    }

                                    aRequest.Done();
                                }
                                else
                                {
                                    rData.GetDocShell()->CancelAutoDBRange();
                                }
                            };

                        pDlg->StartExecuteAsync(aContext);
                    }
                }
            }
            break;

        case SID_FILTER:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    OSL_FAIL("SID_FILTER with arguments?");
                    pTabViewShell->Query(
                            pArgs->Get( SCITEM_QUERYDATA ).GetQueryData(), nullptr, true );
                    rReq.Done();
                }
                else
                {
                    sal_uInt16          nId  = ScFilterDlgWrapper::GetChildWindowId();
                    SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );

                    pScMod->SetRefDialog( nId, pWnd == nullptr );
                }
            }
            break;

        case SID_SPECIAL_FILTER:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    OSL_FAIL("SID_SPECIAL_FILTER with arguments?");
                    pTabViewShell->Query(
                            pArgs->Get( SCITEM_QUERYDATA ).GetQueryData(), nullptr, true );
                    rReq.Done();
                }
                else
                {
                    sal_uInt16          nId  = ScSpecialFilterDlgWrapper::GetChildWindowId();
                    SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                    SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );

                    pScMod->SetRefDialog( nId, pWnd == nullptr );
                }
            }
            break;

        case FID_FILTER_OK:
            {
                const ScQueryItem* pQueryItem;
                if ( pReqArgs && (pQueryItem =
                        pReqArgs->GetItemIfSet( SCITEM_QUERYDATA )) )
                {
                    SCTAB nCurTab = GetViewData().GetTabNo();
                    SCTAB nRefTab = GetViewData().GetRefTabNo();

                    // If RefInput switched to a different sheet from the data sheet,
                    // switch back:

                    if ( nCurTab != nRefTab )
                    {
                        pTabViewShell->SetTabNo( nRefTab );
                        pTabViewShell->PaintExtras();
                    }

                    ScRange aAdvSource;
                    if (pQueryItem->GetAdvancedQuerySource(aAdvSource))
                        pTabViewShell->Query( pQueryItem->GetQueryData(), &aAdvSource, true );
                    else
                        pTabViewShell->Query( pQueryItem->GetQueryData(), nullptr, true );
                    rReq.Done( *pReqArgs );
                }
            }
            break;

        case SID_UNFILTER:
            {
                ScQueryParam aParam;
                ScDBData*    pDBData = pTabViewShell->GetDBData();

                pDBData->GetQueryParam( aParam );
                SCSIZE nEC = aParam.GetEntryCount();
                for (SCSIZE i=0; i<nEC; i++)
                    aParam.GetEntry(i).bDoQuery = false;
                aParam.bDuplicate = true;
                pTabViewShell->Query( aParam, nullptr, true );
                rReq.Done();
            }
            break;

        case SID_AUTO_FILTER:
            pTabViewShell->ToggleAutoFilter();
            rReq.Done();
            break;

        case SID_AUTOFILTER_HIDE:
            pTabViewShell->HideAutoFilter();
            rReq.Done();
            break;

        case SID_PIVOT_TABLE:
            {
                const ScPivotItem* pPItem;
                if ( pReqArgs && (pPItem =
                        pReqArgs->GetItemIfSet( SCITEM_PIVOTDATA )) )
                {
                    SCTAB nCurTab = GetViewData().GetTabNo();
                    SCTAB nRefTab = GetViewData().GetRefTabNo();

                    // If RefInput switched to a different sheet from the data sheet,
                    // switch back:

                    if ( nCurTab != nRefTab )
                    {
                        pTabViewShell->SetTabNo( nRefTab );
                        pTabViewShell->PaintExtras();
                    }

                    const ScDPObject* pDPObject = pTabViewShell->GetDialogDPObject();
                    if ( pDPObject )
                    {
                        bool bSuccess = pTabViewShell->MakePivotTable(
                            pPItem->GetData(), pPItem->GetDestRange(), pPItem->IsNewSheet(), *pDPObject );
                        SfxBoolItem aRet(0, bSuccess);
                        rReq.SetReturnValue(aRet);
                    }
                    rReq.Done();
                }
#if HAVE_FEATURE_SCRIPTING
                else if (rReq.IsAPI())
                    SbxBase::SetError(ERRCODE_BASIC_BAD_PARAMETER);
#endif
            }
            break;

        case SID_OPENDLG_PIVOTTABLE:
            ExecuteDataPilotDialog();
            break;
        case SID_DEFINE_DBNAME:
            {

                sal_uInt16          nId  = ScDbNameDlgWrapper::GetChildWindowId();
                SfxViewFrame& rViewFrm = pTabViewShell->GetViewFrame();
                SfxChildWindow* pWnd = rViewFrm.GetChildWindow( nId );

                pScMod->SetRefDialog( nId, pWnd == nullptr );

            }
            break;

        case SID_SELECT_DB:
            {
                if ( pReqArgs )
                {
                    const SfxStringItem& rItem = pReqArgs->Get(SID_SELECT_DB);
                    pTabViewShell->GotoDBArea(rItem.GetValue());
                    rReq.Done();
                }
                else
                {
                    ScDocument& rDoc   = GetViewData().GetDocument();
                    ScDBCollection* pDBCol = rDoc.GetDBCollection();

                    if ( pDBCol )
                    {
                        std::vector<OUString> aList;
                        const ScDBCollection::NamedDBs& rDBs = pDBCol->getNamedDBs();
                        for (const auto& rxDB : rDBs)
                            aList.push_back(rxDB->GetName());

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                        ScopedVclPtr<AbstractScSelEntryDlg> pDlg(pFact->CreateScSelEntryDlg(pTabViewShell->GetFrameWeld(), aList));
                        if ( pDlg->Execute() == RET_OK )
                        {
                            OUString aName = pDlg->GetSelectedEntry();
                            pTabViewShell->GotoDBArea( aName );
                            rReq.AppendItem( SfxStringItem( SID_SELECT_DB, aName ) );
                            rReq.Done();
                        }
                    }
                }
            }
            break;
        case SID_DATA_STREAMS:
        {
            sc::DataStreamDlg aDialog(GetViewData().GetDocShell(), pTabViewShell->GetFrameWeld());
            ScDocument& rDoc = GetViewData().GetDocument();
            sc::DocumentLinkManager& rMgr = rDoc.GetDocLinkManager();
            sc::DataStream* pStrm = rMgr.getDataStream();
            if (pStrm)
                aDialog.Init(*pStrm);

            if (aDialog.run() == RET_OK)
                aDialog.StartStream();
        }
        break;
        case SID_DATA_STREAMS_PLAY:
        {
            ScDocument& rDoc = GetViewData().GetDocument();
            sc::DocumentLinkManager& rMgr = rDoc.GetDocLinkManager();
            sc::DataStream* pStrm = rMgr.getDataStream();
            if (pStrm)
                pStrm->StartImport();
        }
        break;
        case SID_DATA_STREAMS_STOP:
        {
            ScDocument& rDoc = GetViewData().GetDocument();
            sc::DocumentLinkManager& rMgr = rDoc.GetDocLinkManager();
            sc::DataStream* pStrm = rMgr.getDataStream();
            if (pStrm)
                pStrm->StopImport();
        }
        break;
        case SID_DATA_PROVIDER:
        {
            auto xDoc = o3tl::make_shared<ScDocument>();
            xDoc->InsertTab(0, "test");
            ScDocument& rDoc = GetViewData().GetDocument();
            ScDataProviderDlg aDialog(pTabViewShell->GetDialogParent(), xDoc, &rDoc);
            if (aDialog.run() == RET_OK)
            {
                aDialog.import(rDoc);
            }
        }
        break;
        case SID_DATA_PROVIDER_REFRESH:
        {
            ScDocument& rDoc = GetViewData().GetDocument();
            auto& rDataMapper = rDoc.GetExternalDataMapper();
            for (auto& rDataSource : rDataMapper.getDataSources())
            {
                rDataSource.refresh(&rDoc, false);
            }
        }
        break;
        case SID_MANAGE_XML_SOURCE:
            ExecuteXMLSourceDialog();
        break;
        case FID_VALIDATION:
        case FID_CURRENTVALIDATION:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();
                if ( pArgs )
                {
                    OSL_FAIL("later...");
                }
                else
                {
                    SfxItemSet aArgSet( GetPool(), ScTPValidationValue::GetRanges() );
                    ScValidationMode eMode = SC_VALID_ANY;
                    ScConditionMode eOper = ScConditionMode::Equal;
                    OUString aExpr1, aExpr2;
                    bool bBlank = true;
                    sal_Int16 nListType = css::sheet::TableValidationVisibility::UNSORTED;
                    bool bShowHelp = false;
                    OUString aHelpTitle, aHelpText;
                    bool bShowError = false;
                    ScValidErrorStyle eErrStyle = SC_VALERR_STOP;
                    OUString aErrTitle, aErrText;

                    ScDocument& rDoc = GetViewData().GetDocument();
                    SCCOL nCurX = GetViewData().GetCurX();
                    SCROW nCurY = GetViewData().GetCurY();
                    SCTAB nTab = GetViewData().GetTabNo();
                    ScAddress aCursorPos( nCurX, nCurY, nTab );
                    sal_uInt32 nIndex = rDoc.GetAttr(
                                nCurX, nCurY, nTab, ATTR_VALIDDATA )->GetValue();
                    if ( nIndex )
                    {
                        const ScValidationData* pOldData = rDoc.GetValidationEntry( nIndex );
                        if ( pOldData )
                        {
                            eMode = pOldData->GetDataMode();
                            eOper = pOldData->GetOperation();
                            sal_uInt32 nNumFmt = 0;
                            if ( eMode == SC_VALID_DATE || eMode == SC_VALID_TIME )
                            {
                                SvNumFormatType nType = ( eMode == SC_VALID_DATE ) ? SvNumFormatType::DATE
                                                                         : SvNumFormatType::TIME;
                                nNumFmt = rDoc.GetFormatTable()->GetStandardFormat(
                                                                    nType, ScGlobal::eLnge );
                            }
                            aExpr1 = pOldData->GetExpression( aCursorPos, 0, nNumFmt );
                            aExpr2 = pOldData->GetExpression( aCursorPos, 1, nNumFmt );
                            bBlank = pOldData->IsIgnoreBlank();
                            nListType = pOldData->GetListType();

                            bShowHelp = pOldData->GetInput( aHelpTitle, aHelpText );
                            bShowError = pOldData->GetErrMsg( aErrTitle, aErrText, eErrStyle );

                            aArgSet.Put( SfxUInt16Item( FID_VALID_MODE,         sal::static_int_cast<sal_uInt16>(eMode) ) );
                            aArgSet.Put( SfxUInt16Item( FID_VALID_CONDMODE,     sal::static_int_cast<sal_uInt16>(eOper) ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_VALUE1,      aExpr1 ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_VALUE2,      aExpr2 ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_BLANK,       bBlank ) );
                            aArgSet.Put( SfxInt16Item(   FID_VALID_LISTTYPE,    nListType ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_SHOWHELP,    bShowHelp ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_HELPTITLE,   aHelpTitle ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_HELPTEXT,    aHelpText ) );
                            aArgSet.Put( SfxBoolItem(    FID_VALID_SHOWERR,     bShowError ) );
                            aArgSet.Put( SfxUInt16Item( FID_VALID_ERRSTYLE,     sal::static_int_cast<sal_uInt16>(eErrStyle) ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_ERRTITLE,    aErrTitle ) );
                            aArgSet.Put( SfxStringItem(  FID_VALID_ERRTEXT,     aErrText ) );
                        }
                    }

                    // cell range picker
                    vcl::Window* pWin = GetViewData().GetActiveWin();
                    weld::Window* pParentWin = pWin ? pWin->GetFrameWeld() : nullptr;
                    auto xDlg = std::make_shared<ScValidationDlg>(pParentWin, &aArgSet, pTabViewShell);
                    ScValidationRegisteredDlg aRegisterThatDlgExists(pParentWin, xDlg);

                    short nResult = xDlg->run();
                    if ( nResult == RET_OK )
                    {
                        const SfxItemSet* pOutSet = xDlg->GetOutputItemSet();

                        if ( const SfxUInt16Item* pItem = pOutSet->GetItemIfSet( FID_VALID_MODE ) )
                            eMode = static_cast<ScValidationMode>(pItem->GetValue());
                        if ( const SfxUInt16Item* pItem = pOutSet->GetItemIfSet( FID_VALID_CONDMODE ) )
                            eOper = static_cast<ScConditionMode>(pItem->GetValue());
                        if ( const SfxStringItem* pItem = pOutSet->GetItemIfSet( FID_VALID_VALUE1 ) )
                        {
                            OUString aTemp1 = pItem->GetValue();
                            if (eMode == SC_VALID_DATE || eMode == SC_VALID_TIME)
                            {
                                sal_uInt32 nNumIndex = 0;
                                double nVal;
                                if (rDoc.GetFormatTable()->IsNumberFormat(aTemp1, nNumIndex, nVal))
                                    aExpr1 = ::rtl::math::doubleToUString( nVal,
                                            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                            ScGlobal::getLocaleData().getNumDecimalSep()[0], true);
                                else
                                    aExpr1 = aTemp1;
                            }
                            else
                                aExpr1 = aTemp1;
                        }
                        if ( const SfxStringItem* pItem = pOutSet->GetItemIfSet( FID_VALID_VALUE2 ) )
                        {
                            OUString aTemp2 = pItem->GetValue();
                            if (eMode == SC_VALID_DATE || eMode == SC_VALID_TIME)
                            {
                                sal_uInt32 nNumIndex = 0;
                                double nVal;
                                if (rDoc.GetFormatTable()->IsNumberFormat(aTemp2, nNumIndex, nVal))
                                    aExpr2 = ::rtl::math::doubleToUString( nVal,
                                            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                            ScGlobal::getLocaleData().getNumDecimalSep()[0], true);
                                else
                                    aExpr2 = aTemp2;
                                if ( eMode == SC_VALID_TIME ) {
                                    sal_Int32 wraparound = aExpr1.compareTo(aExpr2);
                                    if (wraparound > 0) {
                                        if (eOper == ScConditionMode::Between) {
                                            eOper = ScConditionMode::NotBetween;
                                            std::swap( aExpr1, aExpr2 );
                                        }
                                        else if (eOper == ScConditionMode::NotBetween) {
                                            eOper = ScConditionMode::Between;
                                            std::swap( aExpr1, aExpr2 );
                                        }
                                    }
                                }
                            }
                            else
                                aExpr2 = aTemp2;
                        }
                        if ( const SfxBoolItem* pItem = pOutSet->GetItemIfSet( FID_VALID_BLANK ) )
                            bBlank = pItem->GetValue();
                        if ( const SfxInt16Item* pItem = pOutSet->GetItemIfSet( FID_VALID_LISTTYPE ) )
                            nListType = pItem->GetValue();

                        if ( const SfxBoolItem* pItem = pOutSet->GetItemIfSet( FID_VALID_SHOWHELP ) )
                            bShowHelp = pItem->GetValue();
                        if ( const SfxStringItem* pItem = pOutSet->GetItemIfSet( FID_VALID_HELPTITLE ) )
                            aHelpTitle = pItem->GetValue();
                        if ( const SfxStringItem* pItem = pOutSet->GetItemIfSet( FID_VALID_HELPTEXT ) )
                            aHelpText = pItem->GetValue();

                        if ( const SfxBoolItem* pItem = pOutSet->GetItemIfSet( FID_VALID_SHOWERR ) )
                            bShowError = pItem->GetValue();
                        if ( const SfxUInt16Item* pItem = pOutSet->GetItemIfSet( FID_VALID_ERRSTYLE ) )
                            eErrStyle = static_cast<ScValidErrorStyle>(pItem->GetValue());
                        if ( const SfxStringItem* pItem = pOutSet->GetItemIfSet( FID_VALID_ERRTITLE ) )
                            aErrTitle = pItem->GetValue();
                        if ( const SfxStringItem* pItem = pOutSet->GetItemIfSet( FID_VALID_ERRTEXT ) )
                            aErrText = pItem->GetValue();

                        ScValidationData aData( eMode, eOper, aExpr1, aExpr2, rDoc, aCursorPos );
                        aData.SetIgnoreBlank( bBlank );
                        aData.SetListType( nListType );

                        aData.SetInput(aHelpTitle, aHelpText);          // sets bShowInput to TRUE
                        if (!bShowHelp)
                            aData.ResetInput();                         // reset only bShowInput

                        aData.SetError(aErrTitle, aErrText, eErrStyle); // sets bShowError to TRUE
                        if (!bShowError)
                            aData.ResetError();                         // reset only bShowError

                        pTabViewShell->SetValidation( aData );
                        pTabViewShell->TestHintWindow();
                        rReq.Done( *pOutSet );
                    }
                }
            }
            break;

        case SID_TEXT_TO_COLUMNS:
            {
                ScViewData& rData = GetViewData();
                ScRange aRange;

                if ( lcl_GetTextToColumnsRange( rData, aRange, false ) )
                {
                    ScDocument& rDoc = rData.GetDocument();

                    ScImportExport aExport( rDoc, aRange );
                    aExport.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::None, 0, false ) );

                    // #i87703# text to columns fails with tab separator
                    aExport.SetDelimiter( u'\0' );

                    SvMemoryStream aStream;
                    aStream.SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
                    ScImportExport::SetNoEndianSwap( aStream );
                    aExport.ExportStream( aStream, OUString(), SotClipboardFormatId::STRING );

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    ScopedVclPtr<AbstractScImportAsciiDlg> pDlg(pFact->CreateScImportAsciiDlg(
                            pTabViewShell->GetFrameWeld(), OUString(), &aStream, SC_TEXTTOCOLUMNS));

                    if ( pDlg->Execute() == RET_OK )
                    {
                        ScDocShell* pDocSh = rData.GetDocShell();
                        OSL_ENSURE( pDocSh, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pDocSh is null!" );

                        OUString aUndo = ScResId( STR_UNDO_TEXTTOCOLUMNS );
                        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, rData.GetViewShell()->GetViewShellId() );

                        ScImportExport aImport( rDoc, aRange.aStart );
                        ScAsciiOptions aOptions;
                        pDlg->GetOptions( aOptions );
                        pDlg->SaveParameters();
                        aImport.SetExtOptions( aOptions );
                        aImport.SetApi( false );
                        aImport.SetImportBroadcast( true );
                        aImport.SetOverwriting( true );
                        aStream.Seek( 0 );
                        aImport.ImportStream( aStream, OUString(), SotClipboardFormatId::STRING );

                        pDocSh->GetUndoManager()->LeaveListAction();
                    }
                }
            }
            break;
        }
}

void ScCellShell::GetDBState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    ScViewData& rData       = GetViewData();
    ScDocShell* pDocSh      = rData.GetDocShell();
    ScDocument& rDoc        = pDocSh->GetDocument();
    SCCOL       nPosX       = rData.GetCurX();
    SCROW       nPosY       = rData.GetCurY();
    SCTAB       nTab        = rData.GetTabNo();

    bool bAutoFilter = false;
    bool bAutoFilterTested = false;

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_REFRESH_DBAREA:
                {
                    //  imported data without selection
                    //  or filter,sort,subtotal (also without import)
                    bool bOk = false;
                    ScDBData* pDBData = pTabViewShell->GetDBData(false,SC_DB_OLD);
                    if (pDBData && rDoc.GetChangeTrack() == nullptr)
                    {
                        if ( pDBData->HasImportParam() )
                            bOk = !pDBData->HasImportSelection();
                        else
                        {
                            bOk = pDBData->HasQueryParam() ||
                                  pDBData->HasSortParam() ||
                                  pDBData->HasSubTotalParam();
                        }
                    }
                    if (!bOk)
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_FILTER:
            case SID_SPECIAL_FILTER:
                {
                    ScRange aDummy;
                    ScMarkType eMarkType = GetViewData().GetSimpleArea( aDummy);
                    if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

                //in case of Redlining and multiselection disable
            case SID_SORT_ASCENDING:
            case SID_SORT_DESCENDING:
            case SCITEM_SORTDATA:
            case SCITEM_SUBTDATA:
            case SID_OPENDLG_PIVOTTABLE:
                {
                    //! move ReadOnly check to idl flags

                    if ( pDocSh->IsReadOnly() || rDoc.GetChangeTrack()!=nullptr ||
                            GetViewData().IsMultiMarked() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_REIMPORT_DATA:
                {
                    //  only imported data without selection
                    ScDBData* pDBData = pTabViewShell->GetDBData(false,SC_DB_OLD);
                    if (!pDBData || !pDBData->HasImportParam() || pDBData->HasImportSelection() ||
                        rDoc.GetChangeTrack()!=nullptr)
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_VIEW_DATA_SOURCE_BROWSER:
                {
                    if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::EModule::DATABASE))
                        rSet.Put(SfxVisibilityItem(nWhich, false));
                    else
                        //  get state (BoolItem) from SfxViewFrame
                        pTabViewShell->GetViewFrame().GetSlotState( nWhich, nullptr, &rSet );
                }
                break;
            case SID_SBA_BRW_INSERT:
                {
                    //  SBA wants a sal_Bool-item, enabled

                    rSet.Put(SfxBoolItem(nWhich, true));
                }
                break;

            case SID_AUTO_FILTER:
            case SID_AUTOFILTER_HIDE:
                {
                    if (!bAutoFilterTested)
                    {
                        bAutoFilter = rDoc.HasAutoFilter( nPosX, nPosY, nTab );
                        bAutoFilterTested = true;
                    }
                    if ( nWhich == SID_AUTO_FILTER )
                    {
                        ScRange aDummy;
                        ScMarkType eMarkType = GetViewData().GetSimpleArea( aDummy);
                        if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
                        {
                            rSet.DisableItem( nWhich );
                        }
                        else if (rDoc.GetDPAtBlock(aDummy))
                        {
                            rSet.DisableItem( nWhich );
                        }
                        else
                            rSet.Put( SfxBoolItem( nWhich, bAutoFilter ) );
                    }
                    else
                        if (!bAutoFilter)
                            rSet.DisableItem( nWhich );
                }
                break;

            case SID_UNFILTER:
                {
                    SCCOL nStartCol, nEndCol;
                    SCROW  nStartRow, nEndRow;
                    SCTAB  nStartTab, nEndTab;
                    bool bAnyQuery = false;

                    bool bSelected = (GetViewData().GetSimpleArea(
                                nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab )
                            == SC_MARK_SIMPLE);

                    if ( bSelected )
                    {
                        if (nStartCol==nEndCol && nStartRow==nEndRow)
                            bSelected = false;
                    }
                    else
                    {
                        nStartCol = GetViewData().GetCurX();
                        nStartRow = GetViewData().GetCurY();
                        nStartTab = GetViewData().GetTabNo();
                    }

                    ScDBData* pDBData = bSelected
                                ? rDoc.GetDBAtArea( nStartTab, nStartCol, nStartRow, nEndCol, nEndRow )
                                : rDoc.GetDBAtCursor( nStartCol, nStartRow, nStartTab, ScDBDataPortion::AREA );

                    if ( pDBData )
                    {
                        ScQueryParam aParam;
                        pDBData->GetQueryParam( aParam );
                        if ( aParam.GetEntry(0).bDoQuery )
                            bAnyQuery = true;
                    }

                    if ( !bAnyQuery )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_DEFINE_DBNAME:
                {
                    if ( pDocSh->IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
            case SID_DATA_PROVIDER:
            break;
            case SID_DATA_PROVIDER_REFRESH:
            {
                ScDocument& rViewDoc = GetViewData().GetDocument();
                auto& rDataMapper = rViewDoc.GetExternalDataMapper();
                if (rDataMapper.getDataSources().empty())
                    rSet.DisableItem(nWhich);
            }
            break;
            case SID_DATA_STREAMS:
            case SID_DATA_STREAMS_PLAY:
            case SID_DATA_STREAMS_STOP:
                {
                    if ( !officecfg::Office::Common::Misc::ExperimentalMode::get() )
                        rSet.DisableItem( nWhich );
                }
                break;
            case SID_TEXT_TO_COLUMNS:
                {
                    ScRange aRange;
                    if ( !lcl_GetTextToColumnsRange( rData, aRange, true ) )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
            case SID_MANAGE_XML_SOURCE:
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
