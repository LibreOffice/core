/*************************************************************************
 *
 *  $RCSfile: certvalidity.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mmi $ $Date: 2004-08-02 04:46:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <xmlsecurity/certvalidity.hxx>

#ifndef _COM_SUN_STAR_SECURITY_CERTIFICATEVALIDITY_HPP_
#include <com/sun/star/security/CertificateValidity.hpp>
#endif

using ::rtl::OUString ;
using ::com::sun::star::security::CertificateValidity ;

#define CERT_VALIDITY_VALID_STR "valid certificate"
#define CERT_VALIDITY_INVALID_STR "invalid certificate"
#define CERT_VALIDITY_UNTRUSTED_STR "untrusted certificate"
#define CERT_VALIDITY_TIMEOUT_STR "expired certificate"
#define CERT_VALIDITY_REVOKED_STR "revoked certificate"
#define CERT_VALIDITY_UNKNOWN_REVOKATION_STR "unknown certificate revocation status"
#define CERT_VALIDITY_SIGNATURE_INVALID_STR "invalid certificate signature"
#define CERT_VALIDITY_EXTENSION_INVALID_STR "invalid certificate extension"
#define CERT_VALIDITY_EXTENSION_UNKNOWN_STR "unknown critical certificate extension"
#define CERT_VALIDITY_ISSUER_UNKNOWN_STR "unknown certificate issuer"
#define CERT_VALIDITY_ISSUER_UNTRUSTED_STR "untrusted certificate issuer"
#define CERT_VALIDITY_ISSUER_INVALID_STR "invalid certificate issuer"
#define CERT_VALIDITY_ROOT_UNKNOWN_STR "unknown root certificate"
#define CERT_VALIDITY_ROOT_UNTRUSTED_STR "untrusted root certificate"
#define CERT_VALIDITY_ROOT_INVALID_STR "invalid root certificate"
#define CERT_VALIDITY_CHAIN_INCOMPLETE_STR "invalid certification path"

rtl::OUString certificateValidityToOUString( ::sal_Int32 certValidity ) {
    OUString aValidity ;

    if( !certValidity ) {
        aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_VALID_STR ) ;
    } else if( certValidity & CertificateValidity::CERT_VALIDITY_INVALID == CertificateValidity::CERT_VALIDITY_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_INVALID_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_UNTRUSTED ==  CertificateValidity::CERT_VALIDITY_UNTRUSTED ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_UNTRUSTED_STR ) ;
    } else if( certValidity & CertificateValidity::CERT_VALIDITY_TIMEOUT == CertificateValidity::CERT_VALIDITY_TIMEOUT ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_TIMEOUT_STR ) ;
    } else if( certValidity & CertificateValidity::CERT_VALIDITY_REVOKED == CertificateValidity::CERT_VALIDITY_REVOKED ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_REVOKED_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_UNKNOWN_REVOKATION == CertificateValidity::CERT_VALIDITY_UNKNOWN_REVOKATION ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_UNKNOWN_REVOKATION_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_SIGNATURE_INVALID == CertificateValidity::CERT_VALIDITY_SIGNATURE_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_SIGNATURE_INVALID_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_EXTENSION_INVALID == CertificateValidity::CERT_VALIDITY_EXTENSION_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_EXTENSION_INVALID_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_EXTENSION_UNKNOWN == CertificateValidity::CERT_VALIDITY_EXTENSION_UNKNOWN ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_EXTENSION_UNKNOWN_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_ISSUER_UNKNOWN == CertificateValidity::CERT_VALIDITY_ISSUER_UNKNOWN ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_ISSUER_UNKNOWN_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_ISSUER_UNTRUSTED == CertificateValidity::CERT_VALIDITY_ISSUER_UNTRUSTED ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_ISSUER_UNTRUSTED_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_ISSUER_INVALID == CertificateValidity::CERT_VALIDITY_ISSUER_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_ISSUER_INVALID_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_ROOT_UNKNOWN == CertificateValidity::CERT_VALIDITY_ROOT_UNKNOWN ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_ROOT_UNKNOWN_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_ROOT_UNTRUSTED == CertificateValidity::CERT_VALIDITY_ROOT_UNTRUSTED ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_ROOT_UNTRUSTED_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_ROOT_INVALID == CertificateValidity::CERT_VALIDITY_ROOT_INVALID ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_ROOT_INVALID_STR ) ;
    } else if( certValidity &  CertificateValidity::CERT_VALIDITY_CHAIN_INCOMPLETE == CertificateValidity::CERT_VALIDITY_CHAIN_INCOMPLETE ) {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_CHAIN_INCOMPLETE_STR ) ;
    } else {
         aValidity = OUString::createFromAscii( ( const char* )CERT_VALIDITY_INVALID_STR ) ;
    }

    return aValidity ;
}

