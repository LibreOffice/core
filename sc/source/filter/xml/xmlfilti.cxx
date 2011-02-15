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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


// INCLUDE ---------------------------------------------------------------

#include "xmlfilti.hxx"
#include "xmlimprt.hxx"
#include "docuno.hxx"
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLFilterContext::ScXMLFilterContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDatabaseRangeContext(pTempDatabaseRangeContext),
    aFilterFields(),
    bSkipDuplicates(sal_False),
    bCopyOutputData(sal_False),
    bUseRegularExpressions(sal_False),
    bConnectionOr(sal_True),
    bNextConnectionOr(sal_True),
    bConditionSourceRange(sal_False)
{
    ScDocument* pDoc(GetScImport().GetDocument());

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScRange aScRange;
                sal_Int32 nOffset(0);
                if (ScRangeStringConverter::GetRangeFromString( aScRange, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ))
                {
                    ScUnoConversion::FillApiAddress( aOutputPosition, aScRange.aStart );
                    bCopyOutputData = sal_True;
                }
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS :
            {
                sal_Int32 nOffset(0);
                if (ScRangeStringConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ))
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

SvXMLImportContext *ScXMLFilterContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetFilterElemTokenMap());
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
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */,
                                        ScXMLFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(sal_False);
}

ScXMLAndContext::~ScXMLAndContext()
{
}

SvXMLImportContext *ScXMLAndContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetFilterElemTokenMap());
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
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */,
                                        ScXMLFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(sal_True);
}

ScXMLOrContext::~ScXMLOrContext()
{
}

SvXMLImportContext *ScXMLOrContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetFilterElemTokenMap());
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
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pFilterContext(pTempFilterContext),
    bIsCaseSensitive(sal_False)
{
    sDataType = GetXMLToken(XML_TEXT);

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetFilterConditionAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

SvXMLImportContext *ScXMLConditionContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLConditionContext::getOperatorXML(const rtl::OUString sTempOperator, sal_Int32& aFilterOperator, sal_Bool& bUseRegularExpressions) const
{
    bUseRegularExpressions = sal_False;
    if (IsXMLToken(sTempOperator, XML_MATCH))
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = sheet::FilterOperator2::EQUAL;
    }
    else if (IsXMLToken(sTempOperator, XML_NOMATCH))
    {
        bUseRegularExpressions = sal_True;
        aFilterOperator = sheet::FilterOperator2::NOT_EQUAL;
    }
    else if (sTempOperator.compareToAscii("=") == 0)
        aFilterOperator = sheet::FilterOperator2::EQUAL;
    else if (sTempOperator.compareToAscii("!=") == 0)
        aFilterOperator = sheet::FilterOperator2::NOT_EQUAL;
    else if (IsXMLToken(sTempOperator, XML_BOTTOM_PERCENT))
        aFilterOperator = sheet::FilterOperator2::BOTTOM_PERCENT;
    else if (IsXMLToken(sTempOperator, XML_BOTTOM_VALUES))
        aFilterOperator = sheet::FilterOperator2::BOTTOM_VALUES;
    else if (IsXMLToken(sTempOperator, XML_EMPTY))
        aFilterOperator = sheet::FilterOperator2::EMPTY;
    else if (sTempOperator.compareToAscii(">") == 0)
        aFilterOperator = sheet::FilterOperator2::GREATER;
    else if (sTempOperator.compareToAscii(">=") == 0)
        aFilterOperator = sheet::FilterOperator2::GREATER_EQUAL;
    else if (sTempOperator.compareToAscii("<") == 0)
        aFilterOperator = sheet::FilterOperator2::LESS;
    else if (sTempOperator.compareToAscii("<=") == 0)
        aFilterOperator = sheet::FilterOperator2::LESS_EQUAL;
    else if (IsXMLToken(sTempOperator, XML_NOEMPTY))
        aFilterOperator = sheet::FilterOperator2::NOT_EMPTY;
    else if (IsXMLToken(sTempOperator, XML_TOP_PERCENT))
        aFilterOperator = sheet::FilterOperator2::TOP_PERCENT;
    else if (IsXMLToken(sTempOperator, XML_TOP_VALUES))
        aFilterOperator = sheet::FilterOperator2::TOP_VALUES;
    else if (IsXMLToken(sTempOperator, XML_CONTAINS))
        aFilterOperator = sheet::FilterOperator2::CONTAINS;
    else if (IsXMLToken(sTempOperator, XML_DOES_NOT_CONTAIN))
        aFilterOperator = sheet::FilterOperator2::DOES_NOT_CONTAIN;
    else if (IsXMLToken(sTempOperator, XML_BEGINS_WITH))
        aFilterOperator = sheet::FilterOperator2::BEGINS_WITH;
    else if (IsXMLToken(sTempOperator, XML_DOES_NOT_BEGIN_WITH))
        aFilterOperator = sheet::FilterOperator2::DOES_NOT_BEGIN_WITH;
    else if (IsXMLToken(sTempOperator, XML_ENDS_WITH))
        aFilterOperator = sheet::FilterOperator2::ENDS_WITH;
    else if (IsXMLToken(sTempOperator, XML_DOES_NOT_END_WITH))
        aFilterOperator = sheet::FilterOperator2::DOES_NOT_END_WITH;
}

void ScXMLConditionContext::EndElement()
{
    sheet::TableFilterField2 aFilterField;
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
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDataPilotTableContext* pTempDataPilotTableContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotTable(pTempDataPilotTableContext),
    aFilterFields(),
    nFilterFieldCount(0),
    bSkipDuplicates(sal_False),
    bCopyOutputData(sal_False),
    bUseRegularExpressions(sal_False),
    bConnectionOr(sal_True),
    bNextConnectionOr(sal_True),
    bConditionSourceRange(sal_False)
{
    ScDocument* pDoc(GetScImport().GetDocument());

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetFilterAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_FILTER_ATTR_TARGET_RANGE_ADDRESS :
            {
                ScRange aScRange;
                sal_Int32 nOffset(0);
                if (ScRangeStringConverter::GetRangeFromString( aScRange, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ))
                {
                    aOutputPosition = aScRange.aStart;
                    bCopyOutputData = sal_True;
                }
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS :
            {
                sal_Int32 nOffset(0);
                if(ScRangeStringConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ))
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

SvXMLImportContext *ScXMLDPFilterContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetFilterElemTokenMap());
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
//  pDataPilotTable->SetFilterUseRegularExpressions(bUseRegularExpressions);
    if (bCopyOutputData)
    {
        pDataPilotTable->SetFilterOutputPosition(aOutputPosition);
        pDataPilotTable->SetFilterCopyOutputData(bCopyOutputData);
    }
    else
        pDataPilotTable->SetFilterCopyOutputData(sal_False);
//  pDataPilotTable->SetFilterIsCaseSensitive(bIsCaseSensitive);
//  pDataPilotTable->SetFilterSkipDuplicates(bSkipDuplicates);
    pDataPilotTable->SetSourceQueryParam(aFilterFields);
    if (bConditionSourceRange)
        pDataPilotTable->SetFilterSourceRange(aConditionSourceRangeAddress);
}

void ScXMLDPFilterContext::AddFilterField (const ScQueryEntry& aFilterField)
{
    aFilterFields.Resize(nFilterFieldCount + 1);
    ScQueryEntry& rEntry(aFilterFields.GetEntry(nFilterFieldCount));
    rEntry = aFilterField;
    rEntry.bDoQuery = sal_True;
    ++nFilterFieldCount;
}

ScXMLDPAndContext::ScXMLDPAndContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */,
                                        ScXMLDPFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    pFilterContext->OpenConnection(sal_False);
}

ScXMLDPAndContext::~ScXMLDPAndContext()
{
}

SvXMLImportContext *ScXMLDPAndContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetFilterElemTokenMap());
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
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */,
                                        ScXMLDPFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(sal_True);
}

ScXMLDPOrContext::~ScXMLDPOrContext()
{
}

SvXMLImportContext *ScXMLDPOrContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetFilterElemTokenMap());
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
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                        ScXMLDPFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pFilterContext(pTempFilterContext),
    sDataType(GetXMLToken(XML_TEXT)),
    bIsCaseSensitive(sal_False)
{

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetFilterConditionAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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

SvXMLImportContext *ScXMLDPConditionContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLDPConditionContext::getOperatorXML(const rtl::OUString sTempOperator, ScQueryOp& aFilterOperator, sal_Bool& bUseRegularExpressions,
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
    aFilterField.nField = nField;
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



