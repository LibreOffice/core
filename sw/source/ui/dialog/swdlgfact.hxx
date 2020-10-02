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
#include <break.hxx>
#include <cnttab.hxx>
#include <colwd.hxx>
#include <convert.hxx>
#include <DateFormFieldDialog.hxx>
#include <dbinsdlg.hxx>
#include <DropDownFieldDialog.hxx>
#include <DropDownFormFieldDialog.hxx>
#include <fldtdlg.hxx>
#include <glossary.hxx>
#include <inpdlg.hxx>
#include <insfnote.hxx>
#include <instable.hxx>
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
#include <wordcountdialog.hxx>

class SwInsertAbstractDlg;
class SwAsciiFilterDlg;
class SwBreakDlg;
class SwMultiTOXMarkDlg;
class SwSortDlg;
class SwTableHeightDlg;
class SwTableWidthDlg;
class SwMergeTableDlg;
class SfxTabDialog;
class SwConvertTableDlg;
class SwInsertDBColAutoPilot;
class SwLabDlg;
class SwSelGlossaryDlg;
class SwAutoFormatDlg;
class SwFieldDlg;
class SwRenameXNamedDlg;
class SwModalRedlineAcceptDlg;
class SwTOXMark;
class SwSplitTableDlg;

#include <itabenum.hxx>
#include <optional>
#include <o3tl/deleter.hxx>

namespace sw
{
class DropDownFieldDialog;
class DropDownFormFieldDialog;
class DateFormFieldDialog;
}

class SwWordCountFloatDlg;
class AbstractSwWordCountFloatDlg_Impl : public AbstractSwWordCountFloatDlg
{
    std::shared_ptr<SwWordCountFloatDlg> m_xDlg;
public:
    explicit AbstractSwWordCountFloatDlg_Impl(std::shared_ptr<SwWordCountFloatDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void  UpdateCounts() override;
    virtual void  SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat) override;
    virtual std::shared_ptr<SfxDialogController> GetController() override;
};

class AbstractSwInsertAbstractDlg_Impl : public AbstractSwInsertAbstractDlg
{
    std::unique_ptr<SwInsertAbstractDlg> m_xDlg;
public:
    explicit AbstractSwInsertAbstractDlg_Impl(std::unique_ptr<SwInsertAbstractDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual sal_uInt8   GetLevel() const override ;
    virtual sal_uInt8   GetPara() const override ;
};

class SwAbstractSfxController_Impl : public SfxAbstractDialog
{
    std::unique_ptr<SfxSingleTabDialogController> m_xDlg;
public:
    explicit SwAbstractSfxController_Impl(std::unique_ptr<SfxSingleTabDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual const SfxItemSet* GetOutputItemSet() const override;
    virtual void SetText(const OUString& rStr) override;
};

class AbstractSwAsciiFilterDlg_Impl : public AbstractSwAsciiFilterDlg
{
    std::unique_ptr<SwAsciiFilterDlg> m_xDlg;
public:
    explicit AbstractSwAsciiFilterDlg_Impl(std::unique_ptr<SwAsciiFilterDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void FillOptions( SwAsciiOptions& rOptions ) override;
};

class AbstractGenericDialog_Impl : public VclAbstractDialog
{
    std::shared_ptr<weld::GenericDialogController> m_xDlg;
public:
    explicit AbstractGenericDialog_Impl(std::shared_ptr<weld::GenericDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(AsyncContext &rCtx) override;
};

class AbstractSwSortDlg_Impl : public VclAbstractDialog
{
    std::unique_ptr<SwSortDlg> m_xDlg;
public:
    explicit AbstractSwSortDlg_Impl(std::unique_ptr<SwSortDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class AbstractMultiTOXMarkDlg_Impl : public VclAbstractDialog
{
    std::unique_ptr<SwMultiTOXMarkDlg> m_xDlg;
public:
    explicit AbstractMultiTOXMarkDlg_Impl(std::unique_ptr<SwMultiTOXMarkDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class AbstractSwBreakDlg_Impl : public AbstractSwBreakDlg
{
    std::shared_ptr<weld::DialogController> m_xDlg;
public:
    explicit AbstractSwBreakDlg_Impl(std::shared_ptr<weld::DialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual OUString                        GetTemplateName() override;
    virtual sal_uInt16                      GetKind() override;
    virtual ::std::optional<sal_uInt16>   GetPageNumber() override;

    virtual std::shared_ptr<weld::DialogController> getDialogController() override { return m_xDlg; }
};

class AbstractSwTableWidthDlg_Impl : public VclAbstractDialog
{
    std::unique_ptr<SwTableWidthDlg> m_xDlg;
public:
    explicit AbstractSwTableWidthDlg_Impl(std::unique_ptr<SwTableWidthDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class AbstractSwTableHeightDlg_Impl : public VclAbstractDialog
{
    std::unique_ptr<SwTableHeightDlg> m_xDlg;
public:
    explicit AbstractSwTableHeightDlg_Impl(std::unique_ptr<SwTableHeightDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class AbstractSwMergeTableDlg_Impl : public VclAbstractDialog
{
    std::unique_ptr<SwMergeTableDlg> m_xDlg;
public:
    explicit AbstractSwMergeTableDlg_Impl(std::unique_ptr<SwMergeTableDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class AbstractSplitTableDialog_Impl : public AbstractSplitTableDialog // add for
{
    std::unique_ptr<SwSplitTableDlg> m_xDlg;
public:
    explicit AbstractSplitTableDialog_Impl(std::unique_ptr<SwSplitTableDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual SplitTable_HeadlineOption GetSplitMode() override;
};

class AbstractTabController_Impl : virtual public SfxAbstractTabDialog
{
protected:
    std::shared_ptr<SfxTabDialogController> m_xDlg;
public:
    explicit AbstractTabController_Impl(std::shared_ptr<SfxTabDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool  StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*   GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
};

class AbstractApplyTabController_Impl : public AbstractTabController_Impl, virtual public SfxAbstractApplyTabDialog
{
public:
    explicit AbstractApplyTabController_Impl(std::shared_ptr<SfxTabDialogController> p)
        : AbstractTabController_Impl(std::move(p))
    {
    }
    DECL_LINK(ApplyHdl, weld::Button&, void);
private:
    Link<LinkParamNone*,void> m_aHandler;
    virtual void                SetApplyHdl( const Link<LinkParamNone*,void>& rLink ) override;
};

class AbstractSwConvertTableDlg_Impl :  public AbstractSwConvertTableDlg
{
    std::unique_ptr<SwConvertTableDlg> m_xDlg;
public:
    explicit AbstractSwConvertTableDlg_Impl(std::unique_ptr<SwConvertTableDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void GetValues( sal_Unicode& rDelim,SwInsertTableOptions& rInsTableFlags,
                    SwTableAutoFormat const*& prTAFormat) override;
};

class AbstractSwInsertDBColAutoPilot_Impl :  public AbstractSwInsertDBColAutoPilot
{
    std::unique_ptr<SwInsertDBColAutoPilot> m_xDlg;
public:
    explicit AbstractSwInsertDBColAutoPilot_Impl(std::unique_ptr<SwInsertDBColAutoPilot> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void DataToDoc( const css::uno::Sequence< css::uno::Any >& rSelection,
        css::uno::Reference< css::sdbc::XDataSource> rxSource,
        css::uno::Reference< css::sdbc::XConnection> xConnection,
        css::uno::Reference< css::sdbc::XResultSet > xResultSet) override;
};

class AbstractDropDownFieldDialog_Impl : public AbstractDropDownFieldDialog
{
    std::unique_ptr<sw::DropDownFieldDialog> m_xDlg;
public:
    explicit AbstractDropDownFieldDialog_Impl(std::unique_ptr<sw::DropDownFieldDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool          PrevButtonPressed() const override;
    virtual bool          NextButtonPressed() const override;
};

class AbstractDropDownFormFieldDialog_Impl : public VclAbstractDialog
{
    std::unique_ptr<sw::DropDownFormFieldDialog> m_xDlg;
public:
    explicit AbstractDropDownFormFieldDialog_Impl(std::unique_ptr<sw::DropDownFormFieldDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class AbstractDateFormFieldDialog_Impl : public VclAbstractDialog
{
    std::unique_ptr<sw::DateFormFieldDialog> m_xDlg;
public:
    explicit AbstractDateFormFieldDialog_Impl(std::unique_ptr<sw::DateFormFieldDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class AbstractSwLabDlg_Impl  : public AbstractSwLabDlg
{
    std::unique_ptr<SwLabDlg> m_xDlg;
public:
    explicit AbstractSwLabDlg_Impl(std::unique_ptr<SwLabDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual const OUString& GetBusinessCardStr() const override;
    virtual Printer *GetPrt() override;
};

class AbstractSwSelGlossaryDlg_Impl : public AbstractSwSelGlossaryDlg
{
    std::unique_ptr<SwSelGlossaryDlg> m_xDlg;
public:
    explicit AbstractSwSelGlossaryDlg_Impl(std::unique_ptr<SwSelGlossaryDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void InsertGlos(const OUString &rRegion, const OUString &rGlosName) override;    // inline
    virtual sal_Int32 GetSelectedIdx() const override;  // inline
    virtual void SelectEntryPos(sal_Int32 nIdx) override;   // inline
};

class AbstractSwAutoFormatDlg_Impl : public AbstractSwAutoFormatDlg
{
    std::unique_ptr<SwAutoFormatDlg, o3tl::default_delete<SwAutoFormatDlg>> m_xDlg;
public:
    explicit AbstractSwAutoFormatDlg_Impl(std::unique_ptr<SwAutoFormatDlg, o3tl::default_delete<SwAutoFormatDlg>> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual std::unique_ptr<SwTableAutoFormat> FillAutoFormatOfIndex() const override;
};

class AbstractSwFieldDlg_Impl : public AbstractSwFieldDlg
{
    std::shared_ptr<SwFieldDlg> m_xDlg;
public:
    explicit AbstractSwFieldDlg_Impl(std::shared_ptr<SwFieldDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*   GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void                SetText( const OUString& rStr ) override;
    virtual void                ShowReferencePage() override;
    virtual void                Initialize(SfxChildWinInfo *pInfo) override;
    virtual void                ReInitDlg() override;
    virtual void                ActivateDatabasePage() override;
    virtual std::shared_ptr<SfxDialogController> GetController() override;
};

class AbstractSwRenameXNamedDlg_Impl : public AbstractSwRenameXNamedDlg
{
    std::unique_ptr<SwRenameXNamedDlg> m_xDlg;
public:
    explicit AbstractSwRenameXNamedDlg_Impl(std::unique_ptr<SwRenameXNamedDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void SetForbiddenChars( const OUString& rSet ) override;
    virtual void SetAlternativeAccess(
             css::uno::Reference< css::container::XNameAccess > & xSecond,
             css::uno::Reference< css::container::XNameAccess > & xThird ) override;
};

class AbstractSwModalRedlineAcceptDlg_Impl : public AbstractSwModalRedlineAcceptDlg
{
    std::unique_ptr<SwModalRedlineAcceptDlg> m_xDlg;
public:
    explicit AbstractSwModalRedlineAcceptDlg_Impl(std::unique_ptr<SwModalRedlineAcceptDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void AcceptAll(bool bAccept) override;
};

class SwGlossaryDlg;
class AbstractGlossaryDlg_Impl : public AbstractGlossaryDlg
{
    std::unique_ptr<SwGlossaryDlg> m_xDlg;
public:
    explicit AbstractGlossaryDlg_Impl(std::unique_ptr<SwGlossaryDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual OUString        GetCurrGrpName() const override;
    virtual OUString        GetCurrShortName() const override;
};

class SwFieldInputDlg;
class AbstractFieldInputDlg_Impl : public AbstractFieldInputDlg
{
    std::unique_ptr<SwFieldInputDlg> m_xDlg;
public:
    explicit AbstractFieldInputDlg_Impl(std::unique_ptr<SwFieldInputDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void          EndDialog(sal_Int32) override;
    virtual bool          PrevButtonPressed() const override;
    virtual bool          NextButtonPressed() const override;
};

class SwInsFootNoteDlg;
class AbstractInsFootNoteDlg_Impl : public AbstractInsFootNoteDlg
{
    std::unique_ptr<SwInsFootNoteDlg> m_xDlg;
public:
    explicit AbstractInsFootNoteDlg_Impl(std::unique_ptr<SwInsFootNoteDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual OUString        GetFontName() override;
    virtual bool            IsEndNote() override;
    virtual OUString        GetStr() override;
    //from class Window
    virtual void    SetHelpId( const OString& sHelpId ) override;
    virtual void    SetText( const OUString& rStr ) override;
};

class SwInsTableDlg;
class AbstractInsTableDlg_Impl : public AbstractInsTableDlg
{
protected:
    std::shared_ptr<weld::DialogController> m_xDlg;
public:
    explicit AbstractInsTableDlg_Impl(std::shared_ptr<weld::DialogController> p)
        : m_xDlg(p)
    {
    }
    virtual void  GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                             SwInsertTableOptions& rInsTableFlags, OUString& rTableAutoFormatName,
                             std::unique_ptr<SwTableAutoFormat>& prTAFormat ) override;
    virtual std::shared_ptr<weld::DialogController> getDialogController() override { return m_xDlg; }
};

class SwJavaEditDialog;
class AbstractJavaEditDialog_Impl : public AbstractJavaEditDialog
{
    std::unique_ptr<SwJavaEditDialog> m_xDlg;
public:
    explicit AbstractJavaEditDialog_Impl(std::unique_ptr<SwJavaEditDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual OUString            GetScriptText() const override;
    virtual OUString            GetScriptType() const override;
    virtual bool                IsUrl() const override;
    virtual bool                IsNew() const override;
    virtual bool                IsUpdate() const override;
};

class SwMailMergeDlg;
class AbstractMailMergeDlg_Impl : public AbstractMailMergeDlg
{
    std::unique_ptr<SwMailMergeDlg> m_xDlg;
public:
    explicit AbstractMailMergeDlg_Impl(std::unique_ptr<SwMailMergeDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual DBManagerOptions GetMergeType() override ;
    virtual const OUString& GetSaveFilter() const override;
    virtual css::uno::Sequence< css::uno::Any > GetSelection() const override ;
    virtual css::uno::Reference< css::sdbc::XResultSet> GetResultSet() const override;
    virtual bool IsSaveSingleDoc() const override;
    virtual bool IsGenerateFromDataBase() const override;
    virtual bool IsFileEncryptedFromDataBase() const override;
    virtual OUString GetColumnName() const override;
    virtual OUString GetPasswordColumnName() const override;
    virtual OUString GetTargetURL() const override;
};

class SwMailMergeCreateFromDlg;
class AbstractMailMergeCreateFromDlg_Impl : public AbstractMailMergeCreateFromDlg
{
    std::unique_ptr<SwMailMergeCreateFromDlg> m_xDlg;
public:
    explicit AbstractMailMergeCreateFromDlg_Impl(std::unique_ptr<SwMailMergeCreateFromDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool    IsThisDocument() const override ;
};

class SwMailMergeFieldConnectionsDlg;
class AbstractMailMergeFieldConnectionsDlg_Impl : public AbstractMailMergeFieldConnectionsDlg
{
    std::unique_ptr<SwMailMergeFieldConnectionsDlg> m_xDlg;
public:
    explicit AbstractMailMergeFieldConnectionsDlg_Impl(std::unique_ptr<SwMailMergeFieldConnectionsDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool IsUseExistingConnections() const override ;
};

class SwMultiTOXTabDialog;
class AbstractMultiTOXTabDialog_Impl : public AbstractMultiTOXTabDialog
{
protected:
    std::shared_ptr<SwMultiTOXTabDialog> m_xDlg;
public:
    explicit AbstractMultiTOXTabDialog_Impl(std::shared_ptr<SwMultiTOXTabDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx) override;
    virtual CurTOXType          GetCurrentTOXType() const override ;
    virtual SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes) override;
    //from SfxTabDialog
    virtual const SfxItemSet*   GetOutputItemSet() const override;
};

class SwEditRegionDlg;
class AbstractEditRegionDlg_Impl : public AbstractEditRegionDlg
{
    std::shared_ptr<SwEditRegionDlg> m_xDlg;
public:
    explicit AbstractEditRegionDlg_Impl(std::shared_ptr<SwEditRegionDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void    SelectSection(const OUString& rSectionName) override;
};

class SwInsertSectionTabDialog;
class AbstractInsertSectionTabDialog_Impl : public AbstractInsertSectionTabDialog
{
    std::shared_ptr<SwInsertSectionTabDialog> m_xDlg;
public:
    explicit AbstractInsertSectionTabDialog_Impl(std::shared_ptr<SwInsertSectionTabDialog> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void SetSectionData(SwSectionData const& rSect) override;
};

class SwIndexMarkFloatDlg;
class AbstractIndexMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    std::shared_ptr<SwIndexMarkFloatDlg> m_xDlg;
public:
    explicit AbstractIndexMarkFloatDlg_Impl(std::shared_ptr<SwIndexMarkFloatDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void ReInitDlg(SwWrtShell& rWrtShell) override;
    virtual std::shared_ptr<SfxDialogController> GetController() override;
};

class SwAuthMarkFloatDlg;
class AbstractAuthMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    std::shared_ptr<SwAuthMarkFloatDlg> m_xDlg;
public:
    explicit AbstractAuthMarkFloatDlg_Impl(std::shared_ptr<SwAuthMarkFloatDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void ReInitDlg(SwWrtShell& rWrtShell) override;
    virtual std::shared_ptr<SfxDialogController> GetController() override;
};

class SwMailMergeWizard;
class AbstractMailMergeWizard_Impl : public AbstractMailMergeWizard
{
    std::shared_ptr<SwMailMergeWizard> m_xDlg;

public:
    explicit AbstractMailMergeWizard_Impl(std::shared_ptr<SwMailMergeWizard> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual         ~AbstractMailMergeWizard_Impl() override;
    virtual bool    StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx) override;
    virtual short   Execute() override;

    virtual OUString            GetReloadDocument() const override;
    virtual void                ShowPage( sal_uInt16 nLevel ) override;
    virtual sal_uInt16          GetRestartPage() const override;
};

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
    virtual VclPtr<VclAbstractDialog> CreateSwInsertBookmarkDlg(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual std::shared_ptr<AbstractSwBreakDlg> CreateSwBreakDlg(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<VclAbstractDialog> CreateSwChangeDBDlg(SwView& rVw) override;
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
    virtual VclPtr<VclAbstractDialog> CreateDropDownFormFieldDialog(weld::Widget* pParent, sw::mark::IFieldmark* pDropDownField) override;
    virtual VclPtr<VclAbstractDialog> CreateDateFormFieldDialog(weld::Widget* pParent, sw::mark::IDateFieldmark* pDateField, SwDoc& rDoc) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateSwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert) override;
    virtual VclPtr<AbstractSwLabDlg> CreateSwLabDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel) override;

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod () override;
    virtual VclPtr<SfxAbstractTabDialog> CreateSwParaDlg(weld::Window *pParent,
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    bool bDraw,
                                                    const OString& sDefPage = OString()) override;

    virtual VclPtr<VclAbstractDialog> CreateSwAutoMarkDialog(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<AbstractSwSelGlossaryDlg> CreateSwSelGlossaryDlg(weld::Window *pParent, const OUString &rShortName) override;
    virtual VclPtr<VclAbstractDialog> CreateSwSortingDialog(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<VclAbstractDialog> CreateSwTableHeightDialog(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<VclAbstractDialog> CreateSwColumnDialog(weld::Window *pParent, SwWrtShell &rSh) override;
    virtual VclPtr<AbstractSplitTableDialog> CreateSplitTableDialog(weld::Window* pParent, SwWrtShell &rSh) override;

    virtual VclPtr<AbstractSwAutoFormatDlg> CreateSwAutoFormatDlg(weld::Window* pParent, SwWrtShell* pShell,
                                                                  bool bSetAutoFormat = true,
                                                                  const SwTableAutoFormat* pSelFormat = nullptr) override;
    virtual VclPtr<SfxAbstractDialog> CreateSwBorderDlg(weld::Window* pParent, SfxItemSet& rSet, SwBorderModes nType) override;

    virtual VclPtr<SfxAbstractDialog> CreateSwWrapDlg(weld::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh) override;
    virtual VclPtr<VclAbstractDialog> CreateSwTableWidthDlg(weld::Window *pParent, SwTableFUNC &rFnc) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateSwTableTabDlg(weld::Window* pParent,
        const SfxItemSet* pItemSet, SwWrtShell* pSh) override;
    virtual VclPtr<AbstractSwFieldDlg> CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, weld::Window *pParent) override;
    virtual VclPtr<SfxAbstractDialog>   CreateSwFieldEditDlg ( SwView& rVw ) override;
    virtual VclPtr<AbstractSwRenameXNamedDlg> CreateSwRenameXNamedDlg(weld::Window* pParent,
        css::uno::Reference< css::container::XNamed > & xNamed,
        css::uno::Reference< css::container::XNameAccess > & xNameAccess) override;
    virtual VclPtr<AbstractSwModalRedlineAcceptDlg> CreateSwModalRedlineAcceptDlg(weld::Window *pParent) override;

    virtual VclPtr<VclAbstractDialog>          CreateTableMergeDialog(weld::Window* pParent, bool& rWithPrev) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateFrameTabDialog( const OUString &rDialogType,
                                                SfxViewFrame *pFrame, weld::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool bNewFrame  = true,
                                                const OString& sDefPage = OString()) override;
    virtual VclPtr<SfxAbstractApplyTabDialog>  CreateTemplateDialog(
                                                weld::Window *pParent,
                                                SfxStyleSheetBase&  rBase,
                                                SfxStyleFamily      nRegion,
                                                const OString&      sPage,
                                                SwWrtShell*         pActShell,
                                                bool                bNew) override;
    virtual VclPtr<AbstractGlossaryDlg>        CreateGlossaryDlg(SfxViewFrame* pViewFrame,
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
    virtual VclPtr<SfxAbstractTabDialog>       CreateSvxNumBulletTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &) override;
    virtual VclPtr<AbstractMultiTOXTabDialog>  CreateMultiTOXTabDialog(
                                                weld::Window* pParent, const SfxItemSet& rSet,
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

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) override;

    virtual void ExecuteMMResultSaveDialog(weld::Window* pParent) override;
    virtual void ExecuteMMResultPrintDialog(weld::Window* pParent) override;
    virtual void ExecuteMMResultEmailDialog(weld::Window* pParent) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
