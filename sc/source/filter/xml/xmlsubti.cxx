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

#include "xmlsubti.hxx"
#include "xmlstyli.hxx"
#include "xmlimprt.hxx"
#include <document.hxx>
#include "XMLConverter.hxx"
#include <cellsuno.hxx>
#include <docuno.hxx>
#include "XMLStylesImportHelper.hxx"
#include <sheetdata.hxx>
#include <tabprotection.hxx>
#include <tokenarray.hxx>
#include <documentimport.hxx>

#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <comphelper/base64.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>

using namespace com::sun::star;

ScXMLTabProtectionData::ScXMLTabProtectionData() :
    meHash1(PASSHASH_SHA1),
    meHash2(PASSHASH_UNSPECIFIED),
    mbProtected(false),
    mbSelectProtectedCells(true),
    mbSelectUnprotectedCells(true),
    mbInsertColumns(false),
    mbInsertRows(false),
    mbDeleteColumns(false),
    mbDeleteRows(false),
    mbUseAutoFilter(false),
    mbUsePivot(false)
{
}

ScMyTables::ScMyTables(ScXMLImport& rTempImport)
    : rImport(rTempImport),
    aFixupOLEs(rTempImport),
    maCurrentCellPos(ScAddress::INITIALIZE_INVALID),
    nCurrentColCount(0),
    nCurrentDrawPage( -1 ),
    nCurrentXShapes( -1 )
{
}

ScMyTables::~ScMyTables()
{
}

namespace {

rtl::Reference<ScTableSheetObj> getCurrentSheet(const uno::Reference<frame::XModel>& xModel, SCTAB nSheet)
{
    rtl::Reference<ScTableSheetObj> xSheet;
    ScModelObj* pSpreadDoc = dynamic_cast<ScModelObj*>(xModel.get());
    if (!pSpreadDoc)
        return xSheet;

    rtl::Reference<ScTableSheetsObj> xSheets(pSpreadDoc->getScSheets());
    if (!xSheets.is())
        return xSheet;

    return xSheets->GetSheetByIndex(nSheet);
}

}

void ScMyTables::NewSheet(const OUString& sTableName, const OUString& sStyleName,
                          const ScXMLTabProtectionData& rProtectData)
{
    if (!rImport.GetModel().is())
        return;

    nCurrentColCount = 0;
    sCurrentSheetName = sTableName;
    //reset cols and rows for new sheet, but increment tab
    maCurrentCellPos.SetCol(-1);
    maCurrentCellPos.SetRow(-1);
    maCurrentCellPos.SetTab(maCurrentCellPos.Tab() + 1);

    maProtectionData = rProtectData;
    ScDocument *pDoc = ScXMLConverter::GetScDocument(rImport.GetModel());

    // The document contains one sheet when created. So for the first
    // sheet, we only need to set its name.
    if (maCurrentCellPos.Tab() > 0)
        pDoc->AppendTabOnLoad(sTableName);
    else
        pDoc->SetTabNameOnLoad(maCurrentCellPos.Tab(), sTableName);

    xCurrentSheet = getCurrentSheet(rImport.GetModel(), maCurrentCellPos.Tab());
    if (xCurrentSheet.is())
    {
        // We need to set the current cell range here regardless of
        // presence of style name.
        SetTableStyle(sStyleName);
    }
}

void ScMyTables::SetTableStyle(const OUString& sStyleName)
{
    //these uno calls are a bit difficult to remove, XMLTableStyleContext::FillPropertySet uses
    //SvXMLImportPropertyMapper::FillPropertySet
    if ( sStyleName.isEmpty() )
        return;

    // #i57869# All table style properties for all sheets are now applied here,
    // before importing the contents.
    // This is needed for the background color.
    // Sheet visibility has special handling in ScDocFunc::SetTableVisible to
    // allow hiding the first sheet.
    // RTL layout is only remembered, not actually applied, so the shapes can
    // be loaded before mirroring.

    if ( !xCurrentSheet.is() )
        return;

    XMLTableStylesContext *pStyles = static_cast<XMLTableStylesContext *>(rImport.GetAutoStyles());
    if ( pStyles )
    {
        XMLTableStyleContext* pStyle = const_cast<XMLTableStyleContext*>(static_cast<const XMLTableStyleContext *>(pStyles->FindStyleChildContext(
                XmlStyleFamily::TABLE_TABLE, sStyleName, true)));
        if ( pStyle )
        {
            pStyle->FillPropertySet(xCurrentSheet);

            ScSheetSaveData* pSheetData = rImport.GetScModel()->GetSheetSaveData();
            pSheetData->AddTableStyle( sStyleName, ScAddress( 0, 0, maCurrentCellPos.Tab() ) );
        }
    }
}

void ScMyTables::AddRow()
{
    maCurrentCellPos.SetRow(maCurrentCellPos.Row() + 1);
    maCurrentCellPos.SetCol(-1); //reset columns for new row
}

void ScMyTables::SetRowStyle(const OUString& rCellStyleName)
{
    rImport.GetStylesImportHelper()->SetRowStyle(rCellStyleName);
}

void ScMyTables::AddColumn(bool bIsCovered)
{
    maCurrentCellPos.SetCol( maCurrentCellPos.Col() + 1 );
    //here only need to set column style if this is the first row and
    //the cell is not covered.
    if(maCurrentCellPos.Row() == 0 && !bIsCovered)
        rImport.GetStylesImportHelper()->InsertCol(maCurrentCellPos.Col(), maCurrentCellPos.Tab());
}

void ScMyTables::AddColumns(sal_Int32 nRepeat)
{
    maCurrentCellPos.SetCol( maCurrentCellPos.Col() + nRepeat );
}

void ScMyTables::DeleteTable()
{
    ScXMLImport::MutexGuard aGuard(rImport);

    rImport.GetStylesImportHelper()->SetStylesToRanges();
    rImport.SetStylesToRangesFinished();

    maMatrixRangeList.RemoveAll();

    if (!(rImport.GetDocument() && maProtectionData.mbProtected))
        return;

    uno::Sequence<sal_Int8> aHash;
    ::comphelper::Base64::decode(aHash, maProtectionData.maPassword);

    ScTableProtection aProtect;
    aProtect.setProtected(maProtectionData.mbProtected);
    aProtect.setPasswordHash(aHash, maProtectionData.meHash1, maProtectionData.meHash2);
    aProtect.setOption(ScTableProtection::SELECT_LOCKED_CELLS,   maProtectionData.mbSelectProtectedCells);
    aProtect.setOption(ScTableProtection::SELECT_UNLOCKED_CELLS, maProtectionData.mbSelectUnprotectedCells);
    aProtect.setOption(ScTableProtection::INSERT_COLUMNS, maProtectionData.mbInsertColumns);
    aProtect.setOption(ScTableProtection::INSERT_ROWS,    maProtectionData.mbInsertRows);
    aProtect.setOption(ScTableProtection::DELETE_COLUMNS, maProtectionData.mbDeleteColumns);
    aProtect.setOption(ScTableProtection::DELETE_ROWS,    maProtectionData.mbDeleteRows);
    aProtect.setOption(ScTableProtection::AUTOFILTER,     maProtectionData.mbUseAutoFilter);
    aProtect.setOption(ScTableProtection::PIVOT_TABLES,   maProtectionData.mbUsePivot);
    rImport.GetDocument()->SetTabProtection(maCurrentCellPos.Tab(), &aProtect);
}

void ScMyTables::AddColStyle(const sal_Int32 nRepeat, const OUString& rCellStyleName)
{
    rImport.GetStylesImportHelper()->AddColumnStyle(rCellStyleName, nCurrentColCount, nRepeat);
    nCurrentColCount += nRepeat;
    if (ScDocument* pDoc = rImport.GetDocument())
    {
        SAL_WARN_IF(nCurrentColCount > pDoc->GetSheetLimits().GetMaxColCount(),
            "sc", "more columns than fit into SCCOL");
        nCurrentColCount = std::min<sal_Int32>( nCurrentColCount, pDoc->GetSheetLimits().GetMaxColCount() );
    }
}

uno::Reference< drawing::XDrawPage > const & ScMyTables::GetCurrentXDrawPage()
{
    if( (maCurrentCellPos.Tab() != nCurrentDrawPage) || !xDrawPage.is() )
    {
        if( xCurrentSheet.is() )
            xDrawPage.set(xCurrentSheet->getDrawPage());
        nCurrentDrawPage = sal::static_int_cast<sal_Int16>(maCurrentCellPos.Tab());
    }
    return xDrawPage;
}

uno::Reference< drawing::XShapes > const & ScMyTables::GetCurrentXShapes()
{
    if( (maCurrentCellPos.Tab() != nCurrentXShapes) || !xShapes.is() )
    {
        xShapes = GetCurrentXDrawPage();
        rImport.GetShapeImport()->startPage(xShapes);
        rImport.GetShapeImport()->pushGroupForPostProcessing ( xShapes );
        nCurrentXShapes = sal::static_int_cast<sal_Int16>(maCurrentCellPos.Tab());
    }
    return xShapes;
}

bool ScMyTables::HasDrawPage() const
{
    return (maCurrentCellPos.Tab() == nCurrentDrawPage) && xDrawPage.is();
}

bool ScMyTables::HasXShapes() const
{
    return (maCurrentCellPos.Tab() == nCurrentXShapes) && xShapes.is();
}

void ScMyTables::AddOLE(const uno::Reference <drawing::XShape>& rShape,
      const OUString &rRangeList)
{
      aFixupOLEs.AddOLE(rShape, rRangeList);
}

void ScMyTables::AddMatrixRange(
        const SCCOL nStartColumn, const SCROW nStartRow, const SCCOL nEndColumn, const SCROW nEndRow,
        const OUString& rFormula, const OUString& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar)
{
    OSL_ENSURE(nEndRow >= nStartRow, "wrong row order");
    OSL_ENSURE(nEndColumn >= nStartColumn, "wrong column order");
    ScRange aScRange(
        nStartColumn, nStartRow, maCurrentCellPos.Tab(),
        nEndColumn, nEndRow, maCurrentCellPos.Tab()
    );

    maMatrixRangeList.push_back(aScRange);

    ScDocumentImport& rDoc = rImport.GetDoc();
    ScTokenArray aCode(rDoc.getDoc());
    aCode.AssignXMLString( rFormula,
            ((eGrammar == formula::FormulaGrammar::GRAM_EXTERNAL) ? rFormulaNmsp : OUString()));
    rDoc.setMatrixCells(aScRange, aCode, eGrammar);
    rDoc.getDoc().IncXMLImportedFormulaCount( rFormula.getLength() );
}

bool ScMyTables::IsPartOfMatrix(const ScAddress& rScAddress) const
{
    if (!maMatrixRangeList.empty())
        return maMatrixRangeList.Contains(ScRange(rScAddress));
    return false;
}

SCCOL ScMyTables::GetCurrentColCount() const
{
    ScDocument* pDoc = rImport.GetDocument();
    if (!pDoc)
        return nCurrentColCount;
    return std::min<sal_Int32>(nCurrentColCount, pDoc->MaxCol());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
