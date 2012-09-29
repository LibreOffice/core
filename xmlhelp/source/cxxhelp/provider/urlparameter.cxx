/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "bufferedinputstream.hxx"

#include <string.h>
#include <osl/diagnose.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include "db.hxx"
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include "urlparameter.hxx"
#include "databases.hxx"

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
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace berkeleydbproxy;
using namespace chelp;


URLParameter::URLParameter( const rtl::OUString& aURL,
                            Databases* pDatabases )
    throw( com::sun::star::ucb::IllegalIdentifierException )
    : m_pDatabases( pDatabases ),
      m_aURL( aURL )
{
    init( false );
    parse();
}


bool URLParameter::isErrorDocument()
{
    bool bErrorDoc = false;

    if( isFile() )
    {
        Reference< XHierarchicalNameAccess > xNA =
            m_pDatabases->findJarFileForPath( get_jar(), get_language(), get_path() );
        bErrorDoc = !xNA.is();
    }

    return bErrorDoc;
}


rtl::OString URLParameter::getByName( const char* par )
{
    rtl::OUString val;

    if( strcmp( par,"Program" ) == 0 )
        val = get_program();
    else if( strcmp( par,"Database" ) == 0 )
        val = get_module();
    else if( strcmp( par,"DatabasePar" ) == 0 )
        val = get_dbpar();
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
    if( m_aId.compareToAscii("start") == 0 )
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
        m_aTitle = rtl::OUString("root");

    return m_aTitle;
}


rtl::OUString URLParameter::get_language()
{
    if( m_aLanguage.isEmpty() )
        return m_aDefaultLanguage;

    return m_aLanguage;
}


rtl::OUString URLParameter::get_program()
{
    if( m_aProgram.isEmpty() )
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
    (void)bDefaultLanguageIsInitialized;

    m_bBerkeleyRead = false;
    m_bStart = false;
    m_bUseDB = true;
    m_nHitCount = 100;                // The default maximum hitcount
}


rtl::OUString URLParameter::get_the_tag()
{
    if(m_bUseDB) {
        if( ! m_bBerkeleyRead )
            readBerkeley();

        m_bBerkeleyRead = true;

        return m_aTag;
    }
    else
        return rtl::OUString();
}



rtl::OUString URLParameter::get_the_path()
{
    if(m_bUseDB) {
        if( ! m_bBerkeleyRead )
            readBerkeley();
        m_bBerkeleyRead = true;

        return m_aPath;
    }
    else
        return get_id();
}



rtl::OUString URLParameter::get_the_title()
{
    if(m_bUseDB) {
        if( ! m_bBerkeleyRead )
            readBerkeley();
        m_bBerkeleyRead = true;

        return m_aTitle;
    }
    else
        return rtl::OUString();
}


rtl::OUString URLParameter::get_the_jar()
{
    if(m_bUseDB) {
        if( ! m_bBerkeleyRead )
            readBerkeley();
        m_bBerkeleyRead = true;

        return m_aJar;
    }
    else
        return get_module() + rtl::OUString(".jar");
}




void URLParameter::readBerkeley()
{
    if( get_id().compareToAscii("") == 0 )
        return;

    rtl::OUString aModule = get_module();
    rtl::OUString aLanguage = get_language();

    DataBaseIterator aDbIt( *m_pDatabases, aModule, aLanguage, false );
    bool bSuccess = false;

    const sal_Char* pData = NULL;

    Dbt data;
    DBData aDBData;
    rtl::OUString aExtensionPath;
    rtl::OUString aExtensionRegistryPath;
    while( true )
    {
        Db* db = aDbIt.nextDb( &aExtensionPath, &aExtensionRegistryPath );
        if( !db )
            break;

        rtl::OString keyStr( m_aId.getStr(),m_aId.getLength(),RTL_TEXTENCODING_UTF8 );

        DBHelp* pDBHelp = db->getDBHelp();
        if( pDBHelp != NULL )
        {
            bSuccess = pDBHelp->getValueForKey( keyStr, aDBData );
            if( bSuccess )
            {
                pData = aDBData.getData();
                break;
            }
        }
        else
        {
            Dbt key( static_cast< void* >( const_cast< sal_Char* >( keyStr.getStr() ) ),
                     keyStr.getLength() );
            int err = db->get( 0,&key,&data,0 );
            if( err == 0 )
            {
                bSuccess = true;
                pData = static_cast<sal_Char*>( data.get_data() );
                break;
            }
        }
    }

    if( bSuccess )
    {
        DbtToStringConverter converter( pData );
        m_aTitle = converter.getTitle();
        m_pDatabases->replaceName( m_aTitle );
        m_aPath  = converter.getFile();
        m_aJar   = converter.getDatabase();
        if( !aExtensionPath.isEmpty() )
        {
            rtl::OUStringBuffer aExtendedJarStrBuf;
            aExtendedJarStrBuf.append( '?' );
            aExtendedJarStrBuf.append( aExtensionPath );
            aExtendedJarStrBuf.append( '?' );
            aExtendedJarStrBuf.append( m_aJar );
            m_aJar = aExtendedJarStrBuf.makeStringAndClear();
            m_aExtensionRegistryPath = aExtensionRegistryPath;
        }
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
    (void)rxSMgr;
    (void)aCommand;
    (void)CommandId;
    (void)Environment;

    if( ! xDataSink.is() )
        return;

    if( isPicture() )
    {
        Reference< XInputStream > xStream;
        Reference< XHierarchicalNameAccess > xNA =
            m_pDatabases->jarFile( rtl::OUString( "picture.jar" ),
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
    (void)rxSMgr;
    (void)aCommand;
    (void)CommandId;
    (void)Environment;

    if( isPicture() )
    {
        Reference< XInputStream > xStream;
        Reference< XHierarchicalNameAccess > xNA =
            m_pDatabases->jarFile( rtl::OUString( "picture.jar" ),
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
        xDataSink->setInputStream( turnToSeekable(xStream) );
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
        m_aLanguage.isEmpty() ||
        m_aSystem.isEmpty() )
        throw com::sun::star::ucb::IllegalIdentifierException();
}


bool URLParameter::scheme()
{
    // Correct extension help links as sometimes the
    // module is missing resulting in a misformed URL
    if( m_aExpr.compareToAscii( "vnd.sun.star.help:///", 21 ) == 0 )
    {
        sal_Int32 nLen = m_aExpr.getLength();
        rtl::OUString aLastStr =
            m_aExpr.copy(sal::static_int_cast<sal_uInt32>(nLen) - 6);
        if( aLastStr.compareToAscii( "DbPAR=" ) == 0 )
        {
            rtl::OUString aNewExpr = m_aExpr.copy( 0, 20 );
            rtl::OUString aSharedStr("shared");
            aNewExpr += aSharedStr;
            aNewExpr += m_aExpr.copy( 20 );
            aNewExpr += aSharedStr;
            m_aExpr = aNewExpr;
        }
    }

    for( sal_Int32 nPrefixLen = 20 ; nPrefixLen >= 18 ; --nPrefixLen )
    {
        if( m_aExpr.compareToAscii( "vnd.sun.star.help://", nPrefixLen ) == 0 )
        {
            m_aExpr = m_aExpr.copy( nPrefixLen );
            return true;
        }
    }
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
        while( idx < length && (m_aExpr.getStr())[idx] != '?' )
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
    rtl::OUString query_;

    if( m_aExpr.isEmpty() )
        return true;
    else if( (m_aExpr.getStr())[0] == sal_Unicode( '?' ) )
        query_ = m_aExpr.copy( 1 ).trim();
    else
        return false;


    bool ret = true;
    sal_Int32 delimIdx,equalIdx;
    rtl::OUString parameter,value;

    while( !query_.isEmpty() )
    {
        delimIdx = query_.indexOf( sal_Unicode( '&' ) );
        equalIdx = query_.indexOf( sal_Unicode( '=' ) );
        parameter = query_.copy( 0,equalIdx ).trim();
        if( delimIdx == -1 )
        {
            value = query_.copy( equalIdx + 1 ).trim();
            query_ = rtl::OUString();
        }
        else
        {
            value = query_.copy( equalIdx+1,delimIdx - equalIdx - 1 ).trim();
            query_ = query_.copy( delimIdx+1 ).trim();
        }

        if( parameter.compareToAscii( "Language" ) == 0 )
            m_aLanguage = value;
        else if( parameter.compareToAscii( "Device" ) == 0 )
            m_aDevice = value;
        else if( parameter.compareToAscii( "Program" ) == 0 )
            m_aProgram = value;
        else if( parameter.compareToAscii( "Eid" ) == 0 )
            m_aEid = value;
        else if( parameter.compareToAscii( "UseDB" ) == 0 )
            m_bUseDB = ! ( value.compareToAscii("no") == 0 );
        else if( parameter.compareToAscii( "DbPAR" ) == 0 )
            m_aDbPar = value;
        else if( parameter.compareToAscii( "Query" ) == 0 )
        {
            if( m_aQuery.isEmpty() )
                m_aQuery = value;
            else
                m_aQuery += ( rtl::OUString( " " ) + value );
        }
        else if( parameter.compareToAscii( "Scope" ) == 0 )
            m_aScope = value;
        else if( parameter.compareToAscii( "System" ) == 0 )
            m_aSystem = value;
        else if( parameter.compareToAscii( "HelpPrefix" ) == 0 )
            m_aPrefix = rtl::Uri::decode(
                value,
                rtl_UriDecodeWithCharset,
                RTL_TEXTENCODING_UTF8 );
        else if( parameter.compareToAscii( "HitCount" ) == 0 )
            m_nHitCount = value.toInt32();
        else if( parameter.compareToAscii( "Active" ) == 0 )
            m_aActive = value;
        else if( parameter.compareToAscii( "Version" ) == 0 )
            ; // ignored (but accepted) in the build-in help, useful only for the online help
        else
            ret = false;
    }

    return ret;
}

struct UserData {

    UserData( InputStreamTransformer* pTransformer,
              URLParameter*           pInitial,
              Databases*              pDatabases )
        : m_pTransformer( pTransformer ),
          m_pDatabases( pDatabases ),
          m_pInitial( pInitial )
    {
    }

    InputStreamTransformer*             m_pTransformer;
    Databases*                          m_pDatabases;
    URLParameter*                       m_pInitial;
};

UserData *ugblData = 0;

extern "C" {

static int
fileMatch(const char * URI) {
    if ((URI != NULL) && !strncmp(URI, "file:/", 6))
        return 1;
    return 0;
}

static int
zipMatch(const char * URI) {
    if ((URI != NULL) && !strncmp(URI, "vnd.sun.star.zip:/", 18))
        return 1;
    return 0;
}

static int
helpMatch(const char * URI) {
    if ((URI != NULL) && !strncmp(URI, "vnd.sun.star.help:/", 19))
        return 1;
    return 0;
}

static void *
fileOpen(const char *URI) {
    osl::File *pRet = new osl::File(rtl::OUString(URI, strlen(URI), RTL_TEXTENCODING_UTF8));
    pRet->open(osl_File_OpenFlag_Read);
    return pRet;
}

static void *
zipOpen(SAL_UNUSED_PARAMETER const char *) {
    rtl::OUString language,jar,path;

    if( !ugblData->m_pInitial->get_eid().isEmpty() )
        return (void*)(new Reference< XHierarchicalNameAccess >);
    else
    {
        jar = ugblData->m_pInitial->get_jar();
        language = ugblData->m_pInitial->get_language();
        path = ugblData->m_pInitial->get_path();
    }

    Reference< XHierarchicalNameAccess > xNA =
        ugblData->m_pDatabases->findJarFileForPath( jar, language, path );

    Reference< XInputStream > xInputStream;

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
        return new Reference<XInputStream>(xInputStream);
    }
    return 0;
}

static void *
helpOpen(const char * URI) {
    rtl::OUString language,jar,path;

    URLParameter urlpar( rtl::OUString::createFromAscii( URI ),
                         ugblData->m_pDatabases );

    jar = urlpar.get_jar();
    language = urlpar.get_language();
    path = urlpar.get_path();

    Reference< XHierarchicalNameAccess > xNA =
        ugblData->m_pDatabases->findJarFileForPath( jar, language, path );

    Reference< XInputStream > xInputStream;

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
        return new Reference<XInputStream>(xInputStream);
    return 0;
}

static int
helpRead(void * context, char * buffer, int len) {
    Reference< XInputStream > *pRef = (Reference< XInputStream >*)context;

    Sequence< sal_Int8 > aSeq;
    len = (*pRef)->readBytes( aSeq,len);
    memcpy(buffer, aSeq.getConstArray(), len);

    return len;
}

static int
zipRead(void * context, char * buffer, int len) {
    if( !ugblData->m_pInitial->get_eid().isEmpty() )
    {
        ugblData->m_pDatabases->popupDocument( ugblData->m_pInitial,&buffer,&len);
        return len;
    }
    else
        return helpRead(context, buffer, len);
}

static int
fileRead(void * context, char * buffer, int len) {
    int nRead = 0;
    osl::File *pFile = (osl::File*)context;
    if (pFile)
    {
        sal_uInt64 uRead = 0;
        if (osl::FileBase::E_None == pFile->read(buffer, len, uRead))
            nRead = static_cast<int>(uRead);
    }
    return nRead;
}

static int
uriClose(void * context) {
    Reference< XInputStream > *pRef = (Reference< XInputStream >*)context;
    delete pRef;
    return 0;
}

static int
fileClose(void * context) {
    osl::File *pFile = (osl::File*)context;
    if (pFile)
    {
        pFile->close();
        delete pFile;
    }
    return 0;
}

} // extern "C"

InputStreamTransformer::InputStreamTransformer( URLParameter* urlParam,
                                                Databases*    pDatabases,
                                                bool isRoot )
    : len( 0 ),
      pos( 0 ),
      buffer( new char[1] ) // Initializing with one element to avoid gcc compiler warning
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
        UserData userData( this,urlParam,pDatabases );

        // Uses the implementation detail, that rtl::OString::getStr returns a zero terminated character-array

        const char* parameter[47];
        rtl::OString parString[46];
        int last = 0;

        parString[last++] = "Program";
        rtl::OString aPureProgramm( urlParam->getByName( "Program" ) );
        parString[last++] = rtl::OString('\'') + aPureProgramm + rtl::OString('\'');
        parString[last++] = "Database";
        parString[last++] = rtl::OString('\'') + urlParam->getByName( "DatabasePar" ) + rtl::OString('\'');
        parString[last++] = "Id";
        parString[last++] = rtl::OString('\'') + urlParam->getByName( "Id" ) + rtl::OString('\'');
        parString[last++] = "Path";
        rtl::OString aPath( urlParam->getByName( "Path" ) );
        parString[last++] = rtl::OString('\'') + aPath + rtl::OString('\'');

        rtl::OString aPureLanguage = urlParam->getByName( "Language" );
        parString[last++] = "Language";
        parString[last++] = rtl::OString('\'') + aPureLanguage + rtl::OString('\'');
        parString[last++] = "System";
        parString[last++] = rtl::OString('\'') + urlParam->getByName( "System" ) + rtl::OString('\'');
        parString[last++] = "productname";
        parString[last++] = rtl::OString('\'') + rtl::OString(
            pDatabases->getProductName().getStr(),
            pDatabases->getProductName().getLength(),
            RTL_TEXTENCODING_UTF8 ) + rtl::OString('\'');
        parString[last++] = "productversion";
        parString[last++] = rtl::OString('\'') +
            rtl::OString(  pDatabases->getProductVersion().getStr(),
                          pDatabases->getProductVersion().getLength(),
                          RTL_TEXTENCODING_UTF8 ) + rtl::OString('\'');

        parString[last++] = "imgrepos";
        parString[last++] = rtl::OString('\'') + pDatabases->getImagesZipFileURL() + rtl::OString('\'');
        parString[last++] = "hp";
        parString[last++] = rtl::OString('\'') + urlParam->getByName( "HelpPrefix" ) + rtl::OString('\'');

        if( !parString[last-1].isEmpty() )
        {
            parString[last++] = "sm";
            parString[last++] = "'vnd.sun.star.help%3A%2F%2F'";
            parString[last++] = "qm";
            parString[last++] = "'%3F'";
            parString[last++] = "es";
            parString[last++] = "'%3D'";
            parString[last++] = "am";
            parString[last++] = "'%26'";
            parString[last++] = "cl";
            parString[last++] = "'%3A'";
            parString[last++] = "sl";
            parString[last++] = "'%2F'";
            parString[last++] = "hm";
            parString[last++] = "'%23'";
            parString[last++] = "cs";
            parString[last++] = "'css'";

            parString[last++] = "vendorname";
            parString[last++] = rtl::OString("''");
            parString[last++] = "vendorversion";
            parString[last++] = rtl::OString("''");
            parString[last++] = "vendorshort";
            parString[last++] = rtl::OString("''");
        }

        // Do we need to add extension path?
        ::rtl::OUString aExtensionPath;
        rtl::OUString aJar = urlParam->get_jar();

        bool bAddExtensionPath = false;
        rtl::OUString aExtensionRegistryPath;
        sal_Int32 nQuestionMark1 = aJar.indexOf( sal_Unicode('?') );
        sal_Int32 nQuestionMark2 = aJar.lastIndexOf( sal_Unicode('?') );
        if( nQuestionMark1 != -1 && nQuestionMark2 != -1 && nQuestionMark1 != nQuestionMark2 )
        {
            aExtensionPath = aJar.copy( nQuestionMark1 + 1, nQuestionMark2 - nQuestionMark1 - 1 );
            aExtensionRegistryPath = urlParam->get_ExtensionRegistryPath();
            bAddExtensionPath = true;
        }
        else
        {
            // Path not yet specified, search directly
            Reference< XHierarchicalNameAccess > xNA = pDatabases->findJarFileForPath
                ( aJar, urlParam->get_language(), urlParam->get_path(), &aExtensionPath, &aExtensionRegistryPath );
            if( xNA.is() && !aExtensionPath.isEmpty() )
                bAddExtensionPath = true;
        }

        if( bAddExtensionPath )
        {
            Reference< XComponentContext > xContext(
                comphelper::getProcessComponentContext() );

            rtl::OUString aOUExpandedExtensionPath = Databases::expandURL( aExtensionRegistryPath, xContext );
            rtl::OString aExpandedExtensionPath = rtl::OUStringToOString( aOUExpandedExtensionPath, osl_getThreadTextEncoding() );

            parString[last++] = "ExtensionPath";
            parString[last++] = rtl::OString('\'') + aExpandedExtensionPath + rtl::OString('\'');

            // ExtensionId
            rtl::OString aPureExtensionId;
            sal_Int32 iSlash = aPath.indexOf( '/' );
            if( iSlash != -1 )
                aPureExtensionId = aPath.copy( 0, iSlash );

            parString[last++] = "ExtensionId";
            parString[last++] = rtl::OString('\'') + aPureExtensionId + rtl::OString('\'');
        }

        for( int i = 0; i < last; ++i )
            parameter[i] = parString[i].getStr();
        parameter[last] = 0;

        rtl::OUString xslURL = pDatabases->getInstallPathAsURL();

        rtl::OString xslURLascii(
            xslURL.getStr(),
            xslURL.getLength(),
            RTL_TEXTENCODING_UTF8);
        xslURLascii += "main_transform.xsl";

        ugblData = &userData;

        xmlInitParser();
        xmlRegisterInputCallbacks(zipMatch, zipOpen, zipRead, uriClose);
        xmlRegisterInputCallbacks(helpMatch, helpOpen, helpRead, uriClose);
        xmlRegisterInputCallbacks(fileMatch, fileOpen, fileRead, fileClose);

        xsltStylesheetPtr cur =
            xsltParseStylesheetFile((const xmlChar *)xslURLascii.getStr());

        xmlDocPtr doc = xmlParseFile("vnd.sun.star.zip:/");

        xmlDocPtr res = xsltApplyStylesheet(cur, doc, parameter);
        if (res)
        {
            xmlChar *doc_txt_ptr=0;
            int doc_txt_len;
            xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, res, cur);
            addToBuffer((const char*)doc_txt_ptr, doc_txt_len);
            xmlFree(doc_txt_ptr);
        }
        xmlPopInputCallbacks(); //filePatch
        xmlPopInputCallbacks(); //helpPatch
        xmlPopInputCallbacks(); //zipMatch
        xmlFreeDoc(res);
        xmlFreeDoc(doc);
        xsltFreeStylesheet(cur);
    }
}


InputStreamTransformer::~InputStreamTransformer()
{
    delete[] buffer;
}


Any SAL_CALL InputStreamTransformer::queryInterface( const Type& rType ) throw( RuntimeException )
{
    Any aRet = ::cppu::queryInterface( rType,
                                       (static_cast< XInputStream* >(this)),
                                       (static_cast< XSeekable* >(this)) );

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

    int curr,available_ = len-pos;
    if( nBytesToRead <= available_ )
        curr = nBytesToRead;
    else
        curr = available_;

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
        pos = sal::static_int_cast<sal_Int32>( location );

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
    memcpy( (void*)(buffer),(void*)(tmp),sal_uInt32( len ) );
    memcpy( (void*)(buffer+len),(void*)(buffer_),sal_uInt32( len_ ) );
    delete[] tmp;
    len += len_;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
