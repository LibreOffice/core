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
#ifndef _SC_DLGFACT_HXX
#define _SC_DLGFACT_HXX

#include "scabstdlg.hxx"
class Dialog;
class ScImportAsciiDlg;
class ScAutoFormatDlg;
class ScColRowLabelDlg;
class ScDataPilotDatabaseDlg;
class ScDataPilotSourceTypeDlg;
class ScDataPilotServiceDlg;
class ScDeleteCellDlg;
class ScDataFormDlg;
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
class ScDPNumGroupDlg;
class ScDPDateGroupDlg;
class ScDPShowDetailDlg;
class ScSortWarningDlg;
class ScNewScenarioDlg;
class ScShowTabDlg;
class ScStringInputDlg;
class ScTabBgColorDlg;
class ScImportOptionsDlg;
class SfxTabDialog;
class ScTextImportOptionsDlg;
class ScCondFormatManagerDlg;

#define DECL_ABSTDLG_BASE(Class,DialogClass)        \
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual short   Execute() ;

#define DECL_ABSTDLG2_BASE(Class,DialogClass)        \
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual void    StartExecuteModal( const Link& rEndDialogHdl ); \
    long            GetResult();

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
short Class::Execute()                             \
{                                                   \
    return pDlg->Execute();                         \
}

#define IMPL_ABSTDLG2_BASE(Class)                   \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
void Class::StartExecuteModal( const Link& rEndDialogHdl ) \
{                                                   \
    pDlg->StartExecuteModal( rEndDialogHdl ) ;      \
}                                                   \
long Class::GetResult()                             \
{                                                   \
    return pDlg->GetResult();                       \
}

class ScVclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(ScVclAbstractDialog_Impl,Dialog)
};

class AbstractScImportAsciiDlg_Impl : public AbstractScImportAsciiDlg
{
    DECL_ABSTDLG_BASE(AbstractScImportAsciiDlg_Impl, ScImportAsciiDlg)
    virtual void                        GetOptions( ScAsciiOptions& rOpt );
    virtual void                        SaveParameters();
};

class AbstractScAutoFormatDlg_Impl : public AbstractScAutoFormatDlg
{
    DECL_ABSTDLG_BASE(AbstractScAutoFormatDlg_Impl, ScAutoFormatDlg)
    virtual sal_uInt16 GetIndex() const;
    virtual String GetCurrFormatName();
};

class AbstractScColRowLabelDlg_Impl : public AbstractScColRowLabelDlg
{
    DECL_ABSTDLG_BASE(AbstractScColRowLabelDlg_Impl,ScColRowLabelDlg)
    virtual sal_Bool IsCol();
    virtual sal_Bool IsRow();
};

class AbstractScCondFormatManagerDlg_Impl : public AbstractScCondFormatManagerDlg
{
    DECL_ABSTDLG_BASE(AbstractScCondFormatManagerDlg_Impl, ScCondFormatManagerDlg)

    virtual ScConditionalFormatList* GetConditionalFormatList();

    virtual bool CondFormatsChanged();
};

class AbstractScDataPilotDatabaseDlg_Impl  :public AbstractScDataPilotDatabaseDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotDatabaseDlg_Impl, ScDataPilotDatabaseDlg)
    virtual void    GetValues( ScImportSourceDesc& rDesc );
};

class AbstractScDataPilotSourceTypeDlg_Impl  :public AbstractScDataPilotSourceTypeDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotSourceTypeDlg_Impl, ScDataPilotSourceTypeDlg)
    virtual bool IsDatabase() const;
    virtual bool IsExternal() const;
    virtual bool IsNamedRange() const;
    virtual OUString GetSelectedNamedRange() const;
    virtual void AppendNamedRange(const OUString& rName);
};

class AbstractScDataPilotServiceDlg_Impl : public AbstractScDataPilotServiceDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotServiceDlg_Impl, ScDataPilotServiceDlg)
    virtual String  GetServiceName() const;
    virtual String  GetParSource() const;
    virtual String  GetParName() const;
    virtual String  GetParUser() const;
    virtual String  GetParPass() const;
};

class AbstractScDeleteCellDlg_Impl : public AbstractScDeleteCellDlg
{
    DECL_ABSTDLG_BASE(AbstractScDeleteCellDlg_Impl,ScDeleteCellDlg)
    virtual DelCellCmd GetDelCellCmd() const;
};

//for dataform
class AbstractScDataFormDlg_Impl : public AbstractScDataFormDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataFormDlg_Impl,ScDataFormDlg);
};

class AbstractScDeleteContentsDlg_Impl : public AbstractScDeleteContentsDlg
{
    DECL_ABSTDLG_BASE( AbstractScDeleteContentsDlg_Impl,ScDeleteContentsDlg)
    virtual void    DisableObjects();
    virtual sal_uInt16  GetDelContentsCmdBits() const;
};

class AbstractScFillSeriesDlg_Impl:public AbstractScFillSeriesDlg
{
    DECL_ABSTDLG_BASE(AbstractScFillSeriesDlg_Impl, ScFillSeriesDlg)
    virtual FillDir     GetFillDir() const;
    virtual FillCmd     GetFillCmd() const;
    virtual FillDateCmd GetFillDateCmd() const;
    virtual double      GetStart() const;
    virtual double      GetStep() const;
    virtual double      GetMax() const;
    virtual String      GetStartStr() const;
    virtual void        SetEdStartValEnabled(sal_Bool bFlag=false);
};

class AbstractScGroupDlg_Impl :  public AbstractScGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScGroupDlg_Impl, ScGroupDlg)
    virtual sal_Bool GetColsChecked() const;
};

class AbstractScInsertCellDlg_Impl : public AbstractScInsertCellDlg
{
    DECL_ABSTDLG_BASE( AbstractScInsertCellDlg_Impl, ScInsertCellDlg)
    virtual InsCellCmd GetInsCellCmd() const ;
};

class AbstractScInsertContentsDlg_Impl : public AbstractScInsertContentsDlg
{
    DECL_ABSTDLG_BASE(AbstractScInsertContentsDlg_Impl, ScInsertContentsDlg)
    virtual sal_uInt16      GetInsContentsCmdBits() const;
    virtual sal_uInt16      GetFormulaCmdBits() const;
    virtual sal_Bool        IsSkipEmptyCells() const;
    virtual sal_Bool        IsLink() const;
    virtual void    SetFillMode( sal_Bool bSet );
    virtual void    SetOtherDoc( sal_Bool bSet );
    virtual sal_Bool        IsTranspose() const;
    virtual void    SetChangeTrack( sal_Bool bSet );
    virtual void    SetCellShiftDisabled( int nDisable );
    virtual InsCellCmd  GetMoveMode();
};

class AbstractScInsertTableDlg_Impl : public AbstractScInsertTableDlg
{
    DECL_ABSTDLG_BASE( AbstractScInsertTableDlg_Impl, ScInsertTableDlg)
    virtual sal_Bool            GetTablesFromFile();
    virtual sal_Bool            GetTablesAsLink();
    virtual const String*   GetFirstTable( sal_uInt16* pN = NULL );
    virtual ScDocShell*     GetDocShellTables();
    virtual sal_Bool            IsTableBefore();
    virtual sal_uInt16          GetTableCount();
    virtual const String*   GetNextTable( sal_uInt16* pN = NULL );

};

class AbstractScSelEntryDlg_Impl : public AbstractScSelEntryDlg
{
    DECL_ABSTDLG_BASE( AbstractScSelEntryDlg_Impl, ScSelEntryDlg )
    virtual String GetSelectEntry() const;
};

class AbstractScLinkedAreaDlg_Impl : public AbstractScLinkedAreaDlg
{
    DECL_ABSTDLG2_BASE( AbstractScLinkedAreaDlg_Impl, ScLinkedAreaDlg)

    virtual void            InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        sal_uLong nRefresh );
    virtual String          GetURL();
    virtual String          GetFilter();        // may be empty
    virtual String          GetOptions();       // filter options
    virtual String          GetSource();        // separated by ";"
    virtual sal_uLong           GetRefresh();       // 0 if disabled
};

class AbstractScMetricInputDlg_Impl : public AbstractScMetricInputDlg
{
    DECL_ABSTDLG_BASE( AbstractScMetricInputDlg_Impl, ScMetricInputDlg)
    virtual long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const;
};

class AbstractScMoveTableDlg_Impl : public AbstractScMoveTableDlg
{
    DECL_ABSTDLG_BASE( AbstractScMoveTableDlg_Impl, ScMoveTableDlg)
    virtual sal_uInt16  GetSelectedDocument     () const;
    virtual sal_uInt16  GetSelectedTable        () const;
    virtual bool    GetCopyTable            () const;
    virtual bool    GetRenameTable          () const;
    virtual void    GetTabNameString( OUString& rString ) const;
    virtual void    SetForceCopyTable       ();
    virtual void    EnableCopyTable         (sal_Bool bFlag=true);
    virtual void    EnableRenameTable       (sal_Bool bFlag=true);
};

class AbstractScNameCreateDlg_Impl : public AbstractScNameCreateDlg
{
    DECL_ABSTDLG_BASE( AbstractScNameCreateDlg_Impl, ScNameCreateDlg)
    virtual sal_uInt16          GetFlags() const;
};

class AbstractScNamePasteDlg_Impl : public AbstractScNamePasteDlg
{
    DECL_ABSTDLG_BASE( AbstractScNamePasteDlg_Impl, ScNamePasteDlg )
    virtual std::vector<OUString>          GetSelectedNames() const;
    virtual bool                                IsAllSelected() const;
};

class AbstractScPivotFilterDlg_Impl : public AbstractScPivotFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractScPivotFilterDlg_Impl, ScPivotFilterDlg)
    virtual const ScQueryItem&  GetOutputItem();
};

class AbstractScDPFunctionDlg_Impl : public AbstractScDPFunctionDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPFunctionDlg_Impl, ScDPFunctionDlg)
    virtual sal_uInt16 GetFuncMask() const;
    virtual ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const;
};

class AbstractScDPSubtotalDlg_Impl : public AbstractScDPSubtotalDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPSubtotalDlg_Impl, ScDPSubtotalDlg)
    virtual sal_uInt16 GetFuncMask() const;
    virtual void FillLabelData( ScDPLabelData& rLabelData ) const;
};

class AbstractScDPNumGroupDlg_Impl : public AbstractScDPNumGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPNumGroupDlg_Impl, ScDPNumGroupDlg )
    virtual ScDPNumGroupInfo GetGroupInfo() const;
};

class AbstractScDPDateGroupDlg_Impl : public AbstractScDPDateGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPDateGroupDlg_Impl, ScDPDateGroupDlg )
    virtual ScDPNumGroupInfo GetGroupInfo() const;
    virtual sal_Int32 GetDatePart() const;
};

class AbstractScDPShowDetailDlg_Impl : public AbstractScDPShowDetailDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPShowDetailDlg_Impl, ScDPShowDetailDlg)
    virtual String GetDimensionName() const;
};

class AbstractScNewScenarioDlg_Impl : public AbstractScNewScenarioDlg
{
    DECL_ABSTDLG_BASE( AbstractScNewScenarioDlg_Impl, ScNewScenarioDlg )
    virtual void SetScenarioData( const OUString& rName, const OUString& rComment,
                            const Color& rColor, sal_uInt16 nFlags );

    virtual void GetScenarioData( OUString& rName, OUString& rComment,
                            Color& rColor, sal_uInt16& rFlags ) const;
};

class AbstractScShowTabDlg_Impl : public AbstractScShowTabDlg
{
    DECL_ABSTDLG_BASE(AbstractScShowTabDlg_Impl,ScShowTabDlg)
    virtual void    Insert( const String& rString, sal_Bool bSelected );
    virtual sal_uInt16  GetSelectEntryCount() const;
    virtual void SetDescription(const String& rTitle, const String& rFixedText, const OString& sDlgHelpId, const OString& sLbHelpId );
    virtual String  GetSelectEntry(sal_uInt16 nPos) const;
    virtual sal_uInt16  GetSelectEntryPos(sal_uInt16 nPos) const;
};

class AbstractScSortWarningDlg_Impl : public AbstractScSortWarningDlg
{
    DECL_ABSTDLG_BASE( AbstractScSortWarningDlg_Impl, ScSortWarningDlg )
};

class AbstractScStringInputDlg_Impl :  public AbstractScStringInputDlg
{
    DECL_ABSTDLG_BASE( AbstractScStringInputDlg_Impl, ScStringInputDlg )
    virtual void GetInputString( OUString& rString ) const;
};

class AbstractScTabBgColorDlg_Impl :  public AbstractScTabBgColorDlg
{
    DECL_ABSTDLG_BASE( AbstractScTabBgColorDlg_Impl, ScTabBgColorDlg )
    virtual void GetSelectedColor( Color& rColor ) const;
};

class AbstractScImportOptionsDlg_Impl : public AbstractScImportOptionsDlg
{
    DECL_ABSTDLG_BASE( AbstractScImportOptionsDlg_Impl, ScImportOptionsDlg)
    virtual void GetImportOptions( ScImportOptions& rOptions ) const;
};

class AbstractScTextImportOptionsDlg_Impl : public AbstractScTextImportOptionsDlg
{
    DECL_ABSTDLG_BASE( AbstractScTextImportOptionsDlg_Impl, ScTextImportOptionsDlg)
    virtual LanguageType GetLanguageType() const;
    virtual bool IsDateConversionSet() const;
};

class ScAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( ScAbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual void                SetCurPageId( const OString &rName );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
};

//------------------------------------------------------------------------
//AbstractDialogFactory_Impl implementations
class ScAbstractDialogFactory_Impl : public ScAbstractDialogFactory
{

public:
    virtual ~ScAbstractDialogFactory_Impl() {}

    virtual     AbstractScImportAsciiDlg * CreateScImportAsciiDlg( Window* pParent, String aDatName,
                                                                    SvStream* pInStream,
                                                                    ScImportAsciiCall eCall);

    virtual AbstractScTextImportOptionsDlg * CreateScTextImportOptionsDlg(Window* pParent);

    virtual     AbstractScAutoFormatDlg * CreateScAutoFormatDlg( Window*                    pParent,
                                                                ScAutoFormat*               pAutoFormat,
                                                                const ScAutoFormatData*    pSelFormatData,
                                                                ScDocument*                pDoc,
                                                                int nId);
    virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (Window* pParent,
                                                                int nId,
                                                                sal_Bool bCol = false,
                                                                sal_Bool bRow = false);

    virtual VclAbstractDialog * CreateScColOrRowDlg( Window*            pParent,
                                                    const String&   rStrTitle,
                                                    const String&   rStrLabel,
                                                    int nId,
                                                    sal_Bool                bColDefault = sal_True );

    virtual AbstractScSortWarningDlg * CreateScSortWarningDlg(Window* pParent, const String& rExtendText, const String& rCurrentText );

    virtual AbstractScCondFormatManagerDlg* CreateScCondFormatMgrDlg(Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList,
                                                                const ScAddress& rPos, int nId );

    virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg (Window* pParent ,int nId );

    virtual AbstractScDataPilotSourceTypeDlg* CreateScDataPilotSourceTypeDlg(Window* pParent,
        bool bEnableExternal);

    virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( Window* pParent,
                                                                        const com::sun::star::uno::Sequence<OUString>& rServices,
                                                                        int nId );
    virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg(Window* pParent, bool bDisallowCellMove = false );

    //for dataform
    virtual AbstractScDataFormDlg* CreateScDataFormDlg(Window* pParent,
        ScTabViewShell* pTabViewShell);

    virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(Window* pParent,
                                                                    sal_uInt16  nCheckDefaults = 0);

    virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( Window*        pParent,
                                                            ScDocument& rDocument,
                                                            FillDir     eFillDir,
                                                            FillCmd     eFillCmd,
                                                            FillDateCmd eFillDateCmd,
                                                            String          aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16          nPossDir);
    virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( Window* pParent,
                                                            bool bUnGroup = false,
                                                            bool bRows = true);

    virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( Window* pParent,
                                                                int nId,
                                                            sal_Bool bDisallowCellMove = false );

    virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( Window*        pParent,
                                                                    int nId,
                                                                    sal_uInt16      nCheckDefaults = 0,
                                                                    const OUString* pStrTitle = NULL );

    virtual AbstractScInsertTableDlg * CreateScInsertTableDlg(Window* pParent, ScViewData& rViewData,
        SCTAB nTabCount, bool bFromFile);

    virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( Window* pParent,
                                                          const std::vector<String> &rEntryList );

    virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg(Window* pParent);

    virtual AbstractScMetricInputDlg * CreateScMetricInputDlg (  Window*        pParent,
                                                                sal_uInt16      nResId,     // Ableitung fuer jeden Dialog!
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                int nId ,
                                                                FieldUnit       eFUnit    = FUNIT_MM,
                                                                sal_uInt16      nDecimals = 2,
                                                                long            nMaximum  = 1000,
                                                                long            nMinimum  = 0,
                                                                long            nFirst    = 1,
                                                                long          nLast     = 100 );

    virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(Window * pParent,
        const String& rDefault);

    virtual AbstractScNameCreateDlg * CreateScNameCreateDlg(Window * pParent,
        sal_uInt16 nFlags);

    virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( Window * pParent, ScDocShell* pShell, bool bInsList=true );

    virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg ( Window* pParent,
                                                                const SfxItemSet&   rArgSet, sal_uInt16 nSourceTab , int nId );

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( Window* pParent, int nId,
                                                                const ScDPLabelDataVector& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData );

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout );

    virtual AbstractScDPNumGroupDlg * CreateScDPNumGroupDlg( Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo );

    virtual AbstractScDPDateGroupDlg * CreateScDPDateGroupDlg( Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo,
                                                                sal_Int32 nDatePart,
                                                                const Date& rNullDate );

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                sal_uInt16 nOrient );

    virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( Window* pParent, const String& rName,
                                                                int nId,
                                                                sal_Bool bEdit = false, sal_Bool bSheetProtected = false );
    virtual AbstractScShowTabDlg * CreateScShowTabDlg ( Window* pParent, int nId );

    virtual AbstractScStringInputDlg * CreateScStringInputDlg (  Window* pParent,
                                                                const String& rTitle,
                                                                const String& rEditTitle,
                                                                const String& rDefault,
                                                                const OString& sHelpId, const OString& sEditHelpId,
                                                                int nId );

    virtual AbstractScTabBgColorDlg * CreateScTabBgColorDlg (  Window* pParent,
                                                                const String& rTitle, //Dialog Title
                                                                const String& rTabBgColorNoColorText, //Label for no tab color
                                                                const Color& rDefaultColor, //Currently selected Color
                                                                const OString& sHelpId ,
                                                                int nId );

    virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( Window*                 pParent,
                                                                    int nId,
                                                                    sal_Bool                    bAscii = sal_True,
                                                                    const ScImportOptions*  pOptions = NULL,
                                                                    const String*           pStrTitle = NULL,
                                                                    sal_Bool                    bMultiByte = false,
                                                                    sal_Bool                    bOnlyDbtoolsEncodings = false,
                                                                    sal_Bool                    bImport = sal_True );
    virtual SfxAbstractTabDialog * CreateScAttrDlg( SfxViewFrame*    pFrame,
                                                    Window*          pParent,
                                                    const SfxItemSet* pCellAttrs );

    virtual SfxAbstractTabDialog * CreateScHFEditDlg( SfxViewFrame*     pFrame,
                                                    Window*         pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const OUString&     rPageStyle,
                                                    sal_uInt16              nResId = RID_SCDLG_HFEDIT );

    virtual SfxAbstractTabDialog * CreateScStyleDlg( Window*                pParent,
                                                    SfxStyleSheetBase&  rStyleBase,
                                                    sal_uInt16              nRscId,
                                                    int nId);

    virtual SfxAbstractTabDialog * CreateScSubTotalDlg( Window*             pParent,
                                                        const SfxItemSet*   pArgSet,
                                                        int nId);
    virtual SfxAbstractTabDialog * CreateScCharDlg( Window* pParent, const SfxItemSet* pAttr,
                                                    const SfxObjectShell* pDocShell, int nId );

    virtual SfxAbstractTabDialog * CreateScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ,
                                                            int nId );

    virtual SfxAbstractTabDialog * CreateScValidationDlg( Window* pParent,
                                                        const SfxItemSet* pArgSet,int nId, ScTabViewShell *pTabVwSh  );

    virtual SfxAbstractTabDialog * CreateScSortDlg( Window*          pParent,
                                                    const SfxItemSet* pArgSet,int nId );

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId );

    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
