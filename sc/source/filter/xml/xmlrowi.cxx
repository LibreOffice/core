/*************************************************************************
 *
 *  $RCSfile: xmlrowi.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-01 13:19:03 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "xmlrowi.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#include "global.hxx"
#include "xmlstyli.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "olinetab.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCOLUMNROWRANGE_HPP_
#include <com/sun/star/table/XColumnRowRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XPRINTAREAS_HPP_
#include <com/sun/star/sheet/XPrintAreas.hpp>
#endif

#include <com/sun/star/table/CellAddress.hpp>

#define SC_ISVISIBLE "IsVisible"
#define SC_OPTIMALHEIGHT "OptimalHeight"
#define SC_ISFILTERED "IsFiltered"

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLTableRowContext::ScXMLTableRowContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sVisibility(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_visible))),
    nRepeatedRows(1)
//  sOptimalHeight(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_false)))
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableRowAttrTokenMap();

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
                nRepeatedRows = sValue.toInt32();
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
}

ScXMLTableRowContext::~ScXMLTableRowContext()
{
}

SvXMLImportContext *ScXMLTableRowContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetTableRowElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROW_CELL:
//      if( IsInsertCellPossible() )
            pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList, sal_False, nRepeatedRows
                                                      //this
                                                      );
        break;
    case XML_TOK_TABLE_ROW_COVERED_CELL:
//      if( IsInsertCellPossible() )
            pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList, sal_True, nRepeatedRows
                                                      //this
                                                      );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableRowContext::EndElement()
{
    ScXMLImport& rXMLImport = GetScImport();
    sal_Int16 nSheet = rXMLImport.GetTables().GetCurrentSheet();
    sal_Int32 nCurrentRow = rXMLImport.GetTables().GetCurrentRow();
    uno::Reference <sheet::XSpreadsheetDocument> xSheetsDocument (rXMLImport.GetModel(), uno::UNO_QUERY);
    if(xSheetsDocument.is())
    {
        uno::Reference<sheet::XSpreadsheets> xSheets = xSheetsDocument->getSheets();
        if (xSheets.is())
        {
            uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY);
            if (xIndex.is())
            {
                uno::Any aSheet = xIndex->getByIndex(nSheet);
                uno::Reference<sheet::XSpreadsheet> xSheet;
                if(aSheet >>= xSheet)
                {
                    uno::Reference<table::XColumnRowRange> xColumnRowRange (xSheet, uno::UNO_QUERY);
                    if (xColumnRowRange.is())
                    {
                        uno::Reference<table::XTableRows> xTableRows = xColumnRowRange->getRows();
                        if (xTableRows.is())
                        {
                            uno::Any aRow;
                            if (nCurrentRow <= MAXROW)
                                aRow = xTableRows->getByIndex(nCurrentRow);
                            else
                                aRow = xTableRows->getByIndex(MAXROW);
                            uno::Reference<table::XCellRange> xTableRow;
                            if (aRow >>= xTableRow)
                            {
                                uno::Reference <beans::XPropertySet> xRowProperties(xTableRow, uno::UNO_QUERY);
                                if (xRowProperties.is())
                                {
                                    XMLTableStylesContext *pStyles = (XMLTableStylesContext *)&rXMLImport.GetAutoStyles();
                                    XMLTableStyleContext* pStyle = (XMLTableStyleContext *)pStyles->FindStyleChildContext(
                                        XML_STYLE_FAMILY_TABLE_ROW, sStyleName, sal_True);
                                    if (pStyle)
                                    {
                                        pStyle->FillPropertySet(xRowProperties);
                                    }
                                    uno::Any aVisibleAny = xRowProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISVISIBLE)));
                                    uno::Any aFilteredAny = xRowProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISFILTERED)));
                                    if (sVisibility.compareToAscii(sXML_collapse) == 0)
                                    {
                                        sal_Bool bValue = sal_False;
                                        aVisibleAny <<= bValue;
                                        aFilteredAny <<= bValue;
                                    }
                                    else if (sVisibility.compareToAscii(sXML_filter) == 0)
                                    {
                                        sal_Bool bValue = sal_False;
                                        aVisibleAny <<= bValue;
                                        bValue = sal_True;
                                        aFilteredAny <<= bValue;
                                    }
                                    else
                                    {
                                        sal_Bool bValue = sal_True;
                                        aVisibleAny <<= bValue;
                                        bValue = sal_False;
                                        aFilteredAny <<= bValue;
                                    }
                                    /*uno::Any aAny = xRowProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_OPTIMALHEIGHT)));
                                    if (sOptimalHeight.compareToAscii(sXML_true) != 0)
                                    {
                                        sal_Bool bValue = sal_False;
                                        aAny <<= bValue;
                                    }
                                    else
                                    {
                                        sal_Bool bValue = sal_True;
                                        aAny <<= bValue;
                                    }*/
                                    xRowProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISVISIBLE)), aVisibleAny);
                                    xRowProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISFILTERED)), aFilteredAny);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

ScXMLTableRowsContext::ScXMLTableRowsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
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
        nHeaderStartRow++;
    }
    else if (bGroup)
    {
        nGroupStartRow = rImport.GetTables().GetCurrentRow();
        nGroupStartRow++;
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            rtl::OUString aLocalName;
            sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            rtl::OUString sValue = xAttrList->getValueByIndex( i );

            if (nPrfx == XML_NAMESPACE_TABLE && aLocalName.compareToAscii(sXML_display) == 0)
            {
                if (sValue.compareToAscii(sXML_false) == 0)
                    bGroupDisplay = sal_False;
            }
        }
    }
}

ScXMLTableRowsContext::~ScXMLTableRowsContext()
{
}

SvXMLImportContext *ScXMLTableRowsContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetTableRowsElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROWS_ROW_GROUP:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_False, sal_True );
        break;
    case XML_TOK_TABLE_ROWS_HEADER_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_True, sal_False );
        break;
    case XML_TOK_TABLE_ROWS_ROWS:
        pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_False, sal_False );
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
    ScXMLImport& rXMLImport = GetScImport();
    sal_Int16 nSheet = rXMLImport.GetTables().GetCurrentSheet();
    if (bHeader)
    {
        nHeaderEndRow = rXMLImport.GetTables().GetCurrentRow();
        if (nHeaderStartRow <= nHeaderEndRow)
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSheetsDocument (rXMLImport.GetModel(), uno::UNO_QUERY);
            if(xSheetsDocument.is())
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSheetsDocument->getSheets();
                if (xSheets.is())
                {
                    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY);
                    if (xIndex.is())
                    {
                        uno::Any aSheet = xIndex->getByIndex(nSheet);
                        uno::Reference<sheet::XSpreadsheet> xSheet;
                        if(aSheet >>= xSheet)
                        {
                            uno::Reference <sheet::XPrintAreas> xPrintAreas (xSheet, uno::UNO_QUERY);
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
                                    table::CellRangeAddress aRowHeaderRange = xPrintAreas->getTitleRows();
                                    aRowHeaderRange.EndRow = nHeaderEndRow;
                                    xPrintAreas->setTitleRows(aRowHeaderRange);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (bGroup)
    {
        nGroupEndRow = rXMLImport.GetTables().GetCurrentRow();
        if (nGroupStartRow <= nGroupEndRow)
        {
            ScModelObj* pDocObj = ScModelObj::getImplementation( rXMLImport.GetModel() );
            if( pDocObj )
            {
                ScDocument* pDoc = pDocObj->GetDocument();
                if( pDoc )
                {
                    ScOutlineTable* pOutlineTable = pDoc->GetOutlineTable(nSheet, sal_True);
                    ScOutlineArray* pRowArray = pOutlineTable->GetRowArray();
                    sal_Bool bResized;
                    pRowArray->Insert(nGroupStartRow, nGroupEndRow, bResized, !bGroupDisplay, sal_True);
                }
            }
        }
    }
}
