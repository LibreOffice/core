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
#include <sfx2/sfxdlg.hxx>
#include <vcl/syswin.hxx>
#include <vcl/field.hxx>
#include "sc.hrc"
#include "global.hxx"
#include "pivot.hxx"
#include <i18nlangtag/lang.h>
#include "asciiopt.hxx"

#include <tabvwsh.hxx>

class ScAsciiOptions;
class ScAutoFormat;
class ScAutoFormatData;
class ScDocument;
struct ScImportSourceDesc;
class ScViewData;
class ScQueryItem;
class ScImportOptions;
class SfxStyleSheetBase;
class ScDPObject;
struct ScPivotFuncData;
struct ScDPNumGroupInfo;
class ScSortWarningDlg;
class ScTabViewShell;
class ScConditionalFormat;
class ScConditionalFormatList;

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
} } } }

class AbstractScImportAsciiDlg : public VclAbstractDialog
{
public:
    virtual void                        GetOptions( ScAsciiOptions& rOpt ) = 0;
    virtual void                        SaveParameters() = 0;
};

class AbstractScAutoFormatDlg : public VclAbstractDialog
{
public:
    virtual sal_uInt16 GetIndex() const = 0 ;
    virtual OUString GetCurrFormatName() = 0;
};

class AbstractScColRowLabelDlg : public VclAbstractDialog
{
public:
    virtual bool IsCol() = 0;
    virtual bool IsRow() = 0;
};

class AbstractScCondFormatManagerDlg : public VclAbstractDialog
{
public:
    virtual ScConditionalFormatList* GetConditionalFormatList() = 0;

    virtual bool CondFormatsChanged() = 0;

    virtual ScConditionalFormat* GetCondFormatSelected() = 0;
};

class AbstractScDataPilotDatabaseDlg  :public VclAbstractDialog
{
public:
    virtual void    GetValues( ScImportSourceDesc& rDesc ) = 0;
};

class AbstractScDataPilotSourceTypeDlg  : public VclAbstractDialog
{
public:
    virtual bool IsDatabase() const = 0;
    virtual bool IsExternal() const = 0;
    virtual bool IsNamedRange() const = 0;
    virtual OUString GetSelectedNamedRange() const = 0;
    virtual void AppendNamedRange(const OUString& rName) = 0;
};

class AbstractScDataPilotServiceDlg : public VclAbstractDialog
{
public:
    virtual OUString  GetServiceName() const = 0;
    virtual OUString  GetParSource() const = 0 ;
    virtual OUString  GetParName() const = 0 ;
    virtual OUString  GetParUser() const = 0;
    virtual OUString  GetParPass() const = 0;
};

class AbstractScDeleteCellDlg : public VclAbstractDialog
{
public:
    virtual DelCellCmd GetDelCellCmd() const = 0;
};

//for dataform
class AbstractScDataFormDlg : public VclAbstractDialog
{

};

class AbstractScDeleteContentsDlg: public VclAbstractDialog
{
public:
    virtual void    DisableObjects() = 0 ;
    virtual InsertDeleteFlags GetDelContentsCmdBits() const = 0;
};

class AbstractScFillSeriesDlg: public VclAbstractDialog
{
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
public:
    virtual bool GetColsChecked() const = 0;
};

class AbstractScInsertCellDlg : public VclAbstractDialog
{
public:
    virtual InsCellCmd GetInsCellCmd() const = 0;
};

class AbstractScInsertContentsDlg : public VclAbstractDialog
{
public:
    virtual InsertDeleteFlags GetInsContentsCmdBits() const = 0;
    virtual ScPasteFunc   GetFormulaCmdBits() const = 0 ;
    virtual bool        IsSkipEmptyCells() const = 0;
    virtual bool        IsLink() const = 0;
    virtual void        SetFillMode( bool bSet ) = 0;
    virtual void        SetOtherDoc( bool bSet ) = 0;
    virtual bool        IsTranspose() const = 0;
    virtual void        SetChangeTrack( bool bSet ) = 0;
    virtual void        SetCellShiftDisabled( int nDisable ) = 0;
    virtual InsCellCmd  GetMoveMode() = 0;
};

class AbstractScInsertTableDlg : public VclAbstractDialog
{
public:
    virtual bool            GetTablesFromFile() = 0;
    virtual bool            GetTablesAsLink()   = 0;
    virtual const OUString* GetFirstTable( sal_uInt16* pN = nullptr ) = 0;
    virtual ScDocShell*     GetDocShellTables() = 0;
    virtual bool            IsTableBefore() = 0;
    virtual sal_uInt16      GetTableCount() = 0;
    virtual const OUString* GetNextTable( sal_uInt16* pN = nullptr ) = 0;

};

class AbstractScSelEntryDlg : public VclAbstractDialog
{
public:
    virtual OUString GetSelectEntry() const = 0;
};

class AbstractScLinkedAreaDlg : public VclAbstractDialog2
{
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
public:
    virtual long GetInputValue() const = 0;
};

class AbstractScMoveTableDlg : public VclAbstractDialog
{
public:
    virtual sal_uInt16  GetSelectedDocument     () const = 0;
    virtual sal_uInt16  GetSelectedTable        () const = 0;
    virtual bool    GetCopyTable            () const = 0;
    virtual bool    GetRenameTable          () const = 0;
    virtual void    GetTabNameString( OUString& rString ) const = 0;
    virtual void    SetForceCopyTable       () = 0;
    virtual void    EnableRenameTable       (bool bFlag=true) = 0;
};

class AbstractScNameCreateDlg : public VclAbstractDialog  // Manage Name Dialog
{
public:
    virtual sal_uInt16          GetFlags() const = 0;
};

class AbstractScNamePasteDlg : public VclAbstractDialog
{
public:
    virtual std::vector<OUString> GetSelectedNames() const = 0;
};

class AbstractScPivotFilterDlg : public VclAbstractDialog
{
public:
    virtual const ScQueryItem&  GetOutputItem() = 0;
};

class AbstractScDPFunctionDlg : public VclAbstractDialog
{
public:
    virtual PivotFunc  GetFuncMask() const = 0;
    virtual css::sheet::DataPilotFieldReference GetFieldRef() const = 0;
};

class AbstractScDPSubtotalDlg : public VclAbstractDialog
{
public:
    virtual PivotFunc  GetFuncMask() const = 0;
    virtual void    FillLabelData( ScDPLabelData& rLabelData ) const = 0;
};

class AbstractScDPNumGroupDlg : public VclAbstractDialog
{
public:
    virtual ScDPNumGroupInfo GetGroupInfo() const = 0;
};

class AbstractScDPDateGroupDlg : public VclAbstractDialog
{
public:
    virtual ScDPNumGroupInfo GetGroupInfo() const = 0;
    virtual sal_Int32 GetDatePart() const = 0;
};

class AbstractScDPShowDetailDlg : public VclAbstractDialog
{
public:
    virtual OUString  GetDimensionName() const = 0;
};

class AbstractScNewScenarioDlg : public VclAbstractDialog
{
public:

    virtual void SetScenarioData( const OUString& rName, const OUString& rComment,
                            const Color& rColor, ScScenarioFlags nFlags ) = 0;

    virtual void GetScenarioData( OUString& rName, OUString& rComment,
                            Color& rColor, ScScenarioFlags& rFlags ) const = 0;
};

class AbstractScShowTabDlg : public VclAbstractDialog
{
public:
    virtual void    Insert( const OUString& rString, bool bSelected ) = 0;
    virtual sal_Int32 GetSelectEntryCount() const = 0;
    virtual void SetDescription(const OUString& rTitle, const OUString& rFixedText, const OString& nDlgHelpId, const OString& nLbHelpId ) = 0;
    virtual OUString  GetSelectEntry(sal_Int32 nPos) const = 0;
    virtual sal_Int32 GetSelectEntryPos(sal_Int32 nPos) const = 0;
};

class AbstractScSortWarningDlg : public VclAbstractDialog
{
};

class AbstractScStringInputDlg :  public VclAbstractDialog
{
public:
    virtual OUString GetInputString() const = 0;
};

class AbstractScTabBgColorDlg : public VclAbstractDialog
{
public:
    virtual void GetSelectedColor( Color& rColor ) const = 0;
};

class AbstractScImportOptionsDlg : public VclAbstractDialog
{
public:
    virtual void GetImportOptions( ScImportOptions& rOptions ) const = 0;
};

class AbstractScTextImportOptionsDlg : public VclAbstractDialog
{
public:
    virtual LanguageType GetLanguageType() const = 0;
    virtual bool IsDateConversionSet() const = 0;
};

class ScAbstractDialogFactory
{
public:
    SC_DLLPUBLIC static ScAbstractDialogFactory*    Create();

    virtual     AbstractScImportAsciiDlg * CreateScImportAsciiDlg( const OUString& aDatName,
                                                                    SvStream* pInStream,
                                                                    ScImportAsciiCall eCall) = 0;

    virtual     AbstractScTextImportOptionsDlg * CreateScTextImportOptionsDlg() = 0;

    virtual     AbstractScAutoFormatDlg * CreateScAutoFormatDlg(vcl::Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData *pViewData) = 0;
    virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (vcl::Window* pParent,
                                                                bool bCol = false,
                                                                bool bRow = false) = 0;

    virtual AbstractScSortWarningDlg * CreateScSortWarningDlg(vcl::Window* pParent, const OUString& rExtendText, const OUString& rCurrentText ) = 0;

    virtual AbstractScCondFormatManagerDlg* CreateScCondFormatMgrDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList,
                                                                int nId ) = 0;

    virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg(vcl::Window* pParent) = 0;

    virtual AbstractScDataPilotSourceTypeDlg * CreateScDataPilotSourceTypeDlg(vcl::Window* pParent,
        bool bEnableExternal) = 0;

    virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( vcl::Window* pParent,
                                                                        const std::vector<OUString>& rServices,
                                                                        int nId ) = 0;

    virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg(vcl::Window* pParent, bool bDisallowCellMove = false) = 0 ;

    //for dataform
    virtual AbstractScDataFormDlg * CreateScDataFormDlg(vcl::Window* pParent,
        ScTabViewShell* pTabViewShell) = 0;

    virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(vcl::Window* pParent) = 0;
    virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( vcl::Window*        pParent,
                                                            ScDocument& rDocument,
                                                            FillDir     eFillDir,
                                                            FillCmd     eFillCmd,
                                                            FillDateCmd eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16          nPossDir) = 0;

    virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( vcl::Window* pParent,
                                                            bool bUnGroup = false ) = 0;

    virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( vcl::Window* pParent,
                                                             int nId,
                                                             bool bDisallowCellMove = false ) = 0;

    virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( vcl::Window*        pParent,
                                                                    const OUString* pStrTitle = nullptr ) = 0;

    virtual AbstractScInsertTableDlg * CreateScInsertTableDlg(vcl::Window* pParent, ScViewData& rViewData,
        SCTAB nTabCount, bool bFromFile) = 0;

    virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( vcl::Window* pParent,
                                                          const std::vector<OUString> &rEntryList ) = 0;
    virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg(vcl::Window* pParent) = 0;

    virtual AbstractScMetricInputDlg * CreateScMetricInputDlg ( vcl::Window*        pParent,
                                                                const OString&  sDialogName,
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                FieldUnit       eFUnit    = FUNIT_MM,
                                                                sal_uInt16      nDecimals = 2,
                                                                long            nMaximum  = 1000,
                                                                long            nMinimum  = 0,
                                                                long            nFirst    = 1,
                                                                long            nLast     = 100 ) = 0;

    virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(vcl::Window * pParent,
        const OUString& rDefault) = 0;

    virtual AbstractScNameCreateDlg * CreateScNameCreateDlg(vcl::Window * pParent,
        sal_uInt16 nFlags) = 0;

    virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( vcl::Window * pParent, ScDocShell* pShell, bool bInsList=true ) = 0;

    virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg(vcl::Window* pParent,
        const SfxItemSet& rArgSet, sal_uInt16 nSourceTab) = 0;

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( vcl::Window* pParent,
                                                                const ScDPLabelDataVector& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData ) = 0;

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( vcl::Window* pParent,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields ) = 0;

    virtual AbstractScDPNumGroupDlg * CreateScDPNumGroupDlg( vcl::Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo ) = 0;

    virtual AbstractScDPDateGroupDlg * CreateScDPDateGroupDlg( vcl::Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo,
                                                                sal_Int32 nDatePart,
                                                                const Date& rNullDate ) = 0;

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( vcl::Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                sal_uInt16 nOrient ) = 0;

    virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( vcl::Window* pParent, const OUString& rName,
                                                                bool bEdit = false, bool bSheetProtected = false ) = 0;
    virtual AbstractScShowTabDlg * CreateScShowTabDlg(vcl::Window* pParent) = 0;

    virtual AbstractScStringInputDlg * CreateScStringInputDlg (  vcl::Window* pParent,
                                                                const OUString& rTitle,
                                                                const OUString& rEditTitle,
                                                                const OUString& rDefault,
                                                                const OString& sHelpId, const OString& sEditHelpId ) = 0;

    virtual AbstractScTabBgColorDlg * CreateScTabBgColorDlg (  vcl::Window* pParent,
                                                                const OUString& rTitle, //Dialog Title
                                                                const OUString& rTabBgColorNoColorText, //Label for no tab color
                                                                const Color& rDefaultColor, //Currently selected Color
                                                                const OString& ) = 0;

    virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( bool                    bAscii = true,
                                                                    const ScImportOptions*  pOptions = nullptr,
                                                                    const OUString*         pStrTitle = nullptr,
                                                                    bool                    bMultiByte = false,
                                                                    bool                    bOnlyDbtoolsEncodings = false,
                                                                    bool                    bImport = true ) = 0;

    virtual SfxAbstractTabDialog * CreateScAttrDlg( vcl::Window*          pParent,
                                                    const SfxItemSet* pCellAttrs ) = 0;

    virtual SfxAbstractTabDialog * CreateScHFEditDlg(vcl::Window*         pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const OUString&       rPageStyle,
                                                    sal_uInt16              nResId = RID_SCDLG_HFEDIT ) = 0;

    virtual SfxAbstractTabDialog * CreateScStyleDlg( vcl::Window*                pParent,
                                                    SfxStyleSheetBase&  rStyleBase,
                                                    sal_uInt16              nRscId,
                                                    int nId) = 0;

    virtual SfxAbstractTabDialog * CreateScSubTotalDlg( vcl::Window*             pParent,
                                                        const SfxItemSet*   pArgSet ) = 0;

    virtual SfxAbstractTabDialog * CreateScCharDlg(vcl::Window* pParent,
        const SfxItemSet* pAttr, const SfxObjectShell* pDocShell) = 0;

    virtual SfxAbstractTabDialog * CreateScParagraphDlg(vcl::Window* pParent,
        const SfxItemSet* pAttr) = 0;

    virtual SfxAbstractTabDialog * CreateScSortDlg(vcl::Window* pParent, const SfxItemSet* pArgSet) = 0;

    // for tabpage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) = 0;

protected:
    ~ScAbstractDialogFactory() {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
