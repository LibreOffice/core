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
    class XStream;
    class XOutputStream;
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
    virtual void SAL_CALL startElement( const rtl::OUString& aName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL startDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL endDocument(  )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL endElement( const rtl::OUString& aName )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL characters( const rtl::OUString& aChars )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL ignorableWhitespace( const rtl::OUString& aWhitespaces )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL processingInstruction( const rtl::OUString& aTarget, const rtl::OUString& aData )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setDocumentLocator( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XLocator >& xLocator )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException);
};


// ---------------------------------------------------------------------------------
// XUriBinding
// ---------------------------------------------------------------------------------

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

    void SAL_CALL setUriBinding( const rtl::OUString& uri, const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& aInputStream )
        throw (com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL getUriBinding( const rtl::OUString& uri )
        throw (com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    static com::sun::star::uno::Reference < com::sun::star::io::XInputStream > OpenInputStream( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStore, const rtl::OUString& rURI );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
