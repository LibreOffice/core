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

#include "xmlnexpi.hxx"
#include "xmlimprt.hxx"
#include <document.hxx>

#include <xmloff/xmlnmspe.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

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
    ScXMLImport& rImport,
    Inserter* pInserter ) :
    ScXMLImportContext( rImport ),
    mpInserter(pInserter)
{
    rImport.LockSolarMutex();
}

ScXMLNamedExpressionsContext::~ScXMLNamedExpressionsContext()
{
    GetScImport().UnlockSolarMutex();
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLNamedExpressionsContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
        case XML_ELEMENT( TABLE, XML_NAMED_RANGE ):
            pContext = new ScXMLNamedRangeContext(
                GetScImport(), pAttribList, mpInserter.get() );
            break;
        case XML_ELEMENT( TABLE, XML_NAMED_EXPRESSION ):
            pContext = new ScXMLNamedExpressionContext(
                GetScImport(), pAttribList, mpInserter.get() );
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLNamedRangeContext::ScXMLNamedRangeContext(
    ScXMLImport& rImport,
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
    ScXMLNamedExpressionsContext::Inserter* pInserter ) :
    ScXMLImportContext( rImport )
{
    if (!pInserter)
        return;

    ScMyNamedExpression* pNamedExpression(new ScMyNamedExpression);
    // A simple table:cell-range-address is not a formula expression, stored
    // without [] brackets but with dot, .A1
    pNamedExpression->eGrammar = formula::FormulaGrammar::mergeToGrammar(
            GetScImport().GetDocument()->GetStorageGrammar(),
            formula::FormulaGrammar::CONV_OOO);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_NAME ):
                    pNamedExpression->sName = aIter.toString();
                    break;
                case XML_ELEMENT( TABLE, XML_CELL_RANGE_ADDRESS ):
                    pNamedExpression->sContent = aIter.toString();
                    break;
                case XML_ELEMENT( TABLE, XML_BASE_CELL_ADDRESS ):
                    pNamedExpression->sBaseCellAddress = aIter.toString();
                    break;
                case XML_ELEMENT( TABLE, XML_RANGE_USABLE_AS ):
                    pNamedExpression->sRangeType = aIter.toString();
                    break;
            }
        }
    }
    pNamedExpression->bIsExpression = false;
    pInserter->insert(pNamedExpression);
}

ScXMLNamedRangeContext::~ScXMLNamedRangeContext()
{
}

ScXMLNamedExpressionContext::ScXMLNamedExpressionContext(
    ScXMLImport& rImport,
    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
    ScXMLNamedExpressionsContext::Inserter* pInserter ) :
    ScXMLImportContext( rImport )
{
    if (!pInserter)
        return;

    ScMyNamedExpression* pNamedExpression(new ScMyNamedExpression);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_NAME ):
                    pNamedExpression->sName = aIter.toString();
                    break;
                case XML_ELEMENT( TABLE, XML_EXPRESSION ):
                    GetScImport().ExtractFormulaNamespaceGrammar(
                        pNamedExpression->sContent, pNamedExpression->sContentNmsp,
                        pNamedExpression->eGrammar, aIter.toString() );
                    break;
                case XML_ELEMENT( TABLE, XML_BASE_CELL_ADDRESS ):
                    pNamedExpression->sBaseCellAddress = aIter.toString();
                    break;
            }
        }
    }
    pNamedExpression->bIsExpression = true;
    pInserter->insert(pNamedExpression);
}

ScXMLNamedExpressionContext::~ScXMLNamedExpressionContext()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
