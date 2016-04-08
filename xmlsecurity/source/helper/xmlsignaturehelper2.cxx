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
#include "xmlsignaturehelper2.hxx"

#include <tools/solar.h>
#include <unotools/streamhelper.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XStorageRawAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/diagnose.h>
#include <rtl/uri.hxx>

using namespace com::sun::star;

ImplXMLSignatureListener::ImplXMLSignatureListener(const Link<XMLSignatureCreationResult&,void>& rCreationResultListenerListener,
                                                   const Link<XMLSignatureVerifyResult&,void>& rVerifyResultListenerListener,
                                                   const Link<LinkParamNone*,void>& rStartSignatureElement)
{
    maCreationResultListenerListener = rCreationResultListenerListener;
    maVerifyResultListenerListener = rVerifyResultListenerListener;
    maStartVerifySignatureElementListener = rStartSignatureElement;
}

ImplXMLSignatureListener::~ImplXMLSignatureListener()
{
}

void ImplXMLSignatureListener::setNextHandler(
    const uno::Reference< xml::sax::XDocumentHandler >& xNextHandler)
{
    m_xNextHandler = xNextHandler;
}

void SAL_CALL ImplXMLSignatureListener::signatureCreated( sal_Int32 securityId, css::xml::crypto::SecurityOperationStatus nResult )
        throw (css::uno::RuntimeException, std::exception)
{
    XMLSignatureCreationResult aResult( securityId, nResult );
    maCreationResultListenerListener.Call( aResult );
}

void SAL_CALL ImplXMLSignatureListener::signatureVerified( sal_Int32 securityId, css::xml::crypto::SecurityOperationStatus nResult )
        throw (css::uno::RuntimeException, std::exception)
{
    XMLSignatureVerifyResult aResult( securityId, nResult );
    maVerifyResultListenerListener.Call( aResult );
}

// XDocumentHandler
void SAL_CALL ImplXMLSignatureListener::startDocument(  )
    throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->startDocument();
    }
}

void SAL_CALL ImplXMLSignatureListener::endDocument(  )
    throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endDocument();
    }
}

void SAL_CALL ImplXMLSignatureListener::startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
{
    if ( aName == "Signature" )
    {
        maStartVerifySignatureElementListener.Call( nullptr );
    }

    if (m_xNextHandler.is())
    {
        m_xNextHandler->startElement( aName, xAttribs );
    }
}

void SAL_CALL ImplXMLSignatureListener::endElement( const OUString& aName )
    throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endElement( aName );
    }
}

void SAL_CALL ImplXMLSignatureListener::characters( const OUString& aChars )
    throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->characters( aChars );
    }
}

void SAL_CALL ImplXMLSignatureListener::ignorableWhitespace( const OUString& aWhitespaces )
    throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->ignorableWhitespace( aWhitespaces );
    }
}

void SAL_CALL ImplXMLSignatureListener::processingInstruction( const OUString& aTarget, const OUString& aData )
    throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->processingInstruction( aTarget, aData );
    }
}

void SAL_CALL ImplXMLSignatureListener::setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
    throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
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

UriBindingHelper::UriBindingHelper( const css::uno::Reference < css::embed::XStorage >& rxStorage )
{
    mxStorage = rxStorage;
}

void SAL_CALL UriBindingHelper::setUriBinding( const OUString& /*uri*/, const uno::Reference< io::XInputStream >&)
    throw (uno::Exception, uno::RuntimeException, std::exception)
{
}

uno::Reference< io::XInputStream > SAL_CALL UriBindingHelper::getUriBinding( const OUString& uri )
    throw (uno::Exception, uno::RuntimeException, std::exception)
{
    uno::Reference< io::XInputStream > xInputStream;
    if ( mxStorage.is() )
    {
        xInputStream = OpenInputStream( mxStorage, uri );
    }
    else
    {
        SvFileStream* pStream = new SvFileStream( uri, StreamMode::READ );
        pStream->Seek( STREAM_SEEK_TO_END );
        sal_uLong nBytes = pStream->Tell();
        pStream->Seek( STREAM_SEEK_TO_BEGIN );
        SvLockBytesRef xLockBytes = new SvLockBytes( pStream, true );
        xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );
    }
    return xInputStream;
}

uno::Reference < io::XInputStream > UriBindingHelper::OpenInputStream( const uno::Reference < embed::XStorage >& rxStore, const OUString& rURI )
{
    OSL_ASSERT(!rURI.isEmpty());
    uno::Reference < io::XInputStream > xInStream;

    OUString aURI(rURI);
    // Ignore leading slash, don't attempt to open a storage with name "".
    if (aURI.startsWith("/"))
        aURI = aURI.copy(1);
    // Ignore query part of the URI.
    sal_Int32 nQueryPos = aURI.indexOf('?');
    if (nQueryPos != -1)
        aURI = aURI.copy(0, nQueryPos);


    sal_Int32 nSepPos = aURI.indexOf( '/' );
    if ( nSepPos == -1 )
    {
        // Cloning because of I can't keep all storage references open
        // MBA with think about a better API...
        const OUString sName = ::rtl::Uri::decode(
            aURI, rtl_UriDecodeStrict, rtl_UriCharClassRelSegment);
        if (sName.isEmpty() && !aURI.isEmpty())
            throw uno::Exception("Could not decode URI for stream element.", nullptr);

        uno::Reference< io::XStream > xStream;
        uno::Reference<container::XNameAccess> xNameAccess(rxStore, uno::UNO_QUERY);
        if (!xNameAccess->hasByName(sName))
            SAL_WARN("xmlsecurity.helper", "expected stream, but not found: " << sName);
        else
            xStream = rxStore->cloneStreamElement( sName );
        if ( !xStream.is() )
            throw uno::RuntimeException();
        xInStream = xStream->getInputStream();
    }
    else
    {
        const OUString aStoreName = ::rtl::Uri::decode(
            aURI.copy( 0, nSepPos ), rtl_UriDecodeStrict, rtl_UriCharClassRelSegment);
        if (aStoreName.isEmpty() && !aURI.isEmpty())
            throw uno::Exception("Could not decode URI for stream element.", nullptr);

        OUString aElement = aURI.copy( nSepPos+1 );
        uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aStoreName, embed::ElementModes::READ );
        xInStream = OpenInputStream( xSubStore, aElement );
    }
    return xInStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
