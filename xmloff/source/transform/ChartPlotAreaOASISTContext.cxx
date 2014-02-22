/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ChartPlotAreaOASISTContext.hxx"
#include "TransformerBase.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include "DeepTContext.hxx"
#include "ActionMapTypesOASIS.hxx"
#include "MutableAttrList.hxx"

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;

class XMLAxisOASISContext : public XMLPersElemContentTContext
{
public:
    TYPEINFO();

    XMLAxisOASISContext( XMLTransformerBase& rTransformer,
                         const OUString& rQName,
                         ::rtl::Reference< XMLPersAttrListTContext > & rOutCategoriesContext );
    ~XMLAxisOASISContext();

    virtual XMLTransformerContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const Reference< xml::sax::XAttributeList >& rAttrList );
    virtual void EndElement();

    bool IsCategoryAxis() const;

private:
    ::rtl::Reference< XMLPersAttrListTContext > &   m_rCategoriesContext;
    bool                                            m_bHasCategories;
};

TYPEINIT1( XMLAxisOASISContext, XMLPersElemContentTContext );

XMLAxisOASISContext::XMLAxisOASISContext(
    XMLTransformerBase& rTransformer,
    const OUString& rQName,
    ::rtl::Reference< XMLPersAttrListTContext > & rOutCategoriesContext ) :
        XMLPersElemContentTContext( rTransformer, rQName ),
        m_rCategoriesContext( rOutCategoriesContext ),
        m_bHasCategories( false )
{}

XMLAxisOASISContext::~XMLAxisOASISContext()
{}

XMLTransformerContext * XMLAxisOASISContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const OUString& rQName,
    const Reference< xml::sax::XAttributeList >& xAttrList )
{
    XMLTransformerContext * pContext = 0;

    if( XML_NAMESPACE_CHART == nPrefix &&
        IsXMLToken( rLocalName, XML_CATEGORIES ) )
    {
        
        m_rCategoriesContext.set( new XMLPersAttrListTContext( GetTransformer(), rQName ));
        m_bHasCategories = true;
        pContext = m_rCategoriesContext.get();
    }
    else
    {
        pContext =  XMLPersElemContentTContext::CreateChildContext(
            nPrefix, rLocalName, rQName, xAttrList );
    }

    return pContext;
}

void XMLAxisOASISContext::StartElement(
    const Reference< xml::sax::XAttributeList >& rAttrList )
{
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
            IsXMLToken( aLocalName, XML_DIMENSION ) )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList = new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }

            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            XMLTokenEnum eToken = XML_TOKEN_INVALID;
            if( IsXMLToken( rAttrValue, XML_X ))
            {
                eToken = XML_DOMAIN;
                
                
                
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
                OUString aNewAttrQName(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_CHART, GetXMLToken( XML_CLASS )));
                pMutableAttrList->RenameAttributeByIndex( i, aNewAttrQName );

                pMutableAttrList->SetValueByIndex( i, GetXMLToken( eToken ));
            }
        }
    }

    XMLPersElemContentTContext::StartElement( xAttrList );
}

void XMLAxisOASISContext::EndElement()
{
    
    if( IsCategoryAxis() &&
        m_rCategoriesContext.is() )
    {
        OSL_ENSURE( GetAttrList().is(), "Invalid attribute list" );
        XMLMutableAttributeList * pMutableAttrList =
            new XMLMutableAttributeList( GetAttrList());
        OUString aAttrQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
                                 XML_NAMESPACE_CHART, GetXMLToken( XML_CLASS )));
        sal_Int16 nIndex = pMutableAttrList->GetIndexByName( aAttrQName );
        if( nIndex != -1 )
        {
            OSL_ENSURE( IsXMLToken( pMutableAttrList->getValueByIndex( nIndex ),
                                    XML_DOMAIN ), "Axis Dimension: invalid former value" );
            pMutableAttrList->SetValueByIndex( nIndex, GetXMLToken( XML_CATEGORY ));
            OSL_ENSURE( IsXMLToken( pMutableAttrList->getValueByIndex( nIndex ),
                                    XML_CATEGORY ), "Axis Dimension: invalid new value" );
        }

        GetTransformer().GetDocHandler()->startElement(
            GetExportQName(),
            Reference< xml::sax::XAttributeList >( pMutableAttrList ));
        ExportContent();
        GetTransformer().GetDocHandler()->endElement( GetExportQName());
    }
    else
        Export();
}

bool XMLAxisOASISContext::IsCategoryAxis() const
{
    return m_bHasCategories;
}


TYPEINIT1( XMLChartPlotAreaOASISTContext, XMLProcAttrTransformerContext );

XMLChartPlotAreaOASISTContext::XMLChartPlotAreaOASISTContext(
    XMLTransformerBase & rTransformer, const OUString & rQName ) :
        XMLProcAttrTransformerContext( rTransformer, rQName, OASIS_SHAPE_ACTIONS )
{
}

XMLChartPlotAreaOASISTContext::~XMLChartPlotAreaOASISTContext()
{}

XMLTransformerContext * XMLChartPlotAreaOASISTContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const OUString& rQName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( XML_NAMESPACE_CHART == nPrefix &&
        IsXMLToken( rLocalName, XML_AXIS ) )
    {
        pContext = new XMLAxisOASISContext( GetTransformer(), rQName, m_rCategoriesContext );
    }
    else
    {
        
        
        ExportCategories();
        pContext =  XMLProcAttrTransformerContext::CreateChildContext(
                nPrefix, rLocalName, rQName, xAttrList );
    }

    return pContext;
}

void XMLChartPlotAreaOASISTContext::EndElement()
{
    ExportCategories();
    XMLProcAttrTransformerContext::EndElement();
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
