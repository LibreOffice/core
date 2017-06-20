/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlsec/xmlsec_init.hxx"

#include <com/sun/star/uno/RuntimeException.hpp>

#include "xmlsec/xmlstreamio.hxx"
#include "xmlsec-wrapper.h"

using namespace css::uno;

XSECXMLSEC_DLLPUBLIC void initXmlSec()
{
    //Init xmlsec library
    if( xmlSecInit() < 0 ) {
        throw RuntimeException() ;
    }

    //Init xmlsec crypto engine library
    if( xmlSecCryptoInit() < 0 ) {
        xmlSecShutdown() ;
        throw RuntimeException() ;
    }

    //Enable external stream handlers
    if( xmlEnableStreamInputCallbacks() < 0 ) {
        xmlSecCryptoShutdown() ;
        xmlSecShutdown() ;
        throw RuntimeException() ;
    }
}

XSECXMLSEC_DLLPUBLIC void deInitXmlSec()
{
    xmlDisableStreamInputCallbacks();
    xmlSecCryptoShutdown();
    xmlSecShutdown();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
