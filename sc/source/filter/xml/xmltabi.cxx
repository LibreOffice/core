/*************************************************************************
 *
 *  $RCSfile: xmltabi.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2000-10-10 14:26:31 $
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

#include "xmltabi.hxx"
#include "xmlimprt.hxx"
#include "xmlrowi.hxx"
#include "xmlcoli.hxx"
#include "xmlsceni.hxx"
#include "document.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlkywd.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/table/CellAddress.hpp>

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLTableContext::ScXMLTableContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      const sal_Bool bTempIsSubTable,
                                      const sal_Int32 nSpannedCols) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    if (!bTempIsSubTable)
    {
        sal_Bool bProtection(sal_False);
        rtl::OUString sName;
        rtl::OUString sStyleName;
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; i++ )
        {
            rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            rtl::OUString aLocalName;
            USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            rtl::OUString sValue = xAttrList->getValueByIndex( i );

            const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAttrTokenMap();

            switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
            {
                case XML_TOK_TABLE_NAME:
                        sName = sValue;
                    break;
                case XML_TOK_TABLE_STYLE_NAME:
                        sStyleName = sValue;
                    break;
                case XML_TOK_TABLE_PROTECTION:
                        if (sValue.compareToAscii(sXML_true) == 0)
                            bProtection = sal_True;
                    break;
                case XML_TOK_TABLE_PRINT_RANGES:
                        sPrintRanges = sValue;
                    break;
            }
        }
        GetScImport().GetTables().NewSheet(sName, sStyleName, bProtection);
    }
    else
    {
        GetScImport().GetTables().NewTable(nSpannedCols);
    }
}

ScXMLTableContext::~ScXMLTableContext()
{
}

SvXMLImportContext *ScXMLTableContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetTableElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
//  case XML_TOK_TABLE_HEADER_COLS:
//      bHeader = TRUE;
//  case XML_TOK_TABLE_COLS:
//      pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
//                                                 rLName, xAttrList,
//                                                 this, bHeader );
//      break;
    case XML_TOK_TABLE_COL:
//      if( IsInsertColPossible() )
            pContext = new ScXMLTableColContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList );
        break;
//  case XML_TOK_TABLE_HEADER_ROWS:
//      bHeader = TRUE;
//  case XML_TOK_TABLE_ROWS:
//      pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
//                                                 rLName, xAttrList,
//                                                 this, bHeader );
//      break;
    case XML_TOK_TABLE_ROW:
//      if( IsInsertRowPossible() )
            pContext = new ScXMLTableRowContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList//,
                                                      //this
                                                      );
        break;
    case XML_TOK_TABLE_SCENARIO:
        pContext = new ScXMLTableScenarioContext( GetScImport(), nPrefix, rLName, xAttrList);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableContext::EndElement()
{
    sal_Int16 nTable = GetScImport().GetTables().GetCurrentSheet();
    uno::Reference< sheet::XSpreadsheetDocument > xSpreadDoc( GetScImport().GetModel(), uno::UNO_QUERY );
    if( xSpreadDoc.is() )
    {
        uno::Reference< sheet::XSpreadsheets > xSheets = xSpreadDoc->getSheets();
        uno::Reference< container::XIndexAccess > xIndex( xSheets, uno::UNO_QUERY );
        if( xIndex.is() )
        {
            uno::Reference< sheet::XSpreadsheet > xTable;
            uno::Any aTable = xIndex->getByIndex( nTable );
            if( aTable >>= xTable )
            {
                uno::Reference< sheet::XPrintAreas > xPrintAreas( xTable, uno::UNO_QUERY );
                if( xPrintAreas.is() )
                {
                    uno::Sequence< table::CellRangeAddress > aRangeList;
                    sal_Int32 nIndex = 0;
                    while( nIndex >= 0 )
                    {
                        table::CellRangeAddress aCellRange;
                        nIndex = GetScImport().GetRangeFromString( sPrintRanges, nIndex, aCellRange );
                        aRangeList.realloc( aRangeList.getLength() + 1 );
                        aRangeList[ aRangeList.getLength() - 1 ] = aCellRange;
                    }
                    xPrintAreas->setPrintAreas( aRangeList );
                }
            }
        }
    }
    GetScImport().GetTables().DeleteTable();
}

