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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------
#include <rtl/ustrbuf.hxx>

#include "xmlnexpi.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#include "docuno.hxx"
#include "global.hxx"
#include "document.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>

#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLNamedExpressionsContext::GlobalInserter::GlobalInserter(ScXMLImport& rImport) : mrImport(rImport) {}

void ScXMLNamedExpressionsContext::GlobalInserter::insert(ScMyNamedExpression* pExp)
{
    if (pExp)
        mrImport.AddNamedExpression(pExp);
}

ScXMLNamedExpressionsContext::SheetLocalInserter::SheetLocalInserter(
    ScDocument* pDoc, ScRangeName& rRangeName) : mpDoc(pDoc), mrRangeName(rRangeName) {}

void ScXMLNamedExpressionsContext::SheetLocalInserter::insert(ScMyNamedExpression* pExp)
{
    using namespace formula;

    ::boost::scoped_ptr<ScMyNamedExpression> p(pExp);

    if (p->sRangeType.getLength() > 0)
        // For now, we only accept normal named expressions.
        return;

    if (mpDoc && !mrRangeName.findByName(p->sName))
    {
        // Insert a new name.
        ScAddress aPos;
        sal_Int32 nOffset = 0;
        bool bSuccess = ScRangeStringConverter::GetAddressFromString(
            aPos, p->sBaseCellAddress, mpDoc, FormulaGrammar::CONV_OOO, nOffset);

        if (bSuccess)
        {
            ::rtl::OUString aContent = p->sContent;
            if (!p->bIsExpression)
                ScXMLConverter::ParseFormula(aContent, false);

            ScRangeData* pData = new ScRangeData(
                mpDoc, p->sName, p->sContent, aPos, RT_NAME, p->eGrammar);
            pData->SetIndex(-1);
            mrRangeName.insert(pData);
        }
    }
}

ScXMLNamedExpressionsContext::ScXMLNamedExpressionsContext(
    ScXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
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
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
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
    // happends in ScXMLImport::EndDocument()
    // because it has to be set after the Database Ranges
}

ScXMLNamedRangeContext::ScXMLNamedRangeContext(
    ScXMLImport& rImport,
    sal_uInt16 nPrfx,
    const ::rtl::OUString& rLName,
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
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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
    pNamedExpression->bIsExpression = sal_False;
    mpInserter->insert(pNamedExpression);
}

ScXMLNamedRangeContext::~ScXMLNamedRangeContext()
{
}

SvXMLImportContext *ScXMLNamedRangeContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLNamedRangeContext::EndElement()
{
}

ScXMLNamedExpressionContext::ScXMLNamedExpressionContext(
    ScXMLImport& rImport, sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
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
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

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
    pNamedExpression->bIsExpression = sal_True;
    mpInserter->insert(pNamedExpression);
}

ScXMLNamedExpressionContext::~ScXMLNamedExpressionContext()
{
}

SvXMLImportContext *ScXMLNamedExpressionContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );;
}

void ScXMLNamedExpressionContext::EndElement()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
