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


#include "MetaImportComponent.hxx"
#include "xmloff/xmlnmspe.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/nmspmap.hxx>

#include <com/sun/star/xml/dom/SAXDocumentBuilder.hpp>
#include <comphelper/componentcontext.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;


//===========================================================================

// #110680#
XMLMetaImportComponent::XMLMetaImportComponent(
    const uno::Reference< lang::XMultiServiceFactory >& xServiceFactory) throw()
    :   SvXMLImport(xServiceFactory), mxDocProps()
{
}

XMLMetaImportComponent::~XMLMetaImportComponent() throw()
{
}


SvXMLImportContext* XMLMetaImportComponent::CreateContext(
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList > & xAttrList )
{
    if (  (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken(rLocalName, XML_DOCUMENT_META) )
    {
        if (!mxDocProps.is()) {
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "XMLMetaImportComponent::CreateContext: setTargetDocument "
                "has not been called")), *this);
        }
        uno::Reference<xml::sax::XDocumentHandler> xDocBuilder(
            xml::dom::SAXDocumentBuilder::create(comphelper::ComponentContext(mxServiceFactory).getUNOContext()),
                 uno::UNO_QUERY_THROW);
        return new SvXMLMetaDocumentContext(
                        *this, nPrefix, rLocalName, mxDocProps, xDocBuilder);
    }
    else
    {
        return SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }
}

void SAL_CALL XMLMetaImportComponent::setTargetDocument(
    const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    mxDocProps = uno::Reference< document::XDocumentProperties >::query( xDoc );
    if( !mxDocProps.is() )
        throw lang::IllegalArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "XMLMetaImportComponent::setTargetDocument: argument is no "
            "XDocumentProperties")), uno::Reference<uno::XInterface>(*this), 0);
}

uno::Sequence< rtl::OUString > SAL_CALL
    XMLMetaImportComponent_getSupportedServiceNames()
        throw()
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.document.XMLOasisMetaImporter" ) );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

rtl::OUString SAL_CALL XMLMetaImportComponent_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLMetaImportComponent" ) );
}

uno::Reference< uno::XInterface > SAL_CALL XMLMetaImportComponent_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    // #110680#
    return (cppu::OWeakObject*)new XMLMetaImportComponent(rSMgr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
