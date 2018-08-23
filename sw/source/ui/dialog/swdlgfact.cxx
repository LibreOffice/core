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

IMPL_ABSTDLG_BASE(AbstractSwWordCountFloatDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSwInsertAbstractDlg_Impl);
IMPL_ABSTDLG_BASE(SwAbstractSfxDialog_Impl);
short AbstractSwAsciiFilterDlg_Impl::Execute()
{
    return m_xDlg->run();
}
IMPL_ABSTDLG_BASE(VclAbstractDialog_Impl);
short AbstractSplitTableDialog_Impl::Execute()
{
    return m_xDlg->execute();
}
short AbstractSwBreakDlg_Impl::Execute()
{
    return m_xDlg->execute();
}
short AbstractSwTableWidthDlg_Impl::Execute()
{
    return m_xDlg->execute();
}
short AbstractSwTableHeightDlg_Impl::Execute()
{
    return m_xDlg->execute();
}
short AbstractSwMergeTableDlg_Impl::Execute()
{
    return m_xDlg->execute();
}
short AbstractGenericDialog_Impl::Execute()
{
    return m_xDlg->run();
}
short AbstractSwSortDlg_Impl::Execute()
{
    return m_xDlg->execute();
}
short AbstractMultiTOXMarkDlg_Impl::Execute()
{
    return m_xDlg->execute();
}
short AbstractTabController_Impl::Execute()
{
    return m_xDlg->execute();
}
IMPL_ABSTDLG_BASE(AbstractTabDialog_Impl);
short AbstractSwConvertTableDlg_Impl::Execute()
{
    return m_xDlg->run();
}
IMPL_ABSTDLG_BASE(AbstractSwInsertDBColAutoPilot_Impl);

short AbstractDropDownFieldDialog_Impl::Execute()
{
    return m_xDlg->execute();
}

short AbstractSwLabDlg_Impl::Execute()
{
    return m_xDlg->execute();
}

IMPL_ABSTDLG_BASE(AbstractSwSelGlossaryDlg_Impl);
short AbstractSwAutoFormatDlg_Impl::Execute()
{
    return m_xDlg->execute();
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
    return m_xDlg->execute();
}

short AbstractInsFootNoteDlg_Impl::Execute()
{
    return m_xDlg->execute();
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
IMPL_ABSTDLG_BASE(AbstractMailMergeCreateFromDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractMailMergeFieldConnectionsDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractMultiTOXTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractEditRegionDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractInsertSectionTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractIndexMarkFloatDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractAuthMarkFloatDlg_Impl);

void AbstractTabDialog_Impl::SetCurPageId( const OString &rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void AbstractTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}

//From class Window.
void AbstractTabDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
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

//From class Window.
void AbstractTabController_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

IMPL_LINK_NOARG(AbstractApplyTabDialog_Impl, ApplyHdl, Button*, void)
{
    if (pDlg->Apply())
        m_aHandler.Call(nullptr);
}

void AbstractApplyTabDialog_Impl::SetApplyHdl( const Link<LinkParamNone*,void>& rLink )
{
    m_aHandler = rLink;
    pDlg->SetApplyHandler(LINK(this, AbstractApplyTabDialog_Impl, ApplyHdl));
}

sal_uInt8 AbstractSwInsertAbstractDlg_Impl::GetLevel() const
{
    return pDlg->GetLevel();
}

sal_uInt8 AbstractSwInsertAbstractDlg_Impl::GetPara() const
{
    return pDlg->GetPara();
}

const SfxItemSet* SwAbstractSfxDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

void SwAbstractSfxDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
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
    pDlg->DataToDoc(rSelection, rxSource, xConnection, xResultSet);
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
    pDlg->InsertGlos( rRegion, rGlosName );
}

sal_Int32 AbstractSwSelGlossaryDlg_Impl::GetSelectedIdx() const
{
    return pDlg->GetSelectedIdx();
}

void AbstractSwSelGlossaryDlg_Impl::SelectEntryPos(sal_Int32 nIdx)
{
    pDlg->SelectEntryPos( nIdx );
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
                                SwTableAutoFormat *& prTAFormat )
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
    return pDlg->IsThisDocument();
}

bool AbstractMailMergeFieldConnectionsDlg_Impl::IsUseExistingConnections() const
{
    return pDlg->IsUseExistingConnections();
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
    pDlg->SelectSection(rSectionName);
}

void
AbstractInsertSectionTabDialog_Impl::SetSectionData(SwSectionData const& rSect)
{
    pDlg->SetSectionData(rSect);
}

void AbstractIndexMarkFloatDlg_Impl::ReInitDlg(SwWrtShell& rWrtShell)
{
    pDlg->ReInitDlg( rWrtShell);
}

vcl::Window* AbstractIndexMarkFloatDlg_Impl::GetWindow()
{
    return static_cast<vcl::Window*>(pDlg);
}

void AbstractAuthMarkFloatDlg_Impl::ReInitDlg(SwWrtShell& rWrtShell)
{
    pDlg->ReInitDlg( rWrtShell);
}

vcl::Window* AbstractAuthMarkFloatDlg_Impl::GetWindow()
{
    return static_cast<vcl::Window*>(pDlg);
}

vcl::Window* AbstractSwWordCountFloatDlg_Impl::GetWindow()
{
    return static_cast<vcl::Window*>(pDlg);
}

void AbstractSwWordCountFloatDlg_Impl::UpdateCounts()
{
    pDlg->UpdateCounts();
}

void AbstractSwWordCountFloatDlg_Impl::SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat)
{
    pDlg->SetCounts(rCurrCnt, rDocStat);
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

void AbstractMailMergeWizard_Impl::StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl )
{
    aEndDlgHdl = rEndDialogHdl;
    pDlg->StartExecuteModal(
        LINK( this, AbstractMailMergeWizard_Impl, EndDialogHdl ) );
}

sal_Int32 AbstractMailMergeWizard_Impl::GetResult()
{
    return pDlg->GetResult();
}

IMPL_LINK( AbstractMailMergeWizard_Impl, EndDialogHdl, Dialog&, rDialog, void )
{
    OSL_ENSURE( &rDialog == pDlg, "wrong dialog passed to EndDialogHdl!" );
    (void) rDialog; // unused in non-debug

    aEndDlgHdl.Call( *pDlg );
    aEndDlgHdl = Link<Dialog&,void>();
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

VclPtr<AbstractSwInsertAbstractDlg> SwAbstractDialogFactory_Impl::CreateSwInsertAbstractDlg()
{
    VclPtr<SwInsertAbstractDlg> pDlg = VclPtr<SwInsertAbstractDlg>::Create(nullptr);
    return VclPtr<AbstractSwInsertAbstractDlg_Impl>::Create(pDlg);
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwAddressAbstractDlg(vcl::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    VclPtr<SfxModalDialog> pDlg = VclPtr<SwAddrDlg>::Create( pParent, rSet );
    return VclPtr<SwAbstractSfxDialog_Impl>::Create(pDlg);
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwDropCapsDialog( vcl::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    VclPtr<SfxModalDialog> pDlg = VclPtr<SwDropCapsDlg>::Create(pParent, rSet);
    return VclPtr<SwAbstractSfxDialog_Impl>::Create( pDlg );
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwBackgroundDialog( vcl::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    VclPtr<SfxModalDialog> pDlg = VclPtr<SwBackgroundDlg>::Create( pParent, rSet );
    return VclPtr<SwAbstractSfxDialog_Impl>::Create( pDlg );
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateNumFormatDialog( vcl::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    VclPtr<SfxModalDialog> pDlg = VclPtr<SwNumFormatDlg>::Create( pParent, rSet );
    return VclPtr<SwAbstractSfxDialog_Impl>::Create( pDlg );
}

VclPtr<AbstractSwAsciiFilterDlg> SwAbstractDialogFactory_Impl::CreateSwAsciiFilterDlg(weld::Window* pParent,
       SwDocShell& rDocSh, SvStream* pStream)
{
    return VclPtr<AbstractSwAsciiFilterDlg_Impl>::Create(o3tl::make_unique<SwAsciiFilterDlg>(pParent, rDocSh, pStream));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwInsertBookmarkDlg( vcl::Window *pParent,
                                                                           SwWrtShell &rSh,
                                                                           SfxRequest& rReq )
{
    VclPtr<SwInsertBookmarkDlg> pDlg = VclPtr<SwInsertBookmarkDlg>::Create( pParent, rSh, rReq );
    return VclPtr<VclAbstractDialog_Impl>::Create( pDlg );
}

VclPtr<AbstractSwBreakDlg> SwAbstractDialogFactory_Impl::CreateSwBreakDlg(weld::Window* pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwBreakDlg_Impl>::Create(o3tl::make_unique<SwBreakDlg>(pParent, rSh));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwChangeDBDlg(SwView& rVw)
{
    VclPtr<Dialog> pDlg = VclPtr<SwChangeDBDlg>::Create(rVw);
    return VclPtr<VclAbstractDialog_Impl>::Create(pDlg);
}

VclPtr<SfxAbstractTabDialog>  SwAbstractDialogFactory_Impl::CreateSwCharDlg(vcl::Window* pParent, SwView& pVw,
    const SfxItemSet& rCoreSet, SwCharDlgMode nDialogMode, const OUString* pFormatStr)
{
    VclPtr<SfxTabDialog> pDlg = VclPtr<SwCharDlg>::Create(pParent, pVw, rCoreSet, nDialogMode, pFormatStr);
    return VclPtr<AbstractTabDialog_Impl>::Create(pDlg);
}

VclPtr<AbstractSwConvertTableDlg> SwAbstractDialogFactory_Impl::CreateSwConvertTableDlg(SwView& rView, bool bToTable)
{
    return VclPtr<AbstractSwConvertTableDlg_Impl>::Create(o3tl::make_unique<SwConvertTableDlg>(rView, bToTable));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwCaptionDialog ( vcl::Window *pParent, SwView &rV)
{
    VclPtr<Dialog> pDlg = VclPtr<SwCaptionDialog>::Create( pParent, rV );
    return VclPtr<VclAbstractDialog_Impl>::Create( pDlg );
}

VclPtr<AbstractSwInsertDBColAutoPilot> SwAbstractDialogFactory_Impl::CreateSwInsertDBColAutoPilot( SwView& rView,
        uno::Reference< sdbc::XDataSource> rxSource,
        uno::Reference<sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData)
{
    VclPtr<SwInsertDBColAutoPilot> pDlg = VclPtr<SwInsertDBColAutoPilot>::Create( rView, rxSource, xColSupp, rData );
    return VclPtr<AbstractSwInsertDBColAutoPilot_Impl>::Create( pDlg );
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwFootNoteOptionDlg(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractTabController_Impl>::Create(o3tl::make_unique<SwFootNoteOptionDlg>(pParent, rSh));
}

VclPtr<AbstractDropDownFieldDialog> SwAbstractDialogFactory_Impl::CreateDropDownFieldDialog(weld::Window *pParent,
    SwWrtShell &rSh, SwField* pField, bool bPrevButton, bool bNextButton)
{
    return VclPtr<AbstractDropDownFieldDialog_Impl>::Create(o3tl::make_unique<sw::DropDownFieldDialog>(pParent, rSh, pField, bPrevButton, bNextButton));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                                 SwWrtShell* pWrtSh, Printer* pPrt,
                                                                 bool bInsert)
{
    return VclPtr<AbstractTabController_Impl>::Create(o3tl::make_unique<SwEnvDlg>(pParent, rSet, pWrtSh,pPrt, bInsert));
}

VclPtr<AbstractSwLabDlg> SwAbstractDialogFactory_Impl::CreateSwLabDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel)
{
    return VclPtr<AbstractSwLabDlg_Impl>::Create(o3tl::make_unique<SwLabDlg>(pParent, rSet, pDBManager, bLabel));
}

SwLabDlgMethod SwAbstractDialogFactory_Impl::GetSwLabDlgStaticMethod ()
{
    return SwLabDlg::UpdateFieldInformation;
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwParaDlg ( vcl::Window *pParent, SwView& rVw,
                                                    const SfxItemSet& rCoreSet  ,
                                                    bool bDraw ,
                                                    const OString& sDefPage)
{
    VclPtr<SfxTabDialog> pDlg = VclPtr<SwParaDlg>::Create( pParent, rVw, rCoreSet, DLG_STD, nullptr, bDraw, sDefPage );
    return VclPtr<AbstractTabDialog_Impl>::Create( pDlg );
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwAutoMarkDialog(vcl::Window *pParent, SwWrtShell &rSh)
{
    VclPtr<Dialog> pDlg = VclPtr<SwAuthMarkModalDlg>::Create( pParent, rSh );
    return VclPtr<VclAbstractDialog_Impl>::Create( pDlg );
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwColumnDialog(vcl::Window *pParent, SwWrtShell &rSh)
{
    VclPtr<Dialog> pDlg = VclPtr<SwColumnDlg>::Create( pParent, rSh );
    return VclPtr<VclAbstractDialog_Impl>::Create( pDlg );
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwTableHeightDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwTableHeightDlg_Impl>::Create(o3tl::make_unique<SwTableHeightDlg>(pParent, rSh));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwSortingDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwSortDlg_Impl>::Create(o3tl::make_unique<SwSortDlg>(pParent, rSh));
}

VclPtr<AbstractSplitTableDialog> SwAbstractDialogFactory_Impl::CreateSplitTableDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSplitTableDialog_Impl>::Create(o3tl::make_unique<SwSplitTableDlg>(pParent, rSh));
}

VclPtr<AbstractSwSelGlossaryDlg> SwAbstractDialogFactory_Impl::CreateSwSelGlossaryDlg(const OUString &rShortName)
{
    VclPtr<SwSelGlossaryDlg> pDlg = VclPtr<SwSelGlossaryDlg>::Create(nullptr, rShortName);
    return VclPtr<AbstractSwSelGlossaryDlg_Impl>::Create(pDlg);
}

VclPtr<AbstractSwAutoFormatDlg> SwAbstractDialogFactory_Impl::CreateSwAutoFormatDlg(weld::Window* pParent,
    SwWrtShell* pShell, bool bSetAutoFormat, const SwTableAutoFormat* pSelFormat)
{
    return VclPtr<AbstractSwAutoFormatDlg_Impl>::Create(
                std::unique_ptr<SwAutoFormatDlg, o3tl::default_delete<SwAutoFormatDlg>>(
                    new SwAutoFormatDlg(pParent, pShell, bSetAutoFormat, pSelFormat)));
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwBorderDlg(vcl::Window* pParent, SfxItemSet& rSet, SwBorderModes nType )
{
    VclPtr<SfxModalDialog> pDlg = VclPtr<SwBorderDlg>::Create( pParent, rSet, nType );
    return VclPtr<SwAbstractSfxDialog_Impl>::Create( pDlg );
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwWrapDlg ( vcl::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh )
{
    VclPtr<SfxModalDialog> pDlg = VclPtr<SwWrapDlg>::Create( pParent, rSet, pSh, true/*bDrawMode*/ );
    return VclPtr<SwAbstractSfxDialog_Impl>::Create( pDlg );
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwTableWidthDlg(weld::Window *pParent, SwTableFUNC &rFnc)
{
    return VclPtr<AbstractSwTableWidthDlg_Impl>::Create(o3tl::make_unique<SwTableWidthDlg>(pParent, rFnc));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwTableTabDlg(vcl::Window* pParent,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
{
    VclPtr<SfxTabDialog> pDlg = VclPtr<SwTableTabDlg>::Create(pParent, pItemSet, pSh);
    return VclPtr<AbstractTabDialog_Impl>::Create(pDlg);
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
    return VclPtr<AbstractSwRenameXNamedDlg_Impl>::Create(o3tl::make_unique<SwRenameXNamedDlg>(pParent,xNamed, xNameAccess));
}

VclPtr<AbstractSwModalRedlineAcceptDlg> SwAbstractDialogFactory_Impl::CreateSwModalRedlineAcceptDlg(vcl::Window *pParent)
{
    VclPtr<SwModalRedlineAcceptDlg> pDlg = VclPtr<SwModalRedlineAcceptDlg>::Create( pParent );
    return VclPtr<AbstractSwModalRedlineAcceptDlg_Impl>::Create( pDlg );
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateTableMergeDialog(weld::Window* pParent, bool& rWithPrev)
{
    return VclPtr<AbstractSwMergeTableDlg_Impl>::Create(o3tl::make_unique<SwMergeTableDlg>(pParent, rWithPrev));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateFrameTabDialog(const OUString &rDialogType,
                                                SfxViewFrame *pFrame, vcl::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool        bNewFrame,
                                                const OString&  sDefPage )
{
    VclPtr<SfxTabDialog> pDlg = VclPtr<SwFrameDlg>::Create(pFrame, pParent, rCoreSet, bNewFrame, rDialogType, false/*bFormat*/, sDefPage, nullptr);
    return VclPtr<AbstractTabDialog_Impl>::Create(pDlg);
}

VclPtr<SfxAbstractApplyTabDialog> SwAbstractDialogFactory_Impl::CreateTemplateDialog(
                                                vcl::Window *pParent,
                                                SfxStyleSheetBase&  rBase,
                                                SfxStyleFamily      nRegion,
                                                const OString&      sPage,
                                                SwWrtShell*         pActShell,
                                                bool                bNew )
{
    VclPtr<SfxTabDialog> pDlg = VclPtr<SwTemplateDlg>::Create(pParent, rBase, nRegion, sPage, pActShell, bNew);
    return VclPtr<AbstractApplyTabDialog_Impl>::Create(pDlg);
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
    return VclPtr<AbstractFieldInputDlg_Impl>::Create(o3tl::make_unique<SwFieldInputDlg>(pParent, rSh, pField, bPrevButton, bNextButton));
}

VclPtr<AbstractInsFootNoteDlg> SwAbstractDialogFactory_Impl::CreateInsFootNoteDlg(
    weld::Window * pParent, SwWrtShell &rSh, bool bEd )
{
    return VclPtr<AbstractInsFootNoteDlg_Impl>::Create(o3tl::make_unique<SwInsFootNoteDlg>(pParent, rSh, bEd));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateTitlePageDlg(weld::Window *pParent)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(o3tl::make_unique<SwTitlePageDlg>(pParent));
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateVclSwViewDialog(SwView& rView)
{
    return VclPtr<AbstractGenericDialog_Impl>::Create(o3tl::make_unique<SwLineNumberingDlg>(rView));
}

VclPtr<AbstractInsTableDlg> SwAbstractDialogFactory_Impl::CreateInsTableDlg(SwView& rView)
{
    return VclPtr<AbstractInsTableDlg_Impl>::Create(o3tl::make_unique<SwInsTableDlg>(rView));
}

VclPtr<AbstractJavaEditDialog> SwAbstractDialogFactory_Impl::CreateJavaEditDialog(
    weld::Window* pParent, SwWrtShell* pWrtSh)
{
    return VclPtr<AbstractJavaEditDialog_Impl>::Create(o3tl::make_unique<SwJavaEditDialog>(pParent, pWrtSh));
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

VclPtr<AbstractMailMergeCreateFromDlg> SwAbstractDialogFactory_Impl::CreateMailMergeCreateFromDlg(vcl::Window* pParent)
{
    VclPtr<SwMailMergeCreateFromDlg> pDlg = VclPtr<SwMailMergeCreateFromDlg>::Create(pParent);
    return VclPtr<AbstractMailMergeCreateFromDlg_Impl>::Create(pDlg);
}

VclPtr<AbstractMailMergeFieldConnectionsDlg> SwAbstractDialogFactory_Impl::CreateMailMergeFieldConnectionsDlg(vcl::Window* pParent)
{
    VclPtr<SwMailMergeFieldConnectionsDlg> pDlg = VclPtr<SwMailMergeFieldConnectionsDlg>::Create( pParent );
    return VclPtr<AbstractMailMergeFieldConnectionsDlg_Impl>::Create( pDlg );
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateMultiTOXMarkDlg(weld::Window* pParent, SwTOXMgr &rTOXMgr)
{
    return VclPtr<AbstractMultiTOXMarkDlg_Impl>::Create(o3tl::make_unique<SwMultiTOXMarkDlg>(pParent, rTOXMgr));
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSvxNumBulletTabDialog(vcl::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell & rWrtSh)
{
    VclPtr<SfxTabDialog> pDlg = VclPtr<SwSvxNumBulletTabDialog>::Create(pParent, pSwItemSet, rWrtSh);
    return VclPtr<AbstractTabDialog_Impl>::Create( pDlg );
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateOutlineTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell & rWrtSh )
{
    return VclPtr<AbstractTabController_Impl>::Create(o3tl::make_unique<SwOutlineTabDialog>(pParent, pSwItemSet, rWrtSh));
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

VclPtr<AbstractEditRegionDlg> SwAbstractDialogFactory_Impl::CreateEditRegionDlg(vcl::Window* pParent, SwWrtShell& rWrtSh)
{
    VclPtr<SwEditRegionDlg> pDlg = VclPtr<SwEditRegionDlg>::Create( pParent, rWrtSh );
    return VclPtr<AbstractEditRegionDlg_Impl>::Create( pDlg );
}

VclPtr<AbstractInsertSectionTabDialog> SwAbstractDialogFactory_Impl::CreateInsertSectionTabDialog(
                                                    vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh)
{
    VclPtr<SwInsertSectionTabDialog> pDlg = VclPtr<SwInsertSectionTabDialog>::Create(pParent, rSet, rSh);
    return VclPtr<AbstractInsertSectionTabDialog_Impl>::Create(pDlg);
}

VclPtr<AbstractMarkFloatDlg> SwAbstractDialogFactory_Impl::CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       vcl::Window *pParent,
                                                       SfxChildWinInfo* pInfo )
{
    VclPtr<SwIndexMarkFloatDlg> pDlg = VclPtr<SwIndexMarkFloatDlg>::Create(pBindings, pChild, pParent, pInfo, true/*bNew*/);
    return VclPtr<AbstractIndexMarkFloatDlg_Impl>::Create(pDlg);
}

VclPtr<AbstractMarkFloatDlg> SwAbstractDialogFactory_Impl::CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       vcl::Window *pParent,
                                                       SfxChildWinInfo* pInfo)
{
    VclPtr<SwAuthMarkFloatDlg> pDlg = VclPtr<SwAuthMarkFloatDlg>::Create( pBindings, pChild, pParent, pInfo, true/*bNew*/ );
    return VclPtr<AbstractAuthMarkFloatDlg_Impl>::Create( pDlg );
}

VclPtr<AbstractSwWordCountFloatDlg> SwAbstractDialogFactory_Impl::CreateSwWordCountDialog(
                                                                              SfxBindings* pBindings,
                                                                              SfxChildWindow* pChild,
                                                                              vcl::Window *pParent,
                                                                              SfxChildWinInfo* pInfo)
{
    VclPtr<SwWordCountFloatDlg> pDlg = VclPtr<SwWordCountFloatDlg>::Create( pBindings, pChild, pParent, pInfo );
    return VclPtr<AbstractSwWordCountFloatDlg_Impl>::Create( pDlg );
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateIndexMarkModalDlg(
                                                vcl::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark )
{
    VclPtr<Dialog> pDlg = VclPtr<SwIndexMarkModalDlg>::Create( pParent, rSh, pCurTOXMark );
    return VclPtr<VclAbstractDialog_Impl>::Create( pDlg );
}

VclPtr<AbstractMailMergeWizard> SwAbstractDialogFactory_Impl::CreateMailMergeWizard(
                                    SwView& rView, std::shared_ptr<SwMailMergeConfigItem>& rConfigItem)
{
    return VclPtr<AbstractMailMergeWizard_Impl>::Create( VclPtr<SwMailMergeWizard>::Create(rView, rConfigItem));
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
    SwMMResultSaveDialog aDialog(pParent);
    aDialog.run();
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultPrintDialog(weld::Window* pParent)
{
    SwMMResultPrintDialog aDialog(pParent);
    aDialog.run();
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultEmailDialog()
{
    ScopedVclPtrInstance<SwMMResultEmailDialog> pDialog;
    pDialog->Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
