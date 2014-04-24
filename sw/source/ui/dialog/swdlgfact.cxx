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
#include "dialog.hrc"
#include "misc.hrc"
#include "chrdlg.hrc"
#include "table.hrc"
#include "frmui.hrc"
#include "dbui.hrc"
#include "globals.hrc"
#include "fldui.hrc"
#include "envelp.hrc"
#include "dochdl.hrc"
#include <index.hrc>
#include <../../core/uibase/dialog/regionsw.hrc>

#include <wordcountdialog.hxx>
#include "abstract.hxx"
#include "addrdlg.hxx"
#include "ascfldlg.hxx"
#include "bookmark.hxx"
#include "break.hxx"
#include "changedb.hxx"
#include "chrdlg.hxx"
#include "convert.hxx"
#include "cption.hxx"
#include "dbinsdlg.hxx"
#include "docfnote.hxx"
#include "docstdlg.hxx"
#include "DropDownFieldDialog.hxx"
#include "envlop.hxx"
#include "label.hxx"
#include "drpcps.hxx"
#include "swuipardlg.hxx"
#include "pattern.hxx"
#include "rowht.hxx"
#include "selglos.hxx"
#include "splittbl.hxx"
#include "srtdlg.hxx"
#include "tautofmt.hxx"
#include "tblnumfm.hxx"
#include "wrap.hxx"
#include "colwd.hxx"
#include "tabledlg.hxx"
#include "fldtdlg.hxx"
#include "fldedt.hxx"
#include "swrenamexnameddlg.hxx"
#include "swmodalredlineacceptdlg.hxx"
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

using namespace ::com::sun::star;

IMPL_ABSTDLG_BASE(AbstractSwWordCountFloatDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSwInsertAbstractDlg_Impl);
IMPL_ABSTDLG_BASE(SwAbstractSfxDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSwAsciiFilterDlg_Impl);
IMPL_ABSTDLG_BASE(VclAbstractDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSplitTableDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSwBreakDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSwConvertTableDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSwInsertDBColAutoPilot_Impl);
IMPL_ABSTDLG_BASE(AbstractDropDownFieldDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractSwLabDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSwSelGlossaryDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSwAutoFormatDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSwFldDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSwRenameXNamedDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractSwModalRedlineAcceptDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractGlossaryDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractFldInputDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractInsFootNoteDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractInsTableDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractJavaEditDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractMailMergeDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractMailMergeCreateFromDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractMailMergeFieldConnectionsDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractMultiTOXTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractEditRegionDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractInsertSectionTabDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractIndexMarkFloatDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractAuthMarkFloatDlg_Impl);

void AbstractTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

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

OUString AbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

IMPL_LINK_NOARG(AbstractApplyTabDialog_Impl, ApplyHdl)
{
    if (pDlg->Apply())
        m_aHandler.Call(NULL);
    return 0;
}

void AbstractApplyTabDialog_Impl::SetApplyHdl( const Link& rLink )
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

OUString SwAbstractSfxDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

void AbstractSwAsciiFilterDlg_Impl::FillOptions( SwAsciiOptions& rOptions )
{
    pDlg->FillOptions(rOptions);
}

sal_uInt16 AbstractSplitTableDialog_Impl::GetSplitMode()
{
    return pDlg->GetSplitMode();
}

OUString AbstractSwBreakDlg_Impl::GetTemplateName()
{
    return pDlg->GetTemplateName();
}

sal_uInt16 AbstractSwBreakDlg_Impl:: GetKind()
{
    return pDlg->GetKind();
}

::boost::optional<sal_uInt16> AbstractSwBreakDlg_Impl:: GetPageNumber()
{
    return pDlg->GetPageNumber();
}

void AbstractSwConvertTableDlg_Impl::GetValues( sal_Unicode& rDelim,SwInsertTableOptions& rInsTblFlags,
                                                SwTableAutoFmt const*& prTAFmt)
{
    pDlg->GetValues(rDelim,rInsTblFlags, prTAFmt);
}

void AbstractSwInsertDBColAutoPilot_Impl::DataToDoc( const uno::Sequence< uno::Any >& rSelection,
        uno::Reference< sdbc::XDataSource> rxSource,
        uno::Reference< sdbc::XConnection> xConnection,
        uno::Reference< sdbc::XResultSet > xResultSet)
{
    pDlg->DataToDoc(rSelection, rxSource, xConnection, xResultSet);
}

OString AbstractDropDownFieldDialog_Impl::GetWindowState( sal_uLong nMask  ) const
{
    return pDlg->GetWindowState(nMask);
}

void AbstractDropDownFieldDialog_Impl::SetWindowState( const OString& rStr )
{
    pDlg->SetWindowState(rStr);
}

void AbstractSwLabDlg_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

void AbstractSwLabDlg_Impl::SetCurPageId( const OString &rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSwLabDlg_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractSwLabDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void AbstractSwLabDlg_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}

void AbstractSwLabDlg_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

OUString AbstractSwLabDlg_Impl::GetText() const
{
    return pDlg->GetText();
}

const OUString& AbstractSwLabDlg_Impl::GetBusinessCardStr() const
{
    return pDlg->GetBusinessCardStr();
}

Printer * AbstractSwLabDlg_Impl::GetPrt()
{
    return pDlg->GetPrt();
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

void AbstractSwAutoFormatDlg_Impl::FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const
{
    pDlg->FillAutoFmtOfIndex(rToFill);
}

void AbstractSwFldDlg_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

void AbstractSwFldDlg_Impl::SetCurPageId( const OString &rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* AbstractSwFldDlg_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* AbstractSwFldDlg_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void AbstractSwFldDlg_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}

void AbstractSwFldDlg_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

OUString AbstractSwFldDlg_Impl::GetText() const
{
    return pDlg->GetText();
}

void AbstractSwFldDlg_Impl::Start( sal_Bool bShowl )
{
    pDlg->Start( bShowl );
}

void AbstractSwFldDlg_Impl::Initialize(SfxChildWinInfo *pInfo)
{
    pDlg->Initialize( pInfo );
}

void AbstractSwFldDlg_Impl::ReInitDlg()
{
    pDlg->ReInitDlg();
}

void AbstractSwFldDlg_Impl::ActivateDatabasePage()
{
    pDlg->ActivateDatabasePage();
}

Window* AbstractSwFldDlg_Impl::GetWindow()
{
    return (Window*)pDlg;
}

void AbstractSwFldDlg_Impl::ShowReferencePage()
{
    pDlg->ShowReferencePage();
}

void AbstractSwRenameXNamedDlg_Impl::SetForbiddenChars( const OUString& rSet )
{
    pDlg->SetForbiddenChars( rSet );
}

void    AbstractSwRenameXNamedDlg_Impl::SetAlternativeAccess(
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xSecond,
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xThird )
{
    pDlg->SetAlternativeAccess( xSecond, xThird);
}

void    AbstractSwModalRedlineAcceptDlg_Impl::AcceptAll( sal_Bool bAccept )
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

void AbstractFldInputDlg_Impl::SetWindowState( const OString& rStr )
{
    pDlg->SetWindowState( rStr );
}

OString AbstractFldInputDlg_Impl::GetWindowState( sal_uLong nMask ) const
{
    return pDlg->GetWindowState( nMask );
}

void AbstractFldInputDlg_Impl::EndDialog(long n)
{
    pDlg->EndDialog(n);
}

OUString AbstractInsFootNoteDlg_Impl::GetFontName()
{
    return pDlg->GetFontName();
}

sal_Bool AbstractInsFootNoteDlg_Impl::IsEndNote()
{
    return pDlg->IsEndNote();
}

OUString AbstractInsFootNoteDlg_Impl::GetStr()
{
    return pDlg->GetStr();
}

void AbstractInsFootNoteDlg_Impl::SetHelpId( const OString& sHelpId )
{
    pDlg->SetHelpId( sHelpId );
}

void AbstractInsFootNoteDlg_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}

void AbstractInsTableDlg_Impl::GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTblFlags, OUString& rTableAutoFmtName,
                                SwTableAutoFmt *& prTAFmt )
{
    pDlg->GetValues( rName, rRow, rCol, rInsTblFlags, rTableAutoFmtName, prTAFmt);
}

OUString AbstractJavaEditDialog_Impl::GetScriptText() const
{
    return pDlg->GetScriptText();
}

OUString AbstractJavaEditDialog_Impl::GetScriptType() const
{
    return pDlg->GetScriptType();
}

sal_Bool AbstractJavaEditDialog_Impl::IsUrl() const
{
    return pDlg->IsUrl();
}

sal_Bool AbstractJavaEditDialog_Impl::IsNew() const
{
    return pDlg->IsNew();
}

bool AbstractJavaEditDialog_Impl::IsUpdate() const
{
    return pDlg->IsUpdate();
}

sal_uInt16 AbstractMailMergeDlg_Impl::GetMergeType()
{
    return pDlg->GetMergeType();
}

const OUString& AbstractMailMergeDlg_Impl::GetSaveFilter() const
{
    return pDlg->GetSaveFilter();
}

const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > AbstractMailMergeDlg_Impl::GetSelection() const
{
    return pDlg->GetSelection();
}

uno::Reference< sdbc::XResultSet> AbstractMailMergeDlg_Impl::GetResultSet() const
{
    return pDlg->GetResultSet();
}

bool AbstractMailMergeDlg_Impl::IsSaveIndividualDocs() const
{
    return pDlg->IsSaveIndividualDocs();
}

bool AbstractMailMergeDlg_Impl::IsGenerateFromDataBase() const
{
    return pDlg->IsGenerateFromDataBase();
}

OUString AbstractMailMergeDlg_Impl::GetColumnName() const
{
    return pDlg->GetColumnName();
}

OUString AbstractMailMergeDlg_Impl::GetPath() const
{
    return pDlg->GetPath();
}

sal_Bool AbstractMailMergeCreateFromDlg_Impl::IsThisDocument() const
{
    return pDlg->IsThisDocument();
}

sal_Bool AbstractMailMergeFieldConnectionsDlg_Impl::IsUseExistingConnections() const
{
    return pDlg->IsUseExistingConnections();
}

SwForm* AbstractMultiTOXTabDialog_Impl::GetForm(CurTOXType eType)
{
    return pDlg->GetForm(eType);
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

Window* AbstractIndexMarkFloatDlg_Impl::GetWindow()
{
    return (Window*)pDlg;
}

void AbstractAuthMarkFloatDlg_Impl::ReInitDlg(SwWrtShell& rWrtShell)
{
    pDlg->ReInitDlg( rWrtShell);
}

Window* AbstractAuthMarkFloatDlg_Impl::GetWindow()
{
    return (Window*)pDlg;
}

Window* AbstractSwWordCountFloatDlg_Impl::GetWindow()
{
    return (Window*)pDlg;
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
    delete pDlg;
}

void AbstractMailMergeWizard_Impl::StartExecuteModal( const Link& rEndDialogHdl )
{
    aEndDlgHdl = rEndDialogHdl;
    pDlg->StartExecuteModal(
        LINK( this, AbstractMailMergeWizard_Impl, EndDialogHdl ) );
}

long AbstractMailMergeWizard_Impl::GetResult()
{
    return pDlg->GetResult();
}

IMPL_LINK( AbstractMailMergeWizard_Impl, EndDialogHdl, SwMailMergeWizard*, pDialog )
{
    OSL_ENSURE( pDialog == pDlg, "wrong dialog passed to EndDialogHdl!" );
    (void) pDialog; // unused in non-debug

    aEndDlgHdl.Call( this );
    aEndDlgHdl = Link();

    return 0L;
}

void AbstractMailMergeWizard_Impl::SetReloadDocument(const OUString& rURL)
{
    pDlg->SetReloadDocument(rURL);
}

OUString AbstractMailMergeWizard_Impl::GetReloadDocument() const
{
    return pDlg->GetReloadDocument();
}

sal_Bool AbstractMailMergeWizard_Impl::ShowPage( sal_uInt16 nLevel )
{
    return pDlg->skipUntil(nLevel);
}

sal_uInt16 AbstractMailMergeWizard_Impl::GetRestartPage() const
{
    return pDlg->GetRestartPage();
}

AbstractSwInsertAbstractDlg * SwAbstractDialogFactory_Impl::CreateSwInsertAbstractDlg(Window* pParent)
{
    SwInsertAbstractDlg* pDlg = new SwInsertAbstractDlg( pParent);
    return new AbstractSwInsertAbstractDlg_Impl(pDlg);
}

SfxAbstractDialog* SwAbstractDialogFactory_Impl::CreateSfxDialog( Window* pParent,
                                                                        const SfxItemSet& rSet,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&,
                                                                        sal_uInt32 nResId
                                                                )
{
    SfxModalDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RC_DLG_ADDR :
            pDlg = new SwAddrDlg( pParent, rSet );
            break;
        case DLG_SWDROPCAPS :
            pDlg = new SwDropCapsDlg( pParent, rSet );
            break;
        case RC_SWDLG_BACKGROUND :
            pDlg = new SwBackgroundDlg( pParent, rSet );
            break;
        case RC_DLG_SWNUMFMTDLG :
            pDlg = new SwNumFmtDlg( pParent, rSet );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new SwAbstractSfxDialog_Impl( pDlg );
    return 0;
}

AbstractSwAsciiFilterDlg* SwAbstractDialogFactory_Impl::CreateSwAsciiFilterDlg( Window* pParent,
                                                                               SwDocShell& rDocSh,
                                                                                SvStream* pStream )
{
    SwAsciiFilterDlg* pDlg = new SwAsciiFilterDlg( pParent, rDocSh, pStream );
    return new AbstractSwAsciiFilterDlg_Impl( pDlg );
}

VclAbstractDialog* SwAbstractDialogFactory_Impl::CreateSwInsertBookmarkDlg( Window *pParent,
                                                                           SwWrtShell &rSh,
                                                                           SfxRequest& rReq,
                                                                           int nResId )
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case DLG_INSERT_BOOKMARK :
            pDlg = new SwInsertBookmarkDlg( pParent, rSh, rReq );
            break;

        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

AbstractSwBreakDlg * SwAbstractDialogFactory_Impl::CreateSwBreakDlg(Window *pParent,
                                                                     SwWrtShell &rSh)
{
    SwBreakDlg* pDlg = new SwBreakDlg(pParent, rSh);
    return new AbstractSwBreakDlg_Impl(pDlg);
}

VclAbstractDialog   * SwAbstractDialogFactory_Impl::CreateSwChangeDBDlg(SwView& rVw)
{
    Dialog* pDlg = new SwChangeDBDlg(rVw);
    return new VclAbstractDialog_Impl(pDlg);
}

SfxAbstractTabDialog *  SwAbstractDialogFactory_Impl::CreateSwCharDlg(Window* pParent, SwView& pVw,
    const SfxItemSet& rCoreSet, sal_uInt8 nDialogMode, const OUString* pFmtStr)
{

    SfxTabDialog* pDlg = new SwCharDlg(pParent, pVw, rCoreSet, nDialogMode, pFmtStr);
    return new AbstractTabDialog_Impl(pDlg);
}

AbstractSwConvertTableDlg* SwAbstractDialogFactory_Impl::CreateSwConvertTableDlg(SwView& rView, bool bToTable)
{
    SwConvertTableDlg* pDlg = new SwConvertTableDlg(rView, bToTable);
    return new AbstractSwConvertTableDlg_Impl(pDlg);
}

VclAbstractDialog * SwAbstractDialogFactory_Impl::CreateSwCaptionDialog ( Window *pParent, SwView &rV,int nResId)
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case DLG_CAPTION :
            pDlg = new SwCaptionDialog( pParent, rV );
            break;

        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

AbstractSwInsertDBColAutoPilot* SwAbstractDialogFactory_Impl::CreateSwInsertDBColAutoPilot( SwView& rView,
        uno::Reference< sdbc::XDataSource> rxSource,
        uno::Reference<sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData)
{
    SwInsertDBColAutoPilot* pDlg = new SwInsertDBColAutoPilot( rView, rxSource, xColSupp, rData );
    return new AbstractSwInsertDBColAutoPilot_Impl( pDlg );
}

SfxAbstractTabDialog *  SwAbstractDialogFactory_Impl::CreateSwFootNoteOptionDlg(Window *pParent, SwWrtShell &rSh)
{
    SfxTabDialog* pDlg = new SwFootNoteOptionDlg( pParent, rSh );
    return new AbstractTabDialog_Impl( pDlg );
}

AbstractDropDownFieldDialog *  SwAbstractDialogFactory_Impl::CreateDropDownFieldDialog(Window *pParent,
    SwWrtShell &rSh, SwField* pField, sal_Bool bNextButton)
{
    sw::DropDownFieldDialog* pDlg = new sw::DropDownFieldDialog(pParent, rSh, pField, bNextButton);
    return new AbstractDropDownFieldDialog_Impl( pDlg );
}

SfxAbstractTabDialog* SwAbstractDialogFactory_Impl::CreateSwEnvDlg ( Window* pParent, const SfxItemSet& rSet,
                                                                 SwWrtShell* pWrtSh, Printer* pPrt,
                                                                 sal_Bool bInsert )
{
    SfxTabDialog* pDlg = new SwEnvDlg( pParent, rSet, pWrtSh,pPrt, bInsert  );
    return new AbstractTabDialog_Impl( pDlg );
}

AbstractSwLabDlg* SwAbstractDialogFactory_Impl::CreateSwLabDlg(Window* pParent, const SfxItemSet& rSet,
                                                     SwDBMgr* pDBMgr, bool bLabel)
{
    SwLabDlg* pDlg = new SwLabDlg(pParent, rSet, pDBMgr, bLabel);
    return new AbstractSwLabDlg_Impl(pDlg);
}

SwLabDlgMethod SwAbstractDialogFactory_Impl::GetSwLabDlgStaticMethod ()
{
    return SwLabDlg::UpdateFieldInformation;
}

SfxAbstractTabDialog* SwAbstractDialogFactory_Impl::CreateSwParaDlg ( Window *pParent, SwView& rVw,
                                                    const SfxItemSet& rCoreSet  ,
                                                    sal_uInt8 nDialogMode,
                                                    const OUString *pCollName,
                                                    sal_Bool bDraw ,
                                                    const OString& sDefPage)
{
    SfxTabDialog* pDlg = new SwParaDlg( pParent, rVw, rCoreSet,nDialogMode, pCollName, bDraw, sDefPage );
    return new AbstractTabDialog_Impl( pDlg );
}

VclAbstractDialog * SwAbstractDialogFactory_Impl::CreateVclAbstractDialog ( Window *pParent, SwWrtShell &rSh, int nResId )
{
    Dialog* pDlg=NULL;
    switch ( nResId )
    {
        case DLG_ROW_HEIGHT :
            pDlg = new SwTableHeightDlg( pParent, rSh);
            break;
        case DLG_SORTING :
            pDlg = new SwSortDlg( pParent, rSh);
            break;
        case DLG_COLUMN :
            pDlg = new SwColumnDlg( pParent, rSh );
            break;
        case DLG_EDIT_AUTHMARK :
            pDlg = new SwAuthMarkModalDlg( pParent, rSh );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

AbstractSplitTableDialog * SwAbstractDialogFactory_Impl::CreateSplitTblDialog ( Window *pParent, SwWrtShell &rSh )
{
    return new AbstractSplitTableDialog_Impl( new SwSplitTblDlg( pParent, rSh) );
}

AbstractSwSelGlossaryDlg * SwAbstractDialogFactory_Impl::CreateSwSelGlossaryDlg(Window * pParent, const OUString &rShortName)
{
    SwSelGlossaryDlg* pDlg = new SwSelGlossaryDlg(pParent, rShortName);
    return new AbstractSwSelGlossaryDlg_Impl(pDlg);
}

AbstractSwAutoFormatDlg * SwAbstractDialogFactory_Impl::CreateSwAutoFormatDlg(Window* pParent,
    SwWrtShell* pShell, sal_Bool bSetAutoFmt, const SwTableAutoFmt* pSelFmt)
{
    SwAutoFormatDlg* pDlg = new SwAutoFormatDlg(pParent, pShell, bSetAutoFmt, pSelFmt);
    return new AbstractSwAutoFormatDlg_Impl(pDlg);
}

SfxAbstractDialog * SwAbstractDialogFactory_Impl::CreateSwBorderDlg(Window* pParent, SfxItemSet& rSet, sal_uInt16 nType, int nResId )
{
    SfxModalDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RC_DLG_SWBORDERDLG :
            pDlg = new SwBorderDlg( pParent, rSet, nType );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new SwAbstractSfxDialog_Impl( pDlg );
    return 0;
}

SfxAbstractDialog* SwAbstractDialogFactory_Impl::CreateSwWrapDlg ( Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, sal_Bool bDrawMode, int nResId )
{
    SfxModalDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RC_DLG_SWWRAPDLG :
            pDlg = new SwWrapDlg( pParent, rSet, pSh, bDrawMode );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new SwAbstractSfxDialog_Impl( pDlg );
    return 0;
}

VclAbstractDialog * SwAbstractDialogFactory_Impl::CreateSwTableWidthDlg(Window *pParent, SwTableFUNC &rFnc)
{
    Dialog* pDlg = new SwTableWidthDlg(pParent, rFnc);
    return new VclAbstractDialog_Impl( pDlg );
}

SfxAbstractTabDialog* SwAbstractDialogFactory_Impl::CreateSwTableTabDlg(Window* pParent, SfxItemPool& Pool,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
{
    SfxTabDialog* pDlg = new SwTableTabDlg(pParent, Pool, pItemSet, pSh);
    return new AbstractTabDialog_Impl(pDlg);
}

AbstractSwFldDlg * SwAbstractDialogFactory_Impl::CreateSwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent)
{
    SwFldDlg* pDlg = new SwFldDlg(pB, pCW, pParent);
    return new AbstractSwFldDlg_Impl(pDlg);
}

SfxAbstractDialog*   SwAbstractDialogFactory_Impl::CreateSwFldEditDlg ( SwView& rVw, int nResId )
{
    SfxModalDialog* pDlg=NULL;
    switch ( nResId )
    {
        case RC_DLG_SWFLDEDITDLG :
            pDlg = new SwFldEditDlg( rVw );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new SwAbstractSfxDialog_Impl( pDlg );
    return 0;
}

AbstractSwRenameXNamedDlg * SwAbstractDialogFactory_Impl::CreateSwRenameXNamedDlg(Window* pParent,
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > & xNamed,
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xNameAccess)
{
    SwRenameXNamedDlg* pDlg = new SwRenameXNamedDlg( pParent,xNamed, xNameAccess);
    return new AbstractSwRenameXNamedDlg_Impl( pDlg );
}

AbstractSwModalRedlineAcceptDlg * SwAbstractDialogFactory_Impl::CreateSwModalRedlineAcceptDlg(Window *pParent)
{
    SwModalRedlineAcceptDlg* pDlg = new SwModalRedlineAcceptDlg( pParent );
    return new AbstractSwModalRedlineAcceptDlg_Impl( pDlg );
}

VclAbstractDialog * SwAbstractDialogFactory_Impl::CreateTblMergeDialog(Window* pParent, sal_Bool& rWithPrev)
{
    Dialog* pDlg = new SwMergeTblDlg( pParent, rWithPrev );
    return new VclAbstractDialog_Impl( pDlg );
}

SfxAbstractTabDialog* SwAbstractDialogFactory_Impl::CreateFrmTabDialog(const OString &rDialogType,
                                                SfxViewFrame *pFrame, Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool        bNewFrm,
                                                bool        bFmt,
                                                const OString&  sDefPage,
                                                const OUString* pFmtStr )
{
    SfxTabDialog* pDlg = new SwFrmDlg(pFrame, pParent, rCoreSet, bNewFrm, rDialogType, bFmt, sDefPage, pFmtStr);
    return new AbstractTabDialog_Impl(pDlg);
}

SfxAbstractApplyTabDialog* SwAbstractDialogFactory_Impl::CreateTemplateDialog(
                                                Window*             pParent,
                                                SfxStyleSheetBase&  rBase,
                                                sal_uInt16          nRegion,
                                                const OString&      sPage,
                                                SwWrtShell*         pActShell,
                                                bool                bNew )
{
    SfxTabDialog* pDlg = new SwTemplateDlg( pParent, rBase, nRegion, sPage, pActShell, bNew );
    return new AbstractApplyTabDialog_Impl( pDlg );
}

AbstractGlossaryDlg* SwAbstractDialogFactory_Impl::CreateGlossaryDlg(SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell)
{
    SwGlossaryDlg* pDlg = new SwGlossaryDlg(pViewFrame, pGlosHdl, pWrtShell);
    return new AbstractGlossaryDlg_Impl( pDlg );
}

AbstractFldInputDlg* SwAbstractDialogFactory_Impl::CreateFldInputDlg(Window *pParent,
    SwWrtShell &rSh, SwField* pField, bool bNextButton)
{
    SwFldInputDlg* pDlg = new SwFldInputDlg( pParent, rSh, pField, bNextButton );
    return new AbstractFldInputDlg_Impl( pDlg );
}

AbstractInsFootNoteDlg* SwAbstractDialogFactory_Impl::CreateInsFootNoteDlg(
    Window * pParent, SwWrtShell &rSh, sal_Bool bEd )
{
    SwInsFootNoteDlg* pDlg = new SwInsFootNoteDlg(pParent, rSh, bEd);
    return new AbstractInsFootNoteDlg_Impl( pDlg );
}

VclAbstractDialog * SwAbstractDialogFactory_Impl::CreateTitlePageDlg ( Window *pParent )
{
    Dialog* pDlg = new SwTitlePageDlg( pParent );
    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}

VclAbstractDialog * SwAbstractDialogFactory_Impl::CreateVclSwViewDialog(SwView& rView)
{
    Dialog* pDlg = new SwLineNumberingDlg( &rView );
    return new VclAbstractDialog_Impl( pDlg );
}

AbstractInsTableDlg * SwAbstractDialogFactory_Impl::CreateInsTableDlg(SwView& rView)
{
    SwInsTableDlg* pDlg = new SwInsTableDlg(rView);
    return new AbstractInsTableDlg_Impl( pDlg );
}

AbstractJavaEditDialog * SwAbstractDialogFactory_Impl::CreateJavaEditDialog(
    Window* pParent, SwWrtShell* pWrtSh)
{
    SwJavaEditDialog* pDlg = new SwJavaEditDialog(pParent, pWrtSh);
    return new AbstractJavaEditDialog_Impl( pDlg );
}

AbstractMailMergeDlg * SwAbstractDialogFactory_Impl::CreateMailMergeDlg( int nResId,
                                                Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTblName,
                                                sal_Int32 nCommandType,
                                                const uno::Reference< sdbc::XConnection>& xConnection,
                                                uno::Sequence< uno::Any >* pSelection )
{
    SwMailMergeDlg* pDlg=NULL;
    switch ( nResId )
    {
        case DLG_MAILMERGE :
            pDlg = new SwMailMergeDlg( pParent, rSh, rSourceName, rTblName, nCommandType, xConnection, pSelection );
            break;
        default:
            break;
    }
    if ( pDlg )
        return new AbstractMailMergeDlg_Impl( pDlg );
    return 0;
}

AbstractMailMergeCreateFromDlg * SwAbstractDialogFactory_Impl::CreateMailMergeCreateFromDlg(Window* pParent)
{
    SwMailMergeCreateFromDlg* pDlg = new SwMailMergeCreateFromDlg(pParent);
    return new AbstractMailMergeCreateFromDlg_Impl(pDlg);
}

AbstractMailMergeFieldConnectionsDlg * SwAbstractDialogFactory_Impl::CreateMailMergeFieldConnectionsDlg(Window* pParent)
{
    SwMailMergeFieldConnectionsDlg* pDlg = new SwMailMergeFieldConnectionsDlg( pParent );
    return new AbstractMailMergeFieldConnectionsDlg_Impl( pDlg );
}

VclAbstractDialog * SwAbstractDialogFactory_Impl::CreateMultiTOXMarkDlg(Window* pParent, SwTOXMgr &rTOXMgr)
{
    Dialog* pDlg = new SwMultiTOXMarkDlg(pParent, rTOXMgr);
    return new VclAbstractDialog_Impl(pDlg);
}

SfxAbstractTabDialog* SwAbstractDialogFactory_Impl::CreateSwTabDialog( int nResId,
                                                Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell & rWrtSh )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nResId )
    {
        case DLG_SVXTEST_NUM_BULLET :
            pDlg = new SwSvxNumBulletTabDialog( pParent, pSwItemSet, rWrtSh );
            break;
        case DLG_TAB_OUTLINE :
            pDlg = new SwOutlineTabDialog( pParent, pSwItemSet, rWrtSh );
            break;

        default:
            break;
    }
    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}

AbstractMultiTOXTabDialog * SwAbstractDialogFactory_Impl::CreateMultiTOXTabDialog(
                                                Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX, sal_uInt16 nToxType,
                                                sal_Bool bGlobal)
{
    SwMultiTOXTabDialog* pDlg = new SwMultiTOXTabDialog( pParent, rSet, rShell, pCurTOX, nToxType, bGlobal );
    return new AbstractMultiTOXTabDialog_Impl( pDlg );
}

AbstractEditRegionDlg * SwAbstractDialogFactory_Impl::CreateEditRegionDlg(Window* pParent, SwWrtShell& rWrtSh)
{
    SwEditRegionDlg* pDlg = new SwEditRegionDlg( pParent, rWrtSh );
    return new AbstractEditRegionDlg_Impl( pDlg );
}

AbstractInsertSectionTabDialog * SwAbstractDialogFactory_Impl::CreateInsertSectionTabDialog(
                                                    Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh)
{
    SwInsertSectionTabDialog* pDlg = new SwInsertSectionTabDialog(pParent, rSet, rSh);
    return new AbstractInsertSectionTabDialog_Impl(pDlg);
}

AbstractMarkFloatDlg * SwAbstractDialogFactory_Impl::CreateIndexMarkFloatDlg(
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew )
{
    SwIndexMarkFloatDlg* pDlg = new SwIndexMarkFloatDlg(pBindings, pChild, pParent, pInfo, bNew);
    return new AbstractIndexMarkFloatDlg_Impl(pDlg);
}

AbstractMarkFloatDlg * SwAbstractDialogFactory_Impl::CreateAuthMarkFloatDlg(
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew)
{
    SwAuthMarkFloatDlg* pDlg = new SwAuthMarkFloatDlg( pBindings, pChild, pParent, pInfo, bNew );
    return new AbstractAuthMarkFloatDlg_Impl( pDlg );
}

AbstractSwWordCountFloatDlg * SwAbstractDialogFactory_Impl::CreateSwWordCountDialog(
                                                                              SfxBindings* pBindings,
                                                                              SfxChildWindow* pChild,
                                                                              Window *pParent,
                                                                              SfxChildWinInfo* pInfo)
{
    SwWordCountFloatDlg* pDlg = new SwWordCountFloatDlg( pBindings, pChild, pParent, pInfo );
    return new AbstractSwWordCountFloatDlg_Impl( pDlg );
}

VclAbstractDialog * SwAbstractDialogFactory_Impl::CreateIndexMarkModalDlg(
                                                Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark )
{
    Dialog* pDlg = new SwIndexMarkModalDlg( pParent, rSh, pCurTOXMark );
    return new VclAbstractDialog_Impl( pDlg );
}

AbstractMailMergeWizard*    SwAbstractDialogFactory_Impl::CreateMailMergeWizard(
                                    SwView& rView, SwMailMergeConfigItem& rConfigItem)
{
    return new AbstractMailMergeWizard_Impl( new SwMailMergeWizard(rView, rConfigItem));
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
    CreateTabPage pRet = 0;
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
            pRet = SwShdwCrsrOptionsTabPage::Create;
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
        case TP_DOC_STAT :
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

GetTabPageRanges SwAbstractDialogFactory_Impl::GetTabPageRangesFunc( sal_uInt16 nId )
{
    switch ( nId )
    {
    case 1 : //RID_SVXPAGE_TEXTANIMATION :
            //return SvxTextAnimationPage::GetRanges;
            break;
        default:
            break;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
