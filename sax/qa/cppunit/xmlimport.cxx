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
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/file.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <sax/fastattribs.hxx>
#include <stack>
#include <string_view>
#include <deque>
#include <rtl/ref.hxx>


namespace {

using namespace css;
using namespace uno;
using namespace io;
using namespace xml::sax;
using namespace ::osl;
using namespace sax_fastparser;

Reference< XInputStream > createStreamFromFile (
    const OUString & filePath)
{
    Reference<  XInputStream >  xInputStream;
    OUString aInStr;
    FileBase::getFileURLFromSystemPath(filePath, aInStr);
    std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(aInStr, StreamMode::READ);
    if(pStream == nullptr)
        CPPUNIT_ASSERT(false);
    Reference< XStream > xStream(new utl::OStreamWrapper(std::move(pStream)));
    xInputStream.set(xStream, UNO_QUERY);
    return xInputStream;
}

class TestDocumentHandler : public cppu::WeakImplHelper< XDocumentHandler >
{
private:
    OUString m_aStr;
    std::deque< std::pair<OUString,OUString> > m_aNamespaceStack;
    std::stack<sal_uInt16> m_aCountStack;

    OUString canonicalform(const OUString &sName, const OUString &sValue, bool isElement);
    OUString getNamespace(std::u16string_view sName);

public:
    TestDocumentHandler() {}
    const OUString & getString() const { return m_aStr; }

    // XDocumentHandler
    virtual void SAL_CALL startDocument() override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL startElement( const OUString& aName, const Reference< XAttributeList >& xAttribs ) override;
    virtual void SAL_CALL endElement( const OUString& aName ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) override;
    virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator ) override;
};

OUString TestDocumentHandler::canonicalform(const OUString &sName, const OUString &sValue, bool isElement)
{
    sal_Int16 nIndex = sName.indexOf(":");
    if ( !isElement && sName.match( "xmlns" ) )
    {
        m_aCountStack.top() += 1;
        if ( nIndex < 0 )
            m_aNamespaceStack.emplace_back( u"default"_ustr, sValue );
        else
            m_aNamespaceStack.emplace_back( sName.copy( nIndex + 1 ), sValue );
    }
    else
    {
        if ( nIndex >= 0 )
        {
            OUString sNamespace = getNamespace( sName.subView( 0, nIndex ) );
            return sNamespace + sName.subView(nIndex);
        }
        else
        {
            OUString sDefaultns = getNamespace( u"default" );
            if ( !isElement || sDefaultns.isEmpty() )
                return sName;
            else
                return sDefaultns + ":" + sName;
        }
    }
    return OUString();
}

OUString TestDocumentHandler::getNamespace(std::u16string_view sName)
{
    for (sal_Int16 i = m_aNamespaceStack.size() - 1; i>=0; i--)
    {
        std::pair<OUString, OUString> aPair = m_aNamespaceStack.at(i);
        if (aPair.first == sName)
            return aPair.second;
    }
    return OUString();
}

void SAL_CALL TestDocumentHandler::startDocument()
{
    m_aStr.clear();
    m_aNamespaceStack.clear();
    m_aNamespaceStack.emplace_back( std::make_pair( u"default"_ustr, OUString() ) );
    m_aCountStack = std::stack<sal_uInt16>();
    m_aCountStack.emplace(0);
}


void SAL_CALL TestDocumentHandler::endDocument()
{
}

void SAL_CALL TestDocumentHandler::startElement( const OUString& aName, const Reference< XAttributeList >& xAttribs )
{
    OUString sAttributes;
    m_aCountStack.push(0);
    sal_uInt16 len = xAttribs->getLength();
    for (sal_uInt16 i=0; i<len; i++)
    {
        OUString sAttrValue = xAttribs->getValueByIndex(i);
        OUString sAttrName = canonicalform(xAttribs->getNameByIndex(i), sAttrValue, false);
        if (!sAttrName.isEmpty())
            sAttributes += sAttrName + sAttrValue;
    }
    m_aStr += canonicalform(aName, u""_ustr, true) + sAttributes;
}


void SAL_CALL TestDocumentHandler::endElement( const OUString& aName )
{
    m_aStr += canonicalform(aName, u""_ustr, true);
    sal_uInt16 nPopQty = m_aCountStack.top();
    for (sal_uInt16 i=0; i<nPopQty; i++)
        m_aNamespaceStack.pop_back();
    m_aCountStack.pop();
}


void SAL_CALL TestDocumentHandler::characters( const OUString& aChars )
{
    m_aStr += aChars;
}


void SAL_CALL TestDocumentHandler::ignorableWhitespace( const OUString& aWhitespaces )
{
    m_aStr += aWhitespaces;
}


void SAL_CALL TestDocumentHandler::processingInstruction( const OUString& aTarget, const OUString& aData )
{
    m_aStr += aTarget + aData;
}


void SAL_CALL TestDocumentHandler::setDocumentLocator( const Reference< XLocator >& /*xLocator*/ )
{
}

class NSDocumentHandler : public cppu::WeakImplHelper< XDocumentHandler >
{
public:
    NSDocumentHandler() {}

    // XDocumentHandler
    virtual void SAL_CALL startDocument() override {}
    virtual void SAL_CALL endDocument() override {}
    virtual void SAL_CALL startElement( const OUString& aName, const Reference< XAttributeList >& xAttribs ) override;
    virtual void SAL_CALL endElement( const OUString& /* aName */ ) override {}
    virtual void SAL_CALL characters( const OUString& /* aChars */ ) override {}
    virtual void SAL_CALL ignorableWhitespace( const OUString& /* aWhitespaces */ ) override {}
    virtual void SAL_CALL processingInstruction( const OUString& /* aTarget */, const OUString& /* aData */ ) override {}
    virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& /* xLocator */ ) override {}
};

OUString getNamespaceValue( std::u16string_view rNamespacePrefix )
{
    OUString aNamespaceURI;
    if (rNamespacePrefix == u"office")
        aNamespaceURI = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    else if (rNamespacePrefix == u"text")
        aNamespaceURI = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    else if (rNamespacePrefix == u"note")
        aNamespaceURI = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    return aNamespaceURI;
}

OUString resolveNamespace( const OUString& aName )
{
    int index;
    if (( index = aName.indexOf( ':' )) > 0 )
    {
        if ( aName.getLength() > index + 1 )
        {
            OUString aAttributeName = getNamespaceValue( aName.subView( 0, index ) ) +
                ":" + aName.subView( index + 1 );
            return aAttributeName;
        }
    }
    return aName;
}

void SAL_CALL NSDocumentHandler::startElement( const OUString& aName, const Reference< XAttributeList >&/* xAttribs */ )
{
    if (! (aName == "office:document" || aName == "office:body" || aName == "office:text" ||
        aName == "text:p" || aName == "note:p") )
        CPPUNIT_ASSERT(false);

    OUString sResolvedName = resolveNamespace(aName);
    if (! ( sResolvedName == "urn:oasis:names:tc:opendocument:xmlns:office:1.0:document" ||
        sResolvedName == "urn:oasis:names:tc:opendocument:xmlns:office:1.0:body" ||
        sResolvedName == "urn:oasis:names:tc:opendocument:xmlns:office:1.0:text" ||
        sResolvedName == "urn:oasis:names:tc:opendocument:xmlns:text:1.0:p") )
        CPPUNIT_ASSERT(false);
}

class DummyTokenHandler : public sax_fastparser::FastTokenHandlerBase
{
public:
    const static std::string_view tokens[];
    const static std::u16string_view namespaceURIs[];
    const static std::string_view namespacePrefixes[];

    // XFastTokenHandler
    virtual Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 nToken ) override;
    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const css::uno::Sequence< sal_Int8 >& Identifier ) override;
    //FastTokenHandlerBase
    virtual sal_Int32 getTokenDirect(std::string_view sToken) const override;
};

const std::string_view DummyTokenHandler::tokens[] = {
    "Signature", "CanonicalizationMethod",
    "Algorithm", "Type",
    "DigestMethod", "Reference",
    "document", "spacing",
    "Player", "Height" };

const std::u16string_view DummyTokenHandler::namespaceURIs[] = {
    u"http://www.w3.org/2000/09/xmldsig#",
    u"http://schemas.openxmlformats.org/wordprocessingml/2006/main/",
    u"xyzsports.com/players/football/" };

const std::string_view DummyTokenHandler::namespacePrefixes[] = {
    "",
    "w",
    "Player" };

Sequence< sal_Int8 > DummyTokenHandler::getUTF8Identifier( sal_Int32 nToken )
{
    std::string_view aUtf8Token;
    if ( ( nToken & 0xffff0000 ) != 0 ) //namespace
    {
        sal_uInt32 nNamespaceToken = ( nToken >> 16 ) - 1;
        if ( nNamespaceToken < std::size(namespacePrefixes) )
            aUtf8Token = namespacePrefixes[ nNamespaceToken ];
    }
    else //element or attribute
    {
        size_t nElementToken = nToken & 0xffff;
        if ( nElementToken < std::size(tokens) )
            aUtf8Token = tokens[ nElementToken ];
    }
    Sequence< sal_Int8 > aSeq( reinterpret_cast< const sal_Int8* >(
                aUtf8Token.data() ), aUtf8Token.size() );
    return aSeq;
}

sal_Int32 DummyTokenHandler::getTokenFromUTF8( const uno::Sequence< sal_Int8 >& rIdentifier )
{
    return getTokenDirect(std::string_view(
        reinterpret_cast<const char*>(rIdentifier.getConstArray()), rIdentifier.getLength()));
}

sal_Int32 DummyTokenHandler::getTokenDirect(std::string_view sToken) const
{
    for( size_t  i = 0; i < std::size(tokens); i++ )
    {
        if ( tokens[i] == sToken )
            return static_cast<sal_Int32>(i);
    }
    return FastToken::DONTKNOW;
}


class XMLImportTest : public test::BootstrapFixture
{
private:
    OUString m_sDirPath;
    rtl::Reference< TestDocumentHandler > m_xDocumentHandler;
    Reference< XParser > m_xParser;
    Reference< XParser > m_xLegacyFastParser;

public:
    virtual void setUp() override;

    XMLImportTest() : BootstrapFixture(true, false) {}
    void parse();
    void testMissingNamespaceDeclaration();
    void testIllegalNamespaceUse();

    CPPUNIT_TEST_SUITE( XMLImportTest );
    CPPUNIT_TEST( parse );
    CPPUNIT_TEST( testMissingNamespaceDeclaration );
    CPPUNIT_TEST( testIllegalNamespaceUse );
    CPPUNIT_TEST_SUITE_END();
};

void XMLImportTest::setUp()
{
    test::BootstrapFixture::setUp();
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    m_xDocumentHandler.set( new TestDocumentHandler() );
    m_xParser = Parser::create( xContext );
    m_xParser->setDocumentHandler( m_xDocumentHandler );
    m_xLegacyFastParser.set( xContext->getServiceManager()->createInstanceWithContext
                    ( u"com.sun.star.xml.sax.LegacyFastParser"_ustr, xContext ), UNO_QUERY );
    m_xLegacyFastParser->setDocumentHandler( m_xDocumentHandler );

    Reference< XFastTokenHandler > xTokenHandler;
    xTokenHandler.set( new DummyTokenHandler );
    uno::Reference<lang::XInitialization> const xInit(m_xLegacyFastParser,
                            uno::UNO_QUERY_THROW);
    xInit->initialize({ uno::Any(xTokenHandler) });

    sal_Int32 nNamespaceCount = SAL_N_ELEMENTS(DummyTokenHandler::namespaceURIs);
    uno::Sequence<uno::Any> namespaceArgs( nNamespaceCount + 1 );
    auto p_namespaceArgs = namespaceArgs.getArray();
    p_namespaceArgs[0] <<= u"registerNamespaces"_ustr;
    for (sal_Int32 i = 1; i <= nNamespaceCount; i++ )
    {
        css::beans::Pair<OUString, sal_Int32> rPair( OUString(DummyTokenHandler::namespaceURIs[i - 1]), i << 16 );
        p_namespaceArgs[i] <<= rPair;
    }
    xInit->initialize( namespaceArgs );

    m_sDirPath = m_directories.getPathFromSrc( u"/sax/qa/data/" );
}

void XMLImportTest::parse()
{
    OUString fileNames[] = {u"simple.xml"_ustr, u"defaultns.xml"_ustr, u"inlinens.xml"_ustr,
                            u"multiplens.xml"_ustr, u"multiplepfx.xml"_ustr,
                            u"nstoattributes.xml"_ustr, u"nestedns.xml"_ustr, u"testthreading.xml"_ustr};

    for (size_t i = 0; i < std::size( fileNames ); i++)
    {
        InputSource source;
        source.sSystemId    = "internal";

        source.aInputStream = createStreamFromFile( m_sDirPath + fileNames[i] );
        m_xParser->parseStream(source);
        const OUString rParserStr = m_xDocumentHandler->getString();

        source.aInputStream = createStreamFromFile( m_sDirPath + fileNames[i] );
        m_xLegacyFastParser->parseStream(source);
        const OUString rLegacyFastParserStr = m_xDocumentHandler->getString();

        CPPUNIT_ASSERT_EQUAL( rParserStr, rLegacyFastParserStr );
        // OString o = OUStringToOString( Str, RTL_TEXTENCODING_ASCII_US );
        // CPPUNIT_ASSERT_MESSAGE( string(o.pData->buffer), false );
    }
}

void XMLImportTest::testMissingNamespaceDeclaration()
{
    OUString fileNames[] = { u"manifestwithnsdecl.xml"_ustr, u"manifestwithoutnsdecl.xml"_ustr };

    uno::Reference<lang::XInitialization> const xInit(m_xLegacyFastParser,
                            uno::UNO_QUERY_THROW);
    xInit->initialize({ uno::Any(u"IgnoreMissingNSDecl"_ustr) });

    for (sal_uInt16 i = 0; i < std::size( fileNames ); i++)
    {
        try
        {
            InputSource source;
            source.sSystemId    = "internal";

            source.aInputStream = createStreamFromFile( m_sDirPath + fileNames[i] );
            m_xParser->parseStream(source);
            const OUString rParserStr = m_xDocumentHandler->getString();

            source.aInputStream = createStreamFromFile( m_sDirPath + fileNames[i] );
            m_xLegacyFastParser->parseStream(source);
            const OUString rLegacyFastParserStr = m_xDocumentHandler->getString();

            CPPUNIT_ASSERT_EQUAL( rParserStr, rLegacyFastParserStr );
        }
        catch( const SAXException & )
        {
        }
    }
}

void XMLImportTest::testIllegalNamespaceUse()
{
    rtl::Reference< NSDocumentHandler > m_xNSDocumentHandler;
    m_xNSDocumentHandler.set( new NSDocumentHandler() );
    m_xParser->setDocumentHandler( m_xNSDocumentHandler );
    InputSource source;
    source.sSystemId    = "internal";

    source.aInputStream = createStreamFromFile( m_sDirPath + "multiplepfx.xml" );
    m_xParser->parseStream(source);

    m_xLegacyFastParser->setDocumentHandler( m_xNSDocumentHandler );
    source.aInputStream = createStreamFromFile( m_sDirPath + "multiplepfx.xml" );
    m_xLegacyFastParser->parseStream(source);
}

CPPUNIT_TEST_SUITE_REGISTRATION( XMLImportTest );
} //namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
