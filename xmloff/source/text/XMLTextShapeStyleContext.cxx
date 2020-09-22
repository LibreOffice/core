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

#include <tools/debug.hxx>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <XMLShapePropertySetContext.hxx>
#include <XMLTextColumnsContext.hxx>
#include <XMLBackgroundImageContext.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>

#include <xmloff/XMLTextShapeStyleContext.hxx>

using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

namespace {

class XMLTextShapePropertySetContext_Impl : public XMLShapePropertySetContext
{
public:
    XMLTextShapePropertySetContext_Impl( SvXMLImport& rImport, sal_Int32 nElement,
        const Reference< XFastAttributeList >& xAttrList,
        sal_uInt32 nFamily,
        ::std::vector< XMLPropertyState > &rProps,
        const rtl::Reference < SvXMLImportPropertyMapper > &rMap );

    using SvXMLPropertySetContext::createFastChildContext;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
        ::std::vector< XMLPropertyState > &rProperties,
        const XMLPropertyState& rProp ) override;
};

}

XMLTextShapePropertySetContext_Impl::XMLTextShapePropertySetContext_Impl(
                 SvXMLImport& rImport, sal_Int32 nElement,
                 const Reference< XFastAttributeList > & xAttrList,
                 sal_uInt32 nFamily,
                 ::std::vector< XMLPropertyState > &rProps,
                 const rtl::Reference < SvXMLImportPropertyMapper > &rMap ) :
    XMLShapePropertySetContext( rImport, nElement, xAttrList, nFamily,
                                rProps, rMap )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextShapePropertySetContext_Impl::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
    ::std::vector< XMLPropertyState > &rProperties,
    const XMLPropertyState& rProp )
{
    SvXMLImportContextRef xContext;

    switch( mxMapper->getPropertySetMapper()
                    ->GetEntryContextId( rProp.mnIndex ) )
    {
    case CTF_TEXTCOLUMNS:
        return new XMLTextColumnsContext( GetImport(), nElement,
                                                   xAttrList, rProp,
                                                   rProperties );
        break;

    case CTF_BACKGROUND_URL:
        DBG_ASSERT( rProp.mnIndex >= 3 &&
                    CTF_BACKGROUND_TRANSPARENCY ==
                        mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-3 ) &&
                    CTF_BACKGROUND_POS  == mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-2 ) &&
                    CTF_BACKGROUND_FILTER  == mxMapper->getPropertySetMapper()
                        ->GetEntryContextId( rProp.mnIndex-1 ),
                    "invalid property map!");
        return
            new XMLBackgroundImageContext( GetImport(), nElement,
                                           xAttrList,
                                           rProp,
                                           rProp.mnIndex-2,
                                           rProp.mnIndex-1,
                                           rProp.mnIndex-3,
                                           -1,
                                           rProperties );
        break;
    }

    return XMLShapePropertySetContext::createFastChildContext(
                        nElement, xAttrList, rProperties, rProp );
}

void XMLTextShapeStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey &&
        IsXMLToken( rLocalName, XML_AUTO_UPDATE ) )
    {
          if( IsXMLToken( rValue, XML_TRUE ) )
            bAutoUpdate = true;
    }
    else
    {
        XMLShapeStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}


constexpr OUStringLiteral gsIsAutoUpdate( u"IsAutoUpdate" );

XMLTextShapeStyleContext::XMLTextShapeStyleContext( SvXMLImport& rImport,
        SvXMLStylesContext& rStyles, XmlStyleFamily nFamily ) :
    XMLShapeStyleContext( rImport, rStyles, nFamily ),
    bAutoUpdate( false )
{
}

XMLTextShapeStyleContext::~XMLTextShapeStyleContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextShapeStyleContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( IsTokenInNamespace(nElement, XML_NAMESPACE_STYLE) ||
        IsTokenInNamespace(nElement, XML_NAMESPACE_LO_EXT) )
    {
        sal_Int32 nLocalName = nElement & TOKEN_MASK;
        sal_uInt32 nFamily = 0;
        if( nLocalName == XML_TEXT_PROPERTIES )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( nLocalName == XML_PARAGRAPH_PROPERTIES )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( nLocalName == XML_GRAPHIC_PROPERTIES )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        if( nFamily )
        {
            rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
            {
                return new XMLTextShapePropertySetContext_Impl(
                        GetImport(), nElement, xAttrList, nFamily,
                        GetProperties(), xImpPrMap );
            }
        }
    }
    else if ( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
    {
        // create and remember events import context
        // (for delayed processing of events)
        xEventContext = new XMLEventsImportContext( GetImport() );
        return xEventContext.get();
    }

    return XMLShapeStyleContext::createFastChildContext( nElement, xAttrList );
}

void XMLTextShapeStyleContext::CreateAndInsert( bool bOverwrite )
{
    XMLShapeStyleContext::CreateAndInsert( bOverwrite );
    Reference < XStyle > xStyle = GetStyle();
    if( !xStyle.is() || !(bOverwrite || IsNew()) )
        return;

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
    if( xPropSetInfo->hasPropertyByName( gsIsAutoUpdate ) )
    {
        bool bTmp = bAutoUpdate;
        xPropSet->setPropertyValue( gsIsAutoUpdate, Any(bTmp) );
    }

    // tell the style about it's events (if applicable)
    if( xEventContext.is() )
    {
        // set event supplier and release reference to context
        Reference<XEventsSupplier> xEventsSupplier(xStyle, UNO_QUERY);
        xEventContext->SetEvents(xEventsSupplier);
        xEventContext = nullptr;
    }
}

void XMLTextShapeStyleContext::Finish( bool bOverwrite )
{
    XMLPropStyleContext::Finish( bOverwrite );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
