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
#include <tools/solar.h>
#include <sfx2/sfxdlg.hxx>
#include <svx/svxdlg.hxx>
#include <vcl/syswin.hxx>
#include <globals.hrc>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include "itabenum.hxx"
#include <boost/optional.hpp>
#include "dbmgr.hxx"
#include <cnttab.hxx>

class SfxViewFrame;
class SfxBindings;
class SfxItemSet;
class ResId;
namespace vcl { class Window; }
class SfxItemPool;
class SfxStyleSheetBase;
class SwGlossaryHdl;
class SwField;

class SwLabFormatPage;
class SwLabRec;
class SwAsciiOptions;
class SwDocShell;
class SvStream;
class SwWrtShell;
class SfxRequest;
class SwView;
class SwTableAutoFormat;
class SwTOXMgr;
class SwForm;
struct CurTOXType;
class SwTOXDescription;
class SwTOXBase;
class SwSectionData;
struct SwDBData;
class SwField;
class Printer;
class SwLabItem;
class SwDBManager;
class SwTableFUNC;
class SwChildWinWrapper;
struct SfxChildWinInfo;
class SwTOXMark;
struct SwDocStat;
enum class SwBorderModes;
enum class SwCharDlgMode;
enum class SfxStyleFamily;

namespace com{namespace sun{namespace star{
    namespace frame{
        class XFrame;
    }
    namespace sdbcx{
    class XColumnsSupplier;
    }
    namespace sdbc{
    class XDataSource;
    class XConnection;
    class XResultSet;
    }
}}}

typedef   void (*SwLabDlgMethod) (css::uno::Reference< css::frame::XModel>& xModel,   const SwLabItem& rItem);

typedef OUString    (*GlossaryGetCurrGroup)();
typedef void        (*GlossarySetActGroup)(const OUString& rNewGroup);

class AbstractGlossaryDlg : public VclAbstractDialog
{
public:
    virtual OUString        GetCurrGrpName() const = 0;
    virtual OUString        GetCurrShortName() const = 0;
};

class AbstractFieldInputDlg : public VclAbstractTerminatedDialog
{
public:
    //from class SalFrame
    virtual void         SetWindowState( const OString & rStr ) = 0;
    virtual OString      GetWindowState() const = 0;
    virtual void         EndDialog(long nResult) override = 0;
};

class AbstractInsFootNoteDlg : public VclAbstractDialog
{
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
public:
    virtual void            GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableFlags, OUString& rTableAutoFormatName,
                                SwTableAutoFormat *& prTAFormat ) = 0;
};

class AbstractJavaEditDialog : public VclAbstractDialog
{
public:
    virtual OUString            GetScriptText() const = 0;
    virtual OUString            GetScriptType() const = 0;
    virtual bool            IsUrl() const = 0;
    virtual bool            IsNew() const = 0;
    virtual bool                IsUpdate() const = 0;
};

class AbstractMailMergeDlg : public VclAbstractDialog
{
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
public:
    virtual bool    IsThisDocument() const = 0;
};

class AbstractMailMergeFieldConnectionsDlg : public VclAbstractDialog
{
public:
    virtual bool    IsUseExistingConnections() const = 0;
};

class AbstractMultiTOXTabDialog : public VclAbstractDialog
{
public:
    virtual CurTOXType          GetCurrentTOXType() const = 0;
    virtual SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes) = 0;
    //from SfxTabDialog
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
};

class AbstractEditRegionDlg : public VclAbstractDialog
{
public:
    virtual void    SelectSection(const OUString& rSectionName) = 0;
};
class AbstractInsertSectionTabDialog : public VclAbstractDialog
{
public:
    virtual void     SetSectionData(SwSectionData const& rSect) = 0;
};

class AbstractSwWordCountFloatDlg : public VclAbstractDialog
{
public:
    virtual void        UpdateCounts() = 0;
    virtual void        SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat) = 0;
    virtual vcl::Window *    GetWindow() = 0; //this method is added for return a Window type pointer
};

class AbstractSwInsertAbstractDlg : public VclAbstractDialog
{
public:
    virtual sal_uInt8   GetLevel() const = 0;
    virtual sal_uInt8   GetPara() const = 0;
};

class AbstractSwAsciiFilterDlg : public VclAbstractDialog
{
public:
    virtual void FillOptions( SwAsciiOptions& rOptions ) = 0;

};

class AbstractSwBreakDlg : public VclAbstractDialog
{
public:
    virtual OUString                        GetTemplateName() = 0;
    virtual sal_uInt16                      GetKind() = 0;
    virtual ::boost::optional<sal_uInt16>   GetPageNumber() = 0;

};

class AbstractSplitTableDialog : public VclAbstractDialog // add for
{
public:
    virtual sal_uInt16 GetSplitMode() = 0;
};

class AbstractSwConvertTableDlg :  public VclAbstractDialog
{
public:
    virtual void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTableFlags,
                    SwTableAutoFormat const*& prTAFormat ) = 0;
};

class AbstractSwInsertDBColAutoPilot :  public VclAbstractDialog
{
public:

    virtual void DataToDoc( const css::uno::Sequence< css::uno::Any >& rSelection,
        css::uno::Reference< css::sdbc::XDataSource> rxSource,
        css::uno::Reference< css::sdbc::XConnection> xConnection,
        css::uno::Reference< css::sdbc::XResultSet > xResultSet) = 0;
};

class AbstractDropDownFieldDialog : public VclAbstractDialog
{
public:
     virtual OString GetWindowState() const = 0; //this method inherit from SystemWindow
     virtual void    SetWindowState( const OString & rStr ) = 0; //this method inherit from SystemWindow
};

class AbstractSwLabDlg  : public SfxAbstractTabDialog
{
public:
     virtual const OUString& GetBusinessCardStr() const = 0;
     virtual Printer *GetPrt() =0;
};

class AbstractSwSelGlossaryDlg : public VclAbstractDialog
{
public:
    virtual void InsertGlos(const OUString &rRegion, const OUString &rGlosName) = 0;    // inline
    virtual sal_Int32 GetSelectedIdx() const = 0;  // inline
    virtual void SelectEntryPos(sal_Int32 nIdx) = 0;   // inline
};

class AbstractSwAutoFormatDlg : public VclAbstractDialog
{
public:
    virtual void FillAutoFormatOfIndex( SwTableAutoFormat*& rToFill ) const = 0;
};

class AbstractSwFieldDlg : public SfxAbstractTabDialog
{
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
public:
    virtual void    SetForbiddenChars( const OUString& rSet ) = 0;
    virtual void SetAlternativeAccess(
             css::uno::Reference< css::container::XNameAccess > & xSecond,
             css::uno::Reference< css::container::XNameAccess > & xThird ) = 0;
};

class AbstractSwModalRedlineAcceptDlg : public VclAbstractDialog
{
public:
    virtual void            AcceptAll( bool bAccept ) = 0;
};

class AbstractMarkFloatDlg : public VclAbstractDialog
{
public:
    virtual void    ReInitDlg(SwWrtShell& rWrtShell) = 0;
    virtual vcl::Window *    GetWindow() = 0; //this method is added for return a Window type pointer
};

#define RET_LOAD_DOC            100
#define RET_TARGET_CREATED      103
#define RET_REMOVE_TARGET       104

class SwView;
class SwMailMergeConfigItem;

class AbstractMailMergeWizard : public VclAbstractDialog2
{
public:
    virtual OUString            GetReloadDocument() const = 0;
    virtual bool            ShowPage( sal_uInt16 nLevel ) = 0;
    virtual sal_uInt16          GetRestartPage() const = 0;
};

class SwAbstractDialogFactory
{
public:
    static SwAbstractDialogFactory*     Create();

    virtual SfxAbstractDialog*          CreateSfxDialog( vcl::Window* pParent,
                                                         const SfxItemSet& rAttr,
                                                         const css::uno::Reference< css::frame::XFrame >& _rxFrame,
                                                         sal_uInt32 nResId
                                                        ) = 0;
    virtual AbstractSwWordCountFloatDlg* CreateSwWordCountDialog(SfxBindings* pBindings,
        SfxChildWindow* pChild, vcl::Window *pParent, SfxChildWinInfo* pInfo) = 0;

    virtual AbstractSwInsertAbstractDlg * CreateSwInsertAbstractDlg() = 0;
    virtual AbstractSwAsciiFilterDlg*  CreateSwAsciiFilterDlg ( SwDocShell& rDocSh,
                                                                SvStream* pStream ) = 0;
    virtual VclAbstractDialog * CreateSwInsertBookmarkDlg( vcl::Window *pParent, SwWrtShell &rSh, SfxRequest& rReq, int nResId ) = 0;

    virtual AbstractSwBreakDlg * CreateSwBreakDlg(vcl::Window *pParent, SwWrtShell &rSh) = 0;
    virtual VclAbstractDialog   * CreateSwChangeDBDlg(SwView& rVw) = 0;
    virtual SfxAbstractTabDialog *  CreateSwCharDlg(vcl::Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
        SwCharDlgMode nDialogMode, const OUString* pFormatStr = nullptr) = 0;
    virtual AbstractSwConvertTableDlg* CreateSwConvertTableDlg(SwView& rView, bool bToTable) = 0;
    virtual VclAbstractDialog * CreateSwCaptionDialog ( vcl::Window *pParent, SwView &rV,int nResId) = 0;

    virtual AbstractSwInsertDBColAutoPilot* CreateSwInsertDBColAutoPilot(SwView& rView,
        css::uno::Reference< css::sdbc::XDataSource> rxSource,
        css::uno::Reference<css::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData) = 0;
    virtual SfxAbstractTabDialog * CreateSwFootNoteOptionDlg(vcl::Window *pParent, SwWrtShell &rSh) = 0;

    virtual AbstractDropDownFieldDialog * CreateDropDownFieldDialog(SwWrtShell &rSh,
        SwField* pField, bool bNextButton = false) = 0;
    virtual SfxAbstractTabDialog* CreateSwEnvDlg ( vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert ) = 0;

    virtual AbstractSwLabDlg* CreateSwLabDlg(const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel) = 0;

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod () =0;

    virtual SfxAbstractTabDialog* CreateSwParaDlg ( vcl::Window *pParent,
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    const OUString *pCollName = nullptr,
                                                    bool bDraw = false,
                                                    const OString& sDefPage = OString() ) = 0;

    virtual AbstractSwSelGlossaryDlg * CreateSwSelGlossaryDlg(const OUString &rShortName) = 0;

    virtual VclAbstractDialog * CreateVclAbstractDialog ( vcl::Window * pParent, SwWrtShell &rSh, int nResId ) = 0;
    virtual AbstractSplitTableDialog * CreateSplitTableDialog ( vcl::Window * pParent, SwWrtShell &rSh ) = 0;

    virtual AbstractSwAutoFormatDlg * CreateSwAutoFormatDlg( vcl::Window* pParent, SwWrtShell* pShell,
                                                            bool bSetAutoFormat = true,
                                                            const SwTableAutoFormat* pSelFormat = nullptr ) = 0;
    virtual SfxAbstractDialog * CreateSwBorderDlg ( vcl::Window* pParent, SfxItemSet& rSet, SwBorderModes nType, int nResId ) = 0;
    virtual SfxAbstractDialog * CreateSwWrapDlg ( vcl::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, int nResId ) = 0;

    virtual VclAbstractDialog * CreateSwTableWidthDlg(vcl::Window *pParent, SwTableFUNC &rFnc) = 0;
    virtual SfxAbstractTabDialog* CreateSwTableTabDlg(vcl::Window* pParent, SfxItemPool& Pool,
        const SfxItemSet* pItemSet, SwWrtShell* pSh) = 0;

    virtual AbstractSwFieldDlg * CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, vcl::Window *pParent) = 0;
    virtual SfxAbstractDialog*   CreateSwFieldEditDlg ( SwView& rVw, int nResId ) = 0;
    virtual AbstractSwRenameXNamedDlg * CreateSwRenameXNamedDlg(vcl::Window* pParent,
        css::uno::Reference< css::container::XNamed > & xNamed,
        css::uno::Reference< css::container::XNameAccess > & xNameAccess) = 0;
    virtual AbstractSwModalRedlineAcceptDlg * CreateSwModalRedlineAcceptDlg(vcl::Window *pParent) = 0;

    virtual VclAbstractDialog*          CreateTableMergeDialog( vcl::Window* pParent, bool& rWithPrev ) = 0;
    virtual SfxAbstractTabDialog*       CreateFrameTabDialog(const OUString &rDialogType,
                                                SfxViewFrame *pFrame, vcl::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool bNewFrame = true,
                                                const OString& sDefPage = OString(),
                                                const OUString* pFormatStr = nullptr) = 0;
    /// @param nSlot
    /// Identifies optional Slot by which the creation of the Template (Style) dialog is triggered.
    /// Currently used, if nRegion == SfxStyleFamily::Page in order to activate certain dialog pane
    virtual SfxAbstractApplyTabDialog*  CreateTemplateDialog(vcl::Window* pParent,
                                                SfxStyleSheetBase&  rBase,
                                                SfxStyleFamily      nRegion,
                                                const OString&      sPage = OString(),
                                                SwWrtShell*         pActShell = nullptr,
                                                bool                bNew = false) = 0;
    virtual AbstractGlossaryDlg*        CreateGlossaryDlg(SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell) = 0;
    virtual AbstractFieldInputDlg*        CreateFieldInputDlg(vcl::Window *pParent,
        SwWrtShell &rSh, SwField* pField, bool bNextButton = false) = 0;
    virtual AbstractInsFootNoteDlg*     CreateInsFootNoteDlg(vcl::Window * pParent,
        SwWrtShell &rSh, bool bEd = false) = 0;
    virtual VclAbstractDialog*          CreateTitlePageDlg ( vcl::Window * pParent ) = 0;
    virtual VclAbstractDialog *         CreateVclSwViewDialog(SwView& rView) = 0;
    virtual AbstractInsTableDlg*        CreateInsTableDlg(SwView& rView) = 0;
    virtual AbstractJavaEditDialog*     CreateJavaEditDialog(vcl::Window* pParent,
        SwWrtShell* pWrtSh) = 0;
    virtual AbstractMailMergeDlg*       CreateMailMergeDlg( int nResId,
                                                vcl::Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTableName,
                                                sal_Int32 nCommandType,
                                                const css::uno::Reference< css::sdbc::XConnection>& xConnection ) = 0;
    virtual AbstractMailMergeCreateFromDlg*     CreateMailMergeCreateFromDlg(vcl::Window* pParent) = 0;
    virtual AbstractMailMergeFieldConnectionsDlg* CreateMailMergeFieldConnectionsDlg(vcl::Window* pParent) = 0;
    virtual VclAbstractDialog*          CreateMultiTOXMarkDlg(vcl::Window* pParent, SwTOXMgr &rTOXMgr) = 0;
    virtual SfxAbstractTabDialog*       CreateSwTabDialog( int nResId,
                                                vcl::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &) = 0;
    virtual AbstractMultiTOXTabDialog*      CreateMultiTOXTabDialog(
                                                vcl::Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX, sal_uInt16 nToxType = USHRT_MAX,
                                                bool bGlobal = false) = 0;
    virtual AbstractEditRegionDlg*      CreateEditRegionDlg(vcl::Window* pParent, SwWrtShell& rWrtSh) = 0;
    virtual AbstractInsertSectionTabDialog*     CreateInsertSectionTabDialog(
        vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) = 0;
    virtual AbstractMarkFloatDlg*       CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       vcl::Window *pParent,
                                                       SfxChildWinInfo* pInfo) = 0;
    virtual AbstractMarkFloatDlg*       CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       vcl::Window *pParent,
                                                       SfxChildWinInfo* pInfo) = 0;
    virtual VclAbstractDialog *         CreateIndexMarkModalDlg(
                                                vcl::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark) = 0;

    virtual AbstractMailMergeWizard*    CreateMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rConfigItem) = 0;

    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc() = 0;
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc() = 0;

    // for tabpage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) = 0;

    virtual void ExecuteMMResultSaveDialog() = 0;
    virtual void ExecuteMMResultPrintDialog() = 0;
    virtual void ExecuteMMResultEmailDialog() = 0;

protected:
    ~SwAbstractDialogFactory() {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
