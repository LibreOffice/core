/*************************************************************************
 *
 *  $RCSfile: xmlsignaturehelper2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-16 15:58:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    virtual void SAL_CALL signatureCreated( sal_Int32 securityId, com::sun::star::xml::crypto::sax::SignatureCreationResult creationResult )
        throw (com::sun::star::uno::RuntimeException);

    // com::sun::star::xml::crypto::sax::XSignatureVerifyResultListener
    virtual void SAL_CALL signatureVerified( sal_Int32 securityId, com::sun::star::xml::crypto::sax::SignatureVerifyResult verifyResult )
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

    static com::sun::star::uno::Reference < com::sun::star::io::XStream > OpenInputStream( com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStore, const rtl::OUString& rURI );
};

