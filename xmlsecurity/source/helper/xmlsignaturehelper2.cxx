/*************************************************************************
 *
 *  $RCSfile: xmlsignaturehelper2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-26 15:45:15 $
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

#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <xmlsignaturehelper2.hxx>

#include <unotools/streamhelper.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

using namespace com::sun::star;

ImplXMLSignatureListener::ImplXMLSignatureListener( const Link& rCreationResultListenerListener, const Link rVerifyResultListenerListener, const Link rStartSignatureElement )
{
    maCreationResultListenerListener = rCreationResultListenerListener;
    maVerifyResultListenerListener = rVerifyResultListenerListener;
    maStartVerifySignatureElementListener = rStartSignatureElement;

}
ImplXMLSignatureListener::~ImplXMLSignatureListener()
{
}

void ImplXMLSignatureListener::setNextHandler(
    uno::Reference< xml::sax::XDocumentHandler > xNextHandler)
{
    m_xNextHandler = xNextHandler;
}

void SAL_CALL ImplXMLSignatureListener::signatureCreated( sal_Int32 securityId, com::sun::star::xml::crypto::sax::SignatureCreationResult nResult )
        throw (com::sun::star::uno::RuntimeException)
{
    XMLSignatureCreationResult aResult( securityId, nResult );
    maCreationResultListenerListener.Call( &aResult );
}

void SAL_CALL ImplXMLSignatureListener::signatureVerified( sal_Int32 securityId, com::sun::star::xml::crypto::sax::SignatureVerifyResult nResult )
        throw (com::sun::star::uno::RuntimeException)
{
    XMLSignatureVerifyResult aResult( securityId, nResult );
    maVerifyResultListenerListener.Call( &aResult );
}

// ---------------------------------------------------------------------------------
// XDocumentHandler
// ---------------------------------------------------------------------------------
void SAL_CALL ImplXMLSignatureListener::startDocument(  )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->startDocument();
    }
}

void SAL_CALL ImplXMLSignatureListener::endDocument(  )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endDocument();
    }
}

void SAL_CALL ImplXMLSignatureListener::startElement( const rtl::OUString& aName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if ( aName == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Signature")) )
    {
            maStartVerifySignatureElementListener.Call( (void*)&xAttribs );
    }

    if (m_xNextHandler.is())
    {
        m_xNextHandler->startElement( aName, xAttribs );
    }
}

void SAL_CALL ImplXMLSignatureListener::endElement( const rtl::OUString& aName )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endElement( aName );
    }
}

void SAL_CALL ImplXMLSignatureListener::characters( const rtl::OUString& aChars )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->characters( aChars );
    }
}

void SAL_CALL ImplXMLSignatureListener::ignorableWhitespace( const rtl::OUString& aWhitespaces )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->ignorableWhitespace( aWhitespaces );
    }
}

void SAL_CALL ImplXMLSignatureListener::processingInstruction( const rtl::OUString& aTarget, const rtl::OUString& aData )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->processingInstruction( aTarget, aData );
    }
}

void SAL_CALL ImplXMLSignatureListener::setDocumentLocator( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XLocator >& xLocator )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->setDocumentLocator( xLocator );
    }
}

// ---------------------------------------------------------------------------------
// XUriBinding
// ---------------------------------------------------------------------------------

UriBindingHelper::UriBindingHelper()
{
}

UriBindingHelper::UriBindingHelper( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStorage )
{
    mxStorage = rxStorage;
}


void SAL_CALL UriBindingHelper::setUriBinding( const rtl::OUString& uri, const uno::Reference< io::XInputStream >& aInputStream )
    throw (uno::Exception, uno::RuntimeException)
{
}

uno::Reference< io::XInputStream > SAL_CALL UriBindingHelper::getUriBinding( const rtl::OUString& uri )
    throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< io::XInputStream > xInputStream;
    if ( mxStorage.is() )
    {
        uno::Reference< io::XStream > xStream = OpenInputStream( mxStorage, uri );
        xInputStream = uno::Reference< io::XInputStream >( xStream, uno::UNO_QUERY );
    }
    else
    {
        SvFileStream* pStream = new SvFileStream( uri, STREAM_READ );
        pStream->Seek( STREAM_SEEK_TO_END );
        ULONG nBytes = pStream->Tell();
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        SvLockBytesRef xLockBytes = new SvLockBytes( pStream, TRUE );
        xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );
    }
    return xInputStream;
}

uno::Reference < io::XStream > UriBindingHelper::OpenInputStream( uno::Reference < embed::XStorage >& rxStore, const rtl::OUString& rURI )
{
    uno::Reference < io::XStream > xStream;

    sal_Int32 nSepPos = rURI.indexOf( '/' );
    if ( nSepPos == -1 )
    {
        // Cloning because of I can't keep all storage references open
        // MBA with think about a better API...
        xStream = rxStore->cloneStreamElement( rURI );
        // Doesnt work, so better for EA to pass exception and not to sign the doc...
        /*
        try
        {
            xStream = rxStore->cloneStreamElement( rURI );
        }
        catch (packages::WrongPasswordException)
        {
            xStream = rxStore->cloneEncryptedStreamElement( rURI, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DUMMY")) );
        }
        */
    }
    else
    {
        rtl::OUString aStoreName = rURI.copy( 0, nSepPos );
        rtl::OUString aElement = rURI.copy( nSepPos+1 );
        uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aStoreName, embed::ElementModes::READ );
        xStream = OpenInputStream( xSubStore, aElement );
    }
    return xStream;
}


