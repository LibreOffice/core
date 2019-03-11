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

#include <scuiasciiopt.hxx>
#include <scuiautofmt.hxx>
#include <corodlg.hxx>
#include <dapidata.hxx>
#include <dapitype.hxx>
#include <delcldlg.hxx>
#include <delcodlg.hxx>
#include <filldlg.hxx>
#include <groupdlg.hxx>
#include <inscldlg.hxx>
#include <inscodlg.hxx>
#include <instbdlg.hxx>
#include <lbseldlg.hxx>
#include <linkarea.hxx>
#include <mtrindlg.hxx>
#include <mvtabdlg.hxx>
#include <namecrea.hxx>
#include <namepast.hxx>
#include <pfiltdlg.hxx>
#include <pvfundlg.hxx>
#include <dpgroupdlg.hxx>
#include <scendlg.hxx>
#include <shtabdlg.hxx>
#include <strindlg.hxx>
#include <tabbgcolordlg.hxx>
#include <scuiimoptdlg.hxx>
#include <attrdlg.hxx>
#include <hfedtdlg.hxx>
#include <styledlg.hxx>
#include <subtdlg.hxx>
#include <textdlgs.hxx>
#include <sortdlg.hxx>
#include <textimportoptions.hxx>
#include <opredlin.hxx>
#include <tpcalc.hxx>
#include <tpprint.hxx>
#include <tpstat.hxx>
#include <tpusrlst.hxx>
#include <tpview.hxx>
#include <tpformula.hxx>
#include <datafdlg.hxx>
#include <tpcompatibility.hxx>
#include <tpdefaults.hxx>
#include <condformatmgr.hxx>
#include <scres.hrc>
#include <svx/dialogs.hrc>
#include <sfx2/sfxdlg.hxx>
#include <conditio.hxx>

IMPL_ABSTDLG_BASE(AbstractScImportAsciiDlg_Impl);

short AbstractScAutoFormatDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScColRowLabelDlg_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(AbstractScCondFormatManagerDlg_Impl);

short AbstractScDataPilotDatabaseDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScDataPilotSourceTypeDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScDataPilotServiceDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScDeleteCellDlg_Impl::Execute()
{
    return m_xDlg->run();
}

//for dataform
IMPL_ABSTDLG_BASE(AbstractScDataFormDlg_Impl);

short AbstractScDeleteContentsDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScFillSeriesDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScGroupDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScInsertCellDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScInsertContentsDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScInsertTableDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScSelEntryDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScMetricInputDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScMoveTableDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScNameCreateDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScNamePasteDlg_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(AbstractScPivotFilterDlg_Impl);

short AbstractScDPFunctionDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScDPSubtotalDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScDPNumGroupDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScDPDateGroupDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScDPShowDetailDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScNewScenarioDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScShowTabDlg_Impl::Execute()
{
    return m_xDlg->run();
}

bool AbstractScShowTabDlg_Impl::StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx)
{
    return weld::DialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

short AbstractScSortWarningDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScTabBgColorDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScImportOptionsDlg_Impl::Execute()
{
    return m_xDlg->run();
}

short AbstractScTextImportOptionsDlg_Impl::Execute()
{
    return m_xDlg->run();
}

IMPL_ABSTDLG_BASE(ScAbstractTabDialog_Impl);

AbstractScLinkedAreaDlg_Impl::~AbstractScLinkedAreaDlg_Impl()
{
}

short AbstractScLinkedAreaDlg_Impl::Execute()
{
    return m_xDlg->run();
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
    return m_xDlg->GetIndex();
}

OUString AbstractScAutoFormatDlg_Impl::GetCurrFormatName()
{
    return m_xDlg->GetCurrFormatName();
}

bool AbstractScColRowLabelDlg_Impl::IsCol()
{
    return m_xDlg->IsCol();
}

bool AbstractScColRowLabelDlg_Impl::IsRow()
{
    return m_xDlg->IsRow();
}

void AbstractScDataPilotDatabaseDlg_Impl::GetValues( ScImportSourceDesc& rDesc )
{
    m_xDlg->GetValues(rDesc);
}

bool AbstractScDataPilotSourceTypeDlg_Impl::IsDatabase() const
{
    return m_xDlg->IsDatabase();
}

bool AbstractScDataPilotSourceTypeDlg_Impl::IsExternal() const
{
    return m_xDlg->IsExternal();
}

bool AbstractScDataPilotSourceTypeDlg_Impl::IsNamedRange() const
{
    return m_xDlg->IsNamedRange();
}

OUString AbstractScDataPilotSourceTypeDlg_Impl::GetSelectedNamedRange() const
{
    return m_xDlg->GetSelectedNamedRange();
}

void AbstractScDataPilotSourceTypeDlg_Impl::AppendNamedRange(const OUString& rName)
{
    m_xDlg->AppendNamedRange(rName);
}

OUString AbstractScDataPilotServiceDlg_Impl::GetServiceName() const
{
    return m_xDlg->GetServiceName();
}

OUString AbstractScDataPilotServiceDlg_Impl::GetParSource() const
{
    return m_xDlg->GetParSource();
}

OUString AbstractScDataPilotServiceDlg_Impl::GetParName() const
{
    return m_xDlg->GetParName();
}

OUString AbstractScDataPilotServiceDlg_Impl::GetParUser() const
{
    return m_xDlg->GetParUser();
}

OUString AbstractScDataPilotServiceDlg_Impl::GetParPass() const
{
    return m_xDlg->GetParPass();
}

DelCellCmd AbstractScDeleteCellDlg_Impl::GetDelCellCmd() const
{
    return m_xDlg->GetDelCellCmd();
}

void AbstractScDeleteContentsDlg_Impl::DisableObjects()
{
    m_xDlg->DisableObjects();
}

InsertDeleteFlags AbstractScDeleteContentsDlg_Impl::GetDelContentsCmdBits() const
{
    return m_xDlg->GetDelContentsCmdBits();
}

FillDir AbstractScFillSeriesDlg_Impl::GetFillDir() const
{
    return m_xDlg->GetFillDir();
}

FillCmd AbstractScFillSeriesDlg_Impl::GetFillCmd() const
{
    return m_xDlg->GetFillCmd();
}

FillDateCmd AbstractScFillSeriesDlg_Impl::GetFillDateCmd() const
{
    return m_xDlg->GetFillDateCmd();
}

double  AbstractScFillSeriesDlg_Impl::GetStart() const
{
    return m_xDlg->GetStart();
}

double  AbstractScFillSeriesDlg_Impl::GetStep() const
{
    return m_xDlg->GetStep();
}

double  AbstractScFillSeriesDlg_Impl::GetMax() const
{
    return m_xDlg->GetMax();
}

OUString  AbstractScFillSeriesDlg_Impl::GetStartStr() const
{
    return m_xDlg->GetStartStr();
}

void    AbstractScFillSeriesDlg_Impl::SetEdStartValEnabled(bool bFlag)
{
    m_xDlg->SetEdStartValEnabled(bFlag);
}

bool AbstractScGroupDlg_Impl::GetColsChecked() const
{
    return m_xDlg->GetColsChecked();
}

InsCellCmd  AbstractScInsertCellDlg_Impl::GetInsCellCmd() const
{
    return m_xDlg->GetInsCellCmd();
}

InsertDeleteFlags AbstractScInsertContentsDlg_Impl::GetInsContentsCmdBits() const
{
    return m_xDlg->GetInsContentsCmdBits();
}

ScPasteFunc  AbstractScInsertContentsDlg_Impl::GetFormulaCmdBits() const
{
    return m_xDlg->GetFormulaCmdBits();
}

bool    AbstractScInsertContentsDlg_Impl::IsSkipEmptyCells() const
{
    return m_xDlg->IsSkipEmptyCells();
}

bool    AbstractScInsertContentsDlg_Impl::IsLink() const
{
    return m_xDlg->IsLink();
}

void    AbstractScInsertContentsDlg_Impl::SetFillMode( bool bSet )
{
    m_xDlg->SetFillMode( bSet );
}

void    AbstractScInsertContentsDlg_Impl::SetOtherDoc( bool bSet )
{
    m_xDlg->SetOtherDoc( bSet );
}

bool    AbstractScInsertContentsDlg_Impl::IsTranspose() const
{
    return m_xDlg->IsTranspose();
}

void    AbstractScInsertContentsDlg_Impl::SetChangeTrack( bool bSet )
{
    m_xDlg->SetChangeTrack( bSet );
}

void    AbstractScInsertContentsDlg_Impl::SetCellShiftDisabled( CellShiftDisabledFlags nDisable )
{
    m_xDlg->SetCellShiftDisabled( nDisable );
}

InsCellCmd  AbstractScInsertContentsDlg_Impl::GetMoveMode()
{
    return m_xDlg->GetMoveMode();
}

bool AbstractScInsertTableDlg_Impl::GetTablesFromFile()
{
    return m_xDlg->GetTablesFromFile();
}

bool AbstractScInsertTableDlg_Impl::GetTablesAsLink()
{
    return m_xDlg->GetTablesAsLink();
}

const OUString*  AbstractScInsertTableDlg_Impl::GetFirstTable( sal_uInt16* pN )
{
    return m_xDlg->GetFirstTable( pN );
}

ScDocShell* AbstractScInsertTableDlg_Impl::GetDocShellTables()
{
    return m_xDlg->GetDocShellTables();
}

bool AbstractScInsertTableDlg_Impl::IsTableBefore()
{
    return m_xDlg->IsTableBefore();
}

sal_uInt16 AbstractScInsertTableDlg_Impl::GetTableCount()
{
    return m_xDlg->GetTableCount();
}

const OUString* AbstractScInsertTableDlg_Impl::GetNextTable( sal_uInt16* pN )
{
    return m_xDlg->GetNextTable( pN );
}

OUString AbstractScSelEntryDlg_Impl::GetSelectedEntry() const
{
    return m_xDlg->GetSelectedEntry();
}

void AbstractScLinkedAreaDlg_Impl::InitFromOldLink( const OUString& rFile, const OUString& rFilter,
                                        const OUString& rOptions, const OUString& rSource,
                                        sal_uLong nRefresh )
{
    m_xDlg->InitFromOldLink( rFile, rFilter, rOptions, rSource, nRefresh);
}

OUString  AbstractScLinkedAreaDlg_Impl::GetURL()
{
    return m_xDlg->GetURL();
}

OUString  AbstractScLinkedAreaDlg_Impl::GetFilter()
{
    return m_xDlg->GetFilter();
}

OUString  AbstractScLinkedAreaDlg_Impl::GetOptions()
{
    return m_xDlg->GetOptions();
}

OUString  AbstractScLinkedAreaDlg_Impl::GetSource()
{
    return m_xDlg->GetSource();
}

sal_uLong AbstractScLinkedAreaDlg_Impl::GetRefresh()
{
    return m_xDlg->GetRefresh();
}

std::unique_ptr<ScConditionalFormatList> AbstractScCondFormatManagerDlg_Impl::GetConditionalFormatList()
{
    return pDlg->GetConditionalFormatList();
}

bool AbstractScCondFormatManagerDlg_Impl::CondFormatsChanged() const
{
    return pDlg->CondFormatsChanged();
}

void AbstractScCondFormatManagerDlg_Impl::SetModified()
{
    return pDlg->SetModified();
}

ScConditionalFormat* AbstractScCondFormatManagerDlg_Impl::GetCondFormatSelected()
{
    return pDlg->GetCondFormatSelected();
}

int AbstractScMetricInputDlg_Impl::GetInputValue() const
{
    return m_xDlg->GetInputValue();
}

sal_uInt16 AbstractScMoveTableDlg_Impl::GetSelectedDocument() const
{
    return m_xDlg->GetSelectedDocument();
}

sal_uInt16  AbstractScMoveTableDlg_Impl::GetSelectedTable() const
{
    return m_xDlg->GetSelectedTable();
}

bool AbstractScMoveTableDlg_Impl::GetCopyTable() const
{
    return m_xDlg->GetCopyTable();
}

bool AbstractScMoveTableDlg_Impl::GetRenameTable() const
{
    return m_xDlg->GetRenameTable();
}

void AbstractScMoveTableDlg_Impl::GetTabNameString( OUString& rString ) const
{
    m_xDlg->GetTabNameString( rString );
}

void    AbstractScMoveTableDlg_Impl::SetForceCopyTable()
{
    return m_xDlg->SetForceCopyTable();
}

void    AbstractScMoveTableDlg_Impl::EnableRenameTable(bool bFlag)
{
    return m_xDlg->EnableRenameTable( bFlag);
}

CreateNameFlags AbstractScNameCreateDlg_Impl::GetFlags() const
{
    return m_xDlg->GetFlags();
}

std::vector<OUString> AbstractScNamePasteDlg_Impl::GetSelectedNames() const
{
    return m_xDlg->GetSelectedNames();
}

const ScQueryItem&   AbstractScPivotFilterDlg_Impl::GetOutputItem()
{
    return pDlg->GetOutputItem();
}

PivotFunc AbstractScDPFunctionDlg_Impl::GetFuncMask() const
{
     return m_xDlg->GetFuncMask();
}

css::sheet::DataPilotFieldReference AbstractScDPFunctionDlg_Impl::GetFieldRef() const
{
    return m_xDlg->GetFieldRef();
}

PivotFunc AbstractScDPSubtotalDlg_Impl::GetFuncMask() const
{
     return m_xDlg->GetFuncMask();
}

void AbstractScDPSubtotalDlg_Impl::FillLabelData( ScDPLabelData& rLabelData ) const
{
    m_xDlg->FillLabelData( rLabelData );
}

ScDPNumGroupInfo AbstractScDPNumGroupDlg_Impl::GetGroupInfo() const
{
    return m_xDlg->GetGroupInfo();
}

ScDPNumGroupInfo AbstractScDPDateGroupDlg_Impl::GetGroupInfo() const
{
    return m_xDlg->GetGroupInfo();
}

sal_Int32 AbstractScDPDateGroupDlg_Impl::GetDatePart() const
{
    return m_xDlg->GetDatePart();
}

OUString AbstractScDPShowDetailDlg_Impl::GetDimensionName() const
{
     return m_xDlg->GetDimensionName();
}

void AbstractScNewScenarioDlg_Impl::SetScenarioData(
    const OUString& rName, const OUString& rComment, const Color& rColor, ScScenarioFlags nFlags )
{
    m_xDlg->SetScenarioData(rName, rComment, rColor, nFlags);
}

void AbstractScNewScenarioDlg_Impl::GetScenarioData(
    OUString& rName, OUString& rComment, Color& rColor, ScScenarioFlags& rFlags ) const
{
    m_xDlg->GetScenarioData(rName, rComment, rColor, rFlags);
}

void AbstractScShowTabDlg_Impl::Insert( const OUString& rString, bool bSelected )
{
    m_xDlg->Insert(rString, bSelected);
}

void    AbstractScShowTabDlg_Impl::SetDescription(
                const OUString& rTitle, const OUString& rFixedText,
                const OString& sDlgHelpId, const OString& sLbHelpId )
{
    m_xDlg->SetDescription( rTitle, rFixedText, sDlgHelpId, sLbHelpId );
}

std::vector<sal_Int32> AbstractScShowTabDlg_Impl::GetSelectedRows() const
{
    return m_xDlg->GetSelectedRows();
}

OUString AbstractScShowTabDlg_Impl::GetEntry(sal_Int32 nPos) const
{
    return m_xDlg->GetEntry(nPos);
}

short AbstractScStringInputDlg_Impl::Execute()
{
    return m_xDlg->run();
}

OUString AbstractScStringInputDlg_Impl::GetInputString() const
{
    return m_xDlg->GetInputString();
}

void AbstractScTabBgColorDlg_Impl::GetSelectedColor( Color& rColor ) const
{
    m_xDlg->GetSelectedColor( rColor );
}

void AbstractScImportOptionsDlg_Impl::GetImportOptions( ScImportOptions& rOptions ) const
{
    m_xDlg->GetImportOptions(rOptions);
}

void AbstractScImportOptionsDlg_Impl::SaveImportOptions() const
{
    m_xDlg->SaveImportOptions();
}

LanguageType AbstractScTextImportOptionsDlg_Impl::GetLanguageType() const
{
    return m_xDlg->getLanguageType();
}

bool AbstractScTextImportOptionsDlg_Impl::IsDateConversionSet() const
{
    return m_xDlg->isDateConversionSet();
}

short ScAbstractTabController_Impl::Execute()
{
    return m_xDlg->run();
}

bool ScAbstractTabController_Impl::StartExecuteAsync(AsyncContext &rCtx)
{
    return SfxTabDialogController::runAsync(m_xDlg, rCtx.maEndDialogFn);
}

void ScAbstractTabController_Impl::SetCurPageId( const OString &rName )
{
    m_xDlg->SetCurPageId( rName );
}

const SfxItemSet* ScAbstractTabController_Impl::GetOutputItemSet() const
{
    return m_xDlg->GetOutputItemSet();
}

const sal_uInt16* ScAbstractTabController_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return m_xDlg->GetInputRanges( pItem );
}

void ScAbstractTabController_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     m_xDlg->SetInputSet( pInSet );
}

//From class Window.
void ScAbstractTabController_Impl::SetText( const OUString& rStr )
{
    m_xDlg->set_title(rStr);
}

// =========================Factories  for createdialog ===================
VclPtr<AbstractScImportAsciiDlg> ScAbstractDialogFactory_Impl::CreateScImportAsciiDlg ( vcl::Window* pParent,
                                                    const OUString& aDatName,
                                                    SvStream* pInStream, ScImportAsciiCall eCall )
{
    VclPtr<ScImportAsciiDlg> pDlg = VclPtr<ScImportAsciiDlg>::Create( pParent, aDatName,pInStream, eCall );
    return VclPtr<AbstractScImportAsciiDlg_Impl>::Create( pDlg );
}

VclPtr<AbstractScTextImportOptionsDlg> ScAbstractDialogFactory_Impl::CreateScTextImportOptionsDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScTextImportOptionsDlg_Impl>::Create(std::make_unique<ScTextImportOptionsDlg>(pParent));
}

VclPtr<AbstractScAutoFormatDlg> ScAbstractDialogFactory_Impl::CreateScAutoFormatDlg(weld::Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData *pViewData)
{
    return VclPtr<AbstractScAutoFormatDlg_Impl>::Create(std::make_unique<ScAutoFormatDlg>(pParent, pAutoFormat, pSelFormatData, pViewData));
}

VclPtr<AbstractScColRowLabelDlg>  ScAbstractDialogFactory_Impl::CreateScColRowLabelDlg(weld::Window* pParent,
                                                                bool bCol, bool bRow)
{
    return VclPtr<AbstractScColRowLabelDlg_Impl>::Create(std::make_unique<ScColRowLabelDlg>(pParent, bCol, bRow));
}

VclPtr<AbstractScSortWarningDlg> ScAbstractDialogFactory_Impl::CreateScSortWarningDlg(weld::Window* pParent, const OUString& rExtendText, const OUString& rCurrentText)
{
    return VclPtr<AbstractScSortWarningDlg_Impl>::Create(std::make_unique<ScSortWarningDlg>(pParent, rExtendText, rCurrentText));
}

VclPtr<AbstractScCondFormatManagerDlg> ScAbstractDialogFactory_Impl::CreateScCondFormatMgrDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList )
{
    VclPtr<ScCondFormatManagerDlg> pDlg = VclPtr<ScCondFormatManagerDlg>::Create( pParent, pDoc, pFormatList );
    return VclPtr<AbstractScCondFormatManagerDlg_Impl>::Create( pDlg );
}

VclPtr<AbstractScDataPilotDatabaseDlg> ScAbstractDialogFactory_Impl::CreateScDataPilotDatabaseDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScDataPilotDatabaseDlg_Impl>::Create(std::make_unique<ScDataPilotDatabaseDlg>(pParent));
}

VclPtr<AbstractScDataPilotSourceTypeDlg> ScAbstractDialogFactory_Impl::CreateScDataPilotSourceTypeDlg(
    weld::Window* pParent, bool bEnableExternal)
{
    return VclPtr<AbstractScDataPilotSourceTypeDlg_Impl>::Create(std::make_unique<ScDataPilotSourceTypeDlg>(pParent, bEnableExternal));
}

VclPtr<AbstractScDataPilotServiceDlg> ScAbstractDialogFactory_Impl::CreateScDataPilotServiceDlg(weld::Window* pParent,
                                                                        const std::vector<OUString>& rServices)
{
    return VclPtr<AbstractScDataPilotServiceDlg_Impl>::Create(std::make_unique<ScDataPilotServiceDlg>(pParent, rServices));
}

VclPtr<AbstractScDeleteCellDlg> ScAbstractDialogFactory_Impl::CreateScDeleteCellDlg(weld::Window* pParent,
    bool bDisallowCellMove)
{
    return VclPtr<AbstractScDeleteCellDlg_Impl>::Create(std::make_unique<ScDeleteCellDlg>(pParent, bDisallowCellMove));
}

VclPtr<AbstractScDataFormDlg> ScAbstractDialogFactory_Impl::CreateScDataFormDlg(vcl::Window* pParent,
    ScTabViewShell* pTabViewShell)
{
    VclPtr<ScDataFormDlg> pDlg = VclPtr<ScDataFormDlg>::Create(pParent, pTabViewShell);
    return VclPtr<AbstractScDataFormDlg_Impl>::Create(pDlg);
}

VclPtr<AbstractScDeleteContentsDlg> ScAbstractDialogFactory_Impl::CreateScDeleteContentsDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScDeleteContentsDlg_Impl>::Create(std::make_unique<ScDeleteContentsDlg>(pParent));
}

VclPtr<AbstractScFillSeriesDlg> ScAbstractDialogFactory_Impl::CreateScFillSeriesDlg(weld::Window*       pParent,
                                                            ScDocument&     rDocument,
                                                            FillDir         eFillDir,
                                                            FillCmd         eFillCmd,
                                                            FillDateCmd     eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16      nPossDir)
{
    return VclPtr<AbstractScFillSeriesDlg_Impl>::Create(std::make_unique<ScFillSeriesDlg>(pParent, rDocument,eFillDir, eFillCmd,eFillDateCmd, aStartStr,fStep,fMax,nPossDir));
}

VclPtr<AbstractScGroupDlg> ScAbstractDialogFactory_Impl::CreateAbstractScGroupDlg(weld::Window* pParent, bool bUnGroup)
{
    return VclPtr<AbstractScGroupDlg_Impl>::Create(std::make_unique<ScGroupDlg>(pParent, bUnGroup, true/*bRows*/));
}

VclPtr<AbstractScInsertCellDlg> ScAbstractDialogFactory_Impl::CreateScInsertCellDlg(weld::Window* pParent,
                                                                bool bDisallowCellMove)
{
    return VclPtr<AbstractScInsertCellDlg_Impl>::Create(std::make_unique<ScInsertCellDlg>(pParent, bDisallowCellMove));
}

VclPtr<AbstractScInsertContentsDlg> ScAbstractDialogFactory_Impl::CreateScInsertContentsDlg(weld::Window* pParent,
                                                                                            const OUString* pStrTitle)
{
    return VclPtr<AbstractScInsertContentsDlg_Impl>::Create(std::make_unique<ScInsertContentsDlg>(pParent, pStrTitle));
}

VclPtr<AbstractScInsertTableDlg> ScAbstractDialogFactory_Impl::CreateScInsertTableDlg(weld::Window* pParent, ScViewData& rViewData,
    SCTAB nTabCount, bool bFromFile)
{
    return VclPtr<AbstractScInsertTableDlg_Impl>::Create(std::make_unique<ScInsertTableDlg>(pParent, rViewData,nTabCount, bFromFile));
}

VclPtr<AbstractScSelEntryDlg> ScAbstractDialogFactory_Impl::CreateScSelEntryDlg(weld::Window* pParent,
                                                                                const std::vector<OUString> &rEntryList)
{
    return VclPtr<AbstractScSelEntryDlg_Impl>::Create(std::make_unique<ScSelEntryDlg>(pParent, rEntryList));
}

VclPtr<AbstractScLinkedAreaDlg> ScAbstractDialogFactory_Impl::CreateScLinkedAreaDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScLinkedAreaDlg_Impl>::Create(std::make_unique<ScLinkedAreaDlg>(pParent));
}

VclPtr<AbstractScMetricInputDlg> ScAbstractDialogFactory_Impl::CreateScMetricInputDlg(weld::Window* pParent,
                                                                const OString& sDialogName,
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                FieldUnit       eFUnit,
                                                                sal_uInt16      nDecimals,
                                                                long            nMaximum ,
                                                                long            nMinimum )
{
    return VclPtr<AbstractScMetricInputDlg_Impl>::Create(std::make_unique<ScMetricInputDlg>(pParent, sDialogName, nCurrent ,nDefault, eFUnit,
        nDecimals, nMaximum , nMinimum));
}

VclPtr<AbstractScMoveTableDlg> ScAbstractDialogFactory_Impl::CreateScMoveTableDlg(weld::Window* pParent,
    const OUString& rDefault)
{
    return VclPtr<AbstractScMoveTableDlg_Impl>::Create(std::make_unique<ScMoveTableDlg>(pParent, rDefault));
}

VclPtr<AbstractScNameCreateDlg> ScAbstractDialogFactory_Impl::CreateScNameCreateDlg(weld::Window * pParent, CreateNameFlags nFlags)
{
    return VclPtr<AbstractScNameCreateDlg_Impl>::Create(std::make_unique<ScNameCreateDlg>(pParent, nFlags));
}

VclPtr<AbstractScNamePasteDlg> ScAbstractDialogFactory_Impl::CreateScNamePasteDlg(weld::Window * pParent, ScDocShell* pShell)
{
    return VclPtr<AbstractScNamePasteDlg_Impl>::Create(std::make_unique<ScNamePasteDlg>(pParent, pShell));
}

VclPtr<AbstractScPivotFilterDlg> ScAbstractDialogFactory_Impl::CreateScPivotFilterDlg(vcl::Window* pParent,
    const SfxItemSet& rArgSet, sal_uInt16 nSourceTab)
{
    VclPtr<ScPivotFilterDlg> pDlg = VclPtr<ScPivotFilterDlg>::Create(pParent, rArgSet, nSourceTab);
    return VclPtr<AbstractScPivotFilterDlg_Impl>::Create(pDlg);
}

VclPtr<AbstractScDPFunctionDlg> ScAbstractDialogFactory_Impl::CreateScDPFunctionDlg(weld::Window* pParent,
                                                                                    const ScDPLabelDataVector& rLabelVec,
                                                                                    const ScDPLabelData& rLabelData,
                                                                                    const ScPivotFuncData& rFuncData)
{
    return VclPtr<AbstractScDPFunctionDlg_Impl>::Create(std::make_unique<ScDPFunctionDlg>(pParent, rLabelVec, rLabelData, rFuncData));
}

VclPtr<AbstractScDPSubtotalDlg> ScAbstractDialogFactory_Impl::CreateScDPSubtotalDlg(weld::Window* pParent,
                                                                                    ScDPObject& rDPObj,
                                                                                    const ScDPLabelData& rLabelData,
                                                                                    const ScPivotFuncData& rFuncData,
                                                                                    const ScDPNameVec& rDataFields)
{
    return VclPtr<AbstractScDPSubtotalDlg_Impl>::Create(std::make_unique<ScDPSubtotalDlg>(pParent, rDPObj, rLabelData, rFuncData, rDataFields, true/*bEnableLayout*/));
}

VclPtr<AbstractScDPNumGroupDlg> ScAbstractDialogFactory_Impl::CreateScDPNumGroupDlg(weld::Window* pParent, const ScDPNumGroupInfo& rInfo)
{
    return VclPtr<AbstractScDPNumGroupDlg_Impl>::Create(new ScDPNumGroupDlg(pParent, rInfo));
}

VclPtr<AbstractScDPDateGroupDlg> ScAbstractDialogFactory_Impl::CreateScDPDateGroupDlg(
        weld::Window* pParent, const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart, const Date& rNullDate)
{
    return VclPtr<AbstractScDPDateGroupDlg_Impl>::Create(new ScDPDateGroupDlg(pParent, rInfo, nDatePart, rNullDate));
}

VclPtr<AbstractScDPShowDetailDlg> ScAbstractDialogFactory_Impl::CreateScDPShowDetailDlg (
        weld::Window* pParent, ScDPObject& rDPObj, css::sheet::DataPilotFieldOrientation nOrient )
{
    return VclPtr<AbstractScDPShowDetailDlg_Impl>::Create(new ScDPShowDetailDlg(pParent, rDPObj, nOrient));
}

VclPtr<AbstractScNewScenarioDlg> ScAbstractDialogFactory_Impl::CreateScNewScenarioDlg(weld::Window* pParent, const OUString& rName,
    bool bEdit, bool bSheetProtected)
{
    return VclPtr<AbstractScNewScenarioDlg_Impl>::Create(new ScNewScenarioDlg(pParent, rName, bEdit, bSheetProtected));
}

VclPtr<AbstractScShowTabDlg> ScAbstractDialogFactory_Impl::CreateScShowTabDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScShowTabDlg_Impl>::Create(new ScShowTabDlg(pParent));
}

VclPtr<AbstractScStringInputDlg> ScAbstractDialogFactory_Impl::CreateScStringInputDlg(weld::Window* pParent,
        const OUString& rTitle, const OUString& rEditTitle, const OUString& rDefault, const OString& rHelpId,
        const OString& rEditHelpId)
{
    return VclPtr<AbstractScStringInputDlg_Impl>::Create(std::make_unique<ScStringInputDlg>(pParent, rTitle, rEditTitle,
                rDefault, rHelpId, rEditHelpId));
}

VclPtr<AbstractScTabBgColorDlg> ScAbstractDialogFactory_Impl::CreateScTabBgColorDlg(
                                                            weld::Window* pParent,
                                                            const OUString& rTitle,
                                                            const OUString& rTabBgColorNoColorText,
                                                            const Color& rDefaultColor)
{
    return VclPtr<AbstractScTabBgColorDlg_Impl>::Create(std::make_unique<ScTabBgColorDlg>(pParent, rTitle, rTabBgColorNoColorText, rDefaultColor));
}

VclPtr<AbstractScImportOptionsDlg> ScAbstractDialogFactory_Impl::CreateScImportOptionsDlg(weld::Window* pParent,
                                                                                          bool bAscii,
                                                                                          const ScImportOptions* pOptions,
                                                                                          const OUString* pStrTitle,
                                                                                          bool bOnlyDbtoolsEncodings,
                                                                                          bool bImport)
{
    return VclPtr<AbstractScImportOptionsDlg_Impl>::Create(std::make_unique<ScImportOptionsDlg>(pParent, bAscii, pOptions, pStrTitle, true/*bMultiByte*/, bOnlyDbtoolsEncodings, bImport));
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScAttrDlg(weld::Window* pParent, const SfxItemSet* pCellAttrs)
{
    return VclPtr<ScAbstractTabController_Impl>::Create(std::make_unique<ScAttrDlg>(pParent, pCellAttrs));
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScHFEditDlg( vcl::Window*         pParent,
                                                                        const SfxItemSet&   rCoreSet,
                                                                        const OUString&     rPageStyle,
                                                                        sal_uInt16          nResId )
{
    VclPtr<SfxTabDialog> pDlg;

    switch (nResId)
    {
        case RID_SCDLG_HFED_HEADER:
        case RID_SCDLG_HFEDIT_HEADER:
            pDlg = VclPtr<ScHFEditHeaderDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFED_FOOTER:
        case RID_SCDLG_HFEDIT_FOOTER:
            pDlg = VclPtr<ScHFEditFooterDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_LEFTHEADER:
            pDlg = VclPtr<ScHFEditLeftHeaderDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_RIGHTHEADER:
            pDlg = VclPtr<ScHFEditRightHeaderDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_LEFTFOOTER:
            pDlg = VclPtr<ScHFEditLeftFooterDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_RIGHTFOOTER:
            pDlg = VclPtr<ScHFEditRightFooterDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_SHDR:
            pDlg = VclPtr<ScHFEditSharedHeaderDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_SFTR:
            pDlg = VclPtr<ScHFEditSharedFooterDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        case RID_SCDLG_HFEDIT_ALL:
            pDlg = VclPtr<ScHFEditAllDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
        default:
        case RID_SCDLG_HFEDIT:
            pDlg = VclPtr<ScHFEditActiveDlg>::Create(pParent, rCoreSet, rPageStyle);
            break;
    }

    return pDlg ? VclPtr<ScAbstractTabDialog_Impl>::Create( pDlg ) : nullptr;
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScStyleDlg(weld::Window* pParent,
                                                                            SfxStyleSheetBase& rStyleBase,
                                                                            bool bPage)
{
    return VclPtr<ScAbstractTabController_Impl>::Create(std::make_unique<ScStyleDlg>(pParent, rStyleBase, bPage));
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScSubTotalDlg(weld::Window* pParent, const SfxItemSet* pArgSet)
{
    return VclPtr<ScAbstractTabController_Impl>::Create(std::make_unique<ScSubTotalDlg>(pParent, pArgSet));
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScCharDlg(
    weld::Window* pParent, const SfxItemSet* pAttr, const SfxObjectShell* pDocShell)
{
    return VclPtr<ScAbstractTabController_Impl>::Create(std::make_unique<ScCharDlg>(pParent, pAttr, pDocShell));
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScParagraphDlg(
    weld::Window* pParent, const SfxItemSet* pAttr)
{
    return VclPtr<ScAbstractTabController_Impl>::Create(std::make_unique<ScParagraphDlg>(pParent, pAttr));
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScSortDlg(weld::Window* pParent, const SfxItemSet* pArgSet)
{
    return VclPtr<ScAbstractTabController_Impl>::Create(std::make_unique<ScSortDlg>(pParent, pArgSet));
}

//------------------ Factories for TabPages--------------------
CreateTabPage ScAbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
{
    switch (nId)
    {
        case SID_SC_TP_CHANGES:
            return ScRedlineOptionsTabPage::Create;
        case SID_SC_TP_CALC:
            return ScTpCalcOptions::Create;
        case SID_SC_TP_FORMULA:
            return ScTpFormulaOptions::Create;
        case SID_SC_TP_COMPATIBILITY:
            return ScTpCompatOptions::Create;
        case RID_SC_TP_DEFAULTS:
            return ScTpDefaultsOptions::Create;
        case RID_SC_TP_PRINT:
            return ScTpPrintOptions::Create;
        case SID_SC_TP_STAT:
            return ScDocStatPage::Create;
        case SID_SC_TP_USERLISTS:
             return ScTpUserLists::Create;
        case SID_SC_TP_CONTENT:
            return ScTpContentOptions::Create;
        case SID_SC_TP_LAYOUT:
            return ScTpLayoutOptions::Create;
        default:
            break;
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
