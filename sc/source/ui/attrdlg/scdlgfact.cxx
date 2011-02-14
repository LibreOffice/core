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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#if ! ENABLE_LAYOUT_EXPERIMENTAL
#undef ENABLE_LAYOUT
#endif

#undef SC_DLLIMPLEMENTATION

#include "scdlgfact.hxx"

#include "sc.hrc" //CHINA001
#include "scuiasciiopt.hxx" //add for ScImportAsciiDlg
#include "scuiautofmt.hxx" //add for ScAutoFormatDlg
#include "corodlg.hxx" //add for ScColRowLabelDlg
#include "crdlg.hxx" //add for ScColOrRowDlg
#include "dapidata.hxx" //add for ScDataPilotDatabaseDlg
#include "dapitype.hxx" //add for ScDataPilotSourceTypeDlg, ScDataPilotServiceDlg
#include "delcldlg.hxx" //add for ScDeleteCellDlg
#include "delcodlg.hxx" //add for ScDeleteContentsDlg
#include "filldlg.hxx" //add for ScFillSeriesDlg
#include "groupdlg.hxx" //add for ScGroupDlg
#include "inscldlg.hxx" //add for ScInsertCellDlg
#include "inscodlg.hxx" //add for ScInsertContentsDlg
#include "instbdlg.hxx" //add for ScInsertTableDlg
#include "lbseldlg.hxx" //add for ScSelEntryDlg
#include "linkarea.hxx" //add for ScLinkedAreaDlg
#include "mtrindlg.hxx" //add for ScMetricInputDlg
#include "mvtabdlg.hxx" //add for ScMoveTableDlg
#include "namecrea.hxx" //add for ScNameCreateDlg
#include "namepast.hxx" //add for ScNamePasteDlg
#include "pfiltdlg.hxx" //add for ScPivotFilterDlg
#include "pvfundlg.hxx" //add for ScDPFunctionDlg
#include "dpgroupdlg.hxx"
#include "scendlg.hxx" //add for ScNewScenarioDlg
#include "shtabdlg.hxx" //add for ScShowTabDlg
#include "strindlg.hxx" //add for ScStringInputDlg
#include "tabbgcolordlg.hxx"//add for ScTabBgColorDlg
#include "scuiimoptdlg.hxx" //add for ScImportOptionsDlg
#include "attrdlg.hxx" //add for ScAttrDlg
#include "hfedtdlg.hxx" //add for ScHFEditDlg
#include "styledlg.hxx" //add for ScStyleDlg
#include "subtdlg.hxx" //add for ScSubTotalDlg
#include "textdlgs.hxx" //add for ScCharDlg, ScParagraphDlg
#include "validate.hxx" //add for ScValidationDlg
#include "validate.hrc" //add for ScValidationDlg
#include "sortdlg.hxx" //add for ScSortDlg
#include "textimportoptions.hxx"
#include "opredlin.hxx" //add for  ScRedlineOptionsTabPage
#include "tpcalc.hxx" //add for ScTpCalcOptions
#include "tpprint.hxx" //add for ScTpPrintOptions
#include "tpstat.hxx" //add for ScDocStatPage
#include "tpusrlst.hxx" //add for ScTpUserLists
#include "tpview.hxx" //add for ScTpContentOptions

// ause
#include "editutil.hxx"
#include <sfx2/layout.hxx>

IMPL_ABSTDLG_BASE(VclAbstractDialog_Impl); //add for ScColOrRowDlg
IMPL_ABSTDLG_BASE(AbstractScImportAsciiDlg_Impl);//CHINA001 add for ScImportAsciiDlg
IMPL_ABSTDLG_BASE(AbstractScAutoFormatDlg_Impl); //CHINA001 add for ScAutoFormatDlg
IMPL_ABSTDLG_BASE(AbstractScColRowLabelDlg_Impl); //add for ScColRowLabelDlg
IMPL_ABSTDLG_BASE(AbstractScDataPilotDatabaseDlg_Impl); //add for ScDataPilotDatabaseDlg
IMPL_ABSTDLG_BASE(AbstractScDataPilotSourceTypeDlg_Impl); //add for ScDataPilotSourceTypeDlg
IMPL_ABSTDLG_BASE(AbstractScDataPilotServiceDlg_Impl); //add for ScDataPilotServiceDlg
IMPL_ABSTDLG_BASE(AbstractScDeleteCellDlg_Impl); //add for ScDeleteCellDlg
IMPL_ABSTDLG_BASE(AbstractScDeleteContentsDlg_Impl); //add for ScDeleteContentsDlg
IMPL_ABSTDLG_BASE(AbstractScFillSeriesDlg_Impl); //add for ScFillSeriesDlg
IMPL_ABSTDLG_BASE(AbstractScGroupDlg_Impl); //add for ScGroupDlg
IMPL_ABSTDLG_BASE(AbstractScInsertCellDlg_Impl); //add for ScInsertCellDlg
IMPL_ABSTDLG_BASE(AbstractScInsertContentsDlg_Impl); //add for ScInsertContentsDlg
IMPL_ABSTDLG_BASE(AbstractScInsertTableDlg_Impl); //add for ScInsertTableDlg
IMPL_ABSTDLG_BASE(AbstractScSelEntryDlg_Impl); //add for ScSelEntryDlg
IMPL_ABSTDLG2_BASE(AbstractScLinkedAreaDlg_Impl); //add for ScLinkedAreaDlg
IMPL_ABSTDLG_BASE(AbstractScMetricInputDlg_Impl); //add for ScMetricInputDlg
IMPL_ABSTDLG_BASE(AbstractScMoveTableDlg_Impl); //add for ScMoveTableDlg
IMPL_ABSTDLG_BASE(AbstractScNameCreateDlg_Impl); //add for ScNameCreateDlg
IMPL_ABSTDLG_BASE(AbstractScNamePasteDlg_Impl); //add for ScNamePasteDlg
IMPL_ABSTDLG_BASE(AbstractScPivotFilterDlg_Impl); //add for ScPivotFilterDlg
IMPL_ABSTDLG_BASE(AbstractScDPFunctionDlg_Impl); //add for ScDPFunctionDlg
IMPL_ABSTDLG_BASE(AbstractScDPSubtotalDlg_Impl); //add for ScDPSubtotalDlg
IMPL_ABSTDLG_BASE(AbstractScDPNumGroupDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDPDateGroupDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractScDPShowDetailDlg_Impl); //add for ScDPShowDetailDlg
IMPL_ABSTDLG_BASE(AbstractScNewScenarioDlg_Impl); //add for ScNewScenarioDlg
IMPL_ABSTDLG_BASE(AbstractScShowTabDlg_Impl); //add for ScShowTabDlg
IMPL_ABSTDLG_BASE(AbstractScStringInputDlg_Impl); //add for ScStringInputDlg
IMPL_ABSTDLG_BASE(AbstractScTabBgColorDlg_Impl); //add for ScTabBgColorDlg
IMPL_ABSTDLG_BASE(AbstractScImportOptionsDlg_Impl); //add for ScImportOptionsDlg
IMPL_ABSTDLG_BASE(AbstractScTextImportOptionsDlg_Impl);
IMPL_ABSTDLG_BASE(AbstractTabDialog_Impl); //add for ScAttrDlg, ScHFEditDlg, ScStyleDlg, ScSubTotalDlg,ScCharDlg, ScParagraphDlg, ScValidationDlg, ScSortDlg

// AbstractTabDialog_Impl begin
void AbstractTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

const SfxItemSet* AbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//add by CHINA001
const sal_uInt16* AbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}
//add by CHINA001
void AbstractTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractTabDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
String AbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}

#if ENABLE_LAYOUT
namespace layout
{
IMPL_ABSTDLG_BASE(AbstractTabDialog_Impl); //add for ScAttrDlg, ScHFEditDlg, ScStyleDlg, ScSubTotalDlg,ScCharDlg, ScParagraphDlg, ScValidationDlg, ScSortDlg

// AbstractTabDialog_Impl begin
void AbstractTabDialog_Impl::SetCurPageId( sal_uInt16 nId )
{
    pDlg->SetCurPageId( nId );
}

const SfxItemSet* AbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//add by CHINA001
const sal_uInt16* AbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
{
    return pDlg->GetInputRanges( pItem );
}
//add by CHINA001
void AbstractTabDialog_Impl::SetInputSet( const SfxItemSet* pInSet )
{
     pDlg->SetInputSet( pInSet );
}
//From class Window.
void AbstractTabDialog_Impl::SetText( const XubString& rStr )
{
    pDlg->SetText( rStr );
}
String AbstractTabDialog_Impl::GetText() const
{
    return pDlg->GetText();
}
}
#endif /* ENABLE_LAYOUT */

//add for AbstractTabDialog_Impl end
// AbstractScImportAsciiDlg_Impl begin
void AbstractScImportAsciiDlg_Impl::GetOptions( ScAsciiOptions& rOpt )
{
    pDlg->GetOptions( rOpt );
}

void AbstractScImportAsciiDlg_Impl::SetTextToColumnsMode()
{
    pDlg->SetTextToColumnsMode();
}

void AbstractScImportAsciiDlg_Impl::SaveParameters()
{
    pDlg->SaveParameters();
}

// AbstractScImportAsciiDlg_Impl end

//AbstractScAutoFormatDlg_Impl begin
sal_uInt16 AbstractScAutoFormatDlg_Impl::GetIndex() const
{
    return pDlg->GetIndex();
}

String AbstractScAutoFormatDlg_Impl::GetCurrFormatName()
{
    return pDlg->GetCurrFormatName();
}

//AbstractScAutoFormatDlg_Impl end



//AbstractScColRowLabelDlg_Impl begin
sal_Bool  AbstractScColRowLabelDlg_Impl::IsCol()
{
    return  pDlg->IsCol();
}
sal_Bool AbstractScColRowLabelDlg_Impl::IsRow()
{
    return pDlg->IsRow();
}

//AbstractScColRowLabelDlg_Impl end


//AbstractScDataPilotDatabaseDlg_Impl begin


void AbstractScDataPilotDatabaseDlg_Impl::GetValues( ScImportSourceDesc& rDesc )
{
    pDlg->GetValues(rDesc);
}

//AbstractScDataPilotDatabaseDlg_Impl end

//AbstractScDataPilotSourceTypeDlg_Impl begin

sal_Bool AbstractScDataPilotSourceTypeDlg_Impl::IsDatabase() const
{
    return pDlg->IsDatabase();
}

sal_Bool AbstractScDataPilotSourceTypeDlg_Impl::IsExternal() const
{
    return pDlg->IsExternal();
}

//AbstractScDataPilotSourceTypeDlg_Impl end


// AbstractScDataPilotServiceDlg_Impl begin
String AbstractScDataPilotServiceDlg_Impl::GetServiceName() const
{
    return pDlg->GetServiceName();
}

String AbstractScDataPilotServiceDlg_Impl::GetParSource() const
{
    return pDlg->GetParSource();
}
String AbstractScDataPilotServiceDlg_Impl::GetParName() const
{
    return pDlg->GetParName();
}
String AbstractScDataPilotServiceDlg_Impl::GetParUser() const
{
    return pDlg->GetParUser();
}
String AbstractScDataPilotServiceDlg_Impl::GetParPass() const
{
    return pDlg->GetParPass();
}

//AbstractScDataPilotServiceDlg_Impl end

//AbstractScDeleteCellDlg_Impl begin
DelCellCmd AbstractScDeleteCellDlg_Impl::GetDelCellCmd() const  //add for ScDeleteCellDlg
{
    return pDlg->GetDelCellCmd();
}
//AbstractScDeleteCellDlg_Impl end

//add for AbstractScDeleteContentsDlg_Impl begin
void AbstractScDeleteContentsDlg_Impl::DisableObjects()
{
    pDlg->DisableObjects();
}
sal_uInt16   AbstractScDeleteContentsDlg_Impl::GetDelContentsCmdBits() const
{
    return pDlg->GetDelContentsCmdBits();
}
//add for AbstractScDeleteContentsDlg_Impl end

//add for AbstractScFillSeriesDlg_Impl begin
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
String  AbstractScFillSeriesDlg_Impl::GetStartStr() const
{
    return pDlg->GetStartStr();
}
void    AbstractScFillSeriesDlg_Impl::SetEdStartValEnabled(sal_Bool bFlag)
{
    pDlg->SetEdStartValEnabled(bFlag);
}
//add for AbstractScFillSeriesDlg_Impl end

//add for AbstractScGroupDlg_Impl begin
sal_Bool AbstractScGroupDlg_Impl::GetColsChecked() const
{
    return pDlg->GetColsChecked();
}
//add for AbstractScGroupDlg_Impl end


//add for AbstractScInsertCellDlg_Impl begin
InsCellCmd  AbstractScInsertCellDlg_Impl::GetInsCellCmd() const
{
    return pDlg->GetInsCellCmd();
}

//add for AbstractScInsertCellDlg_Impl end


//add for AbstractScInsertContentsDlg_Impl begin
sal_uInt16 AbstractScInsertContentsDlg_Impl::GetInsContentsCmdBits() const
{
    return pDlg->GetInsContentsCmdBits();
}


sal_uInt16  AbstractScInsertContentsDlg_Impl::GetFormulaCmdBits() const
{
    return pDlg->GetFormulaCmdBits();
}
sal_Bool    AbstractScInsertContentsDlg_Impl::IsSkipEmptyCells() const
{
    return pDlg->IsSkipEmptyCells();
}
sal_Bool    AbstractScInsertContentsDlg_Impl::IsLink() const
{
    return pDlg->IsLink();
}
void    AbstractScInsertContentsDlg_Impl::SetFillMode( sal_Bool bSet )
{
    pDlg->SetFillMode( bSet );
}

void    AbstractScInsertContentsDlg_Impl::SetOtherDoc( sal_Bool bSet )
{
    pDlg->SetOtherDoc( bSet );
}

sal_Bool    AbstractScInsertContentsDlg_Impl::IsTranspose() const
{
    return pDlg->IsTranspose();
}
void    AbstractScInsertContentsDlg_Impl::SetChangeTrack( sal_Bool bSet )
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
//add for AbstractScInsertContentsDlg_Impl end


//add for AbstractScInsertTableDlg_Impl begin
sal_Bool AbstractScInsertTableDlg_Impl::GetTablesFromFile()
{
    return pDlg->GetTablesFromFile();
}

sal_Bool    AbstractScInsertTableDlg_Impl::GetTablesAsLink()
{
    return pDlg->GetTablesAsLink();
}
const String*   AbstractScInsertTableDlg_Impl::GetFirstTable( sal_uInt16* pN )
{
    return pDlg->GetFirstTable( pN );
}
ScDocShell*     AbstractScInsertTableDlg_Impl::GetDocShellTables()
{
    return pDlg->GetDocShellTables();
}
sal_Bool    AbstractScInsertTableDlg_Impl::IsTableBefore()
{
    return pDlg->IsTableBefore();
}
sal_uInt16  AbstractScInsertTableDlg_Impl::GetTableCount()
{
    return pDlg->GetTableCount();
}
const String*   AbstractScInsertTableDlg_Impl::GetNextTable( sal_uInt16* pN )
{
    return pDlg->GetNextTable( pN );
}
//add for AbstractScInsertTableDlg_Impl end

//add for AbstractScSelEntryDlg_Impl begin
String AbstractScSelEntryDlg_Impl::GetSelectEntry() const
{
    return pDlg->GetSelectEntry();
}
//add for AbstractScSelEntryDlg_Impl end

//add for AbstractScLinkedAreaDlg_Impl begin
void AbstractScLinkedAreaDlg_Impl::InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        sal_uLong nRefresh )
{
    pDlg->InitFromOldLink( rFile, rFilter, rOptions, rSource, nRefresh);
}

String  AbstractScLinkedAreaDlg_Impl::GetURL()
{
    return pDlg->GetURL();
}
String  AbstractScLinkedAreaDlg_Impl::GetFilter()
{
    return pDlg->GetFilter();
}
String  AbstractScLinkedAreaDlg_Impl::GetOptions()
{
    return pDlg->GetOptions();
}
String  AbstractScLinkedAreaDlg_Impl::GetSource()
{
    return pDlg->GetSource();
}
sal_uLong   AbstractScLinkedAreaDlg_Impl::GetRefresh()
{
    return pDlg->GetRefresh();
}
//add for AbstractScLinkedAreaDlg_Impl end


//add for AbstractScMetricInputDlg_Impl begin
long AbstractScMetricInputDlg_Impl::GetInputValue( FieldUnit eUnit ) const
{

    return pDlg->GetInputValue( eUnit );
}
//add for AbstractScMetricInputDlg_Impl end

//add for AbstractScMoveTableDlg_Impl begin
sal_uInt16 AbstractScMoveTableDlg_Impl::GetSelectedDocument() const //add for ScMoveTableDlg
{
    return pDlg->GetSelectedDocument();
}

sal_uInt16  AbstractScMoveTableDlg_Impl::GetSelectedTable() const
{
    return pDlg->GetSelectedTable();
}
sal_Bool    AbstractScMoveTableDlg_Impl::GetCopyTable() const
{
    return pDlg->GetCopyTable();
}
void    AbstractScMoveTableDlg_Impl::SetCopyTable(sal_Bool bFla)
{
    return pDlg->SetCopyTable( bFla );
}
void    AbstractScMoveTableDlg_Impl::EnableCopyTable(sal_Bool bFlag)
{
    return pDlg->EnableCopyTable( bFlag);
}
//add for AbstractScMoveTableDlg_Impl end

//add for AbstractScNameCreateDlg_Impl begin
sal_uInt16 AbstractScNameCreateDlg_Impl::GetFlags() const  //add for ScNameCreateDlg
{
    return pDlg->GetFlags();
}
//add for AbstractScNameCreateDlg_Impl end

//add for AbstractScNamePasteDlg_Impl begin
String AbstractScNamePasteDlg_Impl::GetSelectedName() const  //add for ScNamePasteDlg
{
    return pDlg->GetSelectedName();
}

//add for AbstractScNamePasteDlg_Impl end

//add for AbstractScPivotFilterDlg_Impl begin
const ScQueryItem&   AbstractScPivotFilterDlg_Impl::GetOutputItem()  //add for ScPivotFilterDlg
{
    return pDlg->GetOutputItem();
}
//add for AbstractScPivotFilterDlg_Impl end

//add for AbstractScDPFunctionDlg_Impl begin
sal_uInt16 AbstractScDPFunctionDlg_Impl::GetFuncMask() const //add for ScDPFunctionDlg
{
     return pDlg->GetFuncMask();
}
::com::sun::star::sheet::DataPilotFieldReference AbstractScDPFunctionDlg_Impl::GetFieldRef() const
{
    return pDlg->GetFieldRef();
}
//add for AbstractScDPFunctionDlg_Impl end

//add for AbstractScDPSubtotalDlg_Impl begin
sal_uInt16 AbstractScDPSubtotalDlg_Impl::GetFuncMask() const //add for ScDPSubtotalDlg
{
     return pDlg->GetFuncMask();
}
void AbstractScDPSubtotalDlg_Impl::FillLabelData( ScDPLabelData& rLabelData ) const
{
    pDlg->FillLabelData( rLabelData );
}
//add for AbstractScDPSubtotalDlg_Impl end

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

//add for AbstractScDPShowDetailDlg_Impl begin
String AbstractScDPShowDetailDlg_Impl::GetDimensionName() const
{
     return pDlg->GetDimensionName();
}
//add for AbstractScDPShowDetailDlg_Impl end

//add for AbstractScNewScenarioDlg_Impl begin
void AbstractScNewScenarioDlg_Impl::SetScenarioData( const String& rName, const String& rComment,
                            const Color& rColor, sal_uInt16 nFlags )
{
    pDlg->SetScenarioData(rName,rComment, rColor,nFlags);
}

void AbstractScNewScenarioDlg_Impl::GetScenarioData( String& rName, String& rComment,
                            Color& rColor, sal_uInt16& rFlags ) const
{
    pDlg->GetScenarioData( rName,rComment,rColor,rFlags);
}

//add for AbstractScNewScenarioDlg_Impl end

//add for  AbstractScShowTabDlg_Impl begin
void AbstractScShowTabDlg_Impl::Insert( const String& rString, sal_Bool bSelected )  //add for ScShowTabDlg
{
    pDlg->Insert( rString, bSelected);
}

sal_uInt16  AbstractScShowTabDlg_Impl::GetSelectEntryCount() const
{
    return pDlg->GetSelectEntryCount();
}
void    AbstractScShowTabDlg_Impl::SetDescription(
                const String& rTitle, const String& rFixedText,
                const rtl::OString& sDlgHelpId, const rtl::OString& sLbHelpId )
{
    pDlg->SetDescription( rTitle, rFixedText, sDlgHelpId, sLbHelpId );
}
sal_uInt16  AbstractScShowTabDlg_Impl::GetSelectEntryPos(sal_uInt16 nPos) const
{
    return pDlg->GetSelectEntryPos( nPos);
}
String   AbstractScShowTabDlg_Impl::GetSelectEntry(sal_uInt16 nPos) const
{
        return pDlg->GetSelectEntry(nPos);
}
//add for AbstractScShowTabDlg_Impl end

//add for AbstractScStringInputDlg_Impl begin
void AbstractScStringInputDlg_Impl::GetInputString( String& rString ) const  //add for ScStringInputDlg
{
    pDlg->GetInputString( rString );
}
//add for AbstractScStringInputDlg_Impl end

//add for AbstractScTabBgColorDlg_Impl begin
void AbstractScTabBgColorDlg_Impl::GetSelectedColor( Color& rColor ) const  //add for ScTabBgColorDlg
{
    pDlg->GetSelectedColor( rColor );
}
//add for AbstractScTabBgColorDlg_Impl end


//add for AbstractScImportOptionsDlg_Impl begin
void AbstractScImportOptionsDlg_Impl::GetImportOptions( ScImportOptions& rOptions ) const  //add for ScImportOptionsDlg
{
    pDlg->GetImportOptions(rOptions);
}
// add for AbstractScImportOptionsDlg_Impl end

//add for AbstractScLangChooserDlg_Impl begin
LanguageType AbstractScTextImportOptionsDlg_Impl::GetLanguageType() const
{
    return pDlg->getLanguageType();
}

bool AbstractScTextImportOptionsDlg_Impl::IsDateConversionSet() const
{
    return pDlg->isDateConversionSet();
}

//add for AbstractScLangChooserDlg_Impl end

// =========================Factories  for createdialog ===================

//add for ScImportAsciiDlg begin
AbstractScImportAsciiDlg * ScAbstractDialogFactory_Impl::CreateScImportAsciiDlg ( Window* pParent, String aDatName,
                                                    SvStream* pInStream, int nId, sal_Unicode cSep )
{
    ScImportAsciiDlg* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_ASCII :
            pDlg = new ScImportAsciiDlg( pParent, aDatName,pInStream, cSep );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScImportAsciiDlg_Impl( pDlg );
    return 0;
}
// ScImportAsciiDlg end

AbstractScTextImportOptionsDlg * ScAbstractDialogFactory_Impl::CreateScTextImportOptionsDlg( Window* pParent, int nId )
{
    ScTextImportOptionsDlg* pDlg = NULL;
    switch (nId)
    {
        case RID_SCDLG_TEXT_IMPORT_OPTIONS:
            pDlg = new ScTextImportOptionsDlg(pParent);
        break;
        default:
            ;
    }

    return pDlg ? new AbstractScTextImportOptionsDlg_Impl(pDlg) : NULL;
}

//add for ScAutoFormatDlg begin

AbstractScAutoFormatDlg * ScAbstractDialogFactory_Impl::CreateScAutoFormatDlg( Window*                  pParent, //add for ScAutoFormatDlg
                                                                ScAutoFormat*               pAutoFormat,
                                                                const ScAutoFormatData*    pSelFormatData,
                                                                ScDocument*                pDoc,
                                                                int nId)
{
    ScAutoFormatDlg* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_AUTOFORMAT :
            pDlg = new ScAutoFormatDlg( pParent, pAutoFormat,pSelFormatData, pDoc );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScAutoFormatDlg_Impl( pDlg );
    return 0;
}
//add for ScAutoFormatDlg end

//add for ScColRowLabelDlg begin

AbstractScColRowLabelDlg *  ScAbstractDialogFactory_Impl::CreateScColRowLabelDlg(Window* pParent, //add for ScColRowLabelDlg
                                                                int nId,
                                                                sal_Bool bCol ,
                                                                sal_Bool bRow)
{
    ScColRowLabelDlg* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_CHARTCOLROW :
            pDlg = new ScColRowLabelDlg( pParent, bCol,bRow );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScColRowLabelDlg_Impl( pDlg );
    return 0;
}
//add for ScColRowLabelDlg end


//add for ScColOrRowDlg begin

VclAbstractDialog *  ScAbstractDialogFactory_Impl::CreateScColOrRowDlg(Window*          pParent,
                                                    const String&   rStrTitle,
                                                    const String&   rStrLabel,
                                                    int nId,
                                                    sal_Bool                bColDefault)
{
    Dialog * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_COLORROW :
            pDlg = new ScColOrRowDlg( pParent, rStrTitle,rStrLabel,bColDefault );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}
//add for ScColOrRowDlg end

//add for ScSortWarningDlg begin
VclAbstractDialog * ScAbstractDialogFactory_Impl::CreateScSortWarningDlg( Window* pParent, const String& rExtendText,
                                                                          const String& rCurrentText, int nId )
{
    Dialog * pDlg=NULL;
    switch ( nId )
    {
    case RID_SCDLG_SORT_WARNING:
        pDlg = new ScSortWarningDlg( pParent, rExtendText, rCurrentText );
        break;
    default:
        break;
    }
    if( pDlg )
        return new VclAbstractDialog_Impl( pDlg );
    return 0;
}
//add for ScSortWarningDlg end

//add for ScDataPilotDatabaseDlg begin

AbstractScDataPilotDatabaseDlg *  ScAbstractDialogFactory_Impl::CreateScDataPilotDatabaseDlg (Window* pParent ,
                                                                                              int nId )  //add for ScDataPilotDatabaseDlg
{
    ScDataPilotDatabaseDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_DAPIDATA :
            pDlg = new ScDataPilotDatabaseDlg( pParent );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScDataPilotDatabaseDlg_Impl( pDlg );
    return 0;
}
//add for ScDataPilotDatabaseDlg end

//add for ScDataPilotSourceTypeDlg begin
AbstractScDataPilotSourceTypeDlg* ScAbstractDialogFactory_Impl::CreateScDataPilotSourceTypeDlg( Window* pParent,
                                                                                               sal_Bool bEnableExternal,
                                                                                               int nId )
{
    ScDataPilotSourceTypeDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_DAPITYPE :
            pDlg = new ScDataPilotSourceTypeDlg( pParent, bEnableExternal );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScDataPilotSourceTypeDlg_Impl( pDlg );
    return 0;
}

// add for ScDataPilotSourceTypeDlg end


//add for ScDataPilotServiceDlg begin
AbstractScDataPilotServiceDlg* ScAbstractDialogFactory_Impl::CreateScDataPilotServiceDlg( Window* pParent,
                                                                        const com::sun::star::uno::Sequence<rtl::OUString>& rServices,
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

// add for ScDataPilotServiceDlg end

//add for ScDeleteCellDlg begin
AbstractScDeleteCellDlg* ScAbstractDialogFactory_Impl::CreateScDeleteCellDlg( Window* pParent, int nId,
                                                                             sal_Bool bDisallowCellMove )
{
    ScDeleteCellDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_DELCELL :
            pDlg = new ScDeleteCellDlg( pParent, bDisallowCellMove );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScDeleteCellDlg_Impl( pDlg );
    return 0;
}

//add for ScDeleteCellDlg  end

//add for ScDeleteContentsDlg begin
AbstractScDeleteContentsDlg* ScAbstractDialogFactory_Impl::CreateScDeleteContentsDlg(Window* pParent,int nId, //add for ScDeleteContentsDlg
                                                                 sal_uInt16  nCheckDefaults )
{
    ScDeleteContentsDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_DELCONT :
            pDlg = new ScDeleteContentsDlg( pParent, nCheckDefaults );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScDeleteContentsDlg_Impl( pDlg );
    return 0;
}

//add for ScDeleteContentsDlg  end

//add for ScFillSeriesDlg begin
AbstractScFillSeriesDlg* ScAbstractDialogFactory_Impl::CreateScFillSeriesDlg( Window*       pParent, //add for ScFillSeriesDlg
                                                            ScDocument& rDocument,
                                                            FillDir     eFillDir,
                                                            FillCmd     eFillCmd,
                                                            FillDateCmd eFillDateCmd,
                                                            String          aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            sal_uInt16          nPossDir,
                                                            int nId)
{
    ScFillSeriesDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_FILLSERIES :
            pDlg = new ScFillSeriesDlg( pParent, rDocument,eFillDir, eFillCmd,eFillDateCmd, aStartStr,fStep,fMax,nPossDir);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScFillSeriesDlg_Impl( pDlg );
    return 0;
}

//add for ScFillSeriesDlg  end

//add for ScGroupDlg begin
AbstractScGroupDlg* ScAbstractDialogFactory_Impl::CreateAbstractScGroupDlg( Window* pParent,
                                                            sal_uInt16  nResId,
                                                            int nId,
                                                            sal_Bool    bUnGroup ,
                                                            sal_Bool    bRows   )
{
    ScGroupDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_GRP_KILL :
        case RID_SCDLG_GRP_MAKE :
            pDlg = new ScGroupDlg( pParent, nResId,bUnGroup, bRows);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScGroupDlg_Impl( pDlg );
    return 0;
}

//add for ScGroupDlg  end


//add for ScInsertCellDlg begin
AbstractScInsertCellDlg * ScAbstractDialogFactory_Impl::CreateScInsertCellDlg( Window* pParent, //add for ScInsertCellDlg
                                                                int nId,
                                                            sal_Bool bDisallowCellMove )
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

//add for ScInsertCellDlg end

//add for ScInsertContentsDlg begin
AbstractScInsertContentsDlg * ScAbstractDialogFactory_Impl::CreateScInsertContentsDlg( Window*      pParent,
                                                                                    int nId,
                                                                                    sal_uInt16          nCheckDefaults,
                                                                                    const String*   pStrTitle )
{
    ScInsertContentsDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_INSCONT :
            pDlg = new ScInsertContentsDlg( pParent, nCheckDefaults,pStrTitle );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScInsertContentsDlg_Impl( pDlg );
    return 0;
}
//add for ScInsertContentsDlg end

//add for ScInsertTableDlg begin
AbstractScInsertTableDlg * ScAbstractDialogFactory_Impl::CreateScInsertTableDlg ( Window* pParent, ScViewData& rViewData,
                                                                SCTAB nTabCount, bool bFromFile, int nId)
{
    ScInsertTableDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_INSERT_TABLE :
            pDlg = new ScInsertTableDlg( pParent, rViewData,nTabCount, bFromFile );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScInsertTableDlg_Impl( pDlg );
    return 0;
}
//add for ScInsertTableDlg end

// add for ScSelEntryDlg begin
AbstractScSelEntryDlg * ScAbstractDialogFactory_Impl::CreateScSelEntryDlg ( Window* pParent,
                                                            sal_uInt16  nResId,
                                                        const String& aTitle,
                                                        const String& aLbTitle,
                                                                List&   aEntryList,
                                                            int nId )
{
    ScSelEntryDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_SELECTDB :
            pDlg = new ScSelEntryDlg( pParent, nResId,aTitle, aLbTitle, aEntryList );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScSelEntryDlg_Impl( pDlg );
    return 0;
}
// add for ScSelEntryDlg end

//add for ScLinkedAreaDlg begin
AbstractScLinkedAreaDlg * ScAbstractDialogFactory_Impl::CreateScLinkedAreaDlg (  Window* pParent,
                                                                int nId)
{
    ScLinkedAreaDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_LINKAREA :
            pDlg = new ScLinkedAreaDlg( pParent );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScLinkedAreaDlg_Impl( pDlg );
    return 0;
}
//add for ScLinkedAreaDlg end

//add for ScMetricInputDlg begin
AbstractScMetricInputDlg * ScAbstractDialogFactory_Impl::CreateScMetricInputDlg (  Window*      pParent,
                                                                sal_uInt16      nResId,     // Ableitung fuer jeden Dialog!
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                int nId ,
                                                                FieldUnit       eFUnit,
                                                                sal_uInt16      nDecimals,
                                                                long            nMaximum ,
                                                                long            nMinimum,
                                                                long            nFirst,
                                                                long          nLast )
{
    ScMetricInputDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_ROW_MAN :
        case RID_SCDLG_ROW_OPT :
        case RID_SCDLG_COL_MAN :
        case RID_SCDLG_COL_OPT :
            pDlg = new ScMetricInputDlg( pParent , nResId,nCurrent ,nDefault, eFUnit,
                                        nDecimals, nMaximum , nMinimum , nFirst, nLast);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScMetricInputDlg_Impl( pDlg );
    return 0;
}
//add for ScMetricInputDlg end


//add for  ScMoveTableDlg  begin
AbstractScMoveTableDlg * ScAbstractDialogFactory_Impl::CreateScMoveTableDlg(  Window* pParent, int nId )
{
    ScMoveTableDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_MOVETAB :
            pDlg = new ScMoveTableDlg( pParent );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScMoveTableDlg_Impl( pDlg );
    return 0;
}
//add for  ScMoveTableDlg  end


//add for ScNameCreateDlg begin
AbstractScNameCreateDlg * ScAbstractDialogFactory_Impl::CreateScNameCreateDlg ( Window * pParent, sal_uInt16 nFlags, int nId )
{
    ScNameCreateDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_NAMES_CREATE :
            pDlg = new ScNameCreateDlg( pParent, nFlags );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScNameCreateDlg_Impl( pDlg );
    return 0;
}
//add for ScNameCreateDlg end


//add for ScNamePasteDlg begin
 AbstractScNamePasteDlg * ScAbstractDialogFactory_Impl::CreateScNamePasteDlg ( Window * pParent, const ScRangeName* pList,
                                                            int nId , sal_Bool bInsList )
{
    ScNamePasteDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_NAMES_PASTE :
            pDlg = new ScNamePasteDlg( pParent, pList, bInsList );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScNamePasteDlg_Impl( pDlg );
    return 0;
}
//add for ScNamePasteDlg end

//add for ScPivotFilterDlg begin
AbstractScPivotFilterDlg * ScAbstractDialogFactory_Impl::CreateScPivotFilterDlg ( Window* pParent,
                                                                const SfxItemSet&   rArgSet, sal_uInt16 nSourceTab , int nId )
{
    ScPivotFilterDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_PIVOTFILTER :
            pDlg = new ScPivotFilterDlg( pParent, rArgSet, nSourceTab );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScPivotFilterDlg_Impl( pDlg );
    return 0;
}
//add for ScPivotFilterDlg end


//add for ScDPFunctionDlg begin
AbstractScDPFunctionDlg * ScAbstractDialogFactory_Impl::CreateScDPFunctionDlg ( Window* pParent,
                                                                int nId,
                                                                const ScDPLabelDataVec& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData )
{
    ScDPFunctionDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_DPDATAFIELD :
            pDlg = new ScDPFunctionDlg( pParent, rLabelVec, rLabelData, rFuncData );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScDPFunctionDlg_Impl( pDlg );
    return 0;
}
//add for ScDPFunctionDlg end

//add for ScDPSubtotalDlg begin
AbstractScDPSubtotalDlg * ScAbstractDialogFactory_Impl::CreateScDPSubtotalDlg ( Window* pParent,
                                                                int nId,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout )
{
    ScDPSubtotalDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_PIVOTSUBT :
            pDlg = new ScDPSubtotalDlg( pParent, rDPObj, rLabelData, rFuncData, rDataFields, bEnableLayout );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScDPSubtotalDlg_Impl( pDlg );
    return 0;
}
//add for ScDPSubtotalDlg end

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

//add for ScDPShowDetailDlg begin
AbstractScDPShowDetailDlg * ScAbstractDialogFactory_Impl::CreateScDPShowDetailDlg (
        Window* pParent, int nId, ScDPObject& rDPObj, sal_uInt16 nOrient )
{
    if( nId == RID_SCDLG_DPSHOWDETAIL )
        return new AbstractScDPShowDetailDlg_Impl( new ScDPShowDetailDlg( pParent, rDPObj, nOrient ) );
    return 0;
}
//add for ScDPShowDetailDlg end

//add for ScNewScenarioDlg begin
AbstractScNewScenarioDlg * ScAbstractDialogFactory_Impl::CreateScNewScenarioDlg ( Window* pParent, const String& rName,
                                                                int nId,
                                                                sal_Bool bEdit , sal_Bool bSheetProtected )
{
    ScNewScenarioDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_NEWSCENARIO :
            pDlg = new ScNewScenarioDlg( pParent, rName, bEdit,bSheetProtected );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScNewScenarioDlg_Impl( pDlg );
    return 0;
}
//add for ScNewScenarioDlg end

//add for ScShowTabDlg begin
AbstractScShowTabDlg * ScAbstractDialogFactory_Impl::CreateScShowTabDlg ( Window* pParent, int nId )
{
    ScShowTabDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_SHOW_TAB :
            pDlg = new ScShowTabDlg( pParent);
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScShowTabDlg_Impl( pDlg );
    return 0;
}

//add for ScShowTabDlg end


//add for ScStringInputDlg begin
 AbstractScStringInputDlg * ScAbstractDialogFactory_Impl::CreateScStringInputDlg (  Window* pParent,
                                                                const String& rTitle,
                                                                const String& rEditTitle,
                                                                const String& rDefault,
                                                                const rtl::OString& sHelpId, const rtl::OString& sEditHelpId,
                                                                int nId )
{
    ScStringInputDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_STRINPUT :
            pDlg = new ScStringInputDlg( pParent, rTitle, rEditTitle, rDefault, sHelpId, sEditHelpId );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScStringInputDlg_Impl( pDlg );
    return 0;
}
 //add for ScStringInputDlg end

//add for ScTabBgColorDlg begin
AbstractScTabBgColorDlg * ScAbstractDialogFactory_Impl::CreateScTabBgColorDlg (
                                                            Window* pParent,
                                                            const String& rTitle,
                                                            const String& rTabBgColorNoColorText,
                                                            const Color& rDefaultColor,
                                                            const rtl::OString& sHelpId ,
                                                            int nId )
{
ScTabBgColorDlg * pDlg=NULL;
switch ( nId )
{
    case RID_SCDLG_TAB_BG_COLOR :
        pDlg = new ScTabBgColorDlg( pParent, rTitle, rTabBgColorNoColorText, rDefaultColor, sHelpId );
        break;
    default:
        break;
}

if ( pDlg )
    return new AbstractScTabBgColorDlg_Impl( pDlg );
return 0;
}
//add for ScTabBgColorDlg end

//add for ScImportOptionsDlg begin
AbstractScImportOptionsDlg * ScAbstractDialogFactory_Impl::CreateScImportOptionsDlg ( Window*               pParent,
                                                                    int nId,
                                                                    sal_Bool                    bAscii,
                                                                    const ScImportOptions*  pOptions,
                                                                    const String*           pStrTitle,
                                                                    sal_Bool                    bMultiByte,
                                                                    sal_Bool                    bOnlyDbtoolsEncodings,
                                                                    sal_Bool                    bImport )
{
    ScImportOptionsDlg * pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_IMPORTOPT :
            pDlg = new ScImportOptionsDlg( pParent, bAscii, pOptions,pStrTitle, bMultiByte,bOnlyDbtoolsEncodings, bImport );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScImportOptionsDlg_Impl( pDlg );
    return 0;
}
//add for ScImportOptionsDlg end

#if ENABLE_LAYOUT && !LAYOUT_SFX_TABDIALOG_BROKEN
#define SfxTabDialog layout::SfxTabDialog
#define AbstractTabDialog_Impl layout::AbstractTabDialog_Impl
#endif /* ENABLE_LAYOUT */
//add for ScAttrDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScAttrDlg( SfxViewFrame*  pFrame,
                                                                        Window*          pParent,
                                                                        const SfxItemSet* pCellAttrs,
                                                                        int nId)
{
    SfxTabDialog* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_ATTR :
            pDlg = new ScAttrDlg( pFrame, pParent, pCellAttrs );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;

}
//add for ScAttrDlg end
#undef SfxTabDialog
#undef AbstractTabDialog_Impl

//add for ScHFEditDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScHFEditDlg( SfxViewFrame*       pFrame,
                                                                        Window*         pParent,
                                                                        const SfxItemSet&   rCoreSet,
                                                                        const String&       rPageStyle,
                                                                        int nId,
                                                                        sal_uInt16              nResId )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_HFEDIT :
            pDlg = new ScHFEditDlg( pFrame, pParent, rCoreSet,rPageStyle, nResId );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//add for ScHFEditDlg end

//add for ScStyleDlg begin
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
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//add for ScStyleDlg end

//add for ScSubTotalDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScSubTotalDlg( Window*               pParent,
                                                                        const SfxItemSet*   pArgSet,
                                                                        int nId)
{
    SfxTabDialog* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_SUBTOTALS :
            pDlg = new ScSubTotalDlg( pParent, pArgSet );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//add for ScSubTotalDlg end

//add for ScCharDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScCharDlg( Window* pParent, const SfxItemSet* pAttr,
                                                    const SfxObjectShell* pDocShell, int nId )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_CHAR :
            pDlg = new ScCharDlg( pParent, pAttr, pDocShell );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//add for ScCharDlg end

//add for ScParagraphDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ,
                                                                            int nId )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_PARAGRAPH :
            pDlg = new ScParagraphDlg( pParent, pAttr );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//add for ScParagraphDlg end

//add for ScValidationDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScValidationDlg( Window* pParent,
//<!--Modified by PengYunQuan for Validity Cell Range Picker
//                                                      const SfxItemSet* pArgSet,int nId  )
                                                        const SfxItemSet* pArgSet,int nId, ScTabViewShell *pTabVwSh  )
//-->Modified by PengYunQuan for Validity Cell Range Picke
{
    SfxTabDialog* pDlg=NULL;
    switch ( nId )
    {
        case TAB_DLG_VALIDATION :
            //<!--Modified by PengYunQuan for Validity Cell Range Picker
            //pDlg = new ScValidationDlg( pParent, pArgSet );
            pDlg = new ScValidationDlg( pParent, pArgSet, pTabVwSh );
            //-->Modified by PengYunQuan for Validity Cell Range Picker
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//add for ScValidationDlg end

#if ENABLE_LAYOUT && !LAYOUT_SFX_TABDIALOG_BROKEN
#define SfxTabDialog layout::SfxTabDialog
#define AbstractTabDialog_Impl layout::AbstractTabDialog_Impl
#endif /* ENABLE_LAYOUT */
//add for ScSortDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScSortDlg( Window*            pParent,
                                                    const SfxItemSet* pArgSet,int nId )
{
    SfxTabDialog* pDlg=NULL;
    switch ( nId )
    {
        case RID_SCDLG_SORT :
            pDlg = new ScSortDlg( pParent, pArgSet );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
#undef SfxTabDialog
#undef AbstractTabDialog_Impl

//add for ScSortDlg end
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

void ScDPFunctionDlg_Dummy()
{
    // use ScDPListBoxWrapper to avoid warning (this isn't called)
    ListBox* pListBox = NULL;
    ScDPListBoxWrapper aWrapper( *pListBox );
}

