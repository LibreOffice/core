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

#include "swdlgfact.hxx"
#include <svl/style.hxx>
#include <svx/svxids.hrc>
#include <dbui.hrc>
#include <globals.hrc>

#include <wordcountdialog.hxx>
#include <abstract.hxx>
#include <addrdlg.hxx>
#include <ascfldlg.hxx>
#include <bookmark.hxx>
#include <break.hxx>
#include <changedb.hxx>
#include <chrdlg.hxx>
#include <colwd.hxx>
#include <convert.hxx>
#include <cption.hxx>
#include <dbinsdlg.hxx>
#include <docfnote.hxx>
#include <docstdlg.hxx>
#include <DropDownFieldDialog.hxx>
#include <DropDownFormFieldDialog.hxx>
#include <envlop.hxx>
#include <label.hxx>
#include <drpcps.hxx>
#include <swuipardlg.hxx>
#include <pattern.hxx>
#include <rowht.hxx>
#include <selglos.hxx>
#include <splittbl.hxx>
#include <srtdlg.hxx>
#include <tautofmt.hxx>
#include <tblnumfm.hxx>
#include <wrap.hxx>
#include <tabledlg.hxx>
#include <fldtdlg.hxx>
#include <fldedt.hxx>
#include <swrenamexnameddlg.hxx>
#include <swmodalredlineacceptdlg.hxx>
#include <frmdlg.hxx>
#include <tmpdlg.hxx>
#include <glossary.hxx>
#include <inpdlg.hxx>
#include <insfnote.hxx>
#include <instable.hxx>
#include <javaedit.hxx>
#include <linenum.hxx>
#include <titlepage.hxx>
#include <mailmrge.hxx>
#include <mergetbl.hxx>
#include <multmrk.hxx>
#include <num.hxx>
#include <outline.hxx>
#include <column.hxx>
#include <cnttab.hxx>
#include <swuicnttab.hxx>
#include <regionsw.hxx>
#include <optcomp.hxx>
#include <optload.hxx>
#include <optpage.hxx>
#include <swuiidxmrk.hxx>
#include <svx/dialogs.hrc>
#include <mailmergewizard.hxx>
#include <mailconfigpage.hxx>
#include <uiborder.hxx>
#include <mmresultdialogs.hxx>


using namespace ::com::sun::star;
using namespace css::frame;
using namespace css::uno;

short AbstractSwWordCountFloatDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwInsertAbstractDlg_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(SwAbstractSfxDialog_Impl);

short SwAbstractSfxController_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwAsciiFilterDlg_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(VclAbstractDialog_Impl);

short AbstractSplitTableDialog_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwBreakDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwTableWidthDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwTableHeightDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwMergeTableDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractGenericDialog_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractGenericDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractSwSortDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractMultiTOXMarkDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractTabController_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwConvertTableDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwInsertDBColAutoPilot_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractDropDownFieldDialog_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractDropDownFormFieldDialog_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwLabDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwSelGlossaryDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwAutoFormatDlg_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(AbstractSwFieldDlg_Impl);

short AbstractSwRenameXNamedDlg_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(AbstractSwModalRedlineAcceptDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractGlossaryDlg_Impl);

short AbstractFieldInputDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractInsFootNoteDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractInsTableDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractJavaEditDialog_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(AbstractMailMergeDlg_Impl);

short AbstractMailMergeCreateFromDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractMailMergeFieldConnectionsDlg_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(AbstractMultiTOXTabDialog_Impl);

short AbstractEditRegionDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractInsertSectionTabDialog_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractInsertSectionTabDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractIndexMarkFloatDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractAuthMarkFloatDlg_Impl::Execute()
{
    return m_xDlg->run();
}

void AbstractTabController_Impl::SetCurPageId( const OString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractTabController_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractTabController_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void AbstractTabController_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

bool AbstractTabController_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

//From class Window.
void AbstractTabController_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

IMPL_LINK_NOARG(AbstractApplyTabController_Impl, ApplyHdl, weld::Button&, void)
{
    if (m_xDlg->Apply())
        m_aHandler.Call(nullptr);
}

void AbstractApplyTabController_Impl::SetApplyHdl( const Link<LinkParamNone*,void>& rLink )
{
    m_aHandler = rLink;
    m_xDlg->SetApplyHandler(LINK(this, AbstractApplyTabController_Impl, ApplyHdl));
}

sal_uInt8 AbstractSwInsertAbstractDlg_Impl::GetLevel() const
{
    return m_xDlg->GetLevel();
}

sal_uInt8 AbstractSwInsertAbstractDlg_Impl::GetPara() const
{
    return m_xDlg->GetPara();
}

const SfxItemSet* SwAbstractSfxDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

void SwAbstractSfxDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

const SfxItemSet* SwAbstractSfxController_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

void SwAbstractSfxController_Impl::SetText(const OUString& rStr)
{
    m_xDlg->set_title(rStr);
}

void AbstractSwAsciiFilterDlg_Impl::FillOptions( SwAsciiOptions& rOptions )
{
    m_xDlg->FillOptions(rOptions);
}

SplitTable_HeadlineOption AbstractSplitTableDialog_Impl::GetSplitMode()
{
    return m_xDlg->GetSplitMode();
}

OUString AbstractSwBreakDlg_Impl::GetTemplateName()
{
    return m_xDlg->GetTemplateName();
}

sal_uInt16 AbstractSwBreakDlg_Impl:: GetKind()
{
    return m_xDlg->GetKind();
}

::boost::optional<sal_uInt16> AbstractSwBreakDlg_Impl:: GetPageNumber()
{
    return m_xDlg->GetPageNumber();
}

void AbstractSwConvertTableDlg_Impl::GetValues( sal_Unicode& rDelim,SwInsertTableOptions& rInsTableFlags,
                                                SwTableAutoFormat const*& prTAFormat)
{
    m_xDlg->GetValues(rDelim,rInsTableFlags, prTAFormat);
}

void AbstractSwInsertDBColAutoPilot_Impl::DataToDoc( const uno::Sequence< uno::Any >& rSelection,
        uno::Reference< sdbc::XDataSource> rxSource,
        uno::Reference< sdbc::XConnection> xConnection,
        uno::Reference< sdbc::XResultSet > xResultSet)
{
#if HAVE_FEATURE_DBCONNECTIVITY
    m_xDlg->DataToDoc(rSelection, rxSource, xConnection, xResultSet);
#else
    (void) rSelection;
    (void) rxSource;
    (void) xConnection;
    (void) xResultSet;
#endif
}

bool AbstractDropDownFieldDialog_Impl::PrevButtonPressed() const
{
    return m_xDlg->PrevButtonPressed();
}

bool AbstractDropDownFieldDialog_Impl::NextButtonPressed() const
{
    return m_xDlg->NextButtonPressed();
}

void AbstractSwLabDlg_Impl::SetCurPageId( const OString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSwLabDlg_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractSwLabDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void AbstractSwLabDlg_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

void AbstractSwLabDlg_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

const OUString& AbstractSwLabDlg_Impl::GetBusinessCardStr() const
{
    return m_xDlg->GetBusinessCardStr();
}

Printer * AbstractSwLabDlg_Impl::GetPrt()
{
    return m_xDlg->GetPrt();
}

void AbstractSwSelGlossaryDlg_Impl::InsertGlos(const OUString &rRegion, const OUString &rGlosName)
{
    m_xDlg->InsertGlos( rRegion, rGlosName );
}

sal_Int32 AbstractSwSelGlossaryDlg_Impl::GetSelectedIdx() const
{
    return m_xDlg->GetSelectedIdx();
}

void AbstractSwSelGlossaryDlg_Impl::SelectEntryPos(sal_Int32 nIdx)
{
    m_xDlg->SelectEntryPos( nIdx );
}

SwTableAutoFormat* AbstractSwAutoFormatDlg_Impl::FillAutoFormatOfIndex() const
{
    return m_xDlg->FillAutoFormatOfIndex();
}

void AbstractSwFieldDlg_Impl::SetCurPageId( const OString &rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSwFieldDlg_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractSwFieldDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void AbstractSwFieldDlg_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}

void AbstractSwFieldDlg_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

void AbstractSwFieldDlg_Impl::Start()
{
    pDlg->Start();
}

void AbstractSwFieldDlg_Impl::Initialize(SfxChildWinInfo *pInfo)
{
    pDlg->Initialize( pInfo );
}

void AbstractSwFieldDlg_Impl::ReInitDlg()
{
    pDlg->ReInitDlg();
}

void AbstractSwFieldDlg_Impl::ActivateDatabasePage()
{
    pDlg->ActivateDatabasePage();
}

vcl::Window* AbstractSwFieldDlg_Impl::GetWindow()
{
    return static_cast<vcl::Window*>(pDlg);
}

void AbstractSwFieldDlg_Impl::ShowReferencePage()
{
    pDlg->ShowReferencePage();
}

void AbstractSwRenameXNamedDlg_Impl::SetForbiddenChars( const OUString& rSet )
{
    m_xDlg->SetForbiddenChars( rSet );
}

void    AbstractSwRenameXNamedDlg_Impl::SetAlternativeAccess(
             css::uno::Reference< css::container::XNameAccess > & xSecond,
             css::uno::Reference< css::container::XNameAccess > & xThird )
{
    m_xDlg->SetAlternativeAccess( xSecond, xThird);
}

void    AbstractSwModalRedlineAcceptDlg_Impl::AcceptAll( bool bAccept )
{
    pDlg->AcceptAll( bAccept);
}

OUString AbstractGlossaryDlg_Impl::GetCurrGrpName() const
{
    return pDlg->GetCurrGrpName();
}

OUString AbstractGlossaryDlg_Impl::GetCurrShortName() const
{
    return pDlg->GetCurrShortName();
}

void AbstractFieldInputDlg_Impl::EndDialog(sal_Int32 n)
{
    m_xDlg->response(n);
}

bool AbstractFieldInputDlg_Impl::PrevButtonPressed() const
{
    return m_xDlg->PrevButtonPressed();
}

bool AbstractFieldInputDlg_Impl::NextButtonPressed() const
{
    return m_xDlg->NextButtonPressed();
}

OUString AbstractInsFootNoteDlg_Impl::GetFontName()
{
    return m_xDlg->GetFontName();
}

bool AbstractInsFootNoteDlg_Impl::IsEndNote()
{
    return m_xDlg->IsEndNote();
}

OUString AbstractInsFootNoteDlg_Impl::GetStr()
{
    return m_xDlg->GetStr();
}

void AbstractInsFootNoteDlg_Impl::SetHelpId(const OString& rHelpId)
{
    m_xDlg->set_help_id(rHelpId);
}

void AbstractInsFootNoteDlg_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

void AbstractInsTableDlg_Impl::GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableFlags, OUString& rTableAutoFormatName,
                                std::unique_ptr<SwTableAutoFormat>& prTAFormat )
{
    m_xDlg->GetValues(rName, rRow, rCol, rInsTableFlags, rTableAutoFormatName, prTAFormat);
}

OUString AbstractJavaEditDialog_Impl::GetScriptText() const
{
    return m_xDlg->GetScriptText();
}

OUString AbstractJavaEditDialog_Impl::GetScriptType() const
{
    return m_xDlg->GetScriptType();
}

bool AbstractJavaEditDialog_Impl::IsUrl() const
{
    return m_xDlg->IsUrl();
}

bool AbstractJavaEditDialog_Impl::IsNew() const
{
    return m_xDlg->IsNew();
}

bool AbstractJavaEditDialog_Impl::IsUpdate() const
{
    return m_xDlg->IsUpdate();
}

DBManagerOptions AbstractMailMergeDlg_Impl::GetMergeType()
{
    return pDlg->GetMergeType();
}

const OUString& AbstractMailMergeDlg_Impl::GetSaveFilter() const
{
    return pDlg->GetSaveFilter();
}

const css::uno::Sequence< css::uno::Any > AbstractMailMergeDlg_Impl::GetSelection() const
{
    return pDlg->GetSelection();
}

uno::Reference< sdbc::XResultSet> AbstractMailMergeDlg_Impl::GetResultSet() const
{
    return pDlg->GetResultSet();
}

bool AbstractMailMergeDlg_Impl::IsSaveSingleDoc() const
{
    return pDlg->IsSaveSingleDoc();
}

bool AbstractMailMergeDlg_Impl::IsGenerateFromDataBase() const
{
    return pDlg->IsGenerateFromDataBase();
}

OUString AbstractMailMergeDlg_Impl::GetColumnName() const
{
    return pDlg->GetColumnName();
}

OUString AbstractMailMergeDlg_Impl::GetTargetURL() const
{
    return pDlg->GetTargetURL();
}

bool AbstractMailMergeCreateFromDlg_Impl::IsThisDocument() const
{
    return m_xDlg->IsThisDocument();
}

bool AbstractMailMergeFieldConnectionsDlg_Impl::IsUseExistingConnections() const
{
    return m_xDlg->IsUseExistingConnections();
}

CurTOXType AbstractMultiTOXTabDialog_Impl::GetCurrentTOXType() const
{
    return pDlg->GetCurrentTOXType();
}

SwTOXDescription& AbstractMultiTOXTabDialog_Impl::GetTOXDescription(CurTOXType eTOXTypes)
{
    return pDlg->GetTOXDescription(eTOXTypes);
}

const SfxItemSet* AbstractMultiTOXTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

void AbstractEditRegionDlg_Impl::SelectSection(const OUString& rSectionName)
{
    m_xDlg->SelectSection(rSectionName);
}

void
AbstractInsertSectionTabDialog_Impl::SetSectionData(SwSectionData const& rSect)
{
    m_xDlg->SetSectionData(rSect);
}

void AbstractIndexMarkFloatDlg_Impl::ReInitDlg(SwWrtShell& rWrtShell)
{
    m_xDlg->ReInitDlg( rWrtShell);
}

std::shared_ptr<SfxModelessDialogController> AbstractIndexMarkFloatDlg_Impl::GetController()
{
    return m_xDlg;
}

void AbstractAuthMarkFloatDlg_Impl::ReInitDlg(SwWrtShell& rWrtShell)
{
    m_xDlg->ReInitDlg(rWrtShell);
}

std::shared_ptr<SfxModelessDialogController> AbstractAuthMarkFloatDlg_Impl::GetController()
{
    return m_xDlg;
}

std::shared_ptr<SfxModelessDialogController> AbstractSwWordCountFloatDlg_Impl::GetController()
{
    return m_xDlg;
}

void AbstractSwWordCountFloatDlg_Impl::UpdateCounts()
{
    m_xDlg->UpdateCounts();
}

void AbstractSwWordCountFloatDlg_Impl::SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat)
{
    m_xDlg->SetCounts(rCurrCnt, rDocStat);
}

AbstractMailMergeWizard_Impl::~AbstractMailMergeWizard_Impl()
{
    disposeOnce();
}

void AbstractMailMergeWizard_Impl::dispose()
{
    pDlg.disposeAndClear();
    AbstractMailMergeWizard::dispose();
}

bool AbstractMailMergeWizard_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    // SwMailMergeWizardExecutor wants to run the lifecycle of this dialog
    // so clear mxOwner here and leave it up to SwMailMergeWizardExecutor
    rCtx.mxOwner.clear();
    return pDlg->StartExecuteAsync(rCtx);
}

short AbstractMailMergeWizard_Impl::Execute()
{
    return pDlg->Execute();
}

OUString AbstractMailMergeWizard_Impl::GetReloadDocument() const
{
    return pDlg->GetReloadDocument();
}

void AbstractMailMergeWizard_Impl::ShowPage( sal_uInt16 nLevel )
{
    pDlg->skipUntil(nLevel);
}

sal_uInt16 AbstractMailMergeWizard_Impl::GetRestartPage() const
{
    return pDlg->GetRestartPage();
}

VclPtr<AbstractSwInsertAbstractDlg> SwAbstractDialogFactory_Impl::CreateSwInsertAbstractDlg(weld::Window* pParent)
{
    return VclPtr<AbstractSwInsertAbstractDlg_Impl>::Create(std::make_unique<SwInsertAbstractDlg>(pParent));
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwAddressAbstractDlg(weld::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    return VclPtr<SwAbstractSfxController_Impl>::Create(std::make_unique<SwAddrDlg>(pParent, rSet));
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwDropCapsDialog(weld::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    return VclPtr<SwAbstractSfxController_Impl>::Create(std::make_unique<SwDropCapsDlg>(pParent, rSet));
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwBackgroundDialog(weld::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    return VclPtr<SwAbstractSfxController_Impl>::Create(std::make_unique<SwBackgroundDlg>(pParent, rSet));
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateNumFormatDialog(weld::Widget* pParent, const SfxItemSet& rSet)
{
    return VclPtr<SwAbstractSfxController_Impl>::Create(std::make_unique<SwNumFormatDlg>(pParent, rSet));
}

VclPtr<AbstractSwAsciiFilterDlg> SwAbstractDialogFactory_Impl::CreateSwAsciiFilterDlg(weld::Window* pParent,
       SwDocShell& rDocSh, SvStream* pStream)
{
    return VclPtr<AbstractSwAsciiFilterDlg_Impl>::Create(std::make_unique<SwAsciiFilterDlg>(pParent, rDocSh, pStream));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwInsertBookmarkDlg(weld::Window *pParent,
                                                                                  SwWrtShell &rSh, SfxRequest& rReq)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_unique<SwInsertBookmarkDlg>(pParent, rSh, rReq));
}

VclPtr<AbstractSwBreakDlg> SwAbstractDialogFactory_Impl::CreateSwBreakDlg(weld::Window* pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwBreakDlg_Impl>::Create(std::make_unique<SwBreakDlg>(pParent, rSh));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwChangeDBDlg(SwView& rVw)
{
#if HAVE_FEATURE_DBCONNECTIVITY
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_unique<SwChangeDBDlg>(rVw));
#else
    (void) rVw;
    return nullptr;
#endif
}

VclPtr<SfxAbstractTabDialog>  SwAbstractDialogFactory_Impl::CreateSwCharDlg(weld::Window* pParent, SwView& pVw,
    const SfxItemSet& rCoreSet, SwCharDlgMode nDialogMode, const OUString* pFormatStr)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_unique<SwCharDlg>(pParent, pVw, rCoreSet, nDialogMode, pFormatStr));
}

VclPtr<AbstractSwConvertTableDlg> SwAbstractDialogFactory_Impl::CreateSwConvertTableDlg(SwView& rView, bool bToTable)
{
    return VclPtr<AbstractSwConvertTableDlg_Impl>::Create(std::make_unique<SwConvertTableDlg>(rView, bToTable));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwCaptionDialog(weld::Window *pParent, SwView &rV)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_unique<SwCaptionDialog>(pParent, rV));
}

VclPtr<AbstractSwInsertDBColAutoPilot> SwAbstractDialogFactory_Impl::CreateSwInsertDBColAutoPilot( SwView& rView,
        uno::Reference< sdbc::XDataSource> rxSource,
        uno::Reference<sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData)
{
#if HAVE_FEATURE_DBCONNECTIVITY
    return VclPtr<AbstractSwInsertDBColAutoPilot_Impl>::Create(std::make_unique<SwInsertDBColAutoPilot>(rView, rxSource, xColSupp, rData));
#else
    (void) rView;
    (void) rxSource;
    (void) xColSupp;
    (void) rData;
    return nullptr;
#endif
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwFootNoteOptionDlg(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_unique<SwFootNoteOptionDlg>(pParent, rSh));
}

VclPtr<AbstractDropDownFieldDialog> SwAbstractDialogFactory_Impl::CreateDropDownFieldDialog(weld::Window *pParent,
    SwWrtShell &rSh, SwField* pField, bool bPrevButton, bool bNextButton)
{
    return VclPtr<AbstractDropDownFieldDialog_Impl>::Create(std::make_unique<sw::DropDownFieldDialog>(pParent, rSh, pField, bPrevButton, bNextButton));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateDropDownFormFieldDialog(weld::Window *pParent, sw::mark::IFieldmark* pDropDownField)
{
    return VclPtr<AbstractDropDownFormFieldDialog_Impl>::Create(std::make_unique<sw::DropDownFormFieldDialog>(pParent, pDropDownField));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                                 SwWrtShell* pWrtSh, Printer* pPrt,
                                                                 bool bInsert)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_unique<SwEnvDlg>(pParent, rSet, pWrtSh,pPrt, bInsert));
}

VclPtr<AbstractSwLabDlg> SwAbstractDialogFactory_Impl::CreateSwLabDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel)
{
    return VclPtr<AbstractSwLabDlg_Impl>::Create(std::make_unique<SwLabDlg>(pParent, rSet, pDBManager, bLabel));
}

SwLabDlgMethod SwAbstractDialogFactory_Impl::GetSwLabDlgStaticMethod ()
{
    return SwLabDlg::UpdateFieldInformation;
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwParaDlg(weld::Window *pParent, SwView& rVw,
                                                                           const SfxItemSet& rCoreSet,
                                                                           bool bDraw ,
                                                                           const OString& sDefPage)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_unique<SwParaDlg>(pParent, rVw, rCoreSet, DLG_STD, nullptr, bDraw, sDefPage));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwAutoMarkDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_unique<SwAuthMarkModalDlg>(pParent, rSh));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwColumnDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_unique<SwColumnDlg>(pParent, rSh));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwTableHeightDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwTableHeightDlg_Impl>::Create(std::make_unique<SwTableHeightDlg>(pParent, rSh));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwSortingDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwSortDlg_Impl>::Create(std::make_unique<SwSortDlg>(pParent, rSh));
}

VclPtr<AbstractSplitTableDialog> SwAbstractDialogFactory_Impl::CreateSplitTableDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSplitTableDialog_Impl>::Create(std::make_unique<SwSplitTableDlg>(pParent, rSh));
}

VclPtr<AbstractSwSelGlossaryDlg> SwAbstractDialogFactory_Impl::CreateSwSelGlossaryDlg(weld::Window *pParent, const OUString &rShortName)
{
    return VclPtr<AbstractSwSelGlossaryDlg_Impl>::Create(std::make_unique<SwSelGlossaryDlg>(pParent, rShortName));
}

VclPtr<AbstractSwAutoFormatDlg> SwAbstractDialogFactory_Impl::CreateSwAutoFormatDlg(weld::Window* pParent,
    SwWrtShell* pShell, bool bSetAutoFormat, const SwTableAutoFormat* pSelFormat)
{
    return VclPtr<AbstractSwAutoFormatDlg_Impl>::Create(
                std::unique_ptr<SwAutoFormatDlg, o3tl::default_delete<SwAutoFormatDlg>>(
                    new SwAutoFormatDlg(pParent, pShell, bSetAutoFormat, pSelFormat)));
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwBorderDlg(weld::Window* pParent, SfxItemSet& rSet, SwBorderModes nType )
{
    return VclPtr<SwAbstractSfxController_Impl>::Create(std::make_unique<SwBorderDlg>(pParent, rSet, nType));
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwWrapDlg(weld::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh)
{
    return VclPtr<SwAbstractSfxController_Impl>::Create(std::make_unique<SwWrapDlg>(pParent, rSet, pSh, true/*bDrawMode*/));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwTableWidthDlg(weld::Window *pParent, SwTableFUNC &rFnc)
{
    return VclPtr<AbstractSwTableWidthDlg_Impl>::Create(std::make_unique<SwTableWidthDlg>(pParent, rFnc));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwTableTabDlg(weld::Window* pParent,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_unique<SwTableTabDlg>(pParent, pItemSet, pSh));
}

VclPtr<AbstractSwFieldDlg> SwAbstractDialogFactory_Impl::CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, vcl::Window *pParent)
{
    VclPtr<SwFieldDlg> pDlg = VclPtr<SwFieldDlg>::Create(pB, pCW, pParent);
    return VclPtr<AbstractSwFieldDlg_Impl>::Create(pDlg);
}

VclPtr<SfxAbstractDialog>   SwAbstractDialogFactory_Impl::CreateSwFieldEditDlg ( SwView& rVw )
{
    VclPtr<SfxModalDialog> pDlg = VclPtr<SwFieldEditDlg>::Create( rVw );
    return VclPtr<SwAbstractSfxDialog_Impl>::Create( pDlg );
}

VclPtr<AbstractSwRenameXNamedDlg> SwAbstractDialogFactory_Impl::CreateSwRenameXNamedDlg(weld::Window* pParent,
    css::uno::Reference< css::container::XNamed > & xNamed,
    css::uno::Reference< css::container::XNameAccess > & xNameAccess)
{
    return VclPtr<AbstractSwRenameXNamedDlg_Impl>::Create(std::make_unique<SwRenameXNamedDlg>(pParent,xNamed, xNameAccess));
}

VclPtr<AbstractSwModalRedlineAcceptDlg> SwAbstractDialogFactory_Impl::CreateSwModalRedlineAcceptDlg(vcl::Window *pParent)
{
    VclPtr<SwModalRedlineAcceptDlg> pDlg = VclPtr<SwModalRedlineAcceptDlg>::Create( pParent );
    return VclPtr<AbstractSwModalRedlineAcceptDlg_Impl>::Create( pDlg );
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateTableMergeDialog(weld::Window* pParent, bool& rWithPrev)
{
    return VclPtr<AbstractSwMergeTableDlg_Impl>::Create(std::make_unique<SwMergeTableDlg>(pParent, rWithPrev));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateFrameTabDialog(const OUString &rDialogType,
                                                SfxViewFrame *pFrame, weld::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool        bNewFrame,
                                                const OString&  sDefPage )
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_unique<SwFrameDlg>(pFrame, pParent, rCoreSet, bNewFrame, rDialogType, false/*bFormat*/, sDefPage, nullptr));
}

VclPtr<SfxAbstractApplyTabDialog> SwAbstractDialogFactory_Impl::CreateTemplateDialog(
                                                weld::Window *pParent,
                                                SfxStyleSheetBase&  rBase,
                                                SfxStyleFamily      nRegion,
                                                const OString&      sPage,
                                                SwWrtShell*         pActShell,
                                                bool                bNew )
{
    return VclPtr<AbstractApplyTabController_Impl>::Create(std::make_unique<SwTemplateDlgController>(pParent, rBase, nRegion, sPage, pActShell, bNew));
}

VclPtr<AbstractGlossaryDlg> SwAbstractDialogFactory_Impl::CreateGlossaryDlg(SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell)
{
    VclPtr<SwGlossaryDlg> pDlg = VclPtr<SwGlossaryDlg>::Create(pViewFrame, pGlosHdl, pWrtShell);
    return VclPtr<AbstractGlossaryDlg_Impl>::Create( pDlg );
}

VclPtr<AbstractFieldInputDlg> SwAbstractDialogFactory_Impl::CreateFieldInputDlg(weld::Window *pParent,
    SwWrtShell &rSh, SwField* pField, bool bPrevButton, bool bNextButton)
{
    return VclPtr<AbstractFieldInputDlg_Impl>::Create(std::make_unique<SwFieldInputDlg>(pParent, rSh, pField, bPrevButton, bNextButton));
}

VclPtr<AbstractInsFootNoteDlg> SwAbstractDialogFactory_Impl::CreateInsFootNoteDlg(
    weld::Window * pParent, SwWrtShell &rSh, bool bEd )
{
    return VclPtr<AbstractInsFootNoteDlg_Impl>::Create(std::make_unique<SwInsFootNoteDlg>(pParent, rSh, bEd));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateTitlePageDlg(weld::Window *pParent)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_unique<SwTitlePageDlg>(pParent));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateVclSwViewDialog(SwView& rView)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_unique<SwLineNumberingDlg>(rView));
}

VclPtr<AbstractInsTableDlg> SwAbstractDialogFactory_Impl::CreateInsTableDlg(SwView& rView)
{
    return VclPtr<AbstractInsTableDlg_Impl>::Create(std::make_unique<SwInsTableDlg>(rView));
}

VclPtr<AbstractJavaEditDialog> SwAbstractDialogFactory_Impl::CreateJavaEditDialog(
    weld::Window* pParent, SwWrtShell* pWrtSh)
{
    return VclPtr<AbstractJavaEditDialog_Impl>::Create(std::make_unique<SwJavaEditDialog>(pParent, pWrtSh));
}

VclPtr<AbstractMailMergeDlg> SwAbstractDialogFactory_Impl::CreateMailMergeDlg(
                                                vcl::Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTableName,
                                                sal_Int32 nCommandType,
                                                const uno::Reference< sdbc::XConnection>& xConnection )
{
    VclPtr<SwMailMergeDlg> pDlg = VclPtr<SwMailMergeDlg>::Create( pParent, rSh, rSourceName, rTableName, nCommandType, xConnection, nullptr );
    return VclPtr<AbstractMailMergeDlg_Impl>::Create( pDlg );
}

VclPtr<AbstractMailMergeCreateFromDlg> SwAbstractDialogFactory_Impl::CreateMailMergeCreateFromDlg(weld::Window* pParent)
{
    return VclPtr<AbstractMailMergeCreateFromDlg_Impl>::Create(std::make_unique<SwMailMergeCreateFromDlg>(pParent));
}

VclPtr<AbstractMailMergeFieldConnectionsDlg> SwAbstractDialogFactory_Impl::CreateMailMergeFieldConnectionsDlg(weld::Window* pParent)
{
    return VclPtr<AbstractMailMergeFieldConnectionsDlg_Impl>::Create(std::make_unique<SwMailMergeFieldConnectionsDlg>(pParent));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateMultiTOXMarkDlg(weld::Window* pParent, SwTOXMgr &rTOXMgr)
{
    return VclPtr<AbstractMultiTOXMarkDlg_Impl>::Create(std::make_unique<SwMultiTOXMarkDlg>(pParent, rTOXMgr));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSvxNumBulletTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell & rWrtSh)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_unique<SwSvxNumBulletTabDialog>(pParent, pSwItemSet, rWrtSh));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateOutlineTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell & rWrtSh )
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_unique<SwOutlineTabDialog>(pParent, pSwItemSet, rWrtSh));
}

VclPtr<AbstractMultiTOXTabDialog> SwAbstractDialogFactory_Impl::CreateMultiTOXTabDialog(
                                                vcl::Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX,
                                                bool bGlobal)
{
    VclPtr<SwMultiTOXTabDialog> pDlg = VclPtr<SwMultiTOXTabDialog>::Create( pParent, rSet, rShell, pCurTOX, USHRT_MAX, bGlobal );
    return VclPtr<AbstractMultiTOXTabDialog_Impl>::Create( pDlg );
}

VclPtr<AbstractEditRegionDlg> SwAbstractDialogFactory_Impl::CreateEditRegionDlg(weld::Window* pParent, SwWrtShell& rWrtSh)
{
    return VclPtr<AbstractEditRegionDlg_Impl>::Create(std::make_unique<SwEditRegionDlg>(pParent, rWrtSh));
}

VclPtr<AbstractInsertSectionTabDialog> SwAbstractDialogFactory_Impl::CreateInsertSectionTabDialog(weld::Window* pParent,
        const SfxItemSet& rSet, SwWrtShell& rSh)
{
    return VclPtr<AbstractInsertSectionTabDialog_Impl>::Create(std::make_unique<SwInsertSectionTabDialog>(pParent, rSet, rSh));
}

VclPtr<AbstractMarkFloatDlg> SwAbstractDialogFactory_Impl::CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo )
{
    return VclPtr<AbstractIndexMarkFloatDlg_Impl>::Create(std::make_unique<SwIndexMarkFloatDlg>(pBindings, pChild, pParent, pInfo, true/*bNew*/));
}

VclPtr<AbstractMarkFloatDlg> SwAbstractDialogFactory_Impl::CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo)
{
    return VclPtr<AbstractAuthMarkFloatDlg_Impl>::Create(std::make_unique<SwAuthMarkFloatDlg>(pBindings, pChild, pParent, pInfo, true/*bNew*/));
}

VclPtr<AbstractSwWordCountFloatDlg> SwAbstractDialogFactory_Impl::CreateSwWordCountDialog(
                                                                              SfxBindings* pBindings,
                                                                              SfxChildWindow* pChild,
                                                                              weld::Window *pParent,
                                                                              SfxChildWinInfo* pInfo)
{
    return VclPtr<AbstractSwWordCountFloatDlg_Impl>::Create(std::make_unique<SwWordCountFloatDlg>(pBindings, pChild, pParent, pInfo));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateIndexMarkModalDlg(weld::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark )
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_unique<SwIndexMarkModalDlg>(pParent, rSh, pCurTOXMark));
}

VclPtr<AbstractMailMergeWizard> SwAbstractDialogFactory_Impl::CreateMailMergeWizard(
                                    SwView& rView, std::shared_ptr<SwMailMergeConfigItem>& rConfigItem)
{
#if HAVE_FEATURE_DBCONNECTIVITY
    return VclPtr<AbstractMailMergeWizard_Impl>::Create( VclPtr<SwMailMergeWizard>::Create(rView, rConfigItem));
#else
    (void) rView;
    (void) rConfigItem;
    return nullptr;
#endif
}

GlossaryGetCurrGroup    SwAbstractDialogFactory_Impl::GetGlossaryCurrGroupFunc()
{
    return SwGlossaryDlg::GetCurrGroup;
}

GlossarySetActGroup SwAbstractDialogFactory_Impl::SetGlossaryActGroupFunc()
{
    return SwGlossaryDlg::SetActGroup;
}

// Factories for TabPages
CreateTabPage SwAbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
{
    CreateTabPage pRet = nullptr;
    switch ( nId )
    {
        case RID_SW_TP_OPTCOMPATIBILITY_PAGE :
            pRet = SwCompatibilityOptPage::Create;
            break;
        case RID_SW_TP_OPTLOAD_PAGE :
            pRet = SwLoadOptPage::Create;
            break;
        case RID_SW_TP_OPTCAPTION_PAGE:
            return SwCaptionOptPage::Create;
        case RID_SW_TP_CONTENT_OPT:
        case RID_SW_TP_HTML_CONTENT_OPT:
            pRet = SwContentOptPage::Create;
            break;
        case RID_SW_TP_OPTSHDWCRSR:
        case RID_SW_TP_HTML_OPTSHDWCRSR:
            pRet = SwShdwCursorOptionsTabPage::Create;
            break;
        case RID_SW_TP_REDLINE_OPT :
            pRet = SwRedlineOptionsTabPage::Create;
            break;
        case RID_SW_TP_OPTTEST_PAGE :
#ifdef DBG_UTIL
            pRet = SwTestTabPage::Create;
#endif
            break;
        case TP_OPTPRINT_PAGE :
        case RID_SW_TP_HTML_OPTPRINT_PAGE:
        case RID_SW_TP_OPTPRINT_PAGE:
            pRet = SwAddPrinterTabPage::Create;
            break;
        case RID_SW_TP_STD_FONT:
        case RID_SW_TP_STD_FONT_CJK:
        case RID_SW_TP_STD_FONT_CTL:
            pRet = SwStdFontTabPage::Create;
            break;
        case RID_SW_TP_HTML_OPTTABLE_PAGE:
        case RID_SW_TP_OPTTABLE_PAGE:
            pRet = SwTableOptionsTabPage::Create;
            break;
        case RID_SW_TP_DOC_STAT :
            pRet = SwDocStatPage::Create;
            break;
        case RID_SW_TP_MAILCONFIG:
            pRet = SwMailConfigPage::Create;
        break;
        case RID_SW_TP_COMPARISON_OPT :
            pRet = SwCompareOptionsTabPage::Create;
        break;
    }

    return pRet;
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultSaveDialog(weld::Window* pParent)
{
#if HAVE_FEATURE_DBCONNECTIVITY
    SwMMResultSaveDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultPrintDialog(weld::Window* pParent)
{
#if HAVE_FEATURE_DBCONNECTIVITY
    SwMMResultPrintDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultEmailDialog(weld::Window* pParent)
{
#if HAVE_FEATURE_DBCONNECTIVITY
    SwMMResultEmailDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
