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
#include "global.hxx"
#include "xmlstyli.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "olinetab.hxx"
#include "sheetdata.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>

#include <com/sun/star/table/CellAddress.hpp>

#define SC_ISVISIBLE "IsVisible"
#define SC_ISFILTERED "IsFiltered"

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableRowContext::ScXMLTableRowContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sVisibility(GetXMLToken(XML_VISIBLE)),
    nRepeatedRows(1),
    bHasCell(false)
{
    OUString sCellStyleName;
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetTableRowAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_ROW_ATTR_STYLE_NAME:
            {
                sStyleName = sValue;
            }
            break;
            case XML_TOK_TABLE_ROW_ATTR_VISIBILITY:
            {
                sVisibility = sValue;
            }
            break;
            case XML_TOK_TABLE_ROW_ATTR_REPEATED:
            {
                nRepeatedRows = std::max( sValue.toInt32(), (sal_Int32) 1 );
                nRepeatedRows = std::min( nRepeatedRows, MAXROWCOUNT );
            }
            break;
            case XML_TOK_TABLE_ROW_ATTR_DEFAULT_CELL_STYLE_NAME:
            {
                sCellStyleName = sValue;
            }
            break;
            /*case XML_TOK_TABLE_ROW_ATTR_USE_OPTIMAL_HEIGHT:
            {
                sOptimalHeight = sValue;
            }
            break;*/
        }
    }
    GetScImport().GetTables().AddRow();
    GetScImport().GetTables().SetRowStyle(sCellStyleName);
}

ScXMLTableRowContext::~ScXMLTableRowContext()
{
}

SvXMLImportContext *ScXMLTableRowContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetTableRowElemTokenMap());
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROW_CELL:
//      if( IsInsertCellPossible() )
        {
            bHasCell = true;
            pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList, false, static_cast<SCROW>(nRepeatedRows)
                                                      //this
                                                      );
        }
        break;
    case XML_TOK_TABLE_ROW_COVERED_CELL:
//      if( IsInsertCellPossible() )
        {
            bHasCell = true;
            pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList, true, static_cast<SCROW>(nRepeatedRows)
                                                      //this
                                                      );
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableRowContext::EndElement()
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
                                    pSheetData->AddRowStyle( sStyleName, ScAddress( 0, (SCROW)nFirstRow, nSheet ) );
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
                        xRowProperties->setPropertyValue(SC_ISVISIBLE, uno::makeAny(bVisible));
                    if (bFiltered)
                        xRowProperties->setPropertyValue(SC_ISFILTERED, uno::makeAny(bFiltered));
                }
            }
        }
    }
}

ScXMLTableRowsContext::ScXMLTableRowsContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      const bool bTempHeader,
                                      const bool bTempGroup ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
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
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const OUString& sAttrName(xAttrList->getNameByIndex( i ));
            OUString aLocalName;
            sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName ));
            const OUString& sValue(xAttrList->getValueByIndex( i ));

            if ((nPrefix == XML_NAMESPACE_TABLE) && IsXMLToken(aLocalName, XML_DISPLAY))
                bGroupDisplay = IsXMLToken(sValue, XML_TRUE);
        }
    }
}

ScXMLTableRowsContext::~ScXMLTableRowsContext()
{
}

SvXMLImportContext *ScXMLTableRowsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetTableRowsElemTokenMap());
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROWS_ROW_GROUP:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   false, true );
        break;
    case XML_TOK_TABLE_ROWS_HEADER_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   true, false );
        break;
    case XML_TOK_TABLE_ROWS_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   false, false );
        break;
    case XML_TOK_TABLE_ROWS_ROW:
            pContext = new ScXMLTableRowContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList//,
                                                      //this
                                                      );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableRowsContext::EndElement()
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
                rRowArray.Insert(static_cast<SCROW>(nGroupStartRow), static_cast<SCROW>(nGroupEndRow), bResized, !bGroupDisplay);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
