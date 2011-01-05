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
#include "ChartPlotAreaOASISTContext.hxx"
#include "TransformerBase.hxx"
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include "DeepTContext.hxx"
#include "ActionMapTypesOASIS.hxx"
#include "MutableAttrList.hxx"

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

class XMLAxisOASISContext : public XMLPersElemContentTContext
{
public:
    TYPEINFO();

    XMLAxisOASISContext( XMLTransformerBase& rTransformer,
                         const ::rtl::OUString& rQName,
                         ::rtl::Reference< XMLPersAttrListTContext > & rOutCategoriesContext );
    ~XMLAxisOASISContext();

    virtual XMLTransformerContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::rtl::OUString& rQName,
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
    const ::rtl::OUString& rQName,
    ::rtl::Reference< XMLPersAttrListTContext > & rOutCategoriesContext ) :
        XMLPersElemContentTContext( rTransformer, rQName ),
        m_rCategoriesContext( rOutCategoriesContext ),
        m_bHasCategories( false )
{}

XMLAxisOASISContext::~XMLAxisOASISContext()
{}

XMLTransformerContext * XMLAxisOASISContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const ::rtl::OUString& rQName,
    const Reference< xml::sax::XAttributeList >& xAttrList )
{
    XMLTransformerContext * pContext = 0;

    if( XML_NAMESPACE_CHART == nPrefix &&
        IsXMLToken( rLocalName, XML_CATEGORIES ) )
    {
        // store categories element at parent
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
                OSL_ENSURE( false, "ChartAxis: Invalid attribute value" );
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
    // if we have categories, change the "class" attribute
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
    XMLTransformerBase & rTransformer, const ::rtl::OUString & rQName ) :
        XMLProcAttrTransformerContext( rTransformer, rQName, OASIS_SHAPE_ACTIONS )
{
}

XMLChartPlotAreaOASISTContext::~XMLChartPlotAreaOASISTContext()
{}

XMLTransformerContext * XMLChartPlotAreaOASISTContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const ::rtl::OUString& rLocalName,
    const ::rtl::OUString& rQName,
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
        // export (and forget) categories if found in an axis-element
        // otherwise export regularly
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
