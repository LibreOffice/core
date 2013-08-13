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
#ifndef _SW_DLGFACT_HXX
#define _SW_DLGFACT_HXX

#include "swabstdlg.hxx"

class SwInsertAbstractDlg;
class SfxNoLayoutSingleTabDialog;
class SwAsciiFilterDlg;
class Dialog;
class SwBreakDlg;
class SfxTabDialog;
class SwConvertTableDlg;
class SwInsertDBColAutoPilot;
class SwLabDlg;
class SwSelGlossaryDlg;
class SwAutoFormatDlg;
class SwFldDlg;
class SwRenameXNamedDlg;
class SwModalRedlineAcceptDlg;
class SwTOXMark;
class SwSplitTblDlg;

#include "itabenum.hxx"

namespace sw
{
class DropDownFieldDialog;
}

#define DECL_ABSTDLG_BASE(Class,DialogClass)        \
protected:                                          \
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual short   Execute() ;

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
short Class::Execute()                             \
{                                                   \
    return pDlg->Execute();                         \
}

class SwWordCountFloatDlg;
class AbstractSwWordCountFloatDlg_Impl : public AbstractSwWordCountFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractSwWordCountFloatDlg_Impl,SwWordCountFloatDlg)
    virtual void                UpdateCounts();
    virtual void                SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat);
    virtual Window *            GetWindow(); //this method is added for return a Window type pointer
};

//add for SwInsertAbstractDlg begin
class AbstractSwInsertAbstractDlg_Impl : public AbstractSwInsertAbstractDlg
{
    DECL_ABSTDLG_BASE(AbstractSwInsertAbstractDlg_Impl,SwInsertAbstractDlg)
    virtual sal_uInt8   GetLevel() const ;
    virtual sal_uInt8   GetPara() const ;
};

//add for SwInsertAbstractDlg end

// add for SwAddrDlg, SwDropCapsDlg, SwBackgroundDlg SwNumFmtDlg  SwWrapDlg, SwFldEditDlg  begin
class SfxNoLayoutSingleTabDialog;
class SwAbstractSfxDialog_Impl :public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(SwAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
};
// add for SwAddrDlg,SwDropCapsDlg , SwBackgroundDlg  SwNumFmtDlg SwWrapDlg, SwFldEditDlg  end

// add for SwAsciiFilterDlg begin
class AbstractSwAsciiFilterDlg_Impl : public AbstractSwAsciiFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractSwAsciiFilterDlg_Impl,SwAsciiFilterDlg )
    virtual void FillOptions( SwAsciiOptions& rOptions );

};
// add for SwAsciiFilterDlg end

// add for SwInsertBookmarkDlg  SwChangeDBDlg, SwTableHeightDlg, SwSplitTblDlg  SwSortDlg   SwTableWidthDlgbegin
class VclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog)
};
// add for SwInsertBookmarkDlg SwChangeDBDlg, SwTableHeightDlg SwSplitTblDlg  SwSortDlg   SwTableWidthDlg end

// add for SwBreakDlg begin
class AbstractSwBreakDlg_Impl : public AbstractSwBreakDlg // add for SwBreakDlg
{
    DECL_ABSTDLG_BASE(AbstractSwBreakDlg_Impl,SwBreakDlg)
    virtual String  GetTemplateName();
    virtual sal_uInt16  GetKind();
    virtual sal_uInt16  GetPageNumber();

};
class AbstractSplitTableDialog_Impl : public AbstractSplitTableDialog // add for
{
    DECL_ABSTDLG_BASE(AbstractSplitTableDialog_Impl, SwSplitTblDlg)
    virtual sal_uInt16 GetSplitMode();
};

// add for SwBreakDlg end

//add for SwCharDlg , SwEnvDlg , SwFootNoteOptionDlg SwParaDlg  SwTableTabDlg begin
class AbstractTabDialog_Impl : virtual public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual void                SetCurPageId( const OString &rName );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
};
//add for SwCharDlg, SwEnvDlg ,SwFootNoteOptionDlg SwParaDlg  SwTableTabDlg end

class AbstractApplyTabDialog_Impl : public AbstractTabDialog_Impl, virtual public SfxAbstractApplyTabDialog
{
public:
    AbstractApplyTabDialog_Impl( SfxTabDialog* p)
        : AbstractTabDialog_Impl(p)
    {
    }
    DECL_LINK(ApplyHdl, void*);
private:
    Link m_aHandler;
    virtual void                SetApplyHdl( const Link& rLink );
};

//add for SwConvertTableDlg begin
class AbstractSwConvertTableDlg_Impl :  public AbstractSwConvertTableDlg // add for SwConvertTableDlg
{
    DECL_ABSTDLG_BASE( AbstractSwConvertTableDlg_Impl,SwConvertTableDlg)
    virtual void GetValues( sal_Unicode& rDelim,SwInsertTableOptions& rInsTblFlags,
                    SwTableAutoFmt const*& prTAFmt);
};
//add for SwConvertTableDlg end

//add for SwInsertDBColAutoPilot begin
class AbstractSwInsertDBColAutoPilot_Impl :  public AbstractSwInsertDBColAutoPilot // add for SwInsertDBColAutoPilot
{
    DECL_ABSTDLG_BASE( AbstractSwInsertDBColAutoPilot_Impl,SwInsertDBColAutoPilot)
    virtual void DataToDoc( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rSelection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet);
};
//add for SwInsertDBColAutoPilot end

//add for DropDownFieldDialog begin
class AbstractDropDownFieldDialog_Impl : public AbstractDropDownFieldDialog //add for DropDownFieldDialog
{
    DECL_ABSTDLG_BASE(AbstractDropDownFieldDialog_Impl, sw::DropDownFieldDialog)
    virtual OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const; //this method inherit from SystemWindow
    virtual void         SetWindowState( const OString& rStr );//this method inherit from SystemWindow
};
//add for DropDownFieldDialog end


class AbstractSwLabDlg_Impl  : public AbstractSwLabDlg
{
    DECL_ABSTDLG_BASE(AbstractSwLabDlg_Impl,SwLabDlg)
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual void                SetCurPageId( const OString &rName );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
    virtual const OUString& GetBusinessCardStr() const;
    virtual Printer *GetPrt();
};
//add for SwLabDlg end

//add for SwSelGlossaryDlg begin
class AbstractSwSelGlossaryDlg_Impl : public AbstractSwSelGlossaryDlg
{
    DECL_ABSTDLG_BASE(AbstractSwSelGlossaryDlg_Impl,SwSelGlossaryDlg)
    virtual void InsertGlos(const String &rRegion, const String &rGlosName);    // inline
    virtual sal_uInt16 GetSelectedIdx() const;  // inline
    virtual void SelectEntryPos(sal_uInt16 nIdx);   // inline
};
//add for SwSelGlossaryDlg end

//add for SwAutoFormatDlg begin
class AbstractSwAutoFormatDlg_Impl : public AbstractSwAutoFormatDlg
{
    DECL_ABSTDLG_BASE(AbstractSwAutoFormatDlg_Impl,SwAutoFormatDlg )
    virtual void FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const;
};
//add for SwAutoFormatDlg end

//add for SwFldDlg begin

class AbstractSwFldDlg_Impl : public AbstractSwFldDlg //add for SwFldDlg
{
    DECL_ABSTDLG_BASE(AbstractSwFldDlg_Impl,SwFldDlg )
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual void                SetCurPageId( const OString &rName );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
    virtual void                Start( sal_Bool bShow = sal_True );  //this method from SfxTabDialog
    virtual void                ShowPage( sal_uInt16 nId );// this method from SfxTabDialog
    virtual void                Initialize(SfxChildWinInfo *pInfo);
    virtual void                ReInitDlg();
    virtual void                ActivateDatabasePage();
    virtual Window *            GetWindow(); //this method is added for return a Window type pointer
};
//add for SwFldD end

//add for SwRenameXNamedDlg begin
class AbstractSwRenameXNamedDlg_Impl : public AbstractSwRenameXNamedDlg
{
    DECL_ABSTDLG_BASE(AbstractSwRenameXNamedDlg_Impl,SwRenameXNamedDlg )
    virtual void    SetForbiddenChars( const String& rSet );
    virtual void SetAlternativeAccess(
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xSecond,
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xThird );
};
//add for SwRenameXNamedDlg end
//add for SwModalRedlineAcceptDlg begin
class AbstractSwModalRedlineAcceptDlg_Impl : public AbstractSwModalRedlineAcceptDlg
{
    DECL_ABSTDLG_BASE(AbstractSwModalRedlineAcceptDlg_Impl,SwModalRedlineAcceptDlg )
    virtual void            AcceptAll( sal_Bool bAccept );
};
//add for SwModalRedlineAcceptDlg end

//for SwGlossaryDlg begin
class SwGlossaryDlg;
class AbstractGlossaryDlg_Impl : public AbstractGlossaryDlg
{
    DECL_ABSTDLG_BASE(AbstractGlossaryDlg_Impl,SwGlossaryDlg)
    virtual String          GetCurrGrpName() const;
    virtual String          GetCurrShortName() const;
};
//for SwGlossaryDlg end

//for SwFldInputDlg begin
class SwFldInputDlg;
class AbstractFldInputDlg_Impl : public AbstractFldInputDlg
{
    DECL_ABSTDLG_BASE(AbstractFldInputDlg_Impl,SwFldInputDlg)
    //from class SalFrame
    virtual void         SetWindowState( const OString & rStr ) ;
    virtual OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const ;
};
//for SwFldInputDlg end

//for SwInsFootNoteDlg begin
class SwInsFootNoteDlg;
class AbstractInsFootNoteDlg_Impl : public AbstractInsFootNoteDlg
{
    DECL_ABSTDLG_BASE(AbstractInsFootNoteDlg_Impl,SwInsFootNoteDlg)
    virtual String          GetFontName();
    virtual sal_Bool            IsEndNote();
    virtual String          GetStr();
    //from class Window
    virtual void    SetHelpId( const OString& sHelpId );
    virtual void    SetText( const OUString& rStr );
};
//for SwInsFootNoteDlg end

//for SwInsTableDlg begin
class SwInsTableDlg;
class AbstractInsTableDlg_Impl : public AbstractInsTableDlg
{
    DECL_ABSTDLG_BASE(AbstractInsTableDlg_Impl,SwInsTableDlg)
    virtual void            GetValues( String& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTblFlags, String& rTableAutoFmtName,
                                SwTableAutoFmt *& prTAFmt );
};
//for SwInsTableDlg end

//for SwJavaEditDialog begin
class SwJavaEditDialog;
class AbstractJavaEditDialog_Impl : public AbstractJavaEditDialog
{
    DECL_ABSTDLG_BASE(AbstractJavaEditDialog_Impl,SwJavaEditDialog)
    virtual OUString                GetText();
    virtual String              GetType();
    virtual sal_Bool                IsUrl();
    virtual sal_Bool                IsNew();
    virtual bool                IsUpdate();
};
//for SwJavaEditDialog end

//for SwMailMergeDlg begin
class SwMailMergeDlg;
class AbstractMailMergeDlg_Impl : public AbstractMailMergeDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeDlg_Impl,SwMailMergeDlg)
    virtual sal_uInt16  GetMergeType() ;
    virtual const OUString& GetSaveFilter() const;
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetSelection() const ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> GetResultSet() const;
    virtual bool IsSaveIndividualDocs() const;
    virtual bool IsGenerateFromDataBase() const;
    virtual String GetColumnName() const;
    virtual String GetPath() const;
};
//for SwMailMergeDlg end
//for SwMailMergeCreateFromDlg begin
class SwMailMergeCreateFromDlg;
class AbstractMailMergeCreateFromDlg_Impl : public AbstractMailMergeCreateFromDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeCreateFromDlg_Impl,SwMailMergeCreateFromDlg)
    virtual sal_Bool    IsThisDocument() const ;
};
//for SwMailMergeCreateFromDlg end
//for SwMailMergeFieldConnectionsDlg begin
class SwMailMergeFieldConnectionsDlg;
class AbstractMailMergeFieldConnectionsDlg_Impl : public AbstractMailMergeFieldConnectionsDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeFieldConnectionsDlg_Impl,SwMailMergeFieldConnectionsDlg)
    virtual sal_Bool    IsUseExistingConnections() const ;
};
//for SwMailMergeFieldConnectionsDlg end

//for SwMultiTOXTabDialog begin
class SwMultiTOXTabDialog;
class AbstractMultiTOXTabDialog_Impl : public AbstractMultiTOXTabDialog
{
    DECL_ABSTDLG_BASE(AbstractMultiTOXTabDialog_Impl,SwMultiTOXTabDialog)
    virtual SwForm*             GetForm(CurTOXType eType);
    virtual CurTOXType          GetCurrentTOXType() const ;
    virtual SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes);
    //from SfxTabDialog
    virtual const SfxItemSet*   GetOutputItemSet() const;
};
//for SwMultiTOXTabDialog end

//for SwEditRegionDlg begin
class SwEditRegionDlg;
class AbstractEditRegionDlg_Impl : public AbstractEditRegionDlg
{
    DECL_ABSTDLG_BASE(AbstractEditRegionDlg_Impl,SwEditRegionDlg)
    virtual void    SelectSection(const String& rSectionName);
};
//for SwEditRegionDlg end
//for SwInsertSectionTabDialog begin
class SwInsertSectionTabDialog;
class AbstractInsertSectionTabDialog_Impl : public AbstractInsertSectionTabDialog
{
    DECL_ABSTDLG_BASE(AbstractInsertSectionTabDialog_Impl,SwInsertSectionTabDialog)
    virtual void        SetSectionData(SwSectionData const& rSect);
};
//for SwInsertSectionTabDialog end

//for SwIndexMarkFloatDlg begin
class SwIndexMarkFloatDlg;
class AbstractIndexMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractIndexMarkFloatDlg_Impl,SwIndexMarkFloatDlg)
    virtual void    ReInitDlg(SwWrtShell& rWrtShell);
    virtual Window *            GetWindow(); //this method is added for return a Window type pointer
};
//for SwIndexMarkFloatDlg end

//for SwAuthMarkFloatDlg begin
class SwAuthMarkFloatDlg;
class AbstractAuthMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractAuthMarkFloatDlg_Impl,SwAuthMarkFloatDlg)
    virtual void    ReInitDlg(SwWrtShell& rWrtShell);
    virtual Window *            GetWindow(); //this method is added for return a Window type pointer
};
//for SwAuthMarkFloatDlg end

class SwMailMergeWizard;
class AbstractMailMergeWizard_Impl : public AbstractMailMergeWizard
{
    SwMailMergeWizard* pDlg;
    Link               aEndDlgHdl;

    DECL_LINK( EndDialogHdl, SwMailMergeWizard* );
public:
                    AbstractMailMergeWizard_Impl( SwMailMergeWizard* p )
                     : pDlg(p)
                     {}
    virtual         ~AbstractMailMergeWizard_Impl();
    virtual void    StartExecuteModal( const Link& rEndDialogHdl );
    virtual long    GetResult();

    virtual void                SetReloadDocument(const String& rURL);
    virtual const String&       GetReloadDocument() const;
    virtual sal_Bool                ShowPage( sal_uInt16 nLevel );
    virtual sal_uInt16          GetRestartPage() const;
};

//------------------------------------------------------------------------
//AbstractDialogFactory_Impl implementations
class SwAbstractDialogFactory_Impl : public SwAbstractDialogFactory
{

public:
    virtual ~SwAbstractDialogFactory_Impl() {}

    virtual SfxAbstractDialog*              CreateSfxDialog( Window* pParent, //add for SvxMeasureDialog & SvxConnectionDialog
                                                                        const SfxItemSet& rAttr,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame,
                                                                        sal_uInt32 nResId
                                                                        );
    virtual AbstractSwWordCountFloatDlg* CreateSwWordCountDialog(SfxBindings* pBindings,
        SfxChildWindow* pChild, Window *pParent, SfxChildWinInfo* pInfo);
    virtual AbstractSwInsertAbstractDlg * CreateSwInsertAbstractDlg(Window* pParent);
    virtual AbstractSwAsciiFilterDlg*  CreateSwAsciiFilterDlg ( Window* pParent, SwDocShell& rDocSh,
                                                                SvStream* pStream ); //add for SwAsciiFilterDlg
    virtual VclAbstractDialog * CreateSwInsertBookmarkDlg( Window *pParent, SwWrtShell &rSh, SfxRequest& rReq, int nResId );//add for SwInsertBookmarkDlg
    virtual AbstractSwBreakDlg * CreateSwBreakDlg(Window *pParent, SwWrtShell &rSh); // add for SwBreakDlg
    virtual VclAbstractDialog   * CreateSwChangeDBDlg(SwView& rVw); //add for SwChangeDBDlg
    virtual SfxAbstractTabDialog *  CreateSwCharDlg(Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
        sal_uInt8 nDialogMode, const String* pFmtStr = 0); // add for SwCharDlg
    virtual AbstractSwConvertTableDlg* CreateSwConvertTableDlg(SwView& rView, bool bToTable); //add for SwConvertTableDlg
    virtual VclAbstractDialog * CreateSwCaptionDialog ( Window *pParent, SwView &rV,int nResId); //add for SwCaptionDialog

    virtual AbstractSwInsertDBColAutoPilot* CreateSwInsertDBColAutoPilot( SwView& rView, // add for SwInsertDBColAutoPilot
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        com::sun::star::uno::Reference<com::sun::star::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData,  int nResId);
    virtual SfxAbstractTabDialog * CreateSwFootNoteOptionDlg(Window *pParent, SwWrtShell &rSh);//add for  SwFootNoteOptionDlg

    //add for DropDownFieldDialog
    virtual AbstractDropDownFieldDialog * CreateDropDownFieldDialog (Window *pParent, SwWrtShell &rSh,
        SwField* pField, sal_Bool bNextButton = sal_False);
    virtual SfxAbstractTabDialog* CreateSwEnvDlg ( Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, sal_Bool bInsert,int nResId ); //add for SwEnvDlg
    virtual AbstractSwLabDlg* CreateSwLabDlg(Window* pParent, const SfxItemSet& rSet, //add for SwLabDlg
                                                     SwNewDBMgr* pNewDBMgr, bool bLabel);

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod (); //add for SwLabDlg
    virtual SfxAbstractTabDialog* CreateSwParaDlg ( Window *pParent, //add for SwParaDlg
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    sal_uInt8 nDialogMode,
                                                    const String *pCollName = 0,
                                                    sal_Bool bDraw = sal_False,
                                                    OString sDefPage = OString() );

    virtual AbstractSwSelGlossaryDlg * CreateSwSelGlossaryDlg(Window * pParent, const String &rShortName); //add for SwSelGlossaryDlg
    virtual VclAbstractDialog * CreateVclAbstractDialog ( Window * pParent, SwWrtShell &rSh, int nResId ); //add for  SwTableHeightDlg SwSortDlg
    virtual AbstractSplitTableDialog * CreateSplitTblDialog ( Window * pParent, SwWrtShell &rSh ); //add for  SwSplitTblDlg

    virtual AbstractSwAutoFormatDlg * CreateSwAutoFormatDlg( Window* pParent, SwWrtShell* pShell, //add for SwAutoFormatDlg
                                                            sal_Bool bSetAutoFmt = sal_True,
                                                            const SwTableAutoFmt* pSelFmt = 0 );

    virtual SfxAbstractDialog * CreateSwWrapDlg ( Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, sal_Bool bDrawMode, int nResId ); //add for SwWrapDlg
    virtual VclAbstractDialog * CreateSwTableWidthDlg(Window *pParent, SwTableFUNC &rFnc);
    virtual SfxAbstractTabDialog* CreateSwTableTabDlg(Window* pParent, SfxItemPool& Pool,
        const SfxItemSet* pItemSet, SwWrtShell* pSh); //add for SwTableTabDlg
    virtual AbstractSwFldDlg * CreateSwFldDlg ( SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent, int nResId ); //add for SwFldDlg
    virtual SfxAbstractDialog*   CreateSwFldEditDlg ( SwView& rVw, int nResId ); //add for SwFldEditDlg
    virtual AbstractSwRenameXNamedDlg * CreateSwRenameXNamedDlg(Window* pParent, //add for SwRenameXNamedDlg
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > & xNamed,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xNameAccess);
    virtual AbstractSwModalRedlineAcceptDlg * CreateSwModalRedlineAcceptDlg(Window *pParent); //add for SwModalRedlineAcceptDlg

    virtual VclAbstractDialog*          CreateTblMergeDialog(Window* pParent, sal_Bool& rWithPrev); //add for SwMergeTblDlg
    virtual SfxAbstractTabDialog*       CreateFrmTabDialog( int nResId,
                                                SfxViewFrame *pFrame, Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                sal_Bool            bNewFrm  = sal_True,
                                                sal_uInt16          nResType = DLG_FRM_STD,
                                                sal_Bool            bFmt     = sal_False,
                                                sal_uInt16          nDefPage = 0,
                                                const String*   pFmtStr  = 0); //add for SwFrmDlg
    virtual SfxAbstractApplyTabDialog*  CreateTemplateDialog(
                                                Window*             pParent,
                                                SfxStyleSheetBase&  rBase,
                                                sal_uInt16          nRegion,
                                                OString             sPage = OString(),
                                                SwWrtShell*         pActShell = 0,
                                                bool                bNew = false); //add for SwTemplateDlg
    virtual AbstractGlossaryDlg*        CreateGlossaryDlg(SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell); //add for SwGlossaryDlg
    virtual AbstractFldInputDlg*        CreateFldInputDlg(Window *pParent,
        SwWrtShell &rSh, SwField* pField, bool bNextButton = false); //add for SwFldInputDlg
    virtual AbstractInsFootNoteDlg*     CreateInsFootNoteDlg(
        Window * pParent, SwWrtShell &rSh, sal_Bool bEd = sal_False); //add for SwInsFootNoteDlg
    virtual VclAbstractDialog *         CreateTitlePageDlg ( Window * pParent );
    virtual VclAbstractDialog *         CreateVclSwViewDialog(SwView& rView); //add for SwInsRowColDlg, SwLineNumberingDlg
    virtual AbstractInsTableDlg*        CreateInsTableDlg(SwView& rView); //add for SwInsTableDlg
    virtual AbstractJavaEditDialog*     CreateJavaEditDialog(Window* pParent,
        SwWrtShell* pWrtSh); //add for SwJavaEditDialog
    virtual AbstractMailMergeDlg*       CreateMailMergeDlg( int nResId,
                                                Window* pParent, SwWrtShell& rSh,
                                                 const String& rSourceName,
                                                const String& rTblName,
                                                sal_Int32 nCommandType,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection,
                                                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >* pSelection = 0 ); //add for SwMailMergeDlg
    virtual AbstractMailMergeCreateFromDlg*     CreateMailMergeCreateFromDlg(Window* pParent);
    virtual AbstractMailMergeFieldConnectionsDlg*       CreateMailMergeFieldConnectionsDlg( int nResId,
                                                            Window* pParent ); //add for SwMailMergeFieldConnectionsDlg
    virtual VclAbstractDialog*          CreateMultiTOXMarkDlg( int nResId,
                                                Window* pParent, SwTOXMgr &rTOXMgr ); //add for SwMultiTOXMarkDlg
    virtual SfxAbstractTabDialog*       CreateSwTabDialog( int nResId,
                                                Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &); //add for SwSvxNumBulletTabDialog, SwOutlineTabDialog
    virtual AbstractMultiTOXTabDialog*      CreateMultiTOXTabDialog( int nResId,
                                                Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX, sal_uInt16 nToxType = USHRT_MAX,
                                                sal_Bool bGlobal = sal_False); //add for SwMultiTOXTabDialog
    virtual AbstractEditRegionDlg*      CreateEditRegionDlg(Window* pParent, SwWrtShell& rWrtSh); //add for SwEditRegionDlg
    virtual AbstractInsertSectionTabDialog*     CreateInsertSectionTabDialog(
        Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh); //add for SwInsertSectionTabDialog
    virtual AbstractMarkFloatDlg*       CreateIndexMarkFloatDlg(
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew=sal_True); //add for SwIndexMarkFloatDlg
    virtual AbstractMarkFloatDlg*       CreateAuthMarkFloatDlg(
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew=sal_True); //add for SwAuthMarkFloatDlg
    virtual VclAbstractDialog *         CreateIndexMarkModalDlg(
                                                Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark ); //add for SwIndexMarkModalDlg

    virtual AbstractMailMergeWizard*    CreateMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rConfigItem);

    //add for static func in SwGlossaryDlg
    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc();
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc();

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId );

    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
