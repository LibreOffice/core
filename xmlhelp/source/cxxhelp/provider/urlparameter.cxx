/*************************************************************************
 *
 *  $RCSfile: urlparameter.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: abi $ $Date: 2001-05-29 15:14:49 $
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

#ifndef SablotHIncl
#include <sablot.h>
#endif
#ifndef ShandlerHIncl
#include <shandler.h>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _DB_CXX_H_
#include <berkeleydb/db_cxx.h>
#endif
#ifndef _URLPARAMETER_HXX_
#include <provider/urlparameter.hxx>
#endif
#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIER_HPP_
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif

namespace chelp {

    inline bool ascii_isDigit( sal_Unicode ch )
    {
        return ((ch >= 0x0030) && (ch <= 0x0039));
    }

    inline bool ascii_isLetter( sal_Unicode ch )
    {
        return (( (ch >= 0x0041) && (ch <= 0x005A)) || ((ch >=
                                                         0x0061) && (ch <= 0x007A)));
    }

    inline bool isLetterOrDigit( sal_Unicode ch )
    {
        return ascii_isLetter( ch ) || ascii_isDigit( ch );
    }

}

using namespace cppu;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace chelp;


URLParameter::URLParameter( const rtl::OUString& aURL ) throw( com::sun::star::ucb::IllegalIdentifierException )
    : m_aURL( aURL )
{
    init( false );
    parse();
}


URLParameter::URLParameter( const rtl::OUString&  aURL,
                            const rtl::OUString& aDefaultLanguage ) throw( com::sun::star::ucb::IllegalIdentifierException )
    : m_aURL( aURL ),
      m_aDefaultLanguage( aDefaultLanguage )
{
    init( true );
    parse();
}


rtl::OUString URLParameter::get_id()
{
    if( m_aId.compareToAscii("start") == 0 )
    {   // module is set
        StaticModuleInformation* inf =
            Databases::getStaticInformationForModule( get_module(),
                                                          get_language() );
        if( inf )
            m_aId = inf->get_id();

        m_bStart = true;
    }

    return m_aId;
}

rtl::OUString URLParameter::get_tag()
{
    if( isFile() )
        return get_the_tag();
    else
        return m_aTag;
}


rtl::OUString URLParameter::get_title()
{
    if( isFile() )
        return get_the_title();
    else if( m_aModule.compareToAscii("") != 0 )
    {
        StaticModuleInformation* inf =
            Databases::getStaticInformationForModule( get_module(),
                                                      get_language() );
        if( inf )
            m_aTitle = inf->get_title();
    }
    else   // This must be the root
        m_aTitle = rtl::OUString::createFromAscii("root");

    return m_aTitle;
}


rtl::OUString URLParameter::get_language()
{
    if( m_aLanguage.getLength() == 0 )
        return m_aDefaultLanguage;

    return m_aLanguage;
}


rtl::OUString URLParameter::get_program()
{
    if( m_aProgram.compareToAscii( "" ) != 0 )
    {
        StaticModuleInformation* inf =
            Databases::getStaticInformationForModule( get_module(),
                                                          get_language() );
        if( inf )
            m_aProgram = inf->get_program();
    }
    return m_aProgram;
}


void URLParameter::init( bool bDefaultLanguageIsInitialized )
{
    m_bBerkeleyRead = false;
    m_bStart = false;
    m_nHitCount = 100;                // The default maximum hitcount
}


rtl::OUString URLParameter::get_the_tag()
{
    if( ! m_bBerkeleyRead )
        readBerkeley();

    m_bBerkeleyRead = true;

    return m_aTag;
}



rtl::OUString URLParameter::get_the_path()
{
    if( ! m_bBerkeleyRead )
        readBerkeley();
    m_bBerkeleyRead = true;

    return m_aPath;
}



rtl::OUString URLParameter::get_the_title()
{
    if( ! m_bBerkeleyRead )
        readBerkeley();
    m_bBerkeleyRead = true;

    return m_aTitle;
}


rtl::OUString URLParameter::get_the_jar()
{
    if( ! m_bBerkeleyRead )
        readBerkeley();
    m_bBerkeleyRead = true;

    return m_aJar;
}




void URLParameter::readBerkeley()
{
    if( get_id().compareToAscii("") != 0 )
    {
        Db* db = Databases::getBerkeley( get_module(),
                                         get_language() );

        rtl::OString keyStr( m_aId.getStr(),m_aId.getLength(),RTL_TEXTENCODING_UTF8 );
        Dbt key( static_cast< void* >( const_cast< sal_Char* >( keyStr.getStr() ) ),
                 keyStr.getLength() );
        Dbt data;

        int err = db->get( 0,&key,&data,0 );

        DbtToStringConverter converter( static_cast< sal_Char* >( data.get_data() ),
                                        data.get_size() );
        m_aTitle = converter.getTitle();
        m_aPath  = converter.getFile();
        m_aJar   = converter.getDatabase();
        m_aTag   = converter.getHash();
    }
}



// Class encapsulating the transformation of the XInputStream to XHTML


class InputStreamTransformer
    : public OWeakObject,
      public XInputStream,
      public XSeekable
{
public:

    InputStreamTransformer( const Reference< XMultiServiceFactory >& rxSMgr,
                            URLParameter* urlParam );
//  const rtl::OUString& aUri );

    ~InputStreamTransformer();

    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException );
    virtual void SAL_CALL acquire( void ) throw( RuntimeException );
    virtual void SAL_CALL release( void ) throw( RuntimeException );

    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead )
        throw( NotConnectedException,
               BufferSizeExceededException,
               IOException,
               RuntimeException);

    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
        throw( NotConnectedException,
               BufferSizeExceededException,
               IOException,
               RuntimeException);

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw( NotConnectedException,
                                                                     BufferSizeExceededException,
                                                                     IOException,
                                                                     RuntimeException );

    virtual sal_Int32 SAL_CALL available( void ) throw( NotConnectedException,
                                                        IOException,
                                                        RuntimeException );

    virtual void SAL_CALL closeInput( void ) throw( NotConnectedException,
                                                    IOException,
                                                    RuntimeException );

    virtual void SAL_CALL seek( sal_Int64 location ) throw( IllegalArgumentException,
                                                            IOException,
                                                            RuntimeException );

    virtual sal_Int64 SAL_CALL getPosition( void ) throw( IOException,RuntimeException );

    virtual sal_Int64 SAL_CALL getLength( void ) throw( IOException,RuntimeException );

    void addToBuffer( const char* buffer,int len );


private:

    int len,pos;
    char *buffer;

      Reference< XInputStream > m_xInputStream;
      Reference< XSeekable >    m_xSeekable;
};




void URLParameter::open( const Reference< XMultiServiceFactory >& rxSMgr,
                         const Command& aCommand,
                         sal_Int32 CommandId,
                         const Reference< XCommandEnvironment >& Environment,
                         const Reference< XActiveDataSink >& xDataSink )
{
    bool IsRoot;

    if( ( IsRoot = isRoot() ) || isPicture() )
    {
        rtl::OUString url;

        if( IsRoot )
            url =
                Databases::getInstallPathAsURL()         +
                rtl::OUString::createFromAscii( "custom.css" );
        else
        {
            url = rtl::OUString::createFromAscii( "vnd.sun.star.pkg://" );

            rtl::OUString jar =
                Databases::getInstallPathAsURL()         +
                get_language()                           +
                rtl::OUString::createFromAscii( "/picture.jar" );

            url+= rtl::Uri::encode( jar,
                                    rtl_UriCharClassUricNoSlash,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8 );

            url += ( rtl::OUString::createFromAscii( "/" ) + get_path() );
        }

        rtl::OUString service = rtl::OUString::createFromAscii( "com.sun.star.ucb.UniversalContentBroker" );
        Reference< XContentProvider > provider( rxSMgr->createInstance( service ),UNO_QUERY );
        Reference< XContentIdentifierFactory > factory( provider,UNO_QUERY );
        Reference< XContentIdentifier > xIdentifier = factory->createContentIdentifier( url );
        Reference< XContent > xContent = provider->queryContent( xIdentifier );
        Reference< XCommandProcessor > processor( xContent,UNO_QUERY );

        processor->execute( aCommand,
                            CommandId,
                            Environment );
    }
    else
    {   // a standard document, plug in the new input stream
        xDataSink->setInputStream( new InputStreamTransformer( rxSMgr,this ) );
    }
}



void URLParameter::parse() throw( com::sun::star::ucb::IllegalIdentifierException )
{
    m_aExpr = m_aURL;

    sal_Int32 lstIdx = m_aExpr.lastIndexOf( sal_Unicode( '#' ) );
    if( lstIdx != -1 )
        m_aExpr = m_aExpr.copy( 0,lstIdx );

    if( ! scheme() || ! name( module() ) || ! query() )
        throw com::sun::star::ucb::IllegalIdentifierException();
}



bool URLParameter::scheme()
{
#define PREFIX_LENGTH 20
    if( m_aExpr.compareToAscii( "vnd.sun.star.help://",PREFIX_LENGTH ) == 0 )
    {
        m_aExpr = m_aExpr.copy( PREFIX_LENGTH );
#undef PREFIX_LENGTH
        return true;
    }
#define PREFIX_LENGTH 19
    else if( m_aExpr.compareToAscii( "vnd.sun.star.help:/",PREFIX_LENGTH ) == 0 )
    {
        m_aExpr = m_aExpr.copy( PREFIX_LENGTH );
#undef PREFIX_LENGTH
        return true;
    }
#define PREFIX_LENGTH 18
    else if( m_aExpr.compareToAscii( "vnd.sun.star.help:",PREFIX_LENGTH ) == 0 )
    {
        m_aExpr = m_aExpr.copy( PREFIX_LENGTH );
#undef PREFIX_LENGTH
        return true;
    }
    else
        return false;
}



bool URLParameter::module()
{
    sal_Int32 idx = 0,length = m_aExpr.getLength();

    while( idx < length && isLetterOrDigit( (m_aExpr.getStr())[idx] ) )
        ++idx;

    if( idx != 0 )
    {
        m_aModule = m_aExpr.copy( 0,idx );
        m_aExpr = m_aExpr.copy( idx );
        return true;
    }
    else
        return false;
}



bool URLParameter::name( bool modulePresent )
{
    // if modulepresent, a name may be present, but must not

    sal_Int32 length = m_aExpr.getLength();


    if( length != 0 && (m_aExpr.getStr())[0] == sal_Unicode( '/' ) )
    {
        sal_Int32 idx = 1;
        while( idx < length && isLetterOrDigit( (m_aExpr.getStr())[idx] ) )
            ++idx;

        if( idx != 1 && ! modulePresent )
            return false;
        else
        {
            m_aId = m_aExpr.copy( 1,idx-1 );
            m_aExpr = m_aExpr.copy( idx );
        }
    }

    return true;
}


bool URLParameter::query()
{
    rtl::OUString query;

    if( ! m_aExpr.getLength() )
        return true;
    else if( (m_aExpr.getStr())[0] == sal_Unicode( '?' ) )
        query = m_aExpr.copy( 1 ).trim();
    else
        return false;


    bool ret = true;
    sal_Int32 delimIdx,equalIdx;
    rtl::OUString parameter,value;

    while( query.getLength() != 0 )
    {
        delimIdx = query.indexOf( sal_Unicode( '&' ) );
        equalIdx = query.indexOf( sal_Unicode( '=' ) );
        parameter = query.copy( 0,equalIdx ).trim();
        if( delimIdx == -1 )
        {
            value = query.copy( equalIdx + 1 ).trim();
            query = rtl::OUString();
        }
        else
        {
            value = query.copy( equalIdx+1,delimIdx - equalIdx - 1 ).trim();
            query = query.copy( delimIdx+1 ).trim();
        }

        if( parameter.compareToAscii( "Language" ) == 0 )
            m_aLanguage = value;
        else if( parameter.compareToAscii( "Device" ) == 0 )
            m_aDevice = value;
        else if( parameter.compareToAscii( "Program" ) == 0 )
            m_aProgram = value;
        else if( parameter.compareToAscii( "Eid" ) == 0 )
            m_aEid = value;
        else if( parameter.compareToAscii( "Query" ) == 0 )
        {
            if( ! m_aQuery.getLength() )
                m_aQuery = value;
            else
                m_aQuery += ( rtl::OUString::createFromAscii( " " ) + value );
        }
        else if( parameter.compareToAscii( "Scope" ) == 0 )
            m_aScope = value;
        else if( parameter.compareToAscii( "System" ) == 0 )
        {
//          if( ! value.getLength() )
//                  m_aSystem = rtl::OUString::createFromAscii( "WIN" );
//          else
            m_aSystem = value;
        }
        else if( parameter.compareToAscii( "HelpPrefix" ) == 0 )
            m_aPrefix = value;
        else if( parameter.compareToAscii( "HitCount" ) == 0 )
            m_nHitCount = value.toInt32();
        else if( parameter.compareToAscii( "Active" ) == 0 )
            m_aActive = value;
        else
            ret = false;
    }

    return ret;
}




////////////////////////////////////////////////////////////////////////////////
//                           InutStreamTransformerImpl                        //
////////////////////////////////////////////////////////////////////////////////

int schemehandlergetall( void *userData,
                         SablotHandle processor_,
                         const char *scheme,
                         const char *rest,
                         char **buffer,
                         int *byteCount);
int schemehandlerfreememory( void *userData,
                             SablotHandle processor_,
                             char *buffer );
int schemehandleropen( void *userData,
                       SablotHandle processor_,
                       const char *scheme,
                       const char *rest,
                       int *handle );
int schemehandlerget( void *userData,
                      SablotHandle processor_,
                      int handle,
                      char *buffer,
                      int *byteCount );
int schemehandlerput( void *userData,
                      SablotHandle processor_,
                      int handle,
                      const char *buffer,
                      int *byteCount );
int schemehandlerclose( void *userData,
                        SablotHandle processor_,
                        int handle );


struct UserData {

    InputStreamTransformer*             m_pTransformer;
    Reference< XMultiServiceFactory >   m_xSMgr;
};


const char* parameterFunc( const char* par,URLParameter* urlParam )
{
    rtl::OString aString;
    if( strcmp( par,"Program" ) == 0 )
        aString = rtl::OString( urlParam->get_program(),
                                urlParam->get_program().getLength(),
                                RTL_TEXTENCODING_UTF8 );
    else if( strcmp( par,"Database" ) == 0 )
        aString = rtl::OString( urlParam->get_module(),
                                urlParam->get_module().getLength(),
                                RTL_TEXTENCODING_UTF8 );
    else if( strcmp( par,"Id" ) == 0 )
        aString = rtl::OString( urlParam->get_id(),
                                urlParam->get_id().getLength(),
                                RTL_TEXTENCODING_UTF8 );
    else if( strcmp( par,"Path" ) == 0 )
        aString = rtl::OString( urlParam->get_path(),
                                urlParam->get_path().getLength(),
                                RTL_TEXTENCODING_UTF8 );
    else if( strcmp( par,"Language" ) == 0 )
        aString = rtl::OString( urlParam->get_language(),
                                urlParam->get_language().getLength(),
                                RTL_TEXTENCODING_UTF8 );
    else if( strcmp( par,"System" ) == 0 )
        aString = rtl::OString( urlParam->get_system(),
                                urlParam->get_system().getLength(),
                                RTL_TEXTENCODING_UTF8 );

    char* ret = new char[ 1+aString.getLength() ];
    ret[ aString.getLength() ] = 0;
    rtl_copyMemory( (void*)(ret),(void*)(aString.getStr()),sal_uInt32( aString.getLength() ) );
    return ret;
}


InputStreamTransformer::InputStreamTransformer( const Reference< XMultiServiceFactory >& rxSMgr,
                                                URLParameter* urlParam )
    : len( 0 ),
      pos( 0 ),
      buffer( new char[0] )
{
    rtl::OUString url = rtl::OUString::createFromAscii( "vnd.sun.star.pkg://" );
    rtl::OUString jar =
        Databases::getInstallPathAsURL()         +
        urlParam->get_language()                 +
        rtl::OUString::createFromAscii( "/" )    +
        urlParam->get_jar();

    url+= rtl::Uri::encode( jar,
                            rtl_UriCharClassUricNoSlash,
                            rtl_UriEncodeIgnoreEscapes,
                            RTL_TEXTENCODING_UTF8 );

    url += ( rtl::OUString::createFromAscii( "/" ) + urlParam->get_path() );

    SchemeHandler schemeHandler;
    schemeHandler.getAll = schemehandlergetall;
    schemeHandler.freeMemory = schemehandlerfreememory;
    schemeHandler.open = schemehandleropen;
    schemeHandler.get = schemehandlerget;
    schemeHandler.put = schemehandlerput;
    schemeHandler.close = schemehandlerclose;

    UserData userData;
    userData.m_xSMgr = rxSMgr;
    userData.m_pTransformer = this;

    SablotHandle p;
    SablotCreateProcessor(&p);
    SablotRegHandler( p,HLR_SCHEME,&schemeHandler,(void*)(&userData) );

    rtl::OString aString = rtl::OString( url.getStr(),url.getLength(),RTL_TEXTENCODING_UTF8 );
    char* inputStr = new char[ 1+aString.getLength() ];
    inputStr[ aString.getLength() ] = 0;
    rtl_copyMemory( (void*)(inputStr),(void*)(aString.getStr()),sal_uInt32( aString.getLength() ) );

    const sal_Int32 parCount = 6;
    const char* parameter[ 1+2*parCount ];

    parameter[ 0] = "Program";
    parameter[ 1] = parameterFunc( parameter[ 0],urlParam );
    parameter[ 2] = "Database";
    parameter[ 3] = parameterFunc( parameter[ 2],urlParam );
    parameter[ 4] = "Id";
    parameter[ 5] = parameterFunc( parameter[ 4],urlParam );
    parameter[ 6] = "Path";
    parameter[ 7] = parameterFunc( parameter[ 6],urlParam );
    parameter[ 8] = "Language";
    parameter[ 9] = parameterFunc( parameter[ 8],urlParam );
    parameter[10] = "System";
    parameter[11] = "WIN";// parameterFunc( parameter[10],urlParam );
    parameter[12] = 0;

    SablotRunProcessor( p,
                        "file://e:/src632b/help/main_transform.xsl",
                        inputStr,
                        "vnd.sun.star.resultat://resultbuff",
                        const_cast<char**>(parameter),
                        0 );

    char* my_buf;
    SablotGetResultArg( p,"arg:/somename",&my_buf );
    SablotDestroyProcessor( p );
    delete[] inputStr;
    for( int i = 1; i < 1+2*parCount; i+=2 )
        if( i != 11 )
            delete[] const_cast<char*>(parameter[i]);
}


InputStreamTransformer::~InputStreamTransformer()
{
    delete[] buffer;
}


Any SAL_CALL InputStreamTransformer::queryInterface( const Type& rType ) throw( RuntimeException )
{
    Any aRet = ::cppu::queryInterface( rType,
                                       SAL_STATIC_CAST( XInputStream*,this ),
                                       SAL_STATIC_CAST( XSeekable*,this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



void SAL_CALL InputStreamTransformer::acquire( void ) throw( RuntimeException )
{
    OWeakObject::acquire();
}



void SAL_CALL InputStreamTransformer::release( void ) throw( RuntimeException )
{
    OWeakObject::release();
}



sal_Int32 SAL_CALL InputStreamTransformer::readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead ) throw( NotConnectedException,
                                                                                                                  BufferSizeExceededException,
                                                                                                                  IOException,
                                                                                                                  RuntimeException)
{
    int curr,available = len-pos;
    if( nBytesToRead <= available )
        curr = nBytesToRead;
    else
        curr = available;

    if( 0 <= curr && aData.getLength() < curr )
        aData.realloc( curr );

    for( int k = 0; k < curr; ++k )
        aData[k] = buffer[pos++];

    return curr > 0 ? curr : 0;
}


sal_Int32 SAL_CALL InputStreamTransformer::readSomeBytes( Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException)
{
    return readBytes( aData,nMaxBytesToRead );
}



void SAL_CALL InputStreamTransformer::skipBytes( sal_Int32 nBytesToSkip ) throw( NotConnectedException,
                                                                                 BufferSizeExceededException,
                                                                                 IOException,
                                                                                 RuntimeException )
{
    while( nBytesToSkip-- ) ++pos;
}



sal_Int32 SAL_CALL InputStreamTransformer::available( void ) throw( NotConnectedException,
                                                                    IOException,
                                                                    RuntimeException )
{
    return len-pos > 0 ? len - pos : 0 ;
}



void SAL_CALL InputStreamTransformer::closeInput( void ) throw( NotConnectedException,
                                                                IOException,
                                                                RuntimeException )
{
}



void SAL_CALL InputStreamTransformer::seek( sal_Int64 location ) throw( IllegalArgumentException,
                                                                        IOException,
                                                                        RuntimeException )
{
    if( location < 0 )
        throw IllegalArgumentException();
    else
        pos = location;

    if( pos > len )
        pos = len;
}



sal_Int64 SAL_CALL InputStreamTransformer::getPosition( void ) throw( IOException,
                                                                      RuntimeException )
{
    return sal_Int64( pos );
}



sal_Int64 SAL_CALL InputStreamTransformer::getLength( void ) throw( IOException,RuntimeException )
{
    return len;
}


void InputStreamTransformer::addToBuffer( const char* buffer_,int len_ )
{
    char* tmp = buffer;
    buffer = new char[ len+len_ ];
    rtl_copyMemory( (void*)(buffer),(void*)(tmp),sal_uInt32( len ) );
    rtl_copyMemory( (void*)(buffer+len),(void*)(buffer_),sal_uInt32( len_ ) );
    delete tmp;
    len += len_;
}



class XActiveDataSinkImpl
    : public OWeakObject,
      public XActiveDataSink
{
    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException )
    {
        Any aRet = ::cppu::queryInterface( rType,
                                           SAL_STATIC_CAST( XActiveDataSink*,this ) );

        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }

    virtual void SAL_CALL acquire( void ) throw( RuntimeException )
    {
        OWeakObject::acquire();
    }

    virtual void SAL_CALL release( void ) throw( RuntimeException )
    {
        OWeakObject::release();
    }

    virtual void SAL_CALL setInputStream( const Reference< XInputStream >& xInputStream )
    {
        m_xInputStream = xInputStream;
    }

    virtual Reference< XInputStream > SAL_CALL getInputStream()
    {
        return m_xInputStream;
    }

private:

    Reference< XInputStream > m_xInputStream;
};


#include <string.h>


/*  getAll: open the URI and return the whole string
        scheme = URI scheme (e.g. "http")
        rest = the rest of the URI (without colon)
        the document is returned in a handler-allocated buffer
        byteCount holds the byte count on return
        return *buffer = NULL if not processed
*/


int schemehandlergetall( void *userData,
                         SablotHandle processor_,
                         const char *scheme,
                         const char *rest,
                         char **buffer,
                         int *byteCount )
{
    rtl::OUString url( rtl::OUString::createFromAscii( "vnd.sun.star.pkg:" ) );

    if( strcmp( scheme,"vnd.sun.star.help" ) == 0 )
    {
        url += rtl::OUString::createFromAscii( "//" );

        URLParameter urlpar( rtl::OUString::createFromAscii( scheme ) +
                             rtl::OUString::createFromAscii( ":" )    +
                             rtl::OUString::createFromAscii( rest ) );

        rtl::OUString jar =
            Databases::getInstallPathAsURL()         +
            urlpar.get_language()                    +
            rtl::OUString::createFromAscii( "/" )    +
            urlpar.get_jar();

        url+= rtl::Uri::encode( jar,
                                rtl_UriCharClassUricNoSlash,
                                rtl_UriEncodeIgnoreEscapes,
                                RTL_TEXTENCODING_UTF8 );

        url += ( rtl::OUString::createFromAscii( "/" ) + urlpar.get_path() );
    }
    else if( strcmp( scheme,"vnd.sun.star.pkg" ) == 0 )
        url += rtl::OUString::createFromAscii( rest );
    else
    {
        *buffer = 0;
        *byteCount = 0;
        return 0;
    }

    UserData *uData = reinterpret_cast< UserData* >( userData );

    rtl::OUString service = rtl::OUString::createFromAscii( "com.sun.star.ucb.UniversalContentBroker" );
    Reference< XContentProvider > provider( uData->m_xSMgr->createInstance( service ),UNO_QUERY );
    Reference< XContentIdentifierFactory > factory( provider,UNO_QUERY );
    Reference< XContentIdentifier > xIdentifier = factory->createContentIdentifier( url );
    Reference< XContent > xContent = provider->queryContent( xIdentifier );
    Reference< XCommandProcessor > processor( xContent,UNO_QUERY );

    OpenCommandArgument2 argument;
    argument.Mode = OpenMode::ALL;
    Reference< XActiveDataSink > xActiveDataSink( new XActiveDataSinkImpl() );
    argument.Sink = xActiveDataSink;
    argument.Priority = 0;

    Command command;
    command.Name = rtl::OUString::createFromAscii( "open" );
    command.Handle = -1;
    command.Argument <<= argument;

    sal_Int32 commandId = processor->createCommandIdentifier();

    try
    {
        processor->execute( command,
                            commandId,
                            Reference< XCommandEnvironment >( 0 ) );
    }
    catch( const CommandAbortedException& e )
    {
        printf( "catched exception" );
    }

    Reference< XInputStream > xInputStream = xActiveDataSink->getInputStream();
    if( xInputStream.is() )
    {
        Reference< XSeekable > xSeekable( xInputStream,UNO_QUERY );

        sal_Int32 size = 0;

        if( xSeekable.is() )
            size = sal_Int32( xSeekable->getLength() );

        *buffer = new char[ 1+size ];
        (*buffer)[ size ] = 0;

        Sequence< sal_Int8 > aSeq;
        xInputStream->readBytes( aSeq,size );

        rtl_copyMemory( (void*)(*buffer),(void*)(aSeq.getConstArray()),sal_uInt32(size) );
        *byteCount = size;
    }
    else
    {
        *buffer = 0;
        *byteCount = 0;
    }

    return 0;
}

/*  freeMemory: free the buffer allocated by getAll
 */

int schemehandlerfreememory( void *userData,
                             SablotHandle processor_,
                             char *buffer )
{
    delete[] buffer;
    return 0;
}


/*  open: open the URI and return a handle
    scheme = URI scheme (e.g. "http")
    rest = the rest of the URI (without colon)
    the resulting handle is returned in '*handle'
*/

int schemehandleropen( void *userData,
                       SablotHandle processor_,
                       const char *scheme,
                       const char *rest,
                       int *handle )
{
    *handle = 0;
    return 0;
}

/*  get: retrieve data from the URI
    handle = the handle assigned on open
    buffer = pointer to the data
    *byteCount = number of bytes to read
    (the number actually read is returned here)
*/

int schemehandlerget( void *userData,
                      SablotHandle processor_,
                      int handle,
                      char *buffer,
                      int *byteCount )
{
    *byteCount = 0;
    return 0;
}

/*  put: save data to the URI (if possible)
    handle = the handle assigned on open
    buffer = pointer to the data
    *byteCount = number of bytes to write
    (the number actually written is returned here)
*/
int schemehandlerput( void *userData,
                      SablotHandle processor_,
                      int handle,
                      const char *buffer,
                      int *byteCount )
{
    UserData *uData = reinterpret_cast< UserData* >( userData );
    uData->m_pTransformer->addToBuffer( buffer,*byteCount );
    return 0;
}

/*  close: close the URI with the given handle
    handle = the handle assigned on open
*/

int schemehandlerclose( void *userData,
                        SablotHandle processor_,
                        int handle )
{
    return 0;
}
