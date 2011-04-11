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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

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
#define SC_OPTIMALHEIGHT "OptimalHeight"
#define SC_ISFILTERED "IsFiltered"

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLTableRowContext::ScXMLTableRowContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sVisibility(GetXMLToken(XML_VISIBLE)),
    nRepeatedRows(1),
    bHasCell(false)
{
    rtl::OUString sCellStyleName;
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetTableRowAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetTableRowElemTokenMap());
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROW_CELL:
//      if( IsInsertCellPossible() )
        {
            bHasCell = sal_True;
            pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList, false, nRepeatedRows
                                                      //this
                                                      );
        }
        break;
    case XML_TOK_TABLE_ROW_COVERED_CELL:
//      if( IsInsertCellPossible() )
        {
            bHasCell = sal_True;
            pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList, sal_True, nRepeatedRows
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
        DBG_ERRORFILE("it seems here is a nonvalid file; possible missing of table:table-cell element");
    }
    sal_Int32 nSheet = rXMLImport.GetTables().GetCurrentSheet();
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
                    if (sStyleName.getLength())
                    {
                        XMLTableStylesContext *pStyles((XMLTableStylesContext *)rXMLImport.GetAutoStyles());
                        if ( pStyles )
                        {
                            XMLTableStyleContext* pStyle((XMLTableStyleContext *)pStyles->FindStyleChildContext(
                                XML_STYLE_FAMILY_TABLE_ROW, sStyleName, sal_True));
                            if (pStyle)
                            {
                                pStyle->FillPropertySet(xRowProperties);

                                if ( nSheet != pStyle->GetLastSheet() )
                                {
                                    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rXMLImport.GetModel())->GetSheetSaveData();
                                    pSheetData->AddRowStyle( sStyleName, ScAddress( 0, (SCROW)nFirstRow, (SCTAB)nSheet ) );
                                    pStyle->SetLastSheet(nSheet);
                                }
                            }
                        }
                    }
                    sal_Bool bVisible (sal_True);
                    sal_Bool bFiltered (false);
                    if (IsXMLToken(sVisibility, XML_COLLAPSE))
                    {
                        bVisible = false;
                    }
                    else if (IsXMLToken(sVisibility, XML_FILTER))
                    {
                        bVisible = false;
                        bFiltered = sal_True;
                    }
                    if (!bVisible)
                        xRowProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISVISIBLE)), uno::makeAny(bVisible));
                    if (bFiltered)
                        xRowProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISFILTERED)), uno::makeAny(bFiltered));
                }
            }
        }
    }
}

ScXMLTableRowsContext::ScXMLTableRowsContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      const sal_Bool bTempHeader, const sal_Bool bTempGroup ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nHeaderStartRow(0),
    nHeaderEndRow(0),
    nGroupStartRow(0),
    nGroupEndRow(0),
    bHeader(bTempHeader),
    bGroup(bTempGroup),
    bGroupDisplay(sal_True)
{
    // don't have any attributes
    if (bHeader)
    {
        nHeaderStartRow = rImport.GetTables().GetCurrentRow();
        ++nHeaderStartRow;
    }
    else if (bGroup)
    {
        nGroupStartRow = rImport.GetTables().GetCurrentRow();
        ++nGroupStartRow;
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
            rtl::OUString aLocalName;
            sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName ));
            const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

            if ((nPrefix == XML_NAMESPACE_TABLE) && IsXMLToken(aLocalName, XML_DISPLAY))
                bGroupDisplay = IsXMLToken(sValue, XML_TRUE);
        }
    }
}

ScXMLTableRowsContext::~ScXMLTableRowsContext()
{
}

SvXMLImportContext *ScXMLTableRowsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetTableRowsElemTokenMap());
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROWS_ROW_GROUP:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   false, sal_True );
        break;
    case XML_TOK_TABLE_ROWS_HEADER_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_True, false );
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
        nHeaderEndRow = rXMLImport.GetTables().GetCurrentRow();
        if (nHeaderStartRow <= nHeaderEndRow)
        {
            uno::Reference <sheet::XPrintAreas> xPrintAreas (rXMLImport.GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
            if (xPrintAreas.is())
            {
                if (!xPrintAreas->getPrintTitleRows())
                {
                    xPrintAreas->setPrintTitleRows(sal_True);
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
        nGroupEndRow = rXMLImport.GetTables().GetCurrentRow();
        sal_Int32 nSheet(rXMLImport.GetTables().GetCurrentSheet());
        if (nGroupStartRow <= nGroupEndRow)
        {
            ScDocument* pDoc(GetScImport().GetDocument());
            if (pDoc)
            {
                ScXMLImport::MutexGuard aGuard(GetScImport());
                ScOutlineTable* pOutlineTable(pDoc->GetOutlineTable(static_cast<SCTAB>(nSheet), sal_True));
                ScOutlineArray* pRowArray(pOutlineTable->GetRowArray());
                sal_Bool bResized;
                pRowArray->Insert(static_cast<SCROW>(nGroupStartRow), static_cast<SCROW>(nGroupEndRow), bResized, !bGroupDisplay, sal_True);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
