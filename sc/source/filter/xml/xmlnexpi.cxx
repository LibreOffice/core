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

#include <rtl/ustrbuf.hxx>

#include "xmlnexpi.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#include "docuno.hxx"
#include "document.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>

using namespace com::sun::star;

ScXMLNamedExpressionsContext::GlobalInserter::GlobalInserter(ScXMLImport& rImport) : mrImport(rImport) {}

void ScXMLNamedExpressionsContext::GlobalInserter::insert(ScMyNamedExpression* pExp)
{
    if (pExp)
        mrImport.AddNamedExpression(pExp);
}

ScXMLNamedExpressionsContext::SheetLocalInserter::SheetLocalInserter(ScXMLImport& rImport, SCTAB nTab) :
    mrImport(rImport), mnTab(nTab) {}

void ScXMLNamedExpressionsContext::SheetLocalInserter::insert(ScMyNamedExpression* pExp)
{
    mrImport.AddNamedExpression(mnTab, pExp);
}

ScXMLNamedExpressionsContext::ScXMLNamedExpressionsContext(
    ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
    const uno::Reference<xml::sax::XAttributeList>& /* xAttrList */,
    Inserter* pInserter ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpInserter(pInserter)
{
    rImport.LockSolarMutex();
}

ScXMLNamedExpressionsContext::~ScXMLNamedExpressionsContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext *ScXMLNamedExpressionsContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(0);

    const SvXMLTokenMap& rTokenMap(GetScImport().GetNamedExpressionsElemTokenMap());
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_NAMED_EXPRESSIONS_NAMED_RANGE:
        pContext = new ScXMLNamedRangeContext(
            GetScImport(), nPrefix, rLName, xAttrList, mpInserter.get() );
        break;
    case XML_TOK_NAMED_EXPRESSIONS_NAMED_EXPRESSION:
        pContext = new ScXMLNamedExpressionContext(
            GetScImport(), nPrefix, rLName, xAttrList, mpInserter.get() );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLNamedExpressionsContext::EndElement()
{
    // happens in ScXMLImport::EndDocument()
    // because it has to be set after the Database Ranges
}

ScXMLNamedRangeContext::ScXMLNamedRangeContext(
    ScXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
    ScXMLNamedExpressionsContext::Inserter* pInserter ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpInserter(pInserter)
{
    if (!mpInserter)
        return;

    ScMyNamedExpression* pNamedExpression(new ScMyNamedExpression);
    // A simple table:cell-range-address is not a formula expression, stored
    // without [] brackets but with dot, .A1
    pNamedExpression->eGrammar = formula::FormulaGrammar::mergeToGrammar(
            GetScImport().GetDocument()->GetStorageGrammar(),
            formula::FormulaGrammar::CONV_OOO);
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetNamedRangeAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_NAMED_RANGE_ATTR_NAME :
            {
                pNamedExpression->sName = sValue;
            }
            break;
            case XML_TOK_NAMED_RANGE_ATTR_CELL_RANGE_ADDRESS :
            {
                pNamedExpression->sContent = sValue;
            }
            break;
            case XML_TOK_NAMED_RANGE_ATTR_BASE_CELL_ADDRESS :
            {
                pNamedExpression->sBaseCellAddress = sValue;
            }
            break;
            case XML_TOK_NAMED_RANGE_ATTR_RANGE_USABLE_AS :
            {
                pNamedExpression->sRangeType = sValue;
            }
            break;
        }
    }
    pNamedExpression->bIsExpression = false;
    mpInserter->insert(pNamedExpression);
}

ScXMLNamedRangeContext::~ScXMLNamedRangeContext()
{
}

SvXMLImportContext *ScXMLNamedRangeContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLNamedRangeContext::EndElement()
{
}

ScXMLNamedExpressionContext::ScXMLNamedExpressionContext(
    ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList,
    ScXMLNamedExpressionsContext::Inserter* pInserter ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mpInserter(pInserter)
{
    if (!mpInserter)
        return;

    ScMyNamedExpression* pNamedExpression(new ScMyNamedExpression);
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetNamedExpressionAttrTokenMap());
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_NAMED_EXPRESSION_ATTR_NAME :
            {
                pNamedExpression->sName = sValue;
            }
            break;
            case XML_TOK_NAMED_EXPRESSION_ATTR_EXPRESSION :
            {
                GetScImport().ExtractFormulaNamespaceGrammar(
                    pNamedExpression->sContent, pNamedExpression->sContentNmsp,
                    pNamedExpression->eGrammar, sValue );
            }
            break;
            case XML_TOK_NAMED_EXPRESSION_ATTR_BASE_CELL_ADDRESS :
            {
                pNamedExpression->sBaseCellAddress = sValue;
            }
            break;
        }
    }
    pNamedExpression->bIsExpression = true;
    mpInserter->insert(pNamedExpression);
}

ScXMLNamedExpressionContext::~ScXMLNamedExpressionContext()
{
}

SvXMLImportContext *ScXMLNamedExpressionContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<
                                        css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );;
}

void ScXMLNamedExpressionContext::EndElement()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
