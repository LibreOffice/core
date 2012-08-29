/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SW_DLGFACT_HXX
#define _SW_DLGFACT_HXX

#include "swabstdlg.hxx"

class SwInsertAbstractDlg;
class SfxSingleTabDialog;
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
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual short   Execute() ;
//  virtual void    Show( sal_Bool bVisible = sal_True, sal_uInt16 nFlags = 0 )

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

// add for SwAddrDlg, SwDropCapsDlg, SwBackgroundDlg SwNumFmtDlg  SwWrapDlg SwBorderDlg, SwFldEditDlg  begin
class SfxSingleTabDialog;
class AbstractSfxDialog_Impl :public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(AbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};
// add for SwAddrDlg,SwDropCapsDlg , SwBackgroundDlg  SwNumFmtDlg SwWrapDlg SwBorderDlg, SwFldEditDlg  end

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
class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};
//add for SwCharDlg, SwEnvDlg ,SwFootNoteOptionDlg SwParaDlg  SwTableTabDlg end

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
    virtual rtl::OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const; //this method inherit from SystemWindow
    virtual void         SetWindowState( const rtl::OString& rStr );//this method inherit from SystemWindow
};
//add for DropDownFieldDialog end


class AbstractSwLabDlg_Impl  : public AbstractSwLabDlg
{
    DECL_ABSTDLG_BASE(AbstractSwLabDlg_Impl,SwLabDlg)
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
     virtual const String& GetBusinessCardStr() const;
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
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
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
    virtual void         SetWindowState( const rtl::OString & rStr ) ;
    virtual rtl::OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const ;
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
    virtual void    SetHelpId( const rtl::OString& sHelpId );
    virtual void    SetText( const XubString& rStr );
};
//for SwInsFootNoteDlg end

//for SwInsertGrfRulerDlg begin
class SwInsertGrfRulerDlg;
class AbstractInsertGrfRulerDlg_Impl : public AbstractInsertGrfRulerDlg
{
    DECL_ABSTDLG_BASE(AbstractInsertGrfRulerDlg_Impl,SwInsertGrfRulerDlg)
    virtual String          GetGraphicName();
    virtual sal_Bool            IsSimpleLine();
    virtual sal_Bool            HasImages() const ;
};
//for SwInsertGrfRulerDlg end

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
    virtual String              GetText();
    virtual String              GetType();
    virtual sal_Bool                IsUrl();
    virtual sal_Bool                IsNew();
    virtual sal_Bool                IsUpdate();
};
//for SwJavaEditDialog end

//for SwMailMergeDlg begin
class SwMailMergeDlg;
class AbstractMailMergeDlg_Impl : public AbstractMailMergeDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeDlg_Impl,SwMailMergeDlg)
    virtual sal_uInt16  GetMergeType() ;
    virtual const ::rtl::OUString& GetSaveFilter() const;
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
    virtual AbstractSwInsertAbstractDlg * CreateSwInsertAbstractDlg( Window* pParent,int nResId );
    virtual AbstractSwAsciiFilterDlg*  CreateSwAsciiFilterDlg ( Window* pParent, SwDocShell& rDocSh,
                                                                SvStream* pStream, int nResId ); //add for SwAsciiFilterDlg
    virtual VclAbstractDialog * CreateSwInsertBookmarkDlg( Window *pParent, SwWrtShell &rSh, SfxRequest& rReq, int nResId );//add for SwInsertBookmarkDlg
    virtual AbstractSwBreakDlg * CreateSwBreakDlg ( Window *pParent, SwWrtShell &rSh,int nResId ); // add for SwBreakDlg
    virtual VclAbstractDialog   * CreateSwChangeDBDlg( SwView& rVw, int nResId  ); //add for SwChangeDBDlg
    virtual SfxAbstractTabDialog *  CreateSwCharDlg( Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet, int nResId, // add for SwCharDlg
                                                const String* pFmtStr = 0, sal_Bool bIsDrwTxtDlg = sal_False);
    virtual AbstractSwConvertTableDlg* CreateSwConvertTableDlg ( SwView& rView, int nResId, bool bToTable ); //add for SwConvertTableDlg
    virtual VclAbstractDialog * CreateSwCaptionDialog ( Window *pParent, SwView &rV,int nResId); //add for SwCaptionDialog

    virtual AbstractSwInsertDBColAutoPilot* CreateSwInsertDBColAutoPilot( SwView& rView, // add for SwInsertDBColAutoPilot
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        com::sun::star::uno::Reference<com::sun::star::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData,  int nResId);
    virtual SfxAbstractTabDialog * CreateSwFootNoteOptionDlg( Window *pParent, SwWrtShell &rSh,int nResId);//add for  SwFootNoteOptionDlg

    virtual AbstractDropDownFieldDialog * CreateDropDownFieldDialog ( Window *pParent, SwWrtShell &rSh, //add for DropDownFieldDialog
                                SwField* pField,int nResId, sal_Bool bNextButton = sal_False );
    virtual SfxAbstractTabDialog* CreateSwEnvDlg ( Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, sal_Bool bInsert,int nResId ); //add for SwEnvDlg
    virtual AbstractSwLabDlg* CreateSwLabDlg ( Window* pParent, const SfxItemSet& rSet, //add for SwLabDlg
                                                     SwNewDBMgr* pNewDBMgr, sal_Bool bLabel,int nResId  );

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod (); //add for SwLabDlg
    virtual SfxAbstractTabDialog* CreateSwParaDlg ( Window *pParent, //add for SwParaDlg
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    sal_uInt8 nDialogMode,
                                                    int nResId,
                                                    const String *pCollName = 0,
                                                    sal_Bool bDraw = sal_False,
                                                    sal_uInt16 nDefPage = 0);

    virtual AbstractSwSelGlossaryDlg * CreateSwSelGlossaryDlg ( Window * pParent, const String &rShortName, int nResId ); //add for SwSelGlossaryDlg
    virtual VclAbstractDialog * CreateVclAbstractDialog ( Window * pParent, SwWrtShell &rSh, int nResId ); //add for  SwTableHeightDlg SwSortDlg
    virtual AbstractSplitTableDialog * CreateSplitTblDialog ( Window * pParent, SwWrtShell &rSh ); //add for  SwSplitTblDlg

    virtual AbstractSwAutoFormatDlg * CreateSwAutoFormatDlg( Window* pParent, SwWrtShell* pShell, //add for SwAutoFormatDlg
                                                            int nResId,
                                                            sal_Bool bSetAutoFmt = sal_True,
                                                            const SwTableAutoFmt* pSelFmt = 0 );
    virtual SfxAbstractDialog * CreateSwBorderDlg (Window* pParent, SfxItemSet& rSet, sal_uInt16 nType,int nResId );//add for SwBorderDlg

    virtual SfxAbstractDialog * CreateSwWrapDlg ( Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, sal_Bool bDrawMode, int nResId ); //add for SwWrapDlg
    virtual VclAbstractDialog * CreateSwTableWidthDlg ( Window *pParent, SwTableFUNC &rFnc , int nResId ); //add for SwTableWidthDlg
    virtual SfxAbstractTabDialog* CreateSwTableTabDlg( Window* pParent, SfxItemPool& Pool,
                                                        const SfxItemSet* pItemSet, SwWrtShell* pSh,int nResId ); //add for SwTableTabDlg
    virtual AbstractSwFldDlg * CreateSwFldDlg ( SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent, int nResId ); //add for SwFldDlg
    virtual SfxAbstractDialog*   CreateSwFldEditDlg ( SwView& rVw, int nResId ); //add for SwFldEditDlg
    virtual AbstractSwRenameXNamedDlg * CreateSwRenameXNamedDlg( Window* pParent, //add for SwRenameXNamedDlg
                                                                ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > & xNamed,
                                                                ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xNameAccess, int nResId );
    virtual AbstractSwModalRedlineAcceptDlg * CreateSwModalRedlineAcceptDlg ( Window *pParent, int nResId ); //add for SwModalRedlineAcceptDlg

    virtual VclAbstractDialog*          CreateSwVclDialog( int nResId,
                                                Window* pParent, sal_Bool& rWithPrev ); //add for SwMergeTblDlg
    virtual SfxAbstractTabDialog*       CreateFrmTabDialog( int nResId,
                                                SfxViewFrame *pFrame, Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                sal_Bool            bNewFrm  = sal_True,
                                                sal_uInt16          nResType = DLG_FRM_STD,
                                                sal_Bool            bFmt     = sal_False,
                                                sal_uInt16          nDefPage = 0,
                                                const String*   pFmtStr  = 0); //add for SwFrmDlg
    virtual SfxAbstractTabDialog*       CreateTemplateDialog( int nResId,
                                                Window*             pParent,
                                                SfxStyleSheetBase&  rBase,
                                                sal_uInt16              nRegion,
                                                sal_uInt16          nPageId = 0,
                                                SwWrtShell*         pActShell = 0,
                                                sal_Bool                bNew = sal_False ); //add for SwTemplateDlg
    virtual AbstractGlossaryDlg*        CreateGlossaryDlg( int nResId,
                                                SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell); //add for SwGlossaryDlg
    virtual AbstractFldInputDlg*        CreateFldInputDlg( int nResId,
                                                Window *pParent, SwWrtShell &rSh,
                                                SwField* pField, sal_Bool bNextButton = sal_False ); //add for SwFldInputDlg
    virtual AbstractInsFootNoteDlg*     CreateInsFootNoteDlg( int nResId,
                                                Window * pParent, SwWrtShell &rSh, sal_Bool bEd = sal_False); //add for SwInsFootNoteDlg
   virtual VclAbstractDialog *         CreateTitlePageDlg ( Window * pParent );
    virtual VclAbstractDialog *         CreateVclSwViewDialog( int nResId,
                                                SwView& rView, sal_Bool bCol = sal_False ); //add for SwInsRowColDlg, SwLineNumberingDlg
    virtual AbstractInsertGrfRulerDlg*      CreateInsertGrfRulerDlg( int nResId,
                                                Window * pParent ); //add for SwInsertGrfRulerDlg
    virtual AbstractInsTableDlg*        CreateInsTableDlg( int nResId,
                                                SwView& rView ); //add for SwInsTableDlg
    virtual AbstractJavaEditDialog*     CreateJavaEditDialog( int nResId,
                                                Window* pParent, SwWrtShell* pWrtSh ); //add for SwJavaEditDialog
    virtual AbstractMailMergeDlg*       CreateMailMergeDlg( int nResId,
                                                Window* pParent, SwWrtShell& rSh,
                                                 const String& rSourceName,
                                                const String& rTblName,
                                                sal_Int32 nCommandType,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection,
                                                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >* pSelection = 0 ); //add for SwMailMergeDlg
    virtual AbstractMailMergeCreateFromDlg*     CreateMailMergeCreateFromDlg( int nResId,
                                                    Window* pParent ); //add for SwMailMergeCreateFromDlg
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
    virtual AbstractEditRegionDlg*      CreateEditRegionDlg( int nResId,
                                                Window* pParent, SwWrtShell& rWrtSh ); //add for SwEditRegionDlg
    virtual AbstractInsertSectionTabDialog*     CreateInsertSectionTabDialog( int nResId,
                                                    Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh); //add for SwInsertSectionTabDialog
    virtual AbstractMarkFloatDlg*       CreateIndexMarkFloatDlg( int nResId,
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew=sal_True); //add for SwIndexMarkFloatDlg
    virtual AbstractMarkFloatDlg*       CreateAuthMarkFloatDlg( int nResId,
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew=sal_True); //add for SwAuthMarkFloatDlg
    virtual VclAbstractDialog *         CreateIndexMarkModalDlg( int nResId,
                                                Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark ); //add for SwIndexMarkModalDlg

    virtual AbstractMailMergeWizard*    CreateMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rConfigItem);

    //add for static func in SwGlossaryDlg
    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc( sal_uInt16 nId );
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc( sal_uInt16 nId );

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId );

    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
