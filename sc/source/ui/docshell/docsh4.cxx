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

#include <com/sun/star/frame/Desktop.hpp>

#include <scitems.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <svtools/ehdl.hxx>
#include <svtools/langtab.hxx>
#include <basic/sbxcore.hxx>
#include <basic/sberrors.hxx>
#include <svtools/sfxecode.hxx>
#include <svx/ofaitem.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/drawitem.hxx>
#include <svx/fmshell.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svl/PasswordHelper.hxx>
#include <svl/documentlockfile.hxx>
#include <svl/sharecontrolfile.hxx>
#include <tools/json_writer.hxx>
#include <unotools/securityoptions.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sal/log.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <docuno.hxx>

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
#include <undodat.hxx>
#include <undocell.hxx>
#include <inputhdl.hxx>
#include <dbdata.hxx>
#include <servobj.hxx>
#include <rangenam.hxx>
#include <scmod.hxx>
#include <chgviset.hxx>
#include <reffact.hxx>
#include <chartlis.hxx>
#include <chartpos.hxx>
#include <tablink.hxx>
#include <drwlayer.hxx>
#include <docoptio.hxx>
#include <undostyl.hxx>
#include <rangeseq.hxx>
#include <chgtrack.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <scresid.hxx>
#include <scabstdlg.hxx>
#include <sharedocdlg.hxx>
#include <conditio.hxx>
#include <sheetevents.hxx>
#include <formulacell.hxx>
#include <documentlinkmgr.hxx>
#include <memory>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <helpids.h>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <officecfg/Office/Common.hxx>

#include <svx/xdef.hxx>

using namespace ::com::sun::star;

void ScDocShell::SetInitialLinkUpdate( const SfxMedium* pMed )
{
    if (pMed)
    {
        const SfxUInt16Item* pUpdateDocItem = pMed->GetItemSet().GetItem(SID_UPDATEDOCMODE, false);
        m_nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : css::document::UpdateDocMode::NO_UPDATE;
    }

    // GetLinkUpdateModeState() evaluates m_nCanUpdate so that must have
    // been set first. Do not override an already forbidden LinkUpdate (the
    // default is allow).
    comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = getEmbeddedObjectContainer();
    if (rEmbeddedObjectContainer.getUserAllowsLinkUpdate())
    {
        // For anything else than LM_ALWAYS we need user confirmation.
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate( GetLinkUpdateModeState() == LM_ALWAYS);
    }
}

ScLkUpdMode ScDocShell::GetLinkUpdateModeState() const
{
    ScLkUpdMode nSet;
    if (m_nCanUpdate == css::document::UpdateDocMode::NO_UPDATE)
        nSet = LM_NEVER;
    else if (m_nCanUpdate == css::document::UpdateDocMode::FULL_UPDATE)
        nSet = LM_ALWAYS;
    else
    {
        nSet = GetDocument().GetLinkMode();
        if (nSet == LM_UNKNOWN)
        {
            ScAppOptions aAppOptions = SC_MOD()->GetAppOptions();
            nSet = aAppOptions.GetLinkMode();
        }
    }

    if (nSet == LM_ALWAYS
            && !(SvtSecurityOptions::isTrustedLocationUriForUpdatingLinks(
                    GetMedium() == nullptr ? OUString() : GetMedium()->GetName())
                || (IsDocShared()
                    && SvtSecurityOptions::isTrustedLocationUriForUpdatingLinks(
                        GetSharedFileURL()))))
    {
        nSet = LM_ON_DEMAND;
    }
    if (m_nCanUpdate == css::document::UpdateDocMode::QUIET_UPDATE
            && nSet == LM_ON_DEMAND)
    {
        nSet = LM_NEVER;
    }

    return nSet;
}

void ScDocShell::AllowLinkUpdate()
{
    m_pDocument->SetLinkFormulaNeedingCheck(false);
    getEmbeddedObjectContainer().setUserAllowsLinkUpdate(true);
}

void ScDocShell::ReloadAllLinks()
{
    AllowLinkUpdate();

    ReloadTabLinks();
    weld::Window *pDialogParent = GetActiveDialogParent();
    m_pDocument->UpdateExternalRefLinks(pDialogParent);

    bool bAnyDde = m_pDocument->GetDocLinkManager().updateDdeOrOleOrWebServiceLinks(pDialogParent);

    if (bAnyDde)
    {
        //  calculate formulas and paint like in the TrackTimeHdl
        m_pDocument->TrackFormulas();
        Broadcast(SfxHint(SfxHintId::ScDataChanged));

        //  Should FID_DATACHANGED become asynchronous some time
        //  (e.g., with Invalidate at Window), an update needs to be forced here.
    }

    m_pDocument->UpdateAreaLinks();
}

IMPL_LINK( ScDocShell, ReloadAllLinksHdl, weld::Button&, rButton, void )
{
    ScDocument& rDoc = GetDocument();
    if (rDoc.HasLinkFormulaNeedingCheck() && rDoc.GetDocLinkManager().hasExternalRefLinks())
    {
        // If we have WEBSERVICE/Dde link and other external links in the document, it might indicate some
        // exfiltration attempt, add *another* warning about this on top of the "Security Warning"
        // shown in the infobar before they got here.
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(&rButton,
                                                       VclMessageType::Warning, VclButtonsType::YesNo,
                                                       ScResId(STR_TRUST_DOCUMENT_WARNING)));
        xQueryBox->set_secondary_text(ScResId(STR_WEBSERVICE_WITH_LINKS_WARNING));
        xQueryBox->set_default_response(RET_NO);
        if (xQueryBox->run() != RET_YES)
            return;
    }

    ReloadAllLinks();

    ScTabViewShell* pViewSh = GetBestViewShell();
    SfxViewFrame* pViewFrame = pViewSh ? pViewSh->GetFrame() : nullptr;
    if (pViewFrame)
        pViewFrame->RemoveInfoBar(u"enablecontent");
    SAL_WARN_IF(!pViewFrame, "sc", "expected there to be a ViewFrame");
}

namespace
{
    class LinkHelp
    {
    public:
        DECL_STATIC_LINK(LinkHelp, DispatchHelpLinksHdl, weld::Button&, void);
    };
}

IMPL_STATIC_LINK(LinkHelp, DispatchHelpLinksHdl, weld::Button&, rBtn, void)
{
    if (Help* pHelp = Application::GetHelp())
        pHelp->Start(HID_UPDATE_LINK_WARNING, &rBtn);
}

void ScDocShell::Execute( SfxRequest& rReq )
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    SfxBindings* pBindings = GetViewBindings();
    bool bUndo (m_pDocument->IsUndoEnabled());

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

                SCTAB nTabCount = m_pDocument->GetTableCount();
                if ( m_pDocument->ValidCol(nCol) && m_pDocument->ValidRow(nRow) && ValidTab(nTab,nTabCount) )
                {
                    if ( m_pDocument->IsBlockEditable( nTab, nCol,nRow, nCol, nRow ) )
                    {
                        OUString aVal = static_cast<const SfxStringItem*>(pTextItem)->GetValue();
                        m_pDocument->SetString( nCol, nRow, nTab, aVal );

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
                    ScDBCollection* pDBColl = m_pDocument->GetDBCollection();
                    if ( !pDBColl || !pDBColl->getNamedDBs().findByUpperName(ScGlobal::getCharClass().uppercase(sTarget)) )
                    {
                        ScAddress aPos;
                        if ( aPos.Parse( sTarget, *m_pDocument, m_pDocument->GetAddressConvention() ) & ScRefFlags::VALID )
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
                            assert(pDBData && "Cannot create DB data");
                            sTarget = pDBData->GetName();
                        }
                    }
                }

                // inquire, before old DB range gets overwritten
                bool bDo = true;
                if (!bIsNewArea)
                {
                    OUString aTemplate = ScResId( STR_IMPORT_REPLACE );
                    OUString aMessage = o3tl::getToken(aTemplate, 0, '#' )
                        + sTarget
                        + o3tl::getToken(aTemplate, 1, '#' );

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
            ExecuteChartSource(rReq);
            break;

        case FID_AUTO_CALC:
            {
                bool bNewVal;
                const SfxPoolItem* pItem;
                if ( pReqArgs && SfxItemState::SET == pReqArgs->GetItemState( nSlot, true, &pItem ) )
                    bNewVal = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                else
                    bNewVal = !m_pDocument->GetAutoCalc();     // Toggle for menu
                m_pDocument->SetAutoCalc( bNewVal );
                SetDocumentModified();
                if (pBindings)
                {
                    pBindings->Invalidate( FID_AUTO_CALC );
                }
                rReq.AppendItem( SfxBoolItem( FID_AUTO_CALC, bNewVal ) );
                rReq.Done();
            }
            break;
        case SID_OPEN_HYPERLINK:
            {
                ScViewData* pViewData = GetViewData();
                if ( !pViewData )
                {
                    rReq.Ignore();
                    break;
                }

                if (SC_MOD()->IsEditMode())
                {
                    if (EditView* pEditView = pViewData->GetEditView(pViewData->GetActivePart()))
                    {
                        const SvxFieldItem* pFieldItem = pEditView->GetFieldAtSelection(/*bAlsoCheckBeforeCursor=*/true);
                        const SvxFieldData* pField = pFieldItem ? pFieldItem->GetField() : nullptr;
                        if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
                        {
                            ScGlobal::OpenURL(pURLField->GetURL(), pURLField->GetTargetFrame(), true);
                            rReq.Done();
                            break;
                        }
                    }
                    rReq.Ignore();
                    break;
                }

                ScGridWindow* pWin = pViewData->GetActiveWin();
                if ( !pWin )
                {
                    rReq.Ignore();
                    break;
                }

                ScAddress aCell {pViewData->GetCurPos()};
                std::vector<UrlData> vUrls = pWin->GetEditUrls(aCell);
                if (vUrls.empty())
                {
                    rReq.Ignore();
                    break;
                }

                for (UrlData& data : vUrls)
                {
                    ScGlobal::OpenURL(data.aUrl, data.aTarget, true);
                }
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
                ScLkUpdMode nSet = GetLinkUpdateModeState();

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
                        pViewFrame->RemoveInfoBar(u"enablecontent");
                        auto pInfoBar = pViewFrame->AppendInfoBar(u"enablecontent"_ustr, SfxResId(RID_SECURITY_WARNING_TITLE),
                                                                  ScResId(STR_RELOAD_TABLES), InfobarType::WARNING);
                        if (pInfoBar)
                        {
                            weld::Button& rHelpBtn = pInfoBar->addButton();
                            rHelpBtn.set_label(GetStandardText(StandardButtonType::Help).replaceFirst("~", ""));
                            rHelpBtn.connect_clicked(LINK(nullptr, LinkHelp, DispatchHelpLinksHdl));
                            weld::Button& rBtn = pInfoBar->addButton();
                            rBtn.set_label(ScResId(STR_ENABLE_CONTENT));
                            rBtn.set_tooltip_text(ScResId(STR_ENABLE_CONTENT_TOOLTIP));
                            rBtn.connect_clicked(LINK(this, ScDocShell, ReloadAllLinksHdl));

                            // when active content is disabled the "Allow updating" button has no functionality.
                            if (officecfg::Office::Common::Security::Scripting::DisableActiveContent::get())
                            {
                                rBtn.set_tooltip_text(ScResId(STR_ENABLE_CONTENT_TOOLTIP_DISABLED));
                                rBtn.set_sensitive(false);
                            }
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
                ScDBCollection* pDBColl = m_pDocument->GetDBCollection();

                if ((m_nCanUpdate != css::document::UpdateDocMode::NO_UPDATE) &&
                   (m_nCanUpdate != css::document::UpdateDocMode::QUIET_UPDATE))
                {
                    ScRange aRange;
                    ScTabViewShell* pViewSh = GetBestViewShell();
                    OSL_ENSURE(pViewSh,"SID_REIMPORT_AFTER_LOAD: no View");
                    if (pViewSh && pDBColl)
                    {
                        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetActiveDialogParent(),
                                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                                       ScResId(STR_REIMPORT_AFTER_LOAD)));
                        xQueryBox->set_default_response(RET_YES);
                        if (xQueryBox->run() == RET_YES)
                        {
                            ScDBCollection::NamedDBs& rDBs = pDBColl->getNamedDBs();
                            for (const auto& rxDB : rDBs)
                            {
                                ScDBData& rDBData = *rxDB;
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

                    m_pDocument->CalcAll();        //! only for the dependent
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
                const XColorListRef& pList = pColItem->GetColorList();
                rReq.SetReturnValue(OfaXColorListItem(SID_GET_COLORLIST, pList));
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
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetActiveDialogParent(),
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
                ScChangeTrack* pChangeTrack = m_pDocument->GetChangeTrack();
                if ( pChangeTrack && !m_pImpl->bIgnoreLostRedliningWarning )
                {
                    if ( nSlot == SID_DOCUMENT_COMPARE )
                    {   //! old changes trace will be lost
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetActiveDialogParent(),
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
                const SfxStringItem* pFileNameItem(nullptr);
                SfxMedium* pMed = nullptr;
                if (pReqArgs)
                    pFileNameItem = pReqArgs->GetItemIfSet(SID_FILE_NAME);
                if (pFileNameItem)
                {
                    OUString aFileName = pFileNameItem->GetValue();

                    OUString aFilterName;
                    if (const SfxStringItem* pFilterItem = pReqArgs->GetItemIfSet(SID_FILTER_NAME))
                    {
                        aFilterName = pFilterItem->GetValue();
                    }
                    OUString aOptions;
                    if (const SfxStringItem* pOptionsItem = pReqArgs->GetItemIfSet(SID_FILE_FILTEROPTIONS))
                    {
                        aOptions = pOptionsItem->GetValue();
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
                    auto pSet = std::make_shared<SfxAllItemSet>( pApp->GetPool() );
                    if (!aOptions.isEmpty())
                        pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );
                    if ( nVersion != 0 )
                        pSet->Put( SfxInt16Item( SID_VERSION, nVersion ) );
                    pMed = new SfxMedium( aFileName, StreamMode::STD_READ, pFilter, std::move(pSet) );
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
                    weld::Window* pParent = pViewSh ? pViewSh->GetFrameWeld() : nullptr;
                    m_pImpl->pDocInserter.reset( new ::sfx2::DocumentInserter(pParent,
                        ScDocShell::Factory().GetFactoryName(), mode ) );
                    m_pImpl->pDocInserter->StartExecuteModal( LINK( this, ScDocShell, DialogClosedHdl ) );
                    return ;
                }

                // now execute in earnest...
                SfxErrorContext aEc( ERRCTX_SFX_OPENDOC, pMed->GetName() );

                // pOtherDocSh->DoClose() will be called explicitly later, but it is still more safe to use SfxObjectShellLock here
                rtl::Reference<ScDocShell> pOtherDocSh = new ScDocShell;
                pOtherDocSh->DoLoad( pMed );
                ErrCodeMsg nErr = pOtherDocSh->GetErrorCode();
                if (nErr)
                    ErrorHandler::HandleError( nErr );          // also warnings

                if ( !pOtherDocSh->GetErrorIgnoreWarning() )                 // only errors
                {
                    bool bHadTrack = ( m_pDocument->GetChangeTrack() != nullptr );
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

                    rReq.SetReturnValue( SfxInt32Item( TypedWhichId<SfxInt32Item>(nSlot), 0 ) );        //! ???????
                    rReq.Done();

                    if (!bHadTrack)         //  newly turned on -> show as well
                    {
                        ScChangeViewSettings* pOldSet = m_pDocument->GetChangeViewSettings();
                        if ( !pOldSet || !pOldSet->ShowChanges() )
                        {
                            ScChangeViewSettings aChangeViewSet;
                            aChangeViewSet.SetShowChanges(true);
                            m_pDocument->SetChangeViewSettings(aChangeViewSet);
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
                            m_pDocument->SetChangeViewSettings( aChangeViewSet );

                            // update view
                            PostPaintExtras();
                            PostPaintGridAll();
                        }
                    }
#endif
                }
                pOtherDocSh->DoClose();     // delete happens with the Ref
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
                        const OUString& aName = pStringItem->GetValue();
                        SCTAB nTab;
                        if (m_pDocument->GetTable( aName, nTab ))
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
                        if (m_pDocument->GetTable( aName, nTab ))
                        {
                            if (m_pDocument->IsScenario(nTab))
                            {
                                OUString aComment;
                                Color aColor;
                                ScScenarioFlags nFlags;
                                m_pDocument->GetScenarioData( nTab, aComment, aColor, nFlags );

                                // Determine if the Sheet that the Scenario was created on
                                // is protected. But first we need to find that Sheet.
                                // Rewind back to the actual sheet.
                                SCTAB nActualTab = nTab;
                                do
                                {
                                    nActualTab--;
                                }
                                while(m_pDocument->IsScenario(nActualTab));
                                bool bSheetProtected = m_pDocument->IsTabProtected(nActualTab);

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
                    ScDocOptions aDocOpt( m_pDocument->GetDocOptions() );
                    aDocOpt.SetYear2000( nY2k );
                    m_pDocument->SetDocOptions( aDocOpt );
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

                weld::Window* pWin = GetActiveDialogParent();
                ScShareDocumentDlg aDlg(pWin, pViewData);
                if (aDlg.run() == RET_OK)
                {
                    bool bSetShared = aDlg.IsShareDocumentChecked();
                    if ( bSetShared != IsDocShared() )
                    {
                        if ( bSetShared )
                        {
                            bool bContinue = true;
                            if ( HasName() )
                            {
                                std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin,
                                                                               VclMessageType::Question, VclButtonsType::YesNo,
                                                                               ScResId(STR_DOC_WILLBESAVED)));
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

                                ScTabView* pTabView = pViewData->GetView();
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
                                xModel.set( LoadSharedDocument(), uno::UNO_SET_THROW );
                                uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY_THROW );

                                // check if shared flag is set in shared file
                                bool bShared = false;
                                ScModelObj* pDocObj = comphelper::getFromUnoTunnel<ScModelObj>( xModel );
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

                                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin,
                                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                                   aMessage));
                                        xWarn->run();
                                    }
                                    else
                                    {
                                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin,
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

                                            // Do *not* use dispatch mechanism in this place - we don't want others (extensions etc.) to intercept this.
                                            GetModel()->store();

                                            ScTabView* pTabView = pViewData->GetView();
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
                                    std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin,
                                                                               VclMessageType::Warning, VclButtonsType::Ok,
                                                                               ScResId(STR_DOC_NOLONGERSHARED)));
                                    xWarn->run();
                                }
                            }
                            catch ( uno::Exception& )
                            {
                                TOOLS_WARN_EXCEPTION( "sc", "SID_SHARE_DOC" );
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
            ScViewData* pViewData = GetViewData();
            if (pViewData)
            {
                SfxStringItem aApp(SID_DOC_SERVICE, u"com.sun.star.sheet.SpreadsheetDocument"_ustr);
                SfxStringItem aTarget(SID_TARGETNAME, u"_blank"_ustr);
                pViewData->GetDispatcher().ExecuteList(
                    SID_OPENDOC, SfxCallMode::API|SfxCallMode::SYNCHRON,
                    { &aApp, &aTarget });
            }
        }
        break;
        case SID_NOTEBOOKBAR:
        {
            const SfxStringItem* pFile = rReq.GetArg<SfxStringItem>( SID_NOTEBOOKBAR );

            if ( pBindings && sfx2::SfxNotebookBar::IsActive() )
                sfx2::SfxNotebookBar::ExecMethod(*pBindings, pFile ? pFile->GetValue() : u""_ustr);
            else if ( pBindings )
                sfx2::SfxNotebookBar::CloseMethod(*pBindings);
        }
        break;
        case SID_LANGUAGE_STATUS:
        {
            OUString aLangText;
            const SfxStringItem* pItem = rReq.GetArg<SfxStringItem>(nSlot);
            if ( pItem )
                aLangText = pItem->GetValue();

            if ( !aLangText.isEmpty() )
            {
                LanguageType eLang, eLatin, eCjk, eCtl;
                static constexpr OUString aSelectionLangPrefix(u"Current_"_ustr);
                static constexpr OUString aParagraphLangPrefix(u"Paragraph_"_ustr);
                static constexpr OUString aDocLangPrefix(u"Default_"_ustr);

                bool bSelection = false;
                bool bParagraph = false;

                ScDocument& rDoc = GetDocument();
                rDoc.GetLanguage( eLatin, eCjk, eCtl );

                sal_Int32 nPos = 0;
                if ( aLangText == "*" )
                {
                    if (ScTabViewShell* pSh = GetBestViewShell())
                    {
                        pSh->ExecuteCellFormatDlg(rReq, u"font"_ustr);
                        pBindings->Invalidate(SID_LANGUAGE_STATUS);
                    }
                }
                else if ( (nPos = aLangText.indexOf(aDocLangPrefix)) != -1 )
                {
                    aLangText = aLangText.replaceAt(nPos, aDocLangPrefix.getLength(), u"");

                    if ( aLangText == "LANGUAGE_NONE" )
                    {
                        eLang = LANGUAGE_NONE;
                        rDoc.SetLanguage( eLang, eCjk, eCtl );
                    }
                    else if ( aLangText == "RESET_LANGUAGES" )
                    {
                        ScModule::GetSpellSettings(eLang, eCjk, eCtl);
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
                else if (-1 != (nPos = aLangText.indexOf( aSelectionLangPrefix )))
                {
                    bSelection = true;
                    aLangText = aLangText.replaceAt( nPos, aSelectionLangPrefix.getLength(), u"" );
                }
                else if (-1 != (nPos = aLangText.indexOf( aParagraphLangPrefix )))
                {
                    bParagraph = true;
                    aLangText = aLangText.replaceAt( nPos, aParagraphLangPrefix.getLength(), u"" );
                }

                if (bSelection)
                {
                    ScTabViewShell* pViewShell = GetBestViewShell();
                    if (pViewShell)
                    {
                        const ScPatternAttr* pSelAttrs = pViewShell->GetSelectionPattern();
                        if (pSelAttrs)
                        {
                            const SfxItemSet& rOldSet = pSelAttrs->GetItemSet();
                            SfxItemPool* pItemPool = rOldSet.GetPool();
                            auto pNewSet = std::make_shared<SfxItemSet>(rOldSet);

                            if (aLangText == "LANGUAGE_NONE")
                            {
                                pNewSet->Put(SvxLanguageItem(LANGUAGE_NONE,
                                                             pItemPool->GetWhichIDFromSlotID(SID_ATTR_CHAR_LANGUAGE)));
                                pNewSet->Put(SvxLanguageItem(LANGUAGE_NONE,
                                                             pItemPool->GetWhichIDFromSlotID(SID_ATTR_CHAR_CJK_LANGUAGE)));
                                pNewSet->Put(SvxLanguageItem(LANGUAGE_NONE,
                                                             pItemPool->GetWhichIDFromSlotID(SID_ATTR_CHAR_CTL_LANGUAGE)));
                            }
                            else
                            {
                                const LanguageType nLangType = SvtLanguageTable::GetLanguageType(aLangText);
                                const SvtScriptType nScriptType =
                                    SvtLanguageOptions::GetScriptTypeOfLanguage(nLangType);
                                if (nScriptType == SvtScriptType::LATIN)
                                    pNewSet->Put(SvxLanguageItem(nLangType,
                                                                 pItemPool->GetWhichIDFromSlotID(SID_ATTR_CHAR_LANGUAGE)));
                                if (nScriptType == SvtScriptType::COMPLEX)
                                    pNewSet->Put(SvxLanguageItem(nLangType,
                                                                 pItemPool->GetWhichIDFromSlotID(SID_ATTR_CHAR_CTL_LANGUAGE)));
                                if (nScriptType == SvtScriptType::ASIAN)
                                    pNewSet->Put(SvxLanguageItem(nLangType,
                                                                 pItemPool->GetWhichIDFromSlotID(SID_ATTR_CHAR_CJK_LANGUAGE)));
                            }
                            pViewShell->ApplyAttributes(*pNewSet, rOldSet);
                            pBindings->Invalidate(SID_LANGUAGE_STATUS);
                        }
                    }
                }
                else if (bParagraph)
                {
                    ScViewData* pViewData = GetViewData();
                    if (!pViewData)
                        return;

                    EditView* pEditView = pViewData->GetEditView(pViewData->GetActivePart());
                    if (!pEditView)
                        return;

                    const LanguageType nLangToUse = SvtLanguageTable::GetLanguageType( aLangText );
                    SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( nLangToUse );

                    SfxItemSet aAttrs = pEditView->getEditEngine().GetEmptyItemSet();
                    if (nScriptType == SvtScriptType::LATIN)
                        aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE ) );
                    if (nScriptType == SvtScriptType::COMPLEX)
                        aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CTL ) );
                    if (nScriptType == SvtScriptType::ASIAN)
                        aAttrs.Put( SvxLanguageItem( nLangToUse, EE_CHAR_LANGUAGE_CJK ) );
                    ESelection aOldSel;
                    if (bParagraph)
                    {
                        ESelection aSel = pEditView->GetSelection();
                        aOldSel = aSel;
                        aSel.nStartPos = 0;
                        aSel.nEndPos = EE_TEXTPOS_ALL;
                        pEditView->SetSelection( aSel );
                    }

                    pEditView->SetAttribs( aAttrs );
                    if (bParagraph)
                        pEditView->SetSelection( aOldSel );
                }
                else if ( eLang != eLatin )
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
        case SID_SPELLCHECK_IGNORE_ALL:
        {
            ScViewData* pViewData = GetViewData();
            if (!pViewData)
                return;

            EditView* pEditView = pViewData->GetEditView(pViewData->GetActivePart());
            if (!pEditView)
                return;

            OUString sIgnoreText;
            const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
            if (pItem2)
                sIgnoreText = pItem2->GetValue();

            if(sIgnoreText == "Spelling")
            {
                ESelection aOldSel = pEditView->GetSelection();
                pEditView->SpellIgnoreWord();
                pEditView->SetSelection( aOldSel );
            }
        }
        break;
        case SID_SPELLCHECK_APPLY_SUGGESTION:
        {
            ScViewData* pViewData = GetViewData();
            if (!pViewData)
                return;

            EditView* pEditView = pViewData->GetEditView(pViewData->GetActivePart());
            if (!pEditView)
                return;

            OUString sApplyText;
            const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
            if (pItem2)
                sApplyText = pItem2->GetValue();

            static constexpr OUString sSpellingRule(u"Spelling_"_ustr);
            sal_Int32 nPos = 0;
            if(-1 != (nPos = sApplyText.indexOf( sSpellingRule )))
            {
                sApplyText = sApplyText.replaceAt(nPos, sSpellingRule.getLength(), u"");
                pEditView->InsertText( sApplyText );
            }
        }
        break;
        case SID_REFRESH_VIEW:
        {
            PostPaintGridAll();
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

void ScDocShell::ExecuteChartSource(SfxRequest& rReq)
{
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    sal_uInt16 nSlot = rReq.GetSlot();
    bool bUndo (m_pDocument->IsUndoEnabled());
    if (!pReqArgs)
    {
        OSL_FAIL("SID_CHART_SOURCE without arguments");
        return;
    }

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

    if( const SfxStringItem* pChartItem = pReqArgs->GetItemIfSet( SID_CHART_NAME ) )
        aChartName = pChartItem->GetValue();

    if( const SfxStringItem* pChartItem = pReqArgs->GetItemIfSet( SID_CHART_SOURCE ) )
        aRangeName = pChartItem->GetValue();

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
    bool bValid = (aSingleRange.ParseAny(aRangeName, rDoc, aDetails) & ScRefFlags::VALID) != ScRefFlags::ZERO;
    if (!bValid)
    {
        aRangeListRef = new ScRangeList;
        aRangeListRef->Parse( aRangeName, rDoc, rDoc.GetAddressConvention());
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
    if (!pViewSh || !bValid || aChartName.isEmpty() )
    {
        OSL_FAIL("UpdateChartArea: no ViewShell or wrong data");
        rReq.Done();
        return;
    }

    weld::Window* pParent = pViewSh->GetFrameWeld();

    SCCOL nCol1 = aSingleRange.aStart.Col();
    SCROW nRow1 = aSingleRange.aStart.Row();
    SCCOL nCol2 = aSingleRange.aEnd.Col();
    SCROW nRow2 = aSingleRange.aEnd.Row();
    SCTAB nTab = aSingleRange.aStart.Tab();

    //! limit always or not at all ???
    if (!bMultiRange)
        m_pDocument->LimitChartArea( nTab, nCol1,nRow1, nCol2,nRow2 );

    // Dialog for column/row headers
    if ( !bAddRange && ( !bColInit || !bRowInit ) )
    {
        ScChartPositioner aChartPositioner( *m_pDocument, nTab, nCol1,nRow1, nCol2,nRow2 );
        if (!bColInit)
            bColHeaders = aChartPositioner.HasColHeaders();
        if (!bRowInit)
            bRowHeaders = aChartPositioner.HasRowHeaders();

        auto xRequest = std::make_shared<SfxRequest>(rReq);
        rReq.Ignore(); // the 'old' request is not relevant any more
        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        VclPtr<AbstractScColRowLabelDlg> pDlg(pFact->CreateScColRowLabelDlg(pParent, bRowHeaders, bColHeaders));
        pDlg->StartExecuteAsync(
            [this, pDlg, xRequest=std::move(xRequest), bUndo, bMultiRange, aChartName, aRangeListRef, bAddRange,
             nCol1, nRow1, nCol2, nRow2, nTab] (sal_Int32 nResult)->void
            {
                if (nResult == RET_OK)
                {
                    bool bColHeaders2 = pDlg->IsRow();
                    bool bRowHeaders2 = pDlg->IsCol();

                    xRequest->AppendItem(SfxBoolItem(FN_PARAM_1, bColHeaders2));
                    xRequest->AppendItem(SfxBoolItem(FN_PARAM_2, bRowHeaders2));
                    ExecuteChartSourcePost(bUndo, bMultiRange,
                        aChartName, aRangeListRef, bColHeaders2, bRowHeaders2, bAddRange,
                        nCol1, nRow1, nCol2, nRow2, nTab);
                }
                pDlg->disposeOnce();
                xRequest->Done();
            }
        );
    }
    else
    {
        ExecuteChartSourcePost(bUndo, bMultiRange,
            aChartName, aRangeListRef, bColHeaders, bRowHeaders, bAddRange,
            nCol1, nRow1,nCol2, nRow2, nTab);
        rReq.Done();
    }
}

void ScDocShell::ExecuteChartSourcePost(bool bUndo, bool bMultiRange,
    const OUString& rChartName, const ScRangeListRef& rRangeListRef,
    bool bColHeaders, bool bRowHeaders, bool bAddRange,
    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCTAB nTab )
{
    if (bMultiRange)
    {
        if (bUndo)
        {
            GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoChartData>( this, rChartName, rRangeListRef,
                                        bColHeaders, bRowHeaders, bAddRange ) );
        }
        m_pDocument->UpdateChartArea( rChartName, rRangeListRef,
                                    bColHeaders, bRowHeaders, bAddRange );
    }
    else
    {
        ScRange aNewRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
        if (bUndo)
        {
            GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoChartData>( this, rChartName, aNewRange,
                                        bColHeaders, bRowHeaders, bAddRange ) );
        }
        m_pDocument->UpdateChartArea( rChartName, aNewRange,
                                    bColHeaders, bRowHeaders, bAddRange );
    }
}

bool ScDocShell::ExecuteChangeProtectionDialog( bool bJustQueryIfProtected )
{
    bool bDone = false;
    ScChangeTrack* pChangeTrack = m_pDocument->GetChangeTrack();
    if ( pChangeTrack )
    {
        bool bProtected = pChangeTrack->IsProtected();
        if ( bJustQueryIfProtected && !bProtected )
            return true;

        OUString aTitle( ScResId( bProtected ? SCSTR_CHG_UNPROTECT : SCSTR_CHG_PROTECT ) );
        OUString aText( ScResId( SCSTR_PASSWORD ) );
        OUString aPassword;

        weld::Window* pWin = ScDocShell::GetActiveDialogParent();
        SfxPasswordDialog aDlg(pWin, &aText);
        aDlg.set_title(aTitle);
        aDlg.SetMinLen(1);
        aDlg.set_help_id(GetStaticInterface()->GetSlot(SID_CHG_PROTECT)->GetCommand());
        aDlg.SetEditHelpId( HID_CHG_PROTECT );
        if ( !bProtected )
            aDlg.ShowExtras(SfxShowExtras::CONFIRM);
        if (aDlg.run() == RET_OK)
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
                        pChangeTrack->SetProtection( {} );
                }
                else
                {
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin,
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
    if (m_pDocument->IsInDocShellRecalc())
    {
        SAL_WARN("sc","ScDocShell::DoRecalc tries re-entering while in Recalc; probably Forms->BASIC->Dispatcher.");
        return;
    }
    ScDocShellRecalcGuard aGuard(*m_pDocument);
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
    if (bDone)                         // otherwise re-calculate document
        return;

    weld::WaitObject aWaitObj( GetActiveDialogParent() );
    if ( pHdl )
    {
        // tdf97897 set current cell to Dirty to force recalculation of cell
        ScFormulaCell* pFC = m_pDocument->GetFormulaCell( pHdl->GetCursorPos());
        if (pFC)
            pFC->SetDirty();
    }
    m_pDocument->CalcFormulaTree();
    if ( pSh )
        pSh->UpdateCharts(true);

    m_pDocument->BroadcastUno( SfxHint( SfxHintId::DataChanged ) );

    //  If there are charts, then paint everything, so that PostDataChanged
    //  and the charts do not come one after the other and parts are painted twice.

    ScChartListenerCollection* pCharts = m_pDocument->GetChartListenerCollection();
    if ( pCharts && pCharts->hasListeners() )
        PostPaintGridAll();
    else
        PostDataChanged();
}

void ScDocShell::DoHardRecalc()
{
    if (m_pDocument->IsInDocShellRecalc())
    {
        SAL_WARN("sc","ScDocShell::DoHardRecalc tries re-entering while in Recalc; probably Forms->BASIC->Dispatcher.");
        return;
    }
    auto start = std::chrono::steady_clock::now();
    ScDocShellRecalcGuard aGuard(*m_pDocument);
    weld::WaitObject aWaitObj( GetActiveDialogParent() );
    ScTabViewShell* pSh = GetBestViewShell();
    if ( pSh )
    {
        ScTabView::UpdateInputLine();     // InputEnterHandler
        pSh->UpdateInputHandler();
    }
    m_pDocument->CalcAll();
    GetDocFunc().DetectiveRefresh();    // creates own Undo
    if ( pSh )
        pSh->UpdateCharts(true);

    // set notification flags for "calculate" event (used in SfxHintId::DataChanged broadcast)
    // (might check for the presence of any formulas on each sheet)
    SCTAB nTabCount = m_pDocument->GetTableCount();
    if (m_pDocument->HasAnySheetEventScript( ScSheetEventId::CALCULATE, true )) // search also for VBA handler
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
            m_pDocument->SetCalcNotification(nTab);

    // CalcAll doesn't broadcast value changes, so SfxHintId::ScCalcAll is broadcasted globally
    // in addition to SfxHintId::DataChanged.
    m_pDocument->BroadcastUno( SfxHint( SfxHintId::ScCalcAll ) );
    m_pDocument->BroadcastUno( SfxHint( SfxHintId::DataChanged ) );

    // use hard recalc also to disable stream-copying of all sheets
    // (somewhat consistent with charts)
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        m_pDocument->SetStreamValid(nTab, false);

    PostPaintGridAll();
    auto end = std::chrono::steady_clock::now();
    SAL_INFO("sc.timing", "ScDocShell::DoHardRecalc(): took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms");
}

void ScDocShell::DoAutoStyle( const ScRange& rRange, const OUString& rStyle )
{
    ScStyleSheetPool* pStylePool = m_pDocument->GetStyleSheetPool();
    ScStyleSheet* pStyleSheet = pStylePool->FindAutoStyle(rStyle);
    if (!pStyleSheet)
        return;

    OSL_ENSURE(rRange.aStart.Tab() == rRange.aEnd.Tab(),
                    "DoAutoStyle with several tables");
    SCTAB nTab = rRange.aStart.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    m_pDocument->ApplyStyleAreaTab( nStartCol, nStartRow, nEndCol, nEndRow, nTab, *pStyleSheet );
    m_pDocument->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow, nTab );
    PostPaint( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab, PaintPartFlags::Grid );
}

void ScDocShell::NotifyStyle( const SfxStyleSheetHint& rHint )
{
    SfxHintId nId = rHint.GetId();
    const SfxStyleSheetBase* pStyle = rHint.GetStyleSheet();
    if (!pStyle)
        return;

    if ( pStyle->GetFamily() == SfxStyleFamily::Page )
    {
        if ( nId == SfxHintId::StyleSheetModified || nId == SfxHintId::StyleSheetModifiedExtended )
        {
            ScDocShellModificator aModificator( *this );

            const OUString& aNewName = pStyle->GetName();
            OUString aOldName = aNewName;
            if ( nId == SfxHintId::StyleSheetModifiedExtended )
            {
                const SfxStyleSheetModifiedHint& rExtendedHint = static_cast<const SfxStyleSheetModifiedHint&>(rHint); // name changed?
                aOldName = rExtendedHint.GetOldName();
            }
            if ( aNewName != aOldName )
                m_pDocument->RenamePageStyleInUse( aOldName, aNewName );

            SCTAB nTabCount = m_pDocument->GetTableCount();
            for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                if (m_pDocument->GetPageStyle(nTab) == aNewName)   // already adjusted to new
                {
                    m_pDocument->PageStyleModified( nTab, aNewName );
                    ScPrintFunc aPrintFunc( this, GetPrinter(), nTab );
                    aPrintFunc.UpdatePages();
                }

            aModificator.SetDocumentModified();

            if (nId == SfxHintId::StyleSheetModifiedExtended)
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
        if (nId == SfxHintId::StyleSheetModifiedExtended)
        {
            const SfxStyleSheetModifiedHint& rExtendedHint = static_cast<const SfxStyleSheetModifiedHint&>(rHint);
            const OUString& aNewName = pStyle->GetName();
            const OUString& aOldName = rExtendedHint.GetOldName();
            if ( aNewName != aOldName )
            {
                for(SCTAB i = 0; i < m_pDocument->GetTableCount(); ++i)
                {
                    ScConditionalFormatList* pList = m_pDocument->GetCondFormList(i);
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
    OUString aStyleName = m_pDocument->GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = m_pDocument->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SfxStyleFamily::Page );
    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
    if ( !pStyleSheet )
        return;

    ScDocShellModificator aModificator( *this );

    SfxItemSet& rSet = pStyleSheet->GetItemSet();
    const bool bUndo(m_pDocument->IsUndoEnabled());
    if (bUndo)
    {
        sal_uInt16 nOldScale = rSet.Get(ATTR_PAGE_SCALE).GetValue();
        sal_uInt16 nOldPages = rSet.Get(ATTR_PAGE_SCALETOPAGES).GetValue();
        GetUndoManager()->AddUndoAction( std::make_unique<ScUndoPrintZoom>(
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

bool ScDocShell::AdjustPrintZoom( const ScRange& rRange )
{
    bool bChange = false;
    SCTAB nTab = rRange.aStart.Tab();

    OUString aStyleName = m_pDocument->GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = m_pDocument->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SfxStyleFamily::Page );
    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        bool bHeaders = rSet.Get(ATTR_PAGE_HEADERS).GetValue();
        sal_uInt16 nOldScale = rSet.Get(ATTR_PAGE_SCALE).GetValue();
        sal_uInt16 nOldPages = rSet.Get(ATTR_PAGE_SCALETOPAGES).GetValue();
        std::optional<ScRange> oRepeatCol = m_pDocument->GetRepeatColRange( nTab );
        std::optional<ScRange> oRepeatRow = m_pDocument->GetRepeatRowRange( nTab );

        //  calculate needed scaling for selection

        sal_uInt16 nNewScale = nOldScale;

        tools::Long nBlkTwipsX = 0;
        if (bHeaders)
            nBlkTwipsX += PRINT_HEADER_WIDTH;
        SCCOL nStartCol = rRange.aStart.Col();
        SCCOL nEndCol = rRange.aEnd.Col();
        if ( oRepeatCol && nStartCol >= oRepeatCol->aStart.Col() )
        {
            for (SCCOL i=oRepeatCol->aStart.Col(); i<=oRepeatCol->aEnd.Col(); i++ )
                nBlkTwipsX += m_pDocument->GetColWidth( i, nTab );
            if ( nStartCol <= oRepeatCol->aEnd.Col() )
                nStartCol = oRepeatCol->aEnd.Col() + 1;
        }
        // legacy compilers' own scope for i
        {
            for ( SCCOL i=nStartCol; i<=nEndCol; i++ )
                nBlkTwipsX += m_pDocument->GetColWidth( i, nTab );
        }

        tools::Long nBlkTwipsY = 0;
        if (bHeaders)
            nBlkTwipsY += PRINT_HEADER_HEIGHT;
        SCROW nStartRow = rRange.aStart.Row();
        SCROW nEndRow = rRange.aEnd.Row();
        if ( oRepeatRow && nStartRow >= oRepeatRow->aStart.Row() )
        {
            nBlkTwipsY += m_pDocument->GetRowHeight( oRepeatRow->aStart.Row(),
                    oRepeatRow->aEnd.Row(), nTab );
            if ( nStartRow <= oRepeatRow->aEnd.Row() )
                nStartRow = oRepeatRow->aEnd.Row() + 1;
        }
        nBlkTwipsY += m_pDocument->GetRowHeight( nStartRow, nEndRow, nTab );

        Size aPhysPage;
        tools::Long nHdr, nFtr;
        ScPrintFunc aOldPrFunc( this, GetPrinter(), nTab );
        aOldPrFunc.GetScaleData( aPhysPage, nHdr, nFtr );
        nBlkTwipsY += nHdr + nFtr;

        if ( nBlkTwipsX == 0 )      // hidden columns/rows may lead to 0
            nBlkTwipsX = 1;
        if ( nBlkTwipsY == 0 )
            nBlkTwipsY = 1;

        tools::Long nNeeded = std::min( aPhysPage.Width()  * 100 / nBlkTwipsX,
                            aPhysPage.Height() * 100 / nBlkTwipsY );
        if ( nNeeded < ZOOM_MIN )
            nNeeded = ZOOM_MIN;         // boundary
        if ( nNeeded < static_cast<tools::Long>(nNewScale) )
            nNewScale = static_cast<sal_uInt16>(nNeeded);

        bChange = ( nNewScale != nOldScale || nOldPages != 0 );
        if ( bChange )
            SetPrintZoom( nTab, nNewScale, 0 );
    }
    return bChange;
}

void ScDocShell::PageStyleModified( std::u16string_view rStyleName, bool bApi )
{
    ScDocShellModificator aModificator( *this );

    SCTAB nTabCount = m_pDocument->GetTableCount();
    SCTAB nUseTab = MAXTAB+1;
    for (SCTAB nTab=0; nTab<nTabCount && nUseTab>MAXTAB; nTab++)
        if ( m_pDocument->GetPageStyle(nTab) == rStyleName &&
                ( !bApi || m_pDocument->GetPageSize(nTab).Width() ) )
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
            weld::Window* pWin = GetActiveDialogParent();
            weld::WaitObject aWaitOff(pWin);
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin,
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
                    OUString aOldName = m_pDocument->GetPageStyle( nCurTab );
                    ScStyleSheetPool* pStylePool = m_pDocument->GetStyleSheetPool();
                    SfxStyleSheetBase* pStyleSheet
                        = pStylePool->Find( aOldName, SfxStyleFamily::Page );

                    OSL_ENSURE( pStyleSheet, "PageStyle not found! :-/" );

                    if ( pStyleSheet )
                    {
                        ScStyleSaveData aOldData;
                        const bool bUndo(m_pDocument->IsUndoEnabled());
                        if (bUndo)
                            aOldData.InitFromStyle( pStyleSheet );

                        SfxItemSet&     rStyleSet = pStyleSheet->GetItemSet();
                        rStyleSet.MergeRange( XATTR_FILL_FIRST, XATTR_FILL_LAST );

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

                        VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScStyleDlg(GetActiveDialogParent(), *pStyleSheet, true));

                        auto xRequest = std::make_shared<SfxRequest>(rReq);
                        rReq.Ignore(); // the 'old' request is not relevant any more
                        pDlg->StartExecuteAsync([this, pDlg, xRequest=std::move(xRequest), pStyleSheet, aOldData,
                                                 aOldName, &rStyleSet, nCurTab, &rCaller, bUndo](sal_Int32 nResult) {
                            if ( nResult == RET_OK )
                            {
                                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                                weld::WaitObject aWait( GetActiveDialogParent() );

                                OUString aNewName = pStyleSheet->GetName();
                                if ( aNewName != aOldName &&
                                    m_pDocument->RenamePageStyleInUse( aOldName, aNewName ) )
                                {
                                    SfxBindings* pBindings = GetViewBindings();
                                    if (pBindings)
                                    {
                                        pBindings->Invalidate( SID_STATUS_PAGESTYLE );
                                        pBindings->Invalidate( FID_RESET_PRINTZOOM );
                                    }
                                }

                                if ( pOutSet )
                                    m_pDocument->ModifyStyleSheet( *pStyleSheet, *pOutSet );

                                // memorizing for GetState():
                                GetPageOnFromPageStyleSet( &rStyleSet, nCurTab, m_bHeaderOn, m_bFooterOn );
                                rCaller.GetViewFrame().GetBindings().Invalidate( SID_HFEDIT );

                                ScStyleSaveData aNewData;
                                aNewData.InitFromStyle( pStyleSheet );
                                if (bUndo)
                                {
                                    GetUndoManager()->AddUndoAction(
                                            std::make_unique<ScUndoModifyStyle>( this, SfxStyleFamily::Page,
                                                        aOldData, aNewData ) );
                                }

                                PageStyleModified( aNewName, false );
                                xRequest->Done();
                            }
                            pDlg->disposeOnce();
                        });
                    }
                }
            }
            break;

        case SID_HFEDIT:
            {
                if ( pReqArgs == nullptr )
                {
                    OUString aStr( m_pDocument->GetPageStyle( nCurTab ) );

                    ScStyleSheetPool* pStylePool
                        = m_pDocument->GetStyleSheetPool();

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

                        VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScHFEditDlg(
                                                                                GetActiveDialogParent(),
                                                                                rStyleSet,
                                                                                aStr,
                                                                                nResId));
                        auto xRequest = std::make_shared<SfxRequest>(rReq);
                        rReq.Ignore(); // the 'old' request is not relevant any more
                        pDlg->StartExecuteAsync([this, pDlg, pStyleSheet, xRequest=std::move(xRequest)](sal_Int32 nResult){
                            if ( nResult == RET_OK )
                            {
                                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                                if ( pOutSet )
                                    m_pDocument->ModifyStyleSheet( *pStyleSheet, *pOutSet );

                                SetDocumentModified();
                                xRequest->Done();
                            }
                            pDlg->disposeOnce();
                        });
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
                rSet.Put( SfxStringItem( nWhich, m_pDocument->GetPageStyle( nCurTab ) ) );
                break;

            case SID_HFEDIT:
                {
                    OUString            aStr        = m_pDocument->GetPageStyle( nCurTab );
                    ScStyleSheetPool*   pStylePool  = m_pDocument->GetStyleSheetPool();
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
                if ( m_pDocument->GetHardRecalcState() != ScDocument::HardRecalcState::OFF )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich, m_pDocument->GetAutoCalc() ) );
                break;

            case FID_CHG_RECORD:
                if ( IsDocShared() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( nWhich,
                        m_pDocument->GetChangeTrack() != nullptr ) );
                break;

            case SID_CHG_PROTECT:
                {
                    ScChangeTrack* pChangeTrack = m_pDocument->GetChangeTrack();
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
                rSet.Put( SfxInt16Item( nWhich, m_pDocument->GetTableCount() ) );
                break;

            case SID_ATTR_YEAR2000 :
                rSet.Put( SfxUInt16Item( nWhich,
                    m_pDocument->GetDocOptions().GetYear2000() ) );
            break;

            case SID_SHARE_DOC:
                {
                    if ( IsReadOnly() || GetObjectShell()->isExportLocked() )
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
                                                                          u"modules/scalc/ui/");
                        rSet.Put( SfxBoolItem( SID_NOTEBOOKBAR, bVisible ) );
                    }
                }
                break;

            case SID_LANGUAGE_STATUS:
                {
                    OUString sLanguage;
                    sal_uInt16 nLangWhich = 0;
                    LanguageType eLatin = LANGUAGE_DONTKNOW, eCjk = LANGUAGE_DONTKNOW,
                        eCtl = LANGUAGE_DONTKNOW;

                    if (comphelper::LibreOfficeKit::isActive())
                    {
                        GetDocument().GetLanguage( eLatin, eCjk, eCtl );
                        sLanguage = SvtLanguageTable::GetLanguageString(eLatin);

                        if (eLatin == LANGUAGE_NONE)
                            sLanguage += ";-";
                        else
                            sLanguage += ";" + LanguageTag(eLatin).getBcp47(false);
                    }
                    else if (ScTabViewShell* pViewShell = GetBestViewShell())
                    {
                        ScMarkData aMark = pViewShell->GetViewData().GetMarkData();
                        SCCOL  nCol = pViewShell->GetViewData().GetCurX();
                        SCROW  nRow = pViewShell->GetViewData().GetCurY();
                        SCTAB  nTab = pViewShell->GetViewData().GetTabNo();

                        aMark.SetMarkArea(ScRange(nCol, nRow, nTab));
                        const ScPatternAttr* pSelAttrs = GetDocument().GetSelectionPattern(aMark);
                        if (pSelAttrs)
                        {
                            const SfxItemSet& rItemSet = pSelAttrs->GetItemSet();
                            nLangWhich = rItemSet.GetPool()->GetWhichIDFromSlotID(SID_ATTR_CHAR_LANGUAGE);
                            if (SfxItemState::SET == rItemSet.GetItemState(nLangWhich))
                                eLatin = static_cast<const SvxLanguageItem&>(rItemSet.Get(nLangWhich)).GetLanguage();

                            nLangWhich = rItemSet.GetPool()->GetWhichIDFromSlotID(SID_ATTR_CHAR_CJK_LANGUAGE);
                            if (SfxItemState::SET == rItemSet.GetItemState(nLangWhich))
                                eCjk = static_cast<const SvxLanguageItem&>(rItemSet.Get(nLangWhich)).GetLanguage();

                            nLangWhich = rItemSet.GetPool()->GetWhichIDFromSlotID(SID_ATTR_CHAR_CTL_LANGUAGE);
                            if (SfxItemState::SET == rItemSet.GetItemState(nLangWhich))
                                eCtl = static_cast<const SvxLanguageItem&>(rItemSet.Get(nLangWhich)).GetLanguage();

                            if (eLatin != LANGUAGE_DONTKNOW)
                                sLanguage = SvtLanguageTable::GetLanguageString(eLatin);
                            else if (eCjk != LANGUAGE_DONTKNOW)
                                sLanguage = SvtLanguageTable::GetLanguageString(eCjk);
                            else if (eCtl != LANGUAGE_DONTKNOW)
                                sLanguage = SvtLanguageTable::GetLanguageString(eCtl);

                            if (sLanguage.isEmpty())
                            {
                                GetDocument().GetLanguage(eLatin, eCjk, eCtl);
                                sLanguage = SvtLanguageTable::GetLanguageString(eLatin);
                            }
                        }
                    }
                    rSet.Put(SfxStringItem(nWhich, sLanguage));
                }
                break;

            default:
                {
                }
                break;
        }
    }
}

void ScDocShell::Draw( OutputDevice* pDev, const JobSetup & /* rSetup */, sal_uInt16 nAspect, bool /*bOutputToWindow*/ )
{

    SCTAB nVisTab = m_pDocument->GetVisibleTab();
    if (!m_pDocument->HasTable(nVisTab))
        return;

    vcl::text::ComplexTextLayoutFlags nOldLayoutMode = pDev->GetLayoutMode();
    pDev->SetLayoutMode( vcl::text::ComplexTextLayoutFlags::Default );     // even if it's the same, to get the metafile action

    if ( nAspect == ASPECT_THUMBNAIL )
    {
        tools::Rectangle aBoundRect = GetVisArea( ASPECT_THUMBNAIL );
        ScViewData aTmpData( *this, nullptr );
        aTmpData.SetTabNo(nVisTab);
        SnapVisArea( aBoundRect );
        aTmpData.SetScreen( aBoundRect );
        ScPrintFunc::DrawToDev( *m_pDocument, pDev, 1.0, aBoundRect, &aTmpData, true );
    }
    else
    {
        tools::Rectangle aOldArea = SfxObjectShell::GetVisArea();
        tools::Rectangle aNewArea = aOldArea;
        ScViewData aTmpData( *this, nullptr );
        aTmpData.SetTabNo(nVisTab);
        SnapVisArea( aNewArea );
        if ( aNewArea != aOldArea && (m_pDocument->GetPosLeft() > 0 || m_pDocument->GetPosTop() > 0) )
            SfxObjectShell::SetVisArea( aNewArea );
        aTmpData.SetScreen( aNewArea );
        ScPrintFunc::DrawToDev( *m_pDocument, pDev, 1.0, aNewArea, &aTmpData, true );
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
        SCTAB nVisTab = m_pDocument->GetVisibleTab();
        if (!m_pDocument->HasTable(nVisTab))
        {
            nVisTab = 0;
            const_cast<ScDocShell*>(this)->m_pDocument->SetVisibleTab(nVisTab);
        }
        Size aSize = m_pDocument->GetPageSize(nVisTab);
        const tools::Long SC_PREVIEW_SIZE_X = 10000;
        const tools::Long SC_PREVIEW_SIZE_Y = 12400;
        tools::Rectangle aArea( 0,0, SC_PREVIEW_SIZE_X, SC_PREVIEW_SIZE_Y);
        if (aSize.Width() > aSize.Height())
        {
            aArea.SetRight( SC_PREVIEW_SIZE_Y );
            aArea.SetBottom( SC_PREVIEW_SIZE_X );
        }

        bool bNegativePage = m_pDocument->IsNegativePage( m_pDocument->GetVisibleTab() );
        if ( bNegativePage )
            ScDrawLayer::MirrorRectRTL( aArea );
        SnapVisArea( aArea );
        return aArea;
    }
    else if( nAspect == ASPECT_CONTENT && eShellMode != SfxObjectCreateMode::EMBEDDED )
    {
        //  fetch visarea like after loading

        SCTAB nVisTab = m_pDocument->GetVisibleTab();
        if (!m_pDocument->HasTable(nVisTab))
        {
            nVisTab = 0;
            const_cast<ScDocShell*>(this)->m_pDocument->SetVisibleTab(nVisTab);
        }
        SCCOL nStartCol;
        SCROW nStartRow;
        m_pDocument->GetDataStart( nVisTab, nStartCol, nStartRow );
        SCCOL nEndCol;
        SCROW nEndRow;
        m_pDocument->GetPrintArea( nVisTab, nEndCol, nEndRow );
        if (nStartCol>nEndCol)
            nStartCol = nEndCol;
        if (nStartRow>nEndRow)
            nStartRow = nEndRow;
        tools::Rectangle aNewArea = m_pDocument
                                ->GetMMRect( nStartCol,nStartRow, nEndCol,nEndRow, nVisTab );
        return aNewArea;
    }
    else
        return SfxObjectShell::GetVisArea( nAspect );
}

namespace {

[[nodiscard]]
tools::Long SnapHorizontal( const ScDocument& rDoc, SCTAB nTab, tools::Long nVal, SCCOL& rStartCol )
{
    SCCOL nCol = 0;
    tools::Long nTwips = o3tl::convert(nVal, o3tl::Length::mm100, o3tl::Length::twip);
    tools::Long nSnap = 0;
    while ( nCol<rDoc.MaxCol() )
    {
        tools::Long nAdd = rDoc.GetColWidth(nCol, nTab);
        if ( nSnap + nAdd/2 < nTwips || nCol < rStartCol )
        {
            nSnap += nAdd;
            ++nCol;
        }
        else
            break;
    }
    nVal = o3tl::convert(nSnap, o3tl::Length::twip, o3tl::Length::mm100);
    rStartCol = nCol;
    return nVal;
}

[[nodiscard]]
tools::Long SnapVertical( const ScDocument& rDoc, SCTAB nTab, tools::Long nVal, SCROW& rStartRow )
{
    SCROW nRow = 0;
    tools::Long nTwips = o3tl::convert(nVal, o3tl::Length::mm100, o3tl::Length::twip);
    tools::Long nSnap = 0;

    bool bFound = false;
    for (SCROW i = nRow; i <= rDoc.MaxRow(); ++i)
    {
        SCROW nLastRow;
        if (rDoc.RowHidden(i, nTab, nullptr, &nLastRow))
        {
            i = nLastRow;
            continue;
        }

        nRow = i;
        tools::Long nAdd = rDoc.GetRowHeight(i, nTab);
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
        nRow = rDoc.MaxRow();  // all hidden down to the bottom

    nVal = o3tl::convert(nSnap, o3tl::Length::twip, o3tl::Length::mm100);
    rStartRow = nRow;
    return nVal;
}

}

void ScDocShell::SnapVisArea( tools::Rectangle& rRect ) const
{
    SCTAB nTab = m_pDocument->GetVisibleTab();
    tools::Long nOrigTop = rRect.Top();
    tools::Long nOrigLeft = rRect.Left();
    bool bNegativePage = m_pDocument->IsNegativePage( nTab );
    if ( bNegativePage )
        ScDrawLayer::MirrorRectRTL( rRect );        // calculate with positive (LTR) values

    SCCOL nCol = m_pDocument->GetPosLeft();
    tools::Long nSetLeft = SnapHorizontal( *m_pDocument, nTab, rRect.Left(), nCol );
    rRect.SetLeft( nSetLeft );
    ++nCol;                                         // at least one column
    tools::Long nCorrectionLeft = (nOrigLeft == 0 && nCol > 0) ? nSetLeft : 0; // initial correction
    rRect.SetRight( SnapHorizontal( *m_pDocument, nTab, rRect.Right() + nCorrectionLeft, nCol ));

    SCROW nRow = m_pDocument->GetPosTop();
    tools::Long nSetTop = SnapVertical( *m_pDocument, nTab, rRect.Top(), nRow );
    rRect.SetTop( nSetTop );
    ++nRow;                                         // at least one row
    tools::Long nCorrectionTop = (nOrigTop == 0 && nRow > 0) ? nSetTop : 0; // initial correction
    rRect.SetBottom( SnapVertical( *m_pDocument, nTab, rRect.Bottom() + nCorrectionTop, nRow ));

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
        ScStyleSheetPool*  pStylePool  = m_pDocument->GetStyleSheetPool();
        SfxStyleSheetBase* pStyleSheet = pStylePool->
                                            Find( m_pDocument->GetPageStyle( nCurTab ),
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
        ScImportExport aObj( *m_pDocument, rItem );
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

    ScImportExport aObj( *m_pDocument, rItem );
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
            if ( ScByteSequenceToString::GetString( m_aDdeTextFmt, rValue ) )
            {
                m_aDdeTextFmt = m_aDdeTextFmt.toAsciiUpperCase();
                return true;
            }
            return false;
        }
        ScImportExport aObj( *m_pDocument, rItem );
        if( m_aDdeTextFmt[0] == 'F' )
            aObj.SetFormulas( true );
        if( m_aDdeTextFmt == "SYLK" ||
            m_aDdeTextFmt == "FSYLK" )
        {
            OUString aData;
            if ( ScByteSequenceToString::GetString( aData, rValue ) )
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
    /*ScImportExport aObj( aDocument, rItem );
    return aObj.IsRef() && ScImportExport::ImportData( rMimeType, rValue );*/
    OSL_ENSURE( false, "Implementation is missing" );
    return false;
}
#endif

::sfx2::SvLinkSource* ScDocShell::DdeCreateLinkSource( const OUString& rItem )
{
    if (officecfg::Office::Common::Security::Scripting::DisableActiveContent::get())
        return nullptr;

    //  only check for valid item string - range is parsed again in ScServerObject ctor

    //  named range?
    OUString aPos = rItem;
    ScRangeName* pRange = m_pDocument->GetRangeName();
    if( pRange )
    {
        const ScRangeData* pData = pRange->findByUpperName(ScGlobal::getCharClass().uppercase(aPos));
        if (pData)
        {
            if( pData->HasType( ScRangeData::Type::RefArea    )
                || pData->HasType( ScRangeData::Type::AbsArea )
                || pData->HasType( ScRangeData::Type::AbsPos  ) )
                aPos = pData->GetSymbol();           // continue with the name's contents
        }
    }

    // Address in DDE function must be always parsed as CONV_OOO so that it
    // would always work regardless of current address conversion.  We do this
    // because the address item in a DDE entry is *not* normalized when saved
    // into ODF.
    ScRange aRange;
    bool bValid = ( (aRange.Parse(aPos, *m_pDocument, formula::FormulaGrammar::CONV_OOO ) & ScRefFlags::VALID) ||
                    (aRange.aStart.Parse(aPos, *m_pDocument, formula::FormulaGrammar::CONV_OOO) & ScRefFlags::VALID) );

    ScServerObject* pObj = nullptr;            // NULL = error
    if ( bValid )
        pObj = new ScServerObject( this, rItem );

    //  GetLinkManager()->InsertServer() is in the ScServerObject ctor

    return pObj;
}

void ScDocShell::LOKCommentNotify(LOKCommentNotificationType nType, const ScDocument& rDocument, const ScAddress& rPos, const ScPostIt* pNote)
{
    if ( !rDocument.IsDocVisible() || // don't want callbacks until document load
         !comphelper::LibreOfficeKit::isActive() ||
         comphelper::LibreOfficeKit::isTiledAnnotations() )
        return;

    tools::JsonWriter aAnnotation;
    {
        auto commentNode = aAnnotation.startNode("comment");
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
                aAnnotation.put("cellRange", ScPostIt::NoteRangeToJsonString(rDocument, rPos));
        }
    }

    OString aPayload = aAnnotation.finishAndGetAsOString();

    ScViewData* pViewData = GetViewData();
    SfxViewShell* pThisViewShell = ( pViewData ? pViewData->GetViewShell() : nullptr );
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pThisViewShell == nullptr || pViewShell->GetDocId() == pThisViewShell->GetDocId())
            pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload);
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
        return &pViewSh->GetViewFrame().GetBindings();
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
        if ( auto pDocSh = dynamic_cast<ScDocShell*>(pShell) )
        {
            if ( nShellCnt == nDocNo )
                pFound = pDocSh;
            else
                ++nShellCnt;
        }
        pShell = SfxObjectShell::GetNext( *pShell );
    }

    return pFound;
}

IMPL_LINK( ScDocShell, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg, void )
{
    assert( _pFileDlg && "ScDocShell::DialogClosedHdl(): no file dialog");
    OSL_ENSURE( m_pImpl->pDocInserter, "ScDocShell::DialogClosedHdl(): no document inserter" );

    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        sal_uInt16 nSlot = m_pImpl->pRequest->GetSlot();
        std::unique_ptr<SfxMedium> pMed = m_pImpl->pDocInserter->CreateMedium();
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
            if (pMed->GetItemSet().GetItemState(SID_VERSION, true, &pItem) == SfxItemState::SET)
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
        m_pDocument->EndChangeTracking();
        m_pDocument->StartChangeTracking();

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
        m_pDocument->EndChangeTracking();
    }

    ScChangeViewSettings aChangeViewSet;
    aChangeViewSet.SetShowChanges( false );
    m_pDocument->SetChangeViewSettings( aChangeViewSet );
}

uno::Reference< frame::XModel > ScDocShell::LoadSharedDocument()
{
    uno::Reference< frame::XModel > xModel;
    try
    {
        SC_MOD()->SetInSharedDocLoading( true );
        uno::Reference< frame::XDesktop2 > xLoader = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        uno::Sequence aArgs{ comphelper::makePropertyValue(u"Hidden"_ustr, true) };

        if ( GetMedium() )
        {
            const SfxStringItem* pPasswordItem = GetMedium()->GetItemSet().GetItem(SID_PASSWORD, false);
            if ( pPasswordItem && !pPasswordItem->GetValue().isEmpty() )
            {
                aArgs.realloc( 2 );
                auto pArgs = aArgs.getArray();
                pArgs[1].Name = "Password";
                pArgs[1].Value <<= pPasswordItem->GetValue();
            }
            const SfxUnoAnyItem* pEncryptionItem = GetMedium()->GetItemSet().GetItem(SID_ENCRYPTIONDATA, false);
            if (pEncryptionItem)
            {
                aArgs.realloc(aArgs.getLength() + 1);
                auto pArgs = aArgs.getArray();
                pArgs[aArgs.getLength() - 1].Name = "EncryptionData";
                pArgs[aArgs.getLength() - 1].Value = pEncryptionItem->GetValue();
            }
        }

        xModel.set(
            xLoader->loadComponentFromURL( GetSharedFileURL(), u"_blank"_ustr, 0, aArgs ),
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
