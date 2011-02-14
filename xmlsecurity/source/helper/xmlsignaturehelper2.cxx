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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <xmlsignaturehelper2.hxx>

#include <unotools/streamhelper.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XStorageRawAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "rtl/uri.hxx"

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

void SAL_CALL ImplXMLSignatureListener::signatureCreated( sal_Int32 securityId, com::sun::star::xml::crypto::SecurityOperationStatus nResult )
        throw (com::sun::star::uno::RuntimeException)
{
    XMLSignatureCreationResult aResult( securityId, nResult );
    maCreationResultListenerListener.Call( &aResult );
}

void SAL_CALL ImplXMLSignatureListener::signatureVerified( sal_Int32 securityId, com::sun::star::xml::crypto::SecurityOperationStatus nResult )
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


void SAL_CALL UriBindingHelper::setUriBinding( const rtl::OUString& /*uri*/, const uno::Reference< io::XInputStream >&)
    throw (uno::Exception, uno::RuntimeException)
{
}

uno::Reference< io::XInputStream > SAL_CALL UriBindingHelper::getUriBinding( const rtl::OUString& uri )
    throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< io::XInputStream > xInputStream;
    if ( mxStorage.is() )
    {
        xInputStream = OpenInputStream( mxStorage, uri );
    }
    else
    {
        SvFileStream* pStream = new SvFileStream( uri, STREAM_READ );
        pStream->Seek( STREAM_SEEK_TO_END );
        sal_uLong nBytes = pStream->Tell();
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        SvLockBytesRef xLockBytes = new SvLockBytes( pStream, sal_True );
        xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );
    }
    return xInputStream;
}

uno::Reference < io::XInputStream > UriBindingHelper::OpenInputStream( const uno::Reference < embed::XStorage >& rxStore, const rtl::OUString& rURI )
{
    OSL_ASSERT(rURI.getLength());
    uno::Reference < io::XInputStream > xInStream;

    sal_Int32 nSepPos = rURI.indexOf( '/' );
    if ( nSepPos == -1 )
    {
        // Cloning because of I can't keep all storage references open
        // MBA with think about a better API...
        const ::rtl::OUString sName = ::rtl::Uri::decode(
            rURI, rtl_UriDecodeStrict, rtl_UriCharClassRelSegment);
        if (sName.getLength() == 0 && rURI.getLength() != 0)
            throw uno::Exception(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "Could not decode URI for stream element.")), 0);

        uno::Reference< io::XStream > xStream;
        xStream = rxStore->cloneStreamElement( sName );
        if ( !xStream.is() )
            throw uno::RuntimeException();
        xInStream = xStream->getInputStream();
    }
    else
    {
        const rtl::OUString aStoreName = ::rtl::Uri::decode(
            rURI.copy( 0, nSepPos ), rtl_UriDecodeStrict, rtl_UriCharClassRelSegment);
        if (aStoreName.getLength() == 0 && rURI.getLength() != 0)
            throw uno::Exception(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "Could not decode URI for stream element.")), 0);

        rtl::OUString aElement = rURI.copy( nSepPos+1 );
        uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aStoreName, embed::ElementModes::READ );
        xInStream = OpenInputStream( xSubStore, aElement );
    }
    return xInStream;
}


