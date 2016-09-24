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

#include <rtl/ref.hxx>
#include <rtl/byteseq.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <unotest/macros_test.hxx>
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>

using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using css::xml::dom::XDocumentBuilder;
using css::xml::dom::DocumentBuilder;

namespace
{
// valid xml
static const char validTestFile[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
 <office:document-content \
   xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" \
   xmlns:xlink=\"http://www.w3.org/1999/xlink\" \
   office:version=\"1.0\"> \
   <office:scripts/> \
   <xlink:test/> \
   <office:automatic-styles teststyle=\"test\"/> \
   <moretest/> \
    some text \303\266\303\244\303\274 \
 </office:document-content> \
";

// generates a warning: unsupported xml version, unknown xml:space
// value
static const char warningTestFile[] =
"<?xml version=\"47-11.0\" encoding=\"UTF-8\"?> \
 <office:document-content \
   xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" \
   xml:space=\"blafasl\" \
   office:version=\"1.0\"> \
   <office:scripts/> \
   <office:automatic-styles/> \
 </office:document-content> \
";

// <?xml not at start of file
static const char errorTestFile[] =
" <?xml version=\"1.0\" encoding=\"UTF-8\"?> \
 <office:document-content \
   xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" \
   office:version=\"1.0\"> \
   <office:scripts/> \
   <office:automatic-styles/> \
 </office:document-content> \
";

// plain empty
static const char fatalTestFile[] = "";

struct ErrorHandler
    : public ::cppu::WeakImplHelper< xml::sax::XErrorHandler >
{
    sal_uInt32 mnErrCount;
    sal_uInt32 mnFatalCount;
    sal_uInt32 mnWarnCount;

    bool noErrors() const { return !mnErrCount && !mnFatalCount && !mnWarnCount; }

    ErrorHandler() : mnErrCount(0), mnFatalCount(0), mnWarnCount(0)
    {}

    virtual void SAL_CALL error( const uno::Any& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
        ++mnErrCount;
    }

    virtual void SAL_CALL fatalError( const uno::Any& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
        ++mnFatalCount;
    }

    virtual void SAL_CALL warning( const uno::Any& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
        ++mnWarnCount;
    }
};

struct DocumentHandler
    : public ::cppu::WeakImplHelper< xml::sax::XFastDocumentHandler >
{
    // XFastContextHandler
    virtual void SAL_CALL startFastElement( ::sal_Int32 Element, const uno::Reference< xml::sax::XFastAttributeList >& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
        SAL_INFO(
            "unoxml",
            "Seen element: " << (Element & 0xFFFF) << " with namespace "
                << (Element & 0xFFFF0000));
    }

    virtual void SAL_CALL startUnknownElement( const OUString& , const OUString& , const uno::Reference< xml::sax::XFastAttributeList >& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
    }

    virtual void SAL_CALL endFastElement( ::sal_Int32 ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
    }

    virtual void SAL_CALL endUnknownElement( const OUString&, const OUString& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
    }

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 , const uno::Reference< xml::sax::XFastAttributeList >& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
        return this;
    }

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& , const OUString& , const uno::Reference< xml::sax::XFastAttributeList >& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
        return this;
    }

    virtual void SAL_CALL characters( const OUString& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
    }

    // XFastDocumentHandler
    virtual void SAL_CALL startDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
    }

    virtual void SAL_CALL endDocument(  ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
    }

    virtual void SAL_CALL setDocumentLocator( const uno::Reference< xml::sax::XLocator >& ) throw (xml::sax::SAXException, uno::RuntimeException) override
    {
    }
};

struct TokenHandler
    : public ::cppu::WeakImplHelper< xml::sax::XFastTokenHandler >
{
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8( const uno::Sequence< ::sal_Int8 >& Identifier ) throw (uno::RuntimeException) override
    {
        OSL_TRACE("getTokenFromUTF8() %s", reinterpret_cast<const char*>(Identifier.getConstArray()));
        return Identifier.getLength() ? Identifier[0] : 0;
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getUTF8Identifier( ::sal_Int32 ) throw (uno::RuntimeException, std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE( "TokenHandler::getUTF8Identifier() unexpected call",
                                false );
        return uno::Sequence<sal_Int8>();
    }
};

struct BasicTest : public test::BootstrapFixture
{
    uno::Reference<XDocumentBuilder>    mxDomBuilder;
    rtl::Reference<ErrorHandler>        mxErrHandler;
    rtl::Reference<SequenceInputStream> mxValidInStream;
    rtl::Reference<SequenceInputStream> mxWarningInStream;
    rtl::Reference<SequenceInputStream> mxErrorInStream;
    rtl::Reference<SequenceInputStream> mxFatalInStream;

    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();

        mxErrHandler.set( new ErrorHandler() );
        uno::Reference<XDocumentBuilder> xDB( getMultiServiceFactory()->createInstance("com.sun.star.xml.dom.DocumentBuilder"), uno::UNO_QUERY_THROW );
        mxDomBuilder.set( xDB );
        mxValidInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(validTestFile), SAL_N_ELEMENTS(validTestFile))) );
        mxWarningInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(warningTestFile), SAL_N_ELEMENTS(warningTestFile))) );
        mxErrorInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(errorTestFile), SAL_N_ELEMENTS(errorTestFile))) );
        mxFatalInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(fatalTestFile), SAL_N_ELEMENTS(fatalTestFile))) );
        mxDomBuilder->setErrorHandler(mxErrHandler.get());
    }

    void validInputTest()
    {
        CPPUNIT_ASSERT_MESSAGE( "Valid input file did not result in XDocument #1",
                                mxDomBuilder->parse(
                                    uno::Reference<io::XInputStream>(
                                        mxValidInStream.get())).is() );
        CPPUNIT_ASSERT_MESSAGE( "Valid input file resulted in parse errors",
                                mxErrHandler->noErrors() );
    }
/*
    void warningInputTest()
    {
        CPPUNIT_ASSERT_MESSAGE( "Valid input file did not result in XDocument #2",
                                mxDomBuilder->parse(
                                    uno::Reference<io::XInputStream>(
                                        mxWarningInStream.get())).is() );
        CPPUNIT_ASSERT_MESSAGE( "No parse warnings in unclean input file",
                                mxErrHandler->mnWarnCount && !mxErrHandler->mnErrCount && !mxErrHandler->mnFatalCount );
    }

    void errorInputTest()
    {
        CPPUNIT_ASSERT_MESSAGE( "Valid input file did not result in XDocument #3",
                                mxDomBuilder->parse(
                                    uno::Reference<io::XInputStream>(
                                        mxErrorInStream.get())).is() );
        CPPUNIT_ASSERT_MESSAGE( "No parse errors in unclean input file",
                                !mxErrHandler->mnWarnCount && mxErrHandler->mnErrCount && !mxErrHandler->mnFatalCount );
    }

    void fatalInputTest()
    {
        CPPUNIT_ASSERT_MESSAGE( "Broken input file resulted in XDocument",
                                !mxDomBuilder->parse(
                                    uno::Reference<io::XInputStream>(
                                        mxFatalInStream.get())).is() );
        CPPUNIT_ASSERT_MESSAGE( "No fatal parse errors in unclean input file",
                                !mxErrHandler->mnWarnCount && !mxErrHandler->mnErrCount && mxErrHandler->mnFatalCount );
    };
*/
    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.
    CPPUNIT_TEST_SUITE(BasicTest);
    CPPUNIT_TEST(validInputTest);
    //CPPUNIT_TEST(warningInputTest);
    //CPPUNIT_TEST(errorInputTest);
    //CPPUNIT_TEST(fatalInputTest);
    CPPUNIT_TEST_SUITE_END();
};

struct SerializerTest : public test::BootstrapFixture
{
    uno::Reference<XDocumentBuilder>                         mxDomBuilder;
    rtl::Reference<ErrorHandler>                             mxErrHandler;
    rtl::Reference<SequenceInputStream>                      mxInStream;
    rtl::Reference<DocumentHandler>                          mxHandler;
    rtl::Reference<TokenHandler>                             mxTokHandler;
    uno::Sequence< beans::Pair< OUString, sal_Int32 > > maRegisteredNamespaces;

    void setUp() override
    {
        test::BootstrapFixture::setUp();

        mxErrHandler.set( new ErrorHandler() );
        uno::Reference<XDocumentBuilder> xDB( getMultiServiceFactory()->createInstance("com.sun.star.xml.dom.DocumentBuilder"), uno::UNO_QUERY_THROW );
        mxDomBuilder.set( xDB );
        mxInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(validTestFile), SAL_N_ELEMENTS(validTestFile))) );
        mxDomBuilder->setErrorHandler(mxErrHandler.get());
        mxHandler.set( new DocumentHandler() );
        mxTokHandler.set( new TokenHandler() );

        maRegisteredNamespaces.realloc(2);
        maRegisteredNamespaces[0] = beans::make_Pair(
            OUString( "urn:oasis:names:tc:opendocument:xmlns:office:1.0" ),
            xml::sax::FastToken::NAMESPACE);
        maRegisteredNamespaces[1] = beans::make_Pair(
            OUString( "http://www.w3.org/1999/xlink" ),
            2*xml::sax::FastToken::NAMESPACE);
    }

    void serializerTest ()
    {
        uno::Reference< xml::dom::XDocument > xDoc=
            mxDomBuilder->parse(
                uno::Reference<io::XInputStream>(
                    mxInStream.get()));
        CPPUNIT_ASSERT_MESSAGE( "Valid input file did not result in XDocument",
                                xDoc.is() );
        CPPUNIT_ASSERT_MESSAGE( "Valid input file resulted in parse errors",
                                mxErrHandler->noErrors() );

        uno::Reference< xml::sax::XSAXSerializable > xSaxSerializer(
            xDoc, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE( "XSAXSerializable not supported",
                                xSaxSerializer.is() );

        uno::Reference< xml::sax::XFastSAXSerializable > xFastSaxSerializer(
            xDoc, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE( "XFastSAXSerializable not supported",
                                xSaxSerializer.is() );

        xFastSaxSerializer->fastSerialize( mxHandler.get(),
                                           mxTokHandler.get(),
                                           uno::Sequence< beans::StringPair >(),
                                           maRegisteredNamespaces );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(SerializerTest);
    CPPUNIT_TEST(serializerTest);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasicTest);
CPPUNIT_TEST_SUITE_REGISTRATION(SerializerTest);
}

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
