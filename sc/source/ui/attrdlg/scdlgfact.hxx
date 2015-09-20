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
#ifndef INCLUDED_SC_SOURCE_UI_ATTRDLG_SCDLGFACT_HXX
#define INCLUDED_SC_SOURCE_UI_ATTRDLG_SCDLGFACT_HXX

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
    ScopedVclPtr<DialogClass> pDlg;                 \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual short   Execute() SAL_OVERRIDE ;

#define DECL_ABSTDLG2_BASE(Class,DialogClass)       \
    ScopedVclPtr<DialogClass> pDlg;                 \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual void    StartExecuteModal( const Link<>& rEndDialogHdl ) SAL_OVERRIDE; \
    long            GetResult() SAL_OVERRIDE;

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
}                                                   \
short Class::Execute()                              \
{                                                   \
    return pDlg->Execute();                         \
}

#define IMPL_ABSTDLG2_BASE(Class)                   \
Class::~Class()                                     \
{                                                   \
}                                                   \
void Class::StartExecuteModal( const Link<>& rEndDialogHdl ) \
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
    virtual void                        GetOptions( ScAsciiOptions& rOpt ) SAL_OVERRIDE;
    virtual void                        SaveParameters() SAL_OVERRIDE;
};

class AbstractScAutoFormatDlg_Impl : public AbstractScAutoFormatDlg
{
    DECL_ABSTDLG_BASE(AbstractScAutoFormatDlg_Impl, ScAutoFormatDlg)
    virtual sal_uInt16 GetIndex() const SAL_OVERRIDE;
    virtual OUString GetCurrFormatName() SAL_OVERRIDE;
};

class AbstractScColRowLabelDlg_Impl : public AbstractScColRowLabelDlg
{
    DECL_ABSTDLG_BASE(AbstractScColRowLabelDlg_Impl,ScColRowLabelDlg)
    virtual bool IsCol() SAL_OVERRIDE;
    virtual bool IsRow() SAL_OVERRIDE;
};

class AbstractScCondFormatManagerDlg_Impl : public AbstractScCondFormatManagerDlg
{
    DECL_ABSTDLG_BASE(AbstractScCondFormatManagerDlg_Impl, ScCondFormatManagerDlg)

    virtual ScConditionalFormatList* GetConditionalFormatList() SAL_OVERRIDE;

    virtual bool CondFormatsChanged() SAL_OVERRIDE;

    virtual ScConditionalFormat* GetCondFormatSelected() SAL_OVERRIDE;
};

class AbstractScDataPilotDatabaseDlg_Impl  :public AbstractScDataPilotDatabaseDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotDatabaseDlg_Impl, ScDataPilotDatabaseDlg)
    virtual void    GetValues( ScImportSourceDesc& rDesc ) SAL_OVERRIDE;
};

class AbstractScDataPilotSourceTypeDlg_Impl  :public AbstractScDataPilotSourceTypeDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotSourceTypeDlg_Impl, ScDataPilotSourceTypeDlg)
    virtual bool IsDatabase() const SAL_OVERRIDE;
    virtual bool IsExternal() const SAL_OVERRIDE;
    virtual bool IsNamedRange() const SAL_OVERRIDE;
    virtual OUString GetSelectedNamedRange() const SAL_OVERRIDE;
    virtual void AppendNamedRange(const OUString& rName) SAL_OVERRIDE;
};

class AbstractScDataPilotServiceDlg_Impl : public AbstractScDataPilotServiceDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotServiceDlg_Impl, ScDataPilotServiceDlg)
    virtual OUString  GetServiceName() const SAL_OVERRIDE;
    virtual OUString  GetParSource() const SAL_OVERRIDE;
    virtual OUString  GetParName() const SAL_OVERRIDE;
    virtual OUString  GetParUser() const SAL_OVERRIDE;
    virtual OUString  GetParPass() const SAL_OVERRIDE;
};

class AbstractScDeleteCellDlg_Impl : public AbstractScDeleteCellDlg
{
    DECL_ABSTDLG_BASE(AbstractScDeleteCellDlg_Impl,ScDeleteCellDlg)
    virtual DelCellCmd GetDelCellCmd() const SAL_OVERRIDE;
};

//for dataform
class AbstractScDataFormDlg_Impl : public AbstractScDataFormDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataFormDlg_Impl,ScDataFormDlg);
};

class AbstractScDeleteContentsDlg_Impl : public AbstractScDeleteContentsDlg
{
    DECL_ABSTDLG_BASE( AbstractScDeleteContentsDlg_Impl,ScDeleteContentsDlg)
    virtual void    DisableObjects() SAL_OVERRIDE;
    virtual InsertDeleteFlags GetDelContentsCmdBits() const SAL_OVERRIDE;
};

class AbstractScFillSeriesDlg_Impl:public AbstractScFillSeriesDlg
{
    DECL_ABSTDLG_BASE(AbstractScFillSeriesDlg_Impl, ScFillSeriesDlg)
    virtual FillDir     GetFillDir() const SAL_OVERRIDE;
    virtual FillCmd     GetFillCmd() const SAL_OVERRIDE;
    virtual FillDateCmd GetFillDateCmd() const SAL_OVERRIDE;
    virtual double      GetStart() const SAL_OVERRIDE;
    virtual double      GetStep() const SAL_OVERRIDE;
    virtual double      GetMax() const SAL_OVERRIDE;
    virtual OUString    GetStartStr() const SAL_OVERRIDE;
    virtual void        SetEdStartValEnabled(bool bFlag=false) SAL_OVERRIDE;
};

class AbstractScGroupDlg_Impl :  public AbstractScGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScGroupDlg_Impl, ScGroupDlg)
    virtual bool GetColsChecked() const SAL_OVERRIDE;
};

class AbstractScInsertCellDlg_Impl : public AbstractScInsertCellDlg
{
    DECL_ABSTDLG_BASE( AbstractScInsertCellDlg_Impl, ScInsertCellDlg)
    virtual InsCellCmd GetInsCellCmd() const SAL_OVERRIDE ;
};

class AbstractScInsertContentsDlg_Impl : public AbstractScInsertContentsDlg
{
    DECL_ABSTDLG_BASE(AbstractScInsertContentsDlg_Impl, ScInsertContentsDlg)
    virtual InsertDeleteFlags GetInsContentsCmdBits() const SAL_OVERRIDE;
    virtual ScPasteFunc   GetFormulaCmdBits() const SAL_OVERRIDE;
    virtual bool        IsSkipEmptyCells() const SAL_OVERRIDE;
    virtual bool        IsLink() const SAL_OVERRIDE;
    virtual void        SetFillMode( bool bSet ) SAL_OVERRIDE;
    virtual void        SetOtherDoc( bool bSet ) SAL_OVERRIDE;
    virtual bool        IsTranspose() const SAL_OVERRIDE;
    virtual void        SetChangeTrack( bool bSet ) SAL_OVERRIDE;
    virtual void        SetCellShiftDisabled( int nDisable ) SAL_OVERRIDE;
    virtual InsCellCmd  GetMoveMode() SAL_OVERRIDE;
};

class AbstractScInsertTableDlg_Impl : public AbstractScInsertTableDlg
{
    DECL_ABSTDLG_BASE( AbstractScInsertTableDlg_Impl, ScInsertTableDlg)
    virtual bool            GetTablesFromFile() SAL_OVERRIDE;
    virtual bool            GetTablesAsLink() SAL_OVERRIDE;
    virtual const OUString* GetFirstTable( sal_uInt16* pN = NULL ) SAL_OVERRIDE;
    virtual ScDocShell*     GetDocShellTables() SAL_OVERRIDE;
    virtual bool            IsTableBefore() SAL_OVERRIDE;
    virtual sal_uInt16      GetTableCount() SAL_OVERRIDE;
    virtual const OUString* GetNextTable( sal_uInt16* pN = NULL ) SAL_OVERRIDE;

};

class AbstractScSelEntryDlg_Impl : public AbstractScSelEntryDlg
{
    DECL_ABSTDLG_BASE( AbstractScSelEntryDlg_Impl, ScSelEntryDlg )
    virtual OUString GetSelectEntry() const SAL_OVERRIDE;
};

class AbstractScLinkedAreaDlg_Impl : public AbstractScLinkedAreaDlg
{
    DECL_ABSTDLG2_BASE( AbstractScLinkedAreaDlg_Impl, ScLinkedAreaDlg)

    virtual void            InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh ) SAL_OVERRIDE;
    virtual OUString        GetURL() SAL_OVERRIDE;
    virtual OUString        GetFilter() SAL_OVERRIDE;        // may be empty
    virtual OUString        GetOptions() SAL_OVERRIDE;       // filter options
    virtual OUString        GetSource() SAL_OVERRIDE;        // separated by ";"
    virtual sal_uLong       GetRefresh() SAL_OVERRIDE;       // 0 if disabled
};

class AbstractScMetricInputDlg_Impl : public AbstractScMetricInputDlg
{
    DECL_ABSTDLG_BASE( AbstractScMetricInputDlg_Impl, ScMetricInputDlg)
    virtual long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const SAL_OVERRIDE;
};

class AbstractScMoveTableDlg_Impl : public AbstractScMoveTableDlg
{
    DECL_ABSTDLG_BASE( AbstractScMoveTableDlg_Impl, ScMoveTableDlg)
    virtual sal_uInt16  GetSelectedDocument     () const SAL_OVERRIDE;
    virtual sal_uInt16  GetSelectedTable        () const SAL_OVERRIDE;
    virtual bool    GetCopyTable            () const SAL_OVERRIDE;
    virtual bool    GetRenameTable          () const SAL_OVERRIDE;
    virtual void    GetTabNameString( OUString& rString ) const SAL_OVERRIDE;
    virtual void    SetForceCopyTable       () SAL_OVERRIDE;
    virtual void    EnableRenameTable       (bool bFlag=true) SAL_OVERRIDE;
};

class AbstractScNameCreateDlg_Impl : public AbstractScNameCreateDlg
{
    DECL_ABSTDLG_BASE( AbstractScNameCreateDlg_Impl, ScNameCreateDlg)
    virtual sal_uInt16          GetFlags() const SAL_OVERRIDE;
};

class AbstractScNamePasteDlg_Impl : public AbstractScNamePasteDlg
{
    DECL_ABSTDLG_BASE( AbstractScNamePasteDlg_Impl, ScNamePasteDlg )
    virtual std::vector<OUString>          GetSelectedNames() const SAL_OVERRIDE;
};

class AbstractScPivotFilterDlg_Impl : public AbstractScPivotFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractScPivotFilterDlg_Impl, ScPivotFilterDlg)
    virtual const ScQueryItem&  GetOutputItem() SAL_OVERRIDE;
};

class AbstractScDPFunctionDlg_Impl : public AbstractScDPFunctionDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPFunctionDlg_Impl, ScDPFunctionDlg)
    virtual sal_uInt16 GetFuncMask() const SAL_OVERRIDE;
    virtual ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const SAL_OVERRIDE;
};

class AbstractScDPSubtotalDlg_Impl : public AbstractScDPSubtotalDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPSubtotalDlg_Impl, ScDPSubtotalDlg)
    virtual sal_uInt16 GetFuncMask() const SAL_OVERRIDE;
    virtual void FillLabelData( ScDPLabelData& rLabelData ) const SAL_OVERRIDE;
};

class AbstractScDPNumGroupDlg_Impl : public AbstractScDPNumGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPNumGroupDlg_Impl, ScDPNumGroupDlg )
    virtual ScDPNumGroupInfo GetGroupInfo() const SAL_OVERRIDE;
};

class AbstractScDPDateGroupDlg_Impl : public AbstractScDPDateGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPDateGroupDlg_Impl, ScDPDateGroupDlg )
    virtual ScDPNumGroupInfo GetGroupInfo() const SAL_OVERRIDE;
    virtual sal_Int32 GetDatePart() const SAL_OVERRIDE;
};

class AbstractScDPShowDetailDlg_Impl : public AbstractScDPShowDetailDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPShowDetailDlg_Impl, ScDPShowDetailDlg)
    virtual OUString GetDimensionName() const SAL_OVERRIDE;
};

class AbstractScNewScenarioDlg_Impl : public AbstractScNewScenarioDlg
{
    DECL_ABSTDLG_BASE( AbstractScNewScenarioDlg_Impl, ScNewScenarioDlg )
    virtual void SetScenarioData( const OUString& rName, const OUString& rComment,
                            const Color& rColor, sal_uInt16 nFlags ) SAL_OVERRIDE;

    virtual void GetScenarioData( OUString& rName, OUString& rComment,
                            Color& rColor, sal_uInt16& rFlags ) const SAL_OVERRIDE;
};

class AbstractScShowTabDlg_Impl : public AbstractScShowTabDlg
{
    DECL_ABSTDLG_BASE(AbstractScShowTabDlg_Impl,ScShowTabDlg)
    virtual void    Insert( const OUString& rString, bool bSelected ) SAL_OVERRIDE;
    virtual sal_Int32 GetSelectEntryCount() const SAL_OVERRIDE;
    virtual void SetDescription(const OUString& rTitle, const OUString& rFixedText, const OString& sDlgHelpId, const OString& sLbHelpId ) SAL_OVERRIDE;
    virtual OUString  GetSelectEntry(sal_Int32 nPos) const SAL_OVERRIDE;
    virtual sal_Int32 GetSelectEntryPos(sal_Int32 nPos) const SAL_OVERRIDE;
};

class AbstractScSortWarningDlg_Impl : public AbstractScSortWarningDlg
{
    DECL_ABSTDLG_BASE( AbstractScSortWarningDlg_Impl, ScSortWarningDlg )
};

class AbstractScStringInputDlg_Impl :  public AbstractScStringInputDlg
{
    DECL_ABSTDLG_BASE( AbstractScStringInputDlg_Impl, ScStringInputDlg )
    virtual OUString GetInputString() const SAL_OVERRIDE;
};

class AbstractScTabBgColorDlg_Impl :  public AbstractScTabBgColorDlg
{
    DECL_ABSTDLG_BASE( AbstractScTabBgColorDlg_Impl, ScTabBgColorDlg )
    virtual void GetSelectedColor( Color& rColor ) const SAL_OVERRIDE;
};

class AbstractScImportOptionsDlg_Impl : public AbstractScImportOptionsDlg
{
    DECL_ABSTDLG_BASE( AbstractScImportOptionsDlg_Impl, ScImportOptionsDlg)
    virtual void GetImportOptions( ScImportOptions& rOptions ) const SAL_OVERRIDE;
};

class AbstractScTextImportOptionsDlg_Impl : public AbstractScTextImportOptionsDlg
{
    DECL_ABSTDLG_BASE( AbstractScTextImportOptionsDlg_Impl, ScTextImportOptionsDlg)
    virtual LanguageType GetLanguageType() const SAL_OVERRIDE;
    virtual bool IsDateConversionSet() const SAL_OVERRIDE;
};

class ScAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( ScAbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString &rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

//AbstractDialogFactory_Impl implementations
class ScAbstractDialogFactory_Impl : public ScAbstractDialogFactory
{

public:
    virtual ~ScAbstractDialogFactory_Impl() {}

    virtual     AbstractScImportAsciiDlg * CreateScImportAsciiDlg( vcl::Window* pParent, const OUString& aDatName,
                                                                    SvStream* pInStream,
                                                                    ScImportAsciiCall eCall) SAL_OVERRIDE;

    virtual AbstractScTextImportOptionsDlg * CreateScTextImportOptionsDlg(vcl::Window* pParent) SAL_OVERRIDE;

    virtual     AbstractScAutoFormatDlg * CreateScAutoFormatDlg(vcl::Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData *pViewData) SAL_OVERRIDE;
    virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (vcl::Window* pParent,
                                                                bool bCol = false,
                                                                bool bRow = false) SAL_OVERRIDE;

    virtual AbstractScSortWarningDlg * CreateScSortWarningDlg(vcl::Window* pParent, const OUString& rExtendText, const OUString& rCurrentText ) SAL_OVERRIDE;

    virtual AbstractScCondFormatManagerDlg* CreateScCondFormatMgrDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList,
                                                                const ScAddress& rPos, int nId ) SAL_OVERRIDE;

    virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg(vcl::Window* pParent) SAL_OVERRIDE;

    virtual AbstractScDataPilotSourceTypeDlg* CreateScDataPilotSourceTypeDlg(vcl::Window* pParent,
        bool bEnableExternal) SAL_OVERRIDE;

    virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( vcl::Window* pParent,
                                                                        const com::sun::star::uno::Sequence<OUString>& rServices,
                                                                        int nId ) SAL_OVERRIDE;
    virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg(vcl::Window* pParent, bool bDisallowCellMove = false ) SAL_OVERRIDE;

    //for dataform
    virtual AbstractScDataFormDlg* CreateScDataFormDlg(vcl::Window* pParent,
        ScTabViewShell* pTabViewShell) SAL_OVERRIDE;

    virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(vcl::Window* pParent,
                                                                    InsertDeleteFlags nCheckDefaults = IDF_NONE) SAL_OVERRIDE;

    virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( vcl::Window*        pParent,
                                                            ScDocument&     rDocument,
                                                            FillDir         eFillDir,
                                                            FillCmd         eFillCmd,
                                                            FillDateCmd     eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16       nPossDir) SAL_OVERRIDE;
    virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( vcl::Window* pParent,
                                                            bool bUnGroup = false,
                                                            bool bRows = true) SAL_OVERRIDE;

    virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( vcl::Window* pParent,
                                                             int nId,
                                                             bool bDisallowCellMove = false ) SAL_OVERRIDE;

    virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( vcl::Window*        pParent,
                                                                    InsertDeleteFlags nCheckDefaults = IDF_NONE,
                                                                    const OUString* pStrTitle = NULL ) SAL_OVERRIDE;

    virtual AbstractScInsertTableDlg * CreateScInsertTableDlg(vcl::Window* pParent, ScViewData& rViewData,
        SCTAB nTabCount, bool bFromFile) SAL_OVERRIDE;

    virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( vcl::Window* pParent,
                                                          const std::vector<OUString> &rEntryList ) SAL_OVERRIDE;

    virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg(vcl::Window* pParent) SAL_OVERRIDE;

    virtual AbstractScMetricInputDlg * CreateScMetricInputDlg (  vcl::Window*        pParent,
                                                                const OString&  sDialogName,
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                FieldUnit       eFUnit    = FUNIT_MM,
                                                                sal_uInt16      nDecimals = 2,
                                                                long            nMaximum  = 1000,
                                                                long            nMinimum  = 0,
                                                                long            nFirst    = 1,
                                                                long            nLast     = 100 ) SAL_OVERRIDE;

    virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(vcl::Window * pParent,
        const OUString& rDefault) SAL_OVERRIDE;

    virtual AbstractScNameCreateDlg * CreateScNameCreateDlg(vcl::Window * pParent,
        sal_uInt16 nFlags) SAL_OVERRIDE;

    virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( vcl::Window * pParent, ScDocShell* pShell, bool bInsList=true ) SAL_OVERRIDE;

    virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg(vcl::Window* pParent,
        const SfxItemSet& rArgSet, sal_uInt16 nSourceTab) SAL_OVERRIDE;

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( vcl::Window* pParent,
                                                                const ScDPLabelDataVector& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData ) SAL_OVERRIDE;

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( vcl::Window* pParent,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout ) SAL_OVERRIDE;

    virtual AbstractScDPNumGroupDlg * CreateScDPNumGroupDlg( vcl::Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo ) SAL_OVERRIDE;

    virtual AbstractScDPDateGroupDlg * CreateScDPDateGroupDlg( vcl::Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo,
                                                                sal_Int32 nDatePart,
                                                                const Date& rNullDate ) SAL_OVERRIDE;

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( vcl::Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                sal_uInt16 nOrient ) SAL_OVERRIDE;

    virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( vcl::Window* pParent, const OUString& rName,
                                                                bool bEdit = false, bool bSheetProtected = false ) SAL_OVERRIDE;
    virtual AbstractScShowTabDlg * CreateScShowTabDlg(vcl::Window* pParent) SAL_OVERRIDE;

    virtual AbstractScStringInputDlg * CreateScStringInputDlg (  vcl::Window* pParent,
                                                                const OUString& rTitle,
                                                                const OUString& rEditTitle,
                                                                const OUString& rDefault,
                                                                const OString& sHelpId, const OString& sEditHelpId ) SAL_OVERRIDE;

    virtual AbstractScTabBgColorDlg * CreateScTabBgColorDlg (  vcl::Window* pParent,
                                                                const OUString& rTitle, //Dialog Title
                                                                const OUString& rTabBgColorNoColorText, //Label for no tab color
                                                                const Color& rDefaultColor, //Currently selected Color
                                                                const OString& sHelpId ) SAL_OVERRIDE;

    virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( vcl::Window*                 pParent,
                                                                    bool                    bAscii = true,
                                                                    const ScImportOptions*  pOptions = NULL,
                                                                    const OUString*         pStrTitle = NULL,
                                                                    bool                    bMultiByte = false,
                                                                    bool                    bOnlyDbtoolsEncodings = false,
                                                                    bool                    bImport = true ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog * CreateScAttrDlg( SfxViewFrame*    pFrame,
                                                    vcl::Window*          pParent,
                                                    const SfxItemSet* pCellAttrs ) SAL_OVERRIDE;

    virtual SfxAbstractTabDialog * CreateScHFEditDlg( SfxViewFrame*     pFrame,
                                                    vcl::Window*         pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const OUString&     rPageStyle,
                                                    sal_uInt16              nResId = RID_SCDLG_HFEDIT ) SAL_OVERRIDE;

    virtual SfxAbstractTabDialog * CreateScStyleDlg( vcl::Window*                pParent,
                                                    SfxStyleSheetBase&  rStyleBase,
                                                    sal_uInt16              nRscId,
                                                    int nId) SAL_OVERRIDE;

    virtual SfxAbstractTabDialog * CreateScSubTotalDlg( vcl::Window*             pParent,
                                                        const SfxItemSet*   pArgSet ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog * CreateScCharDlg(vcl::Window* pParent,
        const SfxItemSet* pAttr, const SfxObjectShell* pDocShell) SAL_OVERRIDE;

    virtual SfxAbstractTabDialog * CreateScParagraphDlg(vcl::Window* pParent,
        const SfxItemSet* pAttr) SAL_OVERRIDE;

    virtual SfxAbstractTabDialog * CreateScValidationDlg(vcl::Window* pParent,
        const SfxItemSet* pArgSet, ScTabViewShell *pTabVwSh) SAL_OVERRIDE;

    virtual SfxAbstractTabDialog * CreateScSortDlg(vcl::Window* pParent, const SfxItemSet* pArgSet) SAL_OVERRIDE;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) SAL_OVERRIDE;

    virtual GetTabPageRanges            GetTabPageRangesFunc() SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
