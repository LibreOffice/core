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

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/Desktop.hpp>

using namespace ::com::sun::star;

#include <math.h>

#include <scitems.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/langitem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/langtab.hxx>
#include <basic/sbxcore.hxx>
#include <basic/sberrors.hxx>
#include <svtools/sfxecode.hxx>
#include <svx/ofaitem.hxx>
#include <svl/whiter.hxx>
#include <vcl/weld.hxx>
#include <vcl/waitobj.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/drawitem.hxx>
#include <svx/fmshell.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/PasswordHelper.hxx>
#include <svl/documentlockfile.hxx>
#include <svl/sharecontrolfile.hxx>
#include <svl/slstitm.hxx>
#include <unotools/securityoptions.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <docuno.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <docsh.hxx>
#include "docshimp.hxx"
#include <docfunc.hxx>
#include <scres.hrc>
#include <strings.hrc>
#include <stlsheet.hxx>
#include <stlpool.hxx>
#include <appoptio.hxx>
#include <globstr.hrc>
#include <global.hxx>
#include <dbdocfun.hxx>
#include <printfun.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <impex.hxx>
#include <attrib.hxx>
#include <undodat.hxx>
#include <autostyl.hxx>
#include <undocell.hxx>
#include <undotab.hxx>
#include <inputhdl.hxx>
#include <dbdata.hxx>
#include <servobj.hxx>
#include <rangenam.hxx>
#include <scmod.hxx>
#include <chgviset.hxx>
#include <reffact.hxx>
#include <chartlis.hxx>
#include <chartpos.hxx>
#include <waitoff.hxx>
#include <tablink.hxx>
#include <drwlayer.hxx>
#include <docoptio.hxx>
#include <undostyl.hxx>
#include <rangeseq.hxx>
#include <chgtrack.hxx>
#include <printopt.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <scresid.hxx>
#include <scabstdlg.hxx>
#include <externalrefmgr.hxx>
#include <sharedocdlg.hxx>
#include <conditio.hxx>
#include <sheetevents.hxx>
#include <formulacell.hxx>
#include <documentlinkmgr.hxx>
#include <memory>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <helpids.h>

#include <svx/xdef.hxx>

void ScDocShell::ReloadAllLinks()
{
    m_aDocument.SetLinkFormulaNeedingCheck(false);
    getEmbeddedObjectContainer().setUserAllowsLinkUpdate(true);

    ReloadTabLinks();
    vcl::Window *pDialogParent = GetActiveDialogParent();
    m_aDocument.UpdateExternalRefLinks(pDialogParent);

    bool bAnyDde = m_aDocument.GetDocLinkManager().updateDdeOrOleOrWebServiceLinks(pDialogParent ? pDialogParent->GetFrameWeld() : nullptr);

    if (bAnyDde)
    {
        //  calculate formulas and paint like in the TrackTimeHdl
        m_aDocument.TrackFormulas();
        Broadcast(SfxHint(SfxHintId::ScDataChanged));

        //  Should FID_DATACHANGED become asynchronous some time
        //  (e.g., with Invalidate at Window), an update needs to be forced here.
    }

    m_aDocument.UpdateAreaLinks();
}

IMPL_LINK_NOARG( ScDocShell, ReloadAllLinksHdl, Button*, void )
{
    ReloadAllLinks();

    ScTabViewShell* pViewSh = GetBestViewShell();
    SfxViewFrame* pViewFrame = pViewSh ? pViewSh->GetFrame() : nullptr;
    if (pViewFrame)
        pViewFrame->RemoveInfoBar("enablecontent");
    SAL_WARN_IF(!pViewFrame, "sc", "expected there to be a ViewFrame");
}

void ScDocShell::Execute( SfxRequest& rReq )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    SfxBindings* pBindings = GetViewBindings();
    bool bUndo (m_aDocument.IsUndoEnabled());

    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_SC_SETTEXT:
        {
            const SfxPoolItem* pColItem;
            const SfxPoolItem* pRowItem;
            const SfxPoolItem* pTabItem;
            const SfxPoolItem* pTextItem;
            if( pReqArgs && pReqArgs->HasItem( FN_PARAM_1, &pColItem ) &&
                            pReqArgs->HasItem( FN_PARAM_2, &pRowItem ) &&
                            pReqArgs->HasItem( FN_PARAM_3, &pTabItem ) &&
                            pReqArgs->HasItem( SID_SC_SETTEXT, &pTextItem ) )
            {
                //  parameters are  1-based !!!
                SCCOL nCol = static_cast<const SfxInt16Item*>(pColItem)->GetValue() - 1;
                SCROW nRow = static_cast<const SfxInt32Item*>(pRowItem)->GetValue() - 1;
                SCTAB nTab = static_cast<const SfxInt16Item*>(pTabItem)->GetValue() - 1;

                SCTAB nTabCount = m_aDocument.GetTableCount();
                if ( ValidCol(nCol) && ValidRow(nRow) && ValidTab(nTab,nTabCount) )
                {
                    if ( m_aDocument.IsBlockEditable( nTab, nCol,nRow, nCol, nRow ) )
                    {
                        OUString aVal = static_cast<const SfxStringItem*>(pTextItem)->GetValue();
                        m_aDocument.SetString( nCol, nRow, nTab, aVal );

                        PostPaintCell( nCol, nRow, nTab );
                        SetDocumentModified();

                        rReq.Done();
                        break;
                    }
                    else                // protected cell
                    {
#if HAVE_FEATURE_SCRIPTING
                        SbxBase::SetError( ERRCODE_BASIC_BAD_PARAMETER );      //! which error ?
#endif
                        break;
                    }
                }
            }
#if HAVE_FEATURE_SCRIPTING
            SbxBase::SetError( ERRCODE_BASIC_NO_OBJECT );
#endif
        }
        break;

        case SID_SBA_IMPORT:
        {
            if (pReqArgs)
            {
                const SfxPoolItem* pItem;
                svx::ODataAccessDescriptor aDesc;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                {
                    uno::Any aAny = static_cast<const SfxUnoAnyItem*>(pItem)->GetValue();
                    uno::Sequence<beans::PropertyValue> aProperties;
                    if ( aAny >>= aProperties )
                        aDesc.initializeFrom( aProperties );
                }

                OUString sTarget;
                if ( pReqArgs->GetItemState( FN_PARAM_1, true, &pItem ) == SfxItemState::SET )
                    sTarget = static_cast<const SfxStringItem*>(pItem)->GetValue();

                bool bIsNewArea = true;         // Default sal_True (no inquiry)
                if ( pReqArgs->GetItemState( FN_PARAM_2, true, &pItem ) == SfxItemState::SET )
                    bIsNewArea = static_cast<const SfxBoolItem*>(pItem)->GetValue();

                // if necessary, create new database area
                bool bMakeArea = false;
                if (bIsNewArea)
                {
                    ScDBCollection* pDBColl = m_aDocument.GetDBCollection();
                    if ( !pDBColl || !pDBColl->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(sTarget)) )
                    {
                        ScAddress aPos;
                        if ( aPos.Parse( sTarget, &m_aDocument, m_aDocument.GetAddressConvention() ) & ScRefFlags::VALID )
                        {
                            bMakeArea = true;
                            if (bUndo)
                            {
                                OUString aStrImport = ScResId( STR_UNDO_IMPORTDATA );
                                ViewShellId nViewShellId(-1);
                                if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
                                    nViewShellId = pViewSh->GetViewShellId();
                                GetUndoManager()->EnterListAction( aStrImport, aStrImport, 0, nViewShellId );
                            }

                            ScDBData* pDBData = GetDBData( ScRange(aPos), SC_DB_IMPORT, ScGetDBSelection::Keep );
                            OSL_ENSURE(pDBData, "Cannot create DB data");
                            sTarget = pDBData->GetName();
                        }
                    }
                }

                // inquire, before old DB range gets overwritten
                bool bDo = true;
                if (!bIsNewArea)
                {
                    OUString aTemplate = ScResId( STR_IMPORT_REPLACE );
                    OUString aMessage = aTemplate.getToken( 0, '#' );
                    aMessage += sTarget;
                    aMessage += aTemplate.getToken( 1, '#' );

                    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(nullptr,
                                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                                   aMessage));
                    xQueryBox->set_default_response(RET_YES);
                    bDo = xQueryBox->run() == RET_YES;
                }

                if (bDo)
                {
                    ScDBDocFunc(*this).UpdateImport( sTarget, aDesc );
                    rReq.Done();

                    //  UpdateImport also updates the internal operations
                }
                else
                    rReq.Ignore();

                if ( bMakeArea && bUndo)
                    GetUndoManager()->LeaveListAction();
            }
            else
            {
                OSL_FAIL( "arguments expected" );
            }
        }
        break;

        case SID_CHART_SOURCE:
        case SID_CHART_ADDSOURCE:
            if (pReqArgs)
            {
                ScDocument& rDoc = GetDocument();
                const   SfxPoolItem* pItem;
                OUString  aChartName, aRangeName;

                ScRange         aSingleRange;
                ScRangeListRef  aRangeListRef;
                bool            bMultiRange = false;

                bool bColHeaders = true;
                bool bRowHeaders = true;
                bool bColInit = false;
                bool bRowInit = false;
                bool bAddRange = (nSlot == SID_CHART_ADDSOURCE);

                if( pReqArgs->HasItem( SID_CHART_NAME, &pItem ) )
                    aChartName = static_cast<const SfxStringItem*>(pItem)->GetValue();

                if( pReqArgs->HasItem( SID_CHART_SOURCE, &pItem ) )
                    aRangeName = static_cast<const SfxStringItem*>(pItem)->GetValue();

                if( pReqArgs->HasItem( FN_PARAM_1, &pItem ) )
                {
                    bColHeaders = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    bColInit = true;
                }
                if( pReqArgs->HasItem( FN_PARAM_2, &pItem ) )
                {
                    bRowHeaders = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                    bRowInit = true;
                }

                ScAddress::Details aDetails(rDoc.GetAddressConvention(), 0, 0);
                bool bValid = (aSingleRange.ParseAny(aRangeName, &rDoc, aDetails) & ScRefFlags::VALID) == ScRefFlags::ZERO;
                if (!bValid)
                {
                    aRangeListRef = new ScRangeList;
                    aRangeListRef->Parse( aRangeName, &rDoc );
                    if ( !aRangeListRef->empty() )
                    {
                        bMultiRange = true;
                        aSingleRange = aRangeListRef->front(); // for header
                        bValid = true;
                    }
                    else
                        aRangeListRef.clear();
                }

                ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
                if (pViewSh && bValid && !aChartName.isEmpty() )
                {
                    weld::Window* pParent = pViewSh->GetFrameWeld();

                    SCCOL nCol1 = aSingleRange.aStart.Col();
                    SCROW nRow1 = aSingleRange.aStart.Row();
                    SCCOL nCol2 = aSingleRange.aEnd.Col();
                    SCROW nRow2 = aSingleRange.aEnd.Row();
                    SCTAB nTab = aSingleRange.aStart.Tab();

                    //! limit always or not at all ???
                    if (!bMultiRange)
                        m_aDocument.LimitChartArea( nTab, nCol1,nRow1, nCol2,nRow2 );

                                        // Dialog for column/row headers
                    bool bOk = true;
                    if ( !bAddRange && ( !bColInit || !bRowInit ) )
                    {
                        ScChartPositioner aChartPositioner( &m_aDocument, nTab, nCol1,nRow1, nCol2,nRow2 );
                        if (!bColInit)
                            bColHeaders = aChartPositioner.HasColHeaders();
                        if (!bRowInit)
                            bRowHeaders = aChartPositioner.HasRowHeaders();

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                        ScopedVclPtr<AbstractScColRowLabelDlg> pDlg(pFact->CreateScColRowLabelDlg(pParent, bRowHeaders, bColHeaders));
                        if ( pDlg->Execute() == RET_OK )
                        {
                            bColHeaders = pDlg->IsRow();
                            bRowHeaders = pDlg->IsCol();

                            rReq.AppendItem(SfxBoolItem(FN_PARAM_1, bColHeaders));
                            rReq.AppendItem(SfxBoolItem(FN_PARAM_2, bRowHeaders));
                        }
                        else
                            bOk = false;
                    }

                    if (bOk)            // execute
                    {
                        if (bMultiRange)
                        {
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                    new ScUndoChartData( this, aChartName, aRangeListRef,
                                                            bColHeaders, bRowHeaders, bAddRange ) );
                            }
                            m_aDocument.UpdateChartArea( aChartName, aRangeListRef,
                                                        bColHeaders, bRowHeaders, bAddRange );
                        }
                        else
                        {
                            ScRange aNewRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                    new ScUndoChartData( this, aChartName, aNewRange,
                                                            bColHeaders, bRowHeaders, bAddRange ) );
                            }
                            m_aDocument.UpdateChartArea( aChartName, aNewRange,
                                                        bColHeaders, bRowHeaders, bAddRange );
                        }
                    }
                }
                else
                {
                    OSL_FAIL("UpdateChartArea: no ViewShell or wrong data");
                }
                rReq.Done();
            }
            else
            {
                OSL_FAIL("SID_CHART_SOURCE without arguments");
            }
            break;

        case FID_AUTO_CALC:
            {
                bool bNewVal;
                const SfxPoolItem* pItem;
                if ( pReqArgs && SfxItemState::SET == pReqArgs->GetItemState( nSlot, true, &pItem ) )
                    bNewVal = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                else
                    bNewVal = !m_aDocument.GetAutoCalc();     // Toggle for menu
                m_aDocument.SetAutoCalc( bNewVal );
                SetDocumentModified();
                if (pBindings)
                {
                    pBindings->Invalidate( FID_AUTO_CALC );
                }
                rReq.AppendItem( SfxBoolItem( FID_AUTO_CALC, bNewVal ) );
                rReq.Done();
            }
            break;
        case FID_RECALC:
            DoRecalc( rReq.IsAPI() );
            rReq.Done();
            break;
        case FID_HARD_RECALC:
            DoHardRecalc();
            rReq.Done();
            break;
        case SID_UPDATETABLINKS:
            {
                ScDocument& rDoc = GetDocument();

                ScLkUpdMode nSet = rDoc.GetLinkMode();

                if(nSet==LM_UNKNOWN)
                {
                    ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
                    nSet=aAppOptions.GetLinkMode();
                }

                if (m_nCanUpdate == css::document::UpdateDocMode::NO_UPDATE)
                    nSet = LM_NEVER;
                else if (m_nCanUpdate == css::document::UpdateDocMode::FULL_UPDATE)
                    nSet = LM_ALWAYS;

                if (nSet == LM_ALWAYS
                    && !(SvtSecurityOptions()
                         .isTrustedLocationUriForUpdatingLinks(
                             GetMedium() == nullptr
                             ? OUString() : GetMedium()->GetName())))
                {
                    nSet = LM_ON_DEMAND;
                }
                if (m_nCanUpdate == css::document::UpdateDocMode::QUIET_UPDATE
                    && nSet == LM_ON_DEMAND)
                {
                    nSet = LM_NEVER;
                }

                if (nSet == LM_ALWAYS)
                {
                    ReloadAllLinks();
                    rReq.Done();
                }
                else if (nSet == LM_NEVER)
                {
                    getEmbeddedObjectContainer().setUserAllowsLinkUpdate(false);
                    rReq.Ignore();
                }
                else if (nSet == LM_ON_DEMAND)
                {
                    ScTabViewShell* pViewSh = GetBestViewShell();
                    SfxViewFrame* pViewFrame = pViewSh ? pViewSh->GetFrame() : nullptr;
                    if (pViewFrame)
                    {
                        pViewFrame->RemoveInfoBar("enablecontent");
                        auto pInfoBar = pViewFrame->AppendInfoBar("enablecontent", ScResId(STR_RELOAD_TABLES), InfoBarType::Warning);
                        if (pInfoBar)
                        {
                            VclPtrInstance<PushButton> xBtn(&pViewFrame->GetWindow());
                            xBtn->SetText(ScResId(STR_ENABLE_CONTENT));
                            xBtn->SetSizePixel(xBtn->GetOptimalSize());
                            xBtn->SetClickHdl(LINK(this, ScDocShell, ReloadAllLinksHdl));
                            pInfoBar->addButton(xBtn);
                        }
                    }
                    rReq.Done();
                }
            }
            break;

        case SID_REIMPORT_AFTER_LOAD:
            {
                //  Is called after loading if there are DB areas with omitted data

                bool bDone = false;
                ScDBCollection* pDBColl = m_aDocument.GetDBCollection();

                if ((m_nCanUpdate != css::document::UpdateDocMode::NO_UPDATE) &&
                   (m_nCanUpdate != css::document::UpdateDocMode::QUIET_UPDATE))
                {
                    ScRange aRange;
                    ScTabViewShell* pViewSh = GetBestViewShell();
                    OSL_ENSURE(pViewSh,"SID_REIMPORT_AFTER_LOAD: no View");
                    if (pViewSh && pDBColl)
                    {
                        vcl::Window* pWin = GetActiveDialogParent();
                        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                                       ScResId(STR_REIMPORT_AFTER_LOAD)));
                        xQueryBox->set_default_response(RET_YES);
                        if (xQueryBox->run() == RET_YES)
                        {
                            ScDBCollection::NamedDBs& rDBs = pDBColl->getNamedDBs();
                            ScDBCollection::NamedDBs::iterator itr = rDBs.begin(), itrEnd = rDBs.end();
                            for (; itr != itrEnd; ++itr)
                            {
                                ScDBData& rDBData = **itr;
                                if ( rDBData.IsStripData() &&
                                     rDBData.HasImportParam() && !rDBData.HasImportSelection() )
                                {
                                    rDBData.GetArea(aRange);
                                    pViewSh->MarkRange(aRange);

                                    //  Import and internal operations like SID_REFRESH_DBAREA
                                    //  (inquiry for import not needed here)

                                    ScImportParam aImportParam;
                                    rDBData.GetImportParam( aImportParam );
                                    bool bContinue = pViewSh->ImportData( aImportParam );
                                    rDBData.SetImportParam( aImportParam );

                                    //  mark (size may have changed)
                                    rDBData.GetArea(aRange);
                                    pViewSh->MarkRange(aRange);

                                    if ( bContinue )    // error at import -> abort
                                    {
                                        //  internal operations, if some where saved

                                        if ( rDBData.HasQueryParam() || rDBData.HasSortParam() ||
                                             rDBData.HasSubTotalParam() )
                                            pViewSh->RepeatDB();

                                        //  pivot tables, which have the range as source data

                                        RefreshPivotTables(aRange);
                                    }
                                }
                            }
                            bDone = true;
                        }
                    }
                }

                if ( !bDone && pDBColl )
                {
                    //  if not, but then update the dependent formulas
                    //! also for individual ranges, which cannot be updated

                    m_aDocument.CalcAll();        //! only for the dependent
                    PostDataChanged();
                }

                if (bDone)
                    rReq.Done();
                else
                    rReq.Ignore();
            }
            break;

        case SID_AUTO_STYLE:
            OSL_FAIL("use ScAutoStyleHint instead of SID_AUTO_STYLE");
            break;

        case SID_GET_COLORLIST:
            {
                const SvxColorListItem* pColItem = GetItem(SID_COLOR_TABLE);
                XColorListRef pList = pColItem->GetColorList();
                rReq.SetReturnValue(OfaRefItem<XColorList>(SID_GET_COLORLIST, pList));
            }
            break;

        case FID_CHG_RECORD:
            {
                ScDocument& rDoc = GetDocument();
                // get argument (recorded macro)
                const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(FID_CHG_RECORD);
                bool bDo = true;

                // desired state
                ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
                bool bActivateTracking = (pChangeTrack == nullptr);   // toggle
                if ( pItem )
                    bActivateTracking = pItem->GetValue();      // from argument

                if ( !bActivateTracking )
                {
                    if ( !pItem )
                    {
                        // no dialog on playing the macro
                        vcl::Window* pWin = GetActiveDialogParent();
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                   VclMessageType::Warning, VclButtonsType::YesNo,
                                                                   ScResId(STR_END_REDLINING)));
                        xWarn->set_default_response(RET_NO);
                        bDo = (xWarn->run() == RET_YES );
                    }

                    if ( bDo )
                    {
                        if (pChangeTrack)
                        {
                            if ( pChangeTrack->IsProtected() )
                                bDo = ExecuteChangeProtectionDialog();
                        }
                        if ( bDo )
                        {
                            rDoc.EndChangeTracking();
                            PostPaintGridAll();
                        }
                    }
                }
                else
                {
                    rDoc.StartChangeTracking();
                    ScChangeViewSettings aChangeViewSet;
                    aChangeViewSet.SetShowChanges(true);
                    rDoc.SetChangeViewSettings(aChangeViewSet);
                }

                if ( bDo )
                {
                    UpdateAcceptChangesDialog();

                    // invalidate slots
                    if (pBindings)
                        pBindings->InvalidateAll(false);
                    if ( !pItem )
                        rReq.AppendItem( SfxBoolItem( FID_CHG_RECORD, bActivateTracking ) );
                    rReq.Done();
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_CHG_PROTECT :
            {
                if ( ExecuteChangeProtectionDialog() )
                {
                    rReq.Done();
                    SetDocumentModified();
                }
                else
                    rReq.Ignore();
            }
            break;

        case SID_DOCUMENT_MERGE:
        case SID_DOCUMENT_COMPARE:
            {
                bool bDo = true;
                ScChangeTrack* pChangeTrack = m_aDocument.GetChangeTrack();
                if ( pChangeTrack && !m_pImpl->bIgnoreLostRedliningWarning )
                {
                    if ( nSlot == SID_DOCUMENT_COMPARE )
                    {   //! old changes trace will be lost
                        vcl::Window* pWin = GetActiveDialogParent();
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                   VclMessageType::Warning, VclButtonsType::YesNo,
                                                                   ScResId(STR_END_REDLINING)));
                        xWarn->set_default_response(RET_NO);
                        if (xWarn->run() == RET_YES)
                            bDo = ExecuteChangeProtectionDialog( true );
                        else
                            bDo = false;
                    }
                    else    // merge might reject some actions
                        bDo = ExecuteChangeProtectionDialog( true );
                }
                if ( !bDo )
                {
                    rReq.Ignore();
                    break;
                }
                SfxApplication* pApp = SfxGetpApp();
                const SfxPoolItem* pItem;
                const SfxStringItem* pStringItem(nullptr);
                SfxMedium* pMed = nullptr;
                if (pReqArgs && pReqArgs->GetItemState(SID_FILE_NAME, true, &pItem) == SfxItemState::SET)
                {
                    pStringItem = dynamic_cast<const SfxStringItem*>(pItem);
                }
                if (pStringItem)
                {
                    OUString aFileName = pStringItem->GetValue();

                    OUString aFilterName;
                    pStringItem = nullptr;
                    if (pReqArgs->GetItemState(SID_FILTER_NAME, true, &pItem) == SfxItemState::SET)
                        pStringItem = dynamic_cast<const SfxStringItem*>(pItem);
                    if (pStringItem)
                    {
                        aFilterName = pStringItem->GetValue();
                    }
                    OUString aOptions;
                    pStringItem = nullptr;
                    if (pReqArgs->GetItemState(SID_FILE_FILTEROPTIONS, true, &pItem) == SfxItemState::SET)
                        pStringItem = dynamic_cast<const SfxStringItem*>(pItem);
                    if (pStringItem)
                    {
                        aOptions = pStringItem->GetValue();
                    }
                    short nVersion = 0;
                    const SfxInt16Item* pInt16Item(nullptr);
                    if (pReqArgs->GetItemState(SID_VERSION, true, &pItem) == SfxItemState::SET)
                        pInt16Item = dynamic_cast<const SfxInt16Item*>(pItem);
                    if (pInt16Item)
                    {
                        nVersion = pInt16Item->GetValue();
                    }

                    //  no filter specified -> detection
                    if (aFilterName.isEmpty())
                        ScDocumentLoader::GetFilterName( aFileName, aFilterName, aOptions, true, false );

                    //  filter name from dialog contains application prefix,
                    //  GetFilter needs name without the prefix.
                    ScDocumentLoader::RemoveAppPrefix( aFilterName );

                    std::shared_ptr<const SfxFilter> pFilter = ScDocShell::Factory().GetFilterContainer()->GetFilter4FilterName( aFilterName );
                    SfxItemSet* pSet = new SfxAllItemSet( pApp->GetPool() );
                    if (!aOptions.isEmpty())
                        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
                    if ( nVersion != 0 )
                        pSet->Put( SfxInt16Item( SID_VERSION, nVersion ) );
                    pMed = new SfxMedium( aFileName, StreamMode::STD_READ, pFilter, pSet );
                }
                else
                {
                    const sfx2::DocumentInserter::Mode mode { nSlot==SID_DOCUMENT_COMPARE
                        ? sfx2::DocumentInserter::Mode::Compare
                        : sfx2::DocumentInserter::Mode::Merge};
                    // start file dialog asynchronous
                    m_pImpl->bIgnoreLostRedliningWarning = true;
                    m_pImpl->pRequest.reset(new SfxRequest( rReq ));
                    m_pImpl->pDocInserter.reset();

                    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
                    vcl::Window* pParent = pViewSh ? pViewSh->GetDialogParent() : nullptr;

                    m_pImpl->pDocInserter.reset( new ::sfx2::DocumentInserter(pParent ? pParent->GetFrameWeld() : nullptr,
                        ScDocShell::Factory().GetFactoryName(), mode ) );
                    m_pImpl->pDocInserter->StartExecuteModal( LINK( this, ScDocShell, DialogClosedHdl ) );
                    return ;
                }

                if ( pMed )     // now execute in earnest...
                {
                    SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

                    // pOtherDocSh->DoClose() will be called explicitly later, but it is still more safe to use SfxObjectShellLock here
                    ScDocShell* pOtherDocSh = new ScDocShell;
                    SfxObjectShellLock aDocShTablesRef = pOtherDocSh;
                    pOtherDocSh->DoLoad( pMed );
                    ErrCode nErr = pOtherDocSh->GetErrorCode();
                    if (nErr)
                        ErrorHandler::HandleError( nErr );          // also warnings

                    if ( !pOtherDocSh->GetError() )                 // only errors
                    {
                        bool bHadTrack = ( m_aDocument.GetChangeTrack() != nullptr );
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                        sal_uLong nStart = 0;
                        if ( nSlot == SID_DOCUMENT_MERGE && pChangeTrack )
                        {
                            nStart = pChangeTrack->GetActionMax() + 1;
                        }
#endif
                        if ( nSlot == SID_DOCUMENT_COMPARE )
                            CompareDocument( pOtherDocSh->GetDocument() );
                        else
                            MergeDocument( pOtherDocSh->GetDocument() );

                        //  show "accept changes" dialog
                        //! get view for this document!
                        if ( !IsDocShared() )
                        {
                            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                            if ( pViewFrm )
                            {
                                pViewFrm->ShowChildWindow( ScAcceptChgDlgWrapper::GetChildWindowId() ); //@51669
                            }
                            if ( pBindings )
                            {
                                pBindings->Invalidate( FID_CHG_ACCEPT );
                            }
                        }

                        rReq.SetReturnValue( SfxInt32Item( nSlot, 0 ) );        //! ???????
                        rReq.Done();

                        if (!bHadTrack)         //  newly turned on -> show as well
                        {
                            ScChangeViewSettings* pOldSet = m_aDocument.GetChangeViewSettings();
                            if ( !pOldSet || !pOldSet->ShowChanges() )
                            {
                                ScChangeViewSettings aChangeViewSet;
                                aChangeViewSet.SetShowChanges(true);
                                m_aDocument.SetChangeViewSettings(aChangeViewSet);
                            }
                        }
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                        else if ( nSlot == SID_DOCUMENT_MERGE && IsDocShared() && pChangeTrack )
                        {
                            sal_uLong nEnd = pChangeTrack->GetActionMax();
                            if ( nEnd >= nStart )
                            {
                                // only show changes from merged document
                                ScChangeViewSettings aChangeViewSet;
                                aChangeViewSet.SetShowChanges( true );
                                aChangeViewSet.SetShowAccepted( true );
                                aChangeViewSet.SetHasActionRange();
                                aChangeViewSet.SetTheActionRange( nStart, nEnd );
                                m_aDocument.SetChangeViewSettings( aChangeViewSet );

                                // update view
                                PostPaintExtras();
                                PostPaintGridAll();
                            }
                        }
#endif
                    }
                    pOtherDocSh->DoClose();     // delete happens with the Ref
                }
            }
            break;

        case SID_DELETE_SCENARIO:
            if (pReqArgs)
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                {
                    if (const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>(pItem))
                    {
                        OUString aName = pStringItem->GetValue();
                        SCTAB nTab;
                        if (m_aDocument.GetTable( aName, nTab ))
                        {
                            //  move DeleteTable from viewfunc to docfunc!

                            ScTabViewShell* pSh = GetBestViewShell();
                            if ( pSh )
                            {
                                //! omit SetTabNo in DeleteTable?
                                SCTAB nDispTab = pSh->GetViewData().GetTabNo();
                                pSh->DeleteTable( nTab );
                                pSh->SetTabNo(nDispTab);
                                rReq.Done();
                            }
                        }
                    }
                }
            }
            break;

        case SID_EDIT_SCENARIO:
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                {
                    if (const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>(pItem))
                    {
                        OUString aName = pStringItem->GetValue();
                        SCTAB nTab;
                        if (m_aDocument.GetTable( aName, nTab ))
                        {
                            if (m_aDocument.IsScenario(nTab))
                            {
                                OUString aComment;
                                Color aColor;
                                ScScenarioFlags nFlags;
                                m_aDocument.GetScenarioData( nTab, aComment, aColor, nFlags );

                                // Determine if the Sheet that the Scenario was created on
                                // is protected. But first we need to find that Sheet.
                                // Rewind back to the actual sheet.
                                SCTAB nActualTab = nTab;
                                do
                                {
                                    nActualTab--;
                                }
                                while(m_aDocument.IsScenario(nActualTab));
                                bool bSheetProtected = m_aDocument.IsTabProtected(nActualTab);

                                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                                ScopedVclPtr<AbstractScNewScenarioDlg> pNewDlg(pFact->CreateScNewScenarioDlg(GetActiveDialogParent(), aName, true, bSheetProtected));
                                pNewDlg->SetScenarioData( aName, aComment, aColor, nFlags );
                                if ( pNewDlg->Execute() == RET_OK )
                                {
                                    pNewDlg->GetScenarioData( aName, aComment, aColor, nFlags );
                                    ModifyScenario( nTab, aName, aComment, aColor, nFlags );
                                    rReq.Done();
                                }
                            }
                        }
                    }
                }
            }
            break;

        case SID_ATTR_YEAR2000 :
        {
            const SfxPoolItem* pItem;
            if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
            {
                if (const SfxUInt16Item* pInt16Item = dynamic_cast<const SfxUInt16Item*>(pItem))
                {
                    sal_uInt16 nY2k = pInt16Item->GetValue();
                    // set always to DocOptions, so that it is also saved for S050
                    // (and all inquiries run up until now on it as well).
                    // SetDocOptions propagates that to the NumberFormatter
                    ScDocOptions aDocOpt( m_aDocument.GetDocOptions() );
                    aDocOpt.SetYear2000( nY2k );
                    m_aDocument.SetDocOptions( aDocOpt );
                    // the FormShell shall notice it as well
                    ScTabViewShell* pSh = GetBestViewShell();
                    if ( pSh )
                    {
                        FmFormShell* pFSh = pSh->GetFormShell();
                        if ( pFSh )
                            pFSh->SetY2KState( nY2k );
                    }
                }
            }
        }
        break;

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
        case SID_SHARE_DOC:
            {
                ScViewData* pViewData = GetViewData();
                if ( !pViewData )
                {
                    rReq.Ignore();
                    break;
                }

                ScopedVclPtrInstance< ScShareDocumentDlg > aDlg( GetActiveDialogParent(), pViewData );
                if ( aDlg->Execute() == RET_OK )
                {
                    bool bSetShared = aDlg->IsShareDocumentChecked();
                    if ( bSetShared != IsDocShared() )
                    {
                        if ( bSetShared )
                        {
                            bool bContinue = true;
                            if ( HasName() )
                            {
                                vcl::Window* pWin = GetActiveDialogParent();
                                std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                               VclMessageType::Question, VclButtonsType::YesNo,
                                                                               ScResId(STR_REIMPORT_AFTER_LOAD)));
                                xQueryBox->set_default_response(RET_YES);
                                if (xQueryBox->run() == RET_NO)
                                {
                                    bContinue = false;
                                }
                            }
                            if ( bContinue )
                            {
                                EnableSharedSettings( true );

                                SC_MOD()->SetInSharedDocSaving( true );
                                if ( !SwitchToShared( true, true ) )
                                {
                                    // TODO/LATER: what should be done in case the switch has failed?
                                    // for example in case the user has cancelled the saveAs operation
                                }

                                SC_MOD()->SetInSharedDocSaving( false );

                                InvalidateName();
                                GetUndoManager()->Clear();

                                ScTabView* pTabView = dynamic_cast< ScTabView* >( pViewData->GetView() );
                                if ( pTabView )
                                {
                                    pTabView->UpdateLayerLocks();
                                }
                            }
                        }
                        else
                        {
                            uno::Reference< frame::XModel > xModel;
                            try
                            {
                                // load shared file
                                xModel.set( LoadSharedDocument(), uno::UNO_QUERY_THROW );
                                uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY_THROW );

                                // check if shared flag is set in shared file
                                bool bShared = false;
                                ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
                                if ( pDocObj )
                                {
                                    ScDocShell* pDocShell = dynamic_cast< ScDocShell* >( pDocObj->GetEmbeddedObject() );
                                    if ( pDocShell )
                                    {
                                        bShared = pDocShell->HasSharedXMLFlagSet();
                                    }
                                }

                                // #i87870# check if shared status was disabled and enabled again
                                bool bOwnEntry = false;
                                try
                                {
                                    ::svt::ShareControlFile aControlFile( GetSharedFileURL() );
                                    bOwnEntry = aControlFile.HasOwnEntry();
                                }
                                catch ( uno::Exception& )
                                {
                                }

                                if ( bShared && bOwnEntry )
                                {
                                    uno::Reference< frame::XStorable > xStorable( xModel, uno::UNO_QUERY_THROW );
                                    if ( xStorable->isReadonly() )
                                    {
                                        xCloseable->close( true );

                                        OUString aUserName( ScResId( STR_UNKNOWN_USER ) );
                                        try
                                        {
                                            ::svt::DocumentLockFile aLockFile( GetSharedFileURL() );
                                            LockFileEntry aData = aLockFile.GetLockData();
                                            if ( !aData[LockFileComponent::OOOUSERNAME].isEmpty() )
                                            {
                                                aUserName = aData[LockFileComponent::OOOUSERNAME];
                                            }
                                            else if ( !aData[LockFileComponent::SYSUSERNAME].isEmpty() )
                                            {
                                                aUserName = aData[LockFileComponent::SYSUSERNAME];
                                            }
                                        }
                                        catch ( uno::Exception& )
                                        {
                                        }
                                        OUString aMessage( ScResId( STR_FILE_LOCKED_TRY_LATER ) );
                                        aMessage = aMessage.replaceFirst( "%1", aUserName );

                                        vcl::Window* pWin = GetActiveDialogParent();
                                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                                   aMessage));
                                        xWarn->run();
                                    }
                                    else
                                    {
                                        vcl::Window* pWin = GetActiveDialogParent();
                                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                                   VclMessageType::Warning, VclButtonsType::YesNo,
                                                                                   ScResId(STR_DOC_DISABLESHARED)));
                                        xWarn->set_default_response(RET_YES);

                                        if (xWarn->run() == RET_YES)
                                        {
                                            xCloseable->close( true );

                                            if ( !SwitchToShared( false, true ) )
                                            {
                                                // TODO/LATER: what should be done in case the switch has failed?
                                                // for example in case the user has cancelled the saveAs operation
                                            }

                                            EnableSharedSettings( false );

                                            if ( pBindings )
                                            {
                                                pBindings->ExecuteSynchron( SID_SAVEDOC );
                                            }

                                            ScTabView* pTabView = dynamic_cast< ScTabView* >( pViewData->GetView() );
                                            if ( pTabView )
                                            {
                                                pTabView->UpdateLayerLocks();
                                            }
                                        }
                                        else
                                        {
                                            xCloseable->close( true );
                                        }
                                    }
                                }
                                else
                                {
                                    xCloseable->close( true );
                                    vcl::Window* pWin = GetActiveDialogParent();
                                    std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                               VclMessageType::Warning, VclButtonsType::Ok,
                                                                               ScResId(STR_DOC_NOLONGERSHARED)));
                                    xWarn->run();
                                }
                            }
                            catch ( uno::Exception& )
                            {
                                OSL_FAIL( "SID_SHARE_DOC: caught exception" );
                                SC_MOD()->SetInSharedDocSaving( false );

                                try
                                {
                                    uno::Reference< util::XCloseable > xClose( xModel, uno::UNO_QUERY_THROW );
                                    xClose->close( true );
                                }
                                catch ( uno::Exception& )
                                {
                                }
                            }
                        }
                    }
                }
                rReq.Done();
            }
            break;
#endif
        case SID_OPEN_CALC:
        {
            SfxStringItem aApp(SID_DOC_SERVICE, OUString("com.sun.star.sheet.SpreadsheetDocument"));
            SfxStringItem aTarget(SID_TARGETNAME, OUString("_blank"));
            GetViewData()->GetDispatcher().ExecuteList(
                SID_OPENDOC, SfxCallMode::API|SfxCallMode::SYNCHRON,
                { &aApp, &aTarget });
        }
        break;
        case SID_NOTEBOOKBAR:
        {
            const SfxStringItem* pFile = rReq.GetArg<SfxStringItem>( SID_NOTEBOOKBAR );

            if ( pBindings && sfx2::SfxNotebookBar::IsActive() )
                sfx2::SfxNotebookBar::ExecMethod(*pBindings, pFile ? pFile->GetValue() : "");
            else if ( pBindings )
                sfx2::SfxNotebookBar::CloseMethod(*pBindings);
        }
        break;
        case SID_LANGUAGE_STATUS:
        {
            sal_Int32 nPos = 0;
            OUString aLangText;
            const SfxStringItem* pItem = rReq.GetArg<SfxStringItem>(nSlot);
            if ( pItem )
                aLangText = pItem->GetValue();

            if ( !aLangText.isEmpty() )
            {
                LanguageType eLang, eLatin, eCjk, eCtl;
                const OUString aDocLangPrefix("Default_");
                const OUString aNoLang("LANGUAGE_NONE");
                const OUString aResetLang("RESET_LANGUAGES");

                ScDocument& rDoc = GetDocument();
                rDoc.GetLanguage( eLatin, eCjk, eCtl );

                if ( aLangText == "*" )
                {
                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateVclDialog(GetActiveDialogParent(), SID_LANGUAGE_OPTIONS));
                    pDlg->Execute();

                    rDoc.GetLanguage( eLang, eCjk, eCtl );
                }
                else if ( (nPos = aLangText.indexOf(aDocLangPrefix)) != -1 )
                {
                    aLangText = aLangText.replaceAt(nPos, aDocLangPrefix.getLength(), "");

                    if ( aLangText == aNoLang )
                    {
                        eLang = LANGUAGE_NONE;
                        rDoc.SetLanguage( eLang, eCjk, eCtl );
                    }
                    else if ( aLangText == aResetLang )
                    {
                        bool bAutoSpell;

                        ScModule::GetSpellSettings(eLang, eCjk, eCtl, bAutoSpell);
                        rDoc.SetLanguage(eLang, eCjk, eCtl);
                    }
                    else
                    {
                        eLang = SvtLanguageTable::GetLanguageType( aLangText );
                        if ( eLang != LANGUAGE_DONTKNOW  && SvtLanguageOptions::GetScriptTypeOfLanguage(eLang) == SvtScriptType::LATIN )
                        {
                            rDoc.SetLanguage( eLang, eCjk, eCtl );
                        }
                        else
                        {
                            eLang = eLatin;
                        }
                    }
                }

                if ( eLang != eLatin )
                {
                    if ( ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell() )
                    {
                        ScInputHandler* pInputHandler = SC_MOD()->GetInputHdl(pViewSh);
                        if ( pInputHandler )
                            pInputHandler->UpdateSpellSettings();

                        pViewSh->UpdateDrawTextOutliner();
                    }

                    SetDocumentModified();
                    Broadcast(SfxHint(SfxHintId::LanguageChanged));
                    PostPaintGridAll();
                }
            }
        }
        break;
        default:
        {
            // small (?) hack -> forwarding of the slots to TabViewShell
            ScTabViewShell* pSh = GetBestViewShell();
            if ( pSh )
                pSh->Execute( rReq );
#if HAVE_FEATURE_SCRIPTING
            else
                SbxBase::SetError( ERRCODE_BASIC_NO_ACTIVE_OBJECT );
#endif
        }
    }
}

void UpdateAcceptChangesDialog()
{
    //  update "accept changes" dialog
    //! notify all views
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if ( pViewFrm && pViewFrm->HasChildWindow( FID_CHG_ACCEPT ) )
    {
        SfxChildWindow* pChild = pViewFrm->GetChildWindow( FID_CHG_ACCEPT );
        if ( pChild )
            static_cast<ScAcceptChgDlgWrapper*>(pChild)->ReInitDlg();
    }
}

bool ScDocShell::ExecuteChangeProtectionDialog( bool bJustQueryIfProtected )
{
    bool bDone = false;
    ScChangeTrack* pChangeTrack = m_aDocument.GetChangeTrack();
    if ( pChangeTrack )
    {
        bool bProtected = pChangeTrack->IsProtected();
        if ( bJustQueryIfProtected && !bProtected )
            return true;

        OUString aTitle( ScResId( bProtected ? SCSTR_CHG_UNPROTECT : SCSTR_CHG_PROTECT ) );
        OUString aText( ScResId( SCSTR_PASSWORD ) );
        OUString aPassword;

        vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
        SfxPasswordDialog aDlg(pWin ? pWin->GetFrameWeld() : nullptr, &aText);
        aDlg.set_title(aTitle);
        aDlg.SetMinLen(1);
        aDlg.set_help_id(GetStaticInterface()->GetSlot(SID_CHG_PROTECT)->GetCommand());
        aDlg.SetEditHelpId( HID_CHG_PROTECT );
        if ( !bProtected )
            aDlg.ShowExtras(SfxShowExtras::CONFIRM);
        if (aDlg.execute() == RET_OK)
            aPassword = aDlg.GetPassword();

        if (!aPassword.isEmpty())
        {
            if ( bProtected )
            {
                if ( SvPasswordHelper::CompareHashPassword(pChangeTrack->GetProtection(), aPassword) )
                {
                    if ( bJustQueryIfProtected )
                        bDone = true;
                    else
                        pChangeTrack->SetProtection(
                            css::uno::Sequence< sal_Int8 > (0) );
                }
                else
                {
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                                  ScResId(SCSTR_WRONGPASSWORD)));
                    xInfoBox->run();
                }
            }
            else
            {
                css::uno::Sequence< sal_Int8 > aPass;
                SvPasswordHelper::GetHashPassword( aPass, aPassword );
                pChangeTrack->SetProtection( aPass );
            }
            if ( bProtected != pChangeTrack->IsProtected() )
            {
                UpdateAcceptChangesDialog();
                bDone = true;
            }
        }
    }
    else if ( bJustQueryIfProtected )
        bDone = true;
    return bDone;
}

void ScDocShell::DoRecalc( bool bApi )
{
    bool bDone = false;
    ScTabViewShell* pSh = GetBestViewShell();
    ScInputHandler* pHdl = ( pSh ? SC_MOD()->GetInputHdl( pSh ) : nullptr );
    if ( pSh )
    {
        if ( pHdl && pHdl->IsInputMode() && pHdl->IsFormulaMode() && !bApi )
        {
            pHdl->FormulaPreview();     // partial result as QuickHelp
            bDone = true;
        }
        else
        {
            ScTabView::UpdateInputLine();     // InputEnterHandler
            pSh->UpdateInputHandler();
        }
    }
    if (!bDone)                         // otherwise re-calculate document
    {
        WaitObject aWaitObj( GetActiveDialogParent() );
        if ( pHdl )
        {
            // tdf97897 set current cell to Dirty to force recalculation of cell
            ScFormulaCell* pFC = m_aDocument.GetFormulaCell( pHdl->GetCursorPos());
            if (pFC)
                pFC->SetDirty();
        }
        m_aDocument.CalcFormulaTree();
        if ( pSh )
            pSh->UpdateCharts(true);

        m_aDocument.BroadcastUno( SfxHint( SfxHintId::DataChanged ) );

        //  If there are charts, then paint everything, so that PostDataChanged
        //  and the charts do not come one after the other and parts are painted twice.

        ScChartListenerCollection* pCharts = m_aDocument.GetChartListenerCollection();
        if ( pCharts && pCharts->hasListeners() )
            PostPaintGridAll();
        else
            PostDataChanged();
    }
}

void ScDocShell::DoHardRecalc()
{
    auto start = std::chrono::steady_clock::now();
    WaitObject aWaitObj( GetActiveDialogParent() );
    ScTabViewShell* pSh = GetBestViewShell();
    if ( pSh )
    {
        ScTabView::UpdateInputLine();     // InputEnterHandler
        pSh->UpdateInputHandler();
    }
    m_aDocument.CalcAll();
    GetDocFunc().DetectiveRefresh();    // creates own Undo
    if ( pSh )
        pSh->UpdateCharts(true);

    // set notification flags for "calculate" event (used in SfxHintId::DataChanged broadcast)
    // (might check for the presence of any formulas on each sheet)
    SCTAB nTabCount = m_aDocument.GetTableCount();
    if (m_aDocument.HasAnySheetEventScript( ScSheetEventId::CALCULATE, true )) // search also for VBA handler
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            m_aDocument.SetCalcNotification(nTab);

    // CalcAll doesn't broadcast value changes, so SfxHintId::ScCalcAll is broadcasted globally
    // in addition to SfxHintId::DataChanged.
    m_aDocument.BroadcastUno( SfxHint( SfxHintId::ScCalcAll ) );
    m_aDocument.BroadcastUno( SfxHint( SfxHintId::DataChanged ) );

    // use hard recalc also to disable stream-copying of all sheets
    // (somewhat consistent with charts)
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        m_aDocument.SetStreamValid(nTab, false);

    PostPaintGridAll();
    auto end = std::chrono::steady_clock::now();
    SAL_INFO("sc.timing", "ScDocShell::DoHardRecalc(): took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms");
}

void ScDocShell::DoAutoStyle( const ScRange& rRange, const OUString& rStyle )
{
    ScStyleSheetPool* pStylePool = m_aDocument.GetStyleSheetPool();
    ScStyleSheet* pStyleSheet =
        pStylePool->FindCaseIns( rStyle, SfxStyleFamily::Para );
    if (!pStyleSheet)
        pStyleSheet = static_cast<ScStyleSheet*>(
            pStylePool->Find( ScResId(STR_STYLENAME_STANDARD), SfxStyleFamily::Para ));
    if (pStyleSheet)
    {
        OSL_ENSURE(rRange.aStart.Tab() == rRange.aEnd.Tab(),
                        "DoAutoStyle with several tables");
        SCTAB nTab = rRange.aStart.Tab();
        SCCOL nStartCol = rRange.aStart.Col();
        SCROW nStartRow = rRange.aStart.Row();
        SCCOL nEndCol = rRange.aEnd.Col();
        SCROW nEndRow = rRange.aEnd.Row();
        m_aDocument.ApplyStyleAreaTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, *pStyleSheet );
        m_aDocument.ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab );
        PostPaint( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab, PaintPartFlags::Grid );
    }
}

void ScDocShell::NotifyStyle( const SfxStyleSheetHint& rHint )
{
    SfxHintId nId = rHint.GetId();
    const SfxStyleSheetBase* pStyle = rHint.GetStyleSheet();
    if (!pStyle)
        return;

    if ( pStyle->GetFamily() == SfxStyleFamily::Page )
    {
        if ( nId == SfxHintId::StyleSheetModified )
        {
            ScDocShellModificator aModificator( *this );

            OUString aNewName = pStyle->GetName();
            OUString aOldName = aNewName;
            const SfxStyleSheetModifiedHint* pExtendedHint = dynamic_cast<const SfxStyleSheetModifiedHint*>(&rHint); // name changed?
            if (pExtendedHint)
                aOldName = pExtendedHint->GetOldName();

            if ( aNewName != aOldName )
                m_aDocument.RenamePageStyleInUse( aOldName, aNewName );

            SCTAB nTabCount = m_aDocument.GetTableCount();
            for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                if (m_aDocument.GetPageStyle(nTab) == aNewName)   // already adjusted to new
                {
                    m_aDocument.PageStyleModified( nTab, aNewName );
                    ScPrintFunc aPrintFunc( this, GetPrinter(), nTab );
                    aPrintFunc.UpdatePages();
                }

            aModificator.SetDocumentModified();

            if (pExtendedHint)
            {
                SfxBindings* pBindings = GetViewBindings();
                if (pBindings)
                {
                    pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                    pBindings->Invalidate( SID_STYLE_FAMILY4 );
                    pBindings->Invalidate( FID_RESET_PRINTZOOM );
                    pBindings->Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
                    pBindings->Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
                }
            }
        }
    }
    else if ( pStyle->GetFamily() == SfxStyleFamily::Para )
    {
        if ( nId == SfxHintId::StyleSheetModified)
        {
            OUString aNewName = pStyle->GetName();
            OUString aOldName = aNewName;
            const SfxStyleSheetModifiedHint* pExtendedHint = dynamic_cast<const SfxStyleSheetModifiedHint*>(&rHint);
            if (pExtendedHint)
                aOldName = pExtendedHint->GetOldName();
            if ( aNewName != aOldName )
            {
                for(SCTAB i = 0; i < m_aDocument.GetTableCount(); ++i)
                {
                    ScConditionalFormatList* pList = m_aDocument.GetCondFormList(i);
                    if (pList)
                        pList->RenameCellStyle( aOldName,aNewName );
                }
            }
        }
    }

    //  everything else goes via slots...
}

//  like in printfun.cxx
#define ZOOM_MIN    10

void ScDocShell::SetPrintZoom( SCTAB nTab, sal_uInt16 nScale, sal_uInt16 nPages )
{
    OUString aStyleName = m_aDocument.GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = m_aDocument.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SfxStyleFamily::Page );
    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        ScDocShellModificator aModificator( *this );

        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        const bool bUndo(m_aDocument.IsUndoEnabled());
        if (bUndo)
        {
            sal_uInt16 nOldScale = rSet.Get(ATTR_PAGE_SCALE).GetValue();
            sal_uInt16 nOldPages = rSet.Get(ATTR_PAGE_SCALETOPAGES).GetValue();
            GetUndoManager()->AddUndoAction( new ScUndoPrintZoom(
                            this, nTab, nOldScale, nOldPages, nScale, nPages ) );
        }

        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, nScale ) );
        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, nPages ) );

        ScPrintFunc aPrintFunc( this, GetPrinter(), nTab );
        aPrintFunc.UpdatePages();
        aModificator.SetDocumentModified();

        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->Invalidate( FID_RESET_PRINTZOOM );
    }
}

bool ScDocShell::AdjustPrintZoom( const ScRange& rRange )
{
    bool bChange = false;
    SCTAB nTab = rRange.aStart.Tab();

    OUString aStyleName = m_aDocument.GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = m_aDocument.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SfxStyleFamily::Page );
    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        bool bHeaders = rSet.Get(ATTR_PAGE_HEADERS).GetValue();
        sal_uInt16 nOldScale = rSet.Get(ATTR_PAGE_SCALE).GetValue();
        sal_uInt16 nOldPages = rSet.Get(ATTR_PAGE_SCALETOPAGES).GetValue();
        const ScRange* pRepeatCol = m_aDocument.GetRepeatColRange( nTab );
        const ScRange* pRepeatRow = m_aDocument.GetRepeatRowRange( nTab );

        //  calculate needed scaling for selection

        sal_uInt16 nNewScale = nOldScale;

        long nBlkTwipsX = 0;
        if (bHeaders)
            nBlkTwipsX += long(PRINT_HEADER_WIDTH);
        SCCOL nStartCol = rRange.aStart.Col();
        SCCOL nEndCol = rRange.aEnd.Col();
        if ( pRepeatCol && nStartCol >= pRepeatCol->aStart.Col() )
        {
            for (SCCOL i=pRepeatCol->aStart.Col(); i<=pRepeatCol->aEnd.Col(); i++ )
                nBlkTwipsX += m_aDocument.GetColWidth( i, nTab );
            if ( nStartCol <= pRepeatCol->aEnd.Col() )
                nStartCol = pRepeatCol->aEnd.Col() + 1;
        }
        // legacy compilers' own scope for i
        {
            for ( SCCOL i=nStartCol; i<=nEndCol; i++ )
                nBlkTwipsX += m_aDocument.GetColWidth( i, nTab );
        }

        long nBlkTwipsY = 0;
        if (bHeaders)
            nBlkTwipsY += long(PRINT_HEADER_HEIGHT);
        SCROW nStartRow = rRange.aStart.Row();
        SCROW nEndRow = rRange.aEnd.Row();
        if ( pRepeatRow && nStartRow >= pRepeatRow->aStart.Row() )
        {
            nBlkTwipsY += m_aDocument.GetRowHeight( pRepeatRow->aStart.Row(),
                    pRepeatRow->aEnd.Row(), nTab );
            if ( nStartRow <= pRepeatRow->aEnd.Row() )
                nStartRow = pRepeatRow->aEnd.Row() + 1;
        }
        nBlkTwipsY += m_aDocument.GetRowHeight( nStartRow, nEndRow, nTab );

        Size aPhysPage;
        long nHdr, nFtr;
        ScPrintFunc aOldPrFunc( this, GetPrinter(), nTab );
        aOldPrFunc.GetScaleData( aPhysPage, nHdr, nFtr );
        nBlkTwipsY += nHdr + nFtr;

        if ( nBlkTwipsX == 0 )      // hidden columns/rows may lead to 0
            nBlkTwipsX = 1;
        if ( nBlkTwipsY == 0 )
            nBlkTwipsY = 1;

        long nNeeded = std::min( aPhysPage.Width()  * 100 / nBlkTwipsX,
                            aPhysPage.Height() * 100 / nBlkTwipsY );
        if ( nNeeded < ZOOM_MIN )
            nNeeded = ZOOM_MIN;         // boundary
        if ( nNeeded < static_cast<long>(nNewScale) )
            nNewScale = static_cast<sal_uInt16>(nNeeded);

        bChange = ( nNewScale != nOldScale || nOldPages != 0 );
        if ( bChange )
            SetPrintZoom( nTab, nNewScale, 0 );
    }
    return bChange;
}

void ScDocShell::PageStyleModified( const OUString& rStyleName, bool bApi )
{
    ScDocShellModificator aModificator( *this );

    SCTAB nTabCount = m_aDocument.GetTableCount();
    SCTAB nUseTab = MAXTAB+1;
    for (SCTAB nTab=0; nTab<nTabCount && nUseTab>MAXTAB; nTab++)
        if ( m_aDocument.GetPageStyle(nTab) == rStyleName &&
                ( !bApi || m_aDocument.GetPageSize(nTab).Width() ) )
            nUseTab = nTab;
                                // at bApi only if breaks already shown

    if (ValidTab(nUseTab))      // not used -> nothing to do
    {
        bool bWarn = false;

        ScPrintFunc aPrintFunc( this, GetPrinter(), nUseTab );  //! cope without CountPages
        if (!aPrintFunc.UpdatePages())                          //  sets breaks on all tabs
            bWarn = true;

        if (bWarn && !bApi)
        {
            vcl::Window* pWin = GetActiveDialogParent();
            ScWaitCursorOff aWaitOff(pWin);
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          ScResId(STR_PRINT_INVALID_AREA)));
            xInfoBox->run();
        }
    }

    aModificator.SetDocumentModified();

    SfxBindings* pBindings = GetViewBindings();
    if (pBindings)
    {
        pBindings->Invalidate( FID_RESET_PRINTZOOM );
        pBindings->Invalidate( SID_ATTR_PARA_LEFT_TO_RIGHT );
        pBindings->Invalidate( SID_ATTR_PARA_RIGHT_TO_LEFT );
    }
}

void ScDocShell::ExecutePageStyle( const SfxViewShell& rCaller,
                                   SfxRequest&   rReq,
                                   SCTAB         nCurTab )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_STATUS_PAGESTYLE:  // click on StatusBar control
        case SID_FORMATPAGE:
            {
                if ( pReqArgs == nullptr )
                {
                    OUString aOldName = m_aDocument.GetPageStyle( nCurTab );
                    ScStyleSheetPool* pStylePool = m_aDocument.GetStyleSheetPool();
                    SfxStyleSheetBase* pStyleSheet
                        = pStylePool->Find( aOldName, SfxStyleFamily::Page );

                    OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        ScStyleSaveData aOldData;
                        const bool bUndo(m_aDocument.IsUndoEnabled());
                        if (bUndo)
                            aOldData.InitFromStyle( pStyleSheet );

                        SfxItemSet&     rStyleSet = pStyleSheet->GetItemSet();
                        rStyleSet.MergeRange( XATTR_FILL_FIRST, XATTR_FILL_LAST );

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                        ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScStyleDlg( GetActiveDialogParent(), *pStyleSheet, RID_SCDLG_STYLES_PAGE, RID_SCDLG_STYLES_PAGE ));

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                            WaitObject aWait( GetActiveDialogParent() );

                            OUString aNewName = pStyleSheet->GetName();
                            if ( aNewName != aOldName &&
                                m_aDocument.RenamePageStyleInUse( aOldName, aNewName ) )
                            {
                                SfxBindings* pBindings = GetViewBindings();
                                if (pBindings)
                                {
                                    pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                                    pBindings->Invalidate( FID_RESET_PRINTZOOM );
                                }
                            }

                            if ( pOutSet )
                                m_aDocument.ModifyStyleSheet( *pStyleSheet, *pOutSet );

                            // memorizing for GetState():
                            GetPageOnFromPageStyleSet( &rStyleSet, nCurTab, m_bHeaderOn, m_bFooterOn );
                            rCaller.GetViewFrame()->GetBindings().Invalidate( SID_HFEDIT );

                            ScStyleSaveData aNewData;
                            aNewData.InitFromStyle( pStyleSheet );
                            if (bUndo)
                            {
                                GetUndoManager()->AddUndoAction(
                                        new ScUndoModifyStyle( this, SfxStyleFamily::Page,
                                                    aOldData, aNewData ) );
                            }

                            PageStyleModified( aNewName, false );
                            rReq.Done();
                        }
                        pDlg.disposeAndClear();
                    }
                }
            }
            break;

        case SID_HFEDIT:
            {
                if ( pReqArgs == nullptr )
                {
                    OUString aStr( m_aDocument.GetPageStyle( nCurTab ) );

                    ScStyleSheetPool* pStylePool
                        = m_aDocument.GetStyleSheetPool();

                    SfxStyleSheetBase* pStyleSheet
                        = pStylePool->Find( aStr, SfxStyleFamily::Page );

                    OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        SfxItemSet&  rStyleSet = pStyleSheet->GetItemSet();

                        SvxPageUsage eUsage = rStyleSet.Get( ATTR_PAGE ).GetPageUsage();
                        bool bShareHeader = rStyleSet
                                                    .Get(ATTR_PAGE_HEADERSET)
                                                    .GetItemSet()
                                                    .Get(ATTR_PAGE_SHARED)
                                                    .GetValue();
                        bool bShareFooter = rStyleSet
                                                    .Get(ATTR_PAGE_FOOTERSET)
                                                    .GetItemSet()
                                                    .Get(ATTR_PAGE_SHARED)
                                                    .GetValue();
                        sal_uInt16 nResId = 0;

                        switch ( eUsage )
                        {
                            case SvxPageUsage::Left:
                            case SvxPageUsage::Right:
                            {
                                if ( m_bHeaderOn && m_bFooterOn )
                                    nResId = RID_SCDLG_HFEDIT;
                                else if ( SvxPageUsage::Right == eUsage )
                                {
                                    if ( !m_bHeaderOn && m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                    else if ( m_bHeaderOn && !m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                }
                                else
                                {
                                    //  #69193a# respect "shared" setting
                                    if ( !m_bHeaderOn && m_bFooterOn )
                                        nResId = bShareFooter ?
                                                    RID_SCDLG_HFEDIT_RIGHTFOOTER :
                                                    RID_SCDLG_HFEDIT_LEFTFOOTER;
                                    else if ( m_bHeaderOn && !m_bFooterOn )
                                        nResId = bShareHeader ?
                                                    RID_SCDLG_HFEDIT_RIGHTHEADER :
                                                    RID_SCDLG_HFEDIT_LEFTHEADER;
                                }
                            }
                            break;

                            case SvxPageUsage::Mirror:
                            case SvxPageUsage::All:
                            default:
                            {
                                if ( !bShareHeader && !bShareFooter )
                                {
                                    if ( m_bHeaderOn && m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_ALL;
                                    else if ( !m_bHeaderOn && m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_FOOTER;
                                    else if ( m_bHeaderOn && !m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_HEADER;
                                }
                                else if ( bShareHeader && bShareFooter )
                                {
                                    if ( m_bHeaderOn && m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT;
                                    else
                                    {
                                        if ( !m_bHeaderOn && m_bFooterOn )
                                            nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                        else if ( m_bHeaderOn && !m_bFooterOn )
                                            nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                    }
                                }
                                else if ( !bShareHeader &&  bShareFooter )
                                {
                                    if ( m_bHeaderOn && m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_SFTR;
                                    else if ( !m_bHeaderOn && m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTFOOTER;
                                    else if ( m_bHeaderOn && !m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_HEADER;
                                }
                                else if (  bShareHeader && !bShareFooter )
                                {
                                    if ( m_bHeaderOn && m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_SHDR;
                                    else if ( !m_bHeaderOn && m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_FOOTER;
                                    else if ( m_bHeaderOn && !m_bFooterOn )
                                        nResId = RID_SCDLG_HFEDIT_RIGHTHEADER;
                                }
                            }
                        }

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                        ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScHFEditDlg(
                                                                                GetActiveDialogParent(),
                                                                                rStyleSet,
                                                                                aStr,
                                                                                nResId));
                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                            if ( pOutSet )
                                m_aDocument.ModifyStyleSheet( *pStyleSheet, *pOutSet );

                            SetDocumentModified();
                            rReq.Done();
                        }
                    }
                }
            }
            break;

        default:
        break;
    }
}

void ScDocShell::GetStatePageStyle( SfxItemSet&     rSet,
                                    SCTAB           nCurTab )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch (nWhich)
        {
            case SID_STATUS_PAGESTYLE:
                rSet.Put( SfxStringItem( nWhich, m_aDocument.GetPageStyle( nCurTab ) ) );
                break;

            case SID_HFEDIT:
                {
                    OUString            aStr        = m_aDocument.GetPageStyle( nCurTab );
                    ScStyleSheetPool*   pStylePool  = m_aDocument.GetStyleSheetPool();
                    SfxStyleSheetBase*  pStyleSheet = pStylePool->Find( aStr, SfxStyleFamily::Page );

                    OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
                        GetPageOnFromPageStyleSet( &rStyleSet, nCurTab, m_bHeaderOn, m_bFooterOn );

                        if ( !m_bHeaderOn && !m_bFooterOn )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;
        }

        nWhich = aIter.NextWhich();
    }
}

void ScDocShell::GetState( SfxItemSet &rSet )
{
    bool bTabView = GetBestViewShell() != nullptr;

    SfxWhichIter aIter(rSet);
    for (sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich())
    {
        if (!bTabView)
        {
            rSet.DisableItem(nWhich);
            continue;
        }

        switch (nWhich)
        {
            case FID_AUTO_CALC:
                if ( m_aDocument.GetHardRecalcState() != ScDocument::HardRecalcState::OFF )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich, m_aDocument.GetAutoCalc() ) );
                break;

            case FID_CHG_RECORD:
                if ( IsDocShared() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich,
                        m_aDocument.GetChangeTrack() != nullptr ) );
                break;

            case SID_CHG_PROTECT:
                {
                    ScChangeTrack* pChangeTrack = m_aDocument.GetChangeTrack();
                    if ( pChangeTrack && !IsDocShared() )
                        rSet.Put( SfxBoolItem( nWhich,
                            pChangeTrack->IsProtected() ) );
                    else
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_DOCUMENT_COMPARE:
                {
                    if ( IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            //  When a formula is edited, FID_RECALC must be enabled in any case. Recalc for
            //  the doc was disabled once because of a bug if AutoCalc was on, but is now
            //  always enabled because of another bug.

            case SID_TABLES_COUNT:
                rSet.Put( SfxInt16Item( nWhich, m_aDocument.GetTableCount() ) );
                break;

            case SID_ATTR_YEAR2000 :
                rSet.Put( SfxUInt16Item( nWhich,
                    m_aDocument.GetDocOptions().GetYear2000() ) );
            break;

            case SID_SHARE_DOC:
                {
                    if ( IsReadOnly() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_ATTR_CHAR_FONTLIST:
                rSet.Put( SvxFontListItem( m_pImpl->pFontList.get(), nWhich ) );
                break;

            case SID_NOTEBOOKBAR:
                {
                    if (GetViewBindings())
                    {
                        bool bVisible = sfx2::SfxNotebookBar::StateMethod(*GetViewBindings(),
                                                                          "modules/scalc/ui/");
                        rSet.Put( SfxBoolItem( SID_NOTEBOOKBAR, bVisible ) );
                    }
                }
                break;

            case SID_LANGUAGE_STATUS:
                {
                    LanguageType eLatin, eCjk, eCtl;

                    GetDocument().GetLanguage( eLatin, eCjk, eCtl );
                    rSet.Put(SfxStringItem(nWhich, SvtLanguageTable::GetLanguageString(eLatin)));
                }
                break;

            default:
                {
                }
                break;
        }
    }
}

void ScDocShell::Draw( OutputDevice* pDev, const JobSetup & /* rSetup */, sal_uInt16 nAspect )
{

    SCTAB nVisTab = m_aDocument.GetVisibleTab();
    if (!m_aDocument.HasTable(nVisTab))
        return;

    ComplexTextLayoutFlags nOldLayoutMode = pDev->GetLayoutMode();
    pDev->SetLayoutMode( ComplexTextLayoutFlags::Default );     // even if it's the same, to get the metafile action

    if ( nAspect == ASPECT_THUMBNAIL )
    {
        tools::Rectangle aBoundRect = GetVisArea( ASPECT_THUMBNAIL );
        ScViewData aTmpData( this, nullptr );
        aTmpData.SetTabNo(nVisTab);
        SnapVisArea( aBoundRect );
        aTmpData.SetScreen( aBoundRect );
        ScPrintFunc::DrawToDev( &m_aDocument, pDev, 1.0, aBoundRect, &aTmpData, true );
    }
    else
    {
        tools::Rectangle aBoundRect = SfxObjectShell::GetVisArea();
        ScViewData aTmpData( this, nullptr );
        aTmpData.SetTabNo(nVisTab);
        SnapVisArea( aBoundRect );
        aTmpData.SetScreen( aBoundRect );
        ScPrintFunc::DrawToDev( &m_aDocument, pDev, 1.0, aBoundRect, &aTmpData, true );
    }

    pDev->SetLayoutMode( nOldLayoutMode );
}

tools::Rectangle ScDocShell::GetVisArea( sal_uInt16 nAspect ) const
{
    SfxObjectCreateMode eShellMode = GetCreateMode();
    if ( eShellMode == SfxObjectCreateMode::ORGANIZER )
    {
        //  without contents we also don't know how large are the contents;
        //  return empty rectangle, it will then be calculated after the loading
        return tools::Rectangle();
    }

    if( nAspect == ASPECT_THUMBNAIL )
    {
        SCTAB nVisTab = m_aDocument.GetVisibleTab();
        if (!m_aDocument.HasTable(nVisTab))
        {
            nVisTab = 0;
            const_cast<ScDocShell*>(this)->m_aDocument.SetVisibleTab(nVisTab);
        }
        Size aSize = m_aDocument.GetPageSize(nVisTab);
        const long SC_PREVIEW_SIZE_X = 10000;
        const long SC_PREVIEW_SIZE_Y = 12400;
        tools::Rectangle aArea( 0,0, SC_PREVIEW_SIZE_X, SC_PREVIEW_SIZE_Y);
        if (aSize.Width() > aSize.Height())
        {
            aArea.SetRight( SC_PREVIEW_SIZE_Y );
            aArea.SetBottom( SC_PREVIEW_SIZE_X );
        }

        bool bNegativePage = m_aDocument.IsNegativePage( m_aDocument.GetVisibleTab() );
        if ( bNegativePage )
            ScDrawLayer::MirrorRectRTL( aArea );
        SnapVisArea( aArea );
        return aArea;
    }
    else if( nAspect == ASPECT_CONTENT && eShellMode != SfxObjectCreateMode::EMBEDDED )
    {
        //  fetch visarea like after loading

        SCTAB nVisTab = m_aDocument.GetVisibleTab();
        if (!m_aDocument.HasTable(nVisTab))
        {
            nVisTab = 0;
            const_cast<ScDocShell*>(this)->m_aDocument.SetVisibleTab(nVisTab);
        }
        SCCOL nStartCol;
        SCROW nStartRow;
        m_aDocument.GetDataStart( nVisTab, nStartCol, nStartRow );
        SCCOL nEndCol;
        SCROW nEndRow;
        m_aDocument.GetPrintArea( nVisTab, nEndCol, nEndRow );
        if (nStartCol>nEndCol)
            nStartCol = nEndCol;
        if (nStartRow>nEndRow)
            nStartRow = nEndRow;
        tools::Rectangle aNewArea = m_aDocument
                                .GetMMRect( nStartCol,nStartRow, nEndCol,nEndRow, nVisTab );
        //TODO/LATER: different methods for setting VisArea?!
        const_cast<ScDocShell*>(this)->SfxObjectShell::SetVisArea( aNewArea );
        return aNewArea;
    }
    else
        return SfxObjectShell::GetVisArea( nAspect );
}

namespace {

SAL_WARN_UNUSED_RESULT
long SnapHorizontal( const ScDocument& rDoc, SCTAB nTab, long nVal, SCCOL& rStartCol )
{
    SCCOL nCol = 0;
    long nTwips = static_cast<long>(nVal / HMM_PER_TWIPS);
    long nSnap = 0;
    while ( nCol<MAXCOL )
    {
        long nAdd = rDoc.GetColWidth(nCol, nTab);
        if ( nSnap + nAdd/2 < nTwips || nCol < rStartCol )
        {
            nSnap += nAdd;
            ++nCol;
        }
        else
            break;
    }
    nVal = static_cast<long>( nSnap * HMM_PER_TWIPS );
    rStartCol = nCol;
    return nVal;
}

SAL_WARN_UNUSED_RESULT
long SnapVertical( const ScDocument& rDoc, SCTAB nTab, long nVal, SCROW& rStartRow )
{
    SCROW nRow = 0;
    long nTwips = static_cast<long>(nVal / HMM_PER_TWIPS);
    long nSnap = 0;

    bool bFound = false;
    for (SCROW i = nRow; i <= MAXROW; ++i)
    {
        SCROW nLastRow;
        if (rDoc.RowHidden(i, nTab, nullptr, &nLastRow))
        {
            i = nLastRow;
            continue;
        }

        nRow = i;
        long nAdd = rDoc.GetRowHeight(i, nTab);
        if ( nSnap + nAdd/2 < nTwips || nRow < rStartRow )
        {
            nSnap += nAdd;
            ++nRow;
        }
        else
        {
            bFound = true;
            break;
        }
    }
    if (!bFound)
        nRow = MAXROW;  // all hidden down to the bottom

    nVal = static_cast<long>( nSnap * HMM_PER_TWIPS );
    rStartRow = nRow;
    return nVal;
}

}

void ScDocShell::SnapVisArea( tools::Rectangle& rRect ) const
{
    SCTAB nTab = m_aDocument.GetVisibleTab();
    bool bNegativePage = m_aDocument.IsNegativePage( nTab );
    if ( bNegativePage )
        ScDrawLayer::MirrorRectRTL( rRect );        // calculate with positive (LTR) values

    SCCOL nCol = 0;
    rRect.SetLeft( SnapHorizontal( m_aDocument, nTab, rRect.Left(), nCol ) );
    ++nCol;                                         // at least one column
    rRect.SetRight( SnapHorizontal( m_aDocument, nTab, rRect.Right(), nCol ) );

    SCROW nRow = 0;
    rRect.SetTop( SnapVertical( m_aDocument, nTab, rRect.Top(), nRow ) );
    ++nRow;                                         // at least one row
    rRect.SetBottom( SnapVertical( m_aDocument, nTab, rRect.Bottom(), nRow ) );

    if ( bNegativePage )
        ScDrawLayer::MirrorRectRTL( rRect );        // back to real rectangle
}

void ScDocShell::GetPageOnFromPageStyleSet( const SfxItemSet* pStyleSet,
                                            SCTAB             nCurTab,
                                            bool&             rbHeader,
                                            bool&             rbFooter )
{
    if ( !pStyleSet )
    {
        ScStyleSheetPool*  pStylePool  = m_aDocument.GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = pStylePool->
                                            Find( m_aDocument.GetPageStyle( nCurTab ),
                                                  SfxStyleFamily::Page );

        OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

        if ( pStyleSheet )
            pStyleSet = &pStyleSheet->GetItemSet();
        else
            rbHeader = rbFooter = false;
    }

    OSL_ENSURE( pStyleSet, "PageStyle-Set not found! :-(" );
    if (!pStyleSet)
        return;

    const SvxSetItem*   pSetItem = nullptr;
    const SfxItemSet*   pSet     = nullptr;

    pSetItem = &pStyleSet->Get( ATTR_PAGE_HEADERSET );
    pSet     = &pSetItem->GetItemSet();
    rbHeader = pSet->Get(ATTR_PAGE_ON).GetValue();

    pSetItem = &pStyleSet->Get( ATTR_PAGE_FOOTERSET );
    pSet     = &pSetItem->GetItemSet();
    rbFooter = pSet->Get(ATTR_PAGE_ON).GetValue();
}

#if defined(_WIN32)
bool ScDocShell::DdeGetData( const OUString& rItem,
                             const OUString& rMimeType,
                             css::uno::Any & rValue )
{
    SotClipboardFormatId eFormatId = SotExchange::GetFormatIdFromMimeType( rMimeType );
    if (SotClipboardFormatId::STRING == eFormatId || SotClipboardFormatId::STRING_TSVC == eFormatId)
    {
        if( rItem.equalsIgnoreAsciiCase( "Format" ) )
        {
            OString aFmtByte(OUStringToOString(m_aDdeTextFmt,
                osl_getThreadTextEncoding()));
            rValue <<= css::uno::Sequence< sal_Int8 >(
                                        reinterpret_cast<const sal_Int8*>(aFmtByte.getStr()),
                                        aFmtByte.getLength() + 1 );
            return true;
        }
        ScImportExport aObj( &m_aDocument, rItem );
        if ( !aObj.IsRef() )
            return false;                           // invalid range

        if( m_aDdeTextFmt[0] == 'F' )
            aObj.SetFormulas( true );
        if( m_aDdeTextFmt == "SYLK" ||
            m_aDdeTextFmt == "FSYLK" )
        {
            OString aData;
            if( aObj.ExportByteString( aData, osl_getThreadTextEncoding(),
                                        SotClipboardFormatId::SYLK ) )
            {
                rValue <<= css::uno::Sequence< sal_Int8 >(
                                            reinterpret_cast<const sal_Int8*>(aData.getStr()),
                                            aData.getLength() + 1 );
                return true;
            }
            else
                return false;
        }
        if( m_aDdeTextFmt == "CSV" ||
            m_aDdeTextFmt == "FCSV" )
            aObj.SetSeparator( ',' );
        aObj.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::ToSpace, 0, false ) );
        return aObj.ExportData( rMimeType, rValue );
    }

    ScImportExport aObj( &m_aDocument, rItem );
    aObj.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::ToSpace, 0, false ) );
    return aObj.IsRef() && aObj.ExportData( rMimeType, rValue );
}

bool ScDocShell::DdeSetData( const OUString& rItem,
                             const OUString& rMimeType,
                             const css::uno::Any & rValue )
{
    SotClipboardFormatId eFormatId = SotExchange::GetFormatIdFromMimeType( rMimeType );
    if (SotClipboardFormatId::STRING == eFormatId || SotClipboardFormatId::STRING_TSVC == eFormatId)
    {
        if( rItem.equalsIgnoreAsciiCase( "Format" ) )
        {
            if ( ScByteSequenceToString::GetString( m_aDdeTextFmt, rValue, osl_getThreadTextEncoding() ) )
            {
                m_aDdeTextFmt = m_aDdeTextFmt.toAsciiUpperCase();
                return true;
            }
            return false;
        }
        ScImportExport aObj( &m_aDocument, rItem );
        if( m_aDdeTextFmt[0] == 'F' )
            aObj.SetFormulas( true );
        if( m_aDdeTextFmt == "SYLK" ||
            m_aDdeTextFmt == "FSYLK" )
        {
            OUString aData;
            if ( ScByteSequenceToString::GetString( aData, rValue, osl_getThreadTextEncoding() ) )
            {
                return aObj.ImportString( aData, SotClipboardFormatId::SYLK );
            }
            return false;
        }
        if( m_aDdeTextFmt == "CSV" ||
            m_aDdeTextFmt == "FCSV" )
            aObj.SetSeparator( ',' );
        OSL_ENSURE( false, "Implementation is missing" );
        return false;
    }
    /*ScImportExport aObj( &aDocument, rItem );
    return aObj.IsRef() && ScImportExport::ImportData( rMimeType, rValue );*/
    OSL_ENSURE( false, "Implementation is missing" );
    return false;
}
#endif

::sfx2::SvLinkSource* ScDocShell::DdeCreateLinkSource( const OUString& rItem )
{
    //  only check for valid item string - range is parsed again in ScServerObject ctor

    //  named range?
    OUString aPos = rItem;
    ScRangeName* pRange = m_aDocument.GetRangeName();
    if( pRange )
    {
        const ScRangeData* pData = pRange->findByUpperName(ScGlobal::pCharClass->uppercase(aPos));
        if (pData)
        {
            if( pData->HasType( ScRangeData::Type::RefArea    )
                || pData->HasType( ScRangeData::Type::AbsArea )
                || pData->HasType( ScRangeData::Type::AbsPos  ) )
                pData->GetSymbol( aPos );           // continue with the name's contents
        }
    }

    // Address in DDE function must be always parsed as CONV_OOO so that it
    // would always work regardless of current address conversion.  We do this
    // because the address item in a DDE entry is *not* normalized when saved
    // into ODF.
    ScRange aRange;
    bool bValid = ( (aRange.Parse(aPos, &m_aDocument, formula::FormulaGrammar::CONV_OOO ) & ScRefFlags::VALID) ||
                    (aRange.aStart.Parse(aPos, &m_aDocument, formula::FormulaGrammar::CONV_OOO) & ScRefFlags::VALID) );

    ScServerObject* pObj = nullptr;            // NULL = error
    if ( bValid )
        pObj = new ScServerObject( this, rItem );

    //  GetLinkManager()->InsertServer() is in the ScServerObject ctor

    return pObj;
}

void ScDocShell::LOKCommentNotify(LOKCommentNotificationType nType, const ScDocument* pDocument, const ScAddress& rPos, const ScPostIt* pNote)
{
    if ( !pDocument->IsDocVisible() || // don't want callbacks until document load
         !comphelper::LibreOfficeKit::isActive() ||
         comphelper::LibreOfficeKit::isTiledAnnotations() )
        return;

    boost::property_tree::ptree aAnnotation;
    aAnnotation.put("action", (nType == LOKCommentNotificationType::Add ? "Add" :
                               (nType == LOKCommentNotificationType::Remove ? "Remove" :
                                (nType == LOKCommentNotificationType::Modify ? "Modify" : "???"))));

    assert(pNote);
    aAnnotation.put("id", pNote->GetId());
    aAnnotation.put("tab", rPos.Tab());

    if (nType != LOKCommentNotificationType::Remove)
    {
        aAnnotation.put("author", pNote->GetAuthor());
        aAnnotation.put("dateTime", pNote->GetDate());
        aAnnotation.put("text", pNote->GetText());

        // Calculating the cell cursor position
        ScViewData* pViewData = GetViewData();
        if (pViewData && pViewData->GetActiveWin())
        {
            Point aScrPos = pViewData->GetScrPos(rPos.Col(), rPos.Row(), pViewData->GetActivePart(), true);
            long nSizeXPix;
            long nSizeYPix;
            pViewData->GetMergeSizePixel(rPos.Col(), rPos.Row(), nSizeXPix, nSizeYPix);

            const double fPPTX = pViewData->GetPPTX();
            const double fPPTY = pViewData->GetPPTY();
            tools::Rectangle aRect(Point(aScrPos.getX() / fPPTX, aScrPos.getY() / fPPTY),
                            Size(nSizeXPix / fPPTX, nSizeYPix / fPPTY));

            aAnnotation.put("cellPos", aRect.toString());
        }
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("comment", aAnnotation);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    std::string aPayload = aStream.str();

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload.c_str());
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

ScViewData* ScDocShell::GetViewData()
{
    SfxViewShell* pCur = SfxViewShell::Current();
    ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( pCur );
    return pViewSh ? &pViewSh->GetViewData() : nullptr;
}

SCTAB ScDocShell::GetCurTab()
{
    //! this must be made non-static and use a ViewShell from this document!

    ScViewData* pViewData = GetViewData();

    return pViewData ? pViewData->GetTabNo() : static_cast<SCTAB>(0);
}

ScTabViewShell* ScDocShell::GetBestViewShell( bool bOnlyVisible )
{
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    // wrong Doc?
    if( pViewSh && pViewSh->GetViewData().GetDocShell() != this )
        pViewSh = nullptr;
    if( !pViewSh )
    {
        // 1. find ViewShell
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this, bOnlyVisible );
        if( pFrame )
        {
            SfxViewShell* p = pFrame->GetViewShell();
            pViewSh = dynamic_cast< ScTabViewShell *>( p );
        }
    }
    return pViewSh;
}

SfxBindings* ScDocShell::GetViewBindings()
{
    //  used to invalidate slots after changes to this document

    SfxViewShell* pViewSh = GetBestViewShell();
    if (pViewSh)
        return &pViewSh->GetViewFrame()->GetBindings();
    else
        return nullptr;
}

ScDocShell* ScDocShell::GetShellByNum( sal_uInt16 nDocNo )      // static
{
    ScDocShell* pFound = nullptr;
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    sal_uInt16 nShellCnt = 0;

    while ( pShell && !pFound )
    {
        if ( dynamic_cast<const ScDocShell*>(pShell) != nullptr )
        {
            if ( nShellCnt == nDocNo )
                pFound = static_cast<ScDocShell*>(pShell);
            else
                ++nShellCnt;
        }
        pShell = SfxObjectShell::GetNext( *pShell );
    }

    return pFound;
}

IMPL_LINK( ScDocShell, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    OSL_ENSURE( _pFileDlg, "ScDocShell::DialogClosedHdl(): no file dialog" );
    OSL_ENSURE( m_pImpl->pDocInserter, "ScDocShell::DialogClosedHdl(): no document inserter" );

    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        sal_uInt16 nSlot = m_pImpl->pRequest->GetSlot();
        SfxMedium* pMed = m_pImpl->pDocInserter->CreateMedium();
        // #i87094# If a .odt was selected pMed is NULL.
        if (pMed)
        {
            m_pImpl->pRequest->AppendItem( SfxStringItem( SID_FILE_NAME, pMed->GetName() ) );
            if ( SID_DOCUMENT_COMPARE == nSlot )
            {
                if ( pMed->GetFilter() )
                    m_pImpl->pRequest->AppendItem(
                            SfxStringItem( SID_FILTER_NAME, pMed->GetFilter()->GetFilterName() ) );
                OUString sOptions = ScDocumentLoader::GetOptions( *pMed );
                if ( !sOptions.isEmpty() )
                    m_pImpl->pRequest->AppendItem( SfxStringItem( SID_FILE_FILTEROPTIONS, sOptions ) );
            }
            const SfxPoolItem* pItem = nullptr;
            const SfxInt16Item* pInt16Item(nullptr);
            SfxItemSet* pSet = pMed->GetItemSet();
            if (pSet && pSet->GetItemState(SID_VERSION, true, &pItem) == SfxItemState::SET)
            {
                pInt16Item = dynamic_cast<const SfxInt16Item*>(pItem);
            }
            if (pInt16Item)
            {
                m_pImpl->pRequest->AppendItem( *pItem );
            }

            Execute( *(m_pImpl->pRequest) );
        }
    }

    m_pImpl->bIgnoreLostRedliningWarning = false;
}

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT

void ScDocShell::EnableSharedSettings( bool bEnable )
{
    SetDocumentModified();

    if ( bEnable )
    {
        m_aDocument.EndChangeTracking();
        m_aDocument.StartChangeTracking();

        // hide accept or reject changes dialog
        sal_uInt16 nId = ScAcceptChgDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        if ( pViewFrame && pViewFrame->HasChildWindow( nId ) )
        {
            pViewFrame->ToggleChildWindow( nId );
            SfxBindings* pBindings = GetViewBindings();
            if ( pBindings )
            {
                pBindings->Invalidate( FID_CHG_ACCEPT );
            }
        }
    }
    else
    {
        m_aDocument.EndChangeTracking();
    }

    ScChangeViewSettings aChangeViewSet;
    aChangeViewSet.SetShowChanges( false );
    m_aDocument.SetChangeViewSettings( aChangeViewSet );
}

uno::Reference< frame::XModel > ScDocShell::LoadSharedDocument()
{
    uno::Reference< frame::XModel > xModel;
    try
    {
        SC_MOD()->SetInSharedDocLoading( true );
        uno::Reference< frame::XDesktop2 > xLoader = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        uno::Sequence < beans::PropertyValue > aArgs( 1 );
        aArgs[0].Name = "Hidden";
        aArgs[0].Value <<= true;

        if ( GetMedium() )
        {
            const SfxStringItem* pPasswordItem = SfxItemSet::GetItem<SfxStringItem>(GetMedium()->GetItemSet(), SID_PASSWORD, false);
            if ( pPasswordItem && !pPasswordItem->GetValue().isEmpty() )
            {
                aArgs.realloc( 2 );
                aArgs[1].Name = "Password";
                aArgs[1].Value <<= pPasswordItem->GetValue();
            }
            const SfxUnoAnyItem* pEncryptionItem = SfxItemSet::GetItem<SfxUnoAnyItem>(GetMedium()->GetItemSet(), SID_ENCRYPTIONDATA, false);
            if (pEncryptionItem)
            {
                aArgs.realloc(aArgs.getLength() + 1);
                aArgs[aArgs.getLength() - 1].Name = "EncryptionData";
                aArgs[aArgs.getLength() - 1].Value = pEncryptionItem->GetValue();
            }
        }

        xModel.set(
            xLoader->loadComponentFromURL( GetSharedFileURL(), "_blank", 0, aArgs ),
            uno::UNO_QUERY_THROW );
        SC_MOD()->SetInSharedDocLoading( false );
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "ScDocShell::LoadSharedDocument(): caught exception" );
        SC_MOD()->SetInSharedDocLoading( false );
        try
        {
            uno::Reference< util::XCloseable > xClose( xModel, uno::UNO_QUERY_THROW );
            xClose->close( true );
            return uno::Reference< frame::XModel >();
        }
        catch ( uno::Exception& )
        {
            return uno::Reference< frame::XModel >();
        }
    }
    return xModel;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
