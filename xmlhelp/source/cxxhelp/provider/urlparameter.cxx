/*************************************************************************
 *
 *  $RCSfile: urlparameter.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: abi $ $Date: 2001-05-22 14:57:11 $
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
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
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
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
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


//      public InputStream getInputFromJarFile()
//      {
//          try
//          {
//              JarFile jarFile = Databases.getJarFileForLanguage( get_jar(),get_language() );   // For module and language
//              String path = get_path();
//              int idx;
//              if( ( idx = path.indexOf( '#' ) ) != -1 )
//                  path = path.substring(0,idx);

//              JarEntry jarEntry = jarFile.getJarEntry( path );
//              if( jarEntry != null )
//                  return jarFile.getInputStream( jarEntry );
//              else
//              {
//                  // System.out.println( "File not found in jar: " + get_jar() + " " + path );
//                  return Databases.errorFile( get_language() );
//              }
//          }
//          catch( Exception e )
//          {
//              return Databases.errorFile( get_language() );
//          }
//      }




//      public InputStream getInputFromDisk()
//      {
//          try
//          {
//              String fileName = Databases.getInstallDirectory()
//                  + Databases.lang(get_language())
//                  + File.separator
//                  + get_path();

//              int idx;
//              if( ( idx = fileName.indexOf( '#' ) ) != -1 )
//                  fileName = fileName.substring(0,idx);

//              File aFile = new File( fileName );
//              if( aFile.exists() )
//              {
//                  return new FileInputStream( aFile );
//              }
//              else
//              {
//                  System.out.println( "File not found from disk: " + get_path() );
//                  return Databases.errorFile( get_language() );
//              }
//          }
//          catch( Exception e )
//          {
//              return Databases.errorFile( get_language() );
//          }
//      }


//      public byte[] getByteArrayText()
//      {
//          try
//          {
//              Db db = Databases.getHelptextDbForLanguage( get_module(),get_language() );

//              StringDbt key = new StringDbt( _id );
//              StringDbt data = new StringDbt();

//              int err = db.get(null,key,data,0);
//              if( data != null )
//                  try
//                  {
//                      return data.getString().getBytes( "UTF8" );
//                  }
//                  catch( UnsupportedEncodingException e )
//                  {
//                      return data.getString().getBytes();
//                  }
//              else
//                  return new byte[0];
//          }
//          catch( DbException err )
//          {
//              System.out.println( "No database for language: HelpURLParameter._readBerkeley" );
//              return new byte[0];
//          }
//      }


void URLParameter::init( bool bDefaultLanguageIsInitialized )
{
    m_bBerkeleyRead = false;
    m_bStart = false;
    m_nHitCount = 100;                // The default maximum hitcount

//      m_aTag = rtl::OUString::createFromAscii( "" );
//      m_aId = rtl::OUString::createFromAscii( "" );
//      m_aPath = rtl::OUString::createFromAscii( "" );
//      m_aModule = rtl::OUString::createFromAscii( "" );
//      m_aTitle = rtl::OUString::createFromAscii( "" );
//      m_aJar = rtl::OUString::createFromAscii( "" );
//      m_aEid = rtl::OUString::createFromAscii( "" );
//      m_aLanguage =  rtl::OUString::createFromAscii( "" );

//      if( ! bDefaultLanguageIsInitialized )
//          m_aDefaultLanguage = rtl::OUString::createFromAscii( "" );

//      m_aPrefix = rtl::OUString::createFromAscii( "" );
//      m_aDevice = rtl::OUString::createFromAscii( "" );
//      m_aProgram = rtl::OUString::createFromAscii( "" );
//      m_aSystem = rtl::OUString::createFromAscii( "" );
//      m_aActive = rtl::OUString::createFromAscii( "" );

//      m_aQuery = rtl::OUString::createFromAscii( "" );
//      m_aScope = rtl::OUString::createFromAscii( "" );

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

    InputStreamTransformer( const Reference< XInputStream >& xUntransformed );
    ~InputStreamTransformer();

    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException );
    virtual void SAL_CALL acquire( void ) throw( RuntimeException );
    virtual void SAL_CALL release( void ) throw( RuntimeException );


    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead ) throw( NotConnectedException,
                                                                                                      BufferSizeExceededException,
                                                                                                      IOException,
                                                                                                      RuntimeException);

    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead ) throw( NotConnectedException,
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


private:

    Reference< XInputStream > m_xInputStream;
    Reference< XSeekable >    m_xSeekable;
};




void URLParameter::open( const Reference< XMultiServiceFactory >& rxSMgr,
                         const Command& command,
                         sal_Int32 CommandId,
                         const Reference< XCommandEnvironment >& Environment,
                         const Reference< XActiveDataSink >& xDataSink )
{
    rtl::OUString service = rtl::OUString::createFromAscii( "com.sun.star.ucb.UniversalContentBroker" );

    Reference< XContentProvider > provider( rxSMgr->createInstance( service ),UNO_QUERY );
    Reference< XContentIdentifierFactory > factory( provider,UNO_QUERY );

    rtl::OUString url = rtl::OUString::createFromAscii( "vnd.sun.star.pkg://" );
    rtl::OUString jar =
        Databases::getInstallPathAsURL()         +
        get_language()                           +
        rtl::OUString::createFromAscii( "/" )    +
        get_module()                             +
        rtl::OUString::createFromAscii( ".jar" );

    url+= rtl::Uri::encode( jar,
                            rtl_UriCharClassUricNoSlash,
                            rtl_UriEncodeIgnoreEscapes,
                            RTL_TEXTENCODING_UTF8 );

    url += ( rtl::OUString::createFromAscii( "/" ) + get_path() );

    Reference< XContentIdentifier > xIdentifier = factory->createContentIdentifier( url );
    Reference< XContent > xContent = provider->queryContent( xIdentifier );

    Reference< XCommandProcessor > processor( xContent,UNO_QUERY );

    if( isRoot() )
    {
        printf( "isRoot" );
//      getPicture( HelpDatabases.getCssSheet(),m_xOutputStream);
    }
    else if( isPicture() )
    {
        printf( "isPicture" );
//      getPicture( m_xParameter.getInputFromJarFile(),m_xOutputStream );
    }
    else if( isActive() )
    {   // This is a Helptext
        printf( "isActive" );
//      m_xOutputStream.setBigBuffer( m_xParameter.getByteArrayText() );
    }
    else
    {
        processor->execute( command,
                            CommandId,
                             Environment );

        // Now plug in a new XInputStream
        xDataSink->setInputStream( new InputStreamTransformer( xDataSink->getInputStream() ) );
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
            m_aSystem = value;
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



InputStreamTransformer::InputStreamTransformer( const Reference< XInputStream >& xInputStream )
    : m_xInputStream( xInputStream ),
      m_xSeekable( xInputStream,UNO_QUERY )
{
}


InputStreamTransformer::~InputStreamTransformer()
{
}


Any SAL_CALL InputStreamTransformer::queryInterface( const Type& rType ) throw( RuntimeException )
{
    Any aRet;
    if( m_xSeekable.is() )
        aRet = ::cppu::queryInterface( rType,
                                       SAL_STATIC_CAST( XInputStream*,this ),
                                       SAL_STATIC_CAST( XSeekable*,this ) );
    else
        aRet = ::cppu::queryInterface( rType,
                                       SAL_STATIC_CAST( XInputStream*,this ) );

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
    return m_xInputStream->readBytes( aData,nBytesToRead );
}



sal_Int32 SAL_CALL InputStreamTransformer::readSomeBytes( Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException)
{
    return m_xInputStream->readSomeBytes( aData,nMaxBytesToRead );
}



void SAL_CALL InputStreamTransformer::skipBytes( sal_Int32 nBytesToSkip ) throw( NotConnectedException,
                                                                                 BufferSizeExceededException,
                                                                                 IOException,
                                                                                 RuntimeException )
{
    m_xInputStream->skipBytes( nBytesToSkip );
}



sal_Int32 SAL_CALL InputStreamTransformer::available( void ) throw( NotConnectedException,
                                                                    IOException,
                                                                    RuntimeException )
{
    return m_xInputStream->available();
}



void SAL_CALL InputStreamTransformer::closeInput( void ) throw( NotConnectedException,
                                                                IOException,
                                                                RuntimeException )
{
    m_xInputStream->closeInput();
}



void SAL_CALL InputStreamTransformer::seek( sal_Int64 location ) throw( IllegalArgumentException,
                                                                        IOException,
                                                                        RuntimeException )
{
    if( m_xSeekable.is() )
        m_xSeekable->seek( location );
}



sal_Int64 SAL_CALL InputStreamTransformer::getPosition( void ) throw( IOException,
                                                                      RuntimeException )
{
    if( m_xSeekable.is() )
        return m_xSeekable->getPosition();
}



sal_Int64 SAL_CALL InputStreamTransformer::getLength( void ) throw( IOException,RuntimeException )
{
    if( m_xSeekable.is() )
        return m_xSeekable->getLength();
}
