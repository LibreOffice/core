/*************************************************************************
 *
 *  $RCSfile: scdlgfact.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:57:53 $
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
#ifndef _SC_DLGFACT_HXX
#define _SC_DLGFACT_HXX

// include ---------------------------------------------------------------
#include "scabstdlg.hxx" //CHINA001
class ScImportAsciiDlg;
class ScAutoFormatDlg;
class ScColRowLabelDlg;
class ScDataPilotDatabaseDlg;
class ScDataPilotSourceTypeDlg;
class ScDataPilotServiceDlg;
class ScDeleteCellDlg;
class ScDeleteContentsDlg;
class ScFillSeriesDlg;
class ScGroupDlg;
class ScInsertCellDlg;
class ScInsertContentsDlg;
class ScInsertTableDlg;
class ScSelEntryDlg;
class ScLinkedAreaDlg;
class ScMetricInputDlg;
class ScMoveTableDlg;
class ScNameCreateDlg;
class ScNamePasteDlg;
class ScPivotFilterDlg;
class ScDPFunctionDlg;
class ScDPSubtotalDlg;
class ScDPShowDetailDlg;
class ScNewScenarioDlg;
class ScShowTabDlg;
class ScStringInputDlg;
class ScImportOptionsDlg;
class SfxTabDialog;

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


class VclAbstractDialog_Impl : public VclAbstractDialog //add for ScColOrRowDlg
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog);
};

class AbstractScImportAsciiDlg_Impl : public AbstractScImportAsciiDlg  //add for ScImportAsciiDlg
{
    DECL_ABSTDLG_BASE(AbstractScImportAsciiDlg_Impl, ScImportAsciiDlg);
    virtual void                        GetOptions( ScAsciiOptions& rOpt );
};

class AbstractScAutoFormatDlg_Impl : public AbstractScAutoFormatDlg  //add for ScAutoFormatDlg
{
    DECL_ABSTDLG_BASE(AbstractScAutoFormatDlg_Impl, ScAutoFormatDlg);
    virtual USHORT GetIndex() const;
    virtual String GetCurrFormatName();
};

class AbstractScColRowLabelDlg_Impl : public AbstractScColRowLabelDlg  //add for ScColRowLabelDlg
{
    DECL_ABSTDLG_BASE(AbstractScColRowLabelDlg_Impl,ScColRowLabelDlg);
    virtual BOOL IsCol();
    virtual BOOL IsRow();
};

class AbstractScDataPilotDatabaseDlg_Impl  :public AbstractScDataPilotDatabaseDlg  //add for ScDataPilotDatabaseDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotDatabaseDlg_Impl, ScDataPilotDatabaseDlg);
    virtual void    GetValues( ScImportSourceDesc& rDesc );
};

class AbstractScDataPilotSourceTypeDlg_Impl  :public AbstractScDataPilotSourceTypeDlg  //add for ScDataPilotSourceTypeDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotSourceTypeDlg_Impl, ScDataPilotSourceTypeDlg);
    virtual BOOL    IsDatabase() const;
    virtual BOOL    IsExternal() const;
};

class AbstractScDataPilotServiceDlg_Impl : public AbstractScDataPilotServiceDlg  //add for ScDataPilotServiceDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotServiceDlg_Impl, ScDataPilotServiceDlg);
    virtual String  GetServiceName() const;
    virtual String  GetParSource() const;
    virtual String  GetParName() const;
    virtual String  GetParUser() const;
    virtual String  GetParPass() const;
};

class AbstractScDeleteCellDlg_Impl : public AbstractScDeleteCellDlg  //add for ScDeleteCellDlg
{
    DECL_ABSTDLG_BASE(AbstractScDeleteCellDlg_Impl,ScDeleteCellDlg);
    virtual DelCellCmd GetDelCellCmd() const;
};

class AbstractScDeleteContentsDlg_Impl : public AbstractScDeleteContentsDlg  //add for ScDeleteContentsDlg
{
    DECL_ABSTDLG_BASE( AbstractScDeleteContentsDlg_Impl,ScDeleteContentsDlg);
    virtual void    DisableObjects();
    virtual USHORT  GetDelContentsCmdBits() const;
};

class AbstractScFillSeriesDlg_Impl:public AbstractScFillSeriesDlg  //add for ScFillSeriesDlg
{
    DECL_ABSTDLG_BASE(AbstractScFillSeriesDlg_Impl, ScFillSeriesDlg);
    virtual FillDir     GetFillDir() const;
    virtual FillCmd     GetFillCmd() const;
    virtual FillDateCmd GetFillDateCmd() const;
    virtual double      GetStart() const;
    virtual double      GetStep() const;
    virtual double      GetMax() const;
    virtual String      GetStartStr() const;
    virtual void        SetEdStartValEnabled(BOOL bFlag=FALSE);
};

class AbstractScGroupDlg_Impl :  public AbstractScGroupDlg  //add for ScGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScGroupDlg_Impl, ScGroupDlg);
    virtual BOOL GetColsChecked() const;
};

class AbstractScInsertCellDlg_Impl : public AbstractScInsertCellDlg  //add for ScInsertCellDlg
{
    DECL_ABSTDLG_BASE( AbstractScInsertCellDlg_Impl, ScInsertCellDlg);
    virtual InsCellCmd GetInsCellCmd() const ;
};

class AbstractScInsertContentsDlg_Impl : public AbstractScInsertContentsDlg  //add for ScInsertContentsDlg
{
    DECL_ABSTDLG_BASE(AbstractScInsertContentsDlg_Impl, ScInsertContentsDlg);
    virtual USHORT      GetInsContentsCmdBits() const;
    virtual USHORT      GetFormulaCmdBits() const;
    virtual BOOL        IsSkipEmptyCells() const;
    virtual BOOL        IsLink() const;
    virtual void    SetFillMode( BOOL bSet );
    virtual void    SetOtherDoc( BOOL bSet );
    virtual BOOL        IsTranspose() const;
    virtual void    SetChangeTrack( BOOL bSet );
    virtual void    SetCellShiftDisabled( int nDisable );
    virtual InsCellCmd  GetMoveMode();
};

class AbstractScInsertTableDlg_Impl : public AbstractScInsertTableDlg  //add for ScInsertTableDlg
{
    DECL_ABSTDLG_BASE( AbstractScInsertTableDlg_Impl, ScInsertTableDlg);
    virtual BOOL            GetTablesFromFile();
    virtual BOOL            GetTablesAsLink();
    virtual const String*   GetFirstTable( USHORT* pN = NULL );
    virtual ScDocShell*     GetDocShellTables();
    virtual BOOL            IsTableBefore();
    virtual USHORT          GetTableCount();
    virtual const String*   GetNextTable( USHORT* pN = NULL );

};

class AbstractScSelEntryDlg_Impl : public AbstractScSelEntryDlg  //add for ScSelEntryDlg
{
    DECL_ABSTDLG_BASE( AbstractScSelEntryDlg_Impl, ScSelEntryDlg );
    virtual String GetSelectEntry() const;
};

class AbstractScLinkedAreaDlg_Impl : public AbstractScLinkedAreaDlg  //add for ScLinkedAreaDlg
{
    DECL_ABSTDLG_BASE( AbstractScLinkedAreaDlg_Impl, ScLinkedAreaDlg);
    virtual void            InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        ULONG nRefresh );
    virtual String          GetURL();
    virtual String          GetFilter();        // may be empty
    virtual String          GetOptions();       // filter options
    virtual String          GetSource();        // separated by ";"
    virtual ULONG           GetRefresh();       // 0 if disabled
};

class AbstractScMetricInputDlg_Impl : public AbstractScMetricInputDlg  //add for ScMetricInputDlg
{
    DECL_ABSTDLG_BASE( AbstractScMetricInputDlg_Impl, ScMetricInputDlg);
    virtual long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const;
};

class AbstractScMoveTableDlg_Impl : public AbstractScMoveTableDlg  //add for ScMoveTableDlg
{
    DECL_ABSTDLG_BASE( AbstractScMoveTableDlg_Impl, ScMoveTableDlg);
    virtual USHORT  GetSelectedDocument     () const;
    virtual USHORT  GetSelectedTable        () const;
    virtual BOOL    GetCopyTable            () const;
    virtual void    SetCopyTable            (BOOL bFlag=TRUE);
    virtual void    EnableCopyTable         (BOOL bFlag=TRUE);
};

class AbstractScNameCreateDlg_Impl : public AbstractScNameCreateDlg  //add for ScNameCreateDlg
{
    DECL_ABSTDLG_BASE( AbstractScNameCreateDlg_Impl, ScNameCreateDlg);
    virtual USHORT          GetFlags() const;
};

class AbstractScNamePasteDlg_Impl : public AbstractScNamePasteDlg  //add for ScNamePasteDlg
{
    DECL_ABSTDLG_BASE( AbstractScNamePasteDlg_Impl, ScNamePasteDlg );
    virtual String          GetSelectedName() const;
};

class AbstractScPivotFilterDlg_Impl : public AbstractScPivotFilterDlg  //add for ScPivotFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractScPivotFilterDlg_Impl, ScPivotFilterDlg);
    virtual const ScQueryItem&  GetOutputItem();
};

class AbstractScDPFunctionDlg_Impl : public AbstractScDPFunctionDlg  //add for ScDPFunctionDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPFunctionDlg_Impl, ScDPFunctionDlg);
    virtual USHORT GetFuncMask() const;
    virtual ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const;
};

class AbstractScDPSubtotalDlg_Impl : public AbstractScDPSubtotalDlg  //add for ScDPSubtotalDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPSubtotalDlg_Impl, ScDPSubtotalDlg);
    virtual USHORT GetFuncMask() const;
    virtual void FillLabelData( ScDPLabelData& rLabelData ) const;
};

class AbstractScDPShowDetailDlg_Impl : public AbstractScDPShowDetailDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPShowDetailDlg_Impl, ScDPShowDetailDlg);
    virtual String GetDimensionName() const;
};

class AbstractScNewScenarioDlg_Impl : public AbstractScNewScenarioDlg  //add for ScNewScenarioDlg
{
    DECL_ABSTDLG_BASE( AbstractScNewScenarioDlg_Impl, ScNewScenarioDlg );
    virtual void SetScenarioData( const String& rName, const String& rComment,
                            const Color& rColor, USHORT nFlags );

    virtual void GetScenarioData( String& rName, String& rComment,
                            Color& rColor, USHORT& rFlags ) const;
};

class AbstractScShowTabDlg_Impl : public AbstractScShowTabDlg  //add for ScShowTabDlg
{
    DECL_ABSTDLG_BASE(AbstractScShowTabDlg_Impl,ScShowTabDlg);
    virtual void    Insert( const String& rString, BOOL bSelected );
    virtual USHORT  GetSelectEntryCount() const;
    virtual void SetDescription(const String& rTitle, const String& rFixedText,ULONG nDlgHelpId, ULONG nLbHelpId );
    virtual String  GetSelectEntry(USHORT nPos) const;
    virtual USHORT  GetSelectEntryPos(USHORT nPos) const;
};

class AbstractScStringInputDlg_Impl :  public AbstractScStringInputDlg  //add for ScStringInputDlg
{
    DECL_ABSTDLG_BASE( AbstractScStringInputDlg_Impl, ScStringInputDlg );
    virtual void GetInputString( String& rString ) const;
};

class AbstractScImportOptionsDlg_Impl : public AbstractScImportOptionsDlg  //add for ScImportOptionsDlg
{
    DECL_ABSTDLG_BASE( AbstractScImportOptionsDlg_Impl, ScImportOptionsDlg);
    virtual void GetImportOptions( ScImportOptions& rOptions ) const;
};

//add for ScAttrDlg , ScHFEditDlg, ScStyleDlg, ScSubTotalDlg, ScCharDlg, ScParagraphDlg, ScValidationDlg, ScSortDlg
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
//------------------------------------------------------------------------
//AbstractDialogFactory_Impl implementations
class ScAbstractDialogFactory_Impl : public ScAbstractDialogFactory
{

public:
    //CHINA001 AbstractSwSaveLabelDlg*          CreateSwSaveLabelDlg(SwLabFmtPage* pParent, SwLabRec& rRec, const ResId& rResId );

    virtual     AbstractScImportAsciiDlg * CreateScImportAsciiDlg( Window* pParent, String aDatName, //add for ScImportAsciiDlg
                                                                    SvStream* pInStream, const ResId& rResId,
                                                                    sal_Unicode cSep = '\t');

    virtual     AbstractScAutoFormatDlg * CreateScAutoFormatDlg( Window*                    pParent, //add for ScAutoFormatDlg
                                                                ScAutoFormat*               pAutoFormat,
                                                                const ScAutoFormatData*    pSelFormatData,
                                                                ScDocument*                pDoc,
                                                                const ResId& rResId);
    virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (Window* pParent, //add for ScColRowLabelDlg
                                                                const ResId& rResId,
                                                                BOOL bCol = FALSE,
                                                                BOOL bRow = FALSE);

    virtual VclAbstractDialog * CreateScColOrRowDlg( Window*            pParent, //add for ScColOrRowDlg
                                                    const String&   rStrTitle,
                                                    const String&   rStrLabel,
                                                    const ResId& rResId,
                                                    BOOL                bColDefault = TRUE );
    virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg (Window* pParent ,const ResId& rResId ); //add for ScDataPilotDatabaseDlg

    virtual AbstractScDataPilotSourceTypeDlg * CreateScDataPilotSourceTypeDlg (  Window* pParent, BOOL bEnableExternal, const ResId& rResId ) ; //add for ScDataPilotSourceTypeDlg

    virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( Window* pParent, //add for ScDataPilotServiceDlg
                                                                        const com::sun::star::uno::Sequence<rtl::OUString>& rServices,
                                                                        const ResId& rResId );
    virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg( Window* pParent, const ResId& rResId,
                                                            BOOL bDisallowCellMove = FALSE ); //add for ScDeleteCellDlg

    virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(Window* pParent,const ResId& rResId, //add for ScDeleteContentsDlg
                                                                 USHORT  nCheckDefaults = 0 );

    virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( Window*        pParent, //add for ScFillSeriesDlg
                                                            ScDocument& rDocument,
                                                            FillDir     eFillDir,
                                                            FillCmd     eFillCmd,
                                                            FillDateCmd eFillDateCmd,
                                                            String          aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            USHORT          nPossDir,
                                                            const ResId& rResId);
    virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( Window* pParent, //add for ScGroupDlg
                                                            USHORT  nResId,
                                                            const ResId& rResId,
                                                            BOOL    bUnGroup = FALSE,
                                                            BOOL    bRows    = TRUE  );

    virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( Window* pParent, //add for ScInsertCellDlg
                                                                const ResId& rResId,
                                                            BOOL bDisallowCellMove = FALSE );

    virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( Window*        pParent, //add for ScInsertContentsDlg
                                                                    const ResId& rResId,
                                                                    USHORT          nCheckDefaults = 0,
                                                                    const String*   pStrTitle = NULL );

    virtual AbstractScInsertTableDlg * CreateScInsertTableDlg ( Window* pParent, ScViewData& rViewData,  //add for ScInsertTableDlg
                                                                SCTAB nTabCount, bool bFromFile, const ResId& rResId);

    virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( Window* pParent, // add for ScSelEntryDlg
                                                            USHORT  nResId,
                                                        const String& aTitle,
                                                        const String& aLbTitle,
                                                                List&   aEntryList,
                                                            const ResId& rResId );

    virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg (  Window* pParent, //add for ScLinkedAreaDlg
                                                                const ResId& rResId);

    virtual AbstractScMetricInputDlg * CreateScMetricInputDlg (  Window*        pParent, //add for ScMetricInputDlg
                                                                USHORT      nResId,     // Ableitung fuer jeden Dialog!
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                const ResId& rResId ,
                                                                FieldUnit       eFUnit    = FUNIT_MM,
                                                                USHORT      nDecimals = 2,
                                                                long            nMaximum  = 1000,
                                                                long            nMinimum  = 0,
                                                                long            nFirst    = 1,
                                                                long          nLast     = 100 );

    virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(  Window* pParent, const ResId& rResId ); //add for ScMoveTableDlg
    virtual AbstractScNameCreateDlg * CreateScNameCreateDlg ( Window * pParent, USHORT nFlags, const ResId& rResId ); //add for ScNameCreateDlg

    virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( Window * pParent, const ScRangeName* pList, //add for ScNamePasteDlg
                                                            const ResId& rResId , BOOL bInsList=TRUE );

    virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg ( Window* pParent, //add for ScPivotFilterDlg
                                                                const SfxItemSet&   rArgSet, USHORT nSourceTab , const ResId& rResId );

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( Window* pParent, const ResId& rResId,
                                                                const ScDPLabelDataVec& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData );

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( Window* pParent, const ResId& rResId,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout );

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( Window* pParent, const ResId& rResId,
                                                                ScDPObject& rDPObj,
                                                                USHORT nOrient );

    virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( Window* pParent, const String& rName, //add for ScNewScenarioDlg
                                                                const ResId& rResId,
                                                                BOOL bEdit = FALSE, BOOL bSheetProtected = FALSE );
    virtual AbstractScShowTabDlg * CreateScShowTabDlg ( Window* pParent, const ResId& rResId ); //add for ScShowTabDlg

    virtual AbstractScStringInputDlg * CreateScStringInputDlg (  Window* pParent, //add for ScStringInputDlg
                                                                const String& rTitle,
                                                                const String& rEditTitle,
                                                                const String& rDefault,
                                                                ULONG nHelpId ,
                                                                const ResId& rResId );
    virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( Window*                 pParent, //add for ScImportOptionsDlg
                                                                    const ResId& rResId,
                                                                    BOOL                    bAscii = TRUE,
                                                                    const ScImportOptions*  pOptions = NULL,
                                                                    const String*           pStrTitle = NULL,
                                                                    BOOL                    bMultiByte = FALSE,
                                                                    BOOL                    bOnlyDbtoolsEncodings = FALSE,
                                                                    BOOL                    bImport = TRUE );
    virtual SfxAbstractTabDialog * CreateScAttrDlg( SfxViewFrame*    pFrame, //add for ScAttrDlg
                                                    Window*          pParent,
                                                    const SfxItemSet* pCellAttrs,
                                                    const ResId& rResId);

    virtual SfxAbstractTabDialog * CreateScHFEditDlg( SfxViewFrame*     pFrame, //add for ScHFEditDlg
                                                    Window*         pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const String&       rPageStyle,
                                                    const ResId& rResId,
                                                    USHORT              nResId = RID_SCDLG_HFEDIT );

    virtual SfxAbstractTabDialog * CreateScStyleDlg( Window*                pParent,//add for ScStyleDlg
                                                    SfxStyleSheetBase&  rStyleBase,
                                                    USHORT              nRscId,
                                                    const ResId& rResId);

    virtual SfxAbstractTabDialog * CreateScSubTotalDlg( Window*             pParent, //add for ScSubTotalDlg
                                                        const SfxItemSet*   pArgSet,
                                                        const ResId& rResId);
    virtual SfxAbstractTabDialog * CreateScCharDlg( Window* pParent, const SfxItemSet* pAttr,//add for ScCharDlg
                                                    const SfxObjectShell* pDocShell, const ResId& rResId );

    virtual SfxAbstractTabDialog * CreateScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ,//add for ScParagraphDlg
                                                            const ResId& rResId );

    virtual SfxAbstractTabDialog * CreateScValidationDlg( Window* pParent, //add for ScValidationDlg
                                                        const SfxItemSet* pArgSet,const ResId& rResId  );

    virtual SfxAbstractTabDialog * CreateScSortDlg( Window*          pParent, //add for ScSortDlg
                                                    const SfxItemSet* pArgSet,const ResId& rResId );
    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId );

    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId );

};

//CHINA001 struct ScDialogsResMgr
//CHINA001 {
//CHINA001 static ResMgr*       GetResMgr();
//CHINA001 };

#endif


