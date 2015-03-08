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

#include "MetaImportComponent.hxx"
#include <xmloff/xmlnmspe.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

// #110680#
XMLMetaImportComponent::XMLMetaImportComponent(
    const uno::Reference< uno::XComponentContext >& xContext) throw()
    :   SvXMLImport(xContext, ""), mxDocProps()
{
}

XMLMetaImportComponent::~XMLMetaImportComponent() throw()
{
}

SvXMLImportContext* XMLMetaImportComponent::CreateContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList > & xAttrList )
{
    if (  (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken(rLocalName, XML_DOCUMENT_META) )
    {
        if (!mxDocProps.is()) {
            throw uno::RuntimeException(OUString(
                "XMLMetaImportComponent::CreateContext: setTargetDocument "
                "has not been called"), *this);
        }
        return new SvXMLMetaDocumentContext(
                        *this, nPrefix, rLocalName, mxDocProps);
    }
    else
    {
        return SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }
}

void SAL_CALL XMLMetaImportComponent::setTargetDocument(
    const uno::Reference< lang::XComponent >& xDoc )
    throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    mxDocProps = uno::Reference< document::XDocumentProperties >::query( xDoc );
    if( !mxDocProps.is() )
        throw lang::IllegalArgumentException(OUString(
            "XMLMetaImportComponent::setTargetDocument: argument is no "
            "XDocumentProperties"), uno::Reference<uno::XInterface>(*this), 0);
}

uno::Sequence< OUString > SAL_CALL
    XMLMetaImportComponent_getSupportedServiceNames()
        throw()
{
    const OUString aServiceName( "com.sun.star.document.XMLOasisMetaImporter" );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL XMLMetaImportComponent_getImplementationName() throw()
{
    return OUString( "XMLMetaImportComponent" );
}


extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
XMLMetaImportComponenet_get_implementation(::com::sun::star::uno::XComponentContext* context,
                                        ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new XMLMetaImportComponent(context));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
