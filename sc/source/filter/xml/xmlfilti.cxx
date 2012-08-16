/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "xmlfilti.hxx"
#include "xmlimprt.hxx"
#include "docuno.hxx"
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"
#include "queryentry.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

ScXMLFilterContext::ConnStackItem::ConnStackItem(bool bOr) : mbOr(bOr), mnCondCount(0) {}

ScXMLFilterContext::ScXMLFilterContext( ScXMLImport& rImport,
                                        sal_uInt16 nPrfx,
                                        const rtl::OUString& rLName,
                                        const Reference<XAttributeList>& xAttrList,
                                        ScQueryParam& rParam,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrQueryParam(rParam),
    pDatabaseRangeContext(pTempDatabaseRangeContext),
    bSkipDuplicates(false),
    bCopyOutputData(false),
    bConditionSourceRange(false)
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
                    bCopyOutputData = true;
                }
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS :
            {
                sal_Int32 nOffset(0);
                if (ScRangeStringConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ))
                    bConditionSourceRange = true;
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
            pContext = new ScXMLAndContext(
                GetScImport(), nPrefix, rLName, xAttrList, mrQueryParam, this);
        }
        break;
        case XML_TOK_FILTER_OR:
        {
            pContext = new ScXMLOrContext(
                GetScImport(), nPrefix, rLName, xAttrList, mrQueryParam, this);
        }
        break;
        case XML_TOK_FILTER_CONDITION:
        {
            pContext = new ScXMLConditionContext(
                GetScImport(), nPrefix, rLName, xAttrList, mrQueryParam, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLFilterContext::EndElement()
{
    mrQueryParam.bInplace = !bCopyOutputData;
    mrQueryParam.bDuplicate = !bSkipDuplicates;

    if (bCopyOutputData)
    {
        mrQueryParam.nDestCol = aOutputPosition.Column;
        mrQueryParam.nDestRow = aOutputPosition.Row;
        mrQueryParam.nDestTab = aOutputPosition.Sheet;
    }

    if (bConditionSourceRange)
        pDatabaseRangeContext->SetFilterConditionSourceRangeAddress(aConditionSourceRangeAddress);
}

void ScXMLFilterContext::OpenConnection(bool b)
{
    maConnStack.push_back(ConnStackItem(b));
}

void ScXMLFilterContext::CloseConnection()
{
    maConnStack.pop_back();
}

bool ScXMLFilterContext::GetConnection()
{
    // For condition items in each stack, the first one gets the connection of
    // the last stack, while the rest of them get that of the current stack.

    if (maConnStack.empty())
        // This should never happen.
        return true;

    ConnStackItem& rItem = maConnStack.back();
    if (rItem.mnCondCount)
        // secondary item gets the current connection.
        return rItem.mbOr;

    if (maConnStack.size() < 2)
        // There is no last stack.  Likely the first condition in the first
        // stack whose connection is not used.
        return true;

    ++rItem.mnCondCount;
    std::vector<ConnStackItem>::reverse_iterator itr = maConnStack.rbegin();
    ++itr;
    return itr->mbOr; // connection of the last stack.
}

ScXMLAndContext::ScXMLAndContext( ScXMLImport& rImport,
                                  sal_uInt16 nPrfx,
                                  const rtl::OUString& rLName,
                                  const Reference<XAttributeList>& /* xAttrList */,
                                  ScQueryParam& rParam,
                                  ScXMLFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(false);
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
            pContext = new ScXMLConditionContext(
                GetScImport(), nPrefix, rLName, xAttrList, mrQueryParam, pFilterContext);
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
                                const rtl::OUString& rLName,
                                const Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */,
                                ScQueryParam& rParam,
                                ScXMLFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(true);
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
            pContext = new ScXMLAndContext(
                GetScImport(), nPrefix, rLName, xAttrList, mrQueryParam, pFilterContext);
        }
        break;
        case XML_TOK_FILTER_CONDITION:
        {
            pContext = new ScXMLConditionContext(
                GetScImport(), nPrefix, rLName, xAttrList, mrQueryParam, pFilterContext);
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

ScXMLConditionContext::ScXMLConditionContext(
    ScXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLName,
    const Reference<XAttributeList>& xAttrList,
    ScQueryParam& rParam,
    ScXMLFilterContext* pTempFilterContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext),
    bIsCaseSensitive(false)
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
                                            const Reference<XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = NULL;

    const SvXMLTokenMap& rTokenMap(GetScImport().GetFilterConditionElemTokenMap());
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_CONDITION_FILTER_SET_ITEM:
        {
            pContext = new ScXMLSetItemContext(
                GetScImport(), nPrefix, rLName, xAttrList, *this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLConditionContext::GetOperator(
    const rtl::OUString& aOpStr, ScQueryParam& rParam, ScQueryEntry& rEntry) const
{
    rParam.bRegExp = false;
    if (IsXMLToken(aOpStr, XML_MATCH))
    {
        rParam.bRegExp = true;
        rEntry.eOp = SC_EQUAL;
    }
    else if (IsXMLToken(aOpStr, XML_NOMATCH))
    {
        rParam.bRegExp = true;
        rEntry.eOp = SC_NOT_EQUAL;
    }
    else if (aOpStr.compareToAscii("=") == 0)
        rEntry.eOp = SC_EQUAL;
    else if (aOpStr.compareToAscii("!=") == 0)
        rEntry.eOp = SC_NOT_EQUAL;
    else if (IsXMLToken(aOpStr, XML_BOTTOM_PERCENT))
        rEntry.eOp = SC_BOTPERC;
    else if (IsXMLToken(aOpStr, XML_BOTTOM_VALUES))
        rEntry.eOp = SC_BOTVAL;
    else if (IsXMLToken(aOpStr, XML_EMPTY))
        rEntry.SetQueryByEmpty();
    else if (aOpStr.compareToAscii(">") == 0)
        rEntry.eOp = SC_GREATER;
    else if (aOpStr.compareToAscii(">=") == 0)
        rEntry.eOp = SC_GREATER_EQUAL;
    else if (aOpStr.compareToAscii("<") == 0)
        rEntry.eOp = SC_LESS;
    else if (aOpStr.compareToAscii("<=") == 0)
        rEntry.eOp = SC_LESS_EQUAL;
    else if (IsXMLToken(aOpStr, XML_NOEMPTY))
        rEntry.SetQueryByNonEmpty();
    else if (IsXMLToken(aOpStr, XML_TOP_PERCENT))
        rEntry.eOp = SC_TOPPERC;
    else if (IsXMLToken(aOpStr, XML_TOP_VALUES))
        rEntry.eOp = SC_TOPVAL;
    else if (IsXMLToken(aOpStr, XML_CONTAINS))
        rEntry.eOp = SC_CONTAINS;
    else if (IsXMLToken(aOpStr, XML_DOES_NOT_CONTAIN))
        rEntry.eOp = SC_DOES_NOT_CONTAIN;
    else if (IsXMLToken(aOpStr, XML_BEGINS_WITH))
        rEntry.eOp = SC_BEGINS_WITH;
    else if (IsXMLToken(aOpStr, XML_DOES_NOT_BEGIN_WITH))
        rEntry.eOp = SC_DOES_NOT_BEGIN_WITH;
    else if (IsXMLToken(aOpStr, XML_ENDS_WITH))
        rEntry.eOp = SC_ENDS_WITH;
    else if (IsXMLToken(aOpStr, XML_DOES_NOT_END_WITH))
        rEntry.eOp = SC_DOES_NOT_END_WITH;
}

void ScXMLConditionContext::AddSetItem(const ScQueryEntry::Item& rItem)
{
    maQueryItems.push_back(rItem);
}

void ScXMLConditionContext::EndElement()
{
    ScQueryEntry& rEntry = mrQueryParam.AppendEntry();

    // We currently don't support per-condition case sensitivity.
    mrQueryParam.bCaseSens = bIsCaseSensitive;

    rEntry.bDoQuery = true;
    rEntry.eConnect = pFilterContext->GetConnection() ? SC_OR : SC_AND;

    GetOperator(sOperator, mrQueryParam, rEntry);
    SCCOLROW nStartPos = mrQueryParam.bByRow ? mrQueryParam.nCol1 : mrQueryParam.nRow1;
    rEntry.nField = nField + nStartPos;

    if (maQueryItems.empty())
    {
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
        if (IsXMLToken(sDataType, XML_NUMBER))
        {
            rItem.mfVal = sConditionValue.toDouble();
            rItem.meType = ScQueryEntry::ByValue;
        }
        else
        {
            rItem.maString = sConditionValue;
            rItem.meType = ScQueryEntry::ByString;
        }
    }
    else
        rEntry.GetQueryItems().swap(maQueryItems);
}

const ScXMLImport& ScXMLSetItemContext::GetScImport() const
{
    return static_cast<const ScXMLImport&>(GetImport());
}

ScXMLImport& ScXMLSetItemContext::GetScImport()
{
    return static_cast<ScXMLImport&>(GetImport());
}

ScXMLSetItemContext::ScXMLSetItemContext(
    ScXMLImport& rImport, sal_uInt16 nPrfx, const rtl::OUString& rLName,
    const Reference<XAttributeList>& xAttrList, ScXMLConditionContext& rParent) :
    SvXMLImportContext(rImport, nPrfx, rLName)
{
    sal_Int32 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterSetItemAttrTokenMap();
    for (sal_Int32 i = 0; i < nAttrCount; ++i)
    {
        const rtl::OUString& sAttrName = xAttrList->getNameByIndex(i);
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix =
            GetScImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);

        const rtl::OUString& sValue = xAttrList->getValueByIndex(i);

        switch (rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_FILTER_SET_ITEM_ATTR_VALUE:
            {
                ScQueryEntry::Item aItem;
                aItem.maString = sValue;
                aItem.meType = ScQueryEntry::ByString;
                aItem.mfVal = 0.0;
                rParent.AddSetItem(aItem);
            }
            break;
        }
    }
}

SvXMLImportContext* ScXMLSetItemContext::CreateChildContext(
    sal_uInt16 nPrefix, const ::rtl::OUString& rLName,
    const Reference<XAttributeList>& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );;
}

ScXMLSetItemContext::~ScXMLSetItemContext()
{
}

void ScXMLSetItemContext::EndElement()
{
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
    bSkipDuplicates(false),
    bCopyOutputData(false),
    bUseRegularExpressions(false),
    bConnectionOr(true),
    bNextConnectionOr(true),
    bConditionSourceRange(false)
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
                    bCopyOutputData = true;
                }
            }
            break;
            case XML_TOK_FILTER_ATTR_CONDITION_SOURCE_RANGE_ADDRESS :
            {
                sal_Int32 nOffset(0);
                if(ScRangeStringConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ))
                    bConditionSourceRange = true;
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
    if (bCopyOutputData)
        pDataPilotTable->SetFilterOutputPosition(aOutputPosition);

    pDataPilotTable->SetSourceQueryParam(aFilterFields);
    if (bConditionSourceRange)
        pDataPilotTable->SetFilterSourceRange(aConditionSourceRangeAddress);
}

void ScXMLDPFilterContext::AddFilterField (const ScQueryEntry& aFilterField)
{
    aFilterFields.Resize(nFilterFieldCount + 1);
    ScQueryEntry& rEntry(aFilterFields.GetEntry(nFilterFieldCount));
    rEntry = aFilterField;
    rEntry.bDoQuery = true;
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
    pFilterContext->OpenConnection(false);
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
    pFilterContext->OpenConnection(true);
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
    bIsCaseSensitive(false)
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

void ScXMLDPConditionContext::getOperatorXML(
    const rtl::OUString sTempOperator, ScQueryOp& aFilterOperator, bool& bUseRegularExpressions) const
{
    bUseRegularExpressions = false;
    if (IsXMLToken(sTempOperator, XML_MATCH))
    {
        bUseRegularExpressions = true;
        aFilterOperator = SC_EQUAL;
    }
    else if (IsXMLToken(sTempOperator, XML_NOMATCH))
    {
        bUseRegularExpressions = true;
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
    else if (sTempOperator.compareToAscii(">") == 0)
        aFilterOperator = SC_GREATER;
    else if (sTempOperator.compareToAscii(">=") == 0)
        aFilterOperator = SC_GREATER_EQUAL;
    else if (sTempOperator.compareToAscii("<") == 0)
        aFilterOperator = SC_LESS;
    else if (sTempOperator.compareToAscii("<=") == 0)
        aFilterOperator = SC_LESS_EQUAL;
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
    if (IsXMLToken(sOperator, XML_EMPTY))
        aFilterField.SetQueryByEmpty();
    else if (IsXMLToken(sOperator, XML_NOEMPTY))
        aFilterField.SetQueryByNonEmpty();
    else
    {
        bool bUseRegularExpressions = false;
        getOperatorXML(sOperator, aFilterField.eOp, bUseRegularExpressions);
        pFilterContext->SetUseRegularExpressions(bUseRegularExpressions);
        aFilterField.nField = nField;
        ScQueryEntry::Item& rItem = aFilterField.GetQueryItem();
        if (IsXMLToken(sDataType, XML_NUMBER))
        {
            rItem.mfVal = sConditionValue.toDouble();
            rItem.maString = sConditionValue;
            rItem.meType = ScQueryEntry::ByValue;
        }
        else
        {
            rItem.maString = sConditionValue;
            rItem.meType = ScQueryEntry::ByString;
            rItem.mfVal = 0.0;
        }
    }
    pFilterContext->AddFilterField(aFilterField);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
