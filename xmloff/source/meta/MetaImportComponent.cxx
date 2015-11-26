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

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

class XMLMetaImportComponent : public SvXMLImport
{
private:
    css::uno::Reference< css::document::XDocumentProperties> mxDocProps;

public:
    // XMLMetaImportComponent() throw();
    XMLMetaImportComponent(
        const css::uno::Reference< css::uno::XComponentContext >& xContext
        ) throw();

    virtual ~XMLMetaImportComponent() throw();

protected:

    virtual SvXMLImportContext* CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
        throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
};

// global functions to support the component

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
XMLMetaImportComponent_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new XMLMetaImportComponent(context));
}

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
    mxDocProps.set( xDoc, uno::UNO_QUERY );
    if( !mxDocProps.is() )
        throw lang::IllegalArgumentException(OUString(
            "XMLMetaImportComponent::setTargetDocument: argument is no "
            "XDocumentProperties"), uno::Reference<uno::XInterface>(*this), 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
