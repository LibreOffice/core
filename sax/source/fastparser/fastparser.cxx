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

#include <boost/scoped_ptr.hpp>

#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>

#include "fastparser.hxx"

#include <string.h>

using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;

namespace sax_fastparser {

// --------------------------------------------------------------------

struct SaxContextImpl
{
    Reference< XFastContextHandler >    mxContext;
    sal_uInt32      mnNamespaceCount;
    sal_Int32       mnElementToken;
    OUString        maNamespace;
    OUString        maElementName;

    SaxContextImpl() { mnNamespaceCount = 0; mnElementToken = 0; }
    SaxContextImpl( const SaxContextImplPtr& p ) { mnNamespaceCount = p->mnNamespaceCount; mnElementToken = p->mnElementToken; maNamespace = p->maNamespace; }
};

// --------------------------------------------------------------------

struct NamespaceDefine
{
    OString     maPrefix;
    sal_Int32   mnToken;
    OUString    maNamespaceURL;

    NamespaceDefine( const OString& rPrefix, sal_Int32 nToken, const OUString& rNamespaceURL ) : maPrefix( rPrefix ), mnToken( nToken ), maNamespaceURL( rNamespaceURL ) {}
};

// --------------------------------------------------------------------
// FastLocatorImpl
// --------------------------------------------------------------------

class FastSaxParser;

class FastLocatorImpl : public WeakImplHelper1< XLocator >
{
public:
    FastLocatorImpl( FastSaxParser *p ) : mpParser(p) {}

    void dispose() { mpParser = 0; }
    void checkDispose() throw (RuntimeException) { if( !mpParser ) throw DisposedException(); }

    //XLocator
    virtual sal_Int32 SAL_CALL getColumnNumber(void) throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getLineNumber(void) throw (RuntimeException);
    virtual OUString SAL_CALL getPublicId(void) throw (RuntimeException);
    virtual OUString SAL_CALL getSystemId(void) throw (RuntimeException);

private:
    FastSaxParser *mpParser;
};

// --------------------------------------------------------------------
// FastSaxParser
// --------------------------------------------------------------------

//---------------------------------------------
// the implementation part
//---------------------------------------------

extern "C" {

static void call_callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts)
{
    FastSaxParser* pFastParser = reinterpret_cast< FastSaxParser* >( userData );
    pFastParser->callbackStartElement( name, atts );
}

static void call_callbackEndElement(void *userData, const XML_Char *name)
{
    FastSaxParser* pFastParser = reinterpret_cast< FastSaxParser* >( userData );
    pFastParser->callbackEndElement( name );
}

static void call_callbackCharacters( void *userData , const XML_Char *s , int nLen )
{
    FastSaxParser* pFastParser = reinterpret_cast< FastSaxParser* >( userData );
    pFastParser->callbackCharacters( s, nLen );
}

static void call_callbackEntityDecl(void *userData, const XML_Char *entityName,
        int is_parameter_entity, const XML_Char *value, int value_length,
        const XML_Char *base, const XML_Char *systemId,
        const XML_Char *publicId, const XML_Char *notationName)
{
    FastSaxParser* pFastParser = reinterpret_cast<FastSaxParser*>(userData);
    pFastParser->callbackEntityDecl(entityName, is_parameter_entity, value,
            value_length, base, systemId, publicId, notationName);
}

static int call_callbackExternalEntityRef( XML_Parser parser,
        const XML_Char *openEntityNames, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId )
{
    FastSaxParser* pFastParser = reinterpret_cast< FastSaxParser* >( XML_GetUserData( parser ) );
    return pFastParser->callbackExternalEntityRef( parser, openEntityNames, base, systemId, publicId );
}

} // extern "C"

// --------------------------------------------------------------------
// FastLocatorImpl implementation
// --------------------------------------------------------------------

sal_Int32 SAL_CALL FastLocatorImpl::getColumnNumber(void) throw (RuntimeException)
{
    checkDispose();
    return XML_GetCurrentColumnNumber( mpParser->getEntity().mpParser );
}

// --------------------------------------------------------------------

sal_Int32 SAL_CALL FastLocatorImpl::getLineNumber(void) throw (RuntimeException)
{
    checkDispose();
    return XML_GetCurrentLineNumber( mpParser->getEntity().mpParser );
}

// --------------------------------------------------------------------

OUString SAL_CALL FastLocatorImpl::getPublicId(void) throw (RuntimeException)
{
    checkDispose();
    return mpParser->getEntity().maStructSource.sPublicId;
}
// --------------------------------------------------------------------

OUString SAL_CALL FastLocatorImpl::getSystemId(void) throw (RuntimeException)
{
    checkDispose();
    return mpParser->getEntity().maStructSource.sSystemId;
}

// --------------------------------------------------------------------

ParserData::ParserData()
{
}

ParserData::~ParserData()
{
}

// --------------------------------------------------------------------

Entity::Entity( const ParserData& rData ) :
    ParserData( rData )
{
    // performance-improvement. Reference is needed when calling the startTag callback.
    // Handing out the same object with every call is allowed (see sax-specification)
    mxAttributes.set( new FastAttributeList( mxTokenHandler ) );
}

Entity::~Entity()
{
}

// --------------------------------------------------------------------
// FastSaxParser implementation
// --------------------------------------------------------------------

FastSaxParser::FastSaxParser()
{
    mxDocumentLocator.set( new FastLocatorImpl( this ) );
}

// --------------------------------------------------------------------

FastSaxParser::~FastSaxParser()
{
    if( mxDocumentLocator.is() )
        mxDocumentLocator->dispose();
}

// --------------------------------------------------------------------

void FastSaxParser::pushContext()
{
    Entity& rEntity = getEntity();
    if( rEntity.maContextStack.empty() )
    {
        rEntity.maContextStack.push( SaxContextImplPtr( new SaxContextImpl ) );
        DefineNamespace( OString("xml"), "http://www.w3.org/XML/1998/namespace");
    }
    else
    {
        rEntity.maContextStack.push( SaxContextImplPtr( new SaxContextImpl( rEntity.maContextStack.top() ) ) );
    }
}

// --------------------------------------------------------------------

void FastSaxParser::popContext()
{
    Entity& rEntity = getEntity();
    assert(!rEntity.maContextStack.empty()); // pop without push?
    if( !rEntity.maContextStack.empty() )
        rEntity.maContextStack.pop();
}

// --------------------------------------------------------------------

void FastSaxParser::DefineNamespace( const OString& rPrefix, const sal_Char* pNamespaceURL )
{
    Entity& rEntity = getEntity();
    assert(!rEntity.maContextStack.empty()); // need a context!
    if( !rEntity.maContextStack.empty() )
    {
        sal_uInt32 nOffset = rEntity.maContextStack.top()->mnNamespaceCount++;

        if( rEntity.maNamespaceDefines.size() <= nOffset )
            rEntity.maNamespaceDefines.resize( rEntity.maNamespaceDefines.size() + 64 );

        const OUString aNamespaceURL( pNamespaceURL, strlen( pNamespaceURL ), RTL_TEXTENCODING_UTF8 );
        rEntity.maNamespaceDefines[nOffset].reset( new NamespaceDefine( rPrefix, GetNamespaceToken( aNamespaceURL ), aNamespaceURL ) );
    }
}

// --------------------------------------------------------------------

sal_Int32 FastSaxParser::GetToken( const OString& rToken )
{
    Sequence< sal_Int8 > aSeq( (sal_Int8*)rToken.getStr(), rToken.getLength() );

    return getEntity().mxTokenHandler->getTokenFromUTF8( aSeq );
}

sal_Int32 FastSaxParser::GetToken( const sal_Char* pToken, sal_Int32 nLen /* = 0 */ )
{
    if( !nLen )
        nLen = strlen( pToken );

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pToken, nLen );

    return getEntity().mxTokenHandler->getTokenFromUTF8( aSeq );
}

// --------------------------------------------------------------------

sal_Int32 FastSaxParser::GetTokenWithPrefix( const OString& rPrefix, const OString& rName ) throw (SAXException)
{
    sal_Int32 nNamespaceToken = FastToken::DONTKNOW;

    Entity& rEntity = getEntity();
    sal_uInt32 nNamespace = rEntity.maContextStack.top()->mnNamespaceCount;
    while( nNamespace-- )
    {
        if( rEntity.maNamespaceDefines[nNamespace]->maPrefix == rPrefix )
        {
            nNamespaceToken = rEntity.maNamespaceDefines[nNamespace]->mnToken;
            break;
        }

        if( !nNamespace )
            throw SAXException(); // prefix that has no defined namespace url
    }

    if( nNamespaceToken != FastToken::DONTKNOW )
    {
        sal_Int32 nNameToken = GetToken( rName.getStr(), rName.getLength() );
        if( nNameToken != FastToken::DONTKNOW )
            return nNamespaceToken | nNameToken;
    }

    return FastToken::DONTKNOW;
}

sal_Int32 FastSaxParser::GetTokenWithPrefix( const sal_Char*pPrefix, int nPrefixLen, const sal_Char* pName, int nNameLen ) throw (SAXException)
{
    sal_Int32 nNamespaceToken = FastToken::DONTKNOW;

    Entity& rEntity = getEntity();
    sal_uInt32 nNamespace = rEntity.maContextStack.top()->mnNamespaceCount;
    while( nNamespace-- )
    {
        const OString& rPrefix( rEntity.maNamespaceDefines[nNamespace]->maPrefix );
        if( (rPrefix.getLength() == nPrefixLen) &&
            (strncmp( rPrefix.getStr(), pPrefix, nPrefixLen ) == 0 ) )
        {
            nNamespaceToken = rEntity.maNamespaceDefines[nNamespace]->mnToken;
            break;
        }

        if( !nNamespace )
            throw SAXException(); // prefix that has no defined namespace url
    }

    if( nNamespaceToken != FastToken::DONTKNOW )
    {
        sal_Int32 nNameToken = GetToken( pName, nNameLen );
        if( nNameToken != FastToken::DONTKNOW )
            return nNamespaceToken | nNameToken;
    }

    return FastToken::DONTKNOW;
}

// --------------------------------------------------------------------

sal_Int32 FastSaxParser::GetNamespaceToken( const OUString& rNamespaceURL )
{
    NamespaceMap::iterator aIter( maNamespaceMap.find( rNamespaceURL ) );
    if( aIter != maNamespaceMap.end() )
        return (*aIter).second;
    else
        return FastToken::DONTKNOW;
}

// --------------------------------------------------------------------

OUString FastSaxParser::GetNamespaceURL( const OString& rPrefix ) throw (SAXException)
{
    Entity& rEntity = getEntity();
    if( !rEntity.maContextStack.empty() )
    {
        sal_uInt32 nNamespace = rEntity.maContextStack.top()->mnNamespaceCount;
        while( nNamespace-- )
            if( rEntity.maNamespaceDefines[nNamespace]->maPrefix == rPrefix )
                return rEntity.maNamespaceDefines[nNamespace]->maNamespaceURL;
    }

    throw SAXException(); // prefix that has no defined namespace url
}

OUString FastSaxParser::GetNamespaceURL( const sal_Char*pPrefix, int nPrefixLen ) throw(SAXException)
{
    Entity& rEntity = getEntity();
    if( pPrefix && !rEntity.maContextStack.empty() )
    {
        sal_uInt32 nNamespace = rEntity.maContextStack.top()->mnNamespaceCount;
        while( nNamespace-- )
        {
            const OString& rPrefix( rEntity.maNamespaceDefines[nNamespace]->maPrefix );
            if( (rPrefix.getLength() == nPrefixLen) &&
                (strncmp( rPrefix.getStr(), pPrefix, nPrefixLen ) == 0 ) )
            {
                return rEntity.maNamespaceDefines[nNamespace]->maNamespaceURL;
            }
        }
    }

    throw SAXException(); // prefix that has no defined namespace url
}

// --------------------------------------------------------------------

sal_Int32 FastSaxParser::GetTokenWithNamespaceURL( const OUString& rNamespaceURL, const sal_Char* pName, int nNameLen )
{
    sal_Int32 nNamespaceToken = GetNamespaceToken( rNamespaceURL );

    if( nNamespaceToken != FastToken::DONTKNOW )
    {
        sal_Int32 nNameToken = GetToken( pName, nNameLen );
        if( nNameToken != FastToken::DONTKNOW )
            return nNamespaceToken | nNameToken;
    }

    return FastToken::DONTKNOW;
}

// --------------------------------------------------------------------

void FastSaxParser::splitName( const XML_Char *pwName, const XML_Char *&rpPrefix, sal_Int32 &rPrefixLen, const XML_Char *&rpName, sal_Int32 &rNameLen )
{
    XML_Char *p;
    for( p = const_cast< XML_Char* >( pwName ), rNameLen = 0, rPrefixLen = 0; *p; p++ )
    {
        if( *p == ':' )
        {
            rPrefixLen = p - pwName;
            rNameLen = 0;
        }
        else
        {
            rNameLen++;
        }
    }
    if( rPrefixLen )
    {
        rpPrefix = pwName;
        rpName = &pwName[ rPrefixLen + 1 ];
    }
    else
    {
        rpPrefix = 0;
        rpName = pwName;
    }
}

/***************
*
* parseStream does Parser-startup initializations. The FastSaxParser::parse() method does
* the file-specific initialization work. (During a parser run, external files may be opened)
*
****************/
void FastSaxParser::parseStream( const InputSource& maStructSource) throw (SAXException, IOException, RuntimeException)
{
    // Only one text at one time
    MutexGuard guard( maMutex );

    Entity entity( maData );
    entity.maStructSource = maStructSource;

    if( !entity.maStructSource.aInputStream.is() )
        throw SAXException("No input source", Reference< XInterface >(), Any() );

    entity.maConverter.setInputStream( entity.maStructSource.aInputStream );
    if( !entity.maStructSource.sEncoding.isEmpty() )
        entity.maConverter.setEncoding( OUStringToOString( entity.maStructSource.sEncoding, RTL_TEXTENCODING_ASCII_US ) );

    // create parser with proper encoding
    entity.mpParser = XML_ParserCreate( 0 );
    if( !entity.mpParser )
        throw SAXException("Couldn't create parser", Reference< XInterface >(), Any() );

    // set all necessary C-Callbacks
    XML_SetUserData( entity.mpParser, this );
    XML_SetElementHandler( entity.mpParser, call_callbackStartElement, call_callbackEndElement );
    XML_SetCharacterDataHandler( entity.mpParser, call_callbackCharacters );
    XML_SetEntityDeclHandler(entity.mpParser, call_callbackEntityDecl);
    XML_SetExternalEntityRefHandler( entity.mpParser, call_callbackExternalEntityRef );

    pushEntity( entity );
    try
    {
        // start the document
        if( entity.mxDocumentHandler.is() )
        {
            Reference< XLocator > xLoc( mxDocumentLocator.get() );
            entity.mxDocumentHandler->setDocumentLocator( xLoc );
            entity.mxDocumentHandler->startDocument();
        }

        parse();

        // finish document
        if( entity.mxDocumentHandler.is() )
        {
            entity.mxDocumentHandler->endDocument();
        }
    }
    catch (const SAXException&)
    {
        popEntity();
        XML_ParserFree( entity.mpParser );
          throw;
    }
    catch (const IOException&)
    {
        popEntity();
        XML_ParserFree( entity.mpParser );
        throw;
    }
    catch (const RuntimeException&)
    {
        popEntity();
        XML_ParserFree( entity.mpParser );
        throw;
    }

    popEntity();
    XML_ParserFree( entity.mpParser );
}

void FastSaxParser::setFastDocumentHandler( const Reference< XFastDocumentHandler >& Handler ) throw (RuntimeException)
{
    maData.mxDocumentHandler = Handler;
}

void SAL_CALL FastSaxParser::setTokenHandler( const Reference< XFastTokenHandler >& Handler ) throw (RuntimeException)
{
    maData.mxTokenHandler = Handler;
}

void SAL_CALL FastSaxParser::registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken ) throw (IllegalArgumentException, RuntimeException)
{
    if( NamespaceToken >= FastToken::NAMESPACE )
    {
        if( GetNamespaceToken( NamespaceURL ) == FastToken::DONTKNOW )
        {
            maNamespaceMap[ NamespaceURL ] = NamespaceToken;
            return;
        }
    }
    throw IllegalArgumentException();
}

OUString SAL_CALL FastSaxParser::getNamespaceURL( const OUString& rPrefix ) throw(IllegalArgumentException, RuntimeException)
{
    try
    {
        return GetNamespaceURL( OUStringToOString( rPrefix, RTL_TEXTENCODING_UTF8 ) );
    }
    catch (const Exception&)
    {
    }
    throw IllegalArgumentException();
}

void FastSaxParser::setErrorHandler(const Reference< XErrorHandler > & Handler) throw (RuntimeException)
{
    maData.mxErrorHandler = Handler;
}

void FastSaxParser::setEntityResolver(const Reference < XEntityResolver > & Resolver) throw (RuntimeException)
{
    maData.mxEntityResolver = Resolver;
}

void FastSaxParser::setLocale( const Locale & Locale ) throw (RuntimeException)
{
    maData.maLocale = Locale;
}

Sequence< OUString > FastSaxParser::getSupportedServiceNames_Static(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OUString( PARSER_SERVICE_NAME );
    return aRet;
}

// XServiceInfo
OUString FastSaxParser::getImplementationName() throw (RuntimeException)
{
    return OUString( PARSER_IMPLEMENTATION_NAME );
}

// XServiceInfo
sal_Bool FastSaxParser::supportsService(const OUString& ServiceName) throw (RuntimeException)
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > FastSaxParser::getSupportedServiceNames(void) throw (RuntimeException)
{

    Sequence<OUString> seq(1);
    seq.getArray()[0] = OUString( PARSER_SERVICE_NAME );
    return seq;
}


/*---------------------------------------
*
* Helper functions and classes
*
*-------------------------------------------*/

namespace {

OUString lclGetErrorMessage( XML_Error xmlE, const OUString& sSystemId, sal_Int32 nLine )
{
    const sal_Char* pMessage = "";
    switch( xmlE )
    {
        case XML_ERROR_NONE:                            pMessage = "No";                                    break;
        case XML_ERROR_NO_MEMORY:                       pMessage = "no memory";                             break;
        case XML_ERROR_SYNTAX:                          pMessage = "syntax";                                break;
        case XML_ERROR_NO_ELEMENTS:                     pMessage = "no elements";                           break;
        case XML_ERROR_INVALID_TOKEN:                   pMessage = "invalid token";                         break;
        case XML_ERROR_UNCLOSED_TOKEN:                  pMessage = "unclosed token";                        break;
        case XML_ERROR_PARTIAL_CHAR:                    pMessage = "partial char";                          break;
        case XML_ERROR_TAG_MISMATCH:                    pMessage = "tag mismatch";                          break;
        case XML_ERROR_DUPLICATE_ATTRIBUTE:             pMessage = "duplicate attribute";                   break;
        case XML_ERROR_JUNK_AFTER_DOC_ELEMENT:          pMessage = "junk after doc element";                break;
        case XML_ERROR_PARAM_ENTITY_REF:                pMessage = "parameter entity reference";            break;
        case XML_ERROR_UNDEFINED_ENTITY:                pMessage = "undefined entity";                      break;
        case XML_ERROR_RECURSIVE_ENTITY_REF:            pMessage = "recursive entity reference";            break;
        case XML_ERROR_ASYNC_ENTITY:                    pMessage = "async entity";                          break;
        case XML_ERROR_BAD_CHAR_REF:                    pMessage = "bad char reference";                    break;
        case XML_ERROR_BINARY_ENTITY_REF:               pMessage = "binary entity reference";               break;
        case XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF:   pMessage = "attribute external entity reference";   break;
        case XML_ERROR_MISPLACED_XML_PI:                pMessage = "misplaced xml processing instruction";  break;
        case XML_ERROR_UNKNOWN_ENCODING:                pMessage = "unknown encoding";                      break;
        case XML_ERROR_INCORRECT_ENCODING:              pMessage = "incorrect encoding";                    break;
        case XML_ERROR_UNCLOSED_CDATA_SECTION:          pMessage = "unclosed cdata section";                break;
        case XML_ERROR_EXTERNAL_ENTITY_HANDLING:        pMessage = "external entity reference";             break;
        case XML_ERROR_NOT_STANDALONE:                  pMessage = "not standalone";                        break;
        default:;
    }

    OUStringBuffer aBuffer( sal_Unicode( '[' ) );
    aBuffer.append( sSystemId );
    aBuffer.append( " line " );
    aBuffer.append( nLine );
    aBuffer.append( "]: " );
    aBuffer.appendAscii( pMessage );
    aBuffer.append( " error" );
    return aBuffer.makeStringAndClear();
}

} // namespace

// starts parsing with actual parser !
void FastSaxParser::parse()
{
    const int BUFFER_SIZE = 16 * 1024;
    Sequence< sal_Int8 > seqOut( BUFFER_SIZE );

    Entity& rEntity = getEntity();
    int nRead = 0;
    do
    {
        nRead = rEntity.maConverter.readAndConvert( seqOut, BUFFER_SIZE );
        if( nRead <= 0 )
        {
            XML_Parse( rEntity.mpParser, (const char*) seqOut.getConstArray(), 0, 1 );
            break;
        }

        bool const bContinue = XML_STATUS_ERROR != XML_Parse(rEntity.mpParser,
            reinterpret_cast<const char*>(seqOut.getConstArray()), nRead, 0);
        // callbacks used inside XML_Parse may have caught an exception
        if( !bContinue || rEntity.maSavedException.hasValue() )
        {
            // Error during parsing !
            XML_Error xmlE = XML_GetErrorCode( rEntity.mpParser );
            OUString sSystemId = mxDocumentLocator->getSystemId();
            sal_Int32 nLine = mxDocumentLocator->getLineNumber();

            SAXParseException aExcept(
                lclGetErrorMessage( xmlE, sSystemId, nLine ),
                Reference< XInterface >(),
                Any( &rEntity.maSavedException, getCppuType( &rEntity.maSavedException ) ),
                mxDocumentLocator->getPublicId(),
                mxDocumentLocator->getSystemId(),
                mxDocumentLocator->getLineNumber(),
                mxDocumentLocator->getColumnNumber()
            );

            // error handler is set, it may throw the exception
            if( rEntity.mxErrorHandler.is() )
                rEntity.mxErrorHandler->fatalError( Any( aExcept ) );

            // error handler has not thrown, but parsing cannot go on, the
            // exception MUST be thrown
            throw aExcept;
        }
    }
    while( nRead > 0 );
}

//------------------------------------------
//
// The C-Callbacks
//
//-----------------------------------------

namespace {

struct AttributeData
{
    OString             maPrefix;
    OString             maName;
    OString             maValue;
};

} // namespace

void FastSaxParser::callbackStartElement( const XML_Char* pwName, const XML_Char** awAttributes )
{
    Reference< XFastContextHandler > xParentContext;
    Entity& rEntity = getEntity();
    if( !rEntity.maContextStack.empty() )
    {
        xParentContext = rEntity.maContextStack.top()->mxContext;
        if( !xParentContext.is() )
        {
            // we ignore current elements, so no processing needed
            pushContext();
            return;
        }
    }

    pushContext();

    rEntity.mxAttributes->clear();

    // create attribute map and process namespace instructions
    int i = 0;
    sal_Int32 nNameLen, nPrefixLen;
    const XML_Char *pName;
    const XML_Char *pPrefix;

    try
    {
        /*  #158414# Each element may define new namespaces, also for attribues.
            First, process all namespace attributes and cache other attributes in a
            vector. Second, process the attributes after namespaces have been
            initialized. */
        ::std::vector< AttributeData > aAttribs;

        // #158414# first: get namespaces
        for( ; awAttributes[i]; i += 2 )
        {
            assert(awAttributes[i+1]);

            splitName( awAttributes[i], pPrefix, nPrefixLen, pName, nNameLen );
            if( nPrefixLen )
            {
                if( (nPrefixLen == 5) && (strncmp( pPrefix, "xmlns", 5 ) == 0) )
                {
                    DefineNamespace( OString( pName, nNameLen ), awAttributes[i+1] );
                }
                else
                {
                    aAttribs.resize( aAttribs.size() + 1 );
                    aAttribs.back().maPrefix = OString( pPrefix, nPrefixLen );
                    aAttribs.back().maName = OString( pName, nNameLen );
                    aAttribs.back().maValue = OString( awAttributes[i+1] );
                }
            }
            else
            {
                if( (nNameLen == 5) && (strcmp( pName, "xmlns" ) == 0) )
                {
                    // namespace of the element found
                    rEntity.maContextStack.top()->maNamespace = OUString( awAttributes[i+1], strlen( awAttributes[i+1] ), RTL_TEXTENCODING_UTF8 );
                }
                else
                {
                    aAttribs.resize( aAttribs.size() + 1 );
                    aAttribs.back().maName = OString( pName, nNameLen );
                    aAttribs.back().maValue = OString( awAttributes[i+1] );
                }
            }
        }

        // #158414# second: fill attribute list with other attributes
        for( ::std::vector< AttributeData >::const_iterator aIt = aAttribs.begin(), aEnd = aAttribs.end(); aIt != aEnd; ++aIt )
        {
            if( !aIt->maPrefix.isEmpty() )
            {
                sal_Int32 nAttributeToken = GetTokenWithPrefix( aIt->maPrefix, aIt->maName );
                if( nAttributeToken != FastToken::DONTKNOW )
                    rEntity.mxAttributes->add( nAttributeToken, aIt->maValue );
                else
                    rEntity.mxAttributes->addUnknown( GetNamespaceURL( aIt->maPrefix ), aIt->maName, aIt->maValue );
            }
            else
            {
                sal_Int32 nAttributeToken = GetToken( aIt->maName );
                if( nAttributeToken != FastToken::DONTKNOW )
                    rEntity.mxAttributes->add( nAttributeToken, aIt->maValue );
                else
                    rEntity.mxAttributes->addUnknown( aIt->maName, aIt->maValue );
            }
        }

        sal_Int32 nElementToken;
        splitName( pwName, pPrefix, nPrefixLen, pName, nNameLen );
        if( nPrefixLen > 0 )
            nElementToken = GetTokenWithPrefix( pPrefix, nPrefixLen, pName, nNameLen );
        else if( !rEntity.maContextStack.top()->maNamespace.isEmpty() )
            nElementToken = GetTokenWithNamespaceURL( rEntity.maContextStack.top()->maNamespace, pName, nNameLen );
        else
            nElementToken = GetToken( pName );
        rEntity.maContextStack.top()->mnElementToken = nElementToken;

        Reference< XFastAttributeList > xAttr( rEntity.mxAttributes.get() );
        Reference< XFastContextHandler > xContext;
        if( nElementToken == FastToken::DONTKNOW )
        {
            if( nPrefixLen > 0 )
                rEntity.maContextStack.top()->maNamespace = GetNamespaceURL( pPrefix, nPrefixLen );

            const OUString aNamespace( rEntity.maContextStack.top()->maNamespace );
            const OUString aElementName( pName, nNameLen, RTL_TEXTENCODING_UTF8 );
            rEntity.maContextStack.top()->maElementName = aElementName;

            if( xParentContext.is() )
                xContext = xParentContext->createUnknownChildContext( aNamespace, aElementName, xAttr );
            else
                xContext = rEntity.mxDocumentHandler->createUnknownChildContext( aNamespace, aElementName, xAttr );

            if( xContext.is() )
            {
                rEntity.maContextStack.top()->mxContext = xContext;
                xContext->startUnknownElement( aNamespace, aElementName, xAttr );
            }
        }
        else
        {
            if( xParentContext.is() )
                xContext = xParentContext->createFastChildContext( nElementToken, xAttr );
            else
                xContext = rEntity.mxDocumentHandler->createFastChildContext( nElementToken, xAttr );


            if( xContext.is() )
            {
                rEntity.maContextStack.top()->mxContext = xContext;
                xContext->startFastElement( nElementToken, xAttr );
            }
        }
    }
    catch (const Exception& e)
    {
        rEntity.maSavedException <<= e;
    }
}

void FastSaxParser::callbackEndElement( SAL_UNUSED_PARAMETER const XML_Char* )
{
    Entity& rEntity = getEntity();
    assert(!rEntity.maContextStack.empty()); // no context?
    if( !rEntity.maContextStack.empty() )
    {
        SaxContextImplPtr pContext = rEntity.maContextStack.top();
        const Reference< XFastContextHandler >& xContext( pContext->mxContext );
        if( xContext.is() ) try
        {
            sal_Int32 nElementToken = pContext->mnElementToken;
            if( nElementToken != FastToken::DONTKNOW )
                xContext->endFastElement( nElementToken );
            else
                xContext->endUnknownElement( pContext->maNamespace, pContext->maElementName );
        }
        catch (const Exception& e)
        {
            rEntity.maSavedException <<= e;
        }

        popContext();
    }
}


void FastSaxParser::callbackCharacters( const XML_Char* s, int nLen )
{
    Entity& rEntity = getEntity();
    const Reference< XFastContextHandler >& xContext( rEntity.maContextStack.top()->mxContext );
    if( xContext.is() ) try
    {
        xContext->characters( OUString( s, nLen, RTL_TEXTENCODING_UTF8 ) );
    }
    catch (const Exception& e)
    {
        rEntity.maSavedException <<= e;
    }
}

void FastSaxParser::callbackEntityDecl(
    SAL_UNUSED_PARAMETER const XML_Char * /*entityName*/,
    SAL_UNUSED_PARAMETER int /*is_parameter_entity*/,
    const XML_Char *value, SAL_UNUSED_PARAMETER int /*value_length*/,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/,
    SAL_UNUSED_PARAMETER const XML_Char * /*systemId*/,
    SAL_UNUSED_PARAMETER const XML_Char * /*publicId*/,
    SAL_UNUSED_PARAMETER const XML_Char * /*notationName*/)
{
    if (value) { // value != 0 means internal entity
        SAL_INFO("sax", "FastSaxParser: internal entity declaration, stopping");
        XML_StopParser(getEntity().mpParser, XML_FALSE);
        getEntity().maSavedException <<= SAXParseException(
            "FastSaxParser: internal entity declaration, stopping",
            static_cast<OWeakObject*>(this), Any(),
            mxDocumentLocator->getPublicId(),
            mxDocumentLocator->getSystemId(),
            mxDocumentLocator->getLineNumber(),
            mxDocumentLocator->getColumnNumber() );
    } else {
        SAL_INFO("sax", "FastSaxParser: ignoring external entity declaration");
    }
}

int FastSaxParser::callbackExternalEntityRef(
    XML_Parser parser, const XML_Char *context,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/, const XML_Char *systemId,
    const XML_Char *publicId )
{
    bool bOK = true;
    InputSource source;

    Entity& rCurrEntity = getEntity();
    Entity aNewEntity( rCurrEntity );

    if( rCurrEntity.mxEntityResolver.is() ) try
    {
        aNewEntity.maStructSource = rCurrEntity.mxEntityResolver->resolveEntity(
            OUString( publicId, strlen( publicId ), RTL_TEXTENCODING_UTF8 ) ,
            OUString( systemId, strlen( systemId ), RTL_TEXTENCODING_UTF8 ) );
    }
    catch (const SAXParseException & e)
    {
        rCurrEntity.maSavedException <<= e;
        bOK = false;
    }
    catch (const SAXException& e)
    {
        rCurrEntity.maSavedException <<= SAXParseException(
            e.Message, e.Context, e.WrappedException,
            mxDocumentLocator->getPublicId(),
            mxDocumentLocator->getSystemId(),
            mxDocumentLocator->getLineNumber(),
            mxDocumentLocator->getColumnNumber() );
        bOK = false;
    }

    if( aNewEntity.maStructSource.aInputStream.is() )
    {
        aNewEntity.mpParser = XML_ExternalEntityParserCreate( parser, context, 0 );
        if( !aNewEntity.mpParser )
        {
            return false;
        }

        aNewEntity.maConverter.setInputStream( aNewEntity.maStructSource.aInputStream );
        pushEntity( aNewEntity );
        try
        {
            parse();
        }
        catch (const SAXParseException& e)
        {
            rCurrEntity.maSavedException <<= e;
            bOK = false;
        }
        catch (const IOException& e)
        {
            SAXException aEx;
            aEx.WrappedException <<= e;
            rCurrEntity.maSavedException <<= aEx;
            bOK = false;
        }
        catch (const RuntimeException& e)
        {
            SAXException aEx;
            aEx.WrappedException <<= e;
            rCurrEntity.maSavedException <<= aEx;
            bOK = false;
        }

        popEntity();
        XML_ParserFree( aNewEntity.mpParser );
    }

    return bOK;
}

} // namespace sax_fastparser

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
