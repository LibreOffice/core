/*************************************************************************
 *
 *  $RCSfile: xmlcelli.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dr $ $Date: 2000-10-26 13:50:57 $
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

#include "xmlcelli.hxx"
#include "xmlconti.hxx"
#include "xmlimprt.hxx"
#include "xmltabi.hxx"
#include "xmlstyli.hxx"
#include "xmlannoi.hxx"
#include "global.hxx"
#include "document.hxx"
#include "cellsuno.hxx"
#include "docuno.hxx"
#include "unonames.hxx"

// core implementation
#ifndef SC_AREALINK_HXX
#include "arealink.hxx"
#endif
// core implementation
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlkywd.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/families.hxx>
#ifndef XMLOFF_NUMEHELP_HXX
#include <xmloff/numehelp.hxx>
#endif
#include <svtools/zforlist.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#ifndef _COM_SUN_STAR_UTIL_XMERGEABLE_HPP_
#include <com/sun/star/util/XMergeable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITION_HPP_
#include <com/sun/star/sheet/XSheetCondition.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELLRANGE_HPP_
#include <com/sun/star/table/XCellRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#include <tools/intn.hxx>

#define SC_NUMBERFORMAT "NumberFormat"
#define SC_LOCALE "Locale"
#define SC_NULLDATE "NullDate"
#define SC_CURRENCYSYMBOL "CurrencySymbol"
#define SC_CELLSTYLE "CellStyle"
#define SC_TYPE "Type"
#define SC_STANDARDFORMAT "StandardFormat"

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLTableRowCellContext::ScXMLTableRowCellContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      const sal_Bool bTempIsCovered,
                                      const sal_Int32 nTempRepeatedRows ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bIsMerged(sal_False),
    bIsMatrix(sal_False),
    bIsFormula(sal_False),
    bHasSubTable(sal_False),
    bIsCovered(bTempIsCovered),
    bHasAnnotation(sal_False),
    nRepeatedRows(nTempRepeatedRows),
    bIsEmpty(sal_True),
    bHasTextImport(sal_False),
    bIsFirstTextImport(sal_True),
    aCellRangeSource()
{
    GetScImport().GetTables().AddColumn(bTempIsCovered);
    nMergedCols = 1;
    nMergedRows = 1;
    nCellsRepeated = 1;
    fValue = 0.0;
    nCellType = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableRowCellAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME:
                {
                    sStyleName = sValue;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_CONTENT_VALIDATION_NAME:
                {
                    sContentValidationName = sValue;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS:
                {
                    bIsMerged = sal_True;
                    nMergedRows = sValue.toInt32();
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS:
                {
                    bIsMerged = sal_True;
                    nMergedCols = sValue.toInt32();
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED:
                {
                    nCellsRepeated = sValue.toInt32();
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE:
                {
                    nCellType = GetCellType(sValue);
                    bIsEmpty = sal_False;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_VALUE:
                {
                    GetScImport().GetMM100UnitConverter().convertNumber(fValue, sValue);
                    bIsEmpty = sal_False;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE:
                {
                    sOUDateValue = sValue;
                    bIsEmpty = sal_False;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE:
                {
                    sOUTimeValue = sValue;
                    bIsEmpty = sal_False;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE:
                {
                    sOUBooleanValue = sValue;
                    bIsEmpty = sal_False;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE:
                {
                    sOUTextValue = sValue;
                    bIsEmpty = sal_False;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_FORMULA:
                {
                    bIsFormula = sal_True;
                    sOUFormula = sValue;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_CURRENCY :
                {
                    sCurrencySymbol = sValue;
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS :
                {
                    bIsMatrix = sal_True;
                    nMatrixCols = sValue.toInt32();
                }
                break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS :
                {
                    bIsMatrix = sal_True;
                    nMatrixRows = sValue.toInt32();
                }
                break;
        }
    }

}

sal_Int16 ScXMLTableRowCellContext::GetCellType(const rtl::OUString& sOUValue) const
{
    if (sOUValue.equals(GetScImport().sSC_float))
        return util::NumberFormat::NUMBER;
    else
        if (sOUValue.equals(GetScImport().sSC_string))
            return util::NumberFormat::TEXT;
        else
            if (sOUValue.equals(GetScImport().sSC_time))
                return util::NumberFormat::TIME;
            else
                if (sOUValue.equals(GetScImport().sSC_date))
                    return util::NumberFormat::DATETIME;
                else
                    if (sOUValue.equals(GetScImport().sSC_percentage))
                        return util::NumberFormat::PERCENT;
                    else
                        if (sOUValue.equals(GetScImport().sSC_currency))
                            return util::NumberFormat::CURRENCY;
                        else
                            if (sOUValue.equals(GetScImport().sSC_boolean))
                                return util::NumberFormat::LOGICAL;
                            else
                                return 0;
}

ScXMLTableRowCellContext::~ScXMLTableRowCellContext()
{
}

SvXMLImportContext *ScXMLTableRowCellContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetTableRowCellElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROW_CELL_P:
        {
//      if( IsInsertTextPossible() )
/*          pContext = new ScXMLContentContext( GetScImport(), nPrefix,
                                                      rLName, xAttrList, XML_TOK_TABLE_ROW_CELL_P,
                                                      this );*/
            bIsEmpty = sal_False;
            ScXMLImport& rXMLImport = GetScImport();
            if (bIsFirstTextImport)
            {
                com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
                uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rXMLImport.GetModel(), uno::UNO_QUERY );
                if ( xSpreadDoc.is() )
                {
                    uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                    uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                    if ( xIndex.is() )
                    {
                        uno::Any aTable = xIndex->getByIndex(aCellPos.Sheet);
                        uno::Reference<sheet::XSpreadsheet> xTable;
                        aTable>>=xTable;
                        uno::Reference<table::XCellRange> xCellRange ( xTable, uno::UNO_QUERY );
                        if (xCellRange.is())
                        {
                            if (aCellPos.Column > MAXCOL)
                                aCellPos.Column = MAXCOL;
                            if (aCellPos.Row > MAXROW)
                                aCellPos.Row = MAXROW;
                            uno::Reference <table::XCell> xCell = xCellRange->getCellByPosition(aCellPos.Column, aCellPos.Row);
                            if (xCell.is())
                            {
                                uno::Reference<text::XText> xText(xCell, uno::UNO_QUERY);
                                if (xText.is())
                                {
                                    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
                                    rXMLImport.GetTextImport()->SetCursor(xTextCursor);
                                    bIsFirstTextImport = sal_False;
                                    bHasTextImport = sal_True;
                                    pContext = rXMLImport.GetTextImport()->CreateTextChildContext(
                                        GetScImport(), nPrefix, rLName, xAttrList);
                                }
                            }
                        }
                    }
                }
            }
            else
                pContext = rXMLImport.GetTextImport()->CreateTextChildContext(
                    GetScImport(), nPrefix, rLName, xAttrList);
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_SUBTABLE:
        {
            bHasSubTable = sal_True;
            pContext = new ScXMLTableContext( GetScImport() , nPrefix,
                                                        rLName, xAttrList,
                                                        sal_True, nMergedCols);
            nMergedCols = 1;
            bIsMerged = sal_False;
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_ANNOTATION:
        {
            bIsEmpty = sal_False;
            pContext = new ScXMLAnnotationContext( GetScImport(), nPrefix, rLName,
                                                    xAttrList, this);
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE:
        {
            bIsEmpty = sal_False;
            pContext = new ScXMLCellRangeSourceContext(
                GetScImport(), nPrefix, rLName, xAttrList, aCellRangeSource );
        }
        break;
    }

    if (!pContext)
    {
        ScXMLImport& rXMLImport = GetScImport();
        com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rXMLImport.GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            ScModelObj* pDocObj = ScModelObj::getImplementation( GetScImport().GetModel() );
            if ( pDocObj )
            {
                ScDocument* pDoc = pDocObj->GetDocument();
                if (pDoc)
                {
                    uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                    uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                    if ( xIndex.is() )
                    {
                        uno::Any aTable = xIndex->getByIndex(aCellPos.Sheet);
                        uno::Reference<sheet::XSpreadsheet> xTable;
                        if (aTable>>=xTable)
                        {
                            uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTable, uno::UNO_QUERY);
                            if (xDrawPageSupplier.is())
                            {
                                uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
                                if (xDrawPage.is())
                                {
                                    uno::Reference<drawing::XShapes> xShapes (xDrawPage, uno::UNO_QUERY);
                                    if (xShapes.is())
                                    {
                                        if (aCellPos.Column > MAXCOL)
                                            aCellPos.Column = MAXCOL;
                                        if (aCellPos.Row > MAXROW)
                                            aCellPos.Row = MAXROW;
                                        Rectangle aRec = pDoc->GetMMRect(aCellPos.Column, aCellPos.Row, aCellPos.Column, aCellPos.Row, aCellPos.Sheet);
                                        awt::Point aPoint;
                                        aPoint.X = aRec.Left();
                                        aPoint.Y = aRec.Top();
                                        awt::Point* pPoint = &aPoint;
                                        pContext = rXMLImport.GetShapeImport()->CreateGroupChildContext(
                                            rXMLImport, nPrefix, rLName, xAttrList, xShapes/*, pPoint*/);
                                        if (pContext)
                                            bIsEmpty = sal_False;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

sal_Int16 ScXMLTableRowCellContext::GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard)
{
    uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier = GetScImport().GetNumberFormatsSupplier();
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
        if (xNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet = xNumberFormats->getByKey(nNumberFormat);
                uno::Any aIsStandardFormat = xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_STANDARDFORMAT)));
                aIsStandardFormat >>= bIsStandard;
                uno::Any aNumberFormat = xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_TYPE)));
                sal_Int16 nNumberFormat;
                if ( aNumberFormat >>= nNumberFormat )
                {
                    return nNumberFormat;
                }
            }
            catch ( uno::Exception& )
            {
                DBG_ERROR("Numberformat not found");
            }
        }
    }
    return 0;
}

void ScXMLTableRowCellContext::SetType(const uno::Reference<table::XCellRange>& xCellRange,
                                                const table::CellAddress& aCellAddress)
{
    if (nCellType != util::NumberFormat::TEXT)
    {
        uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier = GetScImport().GetNumberFormatsSupplier();
        if (xNumberFormatsSupplier.is())
        {
            uno::Reference <util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
            if (xNumberFormats.is())
            {
                sal_Int32 nBottom = aCellAddress.Row + nRepeatedRows - 1;
                sal_Int32 nRight = aCellAddress.Column + nCellsRepeated - 1;
                if (nBottom > MAXROW)
                    nBottom = MAXROW;
                if (nRight > MAXCOL)
                    nRight = MAXCOL;
                uno::Reference <util::XNumberFormatTypes> xNumberFormatTypes (xNumberFormats, uno::UNO_QUERY);
                uno::Reference <table::XCellRange> xPropCellRange = xCellRange->getCellRangeByPosition(aCellAddress.Column, aCellAddress.Row,
                    nRight, nBottom);
                if (xPropCellRange.is())
                {
                    uno::Reference <beans::XPropertySet> xCellPropertySet (xPropCellRange, uno::UNO_QUERY);
                    if (xCellPropertySet.is() && xNumberFormatTypes.is())
                    {
                        uno::Any aKey = xCellPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT) ));
                        sal_Int32 nKey;
                        if ( aKey >>= nKey )
                        {
                            sal_Bool bIsStandard;
                            if (nCellType != (GetCellType(nKey, bIsStandard) & ~util::NumberFormat::DEFINED))
                            {
                                uno::Reference < beans::XPropertySet> xNumberFormatProperties = xNumberFormats->getByKey(nKey);
                                if (xNumberFormatProperties.is())
                                {
                                    if (nCellType != util::NumberFormat::CURRENCY)
                                    {
                                        uno::Any aNumberLocale = xNumberFormatProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_LOCALE)));
                                        lang::Locale aLocale;
                                        if ( aNumberLocale >>= aLocale )
                                        {
                                            sal_Int32 nNumberFormatPropertyKey = xNumberFormatTypes->getStandardFormat(nCellType, aLocale);
                                            uno::Any aNumberFormatPropertyKey;
                                            aNumberFormatPropertyKey <<= nNumberFormatPropertyKey;
                                            xCellPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aNumberFormatPropertyKey );
                                        }
                                    }
                                    else
                                    {
                                        nKey = SetCurrencySymbol(nKey);
                                        uno::Any aAny;
                                        aAny <<= nKey;
                                        xCellPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aAny);
                                    }
                                }
                            }
                            else
                            {
                                if (nCellType == util::NumberFormat::CURRENCY)
                                {
                                    rtl::OUString sNewCurrencySymbol;
                                    if (XMLNumberFormatAttributesExportHelper::GetCurrencySymbol(nKey, sNewCurrencySymbol, xNumberFormatsSupplier))
                                    {
                                        if (!sNewCurrencySymbol.equals(sCurrencySymbol))
                                        {
                                            nKey = SetCurrencySymbol(nKey);
                                            uno::Any aAny;
                                            aAny <<= nKey;
                                            xCellPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aAny);
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
}

void ScXMLTableRowCellContext::SetType(const uno::Reference<table::XCell>& xCell)
{
    if (nCellType != util::NumberFormat::TEXT)
    {
        uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier = GetScImport().GetNumberFormatsSupplier();
        if (xNumberFormatsSupplier.is())
        {
            uno::Reference <util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
            if (xNumberFormats.is())
            {
                uno::Reference <util::XNumberFormatTypes> xNumberFormatTypes (xNumberFormats, uno::UNO_QUERY);
                uno::Reference <beans::XPropertySet> xCellPropertySet (xCell, uno::UNO_QUERY);
                if (xCellPropertySet.is() && xNumberFormatTypes.is())
                {
                    uno::Any aKey = xCellPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT) ));
                    sal_Int32 nKey;
                    if ( aKey >>= nKey )
                    {
                        sal_Bool bIsStandard;
                        if (nCellType != (GetCellType(nKey, bIsStandard) & ~util::NumberFormat::DEFINED))
                        {
                            uno::Reference < beans::XPropertySet> xNumberFormatProperties = xNumberFormats->getByKey(nKey);
                            if (xNumberFormatProperties.is())
                            {
                                if (nCellType != util::NumberFormat::CURRENCY)
                                {
                                    uno::Any aNumberLocale = xNumberFormatProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_LOCALE)));
                                    lang::Locale aLocale;
                                    if ( aNumberLocale >>= aLocale )
                                    {
                                        sal_Int32 nNumberFormatPropertyKey = xNumberFormatTypes->getStandardFormat(nCellType, aLocale);
                                        uno::Any aNumberFormatPropertyKey;
                                        aNumberFormatPropertyKey <<= nNumberFormatPropertyKey;
                                        xCellPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aNumberFormatPropertyKey );
                                    }
                                }
                                else
                                {
                                    nKey = SetCurrencySymbol(nKey);
                                    uno::Any aAny;
                                    aAny <<= nKey;
                                    xCellPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aAny);
                                }
                            }
                        }
                        else
                        {
                            if (nCellType == util::NumberFormat::CURRENCY)
                            {
                                rtl::OUString sNewCurrencySymbol;
                                if (XMLNumberFormatAttributesExportHelper::GetCurrencySymbol(nKey, sNewCurrencySymbol, xNumberFormatsSupplier))
                                {
                                    if (!sNewCurrencySymbol.equals(sCurrencySymbol))
                                    {
                                        nKey = SetCurrencySymbol(nKey);
                                        uno::Any aAny;
                                        aAny <<= nKey;
                                        xCellPropertySet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aAny);
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

sal_Int32 ScXMLTableRowCellContext::SetCurrencySymbol(const sal_Int32 nKey)
{
    if (sCurrencySymbol.getLength() > 0)
    {
        uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier = GetScImport().GetNumberFormatsSupplier();
        if (xNumberFormatsSupplier.is())
        {
            uno::Reference <util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
            if (xNumberFormats.is())
            {
                uno::Reference <beans::XPropertySet> xProperties = xNumberFormats->getByKey(nKey);
                if (xProperties.is())
                {
                    uno::Any aAny = xProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_LOCALE)));
                    lang::Locale aLocale;
                    if (aAny >>= aLocale)
                    {
                        LanguageType aLanguageType = ConvertIsoNamesToLanguage(aLocale.Language, aLocale.Country);
                        International aInt(aLanguageType);
                        sal_Unicode aDecimalSep = aInt.GetNumDecimalSep();
                        sal_Unicode aThousandSep = aInt.GetNumThousandSep();
                        rtl::OUStringBuffer aBuffer(15);
                        aBuffer.appendAscii("#");
                        aBuffer.append(aThousandSep);
                        aBuffer.appendAscii("##0");
                        aBuffer.append(aDecimalSep);
                        aBuffer.appendAscii("00 [$");
                        aBuffer.append(sCurrencySymbol);
                        aBuffer.appendAscii("]");
                        return xNumberFormats->addNew(aBuffer.makeStringAndClear(), aLocale);
                    }
                }
            }
        }
    }
       return nKey;
}

sal_Bool ScXMLTableRowCellContext::IsMerged (const uno::Reference <table::XCellRange>& xCellRange, const sal_Int32 nCol, const sal_Int32 nRow,
                            table::CellRangeAddress& aCellAddress) const
{
    uno::Reference <table::XCellRange> xMergeCellRange = xCellRange->getCellRangeByPosition(nCol,nRow,nCol,nRow);
    uno::Reference <util::XMergeable> xMergeable (xMergeCellRange, uno::UNO_QUERY);
    if (xMergeable.is())
    {
        uno::Reference<sheet::XSheetCellRange> xMergeSheetCellRange (xMergeCellRange, uno::UNO_QUERY);
        uno::Reference<sheet::XSpreadsheet> xTable = xMergeSheetCellRange->getSpreadsheet();
        uno::Reference<sheet::XSheetCellCursor> xMergeSheetCursor = xTable->createCursorByRange(xMergeSheetCellRange);
        if (xMergeSheetCursor.is())
        {
            xMergeSheetCursor->collapseToMergedArea();
            uno::Reference<sheet::XCellRangeAddressable> xMergeCellAddress (xMergeSheetCursor, uno::UNO_QUERY);
            if (xMergeCellAddress.is())
            {
                aCellAddress = xMergeCellAddress->getRangeAddress();
                if (aCellAddress.StartColumn == nCol && aCellAddress.EndColumn == nCol &&
                    aCellAddress.StartRow == nRow && aCellAddress.EndRow == nRow)
                    return sal_False;
                else
                    return sal_True;
            }
        }
    }
    return sal_False;
}

void ScXMLTableRowCellContext::DoMerge(const com::sun::star::table::CellAddress& aCellPos,
                 const sal_Int32 nCols, const sal_Int32 nRows) const
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetScImport().GetModel(), uno::UNO_QUERY );
    if ( xSpreadDoc.is() )
    {
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
        if ( xIndex.is() )
        {
            table::CellRangeAddress aCellAddress;
            uno::Any aTable = xIndex->getByIndex(aCellPos.Sheet);
            uno::Reference<sheet::XSpreadsheet> xTable;
            aTable>>=xTable;
            uno::Reference<table::XCellRange> xCellRange ( xTable, uno::UNO_QUERY );
            if ( xCellRange.is() )
            {
                if (IsMerged(xCellRange, aCellPos.Column, aCellPos.Row, aCellAddress))
                {
                    //unmerge
                    uno::Reference <table::XCellRange> xMergeCellRange =
                        xCellRange->getCellRangeByPosition(aCellAddress.StartColumn, aCellAddress.StartRow,
                                                            aCellAddress.EndColumn, aCellAddress.EndRow);
                    uno::Reference <util::XMergeable> xMergeable (xMergeCellRange, uno::UNO_QUERY);
                    if (xMergeable.is())
                        xMergeable->merge(sal_False);
                }

                //merge
                uno::Reference <table::XCellRange> xMergeCellRange =
                    xCellRange->getCellRangeByPosition(aCellAddress.StartColumn, aCellAddress.StartRow,
                                                        aCellAddress.EndColumn + nCols, aCellAddress.EndRow + nRows);
                uno::Reference <util::XMergeable> xMergeable (xMergeCellRange, uno::UNO_QUERY);
                if (xMergeable.is())
                    xMergeable->merge(sal_True);
            }
        }
    }
}

void ScXMLTableRowCellContext::ParseFormula(OUString& sFormula, sal_Bool bIsFormula)
{
    OUStringBuffer sBuffer(sFormula.getLength());
    sal_Int16 nCountQuotationMarks = 0;
    sal_Int16 nCountBraces = 0;
    sal_Unicode chPrevious = '=';
    for (sal_Int32 i = 0; i < sFormula.getLength(); i++)
    {
        if (sFormula[i] == '"')
            if (nCountQuotationMarks == 0)
                nCountQuotationMarks++;
            else
                nCountQuotationMarks--;
        if (sFormula[i] != '[' && sFormula[i] != ']')
        {
            if (sFormula[i] != '.' || (nCountBraces == 0 && bIsFormula) ||
                !(chPrevious == '[' || chPrevious == ':' || chPrevious == ' ' || chPrevious == '='))
            {
                sBuffer.append(sFormula[i]);
            }
        }
        else
            if (nCountQuotationMarks == 0)
                if (sFormula[i] == '[')
                    nCountBraces++;
                else
                    nCountBraces--;
            else
                sBuffer.append(sFormula[i]);
        chPrevious = sFormula[i];
    }
    sFormula = sBuffer.makeStringAndClear();
}

void ScXMLTableRowCellContext::SetContentValidation(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xPropSet)
{
    if (sContentValidationName.getLength())
    {
        ScMyImportValidation aValidation;
        if (GetScImport().GetValidation(sContentValidationName, aValidation))
        {
            uno::Any aAny = xPropSet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_VALIDAT)));
            uno::Reference<beans::XPropertySet> xPropertySet;
            if (aAny >>= xPropertySet)
            {
                if (aValidation.sErrorMessage.getLength())
                {
                    aAny <<= aValidation.sErrorMessage;
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRMESS)), aAny);
                }
                if (aValidation.sErrorTitle.getLength())
                {
                    aAny <<= aValidation.sErrorTitle;
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRTITLE)), aAny);
                }
                if (aValidation.sImputMessage.getLength())
                {
                    aAny <<= aValidation.sImputMessage;
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INPMESS)), aAny);
                }
                if (aValidation.sImputTitle.getLength())
                {
                    aAny <<= aValidation.sImputTitle;
                    xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INPTITLE)), aAny);
                }
                aAny <<= aValidation.bShowErrorMessage;
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWERR)), aAny);
                aAny <<= aValidation.bShowImputMessage;
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWINP)), aAny);
                aAny <<= aValidation.aValidationType;
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_TYPE)), aAny);
                aAny <<= aValidation.bIgnoreBlanks;
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_IGNOREBL)), aAny);
                aAny <<= aValidation.aAlertStyle;
                xPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRALSTY)), aAny);
                uno::Reference<sheet::XSheetCondition> xCondition(xPropertySet, uno::UNO_QUERY);
                if (xCondition.is())
                {
                     xCondition->setFormula1(aValidation.sFormula1);
                     xCondition->setFormula2(aValidation.sFormula2);
                     xCondition->setOperator(aValidation.aOperator);
                     xCondition->setSourcePosition(aValidation.aBaseCellAddress);
                }
            }
            aAny <<= xPropertySet;
            xPropSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_VALIDAT)), aAny);
        }
    }
}

void ScXMLTableRowCellContext::SetCellProperties(const uno::Reference<table::XCellRange>& xCellRange,
                                                const table::CellAddress& aCellAddress)
{
    ScXMLImport& rXMLImport = GetScImport();
    sal_Int32 nBottom = aCellAddress.Row + nRepeatedRows - 1;
    sal_Int32 nRight = aCellAddress.Column + nCellsRepeated - 1;
    if (nBottom > MAXROW)
        nBottom = MAXROW;
    if (nRight > MAXCOL)
        nRight = MAXCOL;
    uno::Reference <table::XCellRange> xPropCellRange = xCellRange->getCellRangeByPosition(aCellAddress.Column, aCellAddress.Row,
        nRight, nBottom);
    if (xPropCellRange.is())
    {
        uno::Reference <beans::XPropertySet> xProperties (xPropCellRange, uno::UNO_QUERY);
        if (xProperties.is())
        {
            XMLTableStylesContext *pStyles = (XMLTableStylesContext *)&rXMLImport.GetAutoStyles();
            XMLTableStyleContext* pStyle = (XMLTableStyleContext *)pStyles->FindStyleChildContext(
                XML_STYLE_FAMILY_TABLE_CELL, sStyleName, sal_True);
            if (pStyle)
            {
                rtl::OUString sParentName = pStyle->GetParent();
                uno::Any aStyleName;
                aStyleName <<= sParentName;
                xProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CELLSTYLE)), aStyleName);
                pStyle->FillPropertySet(xProperties);
            }
            else
            {
                uno::Any aStyleName;
                aStyleName <<= sStyleName;
                xProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CELLSTYLE)), aStyleName);
            }
            SetContentValidation(xProperties);
        }
    }
}

void ScXMLTableRowCellContext::SetCellProperties(const uno::Reference<table::XCell>& xCell)
{
    ScXMLImport& rXMLImport = GetScImport();
    uno::Reference <beans::XPropertySet> xProperties (xCell, uno::UNO_QUERY);
    if (xProperties.is())
    {
        XMLTableStylesContext *pStyles = (XMLTableStylesContext *)&rXMLImport.GetAutoStyles();
        XMLTableStyleContext* pStyle = (XMLTableStyleContext *)pStyles->FindStyleChildContext(
            XML_STYLE_FAMILY_TABLE_CELL, sStyleName, sal_True);
        if (pStyle)
        {
            rtl::OUString sParentName = pStyle->GetParent();
            uno::Any aStyleName;
            aStyleName <<= sParentName;
            xProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CELLSTYLE)), aStyleName);
            pStyle->FillPropertySet(xProperties);
        }
        else
        {
            uno::Any aStyleName;
            aStyleName <<= sStyleName;
            xProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CELLSTYLE)), aStyleName);
        }
        SetContentValidation(xProperties);
    }
}

void ScXMLTableRowCellContext::SetAnnotation(const uno::Reference<table::XCell>& xCell)
{
    /*uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor(xCell, uno::UNO_QUERY);
    if (xSheetAnnotationAnchor.is())
    {
        uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation = xSheetAnnotationAnchor->getAnnotation();
        uno::Reference<text::XSimpleText> xSimpleText(xSheetAnnotation, uno::UNO_QUERY);
        if (xSheetAnnotation.is() && xSimpleText.is())
        {
            xSimpleText->setString(aMyAnnotation.sText);
            //xSheetAnnotation->setAuthor(aMyAnnotation.sAuthor);
            //xSheetAnnotation->setDate();
            xSheetAnnotation->setIsVisible(aMyAnnotation.bDisplay);
        }
    }*/
    uno::Reference<sheet::XCellAddressable> xCellAddressable(xCell, uno::UNO_QUERY);
    if (xCellAddressable.is())
    {
        table::CellAddress aCellAddress = xCellAddressable->getCellAddress();
        ScModelObj* pDocObj = ScModelObj::getImplementation( GetScImport().GetModel() );
        if ( pDocObj )
        {
            ScDocument* pDoc = pDocObj->GetDocument();
            if (pDoc)
            {
                double fDate;
                GetScImport().GetMM100UnitConverter().convertDateTime(fDate, aMyAnnotation.sCreateDate);
                SvNumberFormatter* pNumForm = pDoc->GetFormatTable();
                sal_uInt32 nfIndex = pNumForm->GetFormatIndex(NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM);
                String sDate;
                Color* pColor = NULL;
                Color** ppColor = &pColor;
                pNumForm->GetOutputString(fDate, nfIndex, sDate, ppColor);
                ScPostIt aNote(String(aMyAnnotation.sText), sDate, String(aMyAnnotation.sAuthor));
                aNote.SetShown(aMyAnnotation.bDisplay);
                pDoc->SetNote(aCellAddress.Column, aCellAddress.Row, aCellAddress.Sheet, aNote);
            }
        }
    }
}

// core implementation
void ScXMLTableRowCellContext::SetCellRangeSource( sal_Int16 nSheet, sal_Int32 nColumnPos, sal_Int32 nRowPos )
{
    ScXMLImport& rXMLImport = GetScImport();
    ScModelObj* pDocObj = ScModelObj::getImplementation( rXMLImport.GetModel() );
    if ( pDocObj )
    {
        ScDocument* pDoc = pDocObj->GetDocument();
        if( pDoc )
        {
            ScRange aDestRange( nColumnPos, nRowPos, nSheet, nColumnPos + aCellRangeSource.nColumns - 1,
                nRowPos + aCellRangeSource.nRows - 1, nSheet );
            String sFilterName( aCellRangeSource.sFilterName );
            String sSourceStr( aCellRangeSource.sSourceStr );
            ScAreaLink* pLink = new ScAreaLink( pDoc->GetDocumentShell(), aCellRangeSource.sURL,
                sFilterName, aCellRangeSource.sFilterOptions, sSourceStr, aDestRange );
            SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
            pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aCellRangeSource.sURL, &sFilterName, &sSourceStr );
        }
    }
}

void ScXMLTableRowCellContext::EndElement()
{
    if (bHasTextImport)
    {
        GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
        if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, sal_True ) )
        {
            OUString sEmpty;
            GetImport().GetTextImport()->GetText()->insertString(
                GetImport().GetTextImport()->GetCursorAsRange(), sEmpty,
                sal_True );
        }
    }
    GetScImport().GetTextImport()->ResetCursor();
    if (!bHasSubTable)
    {
        ScXMLImport& rXMLImport = GetScImport();
        table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
        if (aCellPos.Column > 0 && nRepeatedRows > 1)
            aCellPos.Row -= (nRepeatedRows - 1);
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rXMLImport.GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
            uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                uno::Any aTable = xIndex->getByIndex(aCellPos.Sheet);
                uno::Reference<sheet::XSpreadsheet> xTable;
                aTable>>=xTable;
                uno::Reference<table::XCellRange> xCellRange ( xTable, uno::UNO_QUERY );
                if (aCellPos.Column > MAXCOL)
                    aCellPos.Column = MAXCOL - nCellsRepeated + 1;
                if (aCellPos.Row > MAXROW)
                    aCellPos.Row = MAXROW - nRepeatedRows + 1;
                uno::Reference <table::XCell> xTempCell = xCellRange->getCellByPosition(aCellPos.Column, aCellPos.Row);
                uno::Reference <text::XText> xTempText (xTempCell, uno::UNO_QUERY);
                if (xTempText.is())
                    sOUText=xTempText->getString();
                if (bIsMerged)
                    DoMerge(aCellPos, nMergedCols - 1, nMergedRows - 1);
                if ( !bIsFormula )
                {
                    uno::Reference <table::XCell> xCell;
                    for (sal_Int32 i = 0; i < nCellsRepeated; i++)
                    {
                        if (i > 0)
                            rXMLImport.GetTables().AddColumn(sal_False);
                        if (!bIsEmpty)
                        {
                            for (sal_Int32 j = 0; j < nRepeatedRows; j++)
                            {
                                if ((aCellPos.Column + i == 0) && (j > 0))
                                    rXMLImport.GetTables().AddRow();
                                xCell = xCellRange->getCellByPosition(aCellPos.Column + i, aCellPos.Row + j);
                                if ((!(bIsCovered) || (xCell->getType() == table::CellContentType_EMPTY)))
                                {
                                    switch (nCellType)
                                    {
                                    case util::NumberFormat::TEXT:
                                        {
                                            uno::Reference <text::XText> xText (xCell, uno::UNO_QUERY);
                                            if (xText.is())
                                            {
                                                if(sOUTextValue.getLength())
                                                    xText->setString(sOUTextValue);
                                                else
                                                    if (i > 0)
                                                        xText->setString(sOUText);
                                            }
                                        }
                                        break;
                                    case util::NumberFormat::NUMBER:
                                    case util::NumberFormat::PERCENT:
                                    case util::NumberFormat::CURRENCY:
                                        {
                                            xCell->setValue(fValue);
                                        }
                                        break;
                                    case util::NumberFormat::TIME:
                                        {
                                            rXMLImport.GetMM100UnitConverter().convertTime(fValue, sOUTimeValue);
                                            xCell->setValue(fValue);
                                        }
                                        break;
                                    case util::NumberFormat::DATETIME:
                                        {
                                            if (rXMLImport.GetMM100UnitConverter().setNullDate(rXMLImport.GetModel()))
                                            {
                                                rXMLImport.GetMM100UnitConverter().convertDateTime(fValue, sOUDateValue);
                                                double fTempValue = SolarMath::ApproxFloor(fValue);
                                                if ( SolarMath::ApproxEqual (fValue, fTempValue) )
                                                {
                                                    nCellType = util::NumberFormat::DATE;
                                                }
                                                xCell->setValue(fValue);
                                            }
                                        }
                                        break;
                                    case util::NumberFormat::LOGICAL:
                                        {
                                            if ( 0 == sOUBooleanValue.compareToAscii(sXML_true) )
                                                xCell->setValue(1.0);
                                            else
                                                xCell->setValue(0.0);
                                        }
                                        break;
                                    default:
                                        {
                                            uno::Reference <text::XText> xText (xCell, uno::UNO_QUERY);
                                            if (xText.is())
                                            {
                                                if (i > 0)
                                                    xText->setString(sOUText);
                                            }
                                        }
                                        break;
                                    }
                                }
                                if (bHasAnnotation)
                                    SetAnnotation(xCell);
                                if( aCellRangeSource.bHas )
                                    SetCellRangeSource( aCellPos.Sheet, aCellPos.Column + i, aCellPos.Row + j );
                            }
                        }
                        else
                            if ((i == 0) && (aCellPos.Column == 0))
                                for (sal_Int32 j = 1; j < nRepeatedRows; j++)
                                {
                                        rXMLImport.GetTables().AddRow();
                                        rXMLImport.GetTables().AddColumn(sal_False);
                                }
                    }
                    if (nCellsRepeated > 1 || nRepeatedRows > 1)
                    {
                        SetCellProperties(xCellRange, aCellPos);
                        SetType(xCellRange, aCellPos);
                    }
                    else
                    {
                        SetCellProperties(xTempCell);
                        SetType(xTempCell);
                    }
                }
                else
                {
                    uno::Reference <table::XCell> xCell = xCellRange->getCellByPosition(aCellPos.Column , aCellPos.Row);
                    SetCellProperties(xCell);
                    ParseFormula(sOUFormula);
                    if (!bIsMatrix)
                    {
                        xCell->setFormula(sOUFormula);
                    }
                    else
                    {
                        if (nMatrixCols > 0 && nMatrixRows > 0)
                        {
                            uno::Reference <table::XCellRange> xMatrixCellRange =
                                xCellRange->getCellRangeByPosition(aCellPos.Column, aCellPos.Row,
                                            aCellPos.Column + nMatrixCols - 1, aCellPos.Row + nMatrixRows - 1);
                            if (xMatrixCellRange.is())
                            {
                                uno::Reference <sheet::XArrayFormulaRange> xArrayFormulaRange(xMatrixCellRange, uno::UNO_QUERY);
                                if (xArrayFormulaRange.is())
                                {
                                    xArrayFormulaRange->setArrayFormula(sOUFormula);
                                }
                            }
                        }
                    }
                    if (bHasAnnotation)
                        SetAnnotation(xCell);
                    if( aCellRangeSource.bHas )
                        SetCellRangeSource( aCellPos.Sheet, aCellPos.Column, aCellPos.Row );
//2do: results of formula cells should be stored in XML and set upon reading
                    // No API implemented because this is the only place where needed ever, isn't it?
                    //! QAD HACK! be sure that getCellByPosition() really returned a ScCellObj
                    ScCellObj* pCellObj = (ScCellObj*) ScCellRangesBase::getImplementation( xCell );
                    if ( pCellObj )
                    {
                        if ( FALSE )
                        {
//2do: make it real
                        BOOL bIsStringResult = TRUE;
                        if ( bIsStringResult )
                            pCellObj->SetFormulaResultString( sOUText );
                        else
                            pCellObj->SetFormulaResultDouble( fValue );
                        }
                    }
                }
            }
        }
    }
    bIsMerged = sal_False;
    bHasSubTable = sal_False;
    nMergedCols = 1;
    nMergedRows = 1;
    nCellsRepeated = 1;
}
