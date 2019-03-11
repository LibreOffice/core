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

#ifndef INCLUDED_SC_INC_SCABSTDLG_HXX
#define INCLUDED_SC_INC_SCABSTDLG_HXX

#include <tools/solar.h>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <sfx2/tabdlg.hxx>
#include <tools/fldunit.hxx>
#include <vcl/abstdlg.hxx>
#include "global.hxx"
#include "pivot.hxx"
#include <i18nlangtag/lang.h>
#include <asciiopt.hxx>

class ScAutoFormat;
class ScAutoFormatData;
class ScDocument;
struct ScImportSourceDesc;
class ScViewData;
class ScQueryItem;
class ScImportOptions;
class SfxStyleSheetBase;
class SfxAbstractTabDialog;
class ScDPObject;
struct ScDPNumGroupInfo;
class ScTabViewShell;
class ScConditionalFormat;
class ScConditionalFormatList;
class Date;
enum class CreateNameFlags;
enum class CellShiftDisabledFlags;

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
} } } }

class AbstractScImportAsciiDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScImportAsciiDlg() override = default;
public:
    virtual void                        GetOptions( ScAsciiOptions& rOpt ) = 0;
    virtual void                        SaveParameters() = 0;
};

class AbstractScAutoFormatDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScAutoFormatDlg() override = default;
public:
    virtual sal_uInt16 GetIndex() const = 0 ;
    virtual OUString GetCurrFormatName() = 0;
};

class AbstractScColRowLabelDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScColRowLabelDlg() override = default;
public:
    virtual bool IsCol() = 0;
    virtual bool IsRow() = 0;
};

class AbstractScCondFormatManagerDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScCondFormatManagerDlg() override = default;
public:
    virtual std::unique_ptr<ScConditionalFormatList> GetConditionalFormatList() = 0;

    virtual bool CondFormatsChanged() const = 0;

    virtual void SetModified() = 0;

    virtual ScConditionalFormat* GetCondFormatSelected() = 0;
};

class AbstractScDataPilotDatabaseDlg  :public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDataPilotDatabaseDlg() override = default;
public:
    virtual void    GetValues( ScImportSourceDesc& rDesc ) = 0;
};

class AbstractScDataPilotSourceTypeDlg  : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDataPilotSourceTypeDlg() override = default;
public:
    virtual bool IsDatabase() const = 0;
    virtual bool IsExternal() const = 0;
    virtual bool IsNamedRange() const = 0;
    virtual OUString GetSelectedNamedRange() const = 0;
    virtual void AppendNamedRange(const OUString& rName) = 0;
};

class AbstractScDataPilotServiceDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDataPilotServiceDlg() override = default;
public:
    virtual OUString  GetServiceName() const = 0;
    virtual OUString  GetParSource() const = 0 ;
    virtual OUString  GetParName() const = 0 ;
    virtual OUString  GetParUser() const = 0;
    virtual OUString  GetParPass() const = 0;
};

class AbstractScDeleteCellDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDeleteCellDlg() override = default;
public:
    virtual DelCellCmd GetDelCellCmd() const = 0;
};

//for dataform
class AbstractScDataFormDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDataFormDlg() override = default;
};

class AbstractScDeleteContentsDlg: public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDeleteContentsDlg() override = default;
public:
    virtual void    DisableObjects() = 0 ;
    virtual InsertDeleteFlags GetDelContentsCmdBits() const = 0;
};

class AbstractScFillSeriesDlg: public VclAbstractDialog
{
protected:
    virtual             ~AbstractScFillSeriesDlg() override = default;
public:
    virtual FillDir     GetFillDir() const = 0;
    virtual FillCmd     GetFillCmd() const = 0;
    virtual FillDateCmd GetFillDateCmd() const = 0;
    virtual double      GetStart() const = 0;
    virtual double      GetStep() const = 0;
    virtual double      GetMax() const = 0;
    virtual OUString    GetStartStr() const = 0;
    virtual void        SetEdStartValEnabled(bool bFlag) = 0;
};

class AbstractScGroupDlg :  public VclAbstractDialog
{
protected:
    virtual             ~AbstractScGroupDlg() override = default;
public:
    virtual bool GetColsChecked() const = 0;
};

class AbstractScInsertCellDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScInsertCellDlg() override = default;
public:
    virtual InsCellCmd GetInsCellCmd() const = 0;
};

class AbstractScInsertContentsDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScInsertContentsDlg() override = default;
public:
    virtual InsertDeleteFlags GetInsContentsCmdBits() const = 0;
    virtual ScPasteFunc   GetFormulaCmdBits() const = 0 ;
    virtual bool        IsSkipEmptyCells() const = 0;
    virtual bool        IsLink() const = 0;
    virtual void        SetFillMode( bool bSet ) = 0;
    virtual void        SetOtherDoc( bool bSet ) = 0;
    virtual bool        IsTranspose() const = 0;
    virtual void        SetChangeTrack( bool bSet ) = 0;
    virtual void        SetCellShiftDisabled( CellShiftDisabledFlags nDisable ) = 0;
    virtual InsCellCmd  GetMoveMode() = 0;
};

class AbstractScInsertTableDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScInsertTableDlg() override = default;
public:
    virtual bool            GetTablesFromFile() = 0;
    virtual bool            GetTablesAsLink()   = 0;
    virtual const OUString* GetFirstTable( sal_uInt16* pN = nullptr ) = 0;
    virtual ScDocShell*     GetDocShellTables() = 0;
    virtual bool            IsTableBefore() = 0;
    virtual sal_uInt16      GetTableCount() = 0;
    virtual const OUString* GetNextTable( sal_uInt16* pN ) = 0;

};

class AbstractScSelEntryDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScSelEntryDlg() override = default;
public:
    virtual OUString GetSelectedEntry() const = 0;
};

class AbstractScLinkedAreaDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScLinkedAreaDlg() override = default;
public:
    virtual void            InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh ) = 0;
    virtual OUString        GetURL() = 0;
    virtual OUString        GetFilter() = 0;        // may be empty
    virtual OUString        GetOptions() = 0;       // filter options
    virtual OUString        GetSource() = 0;        // separated by ";"
    virtual sal_uLong       GetRefresh() = 0;       // 0 if disabled
};

class AbstractScMetricInputDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractScMetricInputDlg() override = default;
public:
    virtual int GetInputValue() const = 0;
};

class AbstractScMoveTableDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScMoveTableDlg() override = default;
public:
    virtual sal_uInt16  GetSelectedDocument     () const = 0;
    virtual sal_uInt16  GetSelectedTable        () const = 0;
    virtual bool    GetCopyTable            () const = 0;
    virtual bool    GetRenameTable          () const = 0;
    virtual void    GetTabNameString( OUString& rString ) const = 0;
    virtual void    SetForceCopyTable       () = 0;
    virtual void    EnableRenameTable       (bool bFlag) = 0;
};

class AbstractScNameCreateDlg : public VclAbstractDialog  // Manage Name Dialog
{
protected:
    virtual             ~AbstractScNameCreateDlg() override = default;
public:
    virtual CreateNameFlags  GetFlags() const = 0;
};

class AbstractScNamePasteDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScNamePasteDlg() override = default;
public:
    virtual std::vector<OUString> GetSelectedNames() const = 0;
};

class AbstractScPivotFilterDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScPivotFilterDlg() override = default;
public:
    virtual const ScQueryItem&  GetOutputItem() = 0;
};

class AbstractScDPFunctionDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDPFunctionDlg() override = default;
public:
    virtual PivotFunc  GetFuncMask() const = 0;
    virtual css::sheet::DataPilotFieldReference GetFieldRef() const = 0;
};

class AbstractScDPSubtotalDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDPSubtotalDlg() override = default;
public:
    virtual PivotFunc  GetFuncMask() const = 0;
    virtual void    FillLabelData( ScDPLabelData& rLabelData ) const = 0;
};

class AbstractScDPNumGroupDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDPNumGroupDlg() override = default;
public:
    virtual ScDPNumGroupInfo GetGroupInfo() const = 0;
};

class AbstractScDPDateGroupDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDPDateGroupDlg() override = default;
public:
    virtual ScDPNumGroupInfo GetGroupInfo() const = 0;
    virtual sal_Int32 GetDatePart() const = 0;
};

class AbstractScDPShowDetailDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScDPShowDetailDlg() override = default;
public:
    virtual OUString  GetDimensionName() const = 0;
};

class AbstractScNewScenarioDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScNewScenarioDlg() override = default;
public:

    virtual void SetScenarioData( const OUString& rName, const OUString& rComment,
                            const Color& rColor, ScScenarioFlags nFlags ) = 0;

    virtual void GetScenarioData( OUString& rName, OUString& rComment,
                            Color& rColor, ScScenarioFlags& rFlags ) const = 0;
};

class AbstractScShowTabDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScShowTabDlg() override = default;
public:
    virtual void Insert( const OUString& rString, bool bSelected ) = 0;
    virtual void SetDescription(const OUString& rTitle, const OUString& rFixedText, const OString& nDlgHelpId, const OString& nLbHelpId ) = 0;
    virtual std::vector<sal_Int32> GetSelectedRows() const = 0;
    virtual OUString GetEntry(sal_Int32 nPos) const = 0;
};

class AbstractScSortWarningDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScSortWarningDlg() override = default;
};

class AbstractScStringInputDlg :  public VclAbstractDialog
{
protected:
    virtual             ~AbstractScStringInputDlg() override = default;
public:
    virtual OUString GetInputString() const = 0;
};

class AbstractScTabBgColorDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScTabBgColorDlg() override = default;
public:
    virtual void GetSelectedColor( Color& rColor ) const = 0;
};

class AbstractScImportOptionsDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScImportOptionsDlg() override = default;
public:
    virtual void GetImportOptions( ScImportOptions& rOptions ) const = 0;
    virtual void SaveImportOptions() const = 0;
};

class AbstractScTextImportOptionsDlg : public VclAbstractDialog
{
protected:
    virtual             ~AbstractScTextImportOptionsDlg() override = default;
public:
    virtual LanguageType GetLanguageType() const = 0;
    virtual bool IsDateConversionSet() const = 0;
};

class ScAbstractDialogFactory
{
public:
    SC_DLLPUBLIC static ScAbstractDialogFactory*    Create();

    virtual     VclPtr<AbstractScImportAsciiDlg> CreateScImportAsciiDlg(vcl::Window* pParent,
                                                                    const OUString& aDatName,
                                                                    SvStream* pInStream,
                                                                    ScImportAsciiCall eCall) = 0;

    virtual     VclPtr<AbstractScTextImportOptionsDlg> CreateScTextImportOptionsDlg(weld::Window* pParent) = 0;

    virtual     VclPtr<AbstractScAutoFormatDlg> CreateScAutoFormatDlg(weld::Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData *pViewData) = 0;
    virtual VclPtr<AbstractScColRowLabelDlg> CreateScColRowLabelDlg (weld::Window* pParent,
                                                                bool bCol,
                                                                bool bRow) = 0;

    virtual VclPtr<AbstractScSortWarningDlg> CreateScSortWarningDlg(weld::Window* pParent, const OUString& rExtendText, const OUString& rCurrentText ) = 0;

    virtual VclPtr<AbstractScCondFormatManagerDlg> CreateScCondFormatMgrDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList ) = 0;

    virtual VclPtr<AbstractScDataPilotDatabaseDlg> CreateScDataPilotDatabaseDlg(weld::Window* pParent) = 0;

    virtual VclPtr<AbstractScDataPilotSourceTypeDlg> CreateScDataPilotSourceTypeDlg(weld::Window* pParent,
        bool bEnableExternal) = 0;

    virtual VclPtr<AbstractScDataPilotServiceDlg> CreateScDataPilotServiceDlg(weld::Window* pParent,
                                                                              const std::vector<OUString>& rServices) = 0;

    virtual VclPtr<AbstractScDeleteCellDlg> CreateScDeleteCellDlg(weld::Window* pParent, bool bDisallowCellMove) = 0 ;

    //for dataform
    virtual VclPtr<AbstractScDataFormDlg> CreateScDataFormDlg(vcl::Window* pParent,
        ScTabViewShell* pTabViewShell) = 0;

    virtual VclPtr<AbstractScDeleteContentsDlg> CreateScDeleteContentsDlg(weld::Window* pParent) = 0;
    virtual VclPtr<AbstractScFillSeriesDlg> CreateScFillSeriesDlg(weld::Window*        pParent,
                                                            ScDocument& rDocument,
                                                            FillDir     eFillDir,
                                                            FillCmd     eFillCmd,
                                                            FillDateCmd eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16          nPossDir) = 0;

    virtual VclPtr<AbstractScGroupDlg> CreateAbstractScGroupDlg(weld::Window* pParent, bool bUnGroup = false) = 0;

    virtual VclPtr<AbstractScInsertCellDlg> CreateScInsertCellDlg(weld::Window* pParent,
                                                             bool bDisallowCellMove) = 0;

    virtual VclPtr<AbstractScInsertContentsDlg> CreateScInsertContentsDlg(weld::Window* pParent,
                                                                          const OUString* pStrTitle = nullptr) = 0;

    virtual VclPtr<AbstractScInsertTableDlg> CreateScInsertTableDlg(weld::Window* pParent, ScViewData& rViewData,
        SCTAB nTabCount, bool bFromFile) = 0;

    virtual VclPtr<AbstractScSelEntryDlg > CreateScSelEntryDlg(weld::Window* pParent, const std::vector<OUString> &rEntryList) = 0;
    virtual VclPtr<AbstractScLinkedAreaDlg> CreateScLinkedAreaDlg(weld::Window* pParent) = 0;

    virtual VclPtr<AbstractScMetricInputDlg> CreateScMetricInputDlg(weld::Window* pParent,
                                                                const OString&  sDialogName,
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                FieldUnit       eFUnit,
                                                                sal_uInt16      nDecimals,
                                                                long            nMaximum,
                                                                long            nMinimum  = 0 ) = 0;

    virtual VclPtr<AbstractScMoveTableDlg> CreateScMoveTableDlg(weld::Window * pParent,
        const OUString& rDefault) = 0;

    virtual VclPtr<AbstractScNameCreateDlg> CreateScNameCreateDlg(weld::Window * pParent,
        CreateNameFlags nFlags) = 0;

    virtual VclPtr<AbstractScNamePasteDlg> CreateScNamePasteDlg(weld::Window * pParent, ScDocShell* pShell) = 0;

    virtual VclPtr<AbstractScPivotFilterDlg> CreateScPivotFilterDlg(vcl::Window* pParent,
        const SfxItemSet& rArgSet, sal_uInt16 nSourceTab) = 0;

    virtual VclPtr<AbstractScDPFunctionDlg> CreateScDPFunctionDlg(weld::Window* pParent,
                                                                  const ScDPLabelDataVector& rLabelVec,
                                                                  const ScDPLabelData& rLabelData,
                                                                  const ScPivotFuncData& rFuncData ) = 0;

    virtual VclPtr<AbstractScDPSubtotalDlg> CreateScDPSubtotalDlg(weld::Window* pParent,
                                                                  ScDPObject& rDPObj,
                                                                  const ScDPLabelData& rLabelData,
                                                                  const ScPivotFuncData& rFuncData,
                                                                  const ScDPNameVec& rDataFields) = 0;

    virtual VclPtr<AbstractScDPNumGroupDlg> CreateScDPNumGroupDlg(weld::Window* pParent,
                                                                  const ScDPNumGroupInfo& rInfo) = 0;

    virtual VclPtr<AbstractScDPDateGroupDlg> CreateScDPDateGroupDlg(weld::Window* pParent,
                                                                    const ScDPNumGroupInfo& rInfo,
                                                                    sal_Int32 nDatePart,
                                                                    const Date& rNullDate ) = 0;

    virtual VclPtr<AbstractScDPShowDetailDlg> CreateScDPShowDetailDlg(weld::Window* pParent,
                                                                ScDPObject& rDPObj,
                                                                css::sheet::DataPilotFieldOrientation nOrient ) = 0;

    virtual VclPtr<AbstractScNewScenarioDlg> CreateScNewScenarioDlg(weld::Window* pParent, const OUString& rName,
                                                                    bool bEdit, bool bSheetProtected) = 0;
    virtual VclPtr<AbstractScShowTabDlg> CreateScShowTabDlg(weld::Window* pParent) = 0;

    virtual VclPtr<AbstractScStringInputDlg> CreateScStringInputDlg(weld::Window* pParent,
                                                                    const OUString& rTitle,
                                                                    const OUString& rEditTitle,
                                                                    const OUString& rDefault,
                                                                    const OString& rHelpId,
                                                                    const OString& rEditHelpId) = 0;

    virtual VclPtr<AbstractScTabBgColorDlg> CreateScTabBgColorDlg(weld::Window* pParent,
                                                                  const OUString& rTitle, //Dialog Title
                                                                  const OUString& rTabBgColorNoColorText, //Label for no tab color
                                                                  const Color& rDefaultColor) = 0; //Currently selected Color

    virtual VclPtr<AbstractScImportOptionsDlg> CreateScImportOptionsDlg(weld::Window* pParent, bool bAscii,
                                                                        const ScImportOptions*  pOptions,
                                                                        const OUString*         pStrTitle,
                                                                        bool                    bOnlyDbtoolsEncodings,
                                                                        bool                    bImport = true ) = 0;

    virtual VclPtr<SfxAbstractTabDialog> CreateScAttrDlg(weld::Window* pParent,
                                                    const SfxItemSet* pCellAttrs) = 0;

    virtual VclPtr<SfxAbstractTabDialog> CreateScHFEditDlg(vcl::Window*       pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const OUString&     rPageStyle,
                                                    sal_uInt16          nResId ) = 0;

    virtual VclPtr<SfxAbstractTabDialog> CreateScStyleDlg(weld::Window* pParent,
                                                          SfxStyleSheetBase&  rStyleBase,
                                                          bool bPage /*true : page, false: para*/) = 0;

    virtual VclPtr<SfxAbstractTabDialog> CreateScSubTotalDlg(weld::Window* pParent,
                                                             const SfxItemSet* pArgSet) = 0;

    virtual VclPtr<SfxAbstractTabDialog> CreateScCharDlg(weld::Window* pParent,
        const SfxItemSet* pAttr, const SfxObjectShell* pDocShell) = 0;

    virtual VclPtr<SfxAbstractTabDialog> CreateScParagraphDlg(weld::Window* pParent,
        const SfxItemSet* pAttr) = 0;

    virtual VclPtr<SfxAbstractTabDialog> CreateScSortDlg(weld::Window* pParent, const SfxItemSet* pArgSet) = 0;

    // for tabpage
    virtual CreateTabPage                GetTabPageCreatorFunc( sal_uInt16 nId ) = 0;

protected:
    ~ScAbstractDialogFactory() {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
