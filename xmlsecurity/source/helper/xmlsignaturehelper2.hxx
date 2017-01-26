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

#ifndef INCLUDED_XMLSECURITY_SOURCE_HELPER_XMLSIGNATUREHELPER2_HXX
#define INCLUDED_XMLSECURITY_SOURCE_HELPER_XMLSIGNATUREHELPER2_HXX

#include <tools/link.hxx>
#include <rtl/ustring.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultListener.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureVerifyResultListener.hpp>

namespace com {
namespace sun {
namespace star {
namespace io {
    class XInputStream; }
namespace embed {
    class XStorage; }
}}}

// MT: Not needed any more, remove later...

// FIXME: I certainly would love to remove as much as possible of this steaming pile of
// over-engineering, but I wonder what exactly the above comment means. Certainly it is not
// straightforward to remove this class, it is used.

class ImplXMLSignatureListener : public cppu::WeakImplHelper
<
    css::xml::crypto::sax::XSignatureCreationResultListener,
    css::xml::crypto::sax::XSignatureVerifyResultListener,
    css::xml::sax::XDocumentHandler
>
{
private:
    Link<XMLSignatureCreationResult&,void> maCreationResultListenerListener;
    Link<XMLSignatureVerifyResult&,void>   maVerifyResultListenerListener;
    Link<LinkParamNone*,void>              maStartVerifySignatureElementListener;

    css::uno::Reference<
        css::xml::sax::XDocumentHandler > m_xNextHandler;

public:
    ImplXMLSignatureListener(const Link<XMLSignatureCreationResult&,void>& rCreationResultListenerListener,
                             const Link<XMLSignatureVerifyResult&,void>& rVerifyResultListenerListener,
                             const Link<LinkParamNone*, void>& rStartVerifySignatureElement);
    virtual ~ImplXMLSignatureListener() override;

    void setNextHandler(const css::uno::Reference< css::xml::sax::XDocumentHandler >& xNextHandler);

    // css::xml::crypto::sax::XSignatureCreationResultListener
    virtual void SAL_CALL signatureCreated( sal_Int32 securityId, css::xml::crypto::SecurityOperationStatus creationResult ) override;

    // css::xml::crypto::sax::XSignatureVerifyResultListener
    virtual void SAL_CALL signatureVerified( sal_Int32 securityId, css::xml::crypto::SecurityOperationStatus verifyResult ) override;

    // css::xml::sax::XDocumentHandler
    virtual void SAL_CALL startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) override;

    virtual void SAL_CALL startDocument(  ) override;

    virtual void SAL_CALL endDocument(  ) override;

    virtual void SAL_CALL endElement( const OUString& aName ) override;

    virtual void SAL_CALL characters( const OUString& aChars ) override;

    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;

    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) override;

    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;
};

// XUriBinding

class UriBindingHelper : public cppu::WeakImplHelper< css::xml::crypto::XUriBinding >
{
private:
    css::uno::Reference < css::embed::XStorage > mxStorage;

public:
    UriBindingHelper();
    explicit UriBindingHelper( const css::uno::Reference < css::embed::XStorage >& rxStorage );

    void SAL_CALL setUriBinding( const OUString& uri, const css::uno::Reference< css::io::XInputStream >& aInputStream ) override;

    css::uno::Reference< css::io::XInputStream > SAL_CALL getUriBinding( const OUString& uri ) override;

    static css::uno::Reference < css::io::XInputStream > OpenInputStream( const css::uno::Reference < css::embed::XStorage >& rxStore, const OUString& rURI );
};

#endif // INCLUDED_XMLSECURITY_SOURCE_HELPER_XMLSIGNATUREHELPER2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
