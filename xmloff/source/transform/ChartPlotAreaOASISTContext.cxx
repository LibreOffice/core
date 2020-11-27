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

#include "ChartPlotAreaOASISTContext.hxx"
#include "TransformerBase.hxx"
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include "DeepTContext.hxx"
#include "ActionMapTypesOASIS.hxx"
#include "MutableAttrList.hxx"
#include <osl/diagnose.h>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;

namespace {

class XMLAxisOASISContext : public XMLPersElemContentTContext
{
public:
    XMLAxisOASISContext( XMLTransformerBase& rTransformer,
                         sal_Int32 nElement,
                         ::rtl::Reference< XMLPersAttrListTContext > & rOutCategoriesContext );

    virtual rtl::Reference<XMLTransformerContext> createFastChildContext( sal_Int32 nElement,
                                   const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void startFastElement(sal_Int32 nElement,
                    const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttribs) override;
    virtual void endFastElement(sal_Int32 Element) override;

private:
    ::rtl::Reference< XMLPersAttrListTContext > &   m_rCategoriesContext;
    bool                                            m_bHasCategories;
};

}

XMLAxisOASISContext::XMLAxisOASISContext(
    XMLTransformerBase& rTransformer,
    sal_Int32 nElement,
    ::rtl::Reference< XMLPersAttrListTContext > & rOutCategoriesContext ) :
        XMLPersElemContentTContext( rTransformer, nElement ),
        m_rCategoriesContext( rOutCategoriesContext ),
        m_bHasCategories( false )
{}

rtl::Reference<XMLTransformerContext> XMLAxisOASISContext::createFastChildContext(
    sal_Int32 nElement,
    const Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( nElement == XML_ELEMENT(CHART, XML_CATEGORIES) )
    {
        // store categories element at parent
        m_rCategoriesContext.set( new XMLPersAttrListTContext( GetTransformer(), nElement ));
        m_bHasCategories = true;
        pContext = m_rCategoriesContext.get();
    }
    else
    {
        pContext =  XMLPersElemContentTContext::createFastChildContext(
            nElement, xAttrList );
    }

    return pContext;
}

void XMLAxisOASISContext::startFastElement(sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    Reference< xml::sax::XFastAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = nullptr;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        if( xAttrList->getTokenByIndex(i) == XML_ELEMENT(CHART, XML_DIMENSION) )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList = new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }

            OUString rAttrValue = xAttrList->getValueByIndex(i);
            XMLTokenEnum eToken = XML_TOKEN_INVALID;
            if( IsXMLToken( rAttrValue, XML_X ))
            {
                eToken = XML_DOMAIN;
                // has to be XML_CATEGORY for axes with a categories
                // sub-element.  The attribute is changed later (when it is
                // known that there is a categories sub-element) in this case.
            }
            else if( IsXMLToken( rAttrValue, XML_Y ))
            {
                eToken = XML_VALUE;
            }
            else if( IsXMLToken( rAttrValue, XML_Z ))
            {
                eToken = XML_SERIES;
            }
            else
            {
                OSL_FAIL( "ChartAxis: Invalid attribute value" );
            }

            if( eToken != XML_TOKEN_INVALID )
            {
                pMutableAttrList->RenameAttributeByIndex( i, XML_ELEMENT(CHART, XML_CLASS)  );
                pMutableAttrList->SetValueByIndex( i, GetXMLToken( eToken ));
            }
        }
    }

    XMLPersElemContentTContext::startFastElement( nElement, xAttrList );
}

void XMLAxisOASISContext::endFastElement(sal_Int32)
{
    // if we have categories, change the "class" attribute
    if( m_bHasCategories &&
        m_rCategoriesContext.is() )
    {
        OSL_ENSURE( GetAttrList().is(), "Invalid attribute list" );
        XMLMutableAttributeList * pMutableAttrList =
            new XMLMutableAttributeList( GetAttrList());
        if( pMutableAttrList->hasAttribute(XML_ELEMENT(CHART, XML_CLASS)) )
        {
            OUString sValue = pMutableAttrList->getValue(XML_ELEMENT(CHART, XML_CLASS));
            OSL_ENSURE( IsXMLToken( sValue,
                                    XML_DOMAIN ), "Axis Dimension: invalid former value" );
            pMutableAttrList->SetValueByToken( XML_ELEMENT(CHART, XML_CLASS), GetXMLToken( XML_CATEGORY ) );
            OSL_ENSURE( IsXMLToken( pMutableAttrList->getValue( XML_ELEMENT(CHART, XML_CLASS) ),
                                    XML_CATEGORY ), "Axis Dimension: invalid new value" );
        }

        GetTransformer().GetDocHandler()->startFastElement(
            GetExportQName(),
            Reference< xml::sax::XFastAttributeList >( pMutableAttrList ));
        ExportContent();
        GetTransformer().GetDocHandler()->endFastElement( GetExportQName());
    }
    else
        Export();
}


XMLChartPlotAreaOASISTContext::XMLChartPlotAreaOASISTContext(
    XMLTransformerBase & rTransformer, sal_Int32 rQName ) :
        XMLProcAttrTransformerContext( rTransformer, rQName, OASIS_SHAPE_ACTIONS )
{
}

XMLChartPlotAreaOASISTContext::~XMLChartPlotAreaOASISTContext()
{}

rtl::Reference<XMLTransformerContext> XMLChartPlotAreaOASISTContext::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( nElement == XML_ELEMENT(CHART, XML_AXIS) )
    {
        pContext.set(new XMLAxisOASISContext( GetTransformer(), nElement, m_rCategoriesContext ));
    }
    else
    {
        // export (and forget) categories if found in an axis-element
        // otherwise export regularly
        ExportCategories();
        pContext =  XMLProcAttrTransformerContext::createFastChildContext(
                nElement, xAttrList );
    }

    return pContext;
}

void XMLChartPlotAreaOASISTContext::endFastElement(sal_Int32 nElement)
{
    ExportCategories();
    XMLProcAttrTransformerContext::endFastElement(nElement);
}

void XMLChartPlotAreaOASISTContext::ExportCategories()
{
    if( m_rCategoriesContext.is())
    {
        m_rCategoriesContext->Export();
        m_rCategoriesContext.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
