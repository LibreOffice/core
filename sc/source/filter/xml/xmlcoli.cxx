/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlcoli.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:51:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "xmlcoli.hxx"
#include "xmlimprt.hxx"
#include "global.hxx"
#include "xmlstyli.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "olinetab.hxx"

#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
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

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLTableColContext::ScXMLTableColContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sVisibility(GetXMLToken(XML_VISIBLE))
{
    nColCount = 1;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableColAttrTokenMap();

    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

SvXMLImportContext *ScXMLTableColContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
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
    //sal_Int32 nSheet = rXMLImport.GetTables().GetCurrentSheet();
    sal_Int32 nCurrentColumn = rXMLImport.GetTables().GetCurrentColumn();
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
                if (sStyleName.getLength())
                {
                    XMLTableStylesContext *pStyles = (XMLTableStylesContext *)rXMLImport.GetAutoStyles();
                    if ( pStyles )
                    {
                        XMLTableStyleContext* pStyle = (XMLTableStyleContext *)pStyles->FindStyleChildContext(
                            XML_STYLE_FAMILY_TABLE_COLUMN, sStyleName, sal_True);
                        if (pStyle)
                            pStyle->FillPropertySet(xColumnProperties);
                    }
                }
                rtl::OUString sVisible(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLVIS));
                sal_Bool bValue(sal_True);
                if (!IsXMLToken(sVisibility, XML_VISIBLE))
                    bValue = sal_False;
                xColumnProperties->setPropertyValue(sVisible, uno::makeAny(bValue));
            }
        }
    }
    GetScImport().GetTables().AddColCount(nColCount);
    GetScImport().GetTables().AddColStyle(nColCount, sCellStyleName);
}

ScXMLTableColsContext::ScXMLTableColsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      const sal_Bool bTempHeader, const sal_Bool bTempGroup) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nHeaderStartCol(0),
    nHeaderEndCol(0),
    nGroupStartCol(0),
    nGroupEndCol(0),
    bHeader(bTempHeader),
    bGroup(bTempGroup),
    bGroupDisplay(sal_True)
{
    // don't have any attributes
    if (bHeader)
        nHeaderStartCol = rImport.GetTables().GetCurrentColumn();
    else if (bGroup)
    {
        nGroupStartCol = rImport.GetTables().GetCurrentColumn();
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
            rtl::OUString aLocalName;
            sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName );
            const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

            if (nPrfx == XML_NAMESPACE_TABLE && IsXMLToken(aLocalName, XML_DISPLAY))
            {
                if (IsXMLToken(sValue, XML_FALSE))
                    bGroupDisplay = sal_False;
            }
        }
    }
}

ScXMLTableColsContext::~ScXMLTableColsContext()
{
}

SvXMLImportContext *ScXMLTableColsContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetTableColsElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_COLS_COL_GROUP:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_False, sal_True );
        break;
    case XML_TOK_TABLE_COLS_HEADER_COLS:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_True, sal_False );
        break;
    case XML_TOK_TABLE_COLS_COLS:
        pContext = new ScXMLTableColsContext( GetScImport(), nPrefix,
                                                   rLName, xAttrList,
                                                   sal_False, sal_False );
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
        nHeaderEndCol = rXMLImport.GetTables().GetCurrentColumn();
        nHeaderEndCol--;
        if (nHeaderStartCol <= nHeaderEndCol)
        {
            uno::Reference <sheet::XPrintAreas> xPrintAreas (rXMLImport.GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
            if (xPrintAreas.is())
            {
                if (!xPrintAreas->getPrintTitleColumns())
                {
                    xPrintAreas->setPrintTitleColumns(sal_True);
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
        sal_Int32 nSheet = rXMLImport.GetTables().GetCurrentSheet();
        nGroupEndCol = rXMLImport.GetTables().GetCurrentColumn();
        nGroupEndCol--;
        if (nGroupStartCol <= nGroupEndCol)
        {
            ScDocument* pDoc = GetScImport().GetDocument();
            if (pDoc)
            {
                rXMLImport.LockSolarMutex();
                ScOutlineTable* pOutlineTable = pDoc->GetOutlineTable(static_cast<SCTAB>(nSheet), sal_True);
                ScOutlineArray* pColArray = pOutlineTable ? pOutlineTable->GetColArray() : NULL;
                if (pColArray)
                {
                    sal_Bool bResized;
                    pColArray->Insert(static_cast<SCCOL>(nGroupStartCol), static_cast<SCCOL>(nGroupEndCol), bResized, !bGroupDisplay, sal_True);
                }
                rXMLImport.UnlockSolarMutex();
            }
        }
    }
}
