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

#include <sax/fastparser.hxx>
#include <sax/fastattribs.hxx>
#include <xml2utf.hxx>

#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/conditn.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <salhelper/thread.hxx>
#include <tools/diagnose_ex.h>

#include <queue>
#include <memory>
#include <stack>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <cstring>
#include <libxml/parser.h>
#include <cstdint>

// Inverse of libxml's BAD_CAST.
#define XML_CAST( str ) reinterpret_cast< const char* >( str )

using namespace std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;
using namespace com::sun::star;
using namespace sax_fastparser;

static void NormalizeURI( OUString& rName );

namespace {

struct Event;
class FastLocatorImpl;
struct NamespaceDefine;
struct Entity;

typedef std::unordered_map< OUString, sal_Int32 > NamespaceMap;

struct EventList
{
    std::vector<Event> maEvents;
    bool mbIsAttributesEmpty;
};

enum class CallbackType { START_ELEMENT, END_ELEMENT, CHARACTERS, PROCESSING_INSTRUCTION, DONE, EXCEPTION };

struct Event
{
    CallbackType maType;
    sal_Int32 mnElementToken;
    OUString msNamespace;
    OUString msElementName;
    rtl::Reference< FastAttributeList > mxAttributes;
    rtl::Reference< FastAttributeList > mxDeclAttributes;
    OUString msChars;
};

struct NameWithToken
{
    OUString msName;
    sal_Int32 mnToken;

    NameWithToken(const OUString& sName, sal_Int32 nToken) :
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
    css::uno::Reference< css::xml::sax::XFastDocumentHandler > mxDocumentHandler;
    rtl::Reference<FastTokenHandlerBase>                       mxTokenHandler;
    css::uno::Reference< css::xml::sax::XErrorHandler >        mxErrorHandler;
    css::uno::Reference< css::xml::sax::XFastNamespaceHandler >mxNamespaceHandler;

    ParserData();
};

struct NamespaceDefine
{
    OString     maPrefix;
    sal_Int32   mnToken;
    OUString    maNamespaceURL;

    NamespaceDefine( const OString& rPrefix, sal_Int32 nToken, const OUString& rNamespaceURL ) : maPrefix( rPrefix ), mnToken( nToken ), maNamespaceURL( rNamespaceURL ) {}
    NamespaceDefine() : mnToken(-1) {}
};

// Entity binds all information needed for a single file | single call of parseStream
struct Entity : public ParserData
{
    // Amount of work producer sends to consumer in one iteration:
    static const size_t mnEventListSize = 1000;

    // unique for each Entity instance:

    // Number of valid events in mxProducedEvents:
    size_t mnProducedEventsSize;
    std::unique_ptr<EventList> mxProducedEvents;
    std::queue<std::unique_ptr<EventList>> maPendingEvents;
    std::queue<std::unique_ptr<EventList>> maUsedEvents;
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
    css::xml::sax::InputSource              maStructSource;
    xmlParserCtxtPtr                        mpParser;
    ::sax_expatwrap::XMLFile2UTFConverter   maConverter;

    // Exceptions cannot be thrown through the C-XmlParser (possible
    // resource leaks), therefore any exception thrown by a UNO callback
    // must be saved somewhere until the C-XmlParser is stopped.
    css::uno::Any                           maSavedException;
    osl::Mutex maSavedExceptionMutex;
    void saveException( const Any & e );
    // Thread-safe check if maSavedException has value
    bool hasException();
    void throwException( const ::rtl::Reference< FastLocatorImpl > &xDocumentLocator,
                         bool mbDuringParse );

    std::stack< NameWithToken, std::vector<NameWithToken> > maNamespaceStack;
    /* Context for main thread consuming events.
     * startElement() stores the data, which characters() and endElement() uses
     */
    std::stack< SaxContext, std::vector<SaxContext> >  maContextStack;
    // Determines which elements of maNamespaceDefines are valid in current context
    std::stack< sal_uInt32, std::vector<sal_uInt32> >  maNamespaceCount;
    std::vector< NamespaceDefine >                     maNamespaceDefines;

    explicit Entity( const ParserData& rData );
    Entity( const Entity& rEntity ) = delete;
    Entity& operator=( const Entity& rEntity ) = delete;
    void startElement( Event const *pEvent );
    void characters( const OUString& sChars );
    void endElement();
    void processingInstruction( const OUString& rTarget, const OUString& rData );
    EventList& getEventList();
    Event& getEvent( CallbackType aType );
};

// Stuff for custom entity names
struct ReplacementPair
{
    OUString name;
    OUString replacement;
};
inline bool operator<(const ReplacementPair& lhs, const ReplacementPair& rhs)
{
    return lhs.name < rhs.name;
}
inline bool operator<(const ReplacementPair& lhs, const char* rhs)
{
    return lhs.name.compareToAscii(rhs) < 0;
}

} // namespace

namespace sax_fastparser {

class FastSaxParserImpl
{
public:
    explicit FastSaxParserImpl();
    ~FastSaxParserImpl();

private:
    std::vector<ReplacementPair> m_Replacements;
    std::vector<xmlEntityPtr> m_TemporalEntities;

public:
    // XFastParser
    /// @throws css::xml::sax::SAXException
    /// @throws css::io::IOException
    /// @throws css::uno::RuntimeException
    void parseStream( const css::xml::sax::InputSource& aInputSource );
    /// @throws css::uno::RuntimeException
    void setFastDocumentHandler( const css::uno::Reference< css::xml::sax::XFastDocumentHandler >& Handler );
    /// @throws css::uno::RuntimeException
    void setTokenHandler( const css::uno::Reference< css::xml::sax::XFastTokenHandler >& Handler );
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken );
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    OUString const & getNamespaceURL( std::u16string_view rPrefix );
    /// @throws css::uno::RuntimeException
    void setErrorHandler( const css::uno::Reference< css::xml::sax::XErrorHandler >& Handler );
    /// @throws css::uno::RuntimeException
    void setNamespaceHandler( const css::uno::Reference< css::xml::sax::XFastNamespaceHandler >& Handler);
    // Fake DTD file
    void setCustomEntityNames(
       const ::css::uno::Sequence<::css::beans::Pair<::rtl::OUString, ::rtl::OUString>>& replacements);

    // called by the C callbacks of the expat parser
    void callbackStartElement( const xmlChar *localName , const xmlChar* prefix, const xmlChar* URI,
        int numNamespaces, const xmlChar** namespaces, int numAttributes, const xmlChar **attributes );
    void callbackEndElement();
    void callbackCharacters( const xmlChar* s, int nLen );
    void callbackProcessingInstruction( const xmlChar *target, const xmlChar *data );
    xmlEntityPtr callbackGetEntity( const xmlChar *name );

    void pushEntity(const ParserData&, xml::sax::InputSource const&);
    void popEntity();
    Entity& getEntity()             { return *mpTop; }
    void parse();
    void produce( bool bForceFlush = false );
    bool m_bIgnoreMissingNSDecl;
    bool m_bDisableThreadedParser;

private:
    bool consume(EventList&);
    void deleteUsedEvents();
    void sendPendingCharacters();
    void addUnknownElementWithPrefix(const xmlChar **attributes, int i, rtl::Reference< FastAttributeList > const & xAttributes);

    sal_Int32 GetToken( const xmlChar* pName, sal_Int32 nameLen );
    /// @throws css::xml::sax::SAXException
    sal_Int32 GetTokenWithPrefix( const xmlChar* pPrefix, int prefixLen, const xmlChar* pName, int nameLen );
    /// @throws css::xml::sax::SAXException
    OUString const & GetNamespaceURL( std::string_view rPrefix );
    sal_Int32 GetNamespaceToken( const OUString& rNamespaceURL );
    sal_Int32 GetTokenWithContextNamespace( sal_Int32 nNamespaceToken, const xmlChar* pName, int nNameLen );
    void DefineNamespace( const OString& rPrefix, const OUString& namespaceURL );

private:
    osl::Mutex maMutex; ///< Protecting whole parseStream() execution
    ::rtl::Reference< FastLocatorImpl >     mxDocumentLocator;
    NamespaceMap                            maNamespaceMap;

    ParserData maData;                      /// Cached parser configuration for next call of parseStream().

    Entity *mpTop;                          /// std::stack::top() is amazingly slow => cache this.
    std::stack< Entity > maEntities;        /// Entity stack for each call of parseStream().
    std::vector<char> pendingCharacters;    /// Data from characters() callback that needs to be sent.
};

} // namespace sax_fastparser

namespace {

class ParserThread: public salhelper::Thread
{
    FastSaxParserImpl *mpParser;
public:
    explicit ParserThread(FastSaxParserImpl *pParser): Thread("Parser"), mpParser(pParser) {}
private:
    virtual void execute() override
    {
        try
        {
            mpParser->parse();
        }
        catch (...)
        {
            Entity &rEntity = mpParser->getEntity();
            rEntity.getEvent( CallbackType::EXCEPTION );
            mpParser->produce( true );
        }
    }
};

extern "C" {

static void call_callbackStartElement(void *userData, const xmlChar *localName , const xmlChar* prefix, const xmlChar* URI,
    int numNamespaces, const xmlChar** namespaces, int numAttributes, int /*defaultedAttributes*/, const xmlChar **attributes)
{
    FastSaxParserImpl* pFastParser = static_cast<FastSaxParserImpl*>( userData );
    pFastParser->callbackStartElement( localName, prefix, URI, numNamespaces, namespaces, numAttributes, attributes );
}

static void call_callbackEndElement(void *userData, const xmlChar* /*localName*/, const xmlChar* /*prefix*/, const xmlChar* /*URI*/)
{
    FastSaxParserImpl* pFastParser = static_cast<FastSaxParserImpl*>( userData );
    pFastParser->callbackEndElement();
}

static void call_callbackCharacters( void *userData , const xmlChar *s , int nLen )
{
    FastSaxParserImpl* pFastParser = static_cast<FastSaxParserImpl*>( userData );
    pFastParser->callbackCharacters( s, nLen );
}

static void call_callbackProcessingInstruction( void *userData, const xmlChar *target, const xmlChar *data )
{
    FastSaxParserImpl* pFastParser = static_cast<FastSaxParserImpl*>( userData );
    pFastParser->callbackProcessingInstruction( target, data );
}

static xmlEntityPtr call_callbackGetEntity( void *userData, const xmlChar *name)
{
    FastSaxParserImpl* pFastParser = static_cast<FastSaxParserImpl*>( userData );
    return pFastParser->callbackGetEntity( name );
}

}

class FastLocatorImpl : public WeakImplHelper< XLocator >
{
public:
    explicit FastLocatorImpl(FastSaxParserImpl *p) : mpParser(p) {}

    void dispose() { mpParser = nullptr; }
    /// @throws RuntimeException
    void checkDispose() const { if( !mpParser ) throw DisposedException(); }

    //XLocator
    virtual sal_Int32 SAL_CALL getColumnNumber() override;
    virtual sal_Int32 SAL_CALL getLineNumber() override;
    virtual OUString SAL_CALL getPublicId() override;
    virtual OUString SAL_CALL getSystemId() override;

private:
    FastSaxParserImpl *mpParser;
};

sal_Int32 SAL_CALL FastLocatorImpl::getColumnNumber()
{
    checkDispose();
    return xmlSAX2GetColumnNumber( mpParser->getEntity().mpParser );
}

sal_Int32 SAL_CALL FastLocatorImpl::getLineNumber()
{
    checkDispose();
    return xmlSAX2GetLineNumber( mpParser->getEntity().mpParser );
}

OUString SAL_CALL FastLocatorImpl::getPublicId()
{
    checkDispose();
    return mpParser->getEntity().maStructSource.sPublicId;
}

OUString SAL_CALL FastLocatorImpl::getSystemId()
{
    checkDispose();
    return mpParser->getEntity().maStructSource.sSystemId;
}

ParserData::ParserData()
{}

Entity::Entity(const ParserData& rData)
    : ParserData(rData)
    , mnProducedEventsSize(0)
    , mxProducedEvents()
    , mbEnableThreads(false)
    , mpParser(nullptr)
{
}

void Entity::startElement( Event const *pEvent )
{
    const sal_Int32& nElementToken = pEvent->mnElementToken;
    const OUString& aNamespace = pEvent->msNamespace;
    const OUString& aElementName = pEvent->msElementName;

    // Use un-wrapped pointers to avoid significant acquire/release overhead
    XFastContextHandler *pParentContext = nullptr;
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

        if ( mxNamespaceHandler.is() )
        {
            const Sequence< xml::Attribute > NSDeclAttribs = pEvent->mxDeclAttributes->getUnknownAttributes();
            for (const auto& rNSDeclAttrib : NSDeclAttribs)
            {
                mxNamespaceHandler->registerNamespace( rNSDeclAttrib.Name, rNSDeclAttrib.Value );
            }
        }

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
        maContextStack.top().mxContext = std::move( xContext );
    }
    catch (...)
    {
        saveException( ::cppu::getCaughtException() );
    }
}

void Entity::characters( const OUString& sChars )
{
    if (maContextStack.empty())
    {
        // Malformed XML stream !?
        return;
    }

    XFastContextHandler * pContext( maContextStack.top().mxContext.get() );
    if( pContext ) try
    {
        pContext->characters( sChars );
    }
    catch (...)
    {
        saveException( ::cppu::getCaughtException() );
    }
}

void Entity::endElement()
{
    if (maContextStack.empty())
    {
        // Malformed XML stream !?
        return;
    }

    const SaxContext& aContext = maContextStack.top();
    XFastContextHandler* pContext( aContext.mxContext.get() );
    if( pContext )
        try
        {
            sal_Int32 nElementToken = aContext.mnElementToken;
            if( nElementToken != FastToken::DONTKNOW )
                pContext->endFastElement( nElementToken );
            else
                pContext->endUnknownElement( aContext.maNamespace, aContext.maElementName );
        }
        catch (...)
        {
            saveException( ::cppu::getCaughtException() );
        }
    maContextStack.pop();
}

void Entity::processingInstruction( const OUString& rTarget, const OUString& rData )
{
    if( mxDocumentHandler.is() ) try
    {
        mxDocumentHandler->processingInstruction( rTarget, rData );
    }
    catch (...)
    {
        saveException( ::cppu::getCaughtException() );
    }
}

EventList& Entity::getEventList()
{
    if (!mxProducedEvents)
    {
        osl::ClearableMutexGuard aGuard(maEventProtector);
        if (!maUsedEvents.empty())
        {
            mxProducedEvents = std::move(maUsedEvents.front());
            maUsedEvents.pop();
            aGuard.clear(); // unlock
            mnProducedEventsSize = 0;
        }
        if (!mxProducedEvents)
        {
            mxProducedEvents.reset(new EventList);
            mxProducedEvents->maEvents.resize(mnEventListSize);
            mxProducedEvents->mbIsAttributesEmpty = false;
            mnProducedEventsSize = 0;
        }
    }
    return *mxProducedEvents;
}

Event& Entity::getEvent( CallbackType aType )
{
    if (!mbEnableThreads)
        return maSharedEvent;

    EventList& rEventList = getEventList();
    if (mnProducedEventsSize == rEventList.maEvents.size())
    {
        SAL_WARN_IF(!maSavedException.hasValue(), "sax",
            "Event vector should only exceed " << mnEventListSize <<
            " temporarily while an exception is pending");
        rEventList.maEvents.resize(mnProducedEventsSize + 1);
    }
    Event& rEvent = rEventList.maEvents[mnProducedEventsSize++];
    rEvent.maType = aType;
    return rEvent;
}

OUString lclGetErrorMessage( xmlParserCtxtPtr ctxt, std::u16string_view sSystemId, sal_Int32 nLine )
{
    const char* pMessage;
    xmlErrorPtr error = xmlCtxtGetLastError( ctxt );
    if( error && error->message )
        pMessage = error->message;
    else
        pMessage = "unknown error";
    OUStringBuffer aBuffer( 128 );
    aBuffer.append( "[" );
    aBuffer.append( sSystemId );
    aBuffer.append( " line " );
    aBuffer.append( nLine );
    aBuffer.append( "]: " );
    aBuffer.appendAscii( pMessage );
    return aBuffer.makeStringAndClear();
}

// throw an exception, but avoid callback if
// during a threaded produce
void Entity::throwException( const ::rtl::Reference< FastLocatorImpl > &xDocumentLocator,
                             bool mbDuringParse )
{
    // Error during parsing !
    Any savedException;
    {
        osl::MutexGuard g(maSavedExceptionMutex);
        if (maSavedException.hasValue())
        {
            savedException.setValue(&maSavedException, cppu::UnoType<decltype(maSavedException)>::get());
        }
    }
    SAXParseException aExcept(
        lclGetErrorMessage( mpParser,
                            xDocumentLocator->getSystemId(),
                            xDocumentLocator->getLineNumber() ),
        Reference< XInterface >(),
        savedException,
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
void Entity::saveException( const Any & e )
{
    // fdo#81214 - allow the parser to run on after an exception,
    // unexpectedly some 'startElements' produce a UNO_QUERY_THROW
    // for XComponent; and yet expect to continue parsing.
    SAL_WARN("sax", "Unexpected exception from XML parser " << exceptionToString(e));
    osl::MutexGuard g(maSavedExceptionMutex);
    if (maSavedException.hasValue())
    {
        SAL_INFO("sax.fastparser", "discarding exception, already have one");
    }
    else
    {
        maSavedException = e;
    }
}

bool Entity::hasException()
{
    osl::MutexGuard g(maSavedExceptionMutex);
    return maSavedException.hasValue();
}

} // namespace

namespace sax_fastparser {

FastSaxParserImpl::FastSaxParserImpl() :
    m_bIgnoreMissingNSDecl(false),
    m_bDisableThreadedParser(false),
    mpTop(nullptr)
{
    mxDocumentLocator.set( new FastLocatorImpl( this ) );
}

FastSaxParserImpl::~FastSaxParserImpl()
{
    if( mxDocumentLocator.is() )
        mxDocumentLocator->dispose();
    for ( size_t i = 0; i < m_TemporalEntities.size(); ++i )
    {
        if (!m_TemporalEntities[i])
            continue;
        xmlNodePtr pPtr = reinterpret_cast<xmlNodePtr>(m_TemporalEntities[i]);
        xmlUnlinkNode(pPtr);
        xmlFreeNode(pPtr);
    }
}

void FastSaxParserImpl::DefineNamespace( const OString& rPrefix, const OUString& namespaceURL )
{
    Entity& rEntity = getEntity();
    assert(!rEntity.maNamespaceCount.empty()); // need a context!

    sal_uInt32 nOffset = rEntity.maNamespaceCount.top()++;
    if( rEntity.maNamespaceDefines.size() <= nOffset )
        rEntity.maNamespaceDefines.resize( rEntity.maNamespaceDefines.size() + 64 );

    rEntity.maNamespaceDefines[nOffset] = NamespaceDefine( rPrefix, GetNamespaceToken( namespaceURL ), namespaceURL );
}

sal_Int32 FastSaxParserImpl::GetToken( const xmlChar* pName, sal_Int32 nameLen /* = 0 */ )
{
    return FastTokenHandlerBase::getTokenFromChars( getEntity(). mxTokenHandler.get(),
                                                    XML_CAST( pName ), nameLen ); // uses utf-8
}

sal_Int32 FastSaxParserImpl::GetTokenWithPrefix( const xmlChar* pPrefix, int nPrefixLen, const xmlChar* pName, int nNameLen )
{
    sal_Int32 nNamespaceToken = FastToken::DONTKNOW;

    Entity& rEntity = getEntity();
    if (rEntity.maNamespaceCount.empty())
        return nNamespaceToken;

    sal_uInt32 nNamespace = rEntity.maNamespaceCount.top();
    while( nNamespace-- )
    {
        const auto & rNamespaceDefine = rEntity.maNamespaceDefines[nNamespace];
        const OString& rPrefix( rNamespaceDefine.maPrefix );
        if( (rPrefix.getLength() == nPrefixLen) &&
            rtl_str_reverseCompare_WithLength(rPrefix.pData->buffer, rPrefix.pData->length, XML_CAST( pPrefix ), nPrefixLen ) == 0 )
        {
            nNamespaceToken = rNamespaceDefine.mnToken;
            break;
        }

        if( !nNamespace && !m_bIgnoreMissingNSDecl )
            throw SAXException("No namespace defined for " + OUString(XML_CAST(pPrefix),
                    nPrefixLen, RTL_TEXTENCODING_UTF8), Reference< XInterface >(), Any());
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

OUString const & FastSaxParserImpl::GetNamespaceURL( std::string_view rPrefix )
{
    Entity& rEntity = getEntity();
    if( !rEntity.maNamespaceCount.empty() )
    {
        sal_uInt32 nNamespace = rEntity.maNamespaceCount.top();
        while( nNamespace-- )
            if( rEntity.maNamespaceDefines[nNamespace].maPrefix == rPrefix )
                return rEntity.maNamespaceDefines[nNamespace].maNamespaceURL;
    }

    throw SAXException("No namespace defined for " + OUString::fromUtf8(rPrefix),
            Reference< XInterface >(), Any());
}

sal_Int32 FastSaxParserImpl::GetTokenWithContextNamespace( sal_Int32 nNamespaceToken, const xmlChar* pName, int nNameLen )
{
    if( nNamespaceToken != FastToken::DONTKNOW )
    {
        sal_Int32 nNameToken = GetToken( pName, nNameLen );
        if( nNameToken != FastToken::DONTKNOW )
            return nNamespaceToken | nNameToken;
    }

    return FastToken::DONTKNOW;
}

namespace
{
    class ParserCleanup
    {
    private:
        FastSaxParserImpl& m_rParser;
        Entity& m_rEntity;
        rtl::Reference<ParserThread> m_xParser;
    public:
        ParserCleanup(FastSaxParserImpl& rParser, Entity& rEntity)
            : m_rParser(rParser)
            , m_rEntity(rEntity)
        {
        }
        ~ParserCleanup()
        {
            if (m_rEntity.mpParser)
            {
                if (m_rEntity.mpParser->myDoc)
                    xmlFreeDoc(m_rEntity.mpParser->myDoc);
                xmlFreeParserCtxt(m_rEntity.mpParser);
            }
            joinThread();
            m_rParser.popEntity();
        }
        void setThread(const rtl::Reference<ParserThread> &xParser)
        {
            m_xParser = xParser;
        }
        void joinThread()
        {
            if (m_xParser.is())
            {
                rtl::Reference<ParserThread> xToJoin = m_xParser;
                m_xParser.clear();
                xToJoin->join();
            }
        }
    };
}
/***************
*
* parseStream does Parser-startup initializations. The FastSaxParser::parse() method does
* the file-specific initialization work. (During a parser run, external files may be opened)
*
****************/
void FastSaxParserImpl::parseStream(const InputSource& rStructSource)
{
    xmlInitParser();

    // Only one text at one time
    MutexGuard guard( maMutex );

    pushEntity(maData, rStructSource);
    Entity& rEntity = getEntity();
    ParserCleanup aEnsureFree(*this, rEntity);

    // start the document
    if( rEntity.mxDocumentHandler.is() )
    {
        Reference< XLocator > xLoc( mxDocumentLocator.get() );
        rEntity.mxDocumentHandler->setDocumentLocator( xLoc );
        rEntity.mxDocumentHandler->startDocument();
    }

    if (!getenv("SAX_DISABLE_THREADS") && !m_bDisableThreadedParser)
    {
        Reference<css::io::XSeekable> xSeekable(rEntity.maStructSource.aInputStream, UNO_QUERY);
        // available() is not __really__ relevant here, but leave it in as a heuristic for non-seekable streams
        rEntity.mbEnableThreads = (xSeekable.is() && xSeekable->getLength() > 10000)
                || (rEntity.maStructSource.aInputStream->available() > 10000);
    }

    if (rEntity.mbEnableThreads)
    {
        rtl::Reference<ParserThread> xParser = new ParserThread(this);
        xParser->launch();
        aEnsureFree.setThread(xParser);
        bool done = false;
        do {
            rEntity.maConsumeResume.wait();
            rEntity.maConsumeResume.reset();

            osl::ResettableMutexGuard aGuard(rEntity.maEventProtector);
            while (!rEntity.maPendingEvents.empty())
            {
                if (rEntity.maPendingEvents.size() <= Entity::mnEventLowWater)
                    rEntity.maProduceResume.set(); // start producer again

                std::unique_ptr<EventList> xEventList = std::move(rEntity.maPendingEvents.front());
                rEntity.maPendingEvents.pop();
                aGuard.clear(); // unlock

                if (!consume(*xEventList))
                    done = true;

                aGuard.reset(); // lock

                if ( rEntity.maPendingEvents.size() <= Entity::mnEventLowWater )
                {
                    aGuard.clear();
                    for (auto& rEvent : xEventList->maEvents)
                    {
                        if (rEvent.mxAttributes.is())
                        {
                            rEvent.mxAttributes->clear();
                            if( rEntity.mxNamespaceHandler.is() )
                                rEvent.mxDeclAttributes->clear();
                        }
                        xEventList->mbIsAttributesEmpty = true;
                    }
                    aGuard.reset();
                }

                rEntity.maUsedEvents.push(std::move(xEventList));
            }
        } while (!done);
        aEnsureFree.joinThread();
        deleteUsedEvents();

        // callbacks used inside XML_Parse may have caught an exception
        // No need to lock maSavedExceptionMutex here because parser
        // thread is joined.
        if( rEntity.maSavedException.hasValue() )
            rEntity.throwException( mxDocumentLocator, true );
    }
    else
    {
        parse();
    }

    // finish document
    if( rEntity.mxDocumentHandler.is() )
    {
        rEntity.mxDocumentHandler->endDocument();
    }
}

void FastSaxParserImpl::setFastDocumentHandler( const Reference< XFastDocumentHandler >& Handler )
{
    maData.mxDocumentHandler = Handler;
}

void FastSaxParserImpl::setTokenHandler( const Reference< XFastTokenHandler >& xHandler )
{
    assert( dynamic_cast< FastTokenHandlerBase *>( xHandler.get() ) && "we expect this handler to be a subclass of FastTokenHandlerBase" );
    maData.mxTokenHandler = dynamic_cast< FastTokenHandlerBase *>( xHandler.get() );
}

void FastSaxParserImpl::registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken )
{
    if( NamespaceToken < FastToken::NAMESPACE )
        throw IllegalArgumentException("Invalid namespace token " + OUString::number(NamespaceToken), css::uno::Reference<css::uno::XInterface >(), 0);

    if( GetNamespaceToken( NamespaceURL ) == FastToken::DONTKNOW )
    {
        maNamespaceMap[ NamespaceURL ] = NamespaceToken;
        return;
    }
    throw IllegalArgumentException("namespace URL is already registered: " + NamespaceURL, css::uno::Reference<css::uno::XInterface >(), 0);
}

OUString const & FastSaxParserImpl::getNamespaceURL( std::u16string_view rPrefix )
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

void FastSaxParserImpl::setErrorHandler(const Reference< XErrorHandler > & Handler)
{
    maData.mxErrorHandler = Handler;
}

void FastSaxParserImpl::setNamespaceHandler( const Reference< XFastNamespaceHandler >& Handler )
{
    maData.mxNamespaceHandler = Handler;
}

void FastSaxParserImpl::setCustomEntityNames(
    const ::css::uno::Sequence<::css::beans::Pair<::rtl::OUString, ::rtl::OUString>>& replacements)
{
    m_Replacements.resize(replacements.size());
    for (size_t i = 0; i < replacements.size(); ++i)
    {
        m_Replacements[i].name = replacements[i].First;
        m_Replacements[i].replacement = replacements[i].Second;
    }
    if (m_Replacements.size() > 1)
        std::sort(m_Replacements.begin(), m_Replacements.end());
}

void FastSaxParserImpl::deleteUsedEvents()
{
    Entity& rEntity = getEntity();
    osl::ResettableMutexGuard aGuard(rEntity.maEventProtector);

    while (!rEntity.maUsedEvents.empty())
    {
        std::unique_ptr<EventList> xEventList = std::move(rEntity.maUsedEvents.front());
        rEntity.maUsedEvents.pop();

        aGuard.clear(); // unlock

        xEventList.reset();

        aGuard.reset(); // lock
    }
}

void FastSaxParserImpl::produce( bool bForceFlush )
{
    Entity& rEntity = getEntity();
    if (!(bForceFlush ||
        rEntity.mnProducedEventsSize >= Entity::mnEventListSize))
        return;

    osl::ResettableMutexGuard aGuard(rEntity.maEventProtector);

    while (rEntity.maPendingEvents.size() >= Entity::mnEventHighWater)
    { // pause parsing for a bit
        aGuard.clear(); // unlock
        rEntity.maProduceResume.wait();
        rEntity.maProduceResume.reset();
        aGuard.reset(); // lock
    }

    rEntity.maPendingEvents.push(std::move(rEntity.mxProducedEvents));
    assert(!rEntity.mxProducedEvents);

    aGuard.clear(); // unlock

    rEntity.maConsumeResume.set();
}

bool FastSaxParserImpl::consume(EventList& rEventList)
{
    Entity& rEntity = getEntity();
    rEventList.mbIsAttributesEmpty = false;
    for (auto& rEvent : rEventList.maEvents)
    {
        switch (rEvent.maType)
        {
            case CallbackType::START_ELEMENT:
                rEntity.startElement( &rEvent );
                break;
            case CallbackType::END_ELEMENT:
                rEntity.endElement();
                break;
            case CallbackType::CHARACTERS:
                rEntity.characters( rEvent.msChars );
                break;
            case CallbackType::PROCESSING_INSTRUCTION:
                rEntity.processingInstruction(
                    rEvent.msNamespace, rEvent.msElementName ); // ( target, data )
                break;
            case CallbackType::DONE:
                return false;
            case CallbackType::EXCEPTION:
                rEntity.throwException( mxDocumentLocator, false );
                [[fallthrough]]; // avoid unreachable code warning with some compilers
            default:
                assert(false);
                return false;
        }
    }
    return true;
}

void FastSaxParserImpl::pushEntity(const ParserData& rEntityData,
        xml::sax::InputSource const& rSource)
{
    if (!rSource.aInputStream.is())
        throw SAXException("No input source", Reference<XInterface>(), Any());

    maEntities.emplace(rEntityData);
    mpTop = &maEntities.top();

    mpTop->maStructSource = rSource;

    mpTop->maConverter.setInputStream(mpTop->maStructSource.aInputStream);
    if (!mpTop->maStructSource.sEncoding.isEmpty())
    {
        mpTop->maConverter.setEncoding(OUStringToOString(mpTop->maStructSource.sEncoding, RTL_TEXTENCODING_ASCII_US));
    }
}

void FastSaxParserImpl::popEntity()
{
    maEntities.pop();
    mpTop = !maEntities.empty() ? &maEntities.top() : nullptr;
}

// starts parsing with actual parser !
void FastSaxParserImpl::parse()
{
    const int BUFFER_SIZE = 16 * 1024;
    Sequence< sal_Int8 > seqOut( BUFFER_SIZE );

    Entity& rEntity = getEntity();

    // set all necessary C-Callbacks
    static xmlSAXHandler callbacks;
    callbacks.startElementNs = call_callbackStartElement;
    callbacks.endElementNs = call_callbackEndElement;
    callbacks.characters = call_callbackCharacters;
    callbacks.processingInstruction = call_callbackProcessingInstruction;
    callbacks.getEntity = call_callbackGetEntity;
    callbacks.initialized = XML_SAX2_MAGIC;
    int nRead = 0;
    do
    {
        nRead = rEntity.maConverter.readAndConvert( seqOut, BUFFER_SIZE );
        if( nRead <= 0 )
        {
            if( rEntity.mpParser != nullptr )
            {
                if( xmlParseChunk( rEntity.mpParser, reinterpret_cast<const char*>(seqOut.getConstArray()), 0, 1 ) != XML_ERR_OK )
                    rEntity.throwException( mxDocumentLocator, true );
                if (rEntity.hasException())
                    rEntity.throwException(mxDocumentLocator, true);
            }
            break;
        }

        bool bContinue = true;
        if( rEntity.mpParser == nullptr )
        {
            // create parser with proper encoding (needs the first chunk of data)
            rEntity.mpParser = xmlCreatePushParserCtxt( &callbacks, this,
                reinterpret_cast<const char*>(seqOut.getConstArray()), nRead, nullptr );
            if( !rEntity.mpParser )
                throw SAXException("Couldn't create parser", Reference< XInterface >(), Any() );

            // Tell libxml2 parser to decode entities in attribute values.
            // Also allow XML attribute values which are larger than 10MB, because this used to work
            // with expat.
            // coverity[unsafe_xml_parse_config] - entity support is required
            xmlCtxtUseOptions(rEntity.mpParser, XML_PARSE_NOENT | XML_PARSE_HUGE);
        }
        else
        {
            bContinue = xmlParseChunk( rEntity.mpParser, reinterpret_cast<const char*>(seqOut.getConstArray()), nRead, 0 )
                            == XML_ERR_OK;
        }

        // callbacks used inside XML_Parse may have caught an exception
        if (!bContinue)
        {
            rEntity.throwException( mxDocumentLocator, true );
        }
        if (rEntity.hasException())
        {
            rEntity.throwException( mxDocumentLocator, true );
        }
    } while( nRead > 0 );
    rEntity.getEvent( CallbackType::DONE );
    if( rEntity.mbEnableThreads )
        produce( true );
}

// The C-Callbacks
void FastSaxParserImpl::callbackStartElement(const xmlChar *localName , const xmlChar* prefix, const xmlChar* URI,
    int numNamespaces, const xmlChar** namespaces, int numAttributes, const xmlChar **attributes)
{
    if (!pendingCharacters.empty())
        sendPendingCharacters();
    Entity& rEntity = getEntity();
    if( rEntity.maNamespaceCount.empty() )
    {
        rEntity.maNamespaceCount.push(0);
        DefineNamespace( "xml", "http://www.w3.org/XML/1998/namespace");
    }
    else
    {
        rEntity.maNamespaceCount.push( rEntity.maNamespaceCount.top() );
    }

    // create attribute map and process namespace instructions
    Event& rEvent = rEntity.getEvent( CallbackType::START_ELEMENT );
    bool bIsAttributesEmpty = false;
    if ( rEntity.mbEnableThreads )
        bIsAttributesEmpty = rEntity.getEventList().mbIsAttributesEmpty;

    if (rEvent.mxAttributes.is())
    {
        if( !bIsAttributesEmpty )
            rEvent.mxAttributes->clear();
    }
    else
        rEvent.mxAttributes.set(
                new FastAttributeList( rEntity.mxTokenHandler.get() ) );

    if( rEntity.mxNamespaceHandler.is() )
    {
        if (rEvent.mxDeclAttributes.is())
        {
            if( !bIsAttributesEmpty )
                rEvent.mxDeclAttributes->clear();
        }
        else
            rEvent.mxDeclAttributes.set(
                new FastAttributeList( rEntity.mxTokenHandler.get() ) );
    }

    OUString sNamespace;
    sal_Int32 nNamespaceToken = FastToken::DONTKNOW;
    if (!rEntity.maNamespaceStack.empty())
    {
        sNamespace = rEntity.maNamespaceStack.top().msName;
        nNamespaceToken = rEntity.maNamespaceStack.top().mnToken;
    }

    try
    {
        /*  #158414# Each element may define new namespaces, also for attributes.
            First, process all namespaces, second, process the attributes after namespaces
            have been initialized. */

        // #158414# first: get namespaces
        for (int i = 0; i < numNamespaces * 2; i += 2)
        {
            // namespaces[] is (prefix/URI)
            if( namespaces[ i ] != nullptr )
            {
                OString aPrefix( XML_CAST( namespaces[ i ] ));
                OUString namespaceURL( XML_CAST( namespaces[ i + 1 ] ), strlen( XML_CAST( namespaces[ i + 1 ] )), RTL_TEXTENCODING_UTF8 );
                NormalizeURI( namespaceURL );
                DefineNamespace(aPrefix, namespaceURL);
                if( rEntity.mxNamespaceHandler.is() )
                    rEvent.mxDeclAttributes->addUnknown( OString( XML_CAST( namespaces[ i ] ) ), OString( XML_CAST( namespaces[ i + 1 ] ) ) );
            }
            else
            {
                // default namespace
                sNamespace = OUString( XML_CAST( namespaces[ i + 1 ] ), strlen( XML_CAST( namespaces[ i + 1 ] )), RTL_TEXTENCODING_UTF8 );
                NormalizeURI( sNamespace );
                nNamespaceToken = GetNamespaceToken( sNamespace );
                if( rEntity.mxNamespaceHandler.is() )
                    rEvent.mxDeclAttributes->addUnknown( "", OString( XML_CAST( namespaces[ i + 1 ] ) ) );
            }
        }

        if ( rEntity.mxTokenHandler.is() )
        {
            // #158414# second: fill attribute list with other attributes
            rEvent.mxAttributes->reserve( numAttributes );
            for (int i = 0; i < numAttributes * 5; i += 5)
            {
                // attributes[] is ( localname / prefix / nsURI / valueBegin / valueEnd )
                if( attributes[ i + 1 ] != nullptr )
                {
                    sal_Int32 nAttributeToken = GetTokenWithPrefix( attributes[ i + 1 ], strlen( XML_CAST( attributes[ i + 1 ] )), attributes[ i ], strlen( XML_CAST( attributes[ i ] )));
                    if( nAttributeToken != FastToken::DONTKNOW )
                        rEvent.mxAttributes->add( nAttributeToken, XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] );
                    else
                        addUnknownElementWithPrefix(attributes, i, rEvent.mxAttributes);
                }
                else
                {
                    sal_Int32 nAttributeToken = GetToken( attributes[ i ], strlen( XML_CAST( attributes[ i ] )));
                    if( nAttributeToken != FastToken::DONTKNOW )
                        rEvent.mxAttributes->add( nAttributeToken, XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] );
                    else
                    {
                        SAL_WARN("xmloff", "unknown attribute " << XML_CAST( attributes[ i ] ) << "=" <<
                            OString( XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] ));
                        rEvent.mxAttributes->addUnknown( XML_CAST( attributes[ i ] ),
                            OString( XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] ));
                    }
                }
            }

            if( prefix != nullptr )
                rEvent.mnElementToken = GetTokenWithPrefix( prefix, strlen( XML_CAST( prefix )), localName, strlen( XML_CAST( localName )));
            else if( !sNamespace.isEmpty() )
                rEvent.mnElementToken = GetTokenWithContextNamespace( nNamespaceToken, localName, strlen( XML_CAST( localName )));
            else
                rEvent.mnElementToken = GetToken( localName, strlen( XML_CAST( localName )));
        }
        else
        {
            for (int i = 0; i < numAttributes * 5; i += 5)
            {
                if( attributes[ i + 1 ] != nullptr )
                    addUnknownElementWithPrefix(attributes, i, rEvent.mxAttributes);
                else
                    rEvent.mxAttributes->addUnknown( XML_CAST( attributes[ i ] ),
                            OString( XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] ));
            }

            rEvent.mnElementToken = FastToken::DONTKNOW;
        }

        if( rEvent.mnElementToken == FastToken::DONTKNOW )
        {
            OUString aElementPrefix;
            if( prefix != nullptr )
            {
                if ( !m_bIgnoreMissingNSDecl || URI != nullptr )
                    sNamespace = OUString( XML_CAST( URI ), strlen( XML_CAST( URI )), RTL_TEXTENCODING_UTF8 );
                else
                    sNamespace.clear();
                nNamespaceToken = GetNamespaceToken( sNamespace );
                aElementPrefix = OUString( XML_CAST( prefix ), strlen( XML_CAST( prefix )), RTL_TEXTENCODING_UTF8 );
            }
            OUString aElementLocalName( XML_CAST( localName ), strlen( XML_CAST( localName )), RTL_TEXTENCODING_UTF8 );
            rEvent.msNamespace = sNamespace;
            if( aElementPrefix.isEmpty() )
                rEvent.msElementName = std::move(aElementLocalName);
            else
                rEvent.msElementName = aElementPrefix + ":" + aElementLocalName;
        }
        else // token is always preferred.
            rEvent.msElementName.clear();

        rEntity.maNamespaceStack.push( NameWithToken(sNamespace, nNamespaceToken) );
        if (rEntity.mbEnableThreads)
            produce();
        else
        {
            SAL_INFO("sax.fastparser", " startElement line " << mxDocumentLocator->getLineNumber() << " column " << mxDocumentLocator->getColumnNumber() << " " << ( prefix ? XML_CAST(prefix) : "(null)" ) << ":" << localName);
            rEntity.startElement( &rEvent );
        }
    }
    catch (...)
    {
        rEntity.saveException( ::cppu::getCaughtException() );
    }
}

void FastSaxParserImpl::addUnknownElementWithPrefix(const xmlChar **attributes, int i, rtl::Reference< FastAttributeList > const & xAttributes)
{
    OUString aNamespaceURI;
    if ( !m_bIgnoreMissingNSDecl || attributes[i + 2] != nullptr )
        aNamespaceURI = OUString( XML_CAST( attributes[ i + 2 ] ), strlen( XML_CAST( attributes[ i + 2 ] )), RTL_TEXTENCODING_UTF8 );
    const OString& rPrefix = OString( XML_CAST( attributes[ i + 1 ] ));
    const OString& rLocalName = OString( XML_CAST( attributes[ i ] ));
    OString aQualifiedName = (rPrefix.isEmpty())? rLocalName : rPrefix + ":" + rLocalName;
    xAttributes->addUnknown( aNamespaceURI, aQualifiedName,
        OString( XML_CAST( attributes[ i + 3 ] ), attributes[ i + 4 ] - attributes[ i + 3 ] ));
    SAL_WARN("xmloff", "unknown element " << aQualifiedName << " " << aNamespaceURI);
}

void FastSaxParserImpl::callbackEndElement()
{
    if (!pendingCharacters.empty())
        sendPendingCharacters();
    Entity& rEntity = getEntity();
    SAL_WARN_IF(rEntity.maNamespaceCount.empty(), "sax", "Empty NamespaceCount");
    if( !rEntity.maNamespaceCount.empty() )
        rEntity.maNamespaceCount.pop();

    SAL_WARN_IF(rEntity.maNamespaceStack.empty(), "sax", "Empty NamespaceStack");
    if( !rEntity.maNamespaceStack.empty() )
        rEntity.maNamespaceStack.pop();

    rEntity.getEvent( CallbackType::END_ELEMENT );
    if (rEntity.mbEnableThreads)
        produce();
    else
        rEntity.endElement();
}

void FastSaxParserImpl::callbackCharacters( const xmlChar* s, int nLen )
{
    // SAX interface allows that the characters callback splits content of one XML node
    // (e.g. because there's an entity that needs decoding), however for consumers it's
    // simpler FastSaxParser's character callback provides the whole string at once,
    // so merge data from possible multiple calls and send them at once (before the element
    // ends or another one starts).
    //
    // We use a std::vector<char> to avoid calling into the OUString constructor more than once when
    // we have multiple callbackCharacters() calls that we have to merge, which happens surprisingly
    // often in writer documents.
    int nOriginalLen = pendingCharacters.size();
    pendingCharacters.resize(nOriginalLen + nLen);
    memcpy(pendingCharacters.data() + nOriginalLen, s, nLen);
}

void FastSaxParserImpl::sendPendingCharacters()
{
    Entity& rEntity = getEntity();
    OUString sChars( pendingCharacters.data(), pendingCharacters.size(), RTL_TEXTENCODING_UTF8 );
    if (rEntity.mbEnableThreads)
    {
        Event& rEvent = rEntity.getEvent( CallbackType::CHARACTERS );
        rEvent.msChars = std::move(sChars);
        produce();
    }
    else
        rEntity.characters( sChars );
    pendingCharacters.resize(0);
}

void FastSaxParserImpl::callbackProcessingInstruction( const xmlChar *target, const xmlChar *data )
{
    if (!pendingCharacters.empty())
        sendPendingCharacters();
    Entity& rEntity = getEntity();
    Event& rEvent = rEntity.getEvent( CallbackType::PROCESSING_INSTRUCTION );

    // This event is very rare, so no need to waste extra space for this
    // Using namespace and element strings to be target and data in that order.
    rEvent.msNamespace = OUString( XML_CAST( target ), strlen( XML_CAST( target ) ), RTL_TEXTENCODING_UTF8 );
    if ( data != nullptr )
        rEvent.msElementName = OUString( XML_CAST( data ), strlen( XML_CAST( data ) ), RTL_TEXTENCODING_UTF8 );
    else
        rEvent.msElementName.clear();

    if (rEntity.mbEnableThreads)
        produce();
    else
        rEntity.processingInstruction( rEvent.msNamespace, rEvent.msElementName );
}

xmlEntityPtr FastSaxParserImpl::callbackGetEntity( const xmlChar *name )
{
    if( !name )
        return xmlGetPredefinedEntity(name);
    const char* dname = XML_CAST(name);
    int lname = strlen(dname);
    if( lname == 0 )
        return xmlGetPredefinedEntity(name);
    if (m_Replacements.size() > 0)
    {
        auto it = std::lower_bound(m_Replacements.begin(), m_Replacements.end(), dname);
        if (it != m_Replacements.end() && it->name.compareToAscii(dname) == 0)
        {
            xmlEntityPtr entpt = xmlNewEntity(
                nullptr, name, XML_INTERNAL_GENERAL_ENTITY, nullptr, nullptr,
                BAD_CAST(OUStringToOString(it->replacement, RTL_TEXTENCODING_UTF8).getStr()));
            m_TemporalEntities.push_back(entpt);
            return entpt;
        }
    }
    if( lname < 2 )
        return xmlGetPredefinedEntity(name);
    if ( dname[0] == '#' )
    {
        sal_uInt32 cval = 0;
        if( dname[1] == 'x' ||  dname[1] == 'X' )
        {
            if( lname < 3 )
                return xmlGetPredefinedEntity(name);
            cval = static_cast<sal_uInt32>( strtoul( dname + 2, nullptr, 16 ) );
            if( cval == 0 )
                return xmlGetPredefinedEntity(name);
            OUString vname( &cval, 1 );
            xmlEntityPtr entpt
                = xmlNewEntity(nullptr, name, XML_INTERNAL_GENERAL_ENTITY, nullptr, nullptr,
                               BAD_CAST(OUStringToOString(vname, RTL_TEXTENCODING_UTF8).getStr()));
            m_TemporalEntities.push_back(entpt);
            return entpt;
        }
        else
        {
            cval = static_cast<sal_uInt32>( strtoul( dname + 2, nullptr, 10 ) );
            if( cval == 0 )
                return xmlGetPredefinedEntity(name);
            OUString vname(&cval, 1);
            xmlEntityPtr entpt
                = xmlNewEntity(nullptr, name, XML_INTERNAL_GENERAL_ENTITY, nullptr, nullptr,
                               BAD_CAST(OUStringToOString(vname, RTL_TEXTENCODING_UTF8).getStr()));
            m_TemporalEntities.push_back(entpt);
            return entpt;
        }
    }
    return xmlGetPredefinedEntity(name);
}

FastSaxParser::FastSaxParser() : mpImpl(new FastSaxParserImpl) {}

FastSaxParser::~FastSaxParser()
{
}

void SAL_CALL
FastSaxParser::initialize(css::uno::Sequence< css::uno::Any > const& rArguments)
{
    if (!rArguments.hasElements())
        return;

    OUString str;
    if ( !(rArguments[0] >>= str) )
        throw IllegalArgumentException();

    if ( str == "IgnoreMissingNSDecl" )
        mpImpl->m_bIgnoreMissingNSDecl = true;
    else if ( str == "DoSmeplease" )
        ; //just ignore as this is already immune to billion laughs
    else if ( str == "DisableThreadedParser" )
        mpImpl->m_bDisableThreadedParser = true;
    else
        throw IllegalArgumentException();

}

void FastSaxParser::parseStream( const xml::sax::InputSource& aInputSource )
{
    mpImpl->parseStream(aInputSource);
}

void FastSaxParser::setFastDocumentHandler( const uno::Reference<xml::sax::XFastDocumentHandler>& Handler )
{
    mpImpl->setFastDocumentHandler(Handler);
}

void FastSaxParser::setTokenHandler( const uno::Reference<xml::sax::XFastTokenHandler>& Handler )
{
    mpImpl->setTokenHandler(Handler);
}

void FastSaxParser::registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken )
{
    mpImpl->registerNamespace(NamespaceURL, NamespaceToken);
}

OUString FastSaxParser::getNamespaceURL( const OUString& rPrefix )
{
    return mpImpl->getNamespaceURL(rPrefix);
}

void FastSaxParser::setErrorHandler( const uno::Reference< xml::sax::XErrorHandler >& Handler )
{
    mpImpl->setErrorHandler(Handler);
}

void FastSaxParser::setEntityResolver( const uno::Reference< xml::sax::XEntityResolver >& )
{
    // not implemented
}

void FastSaxParser::setLocale( const lang::Locale& )
{
    // not implemented
}

void FastSaxParser::setNamespaceHandler( const uno::Reference< css::xml::sax::XFastNamespaceHandler >& Handler)
{
    mpImpl->setNamespaceHandler(Handler);
}

OUString FastSaxParser::getImplementationName()
{
    return "com.sun.star.comp.extensions.xml.sax.FastParser";
}

void FastSaxParser::setCustomEntityNames(
    const ::css::uno::Sequence<::css::beans::Pair<::rtl::OUString, ::rtl::OUString>>& replacements)
{
    mpImpl->setCustomEntityNames(replacements);
}

sal_Bool FastSaxParser::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> FastSaxParser::getSupportedServiceNames()
{
    return { "com.sun.star.xml.sax.FastParser" };
}

} // namespace sax_fastparser

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_extensions_xml_sax_FastParser_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FastSaxParser);
}

// ----------------------------------------------------------
// copy of the code in xmloff/source/core/namespace.cxx, which adds namespace aliases
// for various dodgy namespace decls in the wild.

static bool NormalizeW3URI( OUString& rName );
static bool NormalizeOasisURN( OUString& rName );

static void NormalizeURI( OUString& rName )
{
    // try OASIS + W3 URI normalization
    bool bSuccess = NormalizeOasisURN( rName );
    if( ! bSuccess )
        bSuccess = NormalizeW3URI( rName );
}

constexpr OUStringLiteral XML_URI_W3_PREFIX(u"http://www.w3.org/");
constexpr OUStringLiteral XML_URI_XFORMS_SUFFIX(u"/xforms");
constexpr OUStringLiteral XML_N_XFORMS_1_0(u"http://www.w3.org/2002/xforms");
constexpr OUStringLiteral XML_N_SVG(u"http://www.w3.org/2000/svg");
constexpr OUStringLiteral XML_N_SVG_COMPAT(u"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
constexpr OUStringLiteral XML_N_FO(u"http://www.w3.org/1999/XSL/Format");
constexpr OUStringLiteral XML_N_FO_COMPAT(u"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
constexpr OUStringLiteral XML_N_SMIL(u"http://www.w3.org/2001/SMIL20/");
constexpr OUStringLiteral XML_N_SMIL_OLD(u"http://www.w3.org/2001/SMIL20");
constexpr OUStringLiteral XML_N_SMIL_COMPAT(u"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0");
constexpr OUStringLiteral XML_URN_OASIS_NAMES_TC(u"urn:oasis:names:tc");
constexpr OUStringLiteral XML_XMLNS(u"xmlns");
constexpr OUStringLiteral XML_OPENDOCUMENT(u"opendocument");
constexpr OUStringLiteral XML_1_0(u"1.0");

static bool NormalizeW3URI( OUString& rName )
{
    // check if URI matches:
    // http://www.w3.org/[0-9]*/[:letter:]*
    //                   (year)/(WG name)
    // For the following WG/standards names:
    // - xforms

    bool bSuccess = false;
    const OUString& sURIPrefix = XML_URI_W3_PREFIX;
    if( rName.startsWith( sURIPrefix ) )
    {
        const OUString& sURISuffix = XML_URI_XFORMS_SUFFIX ;
        sal_Int32 nCompareFrom = rName.getLength() - sURISuffix.getLength();
        if( rName.subView( nCompareFrom ) == sURISuffix )
        {
            // found W3 prefix, and xforms suffix
            rName = XML_N_XFORMS_1_0;
            bSuccess = true;
        }
    }
    return bSuccess;
}

static bool NormalizeOasisURN( OUString& rName )
{
    // #i38644#
    // we exported the wrong namespace for smil, so we correct this here on load
    // for older documents
    if( rName == XML_N_SVG )
    {
        rName = XML_N_SVG_COMPAT;
        return true;
    }
    else if( rName == XML_N_FO )
    {
        rName = XML_N_FO_COMPAT;
        return true;
    }
    else if( rName == XML_N_SMIL || rName == XML_N_SMIL_OLD  )
    {
        rName = XML_N_SMIL_COMPAT;
        return true;
    }


    // Check if URN matches
    // :urn:oasis:names:tc:[^:]*:xmlns:[^:]*:1.[^:]*
    //                     |---|       |---| |-----|
    //                     TC-Id      Sub-Id Version

    sal_Int32 nNameLen = rName.getLength();
    // :urn:oasis:names:tc.*
    const OUString& rOasisURN = XML_URN_OASIS_NAMES_TC;
    if( !rName.startsWith( rOasisURN ) )
        return false;

    // :urn:oasis:names:tc:.*
    sal_Int32 nPos = rOasisURN.getLength();
    if( nPos >= nNameLen || rName[nPos] != ':' )
        return false;

    // :urn:oasis:names:tc:[^:]:.*
    sal_Int32 nTCIdStart = nPos+1;
    sal_Int32 nTCIdEnd = rName.indexOf( ':', nTCIdStart );
    if( -1 == nTCIdEnd )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns.*
    nPos = nTCIdEnd + 1;
    OUString sTmp( rName.copy( nPos ) );
    const OUString& rXMLNS = XML_XMLNS;
    if( !sTmp.startsWith( rXMLNS ) )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns:.*
    nPos += rXMLNS.getLength();
    if( nPos >= nNameLen || rName[nPos] != ':' )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:.*
    nPos = rName.indexOf( ':', nPos+1 );
    if( -1 == nPos )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:[^:][^:][^:][^:]*
    sal_Int32 nVersionStart = nPos+1;
    if( nVersionStart+2 >= nNameLen ||
        -1 != rName.indexOf( ':', nVersionStart ) )
        return false;

    // :urn:oasis:names:tc:[^:]:xmlns:[^:]*:1\.[^:][^:]*
    if( rName[nVersionStart] != '1' || rName[nVersionStart+1] != '.' )
        return false;

    // replace [tcid] with current TCID and version with current version.

    rName = rName.subView( 0, nTCIdStart ) +
            XML_OPENDOCUMENT +
            rName.subView( nTCIdEnd, nVersionStart-nTCIdEnd ) +
            XML_1_0;

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
