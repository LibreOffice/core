/*************************************************************************
 *
 *  $RCSfile: sax_expat.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:43:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <vector>

#ifdef WIN32
#include <malloc.h>
#else
#ifndef MACOSX
#include <alloca.h>
#endif
#endif

#include <osl/diagnose.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include <assert.h>

#include "expat/xmlparse.h"

using namespace ::rtl;
using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;

#include "factory.hxx"
#include "attrlistimpl.hxx"
#include "xml2utf.hxx"

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
        for( int nLen = 0 ; p[nLen] ; nLen ++ );
         return XmlNChar2OUString( p , nLen );
     }
     else return OUString();
}


// Useful macros for correct String conversion depending on the choosen expat-mode
#ifdef XML_UNICODE
#define XML_CHAR_TO_OUSTRING(x) XmlChar2OUString(x)
#define XML_CHAR_N_TO_USTRING(x,n) XmlNChar2OUString(x,n)
#else
#define XML_CHAR_TO_OUSTRING(x) OStringToOUString(OString(x), RTL_TEXTENCODING_UTF8)
#define XML_CHAR_N_TO_OUSTRING(x,n) OStringToOUString(OString(x,n), RTL_TEXTENCODING_UTF8 )
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
        catch( SAXParseException &e ) {\
            pThis->callErrorHandler( pThis ,  e );\
         }\
        catch( SAXException &e ) {\
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
    }\
    ((void)0)

#define IMPLEMENTATION_NAME "com.sun.star.comp.extensions.xml.sax.ParserExpat"
#define SERVICE_NAME        "com.sun.star.xml.sax.Parser"

class SaxExpatParser_Impl;


// This class implements the external Parser interface
class SaxExpatParser :
    public WeakImplHelper2<
                XParser,
                XServiceInfo
                          >
{

public:
    SaxExpatParser();
    ~SaxExpatParser();

public:

    // The implementation details
    static Sequence< OUString >     getSupportedServiceNames_Static(void) throw ();
    static OUString                 getImplementationName_Static() throw ();

public:
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
    const Reference< XMultiServiceFactory  >  & rSMgr ) throw(Exception)
{
    SaxExpatParser *p = new SaxExpatParser;

    return Reference< XInterface > ( (OWeakObject * ) p );
}



Sequence< OUString >    SaxExpatParser::getSupportedServiceNames_Static(void) throw ()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = SaxExpatParser::getImplementationName_Static();
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

    Reference< XDocumentHandler >   rDocumentHandler;
    Reference< XExtendedDocumentHandler > rExtendedDocumentHandler;

    Reference< XErrorHandler >  rErrorHandler;
    Reference< XDTDHandler >    rDTDHandler;
    Reference< XEntityResolver > rEntityResolver;
    Reference < XLocator >      rDocumentLocator;


    Reference < XAttributeList >    rAttrList;
    AttributeListImpl   *pAttrList;

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
    sal_Bool                bExceptionWasThrown;

    Locale              locale;

public:
    // the C-Callbacks for the expat parser
    void static callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts);
    void static callbackEndElement(void *userData, const XML_Char *name);
    void static callbackCharacters( void *userData , const XML_Char *s , int nLen );
    void static callbackProcessingInstruction(  void *userData ,
                                                const XML_Char *sTarget ,
                                                const XML_Char *sData );

    void static callbackUnparsedEntityDecl( void *userData ,
                                              const XML_Char *entityName,
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


//---------------------------------------------
// LocatorImpl
//---------------------------------------------
class LocatorImpl :
    public WeakImplHelper1< XLocator >
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

private:

    SaxExpatParser_Impl *m_pParser;
};




SaxExpatParser::SaxExpatParser(  )
{
    m_pImpl = new SaxExpatParser_Impl;

    LocatorImpl *pLoc = new LocatorImpl( m_pImpl );
    m_pImpl->rDocumentLocator = Reference< XLocator > ( pLoc );

    // performance-Improvment. Reference is needed when calling the startTag callback.
    // Handing out the same object with every call is allowed (see sax-specification)
    m_pImpl->pAttrList = new AttributeListImpl;
    m_pImpl->rAttrList = Reference< XAttributeList > ( m_pImpl->pAttrList );

    m_pImpl->bExceptionWasThrown = sal_False;
}

SaxExpatParser::~SaxExpatParser()
{
    delete m_pImpl;
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
        throw SAXException( OUString::createFromAscii( "No input source" ) ,
                            Reference< XInterface > () , Any() );
    }

    entity.converter.setInputStream( entity.structSource.aInputStream );
    if( entity.structSource.sEncoding.len() )
    {
        entity.converter.setEncoding(
            OUStringToOString( entity.structSource.sEncoding , RTL_TEXTENCODING_ASCII_US ) );
    }

    // create parser with proper encoding
    entity.pParser = XML_ParserCreate( 0 );
    if( ! entity.pParser )
    {
        throw SAXException( OUString::createFromAscii( "Couldn't create parser" ) ,
                            Reference< XInterface > (), Any() );
    }

    // set all necessary C-Callbacks
    XML_SetUserData( entity.pParser , m_pImpl );
    XML_SetElementHandler(  entity.pParser ,
                            SaxExpatParser_Impl::callbackStartElement ,
                            SaxExpatParser_Impl::callbackEndElement );
    XML_SetCharacterDataHandler( entity.pParser , SaxExpatParser_Impl::callbackCharacters );
    XML_SetProcessingInstructionHandler(entity.pParser ,
                                        SaxExpatParser_Impl::callbackProcessingInstruction );
    XML_SetUnparsedEntityDeclHandler(   entity.pParser,
                                        SaxExpatParser_Impl::callbackUnparsedEntityDecl );
    XML_SetNotationDeclHandler( entity.pParser, SaxExpatParser_Impl::callbackNotationDecl );
    XML_SetExternalEntityRefHandler(    entity.pParser,
                                        SaxExpatParser_Impl::callbackExternalEntityRef);
    XML_SetUnknownEncodingHandler( entity.pParser,  SaxExpatParser_Impl::callbackUnknownEncoding ,0);

    if( m_pImpl->rExtendedDocumentHandler.is() ) {

        // These handlers just delegate calls to the ExtendedHandler. If no extended handler is
        // given, these callbacks can be ignored
        XML_SetDefaultHandlerExpand( entity.pParser, SaxExpatParser_Impl::callbackDefault );
        XML_SetCommentHandler( entity.pParser, SaxExpatParser_Impl::callbackComment );
        XML_SetCdataSectionHandler(     entity.pParser ,
                                        SaxExpatParser_Impl::callbackStartCDATA ,
                                         SaxExpatParser_Impl::callbackEndCDATA );
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

//      catch( SAXParseException & )
//      {
//          m_pImpl->popEntity();
//          XML_ParserFree( entity.pParser );
//          throw;
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

OUString    SaxExpatParser::getImplementationName_Static() throw ()
{
    return OUString::createFromAscii( IMPLEMENTATION_NAME );
}

// XServiceInfo
OUString SaxExpatParser::getImplementationName() throw ()
{
    return OUString::createFromAscii( IMPLEMENTATION_NAME );
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
    seq.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
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
        Message = OUString::createFromAscii( "No" );
    }
    else if( XML_ERROR_NO_MEMORY == xmlE ) {
        Message = OUString::createFromAscii( "no memory" );
    }
    else if( XML_ERROR_SYNTAX == xmlE ) {
        Message = OUString::createFromAscii( "syntax" );
    }
    else if( XML_ERROR_NO_ELEMENTS == xmlE ) {
        Message = OUString::createFromAscii( "no elements" );
    }
    else if( XML_ERROR_INVALID_TOKEN == xmlE ) {
        Message = OUString::createFromAscii( "invalid token" );
    }
    else if( XML_ERROR_UNCLOSED_TOKEN == xmlE ) {
        Message = OUString::createFromAscii( "unclosed token" );
    }
    else if( XML_ERROR_PARTIAL_CHAR == xmlE ) {
        Message = OUString::createFromAscii( "partial char" );
    }
    else if( XML_ERROR_TAG_MISMATCH == xmlE ) {
        Message = OUString::createFromAscii( "tag mismatch" );
    }
    else if( XML_ERROR_DUPLICATE_ATTRIBUTE == xmlE ) {
        Message = OUString::createFromAscii( "duplicate attribute" );
    }
    else if( XML_ERROR_JUNK_AFTER_DOC_ELEMENT == xmlE ) {
        Message = OUString::createFromAscii( "junk after doc element" );
    }
    else if( XML_ERROR_PARAM_ENTITY_REF == xmlE ) {
        Message = OUString::createFromAscii( "parameter entity reference" );
    }
    else if( XML_ERROR_UNDEFINED_ENTITY == xmlE ) {
        Message = OUString::createFromAscii( "undefined entity" );
    }
    else if( XML_ERROR_RECURSIVE_ENTITY_REF == xmlE ) {
        Message = OUString::createFromAscii( "recursive entity reference" );
    }
    else if( XML_ERROR_ASYNC_ENTITY == xmlE ) {
        Message = OUString::createFromAscii( "async entity" );
    }
    else if( XML_ERROR_BAD_CHAR_REF == xmlE ) {
        Message = OUString::createFromAscii( "bad char reference" );
    }
    else if( XML_ERROR_BINARY_ENTITY_REF == xmlE ) {
        Message = OUString::createFromAscii( "binary entity reference" );
    }
    else if( XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF == xmlE ) {
        Message = OUString::createFromAscii( "attribute external entity reference" );
    }
    else if( XML_ERROR_MISPLACED_XML_PI == xmlE ) {
        Message = OUString::createFromAscii( "misplaced xml processing instruction" );
    }
    else if( XML_ERROR_UNKNOWN_ENCODING == xmlE ) {
        Message = OUString::createFromAscii( "unknown encoding" );
    }
    else if( XML_ERROR_INCORRECT_ENCODING == xmlE ) {
        Message = OUString::createFromAscii( "incorrect encoding" );
    }
    else if( XML_ERROR_UNCLOSED_CDATA_SECTION == xmlE ) {
        Message = OUString::createFromAscii( "unclosed cdata section" );
    }
    else if( XML_ERROR_EXTERNAL_ENTITY_HANDLING == xmlE ) {
        Message = OUString::createFromAscii( "external entity reference" );
    }
    else if( XML_ERROR_NOT_STANDALONE == xmlE ) {
        Message = OUString::createFromAscii( "not standalone" );
    }

    OUString str = OUString::createFromAscii( "[" );
    str += sSystemId;
    str += OUString::createFromAscii( " line " );
    str += OUString::valueOf( nLine );
    str += OUString::createFromAscii( "]: " );
    str += Message;
    str += OUString::createFromAscii( "error" );

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
            XML_Parse( getEntity().pParser , ( const char * ) seqOut.getArray() , 0 , 1 );
            break;
        }

        sal_Bool bContinue = XML_Parse( getEntity().pParser ,(const char *) seqOut.getArray(),nRead,0);

        if( ! bContinue || this->bExceptionWasThrown ) {

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
            assert( awAttributes[i+1] );
            pImpl->pAttrList->addAttribute(
                XML_CHAR_TO_OUSTRING( awAttributes[i] ) ,
                OUString( RTL_CONSTASCII_USTRINGPARAM("CDATA") ) ,  // expat doesn't know types
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
        CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS( pImpl ,
                rDocumentHandler->processingInstruction( XML_CHAR_TO_OUSTRING( sTarget ),
                                                                XML_CHAR_TO_OUSTRING( sData ) ) );
    }
}


void SaxExpatParser_Impl::callbackUnparsedEntityDecl(void *pvThis ,
                                                  const XML_Char *entityName,
                                                  const XML_Char *base,
                                                  const XML_Char *systemId,
                                                  const XML_Char *publicId,
                                                  const XML_Char *notationName)
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);
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

void SaxExpatParser_Impl::callbackNotationDecl( void *pvThis,
                                                const XML_Char *notationName,
                                                const XML_Char *base,
                                                const XML_Char *systemId,
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



int SaxExpatParser_Impl::callbackExternalEntityRef( XML_Parser parser,
                                                    const XML_Char *context,
                                                    const XML_Char *base,
                                                    const XML_Char *systemId,
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
        catch( SAXParseException & e )
        {
            pImpl->exception = e;
            bOK = sal_False;
        }
        catch( SAXException & e )
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
        catch( SAXParseException & e )
        {
            pImpl->exception = e;
            bOK = sal_False;
        }
        catch( IOException &e )
        {
            pImpl->exception.WrappedException <<= e;
            bOK = sal_False;
        }
        catch( RuntimeException &e )
        {
            pImpl->exception.WrappedException <<=e;
            bOK = sal_False;
        }

        pImpl->popEntity();

        XML_ParserFree( entity.pParser );
    }

    return bOK;
}

int SaxExpatParser_Impl::callbackUnknownEncoding(void *encodingHandlerData,
                                              const XML_Char *name,
                                              XML_Encoding *info)
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
            pImpl->bExceptionWasThrown = sal_True;
        }
    }
    catch( SAXParseException & ex ) {
        pImpl->exception = ex;
        pImpl->bExceptionWasThrown = sal_True;
    }
    catch( SAXException & ex ) {
        pImpl->exception = SAXParseException(
                                    ex.Message,
                                    ex.Context,
                                    ex.WrappedException,
                                    pImpl->rDocumentLocator->getPublicId(),
                                    pImpl->rDocumentLocator->getSystemId(),
                                    pImpl->rDocumentLocator->getLineNumber(),
                                    pImpl->rDocumentLocator->getColumnNumber()
                             );
        pImpl->bExceptionWasThrown = sal_True;
    }
}

void SaxExpatParser_Impl::callbackEndCDATA( void *pvThis )
{
    SaxExpatParser_Impl *pImpl = ((SaxExpatParser_Impl*)pvThis);

    CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS(pImpl,rExtendedDocumentHandler->endCDATA() );
}



extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

            Reference< XRegistryKey > xNewKey = xKey->createKey(
                OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) );
            xNewKey->createKey( OUString::createFromAscii( SERVICE_NAME ) );

            xNewKey = xKey->createKey( OUString::createFromAscii("/") +
                                       SaxWriter_getImplementationName()+
                                       OUString::createFromAscii( "/UNO/SERVICES" ) );
            xNewKey->createKey( SaxWriter_getServiceName() );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}


void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager )
    {
        Reference< XSingleServiceFactory > xRet;
        Reference< XMultiServiceFactory > xSMgr =
            reinterpret_cast< XMultiServiceFactory * > ( pServiceManager );

        OUString aImplementationName = OUString::createFromAscii( pImplName );

        if (aImplementationName ==
            OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME  ) ) )
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

