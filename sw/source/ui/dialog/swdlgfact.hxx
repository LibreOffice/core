/*************************************************************************
 *
 *  $RCSfile: swdlgfact.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2004-05-13 12:29:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SW_DLGFACT_HXX
#define _SW_DLGFACT_HXX

// include ---------------------------------------------------------------
#include "swabstdlg.hxx" //CHINA001

class SwSaveLabelDlg;
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
class SwSplitTableDlg;
class SwAutoFormatDlg;
class SwFldDlg;
class SwRenameXNamedDlg;
class SwModalRedlineAcceptDlg;
class SwTOXMark;
class SwWordCountDialog;

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
    virtual USHORT  Execute() ;
//  virtual void    Show( BOOL bVisible = TRUE, USHORT nFlags = 0 )

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
USHORT Class::Execute()                             \
{                                                   \
    return pDlg->Execute();                         \
}


//CHINA001 class AbstractSwSaveLabelDlg_Impl : public AbstractSwSaveLabelDlg    //CHINA001 add for SwSaveLabelDlg
//CHINA001 {
//CHINA001 DECL_ABSTDLG_BASE(AbstractSwSaveLabelDlg_Impl,SwSaveLabelDlg);
//CHINA001 virtual void SetLabel(const rtl::OUString& rMake, const rtl::OUString& rType);
//CHINA001 virtual sal_Bool GetLabel(SwLabItem& rItem);
//CHINA001 }

class AbstractSwWordCountDialog_Impl : public AbstractSwWordCountDialog
{
    DECL_ABSTDLG_BASE(AbstractSwWordCountDialog_Impl,SwWordCountDialog);
    void    SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc);
};

//add for SwInsertAbstractDlg begin
class AbstractSwInsertAbstractDlg_Impl : public AbstractSwInsertAbstractDlg
{
    DECL_ABSTDLG_BASE(AbstractSwInsertAbstractDlg_Impl,SwInsertAbstractDlg);
    virtual BYTE    GetLevel() const ;
    virtual BYTE    GetPara() const ;
};

//add for SwInsertAbstractDlg end

// add for SwAddrDlg, SwDropCapsDlg, SwBackgroundDlg SwNumFmtDlg  SwWrapDlg SwBorderDlg, SwFldEditDlg  begin
class SfxSingleTabDialog;
class AbstractSfxSingleTabDialog_Impl :public AbstractSfxSingleTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl,SfxSingleTabDialog);
    virtual const SfxItemSet*   GetOutputItemSet() const;

};
// add for SwAddrDlg,SwDropCapsDlg , SwBackgroundDlg  SwNumFmtDlg SwWrapDlg SwBorderDlg, SwFldEditDlg  end

// add for SwAsciiFilterDlg begin
class AbstractSwAsciiFilterDlg_Impl : public AbstractSwAsciiFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractSwAsciiFilterDlg_Impl,SwAsciiFilterDlg );
    virtual void FillOptions( SwAsciiOptions& rOptions );

};
// add for SwAsciiFilterDlg end

// add for SwInsertBookmarkDlg  SwChangeDBDlg, SwTableHeightDlg, SwSplitTblDlg  SwSortDlg   SwTableWidthDlgbegin
class VclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog);
};
// add for SwInsertBookmarkDlg SwChangeDBDlg, SwTableHeightDlg SwSplitTblDlg  SwSortDlg   SwTableWidthDlg end

// add for SwBreakDlg begin
class AbstractSwBreakDlg_Impl : public AbstractSwBreakDlg // add for SwBreakDlg
{
    DECL_ABSTDLG_BASE(AbstractSwBreakDlg_Impl,SwBreakDlg);
    virtual String  GetTemplateName();
    virtual USHORT  GetKind();
    virtual USHORT  GetPageNumber();

};
// add for SwBreakDlg end

//add for SwCharDlg , SwEnvDlg , SwFootNoteOptionDlg SwParaDlg  SwTableTabDlg begin
class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog );
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem ); //add by CHINA001
    virtual void                SetInputSet( const SfxItemSet* pInSet );   //add by CHINA001
        //From class Window.
    virtual void        SetText( const XubString& rStr ); //add by CHINA001
    virtual String      GetText() const; //add by CHINA001
};
//add for SwCharDlg, SwEnvDlg ,SwFootNoteOptionDlg SwParaDlg  SwTableTabDlg end

//add for SwConvertTableDlg begin
class AbstractSwConvertTableDlg_Impl :  public AbstractSwConvertTableDlg // add for SwConvertTableDlg
{
    DECL_ABSTDLG_BASE( AbstractSwConvertTableDlg_Impl,SwConvertTableDlg);
    virtual void GetValues( sal_Unicode& rDelim,SwInsertTableOptions& rInsTblFlags,
                    SwTableAutoFmt *& prTAFmt );
};
//add for SwConvertTableDlg end

//add for SwInsertDBColAutoPilot begin
class AbstractSwInsertDBColAutoPilot_Impl :  public AbstractSwInsertDBColAutoPilot // add for SwInsertDBColAutoPilot
{
    DECL_ABSTDLG_BASE( AbstractSwInsertDBColAutoPilot_Impl,SwInsertDBColAutoPilot);
    virtual void DataToDoc( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rSelection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet);
};
//add for SwInsertDBColAutoPilot end

//add for DropDownFieldDialog begin
class AbstractDropDownFieldDialog_Impl : public AbstractDropDownFieldDialog //add for DropDownFieldDialog
{
    DECL_ABSTDLG_BASE(AbstractDropDownFieldDialog_Impl, sw::DropDownFieldDialog);
    virtual ByteString      GetWindowState( ULONG nMask = WINDOWSTATE_MASK_ALL ) const; //this method inherit from SystemWindow
    virtual void            SetWindowState( const ByteString& rStr );//this method inherit from SystemWindow
};
//add for DropDownFieldDialog end


class AbstarctSwLabDlg_Impl  : public AbstarctSwLabDlg
{
    DECL_ABSTDLG_BASE(AbstarctSwLabDlg_Impl,SwLabDlg);
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem ); //add by CHINA001
    virtual void                SetInputSet( const SfxItemSet* pInSet );   //add by CHINA001
        //From class Window.
    virtual void        SetText( const XubString& rStr ); //add by CHINA001
    virtual String      GetText() const; //add by CHINA001
     virtual const String& GetBusinessCardStr() const;
     virtual Printer *GetPrt();
     virtual void MakeConfigItem(SwLabItem& rItem) const;
};
//add for SwLabDlg end

//add for SwSelGlossaryDlg begin
class AbstarctSwSelGlossaryDlg_Impl : public AbstarctSwSelGlossaryDlg
{
    DECL_ABSTDLG_BASE(AbstarctSwSelGlossaryDlg_Impl,SwSelGlossaryDlg);
    virtual void InsertGlos(const String &rRegion, const String &rGlosName);    // inline
    virtual USHORT GetSelectedIdx() const;  // inline
    virtual void SelectEntryPos(USHORT nIdx);   // inline
};
//add for SwSelGlossaryDlg end

//add for SwSplitTableDlg begin
class AbstractSwSplitTableDlg_Impl :public AbstractSwSplitTableDlg
{
    DECL_ABSTDLG_BASE(AbstractSwSplitTableDlg_Impl,SwSplitTableDlg);
    virtual BOOL                IsHorizontal() const;
    virtual BOOL                IsProportional() const;
    virtual long                GetCount() const;
};
//add for SwSplitTableDlg end

//add for SwAutoFormatDlg begin
class AbstractSwAutoFormatDlg_Impl : public AbstractSwAutoFormatDlg
{
    DECL_ABSTDLG_BASE(AbstractSwAutoFormatDlg_Impl,SwAutoFormatDlg );
    virtual void FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const;
};
//add for SwAutoFormatDlg end

//add for SwFldDlg begin

class AbstractSwFldDlg_Impl : public AbstractSwFldDlg //add for SwFldDlg
{
    DECL_ABSTDLG_BASE(AbstractSwFldDlg_Impl,SwFldDlg );
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem ); //add by CHINA001
    virtual void                SetInputSet( const SfxItemSet* pInSet );   //add by CHINA001
        //From class Window.
    virtual void        SetText( const XubString& rStr ); //add by CHINA001
    virtual String      GetText() const; //add by CHINA001
    virtual void                Start( BOOL bShow = TRUE );  //this method from SfxTabDialog
    virtual void                ShowPage( USHORT nId );// this method from SfxTabDialog
    virtual void                Initialize(SfxChildWinInfo *pInfo);
    virtual void                ReInitDlg();
    virtual void                ActivateDatabasePage();
    virtual Window *            GetWindow(); //this method is added for return a Window type pointer
};
//add for SwFldD end

//add for SwRenameXNamedDlg begin
class AbstractSwRenameXNamedDlg_Impl : public AbstractSwRenameXNamedDlg
{
    DECL_ABSTDLG_BASE(AbstractSwRenameXNamedDlg_Impl,SwRenameXNamedDlg );
    virtual void    SetForbiddenChars( const String& rSet );
    virtual void SetAlternativeAccess(
             STAR_REFERENCE( container::XNameAccess ) & xSecond,
             STAR_REFERENCE( container::XNameAccess ) & xThird );
};
//add for SwRenameXNamedDlg end
//add for SwModalRedlineAcceptDlg begin
class AbstractSwModalRedlineAcceptDlg_Impl : public AbstractSwModalRedlineAcceptDlg
{
    DECL_ABSTDLG_BASE(AbstractSwModalRedlineAcceptDlg_Impl,SwModalRedlineAcceptDlg );
    virtual void            AcceptAll( BOOL bAccept );
};
//add for SwModalRedlineAcceptDlg end

//for SwGlossaryDlg begin
class SwGlossaryDlg;
class AbstractGlossaryDlg_Impl : public AbstractGlossaryDlg
{
    DECL_ABSTDLG_BASE(AbstractGlossaryDlg_Impl,SwGlossaryDlg);
    virtual String          GetCurrGrpName() const;
    virtual String          GetCurrShortName() const;
};
//for SwGlossaryDlg end

//for SwFldInputDlg begin
class SwFldInputDlg;
class AbstractFldInputDlg_Impl : public AbstractFldInputDlg
{
    DECL_ABSTDLG_BASE(AbstractFldInputDlg_Impl,SwFldInputDlg);
    //from class SalFrame
    virtual void            SetWindowState( const ByteString& rStr ) ;
    virtual ByteString      GetWindowState( ULONG nMask = WINDOWSTATE_MASK_ALL ) const ;
};
//for SwFldInputDlg end

//for SwInsFootNoteDlg begin
class SwInsFootNoteDlg;
class AbstractInsFootNoteDlg_Impl : public AbstractInsFootNoteDlg
{
    DECL_ABSTDLG_BASE(AbstractInsFootNoteDlg_Impl,SwInsFootNoteDlg);
    virtual String          GetFontName();
    virtual BOOL            IsEndNote();
    virtual String          GetStr();
    //from class Window
    virtual void    SetHelpId( ULONG nHelpId );
    virtual void    SetText( const XubString& rStr );
};
//for SwInsFootNoteDlg end

//for SwInsertGrfRulerDlg begin
class SwInsertGrfRulerDlg;
class AbstractInsertGrfRulerDlg_Impl : public AbstractInsertGrfRulerDlg
{
    DECL_ABSTDLG_BASE(AbstractInsertGrfRulerDlg_Impl,SwInsertGrfRulerDlg);
    virtual String          GetGraphicName();
    virtual BOOL            IsSimpleLine();
    virtual BOOL            HasImages() const ;
};
//for SwInsertGrfRulerDlg end

//for SwInsTableDlg begin
class SwInsTableDlg;
class AbstractInsTableDlg_Impl : public AbstractInsTableDlg
{
    DECL_ABSTDLG_BASE(AbstractInsTableDlg_Impl,SwInsTableDlg);
    virtual void            GetValues( String& rName, USHORT& rRow, USHORT& rCol,
                                SwInsertTableOptions& rInsTblFlags, String& rTableAutoFmtName,
                                SwTableAutoFmt *& prTAFmt );
};
//for SwInsTableDlg end

//for SwJavaEditDialog begin
class SwJavaEditDialog;
class AbstractJavaEditDialog_Impl : public AbstractJavaEditDialog
{
    DECL_ABSTDLG_BASE(AbstractJavaEditDialog_Impl,SwJavaEditDialog);
    virtual String              GetText();
    virtual String              GetType();
    virtual BOOL                IsUrl();
    virtual BOOL                IsNew();
    virtual BOOL                IsUpdate();
};
//for SwJavaEditDialog end

//for SwMailMergeDlg begin
class SwMailMergeDlg;
class AbstractMailMergeDlg_Impl : public AbstractMailMergeDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeDlg_Impl,SwMailMergeDlg);
    virtual USHORT  GetMergeType() ;
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetSelection() const ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> GetResultSet() const;
};
//for SwMailMergeDlg end
//for SwMailMergeCreateFromDlg begin
class SwMailMergeCreateFromDlg;
class AbstractMailMergeCreateFromDlg_Impl : public AbstractMailMergeCreateFromDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeCreateFromDlg_Impl,SwMailMergeCreateFromDlg);
    virtual BOOL    IsThisDocument() const ;
};
//for SwMailMergeCreateFromDlg end
//for SwMailMergeFieldConnectionsDlg begin
class SwMailMergeFieldConnectionsDlg;
class AbstractMailMergeFieldConnectionsDlg_Impl : public AbstractMailMergeFieldConnectionsDlg
{
    DECL_ABSTDLG_BASE(AbstractMailMergeFieldConnectionsDlg_Impl,SwMailMergeFieldConnectionsDlg);
    virtual BOOL    IsUseExistingConnections() const ;
};
//for SwMailMergeFieldConnectionsDlg end

//for SwMultiTOXTabDialog begin
class SwMultiTOXTabDialog;
class AbstractMultiTOXTabDialog_Impl : public AbstractMultiTOXTabDialog
{
    DECL_ABSTDLG_BASE(AbstractMultiTOXTabDialog_Impl,SwMultiTOXTabDialog);
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
    DECL_ABSTDLG_BASE(AbstractEditRegionDlg_Impl,SwEditRegionDlg);
    virtual void    SelectSection(const String& rSectionName);
};
//for SwEditRegionDlg end
//for SwInsertSectionTabDialog begin
class SwInsertSectionTabDialog;
class AbstractInsertSectionTabDialog_Impl : public AbstractInsertSectionTabDialog
{
    DECL_ABSTDLG_BASE(AbstractInsertSectionTabDialog_Impl,SwInsertSectionTabDialog);
    virtual void        SetSection(const SwSection& rSect);
};
//for SwInsertSectionTabDialog end

//for SwIndexMarkFloatDlg begin
class SwIndexMarkFloatDlg;
class AbstractIndexMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractIndexMarkFloatDlg_Impl,SwIndexMarkFloatDlg);
    virtual void    ReInitDlg(SwWrtShell& rWrtShell);
    virtual Window *            GetWindow(); //this method is added for return a Window type pointer
};
//for SwIndexMarkFloatDlg end

//for SwAuthMarkFloatDlg begin
class SwAuthMarkFloatDlg;
class AbstractAuthMarkFloatDlg_Impl : public AbstractMarkFloatDlg
{
    DECL_ABSTDLG_BASE(AbstractAuthMarkFloatDlg_Impl,SwAuthMarkFloatDlg);
    virtual void    ReInitDlg(SwWrtShell& rWrtShell);
    virtual Window *            GetWindow(); //this method is added for return a Window type pointer
};
//for SwAuthMarkFloatDlg end

//------------------------------------------------------------------------
//AbstractDialogFactory_Impl implementations
class SwAbstractDialogFactory_Impl : public SwAbstractDialogFactory
{

public:
    //CHINA001 AbstractSwSaveLabelDlg*          CreateSwSaveLabelDlg(SwLabFmtPage* pParent, SwLabRec& rRec, const ResId& rResId );

    virtual AbstractSwWordCountDialog* CreateSwWordCountDialog(Window* pParent);
    virtual AbstractSwInsertAbstractDlg * CreateSwInsertAbstractDlg( Window* pParent,const ResId& rResId ); //CHINA001 add for SwInsertAbstractDlg
    virtual AbstractSfxSingleTabDialog*  CreateSfxSingleTabDialog ( Window* pParent, SfxItemSet& rSet,const ResId& rResId   );//CHINA001 add for  SwAddrDlg SwDropCapsDlg, SwBackgroundDlg,SwNumFmtDlg,
    virtual AbstractSwAsciiFilterDlg*  CreateSwAsciiFilterDlg ( Window* pParent, SwDocShell& rDocSh,
                                                                SvStream* pStream, const ResId& rResId ); //CHINA001 add for SwAsciiFilterDlg
    virtual VclAbstractDialog * CreateSwInsertBookmarkDlg( Window *pParent, SwWrtShell &rSh, SfxRequest& rReq, const ResId& rResId );//CHINA001 add for SwInsertBookmarkDlg
    virtual AbstractSwBreakDlg * CreateSwBreakDlg ( Window *pParent, SwWrtShell &rSh,const ResId& rResId ); // add for SwBreakDlg
    virtual VclAbstractDialog   * CreateSwChangeDBDlg( SwView& rVw, const ResId& rResId  ); //add for SwChangeDBDlg
    virtual SfxAbstractTabDialog *  CreateSwCharDlg( Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet, const ResId& rResId, // add for SwCharDlg
                                                const String* pFmtStr = 0, BOOL bIsDrwTxtDlg = FALSE);
    virtual AbstractSwConvertTableDlg* CreateSwConvertTableDlg ( SwView& rView, const ResId& rResId ); //add for SwConvertTableDlg
    virtual VclAbstractDialog * CreateSwCaptionDialog ( Window *pParent, SwView &rV,const ResId& rResId); //add for SwCaptionDialog

    virtual AbstractSwInsertDBColAutoPilot* CreateSwInsertDBColAutoPilot( SwView& rView, // add for SwInsertDBColAutoPilot
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        com::sun::star::uno::Reference<com::sun::star::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData,  const ResId& rResId);
    virtual SfxAbstractTabDialog * CreateSwFootNoteOptionDlg( Window *pParent, SwWrtShell &rSh,const ResId& rResId);//add for  SwFootNoteOptionDlg

    virtual AbstractDropDownFieldDialog * CreateDropDownFieldDialog ( Window *pParent, SwWrtShell &rSh, //add for DropDownFieldDialog
                                SwField* pField,const ResId& rResId, BOOL bNextButton = FALSE );
    virtual SfxAbstractTabDialog* CreateSwEnvDlg ( Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, BOOL bInsert,const ResId& rResId ); //add for SwEnvDlg
    virtual AbstarctSwLabDlg* CreateSwLabDlg ( Window* pParent, const SfxItemSet& rSet, //add for SwLabDlg
                                                     SwNewDBMgr* pNewDBMgr, BOOL bLabel,const ResId& rResId  );

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod (); //add for SwLabDlg
    virtual SfxAbstractTabDialog* CreateSwParaDlg ( Window *pParent, //add for SwParaDlg
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    BYTE nDialogMode,
                                                    const ResId& rResId,
                                                    const String *pCollName = 0,
                                                    BOOL bDraw = FALSE,
                                                    UINT16 nDefPage = 0);

    virtual AbstarctSwSelGlossaryDlg * CreateSwSelGlossaryDlg ( Window * pParent, const String &rShortName, const ResId& rResId ); //add for SwSelGlossaryDlg
    virtual AbstractSwSplitTableDlg * CreateSwSplitTableDlg ( Window *pParent, SwWrtShell& rShell,const ResId& rResId ); //add for SwSplitTableDlg
    virtual VclAbstractDialog * CreateVclAbstractDialog ( Window * pParent, SwWrtShell &rSh, const ResId& rResId ); //add for  SwTableHeightDlg SwSortDlg ,SwSplitTblDlg

    virtual AbstractSwAutoFormatDlg * CreateSwAutoFormatDlg( Window* pParent, SwWrtShell* pShell, //add for SwAutoFormatDlg
                                                            const ResId& rResId,
                                                            BOOL bSetAutoFmt = TRUE,
                                                            const SwTableAutoFmt* pSelFmt = 0 );
    virtual AbstractSfxSingleTabDialog * CreateSwBorderDlg (Window* pParent, SfxItemSet& rSet, USHORT nType,const ResId& rResId );//add for SwBorderDlg

    virtual AbstractSfxSingleTabDialog * CreateSwWrapDlg ( Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, BOOL bDrawMode, const ResId& rResId ); //add for SwWrapDlg
    virtual VclAbstractDialog * CreateSwTableWidthDlg ( Window *pParent, SwTableFUNC &rFnc , const ResId& rResId ); //add for SwTableWidthDlg
    virtual SfxAbstractTabDialog* CreateSwTableTabDlg( Window* pParent, SfxItemPool& Pool,
                                                        const SfxItemSet* pItemSet, SwWrtShell* pSh,const ResId& rResId ); //add for SwTableTabDlg
    virtual AbstractSwFldDlg * CreateSwFldDlg ( SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent, const ResId& rResId ); //add for SwFldDlg
    virtual AbstractSfxSingleTabDialog*  CreateSwFldEditDlg ( SwView& rVw, const ResId& rResId ); //add for SwFldEditDlg
    virtual AbstractSwRenameXNamedDlg * CreateSwRenameXNamedDlg( Window* pParent, //add for SwRenameXNamedDlg
                                                                STAR_REFERENCE( container::XNamed ) & xNamed,
                                                                STAR_REFERENCE( container::XNameAccess ) & xNameAccess, const ResId& rResId );
    virtual AbstractSwModalRedlineAcceptDlg * CreateSwModalRedlineAcceptDlg ( Window *pParent, const ResId& rResId ); //add for SwModalRedlineAcceptDlg

    virtual VclAbstractDialog*          CreateSwVclDialog( const ResId& rResId,
                                                Window* pParent, BOOL& rWithPrev ); //add for SwMergeTblDlg
//CHINA001  virtual VclAbstractDialog*          CreateSwWrtShDialog( const ResId& rResId,
//CHINA001                                              Window* pParent, SwWrtShell& rSh ); //add for SwColumnDlg
    virtual SfxAbstractTabDialog*       CreateFrmTabDialog( const ResId& rResId,
                                                SfxViewFrame *pFrame, Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                BOOL            bNewFrm  = TRUE,
                                                USHORT          nResType = DLG_FRM_STD,
                                                BOOL            bFmt     = FALSE,
                                                UINT16          nDefPage = 0,
                                                const String*   pFmtStr  = 0); //add for SwFrmDlg
    virtual SfxAbstractTabDialog*       CreateTemplateDialog( const ResId& rResId,
                                                Window*             pParent,
                                                SfxStyleSheetBase&  rBase,
                                                USHORT              nRegion,
                                                BOOL                bColumn = FALSE,
                                                SwWrtShell*         pActShell = 0,
                                                BOOL                bNew = FALSE ); //add for SwTemplateDlg
    virtual AbstractGlossaryDlg*        CreateGlossaryDlg( const ResId& rResId,
                                                SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell); //add for SwGlossaryDlg
    virtual AbstractFldInputDlg*        CreateFldInputDlg( const ResId& rResId,
                                                Window *pParent, SwWrtShell &rSh,
                                                SwField* pField, BOOL bNextButton = FALSE ); //add for SwFldInputDlg
    virtual AbstractInsFootNoteDlg*     CreateInsFootNoteDlg( const ResId& rResId,
                                                Window * pParent, SwWrtShell &rSh, BOOL bEd = FALSE); //add for SwInsFootNoteDlg
    virtual VclAbstractDialog *         CreateVclSwViewDialog( const ResId& rResId,
                                                SwView& rView, BOOL bCol = FALSE ); //add for SwInsRowColDlg, SwLineNumberingDlg
    virtual AbstractInsertGrfRulerDlg*      CreateInsertGrfRulerDlg( const ResId& rResId,
                                                Window * pParent ); //add for SwInsertGrfRulerDlg
    virtual AbstractInsTableDlg*        CreateInsTableDlg( const ResId& rResId,
                                                SwView& rView ); //add for SwInsTableDlg
    virtual AbstractJavaEditDialog*     CreateJavaEditDialog( const ResId& rResId,
                                                Window* pParent, SwWrtShell* pWrtSh ); //add for SwJavaEditDialog
    virtual AbstractMailMergeDlg*       CreateMailMergeDlg( const ResId& rResId,
                                                Window* pParent, SwWrtShell& rSh,
                                                 const String& rSourceName,
                                                const String& rTblName,
                                                sal_Int32 nCommandType,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection,
                                                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >* pSelection = 0 ); //add for SwMailMergeDlg
    virtual AbstractMailMergeCreateFromDlg*     CreateMailMergeCreateFromDlg( const ResId& rResId,
                                                    Window* pParent ); //add for SwMailMergeCreateFromDlg
    virtual AbstractMailMergeFieldConnectionsDlg*       CreateMailMergeFieldConnectionsDlg( const ResId& rResId,
                                                            Window* pParent ); //add for SwMailMergeFieldConnectionsDlg
    virtual VclAbstractDialog*          CreateMultiTOXMarkDlg( const ResId& rResId,
                                                Window* pParent, SwTOXMgr &rTOXMgr ); //add for SwMultiTOXMarkDlg
    virtual SfxAbstractTabDialog*       CreateSwTabDialog( const ResId& rResId,
                                                Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &); //add for SwSvxNumBulletTabDialog, SwOutlineTabDialog
    virtual AbstractMultiTOXTabDialog*      CreateMultiTOXTabDialog( const ResId& rResId,
                                                Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX, USHORT nToxType = USHRT_MAX,
                                                BOOL bGlobal = FALSE); //add for SwMultiTOXTabDialog
    virtual AbstractEditRegionDlg*      CreateEditRegionDlg( const ResId& rResId,
                                                Window* pParent, SwWrtShell& rWrtSh ); //add for SwEditRegionDlg
    virtual AbstractInsertSectionTabDialog*     CreateInsertSectionTabDialog( const ResId& rResId,
                                                    Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh); //add for SwInsertSectionTabDialog
    virtual AbstractMarkFloatDlg*       CreateIndexMarkFloatDlg( const ResId& rResId,
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew=sal_True); //add for SwIndexMarkFloatDlg
    virtual AbstractMarkFloatDlg*       CreateAuthMarkFloatDlg( const ResId& rResId,
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew=sal_True); //add for SwAuthMarkFloatDlg
    virtual VclAbstractDialog *         CreateIndexMarkModalDlg( const ResId& rResId,
                                                Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark ); //add for SwIndexMarkModalDlg

    //add for static func in SwGlossaryDlg
    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc( USHORT nId );
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc( USHORT nId );

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId );

    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId );

};

struct SwDialogsResMgr
{
    static ResMgr*      GetResMgr();
};

#endif


