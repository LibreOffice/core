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
#include <config_fuzzers.h>
#include <config_wasm_strip.h>

#include "swdlgfact.hxx"
#include <svl/style.hxx>
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
#include <contentcontroldlg.hxx>
#include <contentcontrollistitemdlg.hxx>
#include <pagenumberdlg.hxx>
#include <convert.hxx>
#include <cption.hxx>
#include <dbinsdlg.hxx>
#include <docfnote.hxx>
#include <docstdlg.hxx>
#include <DateFormFieldDialog.hxx>
#include <DropDownFieldDialog.hxx>
#include <DropDownFormFieldDialog.hxx>
#include <envlop.hxx>
#include <label.hxx>
#include <drpcps.hxx>
#include <swuipardlg.hxx>
#include <pattern.hxx>
#include <pardlg.hxx>
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
#include <formatlinebreak.hxx>
#include <translatelangselect.hxx>

using namespace ::com::sun::star;
using namespace css::frame;
using namespace css::uno;

short AbstractSwWordCountFloatDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwInsertAbstractDlg_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractSwInsertAbstractDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short SwAbstractSfxController_Impl::Execute()
{
    return m_xDlg->run();
}

bool SwAbstractSfxController_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractNumFormatDlg_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractNumFormatDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxSingleTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractSwAsciiFilterDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSplitTableDialog_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractSplitTableDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractSwTableWidthDlg_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractSwTableWidthDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractSwTableHeightDlg_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractSwTableHeightDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractSwMergeTableDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwPageNumberDlg_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractSwPageNumberDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
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
    assert(false);
    return -1;
}

bool AbstractSwSortDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractMultiTOXMarkDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractTabController_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractNumBulletDialog_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwConvertTableDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwInsertDBColAutoPilot_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractSwInsertDBColAutoPilot_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractDropDownFieldDialog_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractDropDownFormFieldDialog_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractDropDownFormFieldDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractDateFormFieldDialog_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractDateFormFieldDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
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
    assert(false);
    return -1;
}

bool AbstractSwAutoFormatDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractSwFieldDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwRenameXNamedDlg_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractSwRenameXNamedDlg_Impl::StartExecuteAsync(VclAbstractDialog::AsyncContext& rCtx)
{
    return weld::DialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

bool AbstractSwContentControlListItemDlg_Impl::StartExecuteAsync(VclAbstractDialog::AsyncContext& rCtx)
{
    return weld::DialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractSwContentControlListItemDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractSwModalRedlineAcceptDlg_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractSwModalRedlineAcceptDlg_Impl::StartExecuteAsync(VclAbstractDialog::AsyncContext& rCtx)
{
    return weld::DialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractGlossaryDlg_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractGlossaryDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractFieldInputDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractInsFootNoteDlg_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractInsFootNoteDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractJavaEditDialog_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractMailMergeDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractMailMergeCreateFromDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractMailMergeFieldConnectionsDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractMultiTOXTabDialog_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractMultiTOXTabDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractEditRegionDlg_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractEditRegionDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::DialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
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

void AbstractTabController_Impl::SetCurPageId( const OUString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractTabController_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer AbstractTabController_Impl::GetInputRanges(const SfxItemPool& pItem )
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

void AbstractNumBulletDialog_Impl::SetCurPageId( const OUString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractNumBulletDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

const SfxItemSet* AbstractNumBulletDialog_Impl::GetInputItemSet() const
{
    return m_xDlg->GetInputItemSet();
}

WhichRangesContainer AbstractNumBulletDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void AbstractNumBulletDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

bool AbstractNumBulletDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

//From class Window.
void AbstractNumBulletDialog_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

IMPL_LINK_NOARG(AbstractApplyTabController_Impl, ApplyHdl, weld::Button&, void)
{
    if (m_xDlg->Apply())
    {
        m_aHandler.Call(nullptr);
        m_xDlg->Applied();
    }
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

const SfxItemSet* SwAbstractSfxController_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

const SfxItemSet* AbstractNumFormatDlg_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

void SwAbstractSfxController_Impl::SetText(const OUString& rStr)
{
    m_xDlg->set_title(rStr);
}

void AbstractNumFormatDlg_Impl::SetText(const OUString& rStr)
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
    SwBreakDlg* pDlg = dynamic_cast<SwBreakDlg*>(m_xDlg.get());
    if (pDlg)
        return pDlg->GetTemplateName();

    return u""_ustr;
}

sal_uInt16 AbstractSwBreakDlg_Impl:: GetKind()
{
    SwBreakDlg* pDlg = dynamic_cast<SwBreakDlg*>(m_xDlg.get());
    if (pDlg)
        return pDlg->GetKind();

    return 0;
}

::std::optional<sal_uInt16> AbstractSwBreakDlg_Impl:: GetPageNumber()
{
    SwBreakDlg* pDlg = dynamic_cast<SwBreakDlg*>(m_xDlg.get());
    if (pDlg)
        return pDlg->GetPageNumber();

    return 0;
}

std::optional<SwLineBreakClear> AbstractSwBreakDlg_Impl::GetClear()
{
    SwBreakDlg* pDlg = dynamic_cast<SwBreakDlg*>(m_xDlg.get());
    if (pDlg)
        return pDlg->GetClear();

    return SwLineBreakClear::NONE;
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
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
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

void AbstractSwLabDlg_Impl::SetCurPageId( const OUString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSwLabDlg_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer AbstractSwLabDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
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

std::unique_ptr<SwTableAutoFormat> AbstractSwConvertTableDlg_Impl::FillAutoFormatOfIndex() const
{
    return m_xDlg->FillAutoFormatOfIndex();
}

std::unique_ptr<SwTableAutoFormat> AbstractSwAutoFormatDlg_Impl::FillAutoFormatOfIndex() const
{
    return m_xDlg->FillAutoFormatOfIndex();
}

void AbstractSwFieldDlg_Impl::SetCurPageId( const OUString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSwFieldDlg_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

WhichRangesContainer AbstractSwFieldDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void AbstractSwFieldDlg_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

void AbstractSwFieldDlg_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

bool AbstractSwFieldDlg_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    auto xDlg = m_xDlg;
    return SfxTabDialogController::runAsync(m_xDlg, [rCtx, xDlg=std::move(xDlg)](sal_Int32 nResult){
        xDlg->Close();
        if (rCtx.isSet())
            rCtx.maEndDialogFn(nResult);
    });
}

void AbstractSwFieldDlg_Impl::Initialize(SfxChildWinInfo *pInfo)
{
    m_xDlg->Initialize( pInfo );
}

void AbstractSwFieldDlg_Impl::ReInitDlg()
{
    m_xDlg->ReInitDlg();
}

void AbstractSwFieldDlg_Impl::ActivateDatabasePage()
{
    m_xDlg->ActivateDatabasePage();
}

void AbstractSwFieldDlg_Impl::ShowReferencePage()
{
    m_xDlg->ShowReferencePage();
}

std::shared_ptr<SfxDialogController> AbstractSwFieldDlg_Impl::GetController()
{
    return m_xDlg;
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

OUString AbstractGlossaryDlg_Impl::GetCurrGrpName() const
{
    return m_xDlg->GetCurrGrpName();
}

OUString AbstractGlossaryDlg_Impl::GetCurrShortName() const
{
    return m_xDlg->GetCurrShortName();
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

int AbstractSwPageNumberDlg_Impl::GetPageNumberPosition() const
{
    return m_xDlg->GetPageNumberPosition();
}

int AbstractSwPageNumberDlg_Impl::GetPageNumberAlignment() const
{
    return m_xDlg->GetPageNumberAlignment();
}

bool AbstractSwPageNumberDlg_Impl::GetMirrorOnEvenPages() const
{
    return m_xDlg->GetMirrorOnEvenPages();
}

bool AbstractSwPageNumberDlg_Impl::GetIncludePageTotal() const
{
    return m_xDlg->GetIncludePageTotal();
}

SvxNumType AbstractSwPageNumberDlg_Impl::GetPageNumberType() const
{
    return m_xDlg->GetPageNumberType();
}

void AbstractSwPageNumberDlg_Impl::SetPageNumberType(SvxNumType nSet)
{
    m_xDlg->SetPageNumberType(nSet);
}

bool AbstractInsFootNoteDlg_Impl::IsEndNote()
{
    return m_xDlg->IsEndNote();
}

OUString AbstractInsFootNoteDlg_Impl::GetStr()
{
    return m_xDlg->GetStr();
}

void AbstractInsFootNoteDlg_Impl::SetHelpId(const OUString& rHelpId)
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
    SwInsTableDlg* pDlg = dynamic_cast<SwInsTableDlg*>(m_xDlg.get());
    if (pDlg)
        pDlg->GetValues(rName, rRow, rCol, rInsTableFlags, rTableAutoFormatName, prTAFormat);
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
    return m_xDlg->GetMergeType();
}

const OUString& AbstractMailMergeDlg_Impl::GetSaveFilter() const
{
    return m_xDlg->GetSaveFilter();
}

css::uno::Sequence< css::uno::Any > AbstractMailMergeDlg_Impl::GetSelection() const
{
    return m_xDlg->GetSelection();
}

uno::Reference< sdbc::XResultSet> AbstractMailMergeDlg_Impl::GetResultSet() const
{
    return m_xDlg->GetResultSet();
}

bool AbstractMailMergeDlg_Impl::IsSaveSingleDoc() const
{
    return m_xDlg->IsSaveSingleDoc();
}

bool AbstractMailMergeDlg_Impl::IsGenerateFromDataBase() const
{
    return m_xDlg->IsGenerateFromDataBase();
}

bool AbstractMailMergeDlg_Impl::IsFileEncryptedFromDataBase() const
{
    return m_xDlg->IsFileEncryptedFromDataBase();
}

OUString AbstractMailMergeDlg_Impl::GetColumnName() const
{
    return m_xDlg->GetColumnName();
}

OUString AbstractMailMergeDlg_Impl::GetPasswordColumnName() const
{
    return m_xDlg->GetPasswordColumnName();
}

OUString AbstractMailMergeDlg_Impl::GetTargetURL() const
{
    return m_xDlg->GetTargetURL();
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
    return m_xDlg->GetCurrentTOXType();
}

SwTOXDescription& AbstractMultiTOXTabDialog_Impl::GetTOXDescription(CurTOXType eTOXTypes)
{
    return m_xDlg->GetTOXDescription(eTOXTypes);
}

const SfxItemSet* AbstractMultiTOXTabDialog_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
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

std::shared_ptr<SfxDialogController> AbstractIndexMarkFloatDlg_Impl::GetController()
{
    return m_xDlg;
}

void AbstractAuthMarkFloatDlg_Impl::ReInitDlg(SwWrtShell& rWrtShell)
{
    m_xDlg->ReInitDlg(rWrtShell);
}

std::shared_ptr<SfxDialogController> AbstractAuthMarkFloatDlg_Impl::GetController()
{
    return m_xDlg;
}

std::shared_ptr<SfxDialogController> AbstractSwWordCountFloatDlg_Impl::GetController()
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
}

bool AbstractMailMergeWizard_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    // SwMailMergeWizardExecutor wants to run the lifecycle of this dialog
    // so clear mxOwner here and leave it up to SwMailMergeWizardExecutor
    rCtx.mxOwner.clear();
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractMailMergeWizard_Impl::Execute()
{
    return m_xDlg->run();
}

OUString AbstractMailMergeWizard_Impl::GetReloadDocument() const
{
    return m_xDlg->GetReloadDocument();
}

void AbstractMailMergeWizard_Impl::ShowPage( sal_uInt16 nLevel )
{
    m_xDlg->skipUntil(nLevel);
}

sal_uInt16 AbstractMailMergeWizard_Impl::GetRestartPage() const
{
    return m_xDlg->GetRestartPage();
}

std::optional<SwLanguageListItem> AbstractSwTranslateLangSelectDlg_Impl::GetSelectedLanguage()
{
#if HAVE_FEATURE_CURL && !ENABLE_WASM_STRIP_EXTRA
    return SwTranslateLangSelectDlg::GetSelectedLanguage();
#else
    return {};
#endif
}

short AbstractChangeDbDialog_Impl::Execute()
{
    assert(false);
    return -1;
}

bool AbstractChangeDbDialog_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return weld::GenericDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
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
    return VclPtr<AbstractNumFormatDlg_Impl>::Create(std::make_shared<SwNumFormatDlg>(pParent, rSet));
}

VclPtr<AbstractSwAsciiFilterDlg> SwAbstractDialogFactory_Impl::CreateSwAsciiFilterDlg(weld::Window* pParent,
       SwDocShell& rDocSh, SvStream* pStream)
{
    return VclPtr<AbstractSwAsciiFilterDlg_Impl>::Create(std::make_unique<SwAsciiFilterDlg>(pParent, rDocSh, pStream));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwInsertBookmarkDlg(weld::Window *pParent,
        SwWrtShell &rSh, OUString const*const pSelected)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_shared<SwInsertBookmarkDlg>(pParent, rSh, pSelected));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwContentControlDlg(weld::Window* pParent,
                                                                                  SwWrtShell &rSh)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_shared<SwContentControlDlg>(pParent, rSh));
}

VclPtr<AbstractSwContentControlListItemDlg>
SwAbstractDialogFactory_Impl::CreateSwContentControlListItemDlg(weld::Window* pParent,
                                                                SwContentControlListItem& rItem)
{
    return VclPtr<AbstractSwContentControlListItemDlg_Impl>::Create(
        std::make_shared<SwContentControlListItemDlg>(pParent, rItem));
}

std::shared_ptr<AbstractSwBreakDlg> SwAbstractDialogFactory_Impl::CreateSwBreakDlg(weld::Window* pParent, SwWrtShell &rSh)
{
    return std::make_shared<AbstractSwBreakDlg_Impl>(std::make_unique<SwBreakDlg>(pParent, rSh));
}

std::shared_ptr<AbstractSwTranslateLangSelectDlg> SwAbstractDialogFactory_Impl::CreateSwTranslateLangSelectDlg(weld::Window* pParent, SwWrtShell &rSh)
{
#if HAVE_FEATURE_CURL && !ENABLE_WASM_STRIP_EXTRA
    return std::make_shared<AbstractSwTranslateLangSelectDlg_Impl>(std::make_unique<SwTranslateLangSelectDlg>(pParent, rSh));
#else
    (void) pParent;
    (void) rSh;
    return nullptr;
#endif
}

VclPtr<AbstractChangeDbDialog> SwAbstractDialogFactory_Impl::CreateSwChangeDBDlg(SwView& rVw)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    return VclPtr<AbstractChangeDbDialog_Impl>::Create(std::make_shared<SwChangeDBDlg>(rVw));
#else
    (void) rVw;
    return nullptr;
#endif
}

VclPtr<SfxAbstractTabDialog>  SwAbstractDialogFactory_Impl::CreateSwCharDlg(weld::Window* pParent, SwView& pVw,
    const SfxItemSet& rCoreSet, SwCharDlgMode nDialogMode, const OUString* pFormatStr)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_shared<SwCharDlg>(pParent, pVw, rCoreSet, nDialogMode, pFormatStr));
}

VclPtr<AbstractSwConvertTableDlg> SwAbstractDialogFactory_Impl::CreateSwConvertTableDlg(SwView& rView, bool bToTable)
{
    return VclPtr<AbstractSwConvertTableDlg_Impl>::Create(std::make_unique<SwConvertTableDlg>(rView, bToTable));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwCaptionDialog(weld::Window *pParent, SwView &rV)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_shared<SwCaptionDialog>(pParent, rV));
}

VclPtr<AbstractSwInsertDBColAutoPilot> SwAbstractDialogFactory_Impl::CreateSwInsertDBColAutoPilot( SwView& rView,
        uno::Reference< sdbc::XDataSource> rxSource,
        uno::Reference<sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
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
    return VclPtr<AbstractTabController_Impl>::Create(std::make_shared<SwFootNoteOptionDlg>(pParent, rSh));
}

VclPtr<AbstractDropDownFieldDialog> SwAbstractDialogFactory_Impl::CreateDropDownFieldDialog(weld::Widget *pParent,
    SwWrtShell &rSh, SwField* pField, bool bPrevButton, bool bNextButton)
{
    return VclPtr<AbstractDropDownFieldDialog_Impl>::Create(std::make_unique<sw::DropDownFieldDialog>(pParent, rSh, pField, bPrevButton, bNextButton));
}

VclPtr<AbstractDropDownFormFieldDialog> SwAbstractDialogFactory_Impl::CreateDropDownFormFieldDialog(weld::Widget *pParent, sw::mark::IFieldmark* pDropDownField)
{
    return VclPtr<AbstractDropDownFormFieldDialog_Impl>::Create(std::make_unique<sw::DropDownFormFieldDialog>(pParent, pDropDownField));
}

VclPtr<AbstractDateFormFieldDialog> SwAbstractDialogFactory_Impl::CreateDateFormFieldDialog(weld::Widget *pParent, sw::mark::IDateFieldmark* pDateField, SwDoc& rDoc)
{
    return VclPtr<AbstractDateFormFieldDialog_Impl>::Create(std::make_unique<sw::DateFormFieldDialog>(pParent, pDateField, rDoc));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                                 SwWrtShell* pWrtSh, Printer* pPrt,
                                                                 bool bInsert)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_shared<SwEnvDlg>(pParent, rSet, pWrtSh,pPrt, bInsert));
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
                                                                           const OUString& sDefPage)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_shared<SwParaDlg>(pParent, rVw, rCoreSet, DLG_STD, nullptr, bDraw, sDefPage));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwAutoMarkDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_shared<SwAuthMarkModalDlg>(pParent, rSh));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwColumnDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_shared<SwColumnDlg>(pParent, rSh));
}

VclPtr<AbstractSwTableHeightDlg> SwAbstractDialogFactory_Impl::CreateSwTableHeightDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwTableHeightDlg_Impl>::Create(std::make_unique<SwTableHeightDlg>(pParent, rSh));
}

VclPtr<AbstractSwSortDlg> SwAbstractDialogFactory_Impl::CreateSwSortingDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwSortDlg_Impl>::Create(std::make_unique<SwSortDlg>(pParent, rSh));
}

VclPtr<AbstractSplitTableDialog> SwAbstractDialogFactory_Impl::CreateSplitTableDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSplitTableDialog_Impl>::Create(std::make_shared<SwSplitTableDlg>(pParent, rSh));
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

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwWrapDlg(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pSh)
{
    return VclPtr<SwAbstractSfxController_Impl>::Create(std::make_unique<SwWrapDlg>(pParent, rSet, pSh, true/*bDrawMode*/));
}

VclPtr<AbstractSwTableWidthDlg> SwAbstractDialogFactory_Impl::CreateSwTableWidthDlg(weld::Window *pParent, SwWrtShell *pShell)
{
    return VclPtr<AbstractSwTableWidthDlg_Impl>::Create(std::make_unique<SwTableWidthDlg>(pParent, pShell));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwTableTabDlg(weld::Window* pParent,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_shared<SwTableTabDlg>(pParent, pItemSet, pSh));
}

VclPtr<AbstractSwFieldDlg> SwAbstractDialogFactory_Impl::CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, weld::Window *pParent)
{
    return VclPtr<AbstractSwFieldDlg_Impl>::Create(std::make_shared<SwFieldDlg>(pB, pCW, pParent));
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwFieldEditDlg(SwView& rVw)
{
    auto xDlg = std::make_shared<SwFieldEditDlg>(rVw);
    // without TabPage no dialog
    if (!xDlg->GetTabPage())
        return nullptr;
    return VclPtr<SwAbstractSfxController_Impl>::Create(std::move(xDlg));
}

VclPtr<AbstractSwRenameXNamedDlg> SwAbstractDialogFactory_Impl::CreateSwRenameXNamedDlg(weld::Widget* pParent,
    css::uno::Reference< css::container::XNamed > & xNamed,
    css::uno::Reference< css::container::XNameAccess > & xNameAccess)
{
    return VclPtr<AbstractSwRenameXNamedDlg_Impl>::Create(std::make_unique<SwRenameXNamedDlg>(pParent,xNamed, xNameAccess));
}

VclPtr<AbstractSwModalRedlineAcceptDlg> SwAbstractDialogFactory_Impl::CreateSwModalRedlineAcceptDlg(weld::Window *pParent)
{
    return VclPtr<AbstractSwModalRedlineAcceptDlg_Impl>::Create(std::make_unique<SwModalRedlineAcceptDlg>(pParent));
}

VclPtr<AbstractSwPageNumberDlg> SwAbstractDialogFactory_Impl::CreateSwPageNumberDlg(weld::Window *pParent)
{
    return VclPtr<AbstractSwPageNumberDlg_Impl>::Create(std::make_shared<SwPageNumberDlg>(pParent));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateTableMergeDialog(weld::Window* pParent, bool& rWithPrev)
{
    return VclPtr<AbstractSwMergeTableDlg_Impl>::Create(std::make_unique<SwMergeTableDlg>(pParent, rWithPrev));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateFrameTabDialog(const OUString &rDialogType,
                                                SfxViewFrame& rFrame, weld::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool        bNewFrame,
                                                const OUString&  sDefPage )
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_shared<SwFrameDlg>(rFrame, pParent, rCoreSet, bNewFrame, rDialogType, false/*bFormat*/, sDefPage, nullptr));
}

VclPtr<SfxAbstractApplyTabDialog> SwAbstractDialogFactory_Impl::CreateTemplateDialog(
                                                weld::Window *pParent,
                                                SfxStyleSheetBase&  rBase,
                                                SfxStyleFamily      nRegion,
                                                const OUString&     sPage,
                                                SwWrtShell*         pActShell,
                                                bool                bNew )
{
    return VclPtr<AbstractApplyTabController_Impl>::Create(std::make_shared<SwTemplateDlgController>(pParent, rBase, nRegion,
                                                                                                     sPage, pActShell, bNew));
}

VclPtr<AbstractGlossaryDlg> SwAbstractDialogFactory_Impl::CreateGlossaryDlg(SfxViewFrame& rViewFrame, SwGlossaryHdl* pGlosHdl,
                                                                            SwWrtShell *pWrtShell)
{
    return VclPtr<AbstractGlossaryDlg_Impl>::Create(std::make_unique<SwGlossaryDlg>(rViewFrame, pGlosHdl, pWrtShell));
}

VclPtr<AbstractFieldInputDlg> SwAbstractDialogFactory_Impl::CreateFieldInputDlg(weld::Widget *pParent,
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
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_shared<SwTitlePageDlg>(pParent));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateVclSwViewDialog(SwView& rView)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_shared<SwLineNumberingDlg>(rView));
}

std::shared_ptr<AbstractInsTableDlg> SwAbstractDialogFactory_Impl::CreateInsTableDlg(SwView& rView)
{
    return std::make_shared<AbstractInsTableDlg_Impl>(std::make_shared<SwInsTableDlg>(rView));
}

VclPtr<AbstractJavaEditDialog> SwAbstractDialogFactory_Impl::CreateJavaEditDialog(
    weld::Window* pParent, SwWrtShell* pWrtSh)
{
    return VclPtr<AbstractJavaEditDialog_Impl>::Create(std::make_unique<SwJavaEditDialog>(pParent, pWrtSh));
}

VclPtr<AbstractMailMergeDlg> SwAbstractDialogFactory_Impl::CreateMailMergeDlg(
                                                weld::Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTableName,
                                                sal_Int32 nCommandType,
                                                const uno::Reference< sdbc::XConnection>& xConnection )
{
    return VclPtr<AbstractMailMergeDlg_Impl>::Create(std::make_unique<SwMailMergeDlg>(pParent, rSh, rSourceName, rTableName, nCommandType, xConnection, nullptr));
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

VclPtr<AbstractNumBulletDialog> SwAbstractDialogFactory_Impl::CreateSvxNumBulletTabDialog(weld::Window* pParent,
                                                const SfxItemSet& rSwItemSet,
                                                SwWrtShell & rWrtSh)
{
    return VclPtr<AbstractNumBulletDialog_Impl>::Create(std::make_shared<SwSvxNumBulletTabDialog>(pParent, rSwItemSet, rWrtSh));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateOutlineTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell & rWrtSh )
{
    return VclPtr<AbstractTabController_Impl>::Create(std::make_shared<SwOutlineTabDialog>(pParent, pSwItemSet, rWrtSh));
}

VclPtr<AbstractMultiTOXTabDialog> SwAbstractDialogFactory_Impl::CreateMultiTOXTabDialog(weld::Widget* pParent, const SfxItemSet& rSet,
                                                                                        SwWrtShell &rShell, SwTOXBase* pCurTOX, bool bGlobal)
{
    return VclPtr<AbstractMultiTOXTabDialog_Impl>::Create(std::make_shared<SwMultiTOXTabDialog>(pParent, rSet, rShell, pCurTOX, USHRT_MAX, bGlobal));
}

VclPtr<AbstractEditRegionDlg> SwAbstractDialogFactory_Impl::CreateEditRegionDlg(weld::Window* pParent, SwWrtShell& rWrtSh)
{
    return VclPtr<AbstractEditRegionDlg_Impl>::Create(std::make_shared<SwEditRegionDlg>(pParent, rWrtSh));
}

VclPtr<AbstractInsertSectionTabDialog> SwAbstractDialogFactory_Impl::CreateInsertSectionTabDialog(weld::Window* pParent,
        const SfxItemSet& rSet, SwWrtShell& rSh)
{
    return VclPtr<AbstractInsertSectionTabDialog_Impl>::Create(std::make_shared<SwInsertSectionTabDialog>(pParent, rSet, rSh));
}

VclPtr<AbstractMarkFloatDlg> SwAbstractDialogFactory_Impl::CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo )
{
    return VclPtr<AbstractIndexMarkFloatDlg_Impl>::Create(std::make_shared<SwIndexMarkFloatDlg>(pBindings, pChild, pParent, pInfo, true/*bNew*/));
}

VclPtr<AbstractMarkFloatDlg> SwAbstractDialogFactory_Impl::CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo)
{
    return VclPtr<AbstractAuthMarkFloatDlg_Impl>::Create(std::make_shared<SwAuthMarkFloatDlg>(pBindings, pChild, pParent, pInfo, true/*bNew*/));
}

VclPtr<AbstractSwWordCountFloatDlg> SwAbstractDialogFactory_Impl::CreateSwWordCountDialog(
                                                                              SfxBindings* pBindings,
                                                                              SfxChildWindow* pChild,
                                                                              weld::Window *pParent,
                                                                              SfxChildWinInfo* pInfo)
{
    return VclPtr<AbstractSwWordCountFloatDlg_Impl>::Create(std::make_shared<SwWordCountFloatDlg>(pBindings, pChild, pParent, pInfo));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateIndexMarkModalDlg(weld::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark )
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(std::make_shared<SwIndexMarkModalDlg>(pParent, rSh, pCurTOXMark));
}

VclPtr<AbstractMailMergeWizard> SwAbstractDialogFactory_Impl::CreateMailMergeWizard(
                                    SwView& rView, std::shared_ptr<SwMailMergeConfigItem>& rConfigItem)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    return VclPtr<AbstractMailMergeWizard_Impl>::Create(std::make_shared<SwMailMergeWizard>(rView, rConfigItem));
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
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    SwMMResultSaveDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultPrintDialog(weld::Window* pParent)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    SwMMResultPrintDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultEmailDialog(weld::Window* pParent)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    SwMMResultEmailDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
