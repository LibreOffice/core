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

#include <config_mpl.h>

#include "scitems.hxx"
#include <vcl/msgbox.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/editview.hxx>
#include <inputhdl.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "scres.hrc"
#include "globstr.hrc"
#include "global.hxx"
#include "scmod.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "pivot.hxx"
#include "namedlg.hxx"
#include "namedefdlg.hxx"
#include "solvrdlg.hxx"
#include "optsolver.hxx"
#include "tabopdlg.hxx"
#include "autoform.hxx"
#include "autofmt.hxx"
#include "consdlg.hxx"
#include "filtdlg.hxx"
#include "dbnamdlg.hxx"
#include "areasdlg.hxx"
#include "rangeutl.hxx"
#include "crnrdlg.hxx"
#include "formula.hxx"
#include "formulacell.hxx"
#include "acredlin.hxx"
#include "highred.hxx"
#include "simpref.hxx"
#include "funcdesc.hxx"
#include "dpobject.hxx"
#include "markdata.hxx"
#include "reffact.hxx"
#include "condformatdlg.hxx"
#include "xmlsourcedlg.hxx"
#include "condformatdlgitem.hxx"

#include "RandomNumberGeneratorDialog.hxx"
#include "SamplingDialog.hxx"
#include "DescriptiveStatisticsDialog.hxx"
#include "AnalysisOfVarianceDialog.hxx"
#include "CorrelationDialog.hxx"
#include "CovarianceDialog.hxx"
#include "ExponentialSmoothingDialog.hxx"
#include "MovingAverageDialog.hxx"
#include "RegressionDialog.hxx"
#include "TTestDialog.hxx"
#include "FTestDialog.hxx"
#include "ZTestDialog.hxx"
#include "ChiSquareTestDialog.hxx"

#include "PivotLayoutDialog.hxx"

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/lokhelper.hxx>

void ScTabViewShell::SetCurRefDlgId( sal_uInt16 nNew )
{
    //  CurRefDlgId is stored in ScModule to find if a ref dialog is open,
    //  and in the view to identify the view that has opened the dialog
    nCurRefDlgId = nNew;
}

//ugly hack to call Define Name from Manage Names
void ScTabViewShell::SwitchBetweenRefDialogs(SfxModelessDialog* pDialog)
{
   sal_uInt16 nSlotId = SC_MOD()->GetCurRefDlgId();
   if (nSlotId == FID_DEFINE_NAME)
   {
        mbInSwitch = true;
        static_cast<ScNameDlg*>(pDialog)->GetRangeNames(m_RangeMap);
        static_cast<ScNameDlg*>(pDialog)->Close();
        sal_uInt16 nId  = ScNameDefDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = GetViewFrame();
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

        SC_MOD()->SetRefDialog( nId, pWnd == nullptr );
   }
   else if( nSlotId == FID_ADD_NAME )
   {
        static_cast<ScNameDefDlg*>(pDialog)->GetNewData(maName, maScope);
        static_cast<ScNameDlg*>(pDialog)->Close();
        sal_uInt16 nId  = ScNameDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = GetViewFrame();
        SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

        SC_MOD()->SetRefDialog( nId, pWnd == nullptr );
   }
   else
   {

   }
}

VclPtr<SfxModelessDialog> ScTabViewShell::CreateRefDialog(
                                SfxBindings* pB, SfxChildWindow* pCW,
                                SfxChildWinInfo* pInfo,
                                vcl::Window* pParent, sal_uInt16 nSlotId )
{
    // only open dialog when called through ScModule::SetRefDialog,
    // so that it does not re appear for instance after a crash (#42341#).

    if ( SC_MOD()->GetCurRefDlgId() != nSlotId )
        return nullptr;

    if ( nCurRefDlgId != nSlotId )
    {
        //  the dialog has been opened in a different view
        //  -> lock the dispatcher for this view (modal mode)

        GetViewData().GetDispatcher().Lock( true );    // lock is reset when closing dialog
        return nullptr;
    }

    VclPtr<SfxModelessDialog> pResult;

    if(pCW)
        pCW->SetHideNotDelete(true);

    ScDocument* pDoc = GetViewData().GetDocument();

    switch( nSlotId )
    {
        case FID_DEFINE_NAME:
        {
            if (!mbInSwitch)
            {
                pResult = VclPtr<ScNameDlg>::Create( pB, pCW, pParent, &GetViewData(),
                                     ScAddress( GetViewData().GetCurX(),
                                                GetViewData().GetCurY(),
                                                GetViewData().GetTabNo() ) );
            }
            else
            {
                pResult = VclPtr<ScNameDlg>::Create( pB, pCW, pParent, &GetViewData(),
                                     ScAddress( GetViewData().GetCurX(),
                                                GetViewData().GetCurY(),
                                                GetViewData().GetTabNo() ), &m_RangeMap);
                static_cast<ScNameDlg*>(pResult.get())->SetEntry( maName, maScope);
                mbInSwitch = false;
            }
        }
        break;

        case FID_ADD_NAME:
        {
            if (!mbInSwitch)
            {
                std::map<OUString, ScRangeName*> aRangeMap;
                pDoc->GetRangeNameMap(aRangeMap);
                pResult = VclPtr<ScNameDefDlg>::Create( pB, pCW, pParent, &GetViewData(), aRangeMap,
                                ScAddress( GetViewData().GetCurX(),
                                            GetViewData().GetCurY(),
                                            GetViewData().GetTabNo() ), true );
            }
            else
            {
                std::map<OUString, ScRangeName*> aRangeMap;
                for (auto const& itr : m_RangeMap)
                {
                    aRangeMap.insert(std::pair<OUString, ScRangeName*>(itr.first, itr.second.get()));
                }
                pResult = VclPtr<ScNameDefDlg>::Create( pB, pCW, pParent, &GetViewData(), aRangeMap,
                                ScAddress( GetViewData().GetCurX(),
                                            GetViewData().GetCurY(),
                                            GetViewData().GetTabNo() ), false );
            }
        }
        break;

        case SID_DEFINE_COLROWNAMERANGES:
        {
            pResult = VclPtr<ScColRowNameRangesDlg>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_OPENDLG_CONSOLIDATE:
        {
            SfxItemSet aArgSet( GetPool(),
                                svl::Items<SCITEM_CONSOLIDATEDATA,
                                SCITEM_CONSOLIDATEDATA>{} );

            const ScConsolidateParam* pDlgData =
                            pDoc->GetConsolidateDlgData();

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
            pResult = VclPtr<ScConsolidateDlg>::Create( pB, pCW, pParent, aArgSet );
        }
        break;

        case SID_DEFINE_DBNAME:
        {
            // when called for an existing range, then mark
            GetDBData( true, SC_DB_OLD );
            const ScMarkData& rMark = GetViewData().GetMarkData();
            if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
                MarkDataArea( false );

            pResult = VclPtr<ScDbNameDlg>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_SPECIAL_FILTER:
        {
            ScQueryParam    aQueryParam;
            SfxItemSet      aArgSet( GetPool(),
                                     svl::Items<SCITEM_QUERYDATA,
                                     SCITEM_QUERYDATA>{} );

            ScDBData* pDBData = GetDBData(false, SC_DB_MAKE, ScGetDBSelection::RowDown);
            pDBData->ExtendDataArea(pDoc);
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

            pResult = VclPtr<ScSpecialFilterDlg>::Create( pB, pCW, pParent, aArgSet );
        }
        break;

        case SID_FILTER:
        {

            ScQueryParam    aQueryParam;
            SfxItemSet      aArgSet( GetPool(),
                                     svl::Items<SCITEM_QUERYDATA,
                                     SCITEM_QUERYDATA>{} );

            ScDBData* pDBData = GetDBData(false, SC_DB_MAKE, ScGetDBSelection::RowDown);
            pDBData->ExtendDataArea(pDoc);
            pDBData->GetQueryParam( aQueryParam );

            ScRange aArea;
            pDBData->GetArea(aArea);
            MarkRange(aArea, false);

            aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA,
                                      &GetViewData(),
                                      &aQueryParam ) );

            // mark current sheet (due to RefInput in dialog)
            GetViewData().SetRefTabNo( GetViewData().GetTabNo() );

            pResult = VclPtr<ScFilterDlg>::Create( pB, pCW, pParent, aArgSet );
        }
        break;

        case SID_OPENDLG_TABOP:
        {
            ScViewData&   rViewData  = GetViewData();
            ScRefAddress  aCurPos   ( rViewData.GetCurX(),
                                      rViewData.GetCurY(),
                                      rViewData.GetTabNo());

            pResult = VclPtr<ScTabOpDlg>::Create( pB, pCW, pParent, rViewData.GetDocument(), aCurPos );
        }
        break;

        case SID_OPENDLG_SOLVE:
        {
            ScViewData& rViewData = GetViewData();
            ScAddress   aCurPos( rViewData.GetCurX(),
                                 rViewData.GetCurY(),
                                 rViewData.GetTabNo());
            pResult = VclPtr<ScSolverDlg>::Create( pB, pCW, pParent, rViewData.GetDocument(), aCurPos );
        }
        break;

        case SID_RANDOM_NUMBER_GENERATOR_DIALOG:
        {
            pResult = VclPtr<ScRandomNumberGeneratorDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_SAMPLING_DIALOG:
        {
            pResult = VclPtr<ScSamplingDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_DESCRIPTIVE_STATISTICS_DIALOG:
        {
            pResult = VclPtr<ScDescriptiveStatisticsDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_ANALYSIS_OF_VARIANCE_DIALOG:
        {
            pResult = VclPtr<ScAnalysisOfVarianceDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_CORRELATION_DIALOG:
        {
            pResult = VclPtr<ScCorrelationDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_COVARIANCE_DIALOG:
        {
            pResult = VclPtr<ScCovarianceDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_EXPONENTIAL_SMOOTHING_DIALOG:
        {
            pResult = VclPtr<ScExponentialSmoothingDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_MOVING_AVERAGE_DIALOG:
        {
            pResult = VclPtr<ScMovingAverageDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_REGRESSION_DIALOG:
        {
            pResult = VclPtr<ScRegressionDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_TTEST_DIALOG:
        {
            pResult = VclPtr<ScTTestDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_FTEST_DIALOG:
        {
            pResult = VclPtr<ScFTestDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_ZTEST_DIALOG:
        {
            pResult = VclPtr<ScZTestDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_CHI_SQUARE_TEST_DIALOG:
        {
            pResult = VclPtr<ScChiSquareTestDialog>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case SID_OPENDLG_OPTSOLVER:
        {
            ScViewData& rViewData = GetViewData();
            ScAddress aCurPos( rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo());
            pResult = VclPtr<ScOptSolverDlg>::Create( pB, pCW, pParent, rViewData.GetDocShell(), aCurPos );
        }
        break;

        case SID_OPENDLG_PIVOTTABLE:
        {
            // all settings must be in pDialogDPObject

            if( pDialogDPObject )
            {
                // Check for an existing datapilot output.
                ScViewData& rViewData = GetViewData();
                rViewData.SetRefTabNo( rViewData.GetTabNo() );
                ScDPObject* pObj = pDoc->GetDPAtCursor(rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo());
                pResult = VclPtr<ScPivotLayoutDialog>::Create(pB, pCW, pParent, &rViewData, pDialogDPObject, pObj == nullptr);
            }
        }
        break;

        case SID_OPENDLG_EDIT_PRINTAREA:
        {
            pResult = VclPtr<ScPrintAreasDlg>::Create( pB, pCW, pParent );
        }
        break;

        case SID_OPENDLG_FUNCTION:
        {
            // dialog checks, what is in the cell

            pResult = VclPtr<ScFormulaDlg>::Create( pB, pCW, pParent, &GetViewData(),ScGlobal::GetStarCalcFunctionMgr() );
        }
        break;

        case SID_MANAGE_XML_SOURCE:
        {
            pResult = VclPtr<ScXMLSourceDlg>::Create(pB, pCW, pParent, pDoc);
        }
        break;

        case FID_CHG_SHOW:
        {
            // dialog checks, what is in the cell

            pResult = VclPtr<ScHighlightChgDlg>::Create( pB, pCW, pParent, &GetViewData() );
        }
        break;

        case WID_SIMPLE_REF:
        {
            // dialog checks, what is in the cell

            ScViewData& rViewData = GetViewData();
            rViewData.SetRefTabNo( rViewData.GetTabNo() );
            pResult = VclPtr<ScSimpleRefDlg>::Create( pB, pCW, pParent );
        }
        break;

        case WID_CONDFRMT_REF:
        {
            bool        bFound      = false;
            const ScCondFormatDlgItem* pDlgItem = nullptr;
            // Get the pool item stored by Conditional Format Manager Dialog.
            const SfxPoolItem* pItem = nullptr;
            sal_uInt32 nItems(GetPool().GetItemCount2( SCITEM_CONDFORMATDLGDATA ));
            for( sal_uInt32 nIter = 0; nIter < nItems; ++nIter )
            {
                if( nullptr != (pItem = GetPool().GetItem2( SCITEM_CONDFORMATDLGDATA, nIter ) ) )
                {
                    pDlgItem = static_cast<const ScCondFormatDlgItem*>(pItem);
                    bFound = true;
                    break;
                }
            }

            ScViewData& rViewData = GetViewData();
            rViewData.SetRefTabNo( rViewData.GetTabNo() );

            pResult = VclPtr<ScCondFormatDlg>::Create( pB, pCW, pParent, &rViewData, pDlgItem );

            // Remove the pool item stored by Conditional Format Manager Dialog.
            if ( bFound && pItem )
                GetPool().Remove( *pItem );
        }
        break;
    }

    if (pResult)
    {
        // the dialogs are always displayed with the option button collapsed,
        // the size has to be carried over initialize
        // (or store the option status !!!)

        Size aSize = pResult->GetSizePixel();
        pResult->Initialize( pInfo );
        pResult->SetSizePixel(aSize);
    }

    return pResult;
}

int ScTabViewShell::getPart() const
{
    return GetViewData().GetTabNo();
}

void ScTabViewShell::afterCallbackRegistered()
{
    UpdateInputHandler(true, false);
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
            rEditView.DrawSelection(pOtherShell);
        }
        else
        {
            // Graphic selection.
            pDrView->AdjustMarkHdl(pOtherShell);
        }
    }

    const ScGridWindow* pWin = GetViewData().GetActiveWin();
    if (pWin)
        pWin->updateLibreOfficeKitCellCursor(pOtherShell);
}

void ScTabViewShell::notifyAllViewsHeaderInvalidation(const OString& rPayload, SCTAB nCurrentTabIndex)
{
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(pViewShell);
        if (pTabViewShell && (nCurrentTabIndex == -1 || pTabViewShell->getPart() == nCurrentTabIndex))
        {
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_INVALIDATE_HEADER, rPayload.getStr());
        }
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

bool ScTabViewShell::UseSubTotal(ScRangeList* pRangeList)
{
    bool bSubTotal = false;
    ScDocument* pDoc = GetViewData().GetDocument();
    size_t nRangeCount (pRangeList->size());
    size_t nRangeIndex (0);
    while (!bSubTotal && nRangeIndex < nRangeCount)
    {
        const ScRange* pRange = (*pRangeList)[nRangeIndex];
        if( pRange )
        {
            SCTAB nTabEnd(pRange->aEnd.Tab());
            SCTAB nTab(pRange->aStart.Tab());
            while (!bSubTotal && nTab <= nTabEnd)
            {
                SCROW nRowEnd(pRange->aEnd.Row());
                SCROW nRow(pRange->aStart.Row());
                while (!bSubTotal && nRow <= nRowEnd)
                {
                    if (pDoc->RowFiltered(nRow, nTab))
                        bSubTotal = true;
                    else
                        ++nRow;
                }
                ++nTab;
            }
        }
        ++nRangeIndex;
    }

    const ScDBCollection::NamedDBs& rDBs = pDoc->GetDBCollection()->getNamedDBs();
    ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
    for (; !bSubTotal && itr != itrEnd; ++itr)
    {
        const ScDBData& rDB = **itr;
        if (!rDB.HasAutoFilter())
            continue;

        nRangeIndex = 0;
        while (!bSubTotal && nRangeIndex < nRangeCount)
        {
            const ScRange* pRange = (*pRangeList)[nRangeIndex];
            if( pRange )
            {
                ScRange aDBArea;
                rDB.GetArea(aDBArea);
                if (aDBArea.Intersects(*pRange))
                    bSubTotal = true;
            }
            ++nRangeIndex;
        }
    }
    return bSubTotal;
}

const OUString ScTabViewShell::DoAutoSum(bool& rRangeFinder, bool& rSubTotal)
{
    OUString aFormula;
    const ScMarkData& rMark = GetViewData().GetMarkData();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        ScRangeList aMarkRangeList;
        rRangeFinder = rSubTotal = false;
        rMark.FillRangeListWithMarks( &aMarkRangeList, false );
        ScDocument* pDoc = GetViewData().GetDocument();

        // check if one of the marked ranges is empty
        bool bEmpty = false;
        const size_t nCount = aMarkRangeList.size();
        for ( size_t i = 0; i < nCount; ++i )
        {
            const ScRange aRange( *aMarkRangeList[i] );
            if ( pDoc->IsBlockEmpty( aRange.aStart.Tab(),
                 aRange.aStart.Col(), aRange.aStart.Row(),
                 aRange.aEnd.Col(), aRange.aEnd.Row() ) )
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
                ScAddress aAddr = aRangeList.back()->aEnd;
                aAddr.IncRow();
                const bool bSubTotal( UseSubTotal( &aRangeList ) );
                EnterAutoSum( aRangeList, bSubTotal, aAddr );
            }
        }
        else
        {
            const bool bSubTotal( UseSubTotal( &aMarkRangeList ) );
            for ( size_t i = 0; i < nCount; ++i )
            {
                const ScRange aRange( *aMarkRangeList[i] );
                const bool bSetCursor = ( i == nCount - 1 );
                const bool bContinue = ( i != 0 );
                if ( !AutoSum( aRange, bSubTotal, bSetCursor, bContinue ) )
                {
                    MarkRange( aRange, false );
                    SetCursor( aRange.aEnd.Col(), aRange.aEnd.Row() );
                    const ScRangeList aRangeList;
                    ScAddress aAddr = aRange.aEnd;
                    aAddr.IncRow();
                    aFormula = GetAutoSumFormula( aRangeList, bSubTotal, aAddr );
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
        aFormula = GetAutoSumFormula( aRangeList, rSubTotal, aAddr );
    }
    return aFormula;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
