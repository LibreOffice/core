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
#include "xmloff/xmlnmspe.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/nmspmap.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

// #110680#
XMLMetaImportComponent::XMLMetaImportComponent(
    const uno::Reference< uno::XComponentContext >& xContext) throw()
    :   SvXMLImport(xContext), mxDocProps()
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
    throw(lang::IllegalArgumentException, uno::RuntimeException)
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

uno::Reference< uno::XInterface > SAL_CALL XMLMetaImportComponent_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    // #110680#
    return (cppu::OWeakObject*)new XMLMetaImportComponent( comphelper::getComponentContext(rSMgr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
