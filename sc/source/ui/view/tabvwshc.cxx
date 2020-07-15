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
#include <sfx2/childwin.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/editview.hxx>
#include <inputhdl.hxx>

#include <tabvwsh.hxx>
#include <sc.hrc>
#include <scres.hrc>
#include <global.hxx>
#include <scmod.hxx>
#include <document.hxx>
#include <uiitems.hxx>
#include <namedlg.hxx>
#include <namedefdlg.hxx>
#include <solvrdlg.hxx>
#include <optsolver.hxx>
#include <tabopdlg.hxx>
#include <consdlg.hxx>
#include <filtdlg.hxx>
#include <dbnamdlg.hxx>
#include <areasdlg.hxx>
#include <crnrdlg.hxx>
#include <formula.hxx>
#include <highred.hxx>
#include <simpref.hxx>
#include <funcdesc.hxx>
#include <dpobject.hxx>
#include <markdata.hxx>
#include <reffact.hxx>
#include <condformatdlg.hxx>
#include <xmlsourcedlg.hxx>
#include <condformatdlgitem.hxx>
#include <formdata.hxx>
#include <inputwin.hxx>

#include <RandomNumberGeneratorDialog.hxx>
#include <SamplingDialog.hxx>
#include <DescriptiveStatisticsDialog.hxx>
#include <AnalysisOfVarianceDialog.hxx>
#include <CorrelationDialog.hxx>
#include <CovarianceDialog.hxx>
#include <ExponentialSmoothingDialog.hxx>
#include <MovingAverageDialog.hxx>
#include <RegressionDialog.hxx>
#include <TTestDialog.hxx>
#include <FTestDialog.hxx>
#include <ZTestDialog.hxx>
#include <ChiSquareTestDialog.hxx>
#include <FourierAnalysisDialog.hxx>

#include <PivotLayoutDialog.hxx>

#include <comphelper/lok.hxx>
#include <o3tl/make_shared.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

void ScTabViewShell::SetCurRefDlgId( sal_uInt16 nNew )
{
    //  CurRefDlgId is stored in ScModule to find if a ref dialog is open,
    //  and in the view to identify the view that has opened the dialog
    nCurRefDlgId = nNew;
}

//ugly hack to call Define Name from Manage Names
void ScTabViewShell::SwitchBetweenRefDialogs(SfxModelessDialogController* pDialog)
{
   sal_uInt16 nSlotId = SC_MOD()->GetCurRefDlgId();
   if( nSlotId == FID_ADD_NAME )
   {
        static_cast<ScNameDefDlg*>(pDialog)->GetNewData(maName, maScope);
        static_cast<ScNameDefDlg*>(pDialog)->Close();
        sal_uInt16 nId  = ScNameDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = GetViewFrame();
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

        SC_MOD()->SetRefDialog( nId, pWnd == nullptr );
   }
   else if (nSlotId == FID_DEFINE_NAME)
   {
        mbInSwitch = true;
        static_cast<ScNameDlg*>(pDialog)->GetRangeNames(m_RangeMap);
        static_cast<ScNameDlg*>(pDialog)->Close();
        sal_uInt16 nId  = ScNameDefDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = GetViewFrame();
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

        SC_MOD()->SetRefDialog( nId, pWnd == nullptr );
   }
}

std::shared_ptr<SfxModelessDialogController> ScTabViewShell::CreateRefDialogController(
                                SfxBindings* pB, SfxChildWindow* pCW,
                                const SfxChildWinInfo* pInfo,
                                weld::Window* pParent, sal_uInt16 nSlotId)
{
    // only open dialog when called through ScModule::SetRefDialog,
    // so that it does not re appear for instance after a crash (#42341#).

    if ( SC_MOD()->GetCurRefDlgId() != nSlotId )
        return nullptr;

    if ( nCurRefDlgId != nSlotId )
    {
        if (!(comphelper::LibreOfficeKit::isActive() && nSlotId == SID_OPENDLG_FUNCTION))
        {
            //  the dialog has been opened in a different view
            //  -> lock the dispatcher for this view (modal mode)

            GetViewData().GetDispatcher().Lock( true );    // lock is reset when closing dialog
        }
        return nullptr;
    }

    std::shared_ptr<SfxModelessDialogController> xResult;

    if(pCW)
        pCW->SetHideNotDelete(true);

    ScDocument& rDoc = GetViewData().GetDocument();

    switch( nSlotId )
    {
        case SID_CORRELATION_DIALOG:
            xResult = std::make_shared<ScCorrelationDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_SAMPLING_DIALOG:
            xResult = std::make_shared<ScSamplingDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_DESCRIPTIVE_STATISTICS_DIALOG:
            xResult = std::make_shared<ScDescriptiveStatisticsDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_ANALYSIS_OF_VARIANCE_DIALOG:
            xResult = std::make_shared<ScAnalysisOfVarianceDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_COVARIANCE_DIALOG:
            xResult = std::make_shared<ScCovarianceDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_EXPONENTIAL_SMOOTHING_DIALOG:
            xResult = std::make_shared<ScExponentialSmoothingDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_MOVING_AVERAGE_DIALOG:
            xResult = std::make_shared<ScMovingAverageDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_REGRESSION_DIALOG:
            xResult = std::make_shared<ScRegressionDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_FTEST_DIALOG:
            xResult = std::make_shared<ScFTestDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_TTEST_DIALOG:
            xResult = std::make_shared<ScTTestDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_ZTEST_DIALOG:
            xResult = std::make_shared<ScZTestDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_CHI_SQUARE_TEST_DIALOG:
            xResult = std::make_shared<ScChiSquareTestDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_FOURIER_ANALYSIS_DIALOG:
            xResult = std::make_shared<ScFourierAnalysisDialog>(pB, pCW, pParent, GetViewData());
            break;
        case WID_SIMPLE_REF:
        {
            // dialog checks, what is in the cell

            ScViewData& rViewData = GetViewData();
            rViewData.SetRefTabNo( rViewData.GetTabNo() );
            xResult = std::make_shared<ScSimpleRefDlg>(pB, pCW, pParent);
            break;
        }
        case FID_DEFINE_NAME:
        {
            if (!mbInSwitch)
            {
                xResult = std::make_shared<ScNameDlg>(pB, pCW, pParent, GetViewData(),
                                     ScAddress( GetViewData().GetCurX(),
                                                GetViewData().GetCurY(),
                                                GetViewData().GetTabNo() ) );
            }
            else
            {
                xResult = std::make_shared<ScNameDlg>( pB, pCW, pParent, GetViewData(),
                                     ScAddress( GetViewData().GetCurX(),
                                                GetViewData().GetCurY(),
                                                GetViewData().GetTabNo() ), &m_RangeMap);
                static_cast<ScNameDlg*>(xResult.get())->SetEntry(maName, maScope);
                mbInSwitch = false;
            }
            break;
        }
        case FID_ADD_NAME:
        {
            if (!mbInSwitch)
            {
                std::map<OUString, ScRangeName*> aRangeMap;
                rDoc.GetRangeNameMap(aRangeMap);
                xResult = std::make_shared<ScNameDefDlg>(pB, pCW, pParent, GetViewData(), aRangeMap,
                                ScAddress(GetViewData().GetCurX(),
                                          GetViewData().GetCurY(),
                                          GetViewData().GetTabNo()), true);
            }
            else
            {
                std::map<OUString, ScRangeName*> aRangeMap;
                for (auto const& itr : m_RangeMap)
                {
                    aRangeMap.insert(std::pair<OUString, ScRangeName*>(itr.first, itr.second.get()));
                }
                xResult = std::make_shared<ScNameDefDlg>(pB, pCW, pParent, GetViewData(), aRangeMap,
                                ScAddress(GetViewData().GetCurX(),
                                          GetViewData().GetCurY(),
                                          GetViewData().GetTabNo()), false);
            }
            break;
        }
        case SID_RANDOM_NUMBER_GENERATOR_DIALOG:
            xResult = std::make_shared<ScRandomNumberGeneratorDialog>(pB, pCW, pParent, GetViewData());
            break;
        case SID_DEFINE_DBNAME:
        {
            // when called for an existing range, then mark
            GetDBData( true, SC_DB_OLD );
            const ScMarkData& rMark = GetViewData().GetMarkData();
            if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
                MarkDataArea( false );

            xResult = std::make_shared<ScDbNameDlg>(pB, pCW, pParent, GetViewData());
            break;
        }
        case SID_OPENDLG_EDIT_PRINTAREA:
            xResult = std::make_shared<ScPrintAreasDlg>(pB, pCW, pParent);
            break;
        case SID_DEFINE_COLROWNAMERANGES:
            xResult = std::make_shared<ScColRowNameRangesDlg>(pB, pCW, pParent, GetViewData());
            break;
        case SID_OPENDLG_SOLVE:
        {
            ScViewData& rViewData = GetViewData();
            ScAddress   aCurPos( rViewData.GetCurX(),
                                 rViewData.GetCurY(),
                                 rViewData.GetTabNo());
            xResult = std::make_shared<ScSolverDlg>(pB, pCW, pParent, &rViewData.GetDocument(), aCurPos);
            break;
        }
        case SID_OPENDLG_TABOP:
        {
            ScViewData&   rViewData  = GetViewData();
            ScRefAddress  aCurPos   ( rViewData.GetCurX(),
                                      rViewData.GetCurY(),
                                      rViewData.GetTabNo());

            xResult = std::make_shared<ScTabOpDlg>(pB, pCW, pParent, &rViewData.GetDocument(), aCurPos);
            break;
        }
        case SID_OPENDLG_CONSOLIDATE:
        {
            SfxItemSet aArgSet( GetPool(),
                                svl::Items<SCITEM_CONSOLIDATEDATA,
                                SCITEM_CONSOLIDATEDATA>{} );

            const ScConsolidateParam* pDlgData =
                            rDoc.GetConsolidateDlgData();

            if ( !pDlgData )
            {
                ScConsolidateParam  aConsParam;
                SCCOL nStartCol, nEndCol;
                SCROW nStartRow, nEndRow;
                SCTAB nStartTab, nEndTab;

                GetViewData().GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                              nEndCol,   nEndRow,   nEndTab );

                PutInOrder( nStartCol, nEndCol );
                PutInOrder( nStartRow, nEndRow );
                PutInOrder( nStartTab, nEndTab );

                aConsParam.nCol = nStartCol;
                aConsParam.nRow = nStartRow;
                aConsParam.nTab = nStartTab;

                aArgSet.Put( ScConsolidateItem( SCITEM_CONSOLIDATEDATA,
                                                &aConsParam ) );
            }
            else
            {
                aArgSet.Put( ScConsolidateItem( SCITEM_CONSOLIDATEDATA, pDlgData ) );
            }
            xResult = std::make_shared<ScConsolidateDlg>(pB, pCW, pParent, aArgSet);
            break;
        }
        case SID_FILTER:
        {

            ScQueryParam    aQueryParam;
            SfxItemSet      aArgSet( GetPool(),
                                     svl::Items<SCITEM_QUERYDATA,
                                     SCITEM_QUERYDATA>{} );

            ScDBData* pDBData = GetDBData(false, SC_DB_MAKE, ScGetDBSelection::RowDown);
            pDBData->ExtendDataArea(rDoc);
            pDBData->GetQueryParam( aQueryParam );

            ScRange aArea;
            pDBData->GetArea(aArea);
            MarkRange(aArea, false);

            aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA,
                                      &GetViewData(),
                                      &aQueryParam ) );

            // mark current sheet (due to RefInput in dialog)
            GetViewData().SetRefTabNo( GetViewData().GetTabNo() );

            xResult = std::make_shared<ScFilterDlg>(pB, pCW, pParent, aArgSet);
            break;
        }
        case SID_SPECIAL_FILTER:
        {
            ScQueryParam    aQueryParam;
            SfxItemSet      aArgSet( GetPool(),
                                     svl::Items<SCITEM_QUERYDATA,
                                     SCITEM_QUERYDATA>{} );

            ScDBData* pDBData = GetDBData(false, SC_DB_MAKE, ScGetDBSelection::RowDown);
            pDBData->ExtendDataArea(rDoc);
            pDBData->GetQueryParam( aQueryParam );

            ScRange aArea;
            pDBData->GetArea(aArea);
            MarkRange(aArea, false);

            ScQueryItem aItem( SCITEM_QUERYDATA, &GetViewData(), &aQueryParam );
            ScRange aAdvSource;
            if (pDBData->GetAdvancedQuerySource(aAdvSource))
                aItem.SetAdvancedQuerySource( &aAdvSource );

            aArgSet.Put( aItem );

            // mark current sheet (due to RefInput in dialog)
            GetViewData().SetRefTabNo( GetViewData().GetTabNo() );

            xResult = std::make_shared<ScSpecialFilterDlg>(pB, pCW, pParent, aArgSet);
            break;
        }
        case SID_OPENDLG_OPTSOLVER:
        {
            ScViewData& rViewData = GetViewData();
            ScAddress aCurPos( rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo());
            xResult = std::make_shared<ScOptSolverDlg>(pB, pCW, pParent, rViewData.GetDocShell(), aCurPos);
            break;
        }
        case FID_CHG_SHOW:
        {
            // dialog checks, what is in the cell
            xResult = std::make_shared<ScHighlightChgDlg>(pB, pCW, pParent, GetViewData());
            break;
        }
        case SID_MANAGE_XML_SOURCE:
        {
            xResult = std::make_shared<ScXMLSourceDlg>(pB, pCW, pParent, &rDoc);
            break;
        }
        case SID_OPENDLG_PIVOTTABLE:
        {
            // all settings must be in pDialogDPObject

            if( pDialogDPObject )
            {
                // Check for an existing datapilot output.
                ScViewData& rViewData = GetViewData();
                rViewData.SetRefTabNo( rViewData.GetTabNo() );
                ScDPObject* pObj = rDoc.GetDPAtCursor(rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo());
                xResult = std::make_shared<ScPivotLayoutDialog>(pB, pCW, pParent, &rViewData, pDialogDPObject.get(), pObj == nullptr);
            }

            break;
        }
        case SID_OPENDLG_FUNCTION:
        {
            if (!isLOKMobilePhone())
            {
                // dialog checks, what is in the cell
                xResult = o3tl::make_shared<ScFormulaDlg>(pB, pCW, pParent, GetViewData(), ScGlobal::GetStarCalcFunctionMgr());
            }
            break;
        }
        case WID_CONDFRMT_REF:
        {
            const ScCondFormatDlgItem* pDlgItem = nullptr;
            // Get the pool item stored by Conditional Format Manager Dialog.
            auto itemsRange = GetPool().GetItemSurrogates(SCITEM_CONDFORMATDLGDATA);
            if (itemsRange.begin() != itemsRange.end())
            {
                const SfxPoolItem* pItem = *itemsRange.begin();
                pDlgItem = static_cast<const ScCondFormatDlgItem*>(pItem);
            }

            if (pDlgItem)
            {
                ScViewData& rViewData = GetViewData();
                rViewData.SetRefTabNo( rViewData.GetTabNo() );

                xResult = std::make_shared<ScCondFormatDlg>(pB, pCW, pParent, &rViewData, pDlgItem);

                // Remove the pool item stored by Conditional Format Manager Dialog.
                GetPool().Remove(*pDlgItem);
            }

            break;
        }
    }

    if (xResult)
        xResult->Initialize( pInfo );
    return xResult;
}

int ScTabViewShell::getPart() const
{
    return GetViewData().GetTabNo();
}

void ScTabViewShell::afterCallbackRegistered()
{
    UpdateInputHandler(true, false);

    ScInputHandler* pHdl = mpInputHandler ? mpInputHandler.get() : SC_MOD()->GetInputHdl();
    if (pHdl)
    {
        ScInputWindow* pInputWindow = pHdl->GetInputWindow();
        if (pInputWindow)
        {
            pInputWindow->NotifyLOKClient();
        }
    }
}

void ScTabViewShell::NotifyCursor(SfxViewShell* pOtherShell) const
{
    ScDrawView* pDrView = const_cast<ScTabViewShell*>(this)->GetScDrawView();
    if (pDrView)
    {
        if (pDrView->GetTextEditObject())
        {
            // Blinking cursor.
            EditView& rEditView = pDrView->GetTextEditOutlinerView()->GetEditView();
            rEditView.RegisterOtherShell(pOtherShell);
            rEditView.ShowCursor();
            rEditView.RegisterOtherShell(nullptr);
            // Text selection, if any.
            rEditView.DrawSelectionXOR(pOtherShell);
        }
        else
        {
            // Graphic selection.
            pDrView->AdjustMarkHdl(pOtherShell);
        }
    }

    const ScGridWindow* pWin = GetViewData().GetActiveWin();
    if (pWin)
        pWin->updateKitCellCursor(pOtherShell);
}

css::uno::Reference<css::datatransfer::XTransferable2> ScTabViewShell::GetClipData(vcl::Window* pWin)
{
    SfxViewFrame* pViewFrame = nullptr;
    css::uno::Reference<css::datatransfer::XTransferable2> xTransferable;
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> xClipboard;

    if (pWin)
        xClipboard = pWin->GetClipboard();
    else if ((pViewFrame = SfxViewFrame::GetFirst(nullptr, false)))
        xClipboard = pViewFrame->GetWindow().GetClipboard();

    xTransferable.set(xClipboard.is() ? xClipboard->getContents() : nullptr, css::uno::UNO_QUERY);

    return xTransferable;
}

void ScTabViewShell::notifyAllViewsHeaderInvalidation(const SfxViewShell* pForViewShell, HeaderType eHeaderType, SCTAB nCurrentTabIndex)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    OString aPayload;
    switch (eHeaderType)
    {
        case COLUMN_HEADER:
            aPayload = "column";
            break;
        case ROW_HEADER:
            aPayload = "row";
            break;
        case BOTH_HEADERS:
        default:
            aPayload = "all";
            break;
    }

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
        if (pTabViewShell && pViewShell->GetDocId() == pForViewShell->GetDocId()
            && (nCurrentTabIndex == -1 || pTabViewShell->getPart() == nCurrentTabIndex))
        {
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_HEADER, aPayload.getStr());
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

bool ScTabViewShell::isAnyEditViewInRange(const SfxViewShell* pForViewShell, bool bColumns, SCCOLROW nStart, SCCOLROW nEnd)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        SfxViewShell* pViewShell = SfxViewShell::GetFirst();
        while (pViewShell)
        {
            ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
            if (pTabViewShell && pTabViewShell->GetDocId() == pForViewShell->GetDocId())
            {
                ScInputHandler* pInputHandler = pTabViewShell->GetInputHandler();
                if (pInputHandler && pInputHandler->GetActiveView())
                {
                    const ScViewData& rViewData = pTabViewShell->GetViewData();
                    SCCOLROW nPos = bColumns ? rViewData.GetCurX() : rViewData.GetCurY();
                    if (nStart <= nPos && nPos <= nEnd)
                        return true;
                }
            }
            pViewShell = SfxViewShell::GetNext(*pViewShell);
        }
    }
    return false;
}

void ScTabViewShell::notifyAllViewsSheetGeomInvalidation(const SfxViewShell* pForViewShell, bool bColumns,
                                                         bool bRows, bool bSizes, bool bHidden, bool bFiltered,
                                                         bool bGroups, SCTAB nCurrentTabIndex)
{
    if (!comphelper::LibreOfficeKit::isActive() ||
            !comphelper::LibreOfficeKit::isCompatFlagSet(
                comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs))
        return;

    if (!bColumns && !bRows)
        return;

    bool bAllTypes = bSizes && bHidden && bFiltered && bGroups;
    bool bAllDims = bColumns && bRows;
    OString aPayload = bAllDims ? "all" : bColumns ? "columns" : "rows";

    if (!bAllTypes)
    {
        if (bSizes)
            aPayload += " sizes";

        if (bHidden)
            aPayload += " hidden";

        if (bFiltered)
            aPayload += " filtered";

        if (bGroups)
            aPayload += " groups";
    }

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
        if (pTabViewShell && pViewShell->GetDocId() == pForViewShell->GetDocId() &&
                (nCurrentTabIndex == -1 || pTabViewShell->getPart() == nCurrentTabIndex))
        {
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_SHEET_GEOMETRY, aPayload.getStr());
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

bool ScTabViewShell::UseSubTotal(ScRangeList* pRangeList)
{
    bool bSubTotal = false;
    ScDocument& rDoc = GetViewData().GetDocument();
    size_t nRangeCount (pRangeList->size());
    size_t nRangeIndex (0);
    while (!bSubTotal && nRangeIndex < nRangeCount)
    {
        const ScRange& rRange = (*pRangeList)[nRangeIndex];
        SCTAB nTabEnd(rRange.aEnd.Tab());
        SCTAB nTab(rRange.aStart.Tab());
        while (!bSubTotal && nTab <= nTabEnd)
        {
            SCROW nRowEnd(rRange.aEnd.Row());
            SCROW nRow(rRange.aStart.Row());
            while (!bSubTotal && nRow <= nRowEnd)
            {
                if (rDoc.RowFiltered(nRow, nTab))
                    bSubTotal = true;
                else
                    ++nRow;
            }
            ++nTab;
        }
        ++nRangeIndex;
    }

    if (!bSubTotal)
    {
        const ScDBCollection::NamedDBs& rDBs = rDoc.GetDBCollection()->getNamedDBs();
        for (const auto& rxDB : rDBs)
        {
            const ScDBData& rDB = *rxDB;
            if (!rDB.HasAutoFilter())
                continue;

            nRangeIndex = 0;
            while (!bSubTotal && nRangeIndex < nRangeCount)
            {
                const ScRange & rRange = (*pRangeList)[nRangeIndex];
                ScRange aDBArea;
                rDB.GetArea(aDBArea);
                if (aDBArea.Intersects(rRange))
                    bSubTotal = true;
                ++nRangeIndex;
            }

            if (bSubTotal)
                break;
        }
    }
    return bSubTotal;
}

OUString ScTabViewShell::DoAutoSum(bool& rRangeFinder, bool& rSubTotal, const OpCode eCode)
{
    OUString aFormula;
    const ScMarkData& rMark = GetViewData().GetMarkData();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        ScRangeList aMarkRangeList;
        rRangeFinder = rSubTotal = false;
        rMark.FillRangeListWithMarks( &aMarkRangeList, false );
        ScDocument& rDoc = GetViewData().GetDocument();

        // check if one of the marked ranges is empty
        bool bEmpty = false;
        const size_t nCount = aMarkRangeList.size();
        for ( size_t i = 0; i < nCount; ++i )
        {
            const ScRange & rRange( aMarkRangeList[i] );
            if ( rDoc.IsBlockEmpty( rRange.aStart.Tab(),
                 rRange.aStart.Col(), rRange.aStart.Row(),
                 rRange.aEnd.Col(), rRange.aEnd.Row() ) )
            {
                bEmpty = true;
                break;
            }
        }

        if ( bEmpty )
        {
            ScRangeList aRangeList;
            const bool bDataFound = GetAutoSumArea( aRangeList );
            if ( bDataFound )
            {
                ScAddress aAddr = aRangeList.back().aEnd;
                aAddr.IncRow();
                const bool bSubTotal( UseSubTotal( &aRangeList ) );
                EnterAutoSum( aRangeList, bSubTotal, aAddr, eCode );
            }
        }
        else
        {
            const bool bSubTotal( UseSubTotal( &aMarkRangeList ) );
            for ( size_t i = 0; i < nCount; ++i )
            {
                const ScRange & rRange = aMarkRangeList[i];
                const bool bSetCursor = ( i == nCount - 1 );
                const bool bContinue = ( i != 0 );
                if ( !AutoSum( rRange, bSubTotal, bSetCursor, bContinue, eCode ) )
                {
                    MarkRange( rRange, false );
                    SetCursor( rRange.aEnd.Col(), rRange.aEnd.Row() );
                    const ScRangeList aRangeList;
                    ScAddress aAddr = rRange.aEnd;
                    aAddr.IncRow();
                    aFormula = GetAutoSumFormula( aRangeList, bSubTotal, aAddr , eCode);
                    break;
                }
            }
        }
    }
    else // Only insert into input row
    {
        ScRangeList aRangeList;
        rRangeFinder = GetAutoSumArea( aRangeList );
        rSubTotal = UseSubTotal( &aRangeList );
        ScAddress aAddr = GetViewData().GetCurPos();
        aFormula = GetAutoSumFormula( aRangeList, rSubTotal, aAddr , eCode);
    }
    return aFormula;
}

void ScTabViewShell::InitFormEditData()
{
    mpFormEditData.reset(new ScFormEditData);
}

void ScTabViewShell::ClearFormEditData()
{
    mpFormEditData.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
