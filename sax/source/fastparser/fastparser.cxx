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

#include "sax/fastparser.hxx"
#include "sax/fastattribs.hxx"
#include "xml2utf.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/conditn.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <salhelper/thread.hxx>

#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <stack>
#include <vector>
#include <queue>
#include <cassert>
#include <cstring>
#include <expat.h>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;
using namespace com::sun::star;
using namespace sax_fastparser;

namespace {

struct Event;
class FastLocatorImpl;
struct NamespaceDefine;
struct Entity;

typedef ::boost::shared_ptr< NamespaceDefine > NamespaceDefineRef;

typedef ::boost::unordered_map< OUString, sal_Int32,
        OUStringHash, ::std::equal_to< OUString > > NamespaceMap;

typedef std::vector<Event> EventList;

enum CallbackType { INVALID, START_ELEMENT, END_ELEMENT, CHARACTERS, DONE, EXCEPTION };

struct Event
{
    CallbackType maType;
    sal_Int32 mnElementToken;
    OUString msNamespace;
    OUString msElementName;
    rtl::Reference< FastAttributeList > mxAttributes;
    OUString msChars;
};

struct NameWithToken
{
    OUString msName;
    sal_Int32 mnToken;

    NameWithToken(const OUString& sName, const sal_Int32& nToken) :
        msName(sName), mnToken(nToken) {}
};

struct SaxContext
{
    Reference< XFastContextHandler > mxContext;
    sal_Int32 mnElementToken;
    OUString  maNamespace;
    OUString  maElementName;

    SaxContext( sal_Int32 nElementToken, const OUString& aNamespace, const OUString& aElementName ):
            mnElementToken(nElementToken)
    {
        if (nElementToken == FastToken::DONTKNOW)
        {
            maNamespace = aNamespace;
            maElementName = aElementName;
        }
    }
};


struct ParserData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastDocumentHandler > mxDocumentHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >    mxTokenHandler;
    FastTokenHandlerBase *mpTokenHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XErrorHandler >        mxErrorHandler;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XEntityResolver >      mxEntityResolver;
    ::com::sun::star::lang::Locale          maLocale;

    ParserData();
    ~ParserData();
};

struct NamespaceDefine
{
    OString     maPrefix;
    sal_Int32   mnToken;
    OUString    maNamespaceURL;

    NamespaceDefine( const OString& rPrefix, sal_Int32 nToken, const OUString& rNamespaceURL ) : maPrefix( rPrefix ), mnToken( nToken ), maNamespaceURL( rNamespaceURL ) {}
};

// Entity binds all information needed for a single file | single call of parseStream
struct Entity : public ParserData
{
    // Amount of work producer sends to consumer in one iteration:
    static const size_t mnEventListSize = 1000;

    // unique for each Entity instance:

    // Number of valid events in mpProducedEvents:
    size_t mnProducedEventsSize;
    EventList *mpProducedEvents;
    std::queue< EventList * > maPendingEvents;
    std::queue< EventList * > maUsedEvents;
    osl::Mutex maEventProtector;

    static const size_t mnEventLowWater = 4;
    static const size_t mnEventHighWater = 8;
    osl::Condition maConsumeResume;
    osl::Condition maProduceResume;
    // Event we use to store data if threading is disabled:
    Event maSharedEvent;

    // copied in copy constructor:

    // Allow to disable threading for small documents:
    bool                                    mbEnableThreads;
    ::com::sun::star::xml::sax::InputSource maStructSource;
    XML_Parser                              mpParser;
    ::sax_expatwrap::XMLFile2UTFConverter   maConverter;

    // Exceptions cannot be thrown through the C-XmlParser (possible
    // resource leaks), therefore any exception thrown by a UNO callback
    // must be saved somewhere until the C-XmlParser is stopped.
    ::com::sun::star::uno::Any maSavedException;
    void saveException( const Exception &e );
    void throwException( const ::rtl::Reference< FastLocatorImpl > &xDocumentLocator,
                         bool mbDuringParse );

    ::std::stack< NameWithToken >           maNamespaceStack;
    /* Context for main thread consuming events.
     * startElement() stores the data, which characters() and endElement() uses
     */
    ::std::stack< SaxContext>               maContextStack;
    // Determines which elements of maNamespaceDefines are valid in current context
    ::std::stack< sal_uInt32 >              maNamespaceCount;
    ::std::vector< NamespaceDefineRef >     maNamespaceDefines;

    explicit Entity( const ParserData& rData );
    Entity( const Entity& rEntity );
    ~Entity();
    void startElement( Event *pEvent );
    void characters( const OUString& sChars );
    void endElement();
    EventList* getEventList();
    Event& getEvent( CallbackType aType );
};

} // namespace

namespace sax_fastparser {

class FastSaxParserImpl
{
public:
    FastSaxParserImpl( FastSaxParser* pFront );
    ~FastSaxParserImpl();

    // XFastParser
    void parseStream( const ::com::sun::star::xml::sax::InputSource& aInputSource ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception);
    void setFastDocumentHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastDocumentHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    void setTokenHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    void registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    OUString getNamespaceURL( const OUString& rPrefix ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    void setErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XErrorHandler >& Handler ) throw (::com::sun::star::uno::RuntimeException);
    void setEntityResolver( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XEntityResolver >& Resolver ) throw (::com::sun::star::uno::RuntimeException);
    void setLocale( const ::com::sun::star::lang::Locale& rLocale ) throw (::com::sun::star::uno::RuntimeException);

    // called by the C callbacks of the expat parser
    void callbackStartElement( const XML_Char* name, const XML_Char** atts );
    void callbackEndElement( const XML_Char* name );
    void callbackCharacters( const XML_Char* s, int nLen );
    bool callbackExternalEntityRef( XML_Parser parser, const XML_Char *openEntityNames, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId);
    void callbackEntityDecl(const XML_Char *entityName, int is_parameter_entity,
            const XML_Char *value, int value_length, const XML_Char *base,
            const XML_Char *systemId, const XML_Char *publicId,
            const XML_Char *notationName);

    void pushEntity( const Entity& rEntity );
    void popEntity();
    Entity& getEntity()             { return *mpTop; }
    const Entity& getEntity() const { return *mpTop; }
    void parse();
    void produce( CallbackType aType );

    bool hasNamespaceURL( const OUString& rPrefix ) const;

private:
    bool consume(EventList *);
    void deleteUsedEvents();

    sal_Int32 GetToken( const sal_Char* pToken, sal_Int32 nTokenLen = 0 );
    sal_Int32 GetTokenWithPrefix( const sal_Char*pPrefix, int nPrefixLen, const sal_Char* pName, int nNameLen ) throw (::com::sun::star::xml::sax::SAXException);
    OUString GetNamespaceURL( const OString& rPrefix ) throw (::com::sun::star::xml::sax::SAXException);
    OUString GetNamespaceURL( const sal_Char*pPrefix, int nPrefixLen ) throw (::com::sun::star::xml::sax::SAXException);
    sal_Int32 GetNamespaceToken( const OUString& rNamespaceURL );
    sal_Int32 GetTokenWithContextNamespace( sal_Int32 nNamespaceToken, const sal_Char* pName, int nNameLen );
    void DefineNamespace( const OString& rPrefix, const sal_Char* pNamespaceURL );

    void pushContext();
    void popContext();

    void splitName( const XML_Char *pwName, const XML_Char *&rpPrefix, sal_Int32 &rPrefixLen, const XML_Char *&rpName, sal_Int32 &rNameLen );

private:
    FastSaxParser* mpFront;

    osl::Mutex maMutex; ///< Protecting whole parseStream() execution
    ::rtl::Reference< FastLocatorImpl >     mxDocumentLocator;
    NamespaceMap                            maNamespaceMap;

    ParserData maData;                      /// Cached parser configuration for next call of parseStream().

    Entity *mpTop;                          /// std::stack::top() is amazingly slow => cache this.
    ::std::stack< Entity > maEntities;      /// Entity stack for each call of parseStream().
    FastTokenLookup maTokenLookup;
};

} // namespace sax_fastparser

namespace {

class ParserThread: public salhelper::Thread
{
    FastSaxParserImpl *mpParser;
public:
    ParserThread(FastSaxParserImpl *pParser): Thread("Parser"), mpParser(pParser) {}
private:
    virtual void execute() SAL_OVERRIDE
    {
        try
        {
            mpParser->parse();
        }
        catch (const Exception &)
        {
            Entity &rEntity = mpParser->getEntity();
            rEntity.getEvent( EXCEPTION );
            mpParser->produce( EXCEPTION );
        }
    }
};

extern "C" {

static void call_callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts)
{
    FastSaxParserImpl* pFastParser = reinterpret_cast<FastSaxParserImpl*>( userData );
    pFastParser->callbackStartElement( name, atts );
}

static void call_callbackEndElement(void *userData, const XML_Char *name)
{
    FastSaxParserImpl* pFastParser = reinterpret_cast<FastSaxParserImpl*>( userData );
    pFastParser->callbackEndElement( name );
}

static void call_callbackCharacters( void *userData , const XML_Char *s , int nLen )
{
    FastSaxParserImpl* pFastParser = reinterpret_cast<FastSaxParserImpl*>( userData );
    pFastParser->callbackCharacters( s, nLen );
}

static void call_callbackEntityDecl(void *userData, const XML_Char *entityName,
        int is_parameter_entity, const XML_Char *value, int value_length,
        const XML_Char *base, const XML_Char *systemId,
        const XML_Char *publicId, const XML_Char *notationName)
{
    FastSaxParserImpl* pFastParser = reinterpret_cast<FastSaxParserImpl*>(userData);
    pFastParser->callbackEntityDecl(entityName, is_parameter_entity, value,
            value_length, base, systemId, publicId, notationName);
}

static int call_callbackExternalEntityRef( XML_Parser parser,
        const XML_Char *openEntityNames, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId )
{
    FastSaxParserImpl* pFastParser = reinterpret_cast<FastSaxParserImpl*>( XML_GetUserData( parser ) );
    return pFastParser->callbackExternalEntityRef( parser, openEntityNames, base, systemId, publicId );
}

}

class FastLocatorImpl : public WeakImplHelper1< XLocator >
{
public:
    FastLocatorImpl( FastSaxParserImpl *p ) : mpParser(p) {}

    void dispose() { mpParser = 0; }
    void checkDispose() throw (RuntimeException) { if( !mpParser ) throw DisposedException(); }

    //XLocator
    virtual sal_Int32 SAL_CALL getColumnNumber(void) throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getLineNumber(void) throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getPublicId(void) throw (RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSystemId(void) throw (RuntimeException, std::exception) SAL_OVERRIDE;

private:
    FastSaxParserImpl *mpParser;
};

sal_Int32 SAL_CALL FastLocatorImpl::getColumnNumber(void) throw (RuntimeException, std::exception)
{
    checkDispose();
    return XML_GetCurrentColumnNumber( mpParser->getEntity().mpParser );
}

sal_Int32 SAL_CALL FastLocatorImpl::getLineNumber(void) throw (RuntimeException, std::exception)
{
    checkDispose();
    return XML_GetCurrentLineNumber( mpParser->getEntity().mpParser );
}

OUString SAL_CALL FastLocatorImpl::getPublicId(void) throw (RuntimeException, std::exception)
{
    checkDispose();
    return mpParser->getEntity().maStructSource.sPublicId;
}

OUString SAL_CALL FastLocatorImpl::getSystemId(void) throw (RuntimeException, std::exception)
{
    checkDispose();
    return mpParser->getEntity().maStructSource.sSystemId;
}

ParserData::ParserData()
    : mpTokenHandler( NULL )
{}

ParserData::~ParserData()
{}

Entity::Entity(const ParserData& rData)
    : ParserData(rData)
    , mnProducedEventsSize(0)
    , mpProducedEvents(NULL)
    , mbEnableThreads(false)
    , mpParser(NULL)
{
}

Entity::Entity(const Entity& e)
    : ParserData(e)
    , mnProducedEventsSize(0)
    , mpProducedEvents(NULL)
    , mbEnableThreads(e.mbEnableThreads)
    , maStructSource(e.maStructSource)
    , mpParser(e.mpParser)
    , maConverter(e.maConverter)
    , maSavedException(e.maSavedException)
    , maNamespaceStack(e.maNamespaceStack)
    , maContextStack(e.maContextStack)
    , maNamespaceCount(e.maNamespaceCount)
    , maNamespaceDefines(e.maNamespaceDefines)
{
}

Entity::~Entity()
{
}

void Entity::startElement( Event *pEvent )
{
    const sal_Int32& nElementToken = pEvent->mnElementToken;
    const OUString& aNamespace = pEvent->msNamespace;
    const OUString& aElementName = pEvent->msElementName;

    // Use un-wrapped pointers to avoid significant acquire/release overhead
    XFastContextHandler *pParentContext = NULL;
    if( !maContextStack.empty() )
    {
        pParentContext = maContextStack.top().mxContext.get();
        if( !pParentContext )
        {
            maContextStack.push( SaxContext(nElementToken, aNamespace, aElementName) );
            return;
        }
    }

    maContextStack.push( SaxContext( nElementToken, aNamespace, aElementName ) );

    try
    {
        Reference< XFastAttributeList > xAttr( pEvent->mxAttributes.get() );
        Reference< XFastContextHandler > xContext;
        if( nElementToken == FastToken::DONTKNOW )
        {
            if( pParentContext )
                xContext = pParentContext->createUnknownChildContext( aNamespace, aElementName, xAttr );
            else if( mxDocumentHandler.is() )
                xContext = mxDocumentHandler->createUnknownChildContext( aNamespace, aElementName, xAttr );

            if( xContext.is() )
            {
                xContext->startUnknownElement( aNamespace, aElementName, xAttr );
            }
        }
        else
        {
            if( pParentContext )
                xContext = pParentContext->createFastChildContext( nElementToken, xAttr );
            else if( mxDocumentHandler.is() )
                xContext = mxDocumentHandler->createFastChildContext( nElementToken, xAttr );

            if( xContext.is() )
                xContext->startFastElement( nElementToken, xAttr );
        }
        // swap the reference we own in to avoid referencing thrash.
        maContextStack.top().mxContext.set( static_cast<XFastContextHandler *>( xContext.get() ) );
        xContext.set( NULL, UNO_REF_NO_ACQUIRE );
    }
    catch (const Exception& e)
    {
        saveException( e );
    }
}

void Entity::characters( const OUString& sChars )
{
    const Reference< XFastContextHandler >& xContext( maContextStack.top().mxContext );
    if( xContext.is() ) try
    {
        xContext->characters( sChars );
    }
    catch (const Exception& e)
    {
        saveException( e );
    }
}

void Entity::endElement()
{
    const SaxContext& aContext = maContextStack.top();
    const Reference< XFastContextHandler >& xContext( aContext.mxContext );
    if( xContext.is() ) try
    {
        sal_Int32 nElementToken = aContext.mnElementToken;
        if( nElementToken != FastToken::DONTKNOW )
            xContext->endFastElement( nElementToken );
        else
            xContext->endUnknownElement( aContext.maNamespace, aContext.maElementName );
    }
    catch (const Exception& e)
    {
        saveException( e );
    }
    maContextStack.pop();
}

EventList* Entity::getEventList()
{
    if (!mpProducedEvents)
    {
        osl::ResettableMutexGuard aGuard(maEventProtector);
        if (!maUsedEvents.empty())
        {
            mpProducedEvents = maUsedEvents.front();
            maUsedEvents.pop();
            aGuard.clear(); // unlock
            mnProducedEventsSize = 0;
        }
        if (!mpProducedEvents)
        {
            mpProducedEvents = new EventList();
            mpProducedEvents->resize(mnEventListSize);
            mnProducedEventsSize = 0;
        }
    }
    return mpProducedEvents;
}

Event& Entity::getEvent( CallbackType aType )
{
    if (!mbEnableThreads)
        return maSharedEvent;

    EventList* pEventList = getEventList();
    Event& rEvent = (*pEventList)[mnProducedEventsSize++];
    rEvent.maType = aType;
    return rEvent;
}

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

    OUStringBuffer aBuffer( '[' );
    aBuffer.append( sSystemId );
    aBuffer.append( " line " );
    aBuffer.append( nLine );
    aBuffer.append( "]: " );
    aBuffer.appendAscii( pMessage );
    aBuffer.append( " error" );
    return aBuffer.makeStringAndClear();
}

// throw an exception, but avoid callback if
// during a threaded produce
void Entity::throwException( const ::rtl::Reference< FastLocatorImpl > &xDocumentLocator,
                             bool mbDuringParse )
{
    // Error during parsing !
    SAXParseException aExcept(
        lclGetErrorMessage( XML_GetErrorCode( mpParser ),
                            xDocumentLocator->getSystemId(),
                            xDocumentLocator->getLineNumber() ),
        Reference< XInterface >(),
        Any( &maSavedException, getCppuType( &maSavedException ) ),
        xDocumentLocator->getPublicId(),
        xDocumentLocator->getSystemId(),
        xDocumentLocator->getLineNumber(),
        xDocumentLocator->getColumnNumber()
    );

    // error handler is set, it may throw the exception
    if( !mbDuringParse || !mbEnableThreads )
    {
        if (mxErrorHandler.is() )
            mxErrorHandler->fatalError( Any( aExcept ) );
    }

    // error handler has not thrown, but parsing must stop => throw ourselves
    throw aExcept;
}

// In the single threaded case we emit events via our C
// callbacks, so any exception caught must be queued up until
// we can safely re-throw it from our C++ parent of parse()

// If multi-threaded, we need to push an EXCEPTION event, at
// which point we transfer ownership of maSavedException to
// the consuming thread.
void Entity::saveException( const Exception &e )
{
    // only store the first exception
    if( !maSavedException.hasValue() )
    {
        maSavedException <<= e;
        XML_StopParser( mpParser, /* resumable? */ XML_FALSE );
    }
}

} // namespace

namespace sax_fastparser {

FastSaxParserImpl::FastSaxParserImpl( FastSaxParser* pFront ) :
    mpFront(pFront),
    mpTop(NULL)
{
    mxDocumentLocator.set( new FastLocatorImpl( this ) );
}

FastSaxParserImpl::~FastSaxParserImpl()
{
    if( mxDocumentLocator.is() )
        mxDocumentLocator->dispose();
}

void FastSaxParserImpl::DefineNamespace( const OString& rPrefix, const sal_Char* pNamespaceURL )
{
    Entity& rEntity = getEntity();
    assert(!rEntity.maNamespaceCount.empty()); // need a context!
    if( !rEntity.maNamespaceCount.empty() )
    {
        sal_uInt32 nOffset = rEntity.maNamespaceCount.top()++;

        if( rEntity.maNamespaceDefines.size() <= nOffset )
            rEntity.maNamespaceDefines.resize( rEntity.maNamespaceDefines.size() + 64 );

        const OUString aNamespaceURL( pNamespaceURL, strlen( pNamespaceURL ), RTL_TEXTENCODING_UTF8 );
        rEntity.maNamespaceDefines[nOffset].reset( new NamespaceDefine( rPrefix, GetNamespaceToken( aNamespaceURL ), aNamespaceURL ) );
    }
}

sal_Int32 FastSaxParserImpl::GetToken( const sal_Char* pToken, sal_Int32 nLen /* = 0 */ )
{
    return maTokenLookup.getTokenFromChars( getEntity().mxTokenHandler,
                                            getEntity().mpTokenHandler,
                                            pToken, nLen );
}

sal_Int32 FastSaxParserImpl::GetTokenWithPrefix( const sal_Char*pPrefix, int nPrefixLen, const sal_Char* pName, int nNameLen ) throw (SAXException)
{
    sal_Int32 nNamespaceToken = FastToken::DONTKNOW;

    Entity& rEntity = getEntity();
    sal_uInt32 nNamespace = rEntity.maNamespaceCount.top();
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

sal_Int32 FastSaxParserImpl::GetNamespaceToken( const OUString& rNamespaceURL )
{
    NamespaceMap::iterator aIter( maNamespaceMap.find( rNamespaceURL ) );
    if( aIter != maNamespaceMap.end() )
        return (*aIter).second;
    else
        return FastToken::DONTKNOW;
}

OUString FastSaxParserImpl::GetNamespaceURL( const OString& rPrefix ) throw (SAXException)
{
    Entity& rEntity = getEntity();
    if( !rEntity.maNamespaceCount.empty() )
    {
        sal_uInt32 nNamespace = rEntity.maNamespaceCount.top();
        while( nNamespace-- )
            if( rEntity.maNamespaceDefines[nNamespace]->maPrefix == rPrefix )
                return rEntity.maNamespaceDefines[nNamespace]->maNamespaceURL;
    }

    throw SAXException(); // prefix that has no defined namespace url
}

OUString FastSaxParserImpl::GetNamespaceURL( const sal_Char*pPrefix, int nPrefixLen ) throw(SAXException)
{
    Entity& rEntity = getEntity();
    if( pPrefix && !rEntity.maNamespaceCount.empty() )
    {
        sal_uInt32 nNamespace = rEntity.maNamespaceCount.top();
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

sal_Int32 FastSaxParserImpl::GetTokenWithContextNamespace( sal_Int32 nNamespaceToken, const sal_Char* pName, int nNameLen )
{
    if( nNamespaceToken != FastToken::DONTKNOW )
    {
        sal_Int32 nNameToken = GetToken( pName, nNameLen );
        if( nNameToken != FastToken::DONTKNOW )
            return nNamespaceToken | nNameToken;
    }

    return FastToken::DONTKNOW;
}

void FastSaxParserImpl::splitName( const XML_Char *pwName, const XML_Char *&rpPrefix, sal_Int32 &rPrefixLen, const XML_Char *&rpName, sal_Int32 &rNameLen )
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
void FastSaxParserImpl::parseStream(const InputSource& maStructSource)
    throw (SAXException, IOException, RuntimeException, std::exception)
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
    Entity& rEntity = getEntity();
    try
    {
        // start the document
        if( entity.mxDocumentHandler.is() )
        {
            Reference< XLocator > xLoc( mxDocumentLocator.get() );
            entity.mxDocumentHandler->setDocumentLocator( xLoc );
            entity.mxDocumentHandler->startDocument();
        }

        rEntity.mbEnableThreads = (rEntity.maStructSource.aInputStream->available() > 10000);

        if (rEntity.mbEnableThreads)
        {
            rtl::Reference<ParserThread> xParser;
            xParser = new ParserThread(this);
            xParser->launch();
            bool done = false;
            do {
                rEntity.maConsumeResume.wait();
                rEntity.maConsumeResume.reset();

                osl::ResettableMutexGuard aGuard(rEntity.maEventProtector);
                while (!rEntity.maPendingEvents.empty())
                {
                    if (rEntity.maPendingEvents.size() <= rEntity.mnEventLowWater)
                        rEntity.maProduceResume.set(); // start producer again

                    EventList *pEventList = rEntity.maPendingEvents.front();
                    rEntity.maPendingEvents.pop();
                    aGuard.clear(); // unlock

                    if (!consume(pEventList))
                        done = true;

                    aGuard.reset(); // lock
                    rEntity.maUsedEvents.push(pEventList);
                }
            } while (!done);
            xParser->join();
            deleteUsedEvents();
        }
        else
        {
            parse();
        }

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

void FastSaxParserImpl::setFastDocumentHandler( const Reference< XFastDocumentHandler >& Handler ) throw (RuntimeException)
{
    maData.mxDocumentHandler = Handler;
}

void FastSaxParserImpl::setTokenHandler( const Reference< XFastTokenHandler >& xHandler ) throw (RuntimeException)
{
    maData.mxTokenHandler = xHandler;
    maData.mpTokenHandler = dynamic_cast< FastTokenHandlerBase *>( xHandler.get() );
}

void FastSaxParserImpl::registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken ) throw (IllegalArgumentException, RuntimeException)
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

OUString FastSaxParserImpl::getNamespaceURL( const OUString& rPrefix ) throw(IllegalArgumentException, RuntimeException)
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

void FastSaxParserImpl::setErrorHandler(const Reference< XErrorHandler > & Handler) throw (RuntimeException)
{
    maData.mxErrorHandler = Handler;
}

void FastSaxParserImpl::setEntityResolver(const Reference < XEntityResolver > & Resolver) throw (RuntimeException)
{
    maData.mxEntityResolver = Resolver;
}

void FastSaxParserImpl::setLocale( const Locale & Locale ) throw (RuntimeException)
{
    maData.maLocale = Locale;
}

void FastSaxParserImpl::deleteUsedEvents()
{
    Entity& rEntity = getEntity();
    osl::ResettableMutexGuard aGuard(rEntity.maEventProtector);

    while (!rEntity.maUsedEvents.empty())
    {
        EventList *pEventList = rEntity.maUsedEvents.front();
        rEntity.maUsedEvents.pop();

        aGuard.clear(); // unlock

        delete pEventList;

        aGuard.reset(); // lock
    }
}

void FastSaxParserImpl::produce( CallbackType aType )
{
    Entity& rEntity = getEntity();
    if (aType == DONE ||
        aType == EXCEPTION ||
        rEntity.mnProducedEventsSize == rEntity.mnEventListSize)
    {
        osl::ResettableMutexGuard aGuard(rEntity.maEventProtector);

        while (rEntity.maPendingEvents.size() >= rEntity.mnEventHighWater)
        { // pause parsing for a bit
            aGuard.clear(); // unlock
            rEntity.maProduceResume.wait();
            rEntity.maProduceResume.reset();
            aGuard.reset(); // lock
        }

        rEntity.maPendingEvents.push(rEntity.mpProducedEvents);
        rEntity.mpProducedEvents = 0;

        aGuard.clear(); // unlock

        rEntity.maConsumeResume.set();
    }
}

bool FastSaxParserImpl::hasNamespaceURL( const OUString& rPrefix ) const
{
    const Entity& rEntity = getEntity();

    if (rEntity.maNamespaceCount.empty())
        return false;

    OString aPrefix = OUStringToOString(rPrefix, RTL_TEXTENCODING_UTF8);
    sal_uInt32 nNamespace = rEntity.maNamespaceCount.top();
    while (nNamespace--)
    {
        if (rEntity.maNamespaceDefines[nNamespace]->maPrefix == aPrefix)
            return true;
    }

    return false;
}

bool FastSaxParserImpl::consume(EventList *pEventList)
{
    Entity& rEntity = getEntity();
    for (EventList::iterator aEventIt = pEventList->begin();
         aEventIt != pEventList->end(); ++aEventIt)
    {
        switch ((*aEventIt).maType)
        {
            case START_ELEMENT:
                rEntity.startElement( &(*aEventIt) );
                break;
            case END_ELEMENT:
                rEntity.endElement();
                break;
            case CHARACTERS:
                rEntity.characters( (*aEventIt).msChars );
                break;
            case DONE:
                return false;
            case EXCEPTION:
                rEntity.throwException( mxDocumentLocator, false );
                return false;
            default:
                assert(false);
                return false;
        }
    }
    return true;
}

void FastSaxParserImpl::pushEntity( const Entity& rEntity )
{
    maEntities.push( rEntity );
    mpTop = &maEntities.top();
}

void FastSaxParserImpl::popEntity()
{
    maEntities.pop();
    mpTop = !maEntities.empty() ? &maEntities.top() : NULL;
}

// starts parsing with actual parser !
void FastSaxParserImpl::parse()
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
            rEntity.throwException( mxDocumentLocator, true );
    }
    while( nRead > 0 );
    rEntity.getEvent( DONE );
    if( rEntity.mbEnableThreads )
        produce( DONE );
}

// The C-Callbacks
void FastSaxParserImpl::callbackStartElement( const XML_Char* pwName, const XML_Char** awAttributes )
{
    Entity& rEntity = getEntity();
    if( rEntity.maNamespaceCount.empty() )
    {
        rEntity.maNamespaceCount.push(0);
        DefineNamespace( OString("xml"), "http://www.w3.org/XML/1998/namespace");
    }
    else
    {
        rEntity.maNamespaceCount.push( rEntity.maNamespaceCount.top() );
    }

    // create attribute map and process namespace instructions
    Event& rEvent = getEntity().getEvent( START_ELEMENT );
    if (rEvent.mxAttributes.is())
        rEvent.mxAttributes->clear();
    else
        rEvent.mxAttributes.set(
                new FastAttributeList( rEntity.mxTokenHandler,
                                       rEntity.mpTokenHandler ) );

    sal_Int32 nNameLen, nPrefixLen;
    const XML_Char *pName;
    const XML_Char *pPrefix;

    sal_Int32 nNamespaceToken = FastToken::DONTKNOW;
    if (!rEntity.maNamespaceStack.empty())
    {
        rEvent.msNamespace = rEntity.maNamespaceStack.top().msName;
        nNamespaceToken = rEntity.maNamespaceStack.top().mnToken;
    }

    try
    {
        /*  #158414# Each element may define new namespaces, also for attribues.
            First, process all namespace attributes and cache other attributes in a
            vector. Second, process the attributes after namespaces have been
            initialized. */

        // #158414# first: get namespaces
        for (int i = 0; awAttributes[i]; i += 2)
        {
            assert(awAttributes[i+1]);

            if( awAttributes[i][0] != 'x' ||
                strncmp( awAttributes[i], "xmlns", 5) != 0 )
                continue;

            splitName( awAttributes[i], pPrefix, nPrefixLen, pName, nNameLen );
            if( nPrefixLen )
            {
                if( (nPrefixLen == 5) && (strncmp( pPrefix, "xmlns", 5 ) == 0) )
                {
                    DefineNamespace( OString( pName, nNameLen ), awAttributes[i+1] );
                }
            }
            else
            {
                if( (nNameLen == 5) && (strcmp( pName, "xmlns" ) == 0) )
                {
                    // default namespace is the attribute value
                    rEvent.msNamespace = OUString( awAttributes[i+1], strlen( awAttributes[i+1] ), RTL_TEXTENCODING_UTF8 );
                    nNamespaceToken = GetNamespaceToken( rEvent.msNamespace );
                }
            }
        }

        // #158414# second: fill attribute list with other attributes
        for (int i = 0; awAttributes[i]; i += 2)
        {
            splitName( awAttributes[i], pPrefix, nPrefixLen, pName, nNameLen );
            if( nPrefixLen )
            {
                if( (nPrefixLen != 5) || (strncmp( pPrefix, "xmlns", 5 ) != 0) )
                {
                    sal_Int32 nAttributeToken = GetTokenWithPrefix( pPrefix, nPrefixLen, pName, nNameLen );
                    if( nAttributeToken != FastToken::DONTKNOW )
                        rEvent.mxAttributes->add( nAttributeToken, awAttributes[i+1] );
                    else
                        rEvent.mxAttributes->addUnknown( GetNamespaceURL( pPrefix, nPrefixLen ),
                                OString(pName, nNameLen), awAttributes[i+1] );
                }
            }
            else
            {
                if( (nNameLen != 5) || (strcmp( pName, "xmlns" ) != 0) )
                {
                    sal_Int32 nAttributeToken = GetToken( pName, nNameLen );
                    if( nAttributeToken != FastToken::DONTKNOW )
                        rEvent.mxAttributes->add( nAttributeToken, awAttributes[i+1] );
                    else
                        rEvent.mxAttributes->addUnknown( OString(pName, nNameLen), awAttributes[i+1] );
                }
            }
        }

        splitName( pwName, pPrefix, nPrefixLen, pName, nNameLen );
        if( nPrefixLen > 0 )
            rEvent.mnElementToken = GetTokenWithPrefix( pPrefix, nPrefixLen, pName, nNameLen );
        else if( !rEvent.msNamespace.isEmpty() )
            rEvent.mnElementToken = GetTokenWithContextNamespace( nNamespaceToken, pName, nNameLen );
        else
            rEvent.mnElementToken = GetToken( pName );

        if( rEvent.mnElementToken == FastToken::DONTKNOW )
        {
            if( nPrefixLen > 0 )
            {
                rEvent.msNamespace = GetNamespaceURL( pPrefix, nPrefixLen );
                nNamespaceToken = GetNamespaceToken( rEvent.msNamespace );
            }
            rEvent.msElementName = OUString( pName, nNameLen, RTL_TEXTENCODING_UTF8 );
        }
        else // token is always preferred.
            rEvent.msElementName = OUString( "" );

        rEntity.maNamespaceStack.push( NameWithToken(rEvent.msNamespace, nNamespaceToken) );
        if (rEntity.mbEnableThreads)
            produce( START_ELEMENT );
        else
            rEntity.startElement( &rEvent );
    }
    catch (const Exception& e)
    {
        rEntity.saveException( e );
    }
}

void FastSaxParserImpl::callbackEndElement( SAL_UNUSED_PARAMETER const XML_Char* )
{
    Entity& rEntity = getEntity();
    assert( !rEntity.maNamespaceCount.empty() );
    if( !rEntity.maNamespaceCount.empty() )
        rEntity.maNamespaceCount.pop();

    assert( !rEntity.maNamespaceStack.empty() );
    if( !rEntity.maNamespaceStack.empty() )
        rEntity.maNamespaceStack.pop();

    rEntity.getEvent( END_ELEMENT );
    if (rEntity.mbEnableThreads)
        produce( END_ELEMENT );
    else
        rEntity.endElement();
}

void FastSaxParserImpl::callbackCharacters( const XML_Char* s, int nLen )
{
    Entity& rEntity = getEntity();
    Event& rEvent = rEntity.getEvent( CHARACTERS );
    rEvent.msChars = OUString(s, nLen, RTL_TEXTENCODING_UTF8);
    if (rEntity.mbEnableThreads)
        produce( CHARACTERS );
    else
        rEntity.characters( rEvent.msChars );
}

void FastSaxParserImpl::callbackEntityDecl(
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
        getEntity().saveException( SAXParseException(
            "FastSaxParser: internal entity declaration, stopping",
            static_cast<OWeakObject*>(mpFront), Any(),
            mxDocumentLocator->getPublicId(),
            mxDocumentLocator->getSystemId(),
            mxDocumentLocator->getLineNumber(),
            mxDocumentLocator->getColumnNumber() ) );
    } else {
        SAL_INFO("sax", "FastSaxParser: ignoring external entity declaration");
    }
}

bool FastSaxParserImpl::callbackExternalEntityRef(
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
        rCurrEntity.saveException( e );
        bOK = false;
    }
    catch (const SAXException& e)
    {
        rCurrEntity.saveException( SAXParseException(
            e.Message, e.Context, e.WrappedException,
            mxDocumentLocator->getPublicId(),
            mxDocumentLocator->getSystemId(),
            mxDocumentLocator->getLineNumber(),
            mxDocumentLocator->getColumnNumber() ) );
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
            rCurrEntity.saveException( e );
            bOK = false;
        }
        catch (const IOException& e)
        {
            SAXException aEx;
            aEx.WrappedException <<= e;
            rCurrEntity.saveException( aEx );
            bOK = false;
        }
        catch (const RuntimeException& e)
        {
            SAXException aEx;
            aEx.WrappedException <<= e;
            rCurrEntity.saveException( aEx );
            bOK = false;
        }

        popEntity();
        XML_ParserFree( aNewEntity.mpParser );
    }

    return bOK;
}

FastSaxParser::FastSaxParser() : mpImpl(new FastSaxParserImpl(this)) {}

FastSaxParser::~FastSaxParser()
{
    delete mpImpl;
}

void FastSaxParser::parseStream( const xml::sax::InputSource& aInputSource )
    throw (xml::sax::SAXException, io::IOException,
           uno::RuntimeException, std::exception)
{
    mpImpl->parseStream(aInputSource);
}

void FastSaxParser::setFastDocumentHandler( const uno::Reference<xml::sax::XFastDocumentHandler>& Handler )
    throw (uno::RuntimeException, std::exception)
{
    mpImpl->setFastDocumentHandler(Handler);
}

void FastSaxParser::setTokenHandler( const uno::Reference<xml::sax::XFastTokenHandler>& Handler )
    throw (uno::RuntimeException, std::exception)
{
    mpImpl->setTokenHandler(Handler);
}

void FastSaxParser::registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken )
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    mpImpl->registerNamespace(NamespaceURL, NamespaceToken);
}

OUString FastSaxParser::getNamespaceURL( const OUString& rPrefix )
    throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    return mpImpl->getNamespaceURL(rPrefix);
}

void FastSaxParser::setErrorHandler( const uno::Reference< xml::sax::XErrorHandler >& Handler )
    throw (uno::RuntimeException, std::exception)
{
    mpImpl->setErrorHandler(Handler);
}

void FastSaxParser::setEntityResolver( const uno::Reference< xml::sax::XEntityResolver >& Resolver )
    throw (uno::RuntimeException, std::exception)
{
    mpImpl->setEntityResolver(Resolver);
}

void FastSaxParser::setLocale( const lang::Locale& rLocale )
    throw (uno::RuntimeException, std::exception)
{
    mpImpl->setLocale(rLocale);
}

OUString FastSaxParser::getImplementationName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.extensions.xml.sax.FastParser");
}

sal_Bool FastSaxParser::supportsService( const OUString& ServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> FastSaxParser::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
{
    Sequence<OUString> seq(1);
    seq[0] = OUString("com.sun.star.xml.sax.FastParser");
    return seq;
}

bool FastSaxParser::hasNamespaceURL( const OUString& rPrefix ) const
{
    return mpImpl->hasNamespaceURL(rPrefix);
}

} // namespace sax_fastparser

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_extensions_xml_sax_FastParser_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FastSaxParser);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
