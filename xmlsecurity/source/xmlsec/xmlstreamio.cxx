/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


/*
 * Implementation of the I/O interfaces based on stream and URI binding
 */
#include "xmlstreamio.hxx"
#include <rtl/ustring.hxx>
#include "rtl/uri.hxx"

#include <libxml/uri.h>
#include "xmlsecurity/xmlsec-wrapper.h"

#define XMLSTREAMIO_INITIALIZED 0x01
#define XMLSTREAMIO_REGISTERED  0x02

/* Global variables */
/*-
 * Enable stream I/O or not.
 */
static char enableXmlStreamIO = 0x00 ;

::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XUriBinding > m_xUriBinding ;

extern "C"
int xmlStreamMatch( const char* uri )
{
    ::com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xInputStream ;

    if( ( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) &&
        ( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        if( uri == NULL || !m_xUriBinding.is() )
            return 0 ;
        
        
        
        
        
        OUString sUri =
            ::rtl::Uri::encode( OUString::createFromAscii( uri ),
            rtl_UriCharClassUric, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
        xInputStream = m_xUriBinding->getUriBinding( sUri ) ;
        if (!xInputStream.is())
        {
            
            
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
    ::com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xInputStream ;

    if( ( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) &&
        ( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        if( uri == NULL || !m_xUriBinding.is() )
            return NULL ;

        
        OUString sUri =
            ::rtl::Uri::encode( OUString::createFromAscii( uri ),
            rtl_UriCharClassUric, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
        xInputStream = m_xUriBinding->getUriBinding( sUri ) ;
        if (!xInputStream.is())
        {
            
            
            xInputStream = m_xUriBinding->getUriBinding(
                OUString::createFromAscii(uri));
        }

        if( xInputStream.is() ) {
            ::com::sun::star::io::XInputStream* pInputStream ;
            pInputStream = xInputStream.get() ;
            pInputStream->acquire() ;
            return ( void* )pInputStream ;
        }
    }

    return NULL ;
}

extern "C"
int xmlStreamRead( void* context, char* buffer, int len )
{
    int numbers ;
    ::com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xInputStream ;
    ::com::sun::star::uno::Sequence< sal_Int8 > outSeqs( len ) ;

    numbers = 0 ;
    if( ( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) &&
        ( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        if( context != NULL ) {
            xInputStream = ( com::sun::star::io::XInputStream* )context ;
            if( !xInputStream.is() )
                return 0 ;

            numbers = xInputStream->readBytes( outSeqs, len ) ;
            const sal_Int8* readBytes = ( const sal_Int8* )outSeqs.getArray() ;
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
        if( context != NULL ) {
            ::com::sun::star::io::XInputStream* pInputStream ;
            pInputStream = ( ::com::sun::star::io::XInputStream* )context ;
            pInputStream->release() ;
        }
    }

    return 0 ;
}

int xmlEnableStreamInputCallbacks()
{

    if( !( enableXmlStreamIO & XMLSTREAMIO_INITIALIZED ) ) {
        
        
        

        
        
        xmlSecIOCleanupCallbacks() ;

        
        int cbs = xmlSecIORegisterCallbacks(
                    xmlStreamMatch,
                    xmlStreamOpen,
                    xmlStreamRead,
                    xmlStreamClose ) ;
        if( cbs < 0 ) {
            return -1 ;
        }

        
        
        cbs = xmlSecIORegisterDefaultCallbacks() ;
        if( cbs < 0 ) {
            return -1 ;
        }

        enableXmlStreamIO |= XMLSTREAMIO_INITIALIZED ;
    }

    return 0 ;
}

int xmlRegisterStreamInputCallbacks(
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XUriBinding >& aUriBinding
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

int xmlUnregisterStreamInputCallbacks( void )
{
    if( ( enableXmlStreamIO & XMLSTREAMIO_REGISTERED ) ) {
        
        m_xUriBinding.clear() ;

        
        enableXmlStreamIO &= ~XMLSTREAMIO_REGISTERED ;
    }

    return 0 ;
}

void xmlDisableStreamInputCallbacks() {
    xmlUnregisterStreamInputCallbacks() ;
    enableXmlStreamIO &= ~XMLSTREAMIO_INITIALIZED ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
