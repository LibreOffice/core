/*************************************************************************
 *
 *  $RCSfile: xmlcoli.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:15 $
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

#include "xmlcoli.hxx"
#include "xmlimprt.hxx"
#include "global.hxx"
#include "xmlstyli.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
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

#define SC_ISVISIBLE "IsVisible"

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLTableColContext::ScXMLTableColContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sVisibility(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_visible)))
{
    nColCount = 1;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableColAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_COL_ATTR_REPEATED:
                {
                    nColCount = sValue.toInt32();
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
        }
    }
}

ScXMLTableColContext::~ScXMLTableColContext()
{
}

SvXMLImportContext *ScXMLTableColContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;
/*
    const SvXMLTokenMap& rTokenMap = GetScImport().GetTableRowElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROW_CELL:
//      if( IsInsertCellPossible() )
            pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList//,
                                                      //this
                                                      );
        break;
    case XML_TOK_TABLE_ROW_COVERED_CELL:
//      if( IsInsertCellPossible() )
            pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList//,
                                                      //this
                                                      );
        break;
    }*/

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableColContext::EndElement()
{
    ScXMLImport& rXMLImport = GetScImport();
    sal_Int16 nSheet = rXMLImport.GetTables().GetCurrentSheet();
    sal_Int32 nCurrentColumn = rXMLImport.GetTables().GetCurrentColumn();
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
                        uno::Reference<table::XTableColumns> xTableColumns = xColumnRowRange->getColumns();
                        if (xTableColumns.is())
                        {
                            for (sal_Int32 i = 0; i < nColCount; i++)
                            {
                                uno::Any aColumn = xTableColumns->getByIndex(i + nCurrentColumn);
                                uno::Reference<table::XCellRange> xTableColumn;
                                if (aColumn >>= xTableColumn)
                                {
                                    uno::Reference <beans::XPropertySet> xColumnProperties(xTableColumn, uno::UNO_QUERY);
                                    if (xColumnProperties.is())
                                    {
                                        XMLTableStylesContext *pStyles = (XMLTableStylesContext *)&rXMLImport.GetAutoStyles();
                                        XMLTableStyleContext* pStyle = (XMLTableStyleContext *)pStyles->FindStyleChildContext(
                                            XML_STYLE_FAMILY_TABLE_COLUMN, sStyleName, sal_True);
                                        if (pStyle)
                                        {
                                            pStyle->FillPropertySet(xColumnProperties);
                                        }
                                        uno::Any aAny = xColumnProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISVISIBLE)));
                                        if (sVisibility.compareToAscii(sXML_visible) == 0)
                                        {
                                            sal_Bool bValue = sal_True;
                                            aAny <<= bValue;
                                        }
                                        else
                                        {
                                            sal_Bool bValue = sal_False;
                                            aAny <<= bValue;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    GetScImport().GetTables().AddColCount(nColCount);
}
