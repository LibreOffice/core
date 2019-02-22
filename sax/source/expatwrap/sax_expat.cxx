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
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>


#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#include <comphelper/attributelist.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include <expat.h>

using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;

#include <xml2utf.hxx>

namespace {

#define XML_CHAR_TO_OUSTRING(x) OUString(x , strlen( x ), RTL_TEXTENCODING_UTF8)
#define XML_CHAR_N_TO_USTRING(x,n) OUString(x,n, RTL_TEXTENCODING_UTF8 )


/*
* The following macro encapsulates any call to an event handler.
* It ensures, that exceptions thrown by the event handler are
* treated properly.
*/
#define CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(pThis,call) \
    if( ! pThis->bExceptionWasThrown ) { \
        try {\
            pThis->call;\
        }\
        catch( const SAXParseException &e ) {\
            callErrorHandler( pThis ,  e );\
         }\
        catch( const SAXException &e ) {\
            callErrorHandler( pThis , SAXParseException(\
                                            e.Message, \
                                            e.Context, \
                                            e.WrappedException,\
                                            pThis->rDocumentLocator->getPublicId(),\
                                            pThis->rDocumentLocator->getSystemId(),\
                                            pThis->rDocumentLocator->getLineNumber(),\
                                            pThis->rDocumentLocator->getColumnNumber()\
                                     ) );\
        }\
        catch( const css::uno::RuntimeException &e ) {\
            pThis->bExceptionWasThrown = true; \
            pThis->bRTExceptionWasThrown = true; \
            pImpl->rtexception = e; \
        }\
        catch( const css::uno::Exception &e ) {\
            pThis->bExceptionWasThrown = true; \
            pThis->bRTExceptionWasThrown = true; \
            pImpl->rtexception = WrappedTargetRuntimeException("Non-runtime UNO exception caught during parse", e.Context, makeAny(e)); \
        }\
    }\
    ((void)0)


class SaxExpatParser_Impl;

// This class implements the external Parser interface
class SaxExpatParser
    : public WeakImplHelper< XInitialization
                            , XServiceInfo
                            , XParser >
{

public:
    SaxExpatParser();

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(css::uno::Sequence<css::uno::Any> const& rArguments) override;

    // The SAX-Parser-Interface
    virtual void SAL_CALL parseStream(  const InputSource& structSource) override;
    virtual void SAL_CALL setDocumentHandler(const css::uno::Reference< XDocumentHandler > & xHandler) override;

    virtual void SAL_CALL setErrorHandler(const css::uno::Reference< XErrorHandler > & xHandler) override;
    virtual void SAL_CALL setDTDHandler(const css::uno::Reference < XDTDHandler > & xHandler) override;
    virtual void SAL_CALL setEntityResolver(const css::uno::Reference<  XEntityResolver >& xResolver) override;

    virtual void SAL_CALL setLocale( const Locale &locale ) override;

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() override;
    css::uno::Sequence< OUString >         SAL_CALL getSupportedServiceNames() override;
    sal_Bool                     SAL_CALL supportsService(const OUString& ServiceName) override;

private:
    std::unique_ptr<SaxExpatParser_Impl>   m_pImpl;
};


// Entity binds all information needed for a single file
struct Entity
{
    InputSource         structSource;
    XML_Parser          pParser;
    sax_expatwrap::XMLFile2UTFConverter converter;
};


static constexpr OUStringLiteral gsCDATA = "CDATA";

class SaxExpatParser_Impl
{
public: // module scope
    Mutex               aMutex;
    bool m_bEnableDoS; // fdo#60471 thank you Adobe Illustrator

    css::uno::Reference< XDocumentHandler >   rDocumentHandler;
    css::uno::Reference< XExtendedDocumentHandler > rExtendedDocumentHandler;

    css::uno::Reference< XErrorHandler >  rErrorHandler;
    css::uno::Reference< XDTDHandler >    rDTDHandler;
    css::uno::Reference< XEntityResolver > rEntityResolver;
    css::uno::Reference < XLocator >      rDocumentLocator;


    rtl::Reference < comphelper::AttributeList > rAttrList;

    // External entity stack
    vector<struct Entity>   vecEntity;
    void pushEntity( Entity &&entity )
        { vecEntity.push_back( std::move(entity) ); }
    void popEntity()
        { vecEntity.pop_back( ); }
    struct Entity &getEntity()
        { return vecEntity.back(); }


    // Exception cannot be thrown through the C-XmlParser (possible resource leaks),
    // therefore the exception must be saved somewhere.
    SAXParseException   exception;
    css::uno::RuntimeException    rtexception;
    bool                bExceptionWasThrown;
    bool                bRTExceptionWasThrown;

public:
    SaxExpatParser_Impl()
        : m_bEnableDoS(false)
        , bExceptionWasThrown(false)
        , bRTExceptionWasThrown(false)
    {
    }

    // the C-Callbacks for the expat parser
    void static callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts);
    void static callbackEndElement(void *userData, const XML_Char *name);
    void static callbackCharacters( void *userData , const XML_Char *s , int nLen );
    void static callbackProcessingInstruction(  void *userData ,
                                                const XML_Char *sTarget ,
                                                const XML_Char *sData );

    void static callbackEntityDecl( void *userData ,
                                    const XML_Char *entityName,
                                    int is_parameter_entity,
                                    const XML_Char *value,
                                    int value_length,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId,
                                    const XML_Char *notationName);

    void static callbackNotationDecl(   void *userData,
                                        const XML_Char *notationName,
                                        const XML_Char *base,
                                        const XML_Char *systemId,
                                        const XML_Char *publicId);

    bool static callbackExternalEntityRef(   XML_Parser parser,
                                            const XML_Char *openEntityNames,
                                            const XML_Char *base,
                                            const XML_Char *systemId,
                                            const XML_Char *publicId);

    int static callbackUnknownEncoding(void *encodingHandlerData,
                                                  const XML_Char *name,
                                                  XML_Encoding *info);

    void static callbackDefault( void *userData,  const XML_Char *s,  int len);

    void static callbackStartCDATA( void *userData );
    void static callbackEndCDATA( void *userData );
    void static callbackComment( void *userData , const XML_Char *s );
    void static callErrorHandler( SaxExpatParser_Impl *pImpl , const SAXParseException &e );

public:
    void parse();
};

extern "C"
{
    static void call_callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts)
    {
        SaxExpatParser_Impl::callbackStartElement(userData,name,atts);
    }
    static void call_callbackEndElement(void *userData, const XML_Char *name)
    {
        SaxExpatParser_Impl::callbackEndElement(userData,name);
    }
    static void call_callbackCharacters( void *userData , const XML_Char *s , int nLen )
    {
        SaxExpatParser_Impl::callbackCharacters(userData,s,nLen);
    }
    static void call_callbackProcessingInstruction(void *userData,const XML_Char *sTarget,const XML_Char *sData )
    {
        SaxExpatParser_Impl::callbackProcessingInstruction(userData,sTarget,sData );
    }
    static void call_callbackEntityDecl(void *userData ,
                                        const XML_Char *entityName,
                                        int is_parameter_entity,
                                        const XML_Char *value,
                                        int value_length,
                                        const XML_Char *base,
                                        const XML_Char *systemId,
                                        const XML_Char *publicId,
                                        const XML_Char *notationName)
    {
        SaxExpatParser_Impl::callbackEntityDecl(userData, entityName,
                is_parameter_entity, value, value_length,
                base, systemId, publicId, notationName);
    }
    static void call_callbackNotationDecl(void *userData,
                                          const XML_Char *notationName,
                                          const XML_Char *base,
                                          const XML_Char *systemId,
                                          const XML_Char *publicId)
    {
        SaxExpatParser_Impl::callbackNotationDecl(userData,notationName,base,systemId,publicId);
    }
    static int call_callbackExternalEntityRef(XML_Parser parser,
                                              const XML_Char *openEntityNames,
                                              const XML_Char *base,
                                              const XML_Char *systemId,
                                              const XML_Char *publicId)
    {
        return SaxExpatParser_Impl::callbackExternalEntityRef(parser,openEntityNames,base,systemId,publicId);
    }
    static int call_callbackUnknownEncoding(void *encodingHandlerData,
                                              const XML_Char *name,
                                            XML_Encoding *info)
    {
        return SaxExpatParser_Impl::callbackUnknownEncoding(encodingHandlerData,name,info);
    }
    static void call_callbackDefault( void *userData,  const XML_Char *s,  int len)
    {
        SaxExpatParser_Impl::callbackDefault(userData,s,len);
    }
    static void call_callbackStartCDATA( void *userData )
    {
        SaxExpatParser_Impl::callbackStartCDATA(userData);
    }
    static void call_callbackEndCDATA( void *userData )
    {
        SaxExpatParser_Impl::callbackEndCDATA(userData);
    }
    static void call_callbackComment( void *userData , const XML_Char *s )
    {
        SaxExpatParser_Impl::callbackComment(userData,s);
    }
}


// LocatorImpl

class LocatorImpl :
    public WeakImplHelper< XLocator, css::io::XSeekable >
    // should use a different interface for stream positions!
{
public:
    explicit LocatorImpl(SaxExpatParser_Impl *p)
        : m_pParser(p)
    {
    }

public: //XLocator
    virtual sal_Int32 SAL_CALL getColumnNumber() override
    {
        return XML_GetCurrentColumnNumber( m_pParser->getEntity().pParser );
    }
    virtual sal_Int32 SAL_CALL getLineNumber() override
    {
        return XML_GetCurrentLineNumber( m_pParser->getEntity().pParser );
    }
    virtual OUString SAL_CALL getPublicId() override
    {
        return m_pParser->getEntity().structSource.sPublicId;
    }
    virtual OUString SAL_CALL getSystemId() override
    {
        return m_pParser->getEntity().structSource.sSystemId;
    }

    // XSeekable (only for getPosition)

    virtual void SAL_CALL seek( sal_Int64 ) override
    {
    }
    virtual sal_Int64 SAL_CALL getPosition() override
    {
        return XML_GetCurrentByteIndex( m_pParser->getEntity().pParser );
    }
    virtual ::sal_Int64 SAL_CALL getLength() override
    {
        return 0;
    }

private:

    SaxExpatParser_Impl *m_pParser;
};


SaxExpatParser::SaxExpatParser(  )
{
    m_pImpl.reset( new SaxExpatParser_Impl );

    LocatorImpl *pLoc = new LocatorImpl( m_pImpl.get() );
    m_pImpl->rDocumentLocator.set( pLoc );

    // Performance-improvement; handing out the same object with every call of
    // the startElement callback is allowed (see sax-specification):
    m_pImpl->rAttrList = new comphelper::AttributeList;

    m_pImpl->bExceptionWasThrown = false;
    m_pImpl->bRTExceptionWasThrown = false;
}

// css::lang::XInitialization:
void SAL_CALL
SaxExpatParser::initialize(css::uno::Sequence< css::uno::Any > const& rArguments)
{
    // possible arguments: a string "DoSmeplease"
    if (rArguments.getLength())
    {
        OUString str;
        if ((rArguments[0] >>= str) && "DoSmeplease" == str)
        {
            MutexGuard guard( m_pImpl->aMutex );
            m_pImpl->m_bEnableDoS = true;
        }
    }
}

class ParserCleanup
{
private:
    SaxExpatParser_Impl& m_rParser;
    XML_Parser const m_xmlParser;
public:
    ParserCleanup(SaxExpatParser_Impl& rParser, XML_Parser xmlParser)
        : m_rParser(rParser)
        , m_xmlParser(xmlParser)
    {
    }
    ~ParserCleanup()
    {
        m_rParser.popEntity();
        //XML_ParserFree accepts a null arg
        XML_ParserFree(m_xmlParser);
    }
};

/***************
*
* parseStream does Parser-startup initializations. The SaxExpatParser_Impl::parse() method does
* the file-specific initialization work. (During a parser run, external files may be opened)
*
****************/
void SaxExpatParser::parseStream(   const InputSource& structSource)
{
    // Only one text at one time
    MutexGuard guard( m_pImpl->aMutex );


    struct Entity entity;
    entity.structSource = structSource;

    if( ! entity.structSource.aInputStream.is() )
    {
        throw SAXException("No input source",
                            css::uno::Reference< css::uno::XInterface > () , css::uno::Any() );
    }

    entity.converter.setInputStream( entity.structSource.aInputStream );
    if( !entity.structSource.sEncoding.isEmpty() )
    {
        entity.converter.setEncoding(
            OUStringToOString( entity.structSource.sEncoding , RTL_TEXTENCODING_ASCII_US ) );
    }

    // create parser with proper encoding
    entity.pParser = XML_ParserCreate( nullptr );
    if( ! entity.pParser )
    {
        throw SAXException("Couldn't create parser",
                            css::uno::Reference< css::uno::XInterface > (), css::uno::Any() );
    }

    // set all necessary C-Callbacks
    XML_SetUserData( entity.pParser, m_pImpl.get() );
    XML_SetElementHandler(  entity.pParser ,
                            call_callbackStartElement ,
                            call_callbackEndElement );
    XML_SetCharacterDataHandler( entity.pParser , call_callbackCharacters );
    XML_SetProcessingInstructionHandler(entity.pParser ,
                                        call_callbackProcessingInstruction );
    if (!m_pImpl->m_bEnableDoS)
    {
        XML_SetEntityDeclHandler(entity.pParser, call_callbackEntityDecl);
    }
    XML_SetNotationDeclHandler( entity.pParser, call_callbackNotationDecl );
    XML_SetExternalEntityRefHandler(    entity.pParser,
                                        call_callbackExternalEntityRef);
    XML_SetUnknownEncodingHandler( entity.pParser,  call_callbackUnknownEncoding ,nullptr);

    if( m_pImpl->rExtendedDocumentHandler.is() ) {

        // These handlers just delegate calls to the ExtendedHandler. If no extended handler is
        // given, these callbacks can be ignored
        XML_SetDefaultHandlerExpand( entity.pParser, call_callbackDefault );
        XML_SetCommentHandler( entity.pParser, call_callbackComment );
        XML_SetCdataSectionHandler(     entity.pParser ,
                                        call_callbackStartCDATA ,
                                         call_callbackEndCDATA );
    }


    m_pImpl->exception = SAXParseException();
    auto const xmlParser = entity.pParser;
    m_pImpl->pushEntity( std::move(entity) );

    ParserCleanup aEnsureFree(*m_pImpl, xmlParser);

    // start the document
    if( m_pImpl->rDocumentHandler.is() ) {
        m_pImpl->rDocumentHandler->setDocumentLocator( m_pImpl->rDocumentLocator );
        m_pImpl->rDocumentHandler->startDocument();
    }

    m_pImpl->parse();

    // finish document
    if( m_pImpl->rDocumentHandler.is() ) {
        m_pImpl->rDocumentHandler->endDocument();
    }
}

void SaxExpatParser::setDocumentHandler(const css::uno::Reference< XDocumentHandler > & xHandler)
{
    m_pImpl->rDocumentHandler = xHandler;
    m_pImpl->rExtendedDocumentHandler =
        css::uno::Reference< XExtendedDocumentHandler >( xHandler , css::uno::UNO_QUERY );
}

void SaxExpatParser::setErrorHandler(const css::uno::Reference< XErrorHandler > & xHandler)
{
    m_pImpl->rErrorHandler = xHandler;
}

void SaxExpatParser::setDTDHandler(const css::uno::Reference< XDTDHandler > & xHandler)
{
    m_pImpl->rDTDHandler = xHandler;
}

void SaxExpatParser::setEntityResolver(const css::uno::Reference < XEntityResolver > & xResolver)
{
    m_pImpl->rEntityResolver = xResolver;
}


void SaxExpatParser::setLocale( const Locale & )
{
    // not implemented
}

// XServiceInfo
OUString SaxExpatParser::getImplementationName()
{
    return OUString("com.sun.star.comp.extensions.xml.sax.ParserExpat");
}

// XServiceInfo
sal_Bool SaxExpatParser::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
css::uno::Sequence< OUString > SaxExpatParser::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> seq { "com.sun.star.xml.sax.Parser" };
    return seq;
}


/*---------------------------------------
*
* Helper functions and classes
*
*
*-------------------------------------------*/
OUString getErrorMessage( XML_Error xmlE, const OUString& sSystemId , sal_Int32 nLine )
{
    OUString Message;
    if( XML_ERROR_NONE == xmlE ) {
        Message = "No";
    }
    else if( XML_ERROR_NO_MEMORY == xmlE ) {
        Message = "no memory";
    }
    else if( XML_ERROR_SYNTAX == xmlE ) {
        Message = "syntax";
    }
    else if( XML_ERROR_NO_ELEMENTS == xmlE ) {
        Message = "no elements";
    }
    else if( XML_ERROR_INVALID_TOKEN == xmlE ) {
        Message = "invalid token";
    }
    else if( XML_ERROR_UNCLOSED_TOKEN == xmlE ) {
        Message = "unclosed token";
    }
    else if( XML_ERROR_PARTIAL_CHAR == xmlE ) {
        Message = "partial char";
    }
    else if( XML_ERROR_TAG_MISMATCH == xmlE ) {
        Message = "tag mismatch";
    }
    else if( XML_ERROR_DUPLICATE_ATTRIBUTE == xmlE ) {
        Message = "duplicate attribute";
    }
    else if( XML_ERROR_JUNK_AFTER_DOC_ELEMENT == xmlE ) {
        Message = "junk after doc element";
    }
    else if( XML_ERROR_PARAM_ENTITY_REF == xmlE ) {
        Message = "parameter entity reference";
    }
    else if( XML_ERROR_UNDEFINED_ENTITY == xmlE ) {
        Message = "undefined entity";
    }
    else if( XML_ERROR_RECURSIVE_ENTITY_REF == xmlE ) {
        Message = "recursive entity reference";
    }
    else if( XML_ERROR_ASYNC_ENTITY == xmlE ) {
        Message = "async entity";
    }
    else if( XML_ERROR_BAD_CHAR_REF == xmlE ) {
        Message = "bad char reference";
    }
    else if( XML_ERROR_BINARY_ENTITY_REF == xmlE ) {
        Message = "binary entity reference";
    }
    else if( XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF == xmlE ) {
        Message = "attribute external entity reference";
    }
    else if( XML_ERROR_MISPLACED_XML_PI == xmlE ) {
        Message = "misplaced xml processing instruction";
    }
    else if( XML_ERROR_UNKNOWN_ENCODING == xmlE ) {
        Message = "unknown encoding";
    }
    else if( XML_ERROR_INCORRECT_ENCODING == xmlE ) {
        Message = "incorrect encoding";
    }
    else if( XML_ERROR_UNCLOSED_CDATA_SECTION == xmlE ) {
        Message = "unclosed cdata section";
    }
    else if( XML_ERROR_EXTERNAL_ENTITY_HANDLING == xmlE ) {
        Message = "external entity reference";
    }
    else if( XML_ERROR_NOT_STANDALONE == xmlE ) {
        Message = "not standalone";
    }

    OUString str("[");
    str += sSystemId;
    str += " line ";
    str += OUString::number( nLine );
    str += "]: ";
    str += Message;
    str += "error";

    return str;
}


// starts parsing with actual parser !
void SaxExpatParser_Impl::parse( )
{
    const int nBufSize = 16*1024;

    int nRead   = nBufSize;
    css::uno::Sequence< sal_Int8 > seqOut(nBufSize);

    while( nRead ) {
        nRead = getEntity().converter.readAndConvert( seqOut , nBufSize );

        bool bContinue(false);

        if( ! nRead ) {
            // last call - must return OK
            XML_Status const ret = XML_Parse( getEntity().pParser,
                                   reinterpret_cast<const char *>(seqOut.getConstArray()),
                                   0 ,
                                   1 );
            if (ret == XML_STATUS_OK) {
                break;
            }
        } else {
            bContinue = ( XML_Parse( getEntity().pParser,
                                                reinterpret_cast<const char *>(seqOut.getConstArray()),
                                                nRead,
                                                0 ) != XML_STATUS_ERROR );
        }

        if( ! bContinue || bExceptionWasThrown ) {

            if ( bRTExceptionWasThrown )
                throw rtexception;

            // Error during parsing !
            XML_Error xmlE = XML_GetErrorCode( getEntity().pParser );
            OUString sSystemId = rDocumentLocator->getSystemId();
            sal_Int32 nLine = rDocumentLocator->getLineNumber();

            SAXParseException aExcept(
                getErrorMessage(xmlE , sSystemId, nLine) ,
                css::uno::Reference< css::uno::XInterface >(),
                css::uno::Any( &exception , cppu::UnoType<decltype(exception)>::get() ),
                rDocumentLocator->getPublicId(),
                rDocumentLocator->getSystemId(),
                rDocumentLocator->getLineNumber(),
                rDocumentLocator->getColumnNumber()
                );

            if( rErrorHandler.is() ) {

                // error handler is set, so the handler may throw the exception
                css::uno::Any a;
                a <<= aExcept;
                rErrorHandler->fatalError( a );
            }

            // Error handler has not thrown an exception, but parsing cannot go on,
            // so an exception MUST be thrown.
            throw aExcept;
        } // if( ! bContinue )
    } // while
}


// The C-Callbacks


void SaxExpatParser_Impl::callbackStartElement( void *pvThis ,
                                                const XML_Char *pwName ,
                                                const XML_Char **awAttributes )
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);

    if( pImpl->rDocumentHandler.is() ) {

        int i = 0;
        pImpl->rAttrList->Clear();

        while( awAttributes[i] ) {
            assert(awAttributes[i+1]);
            pImpl->rAttrList->AddAttribute(
                XML_CHAR_TO_OUSTRING( awAttributes[i] ) ,
                gsCDATA,  // expat doesn't know types
                XML_CHAR_TO_OUSTRING( awAttributes[i+1] ) );
            i +=2;
        }

        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(
            pImpl ,
            rDocumentHandler->startElement( XML_CHAR_TO_OUSTRING( pwName ) ,
                                            pImpl->rAttrList.get() ) );
    }
}

void SaxExpatParser_Impl::callbackEndElement( void *pvThis , const XML_Char *pwName  )
{
    SaxExpatParser_Impl  *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);

    if( pImpl->rDocumentHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rDocumentHandler->endElement( XML_CHAR_TO_OUSTRING( pwName ) ) );
    }
}


void SaxExpatParser_Impl::callbackCharacters( void *pvThis , const XML_Char *s , int nLen )
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);

    if( pImpl->rDocumentHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl ,
                rDocumentHandler->characters( XML_CHAR_N_TO_USTRING(s,nLen) ) );
    }
}

void SaxExpatParser_Impl::callbackProcessingInstruction(    void *pvThis,
                                                    const XML_Char *sTarget ,
                                                    const XML_Char *sData )
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);
    if( pImpl->rDocumentHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(
                    pImpl ,
                    rDocumentHandler->processingInstruction( XML_CHAR_TO_OUSTRING( sTarget ),
                    XML_CHAR_TO_OUSTRING( sData ) ) );
    }
}


void SaxExpatParser_Impl::callbackEntityDecl(
    void *pvThis, const XML_Char *entityName,
    SAL_UNUSED_PARAMETER int /*is_parameter_entity*/,
    const XML_Char *value, SAL_UNUSED_PARAMETER int /*value_length*/,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/, const XML_Char *systemId,
    const XML_Char *publicId, const XML_Char *notationName)
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);
    if (value) { // value != 0 means internal entity
        SAL_INFO("sax","SaxExpatParser: internal entity declaration, stopping");
        XML_StopParser(pImpl->getEntity().pParser, XML_FALSE);
        pImpl->exception = SAXParseException(
            "SaxExpatParser: internal entity declaration, stopping",
            nullptr, css::uno::Any(),
            pImpl->rDocumentLocator->getPublicId(),
            pImpl->rDocumentLocator->getSystemId(),
            pImpl->rDocumentLocator->getLineNumber(),
            pImpl->rDocumentLocator->getColumnNumber() );
        pImpl->bExceptionWasThrown = true;
    } else {
        if( pImpl->rDTDHandler.is() ) {
            CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(
                pImpl ,
                rDTDHandler->unparsedEntityDecl(
                    XML_CHAR_TO_OUSTRING( entityName ),
                    XML_CHAR_TO_OUSTRING( publicId ) ,
                    XML_CHAR_TO_OUSTRING( systemId ) ,
                    XML_CHAR_TO_OUSTRING( notationName ) ) );
        }
    }
}

void SaxExpatParser_Impl::callbackNotationDecl(
    void *pvThis, const XML_Char *notationName,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/, const XML_Char *systemId,
    const XML_Char *publicId)
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);
    if( pImpl->rDTDHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rDTDHandler->notationDecl(  XML_CHAR_TO_OUSTRING( notationName ) ,
                                            XML_CHAR_TO_OUSTRING( publicId ) ,
                                            XML_CHAR_TO_OUSTRING( systemId ) ) );
    }

}


bool SaxExpatParser_Impl::callbackExternalEntityRef(
    XML_Parser parser, const XML_Char *context,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/, const XML_Char *systemId,
    const XML_Char *publicId)
{
    bool bOK = true;
    InputSource source;
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(XML_GetUserData( parser ));

    struct Entity entity;

    if( pImpl->rEntityResolver.is() ) {
        try
        {
            entity.structSource = pImpl->rEntityResolver->resolveEntity(
                XML_CHAR_TO_OUSTRING( publicId ) ,
                XML_CHAR_TO_OUSTRING( systemId ) );
        }
        catch( const SAXParseException & e )
        {
            pImpl->exception = e;
            bOK = false;
        }
        catch( const SAXException & e )
        {
            pImpl->exception = SAXParseException(
                e.Message , e.Context , e.WrappedException ,
                pImpl->rDocumentLocator->getPublicId(),
                pImpl->rDocumentLocator->getSystemId(),
                pImpl->rDocumentLocator->getLineNumber(),
                pImpl->rDocumentLocator->getColumnNumber() );
            bOK = false;
        }
    }

    if( entity.structSource.aInputStream.is() ) {
        entity.pParser = XML_ExternalEntityParserCreate( parser , context, nullptr );
        if( ! entity.pParser )
        {
            return false;
        }

        entity.converter.setInputStream( entity.structSource.aInputStream );
        auto const xmlParser = entity.pParser;
        pImpl->pushEntity( std::move(entity) );
        try
        {
            pImpl->parse();
        }
        catch( const SAXParseException & e )
        {
            pImpl->exception = e;
            bOK = false;
        }
        catch( const IOException &e )
        {
            pImpl->exception.WrappedException <<= e;
            bOK = false;
        }
        catch( const css::uno::RuntimeException &e )
        {
            pImpl->exception.WrappedException <<=e;
            bOK = false;
        }

        pImpl->popEntity();

        XML_ParserFree( xmlParser );
    }

    return bOK;
}

int SaxExpatParser_Impl::callbackUnknownEncoding(
    SAL_UNUSED_PARAMETER void * /*encodingHandlerData*/,
    SAL_UNUSED_PARAMETER const XML_Char * /*name*/,
    SAL_UNUSED_PARAMETER XML_Encoding * /*info*/)
{
    return 0;
}

void SaxExpatParser_Impl::callbackDefault( void *pvThis,  const XML_Char *s,  int len)
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(  pImpl,
                rExtendedDocumentHandler->unknown( XML_CHAR_N_TO_USTRING( s ,len) ) );
}

void SaxExpatParser_Impl::callbackComment( void *pvThis , const XML_Char *s )
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);
    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rExtendedDocumentHandler->comment( XML_CHAR_TO_OUSTRING( s ) ) );
}

void SaxExpatParser_Impl::callbackStartCDATA( void *pvThis )
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl, rExtendedDocumentHandler->startCDATA() );
}


void SaxExpatParser_Impl::callErrorHandler( SaxExpatParser_Impl *pImpl ,
                                            const SAXParseException & e )
{
    try
    {
        if( pImpl->rErrorHandler.is() ) {
            css::uno::Any a;
            a <<= e;
            pImpl->rErrorHandler->error( a );
        }
        else {
            pImpl->exception = e;
            pImpl->bExceptionWasThrown = true;
        }
    }
    catch( const SAXParseException & ex ) {
        pImpl->exception = ex;
        pImpl->bExceptionWasThrown = true;
    }
    catch( const SAXException & ex ) {
        pImpl->exception = SAXParseException(
                                    ex.Message,
                                    ex.Context,
                                    ex.WrappedException,
                                    pImpl->rDocumentLocator->getPublicId(),
                                    pImpl->rDocumentLocator->getSystemId(),
                                    pImpl->rDocumentLocator->getLineNumber(),
                                    pImpl->rDocumentLocator->getColumnNumber()
                             );
        pImpl->bExceptionWasThrown = true;
    }
}

void SaxExpatParser_Impl::callbackEndCDATA( void *pvThis )
{
    SaxExpatParser_Impl *pImpl = static_cast<SaxExpatParser_Impl*>(pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(pImpl,rExtendedDocumentHandler->endCDATA() );
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_extensions_xml_sax_ParserExpat_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SaxExpatParser);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
