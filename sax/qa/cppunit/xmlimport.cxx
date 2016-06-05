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
#include <unordered_map>
#include <libxml/parser.h>
#include <sax/fastattribs.hxx>
#include <xml2utf.hxx>
#include <sax/fastparser.hxx>
#include <expat.h>
#include <salhelper/thread.hxx>

#define XML_CAST( str ) reinterpret_cast< const sal_Char* >( str )

namespace {

using namespace css;
using namespace uno;
using namespace io;
using namespace xml::sax;
using namespace std;
using namespace ::osl;
using namespace sax_fastparser;

class TestFastLocator;
typedef unordered_map< OUString, sal_Int32,
        OUStringHash > NamespaceMap;

Reference< XInputStream > createStreamFromFile (
    const OUString filePath)
{
    Reference<  XInputStream >  xInputStream;
    OUString aInStr;
    FileBase::getFileURLFromSystemPath(filePath, aInStr);
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(aInStr, StreamMode::READ);
    if(pStream == nullptr)
        CPPUNIT_ASSERT(false);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
    xInputStream.set(xStream, uno::UNO_QUERY);
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
    OUString getString() { return m_aStr; }

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
    OUString getString() { return m_aStr; }

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
    for (sal_uInt16 i=0; i<len; i++)
    {
        OUString sAttrValue = unknownAttribs[i].Value;
        OUString sAttrName = unknownAttribs[i].Name;
        OUString sAttrNamespaceURL = unknownAttribs[i].NamespaceURL;
        if ( !sAttrNamespaceURL.isEmpty() )
            m_aStr = m_aStr + sAttrNamespaceURL + ":" + sAttrName + sAttrValue;
        else
            m_aStr = m_aStr + sAttrName + sAttrValue;
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
    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const uno::Sequence<sal_Int8>& )
        throw (uno::RuntimeException, exception) override
    {
        return FastToken::DONTKNOW;
    }
    virtual uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 )
        throw (uno::RuntimeException, exception) override
    {
        CPPUNIT_ASSERT_MESSAGE( "getUTF8Identifier: unexpected call", false );
        return uno::Sequence<sal_Int8>();
    }
};

struct NameWithToken
{
    OUString msName;
    sal_Int32 mnToken;

    NameWithToken(const OUString& sName, const sal_Int32& nToken) :
        msName(sName), mnToken(nToken) {}
};


class TestFastParser : public cppu::WeakImplHelper< xml::sax::XFastParser >
{
private:
    InputSource                                                 m_aInputSource;
    ::sax_expatwrap::XMLFile2UTFConverter                       m_aConverter;
    xmlParserCtxtPtr                                            m_pParser;
    Reference< TestFastLocator >                                m_xDocumentLocator;
    Reference< XFastDocumentHandler >                           m_xDocumentHandler;
    Mutex                                                       m_aMutex;
    Reference< XFastTokenHandler >                              m_xTokenHandler;
    FastTokenHandlerBase*                                       m_pTokenHandler;
    NamespaceMap                                                m_aNamespaceMap;
    stack< Reference< XFastContextHandler > >                   m_aContextStack;
    stack< NameWithToken >                                      m_aNamespaceStack;

    void parse();
    sal_Int32 GetToken( const xmlChar* pName, sal_Int32 nameLen );
    sal_Int32 GetTokenWithNamespace( const xmlChar* pName, sal_Int32 nNameLen, const OUString& rNamespaceURL );
    sal_Int32 GetTokenWithContextNamespace( sal_Int32 nNamespaceToken, const xmlChar* pName, int nNameLen );
    sal_Int32 GetNamespaceToken( const OUString& rNamespaceURL );

public:
    TestFastParser();
    xmlParserCtxtPtr getParser() {return m_pParser;}
    InputSource getInputSource() {return m_aInputSource;}

    // XFastParser
    void parseStream( const xml::sax::InputSource& aInputSource ) throw (xml::sax::SAXException, IOException, RuntimeException, exception);
    void setFastDocumentHandler( const Reference< xml::sax::XFastDocumentHandler >& Handler ) throw (RuntimeException) {m_xDocumentHandler = Handler;}
    void setTokenHandler( const Reference< xml::sax::XFastTokenHandler >& Handler ) throw (RuntimeException) override;
    void registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken ) throw (lang::IllegalArgumentException, RuntimeException) override;
    OUString getNamespaceURL( const OUString&/* rPrefix */) throw(lang::IllegalArgumentException, RuntimeException) {return OUString("");}
    void setErrorHandler( const Reference< xml::sax::XErrorHandler >&/* Handler */) throw (RuntimeException){}
    void setEntityResolver( const Reference< xml::sax::XEntityResolver >&/* Resolver */) throw (RuntimeException){}
    void setLocale( const lang::Locale&/* rLocale */) throw (RuntimeException){}

    //Called by C Callbacks
    void onStartElement( const xmlChar *localName , const xmlChar* prefix, const xmlChar* URI,
        int numNamespaces, const xmlChar** namespaces, int numAttributes, int defaultedAttributes, const xmlChar **attributes );
    void onEndElement( const xmlChar* localName, const xmlChar* prefix, const xmlChar* URI );
    void onCharacters( const xmlChar* s, int nLen );

};

class TestFastLocator : public cppu::WeakImplHelper< XLocator >
{
public:
    explicit TestFastLocator(TestFastParser *p) : m_pParser(p) {}

    void dispose() { m_pParser = nullptr; }

    //XLocator
    virtual sal_Int32 SAL_CALL getColumnNumber() throw (RuntimeException, exception) override;
    virtual sal_Int32 SAL_CALL getLineNumber() throw (RuntimeException, exception) override;
    virtual OUString SAL_CALL getPublicId() throw (RuntimeException, exception) override;
    virtual OUString SAL_CALL getSystemId() throw (RuntimeException, exception) override;

private:
    TestFastParser *m_pParser;
};

sal_Int32 SAL_CALL TestFastLocator::getColumnNumber() throw (RuntimeException, exception)
{
    return xmlSAX2GetColumnNumber( m_pParser->getParser() );
}

sal_Int32 SAL_CALL TestFastLocator::getLineNumber() throw (RuntimeException, exception)
{
    return xmlSAX2GetLineNumber( m_pParser->getParser() );
}

OUString SAL_CALL TestFastLocator::getPublicId() throw (RuntimeException, exception)
{
    return m_pParser->getInputSource().sPublicId;
}

OUString SAL_CALL TestFastLocator::getSystemId() throw (RuntimeException, exception)
{
    return m_pParser->getInputSource().sSystemId;
}

extern "C" {

static void call_onStartElement(void *userData, const xmlChar *localName , const xmlChar* prefix, const xmlChar* URI,
    int numNamespaces, const xmlChar** namespaces, int numAttributes, int defaultedAttributes, const xmlChar **attributes)
{
    TestFastParser* pFastParser = static_cast<TestFastParser*>( userData );
    pFastParser->onStartElement( localName, prefix, URI, numNamespaces, namespaces, numAttributes, defaultedAttributes, attributes );
}

static void call_onEndElement(void *userData, const xmlChar* localName, const xmlChar* prefix, const xmlChar* URI)
{
    TestFastParser* pFastParser = static_cast<TestFastParser*>( userData );
    pFastParser->onEndElement( localName, prefix, URI );
}

static void call_onCharacters( void *userData , const xmlChar *s , int nLen )
{
    TestFastParser* pFastParser = static_cast<TestFastParser*>( userData );
    pFastParser->onCharacters( s, nLen );
}

}

TestFastParser::TestFastParser()
{
    m_xDocumentLocator.set( new TestFastLocator(this) );
}

void TestFastParser::setTokenHandler( const Reference< XFastTokenHandler >& xHandler )
        throw (RuntimeException)
{
    m_xTokenHandler = xHandler;
    m_pTokenHandler = dynamic_cast< FastTokenHandlerBase *>( xHandler.get() );
}

void TestFastParser::registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken )
        throw (lang::IllegalArgumentException, RuntimeException)
{
    if( NamespaceToken >= FastToken::NAMESPACE )
    {
        if( GetNamespaceToken( NamespaceURL ) == FastToken::DONTKNOW )
        {
            m_aNamespaceMap[ NamespaceURL ] = NamespaceToken;
            return;
        }
    }
    throw lang::IllegalArgumentException();
}

sal_Int32 TestFastParser::GetToken( const xmlChar* pName, sal_Int32 nameLen /* = 0 */ )
{
    return FastTokenHandlerBase::getTokenFromChars( m_xTokenHandler,
                                                    m_pTokenHandler,
                                                    XML_CAST( pName ), nameLen ); // uses utf-8
}

sal_Int32 TestFastParser::GetTokenWithNamespace( const xmlChar* pName, sal_Int32 nNameLen, const OUString& rNamespaceURL )
{
    return ( GetTokenWithContextNamespace( GetNamespaceToken(rNamespaceURL), pName, nNameLen ) );
}

sal_Int32 TestFastParser::GetTokenWithContextNamespace( sal_Int32 nNamespaceToken, const xmlChar* pName, int nNameLen )
{
    if( nNamespaceToken != FastToken::DONTKNOW )
    {
        sal_Int32 nNameToken = GetToken( pName, nNameLen );
        if( nNameToken != FastToken::DONTKNOW )
            return nNamespaceToken | nNameToken;
    }

    return FastToken::DONTKNOW;
}

sal_Int32 TestFastParser::GetNamespaceToken( const OUString& rNamespaceURL )
{
    NamespaceMap::iterator aIter( m_aNamespaceMap.find( rNamespaceURL ) );
    if( aIter != m_aNamespaceMap.end() )
        return (*aIter).second;
    else
        return FastToken::DONTKNOW;
}

void TestFastParser::parseStream(const InputSource& aInputSource)
        throw (SAXException, IOException, RuntimeException, exception)
{
    MutexGuard guard( m_aMutex );
    m_aInputSource = aInputSource;
    if (!m_aInputSource.aInputStream.is())
        throw SAXException("No input source", Reference< XInterface >(), Any() );
    m_aConverter.setInputStream( m_aInputSource.aInputStream );
    if( !m_aInputSource.sEncoding.isEmpty() )
        m_aConverter.setEncoding( OUStringToOString( m_aInputSource.sEncoding, RTL_TEXTENCODING_ASCII_US ) );
    m_pParser = nullptr;
    m_aNamespaceStack = stack< NameWithToken >();
    m_aContextStack = stack< Reference< XFastContextHandler > >();
    try
    {
        // start the document
        if( m_xDocumentHandler.is() )
        {
            Reference< XLocator > xLoc( m_xDocumentLocator.get() );
            m_xDocumentHandler->setDocumentLocator( xLoc );
            m_xDocumentHandler->startDocument();
        }
        parse();
        // finish document
        if( m_xDocumentHandler.is() )
        {
            m_xDocumentHandler->endDocument();
        }
    }
    catch (const Exception &)
    {}
}

void TestFastParser::parse()
{
    const int BUFFER_SIZE = 16 * 1024;
    Sequence< sal_Int8 > seqOut( BUFFER_SIZE );

    static xmlSAXHandler callbacks;
    callbacks.startElementNs = call_onStartElement;
    callbacks.endElementNs = call_onEndElement;
    callbacks.characters = call_onCharacters;
    callbacks.initialized = XML_SAX2_MAGIC;

    int nRead = 0;
    do
    {
        nRead = m_aConverter.readAndConvert( seqOut, BUFFER_SIZE );
        if( nRead <= 0 )
        {
            if( m_pParser != nullptr )
            {
                xmlParseChunk( m_pParser, reinterpret_cast<const char*>(seqOut.getConstArray()), 0, 1 );
            }
            break;
        }

        if( m_pParser == nullptr )
        {
            m_pParser = xmlCreatePushParserCtxt( &callbacks, this,
                    reinterpret_cast<const char*>(seqOut.getConstArray()), nRead, nullptr );
            if( !m_pParser )
                throw SAXException("Couldn't create parser", Reference< XInterface >(), Any() );
        }
        else
        {
            xmlParseChunk( m_pParser, reinterpret_cast<const char*>(seqOut.getConstArray()), nRead, 0 );
        }

    } while( nRead > 0 );

}

void TestFastParser::onStartElement(const xmlChar* localName , const xmlChar* prefix, const xmlChar* URI,
    int numNamespaces, const xmlChar** namespaces, int numAttributes, int /*defaultedAttributes*/, const xmlChar **attributes)
{
    OUString sDefaultNamespace;
    sal_Int32 nDefaultNamespaceToken = FastToken::DONTKNOW;
    if ( !m_aNamespaceStack.empty() )
    {
        sDefaultNamespace = m_aNamespaceStack.top().msName;
        nDefaultNamespaceToken = m_aNamespaceStack.top().mnToken;
    }

    for (int i = 0; i < numNamespaces * 2; i += 2)
    {
        if( namespaces[ i ] == nullptr )
        {
            sDefaultNamespace = OUString( XML_CAST( namespaces[ i + 1 ] ), strlen( XML_CAST( namespaces[ i + 1 ] )), RTL_TEXTENCODING_UTF8 );
            nDefaultNamespaceToken = GetNamespaceToken( sDefaultNamespace );
        }
    }

    Reference< FastAttributeList > xAttribs;
    xAttribs.set( new FastAttributeList( m_xTokenHandler, m_pTokenHandler ) );
    for (int i = 0; i < numAttributes * 5; i += 5)
    {
        if( attributes[ i + 1 ] != nullptr )
        {
            sal_Int32 nAttributeToken = GetTokenWithNamespace( attributes[ i ], strlen( XML_CAST( attributes[ i ] )), OUString( XML_CAST( attributes[ i + 2 ] ), strlen( XML_CAST( attributes[ i + 2 ] )), RTL_TEXTENCODING_UTF8 ) );
            if( nAttributeToken != FastToken::DONTKNOW )
                xAttribs->add( nAttributeToken, XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] );
            else
                xAttribs->addUnknown( OUString( XML_CAST( attributes[ i + 2 ] ), strlen( XML_CAST( attributes[ i + 2 ] )), RTL_TEXTENCODING_UTF8 ),
                        OString( XML_CAST( attributes[ i ] )), OString( XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] ));
        }
        else
        {
            sal_Int32 nAttributeToken = GetToken( attributes[ i ], strlen( XML_CAST( attributes[ i ] )));
            if( nAttributeToken != FastToken::DONTKNOW )
                xAttribs->add( nAttributeToken, XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] );
            else
                xAttribs->addUnknown( XML_CAST( attributes[ i ] ),
                    OString( XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] ));
        }
    }

    sal_Int32 nElementToken;
    OUString sElementNamespace;
    if( prefix != nullptr )
        nElementToken = GetTokenWithNamespace( localName, strlen( XML_CAST( localName )), OUString( XML_CAST( URI ), strlen( XML_CAST( URI )), RTL_TEXTENCODING_UTF8 ));
    else if( !sDefaultNamespace.isEmpty() )
        nElementToken = GetTokenWithContextNamespace( nDefaultNamespaceToken, localName, strlen( XML_CAST( localName )));
    else
        nElementToken = GetToken( localName, strlen( XML_CAST( localName )));

    m_aNamespaceStack.push( NameWithToken(sDefaultNamespace, nDefaultNamespaceToken) );

    XFastContextHandler *pParentContext = nullptr;

    if( !m_aContextStack.empty() )
    {
        pParentContext = m_aContextStack.top().get();
    }

    Reference< XFastContextHandler > xContext;
    if (prefix != nullptr)
        sElementNamespace = OUString( XML_CAST( URI ), strlen( XML_CAST( URI )), RTL_TEXTENCODING_UTF8 );
    else
        sElementNamespace = sDefaultNamespace;

    if( nElementToken == FastToken::DONTKNOW )
    {
        OUString sElementName = OUString( XML_CAST( localName ), strlen( XML_CAST( localName )), RTL_TEXTENCODING_UTF8 );
        if( pParentContext )
        {
            xContext = pParentContext->createUnknownChildContext( sElementNamespace, sElementName, xAttribs );
        }
        else if( m_xDocumentHandler.is() )
            xContext = m_xDocumentHandler->createUnknownChildContext( sElementNamespace, sElementName, xAttribs );

        if( xContext.is() )
        {
            xContext->startUnknownElement( sElementNamespace, sElementName, xAttribs );
        }
    }
    else
    {
        if( pParentContext )
            xContext = pParentContext->createFastChildContext( nElementToken, xAttribs );
        else if( m_xDocumentHandler.is() )
            xContext = m_xDocumentHandler->createFastChildContext( nElementToken, xAttribs );

        if( xContext.is() )
            xContext->startFastElement( nElementToken, xAttribs );
    }
    m_aContextStack.push(xContext);
}

void TestFastParser::onEndElement( const xmlChar* localName, const xmlChar* prefix, const xmlChar* URI )
{
    sal_uInt32 nDefaultNamespaceToken = FastToken::DONTKNOW;
    OUString sDefaultNamespace;
    if ( !m_aNamespaceStack.empty() )
    {
        sDefaultNamespace = m_aNamespaceStack.top().msName;
        nDefaultNamespaceToken = m_aNamespaceStack.top().mnToken;
        m_aNamespaceStack.pop();
    }
    sal_Int32 nElementToken;
    if (prefix != nullptr)
        nElementToken = GetTokenWithNamespace( localName, strlen( XML_CAST( localName )), OUString( XML_CAST( URI ), strlen( XML_CAST( URI )), RTL_TEXTENCODING_UTF8 ));
    else if( !sDefaultNamespace.isEmpty() )
        nElementToken = GetTokenWithContextNamespace( nDefaultNamespaceToken, localName, strlen( XML_CAST( localName )));
    else
        nElementToken = GetToken( localName, strlen( XML_CAST( localName )));

    const Reference< XFastContextHandler >& xContext = m_aContextStack.top();
    if (xContext != nullptr && xContext.is())
    {
        if( nElementToken != FastToken::DONTKNOW )
            xContext->endFastElement( nElementToken );
        else
        {
            OUString sElementName = OUString( XML_CAST( localName ), strlen( XML_CAST( localName )), RTL_TEXTENCODING_UTF8 );
            OUString sNamespace;
            if (prefix != nullptr)
                sNamespace = OUString( XML_CAST( URI ), strlen( XML_CAST( URI )), RTL_TEXTENCODING_UTF8 );
            else
                sNamespace = sDefaultNamespace;
            xContext->endUnknownElement( sNamespace, sElementName );
        }
    }
    m_aContextStack.pop();
}

void TestFastParser::onCharacters( const xmlChar* s, int nLen )
{
    const Reference< XFastContextHandler >& xContext = m_aContextStack.top();
    if (xContext != nullptr && xContext.is())
    {
        xContext->characters( OUString( XML_CAST( s ), nLen, RTL_TEXTENCODING_UTF8 ) );
    }
}


class XMLImportTest : public test::BootstrapFixture
{
private:
    OUString m_sDirPath;
    Reference< TestDocumentHandler > m_xDocumentHandler;
    Reference< TestFastDocumentHandler > m_xFastDocumentHandler;
    Reference< XParser > m_xParser;
    Reference< TestFastParser > m_xFastParser;
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
    m_xDocumentHandler.set( new TestDocumentHandler() );
    m_xFastDocumentHandler.set( new TestFastDocumentHandler() );
    m_xFastTokenHandler.set( new TestTokenHandler() );
    m_xParser = Parser::create(
        ::comphelper::getProcessComponentContext() );
    m_xFastParser.set( new TestFastParser() );
    m_xParser->setDocumentHandler( m_xDocumentHandler );
    m_xFastParser->setFastDocumentHandler( m_xFastDocumentHandler );
    m_xFastParser->setTokenHandler( m_xFastTokenHandler );
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
        OUString parserStr = m_xDocumentHandler->getString();

        source.aInputStream = createStreamFromFile( m_sDirPath + fileNames[i] );
        m_xFastParser->parseStream(source);
        OUString fastParserStr = m_xFastDocumentHandler->getString();

        CPPUNIT_ASSERT( parserStr == fastParserStr );
        // OString o = OUStringToOString( Str, RTL_TEXTENCODING_ASCII_US );
        // CPPUNIT_ASSERT_MESSAGE( string(o.pData->buffer), false );
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION( XMLImportTest );
} //namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
