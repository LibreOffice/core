/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include "gtest/gtest.h"
#include "sal/types.h"
#include "comphelper/sequence.hxx"
#include <rtl/ustring.hxx>

#include <neon/ne_ssl.h>

using namespace com::sun::star;

#define OID_SUBJECT_ALTERNATIVE_NAME "2.5.29.17"
#define SEINITIALIZER_COMPONENT "com.sun.star.xml.crypto.SEInitializer"


namespace {

    class Test: public ::testing::Test {

    protected:
        static uno::Sequence< security::CertAltNameEntry > altNames;
        static bool runOnce;

        uno::Reference< xml::crypto::XSecurityEnvironment > initUno();
        void init();
        rtl::OString getB64CertFromFile(const char filename[]);
        test::OfficeConnection connection_;

    public:

        Test();

        ~Test();

        virtual void SetUp();

        virtual void TearDown();
    };

    uno::Sequence< security::CertAltNameEntry > Test::altNames;
    bool Test::runOnce = false;


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


    void Test::SetUp() {
    }

    void Test::TearDown() {
    }

    TEST_F(Test, test_Others) {
        ASSERT_NO_THROW( ASSERT_TRUE( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_OTHER_NAME)
            {
                ::com::sun::star::beans::NamedValue otherNameProp;
                if (altNames[n].Value >>= otherNameProp)
                {
                    ASSERT_EQ( rtl::OUString::createFromAscii("1.2.3.4"), otherNameProp.Name);
                    uno::Sequence< sal_Int8 > ipAddress;
                    otherNameProp.Value >>= ipAddress;
                    ASSERT_NO_THROW( ASSERT_TRUE( ipAddress.getLength() > 0 ) );
                }
            }
        }
    }

    TEST_F(Test, test_RFC822) {
        ASSERT_NO_THROW( ASSERT_TRUE( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_RFC822_NAME)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                ASSERT_EQ( rtl::OUString::createFromAscii("my@other.address"), value );
            }
        }
    }

    TEST_F(Test, test_DNS) {
        ASSERT_NO_THROW( ASSERT_TRUE( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_DNS_NAME)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                ASSERT_EQ( rtl::OUString::createFromAscii("alt.openoffice.org"), value);
            }
        }
    }

    TEST_F(Test, test_Direcory) {
        // Not implemented
    }

    TEST_F(Test, test_URI) {
        ASSERT_NO_THROW( ASSERT_TRUE( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_URL)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                ASSERT_EQ( rtl::OUString::createFromAscii("http://my.url.here/"), value);
            }
        }
    }

    TEST_F(Test, test_IP) {
        ASSERT_NO_THROW( ASSERT_TRUE( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_IP_ADDRESS)
            {
                uno::Sequence< sal_Int8 > ipAddress;
                altNames[n].Value >>= ipAddress;
                ASSERT_NO_THROW( ASSERT_TRUE( ipAddress.getLength() > 0 ) );
            }
        }

    }

    TEST_F(Test, test_RID) {
        ASSERT_NO_THROW( ASSERT_TRUE( altNames.getLength() > 0 ) );
        for(int n = 1; n < altNames.getLength(); n++)
        {
            if (altNames[n].Type ==  security::ExtAltNameType_REGISTERED_ID)
            {
                rtl::OUString value;
                altNames[n].Value >>= value;
                ASSERT_TRUE( rtl::OUString::createFromAscii("1.2.3.4").equals(value));
            }
        }
    }

    TEST_F(Test, test_EDI) {
        // Not implemented
    }

    TEST_F(Test, test_X400) {
        // Not implemented
    }
}
