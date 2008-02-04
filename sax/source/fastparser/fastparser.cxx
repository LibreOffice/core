/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fastparser.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:40:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

//#include <stdlib.h>
//#include <sal/alloca.h>

#include <stack>
#include <vector>
#include <hash_map>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <osl/diagnose.h>
#include <rtl/ref.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase2.hxx>

#include "fastattribs.hxx"
#include "xml2utf.hxx"

#ifdef SYSTEM_EXPAT
#include <expat.h>
#else
#include "expat/xmlparse.h"
#endif
#include <string.h>

using ::rtl::OUString;
using ::rtl::OString;
using namespace ::std;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml::sax;
//using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
namespace sax_fastparser
{

// --------------------------------------------------------------------

struct SaxContextImpl;
typedef boost::shared_ptr< SaxContextImpl > SaxContextImplPtr;

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

// Entity binds all information neede for a single file
struct Entity
{
    InputSource         maStructSource;
    XML_Parser          mpParser;
    sax_expatwrap::XMLFile2UTFConverter maConverter;
};

// --------------------------------------------------------------------

struct NamespaceDefine
{
    OString     maPrefix;
    sal_Int32   mnToken;
    OUString    maNamespaceURL;

    NamespaceDefine( const OString& rPrefix, sal_Int32 nToken, const OUString& rNamespaceURL ) : maPrefix( rPrefix ), mnToken( nToken ), maNamespaceURL( rNamespaceURL ) {}
};

typedef ::std::hash_map< ::rtl::OUString, sal_Int32,
                         ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString >
                       > NamespaceMap;

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

#define IMPLEMENTATION_NAME "com.sun.star.comp.extensions.xml.sax.FastParser"
#define SERVICE_NAME        "com.sun.star.xml.sax.FastParser"


// This class implements the external Parser interface
class FastSaxParser : public WeakImplHelper2< XFastParser, XServiceInfo >
{
public:
    FastSaxParser();
    ~FastSaxParser();

    // The implementation details
    static Sequence< OUString >     getSupportedServiceNames_Static(void);
    static OUString                 getImplementationName_Static();

    // XFastParser
    virtual void SAL_CALL parseStream( const InputSource& aInputSource ) throw (SAXException, IOException, RuntimeException);
    virtual void SAL_CALL setFastDocumentHandler( const Reference< XFastDocumentHandler >& Handler ) throw (RuntimeException);
    virtual void SAL_CALL setTokenHandler( const Reference< XFastTokenHandler >& Handler ) throw (RuntimeException);
    virtual void SAL_CALL registerNamespace( const OUString& NamespaceURL, sal_Int32 NamespaceToken ) throw (IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL setErrorHandler( const Reference< XErrorHandler >& Handler ) throw (RuntimeException);
    virtual void SAL_CALL setEntityResolver( const Reference< XEntityResolver >& Resolver ) throw (RuntimeException);
    virtual void SAL_CALL setLocale( const Locale& Locale ) throw (RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

public:
    // the C-Callbacks for the expat parser
    void static callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts);
    void static callbackEndElement(void *userData, const XML_Char *name);
    void static callbackCharacters( void *userData , const XML_Char *s , int nLen );
    int static callbackExternalEntityRef( XML_Parser parser, const XML_Char *openEntityNames, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId);

public:
    void pushEntity( const struct Entity &entity )  { vecEntity.push_back( entity ); }
    void popEntity()                                { vecEntity.pop_back( ); }
    struct Entity &getEntity()                      { return vecEntity.back(); }

private:
    void parse();

    sal_Int32 GetToken( const sal_Char* pToken, sal_Int32 nTokenLen = 0 );
    sal_Int32 GetTokenWithPrefix( const sal_Char*pPrefix, int nPrefixLen, const sal_Char* pName, int nNameLen ) throw (SAXException);
    OUString  GetNamespaceURL( const sal_Char*pPrefix, int nPrefixLen ) throw (SAXException);
    sal_Int32 GetNamespaceToken( const OUString& rNamespaceURL );
    sal_Int32 GetTokenWithNamespaceURL( const OUString& rNamespaceURL, const sal_Char* pName, int nNameLen );
    void DefineNamespace( const OString& rPrefix, const sal_Char* pNamespaceURL );
    sal_Int32 CreateCustomToken( const sal_Char* pToken, int len = 0 );

    void pushContext();
    void popContext();

    void splitName( const XML_Char *pwName, const XML_Char *&rpPrefix, sal_Int32 &rPrefixLen, const XML_Char *&rpName, sal_Int32 &rNameLen );

private:
    Mutex               maMutex;

    Reference< XFastDocumentHandler >       mxDocumentHandler;
    Reference< XFastTokenHandler >          mxTokenHandler;
    Reference< XErrorHandler >              mxErrorHandler;
    Reference< XEntityResolver >            mxEntityResolver;
    rtl::Reference < FastLocatorImpl >      mxDocumentLocator;

    rtl::Reference < FastAttributeList >    mxAttributes;

    // External entity stack
    vector<struct Entity>   vecEntity;

    // Exception cannot be thrown through the C-XmlParser (possible resource leaks),
    // therefor the maSavedException must be saved somewhere.
    Any                 maSavedException;
    sal_Bool            mbExceptionWasThrown;

    Locale              maLocale;

    std::stack< SaxContextImplPtr > maContextStack;
    std::vector< boost::shared_ptr< NamespaceDefine > > maNamespaceDefines;
    NamespaceMap maNamespaceMap;
};

//--------------------------------------
// the extern interface
//---------------------------------------
Reference< XInterface > SAL_CALL FastSaxParser_CreateInstance( const Reference< XMultiServiceFactory  >  & ) throw(Exception)
{
    FastSaxParser *p = new FastSaxParser;
    return Reference< XInterface > ( (OWeakObject * ) p );
}

Sequence< OUString > FastSaxParser::getSupportedServiceNames_Static(void)
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(SERVICE_NAME) );
    return aRet;
}

//---------------------------------------------
// the implementation part
//---------------------------------------------

extern "C"
{
    static void call_callbackStartElement(void *userData, const XML_Char *name , const XML_Char **atts)
    {
        FastSaxParser::callbackStartElement(userData,name,atts);
    }
    static void call_callbackEndElement(void *userData, const XML_Char *name)
    {
        FastSaxParser::callbackEndElement(userData,name);
    }
    static void call_callbackCharacters( void *userData , const XML_Char *s , int nLen )
    {
        FastSaxParser::callbackCharacters(userData,s,nLen);
    }
    static int call_callbackExternalEntityRef(XML_Parser parser,
                                              const XML_Char *openEntityNames,
                                              const XML_Char *base,
                                              const XML_Char *systemId,
                                              const XML_Char *publicId)
    {
        return FastSaxParser::callbackExternalEntityRef(parser,openEntityNames,base,systemId,publicId);
    }
}

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

FastSaxParser::FastSaxParser(  )
{
    mxDocumentLocator.set( new FastLocatorImpl( this ) );

    // performance-Improvment. Reference is needed when calling the startTag callback.
    // Handing out the same object with every call is allowed (see sax-specification)
    mxAttributes.set( new FastAttributeList( mxTokenHandler ) );

    mbExceptionWasThrown = sal_False;
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

sal_Int32 FastSaxParser::GetToken( const sal_Char* pToken, sal_Int32 nLen /* = 0 */ )
{
    if( !nLen )
        nLen = strlen( pToken );

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pToken, nLen );

    return mxTokenHandler->getTokenFromUTF8( aSeq );
}

// --------------------------------------------------------------------

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

    if( ! entity.maStructSource.aInputStream.is() )
    {
        throw SAXException( OUString::createFromAscii( "No input source" ), Reference< XInterface > () , Any() );
    }

    entity.maConverter.setInputStream( entity.maStructSource.aInputStream );
    if( entity.maStructSource.sEncoding.getLength() )
    {
        entity.maConverter.setEncoding( OUStringToOString( entity.maStructSource.sEncoding , RTL_TEXTENCODING_ASCII_US ) );
    }

    // create parser with proper encoding
    entity.mpParser = XML_ParserCreate( 0 );
    if( ! entity.mpParser )
    {
        throw SAXException( OUString::createFromAscii( "Couldn't create parser" ), Reference< XInterface > (), Any() );
    }

    // set all necessary C-Callbacks
    XML_SetUserData( entity.mpParser , this );
    XML_SetElementHandler(  entity.mpParser ,
                            call_callbackStartElement ,
                            call_callbackEndElement );
    XML_SetCharacterDataHandler( entity.mpParser , call_callbackCharacters );
    XML_SetExternalEntityRefHandler(    entity.mpParser,
                                        call_callbackExternalEntityRef);

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

OUString FastSaxParser::getImplementationName_Static()
{
    return OUString::createFromAscii( IMPLEMENTATION_NAME );
}

// XServiceInfo
OUString FastSaxParser::getImplementationName() throw (RuntimeException)
{
    return OUString::createFromAscii( IMPLEMENTATION_NAME );
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
    seq.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
    return seq;
}


/*---------------------------------------
*
* Helper functions and classes
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
void FastSaxParser::parse( )
{
    const int nBufSize = 16*1024;

    int nRead = nBufSize;
    Sequence< sal_Int8 > seqOut(nBufSize);

    while( nRead )
    {
        nRead = getEntity().maConverter.readAndConvert( seqOut, nBufSize );

        if( ! nRead )
        {
            XML_Parse( getEntity().mpParser, ( const char * ) seqOut.getArray(), 0 , 1 );
            break;
        }

        sal_Bool bContinue = ( XML_Parse( getEntity().mpParser, (const char *) seqOut.getArray(), nRead, 0 ) != 0 );

        if( !bContinue || mbExceptionWasThrown )
        {
            // Error during parsing !
            XML_Error xmlE = XML_GetErrorCode( getEntity().mpParser );
            OUString sSystemId = mxDocumentLocator->getSystemId();
            sal_Int32 nLine = mxDocumentLocator->getLineNumber();

            SAXParseException aExcept(
                getErrorMessage(xmlE , sSystemId, nLine) ,
                Reference< XInterface >(),
                Any( &maSavedException , getCppuType( &maSavedException) ),
                mxDocumentLocator->getPublicId(),
                mxDocumentLocator->getSystemId(),
                mxDocumentLocator->getLineNumber(),
                mxDocumentLocator->getColumnNumber()
                );

            if( mxErrorHandler.is() )
            {
                // error handler is set, so the handler may throw the maSavedException
                mxErrorHandler->fatalError( Any( aExcept ) );
            }

            // Error handler has not thrown an maSavedException, but parsing cannot go on,
            // so an maSavedException MUST be thrown.
            throw aExcept;
        }
    }
}

//------------------------------------------
//
// The C-Callbacks
//
//-----------------------------------------
void FastSaxParser::callbackStartElement( void *pvThis, const XML_Char *pwName, const XML_Char **awAttributes )
{
     FastSaxParser *pThis = (FastSaxParser*)pvThis;

    Reference< XFastContextHandler > xParentContext;
    if( !pThis->maContextStack.empty() )
    {
        xParentContext = pThis->maContextStack.top()->mxContext;
        if( !xParentContext.is() )
        {
            // we ignore current elements, so no processing needed
            pThis->pushContext();
            return;
        }
    }

    pThis->pushContext();

    pThis->mxAttributes->clear();

    // create attribute map and process namespace instructions
    int i = 0;
    sal_Int32 nNameLen, nPrefixLen;
    const XML_Char *pName;
    const XML_Char *pPrefix;
    for( ; awAttributes[i]; i += 2 )
    {
        OSL_ASSERT( awAttributes[i+1] );

        pThis->splitName( awAttributes[i], pPrefix, nPrefixLen, pName, nNameLen );
        if( nPrefixLen )
        {
            if( (nPrefixLen == 5) && (strncmp( pPrefix, "xmlns", 5 ) == 0) )
            {
                pThis->DefineNamespace( OString( pName, nNameLen ), awAttributes[i+1] );
            }
            else
            {
                const sal_Int32 nAttributeToken = pThis->GetTokenWithPrefix( pPrefix, nPrefixLen, pName, nNameLen );
                if( nAttributeToken != FastToken::DONTKNOW )
                {
                    pThis->mxAttributes->add( nAttributeToken, OString( awAttributes[i+1] ) );
                }
                else
                {
                    pThis->mxAttributes->addUnknown( pThis->GetNamespaceURL( pPrefix, nPrefixLen ), OString( pName, nNameLen ), OString( awAttributes[i+1] ) );
                }
            }
        }
        else
        {
            if( (nNameLen == 5) && (strcmp( pName, "xmlns" ) == 0) )
            {
                // namespace of the element found
                pThis->maContextStack.top()->maNamespace = OUString( awAttributes[i+1], strlen( awAttributes[i+1] ), RTL_TEXTENCODING_UTF8 );
            }
            else
            {
                const sal_Int32 nAttributeToken = pThis->GetToken( pName, nNameLen );
                if( nAttributeToken != FastToken::DONTKNOW )
                {
                    pThis->mxAttributes->add( nAttributeToken, OString( awAttributes[i+1] ) );
                }
                else
                {
                    pThis->mxAttributes->addUnknown( OString( pName, nNameLen ), OString( awAttributes[i+1] ) );
                }
            }
        }
    }

    sal_Int32 nElementToken;
    pThis->splitName( pwName, pPrefix, nPrefixLen, pName, nNameLen );
    if( nPrefixLen )
    {
        nElementToken = pThis->GetTokenWithPrefix( pPrefix, nPrefixLen, pName, nNameLen );
    }
    else if( pThis->maContextStack.top()->maNamespace.getLength() )
    {
        nElementToken = pThis->GetTokenWithNamespaceURL( pThis->maContextStack.top()->maNamespace, pName, nNameLen );
    }
    else
    {
        nElementToken = pThis->GetToken( pName );
    }
    pThis->maContextStack.top()->mnElementToken = nElementToken;

    try
    {
        Reference< XFastAttributeList > xAttr( pThis->mxAttributes.get() );
        Reference< XFastContextHandler > xContext;
        if( nElementToken == FastToken::DONTKNOW )
        {
            if( nPrefixLen )
            {
                pThis->maContextStack.top()->maNamespace = pThis->GetNamespaceURL( pPrefix, nPrefixLen );
            }

            const OUString aNamespace( pThis->maContextStack.top()->maNamespace );
            const OUString aElementName( pPrefix, nPrefixLen, RTL_TEXTENCODING_UTF8 );
            pThis->maContextStack.top()->maElementName = aElementName;

            if( xParentContext.is() )
                xContext = xParentContext->createUnknownChildContext( aNamespace, aElementName, xAttr );
            else
                xContext = pThis->mxDocumentHandler->createUnknownChildContext( aNamespace, aElementName, xAttr );

            if( xContext.is() )
            {
                pThis->maContextStack.top()->mxContext = xContext;
                xContext->startUnknownElement( aNamespace, aElementName, xAttr );
            }
        }
        else
        {
            if( xParentContext.is() )
                xContext = xParentContext->createFastChildContext( nElementToken, xAttr );
            else
                xContext = pThis->mxDocumentHandler->createFastChildContext( nElementToken, xAttr );


            if( xContext.is() )
            {
                pThis->maContextStack.top()->mxContext = xContext;
                xContext->startFastElement( nElementToken, xAttr );
            }
        }
    }
    catch( Exception& e )
    {
        pThis->maSavedException <<= e;
    }
}

void FastSaxParser::callbackEndElement( void *pvThis , const XML_Char *  )
{
    FastSaxParser *pThis = (FastSaxParser*)pvThis;

    if( !pThis->maContextStack.empty() )
    {
        SaxContextImplPtr pContext( pThis->maContextStack.top() );
        const Reference< XFastContextHandler >& xContext( pContext->mxContext );
        if( xContext.is() ) try
        {
            sal_Int32 nElementToken = pContext->mnElementToken;
            if( nElementToken != FastToken::DONTKNOW )
            {
                xContext->endFastElement( nElementToken );
            }
            else
            {
                xContext->endUnknownElement( pContext->maNamespace, pContext->maElementName );
            }
        }
        catch( Exception& e )
        {
            pThis->maSavedException <<= e;
        }

        pThis->popContext();
    }
    else
    {
        OSL_ENSURE( false, "no context on sax::FastSaxParser::callbackEndElement() ??? ");
    }
}


void FastSaxParser::callbackCharacters( void *pvThis , const XML_Char *s , int nLen )
{
    FastSaxParser *pThis = (FastSaxParser*)pvThis;

    const Reference< XFastContextHandler >& xContext( pThis->maContextStack.top()->mxContext );
    if( xContext.is() ) try
    {
        xContext->characters( OUString( s, nLen, RTL_TEXTENCODING_UTF8 ) );
    }
    catch( Exception& e )
    {
        pThis->maSavedException <<= e;
    }
}

int FastSaxParser::callbackExternalEntityRef( XML_Parser parser,
                                                    const XML_Char *context,
                                                    const XML_Char * /*base*/,
                                                    const XML_Char *systemId,
                                                    const XML_Char *publicId)
{
    bool bOK = true;
    InputSource source;
    FastSaxParser *pImpl = ((FastSaxParser*)XML_GetUserData( parser ));

    struct Entity entity;

    if( pImpl->mxEntityResolver.is() ) {
        try
        {
            entity.maStructSource = pImpl->mxEntityResolver->resolveEntity(
                OUString( publicId, strlen( publicId ), RTL_TEXTENCODING_UTF8 ) ,
                OUString( systemId, strlen( systemId ), RTL_TEXTENCODING_UTF8 ) );
        }
        catch( SAXParseException & e )
        {
            pImpl->maSavedException <<= e;
            bOK = false;
        }
        catch( SAXException & e )
        {
            pImpl->maSavedException <<= SAXParseException(
                e.Message , e.Context , e.WrappedException ,
                pImpl->mxDocumentLocator->getPublicId(),
                pImpl->mxDocumentLocator->getSystemId(),
                pImpl->mxDocumentLocator->getLineNumber(),
                pImpl->mxDocumentLocator->getColumnNumber() );
            bOK = false;
        }
    }

    if( entity.maStructSource.aInputStream.is() )
    {
        entity.mpParser = XML_ExternalEntityParserCreate( parser , context, 0 );
        if( ! entity.mpParser )
        {
            return false;
        }

        entity.maConverter.setInputStream( entity.maStructSource.aInputStream );
        pImpl->pushEntity( entity );
        try
        {
            pImpl->parse();
        }
        catch( SAXParseException & e )
        {
            pImpl->maSavedException <<= e;
            bOK = false;
        }
        catch( IOException &e )
        {
            SAXException aEx;
            aEx.WrappedException <<= e;
            pImpl->maSavedException <<= aEx;
            bOK = false;
        }
        catch( RuntimeException &e )
        {
            SAXException aEx;
            aEx.WrappedException <<= e;
            pImpl->maSavedException <<= aEx;
            bOK = false;
        }

        pImpl->popEntity();

        XML_ParserFree( entity.mpParser );
    }

    return bOK;
}

}

using namespace sax_fastparser;

extern "C"
{

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

            Reference< XRegistryKey > xNewKey( xKey->createKey(
                OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );
            xNewKey->createKey( OUString::createFromAscii( SERVICE_NAME ) );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    if (pServiceManager )
    {
        Reference< XSingleServiceFactory > xRet;
        Reference< XMultiServiceFactory > xSMgr( reinterpret_cast< XMultiServiceFactory * > ( pServiceManager ) );

        OUString aImplementationName( OUString::createFromAscii( pImplName ) );

        if (aImplementationName == OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME  ) ) )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        FastSaxParser_CreateInstance,
                                        FastSaxParser::getSupportedServiceNames_Static() );
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
