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

#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <xmlsignaturehelper2.hxx>

#include <tools/solar.h>
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

// XDocumentHandler
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

void SAL_CALL ImplXMLSignatureListener::startElement( const OUString& aName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if ( aName == OUString("Signature") )
    {
            maStartVerifySignatureElementListener.Call( (void*)&xAttribs );
    }

    if (m_xNextHandler.is())
    {
        m_xNextHandler->startElement( aName, xAttribs );
    }
}

void SAL_CALL ImplXMLSignatureListener::endElement( const OUString& aName )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endElement( aName );
    }
}

void SAL_CALL ImplXMLSignatureListener::characters( const OUString& aChars )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->characters( aChars );
    }
}

void SAL_CALL ImplXMLSignatureListener::ignorableWhitespace( const OUString& aWhitespaces )
    throw (com::sun::star::xml::sax::SAXException, com::sun::star::uno::RuntimeException)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->ignorableWhitespace( aWhitespaces );
    }
}

void SAL_CALL ImplXMLSignatureListener::processingInstruction( const OUString& aTarget, const OUString& aData )
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

// XUriBinding

UriBindingHelper::UriBindingHelper()
{
}

UriBindingHelper::UriBindingHelper( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rxStorage )
{
    mxStorage = rxStorage;
}

void SAL_CALL UriBindingHelper::setUriBinding( const OUString& /*uri*/, const uno::Reference< io::XInputStream >&)
    throw (uno::Exception, uno::RuntimeException)
{
}

uno::Reference< io::XInputStream > SAL_CALL UriBindingHelper::getUriBinding( const OUString& uri )
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

uno::Reference < io::XInputStream > UriBindingHelper::OpenInputStream( const uno::Reference < embed::XStorage >& rxStore, const OUString& rURI )
{
    OSL_ASSERT(!rURI.isEmpty());
    uno::Reference < io::XInputStream > xInStream;

    sal_Int32 nSepPos = rURI.indexOf( '/' );
    if ( nSepPos == -1 )
    {
        // Cloning because of I can't keep all storage references open
        // MBA with think about a better API...
        const OUString sName = ::rtl::Uri::decode(
            rURI, rtl_UriDecodeStrict, rtl_UriCharClassRelSegment);
        if (sName.isEmpty() && !rURI.isEmpty())
            throw uno::Exception("Could not decode URI for stream element.", 0);

        uno::Reference< io::XStream > xStream;
        xStream = rxStore->cloneStreamElement( sName );
        if ( !xStream.is() )
            throw uno::RuntimeException();
        xInStream = xStream->getInputStream();
    }
    else
    {
        const OUString aStoreName = ::rtl::Uri::decode(
            rURI.copy( 0, nSepPos ), rtl_UriDecodeStrict, rtl_UriCharClassRelSegment);
        if (aStoreName.isEmpty() && !rURI.isEmpty())
            throw uno::Exception("Could not decode URI for stream element.", 0);

        OUString aElement = rURI.copy( nSepPos+1 );
        uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aStoreName, embed::ElementModes::READ );
        xInStream = OpenInputStream( xSubStore, aElement );
    }
    return xInStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
