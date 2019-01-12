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
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

class XMLMetaImportComponent : public SvXMLImport
{
private:
    css::uno::Reference< css::document::XDocumentProperties> mxDocProps;

public:
    // XMLMetaImportComponent() throw();
    explicit XMLMetaImportComponent(
        const css::uno::Reference< css::uno::XComponentContext >& xContext
        );

protected:

    virtual SvXMLImportContext *CreateFastContext( sal_Int32 nElement,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& xAttrList ) override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;
};

// global functions to support the component

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
XMLMetaImportComponent_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new XMLMetaImportComponent(context));
}

XMLMetaImportComponent::XMLMetaImportComponent(
    const uno::Reference< uno::XComponentContext >& xContext)
    :   SvXMLImport(xContext, ""), mxDocProps()
{
}

SvXMLImportContext *XMLMetaImportComponent::CreateFastContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    if (nElement == XML_ELEMENT( OFFICE, XML_DOCUMENT_META ))
    {
        if (!mxDocProps.is()) {
            throw uno::RuntimeException(
                "XMLMetaImportComponent::CreateFastContext: setTargetDocument "
                "has not been called", *this);
        }
        return new SvXMLMetaDocumentContext(
                        *this, mxDocProps);
    }
    else
    {
        return SvXMLImport::CreateFastContext(nElement, xAttrList);
    }
}

void SAL_CALL XMLMetaImportComponent::setTargetDocument(
    const uno::Reference< lang::XComponent >& xDoc )
{
    mxDocProps.set( xDoc, uno::UNO_QUERY );
    if( !mxDocProps.is() )
        throw lang::IllegalArgumentException(
            "XMLMetaImportComponent::setTargetDocument: argument is no "
            "XDocumentProperties", uno::Reference<uno::XInterface>(*this), 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
