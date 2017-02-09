/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "xmlfilti.hxx"
#include "xmlimprt.hxx"
#include "docuno.hxx"
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"
#include "queryentry.hxx"
#include "document.hxx"

#include <svl/sharedstringpool.hxx>
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
                                        const OUString& rLName,
                                        const Reference<XAttributeList>& xAttrList,
                                        ScQueryParam& rParam,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
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
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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
                if (ScRangeStringConverter::GetRangeFromString( aConditionSourceRangeAddress, sValue, pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ) )
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
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

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
        mrQueryParam.nDestCol = aOutputPosition.Col();
        mrQueryParam.nDestRow = aOutputPosition.Row();
        mrQueryParam.nDestTab = aOutputPosition.Tab();
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

    // The next condition of this stack will get the current connection.
    ++rItem.mnCondCount;

    if (maConnStack.size() < 2)
        // There is no last stack.  Likely the first condition in the first
        // stack whose connection is not used.  Default in
        // ScQueryEntry::eConnect is SC_AND, so return false (AND instead of
        // OR) here. Otherwise, when saving the document again, we'd write a
        // uselessly stacked
        // <table:filter-or><table:filter-and>...</table:filter-and></table:filter-or>
        // for two conditions connected with AND.
        return false;

    std::vector<ConnStackItem>::reverse_iterator itr = maConnStack.rbegin();
    ++itr;
    return itr->mbOr; // connection of the last stack.
}

ScXMLAndContext::ScXMLAndContext( ScXMLImport& rImport,
                                  sal_uInt16 nPrfx,
                                  const OUString& rLName,
                                  const Reference<XAttributeList>& /* xAttrList */,
                                  ScQueryParam& rParam,
                                  ScXMLFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(false);
}

ScXMLAndContext::~ScXMLAndContext()
{
}

SvXMLImportContext *ScXMLAndContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

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
                                const OUString& rLName,
                                const Reference<css::xml::sax::XAttributeList>& /* xAttrList */,
                                ScQueryParam& rParam,
                                ScXMLFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(true);
}

ScXMLOrContext::~ScXMLOrContext()
{
}

SvXMLImportContext *ScXMLOrContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

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
    ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
    const Reference<XAttributeList>& xAttrList,
    ScQueryParam& rParam,
    ScXMLFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext),
    nField(0),
    bIsCaseSensitive(false)
{
    sDataType = GetXMLToken(XML_TEXT);

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetFilterConditionAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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
                                            const OUString& rLName,
                                            const Reference<XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

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
    const OUString& aOpStr, ScQueryParam& rParam, ScQueryEntry& rEntry)
{
    rParam.eSearchType = utl::SearchParam::SearchType::Normal;
    if (IsXMLToken(aOpStr, XML_MATCH))
    {
        rParam.eSearchType = utl::SearchParam::SearchType::Regexp;
        rEntry.eOp = SC_EQUAL;
    }
    else if (IsXMLToken(aOpStr, XML_NOMATCH))
    {
        rParam.eSearchType = utl::SearchParam::SearchType::Regexp;
        rEntry.eOp = SC_NOT_EQUAL;
    }
    else if (aOpStr == "=")
        rEntry.eOp = SC_EQUAL;
    else if (aOpStr == "!=")
        rEntry.eOp = SC_NOT_EQUAL;
    else if (IsXMLToken(aOpStr, XML_BOTTOM_PERCENT))
        rEntry.eOp = SC_BOTPERC;
    else if (IsXMLToken(aOpStr, XML_BOTTOM_VALUES))
        rEntry.eOp = SC_BOTVAL;
    else if (IsXMLToken(aOpStr, XML_EMPTY))
        rEntry.SetQueryByEmpty();
    else if (aOpStr == ">")
        rEntry.eOp = SC_GREATER;
    else if (aOpStr == ">=")
        rEntry.eOp = SC_GREATER_EQUAL;
    else if (aOpStr == "<")
        rEntry.eOp = SC_LESS;
    else if (aOpStr == "<=")
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
            svl::SharedStringPool& rPool = GetScImport().GetDocument()->GetSharedStringPool();
            rItem.maString = rPool.intern(sConditionValue);
            rItem.meType = ScQueryEntry::ByString;
        }
    }
    else
        rEntry.GetQueryItems().swap(maQueryItems);
}

ScXMLSetItemContext::ScXMLSetItemContext(
    ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
    const Reference<XAttributeList>& xAttrList, ScXMLConditionContext& rParent) :
    ScXMLImportContext(rImport, nPrfx, rLName)
{
    sal_Int32 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetFilterSetItemAttrTokenMap();
    for (sal_Int32 i = 0; i < nAttrCount; ++i)
    {
        const OUString& sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetScImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);

        const OUString& sValue = xAttrList->getValueByIndex(i);

        switch (rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_FILTER_SET_ITEM_ATTR_VALUE:
            {
                svl::SharedStringPool& rPool = GetScImport().GetDocument()->GetSharedStringPool();
                ScQueryEntry::Item aItem;
                aItem.maString = rPool.intern(sValue);
                aItem.meType = ScQueryEntry::ByString;
                aItem.mfVal = 0.0;
                rParent.AddSetItem(aItem);
            }
            break;
        }
    }
}

SvXMLImportContext* ScXMLSetItemContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLName,
    const Reference<XAttributeList>& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

ScXMLSetItemContext::~ScXMLSetItemContext()
{
}

void ScXMLSetItemContext::EndElement()
{
}

ScXMLDPFilterContext::ScXMLDPFilterContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTableContext) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
    pDataPilotTable(pTempDataPilotTableContext),
    aFilterFields(),
    eSearchType(utl::SearchParam::SearchType::Normal),
    nFilterFieldCount(0),
    bSkipDuplicates(false),
    bCopyOutputData(false),
    bIsCaseSensitive(false),
    bConnectionOr(true),
    bNextConnectionOr(true),
    bConditionSourceRange(false)
{
    ScDocument* pDoc(GetScImport().GetDocument());

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetFilterAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

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
    aFilterFields.eSearchType = eSearchType;
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
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */,
                                      ScXMLDPFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport, nPrfx, rLName )
{
    pFilterContext = pTempFilterContext;
    pFilterContext->OpenConnection(false);
}

ScXMLDPAndContext::~ScXMLDPAndContext()
{
}

SvXMLImportContext *ScXMLDPAndContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

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
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */,
                                      ScXMLDPFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(true);
}

ScXMLDPOrContext::~ScXMLDPOrContext()
{
}

SvXMLImportContext *ScXMLDPOrContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

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
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLDPFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport, nPrfx, rLName ),
    pFilterContext(pTempFilterContext),
    sDataType(GetXMLToken(XML_TEXT)),
    nField(0),
    bIsCaseSensitive(false)
{

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetFilterConditionAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

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
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLDPConditionContext::getOperatorXML(
    const OUString& sTempOperator, ScQueryOp& aFilterOperator, utl::SearchParam::SearchType& rSearchType)
{
    rSearchType = utl::SearchParam::SearchType::Normal;
    if (IsXMLToken(sTempOperator, XML_MATCH))
    {
        rSearchType = utl::SearchParam::SearchType::Regexp;
        aFilterOperator = SC_EQUAL;
    }
    else if (IsXMLToken(sTempOperator, XML_NOMATCH))
    {
        rSearchType = utl::SearchParam::SearchType::Regexp;
        aFilterOperator = SC_NOT_EQUAL;
    }
    else if (sTempOperator == "=")
        aFilterOperator = SC_EQUAL;
    else if (sTempOperator == "!=")
        aFilterOperator = SC_NOT_EQUAL;
    else if (IsXMLToken(sTempOperator, XML_BOTTOM_PERCENT))
        aFilterOperator = SC_BOTPERC;
    else if (IsXMLToken(sTempOperator, XML_BOTTOM_VALUES))
        aFilterOperator = SC_BOTVAL;
    else if (sTempOperator == ">")
        aFilterOperator = SC_GREATER;
    else if (sTempOperator == ">=")
        aFilterOperator = SC_GREATER_EQUAL;
    else if (sTempOperator == "<")
        aFilterOperator = SC_LESS;
    else if (sTempOperator == "<=")
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
        utl::SearchParam::SearchType eSearchType = utl::SearchParam::SearchType::Normal;
        getOperatorXML(sOperator, aFilterField.eOp, eSearchType);
        pFilterContext->SetSearchType(eSearchType);
        aFilterField.nField = nField;
        ScQueryEntry::Item& rItem = aFilterField.GetQueryItem();
        svl::SharedStringPool& rPool = GetScImport().GetDocument()->GetSharedStringPool();

        if (IsXMLToken(sDataType, XML_NUMBER))
        {
            rItem.mfVal = sConditionValue.toDouble();
            rItem.maString = rPool.intern(sConditionValue);
            rItem.meType = ScQueryEntry::ByValue;
        }
        else
        {
            rItem.maString = rPool.intern(sConditionValue);
            rItem.meType = ScQueryEntry::ByString;
            rItem.mfVal = 0.0;
        }
    }
    pFilterContext->AddFilterField(aFilterField);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
