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
    virtual short   Execute() SAL_OVERRIDE ;

#define IMPL_ABSTDLG_BASE(Class)                    \
short Class::Execute()                             \
{                                                   \
    return pDlg->Execute();                         \
}

class SwWordCountFloatDlg;
class AbstractSwWordCountFloatDlg_Impl : public AbstractSwWordCountFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractSwWordCountFloatDlg_Impl,SwWordCountFloatDlg)
    virtual void                UpdateCounts() SAL_OVERRIDE;
    virtual void                SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat) SAL_OVERRIDE;
    virtual vcl::Window *            GetWindow() SAL_OVERRIDE; //this method is added for return a Window type pointer
};

class AbstractSwInsertAbstractDlg_Impl : public AbstractSwInsertAbstractDlg
{
    DECL_ABSTDLG_BASE(AbstractSwInsertAbstractDlg_Impl,SwInsertAbstractDlg)
    virtual sal_uInt8   GetLevel() const SAL_OVERRIDE ;
    virtual sal_uInt8   GetPara() const SAL_OVERRIDE ;
};

class SwAbstractSfxDialog_Impl :public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(SwAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class AbstractSwAsciiFilterDlg_Impl : public AbstractSwAsciiFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractSwAsciiFilterDlg_Impl,SwAsciiFilterDlg )
    virtual void FillOptions( SwAsciiOptions& rOptions ) SAL_OVERRIDE;

};

class VclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog)
};

class AbstractSwBreakDlg_Impl : public AbstractSwBreakDlg
{
    DECL_ABSTDLG_BASE(AbstractSwBreakDlg_Impl,SwBreakDlg)
    virtual OUString                        GetTemplateName() SAL_OVERRIDE;
    virtual sal_uInt16                      GetKind() SAL_OVERRIDE;
    virtual ::boost::optional<sal_uInt16>   GetPageNumber() SAL_OVERRIDE;

};
class AbstractSplitTableDialog_Impl : public AbstractSplitTableDialog // add for
{
    DECL_ABSTDLG_BASE(AbstractSplitTableDialog_Impl, SwSplitTableDlg)
    virtual sal_uInt16 GetSplitMode() SAL_OVERRIDE;
};

class AbstractTabDialog_Impl : virtual public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString &rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
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
    Link<> m_aHandler;
    virtual void                SetApplyHdl( const Link<>& rLink ) SAL_OVERRIDE;
};

class AbstractSwConvertTableDlg_Impl :  public AbstractSwConvertTableDlg
{
    DECL_ABSTDLG_BASE( AbstractSwConvertTableDlg_Impl,SwConvertTableDlg)
    virtual void GetValues( sal_Unicode& rDelim,SwInsertTableOptions& rInsTableFlags,
                    SwTableAutoFormat const*& prTAFormat) SAL_OVERRIDE;
};

class AbstractSwInsertDBColAutoPilot_Impl :  public AbstractSwInsertDBColAutoPilot
{
    DECL_ABSTDLG_BASE( AbstractSwInsertDBColAutoPilot_Impl,SwInsertDBColAutoPilot)
    virtual void DataToDoc( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rSelection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet) SAL_OVERRIDE;
};

class AbstractDropDownFieldDialog_Impl : public AbstractDropDownFieldDialog
{
    DECL_ABSTDLG_BASE(AbstractDropDownFieldDialog_Impl, sw::DropDownFieldDialog)
    virtual OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const SAL_OVERRIDE; //this method inherit from SystemWindow
    virtual void         SetWindowState( const OString& rStr ) SAL_OVERRIDE;//this method inherit from SystemWindow
};

class AbstractSwLabDlg_Impl  : public AbstractSwLabDlg
{
    DECL_ABSTDLG_BASE(AbstractSwLabDlg_Impl,SwLabDlg)
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString &rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
    virtual const OUString& GetBusinessCardStr() const SAL_OVERRIDE;
    virtual Printer *GetPrt() SAL_OVERRIDE;
};

class AbstractSwSelGlossaryDlg_Impl : public AbstractSwSelGlossaryDlg
{
    DECL_ABSTDLG_BASE(AbstractSwSelGlossaryDlg_Impl,SwSelGlossaryDlg)
    virtual void InsertGlos(const OUString &rRegion, const OUString &rGlosName) SAL_OVERRIDE;    // inline
    virtual sal_Int32 GetSelectedIdx() const SAL_OVERRIDE;  // inline
    virtual void SelectEntryPos(sal_Int32 nIdx) SAL_OVERRIDE;   // inline
};

class AbstractSwAutoFormatDlg_Impl : public AbstractSwAutoFormatDlg
{
    DECL_ABSTDLG_BASE(AbstractSwAutoFormatDlg_Impl,SwAutoFormatDlg )
    virtual void FillAutoFormatOfIndex( SwTableAutoFormat*& rToFill ) const SAL_OVERRIDE;
};

class AbstractSwFieldDlg_Impl : public AbstractSwFieldDlg
{
    DECL_ABSTDLG_BASE(AbstractSwFieldDlg_Impl,SwFieldDlg )
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString &rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*   GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
        //From class Window.
    virtual void                SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString            GetText() const SAL_OVERRIDE;
    virtual void                Start( bool bShow = true ) SAL_OVERRIDE;  //this method from SfxTabDialog
    virtual void                ShowReferencePage() SAL_OVERRIDE;
    virtual void                Initialize(SfxChildWinInfo *pInfo) SAL_OVERRIDE;
    virtual void                ReInitDlg() SAL_OVERRIDE;
    virtual void                ActivateDatabasePage() SAL_OVERRIDE;
    virtual vcl::Window *            GetWindow() SAL_OVERRIDE; //this method is added for return a Window type pointer
};

class AbstractSwRenameXNamedDlg_Impl : public AbstractSwRenameXNamedDlg
{
    DECL_ABSTDLG_BASE(AbstractSwRenameXNamedDlg_Impl,SwRenameXNamedDlg )
    virtual void    SetForbiddenChars( const OUString& rSet ) SAL_OVERRIDE;
    virtual void SetAlternativeAccess(
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xSecond,
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xThird ) SAL_OVERRIDE;
};

class AbstractSwModalRedlineAcceptDlg_Impl : public AbstractSwModalRedlineAcceptDlg
{
    DECL_ABSTDLG_BASE(AbstractSwModalRedlineAcceptDlg_Impl,SwModalRedlineAcceptDlg )
    virtual void            AcceptAll( bool bAccept ) SAL_OVERRIDE;
};

class SwGlossaryDlg;
class AbstractGlossaryDlg_Impl : public AbstractGlossaryDlg
{
    DECL_ABSTDLG_BASE(AbstractGlossaryDlg_Impl,SwGlossaryDlg)
    virtual OUString        GetCurrGrpName() const SAL_OVERRIDE;
    virtual OUString        GetCurrShortName() const SAL_OVERRIDE;
};

class SwFieldInputDlg;
class AbstractFieldInputDlg_Impl : public AbstractFieldInputDlg
{
    DECL_ABSTDLG_BASE(AbstractFieldInputDlg_Impl,SwFieldInputDlg)
    //from class SalFrame
    virtual void         SetWindowState( const OString & rStr ) SAL_OVERRIDE ;
    virtual OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const SAL_OVERRIDE ;
    virtual void EndDialog(long) SAL_OVERRIDE;
};

class SwInsFootNoteDlg;
class AbstractInsFootNoteDlg_Impl : public AbstractInsFootNoteDlg
{
    DECL_ABSTDLG_BASE(AbstractInsFootNoteDlg_Impl,SwInsFootNoteDlg)
    virtual OUString        GetFontName() SAL_OVERRIDE;
    virtual bool            IsEndNote() SAL_OVERRIDE;
    virtual OUString        GetStr() SAL_OVERRIDE;
    //from class Window
    virtual void    SetHelpId( const OString& sHelpId ) SAL_OVERRIDE;
    virtual void    SetText( const OUString& rStr ) SAL_OVERRIDE;
};

class SwInsTableDlg;
class AbstractInsTableDlg_Impl : public AbstractInsTableDlg
{
    DECL_ABSTDLG_BASE(AbstractInsTableDlg_Impl,SwInsTableDlg)
    virtual void            GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableFlags, OUString& rTableAutoFormatName,
                                SwTableAutoFormat *& prTAFormat ) SAL_OVERRIDE;
};

class SwJavaEditDialog;
class AbstractJavaEditDialog_Impl : public AbstractJavaEditDialog
{
    DECL_ABSTDLG_BASE(AbstractJavaEditDialog_Impl,SwJavaEditDialog)
    virtual OUString            GetScriptText() const SAL_OVERRIDE;
    virtual OUString            GetScriptType() const SAL_OVERRIDE;
    virtual bool                IsUrl() const SAL_OVERRIDE;
    virtual bool                IsNew() const SAL_OVERRIDE;
    virtual bool                IsUpdate() const SAL_OVERRIDE;
};

class SwMailMergeDlg;
class AbstractMailMergeDlg_Impl : public AbstractMailMergeDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeDlg_Impl,SwMailMergeDlg)
    virtual DBManagerOptions GetMergeType() SAL_OVERRIDE ;
    virtual const OUString& GetSaveFilter() const SAL_OVERRIDE;
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetSelection() const SAL_OVERRIDE ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> GetResultSet() const SAL_OVERRIDE;
    virtual bool IsSaveSingleDoc() const SAL_OVERRIDE;
    virtual bool IsGenerateFromDataBase() const SAL_OVERRIDE;
    virtual OUString GetColumnName() const SAL_OVERRIDE;
    virtual OUString GetPath() const SAL_OVERRIDE;
};

class SwMailMergeCreateFromDlg;
class AbstractMailMergeCreateFromDlg_Impl : public AbstractMailMergeCreateFromDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeCreateFromDlg_Impl,SwMailMergeCreateFromDlg)
    virtual bool    IsThisDocument() const SAL_OVERRIDE ;
};

class SwMailMergeFieldConnectionsDlg;
class AbstractMailMergeFieldConnectionsDlg_Impl : public AbstractMailMergeFieldConnectionsDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeFieldConnectionsDlg_Impl,SwMailMergeFieldConnectionsDlg)
    virtual bool    IsUseExistingConnections() const SAL_OVERRIDE ;
};

class SwMultiTOXTabDialog;
class AbstractMultiTOXTabDialog_Impl : public AbstractMultiTOXTabDialog
{
    DECL_ABSTDLG_BASE(AbstractMultiTOXTabDialog_Impl,SwMultiTOXTabDialog)
    virtual CurTOXType          GetCurrentTOXType() const SAL_OVERRIDE ;
    virtual SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes) SAL_OVERRIDE;
    //from SfxTabDialog
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
};

class SwEditRegionDlg;
class AbstractEditRegionDlg_Impl : public AbstractEditRegionDlg
{
    DECL_ABSTDLG_BASE(AbstractEditRegionDlg_Impl,SwEditRegionDlg)
    virtual void    SelectSection(const OUString& rSectionName) SAL_OVERRIDE;
};

class SwInsertSectionTabDialog;
class AbstractInsertSectionTabDialog_Impl : public AbstractInsertSectionTabDialog
{
    DECL_ABSTDLG_BASE(AbstractInsertSectionTabDialog_Impl,SwInsertSectionTabDialog)
    virtual void        SetSectionData(SwSectionData const& rSect) SAL_OVERRIDE;
};

class SwIndexMarkFloatDlg;
class AbstractIndexMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractIndexMarkFloatDlg_Impl,SwIndexMarkFloatDlg)
    virtual void    ReInitDlg(SwWrtShell& rWrtShell) SAL_OVERRIDE;
    virtual vcl::Window *            GetWindow() SAL_OVERRIDE; //this method is added for return a Window type pointer
};

class SwAuthMarkFloatDlg;
class AbstractAuthMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractAuthMarkFloatDlg_Impl,SwAuthMarkFloatDlg)
    virtual void    ReInitDlg(SwWrtShell& rWrtShell) SAL_OVERRIDE;
    virtual vcl::Window *            GetWindow() SAL_OVERRIDE; //this method is added for return a Window type pointer
};

class SwMailMergeWizard;
class AbstractMailMergeWizard_Impl : public AbstractMailMergeWizard
{
    VclPtr<SwMailMergeWizard> pDlg;
    Link<>             aEndDlgHdl;

    DECL_LINK( EndDialogHdl, SwMailMergeWizard* );
public:
    explicit AbstractMailMergeWizard_Impl( SwMailMergeWizard* p )
     : pDlg(p)
     {}
    virtual         ~AbstractMailMergeWizard_Impl();
    virtual void    StartExecuteModal( const Link<>& rEndDialogHdl ) SAL_OVERRIDE;
    virtual long    GetResult() SAL_OVERRIDE;

    virtual OUString            GetReloadDocument() const SAL_OVERRIDE;
    virtual bool                ShowPage( sal_uInt16 nLevel ) SAL_OVERRIDE;
    virtual sal_uInt16          GetRestartPage() const SAL_OVERRIDE;
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
                                                                        ) SAL_OVERRIDE;
    virtual AbstractSwWordCountFloatDlg* CreateSwWordCountDialog(SfxBindings* pBindings,
        SfxChildWindow* pChild, vcl::Window *pParent, SfxChildWinInfo* pInfo) SAL_OVERRIDE;
    virtual AbstractSwInsertAbstractDlg * CreateSwInsertAbstractDlg(vcl::Window* pParent) SAL_OVERRIDE;
    virtual AbstractSwAsciiFilterDlg*  CreateSwAsciiFilterDlg ( vcl::Window* pParent, SwDocShell& rDocSh,
                                                                SvStream* pStream ) SAL_OVERRIDE;
    virtual VclAbstractDialog * CreateSwInsertBookmarkDlg( vcl::Window *pParent, SwWrtShell &rSh, SfxRequest& rReq, int nResId ) SAL_OVERRIDE;
    virtual AbstractSwBreakDlg * CreateSwBreakDlg(vcl::Window *pParent, SwWrtShell &rSh) SAL_OVERRIDE;
    virtual VclAbstractDialog   * CreateSwChangeDBDlg(SwView& rVw) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog *  CreateSwCharDlg(vcl::Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
        sal_uInt8 nDialogMode, const OUString* pFormatStr = 0) SAL_OVERRIDE;
    virtual AbstractSwConvertTableDlg* CreateSwConvertTableDlg(SwView& rView, bool bToTable) SAL_OVERRIDE;
    virtual VclAbstractDialog * CreateSwCaptionDialog ( vcl::Window *pParent, SwView &rV,int nResId) SAL_OVERRIDE;

    virtual AbstractSwInsertDBColAutoPilot* CreateSwInsertDBColAutoPilot(SwView& rView,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        com::sun::star::uno::Reference<com::sun::star::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog * CreateSwFootNoteOptionDlg(vcl::Window *pParent, SwWrtShell &rSh) SAL_OVERRIDE;

    virtual AbstractDropDownFieldDialog * CreateDropDownFieldDialog (vcl::Window *pParent, SwWrtShell &rSh,
        SwField* pField, bool bNextButton = false) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog* CreateSwEnvDlg ( vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert ) SAL_OVERRIDE;
    virtual AbstractSwLabDlg* CreateSwLabDlg(vcl::Window* pParent, const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel) SAL_OVERRIDE;

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod () SAL_OVERRIDE;
    virtual SfxAbstractTabDialog* CreateSwParaDlg ( vcl::Window *pParent,
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    sal_uInt8 nDialogMode,
                                                    const OUString *pCollName = 0,
                                                    bool bDraw = false,
                                                    const OString& sDefPage = OString() ) SAL_OVERRIDE;

    virtual AbstractSwSelGlossaryDlg * CreateSwSelGlossaryDlg(vcl::Window * pParent, const OUString &rShortName) SAL_OVERRIDE;
    virtual VclAbstractDialog * CreateVclAbstractDialog ( vcl::Window * pParent, SwWrtShell &rSh, int nResId ) SAL_OVERRIDE;
    virtual AbstractSplitTableDialog * CreateSplitTableDialog ( vcl::Window * pParent, SwWrtShell &rSh ) SAL_OVERRIDE;

    virtual AbstractSwAutoFormatDlg * CreateSwAutoFormatDlg( vcl::Window* pParent, SwWrtShell* pShell,
                                                            bool bSetAutoFormat = true,
                                                            const SwTableAutoFormat* pSelFormat = 0 ) SAL_OVERRIDE;
    virtual SfxAbstractDialog * CreateSwBorderDlg (vcl::Window* pParent, SfxItemSet& rSet, SwBorderModes nType, int nResId ) SAL_OVERRIDE;

    virtual SfxAbstractDialog * CreateSwWrapDlg ( vcl::Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, bool bDrawMode, int nResId ) SAL_OVERRIDE;
    virtual VclAbstractDialog * CreateSwTableWidthDlg(vcl::Window *pParent, SwTableFUNC &rFnc) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog* CreateSwTableTabDlg(vcl::Window* pParent, SfxItemPool& Pool,
        const SfxItemSet* pItemSet, SwWrtShell* pSh) SAL_OVERRIDE;
    virtual AbstractSwFieldDlg * CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, vcl::Window *pParent) SAL_OVERRIDE;
    virtual SfxAbstractDialog*   CreateSwFieldEditDlg ( SwView& rVw, int nResId ) SAL_OVERRIDE;
    virtual AbstractSwRenameXNamedDlg * CreateSwRenameXNamedDlg(vcl::Window* pParent,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > & xNamed,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xNameAccess) SAL_OVERRIDE;
    virtual AbstractSwModalRedlineAcceptDlg * CreateSwModalRedlineAcceptDlg(vcl::Window *pParent) SAL_OVERRIDE;

    virtual VclAbstractDialog*          CreateTableMergeDialog(vcl::Window* pParent, bool& rWithPrev) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateFrmTabDialog( const OUString &rDialogType,
                                                SfxViewFrame *pFrame, vcl::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool bNewFrm  = true,
                                                bool bFormat     = false,
                                                const OString& sDefPage = OString(),
                                                const OUString*   pFormatStr  = 0) SAL_OVERRIDE;
    virtual SfxAbstractApplyTabDialog*  CreateTemplateDialog(
                                                vcl::Window*             pParent,
                                                SfxStyleSheetBase&  rBase,
                                                sal_uInt16          nRegion,
                                                const OString&      sPage = OString(),
                                                SwWrtShell*         pActShell = 0,
                                                bool                bNew = false) SAL_OVERRIDE;
    virtual AbstractGlossaryDlg*        CreateGlossaryDlg(SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell) SAL_OVERRIDE;
    virtual AbstractFieldInputDlg*        CreateFieldInputDlg(vcl::Window *pParent,
        SwWrtShell &rSh, SwField* pField, bool bNextButton = false) SAL_OVERRIDE;
    virtual AbstractInsFootNoteDlg*     CreateInsFootNoteDlg(
        vcl::Window * pParent, SwWrtShell &rSh, bool bEd = false) SAL_OVERRIDE;
    virtual VclAbstractDialog *         CreateTitlePageDlg ( vcl::Window * pParent ) SAL_OVERRIDE;
    virtual VclAbstractDialog *         CreateVclSwViewDialog(SwView& rView) SAL_OVERRIDE;
    virtual AbstractInsTableDlg*        CreateInsTableDlg(SwView& rView) SAL_OVERRIDE;
    virtual AbstractJavaEditDialog*     CreateJavaEditDialog(vcl::Window* pParent,
        SwWrtShell* pWrtSh) SAL_OVERRIDE;
    virtual AbstractMailMergeDlg*       CreateMailMergeDlg( int nResId,
                                                vcl::Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTableName,
                                                sal_Int32 nCommandType,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection,
                                                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >* pSelection = 0 ) SAL_OVERRIDE;
    virtual AbstractMailMergeCreateFromDlg*     CreateMailMergeCreateFromDlg(vcl::Window* pParent) SAL_OVERRIDE;
    virtual AbstractMailMergeFieldConnectionsDlg* CreateMailMergeFieldConnectionsDlg(vcl::Window* pParent) SAL_OVERRIDE;
    virtual VclAbstractDialog*          CreateMultiTOXMarkDlg(vcl::Window* pParent, SwTOXMgr &rTOXMgr) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateSwTabDialog( int nResId,
                                                vcl::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &) SAL_OVERRIDE;
    virtual AbstractMultiTOXTabDialog*      CreateMultiTOXTabDialog(
                                                vcl::Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX, sal_uInt16 nToxType = USHRT_MAX,
                                                bool bGlobal = false) SAL_OVERRIDE;
    virtual AbstractEditRegionDlg*      CreateEditRegionDlg(vcl::Window* pParent, SwWrtShell& rWrtSh) SAL_OVERRIDE;
    virtual AbstractInsertSectionTabDialog*     CreateInsertSectionTabDialog(
        vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) SAL_OVERRIDE;
    virtual AbstractMarkFloatDlg*       CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       vcl::Window *pParent,
                                                       SfxChildWinInfo* pInfo,
                                                       bool bNew=true) SAL_OVERRIDE;
    virtual AbstractMarkFloatDlg*       CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       vcl::Window *pParent,
                                                       SfxChildWinInfo* pInfo,
                                                       bool bNew=true) SAL_OVERRIDE;
    virtual VclAbstractDialog *         CreateIndexMarkModalDlg(
                                                vcl::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark ) SAL_OVERRIDE;

    virtual AbstractMailMergeWizard*    CreateMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rConfigItem) SAL_OVERRIDE;

    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc() SAL_OVERRIDE;
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc() SAL_OVERRIDE;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
