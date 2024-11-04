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
#include <comphelper/processfactory.hxx>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/xml/xpath/XPathException.hpp>
#include <com/sun/star/xml/xpath/XPathObjectType.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>

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
                                // (See xmlFatalErrMsg from lib2xml/parse.c and __xmlRaiseError from lib2xml/error.c)
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

    virtual sal_Int32 getTokenDirect( std::string_view /* token */ ) const override
    {
        return -1;
    }
};

class BasicTest : public test::BootstrapFixture
{
protected:
    uno::Reference<XDocumentBuilder>    mxDomBuilder;
    rtl::Reference<ErrorHandler>        mxErrHandler;
    rtl::Reference<SequenceInputStream> mxValidInStream;
    rtl::Reference<SequenceInputStream> mxWarningInStream;
    rtl::Reference<SequenceInputStream> mxErrorInStream;

public:

    BasicTest() {}

    void setUp() override
    {
        test::BootstrapFixture::setUp();

        mxErrHandler.set( new ErrorHandler() );
        uno::Reference<XDocumentBuilder> xDB( getMultiServiceFactory()->createInstance(u"com.sun.star.xml.dom.DocumentBuilder"_ustr), uno::UNO_QUERY_THROW );
        mxDomBuilder.set( xDB );
        mxValidInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(validTestFile), std::size(validTestFile) - 1)) );
        mxWarningInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(warningTestFile), std::size(warningTestFile) - 1)) );
        mxErrorInStream.set( new SequenceInputStream(css::uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const *>(errorTestFile), std::size(errorTestFile) - 1)) );
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

    void testXDocumentBuilder()
    {
        mxDomBuilder->isNamespaceAware();
        mxDomBuilder->isValidating();

        Reference< xml::dom::XDocument > xDocument = mxDomBuilder->newDocument();
        CPPUNIT_ASSERT(xDocument);

        try
        {
            mxDomBuilder->parse(nullptr);
            CPPUNIT_FAIL("XDocumentBuilder.parse(null)");
        }
        catch (css::uno::RuntimeException&)
        {
        }

        uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext(), css::uno::UNO_SET_THROW);
        const uno::Reference<css::ucb::XSimpleFileAccess> xFileAccess(
        xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.ucb.SimpleFileAccess"_ustr, xContext),
        uno::UNO_QUERY_THROW);
        uno::Reference<io::XInputStream> xInputStream(xFileAccess->openFileRead(m_directories.getURLFromSrc(u"/unoxml/qa/unit/data/example.rdf")),
                                                  uno::UNO_SET_THROW);

        xDocument = mxDomBuilder->parse(xInputStream);
        CPPUNIT_ASSERT(xDocument);

        try
        {
            mxDomBuilder->parseURI(u""_ustr);
            CPPUNIT_FAIL("XDocumentBuilder.parseURI(\"\")");
        }
        catch (css::ucb::ContentCreationException&)
        {
        }

        xDocument = mxDomBuilder->parseURI(m_directories.getURLFromSrc(u"/unoxml/qa/unit/data/example.rdf"));
        CPPUNIT_ASSERT(xDocument);

        mxDomBuilder->setEntityResolver(nullptr);
        mxDomBuilder->setErrorHandler(nullptr);
    }

    void testXXPathAPI()
    {
        uno::Reference<xml::xpath::XXPathAPI> xXPathAPI( getMultiServiceFactory()->createInstance(u"com.sun.star.xml.xpath.XPathAPI"_ustr), uno::UNO_QUERY_THROW );

        Reference< xml::dom::XDocument > xDocument = mxDomBuilder->newDocument();
        CPPUNIT_ASSERT(xDocument);

        Reference< xml::dom::XElement > xRoot = xDocument->createElement(u"root"_ustr);
        Reference< xml::dom::XElement > xFoo1 = xDocument->createElement(u"foo"_ustr);
        Reference< xml::dom::XElement > xFoo2 = xDocument->createElement(u"foo"_ustr);
        Reference< xml::dom::XElement > xFooNs = xDocument->createElementNS(u"http://example.com/"_ustr, u"ns:foo"_ustr);
        Reference< xml::dom::XElement > xBar = xDocument->createElement(u"bar"_ustr);

        xDocument->appendChild(xRoot);
        xRoot->appendChild(xFoo1);
        xFoo1->appendChild(xBar);
        xBar->appendChild(xFoo2);
        xRoot->appendChild(xFooNs);

        try
        {
            xXPathAPI->eval(xRoot, u"~/-$+&#_"_ustr);
            CPPUNIT_FAIL("XXPathAPI.eval");
        }
        catch (xml::xpath::XPathException&)
        {
        }

        try
        {
            xXPathAPI->evalNS(xRoot, u"~/-$+&#_"_ustr, xRoot);
            CPPUNIT_FAIL("XXPathAPI.evalNS");
        }
        catch (xml::xpath::XPathException&)
        {
        }

        try
        {
            xXPathAPI->selectNodeList(xRoot, u"~/-$+&#_"_ustr);
            CPPUNIT_FAIL("XXPathAPI.selectNodeList");
        }
        catch (xml::xpath::XPathException&)
        {
        }

        try
        {
            xXPathAPI->selectNodeListNS(xRoot, u"~/-$+&#_"_ustr, xRoot);
            CPPUNIT_FAIL("XXPathAPI.selectNodeListNS");
        }
        catch (xml::xpath::XPathException&)
        {
        }

        try
        {
            xXPathAPI->selectSingleNode(xRoot, u"~/-$+&#_"_ustr);
            CPPUNIT_FAIL("XXPathAPI.selectSingleNode");
        }
        catch (xml::xpath::XPathException&)
        {
        }

        try
        {
            xXPathAPI->selectSingleNodeNS(xRoot, u"~/-$+&#_"_ustr, xRoot);
            CPPUNIT_FAIL("XXPathAPI.selectSingleNodeNS");
        }
        catch (xml::xpath::XPathException&)
        {
        }

        try
        {
            xXPathAPI->eval(nullptr, u"child::foo"_ustr);
            CPPUNIT_FAIL("XXPathAPI.eval(null)");
        }
        catch (uno::RuntimeException&)
        {
        }

        try
        {
            xXPathAPI->evalNS(nullptr, u"child::foo"_ustr, xRoot);
            CPPUNIT_FAIL("XXPathAPI.evalNS(null)");
        }
        catch (uno::RuntimeException&)
        {
        }

        try
        {
            xXPathAPI->selectNodeList(nullptr, u"child::foo"_ustr);
            CPPUNIT_FAIL("XXPathAPI.selectNodeList(null)");
        }
        catch (uno::RuntimeException&)
        {
        }

        try
        {
            xXPathAPI->selectNodeListNS(nullptr, u"child::foo"_ustr, xRoot);
            CPPUNIT_FAIL("XXPathAPI.selectNodeListNS(null)");
        }
        catch (uno::RuntimeException&)
        {
        }

        try
        {
            xXPathAPI->selectSingleNode(nullptr, u"child::foo"_ustr);
            CPPUNIT_FAIL("XXPathAPI.selectSingleNode(null)");
        }
        catch (uno::RuntimeException&)
        {
        }

        try
        {
            xXPathAPI->selectSingleNodeNS(nullptr, u"child::foo"_ustr, xRoot);
            CPPUNIT_FAIL("XXPathAPI.selectSingleNodeNS(null)");
        }
        catch (uno::RuntimeException&)
        {
        }

        {
            uno::Reference<xml::xpath::XXPathObject> xResult = xXPathAPI->eval(xRoot, u"count(child::foo)"_ustr);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(xml::xpath::XPathObjectType_XPATH_NUMBER, xResult->getObjectType());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xResult->getLong());
        }
        {
            uno::Reference<xml::xpath::XXPathObject> xResult = xXPathAPI->evalNS(xRoot, u"count(//ns:foo)"_ustr, xFooNs);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(xml::xpath::XPathObjectType_XPATH_NUMBER, xResult->getObjectType());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xResult->getLong());
        }
        {
            uno::Reference<xml::dom::XNodeList> xResult = xXPathAPI->selectNodeList(xRoot, u"child::foo"_ustr);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xResult->getLength());
            CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFoo1, uno::UNO_QUERY),
                    Reference< XInterface >(xResult->item(0), uno::UNO_QUERY));
        }
        {
            uno::Reference<xml::dom::XNodeList> xResult = xXPathAPI->selectNodeListNS(xRoot, u".//ns:foo"_ustr, xFooNs);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xResult->getLength());
            CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFooNs, uno::UNO_QUERY),
                    Reference< XInterface >(xResult->item(0), uno::UNO_QUERY));
        }
        {
            uno::Reference<xml::dom::XNode> xResult = xXPathAPI->selectSingleNode(xBar, u"child::foo"_ustr);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFoo2, uno::UNO_QUERY),
                    Reference< XInterface >(xResult, uno::UNO_QUERY));
        }
        {
            uno::Reference<xml::dom::XNode> xResult = xXPathAPI->selectSingleNodeNS(xFoo2, u"//ns:foo"_ustr, xFooNs);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFooNs, uno::UNO_QUERY),
                    Reference< XInterface >(xResult, uno::UNO_QUERY));
        }

        try
        {
            xXPathAPI->selectSingleNode(xDocument, u"//pre:foo"_ustr);
            CPPUNIT_FAIL("XXPathAPI.selectSingleNode");
        }
        catch (xml::xpath::XPathException&)
        {
        }

        xXPathAPI->registerNS(u"pre"_ustr, u"http://example.com/"_ustr);

        {
            uno::Reference<xml::dom::XNode> xResult = xXPathAPI->selectSingleNode(xRoot, u"//pre:foo"_ustr);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFooNs, uno::UNO_QUERY),
                    Reference< XInterface >(xResult, uno::UNO_QUERY));
        }

        xXPathAPI->unregisterNS(u"pre"_ustr, u"http://example.com/"_ustr);

        try
        {
            xXPathAPI->selectSingleNode(xDocument, u"//pre:foo"_ustr);
            CPPUNIT_FAIL("XXPathAPI.unregisterNS");
        }
        catch (xml::xpath::XPathException&)
        {
        }
    }

    void testXXPathObject()
    {
        uno::Reference<xml::xpath::XXPathAPI> xXPathAPI( getMultiServiceFactory()->createInstance(u"com.sun.star.xml.xpath.XPathAPI"_ustr), uno::UNO_QUERY_THROW );

        Reference< xml::dom::XDocument > xDocument = mxDomBuilder->newDocument();
        CPPUNIT_ASSERT(xDocument);

        Reference< xml::dom::XElement > xRoot = xDocument->createElement(u"root"_ustr);
        Reference< xml::dom::XElement > xFoo1 = xDocument->createElement(u"foo"_ustr);
        Reference< xml::dom::XElement > xFoo2 = xDocument->createElement(u"foo"_ustr);
        Reference< xml::dom::XElement > xFooNs = xDocument->createElementNS(u"http://example.com/"_ustr, u"ns:foo"_ustr);
        Reference< xml::dom::XElement > xBar = xDocument->createElement(u"bar"_ustr);

        xDocument->appendChild(xRoot);
        xRoot->appendChild(xFoo1);
        xFoo1->appendChild(xBar);
        xBar->appendChild(xFoo2);
        xRoot->appendChild(xFooNs);

        {
            uno::Reference<xml::xpath::XXPathObject> xResult = xXPathAPI->eval(xRoot, u"count(//foo)"_ustr);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(xml::xpath::XPathObjectType_XPATH_NUMBER, xResult->getObjectType());
            CPPUNIT_ASSERT_EQUAL(sal_Int8(2), xResult->getByte());
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xResult->getShort());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xResult->getLong());
            CPPUNIT_ASSERT_EQUAL(sal_Int64(2), xResult->getHyper());
            CPPUNIT_ASSERT_EQUAL(float(2), xResult->getFloat());
            CPPUNIT_ASSERT_EQUAL(double(2), xResult->getDouble());
            CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xResult->getString());
        }
        {
            uno::Reference<xml::xpath::XXPathObject> xResult = xXPathAPI->eval(xRoot, u"count(//foo) = 2"_ustr);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(xml::xpath::XPathObjectType_XPATH_BOOLEAN, xResult->getObjectType());
            CPPUNIT_ASSERT_EQUAL(sal_True, xResult->getBoolean());
            CPPUNIT_ASSERT_EQUAL(u"true"_ustr, xResult->getString());
        }
        {
            uno::Reference<xml::xpath::XXPathObject> xResult = xXPathAPI->eval(xRoot, u"local-name(foo)"_ustr);
            CPPUNIT_ASSERT(xResult);
            CPPUNIT_ASSERT_EQUAL(xml::xpath::XPathObjectType_XPATH_STRING, xResult->getObjectType());
            CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, xResult->getString());
        }
    }

    void testXText()
    {
        Reference< xml::dom::XDocument > xDocument = mxDomBuilder->newDocument();
        CPPUNIT_ASSERT(xDocument);
        uno::Reference<xml::dom::XText> xText = xDocument->createTextNode(u"foobar"_ustr);
        CPPUNIT_ASSERT(xText);

        CPPUNIT_ASSERT_EQUAL(u"foobar"_ustr, xText->getData());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xText->getLength());

        xText->setData(u"foo"_ustr);

        xText->appendData(u"baz"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"foobaz"_ustr, xText->getData());

        try
        {
            xText->deleteData(999, 999);
            CPPUNIT_FAIL("XText.deleteData(999,999)");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }

        xText->deleteData(0, 3);
        CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, xText->getData());

        try
        {
            xText->insertData(999, u"blah"_ustr);
            CPPUNIT_FAIL("XText.insertData(999,\"blah\")");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }

        xText->insertData(1, u"arb"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"barbaz"_ustr, xText->getData());

        try
        {
            xText->replaceData(999, 999, u"x"_ustr);
            CPPUNIT_FAIL("XText.replaceData(999, 999, \"x\")");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }

        xText->replaceData(3, 3, u"foo"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"barfoo"_ustr, xText->getData());

        xText->setData(u"quux"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"quux"_ustr, xText->getData());

        try
        {
            xText->subStringData(999, 999);
            CPPUNIT_FAIL("XText.subStringData(999, 999)");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_INDEX_SIZE_ERR, e.Code);
        }

        CPPUNIT_ASSERT_EQUAL(u"x"_ustr, xText->subStringData(3, 1));

        // XNode
        {
            uno::Reference<xml::dom::XNode> xTextCloneN = xText->cloneNode(false);
            CPPUNIT_ASSERT(xTextCloneN);
            uno::Reference<xml::dom::XText> xTextClone(xTextCloneN, uno::UNO_QUERY_THROW);
            CPPUNIT_ASSERT(xTextClone);
            CPPUNIT_ASSERT(!xTextClone->hasChildNodes());
        }
        {
            uno::Reference<xml::dom::XNode> xTextCloneN = xText->cloneNode(true);
            CPPUNIT_ASSERT(xTextCloneN);
            uno::Reference<xml::dom::XText> xTextClone(xTextCloneN, uno::UNO_QUERY_THROW);
            CPPUNIT_ASSERT(xTextClone);
            CPPUNIT_ASSERT(!xTextClone->hasChildNodes());
        }

        CPPUNIT_ASSERT(!xText->getAttributes());

        uno::Reference<xml::dom::XNodeList> xChildList = xText->getChildNodes();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xChildList->getLength());

        CPPUNIT_ASSERT_EQUAL(u""_ustr, xText->getLocalName());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, xText->getNamespaceURI());
        CPPUNIT_ASSERT(!xText->getNextSibling());
        CPPUNIT_ASSERT_EQUAL(u"#text"_ustr, xText->getNodeName());
        CPPUNIT_ASSERT_EQUAL(xml::dom::NodeType::NodeType_TEXT_NODE, xText->getNodeType());
        CPPUNIT_ASSERT_EQUAL(u"quux"_ustr, xText->getNodeValue());
        CPPUNIT_ASSERT_EQUAL(xDocument, xText->getOwnerDocument());
        CPPUNIT_ASSERT(!xText->getParentNode());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, xText->getPrefix());
        CPPUNIT_ASSERT(!xText->getPreviousSibling());
        CPPUNIT_ASSERT(!xText->hasAttributes());
        CPPUNIT_ASSERT(!xText->hasChildNodes());
        CPPUNIT_ASSERT(!xText->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xText->normalize();
        xText->setNodeValue(u"42"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"42"_ustr, xText->getNodeValue());

        try
        {
            xText->setPrefix(u"foo"_ustr);
            CPPUNIT_FAIL("XText.setPrefix()");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        uno::Reference<xml::dom::XText> xText2 = xDocument->createTextNode(u"foobar"_ustr);
        uno::Reference<xml::dom::XText> xText3 = xDocument->createTextNode(u"foobar"_ustr);

        try
        {
            xText->appendChild(nullptr);
            CPPUNIT_FAIL("XText.appendChild(null)");
        }
        catch (css::uno::RuntimeException&)
        {
        }

        try
        {
            xText->appendChild(xText2);
            CPPUNIT_FAIL("XText.appendChild(xText2)");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        try
        {
            xText->insertBefore(xText2, xText3);
            CPPUNIT_FAIL("XText.insertBefore");
        }
        catch (xml::dom::DOMException&)
        {
        }

        try
        {
            xText->replaceChild(xText2, xText3);
            CPPUNIT_FAIL("XText.replaceChild");
        }
        catch (xml::dom::DOMException&)
        {
        }

        try
        {
            xText->removeChild(nullptr);
            CPPUNIT_FAIL("XText.removeChild(null)");
        }
        catch (css::uno::RuntimeException&)
        {
        }

        try
        {
            xText->removeChild(xText2);
            CPPUNIT_FAIL("XText.removeChild");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

    }

    void testXProcessingInstruction()
    {
        Reference< xml::dom::XDocument > xDocument = mxDomBuilder->newDocument();
        CPPUNIT_ASSERT(xDocument);
        Reference< xml::dom::XProcessingInstruction > xPI = xDocument->createProcessingInstruction(u"foo"_ustr, u"bar"_ustr);
        CPPUNIT_ASSERT(xPI);

        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, xPI->getTarget());
        CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, xPI->getData());

        xPI->setData(u"baz"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, xPI->getData());

        {
            uno::Reference< xml::dom::XProcessingInstruction> xPIClone( xPI->cloneNode(false), uno::UNO_QUERY_THROW );
            CPPUNIT_ASSERT(xPIClone);
            CPPUNIT_ASSERT(!xPIClone->hasChildNodes());
        }
        {
            uno::Reference< xml::dom::XProcessingInstruction> xPIClone( xPI->cloneNode(true), uno::UNO_QUERY_THROW );
            CPPUNIT_ASSERT(xPIClone);
            CPPUNIT_ASSERT(!xPIClone->hasChildNodes());
        }

        CPPUNIT_ASSERT(!xPI->getAttributes());

        uno::Reference<xml::dom::XNodeList> xChildList = xPI->getChildNodes();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xChildList->getLength());

        CPPUNIT_ASSERT_EQUAL(u""_ustr, xPI->getLocalName());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, xPI->getNamespaceURI());
        CPPUNIT_ASSERT(!xPI->getNextSibling());
        CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, xPI->getNodeName());
        CPPUNIT_ASSERT_EQUAL(xml::dom::NodeType::NodeType_PROCESSING_INSTRUCTION_NODE, xPI->getNodeType());
        CPPUNIT_ASSERT_EQUAL(u"baz"_ustr, xPI->getNodeValue());
        CPPUNIT_ASSERT_EQUAL(xDocument, xPI->getOwnerDocument());
        CPPUNIT_ASSERT(!xPI->getParentNode());
        CPPUNIT_ASSERT_EQUAL(u""_ustr, xPI->getPrefix());
        CPPUNIT_ASSERT(!xPI->getPreviousSibling());
        CPPUNIT_ASSERT(!xPI->hasAttributes());
        CPPUNIT_ASSERT(!xPI->hasChildNodes());
        CPPUNIT_ASSERT(!xPI->isSupported(u"frobnication"_ustr, u"v99.33.0.0.0.1"_ustr));

        xPI->normalize();
        xPI->setNodeValue(u"42"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"42"_ustr, xPI->getNodeValue());

        try
        {
            xPI->setPrefix(u"foo"_ustr);
            CPPUNIT_FAIL("XProcessingInstruction.setPrefix()");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR, e.Code);
        }

        uno::Reference<xml::dom::XText> xText2 = xDocument->createTextNode(u"foobar"_ustr);
        uno::Reference<xml::dom::XText> xText3 = xDocument->createTextNode(u"foobar"_ustr);

        try
        {
            xPI->appendChild(nullptr);
            CPPUNIT_FAIL("XProcessingInstruction.appendChild(null)");
        }
        catch (css::uno::RuntimeException&)
        {
        }

        try
        {
            xPI->appendChild(xText2);
            CPPUNIT_FAIL("XProcessingInstruction.appendChild(xText2)");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }

        try
        {
            xPI->insertBefore(xText2, xText3);
            CPPUNIT_FAIL("XProcessingInstruction.insertBefore");
        }
        catch (xml::dom::DOMException&)
        {
        }

        try
        {
            xPI->replaceChild(xText2, xText3);
            CPPUNIT_FAIL("XProcessingInstruction.replaceChild");
        }
        catch (xml::dom::DOMException&)
        {
        }

        try
        {
            xPI->removeChild(nullptr);
            CPPUNIT_FAIL("XProcessingInstruction.removeChild(null)");
        }
        catch (css::uno::RuntimeException&)
        {
        }

        try
        {
            xPI->removeChild(xText2);
            CPPUNIT_FAIL("XProcessingInstruction.removeChild");
        }
        catch (xml::dom::DOMException& e)
        {
            CPPUNIT_ASSERT_EQUAL(xml::dom::DOMExceptionType::DOMExceptionType_HIERARCHY_REQUEST_ERR, e.Code);
        }
    }

    void testXNamedNodeMap_AttributesMap()
    {
        Reference< xml::dom::XDocument > xDocument = mxDomBuilder->newDocument();
        CPPUNIT_ASSERT(xDocument);

        Reference< xml::dom::XElement > xElem = xDocument->createElement(u"foo"_ustr);
        Reference< xml::dom::XNamedNodeMap > xAttributes = xElem->getAttributes();
        CPPUNIT_ASSERT(xAttributes);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xAttributes->getLength());

        // Should it throw an exception ?
        CPPUNIT_ASSERT(!xAttributes->item(4));

        xElem->setAttribute(u"bar"_ustr, u"42"_ustr);
        Reference< xml::dom::XAttr > xAttBar = xElem->getAttributeNode(u"bar"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xAttributes->getLength());
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttBar, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->item(0), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttBar, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->getNamedItem(u"bar"_ustr), uno::UNO_QUERY));

        OUString aNS(u"http://example.com/"_ustr);

        xElem->setAttributeNS(aNS, u"n:bar"_ustr, u"43"_ustr);
        Reference< xml::dom::XAttr > xAttBarNS = xElem->getAttributeNodeNS(aNS, u"bar"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xAttributes->getLength());
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttBar, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->item(0), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttBarNS, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->item(1), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttBar, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->getNamedItem(u"bar"_ustr), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttBarNS, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->getNamedItemNS(aNS, u"bar"_ustr), uno::UNO_QUERY));

        uno::Reference<xml::dom::XNode> xAttrBarNsRem = xAttributes->removeNamedItemNS(aNS, u"bar"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xAttributes->getLength());
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttBar, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->item(0), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttBar, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->getNamedItem(u"bar"_ustr), uno::UNO_QUERY));
        CPPUNIT_ASSERT(!xAttrBarNsRem->getParentNode());

        uno::Reference<xml::dom::XNode> xAttrBarRem = xAttributes->removeNamedItem(u"bar"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xAttributes->getLength());
        CPPUNIT_ASSERT(!xAttrBarRem->getParentNode());

        uno::Reference<xml::dom::XNode> xAttrBarSet = xAttributes->setNamedItem(xAttrBarRem);
        CPPUNIT_ASSERT(xAttrBarSet);
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttrBarSet, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->getNamedItem(u"bar"_ustr), uno::UNO_QUERY));

        uno::Reference<xml::dom::XNode> xAttrBarNsSet = xAttributes->setNamedItemNS(xAttrBarNsRem);
        CPPUNIT_ASSERT(xAttrBarNsSet);
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xAttrBarNsSet, uno::UNO_QUERY),
                Reference< XInterface >(xAttributes->getNamedItemNS(aNS, u"bar"_ustr), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xAttributes->getLength());
    }

    void testXNodeList_ChildList()
    {
        Reference< xml::dom::XDocument > xDocument = mxDomBuilder->newDocument();
        CPPUNIT_ASSERT(xDocument);

        Reference< xml::dom::XElement > xRoot = xDocument->createElement(u"root"_ustr);
        Reference< xml::dom::XElement > xFoo = xDocument->createElement(u"foo"_ustr);
        Reference< xml::dom::XElement > xBar = xDocument->createElement(u"bar"_ustr);
        Reference< xml::dom::XElement > xBaz = xDocument->createElement(u"baz"_ustr);

        xDocument->appendChild(xRoot);

        uno::Reference<xml::dom::XNodeList> xChildList = xRoot->getChildNodes();
        CPPUNIT_ASSERT(xChildList);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xChildList->getLength());

        // Should it throw an exception ?
        CPPUNIT_ASSERT(!xChildList->item(4));

        xRoot->appendChild(xFoo);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xChildList->getLength());
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFoo, uno::UNO_QUERY),
                Reference< XInterface >(xChildList->item(0), uno::UNO_QUERY));

        xRoot->appendChild(xBar);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xChildList->getLength());
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFoo, uno::UNO_QUERY),
                Reference< XInterface >(xChildList->item(0), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xBar, uno::UNO_QUERY),
                Reference< XInterface >(xChildList->item(1), uno::UNO_QUERY));

        xRoot->appendChild(xBaz);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xChildList->getLength());
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFoo, uno::UNO_QUERY),
                Reference< XInterface >(xChildList->item(0), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xBar, uno::UNO_QUERY),
                Reference< XInterface >(xChildList->item(1), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xBaz, uno::UNO_QUERY),
                Reference< XInterface >(xChildList->item(2), uno::UNO_QUERY));

        xRoot->removeChild(xBar);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xChildList->getLength());
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFoo, uno::UNO_QUERY),
                Reference< XInterface >(xChildList->item(0), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xBaz, uno::UNO_QUERY),
                Reference< XInterface >(xChildList->item(1), uno::UNO_QUERY));
    }

    void testXNodeList_NodeList()
    {
        uno::Reference<xml::xpath::XXPathAPI> xXPathAPI( getMultiServiceFactory()->createInstance(u"com.sun.star.xml.xpath.XPathAPI"_ustr), uno::UNO_QUERY_THROW );

        Reference< xml::dom::XDocument > xDocument = mxDomBuilder->newDocument();
        CPPUNIT_ASSERT(xDocument);

        Reference< xml::dom::XElement > xRoot = xDocument->createElement(u"root"_ustr);
        Reference< xml::dom::XElement > xFoo1 = xDocument->createElement(u"foo"_ustr);
        Reference< xml::dom::XElement > xFoo2 = xDocument->createElement(u"foo"_ustr);
        Reference< xml::dom::XElement > xFooNs = xDocument->createElementNS(u"http://example.com/"_ustr, u"ns:foo"_ustr);
        Reference< xml::dom::XElement > xBar = xDocument->createElement(u"bar"_ustr);

        xDocument->appendChild(xRoot);
        xRoot->appendChild(xFoo1);
        xFoo1->appendChild(xBar);
        xBar->appendChild(xFoo2);
        xRoot->appendChild(xFooNs);

        uno::Reference<xml::xpath::XXPathObject> xResult = xXPathAPI->eval(xRoot, u"//foo"_ustr);
        CPPUNIT_ASSERT(xResult);
        CPPUNIT_ASSERT_EQUAL(xml::xpath::XPathObjectType_XPATH_NODESET, xResult->getObjectType());
        uno::Reference<xml::dom::XNodeList> xNodeList = xResult->getNodeList();
        CPPUNIT_ASSERT(xNodeList);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xNodeList->getLength());
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFoo1, uno::UNO_QUERY),
                Reference< XInterface >(xNodeList->item(0), uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL(Reference< XInterface >(xFoo2, uno::UNO_QUERY),
                Reference< XInterface >(xNodeList->item(1), uno::UNO_QUERY));
    }

    void serializerTest ()
    {
        rtl::Reference<DocumentHandler> xHandler = new DocumentHandler;
        rtl::Reference<TokenHandler> xTokHandler = new TokenHandler;
        uno::Sequence< beans::Pair< OUString, sal_Int32 > > aRegisteredNamespaces = {
            beans::make_Pair(
                u"urn:oasis:names:tc:opendocument:xmlns:office:1.0"_ustr,
                xml::sax::FastToken::NAMESPACE),
            beans::make_Pair(
                u"http://www.w3.org/1999/xlink"_ustr,
                2*xml::sax::FastToken::NAMESPACE)
        };

        try
        {
            uno::Reference< xml::dom::XDocument > xDoc =
                mxDomBuilder->parse(mxValidInStream);
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

            xFastSaxSerializer->fastSerialize(xHandler,
                xTokHandler,
                uno::Sequence< beans::StringPair >(),
                aRegisteredNamespaces);
        }
        catch (const css::xml::sax::SAXParseException&)
        {
            CPPUNIT_ASSERT_MESSAGE("Valid input file did not result in XDocument (exception thrown)", false);
        }
    }

    CPPUNIT_TEST_SUITE(BasicTest);
    CPPUNIT_TEST(validInputTest);
    CPPUNIT_TEST(warningInputTest);
    CPPUNIT_TEST(errorInputTest);
    CPPUNIT_TEST(testXDocumentBuilder);
    CPPUNIT_TEST(testXXPathAPI);
    CPPUNIT_TEST(testXXPathObject);
    CPPUNIT_TEST(testXText);
    CPPUNIT_TEST(testXProcessingInstruction);
    CPPUNIT_TEST(testXNamedNodeMap_AttributesMap);
    CPPUNIT_TEST(testXNodeList_ChildList);
    CPPUNIT_TEST(testXNodeList_NodeList);
    CPPUNIT_TEST(serializerTest);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasicTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
