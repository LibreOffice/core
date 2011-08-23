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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "xmlfilti.hxx"
#include "xmlimprt.hxx"

#ifndef SC_CONVUNO_HXX
#include "convuno.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#include <bf_xmloff/nmspmap.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLFilterContext::ScXMLFilterContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    bSkipDuplicates(sal_False),
    bUseRegularExpressions(sal_False),
    bConnectionOr(sal_True),
    bNextConnectionOr(sal_True),
    bCopyOutputData(sal_False),
    bConditionSourceRange(sal_False),
    aFilterFields(),
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    ScDocument* pDoc = GetScImport().GetDocument();
    pDatabaseRangeContext = pTempDatabaseRangeContext;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScRange aScRange;
                sal_Int32 nOffset(0);
                if (ScXMLConverter::GetRangeFromString( aScRange, sValue, pDoc, nOffset ))
                {
                    ScUnoConversion::FillApiAddress( aOutputPosition, aScRange.aStart );
                    bCopyOutputData = sal_True;
                }
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS :
            {
                sal_Int32 nOffset(0);
                if (ScXMLConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc, nOffset ))
                    bConditionSourceRange = sal_True;
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE :
            {
                // not supported by StarOffice
            }
            break;
            case XML_TOK_FILTER_ATTR_DISPLAY_DUPLICATES :
            {
                bSkipDuplicates = !IsXMLToken(sValue, XML_TRUE);
            }
            break;
        }
    }
}

ScXMLFilterContext::~ScXMLFilterContext()
{
}

SvXMLImportContext *ScXMLFilterContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetFilterElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_FILTER_AND:
        {
            pContext = new ScXMLAndContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_FILTER_OR:
        {
            pContext = new ScXMLOrContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_FILTER_CONDITION:
        {
            pContext = new ScXMLConditionContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLFilterContext::EndElement()
{
    pDatabaseRangeContext->SetFilterUseRegularExpressions(bUseRegularExpressions);
    if (bCopyOutputData)
    {
        pDatabaseRangeContext->SetFilterOutputPosition(aOutputPosition);
        pDatabaseRangeContext->SetFilterCopyOutputData(bCopyOutputData);
    }
    else
        pDatabaseRangeContext->SetFilterCopyOutputData(sal_False);
    pDatabaseRangeContext->SetFilterIsCaseSensitive(bIsCaseSensitive);
    pDatabaseRangeContext->SetFilterSkipDuplicates(bSkipDuplicates);
    pDatabaseRangeContext->SetFilterFields(aFilterFields);
    if (bConditionSourceRange)
        pDatabaseRangeContext->SetFilterConditionSourceRangeAddress(aConditionSourceRangeAddress);
}

ScXMLAndContext::ScXMLAndContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    pFilterContext->OpenConnection(sal_False);
}

ScXMLAndContext::~ScXMLAndContext()
{
}

SvXMLImportContext *ScXMLAndContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetFilterElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_FILTER_OR:
        {
            // not supported in StarOffice
        }
        break;
        case XML_TOK_FILTER_CONDITION:
        {
            pContext = new ScXMLConditionContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pFilterContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLAndContext::EndElement()
{
    pFilterContext->CloseConnection();
}

ScXMLOrContext::ScXMLOrContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    pFilterContext->OpenConnection(sal_True);
}

ScXMLOrContext::~ScXMLOrContext()
{
}

SvXMLImportContext *ScXMLOrContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetFilterElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_FILTER_AND:
        {
            pContext = new ScXMLAndContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pFilterContext);
        }
        break;
        case XML_TOK_FILTER_CONDITION:
        {
            pContext = new ScXMLConditionContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pFilterContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLOrContext::EndElement()
{
    pFilterContext->CloseConnection();
}

ScXMLConditionContext::ScXMLConditionContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext) :
    bIsCaseSensitive(sal_False),
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    sDataType = GetXMLToken(XML_TEXT);

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterConditionAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONDITION_ATTR_FIELD_NUMBER :
            {
                nField = sValue.toInt32();
            }
            break;
            case XML_TOK_CONDITION_ATTR_CASE_SENSITIVE :
            {
                bIsCaseSensitive = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_CONDITION_ATTR_DATA_TYPE :
            {
                sDataType = sValue;
            }
            break;
            case XML_TOK_CONDITION_ATTR_VALUE :
            {
                sConditionValue = sValue;
            }
            break;
            case XML_TOK_CONDITION_ATTR_OPERATOR :
            {
                sOperator = sValue;
            }
            break;
        }
    }
}

ScXMLConditionContext::~ScXMLConditionContext()
{
}

SvXMLImportContext *ScXMLConditionContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLConditionContext::getOperatorXML(const ::rtl::OUString sTempOperator, sheet::FilterOperator& aFilterOperator, sal_Bool& bUseRegularExpressions) const
{
    bUseRegularExpressions = sal_False;
    if (IsXMLToken(sTempOperator, XML_MATCH))
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = sheet::FilterOperator_EQUAL;
    }
    else if (IsXMLToken(sTempOperator, XML_NOMATCH))
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = sheet::FilterOperator_NOT_EQUAL;
    }
    else if (sTempOperator.compareToAscii("=") == 0)
        aFilterOperator = sheet::FilterOperator_EQUAL;
    else if (sTempOperator.compareToAscii("!=") == 0)
        aFilterOperator = sheet::FilterOperator_NOT_EQUAL;
    else if (IsXMLToken(sTempOperator, XML_BOTTOM_PERCENT))
        aFilterOperator = sheet::FilterOperator_BOTTOM_PERCENT;
    else if (IsXMLToken(sTempOperator, XML_BOTTOM_VALUES))
        aFilterOperator = sheet::FilterOperator_BOTTOM_VALUES;
    else if (IsXMLToken(sTempOperator, XML_EMPTY))
        aFilterOperator = sheet::FilterOperator_EMPTY;
    else if (sTempOperator.compareToAscii(">") == 0)
        aFilterOperator = sheet::FilterOperator_GREATER;
    else if (sTempOperator.compareToAscii(">=") == 0)
        aFilterOperator = sheet::FilterOperator_GREATER_EQUAL;
    else if (sTempOperator.compareToAscii("<") == 0)
        aFilterOperator = sheet::FilterOperator_LESS;
    else if (sTempOperator.compareToAscii("<=") == 0)
        aFilterOperator = sheet::FilterOperator_LESS_EQUAL;
    else if (IsXMLToken(sTempOperator, XML_NOEMPTY))
        aFilterOperator = sheet::FilterOperator_NOT_EMPTY;
    else if (IsXMLToken(sTempOperator, XML_TOP_PERCENT))
        aFilterOperator = sheet::FilterOperator_TOP_PERCENT;
    else if (IsXMLToken(sTempOperator, XML_TOP_VALUES))
        aFilterOperator = sheet::FilterOperator_TOP_VALUES;
}

void ScXMLConditionContext::EndElement()
{
    sheet::TableFilterField aFilterField;
    if (pFilterContext->GetConnection())
        aFilterField.Connection = sheet::FilterConnection_OR;
    else
        aFilterField.Connection = sheet::FilterConnection_AND;
    pFilterContext->SetIsCaseSensitive(bIsCaseSensitive);
    sal_Bool bUseRegularExpressions;
    getOperatorXML(sOperator, aFilterField.Operator, bUseRegularExpressions);
    pFilterContext->SetUseRegularExpressions(bUseRegularExpressions);
    aFilterField.Field = nField;
    if (IsXMLToken(sDataType, XML_NUMBER))
    {
        aFilterField.NumericValue = sConditionValue.toDouble();
        aFilterField.IsNumeric = sal_True;
    }
    else
    {
        aFilterField.StringValue = sConditionValue;
        aFilterField.IsNumeric = sal_False;
    }
    pFilterContext->AddFilterField(aFilterField);
}

//==========================================================================

ScXMLDPFilterContext::ScXMLDPFilterContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTableContext) :
    bSkipDuplicates(sal_False),
    bUseRegularExpressions(sal_False),
    bConnectionOr(sal_True),
    bNextConnectionOr(sal_True),
    bCopyOutputData(sal_False),
    bConditionSourceRange(sal_False),
    aFilterFields(),
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nFilterFieldCount(0)
{
    ScDocument* pDoc = GetScImport().GetDocument();
    pDataPilotTable = pTempDataPilotTableContext;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScRange aScRange;
                sal_Int32 nOffset(0);
                if (ScXMLConverter::GetRangeFromString( aScRange, sValue, pDoc, nOffset ))
                {
                    aOutputPosition = aScRange.aStart;
                    bCopyOutputData = sal_True;
                }
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS :
            {
                sal_Int32 nOffset(0);
                if(ScXMLConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc, nOffset ))
                    bConditionSourceRange = sal_True;
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE :
            {
                // not supported by StarOffice
            }
            break;
            case XML_TOK_FILTER_ATTR_DISPLAY_DUPLICATES :
            {
                bSkipDuplicates = !IsXMLToken(sValue, XML_TRUE);
            }
            break;
        }
    }
}

ScXMLDPFilterContext::~ScXMLDPFilterContext()
{
}

SvXMLImportContext *ScXMLDPFilterContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetFilterElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_FILTER_AND:
        {
            pContext = new ScXMLDPAndContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_FILTER_OR:
        {
            pContext = new ScXMLDPOrContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
        case XML_TOK_FILTER_CONDITION:
        {
            pContext = new ScXMLDPConditionContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPFilterContext::EndElement()
{
    aFilterFields.bRegExp = bUseRegularExpressions;
    aFilterFields.bCaseSens = bIsCaseSensitive;
    aFilterFields.bDuplicate = !bSkipDuplicates;
//	pDataPilotTable->SetFilterUseRegularExpressions(bUseRegularExpressions);
    if (bCopyOutputData)
    {
        pDataPilotTable->SetFilterOutputPosition(aOutputPosition);
        pDataPilotTable->SetFilterCopyOutputData(bCopyOutputData);
    }
    else
        pDataPilotTable->SetFilterCopyOutputData(sal_False);
//	pDataPilotTable->SetFilterIsCaseSensitive(bIsCaseSensitive);
//	pDataPilotTable->SetFilterSkipDuplicates(bSkipDuplicates);
    pDataPilotTable->SetSourceQueryParam(aFilterFields);
    if (bConditionSourceRange)
        pDataPilotTable->SetFilterSourceRange(aConditionSourceRangeAddress);
}

void ScXMLDPFilterContext::AddFilterField (const ScQueryEntry& aFilterField)
{
    aFilterFields.Resize(nFilterFieldCount + 1);
    ScQueryEntry& rEntry = aFilterFields.GetEntry(nFilterFieldCount);
    rEntry = aFilterField;
    rEntry.bDoQuery = sal_True;
    nFilterFieldCount++;
}

ScXMLDPAndContext::ScXMLDPAndContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    pFilterContext->OpenConnection(sal_False);
}

ScXMLDPAndContext::~ScXMLDPAndContext()
{
}

SvXMLImportContext *ScXMLDPAndContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetFilterElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_FILTER_OR:
        {
            // not supported in StarOffice
        }
        break;
        case XML_TOK_FILTER_CONDITION:
        {
            pContext = new ScXMLDPConditionContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pFilterContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPAndContext::EndElement()
{
    pFilterContext->CloseConnection();
}

ScXMLDPOrContext::ScXMLDPOrContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    pFilterContext->OpenConnection(sal_True);
}

ScXMLDPOrContext::~ScXMLDPOrContext()
{
}

SvXMLImportContext *ScXMLDPOrContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetFilterElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_FILTER_AND:
        {
            pContext = new ScXMLDPAndContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pFilterContext);
        }
        break;
        case XML_TOK_FILTER_CONDITION:
        {
            pContext = new ScXMLDPConditionContext( GetScImport(), nPrefix,
                                                          rLName, xAttrList, pFilterContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPOrContext::EndElement()
{
    pFilterContext->CloseConnection();
}

ScXMLDPConditionContext::ScXMLDPConditionContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext) :
    bIsCaseSensitive(sal_False),
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sDataType(GetXMLToken(XML_TEXT))
{
    pFilterContext = pTempFilterContext;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterConditionAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONDITION_ATTR_FIELD_NUMBER :
            {
                nField = sValue.toInt32();
            }
            break;
            case XML_TOK_CONDITION_ATTR_CASE_SENSITIVE :
            {
                bIsCaseSensitive = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_CONDITION_ATTR_DATA_TYPE :
            {
                sDataType = sValue;
            }
            break;
            case XML_TOK_CONDITION_ATTR_VALUE :
            {
                sConditionValue = sValue;
            }
            break;
            case XML_TOK_CONDITION_ATTR_OPERATOR :
            {
                sOperator = sValue;
            }
            break;
        }
    }
}

ScXMLDPConditionContext::~ScXMLDPConditionContext()
{
}

SvXMLImportContext *ScXMLDPConditionContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPConditionContext::getOperatorXML(const ::rtl::OUString sTempOperator, ScQueryOp& aFilterOperator, sal_Bool& bUseRegularExpressions,
                                            double& dVal) const
{
    bUseRegularExpressions = sal_False;
    if (IsXMLToken(sTempOperator, XML_MATCH))
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = SC_EQUAL;
    }
    else if (IsXMLToken(sTempOperator, XML_NOMATCH))
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = SC_NOT_EQUAL;
    }
    else if (sTempOperator.compareToAscii("=") == 0)
        aFilterOperator = SC_EQUAL;
    else if (sTempOperator.compareToAscii("!=") == 0)
        aFilterOperator = SC_NOT_EQUAL;
    else if (IsXMLToken(sTempOperator, XML_BOTTOM_PERCENT))
        aFilterOperator = SC_BOTPERC;
    else if (IsXMLToken(sTempOperator, XML_BOTTOM_VALUES))
        aFilterOperator = SC_BOTVAL;
    else if (IsXMLToken(sTempOperator, XML_EMPTY))
        dVal = SC_EMPTYFIELDS;
    else if (sTempOperator.compareToAscii(">") == 0)
        aFilterOperator = SC_GREATER;
    else if (sTempOperator.compareToAscii(">=") == 0)
        aFilterOperator = SC_GREATER_EQUAL;
    else if (sTempOperator.compareToAscii("<") == 0)
        aFilterOperator = SC_LESS;
    else if (sTempOperator.compareToAscii("<=") == 0)
        aFilterOperator = SC_LESS_EQUAL;
    else if (IsXMLToken(sTempOperator, XML_NOEMPTY))
        dVal = SC_NONEMPTYFIELDS;
    else if (IsXMLToken(sTempOperator, XML_TOP_PERCENT))
        aFilterOperator = SC_TOPPERC;
    else if (IsXMLToken(sTempOperator, XML_TOP_VALUES))
        aFilterOperator = SC_TOPVAL;
}

void ScXMLDPConditionContext::EndElement()
{
    ScQueryEntry aFilterField;
    if (pFilterContext->GetConnection())
        aFilterField.eConnect = SC_OR;
    else
        aFilterField.eConnect = SC_AND;
    pFilterContext->SetIsCaseSensitive(bIsCaseSensitive);
    sal_Bool bUseRegularExpressions;
    double dVal(0.0);
    getOperatorXML(sOperator, aFilterField.eOp, bUseRegularExpressions, dVal);
    pFilterContext->SetUseRegularExpressions(bUseRegularExpressions);
    aFilterField.nField = static_cast<USHORT>(nField);
    if (IsXMLToken(sDataType, XML_NUMBER))
    {
        aFilterField.nVal = sConditionValue.toDouble();
        *aFilterField.pStr = sConditionValue;
        aFilterField.bQueryByString = sal_False;
        if (dVal != 0.0)
        {
            aFilterField.nVal = dVal;
            *aFilterField.pStr = EMPTY_STRING;
        }
    }
    else
    {
        aFilterField.pStr = new String(sConditionValue);
        aFilterField.bQueryByString = sal_True;
        aFilterField.nVal = 0;
    }
    pFilterContext->AddFilterField(aFilterField);
}



}
