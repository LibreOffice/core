/*************************************************************************
 *
 *  $RCSfile: xmldrani.cxx,v $
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

#include "xmldrani.hxx"
#include "xmlimprt.hxx"
#include "xmlfilti.hxx"
#include "xmlsorti.hxx"
#include "document.hxx"
#include "globstr.hrc"
#include "docuno.hxx"
#include "dbcolect.hxx"
#include "datauno.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlkywd.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#define SC_DATABASERANGES "DatabaseRanges"
#define SC_KEEPFORMATS "KeepFormats"
#define SC_MOVECELLS "MoveCells"
#define SC_STRIPDATA "StripData"
#define SC_DATABASENAME "DatabaseName"
#define SC_SOURCEOBJECT "SourceObject"
#define SC_SOURCETYPE "SourceType"
#define SC_NATIVE "Native"
#define SC_CONTAINSHEADER "ContainsHeader"
#define SC_ORIENTATION "Orientation"
#define SC_COPYOUTPUTDATA "CopyOutputData"
#define SC_ISCASESENSITIVE "IsCaseSensitive"
#define SC_OUTPUTPOSITION "OutputPosition"
#define SC_SKIPDUPLICATES "SkipDuplicates"
#define SC_USEREGULAREXPRESSIONS "UseRegularExpressions"
#define SC_BINDFORMATSTOCONTENT "BindFormatstoContent"
#define SC_ENABLEUSERSORTLIST "EnableUserSortList"
#define SC_USERSORTLISTINDEX "UserSortListIndex"
#define SC_USERLIST "UserList"
#define SC_INSERTPAGEBREAKS "InsertPageBreaks"

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLDatabaseRangesContext::ScXMLDatabaseRangesContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    // has no attributes
}

ScXMLDatabaseRangesContext::~ScXMLDatabaseRangesContext()
{
}

SvXMLImportContext *ScXMLDatabaseRangesContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDatabaseRangesElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATABASE_RANGE :
        {
            pContext = new ScXMLDatabaseRangeContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDatabaseRangesContext::EndElement()
{
}

ScXMLDatabaseRangeContext::ScXMLDatabaseRangeContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bIsSelection(sal_False),
    bKeepFormats(sal_False),
    bMoveCells(sal_False),
    bStripData(sal_False),
    bOrientation(sal_False),
    bContainsHeader(sal_True),
    bAutoFilter(sal_False),
    bFilterConditionSourceRange(sal_False),
    bSubTotalsSortGroups(sal_False),
    bSubTotalsEnabledUserList(sal_False),
    bSubTotalsAscending(sal_True),
    bNative(sal_True),
    aSubTotalColumns(),
    aSortSequence()
{
    nSourceType = sheet::DataImportMode_NONE;
    String sUnbenannt = ScGlobal::GetRscString(STR_DB_NONAME);
    rtl::OUString sOUUnbenannt (sUnbenannt);
    sDatabaseRangeName = sOUUnbenannt;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DATABASE_RANGE_ATTR_NAME :
            {
                sDatabaseRangeName = sValue;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_IS_SELECTION :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)))
                    bIsSelection = sal_True;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_STYLES :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)))
                    bKeepFormats = sal_True;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_ON_UPDATE_KEEP_SIZE :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_false)))
                    bMoveCells = sal_True;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_HAS_PERSISTENT_DATA :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_false)))
                    bStripData = sal_True;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_ORIENTATION :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_column)))
                    bOrientation = sal_True;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_CONTAINS_HEADER :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_false)))
                    bContainsHeader = sal_False;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_DISPLAY_FILTER_BUTTONS :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)))
                    bAutoFilter = sal_True;
            }
            break;
            case XML_TOK_DATABASE_RANGE_ATTR_TARGET_RANGE_ADDRESS :
            {
                sRangeAddress = sValue;
            }
            break;
        }
    }
}

ScXMLDatabaseRangeContext::~ScXMLDatabaseRangeContext()
{
}

SvXMLImportContext *ScXMLDatabaseRangeContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDatabaseRangeElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DATABASE_RANGE_SOURCE_SQL :
        {
            pContext = new ScXMLSourceSQLContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_DATABASE_RANGE_SOURCE_TABLE :
        {
            pContext = new ScXMLSourceTableContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_DATABASE_RANGE_SOURCE_QUERY :
        {
            pContext = new ScXMLSourceQueryContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_FILTER :
        {
            pContext = new ScXMLFilterContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_SORT :
        {
            pContext = new ScXMLSortContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_DATABASE_RANGE_SUBTOTAL_RULES :
        {
            pContext = new ScXMLSubTotalRulesContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDatabaseRangeContext::EndElement()
{
    ScXMLImport& rXMLImport = GetScImport();
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( rXMLImport.GetModel(), uno::UNO_QUERY );
    ScModelObj* pDocObj = ScModelObj::getImplementation( xSpreadDoc );
    if ( pDocObj && xSpreadDoc.is() )
    {
        ScDocument* pDoc = pDocObj->GetDocument();
        uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Any aDatabaseRanges = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DATABASERANGES)));
            uno::Reference <sheet::XDatabaseRanges> xDatabaseRanges;
            if (aDatabaseRanges >>= xDatabaseRanges)
            {
                ScAddress aStartCellAddress;
                ScAddress aEndCellAddress;
                sal_Int16 i = 0;
                while ((sRangeAddress[i] != ':') && (i < sRangeAddress.getLength()))
                    i++;
                rtl::OUString sStartCellAddress = sRangeAddress.copy(0, i);
                rtl::OUString sEndCellAddress = sRangeAddress.copy(i + 1);
                aStartCellAddress.Parse(sStartCellAddress, pDoc);
                aEndCellAddress.Parse(sEndCellAddress, pDoc);
                table::CellRangeAddress aCellRangeAddress;
                aCellRangeAddress.StartColumn = aStartCellAddress.Col();
                aCellRangeAddress.EndColumn = aEndCellAddress.Col();
                aCellRangeAddress.StartRow = aStartCellAddress.Row();
                aCellRangeAddress.EndRow = aEndCellAddress.Row();
                aCellRangeAddress.Sheet = aStartCellAddress.Tab();
                xDatabaseRanges->addNewByName(sDatabaseRangeName, aCellRangeAddress);
                uno::Any aDatabaseRange = xDatabaseRanges->getByName(sDatabaseRangeName);
                uno::Reference <sheet::XDatabaseRange> xDatabaseRange;
                if (aDatabaseRange >>= xDatabaseRange)
                {
                    uno::Reference <beans::XPropertySet> xDatabaseRangePropertySet (xDatabaseRange, uno::UNO_QUERY);
                    if (xDatabaseRangePropertySet.is())
                    {
                        uno::Any aTempValue;
                        aTempValue <<= bKeepFormats;
                        xDatabaseRangePropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_KEEPFORMATS)), aTempValue);
                        aTempValue <<= bMoveCells;
                        xDatabaseRangePropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_MOVECELLS)), aTempValue);
                        aTempValue <<= bStripData;
                        xDatabaseRangePropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_STRIPDATA)), aTempValue);
                    }
                    uno::Sequence <beans::PropertyValue> aImportDescriptor = xDatabaseRange->getImportDescriptor();
                    sal_Int32 nImportProperties = aImportDescriptor.getLength();
                    for (sal_Int16 i = 0; i < nImportProperties; i++)
                    {
                        if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_DATABASENAME)))
                        {
                            uno::Any aDatabaseName;
                            aDatabaseName <<= sDatabaseName;
                            aImportDescriptor[i].Value = aDatabaseName;
                        }
                        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_SOURCEOBJECT)))
                        {
                            uno::Any aSourceObject;
                            aSourceObject <<= sSourceObject;
                            aImportDescriptor[i].Value = aSourceObject;
                        }
                        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_SOURCETYPE)))
                        {
                            uno::Any aSourceType;
                            aSourceType <<= nSourceType;
                            aImportDescriptor[i].Value = aSourceType;
                        }
                        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NATIVE)))
                        {
                            uno::Any aNative;
                            aNative <<= bNative;
                            aImportDescriptor[i].Value = aNative;
                        }
                    }
                    ScDBCollection* pDBCollection = pDoc->GetDBCollection();
                    sal_uInt16 nIndex;
                    pDBCollection->SearchName(sDatabaseRangeName, nIndex);
                    ScDBData* pDBData = (*pDBCollection)[nIndex];
                    pDBData->SetImportSelection(bIsSelection);
                    pDBData->SetAutoFilter(bAutoFilter);
                    ScImportParam aImportParam;
                    ScImportDescriptor::FillImportParam(aImportParam, aImportDescriptor);
                    pDBData->SetImportParam(aImportParam);
                    ScSortParam aSortParam;
                    ScSortDescriptor::FillSortParam(aSortParam, aSortSequence);
                    pDBData->SetSortParam(aSortParam);
                    uno::Reference <sheet::XSheetFilterDescriptor> xSheetFilterDescriptor = xDatabaseRange->getFilterDescriptor();
                    if (xSheetFilterDescriptor.is())
                    {
                        uno::Reference <beans::XPropertySet> xFilterPropertySet (xSheetFilterDescriptor, uno::UNO_QUERY);
                        if (xFilterPropertySet.is())
                        {
                            uno::Any aTemp;
                            aTemp <<= bOrientation;
                            xFilterPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ORIENTATION)), aTemp);
                            aTemp <<= bContainsHeader;
                            xFilterPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CONTAINSHEADER)), aTemp);
                            aTemp <<= bFilterCopyOutputData;
                            xFilterPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_COPYOUTPUTDATA)), aTemp);
                            aTemp <<= bFilterIsCaseSensitive;
                            xFilterPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISCASESENSITIVE)), aTemp);
                            aTemp <<= bFilterSkipDuplicates;
                            xFilterPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_SKIPDUPLICATES)), aTemp);
                            aTemp <<= bFilterUseRegularExpressions;
                            xFilterPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USEREGULAREXPRESSIONS)), aTemp);
                            aTemp <<= aFilterOutputPosition;
                            xFilterPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_OUTPUTPOSITION)), aTemp);
                            if (bFilterConditionSourceRange)
                            {
                                ScRange aAdvSource;
                                aAdvSource.aStart.SetCol(aFilterConditionSourceRangeAddress.StartColumn);
                                aAdvSource.aStart.SetRow(aFilterConditionSourceRangeAddress.StartColumn);
                                aAdvSource.aStart.SetTab(aFilterConditionSourceRangeAddress.Sheet);
                                aAdvSource.aEnd.SetCol(aFilterConditionSourceRangeAddress.EndColumn);
                                aAdvSource.aEnd.SetRow(aFilterConditionSourceRangeAddress.EndRow);
                                aAdvSource.aEnd.SetTab(aFilterConditionSourceRangeAddress.Sheet);
                                pDBData->SetAdvancedQuerySource(&aAdvSource);
                            }
                        }
                        xSheetFilterDescriptor->setFilterFields(aFilterFields);
                    }
                    uno::Reference <sheet::XSubTotalDescriptor> xSubTotalDescriptor = xDatabaseRange->getSubTotalDescriptor();
                    if (xSubTotalDescriptor.is())
                    {
                        uno::Reference <beans::XPropertySet> xSubTotalPropertySet (xSubTotalDescriptor, uno::UNO_QUERY);
                        if( xSubTotalPropertySet.is())
                        {
                            uno::Any aTemp;
                            aTemp <<= bSubTotalsBindFormatsToContent;
                            xSubTotalPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_BINDFORMATSTOCONTENT)), aTemp);
                            aTemp <<= bSubTotalsEnabledUserList;
                            xSubTotalPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ENABLEUSERSORTLIST)), aTemp);
                            aTemp <<= nSubTotalsUserListIndex;
                            xSubTotalPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERSORTLISTINDEX)), aTemp);
                            aTemp <<= bSubTotalsInsertPageBreaks;
                            xSubTotalPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_INSERTPAGEBREAKS)), aTemp);
                            aTemp <<= bSubTotalsIsCaseSensitive;
                            xSubTotalPropertySet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISCASESENSITIVE)), aTemp);
                        }
                        ScSubTotalParam aSubTotalParam;
                        aSubTotalParam.bDoSort = bSubTotalsSortGroups;
                        aSubTotalParam.bAscending = bSubTotalsAscending;
                        aSubTotalParam.bUserDef = bSubTotalsEnabledUserList;
                        aSubTotalParam.nUserIndex = nSubTotalsUserListIndex;
                        pDBData->SetSubTotalParam(aSubTotalParam);
                        xSubTotalDescriptor->addNew(aSubTotalColumns, nSubTotalRuleGroupFieldNumber);
                    }
                }
            }
        }
    }
}

ScXMLSourceSQLContext::ScXMLSourceSQLContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceSQLAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_SQL_ATTR_DATABASE_NAME :
            {
                pDatabaseRangeContext->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_SQL_ATTR_SQL_STATEMENT :
            {
                pDatabaseRangeContext->SetSourceObject(sValue);
            }
            break;
            case XML_TOK_SOURCE_SQL_ATTR_PARSE_SQL_STATEMENT :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)))
                    pDatabaseRangeContext->SetNative(sal_False);
                else
                    pDatabaseRangeContext->SetNative(sal_True);
            }
            break;
        }
    }
    pDatabaseRangeContext->SetSourceType(sheet::DataImportMode_SQL);
}

ScXMLSourceSQLContext::~ScXMLSourceSQLContext()
{
}

SvXMLImportContext *ScXMLSourceSQLContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceSQLContext::EndElement()
{
}

ScXMLSourceTableContext::ScXMLSourceTableContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceTableAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_TABLE_ATTR_DATABASE_NAME :
            {
                pDatabaseRangeContext->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_TABLE_ATTR_TABLE_NAME :
            {
                pDatabaseRangeContext->SetSourceObject(sValue);
            }
            break;
        }
    }
    pDatabaseRangeContext->SetSourceType(sheet::DataImportMode_TABLE);
}

ScXMLSourceTableContext::~ScXMLSourceTableContext()
{
}

SvXMLImportContext *ScXMLSourceTableContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceTableContext::EndElement()
{
}

ScXMLSourceQueryContext::ScXMLSourceQueryContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSourceQueryAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SOURCE_QUERY_ATTR_DATABASE_NAME :
            {
                pDatabaseRangeContext->SetDatabaseName(sValue);
            }
            break;
            case XML_TOK_SOURCE_QUERY_ATTR_QUERY_NAME :
            {
                pDatabaseRangeContext->SetSourceObject(sValue);
            }
            break;
        }
    }
    pDatabaseRangeContext->SetSourceType(sheet::DataImportMode_QUERY);
}

ScXMLSourceQueryContext::~ScXMLSourceQueryContext()
{
}

SvXMLImportContext *ScXMLSourceQueryContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSourceQueryContext::EndElement()
{
}

ScXMLSubTotalRulesContext::ScXMLSubTotalRulesContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetDatabaseRangeSubTotalRulesAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SUBTOTAL_RULES_ATTR_BIND_STYLES_TO_CONTENT :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)))
                    pDatabaseRangeContext->SetSubTotalsBindFormatsToContent(sal_True);
                else
                    pDatabaseRangeContext->SetSubTotalsBindFormatsToContent(sal_False);
            }
            break;
            case XML_TOK_SUBTOTAL_RULES_ATTR_CASE_SENSITIVE :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)))
                    pDatabaseRangeContext->SetSubTotalsIsCaseSensitive(sal_True);
                else
                    pDatabaseRangeContext->SetSubTotalsIsCaseSensitive(sal_False);
            }
            break;
            case XML_TOK_SUBTOTAL_RULES_ATTR_PAGE_BREAKS_ON_GROUP_CHANGE :
            {
                if (sValue == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)))
                    pDatabaseRangeContext->SetSubTotalsInsertPageBreaks(sal_True);
                else
                    pDatabaseRangeContext->SetSubTotalsInsertPageBreaks(sal_False);
            }
            break;
        }
    }
}

ScXMLSubTotalRulesContext::~ScXMLSubTotalRulesContext()
{
}

SvXMLImportContext *ScXMLSubTotalRulesContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetDatabaseRangeSubTotalRulesElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_SUBTOTAL_RULES_SORT_GROUPS :
        {
            pContext = new ScXMLSortGroupsContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pDatabaseRangeContext);
        }
        break;
        case XML_TOK_SUBTOTAL_RULES_SUBTOTAL_RULE :
        {
            pContext = new ScXMLSubTotalRuleContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pDatabaseRangeContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSubTotalRulesContext::EndElement()
{
}

ScXMLSortGroupsContext::ScXMLSortGroupsContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    pDatabaseRangeContext->SetSubTotalsSortGroups(sal_True);
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSubTotalRulesSortGroupsAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SORT_GROUPS_ATTR_DATA_TYPE :
            {
                if (sValue.getLength() > 8)
                {
                    rtl::OUString sTemp = sValue.copy(0, 8);
                    if (sTemp.compareToAscii(SC_USERLIST) == 0)
                    {
                        pDatabaseRangeContext->SetSubTotalsEnabledUserList(sal_True);
                        sTemp = sValue.copy(8);
                        pDatabaseRangeContext->SetSubTotalsUserListIndex(sTemp.toInt32());
                    }
                    else
                    {
                        //if (sValue.compareToAscii(sXML_automatic) == 0)
                            //aSortField.FieldType = util::SortFieldType_AUTOMATIC;
                            // is not supported by StarOffice
                    }
                }
                else
                {
                    //if (sValue.compareToAscii(sXML_text) == 0)
                        //aSortField.FieldType = util::SortFieldType_ALPHANUMERIC;
                        // is not supported by StarOffice
                    //else if (sValue.compareToAscii(sXML_number) == 0)
                        //aSortField.FieldType = util::SortFieldType_NUMERIC;
                        // is not supported by StarOffice
                }
            }
            break;
            case XML_TOK_SORT_GROUPS_ATTR_ORDER :
            {
                if (sValue.compareToAscii(sXML_ascending) == 0)
                    pDatabaseRangeContext->SetSubTotalsAscending(sal_True);
                else
                    pDatabaseRangeContext->SetSubTotalsAscending(sal_False);
            }
            break;
        }
    }
}

ScXMLSortGroupsContext::~ScXMLSortGroupsContext()
{
}

SvXMLImportContext *ScXMLSortGroupsContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSortGroupsContext::EndElement()
{
}

ScXMLSubTotalRuleContext::ScXMLSubTotalRuleContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSubTotalRulesSubTotalRuleAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SUBTOTAL_RULE_ATTR_GROUP_BY_FIELD_NUMBER :
            {
                pDatabaseRangeContext->SetSubTotalRuleGroupFieldNumber(sValue.toInt32());
            }
            break;
        }
    }
}

ScXMLSubTotalRuleContext::~ScXMLSubTotalRuleContext()
{
}

SvXMLImportContext *ScXMLSubTotalRuleContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetSubTotalRulesSubTotalRuleElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_SUBTOTAL_RULE_SUBTOTAL_FIELD :
        {
            pContext = new ScXMLSubTotalFieldContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pDatabaseRangeContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSubTotalRuleContext::EndElement()
{
}

ScXMLSubTotalFieldContext::ScXMLSubTotalFieldContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pDatabaseRangeContext = pTempDatabaseRangeContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetSubTotalRuleSubTotalFieldAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_SUBTOTAL_FIELD_ATTR_FIELD_NUMBER :
            {
                sFieldNumber = sValue;
            }
            break;
            case XML_TOK_SUBTOTAL_FIELD_ATTR_FUNCTION :
            {
                sFunction = sValue;
            }
            break;
        }
    }
}

ScXMLSubTotalFieldContext::~ScXMLSubTotalFieldContext()
{
}

SvXMLImportContext *ScXMLSubTotalFieldContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLSubTotalFieldContext::EndElement()
{
    sheet::SubTotalColumn aSubTotalColumn;
    aSubTotalColumn.Column = sFieldNumber.toInt32();
    if (sFunction.compareToAscii("auto") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_AUTO;
    else if (sFunction.compareToAscii("average") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_AVERAGE;
    else if (sFunction.compareToAscii("count") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_COUNT;
    else if (sFunction.compareToAscii("countnums") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_COUNTNUMS;
    else if (sFunction.compareToAscii("max") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_MAX;
    else if (sFunction.compareToAscii("min") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_MIN;
    else if (sFunction.compareToAscii("none") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_NONE;
    else if (sFunction.compareToAscii("product") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_PRODUCT;
    else if (sFunction.compareToAscii("stdev") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_STDEV;
    else if (sFunction.compareToAscii("stdevp") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_STDEVP;
    else if (sFunction.compareToAscii("sum") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_SUM;
    else if (sFunction.compareToAscii("var") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_VAR;
    else if (sFunction.compareToAscii("varp") == 0)
        aSubTotalColumn.Function = sheet::GeneralFunction_VARP;
    pDatabaseRangeContext->AddSubTotalColumn(aSubTotalColumn);
}

