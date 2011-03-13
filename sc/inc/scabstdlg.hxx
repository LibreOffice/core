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
#ifndef _SC_ABSTDLG_HXX
#define _SC_ABSTDLG_HXX

// include ---------------------------------------------------------------

#include <tools/solar.h>
#include <tools/string.hxx>
#include <sfx2/sfxdlg.hxx>
#include <vcl/syswin.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/field.hxx>
#include "sc.hrc"
#include "global.hxx"
#include "pivot.hxx"
#include "i18npool/lang.h"

#include <tabvwsh.hxx>

class ScAsciiOptions;
class ScAutoFormat;
class ScAutoFormatData;
class ScDocument;
struct ScImportSourceDesc;
class ScViewData;
class ScRangeName;
class ScQueryItem;
class ScImportOptions;
class SfxStyleSheetBase;
class ScDPObject;
struct ScDPFuncData;
struct ScDPNumGroupInfo;
class ScTabViewShell;

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
} } } }

class AbstractScImportAsciiDlg : public VclAbstractDialog  //add for ScImportAsciiDlg
{
public:
    virtual void                        GetOptions( ScAsciiOptions& rOpt ) = 0;
    virtual void                        SetTextToColumnsMode() = 0;
    virtual void                        SaveParameters() = 0;
};


class AbstractScAutoFormatDlg : public VclAbstractDialog  //add for ScAutoFormatDlg
{
public:
    virtual USHORT GetIndex() const = 0 ;
    virtual String GetCurrFormatName() = 0;
};

class AbstractScColRowLabelDlg : public VclAbstractDialog  //add for ScColRowLabelDlg
{
public:
    virtual BOOL IsCol() = 0;
    virtual BOOL IsRow() = 0;
};

class AbstractScDataPilotDatabaseDlg  :public VclAbstractDialog  //add for ScDataPilotDatabaseDlg
{
public:
    virtual void    GetValues( ScImportSourceDesc& rDesc ) = 0;
};

class AbstractScDataPilotSourceTypeDlg  : public VclAbstractDialog  //add for ScDataPilotSourceTypeDlg
{
public:
    virtual bool IsDatabase() const = 0;
    virtual bool IsExternal() const = 0;
    virtual bool IsNamedRange() const = 0;
    virtual rtl::OUString GetSelectedNamedRange() const = 0;
    virtual void AppendNamedRange(const ::rtl::OUString& rName) = 0;
};

class AbstractScDataPilotServiceDlg : public VclAbstractDialog  //add for ScDataPilotServiceDlg
{
public:
    virtual String  GetServiceName() const = 0;
    virtual String  GetParSource() const = 0 ;
    virtual String  GetParName() const = 0 ;
    virtual String  GetParUser() const = 0;
    virtual String  GetParPass() const = 0;
};

class AbstractScDeleteCellDlg : public VclAbstractDialog  //add for ScDeleteCellDlg
{
public:
    virtual DelCellCmd GetDelCellCmd() const = 0;
};

//for dataform
class AbstractScDataFormDlg : public VclAbstractDialog  //add for ScDeleteCellDlg
{

};

class AbstractScDeleteContentsDlg: public VclAbstractDialog  //add for ScDeleteContentsDlg
{
public:
    virtual void    DisableObjects() = 0 ;
    virtual USHORT  GetDelContentsCmdBits() const = 0;
};

class AbstractScFillSeriesDlg: public VclAbstractDialog  //add for ScFillSeriesDlg
{
public:
    virtual FillDir     GetFillDir() const = 0;
    virtual FillCmd     GetFillCmd() const = 0;
    virtual FillDateCmd GetFillDateCmd() const = 0;
    virtual double      GetStart() const = 0;
    virtual double      GetStep() const = 0;
    virtual double      GetMax() const = 0;
    virtual String      GetStartStr() const = 0;
    virtual void        SetEdStartValEnabled(BOOL bFlag=FALSE) = 0;
};

class AbstractScGroupDlg :  public VclAbstractDialog  //add for ScGroupDlg
{
public:
    virtual BOOL GetColsChecked() const = 0;
};

class AbstractScInsertCellDlg : public VclAbstractDialog  //add for ScInsertCellDlg
{
public:
    virtual InsCellCmd GetInsCellCmd() const = 0;
};

class AbstractScInsertContentsDlg : public VclAbstractDialog  //add for ScInsertContentsDlg
{
public:
    virtual USHORT      GetInsContentsCmdBits() const = 0;
    virtual USHORT      GetFormulaCmdBits() const = 0 ;
    virtual BOOL        IsSkipEmptyCells() const = 0;
    virtual BOOL        IsLink() const = 0;
    virtual void    SetFillMode( BOOL bSet ) = 0;
    virtual void    SetOtherDoc( BOOL bSet ) = 0;
    virtual BOOL        IsTranspose() const = 0;
    virtual void    SetChangeTrack( BOOL bSet ) = 0;
    virtual void    SetCellShiftDisabled( int nDisable ) = 0;
    virtual InsCellCmd  GetMoveMode() = 0;
};

class AbstractScInsertTableDlg : public VclAbstractDialog  //add for ScInsertTableDlg
{
public:
    virtual BOOL            GetTablesFromFile() = 0;
    virtual BOOL            GetTablesAsLink()   = 0;
    virtual const String*   GetFirstTable( USHORT* pN = NULL ) = 0;
    virtual ScDocShell*     GetDocShellTables() = 0;
    virtual BOOL            IsTableBefore() = 0;
    virtual USHORT          GetTableCount() = 0;
    virtual const String*   GetNextTable( USHORT* pN = NULL ) = 0;

};

class AbstractScSelEntryDlg : public VclAbstractDialog  //add for ScSelEntryDlg
{
public:
    virtual String GetSelectEntry() const = 0;
};

class AbstractScLinkedAreaDlg : public VclAbstractDialog2  //add for ScLinkedAreaDlg
{
public:
    virtual void            InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        ULONG nRefresh ) = 0;
    virtual String          GetURL() = 0;
    virtual String          GetFilter() = 0;        // may be empty
    virtual String          GetOptions() = 0;       // filter options
    virtual String          GetSource() = 0;        // separated by ";"
    virtual ULONG           GetRefresh() = 0;       // 0 if disabled
};

class AbstractScMetricInputDlg : public VclAbstractDialog  //add for ScMetricInputDlg
{
public:
    virtual long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const = 0;
};

class AbstractScMoveTableDlg : public VclAbstractDialog  //add for ScMoveTableDlg
{
public:
    virtual USHORT  GetSelectedDocument     () const = 0;
    virtual USHORT  GetSelectedTable        () const = 0;
    virtual bool    GetCopyTable            () const = 0;
    virtual bool    GetRenameTable          () const = 0;
    virtual void    GetTabNameString( String& rString ) const = 0;
    virtual void    SetForceCopyTable       () = 0;
    virtual void    EnableCopyTable         (BOOL bFlag=TRUE) = 0;
    virtual void    EnableRenameTable       (BOOL bFlag=TRUE) = 0;
};

class AbstractScNameCreateDlg : public VclAbstractDialog  //add for ScNameCreateDlg
{
public:
    virtual USHORT          GetFlags() const = 0;
};

class AbstractScNamePasteDlg : public VclAbstractDialog  //add for ScNamePasteDlg
{
public:
    virtual String          GetSelectedName() const = 0;
};

class AbstractScPivotFilterDlg : public VclAbstractDialog  //add for ScPivotFilterDlg
{
public:
    virtual const ScQueryItem&  GetOutputItem() = 0;
};

class AbstractScDPFunctionDlg : public VclAbstractDialog  //add for ScDPFunctionDlg
{
public:
    virtual USHORT  GetFuncMask() const = 0;
    virtual ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const = 0;
};

class AbstractScDPSubtotalDlg : public VclAbstractDialog  //add for ScDPSubtotalDlg
{
public:
    virtual USHORT  GetFuncMask() const = 0;
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

class AbstractScDPShowDetailDlg : public VclAbstractDialog  //add for ScDPShowDetailDlg
{
public:
    virtual String  GetDimensionName() const = 0;
};

class AbstractScNewScenarioDlg : public VclAbstractDialog  //add for ScNewScenarioDlg
{
public:

    virtual void SetScenarioData( const String& rName, const String& rComment,
                            const Color& rColor, USHORT nFlags ) = 0;

    virtual void GetScenarioData( String& rName, String& rComment,
                            Color& rColor, USHORT& rFlags ) const = 0;
};

class AbstractScShowTabDlg : public VclAbstractDialog  //add for ScShowTabDlg
{
public:
    virtual void    Insert( const String& rString, BOOL bSelected ) = 0;
    virtual USHORT  GetSelectEntryCount() const = 0;
    virtual void SetDescription(const String& rTitle, const String& rFixedText,ULONG nDlgHelpId, ULONG nLbHelpId ) = 0;
    virtual String  GetSelectEntry(USHORT nPos) const = 0;
    virtual USHORT  GetSelectEntryPos(USHORT nPos) const = 0;
};

class AbstractScStringInputDlg :  public VclAbstractDialog  //add for ScStringInputDlg
{
public:
    virtual void GetInputString( String& rString ) const = 0;
};

class AbstractScTabBgColorDlg : public VclAbstractDialog  //add for ScTabBgColorDlg
{
public:
    virtual void GetSelectedColor( Color& rColor ) const = 0;
};

class AbstractScImportOptionsDlg : public VclAbstractDialog  //add for ScImportOptionsDlg
{
public:
    virtual void GetImportOptions( ScImportOptions& rOptions ) const = 0;
};

class AbstractScTextImportOptionsDlg : public VclAbstractDialog //add for ScLangChooserDlg
{
public:
    virtual LanguageType GetLanguageType() const = 0;
    virtual bool IsDateConversionSet() const = 0;
};

//-------Scabstract fractory ---------------------------
class ScAbstractDialogFactory
{
public:
    SC_DLLPUBLIC static ScAbstractDialogFactory*    Create();

    virtual     AbstractScImportAsciiDlg * CreateScImportAsciiDlg( Window* pParent, String aDatName, //add for ScImportAsciiDlg
                                                                    SvStream* pInStream, int nId,
                                                                    sal_Unicode cSep = '\t') = 0;

    virtual     AbstractScTextImportOptionsDlg * CreateScTextImportOptionsDlg( Window* pParent, int nId ) = 0;

    virtual     AbstractScAutoFormatDlg * CreateScAutoFormatDlg( Window*                    pParent, //add for ScAutoFormatDlg
                                                                ScAutoFormat*               pAutoFormat,
                                                                const ScAutoFormatData*    pSelFormatData,
                                                                ScDocument*                pDoc,
                                                                int nId) = 0;
    virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (Window* pParent, //add for ScColRowLabelDlg
                                                                int nId,
                                                                BOOL bCol = FALSE,
                                                                BOOL bRow = FALSE) = 0;

    virtual VclAbstractDialog * CreateScColOrRowDlg( Window*            pParent, //add for ScColOrRowDlg
                                                    const String&   rStrTitle,
                                                    const String&   rStrLabel,
                                                    int nId,
                                                    BOOL                bColDefault = TRUE ) = 0;
    virtual VclAbstractDialog * CreateScSortWarningDlg ( Window* pParent, const String& rExtendText, const String& rCurrentText, int nId ) = 0;  //add for ScSortWarningDlg
    virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg (Window* pParent ,int nId ) = 0; //add for ScDataPilotDatabaseDlg

    virtual AbstractScDataPilotSourceTypeDlg * CreateScDataPilotSourceTypeDlg ( Window* pParent, BOOL bEnableExternal, int nId ) = 0; //add for ScDataPilotSourceTypeDlg

    virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( Window* pParent, //add for ScDataPilotServiceDlg
                                                                        const com::sun::star::uno::Sequence<rtl::OUString>& rServices,
                                                                        int nId ) = 0;

    virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg( Window* pParent, int nId, BOOL bDisallowCellMove = FALSE ) = 0 ; //add for ScDeleteCellDlg

    //for dataform
    virtual AbstractScDataFormDlg * CreateScDataFormDlg( Window* pParent, int nId, ScTabViewShell*      pTabViewShell ) = 0 ; //add for ScDataFormDlg

    virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(Window* pParent,int nId, //add for ScDeleteContentsDlg
                                                                 USHORT  nCheckDefaults = 0 ) = 0;
    virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( Window*        pParent, //add for ScFillSeriesDlg
                                                            ScDocument& rDocument,
                                                            FillDir     eFillDir,
                                                            FillCmd     eFillCmd,
                                                            FillDateCmd eFillDateCmd,
                                                            String          aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            USHORT          nPossDir,
                                                            int nId) = 0;

    virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( Window* pParent, //add for ScGroupDlg
                                                            USHORT  nResId,
                                                            int nId,
                                                            BOOL    bUnGroup = FALSE,
                                                            BOOL    bRows    = TRUE  ) = 0;

    virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( Window* pParent, //add for ScInsertCellDlg
                                                                int nId,
                                                            BOOL bDisallowCellMove = FALSE ) = 0;

    virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( Window*        pParent, //add for ScInsertContentsDlg
                                                                    int nId,
                                                                    USHORT          nCheckDefaults = 0,
                                                                    const String*   pStrTitle = NULL ) = 0;

    virtual AbstractScInsertTableDlg * CreateScInsertTableDlg ( Window* pParent, ScViewData& rViewData,  //add for ScInsertTableDlg
                                                                SCTAB nTabCount, bool bFromFile, int nId) = 0;

    virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( Window* pParent, // add for ScSelEntryDlg
                                                            USHORT  nResId,
                                                        const String& aTitle,
                                                        const String& aLbTitle,
                                                                List&   aEntryList,
                                                            int nId ) = 0;
    virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg (  Window* pParent, int nId) = 0; //add for ScLinkedAreaDlg

    virtual AbstractScMetricInputDlg * CreateScMetricInputDlg (  Window*        pParent, //add for ScMetricInputDlg
                                                                USHORT      nResId,     // derivative for every dialog!
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                int nId ,
                                                                FieldUnit       eFUnit    = FUNIT_MM,
                                                                USHORT      nDecimals = 2,
                                                                long            nMaximum  = 1000,
                                                                long            nMinimum  = 0,
                                                                long            nFirst    = 1,
                                                                long          nLast     = 100 ) = 0;

    virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(  Window* pParent,  //add for ScMoveTableDlg
                                                            const String& rDefault,
                                                            int nId ) = 0;

    virtual AbstractScNameCreateDlg * CreateScNameCreateDlg ( Window * pParent, USHORT nFlags, int nId ) = 0; //add for ScNameCreateDlg

    virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( Window * pParent, const ScRangeName* pList, //add for ScNamePasteDlg
                                                            int nId , BOOL bInsList=TRUE ) = 0;

    virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg ( Window* pParent, //add for ScPivotFilterDlg
                                                                const SfxItemSet&   rArgSet, USHORT nSourceTab , int nId ) = 0;

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( Window* pParent, int nId,
                                                                const ScDPLabelDataVec& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData ) = 0;

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout ) = 0;

    virtual AbstractScDPNumGroupDlg * CreateScDPNumGroupDlg( Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo ) = 0;

    virtual AbstractScDPDateGroupDlg * CreateScDPDateGroupDlg( Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo,
                                                                sal_Int32 nDatePart,
                                                                const Date& rNullDate ) = 0;

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                USHORT nOrient ) = 0;

    virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( Window* pParent, const String& rName, //add for ScNewScenarioDlg
                                                                int nId,
                                                                BOOL bEdit = FALSE, BOOL bSheetProtected = FALSE ) = 0;
    virtual AbstractScShowTabDlg * CreateScShowTabDlg ( Window* pParent, int nId ) = 0; //add for ScShowTabDlg

    virtual AbstractScStringInputDlg * CreateScStringInputDlg (  Window* pParent, //add for ScStringInputDlg
                                                                const String& rTitle,
                                                                const String& rEditTitle,
                                                                const String& rDefault,
                                                                ULONG nHelpId ,
                                                                int nId ) = 0;

    virtual AbstractScTabBgColorDlg * CreateScTabBgColorDlg (  Window* pParent, //add for ScTabBgColorDlg
                                                                const String& rTitle, //Dialog Title
                                                                const String& rTabBgColorNoColorText, //Label for no tab color
                                                                const Color& rDefaultColor, //Currently selected Color
                                                                ULONG nHelpId ,
                                                                int nId ) = 0;

    virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( Window*                 pParent, //add for ScImportOptionsDlg
                                                                    int nId,
                                                                    BOOL                    bAscii = TRUE,
                                                                    const ScImportOptions*  pOptions = NULL,
                                                                    const String*           pStrTitle = NULL,
                                                                    BOOL                    bMultiByte = FALSE,
                                                                    BOOL                    bOnlyDbtoolsEncodings = FALSE,
                                                                    BOOL                    bImport = TRUE ) = 0;

    virtual SfxAbstractTabDialog * CreateScAttrDlg( SfxViewFrame*    pFrame, //add for ScAttrDlg
                                                    Window*          pParent,
                                                    const SfxItemSet* pCellAttrs,
                                                    int nId) = 0;

    virtual SfxAbstractTabDialog * CreateScHFEditDlg( SfxViewFrame*     pFrame, //add for ScHFEditDlg
                                                    Window*         pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const String&       rPageStyle,
                                                    int nId,
                                                    USHORT              nResId = RID_SCDLG_HFEDIT ) = 0;

    virtual SfxAbstractTabDialog * CreateScStyleDlg( Window*                pParent,//add for ScStyleDlg
                                                    SfxStyleSheetBase&  rStyleBase,
                                                    USHORT              nRscId,
                                                    int nId) = 0;

    virtual SfxAbstractTabDialog * CreateScSubTotalDlg( Window*             pParent, //add for ScSubTotalDlg
                                                        const SfxItemSet*   pArgSet,
                                                        int nId) = 0;

    virtual SfxAbstractTabDialog * CreateScCharDlg( Window* pParent, const SfxItemSet* pAttr,//add for ScCharDlg
                                                    const SfxObjectShell* pDocShell, int nId ) = 0;

    virtual SfxAbstractTabDialog * CreateScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ,//add for ScParagraphDlg
                                                            int nId ) = 0;

    virtual SfxAbstractTabDialog * CreateScValidationDlg( Window* pParent, //add for ScValidationDlg
                                                        const SfxItemSet* pArgSet,int nId, ScTabViewShell *pTabVwSh  ) = 0;
    virtual SfxAbstractTabDialog * CreateScSortDlg( Window*          pParent, //add for ScSortDlg
                                                    const SfxItemSet* pArgSet,int nId ) = 0;
    // for tabpage
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId ) = 0;
    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId ) = 0;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
