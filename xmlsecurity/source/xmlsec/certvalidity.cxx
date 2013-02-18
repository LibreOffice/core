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


#include <xmlsecurity/certvalidity.hxx>
#include <com/sun/star/security/CertificateValidity.hpp>

using ::rtl::OUString ;
using namespace ::com::sun::star::security ;

#define VALID_STR "valid certificate"
#define INVALID_STR "invalid certificate"
#define UNTRUSTED_STR "untrusted certificate"
#define TIME_INVALID_STR "expired certificate"
#define NOT_NESTED_TIME_STR "invalid time nesting"
#define REVOKED_STR "revoked certificate"
#define UNKNOWN_REVOKATION_STR "unknown certificate revocation status"
#define SIGNATURE_INVALID_STR "invalid certificate signature"
#define EXTENSION_INVALID_STR "invalid certificate extension"
#define EXTENSION_UNKNOWN_STR "unknown critical certificate extension"
#define ISSUER_UNKNOWN_STR "unknown certificate issuer"
#define ISSUER_UNTRUSTED_STR "untrusted certificate issuer"
#define ISSUER_INVALID_STR "invalid certificate issuer"
#define ROOT_UNKNOWN_STR "unknown root certificate"
#define ROOT_UNTRUSTED_STR "untrusted root certificate"
#define ROOT_INVALID_STR "invalid root certificate"
#define CHAIN_INCOMPLETE_STR "invalid certification path"

rtl::OUString certificateValidityToOUString( ::sal_Int32 certValidity ) {
    OUString aValidity ;

    if( certValidity == CertificateValidity::VALID ) {
        aValidity = OUString(VALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::UNTRUSTED ) ==  CertificateValidity::UNTRUSTED ) {
         aValidity = OUString(UNTRUSTED_STR ) ;
    } else if( ( certValidity & CertificateValidity::TIME_INVALID ) == CertificateValidity::TIME_INVALID ) {
         aValidity = OUString(TIME_INVALID_STR ) ;
    } else if( ( certValidity & CertificateValidity::NOT_TIME_NESTED ) == CertificateValidity::NOT_TIME_NESTED ) {
         aValidity = OUString(NOT_NESTED_TIME_STR ) ;
    } else if( ( certValidity & CertificateValidity::REVOKED ) == CertificateValidity::REVOKED ) {
         aValidity = OUString(REVOKED_STR ) ;
    } else if( ( certValidity &  CertificateValidity::UNKNOWN_REVOKATION ) == CertificateValidity::UNKNOWN_REVOKATION ) {
         aValidity = OUString(UNKNOWN_REVOKATION_STR ) ;
    } else if( ( certValidity &  CertificateValidity::SIGNATURE_INVALID ) == CertificateValidity::SIGNATURE_INVALID ) {
         aValidity = OUString(SIGNATURE_INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::EXTENSION_INVALID ) == CertificateValidity::EXTENSION_INVALID ) {
         aValidity = OUString(EXTENSION_INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::EXTENSION_UNKNOWN ) == CertificateValidity::EXTENSION_UNKNOWN ) {
         aValidity = OUString(EXTENSION_UNKNOWN_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ISSUER_UNKNOWN ) == CertificateValidity::ISSUER_UNKNOWN ) {
         aValidity = OUString(ISSUER_UNKNOWN_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ISSUER_UNTRUSTED ) == CertificateValidity::ISSUER_UNTRUSTED ) {
         aValidity = OUString(ISSUER_UNTRUSTED_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ISSUER_INVALID ) == CertificateValidity::ISSUER_INVALID ) {
         aValidity = OUString(ISSUER_INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ROOT_UNKNOWN ) == CertificateValidity::ROOT_UNKNOWN ) {
         aValidity = OUString(ROOT_UNKNOWN_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ROOT_UNTRUSTED ) == CertificateValidity::ROOT_UNTRUSTED ) {
         aValidity = OUString(ROOT_UNTRUSTED_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ROOT_INVALID ) == CertificateValidity::ROOT_INVALID ) {
         aValidity = OUString(ROOT_INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::CHAIN_INCOMPLETE ) == CertificateValidity::CHAIN_INCOMPLETE ) {
         aValidity = OUString(CHAIN_INCOMPLETE_STR ) ;
    } else {
         aValidity = OUString(INVALID_STR ) ;
    }

    return aValidity ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
