/*************************************************************************
 *
 *  $RCSfile: XMLExportDatabaseRanges.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-27 14:27:40 $
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
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

#ifndef _COM_SUN_STAR_SHEET_DATAIMPORTMODE_HPP_
#include <com/sun/star/sheet/DataImportMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SORTFIELD_HPP_
#include <com/sun/star/util/SortField.hpp>
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

//! not found in unonames.hxx
#define SC_USERLIST "UserList"
#define SC_SORTASCENDING "SortAscending"
#define SC_ENABLEUSERSORTLIST "EnableUserSortList"
#define SC_USERSORTLISTINDEX "UserSortListIndex"

using namespace com::sun::star;

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
                                        aSkipRanges.AddNewEmptyDatabaseRange(xDatabaseRange->getDataArea());
                                }
                        }
                    }
                }
                if (nSkipRangesCount > 1)
                    aSkipRanges.Sort();
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
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, sDatabaseName);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_query_name, sSourceObject);
            SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, sXML_database_source_query, sal_True, sal_True);
            rExport.CheckAttrList();
        }
        break;
        case sheet::DataImportMode_TABLE :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, sDatabaseName);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_table_name, sSourceObject);
            SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, sXML_database_source_table, sal_True, sal_True);
            rExport.CheckAttrList();
        }
        break;
        case sheet::DataImportMode_SQL :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, sDatabaseName);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_sql_statement, sSourceObject);
            if (!bNative)
                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_parse_sql_statement, sXML_true);
            SvXMLElementExport aElemID(rExport, XML_NAMESPACE_TABLE, sXML_database_source_sql, sal_True, sal_True);
            rExport.CheckAttrList();
        }
        break;
    }
}

rtl::OUString ScXMLExportDatabaseRanges::getOperatorXML(const sheet::FilterOperator aFilterOperator, const sal_Bool bUseRegularExpressions) const
{
    if (bUseRegularExpressions)
    {
        switch (aFilterOperator)
        {
            case sheet::FilterOperator_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_match));
                break;
            case sheet::FilterOperator_NOT_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_nomatch));
                break;
        }
    }
    else
    {
        switch (aFilterOperator)
        {
            case sheet::FilterOperator_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                break;
            case sheet::FilterOperator_NOT_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
                break;
            case sheet::FilterOperator_BOTTOM_PERCENT :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom_percent));
                break;
            case sheet::FilterOperator_BOTTOM_VALUES :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom_values));
                break;
            case sheet::FilterOperator_EMPTY :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_empty));
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
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_noempty));
                break;
            case sheet::FilterOperator_TOP_PERCENT :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top_percent));
                break;
            case sheet::FilterOperator_TOP_VALUES :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top_values));
                break;
        }
    }
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
}

void ScXMLExportDatabaseRanges::WriteCondition(const sheet::TableFilterField& aFilterField, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_field_number, rtl::OUString::valueOf(aFilterField.Field));
    if (bIsCaseSensitive)
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_case_sensitive, sXML_true);
    if (aFilterField.IsNumeric)
    {
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_number);
        rtl::OUStringBuffer sBuffer;
        rExport.GetMM100UnitConverter().convertDouble(sBuffer, aFilterField.NumericValue);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_value, sBuffer.makeStringAndClear());
    }
    else
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_value, aFilterField.StringValue);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_operator, getOperatorXML(aFilterField.Operator, bUseRegularExpressions));
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_filter_condition, sal_True, sal_True);
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
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sOUCellAddress);
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
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sOUCellAddress);
            }

            sal_Bool bSkipDuplicates;
            uno::Any aSkipDuplicates = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SKIPDUP)));
            if (aSkipDuplicates >>= bSkipDuplicates)
                if (bSkipDuplicates)
                    rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display_duplicates, sXML_false);
            SvXMLElementExport aElemF(rExport, XML_NAMESPACE_TABLE, sXML_filter, sal_True, sal_True);
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
                SvXMLElementExport aElemOr(rExport, XML_NAMESPACE_TABLE, sXML_filter_or, sal_True, sal_True);
                for (i = 0; i < nTableFilterFields; i++)
                {
                    WriteCondition(aTableFilterFields[i], bIsCaseSensitive, bUseRegularExpressions);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(rExport, XML_NAMESPACE_TABLE, sXML_filter_and, sal_True, sal_True);
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
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_filter_or, sal_True, sal_True);
                sheet::TableFilterField aPrevFilterField = aTableFilterFields[0];
                sheet::FilterConnection aConnection = aTableFilterFields[1].Connection;
                sal_Bool bOpenAndElement;
                rtl::OUString aName = rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_filter_and)));
                if (aConnection == sheet::FilterConnection_AND)
                {
                    rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
                    rExport.GetDocHandler()->startElement( aName, rExport.GetXAttrList());
                    rExport.ClearAttrList();
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
                            rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
                            rExport.GetDocHandler()->startElement( aName, rExport.GetXAttrList());
                            rExport.ClearAttrList();
                            bOpenAndElement = sal_True;
                            WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                            aPrevFilterField = aTableFilterFields[i];
                            if (i == nTableFilterFields - 1)
                            {
                                WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                                rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
                                rExport.GetDocHandler()->endElement(aName);
                                bOpenAndElement = sal_False;
                            }
                        }
                        else
                        {
                            WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                            aPrevFilterField = aTableFilterFields[i];
                            if (bOpenAndElement)
                            {
                                rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
                                rExport.GetDocHandler()->endElement(aName);
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
    uno::Sequence <util::SortField> aSortFields;
    sal_Bool bBindFormatsToContent = sal_True;
    sal_Bool bCopyOutputData = sal_False;
    sal_Bool bIsCaseSensitive = sal_False;
    sal_Bool bIsUserListEnabled = sal_False;
    table::CellAddress aOutputPosition;
    sal_Int32 nUserListIndex;
    sal_Int32 nProperties = aSortProperties.getLength();
    for (sal_Int32 i = 0; i < nProperties; i++)
    {
        if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_BINDFMT)))
        {
            uno::Any aBindFormatsToContent = aSortProperties[i].Value;
            aBindFormatsToContent >>= bBindFormatsToContent;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COPYOUT)))
        {
            uno::Any aCopyOutputData = aSortProperties[i].Value;
            aCopyOutputData >>= bCopyOutputData;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISCASE)))
        {
            uno::Any aIsCaseSensitive = aSortProperties[i].Value;
            aIsCaseSensitive >>= bIsCaseSensitive;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISULIST)))
        {
            uno::Any aIsUserListEnabled = aSortProperties[i].Value;
            aIsUserListEnabled >>= bIsUserListEnabled;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_OUTPOS)))
        {
            uno::Any aTempOutputPosition = aSortProperties[i].Value;
            aTempOutputPosition >>= aOutputPosition;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_UINDEX)))
        {
            uno::Any aUserListIndex = aSortProperties[i].Value;
            aUserListIndex >>= nUserListIndex;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SORTFLD)))
        {
            uno::Any aTempSortFields = aSortProperties[i].Value;
            aTempSortFields >>= aSortFields;
        }
    }
    sal_Int32 nSortFields = aSortFields.getLength();
    if (nSortFields > 0)
    {
        if (!bBindFormatsToContent)
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_bind_styles_to_content, sXML_false);
        if (bCopyOutputData)
        {
            rtl::OUString sOUCellAddress;
            ScXMLConverter::GetStringFromAddress( sOUCellAddress, aOutputPosition, pDoc );
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sOUCellAddress);
        }
        if (bIsCaseSensitive)
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_case_sensitive, sXML_true);
        SvXMLElementExport aElemS(rExport, XML_NAMESPACE_TABLE, sXML_sort, sal_True, sal_True);
        rExport.CheckAttrList();
        for (i = 0; i < nSortFields; i++)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_field_number, rtl::OUString::valueOf(aSortFields[i].Field));
            if (!aSortFields[i].SortAscending)
                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_order, sXML_descending);
            if (!bIsUserListEnabled)
            {
                switch (aSortFields[i].FieldType)
                {
                    case util::SortFieldType_ALPHANUMERIC :
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_text);
                    break;
                    case util::SortFieldType_AUTOMATIC :
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_automatic);
                    break;
                    case util::SortFieldType_NUMERIC :
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_number);
                    break;
                }
            }
            else
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_data_type, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERLIST)) + rtl::OUString::valueOf(nUserListIndex));
            SvXMLElementExport aElemSb(rExport, XML_NAMESPACE_TABLE, sXML_sort_by, sal_True, sal_True);
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
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_bind_styles_to_content, sXML_false);
                sal_Bool bInsertPageBreaks;
                uno::Any aInsertPageBreaks = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INSBRK)));
                if (aInsertPageBreaks >>= bInsertPageBreaks)
                    if (bInsertPageBreaks)
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_page_breaks_on_group_change, sXML_true);
                sal_Bool bIsCaseSensitive;
                uno::Any aIsCaseSensitive = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISCASE)));
                if (aIsCaseSensitive >>= bIsCaseSensitive)
                    if (bIsCaseSensitive)
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_case_sensitive, sXML_true);
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
            SvXMLElementExport aElemSTRs(rExport, XML_NAMESPACE_TABLE, sXML_subtotal_rules, sal_True, sal_True);
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
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_order, sXML_descending);
                    if (aSubTotalParam.bUserDef)
                    {
                        rtl::OUString sUserList = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERLIST));
                        sUserList += rtl::OUString::valueOf(aSubTotalParam.nUserIndex);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_data_type, sUserList);
                    }
                    SvXMLElementExport aElemSGs(rExport, XML_NAMESPACE_TABLE, sXML_sort_groups, sal_True, sal_True);
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
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_group_by_field_number, rtl::OUString::valueOf(nGroupColumn));
                    SvXMLElementExport aElemSTR(rExport, XML_NAMESPACE_TABLE, sXML_subtotal_rule, sal_True, sal_True);
                    rExport.CheckAttrList();
                    uno::Sequence <sheet::SubTotalColumn> aSubTotalColumns = xSubTotalField->getSubTotalColumns();
                    sal_Int32 nSubTotalColumns = aSubTotalColumns.getLength();
                    for (sal_Int32 j = 0; j < nSubTotalColumns; j++)
                    {
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_field_number, rtl::OUString::valueOf(aSubTotalColumns[j].Column));
                        rtl::OUString sFunction;
                        ScXMLConverter::GetStringFromFunction( sFunction, aSubTotalColumns[j].Function );
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_function, sFunction);
                        SvXMLElementExport aElemSTF(rExport, XML_NAMESPACE_TABLE, sXML_subtotal_field, sal_True, sal_True);
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
                    SvXMLElementExport aElemDRs(rExport, XML_NAMESPACE_TABLE, sXML_database_ranges, sal_True, sal_True);
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
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_name, sDatabaseRangeName);
                            table::CellRangeAddress aRangeAddress = xDatabaseRange->getDataArea();
                            rtl::OUString sOUAddress;
                            ScXMLConverter::GetStringFromRange( sOUAddress, aRangeAddress, pDoc );
                            rExport.AddAttribute (XML_NAMESPACE_TABLE, sXML_target_range_address, sOUAddress);
                            ScDBCollection* pDBCollection = pDoc->GetDBCollection();
                            sal_uInt16 nIndex;
                            pDBCollection->SearchName(sDatabaseRangeName, nIndex);
                            ScDBData* pDBData = (*pDBCollection)[nIndex];
                            if (pDBData->HasImportSelection())
                                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_is_selection, sXML_true);
                            if (pDBData->HasAutoFilter())
                                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display_filter_buttons, sXML_true);
                            uno::Reference <beans::XPropertySet> xPropertySetDatabaseRange (xDatabaseRange, uno::UNO_QUERY);
                            if (xPropertySetDatabaseRange.is())
                            {
                                uno::Any aKeepFormatsProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_KEEPFORM)));
                                sal_Bool bKeepFormats = sal_False;
                                if (aKeepFormatsProperty >>= bKeepFormats)
                                    if (bKeepFormats)
                                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_on_update_keep_styles, sXML_true);
                                uno::Any aMoveCellsProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_MOVCELLS)));
                                sal_Bool bMoveCells = sal_False;
                                if (aMoveCellsProperty >>= bMoveCells)
                                    if (bMoveCells)
                                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_on_update_keep_size, sXML_false);
                                uno::Any aStripDataProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_STRIPDAT)));
                                sal_Bool bStripData = sal_False;
                                if (aStripDataProperty >>= bStripData)
                                    if (bStripData)
                                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_has_persistent_data, sXML_false);
                            }
                            uno::Reference <sheet::XSheetFilterDescriptor> xSheetFilterDescriptor = xDatabaseRange->getFilterDescriptor();
                            if (xSheetFilterDescriptor.is())
                            {
                                uno::Reference <beans::XPropertySet> xFilterProperties (xSheetFilterDescriptor, uno::UNO_QUERY);
                                if (xFilterProperties.is())
                                {
                                    uno::Any aContainsHeaderProperty = xFilterProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CONTHDR)));
                                    sal_Bool bContainsHeader = sal_True;
                                    if (aContainsHeaderProperty >>= bContainsHeader)
                                        if (!bContainsHeader)
                                            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_contains_header, sXML_false);
                                    uno::Any aOrientationProperty = xFilterProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ORIENT)));
                                    sal_Bool bOrientation = sal_False;
                                    if (aOrientationProperty >>= bOrientation)
                                        if (bOrientation)
                                            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_orientation, sXML_column);
                                }
                            }
                            SvXMLElementExport aElemDR(rExport, XML_NAMESPACE_TABLE, sXML_database_range, sal_True, sal_True);
                            rExport.CheckAttrList();
                            WriteImportDescriptor(xDatabaseRange->getImportDescriptor());
                            if (xSheetFilterDescriptor.is())
                                WriteFilterDescriptor(xSheetFilterDescriptor, sDatabaseRangeName);
                            WriteSortDescriptor(xDatabaseRange->getSortDescriptor());
                            WriteSubTotalDescriptor(xDatabaseRange->getSubTotalDescriptor(), sDatabaseRangeName);
                        }
                    }
                }
            }
        }
    }
}
