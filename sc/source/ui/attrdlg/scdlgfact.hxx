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

#include <scabstdlg.hxx>
#include <sfx2/sfxdlg.hxx>

#include <corodlg.hxx>
#include <condformatmgr.hxx>
#include <dapitype.hxx>
#include <dapidata.hxx>
#include <datafdlg.hxx>
#include <delcodlg.hxx>
#include <delcldlg.hxx>
#include <dpgroupdlg.hxx>
#include <filldlg.hxx>
#include <groupdlg.hxx>
#include <linkarea.hxx>
#include <lbseldlg.hxx>
#include <inscldlg.hxx>
#include <instbdlg.hxx>
#include <inscodlg.hxx>
#include <mtrindlg.hxx>
#include <mvtabdlg.hxx>
#include <namecrea.hxx>
#include <namepast.hxx>
#include <pfiltdlg.hxx>
#include <pvfundlg.hxx>
#include <shtabdlg.hxx>
#include <scendlg.hxx>
#include <scuiasciiopt.hxx>
#include <scuiautofmt.hxx>
#include <scuiimoptdlg.hxx>
#include <sortdlg.hxx>
#include <strindlg.hxx>
#include <tabbgcolordlg.hxx>
#include <textimportoptions.hxx>

#define DECL_ABSTDLG_BASE(Class,DialogClass)        \
    ScopedVclPtr<DialogClass> pDlg;                 \
public:                                             \
    explicit        Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class() override;                       \
    virtual short   Execute() override ;            \
    virtual bool    StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx) override; \
    std::vector<OString> getAllPageUIXMLDescriptions() const override; \
    bool selectPageByUIXMLDescription(const OString& rUIXMLDescription) override; \
    virtual BitmapEx createScreenshot() const override; \
    virtual OString GetScreenshotId() const override; \

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
}                                                   \
short Class::Execute()                              \
{                                                   \
    return pDlg->Execute();                         \
}                                                   \
bool Class::StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx)\
{ \
    return pDlg->StartExecuteAsync( rCtx ); \
} \
std::vector<OString> Class::getAllPageUIXMLDescriptions() const \
{                                                   \
    return pDlg->getAllPageUIXMLDescriptions();     \
}                                                   \
bool Class::selectPageByUIXMLDescription(const OString& rUIXMLDescription) \
{                                                   \
   return pDlg->selectPageByUIXMLDescription(rUIXMLDescription);  \
}                                                   \
BitmapEx Class::createScreenshot() const              \
{                                                   \
    return pDlg->createScreenshot();                \
}                                                   \
OString Class::GetScreenshotId() const              \
{                                                   \
    return pDlg->GetScreenshotId();                 \
}

class AbstractScImportAsciiDlg_Impl : public AbstractScImportAsciiDlg
{
    DECL_ABSTDLG_BASE(AbstractScImportAsciiDlg_Impl, ScImportAsciiDlg)
    virtual void                        GetOptions( ScAsciiOptions& rOpt ) override;
    virtual void                        SaveParameters() override;
};

class AbstractScAutoFormatDlg_Impl : public AbstractScAutoFormatDlg
{
    std::unique_ptr<ScAutoFormatDlg> m_xDlg;
public:
    explicit AbstractScAutoFormatDlg_Impl(std::unique_ptr<ScAutoFormatDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual sal_uInt16 GetIndex() const override;
    virtual OUString GetCurrFormatName() override;
};

class AbstractScColRowLabelDlg_Impl : public AbstractScColRowLabelDlg
{
    std::unique_ptr<ScColRowLabelDlg> m_xDlg;
public:
    explicit AbstractScColRowLabelDlg_Impl(std::unique_ptr<ScColRowLabelDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool IsCol() override;
    virtual bool IsRow() override;
};

class AbstractScCondFormatManagerDlg_Impl : public AbstractScCondFormatManagerDlg
{
    DECL_ABSTDLG_BASE(AbstractScCondFormatManagerDlg_Impl, ScCondFormatManagerDlg)

    virtual std::unique_ptr<ScConditionalFormatList> GetConditionalFormatList() override;

    virtual bool CondFormatsChanged() const override;

    virtual void SetModified() override;

    virtual ScConditionalFormat* GetCondFormatSelected() override;
};

class AbstractScDataPilotDatabaseDlg_Impl  :public AbstractScDataPilotDatabaseDlg
{
    std::unique_ptr<ScDataPilotDatabaseDlg> m_xDlg;
public:
    explicit AbstractScDataPilotDatabaseDlg_Impl(std::unique_ptr<ScDataPilotDatabaseDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void    GetValues( ScImportSourceDesc& rDesc ) override;
};

class AbstractScDataPilotSourceTypeDlg_Impl  :public AbstractScDataPilotSourceTypeDlg
{
    std::unique_ptr<ScDataPilotSourceTypeDlg> m_xDlg;
public:
    explicit AbstractScDataPilotSourceTypeDlg_Impl(std::unique_ptr<ScDataPilotSourceTypeDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool IsDatabase() const override;
    virtual bool IsExternal() const override;
    virtual bool IsNamedRange() const override;
    virtual OUString GetSelectedNamedRange() const override;
    virtual void AppendNamedRange(const OUString& rName) override;
};

class AbstractScDataPilotServiceDlg_Impl : public AbstractScDataPilotServiceDlg
{
    std::unique_ptr<ScDataPilotServiceDlg> m_xDlg;
public:
    explicit AbstractScDataPilotServiceDlg_Impl(std::unique_ptr<ScDataPilotServiceDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short     Execute() override;
    virtual OUString  GetServiceName() const override;
    virtual OUString  GetParSource() const override;
    virtual OUString  GetParName() const override;
    virtual OUString  GetParUser() const override;
    virtual OUString  GetParPass() const override;
};

class AbstractScDeleteCellDlg_Impl : public AbstractScDeleteCellDlg
{
    std::unique_ptr<ScDeleteCellDlg> m_xDlg;
public:
    explicit AbstractScDeleteCellDlg_Impl(std::unique_ptr<ScDeleteCellDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short           Execute() override;
    virtual DelCellCmd GetDelCellCmd() const override;
};

//for dataform
class AbstractScDataFormDlg_Impl : public AbstractScDataFormDlg
{
    DECL_ABSTDLG_BASE(AbstractScDataFormDlg_Impl,ScDataFormDlg);
};

class AbstractScDeleteContentsDlg_Impl : public AbstractScDeleteContentsDlg
{
    std::unique_ptr<ScDeleteContentsDlg> m_xDlg;
public:
    explicit AbstractScDeleteContentsDlg_Impl(std::unique_ptr<ScDeleteContentsDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short   Execute() override;
    virtual void    DisableObjects() override;
    virtual InsertDeleteFlags GetDelContentsCmdBits() const override;
};

class AbstractScFillSeriesDlg_Impl:public AbstractScFillSeriesDlg
{
    std::unique_ptr<ScFillSeriesDlg> m_xDlg;
public:
    explicit AbstractScFillSeriesDlg_Impl(std::unique_ptr<ScFillSeriesDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short       Execute() override;
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
    std::unique_ptr<ScGroupDlg> m_xDlg;
public:
    explicit AbstractScGroupDlg_Impl(std::unique_ptr<ScGroupDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool GetColsChecked() const override;
};

class AbstractScInsertCellDlg_Impl : public AbstractScInsertCellDlg
{
    std::unique_ptr<ScInsertCellDlg> m_xDlg;
public:
    explicit AbstractScInsertCellDlg_Impl(std::unique_ptr<ScInsertCellDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short           Execute() override;
    virtual InsCellCmd GetInsCellCmd() const override ;
};

class AbstractScInsertContentsDlg_Impl : public AbstractScInsertContentsDlg
{
    std::unique_ptr<ScInsertContentsDlg> m_xDlg;
public:
    explicit AbstractScInsertContentsDlg_Impl(std::unique_ptr<ScInsertContentsDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short           Execute() override;
    virtual InsertDeleteFlags GetInsContentsCmdBits() const override;
    virtual ScPasteFunc   GetFormulaCmdBits() const override;
    virtual bool        IsSkipEmptyCells() const override;
    virtual bool        IsLink() const override;
    virtual void        SetFillMode( bool bSet ) override;
    virtual void        SetOtherDoc( bool bSet ) override;
    virtual bool        IsTranspose() const override;
    virtual void        SetChangeTrack( bool bSet ) override;
    virtual void        SetCellShiftDisabled( CellShiftDisabledFlags nDisable ) override;
    virtual InsCellCmd  GetMoveMode() override;
};

class AbstractScInsertTableDlg_Impl : public AbstractScInsertTableDlg
{
    std::unique_ptr<ScInsertTableDlg> m_xDlg;
public:
    explicit AbstractScInsertTableDlg_Impl(std::unique_ptr<ScInsertTableDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short           Execute() override;
    virtual bool            GetTablesFromFile() override;
    virtual bool            GetTablesAsLink() override;
    virtual const OUString* GetFirstTable( sal_uInt16* pN = nullptr ) override;
    virtual ScDocShell*     GetDocShellTables() override;
    virtual bool            IsTableBefore() override;
    virtual sal_uInt16      GetTableCount() override;
    virtual const OUString* GetNextTable( sal_uInt16* pN ) override;

};

class AbstractScSelEntryDlg_Impl : public AbstractScSelEntryDlg
{
    std::unique_ptr<ScSelEntryDlg> m_xDlg;
public:
    explicit AbstractScSelEntryDlg_Impl(std::unique_ptr<ScSelEntryDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short    Execute() override;
    virtual OUString GetSelectedEntry() const override;
};

class AbstractScLinkedAreaDlg_Impl : public AbstractScLinkedAreaDlg
{
    std::unique_ptr<ScLinkedAreaDlg> m_xDlg;
public:
    explicit AbstractScLinkedAreaDlg_Impl(std::unique_ptr<ScLinkedAreaDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual                 ~AbstractScLinkedAreaDlg_Impl() override;
    virtual short           Execute() override;
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
    std::unique_ptr<ScMetricInputDlg> m_xDlg;
public:
    explicit AbstractScMetricInputDlg_Impl(std::unique_ptr<ScMetricInputDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual int GetInputValue() const override;
};

class AbstractScMoveTableDlg_Impl : public AbstractScMoveTableDlg
{
    std::unique_ptr<ScMoveTableDlg> m_xDlg;
public:
    explicit AbstractScMoveTableDlg_Impl(std::unique_ptr<ScMoveTableDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual sal_uInt16  GetSelectedDocument     () const override;
    virtual sal_uInt16  GetSelectedTable        () const override;
    virtual bool    GetCopyTable            () const override;
    virtual bool    GetRenameTable          () const override;
    virtual void    GetTabNameString( OUString& rString ) const override;
    virtual void    SetForceCopyTable       () override;
    virtual void    EnableRenameTable       (bool bFlag) override;
};

class AbstractScNameCreateDlg_Impl : public AbstractScNameCreateDlg
{
    std::unique_ptr<ScNameCreateDlg> m_xDlg;
public:
    explicit AbstractScNameCreateDlg_Impl(std::unique_ptr<ScNameCreateDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short           Execute() override;
    virtual CreateNameFlags GetFlags() const override;
};

class AbstractScNamePasteDlg_Impl : public AbstractScNamePasteDlg
{
    std::unique_ptr<ScNamePasteDlg> m_xDlg;
public:
    explicit AbstractScNamePasteDlg_Impl(std::unique_ptr<ScNamePasteDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short           Execute() override;
    virtual std::vector<OUString>          GetSelectedNames() const override;
};

class AbstractScPivotFilterDlg_Impl : public AbstractScPivotFilterDlg
{
    DECL_ABSTDLG_BASE( AbstractScPivotFilterDlg_Impl, ScPivotFilterDlg)
    virtual const ScQueryItem&  GetOutputItem() override;
};

class AbstractScDPFunctionDlg_Impl : public AbstractScDPFunctionDlg
{
    std::unique_ptr<ScDPFunctionDlg> m_xDlg;
public:
    explicit AbstractScDPFunctionDlg_Impl(std::unique_ptr<ScDPFunctionDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short           Execute() override;
    virtual PivotFunc GetFuncMask() const override;
    virtual css::sheet::DataPilotFieldReference GetFieldRef() const override;
};

class AbstractScDPSubtotalDlg_Impl : public AbstractScDPSubtotalDlg
{
    std::unique_ptr<ScDPSubtotalDlg> m_xDlg;
public:
    explicit AbstractScDPSubtotalDlg_Impl(std::unique_ptr<ScDPSubtotalDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short           Execute() override;
    virtual PivotFunc GetFuncMask() const override;
    virtual void FillLabelData( ScDPLabelData& rLabelData ) const override;
};

class AbstractScDPNumGroupDlg_Impl : public AbstractScDPNumGroupDlg
{
    std::unique_ptr<ScDPNumGroupDlg> m_xDlg;
public:
    explicit AbstractScDPNumGroupDlg_Impl(ScDPNumGroupDlg* p)
        : m_xDlg(p)
    {
    }
    virtual short Execute() override;
    virtual ScDPNumGroupInfo GetGroupInfo() const override;
};

class AbstractScDPDateGroupDlg_Impl : public AbstractScDPDateGroupDlg
{
    std::unique_ptr<ScDPDateGroupDlg> m_xDlg;
public:
    explicit AbstractScDPDateGroupDlg_Impl(ScDPDateGroupDlg* p)
        : m_xDlg(p)
    {
    }
    virtual short Execute() override;
    virtual ScDPNumGroupInfo GetGroupInfo() const override;
    virtual sal_Int32 GetDatePart() const override;
};

class AbstractScDPShowDetailDlg_Impl : public AbstractScDPShowDetailDlg
{
    std::shared_ptr<ScDPShowDetailDlg> m_xDlg;
public:
    explicit AbstractScDPShowDetailDlg_Impl(ScDPShowDetailDlg* p)
        : m_xDlg(p)
    {
    }
    virtual short Execute() override;
    virtual OUString GetDimensionName() const override;
};

class AbstractScNewScenarioDlg_Impl : public AbstractScNewScenarioDlg
{
    std::unique_ptr<ScNewScenarioDlg> m_xDlg;
public:
    explicit AbstractScNewScenarioDlg_Impl(ScNewScenarioDlg* p)
        : m_xDlg(p)
    {
    }
    virtual short           Execute() override;

    virtual void SetScenarioData( const OUString& rName, const OUString& rComment,
                            const Color& rColor, ScScenarioFlags nFlags ) override;

    virtual void GetScenarioData( OUString& rName, OUString& rComment,
                            Color& rColor, ScScenarioFlags& rFlags ) const override;
};

class AbstractScShowTabDlg_Impl : public AbstractScShowTabDlg
{
    std::shared_ptr<ScShowTabDlg> m_xDlg;
public:
    explicit AbstractScShowTabDlg_Impl(ScShowTabDlg* p)
        : m_xDlg(p)
    {
    }
    virtual short Execute() override;
    virtual bool StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx) override;
    virtual void Insert( const OUString& rString, bool bSelected ) override;
    virtual void SetDescription(const OUString& rTitle, const OUString& rFixedText, const OString& sDlgHelpId, const OString& sLbHelpId) override;
    virtual OUString GetEntry(sal_Int32 nPos) const override;
    virtual std::vector<sal_Int32> GetSelectedRows() const override;
};

class AbstractScSortWarningDlg_Impl : public AbstractScSortWarningDlg
{
    std::unique_ptr<ScSortWarningDlg> m_xDlg;
public:
    explicit AbstractScSortWarningDlg_Impl(std::unique_ptr<ScSortWarningDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
};

class AbstractScStringInputDlg_Impl :  public AbstractScStringInputDlg
{
    std::unique_ptr<ScStringInputDlg> m_xDlg;
public:
    explicit AbstractScStringInputDlg_Impl(std::unique_ptr<ScStringInputDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual OUString GetInputString() const override;
};

class AbstractScTabBgColorDlg_Impl :  public AbstractScTabBgColorDlg
{
    std::unique_ptr<ScTabBgColorDlg> m_xDlg;
public:
    explicit AbstractScTabBgColorDlg_Impl(std::unique_ptr<ScTabBgColorDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void GetSelectedColor( Color& rColor ) const override;
};

class AbstractScImportOptionsDlg_Impl : public AbstractScImportOptionsDlg
{
    std::unique_ptr<ScImportOptionsDlg> m_xDlg;
public:
    explicit AbstractScImportOptionsDlg_Impl(std::unique_ptr<ScImportOptionsDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual void GetImportOptions( ScImportOptions& rOptions ) const override;
    virtual void SaveImportOptions() const override;
};

class AbstractScTextImportOptionsDlg_Impl : public AbstractScTextImportOptionsDlg
{
    std::unique_ptr<ScTextImportOptionsDlg> m_xDlg;
public:
    explicit AbstractScTextImportOptionsDlg_Impl(std::unique_ptr<ScTextImportOptionsDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual LanguageType GetLanguageType() const override;
    virtual bool IsDateConversionSet() const override;
};

class ScAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( ScAbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
};

class ScAbstractTabController_Impl : public SfxAbstractTabDialog
{
    std::shared_ptr<SfxTabDialogController> m_xDlg;
public:
    explicit ScAbstractTabController_Impl(std::unique_ptr<SfxTabDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool  StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*   GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
};

//AbstractDialogFactory_Impl implementations
class ScAbstractDialogFactory_Impl : public ScAbstractDialogFactory
{

public:
    virtual ~ScAbstractDialogFactory_Impl() {}

    virtual VclPtr<AbstractScImportAsciiDlg> CreateScImportAsciiDlg(vcl::Window* pParent,
                                                                    const OUString& aDatName,
                                                                    SvStream* pInStream,
                                                                    ScImportAsciiCall eCall) override;

    virtual VclPtr<AbstractScTextImportOptionsDlg> CreateScTextImportOptionsDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScAutoFormatDlg> CreateScAutoFormatDlg(weld::Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData *pViewData) override;
    virtual VclPtr<AbstractScColRowLabelDlg> CreateScColRowLabelDlg (weld::Window* pParent,
                                                                bool bCol,
                                                                bool bRow) override;

    virtual VclPtr<AbstractScSortWarningDlg> CreateScSortWarningDlg(weld::Window* pParent, const OUString& rExtendText, const OUString& rCurrentText ) override;

    virtual VclPtr<AbstractScCondFormatManagerDlg> CreateScCondFormatMgrDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList ) override;

    virtual VclPtr<AbstractScDataPilotDatabaseDlg> CreateScDataPilotDatabaseDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScDataPilotSourceTypeDlg> CreateScDataPilotSourceTypeDlg(weld::Window* pParent,
        bool bEnableExternal) override;

    virtual VclPtr<AbstractScDataPilotServiceDlg> CreateScDataPilotServiceDlg(weld::Window* pParent,
                                                                              const std::vector<OUString>& rServices) override;
    virtual VclPtr<AbstractScDeleteCellDlg> CreateScDeleteCellDlg(weld::Window* pParent, bool bDisallowCellMove ) override;

    //for dataform
    virtual VclPtr<AbstractScDataFormDlg> CreateScDataFormDlg(vcl::Window* pParent,
        ScTabViewShell* pTabViewShell) override;

    virtual VclPtr<AbstractScDeleteContentsDlg> CreateScDeleteContentsDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScFillSeriesDlg> CreateScFillSeriesDlg(weld::Window*        pParent,
                                                            ScDocument&     rDocument,
                                                            FillDir         eFillDir,
                                                            FillCmd         eFillCmd,
                                                            FillDateCmd     eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16       nPossDir) override;
    virtual VclPtr<AbstractScGroupDlg> CreateAbstractScGroupDlg(weld::Window* pParent, bool bUnGroup = false) override;

    virtual VclPtr<AbstractScInsertCellDlg> CreateScInsertCellDlg(weld::Window* pParent,
                                                                  bool bDisallowCellMove) override;

    virtual VclPtr<AbstractScInsertContentsDlg> CreateScInsertContentsDlg(weld::Window* pParent,
                                                                          const OUString* pStrTitle = nullptr) override;

    virtual VclPtr<AbstractScInsertTableDlg> CreateScInsertTableDlg(weld::Window* pParent, ScViewData& rViewData,
        SCTAB nTabCount, bool bFromFile) override;

    virtual VclPtr<AbstractScSelEntryDlg> CreateScSelEntryDlg(weld::Window* pParent, const std::vector<OUString> &rEntryList) override;

    virtual VclPtr<AbstractScLinkedAreaDlg> CreateScLinkedAreaDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScMetricInputDlg> CreateScMetricInputDlg(weld::Window* pParent,
                                                                const OString&  sDialogName,
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                FieldUnit       eFUnit,
                                                                sal_uInt16      nDecimals,
                                                                long            nMaximum,
                                                                long            nMinimum  = 0 ) override;

    virtual VclPtr<AbstractScMoveTableDlg> CreateScMoveTableDlg(weld::Window * pParent,
        const OUString& rDefault) override;

    virtual VclPtr<AbstractScNameCreateDlg> CreateScNameCreateDlg(weld::Window * pParent,
        CreateNameFlags nFlags) override;

    virtual VclPtr<AbstractScNamePasteDlg> CreateScNamePasteDlg(weld::Window * pParent, ScDocShell* pShell) override;

    virtual VclPtr<AbstractScPivotFilterDlg> CreateScPivotFilterDlg(vcl::Window* pParent,
        const SfxItemSet& rArgSet, sal_uInt16 nSourceTab) override;

    virtual VclPtr<AbstractScDPFunctionDlg> CreateScDPFunctionDlg(weld::Window* pParent,
                                                                  const ScDPLabelDataVector& rLabelVec,
                                                                  const ScDPLabelData& rLabelData,
                                                                  const ScPivotFuncData& rFuncData ) override;

    virtual VclPtr<AbstractScDPSubtotalDlg> CreateScDPSubtotalDlg(weld::Window* pParent,
                                                                  ScDPObject& rDPObj,
                                                                  const ScDPLabelData& rLabelData,
                                                                  const ScPivotFuncData& rFuncData,
                                                                  const ScDPNameVec& rDataFields ) override;

    virtual VclPtr<AbstractScDPNumGroupDlg> CreateScDPNumGroupDlg(weld::Window* pParent,
                                                                  const ScDPNumGroupInfo& rInfo) override;

    virtual VclPtr<AbstractScDPDateGroupDlg> CreateScDPDateGroupDlg(weld::Window* pParent,
                                                                    const ScDPNumGroupInfo& rInfo,
                                                                    sal_Int32 nDatePart,
                                                                    const Date& rNullDate) override;

    virtual VclPtr<AbstractScDPShowDetailDlg> CreateScDPShowDetailDlg(weld::Window* pParent,
                                                                ScDPObject& rDPObj,
                                                                css::sheet::DataPilotFieldOrientation nOrient) override;

    virtual VclPtr<AbstractScNewScenarioDlg> CreateScNewScenarioDlg(weld::Window* pParent, const OUString& rName,
                                                                    bool bEdit, bool bSheetProtected) override;
    virtual VclPtr<AbstractScShowTabDlg> CreateScShowTabDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScStringInputDlg> CreateScStringInputDlg(weld::Window* pParent,
                                                                    const OUString& rTitle,
                                                                    const OUString& rEditTitle,
                                                                    const OUString& rDefault,
                                                                    const OString& rHelpId,
                                                                    const OString& rEditHelpId) override;

    virtual VclPtr<AbstractScTabBgColorDlg> CreateScTabBgColorDlg(weld::Window* pParent,
                                                                  const OUString& rTitle, //Dialog Title
                                                                  const OUString& rTabBgColorNoColorText, //Label for no tab color
                                                                  const Color& rDefaultColor) override; //Currently selected Color

    virtual VclPtr<AbstractScImportOptionsDlg> CreateScImportOptionsDlg(weld::Window* pParent, bool bAscii,
                                                                        const ScImportOptions* pOptions,
                                                                        const OUString* pStrTitle,
                                                                        bool bOnlyDbtoolsEncodings,
                                                                        bool bImport = true) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScAttrDlg(weld::Window* pParent,
                                                         const SfxItemSet* pCellAttrs) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScHFEditDlg(vcl::Window*       pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const OUString&     rPageStyle,
                                                    sal_uInt16          nResId ) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScStyleDlg(weld::Window* pParent,
                                                          SfxStyleSheetBase& rStyleBase,
                                                          bool bPage) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScSubTotalDlg(weld::Window* pParent,
                                                             const SfxItemSet* pArgSet) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateScCharDlg(weld::Window* pParent,
        const SfxItemSet* pAttr, const SfxObjectShell* pDocShell) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScParagraphDlg(weld::Window* pParent,
        const SfxItemSet* pAttr) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScSortDlg(weld::Window* pParent, const SfxItemSet* pArgSet) override;

    // For TabPage
    virtual CreateTabPage                GetTabPageCreatorFunc( sal_uInt16 nId ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
