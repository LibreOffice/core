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

#include "xmlcoli.hxx"
#include "xmlimprt.hxx"
#include "xmlstyli.hxx"
#include <document.hxx>
#include <docuno.hxx>
#include <olinetab.hxx>
#include <sheetdata.hxx>
#include <unonames.hxx>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableColContext::ScXMLTableColContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport ),
    sVisibility(GetXMLToken(XML_VISIBLE))
{
    nColCount = 1;
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_NUMBER_COLUMNS_REPEATED ):
                {
                    nColCount = std::max<sal_Int32>(aIter.toInt32(), 1);
                    nColCount = std::min<sal_Int32>(nColCount, MAXCOLCOUNT);
                }
                break;
            case XML_ELEMENT( TABLE, XML_STYLE_NAME ):
                {
                    sStyleName = aIter.toString();
                }
                break;
            case XML_ELEMENT( TABLE, XML_VISIBILITY ):
                {
                    sVisibility = aIter.toString();
                }
                break;
            case XML_ELEMENT( TABLE, XML_DEFAULT_CELL_STYLE_NAME ):
                {
                    sCellStyleName = aIter.toString();
                }
                break;
            }
        }
    }
}

ScXMLTableColContext::~ScXMLTableColContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLTableColContext::createFastChildContext(
    sal_Int32 /*nElement*/, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport() );
}

void SAL_CALL ScXMLTableColContext::endFastElement( sal_Int32 /*nElement*/ )
{
    ScXMLImport& rXMLImport = GetScImport();
    SCTAB nSheet = rXMLImport.GetTables().GetCurrentSheet();
    sal_Int32 nCurrentColumn = rXMLImport.GetTables().GetCurrentColCount();
    uno::Reference<sheet::XSpreadsheet> xSheet(rXMLImport.GetTables().GetCurrentXSheet());
    if(xSheet.is())
    {
        sal_Int32 nLastColumn(nCurrentColumn + nColCount - 1);
        if (nLastColumn > MAXCOL)
            nLastColumn = MAXCOL;
        if (nCurrentColumn > MAXCOL)
            nCurrentColumn = MAXCOL;
        uno::Reference<table::XColumnRowRange> xColumnRowRange (xSheet->getCellRangeByPosition(nCurrentColumn, 0, nLastColumn, 0), uno::UNO_QUERY);
        if (xColumnRowRange.is())
        {
            uno::Reference <beans::XPropertySet> xColumnProperties(xColumnRowRange->getColumns(), uno::UNO_QUERY);
            if (xColumnProperties.is())
            {
                if (!sStyleName.isEmpty())
                {
                    XMLTableStylesContext *pStyles = static_cast<XMLTableStylesContext *>(rXMLImport.GetAutoStyles());
                    if ( pStyles )
                    {
                        XMLTableStyleContext* pStyle = const_cast<XMLTableStyleContext*>(static_cast<const XMLTableStyleContext *>(pStyles->FindStyleChildContext(
                            XML_STYLE_FAMILY_TABLE_COLUMN, sStyleName, true)));
                        if (pStyle)
                        {
                            pStyle->FillPropertySet(xColumnProperties);

                            if ( nSheet != pStyle->GetLastSheet() )
                            {
                                ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rXMLImport.GetModel())->GetSheetSaveData();
                                pSheetData->AddColumnStyle( sStyleName, ScAddress( static_cast<SCCOL>(nCurrentColumn), 0, nSheet ) );
                                pStyle->SetLastSheet(nSheet);
                            }
                        }
                    }
                }
                bool bValue(true);
                if (!IsXMLToken(sVisibility, XML_VISIBLE))
                    bValue = false;
                xColumnProperties->setPropertyValue(SC_UNONAME_CELLVIS, uno::makeAny(bValue));
            }
        }
    }

    // #i57915# ScXMLImport::SetStyleToRange can't handle empty style names.
    // The default for a column if there is no attribute is the style "Default" (programmatic API name).
    if ( sCellStyleName.isEmpty() )
        sCellStyleName = "Default";

    GetScImport().GetTables().AddColStyle(nColCount, sCellStyleName);
}

ScXMLTableColsContext::ScXMLTableColsContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      const bool bTempHeader, const bool bTempGroup) :
    ScXMLImportContext( rImport ),
    nHeaderStartCol(0),
    nHeaderEndCol(0),
    nGroupStartCol(0),
    nGroupEndCol(0),
    bHeader(bTempHeader),
    bGroup(bTempGroup),
    bGroupDisplay(true)
{
    // don't have any attributes
    if (bHeader)
        nHeaderStartCol = rImport.GetTables().GetCurrentColCount();
    else if (bGroup)
    {
        nGroupStartCol = rImport.GetTables().GetCurrentColCount();
        if ( rAttrList.is() )
        {
            auto &aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_DISPLAY ) ) );
            if ( aIter != rAttrList->end() && IsXMLToken(aIter, XML_FALSE) )
                bGroupDisplay = false;
        }
    }
}

ScXMLTableColsContext::~ScXMLTableColsContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLTableColsContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( TABLE, XML_TABLE_COLUMN_GROUP ):
        pContext = new ScXMLTableColsContext( GetScImport(), pAttribList,
                                                   false, true );
        break;
    case XML_ELEMENT( TABLE, XML_TABLE_HEADER_COLUMNS ):
        pContext = new ScXMLTableColsContext( GetScImport(), pAttribList,
                                                   true, false );
        break;
    case XML_ELEMENT( TABLE, XML_TABLE_COLUMNS ):
        pContext = new ScXMLTableColsContext( GetScImport(), pAttribList,
                                                   false, false );
        break;
    case XML_ELEMENT( TABLE, XML_TABLE_COLUMN ):
        pContext = new ScXMLTableColContext( GetScImport(), pAttribList );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLTableColsContext::endFastElement( sal_Int32 /*nElement*/ )
{
    ScXMLImport& rXMLImport = GetScImport();
    if (bHeader)
    {
        nHeaderEndCol = rXMLImport.GetTables().GetCurrentColCount();
        nHeaderEndCol--;
        if (nHeaderStartCol <= nHeaderEndCol)
        {
            uno::Reference <sheet::XPrintAreas> xPrintAreas (rXMLImport.GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
            if (xPrintAreas.is())
            {
                if (!xPrintAreas->getPrintTitleColumns())
                {
                    xPrintAreas->setPrintTitleColumns(true);
                    table::CellRangeAddress aColumnHeaderRange;
                    aColumnHeaderRange.StartColumn = nHeaderStartCol;
                    aColumnHeaderRange.EndColumn = nHeaderEndCol;
                    xPrintAreas->setTitleColumns(aColumnHeaderRange);
                }
                else
                {
                    table::CellRangeAddress aColumnHeaderRange(xPrintAreas->getTitleColumns());
                    aColumnHeaderRange.EndColumn = nHeaderEndCol;
                    xPrintAreas->setTitleColumns(aColumnHeaderRange);
                }
            }
        }
    }
    else if (bGroup)
    {
        SCTAB nSheet = rXMLImport.GetTables().GetCurrentSheet();
        nGroupEndCol = rXMLImport.GetTables().GetCurrentColCount();
        nGroupEndCol--;
        if (nGroupStartCol <= nGroupEndCol)
        {
            ScDocument* pDoc = GetScImport().GetDocument();
            if (pDoc)
            {
                ScXMLImport::MutexGuard aGuard(GetScImport());
                ScOutlineTable* pOutlineTable = pDoc->GetOutlineTable(nSheet, true);
                if (pOutlineTable)
                {
                    ScOutlineArray& rColArray = pOutlineTable->GetColArray();
                    bool bResized;
                    rColArray.Insert(static_cast<SCCOL>(nGroupStartCol), static_cast<SCCOL>(nGroupEndCol), bResized, !bGroupDisplay);
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
