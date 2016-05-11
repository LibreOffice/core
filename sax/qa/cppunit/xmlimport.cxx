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

#include <sal/config.h>
#include <sal/types.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <fstream>
#include <osl/file.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

namespace {

using namespace css;
using namespace css::uno;
using namespace css::io;
using namespace std;

// Reference < XInputStream > createStreamFromSequence(
//     const Sequence<sal_Int8> seqBytes ,
//     const Reference < lang::XMultiServiceFactory > &xSMgr )
// {
//     Reference < XInterface > xOutStreamService =
//         xSMgr->createInstance("com.sun.star.io.Pipe");
//     OSL_ASSERT( xOutStreamService.is() );
//     Reference< XOutputStream >  rOutStream( xOutStreamService , UNO_QUERY );
//     OSL_ASSERT( rOutStream.is() );

//     Reference< XInputStream > rInStream( xOutStreamService , UNO_QUERY );
//     OSL_ASSERT( rInStream.is() );

//     rOutStream->writeBytes( seqBytes );
//     rOutStream->flush();
//     rOutStream->closeOutput();

//     return rInStream;
// }

// Reference< XInputStream > createStreamFromFile (
//     const string pcFile ,
//     const Reference < lang::XMultiServiceFactory > &xSMgr )
// {
//     Reference<  XInputStream >  r;
//     ifstream inFile;
//     size_t size = 0;

//     inFile.open(pcFile, ios::in|ios::binary|ios::ate);
//     if(inFile.is_open()) {
//         inFile.seekg(0, ios::end);
//         size = inFile.tellg();
//         inFile.seekg(0, ios::beg);
//         Sequence<sal_Int8> seqIn(size);
//         char *aChar = (char *)seqIn.getArray();
//         inFile.read(aChar, size);
//         // CPPUNIT_ASSERT_MESSAGE(string(aChar),false);
//         r = createStreamFromSequence( seqIn , xSMgr );
//         inFile.close();
//     }
//     return r;
// }

Reference< XInputStream > createStreamFromFile (
    const OUString filePath)
{
    Reference<  XInputStream >  xInputStream;
    OUString aInStr;
    osl::FileBase::getFileURLFromSystemPath(filePath, aInStr);
    // OString sTemp = OUStringToOString( aInStr, RTL_TEXTENCODING_ASCII_US );
    // CPPUNIT_ASSERT_MESSAGE( string(sTemp.pData->buffer), false );
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(aInStr, StreamMode::READ);
    if(pStream == NULL)
        CPPUNIT_ASSERT(false);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    xInputStream.set(xStream, uno::UNO_QUERY);
    return xInputStream;
}

class TestDocumentHandler : public cppu::WeakImplHelper< xml::sax::XDocumentHandler >
{
private:
    OUString m_aStr;

public:
    TestDocumentHandler() {}
    OUString getString() { return m_aStr; }
    // XDocumentHandler
    virtual void SAL_CALL startDocument() throw (xml::sax::SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL endDocument() throw (xml::sax::SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL startElement( const OUString& aName, const Reference< xml::sax::XAttributeList >& xAttribs ) throw (xml::sax::SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement( const OUString& aName ) throw (xml::sax::SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (xml::sax::SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) throw (xml::sax::SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) throw (xml::sax::SAXException, RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator( const Reference< xml::sax::XLocator >& xLocator ) throw (xml::sax::SAXException, RuntimeException, std::exception) override;
};

void SAL_CALL TestDocumentHandler::startDocument()
        throw(xml::sax::SAXException, RuntimeException, std::exception)
{
    m_aStr.clear();
}


void SAL_CALL TestDocumentHandler::endDocument()
        throw(xml::sax::SAXException, RuntimeException, std::exception)
{
}

void SAL_CALL TestDocumentHandler::startElement( const OUString& aName, const Reference< xml::sax::XAttributeList >& xAttribs )
        throw( xml::sax::SAXException, RuntimeException, std::exception )
{
    // m_aStr = m_aStr + "<" + aName + "> ";
}


void SAL_CALL TestDocumentHandler::endElement( const OUString& aName )
    throw( xml::sax::SAXException, RuntimeException, std::exception )
{
        // m_aStr = m_aStr + "</" + aName + ">\n";
}


void SAL_CALL TestDocumentHandler::characters( const OUString& aChars )
        throw(xml::sax::SAXException, RuntimeException, std::exception)
{
    // m_aStr = m_aStr + aChars + " ";
}


void SAL_CALL TestDocumentHandler::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
        throw(xml::sax::SAXException, RuntimeException, std::exception)
{
}


void SAL_CALL TestDocumentHandler::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
        throw(xml::sax::SAXException, RuntimeException, std::exception)
{
}


void SAL_CALL TestDocumentHandler::setDocumentLocator( const Reference< xml::sax::XLocator >& /*xLocator*/ )
        throw(xml::sax::SAXException, RuntimeException, std::exception)
{
}


class XMLImportTest : public test::BootstrapFixture
{
private:
    OUString m_sDirPath;
    Reference< TestDocumentHandler > m_xDocumentHandler;
    Reference< xml::sax::XParser > m_xParser;
    Reference< lang::XMultiServiceFactory > m_xSMgr;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    XMLImportTest() : BootstrapFixture(true, false) {}
    void parse();

    CPPUNIT_TEST_SUITE( XMLImportTest );
    CPPUNIT_TEST( parse );
    CPPUNIT_TEST_SUITE_END();
};

void XMLImportTest::setUp()
{
    test::BootstrapFixture::setUp();
    m_xDocumentHandler.set( new TestDocumentHandler() );
    m_xSMgr = getMultiServiceFactory();
    m_xParser = xml::sax::Parser::create(
        ::comphelper::getProcessComponentContext() );
    m_xParser->setDocumentHandler( m_xDocumentHandler );
    m_sDirPath = m_directories.getPathFromSrc( "/sax2/qa/data/" );
    // OString sTemp = OUStringToOString( sDirPath, RTL_TEXTENCODING_ASCII_US );
    // CPPUNIT_ASSERT_MESSAGE( string(sTemp.pData->buffer), false );

}

void XMLImportTest::tearDown()
{
    test::BootstrapFixture::tearDown();
}

void XMLImportTest::parse()
{
    OUString fileName = "note.xml";
    Reference< XInputStream > rIS = createStreamFromFile( m_sDirPath + fileName );
    xml::sax::InputSource source;
    source.aInputStream = rIS;
    source.sSystemId    = "internal";
    m_xParser->parseStream(source);
    // OUString aStr = m_xDocumentHandler->getString();
    // OString o = OUStringToOString( aStr, RTL_TEXTENCODING_ASCII_US );
    // CPPUNIT_ASSERT_MESSAGE( string(o.pData->buffer), false );
    CPPUNIT_ASSERT(true);
}

CPPUNIT_TEST_SUITE_REGISTRATION( XMLImportTest );
} //namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
