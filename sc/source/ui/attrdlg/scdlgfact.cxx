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

#undef SC_DLLIMPLEMENTATION

#include "scdlgfact.hxx"

#include "sc.hrc"
#include "scuiasciiopt.hxx"
#include "scuiautofmt.hxx"
#include "corodlg.hxx"
#include "crdlg.hxx"
#include "dapidata.hxx"
#include "dapitype.hxx"
#include "delcldlg.hxx"
#include "delcodlg.hxx"
#include "filldlg.hxx"
#include "groupdlg.hxx"
#include "inscldlg.hxx"
#include "inscodlg.hxx"
#include "instbdlg.hxx"
#include "lbseldlg.hxx"
#include "linkarea.hxx"
#include "mtrindlg.hxx"
#include "mvtabdlg.hxx"
#include "namecrea.hxx"
#include "namepast.hxx"
#include "namedefdlg.hxx"
#include "pfiltdlg.hxx"
#include "pvfundlg.hxx"
#include "dpgroupdlg.hxx"
#include "scendlg.hxx"
#include "shtabdlg.hxx"
#include "strindlg.hxx"
#include "tabbgcolordlg.hxx"
#include "scuiimoptdlg.hxx"
#include "attrdlg.hxx"
#include "hfedtdlg.hxx"
#include "styledlg.hxx"
#include "subtdlg.hxx"
#include "textdlgs.hxx"
#include "validate.hxx"
#include "validate.hrc"
#include "sortdlg.hxx"
#include "textimportoptions.hxx"
#include "opredlin.hxx"
#include "tpcalc.hxx"
#include "tpprint.hxx"
#include "tpstat.hxx"
#include "tpusrlst.hxx"
#include "tpview.hxx"
#include "tpformula.hxx"
#include "datafdlg.hxx"
#include "tpcompatibility.hxx"
#include "tpdefaults.hxx"
#include "colorformat.hxx"
#include "condformatdlg.hxx"
#include "condformatmgr.hxx"
#include "xmlsourcedlg.hxx"

// ause
#include "editutil.hxx"

IMPL_ABSTDLG_BASE(ScVclAbstractDialog_Impl);
IMPL_ABSTDLG_BASE(AbstractScImportAsciiDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScAutoFormatDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScColRowLabelDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScCondFormatManagerDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDataPilotDatabaseDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDataPilotSourceTypeDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDataPilotServiceDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDeleteCellDlg_Impl);
//for dataform
IMPL_ABSTDLG_BASE(AbstractScDataFormDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDeleteContentsDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScFillSeriesDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScGroupDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScInsertCellDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScInsertContentsDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScInsertTableDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScSelEntryDlg_Impl);
IMPL_ABSTDLG2_BASE(AbstractScLinkedAreaDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScMetricInputDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScMoveTableDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScNameCreateDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScNamePasteDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScPivotFilterDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDPFunctionDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDPSubtotalDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDPNumGroupDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDPDateGroupDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDPShowDetailDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScNewScenarioDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScShowTabDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScSortWarningDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScStringInputDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScTabBgColorDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScImportOptionsDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScTextImportOptionsDlg_Impl);
IMPL_ABSTDLG_BASE(ScAbstractTabDialog_Impl);

void ScAbstractTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

void ScAbstractTabDialog_Impl::SetCurPageId( const OString& rName )
{
    pDlg->SetCurPageId( rName );
}

const SfxItemSet* ScAbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}

const sal_uInt16* ScAbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}

void ScAbstractTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}

//From class Window.
void ScAbstractTabDialog_Impl::SetText( const OUString& rStr )
{
    pDlg->SetText( rStr );
}
OUString ScAbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

void AbstractScImportAsciiDlg_Impl::GetOptions( ScAsciiOptions& rOpt )
{
    pDlg->GetOptions( rOpt );
}

void AbstractScImportAsciiDlg_Impl::SaveParameters()
{
    pDlg->SaveParameters();
}

sal_uInt16 AbstractScAutoFormatDlg_Impl::GetIndex() const
{
    return pDlg->GetIndex();
}

OUString AbstractScAutoFormatDlg_Impl::GetCurrFormatName()
{
    return pDlg->GetCurrFormatName();
}

bool  AbstractScColRowLabelDlg_Impl::IsCol()
{
    return  pDlg->IsCol();
}

bool AbstractScColRowLabelDlg_Impl::IsRow()
{
    return pDlg->IsRow();
}

void AbstractScDataPilotDatabaseDlg_Impl::GetValues( ScImportSourceDesc& rDesc )
{
    pDlg->GetValues(rDesc);
}

bool AbstractScDataPilotSourceTypeDlg_Impl::IsDatabase() const
{
    return pDlg->IsDatabase();
}

bool AbstractScDataPilotSourceTypeDlg_Impl::IsExternal() const
{
    return pDlg->IsExternal();
}

bool AbstractScDataPilotSourceTypeDlg_Impl::IsNamedRange() const
{
    return pDlg->IsNamedRange();
}

OUString AbstractScDataPilotSourceTypeDlg_Impl::GetSelectedNamedRange() const
{
    return pDlg->GetSelectedNamedRange();
}

void AbstractScDataPilotSourceTypeDlg_Impl::AppendNamedRange(const OUString& rName)
{
    pDlg->AppendNamedRange(rName);
}

OUString AbstractScDataPilotServiceDlg_Impl::GetServiceName() const
{
    return pDlg->GetServiceName();
}

OUString AbstractScDataPilotServiceDlg_Impl::GetParSource() const
{
    return pDlg->GetParSource();
}

OUString AbstractScDataPilotServiceDlg_Impl::GetParName() const
{
    return pDlg->GetParName();
}

OUString AbstractScDataPilotServiceDlg_Impl::GetParUser() const
{
    return pDlg->GetParUser();
}

OUString AbstractScDataPilotServiceDlg_Impl::GetParPass() const
{
    return pDlg->GetParPass();
}

DelCellCmd AbstractScDeleteCellDlg_Impl::GetDelCellCmd() const
{
    return pDlg->GetDelCellCmd();
}

void AbstractScDeleteContentsDlg_Impl::DisableObjects()
{
    pDlg->DisableObjects();
}

sal_uInt16   AbstractScDeleteContentsDlg_Impl::GetDelContentsCmdBits() const
{
    return pDlg->GetDelContentsCmdBits();
}

FillDir AbstractScFillSeriesDlg_Impl::GetFillDir() const
{
    return pDlg->GetFillDir();
}

FillCmd AbstractScFillSeriesDlg_Impl::GetFillCmd() const
{
    return pDlg->GetFillCmd();
}

FillDateCmd AbstractScFillSeriesDlg_Impl::GetFillDateCmd() const
{
    return pDlg->GetFillDateCmd();
}

double  AbstractScFillSeriesDlg_Impl::GetStart() const
{
    return pDlg->GetStart();
}

double  AbstractScFillSeriesDlg_Impl::GetStep() const
{
    return pDlg->GetStep();
}

double  AbstractScFillSeriesDlg_Impl::GetMax() const
{
    return pDlg->GetMax();
}

OUString  AbstractScFillSeriesDlg_Impl::GetStartStr() const
{
    return pDlg->GetStartStr();
}

void    AbstractScFillSeriesDlg_Impl::SetEdStartValEnabled(bool bFlag)
{
    pDlg->SetEdStartValEnabled(bFlag);
}

bool AbstractScGroupDlg_Impl::GetColsChecked() const
{
    return pDlg->GetColsChecked();
}

InsCellCmd  AbstractScInsertCellDlg_Impl::GetInsCellCmd() const
{
    return pDlg->GetInsCellCmd();
}

sal_uInt16 AbstractScInsertContentsDlg_Impl::GetInsContentsCmdBits() const
{
    return pDlg->GetInsContentsCmdBits();
}

sal_uInt16  AbstractScInsertContentsDlg_Impl::GetFormulaCmdBits() const
{
    return pDlg->GetFormulaCmdBits();
}

bool    AbstractScInsertContentsDlg_Impl::IsSkipEmptyCells() const
{
    return pDlg->IsSkipEmptyCells();
}

bool    AbstractScInsertContentsDlg_Impl::IsLink() const
{
    return pDlg->IsLink();
}

void    AbstractScInsertContentsDlg_Impl::SetFillMode( bool bSet )
{
    pDlg->SetFillMode( bSet );
}

void    AbstractScInsertContentsDlg_Impl::SetOtherDoc( bool bSet )
{
    pDlg->SetOtherDoc( bSet );
}

bool    AbstractScInsertContentsDlg_Impl::IsTranspose() const
{
    return pDlg->IsTranspose();
}

void    AbstractScInsertContentsDlg_Impl::SetChangeTrack( bool bSet )
{
    pDlg->SetChangeTrack( bSet );
}

void    AbstractScInsertContentsDlg_Impl::SetCellShiftDisabled( int nDisable )
{
    pDlg->SetCellShiftDisabled( nDisable );
}

InsCellCmd  AbstractScInsertContentsDlg_Impl::GetMoveMode()
{
    return pDlg->GetMoveMode();
}

bool AbstractScInsertTableDlg_Impl::GetTablesFromFile()
{
    return pDlg->GetTablesFromFile();
}

bool AbstractScInsertTableDlg_Impl::GetTablesAsLink()
{
    return pDlg->GetTablesAsLink();
}

const OUString*  AbstractScInsertTableDlg_Impl::GetFirstTable( sal_uInt16* pN )
{
    return pDlg->GetFirstTable( pN );
}

ScDocShell* AbstractScInsertTableDlg_Impl::GetDocShellTables()
{
    return pDlg->GetDocShellTables();
}

bool AbstractScInsertTableDlg_Impl::IsTableBefore()
{
    return pDlg->IsTableBefore();
}

sal_uInt16 AbstractScInsertTableDlg_Impl::GetTableCount()
{
    return pDlg->GetTableCount();
}

const OUString* AbstractScInsertTableDlg_Impl::GetNextTable( sal_uInt16* pN )
{
    return pDlg->GetNextTable( pN );
}

OUString AbstractScSelEntryDlg_Impl::GetSelectEntry() const
{
    return pDlg->GetSelectEntry();
}

void AbstractScLinkedAreaDlg_Impl::InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh )
{
    pDlg->InitFromOldLink( rFile, rFilter, rOptions, rSource, nRefresh);
}

OUString  AbstractScLinkedAreaDlg_Impl::GetURL()
{
    return pDlg->GetURL();
}

OUString  AbstractScLinkedAreaDlg_Impl::GetFilter()
{
    return pDlg->GetFilter();
}

OUString  AbstractScLinkedAreaDlg_Impl::GetOptions()
{
    return pDlg->GetOptions();
}

OUString  AbstractScLinkedAreaDlg_Impl::GetSource()
{
    return pDlg->GetSource();
}

sal_uLong   AbstractScLinkedAreaDlg_Impl::GetRefresh()
{
    return pDlg->GetRefresh();
}

ScConditionalFormatList* AbstractScCondFormatManagerDlg_Impl::GetConditionalFormatList()
{
    return pDlg->GetConditionalFormatList();
}

bool AbstractScCondFormatManagerDlg_Impl::CondFormatsChanged()
{
    return pDlg->CondFormatsChanged();
}

long AbstractScMetricInputDlg_Impl::GetInputValue( FieldUnit eUnit ) const
{

    return pDlg->GetInputValue( eUnit );
}

sal_uInt16 AbstractScMoveTableDlg_Impl::GetSelectedDocument() const
{
    return pDlg->GetSelectedDocument();
}

sal_uInt16  AbstractScMoveTableDlg_Impl::GetSelectedTable() const
{
    return pDlg->GetSelectedTable();
}

bool AbstractScMoveTableDlg_Impl::GetCopyTable() const
{
    return pDlg->GetCopyTable();
}

bool AbstractScMoveTableDlg_Impl::GetRenameTable() const
{
    return pDlg->GetRenameTable();
}

void AbstractScMoveTableDlg_Impl::GetTabNameString( OUString& rString ) const
{
    pDlg->GetTabNameString( rString );
}

void    AbstractScMoveTableDlg_Impl::SetForceCopyTable()
{
    return pDlg->SetForceCopyTable();
}

void    AbstractScMoveTableDlg_Impl::EnableCopyTable(bool bFlag)
{
    return pDlg->EnableCopyTable( bFlag);
}

void    AbstractScMoveTableDlg_Impl::EnableRenameTable(bool bFlag)
{
    return pDlg->EnableRenameTable( bFlag);
}

sal_uInt16 AbstractScNameCreateDlg_Impl::GetFlags() const
{
    return pDlg->GetFlags();
}

std::vector<OUString> AbstractScNamePasteDlg_Impl::GetSelectedNames() const
{
    return pDlg->GetSelectedNames();
}

bool AbstractScNamePasteDlg_Impl::IsAllSelected() const
{
    return pDlg->IsAllSelected();
}

const ScQueryItem&   AbstractScPivotFilterDlg_Impl::GetOutputItem()
{
    return pDlg->GetOutputItem();
}

sal_uInt16 AbstractScDPFunctionDlg_Impl::GetFuncMask() const
{
     return pDlg->GetFuncMask();
}

::com::sun::star::sheet::DataPilotFieldReference AbstractScDPFunctionDlg_Impl::GetFieldRef() const
{
    return pDlg->GetFieldRef();
}

sal_uInt16 AbstractScDPSubtotalDlg_Impl::GetFuncMask() const
{
     return pDlg->GetFuncMask();
}

void AbstractScDPSubtotalDlg_Impl::FillLabelData( ScDPLabelData& rLabelData ) const
{
    pDlg->FillLabelData( rLabelData );
}

ScDPNumGroupInfo AbstractScDPNumGroupDlg_Impl::GetGroupInfo() const
{
    return pDlg->GetGroupInfo();
}

ScDPNumGroupInfo AbstractScDPDateGroupDlg_Impl::GetGroupInfo() const
{
    return pDlg->GetGroupInfo();
}

sal_Int32 AbstractScDPDateGroupDlg_Impl::GetDatePart() const
{
    return pDlg->GetDatePart();
}

OUString AbstractScDPShowDetailDlg_Impl::GetDimensionName() const
{
     return pDlg->GetDimensionName();
}

void AbstractScNewScenarioDlg_Impl::SetScenarioData(
    const OUString& rName, const OUString& rComment, const Color& rColor, sal_uInt16 nFlags )
{
    pDlg->SetScenarioData(rName, rComment, rColor, nFlags);
}

void AbstractScNewScenarioDlg_Impl::GetScenarioData(
    OUString& rName, OUString& rComment, Color& rColor, sal_uInt16& rFlags ) const
{
    pDlg->GetScenarioData(rName, rComment, rColor, rFlags);
}

void AbstractScShowTabDlg_Impl::Insert( const OUString& rString, bool bSelected )
{
    pDlg->Insert( rString, bSelected);
}

sal_Int32 AbstractScShowTabDlg_Impl::GetSelectEntryCount() const
{
    return pDlg->GetSelectEntryCount();
}

void    AbstractScShowTabDlg_Impl::SetDescription(
                const OUString& rTitle, const OUString& rFixedText,
                const OString& sDlgHelpId, const OString& sLbHelpId )
{
    pDlg->SetDescription( rTitle, rFixedText, sDlgHelpId, sLbHelpId );
}

sal_Int32 AbstractScShowTabDlg_Impl::GetSelectEntryPos(sal_Int32 nPos) const
{
    return pDlg->GetSelectEntryPos( nPos);
}

OUString AbstractScShowTabDlg_Impl::GetSelectEntry(sal_Int32 nPos) const
{
    return pDlg->GetSelectEntry(nPos);
}

OUString AbstractScStringInputDlg_Impl::GetInputString() const
{
    return pDlg->GetInputString();
}

void AbstractScTabBgColorDlg_Impl::GetSelectedColor( Color& rColor ) const
{
    pDlg->GetSelectedColor( rColor );
}

void AbstractScImportOptionsDlg_Impl::GetImportOptions( ScImportOptions& rOptions ) const
{
    pDlg->GetImportOptions(rOptions);
}

LanguageType AbstractScTextImportOptionsDlg_Impl::GetLanguageType() const
{
    return pDlg->getLanguageType();
}

bool AbstractScTextImportOptionsDlg_Impl::IsDateConversionSet() const
{
    return pDlg->isDateConversionSet();
}

// =========================Factories  for createdialog ===================
AbstractScImportAsciiDlg * ScAbstractDialogFactory_Impl::CreateScImportAsciiDlg ( Window* pParent, const OUString& aDatName,
                                                    SvStream* pInStream, ScImportAsciiCall eCall )
{
    ScImportAsciiDlg* pDlg = new ScImportAsciiDlg( pParent, aDatName,pInStream, eCall );
    return new AbstractScImportAsciiDlg_Impl( pDlg );
}

AbstractScTextImportOptionsDlg * ScAbstractDialogFactory_Impl::CreateScTextImportOptionsDlg(Window* pParent)
{
    ScTextImportOptionsDlg* pDlg = new ScTextImportOptionsDlg(pParent);
    return new AbstractScTextImportOptionsDlg_Impl(pDlg);
}

AbstractScAutoFormatDlg * ScAbstractDialogFactory_Impl::CreateScAutoFormatDlg(Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData *pViewData)
{
    ScAutoFormatDlg* pDlg = new ScAutoFormatDlg(pParent, pAutoFormat, pSelFormatData, pViewData);
    return new AbstractScAutoFormatDlg_Impl(pDlg);
}

AbstractScColRowLabelDlg *  ScAbstractDialogFactory_Impl::CreateScColRowLabelDlg(Window* pParent,
                                                                bool bCol ,
                                                                bool bRow)
{
    ScColRowLabelDlg* pDlg = new ScColRowLabelDlg( pParent, bCol,bRow );
    return new AbstractScColRowLabelDlg_Impl( pDlg );
}

AbstractScSortWarningDlg* ScAbstractDialogFactory_Impl::CreateScSortWarningDlg( Window* pParent, const OUString& rExtendText, const OUString& rCurrentText )
{
    ScSortWarningDlg* pDlg = new ScSortWarningDlg(pParent, rExtendText, rCurrentText );
    return new AbstractScSortWarningDlg_Impl( pDlg );
}

AbstractScCondFormatManagerDlg* ScAbstractDialogFactory_Impl::CreateScCondFormatMgrDlg(Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList,
                                                                const ScAddress& rPos, int nId )
{
    ScCondFormatManagerDlg* pDlg = NULL;
    switch( nId )
    {
        case RID_SCDLG_COND_FORMAT_MANAGER:
            pDlg = new ScCondFormatManagerDlg( pParent, pDoc, pFormatList, rPos );
            break;
        default:
            break;
    }
    if(pDlg)
        return new AbstractScCondFormatManagerDlg_Impl( pDlg );

    return NULL;
}

AbstractScDataPilotDatabaseDlg *  ScAbstractDialogFactory_Impl::CreateScDataPilotDatabaseDlg(Window* pParent)
{
    ScDataPilotDatabaseDlg * pDlg = new ScDataPilotDatabaseDlg( pParent );
    return new AbstractScDataPilotDatabaseDlg_Impl( pDlg );
}

AbstractScDataPilotSourceTypeDlg* ScAbstractDialogFactory_Impl::CreateScDataPilotSourceTypeDlg(
    Window* pParent, bool bEnableExternal)
{
    ScDataPilotSourceTypeDlg* pDlg = new ScDataPilotSourceTypeDlg(pParent, bEnableExternal);
    return new AbstractScDataPilotSourceTypeDlg_Impl(pDlg);
}

AbstractScDataPilotServiceDlg* ScAbstractDialogFactory_Impl::CreateScDataPilotServiceDlg( Window* pParent,
                                                                        const com::sun::star::uno::Sequence<OUString>& rServices,
                                                            int nId )
{
    ScDataPilotServiceDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_DAPISERVICE :
            pDlg = new ScDataPilotServiceDlg( pParent, rServices );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScDataPilotServiceDlg_Impl( pDlg );
    return 0;
}

AbstractScDeleteCellDlg* ScAbstractDialogFactory_Impl::CreateScDeleteCellDlg(Window* pParent,
    bool bDisallowCellMove)
{
    ScDeleteCellDlg * pDlg = new ScDeleteCellDlg(pParent, bDisallowCellMove);
    return new AbstractScDeleteCellDlg_Impl( pDlg );
}

AbstractScDataFormDlg* ScAbstractDialogFactory_Impl::CreateScDataFormDlg(Window* pParent,
    ScTabViewShell* pTabViewShell)
{
    ScDataFormDlg* pDlg = new ScDataFormDlg(pParent, pTabViewShell);
    return new AbstractScDataFormDlg_Impl(pDlg);
}

AbstractScDeleteContentsDlg* ScAbstractDialogFactory_Impl::CreateScDeleteContentsDlg(Window* pParent,
                                                                                     sal_uInt16  nCheckDefaults)
{
    ScDeleteContentsDlg* pDlg = new ScDeleteContentsDlg(pParent, nCheckDefaults);
    return new AbstractScDeleteContentsDlg_Impl( pDlg );
}

AbstractScFillSeriesDlg* ScAbstractDialogFactory_Impl::CreateScFillSeriesDlg( Window*       pParent,
                                                            ScDocument&     rDocument,
                                                            FillDir         eFillDir,
                                                            FillCmd         eFillCmd,
                                                            FillDateCmd     eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16      nPossDir)
{
    ScFillSeriesDlg * pDlg = new ScFillSeriesDlg( pParent, rDocument,eFillDir, eFillCmd,eFillDateCmd, aStartStr,fStep,fMax,nPossDir);
        return new AbstractScFillSeriesDlg_Impl( pDlg );
}

AbstractScGroupDlg* ScAbstractDialogFactory_Impl::CreateAbstractScGroupDlg( Window* pParent,
                                                            bool bUnGroup,
                                                            bool bRows )
{
    ScGroupDlg * pDlg = new ScGroupDlg( pParent, bUnGroup, bRows);
    return new AbstractScGroupDlg_Impl( pDlg );
}

AbstractScInsertCellDlg * ScAbstractDialogFactory_Impl::CreateScInsertCellDlg( Window* pParent,
                                                                int nId,
                                                                bool bDisallowCellMove )
{
    ScInsertCellDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_INSCELL :
            pDlg = new ScInsertCellDlg( pParent, bDisallowCellMove);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScInsertCellDlg_Impl( pDlg );
    return 0;
}

AbstractScInsertContentsDlg * ScAbstractDialogFactory_Impl::CreateScInsertContentsDlg( Window*      pParent,
                                                                                    sal_uInt16      nCheckDefaults,
                                                                                    const OUString* pStrTitle )
{
    ScInsertContentsDlg * pDlg = new ScInsertContentsDlg(pParent, nCheckDefaults, pStrTitle);
    return new AbstractScInsertContentsDlg_Impl( pDlg );
}

AbstractScInsertTableDlg * ScAbstractDialogFactory_Impl::CreateScInsertTableDlg(Window* pParent, ScViewData& rViewData,
    SCTAB nTabCount, bool bFromFile)
{
    ScInsertTableDlg* pDlg = new ScInsertTableDlg( pParent, rViewData,nTabCount, bFromFile );
    return new AbstractScInsertTableDlg_Impl( pDlg );
}

AbstractScSelEntryDlg * ScAbstractDialogFactory_Impl::CreateScSelEntryDlg ( Window* pParent,
                                                        const std::vector<OUString> &rEntryList )
{
    ScSelEntryDlg * pDlg = new ScSelEntryDlg( pParent, rEntryList );
    return new AbstractScSelEntryDlg_Impl( pDlg );
}

AbstractScLinkedAreaDlg * ScAbstractDialogFactory_Impl::CreateScLinkedAreaDlg(Window* pParent)
{
    ScLinkedAreaDlg* pDlg = new ScLinkedAreaDlg( pParent );
    return new AbstractScLinkedAreaDlg_Impl( pDlg );
}

AbstractScMetricInputDlg * ScAbstractDialogFactory_Impl::CreateScMetricInputDlg (  Window*      pParent,
                                                                const OString& sDialogName,
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                FieldUnit       eFUnit,
                                                                sal_uInt16      nDecimals,
                                                                long            nMaximum ,
                                                                long            nMinimum,
                                                                long            nFirst,
                                                                long            nLast )
{
    ScMetricInputDlg * pDlg = new ScMetricInputDlg(pParent, sDialogName, nCurrent ,nDefault, eFUnit,
        nDecimals, nMaximum , nMinimum , nFirst, nLast);
    return new AbstractScMetricInputDlg_Impl( pDlg );
}

AbstractScMoveTableDlg * ScAbstractDialogFactory_Impl::CreateScMoveTableDlg(Window* pParent,
    const OUString& rDefault)
{
    ScMoveTableDlg * pDlg = new ScMoveTableDlg( pParent, rDefault );
    return new AbstractScMoveTableDlg_Impl( pDlg );
}

AbstractScNameCreateDlg * ScAbstractDialogFactory_Impl::CreateScNameCreateDlg(Window * pParent, sal_uInt16 nFlags)
{
    ScNameCreateDlg * pDlg = new ScNameCreateDlg( pParent, nFlags );
    return new AbstractScNameCreateDlg_Impl( pDlg );
}

AbstractScNamePasteDlg * ScAbstractDialogFactory_Impl::CreateScNamePasteDlg ( Window * pParent, ScDocShell* pShell, bool bInsList )
{
    ScNamePasteDlg * pDlg = new ScNamePasteDlg( pParent, pShell, bInsList );
    return new AbstractScNamePasteDlg_Impl( pDlg );
}

AbstractScPivotFilterDlg * ScAbstractDialogFactory_Impl::CreateScPivotFilterDlg(Window* pParent,
    const SfxItemSet& rArgSet, sal_uInt16 nSourceTab)
{
    ScPivotFilterDlg *pDlg = new ScPivotFilterDlg(pParent, rArgSet, nSourceTab);
    return new AbstractScPivotFilterDlg_Impl(pDlg);
}

AbstractScDPFunctionDlg * ScAbstractDialogFactory_Impl::CreateScDPFunctionDlg ( Window* pParent,
                                                                const ScDPLabelDataVector& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData )
{
    ScDPFunctionDlg * pDlg = new ScDPFunctionDlg( pParent, rLabelVec, rLabelData, rFuncData );
    return new AbstractScDPFunctionDlg_Impl( pDlg );
}

AbstractScDPSubtotalDlg * ScAbstractDialogFactory_Impl::CreateScDPSubtotalDlg ( Window* pParent,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout )
{
    ScDPSubtotalDlg * pDlg = new ScDPSubtotalDlg( pParent, rDPObj, rLabelData, rFuncData, rDataFields, bEnableLayout );
    return new AbstractScDPSubtotalDlg_Impl( pDlg );
}

AbstractScDPNumGroupDlg * ScAbstractDialogFactory_Impl::CreateScDPNumGroupDlg(
        Window* pParent, int nId, const ScDPNumGroupInfo& rInfo )
{
    if( nId == RID_SCDLG_DPNUMGROUP )
        return new AbstractScDPNumGroupDlg_Impl( new ScDPNumGroupDlg( pParent, rInfo ) );
    return 0;
}

AbstractScDPDateGroupDlg * ScAbstractDialogFactory_Impl::CreateScDPDateGroupDlg(
        Window* pParent, int nId,
        const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart, const Date& rNullDate )
{
    if( nId == RID_SCDLG_DPDATEGROUP )
        return new AbstractScDPDateGroupDlg_Impl( new ScDPDateGroupDlg( pParent, rInfo, nDatePart, rNullDate ) );
    return 0;
}

AbstractScDPShowDetailDlg * ScAbstractDialogFactory_Impl::CreateScDPShowDetailDlg (
        Window* pParent, int nId, ScDPObject& rDPObj, sal_uInt16 nOrient )
{
    if( nId == RID_SCDLG_DPSHOWDETAIL )
        return new AbstractScDPShowDetailDlg_Impl( new ScDPShowDetailDlg( pParent, rDPObj, nOrient ) );
    return 0;
}

AbstractScNewScenarioDlg * ScAbstractDialogFactory_Impl::CreateScNewScenarioDlg(Window* pParent, const OUString& rName,
    bool bEdit, bool bSheetProtected)
{
    ScNewScenarioDlg *pDlg = new ScNewScenarioDlg(pParent, rName, bEdit, bSheetProtected);
    return new AbstractScNewScenarioDlg_Impl( pDlg );
}

AbstractScShowTabDlg * ScAbstractDialogFactory_Impl::CreateScShowTabDlg(Window* pParent)
{
    ScShowTabDlg * pDlg = new ScShowTabDlg( pParent);
    return new AbstractScShowTabDlg_Impl( pDlg );
}

AbstractScStringInputDlg * ScAbstractDialogFactory_Impl::CreateScStringInputDlg (  Window* pParent,
                                                                const OUString& rTitle,
                                                                const OUString& rEditTitle,
                                                                const OUString& rDefault,
                                                                const OString& sHelpId, const OString& sEditHelpId )
{
    ScStringInputDlg *pDlg = new ScStringInputDlg( pParent, rTitle, rEditTitle, rDefault, sHelpId, sEditHelpId );
    return new AbstractScStringInputDlg_Impl( pDlg );
}

AbstractScTabBgColorDlg * ScAbstractDialogFactory_Impl::CreateScTabBgColorDlg(
                                                            Window* pParent,
                                                            const OUString& rTitle,
                                                            const OUString& rTabBgColorNoColorText,
                                                            const Color& rDefaultColor,
                                                            const OString& sHelpId)
{
    ScTabBgColorDlg * pDlg = new ScTabBgColorDlg( pParent, rTitle, rTabBgColorNoColorText, rDefaultColor, sHelpId );
    return new AbstractScTabBgColorDlg_Impl( pDlg );
}

AbstractScImportOptionsDlg * ScAbstractDialogFactory_Impl::CreateScImportOptionsDlg ( Window*               pParent,
                                                                    bool                    bAscii,
                                                                    const ScImportOptions*  pOptions,
                                                                    const OUString*         pStrTitle,
                                                                    bool                    bMultiByte,
                                                                    bool                    bOnlyDbtoolsEncodings,
                                                                    bool                    bImport )
{
    ScImportOptionsDlg * pDlg = new ScImportOptionsDlg( pParent, bAscii, pOptions,pStrTitle, bMultiByte,bOnlyDbtoolsEncodings, bImport );
    return new AbstractScImportOptionsDlg_Impl( pDlg );
}

SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScAttrDlg(SfxViewFrame* pFrame,
    Window* pParent, const SfxItemSet* pCellAttrs)
{
    SfxTabDialog* pDlg = new ScAttrDlg(pFrame, pParent, pCellAttrs);
    return new ScAbstractTabDialog_Impl(pDlg);
}

SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScHFEditDlg( SfxViewFrame*       pFrame,
                                                                        Window*         pParent,
                                                                        const SfxItemSet&   rCoreSet,
                                                                        const OUString&     rPageStyle,
                                                                        sal_uInt16          nResId )
{
    SfxTabDialog* pDlg = NULL;

    switch (nResId)
    {
        case RID_SCDLG_HFED_HEADER:
        case RID_SCDLG_HFEDIT_HEADER:
            pDlg = new ScHFEditHeaderDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFED_FOOTER:
        case RID_SCDLG_HFEDIT_FOOTER:
            pDlg = new ScHFEditFooterDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_LEFTHEADER:
            pDlg = new ScHFEditLeftHeaderDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_RIGHTHEADER:
            pDlg = new ScHFEditRightHeaderDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_LEFTFOOTER:
            pDlg = new ScHFEditLeftFooterDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_RIGHTFOOTER:
            pDlg = new ScHFEditRightFooterDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_SHDR:
            pDlg = new ScHFEditSharedHeaderDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_SFTR:
            pDlg = new ScHFEditSharedFooterDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_ALL:
            pDlg = new ScHFEditAllDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
        default:
        case RID_SCDLG_HFEDIT:
            pDlg = new ScHFEditActiveDlg(pFrame, pParent, rCoreSet, rPageStyle);
            break;
    }

    return pDlg ? new ScAbstractTabDialog_Impl( pDlg ) : NULL;
}

SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScStyleDlg( Window*              pParent,
                                                                        SfxStyleSheetBase&  rStyleBase,
                                                                        sal_uInt16              nRscId,
                                                                        int nId)
{
    SfxTabDialog* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_STYLES_PAGE :
        case RID_SCDLG_STYLES_PAR :
            pDlg = new ScStyleDlg( pParent, rStyleBase, nRscId );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new ScAbstractTabDialog_Impl( pDlg );
    return 0;
}

SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScSubTotalDlg(Window* pParent, const SfxItemSet* pArgSet)
{
    SfxTabDialog* pDlg = new ScSubTotalDlg( pParent, pArgSet );
    return new ScAbstractTabDialog_Impl( pDlg );
}

SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScCharDlg(
    Window* pParent, const SfxItemSet* pAttr, const SfxObjectShell* pDocShell)
{
    SfxTabDialog* pDlg = new ScCharDlg(pParent, pAttr, pDocShell);
    return new ScAbstractTabDialog_Impl(pDlg);
}

SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScParagraphDlg(
    Window* pParent, const SfxItemSet* pAttr)
{
    SfxTabDialog* pDlg = new ScParagraphDlg(pParent, pAttr);
    return new ScAbstractTabDialog_Impl(pDlg);
}

SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScValidationDlg(Window* pParent,
    const SfxItemSet* pArgSet, ScTabViewShell *pTabVwSh)
{
    SfxTabDialog* pDlg = new ScValidationDlg(pParent, pArgSet, pTabVwSh);
    return new ScAbstractTabDialog_Impl(pDlg);
}

SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScSortDlg(Window* pParent, const SfxItemSet* pArgSet)
{
    SfxTabDialog* pDlg = new ScSortDlg( pParent, pArgSet );
    return new ScAbstractTabDialog_Impl( pDlg );
}

//------------------ Factories for TabPages--------------------
CreateTabPage ScAbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
{
    switch ( nId )
    {
        case RID_SCPAGE_OPREDLINE :
            return ScRedlineOptionsTabPage::Create;
            //break;
        case    RID_SCPAGE_CALC :
            return ScTpCalcOptions::Create;
            //break;
        case RID_SCPAGE_FORMULA:
            return ScTpFormulaOptions::Create;
        case RID_SCPAGE_COMPATIBILITY:
            return ScTpCompatOptions::Create;
        case RID_SCPAGE_DEFAULTS:
            return ScTpDefaultsOptions::Create;
        case    RID_SCPAGE_PRINT :
            return ScTpPrintOptions::Create;
            //break;
        case    RID_SCPAGE_STAT :
            return ScDocStatPage::Create;
            //break;
        case RID_SCPAGE_USERLISTS :
             return ScTpUserLists::Create;
            //break;
        case RID_SCPAGE_CONTENT :
            return ScTpContentOptions::Create;
            //break;
        case RID_SCPAGE_LAYOUT :
            return ScTpLayoutOptions::Create;
            //break;

        default:
            break;
    }

    return 0;
}

GetTabPageRanges ScAbstractDialogFactory_Impl::GetTabPageRangesFunc( sal_uInt16 nId )
{
    switch ( nId )
    {
    case TP_VALIDATION_VALUES :
            return ScTPValidationValue::GetRanges;
            //break;
        default:
            break;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
