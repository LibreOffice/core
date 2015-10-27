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
#include "global.hxx"
#include "xmlstyli.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "olinetab.hxx"
#include "sheetdata.hxx"
#include "unonames.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableColContext::ScXMLTableColContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sVisibility(GetXMLToken(XML_VISIBLE))
{
    nColCount = 1;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableColAttrTokenMap();

    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_COL_ATTR_REPEATED:
                {
                    nColCount = std::max<sal_Int32>(sValue.toInt32(), 1);
                    nColCount = std::min<sal_Int32>(nColCount, MAXCOLCOUNT);
                }
                break;
            case XML_TOK_TABLE_COL_ATTR_STYLE_NAME:
                {
                    sStyleName = sValue;
                }
                break;
            case XML_TOK_TABLE_COL_ATTR_VISIBILITY:
                {
                    sVisibility = sValue;
                }
                break;
            case XML_TOK_TABLE_COL_ATTR_DEFAULT_CELL_STYLE_NAME:
                {
                    sCellStyleName = sValue;
                }
                break;
        }
    }
}

ScXMLTableColContext::~ScXMLTableColContext()
{
}

SvXMLImportContext *ScXMLTableColContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableColContext::EndElement()
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
                                pSheetData->AddColumnStyle( sStyleName, ScAddress( (SCCOL)nCurrentColumn, 0, nSheet ) );
                                pStyle->SetLastSheet(nSheet);
                            }
                        }
                    }
                }
                OUString sVisible(SC_UNONAME_CELLVIS);
                bool bValue(true);
                if (!IsXMLToken(sVisibility, XML_VISIBLE))
                    bValue = false;
                xColumnProperties->setPropertyValue(sVisible, uno::makeAny(bValue));
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
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      const bool bTempHeader, const bool bTempGroup) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
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
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const OUString& sAttrName(xAttrList->getNameByIndex( i ));
            OUString aLocalName;
            sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            const OUString& sValue(xAttrList->getValueByIndex( i ));

            if (nPrefix == XML_NAMESPACE_TABLE && IsXMLToken(aLocalName, XML_DISPLAY))
            {
                if (IsXMLToken(sValue, XML_FALSE))
                    bGroupDisplay = false;
            }
        }
    }
}

ScXMLTableColsContext::~ScXMLTableColsContext()
{
}

SvXMLImportContext *ScXMLTableColsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetTableColsElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_COLS_COL_GROUP:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   false, true );
        break;
    case XML_TOK_TABLE_COLS_HEADER_COLS:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   true, false );
        break;
    case XML_TOK_TABLE_COLS_COLS:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   false, false );
        break;
    case XML_TOK_TABLE_COLS_COL:
            pContext = new ScXMLTableColContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList//,
                                                      //this
                                                      );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableColsContext::EndElement()
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
