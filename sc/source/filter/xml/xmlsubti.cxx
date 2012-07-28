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

#include "xmlsubti.hxx"
#include "global.hxx"
#include "xmlstyli.hxx"
#include "xmlimprt.hxx"
#include "document.hxx"
#include "markdata.hxx"
#include "XMLConverter.hxx"
#include "docuno.hxx"
#include "cellsuno.hxx"
#include "XMLStylesImportHelper.hxx"
#include "sheetdata.hxx"
#include "tabprotection.hxx"
#include "tokenarray.hxx"
#include "convuno.hxx"
#include <svx/svdpage.hxx>

#include <sax/tools/converter.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlerror.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/CellInsertMode.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/sheet/XArrayFormulaRange.hpp>

#include <memory>

using ::std::auto_ptr;
using namespace com::sun::star;


ScXMLTabProtectionData::ScXMLTabProtectionData() :
    meHash1(PASSHASH_SHA1),
    meHash2(PASSHASH_UNSPECIFIED),
    mbProtected(false),
    mbSelectProtectedCells(true),
    mbSelectUnprotectedCells(true)
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

uno::Reference<sheet::XSpreadsheet> getCurrentSheet(const uno::Reference<frame::XModel>& xModel, SCTAB nSheet)
{
    uno::Reference<sheet::XSpreadsheet> xSheet;
    uno::Reference<sheet::XSpreadsheetDocument> xSpreadDoc(xModel, uno::UNO_QUERY);
    if (!xSpreadDoc.is())
        return xSheet;

    uno::Reference <sheet::XSpreadsheets> xSheets(xSpreadDoc->getSheets());
    if (!xSheets.is())
        return xSheet;

    uno::Reference <container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY);
    if (!xIndex.is())
        return xSheet;

    xSheet.set(xIndex->getByIndex(nSheet), uno::UNO_QUERY);
    return xSheet;
}

}

void ScMyTables::NewSheet(const rtl::OUString& sTableName, const rtl::OUString& sStyleName,
                          const ScXMLTabProtectionData& rProtectData)
{
    if (rImport.GetModel().is())
    {
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

        rImport.SetTableStyle(sStyleName);
        xCurrentSheet = getCurrentSheet(rImport.GetModel(), maCurrentCellPos.Tab());
        if (xCurrentSheet.is())
        {
            // We need to set the current cell range here regardless of
            // presence of style name.
            xCurrentCellRange.set(xCurrentSheet, uno::UNO_QUERY);
            SetTableStyle(sStyleName);
        }
    }
}

void ScMyTables::SetTableStyle(const rtl::OUString& sStyleName)
{
    //these uno calls are a bit difficult to remove, XMLTableStyleContext::FillPropertySet uses
    //SvXMLImportPropertyMapper::FillPropertySet
    if ( !sStyleName.isEmpty() )
    {
        // #i57869# All table style properties for all sheets are now applied here,
        // before importing the contents.
        // This is needed for the background color.
        // Sheet visibility has special handling in ScDocFunc::SetTableVisible to
        // allow hiding the first sheet.
        // RTL layout is only remembered, not actually applied, so the shapes can
        // be loaded before mirroring.

        if ( xCurrentSheet.is() )
        {
            xCurrentCellRange.set(xCurrentSheet, uno::UNO_QUERY);
            uno::Reference <beans::XPropertySet> xProperties(xCurrentSheet, uno::UNO_QUERY);
            if ( xProperties.is() )
            {
                XMLTableStylesContext *pStyles = (XMLTableStylesContext *)rImport.GetAutoStyles();
                if ( pStyles )
                {
                    XMLTableStyleContext* pStyle = (XMLTableStyleContext *)pStyles->FindStyleChildContext(
                            XML_STYLE_FAMILY_TABLE_TABLE, sStyleName, true);
                    if ( pStyle )
                    {
                        pStyle->FillPropertySet(xProperties);

                        ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rImport.GetModel())->GetSheetSaveData();
                        pSheetData->AddTableStyle( sStyleName, ScAddress( 0, 0, maCurrentCellPos.Tab() ) );
                    }
                }
            }
        }
    }
}

void ScMyTables::AddRow()
{
    maCurrentCellPos.SetRow(maCurrentCellPos.Row() + 1);
    maCurrentCellPos.SetCol(-1); //reset columns for new row
}

void ScMyTables::SetRowStyle(const rtl::OUString& rCellStyleName)
{
    rImport.GetStylesImportHelper()->SetRowStyle(rCellStyleName);
}

void ScMyTables::AddColumn(bool bIsCovered)
{
    maCurrentCellPos.SetCol( maCurrentCellPos.Col() + 1 );
    //here only need to set column style if this is the first row and
    //the cell is not covered.
    if(maCurrentCellPos.Row() == 0 && !bIsCovered)
        rImport.GetStylesImportHelper()->InsertCol(maCurrentCellPos.Col(), maCurrentCellPos.Tab(), rImport.GetDocument());
}

void ScMyTables::DeleteTable()
{
    ScXMLImport::MutexGuard aGuard(rImport);

    rImport.GetStylesImportHelper()->SetStylesToRanges();
    rImport.SetStylesToRangesFinished();

    maMatrixRangeList.RemoveAll();

    if (rImport.GetDocument() && maProtectionData.mbProtected)
    {
        uno::Sequence<sal_Int8> aHash;
        ::sax::Converter::decodeBase64(aHash, maProtectionData.maPassword);

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        auto_ptr<ScTableProtection> pProtect(new ScTableProtection);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        pProtect->setProtected(maProtectionData.mbProtected);
        pProtect->setPasswordHash(aHash, maProtectionData.meHash1, maProtectionData.meHash2);
        pProtect->setOption(ScTableProtection::SELECT_LOCKED_CELLS,   maProtectionData.mbSelectProtectedCells);
        pProtect->setOption(ScTableProtection::SELECT_UNLOCKED_CELLS, maProtectionData.mbSelectUnprotectedCells);
        rImport.GetDocument()->SetTabProtection(maCurrentCellPos.Tab(), pProtect.get());
    }
}

void ScMyTables::AddColStyle(const sal_Int32 nRepeat, const rtl::OUString& rCellStyleName)
{
    rImport.GetStylesImportHelper()->AddColumnStyle(rCellStyleName, nCurrentColCount, nRepeat);
    nCurrentColCount += nRepeat;
}

uno::Reference< drawing::XDrawPage > ScMyTables::GetCurrentXDrawPage()
{
    if( (maCurrentCellPos.Tab() != nCurrentDrawPage) || !xDrawPage.is() )
    {
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier( xCurrentSheet, uno::UNO_QUERY );
        if( xDrawPageSupplier.is() )
            xDrawPage.set(xDrawPageSupplier->getDrawPage());
        nCurrentDrawPage = sal::static_int_cast<sal_Int16>(maCurrentCellPos.Tab());
    }
    return xDrawPage;
}

uno::Reference< drawing::XShapes > ScMyTables::GetCurrentXShapes()
{
    if( (maCurrentCellPos.Tab() != nCurrentXShapes) || !xShapes.is() )
    {
        xShapes.set(GetCurrentXDrawPage(), uno::UNO_QUERY);
        rImport.GetShapeImport()->startPage(xShapes);
        rImport.GetShapeImport()->pushGroupForSorting ( xShapes );
        nCurrentXShapes = sal::static_int_cast<sal_Int16>(maCurrentCellPos.Tab());
        return xShapes;
    }
    else
        return xShapes;
}

bool ScMyTables::HasDrawPage()
{
    return !((maCurrentCellPos.Tab() != nCurrentDrawPage) || !xDrawPage.is());
}

bool ScMyTables::HasXShapes()
{
    return !((maCurrentCellPos.Tab() != nCurrentXShapes) || !xShapes.is());
}

void ScMyTables::AddOLE(uno::Reference <drawing::XShape>& rShape,
      const rtl::OUString &rRangeList)
{
      aFixupOLEs.AddOLE(rShape, rRangeList);
}

void ScMyTables::AddMatrixRange(
        const SCCOL nStartColumn, const SCROW nStartRow, const SCCOL nEndColumn, const SCROW nEndRow,
        const rtl::OUString& rFormula, const rtl::OUString& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar)
{
    OSL_ENSURE(nEndRow >= nStartRow, "wrong row order");
    OSL_ENSURE(nEndColumn >= nStartColumn, "wrong column order");
    ScRange aScRange(
        nStartColumn, nStartRow, maCurrentCellPos.Tab(),
        nEndColumn, nEndRow, maCurrentCellPos.Tab()
    );

    maMatrixRangeList.Append(aScRange);

    ScDocument* pDoc = rImport.GetDocument();
    ScMarkData aMark;
    aMark.SetMarkArea( aScRange );
    aMark.SelectTable( aScRange.aStart.Tab(), sal_True );
    ScTokenArray* pCode = new ScTokenArray;
    pCode->AddStringXML( rFormula );
    if( (eGrammar == formula::FormulaGrammar::GRAM_EXTERNAL) && !rFormulaNmsp.isEmpty() )
        pCode->AddStringXML( rFormulaNmsp );
    pDoc->InsertMatrixFormula(
        aScRange.aStart.Col(), aScRange.aStart.Row(),
        aScRange.aEnd.Col(), aScRange.aEnd.Row(),
        aMark, EMPTY_STRING, pCode, eGrammar, false );
    delete pCode;
    pDoc->IncXMLImportedFormulaCount( rFormula.getLength() );
}

bool ScMyTables::IsPartOfMatrix(const ScAddress& rScAddress) const
{
    if (!maMatrixRangeList.empty())
        return maMatrixRangeList.In(rScAddress);
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
