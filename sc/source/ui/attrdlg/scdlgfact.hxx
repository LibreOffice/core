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
    explicit        Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual short   Execute() override ;

#define DECL_ABSTDLG2_BASE(Class,DialogClass)       \
    ScopedVclPtr<DialogClass> pDlg;                 \
public:                                             \
    explicit        Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual void    StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) override; \
    long            GetResult() override;

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
void Class::StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) \
{                                                   \
    pDlg->StartExecuteModal( rEndDialogHdl ) ;      \
}                                                   \
long Class::GetResult()                             \
{                                                   \
    return pDlg->GetResult();                       \
}

class AbstractScImportAsciiDlg_Impl : public AbstractScImportAsciiDlg
{
    DECL_ABSTDLG_BASE(AbstractScImportAsciiDlg_Impl, ScImportAsciiDlg)
    virtual void                        GetOptions( ScAsciiOptions& rOpt ) override;
    virtual void                        SaveParameters() override;
};

class AbstractScAutoFormatDlg_Impl : public AbstractScAutoFormatDlg
{
    DECL_ABSTDLG_BASE(AbstractScAutoFormatDlg_Impl, ScAutoFormatDlg)
    virtual sal_uInt16 GetIndex() const override;
    virtual OUString GetCurrFormatName() override;
};

class AbstractScColRowLabelDlg_Impl : public AbstractScColRowLabelDlg
{
    DECL_ABSTDLG_BASE(AbstractScColRowLabelDlg_Impl,ScColRowLabelDlg)
    virtual bool IsCol() override;
    virtual bool IsRow() override;
};

class AbstractScCondFormatManagerDlg_Impl : public AbstractScCondFormatManagerDlg
{
    DECL_ABSTDLG_BASE(AbstractScCondFormatManagerDlg_Impl, ScCondFormatManagerDlg)

    virtual ScConditionalFormatList* GetConditionalFormatList() override;

    virtual bool CondFormatsChanged() override;

    virtual ScConditionalFormat* GetCondFormatSelected() override;
};

class AbstractScDataPilotDatabaseDlg_Impl  :public AbstractScDataPilotDatabaseDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotDatabaseDlg_Impl, ScDataPilotDatabaseDlg)
    virtual void    GetValues( ScImportSourceDesc& rDesc ) override;
};

class AbstractScDataPilotSourceTypeDlg_Impl  :public AbstractScDataPilotSourceTypeDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotSourceTypeDlg_Impl, ScDataPilotSourceTypeDlg)
    virtual bool IsDatabase() const override;
    virtual bool IsExternal() const override;
    virtual bool IsNamedRange() const override;
    virtual OUString GetSelectedNamedRange() const override;
    virtual void AppendNamedRange(const OUString& rName) override;
};

class AbstractScDataPilotServiceDlg_Impl : public AbstractScDataPilotServiceDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataPilotServiceDlg_Impl, ScDataPilotServiceDlg)
    virtual OUString  GetServiceName() const override;
    virtual OUString  GetParSource() const override;
    virtual OUString  GetParName() const override;
    virtual OUString  GetParUser() const override;
    virtual OUString  GetParPass() const override;
};

class AbstractScDeleteCellDlg_Impl : public AbstractScDeleteCellDlg
{
    DECL_ABSTDLG_BASE(AbstractScDeleteCellDlg_Impl,ScDeleteCellDlg)
    virtual DelCellCmd GetDelCellCmd() const override;
};

//for dataform
class AbstractScDataFormDlg_Impl : public AbstractScDataFormDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataFormDlg_Impl,ScDataFormDlg);
};

class AbstractScDeleteContentsDlg_Impl : public AbstractScDeleteContentsDlg
{
    DECL_ABSTDLG_BASE( AbstractScDeleteContentsDlg_Impl,ScDeleteContentsDlg)
    virtual void    DisableObjects() override;
    virtual InsertDeleteFlags GetDelContentsCmdBits() const override;
};

class AbstractScFillSeriesDlg_Impl:public AbstractScFillSeriesDlg
{
    DECL_ABSTDLG_BASE(AbstractScFillSeriesDlg_Impl, ScFillSeriesDlg)
    virtual FillDir     GetFillDir() const override;
    virtual FillCmd     GetFillCmd() const override;
    virtual FillDateCmd GetFillDateCmd() const override;
    virtual double      GetStart() const override;
    virtual double      GetStep() const override;
    virtual double      GetMax() const override;
    virtual OUString    GetStartStr() const override;
    virtual void        SetEdStartValEnabled(bool bFlag) override;
};

class AbstractScGroupDlg_Impl :  public AbstractScGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScGroupDlg_Impl, ScGroupDlg)
    virtual bool GetColsChecked() const override;
};

class AbstractScInsertCellDlg_Impl : public AbstractScInsertCellDlg
{
    DECL_ABSTDLG_BASE( AbstractScInsertCellDlg_Impl, ScInsertCellDlg)
    virtual InsCellCmd GetInsCellCmd() const override ;
};

class AbstractScInsertContentsDlg_Impl : public AbstractScInsertContentsDlg
{
    DECL_ABSTDLG_BASE(AbstractScInsertContentsDlg_Impl, ScInsertContentsDlg)
    virtual InsertDeleteFlags GetInsContentsCmdBits() const override;
    virtual ScPasteFunc   GetFormulaCmdBits() const override;
    virtual bool        IsSkipEmptyCells() const override;
    virtual bool        IsLink() const override;
    virtual void        SetFillMode( bool bSet ) override;
    virtual void        SetOtherDoc( bool bSet ) override;
    virtual bool        IsTranspose() const override;
    virtual void        SetChangeTrack( bool bSet ) override;
    virtual void        SetCellShiftDisabled( int nDisable ) override;
    virtual InsCellCmd  GetMoveMode() override;
};

class AbstractScInsertTableDlg_Impl : public AbstractScInsertTableDlg
{
    DECL_ABSTDLG_BASE( AbstractScInsertTableDlg_Impl, ScInsertTableDlg)
    virtual bool            GetTablesFromFile() override;
    virtual bool            GetTablesAsLink() override;
    virtual const OUString* GetFirstTable( sal_uInt16* pN = nullptr ) override;
    virtual ScDocShell*     GetDocShellTables() override;
    virtual bool            IsTableBefore() override;
    virtual sal_uInt16      GetTableCount() override;
    virtual const OUString* GetNextTable( sal_uInt16* pN = nullptr ) override;

};

class AbstractScSelEntryDlg_Impl : public AbstractScSelEntryDlg
{
    DECL_ABSTDLG_BASE( AbstractScSelEntryDlg_Impl, ScSelEntryDlg )
    virtual OUString GetSelectEntry() const override;
};

class AbstractScLinkedAreaDlg_Impl : public AbstractScLinkedAreaDlg
{
    DECL_ABSTDLG2_BASE( AbstractScLinkedAreaDlg_Impl, ScLinkedAreaDlg)

    virtual void            InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh ) override;
    virtual OUString        GetURL() override;
    virtual OUString        GetFilter() override;        // may be empty
    virtual OUString        GetOptions() override;       // filter options
    virtual OUString        GetSource() override;        // separated by ";"
    virtual sal_uLong       GetRefresh() override;       // 0 if disabled
};

class AbstractScMetricInputDlg_Impl : public AbstractScMetricInputDlg
{
    DECL_ABSTDLG_BASE( AbstractScMetricInputDlg_Impl, ScMetricInputDlg)
    virtual long GetInputValue() const override;
};

class AbstractScMoveTableDlg_Impl : public AbstractScMoveTableDlg
{
    DECL_ABSTDLG_BASE( AbstractScMoveTableDlg_Impl, ScMoveTableDlg)
    virtual sal_uInt16  GetSelectedDocument     () const override;
    virtual sal_uInt16  GetSelectedTable        () const override;
    virtual bool    GetCopyTable            () const override;
    virtual bool    GetRenameTable          () const override;
    virtual void    GetTabNameString( OUString& rString ) const override;
    virtual void    SetForceCopyTable       () override;
    virtual void    EnableRenameTable       (bool bFlag=true) override;
};

class AbstractScNameCreateDlg_Impl : public AbstractScNameCreateDlg
{
    DECL_ABSTDLG_BASE( AbstractScNameCreateDlg_Impl, ScNameCreateDlg)
    virtual sal_uInt16          GetFlags() const override;
};

class AbstractScNamePasteDlg_Impl : public AbstractScNamePasteDlg
{
    DECL_ABSTDLG_BASE( AbstractScNamePasteDlg_Impl, ScNamePasteDlg )
    virtual std::vector<OUString>          GetSelectedNames() const override;
};

class AbstractScPivotFilterDlg_Impl : public AbstractScPivotFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractScPivotFilterDlg_Impl, ScPivotFilterDlg)
    virtual const ScQueryItem&  GetOutputItem() override;
};

class AbstractScDPFunctionDlg_Impl : public AbstractScDPFunctionDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPFunctionDlg_Impl, ScDPFunctionDlg)
    virtual PivotFunc GetFuncMask() const override;
    virtual css::sheet::DataPilotFieldReference GetFieldRef() const override;
};

class AbstractScDPSubtotalDlg_Impl : public AbstractScDPSubtotalDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPSubtotalDlg_Impl, ScDPSubtotalDlg)
    virtual PivotFunc GetFuncMask() const override;
    virtual void FillLabelData( ScDPLabelData& rLabelData ) const override;
};

class AbstractScDPNumGroupDlg_Impl : public AbstractScDPNumGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPNumGroupDlg_Impl, ScDPNumGroupDlg )
    virtual ScDPNumGroupInfo GetGroupInfo() const override;
};

class AbstractScDPDateGroupDlg_Impl : public AbstractScDPDateGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPDateGroupDlg_Impl, ScDPDateGroupDlg )
    virtual ScDPNumGroupInfo GetGroupInfo() const override;
    virtual sal_Int32 GetDatePart() const override;
};

class AbstractScDPShowDetailDlg_Impl : public AbstractScDPShowDetailDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPShowDetailDlg_Impl, ScDPShowDetailDlg)
    virtual OUString GetDimensionName() const override;
};

class AbstractScNewScenarioDlg_Impl : public AbstractScNewScenarioDlg
{
    DECL_ABSTDLG_BASE( AbstractScNewScenarioDlg_Impl, ScNewScenarioDlg )
    virtual void SetScenarioData( const OUString& rName, const OUString& rComment,
                            const Color& rColor, ScScenarioFlags nFlags ) override;

    virtual void GetScenarioData( OUString& rName, OUString& rComment,
                            Color& rColor, ScScenarioFlags& rFlags ) const override;
};

class AbstractScShowTabDlg_Impl : public AbstractScShowTabDlg
{
    DECL_ABSTDLG_BASE(AbstractScShowTabDlg_Impl,ScShowTabDlg)
    virtual void    Insert( const OUString& rString, bool bSelected ) override;
    virtual sal_Int32 GetSelectEntryCount() const override;
    virtual void SetDescription(const OUString& rTitle, const OUString& rFixedText, const OString& sDlgHelpId, const OString& sLbHelpId ) override;
    virtual OUString  GetSelectEntry(sal_Int32 nPos) const override;
    virtual sal_Int32 GetSelectEntryPos(sal_Int32 nPos) const override;
};

class AbstractScSortWarningDlg_Impl : public AbstractScSortWarningDlg
{
    DECL_ABSTDLG_BASE( AbstractScSortWarningDlg_Impl, ScSortWarningDlg )
};

class AbstractScStringInputDlg_Impl :  public AbstractScStringInputDlg
{
    DECL_ABSTDLG_BASE( AbstractScStringInputDlg_Impl, ScStringInputDlg )
    virtual OUString GetInputString() const override;
};

class AbstractScTabBgColorDlg_Impl :  public AbstractScTabBgColorDlg
{
    DECL_ABSTDLG_BASE( AbstractScTabBgColorDlg_Impl, ScTabBgColorDlg )
    virtual void GetSelectedColor( Color& rColor ) const override;
};

class AbstractScImportOptionsDlg_Impl : public AbstractScImportOptionsDlg
{
    DECL_ABSTDLG_BASE( AbstractScImportOptionsDlg_Impl, ScImportOptionsDlg)
    virtual void GetImportOptions( ScImportOptions& rOptions ) const override;
};

class AbstractScTextImportOptionsDlg_Impl : public AbstractScTextImportOptionsDlg
{
    DECL_ABSTDLG_BASE( AbstractScTextImportOptionsDlg_Impl, ScTextImportOptionsDlg)
    virtual LanguageType GetLanguageType() const override;
    virtual bool IsDateConversionSet() const override;
};

class ScAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( ScAbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

//AbstractDialogFactory_Impl implementations
class ScAbstractDialogFactory_Impl : public ScAbstractDialogFactory
{

public:
    virtual ~ScAbstractDialogFactory_Impl() {}

    virtual     AbstractScImportAsciiDlg * CreateScImportAsciiDlg( const OUString& aDatName,
                                                                    SvStream* pInStream,
                                                                    ScImportAsciiCall eCall) override;

    virtual AbstractScTextImportOptionsDlg * CreateScTextImportOptionsDlg() override;

    virtual     AbstractScAutoFormatDlg * CreateScAutoFormatDlg(vcl::Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData *pViewData) override;
    virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (vcl::Window* pParent,
                                                                bool bCol = false,
                                                                bool bRow = false) override;

    virtual AbstractScSortWarningDlg * CreateScSortWarningDlg(vcl::Window* pParent, const OUString& rExtendText, const OUString& rCurrentText ) override;

    virtual AbstractScCondFormatManagerDlg* CreateScCondFormatMgrDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList,
                                                                int nId ) override;

    virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg(vcl::Window* pParent) override;

    virtual AbstractScDataPilotSourceTypeDlg* CreateScDataPilotSourceTypeDlg(vcl::Window* pParent,
        bool bEnableExternal) override;

    virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( vcl::Window* pParent,
                                                                        const std::vector<OUString>& rServices,
                                                                        int nId ) override;
    virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg(vcl::Window* pParent, bool bDisallowCellMove = false ) override;

    //for dataform
    virtual AbstractScDataFormDlg* CreateScDataFormDlg(vcl::Window* pParent,
        ScTabViewShell* pTabViewShell) override;

    virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(vcl::Window* pParent) override;

    virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( vcl::Window*        pParent,
                                                            ScDocument&     rDocument,
                                                            FillDir         eFillDir,
                                                            FillCmd         eFillCmd,
                                                            FillDateCmd     eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16       nPossDir) override;
    virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( vcl::Window* pParent,
                                                            bool bUnGroup = false) override;

    virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( vcl::Window* pParent,
                                                             int nId,
                                                             bool bDisallowCellMove = false ) override;

    virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( vcl::Window*        pParent,
                                                                    const OUString* pStrTitle = nullptr ) override;

    virtual AbstractScInsertTableDlg * CreateScInsertTableDlg(vcl::Window* pParent, ScViewData& rViewData,
        SCTAB nTabCount, bool bFromFile) override;

    virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( vcl::Window* pParent,
                                                          const std::vector<OUString> &rEntryList ) override;

    virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg(vcl::Window* pParent) override;

    virtual AbstractScMetricInputDlg * CreateScMetricInputDlg (  vcl::Window*        pParent,
                                                                const OString&  sDialogName,
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                FieldUnit       eFUnit    = FUNIT_MM,
                                                                sal_uInt16      nDecimals = 2,
                                                                long            nMaximum  = 1000,
                                                                long            nMinimum  = 0,
                                                                long            nFirst    = 1,
                                                                long            nLast     = 100 ) override;

    virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(vcl::Window * pParent,
        const OUString& rDefault) override;

    virtual AbstractScNameCreateDlg * CreateScNameCreateDlg(vcl::Window * pParent,
        sal_uInt16 nFlags) override;

    virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( vcl::Window * pParent, ScDocShell* pShell, bool bInsList=true ) override;

    virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg(vcl::Window* pParent,
        const SfxItemSet& rArgSet, sal_uInt16 nSourceTab) override;

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( vcl::Window* pParent,
                                                                const ScDPLabelDataVector& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData ) override;

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( vcl::Window* pParent,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields ) override;

    virtual AbstractScDPNumGroupDlg * CreateScDPNumGroupDlg( vcl::Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo ) override;

    virtual AbstractScDPDateGroupDlg * CreateScDPDateGroupDlg( vcl::Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo,
                                                                sal_Int32 nDatePart,
                                                                const Date& rNullDate ) override;

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( vcl::Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                sal_uInt16 nOrient ) override;

    virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( vcl::Window* pParent, const OUString& rName,
                                                                bool bEdit = false, bool bSheetProtected = false ) override;
    virtual AbstractScShowTabDlg * CreateScShowTabDlg(vcl::Window* pParent) override;

    virtual AbstractScStringInputDlg * CreateScStringInputDlg (  vcl::Window* pParent,
                                                                const OUString& rTitle,
                                                                const OUString& rEditTitle,
                                                                const OUString& rDefault,
                                                                const OString& sHelpId, const OString& sEditHelpId ) override;

    virtual AbstractScTabBgColorDlg * CreateScTabBgColorDlg (  vcl::Window* pParent,
                                                                const OUString& rTitle, //Dialog Title
                                                                const OUString& rTabBgColorNoColorText, //Label for no tab color
                                                                const Color& rDefaultColor, //Currently selected Color
                                                                const OString& sHelpId ) override;

    virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( bool                    bAscii = true,
                                                                    const ScImportOptions*  pOptions = nullptr,
                                                                    const OUString*         pStrTitle = nullptr,
                                                                    bool                    bMultiByte = false,
                                                                    bool                    bOnlyDbtoolsEncodings = false,
                                                                    bool                    bImport = true ) override;
    virtual SfxAbstractTabDialog * CreateScAttrDlg( vcl::Window*          pParent,
                                                    const SfxItemSet* pCellAttrs ) override;

    virtual SfxAbstractTabDialog * CreateScHFEditDlg(vcl::Window*         pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const OUString&     rPageStyle,
                                                    sal_uInt16              nResId = RID_SCDLG_HFEDIT ) override;

    virtual SfxAbstractTabDialog * CreateScStyleDlg( vcl::Window*                pParent,
                                                    SfxStyleSheetBase&  rStyleBase,
                                                    sal_uInt16              nRscId,
                                                    int nId) override;

    virtual SfxAbstractTabDialog * CreateScSubTotalDlg( vcl::Window*             pParent,
                                                        const SfxItemSet*   pArgSet ) override;
    virtual SfxAbstractTabDialog * CreateScCharDlg(vcl::Window* pParent,
        const SfxItemSet* pAttr, const SfxObjectShell* pDocShell) override;

    virtual SfxAbstractTabDialog * CreateScParagraphDlg(vcl::Window* pParent,
        const SfxItemSet* pAttr) override;

    virtual SfxAbstractTabDialog * CreateScSortDlg(vcl::Window* pParent, const SfxItemSet* pArgSet) override;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
