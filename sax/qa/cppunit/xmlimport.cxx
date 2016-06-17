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
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/Attribute.hpp>
#include <osl/file.hxx>
#include <osl/conditn.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <string>
#include <stack>
#include <deque>
#include <sax/fastparser.hxx>


namespace {

using namespace css;
using namespace uno;
using namespace io;
using namespace xml::sax;
using namespace std;
using namespace ::osl;
using namespace sax_fastparser;

Reference< XInputStream > createStreamFromFile (
    const OUString & filePath)
{
    Reference<  XInputStream >  xInputStream;
    OUString aInStr;
    FileBase::getFileURLFromSystemPath(filePath, aInStr);
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(aInStr, StreamMode::READ);
    if(pStream == nullptr)
        CPPUNIT_ASSERT(false);
    Reference< XStream > xStream(new utl::OStreamWrapper(*pStream));
    xInputStream.set(xStream, UNO_QUERY);
    return xInputStream;
}

class TestDocumentHandler : public cppu::WeakImplHelper< XDocumentHandler >
{
private:
    OUString m_aStr;
    deque< pair<OUString,OUString> > m_aNamespaceStack;
    stack<sal_uInt16> m_aCountStack;

    OUString canonicalform(const OUString &sName, const OUString &sValue, bool isElement);
    OUString getNamespace(const OUString &sName);

public:
    TestDocumentHandler() {}
    const OUString& getString() { return m_aStr; }
    void setString( const OUString & aStr ) { m_aStr = aStr; }

    // XDocumentHandler
    virtual void SAL_CALL startDocument() throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endDocument() throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL startElement( const OUString& aName, const Reference< XAttributeList >& xAttribs ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endElement( const OUString& aName ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator ) throw (SAXException, RuntimeException, exception) override;
};

OUString TestDocumentHandler::canonicalform(const OUString &sName, const OUString &sValue, bool isElement)
{
    sal_Int16 nIndex = sName.indexOf(":");
    if ( !isElement && sName.match( "xmlns" ) )
    {
        m_aCountStack.top() += 1;
        if ( nIndex < 0 )
            m_aNamespaceStack.push_back( make_pair( OUString( "default" ), sValue ) );
        else
            m_aNamespaceStack.push_back( make_pair( sName.copy( nIndex + 1 ), sValue ) );
    }
    else
    {
        if ( nIndex >= 0 )
        {
            OUString sNamespace = getNamespace( sName.copy( 0, nIndex ) );
            return sNamespace + sName.copy(nIndex);
        }
        else
        {
            OUString sDefaultns = getNamespace( "default" );
            if ( !isElement || sDefaultns.isEmpty() )
                return sName;
            else
                return sDefaultns + ":" + sName;
        }
    }
    return OUString("");
}

OUString TestDocumentHandler::getNamespace(const OUString &sName)
{
    for (sal_Int16 i = m_aNamespaceStack.size() - 1; i>=0; i--)
    {
        pair<OUString, OUString> aPair = m_aNamespaceStack.at(i);
        if (aPair.first == sName)
            return aPair.second;
    }
    return OUString("");
}

void SAL_CALL TestDocumentHandler::startDocument()
        throw(SAXException, RuntimeException, exception)
{
    m_aStr.clear();
    m_aNamespaceStack.emplace_back( make_pair( OUString( "default" ), OUString() ) );
    m_aCountStack.emplace(0);
}


void SAL_CALL TestDocumentHandler::endDocument()
        throw(SAXException, RuntimeException, exception)
{
}

void SAL_CALL TestDocumentHandler::startElement( const OUString& aName, const Reference< XAttributeList >& xAttribs )
        throw( SAXException, RuntimeException, exception )
{
    OUString sAttributes;
    m_aCountStack.push(0);
    sal_uInt16 len = xAttribs->getLength();
    for (sal_uInt16 i=0; i<len; i++)
    {
        OUString sAttrValue = xAttribs->getValueByIndex(i);
        OUString sAttrName = canonicalform(xAttribs->getNameByIndex(i), sAttrValue, false);
        if (!sAttrName.isEmpty())
            sAttributes = sAttributes + sAttrName + sAttrValue;
    }
    m_aStr = m_aStr + canonicalform(aName, "", true) + sAttributes;
}


void SAL_CALL TestDocumentHandler::endElement( const OUString& aName )
    throw( SAXException, RuntimeException, exception )
{
    m_aStr = m_aStr + canonicalform(aName, "", true);
    sal_uInt16 nPopQty = m_aCountStack.top();
    for (sal_uInt16 i=0; i<nPopQty; i++)
        m_aNamespaceStack.pop_back();
    m_aCountStack.pop();
}


void SAL_CALL TestDocumentHandler::characters( const OUString& aChars )
        throw(SAXException, RuntimeException, exception)
{
    m_aStr = m_aStr + aChars;
}


void SAL_CALL TestDocumentHandler::ignorableWhitespace( const OUString& aWhitespaces )
        throw(SAXException, RuntimeException, exception)
{
    m_aStr = m_aStr + aWhitespaces;
}


void SAL_CALL TestDocumentHandler::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
        throw(SAXException, RuntimeException, exception)
{
}


void SAL_CALL TestDocumentHandler::setDocumentLocator( const Reference< XLocator >& /*xLocator*/ )
        throw(SAXException, RuntimeException, exception)
{
}

class TestFastDocumentHandler : public cppu::WeakImplHelper< XFastDocumentHandler >
{
private:
    OUString m_aStr;
public:
    const OUString& getString() { return m_aStr; }

    // XFastDocumentHandler
    virtual void SAL_CALL startDocument() throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endDocument() throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator ) throw (SAXException, RuntimeException, exception) override;

    // XFastContextHandler
    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL startUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endFastElement( sal_Int32 Element ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endUnknownElement( const OUString& Namespace, const OUString& Name ) throw (SAXException, RuntimeException, exception) override;
    virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException, exception) override;
    virtual Reference< XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (SAXException, RuntimeException, exception) override;

};

void SAL_CALL TestFastDocumentHandler::startDocument()
        throw (SAXException, RuntimeException, exception)
{
    m_aStr.clear();
}

void SAL_CALL TestFastDocumentHandler::endDocument()
        throw (SAXException, RuntimeException, exception)
{
}

void SAL_CALL TestFastDocumentHandler::setDocumentLocator( const Reference< XLocator >&/* xLocator */ )
        throw (SAXException, RuntimeException, exception)
{
}

void SAL_CALL TestFastDocumentHandler::startFastElement( sal_Int32/* nElement */, const Reference< XFastAttributeList >&/* Attribs */ )
        throw (SAXException, RuntimeException, exception)
{
}

void SAL_CALL TestFastDocumentHandler::startUnknownElement( const OUString& Namespace, const OUString& Name, const Reference< XFastAttributeList >& Attribs  )
        throw (SAXException, RuntimeException, exception)
{
    if ( !Namespace.isEmpty() )
        m_aStr = m_aStr + Namespace + ":" + Name;
    else
        m_aStr = m_aStr + Name;
    Sequence< xml::Attribute > unknownAttribs = Attribs->getUnknownAttributes();
    sal_uInt16 len = unknownAttribs.getLength();
    for (sal_uInt16 i = 0; i < len; i++)
    {
        OUString& rAttrValue = unknownAttribs[i].Value;
        OUString& rAttrName = unknownAttribs[i].Name;
        OUString& rAttrNamespaceURL = unknownAttribs[i].NamespaceURL;
        if ( !rAttrNamespaceURL.isEmpty() )
            m_aStr = m_aStr + rAttrNamespaceURL + ":" + rAttrName + rAttrValue;
        else
            m_aStr = m_aStr + rAttrName + rAttrValue;
    }

}

void SAL_CALL TestFastDocumentHandler::endFastElement( sal_Int32/* nElement */)
        throw (SAXException, RuntimeException, exception)
{
}


void SAL_CALL TestFastDocumentHandler::endUnknownElement( const OUString& Namespace, const OUString& Name )
        throw (SAXException, RuntimeException, exception)
{
    if ( !Namespace.isEmpty() )
        m_aStr = m_aStr + Namespace + ":" + Name;
    else
        m_aStr = m_aStr + Name;
}

Reference< XFastContextHandler > SAL_CALL TestFastDocumentHandler::createFastChildContext( sal_Int32/* nElement */, const Reference< XFastAttributeList >&/* Attribs */ )
        throw (SAXException, RuntimeException, exception)
{
    return this;
}


Reference< XFastContextHandler > SAL_CALL TestFastDocumentHandler::createUnknownChildContext( const OUString&/* Namespace */, const OUString&/* Name */, const Reference< XFastAttributeList >&/* Attribs */ )
        throw (SAXException, RuntimeException, exception)
{
    return this;
}

void SAL_CALL TestFastDocumentHandler::characters( const OUString& aChars )
        throw (SAXException, RuntimeException, exception)
{
    m_aStr = m_aStr + aChars;
}

class TestTokenHandler : public cppu::WeakImplHelper< XFastTokenHandler >
{
public:
    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const Sequence<sal_Int8>& )
        throw (RuntimeException, exception) override
    {
        return FastToken::DONTKNOW;
    }
    virtual Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 )
        throw (RuntimeException, exception) override
    {
        CPPUNIT_ASSERT_MESSAGE( "getUTF8Identifier: unexpected call", false );
        return Sequence<sal_Int8>();
    }
};

class TestLegacyDocumentHandler : public TestDocumentHandler
{
public:
    virtual void SAL_CALL startElement( const OUString& aName, const Reference< XAttributeList >& xAttribs ) throw (SAXException, RuntimeException, exception) override;
    virtual void SAL_CALL endElement( const OUString& aName ) throw (SAXException, RuntimeException, exception) override;
};

void SAL_CALL TestLegacyDocumentHandler::startElement( const OUString& aName, const Reference< XAttributeList >& xAttribs )
        throw( SAXException, RuntimeException, exception )
{
    setString( getString() + aName );
    sal_uInt16 len = xAttribs->getLength();
    for (sal_uInt16 i = 0; i < len; i++)
    {
        OUString sAttrName = xAttribs->getNameByIndex(i);
        OUString sAttrValue = xAttribs->getValueByIndex(i);
        setString( getString() + sAttrName + sAttrValue );
    }
}


void SAL_CALL TestLegacyDocumentHandler::endElement( const OUString& aName )
    throw( SAXException, RuntimeException, exception )
{
    setString( getString() + aName );
}

class XMLImportTest : public test::BootstrapFixture
{
private:
    OUString m_sDirPath;
    Reference< TestDocumentHandler > m_xDocumentHandler;
    Reference< TestFastDocumentHandler > m_xFastDocumentHandler;
    Reference< XParser > m_xParser;
    Reference< XFastParser > m_xFastParser;
    Reference< XParser > m_xLegacyFastParser;
    Reference< TestLegacyDocumentHandler > m_xLegacyDocumentHandler;
    Reference< XFastTokenHandler > m_xFastTokenHandler;

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
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    m_xDocumentHandler.set( new TestDocumentHandler() );
    m_xFastDocumentHandler.set( new TestFastDocumentHandler() );
    m_xLegacyDocumentHandler.set( new TestLegacyDocumentHandler() );
    m_xFastTokenHandler.set( new TestTokenHandler() );
    m_xParser = Parser::create( xContext );
    m_xFastParser = FastParser::create( xContext );
    m_xParser->setDocumentHandler( m_xDocumentHandler );
    m_xFastParser->setFastDocumentHandler( m_xFastDocumentHandler );
    m_xFastParser->setTokenHandler( m_xFastTokenHandler );
    m_xLegacyFastParser.set( xContext->getServiceManager()->createInstanceWithContext
                    ( "com.sun.star.xml.sax.LegacyFastParser", xContext ), UNO_QUERY );
    m_xLegacyFastParser->setDocumentHandler( m_xLegacyDocumentHandler );
    m_sDirPath = m_directories.getPathFromSrc( "/sax/qa/data/" );
}

void XMLImportTest::tearDown()
{
    test::BootstrapFixture::tearDown();
}

void XMLImportTest::parse()
{
    OUString fileNames[] = {"simple.xml", "defaultns.xml", "inlinens.xml",
                            "multiplens.xml", "multiplepfx.xml",
                            "nstoattributes.xml", "nestedns.xml"};

    for (sal_uInt16 i = 0; i < sizeof( fileNames ) / sizeof( string ); i++)
    {
        InputSource source;
        source.sSystemId    = "internal";

        source.aInputStream = createStreamFromFile( m_sDirPath + fileNames[i] );
        m_xParser->parseStream(source);
        const OUString& rParserStr = m_xDocumentHandler->getString();

        source.aInputStream = createStreamFromFile( m_sDirPath + fileNames[i] );
        m_xFastParser->parseStream(source);
        const OUString& rFastParserStr = m_xFastDocumentHandler->getString();

        source.aInputStream = createStreamFromFile( m_sDirPath + fileNames[i] );
        m_xLegacyFastParser->parseStream(source);
        const OUString& rLegacyFastParserStr = m_xLegacyDocumentHandler->getString();

        CPPUNIT_ASSERT_EQUAL( rParserStr, rFastParserStr );
        CPPUNIT_ASSERT_EQUAL( rFastParserStr, rLegacyFastParserStr );
        // OString o = OUStringToOString( Str, RTL_TEXTENCODING_ASCII_US );
        // CPPUNIT_ASSERT_MESSAGE( string(o.pData->buffer), false );
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION( XMLImportTest );
} //namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
