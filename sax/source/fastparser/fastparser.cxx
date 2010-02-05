/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fastparser.cxx,v $
 * $Revision: 1.4.10.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

//#include <stdlib.h>
//#include <sal/alloca.h>

#include <boost/scoped_ptr.hpp>

#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>

#include "fastparser.hxx"

#include <string.h>

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
//using namespace ::com::sun::star::util;
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
// FastSaxParser implementation
// --------------------------------------------------------------------

FastSaxParser::FastSaxParser()
{
    mxDocumentLocator.set( new FastLocatorImpl( this ) );

    // performance-Improvment. Reference is needed when calling the startTag callback.
    // Handing out the same object with every call is allowed (see sax-specification)
    mxAttributes.set( new FastAttributeList( mxTokenHandler ) );
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
    bool bRootContext = false;
    SaxContextImplPtr p;
    if( maContextStack.empty() )
    {
        p.reset( new SaxContextImpl() );
        bRootContext = true;
    }
    else
    {
        p.reset( new SaxContextImpl( maContextStack.top() ) );
    }
    maContextStack.push( p );

    if( bRootContext )
        DefineNamespace( OString("xml"), "http://www.w3.org/XML/1998/namespace");
}

// --------------------------------------------------------------------

void FastSaxParser::popContext()
{
    OSL_ENSURE( !maContextStack.empty(), "sax::FastSaxParser::popContext(), pop without push?" );
    if( !maContextStack.empty() )
        maContextStack.pop();
}

// --------------------------------------------------------------------

void FastSaxParser::DefineNamespace( const OString& rPrefix, const sal_Char* pNamespaceURL )
{
    OSL_ENSURE( !maContextStack.empty(), "sax::FastSaxParser::DefineNamespace(), I need a context!" );
    if( !maContextStack.empty() )
    {
        sal_uInt32 nOffset = maContextStack.top()->mnNamespaceCount++;

        if( maNamespaceDefines.size() <= nOffset )
            maNamespaceDefines.resize( maNamespaceDefines.size() + 64 );

        const OUString aNamespaceURL( pNamespaceURL, strlen( pNamespaceURL ), RTL_TEXTENCODING_UTF8 );
        maNamespaceDefines[nOffset].reset( new NamespaceDefine( rPrefix, GetNamespaceToken( aNamespaceURL ), aNamespaceURL ) );
    }
}

// --------------------------------------------------------------------

sal_Int32 FastSaxParser::GetToken( const OString& rToken )
{
    Sequence< sal_Int8 > aSeq( (sal_Int8*)rToken.getStr(), rToken.getLength() );

    return mxTokenHandler->getTokenFromUTF8( aSeq );
}

sal_Int32 FastSaxParser::GetToken( const sal_Char* pToken, sal_Int32 nLen /* = 0 */ )
{
    if( !nLen )
        nLen = strlen( pToken );

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pToken, nLen );

    return mxTokenHandler->getTokenFromUTF8( aSeq );
}

// --------------------------------------------------------------------

sal_Int32 FastSaxParser::GetTokenWithPrefix( const OString& rPrefix, const OString& rName ) throw (SAXException)
{
    sal_Int32 nNamespaceToken = FastToken::DONTKNOW;

    sal_uInt32 nNamespace = maContextStack.top()->mnNamespaceCount;
    while( nNamespace-- )
    {
        if( maNamespaceDefines[nNamespace]->maPrefix == rPrefix )
        {
            nNamespaceToken = maNamespaceDefines[nNamespace]->mnToken;
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

    sal_uInt32 nNamespace = maContextStack.top()->mnNamespaceCount;
    while( nNamespace-- )
    {
        const OString& rPrefix( maNamespaceDefines[nNamespace]->maPrefix );
        if( (rPrefix.getLength() == nPrefixLen) &&
            (strncmp( rPrefix.getStr(), pPrefix, nPrefixLen ) == 0 ) )
        {
            nNamespaceToken = maNamespaceDefines[nNamespace]->mnToken;
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
    if( !maContextStack.empty() )
    {
        sal_uInt32 nNamespace = maContextStack.top()->mnNamespaceCount;
        while( nNamespace-- )
            if( maNamespaceDefines[nNamespace]->maPrefix == rPrefix )
                return maNamespaceDefines[nNamespace]->maNamespaceURL;
    }

    throw SAXException(); // prefix that has no defined namespace url
}

OUString FastSaxParser::GetNamespaceURL( const sal_Char*pPrefix, int nPrefixLen ) throw(SAXException)
{
    if( pPrefix && !maContextStack.empty() )
    {
        sal_uInt32 nNamespace = maContextStack.top()->mnNamespaceCount;
        while( nNamespace-- )
        {
            const OString& rPrefix( maNamespaceDefines[nNamespace]->maPrefix );
            if( (rPrefix.getLength() == nPrefixLen) &&
                (strncmp( rPrefix.getStr(), pPrefix, nPrefixLen ) == 0 ) )
            {
                return maNamespaceDefines[nNamespace]->maNamespaceURL;
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

    struct Entity entity;
    entity.maStructSource = maStructSource;

    if( !entity.maStructSource.aInputStream.is() )
        throw SAXException( OUString( RTL_CONSTASCII_USTRINGPARAM( "No input source" ) ), Reference< XInterface >(), Any() );

    entity.maConverter.setInputStream( entity.maStructSource.aInputStream );
    if( entity.maStructSource.sEncoding.getLength() )
        entity.maConverter.setEncoding( OUStringToOString( entity.maStructSource.sEncoding, RTL_TEXTENCODING_ASCII_US ) );

    // create parser with proper encoding
    entity.mpParser = XML_ParserCreate( 0 );
    if( !entity.mpParser )
        throw SAXException( OUString( RTL_CONSTASCII_USTRINGPARAM( "Couldn't create parser" ) ), Reference< XInterface >(), Any() );

    // set all necessary C-Callbacks
    XML_SetUserData( entity.mpParser, this );
    XML_SetElementHandler( entity.mpParser, call_callbackStartElement, call_callbackEndElement );
    XML_SetCharacterDataHandler( entity.mpParser, call_callbackCharacters );
    XML_SetExternalEntityRefHandler( entity.mpParser, call_callbackExternalEntityRef );

    // maSavedException used to transport exceptions through C callbacks
    maSavedException.clear();

    pushEntity( entity );
    try
    {
        // start the document
        if( mxDocumentHandler.is() )
        {
            Reference< XLocator > xLoc( mxDocumentLocator.get() );
            mxDocumentHandler->setDocumentLocator( xLoc );
            mxDocumentHandler->startDocument();
        }

        parse();

        // finish document
        if( mxDocumentHandler.is() )
        {
            mxDocumentHandler->endDocument();
        }
    }
    catch( SAXException & )
    {
        popEntity();
        XML_ParserFree( entity.mpParser );
          throw;
    }
    catch( IOException & )
    {
        popEntity();
        XML_ParserFree( entity.mpParser );
        throw;
    }
    catch( RuntimeException & )
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
    mxDocumentHandler = Handler;
}

void SAL_CALL FastSaxParser::setTokenHandler( const Reference< XFastTokenHandler >& Handler ) throw (RuntimeException)
{
    mxTokenHandler = Handler;
    mxAttributes.set( new FastAttributeList( mxTokenHandler ) );
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

void FastSaxParser::setErrorHandler(const Reference< XErrorHandler > & Handler) throw (RuntimeException)
{
    mxErrorHandler = Handler;
}

void FastSaxParser::setEntityResolver(const Reference < XEntityResolver > & Resolver) throw (RuntimeException)
{
    mxEntityResolver = Resolver;
}

void FastSaxParser::setLocale( const Locale & Locale ) throw (RuntimeException)
{
    maLocale = Locale;
}

Sequence< OUString > FastSaxParser::getSupportedServiceNames_Static(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(PARSER_SERVICE_NAME) );
    return aRet;
}

// XServiceInfo
OUString FastSaxParser::getImplementationName() throw (RuntimeException)
{
    return OUString::createFromAscii( PARSER_IMPLEMENTATION_NAME );
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
    seq.getArray()[0] = OUString::createFromAscii( PARSER_SERVICE_NAME );
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
    aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( " line " ) );
    aBuffer.append( nLine );
    aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "]: " ) );
    aBuffer.appendAscii( pMessage );
    aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( " error" ) );
    return aBuffer.makeStringAndClear();
}

} // namespace

// starts parsing with actual parser !
void FastSaxParser::parse()
{
    const int BUFFER_SIZE = 16 * 1024;
    Sequence< sal_Int8 > seqOut( BUFFER_SIZE );

    int nRead = 0;
    do
    {
        nRead = getEntity().maConverter.readAndConvert( seqOut, BUFFER_SIZE );
        if( nRead <= 0 )
        {
            XML_Parse( getEntity().mpParser, (const char*) seqOut.getConstArray(), 0, 1 );
            break;
        }

        bool bContinue = XML_Parse( getEntity().mpParser, (const char*) seqOut.getConstArray(), nRead, 0 ) != 0;
        // callbacks used inside XML_Parse may have caught an exception
        if( !bContinue || maSavedException.hasValue() )
        {
            // Error during parsing !
            XML_Error xmlE = XML_GetErrorCode( getEntity().mpParser );
            OUString sSystemId = mxDocumentLocator->getSystemId();
            sal_Int32 nLine = mxDocumentLocator->getLineNumber();

            SAXParseException aExcept(
                lclGetErrorMessage( xmlE, sSystemId, nLine ),
                Reference< XInterface >(),
                Any( &maSavedException, getCppuType( &maSavedException ) ),
                mxDocumentLocator->getPublicId(),
                mxDocumentLocator->getSystemId(),
                mxDocumentLocator->getLineNumber(),
                mxDocumentLocator->getColumnNumber()
            );

            // error handler is set, it may throw the exception
            if( mxErrorHandler.is() )
                mxErrorHandler->fatalError( Any( aExcept ) );

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
    if( !maContextStack.empty() )
    {
        xParentContext = maContextStack.top()->mxContext;
        if( !xParentContext.is() )
        {
            // we ignore current elements, so no processing needed
            pushContext();
            return;
        }
    }

    pushContext();

    mxAttributes->clear();

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
            OSL_ASSERT( awAttributes[i+1] );

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
                    maContextStack.top()->maNamespace = OUString( awAttributes[i+1], strlen( awAttributes[i+1] ), RTL_TEXTENCODING_UTF8 );
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
            if( aIt->maPrefix.getLength() > 0 )
            {
                sal_Int32 nAttributeToken = GetTokenWithPrefix( aIt->maPrefix, aIt->maName );
                if( nAttributeToken != FastToken::DONTKNOW )
                    mxAttributes->add( nAttributeToken, aIt->maValue );
                else
                    mxAttributes->addUnknown( GetNamespaceURL( aIt->maPrefix ), aIt->maName, aIt->maValue );
            }
            else
            {
                sal_Int32 nAttributeToken = GetToken( aIt->maName );
                if( nAttributeToken != FastToken::DONTKNOW )
                    mxAttributes->add( nAttributeToken, aIt->maValue );
                else
                    mxAttributes->addUnknown( aIt->maName, aIt->maValue );
            }
        }

        sal_Int32 nElementToken;
        splitName( pwName, pPrefix, nPrefixLen, pName, nNameLen );
        if( nPrefixLen > 0 )
            nElementToken = GetTokenWithPrefix( pPrefix, nPrefixLen, pName, nNameLen );
        else if( maContextStack.top()->maNamespace.getLength() > 0 )
            nElementToken = GetTokenWithNamespaceURL( maContextStack.top()->maNamespace, pName, nNameLen );
        else
            nElementToken = GetToken( pName );
        maContextStack.top()->mnElementToken = nElementToken;

        Reference< XFastAttributeList > xAttr( mxAttributes.get() );
        Reference< XFastContextHandler > xContext;
        if( nElementToken == FastToken::DONTKNOW )
        {
            if( nPrefixLen > 0 )
                maContextStack.top()->maNamespace = GetNamespaceURL( pPrefix, nPrefixLen );

            const OUString aNamespace( maContextStack.top()->maNamespace );
            const OUString aElementName( pPrefix, nPrefixLen, RTL_TEXTENCODING_UTF8 );
            maContextStack.top()->maElementName = aElementName;

            if( xParentContext.is() )
                xContext = xParentContext->createUnknownChildContext( aNamespace, aElementName, xAttr );
            else
                xContext = mxDocumentHandler->createUnknownChildContext( aNamespace, aElementName, xAttr );

            if( xContext.is() )
            {
                maContextStack.top()->mxContext = xContext;
                xContext->startUnknownElement( aNamespace, aElementName, xAttr );
            }
        }
        else
        {
            if( xParentContext.is() )
                xContext = xParentContext->createFastChildContext( nElementToken, xAttr );
            else
                xContext = mxDocumentHandler->createFastChildContext( nElementToken, xAttr );


            if( xContext.is() )
            {
                maContextStack.top()->mxContext = xContext;
                xContext->startFastElement( nElementToken, xAttr );
            }
        }
    }
    catch( Exception& e )
    {
        maSavedException <<= e;
    }
}

void FastSaxParser::callbackEndElement( const XML_Char* )
{
    OSL_ENSURE( !maContextStack.empty(), "FastSaxParser::callbackEndElement - no context" );
    if( !maContextStack.empty() )
    {
        SaxContextImplPtr pContext( maContextStack.top() );
        const Reference< XFastContextHandler >& xContext( pContext->mxContext );
        if( xContext.is() ) try
        {
            sal_Int32 nElementToken = pContext->mnElementToken;
            if( nElementToken != FastToken::DONTKNOW )
                xContext->endFastElement( nElementToken );
            else
                xContext->endUnknownElement( pContext->maNamespace, pContext->maElementName );
        }
        catch( Exception& e )
        {
            maSavedException <<= e;
        }

        popContext();
    }
}


void FastSaxParser::callbackCharacters( const XML_Char* s, int nLen )
{
    const Reference< XFastContextHandler >& xContext( maContextStack.top()->mxContext );
    if( xContext.is() ) try
    {
        xContext->characters( OUString( s, nLen, RTL_TEXTENCODING_UTF8 ) );
    }
    catch( Exception& e )
    {
        maSavedException <<= e;
    }
}

int FastSaxParser::callbackExternalEntityRef( XML_Parser parser,
        const XML_Char *context, const XML_Char * /*base*/, const XML_Char *systemId, const XML_Char *publicId )
{
    bool bOK = true;
    InputSource source;

    struct Entity entity;

    if( mxEntityResolver.is() ) try
    {
        entity.maStructSource = mxEntityResolver->resolveEntity(
            OUString( publicId, strlen( publicId ), RTL_TEXTENCODING_UTF8 ) ,
            OUString( systemId, strlen( systemId ), RTL_TEXTENCODING_UTF8 ) );
    }
    catch( SAXParseException & e )
    {
        maSavedException <<= e;
        bOK = false;
    }
    catch( SAXException & e )
    {
        maSavedException <<= SAXParseException(
            e.Message, e.Context, e.WrappedException,
            mxDocumentLocator->getPublicId(),
            mxDocumentLocator->getSystemId(),
            mxDocumentLocator->getLineNumber(),
            mxDocumentLocator->getColumnNumber() );
        bOK = false;
    }

    if( entity.maStructSource.aInputStream.is() )
    {
        entity.mpParser = XML_ExternalEntityParserCreate( parser, context, 0 );
        if( ! entity.mpParser )
        {
            return false;
        }

        entity.maConverter.setInputStream( entity.maStructSource.aInputStream );
        pushEntity( entity );
        try
        {
            parse();
        }
        catch( SAXParseException & e )
        {
            maSavedException <<= e;
            bOK = false;
        }
        catch( IOException &e )
        {
            SAXException aEx;
            aEx.WrappedException <<= e;
            maSavedException <<= aEx;
            bOK = false;
        }
        catch( RuntimeException &e )
        {
            SAXException aEx;
            aEx.WrappedException <<= e;
            maSavedException <<= aEx;
            bOK = false;
        }

        popEntity();

        XML_ParserFree( entity.mpParser );
    }

    return bOK;
}

} // namespace sax_fastparser
