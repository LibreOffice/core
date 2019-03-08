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
#include "xmldrani.hxx"
#include "xmldpimp.hxx"
#include <rangeutl.hxx>
#include <queryentry.hxx>
#include <document.hxx>

#include <o3tl/safeint.hxx>
#include <svl/sharedstringpool.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;

ScXMLFilterContext::ConnStackItem::ConnStackItem(bool bOr) : mbOr(bOr), mnCondCount(0) {}

ScXMLFilterContext::ScXMLFilterContext( ScXMLImport& rImport,
                                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                        ScQueryParam& rParam,
                                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext) :
    ScXMLImportContext( rImport ),
    mrQueryParam(rParam),
    pDatabaseRangeContext(pTempDatabaseRangeContext),
    bSkipDuplicates(false),
    bCopyOutputData(false),
    bConditionSourceRange(false)
{
    ScDocument* pDoc(GetScImport().GetDocument());

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_TARGET_RANGE_ADDRESS ):
                {
                    ScRange aScRange;
                    sal_Int32 nOffset(0);
                    if (ScRangeStringConverter::GetRangeFromString( aScRange, aIter.toString(), pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ))
                    {
                        aOutputPosition = aScRange.aStart;
                        bCopyOutputData = true;
                    }
                }
                break;
                case XML_ELEMENT( TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS ):
                {
                    sal_Int32 nOffset(0);
                    if (ScRangeStringConverter::GetRangeFromString( aConditionSourceRangeAddress, aIter.toString(), pDoc, ::formula::FormulaGrammar::CONV_OOO, nOffset ) )
                        bConditionSourceRange = true;
                }
                break;
                case XML_ELEMENT( TABLE, XML_CONDITION_SOURCE ):
                {
                    // not supported by StarOffice
                }
                break;
                case XML_ELEMENT( TABLE, XML_DISPLAY_DUPLICATES ):
                {
                    bSkipDuplicates = !IsXMLToken(aIter, XML_TRUE);
                }
                break;
            }
        }
    }
}

ScXMLFilterContext::~ScXMLFilterContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLFilterContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_FILTER_AND ):
        {
            pContext = new ScXMLAndContext(
                GetScImport(), mrQueryParam, this);
        }
        break;
        case XML_ELEMENT( TABLE, XML_FILTER_OR ):
        {
            pContext = new ScXMLOrContext(
                GetScImport(), mrQueryParam, this);
        }
        break;
        case XML_ELEMENT( TABLE, XML_FILTER_CONDITION ):
        {
            pContext = new ScXMLConditionContext(
                GetScImport(), nElement, pAttribList, mrQueryParam, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLFilterContext::endFastElement( sal_Int32 /*nElement*/ )
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
    maConnStack.emplace_back(b);
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
                                  ScQueryParam& rParam,
                                  ScXMLFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(false);
}

ScXMLAndContext::~ScXMLAndContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLAndContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_FILTER_OR ):
        {
            // not supported in StarOffice
        }
        break;
        case XML_ELEMENT( TABLE, XML_FILTER_CONDITION ):
        {
            pContext = new ScXMLConditionContext(
                GetScImport(), nElement, pAttribList, mrQueryParam, pFilterContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLAndContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pFilterContext->CloseConnection();
}

ScXMLOrContext::ScXMLOrContext( ScXMLImport& rImport,
                                ScQueryParam& rParam,
                                ScXMLFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(true);
}

ScXMLOrContext::~ScXMLOrContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLOrContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_FILTER_AND ):
        {
            pContext = new ScXMLAndContext(
                GetScImport(), mrQueryParam, pFilterContext);
        }
        break;
        case XML_ELEMENT( TABLE, XML_FILTER_CONDITION ):
        {
            pContext = new ScXMLConditionContext(
                GetScImport(), nElement, pAttribList, mrQueryParam, pFilterContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLOrContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pFilterContext->CloseConnection();
}

ScXMLConditionContext::ScXMLConditionContext(
    ScXMLImport& rImport, sal_Int32 /*nElement*/,
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
    ScQueryParam& rParam,
    ScXMLFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport ),
    mrQueryParam(rParam),
    pFilterContext(pTempFilterContext),
    nField(0),
    bIsCaseSensitive(false)
{
    sDataType = GetXMLToken(XML_TEXT);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_FIELD_NUMBER ):
                {
                    nField = aIter.toInt32();
                }
                break;
                case XML_ELEMENT( TABLE, XML_CASE_SENSITIVE ):
                {
                    bIsCaseSensitive = IsXMLToken(aIter, XML_TRUE);
                }
                break;
                case XML_ELEMENT( TABLE, XML_DATA_TYPE ):
                {
                    sDataType = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_VALUE ):
                {
                    sConditionValue = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_OPERATOR ):
                {
                    sOperator = aIter.toString();
                }
                break;
            }
        }
    }
}

ScXMLConditionContext::~ScXMLConditionContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLConditionContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_FILTER_SET_ITEM ):
        {
            pContext = new ScXMLSetItemContext(
                GetScImport(), nElement, pAttribList, *this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

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

void SAL_CALL ScXMLConditionContext::endFastElement( sal_Int32 /*nElement*/ )
{
    ScQueryEntry& rEntry = mrQueryParam.AppendEntry();

    // We currently don't support per-condition case sensitivity.
    mrQueryParam.bCaseSens = bIsCaseSensitive;

    rEntry.bDoQuery = true;
    rEntry.eConnect = pFilterContext->GetConnection() ? SC_OR : SC_AND;

    GetOperator(sOperator, mrQueryParam, rEntry);
    SCCOLROW nStartPos = mrQueryParam.bByRow ? mrQueryParam.nCol1 : mrQueryParam.nRow1;
    rEntry.nField = o3tl::saturating_add(nField, nStartPos);

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
    ScXMLImport& rImport, sal_Int32 /*nElement*/,
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList, ScXMLConditionContext& rParent) :
    ScXMLImportContext(rImport)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_VALUE ):
                {
                    svl::SharedStringPool& rPool = GetScImport().GetDocument()->GetSharedStringPool();
                    ScQueryEntry::Item aItem;
                    aItem.maString = rPool.intern(aIter.toString());
                    aItem.meType = ScQueryEntry::ByString;
                    aItem.mfVal = 0.0;
                    rParent.AddSetItem(aItem);
                }
                break;
            }
        }
    }
}

ScXMLSetItemContext::~ScXMLSetItemContext()
{
}

ScXMLDPFilterContext::ScXMLDPFilterContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDataPilotTableContext* pTempDataPilotTableContext) :
    ScXMLImportContext( rImport ),
    pDataPilotTable(pTempDataPilotTableContext),
    aFilterFields(),
    eSearchType(utl::SearchParam::SearchType::Normal),
    nFilterFieldCount(0),
    bSkipDuplicates(false),
    bIsCaseSensitive(false),
    bConnectionOr(true),
    bNextConnectionOr(true)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_TARGET_RANGE_ADDRESS ):
                {
                    // not supported
                }
                break;
                case XML_ELEMENT( TABLE, XML_CONDITION_SOURCE_RANGE_ADDRESS ):
                {
                    // not supported
                }
                break;
                case XML_ELEMENT( TABLE, XML_CONDITION_SOURCE ):
                {
                    // not supported by StarOffice
                }
                break;
                case XML_ELEMENT( TABLE, XML_DISPLAY_DUPLICATES ):
                {
                    bSkipDuplicates = !IsXMLToken(aIter, XML_TRUE);
                }
                break;
                }
        }
    }
}

ScXMLDPFilterContext::~ScXMLDPFilterContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDPFilterContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_FILTER_AND ):
        {
            pContext = new ScXMLDPAndContext( GetScImport(), this);
        }
        break;
        case XML_ELEMENT( TABLE, XML_FILTER_OR ):
        {
            pContext = new ScXMLDPOrContext( GetScImport(), this);
        }
        break;
        case XML_ELEMENT( TABLE, XML_FILTER_CONDITION ):
        {
            pContext = new ScXMLDPConditionContext( GetScImport(), nElement, pAttribList, this);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLDPFilterContext::endFastElement( sal_Int32 /*nElement*/ )
{
    aFilterFields.eSearchType = eSearchType;
    aFilterFields.bCaseSens = bIsCaseSensitive;
    aFilterFields.bDuplicate = !bSkipDuplicates;

    pDataPilotTable->SetSourceQueryParam(aFilterFields);
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
                                      ScXMLDPFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport )
{
    pFilterContext = pTempFilterContext;
    pFilterContext->OpenConnection(false);
}

ScXMLDPAndContext::~ScXMLDPAndContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDPAndContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_FILTER_OR ):
        {
            // not supported in StarOffice
        }
        break;
        case XML_ELEMENT( TABLE, XML_FILTER_CONDITION ):
        {
            pContext = new ScXMLDPConditionContext( GetScImport(), nElement, pAttribList, pFilterContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLDPAndContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pFilterContext->CloseConnection();
}

ScXMLDPOrContext::ScXMLDPOrContext( ScXMLImport& rImport,
                                    ScXMLDPFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport ),
    pFilterContext(pTempFilterContext)
{
    pFilterContext->OpenConnection(true);
}

ScXMLDPOrContext::~ScXMLDPOrContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDPOrContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_FILTER_AND ):
        {
            pContext = new ScXMLDPAndContext( GetScImport(), pFilterContext);
        }
        break;
        case XML_ELEMENT( TABLE, XML_FILTER_CONDITION ):
        {
            pContext = new ScXMLDPConditionContext( GetScImport(), nElement, pAttribList, pFilterContext);
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLDPOrContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pFilterContext->CloseConnection();
}

ScXMLDPConditionContext::ScXMLDPConditionContext( ScXMLImport& rImport,
                                      sal_Int32 /*nElement*/,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLDPFilterContext* pTempFilterContext) :
    ScXMLImportContext( rImport ),
    pFilterContext(pTempFilterContext),
    sDataType(GetXMLToken(XML_TEXT)),
    nField(0),
    bIsCaseSensitive(false)
{

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_FIELD_NUMBER ):
                {
                    nField = aIter.toInt32();
                }
                break;
                case XML_ELEMENT( TABLE, XML_CASE_SENSITIVE ):
                {
                    bIsCaseSensitive = IsXMLToken(aIter, XML_TRUE);
                }
                break;
                case XML_ELEMENT( TABLE, XML_DATA_TYPE ):
                {
                    sDataType = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_VALUE ):
                {
                    sConditionValue = aIter.toString();
                }
                break;
                case XML_ELEMENT( TABLE, XML_OPERATOR ):
                {
                    sOperator = aIter.toString();
                }
                break;
            }
        }
    }
}

ScXMLDPConditionContext::~ScXMLDPConditionContext()
{
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

void SAL_CALL ScXMLDPConditionContext::endFastElement( sal_Int32 /*nElement*/ )
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
