/*************************************************************************
 *
 *  $RCSfile: scdlgfact.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 12:45:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#include "scuiimoptdlg.hxx" //add for ScImportOptionsDlg
#include "attrdlg.hxx" //add for ScAttrDlg
#include "hfedtdlg.hxx" //add for ScHFEditDlg
#include "styledlg.hxx" //add for ScStyleDlg
#include "subtdlg.hxx" //add for ScSubTotalDlg
#include "textdlgs.hxx" //add for ScCharDlg, ScParagraphDlg
#include "validate.hxx" //add for ScValidationDlg
#include "validate.hrc" //add for ScValidationDlg
#include "sortdlg.hxx" //add for ScSortDlg
#include "opredlin.hxx" //add for  ScRedlineOptionsTabPage
#include "tpcalc.hxx" //add for ScTpCalcOptions
#include "tpprint.hxx" //add for ScTpPrintOptions
#include "tpstat.hxx" //add for ScDocStatPage
#include "tpusrlst.hxx" //add for ScTpUserLists
#include "tpview.hxx" //add for ScTpContentOptions

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
IMPL_ABSTDLG_BASE(AbstractScLinkedAreaDlg_Impl); //add for ScLinkedAreaDlg
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
IMPL_ABSTDLG_BASE(AbstractScImportOptionsDlg_Impl); //add for ScImportOptionsDlg
IMPL_ABSTDLG_BASE(AbstractTabDialog_Impl); //add for ScAttrDlg, ScHFEditDlg, ScStyleDlg, ScSubTotalDlg,ScCharDlg, ScParagraphDlg, ScValidationDlg, ScSortDlg


// AbstractTabDialog_Impl begin
void AbstractTabDialog_Impl::SetCurPageId( USHORT nId )
{
    pDlg->SetCurPageId( nId );
}

const SfxItemSet* AbstractTabDialog_Impl::GetOutputItemSet() const
{
    return pDlg->GetOutputItemSet();
}
//add by CHINA001
const USHORT* AbstractTabDialog_Impl::GetInputRanges(const SfxItemPool& pItem )
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

//add for AbstractTabDialog_Impl end
// AbstractScImportAsciiDlg_Impl begin
void AbstractScImportAsciiDlg_Impl::GetOptions( ScAsciiOptions& rOpt )
{
    pDlg->GetOptions( rOpt );
}

// AbstractScImportAsciiDlg_Impl end

//AbstractScAutoFormatDlg_Impl begin
USHORT AbstractScAutoFormatDlg_Impl::GetIndex() const
{
    return pDlg->GetIndex();
}

String AbstractScAutoFormatDlg_Impl::GetCurrFormatName()
{
    return pDlg->GetCurrFormatName();
}

//AbstractScAutoFormatDlg_Impl end



//AbstractScColRowLabelDlg_Impl begin
BOOL  AbstractScColRowLabelDlg_Impl::IsCol()
{
    return  pDlg->IsCol();
}
BOOL AbstractScColRowLabelDlg_Impl::IsRow()
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

BOOL AbstractScDataPilotSourceTypeDlg_Impl::IsDatabase() const
{
    return pDlg->IsDatabase();
}

BOOL AbstractScDataPilotSourceTypeDlg_Impl::IsExternal() const
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
USHORT   AbstractScDeleteContentsDlg_Impl::GetDelContentsCmdBits() const
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
void    AbstractScFillSeriesDlg_Impl::SetEdStartValEnabled(BOOL bFlag)
{
    pDlg->SetEdStartValEnabled(bFlag);
}
//add for AbstractScFillSeriesDlg_Impl end

//add for AbstractScGroupDlg_Impl begin
BOOL AbstractScGroupDlg_Impl::GetColsChecked() const
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
USHORT AbstractScInsertContentsDlg_Impl::GetInsContentsCmdBits() const
{
    return pDlg->GetInsContentsCmdBits();
}


USHORT  AbstractScInsertContentsDlg_Impl::GetFormulaCmdBits() const
{
    return pDlg->GetFormulaCmdBits();
}
BOOL    AbstractScInsertContentsDlg_Impl::IsSkipEmptyCells() const
{
    return pDlg->IsSkipEmptyCells();
}
BOOL    AbstractScInsertContentsDlg_Impl::IsLink() const
{
    return pDlg->IsLink();
}
void    AbstractScInsertContentsDlg_Impl::SetFillMode( BOOL bSet )
{
    pDlg->SetFillMode( bSet );
}

void    AbstractScInsertContentsDlg_Impl::SetOtherDoc( BOOL bSet )
{
    pDlg->SetOtherDoc( bSet );
}

BOOL    AbstractScInsertContentsDlg_Impl::IsTranspose() const
{
    return pDlg->IsTranspose();
}
void    AbstractScInsertContentsDlg_Impl::SetChangeTrack( BOOL bSet )
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
BOOL AbstractScInsertTableDlg_Impl::GetTablesFromFile()
{
    return pDlg->GetTablesFromFile();
}

BOOL    AbstractScInsertTableDlg_Impl::GetTablesAsLink()
{
    return pDlg->GetTablesAsLink();
}
const String*   AbstractScInsertTableDlg_Impl::GetFirstTable( USHORT* pN )
{
    return pDlg->GetFirstTable( pN );
}
ScDocShell*     AbstractScInsertTableDlg_Impl::GetDocShellTables()
{
    return pDlg->GetDocShellTables();
}
BOOL    AbstractScInsertTableDlg_Impl::IsTableBefore()
{
    return pDlg->IsTableBefore();
}
USHORT  AbstractScInsertTableDlg_Impl::GetTableCount()
{
    return pDlg->GetTableCount();
}
const String*   AbstractScInsertTableDlg_Impl::GetNextTable( USHORT* pN )
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
                                        ULONG nRefresh )
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
ULONG   AbstractScLinkedAreaDlg_Impl::GetRefresh()
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
USHORT AbstractScMoveTableDlg_Impl::GetSelectedDocument() const //add for ScMoveTableDlg
{
    return pDlg->GetSelectedDocument();
}

USHORT  AbstractScMoveTableDlg_Impl::GetSelectedTable() const
{
    return pDlg->GetSelectedTable();
}
BOOL    AbstractScMoveTableDlg_Impl::GetCopyTable() const
{
    return pDlg->GetCopyTable();
}
void    AbstractScMoveTableDlg_Impl::SetCopyTable(BOOL bFla)
{
    return pDlg->SetCopyTable( bFla );
}
void    AbstractScMoveTableDlg_Impl::EnableCopyTable(BOOL bFlag)
{
    return pDlg->EnableCopyTable( bFlag);
}
//add for AbstractScMoveTableDlg_Impl end

//add for AbstractScNameCreateDlg_Impl begin
USHORT AbstractScNameCreateDlg_Impl::GetFlags() const  //add for ScNameCreateDlg
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
USHORT AbstractScDPFunctionDlg_Impl::GetFuncMask() const //add for ScDPFunctionDlg
{
     return pDlg->GetFuncMask();
}
::com::sun::star::sheet::DataPilotFieldReference AbstractScDPFunctionDlg_Impl::GetFieldRef() const
{
    return pDlg->GetFieldRef();
}
//add for AbstractScDPFunctionDlg_Impl end

//add for AbstractScDPSubtotalDlg_Impl begin
USHORT AbstractScDPSubtotalDlg_Impl::GetFuncMask() const //add for ScDPSubtotalDlg
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
                            const Color& rColor, USHORT nFlags )
{
    pDlg->SetScenarioData(rName,rComment, rColor,nFlags);
}

void AbstractScNewScenarioDlg_Impl::GetScenarioData( String& rName, String& rComment,
                            Color& rColor, USHORT& rFlags ) const
{
    pDlg->GetScenarioData( rName,rComment,rColor,rFlags);
}

//add for AbstractScNewScenarioDlg_Impl end

//add for  AbstractScShowTabDlg_Impl begin
void AbstractScShowTabDlg_Impl::Insert( const String& rString, BOOL bSelected )  //add for ScShowTabDlg
{
    pDlg->Insert( rString, bSelected);
}

USHORT  AbstractScShowTabDlg_Impl::GetSelectEntryCount() const
{
    return pDlg->GetSelectEntryCount();
}
void    AbstractScShowTabDlg_Impl::SetDescription(
                const String& rTitle, const String& rFixedText,
                ULONG nDlgHelpId, ULONG nLbHelpId )
{
    pDlg->SetDescription( rTitle, rFixedText,nDlgHelpId, nLbHelpId );
}
USHORT  AbstractScShowTabDlg_Impl::GetSelectEntryPos(USHORT nPos) const
{
    return pDlg->GetSelectEntryPos( nPos);
}
String   AbstractScShowTabDlg_Impl::GetSelectEntry(USHORT nPos) const
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

//add for AbstractScImportOptionsDlg_Impl begin
void AbstractScImportOptionsDlg_Impl::GetImportOptions( ScImportOptions& rOptions ) const  //add for ScImportOptionsDlg
{
    pDlg->GetImportOptions(rOptions);
}
// add for AbstractScImportOptionsDlg_Impl end
// =========================Factories  for createdialog ===================

//add for ScImportAsciiDlg begin
AbstractScImportAsciiDlg * ScAbstractDialogFactory_Impl::CreateScImportAsciiDlg ( Window* pParent, String aDatName,
                                                    SvStream* pInStream, const ResId& rResId, sal_Unicode cSep )
{
    ScImportAsciiDlg* pDlg=NULL;
    switch ( rResId.GetId() )
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

//add for ScAutoFormatDlg begin

AbstractScAutoFormatDlg * ScAbstractDialogFactory_Impl::CreateScAutoFormatDlg( Window*                  pParent, //add for ScAutoFormatDlg
                                                                ScAutoFormat*               pAutoFormat,
                                                                const ScAutoFormatData*    pSelFormatData,
                                                                ScDocument*                pDoc,
                                                                const ResId& rResId)
{
    ScAutoFormatDlg* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                const ResId& rResId,
                                                                BOOL bCol ,
                                                                BOOL bRow)
{
    ScColRowLabelDlg* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                    const ResId& rResId,
                                                    BOOL                bColDefault)
{
    Dialog * pDlg=NULL;
    switch ( rResId.GetId() )
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


//add for ScDataPilotDatabaseDlg begin

AbstractScDataPilotDatabaseDlg *  ScAbstractDialogFactory_Impl::CreateScDataPilotDatabaseDlg (Window* pParent ,
                                                                                              const ResId& rResId )  //add for ScDataPilotDatabaseDlg
{
    ScDataPilotDatabaseDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                                               BOOL bEnableExternal,
                                                                                               const ResId& rResId )
{
    ScDataPilotSourceTypeDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            const ResId& rResId )
{
    ScDataPilotServiceDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
AbstractScDeleteCellDlg* ScAbstractDialogFactory_Impl::CreateScDeleteCellDlg( Window* pParent, const ResId& rResId,
                                                                             BOOL bDisallowCellMove )
{
    ScDeleteCellDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
AbstractScDeleteContentsDlg* ScAbstractDialogFactory_Impl::CreateScDeleteContentsDlg(Window* pParent,const ResId& rResId, //add for ScDeleteContentsDlg
                                                                 USHORT  nCheckDefaults )
{
    ScDeleteContentsDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            USHORT          nPossDir,
                                                            const ResId& rResId)
{
    ScFillSeriesDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            USHORT  nResId,
                                                            const ResId& rResId,
                                                            BOOL    bUnGroup ,
                                                            BOOL    bRows   )
{
    ScGroupDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                const ResId& rResId,
                                                            BOOL bDisallowCellMove )
{
    ScInsertCellDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                                    const ResId& rResId,
                                                                                    USHORT          nCheckDefaults,
                                                                                    const String*   pStrTitle )
{
    ScInsertContentsDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                SCTAB nTabCount, bool bFromFile, const ResId& rResId)
{
    ScInsertTableDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            USHORT  nResId,
                                                        const String& aTitle,
                                                        const String& aLbTitle,
                                                                List&   aEntryList,
                                                            const ResId& rResId )
{
    ScSelEntryDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                const ResId& rResId)
{
    ScLinkedAreaDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                USHORT      nResId,     // Ableitung fuer jeden Dialog!
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                const ResId& rResId ,
                                                                FieldUnit       eFUnit,
                                                                USHORT      nDecimals,
                                                                long            nMaximum ,
                                                                long            nMinimum,
                                                                long            nFirst,
                                                                long          nLast )
{
    ScMetricInputDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
AbstractScMoveTableDlg * ScAbstractDialogFactory_Impl::CreateScMoveTableDlg(  Window* pParent, const ResId& rResId )
{
    ScMoveTableDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
AbstractScNameCreateDlg * ScAbstractDialogFactory_Impl::CreateScNameCreateDlg ( Window * pParent, USHORT nFlags, const ResId& rResId )
{
    ScNameCreateDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                            const ResId& rResId , BOOL bInsList )
{
    ScNamePasteDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                const SfxItemSet&   rArgSet, USHORT nSourceTab , const ResId& rResId )
{
    ScPivotFilterDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                const ResId& rResId,
                                                                const ScDPLabelDataVec& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData )
{
    ScDPFunctionDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                const ResId& rResId,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout )
{
    ScDPSubtotalDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
        Window* pParent, const ResId& rResId, const ScDPNumGroupInfo& rInfo )
{
    if( rResId.GetId() == RID_SCDLG_DPNUMGROUP )
        return new AbstractScDPNumGroupDlg_Impl( new ScDPNumGroupDlg( pParent, rInfo ) );
    return 0;
}

AbstractScDPDateGroupDlg * ScAbstractDialogFactory_Impl::CreateScDPDateGroupDlg(
        Window* pParent, const ResId& rResId,
        const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart, const Date& rNullDate )
{
    if( rResId.GetId() == RID_SCDLG_DPDATEGROUP )
        return new AbstractScDPDateGroupDlg_Impl( new ScDPDateGroupDlg( pParent, rInfo, nDatePart, rNullDate ) );
    return 0;
}

//add for ScDPShowDetailDlg begin
AbstractScDPShowDetailDlg * ScAbstractDialogFactory_Impl::CreateScDPShowDetailDlg (
        Window* pParent, const ResId& rResId, ScDPObject& rDPObj, USHORT nOrient )
{
    if( rResId.GetId() == RID_SCDLG_DPSHOWDETAIL )
        return new AbstractScDPShowDetailDlg_Impl( new ScDPShowDetailDlg( pParent, rDPObj, nOrient ) );
    return 0;
}
//add for ScDPShowDetailDlg end

//add for ScNewScenarioDlg begin
AbstractScNewScenarioDlg * ScAbstractDialogFactory_Impl::CreateScNewScenarioDlg ( Window* pParent, const String& rName,
                                                                const ResId& rResId,
                                                                BOOL bEdit , BOOL bSheetProtected )
{
    ScNewScenarioDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
AbstractScShowTabDlg * ScAbstractDialogFactory_Impl::CreateScShowTabDlg ( Window* pParent, const ResId& rResId )
{
    ScShowTabDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                ULONG nHelpId ,
                                                                const ResId& rResId )
{
    ScStringInputDlg * pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case RID_SCDLG_STRINPUT :
            pDlg = new ScStringInputDlg( pParent, rTitle, rEditTitle,rDefault, nHelpId );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractScStringInputDlg_Impl( pDlg );
    return 0;
}
 //add for ScStringInputDlg end

//add for ScImportOptionsDlg begin
AbstractScImportOptionsDlg * ScAbstractDialogFactory_Impl::CreateScImportOptionsDlg ( Window*               pParent,
                                                                    const ResId& rResId,
                                                                    BOOL                    bAscii,
                                                                    const ScImportOptions*  pOptions,
                                                                    const String*           pStrTitle,
                                                                    BOOL                    bMultiByte,
                                                                    BOOL                    bOnlyDbtoolsEncodings,
                                                                    BOOL                    bImport )
{
    ScImportOptionsDlg * pDlg=NULL;
    switch ( rResId.GetId() )
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

//add for ScAttrDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScAttrDlg( SfxViewFrame*  pFrame,
                                                                        Window*          pParent,
                                                                        const SfxItemSet* pCellAttrs,
                                                                        const ResId& rResId)
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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

//add for ScHFEditDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScHFEditDlg( SfxViewFrame*       pFrame,
                                                                        Window*         pParent,
                                                                        const SfxItemSet&   rCoreSet,
                                                                        const String&       rPageStyle,
                                                                        const ResId& rResId,
                                                                        USHORT              nResId )
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                        USHORT              nRscId,
                                                                        const ResId& rResId)
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                        const ResId& rResId)
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                    const SfxObjectShell* pDocShell, const ResId& rResId )
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                                            const ResId& rResId )
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
                                                        const SfxItemSet* pArgSet,const ResId& rResId  )
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
    {
        case TAB_DLG_VALIDATION :
            pDlg = new ScValidationDlg( pParent, pArgSet );
            break;
        default:
            break;
    }

    if ( pDlg )
        return new AbstractTabDialog_Impl( pDlg );
    return 0;
}
//add for ScValidationDlg end

//add for ScSortDlg begin
SfxAbstractTabDialog * ScAbstractDialogFactory_Impl::CreateScSortDlg( Window*            pParent,
                                                    const SfxItemSet* pArgSet,const ResId& rResId )
{
    SfxTabDialog* pDlg=NULL;
    switch ( rResId.GetId() )
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
//add for ScSortDlg end
//------------------ Factories for TabPages--------------------
CreateTabPage ScAbstractDialogFactory_Impl::GetTabPageCreatorFunc( USHORT nId )
{
    switch ( nId )
    {
        case RID_SCPAGE_OPREDLINE :
            return ScRedlineOptionsTabPage::Create;
            break;
        case    RID_SCPAGE_CALC :
            return ScTpCalcOptions::Create;
            break;
        case    RID_SCPAGE_PRINT :
            return ScTpPrintOptions::Create;
            break;
        case    RID_SCPAGE_STAT :
            return ScDocStatPage::Create;
            break;
        case RID_SCPAGE_USERLISTS :
             return ScTpUserLists::Create;
             break;
        case RID_SCPAGE_CONTENT :
            return ScTpContentOptions::Create;
             break;
        case RID_SCPAGE_LAYOUT :
            return ScTpLayoutOptions::Create;
            break;

        default:
            break;
    }

    return 0;
}

GetTabPageRanges ScAbstractDialogFactory_Impl::GetTabPageRangesFunc( USHORT nId )
{
    switch ( nId )
    {
    case TP_VALIDATION_VALUES :
            return ScTPValidationValue::GetRanges;
            break;
        default:
            break;
    }

    return 0;
}
