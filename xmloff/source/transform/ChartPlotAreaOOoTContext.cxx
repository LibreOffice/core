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
#include "precompiled_xmloff.hxx"
#include "ChartPlotAreaOOoTContext.hxx"
#include "TransformerBase.hxx"
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include "DeepTContext.hxx"
#include "ActionMapTypesOOo.hxx"
#include "MutableAttrList.hxx"

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

class XMLAxisOOoContext : public XMLPersElemContentTContext
{
public:
    TYPEINFO();

    XMLAxisOOoContext( XMLTransformerBase& rTransformer,
                       const ::rtl::OUString& rQName );
    ~XMLAxisOOoContext();

    virtual void StartElement( const Reference< xml::sax::XAttributeList >& rAttrList );

    bool IsCategoryAxis() const;

private:
    bool m_bIsCategoryAxis;
};

TYPEINIT1( XMLAxisOOoContext, XMLPersElemContentTContext );

XMLAxisOOoContext::XMLAxisOOoContext(
    XMLTransformerBase& rTransformer,
    const ::rtl::OUString& rQName ) :
        XMLPersElemContentTContext( rTransformer, rQName ),
        m_bIsCategoryAxis( false )
{}

XMLAxisOOoContext::~XMLAxisOOoContext()
{}

void XMLAxisOOoContext::StartElement(
    const Reference< xml::sax::XAttributeList >& rAttrList )
{
    OUString aLocation, aMacroName;
    Reference< xml::sax::XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = 0;
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

bool XMLAxisOOoContext::IsCategoryAxis() const
{
    return m_bIsCategoryAxis;
}


TYPEINIT1( XMLChartPlotAreaOOoTContext, XMLProcAttrTransformerContext )

XMLChartPlotAreaOOoTContext::XMLChartPlotAreaOOoTContext(
    XMLTransformerBase & rTransformer, const ::rtl::OUString & rQName ) :
        XMLProcAttrTransformerContext( rTransformer, rQName, OOO_SHAPE_ACTIONS )
{
}

XMLChartPlotAreaOOoTContext::~XMLChartPlotAreaOOoTContext()
{}

XMLTransformerContext * XMLChartPlotAreaOOoTContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const ::rtl::OUString& rQName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( XML_NAMESPACE_CHART == nPrefix &&
        IsXMLToken( rLocalName, XML_AXIS ) )
    {
        XMLAxisOOoContext * pAxisContext( new XMLAxisOOoContext( GetTransformer(), rQName ));
        AddContent( pAxisContext );
        pContext = pAxisContext;
    }
    else if( XML_NAMESPACE_CHART == nPrefix &&
             IsXMLToken( rLocalName, XML_CATEGORIES ) )
    {
        pContext = new XMLPersAttrListTContext( GetTransformer(), rQName );

        // put categories at correct axis
        XMLAxisContextVector::iterator aIter = m_aChildContexts.begin();
        bool bFound =false;

        // iterate over axis elements
        for( ; ! bFound && aIter != m_aChildContexts.end(); ++aIter )
        {
            XMLAxisOOoContext * pAxisContext = (*aIter).get();
            if( pAxisContext != 0 )
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

void XMLChartPlotAreaOOoTContext::AddContent( XMLAxisOOoContext *pContext )
{
    OSL_ENSURE( pContext && pContext->IsPersistent(),
                "non-persistent context" );
    XMLAxisContextVector::value_type aVal( pContext );
    m_aChildContexts.push_back( aVal );
}


void XMLChartPlotAreaOOoTContext::ExportContent()
{
    XMLAxisContextVector::iterator aIter = m_aChildContexts.begin();

    for( ; aIter != m_aChildContexts.end(); ++aIter )
    {
        (*aIter)->Export();
    }

    m_aChildContexts.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
