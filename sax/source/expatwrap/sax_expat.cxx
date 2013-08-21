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
#include <sal/alloca.h>
#include <vector>

#include <osl/diagnose.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase3.hxx>

#include <expat.h>

using namespace ::rtl;
using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;

#include "factory.hxx"
#include "attrlistimpl.hxx"
#include "xml2utf.hxx"

namespace sax_expatwrap {

// Useful macros for correct String conversion depending on the choosen expat-mode
#ifdef XML_UNICODE
OUString XmlNChar2OUString( const XML_Char *p , int nLen )
{
    if( p ) {
        if( sizeof( sal_Unicode ) == sizeof( XML_Char ) )
        {
            return OUString( (sal_Unicode*)p,nLen);
        }
        else
        {
            sal_Unicode *pWchar = (sal_Unicode *)alloca( sizeof( sal_Unicode ) * nLen );
            for( int n = 0 ; n < nLen ; n++ ) {
                pWchar[n] = (sal_Unicode) p[n];
            }
            return OUString( pWchar , nLen );
        }
    }
    else {
        return OUString();
    }
}

OUString XmlChar2OUString( const XML_Char *p )
{
    if( p ) {
        int nLen;
        for( nLen = 0 ; p[nLen] ; nLen ++ )
            ;
         return XmlNChar2OUString( p , nLen );
     }
     else return OUString();
}


#define XML_CHAR_TO_OUSTRING(x) XmlChar2OUString(x)
#define XML_CHAR_N_TO_USTRING(x,n) XmlNChar2OUString(x,n)
#else
#define XML_CHAR_TO_OUSTRING(x) OUString(x , strlen( x ), RTL_TEXTENCODING_UTF8)
#define XML_CHAR_N_TO_USTRING(x,n) OUString(x,n, RTL_TEXTENCODING_UTF8 )
#endif


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
            pThis->callErrorHandler( pThis ,  e );\
         }\
        catch( const SAXException &e ) {\
            pThis->callErrorHandler( pThis , SAXParseException(\
                                            e.Message, \
                                            e.Context, \
                                            e.WrappedException,\
                                            pThis->rDocumentLocator->getPublicId(),\
                                            pThis->rDocumentLocator->getSystemId(),\
                                            pThis->rDocumentLocator->getLineNumber(),\
                                            pThis->rDocumentLocator->getColumnNumber()\
                                     ) );\
        }\
        catch( const com::sun::star::uno::RuntimeException &e ) {\
            pThis->bExceptionWasThrown = true; \
            pThis->bRTExceptionWasThrown = true; \
            pImpl->rtexception = e; \
        }\
    }\
    ((void)0)

#define IMPLEMENTATION_NAME "com.sun.star.comp.extensions.xml.sax.ParserExpat"
#define SERVICE_NAME        "com.sun.star.xml.sax.Parser"

class SaxExpatParser_Impl;


// This class implements the external Parser interface
class SaxExpatParser
    : public WeakImplHelper3< XInitialization
                            , XServiceInfo
                            , XParser >
{

public:
    SaxExpatParser();
    ~SaxExpatParser();

public:

    // The implementation details
    static Sequence< OUString >     getSupportedServiceNames_Static(void) throw ();

public:
    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(Sequence<Any> const& rArguments)
        throw (RuntimeException, Exception);

    // The SAX-Parser-Interface
    virtual void SAL_CALL parseStream(  const InputSource& structSource)
        throw ( SAXException,
                IOException,
                RuntimeException);
    virtual void SAL_CALL setDocumentHandler(const Reference< XDocumentHandler > & xHandler)
        throw (RuntimeException);

    virtual void SAL_CALL setErrorHandler(const Reference< XErrorHandler > & xHandler)
        throw (RuntimeException);
    virtual void SAL_CALL setDTDHandler(const Reference < XDTDHandler > & xHandler)
        throw (RuntimeException);
    virtual void SAL_CALL setEntityResolver(const Reference<  XEntityResolver >& xResolver)
        throw (RuntimeException);

    virtual void SAL_CALL setLocale( const Locale &locale )                     throw (RuntimeException);

public: // XServiceInfo
    OUString                     SAL_CALL getImplementationName() throw ();
    Sequence< OUString >         SAL_CALL getSupportedServiceNames(void) throw ();
    sal_Bool                     SAL_CALL supportsService(const OUString& ServiceName) throw ();

private:

    SaxExpatParser_Impl         *m_pImpl;

};

//--------------------------------------
// the extern interface
//---------------------------------------
Reference< XInterface > SAL_CALL SaxExpatParser_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference< XMultiServiceFactory > & )
    throw(Exception)
{
    SaxExpatParser *p = new SaxExpatParser;

    return Reference< XInterface > ( (OWeakObject * ) p );
}



Sequence< OUString >    SaxExpatParser::getSupportedServiceNames_Static(void) throw ()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OUString( SERVICE_NAME );
    return aRet;
}


//---------------------------------------------
// the implementation part
//---------------------------------------------


// Entity binds all information neede for a single file
struct Entity
{
    InputSource         structSource;
    XML_Parser          pParser;
    XMLFile2UTFConverter converter;
};


class SaxExpatParser_Impl
{
public: // module scope
    Mutex               aMutex;
    OUString            sCDATA;
    bool m_bEnableDoS; // fdo#60471 thank you Adobe Illustrator

    Reference< XDocumentHandler >   rDocumentHandler;
    Reference< XExtendedDocumentHandler > rExtendedDocumentHandler;

    Reference< XErrorHandler >  rErrorHandler;
    Reference< XDTDHandler >    rDTDHandler;
    Reference< XEntityResolver > rEntityResolver;
    Reference < XLocator >      rDocumentLocator;


    Reference < XAttributeList >    rAttrList;
    AttributeList   *pAttrList;

    // External entity stack
    vector<struct Entity>   vecEntity;
    void pushEntity( const struct Entity &entity )
        { vecEntity.push_back( entity ); }
    void popEntity()
        { vecEntity.pop_back( ); }
    struct Entity &getEntity()
        { return vecEntity.back(); }


    // Exception cannot be thrown through the C-XmlParser (possible resource leaks),
    // therefor the exception must be saved somewhere.
    SAXParseException   exception;
    RuntimeException    rtexception;
    bool                bExceptionWasThrown;
    bool                bRTExceptionWasThrown;

    Locale              locale;

public:
    SaxExpatParser_Impl()
        : sCDATA("CDATA")
        , m_bEnableDoS(false)
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

    int static callbackExternalEntityRef(   XML_Parser parser,
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


//---------------------------------------------
// LocatorImpl
//---------------------------------------------
class LocatorImpl :
    public WeakImplHelper2< XLocator, com::sun::star::io::XSeekable >
    // should use a different interface for stream positions!
{
public:
    LocatorImpl( SaxExpatParser_Impl *p )
    {
        m_pParser    = p;
    }

public: //XLocator
    virtual sal_Int32 SAL_CALL getColumnNumber(void) throw ()
    {
        return XML_GetCurrentColumnNumber( m_pParser->getEntity().pParser );
    }
    virtual sal_Int32 SAL_CALL getLineNumber(void) throw ()
    {
        return XML_GetCurrentLineNumber( m_pParser->getEntity().pParser );
    }
    virtual OUString SAL_CALL getPublicId(void) throw ()
    {
        return m_pParser->getEntity().structSource.sPublicId;
    }
    virtual OUString SAL_CALL getSystemId(void) throw ()
    {
        return m_pParser->getEntity().structSource.sSystemId;
    }

    // XSeekable (only for getPosition)

    virtual void SAL_CALL seek( sal_Int64 ) throw()
    {
    }
    virtual sal_Int64 SAL_CALL getPosition() throw()
    {
        return XML_GetCurrentByteIndex( m_pParser->getEntity().pParser );
    }
    virtual ::sal_Int64 SAL_CALL getLength() throw()
    {
        return 0;
    }

private:

    SaxExpatParser_Impl *m_pParser;
};




SaxExpatParser::SaxExpatParser(  )
{
    m_pImpl = new SaxExpatParser_Impl;

    LocatorImpl *pLoc = new LocatorImpl( m_pImpl );
    m_pImpl->rDocumentLocator = Reference< XLocator > ( pLoc );

    // performance-improvement. Reference is needed when calling the startTag callback.
    // Handing out the same object with every call is allowed (see sax-specification)
    m_pImpl->pAttrList = new AttributeList;
    m_pImpl->rAttrList = Reference< XAttributeList > ( m_pImpl->pAttrList );

    m_pImpl->bExceptionWasThrown = false;
    m_pImpl->bRTExceptionWasThrown = false;
}

SaxExpatParser::~SaxExpatParser()
{
    delete m_pImpl;
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL
SaxExpatParser::initialize(Sequence< Any > const& rArguments)
    throw (RuntimeException, Exception)
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

/***************
*
* parseStream does Parser-startup initializations. The SaxExpatParser_Impl::parse() method does
* the file-specific initialization work. (During a parser run, external files may be opened)
*
****************/
void SaxExpatParser::parseStream(   const InputSource& structSource)
    throw (SAXException,
           IOException,
           RuntimeException)
{
    // Only one text at one time
    MutexGuard guard( m_pImpl->aMutex );


    struct Entity entity;
    entity.structSource = structSource;

    if( ! entity.structSource.aInputStream.is() )
    {
        throw SAXException("No input source",
                            Reference< XInterface > () , Any() );
    }

    entity.converter.setInputStream( entity.structSource.aInputStream );
    if( !entity.structSource.sEncoding.isEmpty() )
    {
        entity.converter.setEncoding(
            OUStringToOString( entity.structSource.sEncoding , RTL_TEXTENCODING_ASCII_US ) );
    }

    // create parser with proper encoding
    entity.pParser = XML_ParserCreate( 0 );
    if( ! entity.pParser )
    {
        throw SAXException("Couldn't create parser",
                            Reference< XInterface > (), Any() );
    }

    // set all necessary C-Callbacks
    XML_SetUserData( entity.pParser , m_pImpl );
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
    XML_SetUnknownEncodingHandler( entity.pParser,  call_callbackUnknownEncoding ,0);

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
    m_pImpl->pushEntity( entity );
    try
    {
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
//      catch( SAXParseException &e )
//  {
//      m_pImpl->popEntity();
//          XML_ParserFree( entity.pParser );
//        Any aAny;
//        aAny <<= e;
//          throw SAXException( e.Message, e.Context, aAny );
//      }
    catch( SAXException & )
    {
        m_pImpl->popEntity();
        XML_ParserFree( entity.pParser );
          throw;
    }
    catch( IOException & )
    {
        m_pImpl->popEntity();
        XML_ParserFree( entity.pParser );
        throw;
    }
    catch( RuntimeException & )
    {
        m_pImpl->popEntity();
        XML_ParserFree( entity.pParser );
        throw;
    }

    m_pImpl->popEntity();
    XML_ParserFree( entity.pParser );
}

void SaxExpatParser::setDocumentHandler(const Reference< XDocumentHandler > & xHandler)
    throw (RuntimeException)
{
    m_pImpl->rDocumentHandler = xHandler;
    m_pImpl->rExtendedDocumentHandler =
        Reference< XExtendedDocumentHandler >( xHandler , UNO_QUERY );
}

void SaxExpatParser::setErrorHandler(const Reference< XErrorHandler > & xHandler)
    throw (RuntimeException)
{
    m_pImpl->rErrorHandler = xHandler;
}

void SaxExpatParser::setDTDHandler(const Reference< XDTDHandler > & xHandler)
    throw (RuntimeException)
{
    m_pImpl->rDTDHandler = xHandler;
}

void SaxExpatParser::setEntityResolver(const Reference < XEntityResolver > & xResolver)
    throw (RuntimeException)
{
    m_pImpl->rEntityResolver = xResolver;
}


void SaxExpatParser::setLocale( const Locale & locale ) throw (RuntimeException)
{
    m_pImpl->locale = locale;
}

// XServiceInfo
OUString SaxExpatParser::getImplementationName() throw ()
{
    return OUString( IMPLEMENTATION_NAME );
}

// XServiceInfo
sal_Bool SaxExpatParser::supportsService(const OUString& ServiceName) throw ()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > SaxExpatParser::getSupportedServiceNames(void) throw ()
{

    Sequence<OUString> seq(1);
    seq.getArray()[0] = OUString( SERVICE_NAME );
    return seq;
}


/*---------------------------------------
*
* Helper functions and classes
*
*
*-------------------------------------------*/
OUString getErrorMessage( XML_Error xmlE, OUString sSystemId , sal_Int32 nLine )
{
    OUString Message;
    if( XML_ERROR_NONE == xmlE ) {
        Message = OUString("No");
    }
    else if( XML_ERROR_NO_MEMORY == xmlE ) {
        Message = OUString("no memory");
    }
    else if( XML_ERROR_SYNTAX == xmlE ) {
        Message = OUString("syntax");
    }
    else if( XML_ERROR_NO_ELEMENTS == xmlE ) {
        Message = OUString("no elements");
    }
    else if( XML_ERROR_INVALID_TOKEN == xmlE ) {
        Message = OUString("invalid token");
    }
    else if( XML_ERROR_UNCLOSED_TOKEN == xmlE ) {
        Message = OUString("unclosed token");
    }
    else if( XML_ERROR_PARTIAL_CHAR == xmlE ) {
        Message = OUString("partial char");
    }
    else if( XML_ERROR_TAG_MISMATCH == xmlE ) {
        Message = OUString("tag mismatch");
    }
    else if( XML_ERROR_DUPLICATE_ATTRIBUTE == xmlE ) {
        Message = OUString("duplicate attribute");
    }
    else if( XML_ERROR_JUNK_AFTER_DOC_ELEMENT == xmlE ) {
        Message = OUString("junk after doc element");
    }
    else if( XML_ERROR_PARAM_ENTITY_REF == xmlE ) {
        Message = OUString("parameter entity reference");
    }
    else if( XML_ERROR_UNDEFINED_ENTITY == xmlE ) {
        Message = OUString("undefined entity");
    }
    else if( XML_ERROR_RECURSIVE_ENTITY_REF == xmlE ) {
        Message = OUString("recursive entity reference");
    }
    else if( XML_ERROR_ASYNC_ENTITY == xmlE ) {
        Message = OUString("async entity");
    }
    else if( XML_ERROR_BAD_CHAR_REF == xmlE ) {
        Message = OUString("bad char reference");
    }
    else if( XML_ERROR_BINARY_ENTITY_REF == xmlE ) {
        Message = OUString("binary entity reference");
    }
    else if( XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF == xmlE ) {
        Message = OUString("attribute external entity reference");
    }
    else if( XML_ERROR_MISPLACED_XML_PI == xmlE ) {
        Message = OUString("misplaced xml processing instruction");
    }
    else if( XML_ERROR_UNKNOWN_ENCODING == xmlE ) {
        Message = OUString("unknown encoding");
    }
    else if( XML_ERROR_INCORRECT_ENCODING == xmlE ) {
        Message = OUString("incorrect encoding");
    }
    else if( XML_ERROR_UNCLOSED_CDATA_SECTION == xmlE ) {
        Message = OUString("unclosed cdata section");
    }
    else if( XML_ERROR_EXTERNAL_ENTITY_HANDLING == xmlE ) {
        Message = OUString("external entity reference");
    }
    else if( XML_ERROR_NOT_STANDALONE == xmlE ) {
        Message = OUString("not standalone");
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
    Sequence< sal_Int8 > seqOut(nBufSize);

    while( nRead ) {
        nRead = getEntity().converter.readAndConvert( seqOut , nBufSize );

        if( ! nRead ) {
            XML_Parse( getEntity().pParser ,
                                   ( const char * ) seqOut.getArray() ,
                                   0 ,
                                   1 );
            break;
        }

        sal_Bool bContinue = ( XML_Parse( getEntity().pParser ,
                                                (const char *) seqOut.getArray(),
                                                nRead,
                                                0 ) != XML_STATUS_ERROR );

        if( ! bContinue || this->bExceptionWasThrown ) {

            if ( this->bRTExceptionWasThrown )
                throw rtexception;

            // Error during parsing !
            XML_Error xmlE = XML_GetErrorCode( getEntity().pParser );
            OUString sSystemId = rDocumentLocator->getSystemId();
            sal_Int32 nLine = rDocumentLocator->getLineNumber();

            SAXParseException aExcept(
                getErrorMessage(xmlE , sSystemId, nLine) ,
                Reference< XInterface >(),
                Any( &exception , getCppuType( &exception) ),
                rDocumentLocator->getPublicId(),
                rDocumentLocator->getSystemId(),
                rDocumentLocator->getLineNumber(),
                rDocumentLocator->getColumnNumber()
                );

            if( rErrorHandler.is() ) {

                // error handler is set, so the handler may throw the exception
                Any a;
                a <<= aExcept;
                rErrorHandler->fatalError( a );
            }

            // Error handler has not thrown an exception, but parsing cannot go on,
            // so an exception MUST be thrown.
            throw aExcept;
        } // if( ! bContinue )
    } // while
}

//------------------------------------------
//
// The C-Callbacks
//
//-----------------------------------------
void SaxExpatParser_Impl::callbackStartElement( void *pvThis ,
                                                const XML_Char *pwName ,
                                                const XML_Char **awAttributes )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    if( pImpl->rDocumentHandler.is() ) {

        int i = 0;
        pImpl->pAttrList->clear();

        while( awAttributes[i] ) {
            assert(awAttributes[i+1]);
            pImpl->pAttrList->addAttribute(
                XML_CHAR_TO_OUSTRING( awAttributes[i] ) ,
                pImpl->sCDATA,  // expat doesn't know types
                XML_CHAR_TO_OUSTRING( awAttributes[i+1] ) );
            i +=2;
        }

        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(
            pImpl ,
            rDocumentHandler->startElement( XML_CHAR_TO_OUSTRING( pwName ) ,
                                            pImpl->rAttrList ) );
    }
}

void SaxExpatParser_Impl::callbackEndElement( void *pvThis , const XML_Char *pwName  )
{
    SaxExpatParser_Impl  *pImpl = ((SaxExpatParser_Impl*)pvThis);

    if( pImpl->rDocumentHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rDocumentHandler->endElement( XML_CHAR_TO_OUSTRING( pwName ) ) );
    }
}


void SaxExpatParser_Impl::callbackCharacters( void *pvThis , const XML_Char *s , int nLen )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    if( pImpl->rDocumentHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl ,
                rDocumentHandler->characters( XML_CHAR_N_TO_USTRING(s,nLen) ) );
    }
}

void SaxExpatParser_Impl::callbackProcessingInstruction(    void *pvThis,
                                                    const XML_Char *sTarget ,
                                                    const XML_Char *sData )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
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
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
    if (value) { // value != 0 means internal entity
        SAL_INFO("sax","SaxExpatParser: internal entity declaration, stopping");
        XML_StopParser(pImpl->getEntity().pParser, XML_FALSE);
        pImpl->exception = SAXParseException(
            "SaxExpatParser: internal entity declaration, stopping",
            0, Any(),
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
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
    if( pImpl->rDTDHandler.is() ) {
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rDTDHandler->notationDecl(  XML_CHAR_TO_OUSTRING( notationName ) ,
                                            XML_CHAR_TO_OUSTRING( publicId ) ,
                                            XML_CHAR_TO_OUSTRING( systemId ) ) );
    }

}



int SaxExpatParser_Impl::callbackExternalEntityRef(
    XML_Parser parser, const XML_Char *context,
    SAL_UNUSED_PARAMETER const XML_Char * /*base*/, const XML_Char *systemId,
    const XML_Char *publicId)
{
    sal_Bool bOK = sal_True;
    InputSource source;
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)XML_GetUserData( parser ));

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
            bOK = sal_False;
        }
        catch( const SAXException & e )
        {
            pImpl->exception = SAXParseException(
                e.Message , e.Context , e.WrappedException ,
                pImpl->rDocumentLocator->getPublicId(),
                pImpl->rDocumentLocator->getSystemId(),
                pImpl->rDocumentLocator->getLineNumber(),
                pImpl->rDocumentLocator->getColumnNumber() );
            bOK = sal_False;
        }
    }

    if( entity.structSource.aInputStream.is() ) {
        entity.pParser = XML_ExternalEntityParserCreate( parser , context, 0 );
        if( ! entity.pParser )
        {
            return sal_False;
        }

        entity.converter.setInputStream( entity.structSource.aInputStream );
        pImpl->pushEntity( entity );
        try
        {
            pImpl->parse();
        }
        catch( const SAXParseException & e )
        {
            pImpl->exception = e;
            bOK = sal_False;
        }
        catch( const IOException &e )
        {
            pImpl->exception.WrappedException <<= e;
            bOK = sal_False;
        }
        catch( const RuntimeException &e )
        {
            pImpl->exception.WrappedException <<=e;
            bOK = sal_False;
        }

        pImpl->popEntity();

        XML_ParserFree( entity.pParser );
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
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(  pImpl,
                rExtendedDocumentHandler->unknown( XML_CHAR_N_TO_USTRING( s ,len) ) );
}

void SaxExpatParser_Impl::callbackComment( void *pvThis , const XML_Char *s )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl,
                rExtendedDocumentHandler->comment( XML_CHAR_TO_OUSTRING( s ) ) );
}

void SaxExpatParser_Impl::callbackStartCDATA( void *pvThis )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl, rExtendedDocumentHandler->startCDATA() );
}


void SaxExpatParser_Impl::callErrorHandler( SaxExpatParser_Impl *pImpl ,
                                            const SAXParseException & e )
{
    try
    {
        if( pImpl->rErrorHandler.is() ) {
            Any a;
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
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(pImpl,rExtendedDocumentHandler->endCDATA() );
}

}
using namespace sax_expatwrap;

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL expwrap_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager,
    SAL_UNUSED_PARAMETER void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    if (pServiceManager )
    {
        Reference< XSingleServiceFactory > xRet;
        Reference< XMultiServiceFactory > xSMgr =
            reinterpret_cast< XMultiServiceFactory * > ( pServiceManager );

        OUString aImplementationName = OUString::createFromAscii( pImplName );

        if ( aImplementationName == IMPLEMENTATION_NAME  )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        SaxExpatParser_CreateInstance,
                                        SaxExpatParser::getSupportedServiceNames_Static() );
        }
        else if ( aImplementationName == SaxWriter_getImplementationName() )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        SaxWriter_CreateInstance,
                                        SaxWriter_getSupportedServiceNames() );
        }

        if (xRet.is())
        {
            xRet->acquire();
            pRet = xRet.get();
        }
    }

    return pRet;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
