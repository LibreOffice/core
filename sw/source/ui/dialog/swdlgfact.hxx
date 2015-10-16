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

#include "swabstdlg.hxx"

class SwInsertAbstractDlg;
class SwAsciiFilterDlg;
class Dialog;
class SwBreakDlg;
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

#include "itabenum.hxx"

#include <boost/optional.hpp>

namespace sw
{
class DropDownFieldDialog;
}

#define DECL_ABSTDLG_BASE(Class,DialogClass)        \
protected:                                          \
    ScopedVclPtr<DialogClass> pDlg;                 \
public:                                             \
    explicit        Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual short   Execute() override ;

#define IMPL_ABSTDLG_BASE(Class)                    \
short Class::Execute()                             \
{                                                   \
    return pDlg->Execute();                         \
}

class SwWordCountFloatDlg;
class AbstractSwWordCountFloatDlg_Impl : public AbstractSwWordCountFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractSwWordCountFloatDlg_Impl,SwWordCountFloatDlg)
    virtual void                UpdateCounts() override;
    virtual void                SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat) override;
    virtual vcl::Window *            GetWindow() override; //this method is added for return a Window type pointer
};

class AbstractSwInsertAbstractDlg_Impl : public AbstractSwInsertAbstractDlg
{
    DECL_ABSTDLG_BASE(AbstractSwInsertAbstractDlg_Impl,SwInsertAbstractDlg)
    virtual sal_uInt8   GetLevel() const override ;
    virtual sal_uInt8   GetPara() const override ;
};

class SwAbstractSfxDialog_Impl :public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(SwAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class AbstractSwAsciiFilterDlg_Impl : public AbstractSwAsciiFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractSwAsciiFilterDlg_Impl,SwAsciiFilterDlg )
    virtual void FillOptions( SwAsciiOptions& rOptions ) override;

};

class VclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog)
};

class AbstractSwBreakDlg_Impl : public AbstractSwBreakDlg
{
    DECL_ABSTDLG_BASE(AbstractSwBreakDlg_Impl,SwBreakDlg)
    virtual OUString                        GetTemplateName() override;
    virtual sal_uInt16                      GetKind() override;
    virtual ::boost::optional<sal_uInt16>   GetPageNumber() override;

};
class AbstractSplitTableDialog_Impl : public AbstractSplitTableDialog // add for
{
    DECL_ABSTDLG_BASE(AbstractSplitTableDialog_Impl, SwSplitTableDlg)
    virtual sal_uInt16 GetSplitMode() override;
};

class AbstractTabDialog_Impl : virtual public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class AbstractApplyTabDialog_Impl : public AbstractTabDialog_Impl, virtual public SfxAbstractApplyTabDialog
{
public:
    explicit AbstractApplyTabDialog_Impl( SfxTabDialog* p)
        : AbstractTabDialog_Impl(p)
    {
    }
    DECL_LINK_TYPED(ApplyHdl, Button*, void);
private:
    Link<LinkParamNone*,void> m_aHandler;
    virtual void                SetApplyHdl( const Link<LinkParamNone*,void>& rLink ) override;
};

class AbstractSwConvertTableDlg_Impl :  public AbstractSwConvertTableDlg
{
    DECL_ABSTDLG_BASE( AbstractSwConvertTableDlg_Impl,SwConvertTableDlg)
    virtual void GetValues( sal_Unicode& rDelim,SwInsertTableOptions& rInsTableFlags,
                    SwTableAutoFormat const*& prTAFormat) override;
};

class AbstractSwInsertDBColAutoPilot_Impl :  public AbstractSwInsertDBColAutoPilot
{
    DECL_ABSTDLG_BASE( AbstractSwInsertDBColAutoPilot_Impl,SwInsertDBColAutoPilot)
    virtual void DataToDoc( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rSelection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet) override;
};

class AbstractDropDownFieldDialog_Impl : public AbstractDropDownFieldDialog
{
    DECL_ABSTDLG_BASE(AbstractDropDownFieldDialog_Impl, sw::DropDownFieldDialog)
    virtual OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const override; //this method inherit from SystemWindow
    virtual void         SetWindowState( const OString& rStr ) override;//this method inherit from SystemWindow
};

class AbstractSwLabDlg_Impl  : public AbstractSwLabDlg
{
    DECL_ABSTDLG_BASE(AbstractSwLabDlg_Impl,SwLabDlg)
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
    virtual const OUString& GetBusinessCardStr() const override;
    virtual Printer *GetPrt() override;
};

class AbstractSwSelGlossaryDlg_Impl : public AbstractSwSelGlossaryDlg
{
    DECL_ABSTDLG_BASE(AbstractSwSelGlossaryDlg_Impl,SwSelGlossaryDlg)
    virtual void InsertGlos(const OUString &rRegion, const OUString &rGlosName) override;    // inline
    virtual sal_Int32 GetSelectedIdx() const override;  // inline
    virtual void SelectEntryPos(sal_Int32 nIdx) override;   // inline
};

class AbstractSwAutoFormatDlg_Impl : public AbstractSwAutoFormatDlg
{
    DECL_ABSTDLG_BASE(AbstractSwAutoFormatDlg_Impl,SwAutoFormatDlg )
    virtual void FillAutoFormatOfIndex( SwTableAutoFormat*& rToFill ) const override;
};

class AbstractSwFieldDlg_Impl : public AbstractSwFieldDlg
{
    DECL_ABSTDLG_BASE(AbstractSwFieldDlg_Impl,SwFieldDlg )
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*   GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void                SetText( const OUString& rStr ) override;
    virtual OUString            GetText() const override;
    virtual void                Start( bool bShow = true ) override;  //this method from SfxTabDialog
    virtual void                ShowReferencePage() override;
    virtual void                Initialize(SfxChildWinInfo *pInfo) override;
    virtual void                ReInitDlg() override;
    virtual void                ActivateDatabasePage() override;
    virtual vcl::Window *            GetWindow() override; //this method is added for return a Window type pointer
};

class AbstractSwRenameXNamedDlg_Impl : public AbstractSwRenameXNamedDlg
{
    DECL_ABSTDLG_BASE(AbstractSwRenameXNamedDlg_Impl,SwRenameXNamedDlg )
    virtual void    SetForbiddenChars( const OUString& rSet ) override;
    virtual void SetAlternativeAccess(
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xSecond,
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xThird ) override;
};

class AbstractSwModalRedlineAcceptDlg_Impl : public AbstractSwModalRedlineAcceptDlg
{
    DECL_ABSTDLG_BASE(AbstractSwModalRedlineAcceptDlg_Impl,SwModalRedlineAcceptDlg )
    virtual void            AcceptAll( bool bAccept ) override;
};

class SwGlossaryDlg;
class AbstractGlossaryDlg_Impl : public AbstractGlossaryDlg
{
    DECL_ABSTDLG_BASE(AbstractGlossaryDlg_Impl,SwGlossaryDlg)
    virtual OUString        GetCurrGrpName() const override;
    virtual OUString        GetCurrShortName() const override;
};

class SwFieldInputDlg;
class AbstractFieldInputDlg_Impl : public AbstractFieldInputDlg
{
    DECL_ABSTDLG_BASE(AbstractFieldInputDlg_Impl,SwFieldInputDlg)
    //from class SalFrame
    virtual void         SetWindowState( const OString & rStr ) override ;
    virtual OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const override ;
    virtual void EndDialog(long) override;
};

class SwInsFootNoteDlg;
class AbstractInsFootNoteDlg_Impl : public AbstractInsFootNoteDlg
{
    DECL_ABSTDLG_BASE(AbstractInsFootNoteDlg_Impl,SwInsFootNoteDlg)
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
    DECL_ABSTDLG_BASE(AbstractInsTableDlg_Impl,SwInsTableDlg)
    virtual void            GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableFlags, OUString& rTableAutoFormatName,
                                SwTableAutoFormat *& prTAFormat ) override;
};

class SwJavaEditDialog;
class AbstractJavaEditDialog_Impl : public AbstractJavaEditDialog
{
    DECL_ABSTDLG_BASE(AbstractJavaEditDialog_Impl,SwJavaEditDialog)
    virtual OUString            GetScriptText() const override;
    virtual OUString            GetScriptType() const override;
    virtual bool                IsUrl() const override;
    virtual bool                IsNew() const override;
    virtual bool                IsUpdate() const override;
};

class SwMailMergeDlg;
class AbstractMailMergeDlg_Impl : public AbstractMailMergeDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeDlg_Impl,SwMailMergeDlg)
    virtual DBManagerOptions GetMergeType() override ;
    virtual const OUString& GetSaveFilter() const override;
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetSelection() const override ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> GetResultSet() const override;
    virtual bool IsSaveSingleDoc() const override;
    virtual bool IsGenerateFromDataBase() const override;
    virtual OUString GetColumnName() const override;
    virtual OUString GetPath() const override;
};

class SwMailMergeCreateFromDlg;
class AbstractMailMergeCreateFromDlg_Impl : public AbstractMailMergeCreateFromDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeCreateFromDlg_Impl,SwMailMergeCreateFromDlg)
    virtual bool    IsThisDocument() const override ;
};

class SwMailMergeFieldConnectionsDlg;
class AbstractMailMergeFieldConnectionsDlg_Impl : public AbstractMailMergeFieldConnectionsDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeFieldConnectionsDlg_Impl,SwMailMergeFieldConnectionsDlg)
    virtual bool    IsUseExistingConnections() const override ;
};

class SwMultiTOXTabDialog;
class AbstractMultiTOXTabDialog_Impl : public AbstractMultiTOXTabDialog
{
    DECL_ABSTDLG_BASE(AbstractMultiTOXTabDialog_Impl,SwMultiTOXTabDialog)
    virtual CurTOXType          GetCurrentTOXType() const override ;
    virtual SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes) override;
    //from SfxTabDialog
    virtual const SfxItemSet*   GetOutputItemSet() const override;
};

class SwEditRegionDlg;
class AbstractEditRegionDlg_Impl : public AbstractEditRegionDlg
{
    DECL_ABSTDLG_BASE(AbstractEditRegionDlg_Impl,SwEditRegionDlg)
    virtual void    SelectSection(const OUString& rSectionName) override;
};

class SwInsertSectionTabDialog;
class AbstractInsertSectionTabDialog_Impl : public AbstractInsertSectionTabDialog
{
    DECL_ABSTDLG_BASE(AbstractInsertSectionTabDialog_Impl,SwInsertSectionTabDialog)
    virtual void        SetSectionData(SwSectionData const& rSect) override;
};

class SwIndexMarkFloatDlg;
class AbstractIndexMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractIndexMarkFloatDlg_Impl,SwIndexMarkFloatDlg)
    virtual void    ReInitDlg(SwWrtShell& rWrtShell) override;
    virtual vcl::Window *            GetWindow() override; //this method is added for return a Window type pointer
};

class SwAuthMarkFloatDlg;
class AbstractAuthMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractAuthMarkFloatDlg_Impl,SwAuthMarkFloatDlg)
    virtual void    ReInitDlg(SwWrtShell& rWrtShell) override;
    virtual vcl::Window *            GetWindow() override; //this method is added for return a Window type pointer
};

class SwMailMergeWizard;
class AbstractMailMergeWizard_Impl : public AbstractMailMergeWizard
{
    VclPtr<SwMailMergeWizard> pDlg;
    Link<Dialog&,void>        aEndDlgHdl;

    DECL_LINK_TYPED( EndDialogHdl, Dialog&, void );
public:
    explicit AbstractMailMergeWizard_Impl( SwMailMergeWizard* p )
     : pDlg(p)
     {}
    virtual         ~AbstractMailMergeWizard_Impl();
    virtual void    StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) override;
    virtual long    GetResult() override;

    virtual OUString            GetReloadDocument() const override;
    virtual bool                ShowPage( sal_uInt16 nLevel ) override;
    virtual sal_uInt16          GetRestartPage() const override;
};

//AbstractDialogFactory_Impl implementations
class SwAbstractDialogFactory_Impl : public SwAbstractDialogFactory
{

public:
    virtual ~SwAbstractDialogFactory_Impl() {}

    virtual SfxAbstractDialog*              CreateSfxDialog( vcl::Window* pParent,
                                                                        const SfxItemSet& rAttr,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame,
                                                                        sal_uInt32 nResId
                                                                        ) override;
    virtual AbstractSwWordCountFloatDlg* CreateSwWordCountDialog(SfxBindings* pBindings,
        SfxChildWindow* pChild, vcl::Window *pParent, SfxChildWinInfo* pInfo) override;
    virtual AbstractSwInsertAbstractDlg * CreateSwInsertAbstractDlg(vcl::Window* pParent) override;
    virtual AbstractSwAsciiFilterDlg*  CreateSwAsciiFilterDlg ( vcl::Window* pParent, SwDocShell& rDocSh,
                                                                SvStream* pStream ) override;
    virtual VclAbstractDialog * CreateSwInsertBookmarkDlg( vcl::Window *pParent, SwWrtShell &rSh, SfxRequest& rReq, int nResId ) override;
    virtual AbstractSwBreakDlg * CreateSwBreakDlg(vcl::Window *pParent, SwWrtShell &rSh) override;
    virtual VclAbstractDialog   * CreateSwChangeDBDlg(SwView& rVw) override;
    virtual SfxAbstractTabDialog *  CreateSwCharDlg(vcl::Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
        sal_uInt8 nDialogMode, const OUString* pFormatStr = 0) override;
    virtual AbstractSwConvertTableDlg* CreateSwConvertTableDlg(SwView& rView, bool bToTable) override;
    virtual VclAbstractDialog * CreateSwCaptionDialog ( vcl::Window *pParent, SwView &rV,int nResId) override;

    virtual AbstractSwInsertDBColAutoPilot* CreateSwInsertDBColAutoPilot(SwView& rView,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        com::sun::star::uno::Reference<com::sun::star::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData) override;
    virtual SfxAbstractTabDialog * CreateSwFootNoteOptionDlg(vcl::Window *pParent, SwWrtShell &rSh) override;

    virtual AbstractDropDownFieldDialog * CreateDropDownFieldDialog (vcl::Window *pParent, SwWrtShell &rSh,
        SwField* pField, bool bNextButton = false) override;
    virtual SfxAbstractTabDialog* CreateSwEnvDlg ( vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert ) override;
    virtual AbstractSwLabDlg* CreateSwLabDlg(vcl::Window* pParent, const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel) override;

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod () override;
    virtual SfxAbstractTabDialog* CreateSwParaDlg ( vcl::Window *pParent,
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    sal_uInt8 nDialogMode,
                                                    const OUString *pCollName = 0,
                                                    bool bDraw = false,
                                                    const OString& sDefPage = OString() ) override;

    virtual AbstractSwSelGlossaryDlg * CreateSwSelGlossaryDlg(vcl::Window * pParent, const OUString &rShortName) override;
    virtual VclAbstractDialog * CreateVclAbstractDialog ( vcl::Window * pParent, SwWrtShell &rSh, int nResId ) override;
    virtual AbstractSplitTableDialog * CreateSplitTableDialog ( vcl::Window * pParent, SwWrtShell &rSh ) override;

    virtual AbstractSwAutoFormatDlg * CreateSwAutoFormatDlg( vcl::Window* pParent, SwWrtShell* pShell,
                                                            bool bSetAutoFormat = true,
                                                            const SwTableAutoFormat* pSelFormat = 0 ) override;
    virtual SfxAbstractDialog * CreateSwBorderDlg (vcl::Window* pParent, SfxItemSet& rSet, SwBorderModes nType, int nResId ) override;

    virtual SfxAbstractDialog * CreateSwWrapDlg ( vcl::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, bool bDrawMode, int nResId ) override;
    virtual VclAbstractDialog * CreateSwTableWidthDlg(vcl::Window *pParent, SwTableFUNC &rFnc) override;
    virtual SfxAbstractTabDialog* CreateSwTableTabDlg(vcl::Window* pParent, SfxItemPool& Pool,
        const SfxItemSet* pItemSet, SwWrtShell* pSh) override;
    virtual AbstractSwFieldDlg * CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, vcl::Window *pParent) override;
    virtual SfxAbstractDialog*   CreateSwFieldEditDlg ( SwView& rVw, int nResId ) override;
    virtual AbstractSwRenameXNamedDlg * CreateSwRenameXNamedDlg(vcl::Window* pParent,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > & xNamed,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xNameAccess) override;
    virtual AbstractSwModalRedlineAcceptDlg * CreateSwModalRedlineAcceptDlg(vcl::Window *pParent) override;

    virtual VclAbstractDialog*          CreateTableMergeDialog(vcl::Window* pParent, bool& rWithPrev) override;
    virtual SfxAbstractTabDialog*       CreateFrmTabDialog( const OUString &rDialogType,
                                                SfxViewFrame *pFrame, vcl::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool bNewFrm  = true,
                                                bool bFormat     = false,
                                                const OString& sDefPage = OString(),
                                                const OUString*   pFormatStr  = 0) override;
    virtual SfxAbstractApplyTabDialog*  CreateTemplateDialog(
                                                vcl::Window*             pParent,
                                                SfxStyleSheetBase&  rBase,
                                                sal_uInt16          nRegion,
                                                const OString&      sPage = OString(),
                                                SwWrtShell*         pActShell = 0,
                                                bool                bNew = false) override;
    virtual AbstractGlossaryDlg*        CreateGlossaryDlg(SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell) override;
    virtual AbstractFieldInputDlg*        CreateFieldInputDlg(vcl::Window *pParent,
        SwWrtShell &rSh, SwField* pField, bool bNextButton = false) override;
    virtual AbstractInsFootNoteDlg*     CreateInsFootNoteDlg(
        vcl::Window * pParent, SwWrtShell &rSh, bool bEd = false) override;
    virtual VclAbstractDialog *         CreateTitlePageDlg ( vcl::Window * pParent ) override;
    virtual VclAbstractDialog *         CreateVclSwViewDialog(SwView& rView) override;
    virtual AbstractInsTableDlg*        CreateInsTableDlg(SwView& rView) override;
    virtual AbstractJavaEditDialog*     CreateJavaEditDialog(vcl::Window* pParent,
        SwWrtShell* pWrtSh) override;
    virtual AbstractMailMergeDlg*       CreateMailMergeDlg( int nResId,
                                                vcl::Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTableName,
                                                sal_Int32 nCommandType,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection,
                                                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >* pSelection = 0 ) override;
    virtual AbstractMailMergeCreateFromDlg*     CreateMailMergeCreateFromDlg(vcl::Window* pParent) override;
    virtual AbstractMailMergeFieldConnectionsDlg* CreateMailMergeFieldConnectionsDlg(vcl::Window* pParent) override;
    virtual VclAbstractDialog*          CreateMultiTOXMarkDlg(vcl::Window* pParent, SwTOXMgr &rTOXMgr) override;
    virtual SfxAbstractTabDialog*       CreateSwTabDialog( int nResId,
                                                vcl::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &) override;
    virtual AbstractMultiTOXTabDialog*      CreateMultiTOXTabDialog(
                                                vcl::Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX, sal_uInt16 nToxType = USHRT_MAX,
                                                bool bGlobal = false) override;
    virtual AbstractEditRegionDlg*      CreateEditRegionDlg(vcl::Window* pParent, SwWrtShell& rWrtSh) override;
    virtual AbstractInsertSectionTabDialog*     CreateInsertSectionTabDialog(
        vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) override;
    virtual AbstractMarkFloatDlg*       CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       vcl::Window *pParent,
                                                       SfxChildWinInfo* pInfo,
                                                       bool bNew=true) override;
    virtual AbstractMarkFloatDlg*       CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       vcl::Window *pParent,
                                                       SfxChildWinInfo* pInfo,
                                                       bool bNew=true) override;
    virtual VclAbstractDialog *         CreateIndexMarkModalDlg(
                                                vcl::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark ) override;

    virtual AbstractMailMergeWizard*    CreateMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rConfigItem) override;

    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc() override;
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc() override;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
