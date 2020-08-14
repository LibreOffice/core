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

#include "ChartPlotAreaOOoTContext.hxx"
#include "TransformerBase.hxx"
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include "DeepTContext.hxx"
#include "ActionMapTypesOOo.hxx"
#include "MutableAttrList.hxx"
#include <osl/diagnose.h>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;

class XMLAxisOOoContext : public XMLPersElemContentTContext
{
public:
    XMLAxisOOoContext( XMLTransformerBase& rTransformer,
                       const OUString& rQName );

    virtual void StartElement( const Reference< xml::sax::XAttributeList >& rAttrList ) override;

    bool IsCategoryAxis() const { return m_bIsCategoryAxis;}

private:
    bool m_bIsCategoryAxis;
};

XMLAxisOOoContext::XMLAxisOOoContext(
    XMLTransformerBase& rTransformer,
    const OUString& rQName ) :
        XMLPersElemContentTContext( rTransformer, rQName ),
        m_bIsCategoryAxis( false )
{}

void XMLAxisOOoContext::StartElement(
    const Reference< xml::sax::XAttributeList >& rAttrList )
{
    Reference< xml::sax::XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = nullptr;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_CHART &&
            IsXMLToken( aLocalName, XML_CLASS ) )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList = new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }

            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            XMLTokenEnum eToken = XML_TOKEN_INVALID;
            if( IsXMLToken( rAttrValue, XML_DOMAIN ) ||
                IsXMLToken( rAttrValue, XML_CATEGORY ))
            {
                eToken = XML_X;
                if( IsXMLToken( rAttrValue, XML_CATEGORY ) )
                    m_bIsCategoryAxis = true;
            }
            else if( IsXMLToken( rAttrValue, XML_VALUE ))
            {
                eToken = XML_Y;
            }
            else if( IsXMLToken( rAttrValue, XML_SERIES ))
            {
                eToken = XML_Z;
            }
            else
            {
                OSL_FAIL( "ChartAxis: Invalid attribute value" );
            }

            if( eToken != XML_TOKEN_INVALID )
            {
                OUString aNewAttrQName(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_CHART, GetXMLToken( XML_DIMENSION )));
                pMutableAttrList->RenameAttributeByIndex( i, aNewAttrQName );
                pMutableAttrList->SetValueByIndex( i, GetXMLToken( eToken ));
            }
        }
    }

    XMLPersElemContentTContext::StartElement( xAttrList );
}

XMLChartPlotAreaOOoTContext::XMLChartPlotAreaOOoTContext(
    XMLTransformerBase & rTransformer, const OUString & rQName ) :
        XMLProcAttrTransformerContext( rTransformer, rQName, OOO_SHAPE_ACTIONS )
{
}

rtl::Reference<XMLTransformerContext> XMLChartPlotAreaOOoTContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const OUString& rQName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( XML_NAMESPACE_CHART == nPrefix &&
        IsXMLToken( rLocalName, XML_AXIS ) )
    {
        rtl::Reference<XMLAxisOOoContext> pAxisContext( new XMLAxisOOoContext( GetTransformer(), rQName ));
        AddContent( pAxisContext );
        pContext.set(pAxisContext.get());
    }
    else if( XML_NAMESPACE_CHART == nPrefix &&
             IsXMLToken( rLocalName, XML_CATEGORIES ) )
    {
        pContext.set(new XMLPersAttrListTContext( GetTransformer(), rQName ));

        // put categories at correct axis
        bool bFound =false;

        // iterate over axis elements
        for( const auto& rChildContext : m_aChildContexts )
        {
            XMLAxisOOoContext * pAxisContext = rChildContext.get();
            if( pAxisContext != nullptr )
            {
                // iterate over attributes to find category axis
                Reference< xml::sax::XAttributeList > xNewAttrList( pAxisContext->GetAttrList());
                sal_Int16 nAttrCount = xNewAttrList.is() ? xNewAttrList->getLength() : 0;

                for( sal_Int16 i=0; i < nAttrCount; i++ )
                {
                    const OUString & rAttrName = xNewAttrList->getNameByIndex( i );
                    OUString aLocalName;
                    sal_uInt16 nNewPrefix =
                        GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                             &aLocalName );
                    if( nNewPrefix == XML_NAMESPACE_CHART &&
                        pAxisContext->IsCategoryAxis() &&
                        IsXMLToken( aLocalName, XML_DIMENSION ) )
                    {
                        // category axis found
                        pAxisContext->AddContent( pContext );
                        bFound = true;
                        break;
                    }
                }
            }

            if (bFound)
                break;
        }
        OSL_ENSURE( bFound, "No suitable axis for categories found." );
    }
    else
    {
        ExportContent();
        pContext =  XMLProcAttrTransformerContext::CreateChildContext(
            nPrefix, rLocalName, rQName, xAttrList );
    }

    return pContext;
}

void XMLChartPlotAreaOOoTContext::EndElement()
{
    ExportContent();
    XMLProcAttrTransformerContext::EndElement();
}

void XMLChartPlotAreaOOoTContext::AddContent(rtl::Reference<XMLAxisOOoContext> const & pContext)
{
    OSL_ENSURE( pContext.is() && pContext->IsPersistent(),
                "non-persistent context" );
    m_aChildContexts.push_back(pContext);
}


void XMLChartPlotAreaOOoTContext::ExportContent()
{
    for( auto& rChildContext : m_aChildContexts )
    {
        rChildContext->Export();
    }

    m_aChildContexts.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
