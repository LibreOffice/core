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

#include <sal/config.h>
#include <xmlsec-wrapper.h>
#include <xmlsec/io.h>

/*
 * Implementation of the I/O interfaces based on stream and URI binding
 */
#include <xmlsec/xmlstreamio.hxx>
#include <xmlsec/errorcallback.hxx>
#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>
#include <comphelper/scopeguard.hxx>
#include <sal/log.hxx>

#include <com/sun/star/xml/crypto/XUriBinding.hpp>

static bool g_bInputCallbacksEnabled = false;
static bool g_bInputCallbacksRegistered = false;

static css::uno::Reference< css::xml::crypto::XUriBinding > m_xUriBinding ;

extern "C" {

static int xmlStreamMatch( const char* uri )
{
    css::uno::Reference< css::io::XInputStream > xInputStream ;

    if (g_bInputCallbacksEnabled && g_bInputCallbacksRegistered)
    {
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
    SAL_INFO("xmlsecurity.xmlsec",
             "xmlStreamMath: uri is '" << uri << "', returning " << xInputStream.is());
    if (xInputStream.is())
        return 1;
    else
        return 0 ;
}

static void* xmlStreamOpen( const char* uri )
{
    css::uno::Reference< css::io::XInputStream > xInputStream ;

    if (g_bInputCallbacksEnabled && g_bInputCallbacksRegistered)
    {
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
            SAL_INFO("xmlsecurity.xmlsec",
                     "xmlStreamOpen: uri is '" << uri << "', returning context " << pInputStream);
            return static_cast<void*>(pInputStream) ;
        }
    }

    return nullptr ;
}

static int xmlStreamRead( void* context, char* buffer, int len )
{
    int numbers ;
    css::uno::Reference< css::io::XInputStream > xInputStream ;
    css::uno::Sequence< sal_Int8 > outSeqs( len ) ;

    numbers = 0 ;
    if (g_bInputCallbacksEnabled && g_bInputCallbacksRegistered)
    {
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

    SAL_INFO("xmlsecurity.xmlsec", "xmlStreamRead: context is " << context << ", buffer is now '"
                                                         << OString(buffer, numbers) << "'");
    return numbers ;
}

static int xmlStreamClose( void * context )
{
    if (g_bInputCallbacksEnabled && g_bInputCallbacksRegistered)
    {
        if( context != nullptr ) {
            css::io::XInputStream* pInputStream ;
            pInputStream = static_cast<css::io::XInputStream*>(context);
            pInputStream->release() ;
            SAL_INFO("xmlsecurity.xmlsec", "xmlStreamRead: closed context " << context);
        }
    }

    return 0 ;
}

}

int xmlEnableStreamInputCallbacks()
{
    if (!g_bInputCallbacksEnabled)
    {
        //Register the callbacks into xmlSec
        //In order to make the xmlsec io finding the callbacks firstly,
        //I put the callbacks at the very beginning.

        //Cleanup the older callbacks.
        //Notes: all none default callbacks will lose.
        xmlSecIOCleanupCallbacks() ;

        // Make sure that errors are reported via SAL_WARN().
        setErrorRecorder();
        comphelper::ScopeGuard g([] { clearErrorRecorder(); });

        // Newer xmlsec wants the callback order in the opposite direction.
        if (xmlSecCheckVersionExt(1, 2, 26, xmlSecCheckVersionABICompatible))
        {
            //Register the default callbacks.
            //Notes: the error will cause xmlsec working problems.
            int cbs = xmlSecIORegisterDefaultCallbacks() ;
            if( cbs < 0 ) {
                return -1 ;
            }

            //Register my classbacks.
            cbs = xmlSecIORegisterCallbacks(
                        xmlStreamMatch,
                        xmlStreamOpen,
                        xmlStreamRead,
                        xmlStreamClose ) ;
            if( cbs < 0 ) {
                return -1 ;
            }
        }
        else
        {
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
        }

        g_bInputCallbacksEnabled = true;
    }

    return 0 ;
}

int xmlRegisterStreamInputCallbacks(
    css::uno::Reference< css::xml::crypto::XUriBinding > const & aUriBinding
) {
    if (!g_bInputCallbacksEnabled)
    {
        if( xmlEnableStreamInputCallbacks() < 0 )
            return -1 ;
    }

    if (!g_bInputCallbacksRegistered)
        g_bInputCallbacksRegistered = true;

    m_xUriBinding = aUriBinding ;

    return 0 ;
}

int xmlUnregisterStreamInputCallbacks()
{
    if (g_bInputCallbacksRegistered)
    {
        //Clear the uri-stream binding
        m_xUriBinding.clear() ;

        //disable the registered flag
        g_bInputCallbacksRegistered = false;
    }

    return 0 ;
}

void xmlDisableStreamInputCallbacks() {
    xmlUnregisterStreamInputCallbacks() ;
    g_bInputCallbacksEnabled = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
