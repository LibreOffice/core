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
#ifndef INCLUDED_SW_SOURCE_UI_DIALOG_SWDLGFACT_HXX
#define INCLUDED_SW_SOURCE_UI_DIALOG_SWDLGFACT_HXX

#include <swabstdlg.hxx>

#include <abstract.hxx>
#include <ascfldlg.hxx>
#include <cnttab.hxx>
#include <colwd.hxx>
#include <contentcontrollistitemdlg.hxx>
#include <convert.hxx>
#include <DateFormFieldDialog.hxx>
#include <dbinsdlg.hxx>
#include <DropDownFieldDialog.hxx>
#include <DropDownFormFieldDialog.hxx>
#include <fldtdlg.hxx>
#include <glossary.hxx>
#include <inpdlg.hxx>
#include <insfnote.hxx>
#include <javaedit.hxx>
#include <label.hxx>
#include <mailmergewizard.hxx>
#include <mailmrge.hxx>
#include <mergetbl.hxx>
#include <multmrk.hxx>
#include <regionsw.hxx>
#include <rowht.hxx>
#include <selglos.hxx>
#include <splittbl.hxx>
#include <srtdlg.hxx>
#include <swmodalredlineacceptdlg.hxx>
#include <swrenamexnameddlg.hxx>
#include <swuicnttab.hxx>
#include <swuiidxmrk.hxx>
#include <tautofmt.hxx>
#include <utility>
#include <wordcountdialog.hxx>
#include <itabenum.hxx>
#include <optional>
#include <o3tl/deleter.hxx>
#include <pagenumberdlg.hxx>
#include <changedb.hxx>
#include <copyfielddlg.hxx>

//AbstractDialogFactory_Impl implementations
class SwAbstractDialogFactory_Impl : public SwAbstractDialogFactory
{

public:
    virtual ~SwAbstractDialogFactory_Impl() {}

    virtual VclPtr<SfxAbstractDialog> CreateNumFormatDialog(weld::Widget* pParent, const SfxItemSet& rAttr) override;
    virtual VclPtr<SfxAbstractDialog> CreateSwDropCapsDialog(weld::Window* pParent, const SfxItemSet& rSet) override;
    virtual VclPtr<SfxAbstractDialog> CreateSwBackgroundDialog(weld::Window* pParent, const SfxItemSet& rSet) override;
    virtual VclPtr<AbstractSwWordCountFloatDlg> CreateSwWordCountDialog(SfxBindings* pBindings,
        SfxChildWindow* pChild, weld::Window *pParent, SfxChildWinInfo* pInfo) override;
    virtual VclPtr<AbstractSwInsertAbstractDlg> CreateSwInsertAbstractDlg(weld::Window* pParent) override;
    virtual VclPtr<SfxAbstractDialog> CreateSwAddressAbstractDlg(weld::Window* pParent, const SfxItemSet& rSet) override;
    virtual VclPtr<AbstractSwAsciiFilterDlg>  CreateSwAsciiFilterDlg(weld::Window* pParent, SwDocShell& rDocSh,
                                                                SvStream* pStream) override;
    virtual VclPtr<VclAbstractDialog> CreateSwInsertBookmarkDlg(weld::Window *pParent, SwWrtShell &rSh, OUString const* pSelected) override;
    virtual VclPtr<VclAbstractDialog> CreateSwContentControlDlg(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<AbstractSwPageNumberDlg> CreateSwPageNumberDlg(weld::Window *pParent) override;

    VclPtr<AbstractSwContentControlListItemDlg>
    CreateSwContentControlListItemDlg(weld::Window* pParent,
                                      SwContentControlListItem& rItem) override;

    virtual std::shared_ptr<AbstractSwBreakDlg> CreateSwBreakDlg(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual std::shared_ptr<AbstractSwTranslateLangSelectDlg> CreateSwTranslateLangSelectDlg(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<AbstractChangeDbDialog> CreateSwChangeDBDlg(SwView& rVw) override;
    virtual VclPtr<SfxAbstractTabDialog>  CreateSwCharDlg(weld::Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
        SwCharDlgMode nDialogMode, const OUString* pFormatStr = nullptr) override;
    virtual VclPtr<AbstractSwConvertTableDlg> CreateSwConvertTableDlg(SwView& rView, bool bToTable) override;
    virtual VclPtr<VclAbstractDialog> CreateSwCaptionDialog(weld::Window *pParent, SwView &rV) override;
    virtual VclPtr<AbstractSwInsertDBColAutoPilot> CreateSwInsertDBColAutoPilot(SwView& rView,
        css::uno::Reference< css::sdbc::XDataSource> rxSource,
        css::uno::Reference<css::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateSwFootNoteOptionDlg(weld::Window *pParent, SwWrtShell &rSh) override;

    virtual VclPtr<AbstractDropDownFieldDialog> CreateDropDownFieldDialog(weld::Widget* pParent, SwWrtShell &rSh,
        SwField* pField, bool bPrevButton, bool bNextButton) override;
    virtual VclPtr<AbstractDropDownFormFieldDialog> CreateDropDownFormFieldDialog(weld::Widget* pParent, sw::mark::Fieldmark* pDropDownField) override;
    virtual VclPtr<AbstractDateFormFieldDialog> CreateDateFormFieldDialog(weld::Widget* pParent, sw::mark::DateFieldmark* pDateField, SwDoc& rDoc) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateSwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert) override;
    virtual VclPtr<AbstractSwLabDlg> CreateSwLabDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel) override;

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod () override;
    virtual VclPtr<SfxAbstractTabDialog> CreateSwParaDlg(weld::Window *pParent,
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    bool bDraw,
                                                    const OUString& sDefPage = {}) override;

    virtual VclPtr<VclAbstractDialog> CreateSwAutoMarkDialog(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<AbstractSwSelGlossaryDlg> CreateSwSelGlossaryDlg(weld::Window *pParent, const OUString &rShortName) override;
    virtual VclPtr<AbstractSwSortDlg> CreateSwSortingDialog(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<AbstractSwTableHeightDlg> CreateSwTableHeightDialog(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<VclAbstractDialog> CreateSwColumnDialog(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<AbstractSplitTableDialog> CreateSplitTableDialog(weld::Window* pParent, SwWrtShell &rSh) override;

    virtual VclPtr<AbstractSwAutoFormatDlg> CreateSwAutoFormatDlg(weld::Window* pParent, SwWrtShell* pShell,
                                                                  bool bSetAutoFormat = true,
                                                                  const SwTableAutoFormat* pSelFormat = nullptr) override;
    virtual VclPtr<SfxAbstractDialog> CreateSwBorderDlg(weld::Window* pParent, SfxItemSet& rSet, SwBorderModes nType) override;

    virtual VclPtr<SfxAbstractDialog> CreateSwWrapDlg(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pSh) override;
    virtual VclPtr<AbstractSwTableWidthDlg> CreateSwTableWidthDlg(weld::Window *pParent, SwWrtShell *pShell) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateSwTableTabDlg(weld::Window* pParent,
        const SfxItemSet* pItemSet, SwWrtShell* pSh) override;
    virtual VclPtr<AbstractSwFieldDlg> CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, weld::Window *pParent) override;
    virtual VclPtr<SfxAbstractDialog>   CreateSwFieldEditDlg ( SwView& rVw ) override;
    virtual VclPtr<AbstractSwRenameXNamedDlg> CreateSwRenameXNamedDlg(weld::Widget* pParent,
        css::uno::Reference< css::container::XNamed > & xNamed,
        css::uno::Reference< css::container::XNameAccess > & xNameAccess) override;
    virtual VclPtr<AbstractSwModalRedlineAcceptDlg> CreateSwModalRedlineAcceptDlg(weld::Window *pParent) override;

    virtual VclPtr<VclAbstractDialog>          CreateTableMergeDialog(weld::Window* pParent, bool& rWithPrev) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateFrameTabDialog( const OUString &rDialogType,
                                                SfxViewFrame& rFrame, weld::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool bNewFrame  = true,
                                                const OUString& sDefPage = {}) override;
    virtual VclPtr<SfxAbstractApplyTabDialog>  CreateTemplateDialog(
                                                weld::Window *pParent,
                                                SfxStyleSheetBase&  rBase,
                                                SfxStyleFamily      nRegion,
                                                const OUString&     sPage,
                                                SwWrtShell*         pActShell,
                                                bool                bNew) override;
    virtual VclPtr<AbstractGlossaryDlg>        CreateGlossaryDlg(SfxViewFrame& rViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell) override;
    virtual VclPtr<AbstractFieldInputDlg>        CreateFieldInputDlg(weld::Widget *pParent,
        SwWrtShell &rSh, SwField* pField, bool bPrevButton, bool bNextButton) override;
    virtual VclPtr<AbstractInsFootNoteDlg>     CreateInsFootNoteDlg(
        weld::Window * pParent, SwWrtShell &rSh, bool bEd = false) override;
    virtual VclPtr<VclAbstractDialog>         CreateTitlePageDlg(weld::Window* pParent) override;
    virtual VclPtr<VclAbstractDialog>         CreateVclSwViewDialog(SwView& rView) override;
    virtual std::shared_ptr<AbstractInsTableDlg> CreateInsTableDlg(SwView& rView) override;
    virtual VclPtr<AbstractJavaEditDialog>     CreateJavaEditDialog(weld::Window* pParent,
        SwWrtShell* pWrtSh) override;
    virtual VclPtr<AbstractMailMergeDlg>       CreateMailMergeDlg(
                                                weld::Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTableName,
                                                sal_Int32 nCommandType,
                                                const css::uno::Reference< css::sdbc::XConnection>& xConnection ) override;
    virtual VclPtr<AbstractMailMergeCreateFromDlg>     CreateMailMergeCreateFromDlg(weld::Window* pParent) override;
    virtual VclPtr<AbstractMailMergeFieldConnectionsDlg> CreateMailMergeFieldConnectionsDlg(weld::Window* pParent) override;
    virtual VclPtr<VclAbstractDialog>          CreateMultiTOXMarkDlg(weld::Window* pParent, SwTOXMgr &rTOXMgr) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateOutlineTabDialog(weld::Window* pParent, const SfxItemSet* pSwItemSet,
                                                SwWrtShell &) override;
    virtual VclPtr<AbstractNumBulletDialog>    CreateSvxNumBulletTabDialog(weld::Window* pParent,
                                                const SfxItemSet& rSwItemSet,
                                                SwWrtShell &) override;
    virtual VclPtr<AbstractMultiTOXTabDialog>  CreateMultiTOXTabDialog(
                                                weld::Widget* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX,
                                                bool bGlobal) override;
    virtual VclPtr<AbstractEditRegionDlg>      CreateEditRegionDlg(weld::Window* pParent, SwWrtShell& rWrtSh) override;
    virtual VclPtr<AbstractInsertSectionTabDialog>     CreateInsertSectionTabDialog(
        weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) override;
    virtual VclPtr<AbstractMarkFloatDlg>       CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo) override;
    virtual VclPtr<AbstractMarkFloatDlg>       CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo) override;
    virtual VclPtr<VclAbstractDialog>         CreateIndexMarkModalDlg(
                                                weld::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark ) override;

    virtual VclPtr<AbstractMailMergeWizard>    CreateMailMergeWizard(SwView& rView, std::shared_ptr<SwMailMergeConfigItem>& rConfigItem) override;

    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc() override;
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc() override;

    virtual VclPtr<AbstractCopyFieldDlg> CreateCopyFieldDlg(weld::Widget* pParent, const rtl::OUString& rFieldValue ) override;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) override;

    virtual void ExecuteMMResultSaveDialog(weld::Window* pParent) override;
    virtual void ExecuteMMResultPrintDialog(weld::Window* pParent) override;
    virtual void ExecuteMMResultEmailDialog(weld::Window* pParent) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
