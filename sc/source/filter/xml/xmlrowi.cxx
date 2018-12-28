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

#include "xmlrowi.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#include "xmlstyli.hxx"
#include <document.hxx>
#include <docuno.hxx>
#include <olinetab.hxx>
#include <sheetdata.hxx>
#include <documentimport.hxx>
#include <unonames.hxx>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <sax/fastattribs.hxx>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <osl/diagnose.h>

#define SC_ISFILTERED "IsFiltered"

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableRowContext::ScXMLTableRowContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport ),
    sVisibility(GetXMLToken(XML_VISIBLE)),
    nRepeatedRows(1),
    bHasCell(false)
{
    OUString sCellStyleName;
    if ( rAttrList.is() )
    {
        for (auto &it : *rAttrList)
        {
            switch (it.getToken())
            {
                case XML_ELEMENT( TABLE, XML_STYLE_NAME ):
                {
                    sStyleName = it.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_VISIBILITY ):
                {
                    sVisibility = it.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_NUMBER_ROWS_REPEATED ):
                {
                    nRepeatedRows = std::max( it.toInt32(), sal_Int32(1) );
                    nRepeatedRows = std::min( nRepeatedRows, MAXROWCOUNT );
                }
                break;
                case XML_ELEMENT( TABLE, XML_DEFAULT_CELL_STYLE_NAME ):
                {
                    sCellStyleName = it.toString();
                }
                break;
                /*case XML_ELEMENT( TABLE, XML_USE_OPTIMAL_HEIGHT ):
                {
                    sOptimalHeight = it.toString();
                }
                break;*/
            }
        }
    }

    GetScImport().GetTables().AddRow();
    GetScImport().GetTables().SetRowStyle(sCellStyleName);
}

ScXMLTableRowContext::~ScXMLTableRowContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
        ScXMLTableRowContext::createFastChildContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch( nElement )
    {
    case XML_ELEMENT( TABLE, XML_TABLE_CELL ):
//      if( IsInsertCellPossible() )
        {
            bHasCell = true;
            pContext = new ScXMLTableRowCellContext( GetScImport(),
                                                       pAttribList, false, static_cast<SCROW>(nRepeatedRows)
                                                      //this
                                                      );
        }
        break;
    case XML_ELEMENT( TABLE, XML_COVERED_TABLE_CELL ):
//      if( IsInsertCellPossible() )
        {
            bHasCell = true;
            pContext = new ScXMLTableRowCellContext( GetScImport(),
                                                      pAttribList, true, static_cast<SCROW>(nRepeatedRows)
                                                      //this
                                                      );
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLTableRowContext::endFastElement(sal_Int32 /*nElement*/)
{
    ScXMLImport& rXMLImport(GetScImport());
    if (!bHasCell && nRepeatedRows > 1)
    {
        for (sal_Int32 i = 0; i < nRepeatedRows - 1; ++i) //one row is always added
            GetScImport().GetTables().AddRow();
        OSL_FAIL("it seems here is a nonvalid file; possible missing of table:table-cell element");
    }
    SCTAB nSheet = rXMLImport.GetTables().GetCurrentSheet();
    sal_Int32 nCurrentRow(rXMLImport.GetTables().GetCurrentRow());
    uno::Reference<sheet::XSpreadsheet> xSheet(rXMLImport.GetTables().GetCurrentXSheet());
    if(xSheet.is())
    {
        sal_Int32 nFirstRow(nCurrentRow - nRepeatedRows + 1);
        if (nFirstRow > MAXROW)
            nFirstRow = MAXROW;
        if (nCurrentRow > MAXROW)
            nCurrentRow = MAXROW;
        uno::Reference <table::XCellRange> xCellRange(xSheet->getCellRangeByPosition(0, nFirstRow, 0, nCurrentRow));
        if (xCellRange.is())
        {
            uno::Reference<table::XColumnRowRange> xColumnRowRange (xCellRange, uno::UNO_QUERY);
            if (xColumnRowRange.is())
            {
                uno::Reference <beans::XPropertySet> xRowProperties(xColumnRowRange->getRows(), uno::UNO_QUERY);
                if (xRowProperties.is())
                {
                    if (!sStyleName.isEmpty())
                    {
                        XMLTableStylesContext *pStyles(static_cast<XMLTableStylesContext *>(rXMLImport.GetAutoStyles()));
                        if ( pStyles )
                        {
                            XMLTableStyleContext* pStyle(const_cast<XMLTableStyleContext*>(static_cast<const XMLTableStyleContext *>(pStyles->FindStyleChildContext(
                                XML_STYLE_FAMILY_TABLE_ROW, sStyleName, true))));
                            if (pStyle)
                            {
                                pStyle->FillPropertySet(xRowProperties);

                                if ( nSheet != pStyle->GetLastSheet() )
                                {
                                    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rXMLImport.GetModel())->GetSheetSaveData();
                                    pSheetData->AddRowStyle( sStyleName, ScAddress( 0, static_cast<SCROW>(nFirstRow), nSheet ) );
                                    pStyle->SetLastSheet(nSheet);
                                }
                            }
                        }
                    }
                    bool bVisible (true);
                    bool bFiltered (false);
                    if (IsXMLToken(sVisibility, XML_COLLAPSE))
                    {
                        bVisible = false;
                    }
                    else if (IsXMLToken(sVisibility, XML_FILTER))
                    {
                        bVisible = false;
                        bFiltered = true;
                    }
                    if (!bVisible)
                    {
                        rXMLImport.GetDoc().setRowsVisible(nSheet, nFirstRow, nCurrentRow, false);
                    }
                    if (bFiltered)
                        xRowProperties->setPropertyValue(SC_ISFILTERED, uno::makeAny(bFiltered));

                    uno::Any any = xRowProperties->getPropertyValue(SC_UNONAME_OHEIGHT);
                    bool bOptionalHeight = false;
                    any >>= bOptionalHeight;
                    if (bOptionalHeight)
                    {
                        // Save this row for later height update
                        std::vector<ScDocRowHeightUpdater::TabRanges>& rRecalcRanges = rXMLImport.GetRecalcRowRanges();
                        while (static_cast<SCTAB>(rRecalcRanges.size()) <= nSheet)
                        {
                            rRecalcRanges.emplace_back(0);
                        }
                        rRecalcRanges.at(nSheet).mnTab = nSheet;
                        rRecalcRanges.at(nSheet).mpRanges->setTrue(nFirstRow, nCurrentRow);
                    }
                }
            }
        }
    }
}

ScXMLTableRowsContext::ScXMLTableRowsContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      const bool bTempHeader,
                                      const bool bTempGroup ) :
    ScXMLImportContext( rImport ),
    nHeaderStartRow(0),
    nGroupStartRow(0),
    bHeader(bTempHeader),
    bGroup(bTempGroup),
    bGroupDisplay(true)
{
    // don't have any attributes
    if (bHeader)
    {
        ScAddress aAddr = rImport.GetTables().GetCurrentCellPos();
        nHeaderStartRow = aAddr.Row();
        ++nHeaderStartRow;
    }
    else if (bGroup)
    {
        nGroupStartRow = rImport.GetTables().GetCurrentRow();
        ++nGroupStartRow;
        if ( rAttrList.is() )
        {
            auto &aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_DISPLAY ) ) );
            if (aIter != rAttrList->end())
                bGroupDisplay = IsXMLToken( aIter, XML_TRUE );
        }
    }
}

ScXMLTableRowsContext::~ScXMLTableRowsContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
        ScXMLTableRowsContext::createFastChildContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch( nElement )
    {
    case XML_ELEMENT( TABLE, XML_TABLE_ROW_GROUP ):
        pContext = new ScXMLTableRowsContext( GetScImport(), pAttribList,
                                                   false, true );
        break;
    case XML_ELEMENT( TABLE, XML_TABLE_HEADER_ROWS ):
        pContext = new ScXMLTableRowsContext( GetScImport(), pAttribList,
                                                   true, false );
        break;
    case XML_ELEMENT( TABLE, XML_TABLE_ROWS ):
        pContext = new ScXMLTableRowsContext( GetScImport(), pAttribList,
                                                   false, false );
        break;
    case XML_ELEMENT( TABLE, XML_TABLE_ROW ):
        pContext = new ScXMLTableRowContext( GetScImport(), pAttribList );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLTableRowsContext::endFastElement(sal_Int32 /*nElement*/)
{
    ScXMLImport& rXMLImport(GetScImport());
    if (bHeader)
    {
        SCROW nHeaderEndRow = rXMLImport.GetTables().GetCurrentRow();
        if (nHeaderStartRow <= nHeaderEndRow)
        {
            uno::Reference <sheet::XPrintAreas> xPrintAreas (rXMLImport.GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
            if (xPrintAreas.is())
            {
                if (!xPrintAreas->getPrintTitleRows())
                {
                    xPrintAreas->setPrintTitleRows(true);
                    table::CellRangeAddress aRowHeaderRange;
                    aRowHeaderRange.StartRow = nHeaderStartRow;
                    aRowHeaderRange.EndRow = nHeaderEndRow;
                    xPrintAreas->setTitleRows(aRowHeaderRange);
                }
                else
                {
                    table::CellRangeAddress aRowHeaderRange(xPrintAreas->getTitleRows());
                    aRowHeaderRange.EndRow = nHeaderEndRow;
                    xPrintAreas->setTitleRows(aRowHeaderRange);
                }
            }
        }
    }
    else if (bGroup)
    {
        SCROW nGroupEndRow = rXMLImport.GetTables().GetCurrentRow();
        SCTAB nSheet(rXMLImport.GetTables().GetCurrentSheet());
        if (nGroupStartRow <= nGroupEndRow)
        {
            ScDocument* pDoc(GetScImport().GetDocument());
            if (pDoc)
            {
                ScXMLImport::MutexGuard aGuard(GetScImport());
                ScOutlineTable* pOutlineTable(pDoc->GetOutlineTable(nSheet, true));
                ScOutlineArray& rRowArray(pOutlineTable->GetRowArray());
                bool bResized;
                rRowArray.Insert(nGroupStartRow, nGroupEndRow, bResized, !bGroupDisplay);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
