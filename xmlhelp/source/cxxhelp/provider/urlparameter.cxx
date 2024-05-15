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
#include <osl/thread.h>
#include <osl/file.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/uri.hxx>
#include <rtl/ref.hxx>
#include <rtl/character.hxx>
#include <o3tl/string_view.hxx>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/security.h>
#include "db.hxx"
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include "urlparameter.hxx"
#include "databases.hxx"

#include <algorithm>
#include <memory>
#include <mutex>

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

    return OUStringToOString( val, RTL_TEXTENCODING_UTF8 );
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


OUString const & URLParameter::get_language() const
{
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


OUString const & URLParameter::get_path()
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

    const char* pData = nullptr;

    helpdatafileproxy::HDFData aHDFData;
    OUString aExtensionPath;
    OUString aExtensionRegistryPath;
    while( true )
    {
        helpdatafileproxy::Hdf* pHdf = aDbIt.nextHdf( &aExtensionPath, &aExtensionRegistryPath );
        if( !pHdf )
            break;

        OString keyStr = OUStringToOString( m_aId,RTL_TEXTENCODING_UTF8 );
        bSuccess = pHdf->getValueForKey( keyStr, aHDFData );
        if( bSuccess )
        {
            pData = aHDFData.getData();
            break;
        }
    }

    if( !bSuccess )
        return;

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


// Class encapsulating the transformation of the XInputStream to XHTML

namespace {

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
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead ) override;

    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead ) override;

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;

    virtual sal_Int32 SAL_CALL available() override;

    virtual void SAL_CALL closeInput() override;

    virtual void SAL_CALL seek( sal_Int64 location ) override;

    virtual sal_Int64 SAL_CALL getPosition() override;

    virtual sal_Int64 SAL_CALL getLength() override;

    void addToBuffer( const char* buffer,int len );

    OStringBuffer const & getData() const { return buffer; }

private:

    std::mutex m_aMutex;

    int pos;
    OStringBuffer buffer;
};

}

void URLParameter::open( const Reference< XOutputStream >& xDataSink )
{
    if( ! xDataSink.is() )
        return;

    // a standard document or else an active help text, plug in the new input stream
    rtl::Reference<InputStreamTransformer> p(new InputStreamTransformer( this,m_pDatabases,isRoot() ));
    try
    {
        xDataSink->writeBytes( Sequence< sal_Int8 >( reinterpret_cast<const sal_Int8*>(p->getData().getStr()), p->getData().getLength() ) );
    }
    catch( const Exception& )
    {
    }
    p.clear();
    xDataSink->closeOutput();
}


void URLParameter::open( const Reference< XActiveDataSink >& xDataSink )
{
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
        std::u16string_view aLastStr =
            m_aExpr.subView(sal::static_int_cast<sal_uInt32>(nLen) - 6);
        if( aLastStr == u"DbPAR=" )
        {
            m_aExpr = OUString::Concat(m_aExpr.subView( 0, 20 )) +
                "shared" +
                m_aExpr.subView( 20 ) +
                "shared";
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

    while( idx < length && rtl::isAsciiAlphanumeric( m_aExpr[idx] ) )
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

    if( length != 0 && m_aExpr[0] == '/' )
    {
        sal_Int32 idx = 1;
        while( idx < length && m_aExpr[idx] != '?' )
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
    else if( m_aExpr[0] == '?' )
        query_ = o3tl::trim(m_aExpr.subView( 1 ));
    else
        return false;


    bool ret = true;
    sal_Int32 delimIdx,equalIdx;
    OUString parameter,value;

    while( !query_.isEmpty() )
    {
        delimIdx = query_.indexOf( '&' );
        equalIdx = query_.indexOf( '=' );
        parameter = o3tl::trim(query_.subView( 0,equalIdx ));
        if( delimIdx == -1 )
        {
            value = o3tl::trim(query_.subView( equalIdx + 1 ));
            query_.clear();
        }
        else
        {
            value = o3tl::trim(query_.subView( equalIdx+1,delimIdx - equalIdx - 1 ));
            query_ = o3tl::trim(query_.subView( delimIdx+1 ));
        }

        if( parameter == "Language" )
            m_aLanguage = value;
        else if( parameter == "Device" )
            ;
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
            ; // ignored (but accepted) in the built-in help, useful only for the online help
        else
            ret = false;
    }

    return ret;
}

namespace {

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

}

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
    (void)pRet->open(osl_File_OpenFlag_Read);
    return pRet;
}

static void *
zipOpen(SAL_UNUSED_PARAMETER const char *) {
    OUString language,jar,path;

    if( !ugblData->m_pInitial->get_eid().isEmpty() )
        return new Reference<XHierarchicalNameAccess>;
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
    : pos( 0 )
{
    if( isRoot )
    {
        buffer.setLength(0);
        pDatabases->cascadingStylesheet( urlParam->get_language(),
                                         buffer );
    }
    else if( urlParam->isActive() )
    {
        buffer.setLength(0);
        pDatabases->setActiveText( urlParam->get_module(),
                                   urlParam->get_language(),
                                   urlParam->get_id(),
                                   buffer );
    }
    else
    {
        UserData userData( urlParam,pDatabases );

        // Uses the implementation detail, that OString::getStr returns a zero terminated character-array

        const char* parameter[47];
        OString parString[46];
        int last = 0;

        parString[last++] = "Program"_ostr;
        OString aPureProgramm( urlParam->getByName( "Program" ) );
        parString[last++] = "'" + aPureProgramm + "'";
        parString[last++] = "Database"_ostr;
        parString[last++] = "'" + urlParam->getByName( "DatabasePar" ) + "'";
        parString[last++] = "Id"_ostr;
        parString[last++] = "'" + urlParam->getByName( "Id" ) + "'";
        parString[last++] = "Path"_ostr;
        OString aPath( urlParam->getByName( "Path" ) );
        parString[last++] = "'" + aPath + "'";

        OString aPureLanguage = urlParam->getByName( "Language" );
        parString[last++] = "Language"_ostr;
        parString[last++] = "'" + aPureLanguage + "'";
        parString[last++] = "System"_ostr;
        parString[last++] = "'" + urlParam->getByName( "System" ) + "'";
        parString[last++] = "productname"_ostr;
        parString[last++] = "'" + OUStringToOString(
            pDatabases->getProductName(),
            RTL_TEXTENCODING_UTF8 ) + "'";
        parString[last++] = "productversion"_ostr;
        parString[last++] = "'" +
            OUStringToOString(  pDatabases->getProductVersion(),
                          RTL_TEXTENCODING_UTF8 ) + "'";

        parString[last++] = "imgtheme"_ostr;
        parString[last++] = "'" + chelp::Databases::getImageTheme() + "'";
        parString[last++] = "hp"_ostr;
        parString[last++] = "'" + urlParam->getByName( "HelpPrefix" ) + "'";

        if( !parString[last-1].isEmpty() )
        {
            parString[last++] = "sm"_ostr;
            parString[last++] = "'vnd.sun.star.help%3A%2F%2F'"_ostr;
            parString[last++] = "qm"_ostr;
            parString[last++] = "'%3F'"_ostr;
            parString[last++] = "es"_ostr;
            parString[last++] = "'%3D'"_ostr;
            parString[last++] = "am"_ostr;
            parString[last++] = "'%26'"_ostr;
            parString[last++] = "cl"_ostr;
            parString[last++] = "'%3A'"_ostr;
            parString[last++] = "sl"_ostr;
            parString[last++] = "'%2F'"_ostr;
            parString[last++] = "hm"_ostr;
            parString[last++] = "'%23'"_ostr;
            parString[last++] = "cs"_ostr;
            parString[last++] = "'css'"_ostr;

            parString[last++] = "vendorname"_ostr;
            parString[last++] = "''"_ostr;
            parString[last++] = "vendorversion"_ostr;
            parString[last++] = "''"_ostr;
            parString[last++] = "vendorshort"_ostr;
            parString[last++] = "''"_ostr;
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

            parString[last++] = "ExtensionPath"_ostr;
            parString[last++] = "'" + aExpandedExtensionPath + "'";

            // ExtensionId
            OString aPureExtensionId;
            sal_Int32 iSlash = aPath.indexOf( '/' );
            if( iSlash != -1 )
                aPureExtensionId = aPath.copy( 0, iSlash );

            parString[last++] = "ExtensionId"_ostr;
            parString[last++] = "'" + aPureExtensionId + "'";
        }

        for( int i = 0; i < last; ++i )
            parameter[i] = parString[i].getStr();
        parameter[last] = nullptr;

        OUString xslURL = pDatabases->getInstallPathAsURL();

        OString xslURLascii = OUStringToOString(
                xslURL,
                RTL_TEXTENCODING_UTF8) +
            "main_transform.xsl";

        ugblData = &userData;

        xmlInitParser();
        xmlRegisterInputCallbacks(zipMatch, zipOpen, zipRead, uriClose);
        xmlRegisterInputCallbacks(helpMatch, helpOpen, helpRead, uriClose);
        xmlRegisterInputCallbacks(fileMatch, fileOpen, fileRead, fileClose);

        xsltStylesheetPtr cur =
            xsltParseStylesheetFile(reinterpret_cast<const xmlChar *>(xslURLascii.getStr()));

        xmlDocPtr doc = xmlParseFile("vnd.sun.star.zip:/");

        xmlDocPtr res = nullptr;
        xsltTransformContextPtr transformContext = xsltNewTransformContext(cur, doc);
        if (transformContext)
        {
            xsltSecurityPrefsPtr securityPrefs = xsltNewSecurityPrefs();
            if (securityPrefs)
            {
                xsltSetSecurityPrefs(securityPrefs, XSLT_SECPREF_READ_FILE, xsltSecurityAllow);
                if (xsltSetCtxtSecurityPrefs(securityPrefs, transformContext) == 0)
                {
                    res = xsltApplyStylesheetUser(cur, doc, parameter, nullptr, nullptr, transformContext);
                    if (res)
                    {
                        xmlChar *doc_txt_ptr=nullptr;
                        int doc_txt_len;
                        xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, res, cur);
                        addToBuffer(reinterpret_cast<char*>(doc_txt_ptr), doc_txt_len);
                        xmlFree(doc_txt_ptr);
                    }
                }
                xsltFreeSecurityPrefs(securityPrefs);
            }
            xsltFreeTransformContext(transformContext);
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
                                       static_cast< XInputStream* >(this),
                                       static_cast< XSeekable* >(this) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL InputStreamTransformer::acquire() noexcept
{
    OWeakObject::acquire();
}


void SAL_CALL InputStreamTransformer::release() noexcept
{
    OWeakObject::release();
}


sal_Int32 SAL_CALL InputStreamTransformer::readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead )
{
    std::scoped_lock aGuard( m_aMutex );

    int curr,available_ = buffer.getLength() - pos;
    if( nBytesToRead <= available_ )
        curr = nBytesToRead;
    else
        curr = available_;

    if( 0 <= curr && aData.getLength() < curr )
        aData.realloc( curr );

    std::copy_n(buffer.getStr() + pos, curr, aData.getArray());
    pos += curr;

    return std::max(curr, 0);
}


sal_Int32 SAL_CALL InputStreamTransformer::readSomeBytes( Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
{
    return readBytes( aData,nMaxBytesToRead );
}


void SAL_CALL InputStreamTransformer::skipBytes( sal_Int32 nBytesToSkip )
{
    std::scoped_lock aGuard( m_aMutex );
    while( nBytesToSkip-- ) ++pos;
}


sal_Int32 SAL_CALL InputStreamTransformer::available()
{
    std::scoped_lock aGuard( m_aMutex );
    return std::min<sal_Int64>(SAL_MAX_INT32, buffer.getLength() - pos);
}


void SAL_CALL InputStreamTransformer::closeInput()
{
}


void SAL_CALL InputStreamTransformer::seek( sal_Int64 location )
{
    std::scoped_lock aGuard( m_aMutex );
    if( location < 0 )
        throw IllegalArgumentException();

    pos = sal::static_int_cast<sal_Int32>( location );

    if( pos > buffer.getLength() )
        pos = buffer.getLength();
}


sal_Int64 SAL_CALL InputStreamTransformer::getPosition()
{
    std::scoped_lock aGuard( m_aMutex );
    return sal_Int64( pos );
}


sal_Int64 SAL_CALL InputStreamTransformer::getLength()
{
    std::scoped_lock aGuard( m_aMutex );

    return buffer.getLength();
}


void InputStreamTransformer::addToBuffer( const char* buffer_,int len_ )
{
    std::scoped_lock aGuard( m_aMutex );

    buffer.append( buffer_, len_ );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
