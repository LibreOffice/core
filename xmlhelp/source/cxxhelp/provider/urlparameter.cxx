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


#include <string.h>
#include <osl/diagnose.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/character.hxx>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include "db.hxx"
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
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

#include <memory>

using namespace cppu;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace chelp;


URLParameter::URLParameter( const OUString& aURL,
                            Databases* pDatabases )
    : m_pDatabases( pDatabases ),
      m_aURL( aURL )
{
    init();
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


OString URLParameter::getByName( const char* par )
{
    OUString val;

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
        val = m_aPrefix;

    return OString( val.getStr(),val.getLength(),RTL_TEXTENCODING_UTF8 );
}


OUString const & URLParameter::get_id()
{
    if( m_aId == "start" )
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

OUString URLParameter::get_tag()
{
    if( isFile() )
        return get_the_tag();
    else
        return m_aTag;
}


OUString URLParameter::get_title()
{
    if( isFile() )
        return get_the_title();
    else if( !m_aModule.isEmpty() )
    {
        StaticModuleInformation* inf =
            m_pDatabases->getStaticInformationForModule( get_module(),
                                                         get_language() );
        if( inf )
            m_aTitle = inf->get_title();
    }
    else   // This must be the root
        m_aTitle = "root";

    return m_aTitle;
}


OUString const & URLParameter::get_language()
{
    if( m_aLanguage.isEmpty() )
        return m_aDefaultLanguage;

    return m_aLanguage;
}


OUString const & URLParameter::get_program()
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


void URLParameter::init()
{
    m_bHelpDataFileRead = false;
    m_bStart = false;
    m_bUseDB = true;
    m_nHitCount = 100;                // The default maximum hitcount
}


OUString URLParameter::get_the_tag()
{
    if(m_bUseDB) {
        if( ! m_bHelpDataFileRead )
            readHelpDataFile();

        m_bHelpDataFileRead = true;

        return m_aTag;
    }
    else
        return OUString();
}


OUString URLParameter::get_path()
{
    if(m_bUseDB) {
        if( ! m_bHelpDataFileRead )
            readHelpDataFile();
        m_bHelpDataFileRead = true;

        return m_aPath;
    }
    else
        return get_id();
}


OUString URLParameter::get_the_title()
{
    if(m_bUseDB) {
        if( ! m_bHelpDataFileRead )
            readHelpDataFile();
        m_bHelpDataFileRead = true;

        return m_aTitle;
    }
    else
        return OUString();
}


OUString URLParameter::get_jar()
{
    if(m_bUseDB) {
        if( ! m_bHelpDataFileRead )
            readHelpDataFile();
        m_bHelpDataFileRead = true;

        return m_aJar;
    }
    else
        return get_module() + ".jar";
}


void URLParameter::readHelpDataFile()
{
    if( get_id().isEmpty() )
        return;

    OUString aModule = get_module();
    OUString aLanguage = get_language();

    DataBaseIterator aDbIt( *m_pDatabases, aModule, aLanguage, false );
    bool bSuccess = false;

    const sal_Char* pData = nullptr;

    helpdatafileproxy::HDFData aHDFData;
    OUString aExtensionPath;
    OUString aExtensionRegistryPath;
    while( true )
    {
        helpdatafileproxy::Hdf* pHdf = aDbIt.nextHdf( &aExtensionPath, &aExtensionRegistryPath );
        if( !pHdf )
            break;

        OString keyStr( m_aId.getStr(),m_aId.getLength(),RTL_TEXTENCODING_UTF8 );
        bSuccess = pHdf->getValueForKey( keyStr, aHDFData );
        if( bSuccess )
        {
            pData = aHDFData.getData();
            break;
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
            m_aJar = "?" + aExtensionPath + "?" + m_aJar;
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
                            bool isRoot );

    virtual Any SAL_CALL queryInterface( const Type& rType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead ) override;

    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead ) override;

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;

    virtual sal_Int32 SAL_CALL available() override;

    virtual void SAL_CALL closeInput() override;

    virtual void SAL_CALL seek( sal_Int64 location ) override;

    virtual sal_Int64 SAL_CALL getPosition() override;

    virtual sal_Int64 SAL_CALL getLength() override;

    void addToBuffer( const char* buffer,int len );

    sal_Int8 const * getData() const { return reinterpret_cast<sal_Int8 const *>(buffer.get()); }

    sal_Int32 getLen() const { return sal_Int32( len ); }

private:

    osl::Mutex m_aMutex;

    int len,pos;
    std::unique_ptr<char[]> buffer;
};


void URLParameter::open( const Command& aCommand,
                         sal_Int32 CommandId,
                         const Reference< XCommandEnvironment >& Environment,
                         const Reference< XOutputStream >& xDataSink )
{
    (void)aCommand;
    (void)CommandId;
    (void)Environment;

    if( ! xDataSink.is() )
        return;

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
    xDataSink->closeOutput();
}


void URLParameter::open( const Command& aCommand,
                         sal_Int32 CommandId,
                         const Reference< XCommandEnvironment >& Environment,
                         const Reference< XActiveDataSink >& xDataSink )
{
    (void)aCommand;
    (void)CommandId;
    (void)Environment;

    // a standard document or else an active help text, plug in the new input stream
    xDataSink->setInputStream( new InputStreamTransformer( this,m_pDatabases,isRoot() ) );
}


void URLParameter::parse()
{
    m_aExpr = m_aURL;

    sal_Int32 lstIdx = m_aExpr.lastIndexOf( '#' );
    if( lstIdx != -1 )
        m_aExpr = m_aExpr.copy( 0,lstIdx );

    if( ! scheme() ||
        ! name( module() ) ||
        ! query() ||
        m_aLanguage.isEmpty() ||
        m_aSystem.isEmpty() )
        throw css::ucb::IllegalIdentifierException();
}


bool URLParameter::scheme()
{
    // Correct extension help links as sometimes the
    // module is missing resulting in a malformed URL
    if( m_aExpr.startsWith("vnd.sun.star.help:///") )
    {
        sal_Int32 nLen = m_aExpr.getLength();
        OUString aLastStr =
            m_aExpr.copy(sal::static_int_cast<sal_uInt32>(nLen) - 6);
        if( aLastStr == "DbPAR=" )
        {
            OUString aNewExpr = m_aExpr.copy( 0, 20 );
            OUString aSharedStr("shared");
            aNewExpr += aSharedStr;
            aNewExpr += m_aExpr.copy( 20 );
            aNewExpr += aSharedStr;
            m_aExpr = aNewExpr;
        }
    }

    for( sal_Int32 nPrefixLen = 20 ; nPrefixLen >= 18 ; --nPrefixLen )
    {
        if( m_aExpr.matchAsciiL( "vnd.sun.star.help://", nPrefixLen ) )
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

    while( idx < length && rtl::isAsciiAlphanumeric( (m_aExpr.getStr())[idx] ) )
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

    if( length != 0 && (m_aExpr.getStr())[0] == '/' )
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
    OUString query_;

    if( m_aExpr.isEmpty() )
        return true;
    else if( (m_aExpr.getStr())[0] == '?' )
        query_ = m_aExpr.copy( 1 ).trim();
    else
        return false;


    bool ret = true;
    sal_Int32 delimIdx,equalIdx;
    OUString parameter,value;

    while( !query_.isEmpty() )
    {
        delimIdx = query_.indexOf( '&' );
        equalIdx = query_.indexOf( '=' );
        parameter = query_.copy( 0,equalIdx ).trim();
        if( delimIdx == -1 )
        {
            value = query_.copy( equalIdx + 1 ).trim();
            query_.clear();
        }
        else
        {
            value = query_.copy( equalIdx+1,delimIdx - equalIdx - 1 ).trim();
            query_ = query_.copy( delimIdx+1 ).trim();
        }

        if( parameter == "Language" )
            m_aLanguage = value;
        else if( parameter == "Device" )
            m_aDevice = value;
        else if( parameter == "Program" )
            m_aProgram = value;
        else if( parameter == "Eid" )
            m_aEid = value;
        else if( parameter == "UseDB" )
            m_bUseDB = value != "no";
        else if( parameter == "DbPAR" )
            m_aDbPar = value;
        else if( parameter == "Query" )
        {
            if( m_aQuery.isEmpty() )
                m_aQuery = value;
            else
                m_aQuery += " " + value;
        }
        else if( parameter == "Scope" )
            m_aScope = value;
        else if( parameter == "System" )
            m_aSystem = value;
        else if( parameter == "HelpPrefix" )
            m_aPrefix = rtl::Uri::decode(
                value,
                rtl_UriDecodeWithCharset,
                RTL_TEXTENCODING_UTF8 );
        else if( parameter == "HitCount" )
            m_nHitCount = value.toInt32();
        else if( parameter == "Active" )
            m_aActive = value;
        else if( parameter == "Version" )
            ; // ignored (but accepted) in the build-in help, useful only for the online help
        else
            ret = false;
    }

    return ret;
}

struct UserData {

    UserData( URLParameter*           pInitial,
              Databases*              pDatabases )
        : m_pDatabases( pDatabases ),
          m_pInitial( pInitial )
    {
    }

    Databases*                          m_pDatabases;
    URLParameter*                       m_pInitial;
};

static UserData *ugblData = nullptr;

extern "C" {

static int
fileMatch(const char * URI) {
    if ((URI != nullptr) && !strncmp(URI, "file:/", 6))
        return 1;
    return 0;
}

static int
zipMatch(const char * URI) {
    if ((URI != nullptr) && !strncmp(URI, "vnd.sun.star.zip:/", 18))
        return 1;
    return 0;
}

static int
helpMatch(const char * URI) {
    if ((URI != nullptr) && !strncmp(URI, "vnd.sun.star.help:/", 19))
        return 1;
    return 0;
}

static void *
fileOpen(const char *URI) {
    osl::File *pRet = new osl::File(OUString(URI, strlen(URI), RTL_TEXTENCODING_UTF8));
    pRet->open(osl_File_OpenFlag_Read);
    return pRet;
}

static void *
zipOpen(SAL_UNUSED_PARAMETER const char *) {
    OUString language,jar,path;

    if( !ugblData->m_pInitial->get_eid().isEmpty() )
        return static_cast<void*>(new Reference< XHierarchicalNameAccess >);
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
    return nullptr;
}

static void *
helpOpen(const char * URI) {
    OUString language,jar,path;

    URLParameter urlpar( OUString::createFromAscii( URI ),
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
    return nullptr;
}

static int
helpRead(void * context, char * buffer, int len) {
    Reference< XInputStream > *pRef = static_cast<Reference< XInputStream >*>(context);

    Sequence< sal_Int8 > aSeq;
    len = (*pRef)->readBytes( aSeq,len);
    memcpy(buffer, aSeq.getConstArray(), len);

    return len;
}

static int
zipRead(void * context, char * buffer, int len) {
    return helpRead(context, buffer, len);
}

static int
fileRead(void * context, char * buffer, int len) {
    int nRead = 0;
    osl::File *pFile = static_cast<osl::File*>(context);
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
    Reference< XInputStream > *pRef = static_cast<Reference< XInputStream >*>(context);
    delete pRef;
    return 0;
}

static int
fileClose(void * context) {
    osl::File *pFile = static_cast<osl::File*>(context);
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
        buffer.reset();
        pDatabases->cascadingStylesheet( urlParam->get_language(),
                                         buffer,
                                         &len );
    }
    else if( urlParam->isActive() )
    {
        buffer.reset();
        pDatabases->setActiveText( urlParam->get_module(),
                                   urlParam->get_language(),
                                   urlParam->get_id(),
                                   buffer,
                                   &len );
    }
    else
    {
        UserData userData( urlParam,pDatabases );

        // Uses the implementation detail, that OString::getStr returns a zero terminated character-array

        const char* parameter[47];
        OString parString[46];
        int last = 0;

        parString[last++] = "Program";
        OString aPureProgramm( urlParam->getByName( "Program" ) );
        parString[last++] = OString('\'') + aPureProgramm + OString('\'');
        parString[last++] = "Database";
        parString[last++] = OString('\'') + urlParam->getByName( "DatabasePar" ) + OString('\'');
        parString[last++] = "Id";
        parString[last++] = OString('\'') + urlParam->getByName( "Id" ) + OString('\'');
        parString[last++] = "Path";
        OString aPath( urlParam->getByName( "Path" ) );
        parString[last++] = OString('\'') + aPath + OString('\'');

        OString aPureLanguage = urlParam->getByName( "Language" );
        parString[last++] = "Language";
        parString[last++] = OString('\'') + aPureLanguage + OString('\'');
        parString[last++] = "System";
        parString[last++] = OString('\'') + urlParam->getByName( "System" ) + OString('\'');
        parString[last++] = "productname";
        parString[last++] = OString('\'') + OString(
            pDatabases->getProductName().getStr(),
            pDatabases->getProductName().getLength(),
            RTL_TEXTENCODING_UTF8 ) + OString('\'');
        parString[last++] = "productversion";
        parString[last++] = OString('\'') +
            OString(  pDatabases->getProductVersion().getStr(),
                          pDatabases->getProductVersion().getLength(),
                          RTL_TEXTENCODING_UTF8 ) + OString('\'');

        parString[last++] = "imgtheme";
        parString[last++] = OString('\'') + pDatabases->getImageTheme() + OString('\'');
        parString[last++] = "hp";
        parString[last++] = OString('\'') + urlParam->getByName( "HelpPrefix" ) + OString('\'');

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
            parString[last++] = OString("''");
            parString[last++] = "vendorversion";
            parString[last++] = OString("''");
            parString[last++] = "vendorshort";
            parString[last++] = OString("''");
        }

        // Do we need to add extension path?
        OUString aExtensionPath;
        OUString aJar = urlParam->get_jar();

        bool bAddExtensionPath = false;
        OUString aExtensionRegistryPath;
        sal_Int32 nQuestionMark1 = aJar.indexOf( '?' );
        sal_Int32 nQuestionMark2 = aJar.lastIndexOf( '?' );
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

            OUString aOUExpandedExtensionPath = Databases::expandURL( aExtensionRegistryPath, xContext );
            OString aExpandedExtensionPath = OUStringToOString( aOUExpandedExtensionPath, osl_getThreadTextEncoding() );

            parString[last++] = "ExtensionPath";
            parString[last++] = OString('\'') + aExpandedExtensionPath + OString('\'');

            // ExtensionId
            OString aPureExtensionId;
            sal_Int32 iSlash = aPath.indexOf( '/' );
            if( iSlash != -1 )
                aPureExtensionId = aPath.copy( 0, iSlash );

            parString[last++] = "ExtensionId";
            parString[last++] = OString('\'') + aPureExtensionId + OString('\'');
        }

        for( int i = 0; i < last; ++i )
            parameter[i] = parString[i].getStr();
        parameter[last] = nullptr;

        OUString xslURL = pDatabases->getInstallPathAsURL();

        OString xslURLascii(
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
            xsltParseStylesheetFile(reinterpret_cast<const xmlChar *>(xslURLascii.getStr()));

        xmlDocPtr doc = xmlParseFile("vnd.sun.star.zip:/");

        xmlDocPtr res = xsltApplyStylesheet(cur, doc, parameter);
        if (res)
        {
            xmlChar *doc_txt_ptr=nullptr;
            int doc_txt_len;
            xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, res, cur);
            addToBuffer(reinterpret_cast<char*>(doc_txt_ptr), doc_txt_len);
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


Any SAL_CALL InputStreamTransformer::queryInterface( const Type& rType )
{
    Any aRet = ::cppu::queryInterface( rType,
                                       (static_cast< XInputStream* >(this)),
                                       (static_cast< XSeekable* >(this)) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL InputStreamTransformer::acquire() throw()
{
    OWeakObject::acquire();
}


void SAL_CALL InputStreamTransformer::release() throw()
{
    OWeakObject::release();
}


sal_Int32 SAL_CALL InputStreamTransformer::readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead )
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
{
    return readBytes( aData,nMaxBytesToRead );
}


void SAL_CALL InputStreamTransformer::skipBytes( sal_Int32 nBytesToSkip )
{
    osl::MutexGuard aGuard( m_aMutex );
    while( nBytesToSkip-- ) ++pos;
}


sal_Int32 SAL_CALL InputStreamTransformer::available()
{
    osl::MutexGuard aGuard( m_aMutex );
    return len-pos > 0 ? len - pos : 0 ;
}


void SAL_CALL InputStreamTransformer::closeInput()
{
}


void SAL_CALL InputStreamTransformer::seek( sal_Int64 location )
{
    osl::MutexGuard aGuard( m_aMutex );
    if( location < 0 )
        throw IllegalArgumentException();
    else
        pos = sal::static_int_cast<sal_Int32>( location );

    if( pos > len )
        pos = len;
}


sal_Int64 SAL_CALL InputStreamTransformer::getPosition()
{
    osl::MutexGuard aGuard( m_aMutex );
    return sal_Int64( pos );
}


sal_Int64 SAL_CALL InputStreamTransformer::getLength()
{
    osl::MutexGuard aGuard( m_aMutex );

    return len;
}


void InputStreamTransformer::addToBuffer( const char* buffer_,int len_ )
{
    osl::MutexGuard aGuard( m_aMutex );

    std::unique_ptr<char[]> tmp(buffer.release());
    buffer.reset( new char[ len+len_ ] );
    memcpy( static_cast<void*>(buffer.get()),static_cast<void*>(tmp.get()),sal_uInt32( len ) );
    memcpy( static_cast<void*>(buffer.get()+len),static_cast<void const *>(buffer_),sal_uInt32( len_ ) );
    len += len_;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
