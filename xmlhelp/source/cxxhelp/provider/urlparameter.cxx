/*************************************************************************
 *
 *  $RCSfile: urlparameter.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: vg $ $Date: 2003-08-05 09:42:33 $
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

#define WORKAROUND_98119

#ifdef WORKAROUND_98119
#include <provider/bufferedinputstream.hxx>
#endif

#include <string.h>
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
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
#ifndef SablotHIncl
#include <sablot/sablot.h>
#endif
#ifndef ShandlerHIncl
#include <sablot/shandler.h>
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
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
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
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif


namespace chelp {

    inline bool ascii_isDigit( sal_Unicode ch )
    {
        return ((ch >= 0x0030) && (ch <= 0x0039));
    }

    inline bool ascii_isLetter( sal_Unicode ch )
    {
        return ( ( (ch >= 0x0041) && (ch <= 0x005A) ) ||
                 ( (ch >= 0x0061) && (ch <= 0x007A) ) );
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
using namespace com::sun::star::container;
using namespace chelp;


URLParameter::URLParameter( const rtl::OUString& aURL,
                            Databases* pDatabases )
    throw( com::sun::star::ucb::IllegalIdentifierException )
    : m_aURL( aURL ),
      m_pDatabases( pDatabases )
{
    init( false );
    parse();
}


URLParameter::URLParameter( const rtl::OUString&  aURL,
                            const rtl::OUString& aDefaultLanguage,
                            Databases* pDatabases )
    throw( com::sun::star::ucb::IllegalIdentifierException )
    : m_aURL( aURL ),
      m_aDefaultLanguage( aDefaultLanguage ),
      m_pDatabases( pDatabases )
{
    init( true );
    parse();
}


bool URLParameter::isErrorDocument()
{
    if( isFile() )
    {
        Reference< XHierarchicalNameAccess > xNA = m_pDatabases->jarFile( get_jar(),
                                                                          get_language() );

        return ! ( xNA.is() && xNA->hasByHierarchicalName( get_path() ) );
    }

    return false;
}


rtl::OString URLParameter::getByName( const char* par )
{
    rtl::OUString val;

    if( strcmp( par,"Program" ) == 0 )
        val = get_program();
    else if( strcmp( par,"Database" ) == 0 )
        val = get_module();
    else if( strcmp( par,"Id" ) == 0 )
        val = get_id();
    else if( strcmp( par,"Path" ) == 0 )
        val = get_path();
    else if( strcmp( par,"Language" ) == 0 )
        val = get_language();
    else if( strcmp( par,"System" ) == 0 )
        val = get_system();
    else if( strcmp( par,"HelpPrefix" ) == 0 )
        val = get_prefix();

    return rtl::OString( val.getStr(),val.getLength(),RTL_TEXTENCODING_UTF8 );
}


rtl::OUString URLParameter::get_id()
{
    if( m_aId.compareToAscii( "52821" ) == 0 || m_aId.compareToAscii("start") == 0 )
    {   // module is set
        StaticModuleInformation* inf =
            m_pDatabases->getStaticInformationForModule( get_module(),
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
            m_pDatabases->getStaticInformationForModule( get_module(),
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
    if( ! m_aProgram.getLength() )
    {
        StaticModuleInformation* inf =
            m_pDatabases->getStaticInformationForModule( get_module(),
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
    if( get_id().compareToAscii("") == 0 )
        return;

    Db* db = m_pDatabases->getBerkeley( get_module(),
                                        get_language() );

    if( ! db )
        return;

    rtl::OString keyStr( m_aId.getStr(),m_aId.getLength(),RTL_TEXTENCODING_UTF8 );
    Dbt key( static_cast< void* >( const_cast< sal_Char* >( keyStr.getStr() ) ),
             keyStr.getLength() );
    Dbt data;

    int err = db->get( 0,&key,&data,0 );

    DbtToStringConverter converter( static_cast< sal_Char* >( data.get_data() ),
                                    data.get_size() );
    m_aTitle = converter.getTitle();
    m_pDatabases->replaceName( m_aTitle );
    m_aPath  = converter.getFile();
    m_aJar   = converter.getDatabase();
    m_aTag   = converter.getHash();
}



// Class encapsulating the transformation of the XInputStream to XHTML


class InputStreamTransformer
    : public OWeakObject,
      public XInputStream,
      public XSeekable
{
public:

    InputStreamTransformer( URLParameter* urlParam,
                            Databases*    pDatatabases,
                            bool isRoot = false );

    ~InputStreamTransformer();

    virtual Any SAL_CALL queryInterface( const Type& rType ) throw( RuntimeException );
    virtual void SAL_CALL acquire( void ) throw();
    virtual void SAL_CALL release( void ) throw();

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

    sal_Int8* getData() const { return (sal_Int8*) buffer; }

    sal_Int32 getLen() const { return sal_Int32( len ); }

private:

    osl::Mutex m_aMutex;

    int len,pos;
    char *buffer;
};



void URLParameter::open( const Reference< XMultiServiceFactory >& rxSMgr,
                         const Command& aCommand,
                         sal_Int32 CommandId,
                         const Reference< XCommandEnvironment >& Environment,
                         const Reference< XOutputStream >& xDataSink )
{
    if( ! xDataSink.is() )
        return;

    if( isPicture() )
    {
        Reference< XInputStream > xStream;
        Reference< XHierarchicalNameAccess > xNA =
            m_pDatabases->jarFile( rtl::OUString::createFromAscii( "picture.jar" ),
                                   get_language() );

        rtl::OUString path = get_path();
        if( xNA.is() )
        {
            try
            {
                Any aEntry = xNA->getByHierarchicalName( path );
                Reference< XActiveDataSink > xSink;
                if( ( aEntry >>= xSink ) && xSink.is() )
                    xStream = xSink->getInputStream();
            }
            catch ( NoSuchElementException & )
            {
            }
        }
        if( xStream.is() )
        {
            sal_Int32 ret;
            Sequence< sal_Int8 > aSeq( 4096 );
            while( true )
            {
                try
                {
                    ret = xStream->readBytes( aSeq,4096 );
                    xDataSink->writeBytes( aSeq );
                    if( ret < 4096 )
                        break;
                }
                catch( const Exception& )
                {
                    break;
                }
            }
        }
    }
    else
    {
        // a standard document or else an active help text, plug in the new input stream
        InputStreamTransformer* p = new InputStreamTransformer( this,m_pDatabases,isRoot() );
        try
        {
            xDataSink->writeBytes( Sequence< sal_Int8 >( p->getData(),p->getLen() ) );
        }
        catch( const Exception& )
        {
        }
        delete p;
    }
    xDataSink->closeOutput();
}



void URLParameter::open( const Reference< XMultiServiceFactory >& rxSMgr,
                         const Command& aCommand,
                         sal_Int32 CommandId,
                         const Reference< XCommandEnvironment >& Environment,
                         const Reference< XActiveDataSink >& xDataSink )
{
    if( isPicture() )
    {
        Reference< XInputStream > xStream;
        Reference< XHierarchicalNameAccess > xNA =
            m_pDatabases->jarFile( rtl::OUString::createFromAscii( "picture.jar" ),
                                   get_language() );

        rtl::OUString path = get_path();
        if( xNA.is() )
        {
            try
            {
                Any aEntry = xNA->getByHierarchicalName( path );
                Reference< XActiveDataSink > xSink;
                if( ( aEntry >>= xSink ) && xSink.is() )
                    xStream = xSink->getInputStream();
            }
            catch ( NoSuchElementException & )
            {
            }
        }
#ifdef WORKAROUND_98119
        xDataSink->setInputStream( turnToSeekable(xStream) );
#else
        xDataSink->setInputStream( xStream );
#endif
    }
    else
        // a standard document or else an active help text, plug in the new input stream
        xDataSink->setInputStream( new InputStreamTransformer( this,m_pDatabases,isRoot() ) );
}



void URLParameter::parse() throw( com::sun::star::ucb::IllegalIdentifierException )
{
    m_aExpr = m_aURL;

    sal_Int32 lstIdx = m_aExpr.lastIndexOf( sal_Unicode( '#' ) );
    if( lstIdx != -1 )
        m_aExpr = m_aExpr.copy( 0,lstIdx );

    if( ! scheme() ||
        ! name( module() ) ||
        ! query() ||
        ! m_aLanguage.getLength() ||
        ! m_aSystem.getLength() )
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
            m_aPrefix = rtl::Uri::decode( value,
                                          rtl_UriDecodeWithCharset,
                                          RTL_TEXTENCODING_UTF8 );
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

    UserData( InputStreamTransformer* pTransformer,
              URLParameter*           pInitial,
              Databases*              pDatabases )
        : m_pTransformer( pTransformer ),
          m_pInitial( pInitial ),
          m_pDatabases( pDatabases )
    {
    }

    InputStreamTransformer*             m_pTransformer;
    Databases*                          m_pDatabases;
    URLParameter*                       m_pInitial;
};



InputStreamTransformer::InputStreamTransformer( URLParameter* urlParam,
                                                Databases*    pDatabases,
                                                bool isRoot )
    : len( 0 ),
      pos( 0 ),
      buffer( new char[0] )
{
    if( isRoot )
    {
        delete[] buffer;
        pDatabases->cascadingStylesheet( urlParam->get_language(),
                                         &buffer,
                                         &len );
    }
    else if( urlParam->isActive() )
    {
        delete[] buffer;
        pDatabases->setActiveText( urlParam->get_module(),
                                   urlParam->get_language(),
                                   urlParam->get_id(),
                                   &buffer,
                                   &len );
    }
    else
    {
        SchemeHandler schemeHandler;
        schemeHandler.getAll = schemehandlergetall;
        schemeHandler.freeMemory = schemehandlerfreememory;
        schemeHandler.open = schemehandleropen;
        schemeHandler.get = schemehandlerget;
        schemeHandler.put = schemehandlerput;
        schemeHandler.close = schemehandlerclose;

        UserData userData( this,urlParam,pDatabases );

        // Uses the implementation detail, that rtl::OString::getStr returns a zero terminated character-array

        const char* parameter[41];
        rtl::OString parString[40];
        int last;

        parString[ 0] = "Program";
        parString[ 1] = urlParam->getByName( "Program" );
        parString[ 2] = "Database";
        parString[ 3] = urlParam->getByName( "Database" );
        parString[ 4] = "Id";
        parString[ 5] = urlParam->getByName( "Id" );
        parString[ 6] = "Path";
        parString[ 7] = urlParam->getByName( "Path" );
        parString[ 8] = "Language";
        parString[ 9] = urlParam->getByName( "Language" );
        parString[10] = "System";
        parString[11] = urlParam->getByName( "System" );
        parString[12] = "productname";
        parString[13] = rtl::OString( pDatabases->getProductName().getStr(),
                                      pDatabases->getProductName().getLength(),
                                      RTL_TEXTENCODING_UTF8 );
        parString[14] = "productversion";
        parString[15] = rtl::OString( pDatabases->getProductVersion().getStr(),
                                      pDatabases->getProductVersion().getLength(),
                                      RTL_TEXTENCODING_UTF8 );

        parString[16] = "hp";
        parString[17] = urlParam->getByName( "HelpPrefix" );
        last = 18;

        if( parString[17].getLength() )
        {
            parString[18] = "sm";
            parString[19] = "vnd.sun.star.help%3A%2F%2F";
            parString[20] = "qm";
            parString[21] = "%3F";
            parString[22] = "es";
            parString[23] = "%3D";
            parString[24] = "am";
            parString[25] = "%26";
            parString[26] = "cl";
            parString[27] = "%3A";
            parString[28] = "sl";
            parString[29] = "%2F";
            parString[30] = "hm";
            parString[31] = "%23";
            parString[32] = "cs";
            parString[33] = "css";

            parString[34] = "vendorname";
            parString[35] = rtl::OString( pDatabases->getVendorName().getStr(),
                                          pDatabases->getVendorName().getLength(),
                                          RTL_TEXTENCODING_UTF8 );
            parString[36] = "vendorversion";
            parString[37] = rtl::OString( pDatabases->getVendorVersion().getStr(),
                                          pDatabases->getVendorVersion().getLength(),
                                          RTL_TEXTENCODING_UTF8 );
            parString[38] = "vendorshort";
            parString[39] = rtl::OString( pDatabases->getVendorShort().getStr(),
                                          pDatabases->getVendorShort().getLength(),
                                          RTL_TEXTENCODING_UTF8 );
            last = 40;
        }

        for( int i = 0; i < last; ++i )
            parameter[i] = parString[i].getStr();
        parameter[last] = 0;


        SablotHandle p;
        SablotCreateProcessor(&p);
        SablotRegHandler( p,HLR_SCHEME,&schemeHandler,(void*)(&userData) );
        rtl::OUString xslURL = pDatabases->getInstallPathAsURL/*WithOutEncoding*/();

        rtl::OString xslURLascii(
            xslURL.getStr(),
            xslURL.getLength(),
            RTL_TEXTENCODING_ASCII_US/*osl_getThreadTextEncoding()*/);
        xslURLascii += "main_transform.xsl";

        SablotRunProcessor( p,
                            const_cast<char*>(xslURLascii.getStr()),
                            "vnd.sun.star.pkg:/",
                            "vnd.sun.star.resultat:/",
                            const_cast<char**>(parameter),
                            0 );

        SablotDestroyProcessor( p );
    }
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



void SAL_CALL InputStreamTransformer::acquire( void ) throw()
{
    OWeakObject::acquire();
}



void SAL_CALL InputStreamTransformer::release( void ) throw()
{
    OWeakObject::release();
}



sal_Int32 SAL_CALL InputStreamTransformer::readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException)
{
    osl::MutexGuard aGuard( m_aMutex );

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
    osl::MutexGuard aGuard( m_aMutex );
    while( nBytesToSkip-- ) ++pos;
}



sal_Int32 SAL_CALL InputStreamTransformer::available( void ) throw( NotConnectedException,
                                                                    IOException,
                                                                    RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
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
    osl::MutexGuard aGuard( m_aMutex );
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
    osl::MutexGuard aGuard( m_aMutex );
    return sal_Int64( pos );
}



sal_Int64 SAL_CALL InputStreamTransformer::getLength( void ) throw( IOException,RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    return len;
}


void InputStreamTransformer::addToBuffer( const char* buffer_,int len_ )
{
    osl::MutexGuard aGuard( m_aMutex );

    char* tmp = buffer;
    buffer = new char[ len+len_ ];
    rtl_copyMemory( (void*)(buffer),(void*)(tmp),sal_uInt32( len ) );
    rtl_copyMemory( (void*)(buffer+len),(void*)(buffer_),sal_uInt32( len_ ) );
    delete[] tmp;
    len += len_;
}





/**
 *    getAll: open the URI and return the whole string
 *    scheme = URI scheme (e.g. "http")
 *    rest = the rest of the URI (without colon)
 *    the document is returned in a handler-allocated buffer
 *    byteCount holds the byte count on return
 *    return *buffer = NULL if not processed
*/

int schemehandlergetall( void *userData,
                         SablotHandle processor_,
                         const char *scheme,
                         const char *rest,
                         char **buffer,
                         int *byteCount )
{
    rtl::OUString language,jar,path;
    UserData *uData = reinterpret_cast< UserData* >( userData );

    if( strcmp( scheme,"vnd.sun.star.help" ) == 0 )
    {
        URLParameter urlpar( rtl::OUString::createFromAscii( scheme ) +
                             rtl::OUString::createFromAscii( ":" )    +
                             rtl::OUString::createFromAscii( rest ),
                             uData->m_pDatabases );

        jar = urlpar.get_jar();
        language = urlpar.get_language();
        path = urlpar.get_path();
    }
    else if( strcmp( scheme,"vnd.sun.star.pkg" ) == 0 )
    {
        if( uData->m_pInitial->get_eid().getLength() )
        {
            uData->m_pDatabases->popupDocument( uData->m_pInitial,buffer,byteCount );
            return 0;
        }
        else
        {
            jar = uData->m_pInitial->get_jar();
            language = uData->m_pInitial->get_language();
            path = uData->m_pInitial->get_path();
        }
    }
    else
    {
        *buffer = 0;
        *byteCount = 0;
        return 0;
    }

    Reference< XInputStream > xInputStream;
    Reference< XHierarchicalNameAccess > xNA = uData->m_pDatabases->jarFile( jar,language );

    if( xNA.is() )
    {
        try
        {
            Any aEntry = xNA->getByHierarchicalName( path );
            Reference< XActiveDataSink > xSink;
            if( ( aEntry >>= xSink ) && xSink.is() )
                xInputStream = xSink->getInputStream();
        }
        catch ( NoSuchElementException & )
        {
        }
    }

    if( xInputStream.is() )
    {
        sal_Int32 size = 0;

        Reference< XSeekable > xSeekable( xInputStream,UNO_QUERY );

        if( xSeekable.is() )
            size = sal_Int32( xSeekable->getLength() );
        else
            size = sal_Int32( xInputStream->available() );

        *buffer = new char[ 1+size ];
        (*buffer)[ size ] = 0;

        Sequence< sal_Int8 > aSeq;
        xInputStream->readBytes( aSeq,size );

        rtl_copyMemory( (void*)(*buffer),(void*)(aSeq.getConstArray()),sal_uInt32(size) );
        *byteCount = size;
    }
    else
        uData->m_pDatabases->errorDocument( language,buffer,byteCount );

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





