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

#include <tools/link.hxx>
#include <rtl/ustring.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

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

class ImplXMLSignatureListener : public cppu::WeakImplHelper3
<
    com::sun::star::xml::crypto::sax::XSignatureCreationResultListener,
    com::sun::star::xml::crypto::sax::XSignatureVerifyResultListener,
    com::sun::star::xml::sax::XDocumentHandler
>
{
private:
    Link        maCreationResultListenerListener;
    Link        maVerifyResultListenerListener;
    Link        maStartVerifySignatureElementListener;

    com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler > m_xNextHandler;

public:
    ImplXMLSignatureListener( const Link& rCreationResultListenerListener, const Link rVerifyResultListenerListener, const Link rStartVerifySignatureElement );
    ~ImplXMLSignatureListener();

    void setNextHandler(com::sun::star::uno::Reference<
        com::sun::star::xml::sax::XDocumentHandler > xNextHandler);

    // com::sun::star::xml::crypto::sax::XSignatureCreationResultListener
    virtual void SAL_CALL signatureCreated( sal_Int32 securityId, com::sun::star::xml::crypto::SecurityOperationStatus creationResult )
        throw (com::sun::star::uno::RuntimeException);

    // com::sun::star::xml::crypto::sax::XSignatureVerifyResultListener
    virtual void SAL_CALL signatureVerified( sal_Int32 securityId, com::sun::star::xml::crypto::SecurityOperationStatus verifyResult )
        throw (com::sun::star::uno::RuntimeException);

    // com::sun::star::xml::sax::XDocumentHandler
    virtual void SAL_CALL startElement( const OUString& aName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL startDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL endDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL endElement( const OUString& aName )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL characters( const OUString& aChars )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setDocumentLocator( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XLocator >& xLocator )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
};

// XUriBinding

class UriBindingHelper : public cppu::WeakImplHelper1
<
    com::sun::star::xml::crypto::XUriBinding
>
{
private:
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > mxStorage;

public:
    UriBindingHelper();
    UriBindingHelper( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStorage );

    void SAL_CALL setUriBinding( const OUString& uri, const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& aInputStream )
        throw (com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL getUriBinding( const OUString& uri )
        throw (com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    static com::sun::star::uno::Reference < com::sun::star::io::XInputStream > OpenInputStream( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStore, const OUString& rURI );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
