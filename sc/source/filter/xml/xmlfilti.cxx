/*************************************************************************
 *
 *  $RCSfile: xmlfilti.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-03 16:34:37 $
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

#include "xmlfilti.hxx"
#include "xmlimprt.hxx"
#include "docuno.hxx"

#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlkywd.hxx>

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLFilterContext::ScXMLFilterContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
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
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScRange aScRange;
                ScXMLConverter::GetRangeFromString( aScRange, sValue, pDoc );
                ScXMLConverter::GetApiAddressFromScAddress( aOutputPosition, aScRange.aStart );
                bCopyOutputData = sal_True;
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS :
            {
                ScXMLConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc );
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
                if (sValue.compareToAscii(sXML_false) == 0)
                    bSkipDuplicates = sal_True;
            }
            break;
        }
    }
}

ScXMLFilterContext::~ScXMLFilterContext()
{
}

SvXMLImportContext *ScXMLFilterContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
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
                                      const NAMESPACE_RTL(OUString)& rLName,
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
                                            const NAMESPACE_RTL(OUString)& rLName,
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
                                      const NAMESPACE_RTL(OUString)& rLName,
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
                                            const NAMESPACE_RTL(OUString)& rLName,
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
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext) :
    bIsCaseSensitive(sal_False),
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    sDataType = rtl::OUString::createFromAscii(sXML_text);

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterConditionAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONDITION_ATTR_FIELD_NUMBER :
            {
                nField = sValue.toInt32();
            }
            break;
            case XML_TOK_CONDITION_ATTR_CASE_SENSITIVE :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bIsCaseSensitive = sal_True;
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
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLConditionContext::getOperatorXML(const rtl::OUString sTempOperator, sheet::FilterOperator& aFilterOperator, sal_Bool& bUseRegularExpressions) const
{
    bUseRegularExpressions = sal_False;
    if (sTempOperator.compareToAscii(sXML_match) == 0)
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = sheet::FilterOperator_EQUAL;
    }
    else if (sTempOperator.compareToAscii(sXML_nomatch) == 0)
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = sheet::FilterOperator_NOT_EQUAL;
    }
    else if (sTempOperator.compareToAscii("=") == 0)
        aFilterOperator = sheet::FilterOperator_EQUAL;
    else if (sTempOperator.compareToAscii("!=") == 0)
        aFilterOperator = sheet::FilterOperator_NOT_EQUAL;
    else if (sTempOperator.compareToAscii(sXML_bottom_percent) == 0)
        aFilterOperator = sheet::FilterOperator_BOTTOM_PERCENT;
    else if (sTempOperator.compareToAscii(sXML_bottom_values) == 0)
        aFilterOperator = sheet::FilterOperator_BOTTOM_VALUES;
    else if (sTempOperator.compareToAscii(sXML_empty) == 0)
        aFilterOperator = sheet::FilterOperator_EMPTY;
    else if (sTempOperator.compareToAscii(">") == 0)
        aFilterOperator = sheet::FilterOperator_GREATER;
    else if (sTempOperator.compareToAscii(">=") == 0)
        aFilterOperator = sheet::FilterOperator_GREATER_EQUAL;
    else if (sTempOperator.compareToAscii("<") == 0)
        aFilterOperator = sheet::FilterOperator_LESS;
    else if (sTempOperator.compareToAscii("<=") == 0)
        aFilterOperator = sheet::FilterOperator_LESS_EQUAL;
    else if (sTempOperator.compareToAscii(sXML_noempty) == 0)
        aFilterOperator = sheet::FilterOperator_NOT_EMPTY;
    else if (sTempOperator.compareToAscii(sXML_top_percent) == 0)
        aFilterOperator = sheet::FilterOperator_TOP_PERCENT;
    else if (sTempOperator.compareToAscii(sXML_top_values) == 0)
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
    if (sDataType.compareToAscii(sXML_number) == 0)
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
                                      const NAMESPACE_RTL(OUString)& rLName,
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
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    ScDocument* pDoc = GetScImport().GetDocument();
    pDataPilotTable = pTempDataPilotTableContext;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScRange aScRange;
                ScXMLConverter::GetRangeFromString( aScRange, sValue, pDoc );
                aOutputPosition = aScRange.aStart;
                bCopyOutputData = sal_True;
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS :
            {
                ScXMLConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc );
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
                if (sValue.compareToAscii(sXML_false) == 0)
                    bSkipDuplicates = sal_True;
            }
            break;
        }
    }
}

ScXMLDPFilterContext::~ScXMLDPFilterContext()
{
}

SvXMLImportContext *ScXMLDPFilterContext::CreateChildContext( USHORT nPrefix,
                                            const NAMESPACE_RTL(OUString)& rLName,
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
    pDataPilotTable->SetFilterUseRegularExpressions(bUseRegularExpressions);
    if (bCopyOutputData)
    {
        pDataPilotTable->SetFilterOutputPosition(aOutputPosition);
        pDataPilotTable->SetFilterCopyOutputData(bCopyOutputData);
    }
    else
        pDataPilotTable->SetFilterCopyOutputData(sal_False);
    pDataPilotTable->SetFilterIsCaseSensitive(bIsCaseSensitive);
    pDataPilotTable->SetFilterSkipDuplicates(bSkipDuplicates);
    pDataPilotTable->SetSourceQueryParam(aFilterFields);
    if (bConditionSourceRange)
        pDataPilotTable->SetFilterSourceRange(aConditionSourceRangeAddress);
}

ScXMLDPAndContext::ScXMLDPAndContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const NAMESPACE_RTL(OUString)& rLName,
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
                                            const NAMESPACE_RTL(OUString)& rLName,
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
                                      const NAMESPACE_RTL(OUString)& rLName,
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
                                            const NAMESPACE_RTL(OUString)& rLName,
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
                                      const NAMESPACE_RTL(OUString)& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext) :
    bIsCaseSensitive(sal_False),
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    sDataType = rtl::OUString::createFromAscii(sXML_text);

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterConditionAttrTokenMap();

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONDITION_ATTR_FIELD_NUMBER :
            {
                nField = sValue.toInt32();
            }
            break;
            case XML_TOK_CONDITION_ATTR_CASE_SENSITIVE :
            {
                if (sValue.compareToAscii(sXML_true) == 0)
                    bIsCaseSensitive = sal_True;
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
                                            const NAMESPACE_RTL(OUString)& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDPConditionContext::getOperatorXML(const rtl::OUString sTempOperator, ScQueryOp& aFilterOperator, sal_Bool& bUseRegularExpressions,
                                            double& dVal) const
{
    bUseRegularExpressions = sal_False;
    if (sTempOperator.compareToAscii(sXML_match) == 0)
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = SC_EQUAL;
    }
    else if (sTempOperator.compareToAscii(sXML_nomatch) == 0)
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = SC_NOT_EQUAL;
    }
    else if (sTempOperator.compareToAscii("=") == 0)
        aFilterOperator = SC_EQUAL;
    else if (sTempOperator.compareToAscii("!=") == 0)
        aFilterOperator = SC_NOT_EQUAL;
    else if (sTempOperator.compareToAscii(sXML_bottom_percent) == 0)
        aFilterOperator = SC_BOTPERC;
    else if (sTempOperator.compareToAscii(sXML_bottom_values) == 0)
        aFilterOperator = SC_BOTVAL;
    else if (sTempOperator.compareToAscii(sXML_empty) == 0)
        dVal = SC_EMPTYFIELDS;
    else if (sTempOperator.compareToAscii(">") == 0)
        aFilterOperator = SC_GREATER;
    else if (sTempOperator.compareToAscii(">=") == 0)
        aFilterOperator = SC_GREATER_EQUAL;
    else if (sTempOperator.compareToAscii("<") == 0)
        aFilterOperator = SC_LESS;
    else if (sTempOperator.compareToAscii("<=") == 0)
        aFilterOperator = SC_LESS_EQUAL;
    else if (sTempOperator.compareToAscii(sXML_noempty) == 0)
        dVal = SC_NONEMPTYFIELDS;
    else if (sTempOperator.compareToAscii(sXML_top_percent) == 0)
        aFilterOperator = SC_TOPPERC;
    else if (sTempOperator.compareToAscii(sXML_top_values) == 0)
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
    double dVal;
    getOperatorXML(sOperator, aFilterField.eOp, bUseRegularExpressions, dVal);
    pFilterContext->SetUseRegularExpressions(bUseRegularExpressions);
    aFilterField.nField = nField;
    if (sDataType.compareToAscii(sXML_number) == 0)
    {
        aFilterField.nVal = sConditionValue.toDouble();
        aFilterField.bQueryByString = sal_False;
    }
    else
    {
        aFilterField.pStr = new String(sConditionValue);
        aFilterField.bQueryByString = sal_True;
        aFilterField.nVal = dVal;
    }
    pFilterContext->AddFilterField(aFilterField);
}



