/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xmlsec/xmlsec_init.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>

#include <xmlsec/xmlstreamio.hxx>
#include <xmlsec-wrapper.h>
#include <svl/cryptosign.hxx>
#ifdef XMLSEC_CRYPTO_MSCRYPTO
#include <xmlsec/mscng/crypto.h>
#else
#include <xmlsec/nss/crypto.h>
#endif

using namespace css::uno;

XSECXMLSEC_DLLPUBLIC void initXmlSec()
{
    //Init xmlsec library
    if( xmlSecInit() < 0 ) {
        throw RuntimeException() ;
    }

    //Init xmlsec crypto engine library
#ifdef XMLSEC_CRYPTO_MSCRYPTO
    if( xmlSecMSCngInit() < 0 ) {
        xmlSecShutdown();
        throw RuntimeException();
    }
#else
    if( xmlSecNssInit() < 0 ) {
        xmlSecShutdown();
        throw RuntimeException();
    }
#endif

    //Enable external stream handlers
    if( xmlEnableStreamInputCallbacks() < 0 ) {
#ifdef XMLSEC_CRYPTO_MSCRYPTO
        xmlSecMSCngShutdown();
#else
        xmlSecNssShutdown();
#endif
        xmlSecShutdown() ;
        throw RuntimeException() ;
    }
}

XSECXMLSEC_DLLPUBLIC void deInitXmlSec()
{
    xmlDisableStreamInputCallbacks();
#ifdef XMLSEC_CRYPTO_MSCRYPTO
    xmlSecMSCngShutdown();
#else
    xmlSecNssShutdown();
#endif
    xmlSecShutdown();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
