/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: certvalidity.cxx,v $
 * $Revision: 1.7 $
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

    if( (certValidity & CertificateValidity::VALID) == CertificateValidity::VALID ) {
        aValidity = OUString::createFromAscii( ( const char* )VALID_STR ) ;
    } else if( ( certValidity & CertificateValidity::INVALID ) == CertificateValidity::INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::UNTRUSTED ) ==  CertificateValidity::UNTRUSTED ) {
         aValidity = OUString::createFromAscii( ( const char* )UNTRUSTED_STR ) ;
    } else if( ( certValidity & CertificateValidity::TIME_INVALID ) == CertificateValidity::TIME_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )TIME_INVALID_STR ) ;
    } else if( ( certValidity & CertificateValidity::NOT_TIME_NESTED ) == CertificateValidity::NOT_TIME_NESTED ) {
         aValidity = OUString::createFromAscii( ( const char* )NOT_NESTED_TIME_STR ) ;
    } else if( ( certValidity & CertificateValidity::REVOKED ) == CertificateValidity::REVOKED ) {
         aValidity = OUString::createFromAscii( ( const char* )REVOKED_STR ) ;
    } else if( ( certValidity &  CertificateValidity::UNKNOWN_REVOKATION ) == CertificateValidity::UNKNOWN_REVOKATION ) {
         aValidity = OUString::createFromAscii( ( const char* )UNKNOWN_REVOKATION_STR ) ;
    } else if( ( certValidity &  CertificateValidity::SIGNATURE_INVALID ) == CertificateValidity::SIGNATURE_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )SIGNATURE_INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::EXTENSION_INVALID ) == CertificateValidity::EXTENSION_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )EXTENSION_INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::EXTENSION_UNKNOWN ) == CertificateValidity::EXTENSION_UNKNOWN ) {
         aValidity = OUString::createFromAscii( ( const char* )EXTENSION_UNKNOWN_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ISSUER_UNKNOWN ) == CertificateValidity::ISSUER_UNKNOWN ) {
         aValidity = OUString::createFromAscii( ( const char* )ISSUER_UNKNOWN_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ISSUER_UNTRUSTED ) == CertificateValidity::ISSUER_UNTRUSTED ) {
         aValidity = OUString::createFromAscii( ( const char* )ISSUER_UNTRUSTED_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ISSUER_INVALID ) == CertificateValidity::ISSUER_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )ISSUER_INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ROOT_UNKNOWN ) == CertificateValidity::ROOT_UNKNOWN ) {
         aValidity = OUString::createFromAscii( ( const char* )ROOT_UNKNOWN_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ROOT_UNTRUSTED ) == CertificateValidity::ROOT_UNTRUSTED ) {
         aValidity = OUString::createFromAscii( ( const char* )ROOT_UNTRUSTED_STR ) ;
    } else if( ( certValidity &  CertificateValidity::ROOT_INVALID ) == CertificateValidity::ROOT_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )ROOT_INVALID_STR ) ;
    } else if( ( certValidity &  CertificateValidity::CHAIN_INCOMPLETE ) == CertificateValidity::CHAIN_INCOMPLETE ) {
         aValidity = OUString::createFromAscii( ( const char* )CHAIN_INCOMPLETE_STR ) ;
    } else {
         aValidity = OUString::createFromAscii( ( const char* )INVALID_STR ) ;
    }

    return aValidity ;
}

