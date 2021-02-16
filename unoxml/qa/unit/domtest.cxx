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
#include <sal/log.hxx>
#include <sax/fastattribs.hxx>
#include <comphelper/seqstream.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using css::xml::dom::XDocumentBuilder;

namespace
{
// valid xml
const char validTestFile[] =
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

// generates a warning: unknown xml:space
// value
const char warningTestFile[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
 <office:document-content \
   xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" \
   xml:space=\"blah\" \
   xmlns:xlink=\"http://www.w3.org/1999/xlink\" \
   office:version=\"1.0\"> \
   <office:scripts/> \
   <xlink:test/> \
   <office:automatic-styles teststyle=\"test\"/> \
   <moretest/> \
    some text \303\266\303\244\303\274 \
 </office:document-content> \
";

// <?xml not at start of file
const char errorTestFile[] =
" <?xml version=\"1.0\" encoding=\"UTF-8\"?> \
 <office:document-content \
   xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" \
   office:version=\"1.0\"> \
   <office:scripts/> \
   <office:automatic-styles/> \
 </office:document-content> \
";

struct ErrorHandler
    : public ::cppu::WeakImplHelper< xml::sax::XErrorHandler >
{
    sal_uInt32 mnErrCount;
//    sal_uInt32 mnFatalCount;  // No fatal error counter, as lib2xml doesn't distinguish between error and fatal error
                                // (see See xmlFatalErrMsg from lib2xml/parse.c and __xmlRaiseError from lib2xml/error.c)
    sal_uInt32 mnWarnCount;

    bool noErrors() const { return !mnErrCount /*&& !mnFatalCount*/ && !mnWarnCount; }

    ErrorHandler() : mnErrCount(0), /*mnFatalCount(0),*/ mnWarnCount(0)
    {}

    virtual void SAL_CALL error( const uno::Any& ) override
    {
        ++mnErrCount;
    }

    // Just implement FatalError function as it is in XErrorHandler
    // This function is never used, as lib2xml doesn't distinguish between error and fatalerror and calls error functions in both cases
    virtual void SAL_CALL fatalError( const uno::Any& ) override
    {
        //++mnFatalCount;
    }

    virtual void SAL_CALL warning( const uno::Any& ) override
    {
        ++mnWarnCount;
    }
};

struct DocumentHandler
    : public ::cppu::WeakImplHelper< xml::sax::XFastDocumentHandler >
{
    // XFastContextHandler
    virtual void SAL_CALL startFastElement( ::sal_Int32 Element, const uno::Reference< xml::sax::XFastAttributeList >& ) override
    {
        SAL_INFO(
            "unoxml",
            "Seen element: " << (Element & 0xFFFF) << " with namespace "
                << (Element & 0xFFFF0000));
    }

    virtual void SAL_CALL startUnknownElement( const OUString& , const OUString& , const uno::Reference< xml::sax::XFastAttributeList >& ) override
    {
    }

    virtual void SAL_CALL endFastElement( ::sal_Int32 ) override
    {
    }

    virtual void SAL_CALL endUnknownElement( const OUString&, const OUString& ) override
    {
    }

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 , const uno::Reference< xml::sax::XFastAttributeList >& ) override
    {
        return this;
    }

    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& , const OUString& , const uno::Reference< xml::sax::XFastAttributeList >& ) override
    {
        return this;
    }

    virtual void SAL_CALL characters( const OUString& ) override
    {
    }

    // XFastDocumentHandler
    virtual void SAL_CALL startDocument(  ) override
    {
    }

    virtual void SAL_CALL endDocument(  ) override
    {
    }

    virtual void SAL_CALL processingInstruction( const OUString& /*rTarget*/, const OUString& /*rData*/ ) override
    {
    }

    virtual void SAL_CALL setDocumentLocator( const uno::Reference< xml::sax::XLocator >& ) override
    {
    }
};

struct TokenHandler : public sax_fastparser::FastTokenHandlerBase
{
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8( const uno::Sequence< ::sal_Int8 >& Identifier ) override
    {
        return Identifier.hasElements() ? Identifier[0] : 0;
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getUTF8Identifier( ::sal_Int32 ) override
    {
        CPPUNIT_ASSERT_MESSAGE( "TokenHandler::getUTF8Identifier() unexpected call",
                                false );
        return uno::Sequence<sal_Int8>();
    }

    virtual sal_Int32 getTokenDirect( const char * /* pToken */, sal_Int32 /* nLength */ ) const override
    {
        return -1;
    }
};

struct BasicTest : public test::BootstrapFixture
{
    uno::Reference<XDocumentBuilder>    mxDomBuilder;
    rtl::Reference<ErrorHandler>        mxErrHandler;
    rtl::Reference<SequenceInputStream> mxValidInStream;
    rtl::Reference<SequenceInputStream> mxWarningInStream;
    rtl::Reference<SequenceInputStream> mxErrorInStream;

    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();

        mxErrHandler.set( new ErrorHandler() );
        uno::Reference<XDocumentBuilder> xDB( getMultiServiceFactory()->createInstance("com.sun.star.xml.dom.DocumentBuilder"), uno::UNO_QUERY_THROW );
        mxDomBuilder.set( xDB );
        mxValidInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(validTestFile), SAL_N_ELEMENTS(validTestFile))) );
        mxWarningInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(warningTestFile), SAL_N_ELEMENTS(warningTestFile))) );
        mxErrorInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(errorTestFile), SAL_N_ELEMENTS(errorTestFile))) );
        mxDomBuilder->setErrorHandler(mxErrHandler);
    }

    void validInputTest()
    {
        try
        {
            CPPUNIT_ASSERT_MESSAGE("Valid input file did not result in XDocument #1",
                mxDomBuilder->parse(mxValidInStream).is());
            CPPUNIT_ASSERT_MESSAGE("Valid input file resulted in parse errors",
                mxErrHandler->noErrors());
        }
        catch (const css::xml::sax::SAXParseException&)
        {
            CPPUNIT_ASSERT_MESSAGE("Valid input file did not result in XDocument (exception thrown)", false);
        }
    }

    void warningInputTest()
    {
        try
        {
            // We DON'T expect exception here, as mxWarningInStream is valid XML Doc
            CPPUNIT_ASSERT_MESSAGE("Valid input file did not result in XDocument #2",
                mxDomBuilder->parse(mxWarningInStream).is());
        }
        catch (const css::xml::sax::SAXParseException& )
        {
            CPPUNIT_ASSERT_MESSAGE("Valid input file did not result in XDocument #2 (exception thrown)", false);
        }
        CPPUNIT_ASSERT_MESSAGE("No parse warnings in unclean input file",
            mxErrHandler->mnWarnCount);
        CPPUNIT_ASSERT_MESSAGE("No parse warnings in unclean input file",
            !mxErrHandler->mnErrCount /*&& !mxErrHandler->mnFatalCount*/);
    }

    void errorInputTest()
    {
        try
        {
            // We expect exception here, as mxErrorInStream is invalid XML Doc
            CPPUNIT_ASSERT_MESSAGE("Invalid input file result in XDocument #2!",
                !mxDomBuilder->parse(mxErrorInStream).is());
            CPPUNIT_ASSERT_MESSAGE("No exception is thrown in unclean input file", false);
        }
        catch (const css::xml::sax::SAXParseException&)
        {
            // It's OK to catch an exception here as we parse incorrect XML file
        }
        CPPUNIT_ASSERT_MESSAGE("No parse errors in unclean input file",
            !mxErrHandler->mnWarnCount);
        CPPUNIT_ASSERT_MESSAGE("No parse errors in unclean input file",
            mxErrHandler->mnErrCount /*&& !mxErrHandler->mnFatalCount*/);
    }

        // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.
    CPPUNIT_TEST_SUITE(BasicTest);
    CPPUNIT_TEST(validInputTest);
    CPPUNIT_TEST(warningInputTest);
    CPPUNIT_TEST(errorInputTest);
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
        mxDomBuilder->setErrorHandler(mxErrHandler);
        mxHandler.set( new DocumentHandler );
        mxTokHandler.set( new TokenHandler );

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
        try
        {
            uno::Reference< xml::dom::XDocument > xDoc =
                mxDomBuilder->parse(mxInStream);
            CPPUNIT_ASSERT_MESSAGE("Valid input file did not result in XDocument",
                xDoc.is());
            CPPUNIT_ASSERT_MESSAGE("Valid input file resulted in parse errors",
                mxErrHandler->noErrors());

            uno::Reference< xml::sax::XSAXSerializable > xSaxSerializer(
                xDoc, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XSAXSerializable not supported",
                xSaxSerializer.is());

            uno::Reference< xml::sax::XFastSAXSerializable > xFastSaxSerializer(
                xDoc, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE("XFastSAXSerializable not supported",
                xSaxSerializer.is());

            xFastSaxSerializer->fastSerialize(mxHandler,
                mxTokHandler,
                uno::Sequence< beans::StringPair >(),
                maRegisteredNamespaces);
        }
        catch (const css::xml::sax::SAXParseException&)
        {
            CPPUNIT_ASSERT_MESSAGE("Valid input file did not result in XDocument (exception thrown)", false);
        }
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
