/*************************************************************************
 *
 *  $RCSfile: XMLExportDatabaseRanges.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-17 11:30:16 $
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

#ifndef SC_XMLEXPORTDATABASERANGES_HXX
#include "XMLExportDatabaseRanges.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif
#ifndef _SC_XMLEXPORTITERATOR_HXX
#include "XMLExportIterator.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_DBCOLECT_HXX
#include "dbcolect.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif
#ifndef SC_XMLEXPORTSHAREDDATA_HXX
#include "XMLExportSharedData.hxx"
#endif

#ifndef _COM_SUN_STAR_SHEET_DATAIMPORTMODE_HPP_
#include <com/sun/star/sheet/DataImportMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLESORTFIELD_HPP_
#include <com/sun/star/table/TableSortField.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLESORTFIELDTYPE_HPP_
#include <com/sun/star/table/TableSortFieldType.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSUBTOTALFIELD_HPP_
#include <com/sun/star/sheet/XSubTotalField.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATABASERANGES_HPP_
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATABASERANGE_HPP_
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEORIENTATION_HPP_
#include <com/sun/star/table/TableOrientation.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

//! not found in unonames.hxx
#define SC_USERLIST "UserList"
#define SC_SORTASCENDING "SortAscending"
#define SC_ENABLEUSERSORTLIST "EnableUserSortList"
#define SC_USERSORTLISTINDEX "UserSortListIndex"

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLExportDatabaseRanges::ScXMLExportDatabaseRanges(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pDoc( NULL )
{
}

ScXMLExportDatabaseRanges::~ScXMLExportDatabaseRanges()
{
}

ScMyEmptyDatabaseRangesContainer ScXMLExportDatabaseRanges::GetEmptyDatabaseRanges()
{
    ScMyEmptyDatabaseRangesContainer aSkipRanges;
    if (rExport.GetModel().is())
    {
        sal_Int32 nSkipRangesCount = 0;
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rExport.GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
            if (xPropertySet.is())
            {
                uno::Any aDatabaseRanges = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_DATABASERNG)));
                uno::Reference <sheet::XDatabaseRanges> xDatabaseRanges;
                rExport.CheckAttrList();
                if (aDatabaseRanges >>= xDatabaseRanges)
                {
                    uno::Sequence <rtl::OUString> aRanges = xDatabaseRanges->getElementNames();
                    sal_Int32 nDatabaseRangesCount = aRanges.getLength();
                    for (sal_Int32 i = 0; i < nDatabaseRangesCount; i++)
                    {
                        rtl::OUString sDatabaseRangeName = aRanges[i];
                        uno::Any aDatabaseRange = xDatabaseRanges->getByName(sDatabaseRangeName);
                        uno::Reference <sheet::XDatabaseRange> xDatabaseRange;
                        if (aDatabaseRange >>= xDatabaseRange)
                        {
                            uno::Reference <beans::XPropertySet> xDatabaseRangePropertySet (xDatabaseRange, uno::UNO_QUERY);
                            if (xDatabaseRangePropertySet.is())
                            {
                                uno::Any aStripDataProperty = xDatabaseRangePropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_STRIPDAT)));
                                sal_Bool bStripData = sal_False;
                                if (aStripDataProperty >>= bStripData)
                                    if (bStripData)
                                    {
                                        uno::Sequence <beans::PropertyValue> aImportProperties = xDatabaseRange->getImportDescriptor();
                                        sal_Int32 nLength = aImportProperties.getLength();
                                        sheet::DataImportMode nSourceType = sheet::DataImportMode_NONE;
                                        for (sal_Int32 j = 0; j < nLength; j++)
                                            if (aImportProperties[j].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SRCTYPE)))
                                            {
                                                uno::Any aSourceType = aImportProperties[j].Value;
                                                aSourceType >>= nSourceType;
                                            }
                                        if (nSourceType != sheet::DataImportMode_NONE)
                                        {
                                            table::CellRangeAddress aArea = xDatabaseRange->getDataArea();
                                            aSkipRanges.AddNewEmptyDatabaseRange(aArea);

                                            // #105276#; set last row/column so default styles are collected
                                            rExport.GetSharedData()->SetLastColumn(aArea.Sheet, aArea.EndColumn);
                                            rExport.GetSharedData()->SetLastRow(aArea.Sheet, aArea.EndRow);
                                        }
                                    }
                            }
                        }
                    }
                    if (nSkipRangesCount > 1)
                        aSkipRanges.Sort();
                }
            }
        }
    }
    return aSkipRanges;
}

void ScXMLExportDatabaseRanges::WriteImportDescriptor(const uno::Sequence <beans::PropertyValue> aImportDescriptor)
{
    sal_Int32 nProperties = aImportDescriptor.getLength();
    rtl::OUString sDatabaseName;
    rtl::OUString sSourceObject;
    sheet::DataImportMode nSourceType;
    sal_Bool bNative;
    for (sal_Int16 i = 0; i < nProperties; i++)
    {
        if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_DBNAME)))
        {
            uno::Any aDatabaseName = aImportDescriptor[i].Value;
            aDatabaseName >>= sDatabaseName;
        }
        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SRCOBJ)))
        {
            uno::Any aSourceObject = aImportDescriptor[i].Value;
            aSourceObject >>= sSourceObject;
        }
        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SRCTYPE)))
        {
            uno::Any aSourceType = aImportDescriptor[i].Value;
            aSourceType >>= nSourceType;
        }
        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NATIVE)))
        {
            uno::Any aNative = aImportDescriptor[i].Value;
            aNative >>= bNative;
        }
    }
    switch (nSourceType)
    {
        case sheet::DataImportMode_NONE : break;
        case sheet::DataImportMode_QUERY :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, sDatabaseName);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_QUERY_NAME, sSourceObject);
            SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_QUERY, sal_True, sal_True);
            rExport.CheckAttrList();
        }
        break;
        case sheet::DataImportMode_TABLE :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, sDatabaseName);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, sSourceObject);
            SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_TABLE, sal_True, sal_True);
            rExport.CheckAttrList();
        }
        break;
        case sheet::DataImportMode_SQL :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATABASE_NAME, sDatabaseName);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_SQL_STATEMENT, sSourceObject);
            if (!bNative)
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PARSE_SQL_STATEMENT, XML_TRUE);
            SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_SOURCE_SQL, sal_True, sal_True);
            rExport.CheckAttrList();
        }
        break;
    }
}

rtl::OUString ScXMLExportDatabaseRanges::getOperatorXML(const sheet::FilterOperator aFilterOperator, const sal_Bool bUseRegularExpressions) const
{
    switch (aFilterOperator)
    {
        case sheet::FilterOperator_EQUAL :
        {
            if (bUseRegularExpressions)
                return GetXMLToken(XML_MATCH);
            else
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
        }
        break;
        case sheet::FilterOperator_NOT_EQUAL :
        {
            if (bUseRegularExpressions)
                return GetXMLToken(XML_NOMATCH);
            else
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
        }
        break;
        case sheet::FilterOperator_BOTTOM_PERCENT :
            return GetXMLToken(XML_BOTTOM_PERCENT);
            break;
        case sheet::FilterOperator_BOTTOM_VALUES :
            return GetXMLToken(XML_BOTTOM_VALUES);
            break;
        case sheet::FilterOperator_EMPTY :
            return GetXMLToken(XML_EMPTY);
            break;
        case sheet::FilterOperator_GREATER :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
            break;
        case sheet::FilterOperator_GREATER_EQUAL :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
            break;
        case sheet::FilterOperator_LESS :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
            break;
        case sheet::FilterOperator_LESS_EQUAL :
            return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
            break;
        case sheet::FilterOperator_NOT_EMPTY :
            return GetXMLToken(XML_NOEMPTY);
            break;
        case sheet::FilterOperator_TOP_PERCENT :
            return GetXMLToken(XML_TOP_PERCENT);
            break;
        case sheet::FilterOperator_TOP_VALUES :
            return GetXMLToken(XML_TOP_VALUES);
            break;
    }
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
}

void ScXMLExportDatabaseRanges::WriteCondition(const sheet::TableFilterField& aFilterField, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, rtl::OUString::valueOf(aFilterField.Field));
    if (bIsCaseSensitive)
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);
    if (aFilterField.IsNumeric)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_NUMBER);
        rtl::OUStringBuffer sBuffer;
        rExport.GetMM100UnitConverter().convertDouble(sBuffer, aFilterField.NumericValue);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, sBuffer.makeStringAndClear());
    }
    else
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, aFilterField.StringValue);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_OPERATOR, getOperatorXML(aFilterField.Operator, bUseRegularExpressions));
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_CONDITION, sal_True, sal_True);
}

void ScXMLExportDatabaseRanges::WriteFilterDescriptor(const uno::Reference <sheet::XSheetFilterDescriptor>& xSheetFilterDescriptor, const rtl::OUString sDatabaseRangeName)
{
    uno::Sequence <sheet::TableFilterField> aTableFilterFields = xSheetFilterDescriptor->getFilterFields();
    sal_Int32 nTableFilterFields = aTableFilterFields.getLength();
    if (nTableFilterFields > 0)
    {
        uno::Reference <beans::XPropertySet> xPropertySet (xSheetFilterDescriptor, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            sal_Bool bCopyOutputData;
            uno::Any aCopyOutputData = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COPYOUT)));
            if (aCopyOutputData >>= bCopyOutputData)
                if (bCopyOutputData)
                {
                    table::CellAddress aOutputPosition;
                    uno::Any aTempOutputPosition = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_OUTPOS)));
                    if (aTempOutputPosition >>= aOutputPosition)
                    {
                        rtl::OUString sOUCellAddress;
                        ScXMLConverter::GetStringFromAddress( sOUCellAddress, aOutputPosition, pDoc );
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, sOUCellAddress);
                    }
                }
            ScDBCollection* pDBCollection = pDoc->GetDBCollection();
            sal_uInt16 nIndex;
            pDBCollection->SearchName(sDatabaseRangeName, nIndex);
            ScDBData* pDBData = (*pDBCollection)[nIndex];
            ScRange aAdvSource;
            if (pDBData->GetAdvancedQuerySource(aAdvSource))
            {
                rtl::OUString sOUCellAddress;
                ScXMLConverter::GetStringFromRange( sOUCellAddress, aAdvSource, pDoc );
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS, sOUCellAddress);
            }

            sal_Bool bSkipDuplicates;
            uno::Any aSkipDuplicates = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SKIPDUP)));
            if (aSkipDuplicates >>= bSkipDuplicates)
                if (bSkipDuplicates)
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_DUPLICATES, XML_FALSE);
            SvXMLElementExport aElemF(rExport, XML_NAMESPACE_TABLE, XML_FILTER, sal_True, sal_True);
            rExport.CheckAttrList();
            sal_Bool bIsCaseSensitive = sal_False;
            uno::Any aIsCaseSensitive = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISCASE)));
            aIsCaseSensitive >>= bIsCaseSensitive;
            sal_Bool bUseRegularExpressions = sal_False;
            uno::Any aUseRegularExpressions = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_USEREGEX)));
            aUseRegularExpressions >>= bUseRegularExpressions;
            sal_Bool bAnd = sal_False;
            sal_Bool bOr = sal_False;
            for (sal_Int32 i = 1; i < nTableFilterFields; i++)
            {
                if (aTableFilterFields[i].Connection == sheet::FilterConnection_AND)
                    bAnd = sal_True;
                else
                    bOr = sal_True;
            }
            if (bOr && !bAnd)
            {
                SvXMLElementExport aElemOr(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, sal_True, sal_True);
                for (i = 0; i < nTableFilterFields; i++)
                {
                    WriteCondition(aTableFilterFields[i], bIsCaseSensitive, bUseRegularExpressions);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(rExport, XML_NAMESPACE_TABLE, XML_FILTER_AND, sal_True, sal_True);
                for (i = 0; i < nTableFilterFields; i++)
                {
                    WriteCondition(aTableFilterFields[i], bIsCaseSensitive, bUseRegularExpressions);
                }
            }
            else if (nTableFilterFields  == 1)
            {
                WriteCondition(aTableFilterFields[0], bIsCaseSensitive, bUseRegularExpressions);
            }
            else
            {
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_FILTER_OR, sal_True, sal_True);
                sheet::TableFilterField aPrevFilterField = aTableFilterFields[0];
                sheet::FilterConnection aConnection = aTableFilterFields[1].Connection;
                sal_Bool bOpenAndElement;
                rtl::OUString aName = rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XML_FILTER_AND));
                if (aConnection == sheet::FilterConnection_AND)
                {
                    rExport.StartElement( aName, sal_True);
                    bOpenAndElement = sal_True;
                }
                else
                    bOpenAndElement = sal_False;
                for (i = 1; i < nTableFilterFields; i++)
                {
                    if (aConnection != aTableFilterFields[i].Connection)
                    {
                        aConnection = aTableFilterFields[i].Connection;
                        if (aTableFilterFields[i].Connection == sheet::FilterConnection_AND)
                        {
                            rExport.StartElement( aName, sal_True );
                            bOpenAndElement = sal_True;
                            WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                            aPrevFilterField = aTableFilterFields[i];
                            if (i == nTableFilterFields - 1)
                            {
                                WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                                rExport.EndElement(aName, sal_True);
                                bOpenAndElement = sal_False;
                            }
                        }
                        else
                        {
                            WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                            aPrevFilterField = aTableFilterFields[i];
                            if (bOpenAndElement)
                            {
                                rExport.EndElement(aName, sal_True);
                                bOpenAndElement = sal_False;
                            }
                            if (i == nTableFilterFields - 1)
                            {
                                WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                            }
                        }
                    }
                    else
                    {
                        WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                        aPrevFilterField = aTableFilterFields[i];
                        if (i == nTableFilterFields - 1)
                            WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                    }
                }
            }
        }
    }
}

void ScXMLExportDatabaseRanges::WriteSortDescriptor(const uno::Sequence <beans::PropertyValue> aSortProperties)
{
    uno::Sequence <table::TableSortField> aSortFields;
    sal_Bool bBindFormatsToContent (sal_True);
    sal_Bool bCopyOutputData (sal_False);
    sal_Bool bIsCaseSensitive (sal_False);
    sal_Bool bIsUserListEnabled (sal_False);
    table::CellAddress aOutputPosition;
    sal_Int32 nUserListIndex;
    sal_Int32 nProperties = aSortProperties.getLength();
    for (sal_Int32 i = 0; i < nProperties; i++)
    {
        if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_BINDFMT) == 0)
        {
            uno::Any aBindFormatsToContent = aSortProperties[i].Value;
            aBindFormatsToContent >>= bBindFormatsToContent;
        }
        else if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_COPYOUT) == 0)
        {
            uno::Any aCopyOutputData = aSortProperties[i].Value;
            aCopyOutputData >>= bCopyOutputData;
        }
//      no longer supported
/*      else if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_ISCASE) == 0)
        {
            uno::Any aIsCaseSensitive = aSortProperties[i].Value;
            aIsCaseSensitive >>= bIsCaseSensitive;
        }*/
        else if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_ISULIST) == 0)
        {
            uno::Any aIsUserListEnabled = aSortProperties[i].Value;
            aIsUserListEnabled >>= bIsUserListEnabled;
        }
        else if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_OUTPOS) == 0)
        {
            uno::Any aTempOutputPosition = aSortProperties[i].Value;
            aTempOutputPosition >>= aOutputPosition;
        }
        else if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_UINDEX) == 0)
        {
            uno::Any aUserListIndex = aSortProperties[i].Value;
            aUserListIndex >>= nUserListIndex;
        }
        else if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_SORTFLD) == 0)
        {
            uno::Any aTempSortFields = aSortProperties[i].Value;
            aTempSortFields >>= aSortFields;
        }
//      no longer supported
/*      else if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_COLLLOC) == 0)
        {
            uno::Any aTemp = aSortProperties[i].Value;
            aTemp >>= aCollatorLocale;
        }
        else if (aSortProperties[i].Name.compareToAscii(SC_UNONAME_COLLALG) == 0)
        {
            uno::Any aTemp = aSortProperties[i].Value;
            aTemp >>= sCollatorAlgorithm;
        }*/
    }
    sal_Int32 nSortFields = aSortFields.getLength();
    if (nSortFields > 0)
    {
        if (!bBindFormatsToContent)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT, XML_FALSE);
        if (bCopyOutputData)
        {
            rtl::OUString sOUCellAddress;
            ScXMLConverter::GetStringFromAddress( sOUCellAddress, aOutputPosition, pDoc );
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, sOUCellAddress);
        }
//      no longer supported
//      if (bIsCaseSensitive)
//          rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);

        if (aSortFields[0].IsCaseSensitive)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);
#ifndef PRODUCT
        sal_Bool bCaseSensitive(aSortFields[0].IsCaseSensitive);
        for (i = 1; i < nSortFields; i++)
        {
            DBG_ASSERT(bCaseSensitive == aSortFields[i].IsCaseSensitive, "seems that it is now possible to have every field case sensitive");
        }
#endif
//      no longer supported
/*      if (aCollatorLocale.Language.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_LANGUAGE, aCollatorLocale.Language);
        if (aCollatorLocale.Country.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_COUNTRY, aCollatorLocale.Country);
        if (sCollatorAlgorithm.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ALGORITHM, sCollatorAlgorithm);*/
        if (aSortFields[0].CollatorLocale.Language.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_LANGUAGE, aSortFields[0].CollatorLocale.Language);
        if (aSortFields[0].CollatorLocale.Country.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_COUNTRY, aSortFields[0].CollatorLocale.Country);
        if (aSortFields[0].CollatorAlgorithm.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ALGORITHM, aSortFields[0].CollatorAlgorithm);
#ifndef PRODUCT
        rtl::OUString sLanguage(aSortFields[0].CollatorLocale.Language);
        rtl::OUString sCountry(aSortFields[0].CollatorLocale.Country);
        rtl::OUString sAlgorithm(aSortFields[0].CollatorAlgorithm);
        for (i = 1; i < nSortFields; i++)
        {
            DBG_ASSERT(sLanguage == aSortFields[i].CollatorLocale.Language, "seems that it is now possible to have every field localized");
            DBG_ASSERT(sCountry == aSortFields[i].CollatorLocale.Country, "seems that it is now possible to have every field localized");
            DBG_ASSERT(sAlgorithm == aSortFields[i].CollatorAlgorithm, "seems that it is now possible to have every field localized");
        }
#endif
        SvXMLElementExport aElemS(rExport, XML_NAMESPACE_TABLE, XML_SORT, sal_True, sal_True);
        rExport.CheckAttrList();
        for (i = 0; i < nSortFields; i++)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, rtl::OUString::valueOf(aSortFields[i].Field));
            if (!aSortFields[i].IsAscending)
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORDER, XML_DESCENDING);
            if (!bIsUserListEnabled)
            {
                switch (aSortFields[i].FieldType)
                {
                    case table::TableSortFieldType_ALPHANUMERIC :
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_TEXT);
                    break;
                    case table::TableSortFieldType_AUTOMATIC :
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_AUTOMATIC);
                    break;
                    case table::TableSortFieldType_NUMERIC :
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, XML_NUMBER);
                    break;
                }
            }
            else
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERLIST)) + rtl::OUString::valueOf(nUserListIndex));
            SvXMLElementExport aElemSb(rExport, XML_NAMESPACE_TABLE, XML_SORT_BY, sal_True, sal_True);
            rExport.CheckAttrList();
        }
    }
}

void ScXMLExportDatabaseRanges::WriteSubTotalDescriptor(const com::sun::star::uno::Reference <com::sun::star::sheet::XSubTotalDescriptor> xSubTotalDescriptor, const rtl::OUString sDatabaseRangeName)
{
    uno::Reference <container::XIndexAccess> xIndexAccess (xSubTotalDescriptor, uno::UNO_QUERY);
    if (xIndexAccess.is())
    {
        sal_Int32 nSubTotalFields = xIndexAccess->getCount();
        if (nSubTotalFields > 0)
        {
            uno::Reference <beans::XPropertySet> xPropertySet (xSubTotalDescriptor, uno::UNO_QUERY);
            sal_Bool bEnableUserSortList = sal_False;
            sal_Bool bSortAscending = sal_True;
            sal_Int32 nUserSortListIndex = 0;
            if (xPropertySet.is())
            {
                sal_Bool bBindFormatsToContent;
                uno::Any aBindFormatsToContent = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_BINDFMT)));
                if (aBindFormatsToContent >>= bBindFormatsToContent)
                    if (!bBindFormatsToContent)
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_BIND_STYLES_TO_CONTENT, XML_FALSE);
                sal_Bool bInsertPageBreaks;
                uno::Any aInsertPageBreaks = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INSBRK)));
                if (aInsertPageBreaks >>= bInsertPageBreaks)
                    if (bInsertPageBreaks)
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PAGE_BREAKS_ON_GROUP_CHANGE, XML_TRUE);
                sal_Bool bIsCaseSensitive;
                uno::Any aIsCaseSensitive = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISCASE)));
                if (aIsCaseSensitive >>= bIsCaseSensitive)
                    if (bIsCaseSensitive)
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_TRUE);
                uno::Any aSortAscending = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_SORTASCENDING)));
                aSortAscending >>= bSortAscending;
                uno::Any aEnabledUserSortList = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ENABLEUSERSORTLIST)));
                if (aEnabledUserSortList >>= bEnableUserSortList)
                    if (bEnableUserSortList)
                    {
                        uno::Any aUserSortListIndex = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERSORTLISTINDEX)));
                        aUserSortListIndex >>= nUserSortListIndex;
                    }
            }
            SvXMLElementExport aElemSTRs(rExport, XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULES, sal_True, sal_True);
            rExport.CheckAttrList();
            {
                ScDBCollection* pDBCollection = pDoc->GetDBCollection();
                sal_uInt16 nIndex;
                pDBCollection->SearchName(sDatabaseRangeName, nIndex);
                ScDBData* pDBData = (*pDBCollection)[nIndex];
                ScSubTotalParam aSubTotalParam;
                pDBData->GetSubTotalParam(aSubTotalParam);
                if (aSubTotalParam.bDoSort)
                {
                    if (!aSubTotalParam.bAscending)
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORDER, XML_DESCENDING);
                    if (aSubTotalParam.bUserDef)
                    {
                        rtl::OUString sUserList = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERLIST));
                        sUserList += rtl::OUString::valueOf(aSubTotalParam.nUserIndex);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATA_TYPE, sUserList);
                    }
                    SvXMLElementExport aElemSGs(rExport, XML_NAMESPACE_TABLE, XML_SORT_GROUPS, sal_True, sal_True);
                    rExport.CheckAttrList();
                }
            }
            for (sal_Int32 i = 0; i < nSubTotalFields; i++)
            {
                uno::Reference <sheet::XSubTotalField> xSubTotalField;
                uno::Any aSubTotalField = xIndexAccess->getByIndex(i);
                if (aSubTotalField >>= xSubTotalField)
                {
                    sal_Int32 nGroupColumn = xSubTotalField->getGroupColumn();
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_GROUP_BY_FIELD_NUMBER, rtl::OUString::valueOf(nGroupColumn));
                    SvXMLElementExport aElemSTR(rExport, XML_NAMESPACE_TABLE, XML_SUBTOTAL_RULE, sal_True, sal_True);
                    rExport.CheckAttrList();
                    uno::Sequence <sheet::SubTotalColumn> aSubTotalColumns = xSubTotalField->getSubTotalColumns();
                    sal_Int32 nSubTotalColumns = aSubTotalColumns.getLength();
                    for (sal_Int32 j = 0; j < nSubTotalColumns; j++)
                    {
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FIELD_NUMBER, rtl::OUString::valueOf(aSubTotalColumns[j].Column));
                        rtl::OUString sFunction;
                        ScXMLConverter::GetStringFromFunction( sFunction, aSubTotalColumns[j].Function );
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FUNCTION, sFunction);
                        SvXMLElementExport aElemSTF(rExport, XML_NAMESPACE_TABLE, XML_SUBTOTAL_FIELD, sal_True, sal_True);
                        rExport.CheckAttrList();
                    }
                }
            }
        }
    }
}

void ScXMLExportDatabaseRanges::WriteDatabaseRanges(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    pDoc = rExport.GetDocument();
    if (pDoc)
    {
        uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Any aDatabaseRanges = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_DATABASERNG)));
            uno::Reference <sheet::XDatabaseRanges> xDatabaseRanges;
            rExport.CheckAttrList();
            if (aDatabaseRanges >>= xDatabaseRanges)
            {
                uno::Sequence <rtl::OUString> aRanges = xDatabaseRanges->getElementNames();
                sal_Int32 nDatabaseRangesCount = aRanges.getLength();
                if (nDatabaseRangesCount > 0)
                {
                    SvXMLElementExport aElemDRs(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_RANGES, sal_True, sal_True);
                    for (sal_Int32 i = 0; i < nDatabaseRangesCount; i++)
                    {
                        rtl::OUString sDatabaseRangeName = aRanges[i];
                        uno::Any aDatabaseRange = xDatabaseRanges->getByName(sDatabaseRangeName);
                        uno::Reference <sheet::XDatabaseRange> xDatabaseRange;
                        if (aDatabaseRange >>= xDatabaseRange)
                        {
                            String sUnbenannt = ScGlobal::GetRscString(STR_DB_NONAME);
                            rtl::OUString sOUUnbenannt (sUnbenannt);
                            if (sOUUnbenannt != sDatabaseRangeName)
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, sDatabaseRangeName);
                            table::CellRangeAddress aRangeAddress = xDatabaseRange->getDataArea();
                            rtl::OUString sOUAddress;
                            ScXMLConverter::GetStringFromRange( sOUAddress, aRangeAddress, pDoc );
                            rExport.AddAttribute (XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, sOUAddress);
                            ScDBCollection* pDBCollection = pDoc->GetDBCollection();
                            sal_uInt16 nIndex;
                            pDBCollection->SearchName(sDatabaseRangeName, nIndex);
                            ScDBData* pDBData = (*pDBCollection)[nIndex];
                            if (pDBData->HasImportSelection())
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_IS_SELECTION, XML_TRUE);
                            if (pDBData->HasAutoFilter())
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_FILTER_BUTTONS, XML_TRUE);
                            uno::Reference <beans::XPropertySet> xPropertySetDatabaseRange (xDatabaseRange, uno::UNO_QUERY);
                            if (xPropertySetDatabaseRange.is())
                            {
                                uno::Any aKeepFormatsProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_KEEPFORM)));
                                sal_Bool bKeepFormats = sal_False;
                                if (aKeepFormatsProperty >>= bKeepFormats)
                                    if (bKeepFormats)
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_STYLES, XML_TRUE);
                                uno::Any aMoveCellsProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_MOVCELLS)));
                                sal_Bool bMoveCells = sal_False;
                                if (aMoveCellsProperty >>= bMoveCells)
                                    if (bMoveCells)
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ON_UPDATE_KEEP_SIZE, XML_FALSE);
                                uno::Any aStripDataProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_STRIPDAT)));
                                sal_Bool bStripData = sal_False;
                                if (aStripDataProperty >>= bStripData)
                                    if (bStripData)
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_HAS_PERSISTENT_DATA, XML_FALSE);
                            }
                            uno::Reference <sheet::XSheetFilterDescriptor> xSheetFilterDescriptor = xDatabaseRange->getFilterDescriptor();
                            uno::Sequence <beans::PropertyValue> aSortProperties = xDatabaseRange->getSortDescriptor();
                            if (xSheetFilterDescriptor.is())
                            {
                                uno::Reference <beans::XPropertySet> xFilterProperties (xSheetFilterDescriptor, uno::UNO_QUERY);
                                if (xFilterProperties.is())
                                {
                                    uno::Any aContainsHeaderProperty = xFilterProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CONTHDR)));
                                    sal_Bool bContainsHeader = sal_True;
                                    if (aContainsHeaderProperty >>= bContainsHeader)
                                        if (!bContainsHeader)
                                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONTAINS_HEADER, XML_FALSE);

                                    // #98317#; there is no orientation on the filter
/*                                  uno::Any aOrientationProperty = xFilterProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ORIENT)));
                                    sal_Bool bOrientation = sal_False;
                                    table::TableOrientation eFilterOrient(table::TableOrientation_ROWS);
                                    if (aOrientationProperty >>= bOrientation)
                                        if (bOrientation)
                                            eFilterOrient = table::TableOrientation_ROWS;*/

                                    sal_Bool bSortColumns(sal_True);
                                    sal_Bool bFound(sal_False);
                                    sal_uInt32 nProperty(0);
                                    while (!bFound && (nProperty < aSortProperties.getLength()))
                                    {
                                        if (aSortProperties[nProperty].Name.compareToAscii(SC_UNONAME_ISSORTCOLUMNS) == 0)
                                        {
                                            bSortColumns = ::cppu::any2bool(aSortProperties[nProperty].Value);
                                            bFound = sal_True;
                                        }
                                        else
                                            ++nProperty;
                                    }

                                    if (bSortColumns)
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ORIENTATION, XML_COLUMN);
                                }
                            }
                            sal_Int32 nRefresh( pDBData->GetRefreshDelay() );
                            if( nRefresh )
                            {
                                rtl::OUStringBuffer sBuffer;
                                SvXMLUnitConverter::convertTime( sBuffer, (double)nRefresh / 86400 );
                                rExport.AddAttribute( XML_NAMESPACE_TABLE, XML_REFRESH_DELAY, sBuffer.makeStringAndClear() );
                            }
                            SvXMLElementExport aElemDR(rExport, XML_NAMESPACE_TABLE, XML_DATABASE_RANGE, sal_True, sal_True);
                            rExport.CheckAttrList();
                            WriteImportDescriptor(xDatabaseRange->getImportDescriptor());
                            if (xSheetFilterDescriptor.is())
                                WriteFilterDescriptor(xSheetFilterDescriptor, sDatabaseRangeName);
                            WriteSortDescriptor(aSortProperties);
                            WriteSubTotalDescriptor(xDatabaseRange->getSubTotalDescriptor(), sDatabaseRangeName);
                        }
                    }
                }
            }
        }
    }
}
