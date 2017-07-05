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


/*
 * Implementation of the I/O interfaces based on stream and URI binding
 */
#include "xmlsec/xmlstreamio.hxx"
#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>

#include <libxml/uri.h>
#include "xmlsec-wrapper.h"

#define XMLSTREAMIO_INITIALIZED 0x01
#define XMLSTREAMIO_REGISTERED  0x02

/* Global variables */
/*-
 * Enable stream I/O or not.
 */
static char enableXmlStreamIO = 0x00 ;

static css::uno::Reference< css::xml::crypto::XUriBinding > m_xUriBinding ;

extern "C"
int xmlStreamMatch( const char* uri )
{
    css::uno::Reference< css::io::XInputStream > xInputStream ;

    if( ( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) &&
        ( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        if( uri == nullptr || !m_xUriBinding.is() )
            return 0 ;
        //XMLSec first unescapes the uri and  calls this function. For example, we pass the Uri
        //ObjectReplacements/Object%201 then XMLSec passes ObjectReplacements/Object 1
        //first. If this failed it would try this
        //again with the original escaped string. However, it does not get this far, because there
        //is another callback registered by libxml which claims to be able to handle this uri.
        OUString sUri =
            ::rtl::Uri::encode( OUString::createFromAscii( uri ),
            rtl_UriCharClassUric, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
        xInputStream = m_xUriBinding->getUriBinding( sUri ) ;
        if (!xInputStream.is())
        {
            //Try the passed in uri directly.
            //For old documents prior OOo 3.0. We did not use URIs then.
            xInputStream = m_xUriBinding->getUriBinding(
                OUString::createFromAscii(uri));
        }
    }
    if (xInputStream.is())
        return 1;
    else
        return 0 ;
}

extern "C"
void* xmlStreamOpen( const char* uri )
{
    css::uno::Reference< css::io::XInputStream > xInputStream ;

    if( ( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) &&
        ( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        if( uri == nullptr || !m_xUriBinding.is() )
            return nullptr ;

        //see xmlStreamMatch
        OUString sUri =
            ::rtl::Uri::encode( OUString::createFromAscii( uri ),
            rtl_UriCharClassUric, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
        xInputStream = m_xUriBinding->getUriBinding( sUri ) ;
        if (!xInputStream.is())
        {
            //For old documents.
            //try the passed in uri directly.
            xInputStream = m_xUriBinding->getUriBinding(
                OUString::createFromAscii(uri));
        }

        if( xInputStream.is() ) {
            css::io::XInputStream* pInputStream ;
            pInputStream = xInputStream.get() ;
            pInputStream->acquire() ;
            return static_cast<void*>(pInputStream) ;
        }
    }

    return nullptr ;
}

extern "C"
int xmlStreamRead( void* context, char* buffer, int len )
{
    int numbers ;
    css::uno::Reference< css::io::XInputStream > xInputStream ;
    css::uno::Sequence< sal_Int8 > outSeqs( len ) ;

    numbers = 0 ;
    if( ( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) &&
        ( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        if( context != nullptr ) {
            xInputStream = static_cast<css::io::XInputStream*>(context);
            if( !xInputStream.is() )
                return 0 ;

            numbers = xInputStream->readBytes( outSeqs, len ) ;
            const sal_Int8* readBytes = outSeqs.getArray() ;
            for( int i = 0 ; i < numbers ; i ++ )
                *( buffer + i ) = *( readBytes + i ) ;
        }
    }

    return numbers ;
}

extern "C"
int xmlStreamClose( void * context )
{
    if( ( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) &&
        ( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        if( context != nullptr ) {
            css::io::XInputStream* pInputStream ;
            pInputStream = static_cast<css::io::XInputStream*>(context);
            pInputStream->release() ;
        }
    }

    return 0 ;
}

XSECXMLSEC_DLLPUBLIC int xmlEnableStreamInputCallbacks()
{

    if( !( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) ) {
        //Register the callbacks into xmlSec
        //In order to make the xmlsec io finding the callbacks firstly,
        //I put the callbacks at the very beginning.

        //Cleanup the older callbacks.
        //Notes: all none default callbacks will lose.
        xmlSecIOCleanupCallbacks() ;

        //Register my classbacks.
        int cbs = xmlSecIORegisterCallbacks(
                    xmlStreamMatch,
                    xmlStreamOpen,
                    xmlStreamRead,
                    xmlStreamClose ) ;
        if( cbs < 0 ) {
            return -1 ;
        }

        //Register the default callbacks.
        //Notes: the error will cause xmlsec working problems.
        cbs = xmlSecIORegisterDefaultCallbacks() ;
        if( cbs < 0 ) {
            return -1 ;
        }

        enableXmlStreamIO |= XMLSTREAMIO_INITIALIZED ;
    }

    return 0 ;
}

XSECXMLSEC_DLLPUBLIC int xmlRegisterStreamInputCallbacks(
    css::uno::Reference< css::xml::crypto::XUriBinding >& aUriBinding
) {
    if( !( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) ) {
        if( xmlEnableStreamInputCallbacks() < 0 )
            return -1 ;
    }

    if( !( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        enableXmlStreamIO |= XMLSTREAMIO_REGISTERED ;
    }

    m_xUriBinding = aUriBinding ;

    return 0 ;
}

XSECXMLSEC_DLLPUBLIC int xmlUnregisterStreamInputCallbacks()
{
    if( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) {
        //Clear the uri-stream binding
        m_xUriBinding.clear() ;

        //disable the registered flag
        enableXmlStreamIO &= ~XMLSTREAMIO_REGISTERED ;
    }

    return 0 ;
}

XSECXMLSEC_DLLPUBLIC void xmlDisableStreamInputCallbacks() {
    xmlUnregisterStreamInputCallbacks() ;
    enableXmlStreamIO &= ~XMLSTREAMIO_INITIALIZED ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
