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
#include "test/officeconnection.hxx"

#include <com/sun/star/security/XSanExtension.hpp>
#include <com/sun/star/security/ExtAltNameType.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/Reference.hxx"

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

#define OID_SUBJECT_ALTERNATIVE_NAME "2.5.29.17"
#define SEINITIALIZER_COMPONENT "com.sun.star.xml.crypto.SEInitializer"


namespace {

    class Test: public CppUnit::TestFixture {

    private:
        static uno::Sequence< security::CertAltNameEntry > altNames;
        static bool runOnce;

        uno::Reference< xml::crypto::XSecurityEnvironment > initUno();
        void init();
        rtl::OString getB64CertFromFile(const char filename[]);
        test::OfficeConnection connection_;

    public:

        Test();

        ~Test();

        virtual void setUp();

        virtual void tearDown();

        void test_Others();

        void test_RFC822();

        void test_DNS();

        void test_Direcory();

        void test_URI();

        void test_IP();

        void test_RID();

        void test_EDI();

        void test_X400();

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
    bool Test::runOnce = false;

    CPPUNIT_TEST_SUITE_REGISTRATION(Test);

    Test::Test()
    {
        if (runOnce)
            return;
        runOnce = true;
        connection_.setUp();
        init();
    }

    Test::~Test()
    {
        if (runOnce)
        {
            connection_.tearDown();
            runOnce = false;
        }
    }


    uno::Reference< xml::crypto::XSecurityEnvironment > Test::initUno()
    {
        uno::Reference< uno::XComponentContext > context(connection_.getComponentContext(), uno::UNO_QUERY_THROW);
        uno::Reference< lang::XMultiServiceFactory > factory(context->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< xml::crypto::XSEInitializer > xSEInitializer(factory->createInstance(
            rtl::OUString::createFromAscii( SEINITIALIZER_COMPONENT )), uno::UNO_QUERY_THROW);
        uno::Reference< xml::crypto::XXMLSecurityContext > xSecurityContext(
            xSEInitializer->createSecurityContext(rtl::OUString()));
        return xSecurityContext->getSecurityEnvironment();
    }


    void Test::init()
    {
        uno::Reference< xml::crypto::XSecurityEnvironment > xSecurityEnv = initUno();
        rtl::OString b64Cert(getB64CertFromFile("User_35_Root_11.crt"));
        uno::Reference< security::XCertificate > xCert = xSecurityEnv->createCertificateFromAscii(
            rtl::OStringToOUString( b64Cert, RTL_TEXTENCODING_ASCII_US ) );
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

    rtl::OString Test::getB64CertFromFile(const char filename[])
    {
        ne_ssl_certificate* cert = ne_ssl_cert_read(filename);
        char* certExportB64 = ne_ssl_cert_export(cert);
        rtl::OString certB64( certExportB64 );
        return certB64;
    }


    void Test::setUp() {
    }

    void Test::tearDown() {
    }

    void Test::test_Others() {
        CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_OTHER_NAME)
            {
                ::com::sun::star::beans::NamedValue otherNameProp;
                if (altNames[n].Value >>= otherNameProp)
                {
                    CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("1.2.3.4"), otherNameProp.Name);
                    uno::Sequence< sal_Int8 > ipAddress;
                    otherNameProp.Value >>= ipAddress;
                    CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( ipAddress.getLength() > 0 ) );
                }
            }
        }
    }

    void Test::test_RFC822() {
        CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_RFC822_NAME)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("my@other.address"), value);
            }
        }
    }

    void Test::test_DNS() {
        CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_DNS_NAME)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("alt.openoffice.org"), value);
            }
        }
    }

    void Test::test_Direcory() {
        // Not implemented
    }

    void Test::test_URI() {
        CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_URL)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                CPPUNIT_ASSERT_EQUAL( rtl::OUString::createFromAscii("http://my.url.here/"), value);
            }
        }
    }

    void Test::test_IP() {
        CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_IP_ADDRESS)
            {
                uno::Sequence< sal_Int8 > ipAddress;
                altNames[n].Value >>= ipAddress;
                CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( ipAddress.getLength() > 0 ) );
            }
        }

    }

    void Test::test_RID() {
        CPPUNIT_ASSERT_ASSERTION_PASS( CPPUNIT_ASSERT( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_REGISTERED_ID)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                CPPUNIT_ASSERT( rtl::OUString::createFromAscii("1.2.3.4").equals(value));
            }
        }
    }

    void Test::test_EDI() {
        // Not implemented
    }

    void Test::test_X400() {
        // Not implemented
    }
}
CPPUNIT_PLUGIN_IMPLEMENT();
