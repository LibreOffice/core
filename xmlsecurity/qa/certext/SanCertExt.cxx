/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_xmlsecurity.hxx"
#include "sal/config.h"

#include "../../source/xmlsec/mscrypt/securityenvironment_mscryptimpl.hxx"
#include <com/sun/star/security/XSanExtension.hpp>
#include <com/sun/star/security/ExtAltNameType.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include "cppuhelper/bootstrap.hxx"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "sal/types.h"
#include "comphelper/sequence.hxx"
#include <rtl/ustring.hxx>

#include <neon/ne_ssl.h>

using namespace com::sun::star;
using ::com::sun::star::lang::XMultiServiceFactory;

#define OID_SUBJECT_ALTERNATIVE_NAME "2.5.29.17"

namespace {

class Test: public CppUnit::TestFixture {

private:

    static uno::Sequence< security::CertAltNameEntry > altNames;

    void init(){
        if (altNames.getLength() == 0){
            cppu::defaultBootstrap_InitialComponentContext();
            ne_ssl_certificate* cert = ne_ssl_cert_read("User_35_Root_11.crt");
            char* certExportB64 = ne_ssl_cert_export(cert);

            uno::Reference< xml::crypto::XSecurityEnvironment > xSecurityEnv( new SecurityEnvironment_MSCryptImpl( uno::Reference< XMultiServiceFactory >() ) );

            uno::Reference< security::XCertificate > xCert = xSecurityEnv->createCertificateFromAscii(
                rtl::OStringToOUString( certExportB64, RTL_TEXTENCODING_ASCII_US ) );

            uno::Sequence< uno::Reference< security::XCertificateExtension > > extensions = xCert->getExtensions();
            for (sal_Int32 i = 0 ; i < extensions.getLength(); i++)
            {
                uno::Reference< security::XCertificateExtension >element = extensions[i];

                rtl::OString aId ( (const sal_Char *)element->getExtensionId().getArray(), element->getExtensionId().getLength());
                if (aId.equals(OID_SUBJECT_ALTERNATIVE_NAME))
                {
                   uno::Reference< security::XSanExtension > sanExtension ( element, uno::UNO_QUERY );
                   altNames = sanExtension->getAlternativeNames();
                   break;
                }
            }
        }

    }

public:
    void test_Others() {
        init();
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_OTHER_NAME)
            {
                ::com::sun::star::beans::NamedValue otherNameProp;
                if (altNames[n].Value >>= otherNameProp)
                {
                    //Name
                    CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("1.2.3.4"), otherNameProp.Name);

                    //Value
                    uno::Sequence< sal_Int8 > ipAddress;
                    otherNameProp.Value >>= ipAddress;
                    CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( ipAddress.getLength() > 0 ) );
                }
            }
        }
    }

    void test_RFC822() {
        init();
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_RFC822_NAME)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                //Value
                CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("my@other.address"), value);
            }
        }
    }

    void test_DNS() {
        init();
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_DNS_NAME)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                //Value
                CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("alt.openoffice.org"), value);
            }
        }
    }

    void test_Direcory() {
        init();
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_DIRECTORY_NAME)
            {
                uno::Sequence< sal_Int8 > value;
                altNames[n].Value >>= value;
                //Value
                CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( value.getLength() > 0 ) );
            }
        }
    }

    void test_URI() {
        init();
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_URL)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                //Value
                CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("http://my.url.here/"), value);
            }
        }
    }

    void test_IP() {
        init();
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_IP_ADDRESS)
            {
                uno::Sequence< sal_Int8 > ipAddress;
                altNames[n].Value >>= ipAddress;
                //Value
                CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( ipAddress.getLength() > 0 ) );
            }
        }

    }

    void test_RID() {
        init();
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_REGISTERED_ID)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                //Value
                CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("1.2.3.4"), value);
            }
        }

    }

    void test_EDI() {
        // Not implemented
    }

    void test_X400() {
        // Not implemented
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_Others);
    CPPUNIT_TEST(test_RFC822);
    CPPUNIT_TEST(test_DNS);
    CPPUNIT_TEST(test_Direcory);
    CPPUNIT_TEST(test_URI);
    CPPUNIT_TEST(test_IP);
    CPPUNIT_TEST(test_RID);
    CPPUNIT_TEST(test_EDI);
    CPPUNIT_TEST(test_X400);
    CPPUNIT_TEST_SUITE_END();
};

uno::Sequence< security::CertAltNameEntry > Test::altNames;

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();
