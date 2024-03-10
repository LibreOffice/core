/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <xmlsec/xmlsec_init.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <xmlsec/xmlstreamio.hxx>
#ifdef XMLSEC_CRYPTO_MSCRYPTO
#include <xmlsec/mscng/crypto.h>
#endif
#ifdef XMLSEC_CRYPTO_NSS
#include <xmlsec/nss/crypto.h>
#endif

using namespace css::uno;

XSECXMLSEC_DLLPUBLIC void initXmlSec()
{
    //Init xmlsec library
    if( xmlSecInit() < 0 ) {
        throw RuntimeException("Failed to initialize XML Security (xmlsec) library") ;
    }

    //Init xmlsec crypto engine library
#ifdef XMLSEC_CRYPTO_MSCRYPTO
    if( xmlSecMSCngInit() < 0 ) {
        xmlSecShutdown();
        throw RuntimeException("Failed to initialize XML Security (xmlsec) for Microsoft crypto engine library");
    }
#endif
#ifdef XMLSEC_CRYPTO_NSS
    if( xmlSecNssInit() < 0 ) {
        xmlSecShutdown();
        throw RuntimeException("Failed to Initialize XML Security (xmlsec) NSS crypto engine library");
    }
#endif

    //Enable external stream handlers
    if( xmlEnableStreamInputCallbacks() < 0 ) {
#ifdef XMLSEC_CRYPTO_MSCRYPTO
        xmlSecMSCngShutdown();
#endif
#ifdef XMLSEC_CRYPTO_NSS
        xmlSecNssShutdown();
#endif
        xmlSecShutdown() ;
        throw RuntimeException("Failed to Initialize XML Security (xmlsec) NSS crypto engine library") ;
    }
}

XSECXMLSEC_DLLPUBLIC void deInitXmlSec()
{
    xmlDisableStreamInputCallbacks();
#ifdef XMLSEC_CRYPTO_MSCRYPTO
    xmlSecMSCngShutdown();
#endif
#ifdef XMLSEC_CRYPTO_NSS
    xmlSecNssShutdown();
#endif
    xmlSecShutdown();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
