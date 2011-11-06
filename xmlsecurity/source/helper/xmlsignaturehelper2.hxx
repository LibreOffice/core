/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

