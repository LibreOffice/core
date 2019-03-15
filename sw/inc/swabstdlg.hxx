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
#ifndef INCLUDED_SW_INC_SWABSTDLG_HXX
#define INCLUDED_SW_INC_SWABSTDLG_HXX

#include <rtl/ustring.hxx>
#include <sfx2/sfxdlg.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <boost/optional.hpp>
#include "dbmgr.hxx"
#include <cnttab.hxx>
#include "tblenum.hxx"

class SfxViewFrame;
class SfxBindings;
class SfxItemSet;
namespace vcl { class Window; }
class SfxStyleSheetBase;
class SwGlossaryHdl;
class SwField;

class SwAsciiOptions;
class SwDocShell;
class SvStream;
class SwWrtShell;
class SfxRequest;
class SwView;
class SwTableAutoFormat;
class SwTOXMgr;
class SwTOXDescription;
class SwTOXBase;
class SwSectionData;
struct SwDBData;
class Printer;
class SwLabItem;
class SwTableFUNC;
class SwChildWinWrapper;
struct SfxChildWinInfo;
class SwTOXMark;
struct SwDocStat;
struct SwInsertTableOptions;
enum class SwBorderModes;
enum class SwCharDlgMode;
enum class SfxStyleFamily;

namespace com{namespace sun{namespace star{
    namespace frame{
        class XModel;
    }
    namespace sdbcx{
    class XColumnsSupplier;
    }
    namespace sdbc{
    class XDataSource;
    class XConnection;
    class XResultSet;
    }
    namespace container { class XNameAccess; }
    namespace container { class XNamed; }
}}}


namespace sw { namespace mark { class IFieldmark; } }

typedef   void (*SwLabDlgMethod) (css::uno::Reference< css::frame::XModel> const & xModel, const SwLabItem& rItem);

typedef OUString    (*GlossaryGetCurrGroup)();
typedef void        (*GlossarySetActGroup)(const OUString& rNewGroup);

class AbstractGlossaryDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractGlossaryDlg() override = default;
public:
    virtual OUString        GetCurrGrpName() const = 0;
    virtual OUString        GetCurrShortName() const = 0;
};

class AbstractFieldInputDlg : public VclAbstractTerminatedDialog
{
protected:
    virtual ~AbstractFieldInputDlg() override = default;
public:
    virtual void          EndDialog(sal_Int32 nResult) override = 0;
    virtual bool          PrevButtonPressed() const = 0;
    virtual bool          NextButtonPressed() const = 0;
};

class AbstractInsFootNoteDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractInsFootNoteDlg() override = default;
public:
    virtual OUString    GetFontName() = 0;
    virtual bool        IsEndNote() = 0;
    virtual OUString    GetStr() = 0;
    //from class Window
    virtual void        SetHelpId( const OString& sHelpId ) = 0;
    virtual void        SetText( const OUString& rStr ) = 0;
};

class AbstractInsTableDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractInsTableDlg() override = default;
public:
    virtual void            GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableFlags, OUString& rTableAutoFormatName,
                                std::unique_ptr<SwTableAutoFormat>& prTAFormat ) = 0;
};

class AbstractJavaEditDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractJavaEditDialog() override = default;
public:
    virtual OUString            GetScriptText() const = 0;
    virtual OUString            GetScriptType() const = 0;
    virtual bool            IsUrl() const = 0;
    virtual bool            IsNew() const = 0;
    virtual bool                IsUpdate() const = 0;
};

class AbstractMailMergeDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractMailMergeDlg() override = default;
public:
    virtual DBManagerOptions GetMergeType() = 0;
    virtual const OUString& GetSaveFilter() const = 0;
    virtual const css::uno::Sequence< css::uno::Any > GetSelection() const = 0;
    virtual css::uno::Reference< css::sdbc::XResultSet> GetResultSet() const = 0;
    virtual bool IsSaveSingleDoc() const = 0;
    virtual bool IsGenerateFromDataBase() const = 0;
    virtual OUString GetColumnName() const = 0;
    virtual OUString GetTargetURL() const = 0;
};

class AbstractMailMergeCreateFromDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractMailMergeCreateFromDlg() override = default;
public:
    virtual bool    IsThisDocument() const = 0;
};

class AbstractMailMergeFieldConnectionsDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractMailMergeFieldConnectionsDlg() override = default;
public:
    virtual bool    IsUseExistingConnections() const = 0;
};

class AbstractMultiTOXTabDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractMultiTOXTabDialog() override = default;
public:
    virtual CurTOXType          GetCurrentTOXType() const = 0;
    virtual SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes) = 0;
    //from SfxTabDialog
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
};

class AbstractEditRegionDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractEditRegionDlg() override = default;
public:
    virtual void    SelectSection(const OUString& rSectionName) = 0;
};

class AbstractInsertSectionTabDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractInsertSectionTabDialog() override = default;
public:
    virtual void     SetSectionData(SwSectionData const& rSect) = 0;
};

class AbstractSwWordCountFloatDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSwWordCountFloatDlg() override = default;
public:
    virtual void        UpdateCounts() = 0;
    virtual void        SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat) = 0;
    virtual std::shared_ptr<SfxModelessDialogController> GetController() = 0;
};

class AbstractSwInsertAbstractDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSwInsertAbstractDlg() override = default;
public:
    virtual sal_uInt8   GetLevel() const = 0;
    virtual sal_uInt8   GetPara() const = 0;
};

class AbstractSwAsciiFilterDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSwAsciiFilterDlg() override = default;
public:
    virtual void FillOptions( SwAsciiOptions& rOptions ) = 0;

};

class AbstractSwBreakDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSwBreakDlg() override = default;
public:
    virtual OUString                        GetTemplateName() = 0;
    virtual sal_uInt16                      GetKind() = 0;
    virtual ::boost::optional<sal_uInt16>   GetPageNumber() = 0;

};

class AbstractSplitTableDialog : public VclAbstractDialog // add for
{
protected:
    virtual ~AbstractSplitTableDialog() override = default;
public:
    virtual SplitTable_HeadlineOption GetSplitMode() = 0;
};

class AbstractSwConvertTableDlg :  public VclAbstractDialog
{
protected:
    virtual ~AbstractSwConvertTableDlg() override = default;
public:
    virtual void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTableFlags,
                    SwTableAutoFormat const*& prTAFormat ) = 0;
};

class AbstractSwInsertDBColAutoPilot :  public VclAbstractDialog
{
protected:
    virtual ~AbstractSwInsertDBColAutoPilot() override = default;
public:
    virtual void DataToDoc( const css::uno::Sequence< css::uno::Any >& rSelection,
        css::uno::Reference< css::sdbc::XDataSource> rxSource,
        css::uno::Reference< css::sdbc::XConnection> xConnection,
        css::uno::Reference< css::sdbc::XResultSet > xResultSet) = 0;
};

class AbstractDropDownFieldDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractDropDownFieldDialog() override = default;
public:
    virtual bool          PrevButtonPressed() const = 0;
    virtual bool          NextButtonPressed() const = 0;
};

class AbstractSwLabDlg  : public SfxAbstractTabDialog
{
protected:
    virtual ~AbstractSwLabDlg() override = default;
public:
    virtual const OUString& GetBusinessCardStr() const = 0;
    virtual Printer *GetPrt() =0;
};

class AbstractSwSelGlossaryDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSwSelGlossaryDlg() override = default;
public:
    virtual void InsertGlos(const OUString &rRegion, const OUString &rGlosName) = 0;    // inline
    virtual sal_Int32 GetSelectedIdx() const = 0;  // inline
    virtual void SelectEntryPos(sal_Int32 nIdx) = 0;   // inline
};

class AbstractSwAutoFormatDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSwAutoFormatDlg() override = default;
public:
    virtual SwTableAutoFormat* FillAutoFormatOfIndex() const = 0;
};

class AbstractSwFieldDlg : public SfxAbstractTabDialog
{
protected:
    virtual ~AbstractSwFieldDlg() override = default;
public:
    virtual void                Start() = 0;  //this method from sfxtabdialog
    virtual void                Initialize(SfxChildWinInfo *pInfo) = 0;
    virtual void                ReInitDlg() = 0;
    virtual void                ActivateDatabasePage() = 0;
    virtual void                ShowReferencePage() = 0;
    virtual vcl::Window *            GetWindow() = 0; //this method is added for return a Window type pointer
};

class AbstractSwRenameXNamedDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSwRenameXNamedDlg() override = default;
public:
    virtual void    SetForbiddenChars( const OUString& rSet ) = 0;
    virtual void SetAlternativeAccess(
             css::uno::Reference< css::container::XNameAccess > & xSecond,
             css::uno::Reference< css::container::XNameAccess > & xThird ) = 0;
};

class AbstractSwModalRedlineAcceptDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSwModalRedlineAcceptDlg() override = default;
public:
    virtual void            AcceptAll( bool bAccept ) = 0;
};

class AbstractMarkFloatDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractMarkFloatDlg() override = default;
public:
    virtual void    ReInitDlg(SwWrtShell& rWrtShell) = 0;
    virtual std::shared_ptr<SfxModelessDialogController> GetController() = 0;
};

#define RET_LOAD_DOC            100
#define RET_TARGET_CREATED      103
#define RET_REMOVE_TARGET       104

class SwMailMergeConfigItem;

class AbstractMailMergeWizard : public VclAbstractDialog
{
protected:
    virtual ~AbstractMailMergeWizard() override = default;
public:
    virtual OUString            GetReloadDocument() const = 0;
    virtual void                ShowPage( sal_uInt16 nLevel ) = 0;
    virtual sal_uInt16          GetRestartPage() const = 0;
};

class SwAbstractDialogFactory
{
public:
    static SwAbstractDialogFactory*     Create();

    virtual VclPtr<SfxAbstractDialog> CreateNumFormatDialog(weld::Widget* pParent, const SfxItemSet& rAttr) = 0;
    virtual VclPtr<SfxAbstractDialog> CreateSwDropCapsDialog(weld::Window* pParent, const SfxItemSet& rSet) = 0;
    virtual VclPtr<SfxAbstractDialog> CreateSwBackgroundDialog(weld::Window* pParent, const SfxItemSet& rSet) = 0;

    virtual VclPtr<AbstractSwWordCountFloatDlg> CreateSwWordCountDialog(SfxBindings* pBindings,
        SfxChildWindow* pChild, weld::Window *pParent, SfxChildWinInfo* pInfo) = 0;

    virtual VclPtr<AbstractSwInsertAbstractDlg> CreateSwInsertAbstractDlg(weld::Window* pParent) = 0;
    virtual VclPtr<SfxAbstractDialog> CreateSwAddressAbstractDlg(weld::Window* pParent, const SfxItemSet& rSet) = 0;
    virtual VclPtr<AbstractSwAsciiFilterDlg>  CreateSwAsciiFilterDlg(weld::Window* pParent, SwDocShell& rDocSh,
                                                                SvStream* pStream) = 0;
    virtual VclPtr<VclAbstractDialog> CreateSwInsertBookmarkDlg(weld::Window *pParent, SwWrtShell &rSh, SfxRequest& rReq) = 0;

    virtual VclPtr<AbstractSwBreakDlg> CreateSwBreakDlg(weld::Window *pParent, SwWrtShell &rSh) = 0;
    virtual VclPtr<VclAbstractDialog> CreateSwChangeDBDlg(SwView& rVw) = 0;
    virtual VclPtr<SfxAbstractTabDialog>  CreateSwCharDlg(weld::Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
        SwCharDlgMode nDialogMode, const OUString* pFormatStr = nullptr) = 0;
    virtual VclPtr<AbstractSwConvertTableDlg> CreateSwConvertTableDlg(SwView& rView, bool bToTable) = 0;
    virtual VclPtr<VclAbstractDialog> CreateSwCaptionDialog(weld::Window *pParent, SwView &rV) = 0;

    virtual VclPtr<AbstractSwInsertDBColAutoPilot> CreateSwInsertDBColAutoPilot(SwView& rView,
        css::uno::Reference< css::sdbc::XDataSource> rxSource,
        css::uno::Reference<css::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData) = 0;
    virtual VclPtr<SfxAbstractTabDialog> CreateSwFootNoteOptionDlg(weld::Window *pParent, SwWrtShell &rSh) = 0;

    virtual VclPtr<AbstractDropDownFieldDialog> CreateDropDownFieldDialog(weld::Window* pParent, SwWrtShell &rSh,
        SwField* pField, bool bPrevButton, bool bNextButton) = 0;
    virtual VclPtr<VclAbstractDialog> CreateDropDownFormFieldDialog(weld::Window* pParent, sw::mark::IFieldmark* pDropDownField) = 0;
    virtual VclPtr<SfxAbstractTabDialog> CreateSwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert) = 0;

    virtual VclPtr<AbstractSwLabDlg> CreateSwLabDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel) = 0;

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod () =0;

    virtual VclPtr<SfxAbstractTabDialog> CreateSwParaDlg(weld::Window *pParent,
                                                         SwView& rVw,
                                                         const SfxItemSet& rCoreSet,
                                                         bool bDraw,
                                                         const OString& sDefPage = OString() ) = 0;

    virtual VclPtr<VclAbstractDialog> CreateSwAutoMarkDialog(weld::Window *pParent, SwWrtShell &rSh) = 0;

    virtual VclPtr<AbstractSwSelGlossaryDlg> CreateSwSelGlossaryDlg(weld::Window *pParent, const OUString &rShortName) = 0;

    virtual VclPtr<VclAbstractDialog> CreateSwSortingDialog(weld::Window *pParent, SwWrtShell &rSh) = 0;
    virtual VclPtr<VclAbstractDialog> CreateSwTableHeightDialog(weld::Window *pParent, SwWrtShell &rSh) = 0;
    virtual VclPtr<VclAbstractDialog> CreateSwColumnDialog(weld::Window *pParent, SwWrtShell &rSh) = 0;
    virtual VclPtr<AbstractSplitTableDialog> CreateSplitTableDialog(weld::Window* pParent, SwWrtShell &rSh) = 0;

    virtual VclPtr<AbstractSwAutoFormatDlg> CreateSwAutoFormatDlg(weld::Window* pParent, SwWrtShell* pShell,
                                                                  bool bSetAutoFormat = true,
                                                                  const SwTableAutoFormat* pSelFormat = nullptr ) = 0;
    virtual VclPtr<SfxAbstractDialog> CreateSwBorderDlg(weld::Window* pParent, SfxItemSet& rSet, SwBorderModes nType) = 0;
    virtual VclPtr<SfxAbstractDialog> CreateSwWrapDlg(weld::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh) = 0;

    virtual VclPtr<VclAbstractDialog> CreateSwTableWidthDlg(weld::Window *pParent, SwTableFUNC &rFnc) = 0;
    virtual VclPtr<SfxAbstractTabDialog> CreateSwTableTabDlg(weld::Window* pParent,
        const SfxItemSet* pItemSet, SwWrtShell* pSh) = 0;

    virtual VclPtr<AbstractSwFieldDlg> CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, vcl::Window *pParent) = 0;
    virtual VclPtr<SfxAbstractDialog>   CreateSwFieldEditDlg ( SwView& rVw ) = 0;
    virtual VclPtr<AbstractSwRenameXNamedDlg> CreateSwRenameXNamedDlg(weld::Window* pParent,
        css::uno::Reference< css::container::XNamed > & xNamed,
        css::uno::Reference< css::container::XNameAccess > & xNameAccess) = 0;
    virtual VclPtr<AbstractSwModalRedlineAcceptDlg> CreateSwModalRedlineAcceptDlg(vcl::Window *pParent) = 0;

    virtual VclPtr<VclAbstractDialog>          CreateTableMergeDialog(weld::Window* pParent, bool& rWithPrev) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateFrameTabDialog(const OUString &rDialogType,
                                                SfxViewFrame *pFrame, weld::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool bNewFrame = true,
                                                const OString& sDefPage = OString()) = 0;
    /// @param nSlot
    /// Identifies optional Slot by which the creation of the Template (Style) dialog is triggered.
    /// Currently used, if nRegion == SfxStyleFamily::Page in order to activate certain dialog pane
    virtual VclPtr<SfxAbstractApplyTabDialog>  CreateTemplateDialog(weld::Window* pParent,
                                                SfxStyleSheetBase&  rBase,
                                                SfxStyleFamily      nRegion,
                                                const OString&      sPage,
                                                SwWrtShell*         pActShell,
                                                bool                bNew) = 0;
    virtual VclPtr<AbstractGlossaryDlg>        CreateGlossaryDlg(SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell) = 0;
    virtual VclPtr<AbstractFieldInputDlg>        CreateFieldInputDlg(weld::Window *pParent,
        SwWrtShell &rSh, SwField* pField, bool bPrevButton, bool bNextButton) = 0;
    virtual VclPtr<AbstractInsFootNoteDlg>     CreateInsFootNoteDlg(weld::Window * pParent,
        SwWrtShell &rSh, bool bEd = false) = 0;
    virtual VclPtr<VclAbstractDialog>          CreateTitlePageDlg(weld::Window* pParent) = 0;
    virtual VclPtr<VclAbstractDialog>         CreateVclSwViewDialog(SwView& rView) = 0;
    virtual VclPtr<AbstractInsTableDlg>        CreateInsTableDlg(SwView& rView) = 0;
    virtual VclPtr<AbstractJavaEditDialog>     CreateJavaEditDialog(weld::Window* pParent,
        SwWrtShell* pWrtSh) = 0;
    virtual VclPtr<AbstractMailMergeDlg>       CreateMailMergeDlg(
                                                vcl::Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTableName,
                                                sal_Int32 nCommandType,
                                                const css::uno::Reference< css::sdbc::XConnection>& xConnection ) = 0;
    virtual VclPtr<AbstractMailMergeCreateFromDlg>   CreateMailMergeCreateFromDlg(weld::Window* pParent) = 0;
    virtual VclPtr<AbstractMailMergeFieldConnectionsDlg> CreateMailMergeFieldConnectionsDlg(weld::Window* pParent) = 0;
    virtual VclPtr<VclAbstractDialog>          CreateMultiTOXMarkDlg(weld::Window* pParent, SwTOXMgr &rTOXMgr) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateOutlineTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSvxNumBulletTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &) = 0;
    virtual VclPtr<AbstractMultiTOXTabDialog>  CreateMultiTOXTabDialog(
                                                vcl::Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX,
                                                bool bGlobal) = 0;
    virtual VclPtr<AbstractEditRegionDlg>      CreateEditRegionDlg(weld::Window* pParent, SwWrtShell& rWrtSh) = 0;
    virtual VclPtr<AbstractInsertSectionTabDialog> CreateInsertSectionTabDialog(weld::Window* pParent,
                                                       const SfxItemSet& rSet, SwWrtShell& rSh) = 0;
    virtual VclPtr<AbstractMarkFloatDlg>       CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo) = 0;
    virtual VclPtr<AbstractMarkFloatDlg>       CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo) = 0;
    virtual VclPtr<VclAbstractDialog>         CreateIndexMarkModalDlg(
                                                weld::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark) = 0;

    virtual VclPtr<AbstractMailMergeWizard>    CreateMailMergeWizard(SwView& rView, std::shared_ptr<SwMailMergeConfigItem>& rConfigItem) = 0;

    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc() = 0;
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc() = 0;

    // for tabpage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) = 0;

    virtual void ExecuteMMResultSaveDialog(weld::Window* pParent) = 0;
    virtual void ExecuteMMResultPrintDialog(weld::Window* pParent) = 0;
    virtual void ExecuteMMResultEmailDialog(weld::Window* pParent) = 0;

protected:
    ~SwAbstractDialogFactory() {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
