/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <memory>
#include "db.hxx"
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/i18n/Collator.hpp>
#include <comphelper/propertysequence.hxx>
#include <algorithm>
#include <cassert>
#include <string.h>
#include <string_view>

#include <helpcompiler/HelpIndexer.hxx>

// Extensible help
#include <comphelper/processfactory.hxx>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarExpandUrl.hpp>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/storagehelper.hxx>
#include <officecfg/Office/Common.hxx>
#include <utility>

#include "databases.hxx"
#include "urlparameter.hxx"

#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

using namespace chelp;
using namespace com::sun::star;
using namespace ::cpo;
using namespace ::cpo::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::container;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

OUString Databases::expandURL( const OUString& aURL )
{
    std::unique_lock aGuard(m_aMutex);
    return expandURL(aGuard, aURL);
}

OUString Databases::expandURL( std::unique_lock<std::mutex>& /*rGuard*/, const OUString& aURL )
{
    OUString aRetURL = expandURL( aURL, m_xContext );
    return aRetURL;
}

OUString Databases::expandURL( const OUString& aURL, const Reference< cpo::uno::XComponentContext >& xContext )
{
    static Reference< util::XMacroExpander > xMacroExpander;
    static Reference< uri::XUriReferenceFactory > xFac;

    if( !xMacroExpander.is() || !xFac.is() )
    {
        xFac = uri::UriReferenceFactory::create( xContext );

        xMacroExpander = util::theMacroExpander::get(xContext);
    }

    OUString aRetURL = aURL;
    if( xMacroExpander.is() )
    {
        Reference< uri::XUriReference > uriRef;
        for (;;)
        {
            uriRef = xFac->parse( aRetURL );
            if ( uriRef.is() )
            {
                Reference < uri::XVndSunStarExpandUrl > sxUri( uriRef, UNO_QUERY );
                if( !sxUri.is() )
                    break;

                aRetURL = sxUri->expand( xMacroExpander );
            }
        }
    }
    return aRetURL;
}

Databases::Databases( bool showBasic,
                      const OUString& instPath,
                      const OUString& productName,
                      const OUString& productVersion,
                      const OUString& styleSheet,
                      Reference< cpo::uno::XComponentContext > const & xContext )
    : m_xContext( xContext ),
      m_bShowBasic(showBasic),
      m_aCSS(styleSheet.toAsciiLowerCase())
{
    m_vAdd[0] = 12;
    m_vAdd[1] = 15;
    m_vAdd[2] = 11;
    m_vAdd[3] = 14;
    m_vAdd[4] = 12;
    m_vAdd[5] = 13;
    m_vAdd[6] = 16;

    m_vReplacement[0] = productName;
    m_vReplacement[1] = productVersion;
    // m_vReplacement[2...4] (vendorName/-Version/-Short) are empty strings
    m_vReplacement[5] = productName;
    m_vReplacement[6] = productVersion;

    setInstallPath( instPath );

    m_xSFA = ucb::SimpleFileAccess::create(m_xContext);
}

Databases::~Databases()
{
    // unload the databases

    // DatabasesTable
    m_aDatabases.clear();

    //  ModInfoTable
    m_aModInfo.clear();

    // KeywordInfoTable
    m_aKeywordInfo.clear();
}

// static
OString Databases::getImageTheme()
{
    OUString aSymbolsStyleName = officecfg::Office::Common::Misc::SymbolStyle::get();

    if ( aSymbolsStyleName.isEmpty() || aSymbolsStyleName == "auto" )
    {
        aSymbolsStyleName = u"colibre"_ustr;
    }
    return aSymbolsStyleName.toUtf8();
}

void Databases::replaceName( OUString& oustring ) const
{
    sal_Int32 idx = -1,idx1 = -1,idx2 = -1,k = 0,off;
    bool cap = false;
    OUStringBuffer aStrBuf( 0 );

    while( true )
    {
        ++idx;
        idx1 = oustring.indexOf( '%', idx);
        idx2 = oustring.indexOf( '$', idx);

        if(idx1 == -1 && idx2 == -1)
            break;

        if(idx1 == -1)
            idx = idx2;
        else if(idx2 == -1)
            idx = idx1;
        else {
            // no index is zero
            if(idx1 < idx2)
                idx = idx1;
            else if(idx2 < idx1 )
                idx = idx2;
        }

        if( oustring.indexOf( u"%PRODUCTNAME",idx ) == idx )
            off = PRODUCTNAME;
        else if( oustring.indexOf( u"%PRODUCTVERSION",idx ) == idx )
            off = PRODUCTVERSION;
        else if( oustring.indexOf( u"%VENDORNAME",idx ) == idx )
            off = VENDORNAME;
        else if( oustring.indexOf( u"%VENDORVERSION",idx ) == idx )
            off = VENDORVERSION;
        else if( oustring.indexOf( u"%VENDORSHORT",idx ) == idx )
            off = VENDORSHORT;
        else if( oustring.indexOf( u"$[officename]",idx ) == idx )
            off = NEWPRODUCTNAME;
        else if( oustring.indexOf( u"$[officeversion]",idx ) == idx )
            off = NEWPRODUCTVERSION;
        else
            off = -1;

        if( off != -1 )
        {
            if( ! cap )
            {
                cap = true;
                aStrBuf.ensureCapacity( 256 );
            }

            aStrBuf.append( &oustring.getStr()[k],idx - k );
            aStrBuf.append( m_vReplacement[off] );
            k = idx + m_vAdd[off];
        }
    }

    if( cap )
    {
        if( k < oustring.getLength() )
            aStrBuf.append( &oustring.getStr()[k],oustring.getLength()-k );
        oustring = aStrBuf.makeStringAndClear();
    }
}

OUString Databases::getInstallPathAsURL()
{
    std::unique_lock aGuard( m_aMutex );

    return m_aInstallDirectory;
}

const OUString & Databases::getInstallPathAsURL(std::unique_lock<std::mutex>& )
{
    return m_aInstallDirectory;
}

const std::vector< OUString >& Databases::getModuleList( const OUString& Language )
{
    if( m_avModules.empty() )
    {
        OUString  fileName,dirName = getInstallPathAsURL() + processLang( Language );
        osl::Directory dirFile( dirName );

        osl::DirectoryItem aDirItem;
        osl::FileStatus    aStatus( osl_FileStatus_Mask_FileName );

        if( osl::FileBase::E_None != dirFile.open() )
            return m_avModules;

        while( dirFile.getNextItem( aDirItem ) == osl::FileBase::E_None &&
               aDirItem.getFileStatus( aStatus ) == osl::FileBase::E_None )
        {
            if( ! aStatus.isValid( osl_FileStatus_Mask_FileName ) )
                continue;

            fileName = aStatus.getFileName();

            // Check, whether fileName is of the form *.cfg
            if (!fileName.endsWithIgnoreAsciiCase(u".cfg", &fileName)) {
                continue;
            }
            fileName = fileName.toAsciiLowerCase();
            if (fileName == "picture"
                || (!m_bShowBasic && fileName == "sbasic"))
            {
                continue;
            }

            m_avModules.push_back( fileName );
        }
    }
    return m_avModules;
}

StaticModuleInformation* Databases::getStaticInformationForModule( std::u16string_view Module,
                                                                   const OUString& Language )
{
    std::unique_lock aGuard( m_aMutex );

    OUString key = processLang(aGuard, Language) + "/" + Module;

    std::pair< ModInfoTable::iterator,bool > aPair =
        m_aModInfo.emplace(key,nullptr);

    ModInfoTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {
        osl::File cfgFile( m_aInstallDirectory + key + ".cfg" );

        if( osl::FileBase::E_None != cfgFile.open( osl_File_OpenFlag_Read ) )
            it->second = nullptr;
        else
        {
            sal_uInt32 pos = 0;
            sal_uInt64 nRead;
            char buffer[2048];
            sal_Unicode lineBuffer[1028];
            OUStringBuffer fileContent;

            while( osl::FileBase::E_None == cfgFile.read( &buffer,2048,nRead ) && nRead )
                fileContent.append(OUString( buffer,sal_Int32( nRead ),RTL_TEXTENCODING_UTF8 ));

            cfgFile.close();

            const sal_Unicode* str = fileContent.getStr();
            OUString current,program,startid,title;
            OUString order( u"1"_ustr );

            for( sal_Int32 i = 0;i < fileContent.getLength();i++ )
            {
                sal_Unicode ch = str[ i ];
                if( ch == '\n' || ch == '\r' )
                {
                    if( pos )
                    {
                        current = OUString( lineBuffer,pos );

                        if( current.startsWith("Title") )
                        {
                            title = current.copy( current.indexOf( '=' ) + 1 );
                        }
                        else if( current.startsWith("Start") )
                        {
                            startid = current.copy( current.indexOf('=') + 1 );
                        }
                        else if( current.startsWith("Program") )
                        {
                            program = current.copy( current.indexOf('=') + 1 );
                        }
                        else if( current.startsWith("Order") )
                        {
                            order = current.copy( current.indexOf('=') + 1 );
                        }
                    }
                    pos = 0;
                }
                else
                    lineBuffer[ pos++ ] = ch;
            }
            replaceName( title );
            it->second.reset(new StaticModuleInformation( title,
                                                      startid,
                                                      program,
                                                      order ));
        }
    }

    return it->second.get();
}

OUString Databases::processLang( const OUString& Language )
{
    std::unique_lock aGuard( m_aMutex );
    return processLang(aGuard, Language);
}

OUString Databases::processLang( std::unique_lock<std::mutex>& /*rGuard*/, const OUString& Language )
{
    OUString ret;
    LangSetTable::iterator it = m_aLangSet.find( Language );

    if( it == m_aLangSet.end() )
    {
        // XXX the old code looked for '-' and '_' as separator between
        // language and country, no idea if '_' actually still can happen
        // (probably not), but play safe and keep that and transform to proper
        // BCP47.
        const OUString aBcp47( Language.replaceAll( "_", "-"));

        // Try if language tag or fallbacks are installed.
        osl::DirectoryItem aDirItem;
        std::vector<OUString> aFallbacks( LanguageTag( aBcp47).getFallbackStrings(true));
        for (auto const & rFB : aFallbacks)
        {
            if (osl::FileBase::E_None == osl::DirectoryItem::get( m_aInstallDirectory + rFB, aDirItem))
            {
                ret = rFB;
                m_aLangSet[ Language ] = ret;
                break;  // for
            }
        }
    }
    else
        ret = it->second;

    return ret;
}

helpdatafileproxy::Hdf* Databases::getHelpDataFile(std::u16string_view Database,
                            const OUString& Language, bool helpText )
{
    std::unique_lock aGuard( m_aMutex );

    return getHelpDataFile(aGuard, Database, Language, helpText);
}

helpdatafileproxy::Hdf* Databases::getHelpDataFile(std::unique_lock<std::mutex>& rGuard,
        std::u16string_view Database,
                            const OUString& Language, bool helpText )

{
    if( Database.empty() || Language.isEmpty() )
        return nullptr;

    OUString aFileExt( helpText ? u".ht"_ustr : u".db"_ustr );
    OUString dbFileName = OUString::Concat("/") + Database + aFileExt;
    OUString key = processLang( rGuard, Language ) + dbFileName;

    std::pair< DatabasesTable::iterator,bool > aPair =
        m_aDatabases.emplace( key, nullptr);

    DatabasesTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {
        std::unique_ptr<helpdatafileproxy::Hdf> pHdf;

        OUString fileNameHDFHelp( m_aInstallDirectory + key );
        //SimpleFileAccess takes file URLs as arguments!!! Using filenames works accidentally but
        //fails for example when using long path names on Windows (starting with \\?\)
        if( m_xSFA->exists( fileNameHDFHelp ) )
        {
            pHdf.reset(new helpdatafileproxy::Hdf( fileNameHDFHelp, m_xSFA ));
        }

        it->second = std::move(pHdf);
    }

    return it->second.get();
}

Reference< XCollator >
Databases::getCollator(std::unique_lock<std::mutex>&, const OUString& Language)
{
    OUString key = Language;

    CollatorTable::iterator it =
        m_aCollatorTable.emplace( key, Reference< XCollator >() ).first;

    if( ! it->second.is() )
    {
        it->second = Collator::create(m_xContext);
        LanguageTag aLanguageTag( Language);
        OUString countryStr = aLanguageTag.getCountry();
        if( countryStr.isEmpty() )
        {
            const OUString langStr = aLanguageTag.getLanguage();
            if( langStr == "de" )
                countryStr = u"DE"_ustr;
            else if( langStr == "en" )
                countryStr = u"US"_ustr;
            else if( langStr == "es" )
                countryStr = u"ES"_ustr;
            else if( langStr == "it" )
                countryStr = u"IT"_ustr;
            else if( langStr == "fr" )
                countryStr = u"FR"_ustr;
            else if( langStr == "sv" )
                countryStr = u"SE"_ustr;
            else if( langStr == "ja" )
                countryStr = u"JP"_ustr;
            else if( langStr == "ko" )
                countryStr = u"KR"_ustr;

            // XXX NOTE: there are no complex language tags involved in those
            // "add country" cases, only because of this we can use this
            // simplified construction.
            if (!countryStr.isEmpty())
                aLanguageTag.reset( langStr + "-" + countryStr);
        }
        it->second->loadDefaultCollator( aLanguageTag.getLocale(), 0);
    }

    return it->second;
}

namespace chelp {

    struct KeywordElementComparator
    {
        explicit KeywordElementComparator( const Reference< XCollator >& xCollator )
            : m_xCollator( xCollator )
        { }

        bool operator()( const KeywordInfo::KeywordElement& la,
                         const KeywordInfo::KeywordElement& ra ) const
        {
            const OUString& l = la.key;
            const OUString& r = ra.key;

            bool ret;

            if( m_xCollator.is() )
            {
                sal_Int32 l1 = l.indexOf( ';' );
                sal_Int32 l3 = ( l1 == -1 ? l.getLength() : l1 );

                sal_Int32 r1 = r.indexOf( ';' );
                sal_Int32 r3 = ( r1 == -1 ? r.getLength() : r1 );

                sal_Int32 c1 = m_xCollator->compareSubstring( l,0,l3,r,0,r3 );

                if( c1 == +1 )
                    ret = false;
                else if( c1 == 0 )
                {
                    sal_Int32 l2 = l.getLength() - l1 - 1;
                    sal_Int32 r2 = r.getLength() - r1 - 1;
                    ret = ( m_xCollator->compareSubstring( l,1+l1,l2,r,1+r1,r2 ) < 0 );
                }
                else
                    ret = true;
            }
            else
                ret = l < r;

            return ret;
        }

        Reference< XCollator > m_xCollator;
    }; // end struct KeywordElementComparator

}

KeywordInfo::KeywordElement::KeywordElement( Databases const *pDatabases,
                                             helpdatafileproxy::Hdf* pHdf,
                                             OUString ky,
                                             std::u16string_view data )
    : key(std::move( ky ))
{
    pDatabases->replaceName( key );
    init( pDatabases,pHdf,data );
}

void KeywordInfo::KeywordElement::init( Databases const *pDatabases,helpdatafileproxy::Hdf* pHdf, std::u16string_view ids )
{
    std::vector< OUString > id,anchor;
    size_t idx = std::u16string_view::npos;
    size_t k = 0;
    for (;;)
    {
        idx = ids.find( ';', k );
        if( idx == std::u16string_view::npos )
            break;
        size_t h = ids.find( '#', k );
        if( h != std::u16string_view::npos && h < idx )
        {
            // found an anchor
            id.push_back( OUString(ids.substr( k, h-k )) );
            anchor.push_back( OUString(ids.substr( h+1, idx-h-1 )) );
        }
        else
        {
            id.push_back( OUString(ids.substr( k, idx-k )) );
            anchor.emplace_back( );
        }
        k = ++idx;
    }

    listId.realloc( id.size() );
    auto plistId = listId.getArray();
    listAnchor.realloc( id.size() );
    auto plistAnchor = listAnchor.getArray();
    listTitle.realloc( id.size() );
    auto plistTitle = listTitle.getArray();

    for( size_t i = 0; i < id.size(); ++i )
    {
        plistId[i] = id[i];
        plistAnchor[i] = anchor[i];

        helpdatafileproxy::HDFData aHDFData;
        const char* pData = nullptr;

        if( pHdf )
        {
            OString idi = OUStringToOString( id[i], RTL_TEXTENCODING_UTF8 );
            bool bSuccess = pHdf->getValueForKey( idi, aHDFData );
            if( bSuccess )
                pData = aHDFData.getData();
        }

        DbtToStringConverter converter( pData );

        OUString title = converter.getTitle();
        pDatabases->replaceName( title );
        plistTitle[i] = title;
    }
}

KeywordInfo::KeywordInfo( const std::vector< KeywordElement >& aVec )
    : listKey( aVec.size() ),
      listId( aVec.size() ),
      listAnchor( aVec.size() ),
      listTitle( aVec.size() )
{
    auto listKeyRange = asNonConstRange(listKey);
    auto listIdRange = asNonConstRange(listId);
    auto listAnchorRange = asNonConstRange(listAnchor);
    auto listTitleRange = asNonConstRange(listTitle);
    for( size_t i = 0; i < aVec.size(); ++i )
    {
        listKeyRange[i] = aVec[i].key;
        listIdRange[i] = aVec[i].listId;
        listAnchorRange[i] = aVec[i].listAnchor;
        listTitleRange[i] = aVec[i].listTitle;
    }
}
KeywordInfo* Databases::getKeyword( const OUString& Database,
                                    const OUString& Language )
{
    std::unique_lock aGuard( m_aMutex );

    OUString key = processLang(aGuard, Language) + "/" + Database;

    std::pair< KeywordInfoTable::iterator,bool > aPair =
        m_aKeywordInfo.emplace( key,nullptr );

    KeywordInfoTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {
        std::vector<KeywordInfo::KeywordElement> aVector;

        KeyDataBaseFileIterator aDbFileIt( m_xContext, *this, Database, Language );
        OUString fileURL;
        for (;;)
        {
            fileURL = aDbFileIt.nextDbFile(aGuard);
            if( fileURL.isEmpty() )
                break;
            OUString fileNameHDFHelp( fileURL );
            if( m_xSFA->exists( fileNameHDFHelp ) )
            {
                helpdatafileproxy::Hdf aHdf( fileNameHDFHelp, m_xSFA );
                helpdatafileproxy::HDFData aKey;
                helpdatafileproxy::HDFData aValue;
                if( aHdf.startIteration() )
                {
                    helpdatafileproxy::Hdf* pHdf = getHelpDataFile(aGuard, Database,Language );
                    if( pHdf != nullptr )
                    {
                        pHdf->releaseHashMap();
                        pHdf->createHashMap( true/*bOptimizeForPerformance*/ );
                    }

                    while( aHdf.getNextKeyAndValue( aKey, aValue ) )
                    {
                        OUString keyword( aKey.getData(), aKey.getSize(),
                                               RTL_TEXTENCODING_UTF8 );
                        OUString doclist( aValue.getData(), aValue.getSize(),
                                               RTL_TEXTENCODING_UTF8 );

                        aVector.emplace_back( this,
                                                                        pHdf,
                                                                        keyword,
                                                                        doclist );
                    }
                    aHdf.stopIteration();

                    if( pHdf != nullptr )
                        pHdf->releaseHashMap();
                }
            }
        }

        // sorting
        Reference<XCollator> xCollator = getCollator(aGuard, Language);
        std::sort(aVector.begin(), aVector.end(), KeywordElementComparator(xCollator));

        it->second.reset(new KeywordInfo( aVector ));
    }

    return it->second.get();
}

Reference< XHierarchicalNameAccess > Databases::jarFile(
        std::unique_lock<std::mutex>& rGuard, std::u16string_view jar,
                                                         const OUString& Language )
{
    if( jar.empty() || Language.isEmpty() )
    {
        return Reference< XHierarchicalNameAccess >( nullptr );
    }

    OUString key = processLang(rGuard, Language) + "/" + jar;

    cpo::uno::Reference< css::container::XHierarchicalNameAccess > xHNameAccess;
    try
    {
        OUString zipFile = m_aInstallDirectory + key;

        // create the package zip file
        Sequence< Any > aArguments{
            Any(zipFile),
            // let ZipPackage be used
            Any(beans::NamedValue(u"StorageFormat"_ustr, Any(ZIP_STORAGE_FORMAT_STRING)))
        };

        xHNameAccess.set(
            m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            u"com.sun.star.packages.comp.ZipPackage"_ustr,
            aArguments, m_xContext ), UNO_QUERY);
    }
    catch ( RuntimeException & )
    {
    }
    catch ( Exception & )
    {
    }
    // get root zip folder
    return xHNameAccess;
}

Reference< XHierarchicalNameAccess > Databases::findJarFileForPath
    ( const OUString& jar, const OUString& Language,
      const OUString& path )
{
    Reference< XHierarchicalNameAccess > xNA;
    if( jar.isEmpty() || Language.isEmpty() )
    {
        return xNA;
    }

    ::std::unique_lock aGuard(m_aMutex);

    JarFileIterator aJarFileIt( m_xContext, *this, jar, Language );
    Reference< XHierarchicalNameAccess > xTestNA;
    for (;;)
    {
        xTestNA = aJarFileIt.nextJarFile(aGuard);
        if( !xTestNA.is() )
            break;
        if( xTestNA->hasByHierarchicalName( path ) )
        {
            xNA = xTestNA;
            break;
        }
    }

    return xNA;
}

void Databases::changeCSS(const OUString& newStyleSheet)
{
    m_aCSS = newStyleSheet.toAsciiLowerCase();
    m_vCustomCSSDoc.clear();
}

void Databases::cascadingStylesheet( const OUString& Language,
                                     OStringBuffer& buffer )
{
    if( m_vCustomCSSDoc.empty() )
    {
        int retry = 2;
        bool error = true;
        OUString fileURL;

        bool bHighContrastMode = false;
        OUString aCSS( m_aCSS );
        if ( aCSS == "default" )
        {
            // #i50760: "default" needs to adapt HC mode
            uno::Reference< awt::XToolkit > xToolkit =
                   awt::Toolkit::create( ::comphelper::getProcessComponentContext() );
            uno::Reference< awt::XTopWindow > xTopWindow = xToolkit->getActiveTopWindow();
            if ( xTopWindow.is() )
            {
                uno::Reference< awt::XVclWindowPeer > xVclWindowPeer( xTopWindow, uno::UNO_QUERY );
                if ( xVclWindowPeer.is() )
                {
                    cpo::uno::Any aHCMode = xVclWindowPeer->getProperty( u"HighContrastMode"_ustr );
                    if ( ( aHCMode >>= bHighContrastMode ) && bHighContrastMode )
                    {
                        aCSS = u"highcontrastblack"_ustr;
                        #ifdef _WIN32
                        HKEY hKey = nullptr;
                        LONG lResult = RegOpenKeyExW( HKEY_CURRENT_USER, L"Control Panel\\Accessibility\\HighContrast", 0, KEY_QUERY_VALUE, &hKey );
                        if ( ERROR_SUCCESS == lResult )
                        {
                            WCHAR szBuffer[1024];
                            DWORD nSize = sizeof( szBuffer );
                            lResult = RegQueryValueExW( hKey, L"High Contrast Scheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer), &nSize );
                            if ( ERROR_SUCCESS == lResult && nSize > 0 )
                            {
                                szBuffer[nSize] = '\0';
                                if ( wcscmp( szBuffer, L"High Contrast #1" ) == 0 )
                                    aCSS = u"highcontrast1"_ustr;
                                if ( wcscmp( szBuffer, L"High Contrast #2" ) == 0 )
                                    aCSS = u"highcontrast2"_ustr;
                                if ( wcscmp( szBuffer, L"High Contrast White" ) == 0 )
                                    aCSS = u"highcontrastwhite"_ustr;
                            }
                            RegCloseKey( hKey );
                        }
                        #endif
                    }
                }
            }
        }

        while( error && retry )
        {

            if( retry == 2 )
                fileURL =
                    getInstallPathAsURL()  +
                    processLang( Language )       +
                    "/" +
                    aCSS +
                    ".css";
            else if( retry == 1 )
                fileURL =
                    getInstallPathAsURL()  +
                    aCSS +
                    ".css";

            osl::DirectoryItem aDirItem;
            osl::File aFile( fileURL );
            osl::FileStatus aStatus( osl_FileStatus_Mask_FileSize );

            if( osl::FileBase::E_None == osl::DirectoryItem::get( fileURL,aDirItem ) &&
                osl::FileBase::E_None == aFile.open( osl_File_OpenFlag_Read )        &&
                osl::FileBase::E_None == aDirItem.getFileStatus( aStatus ) )
            {
                sal_uInt64 nSize;
                aFile.getSize( nSize );
                m_vCustomCSSDoc.resize( nSize + 1);
                m_vCustomCSSDoc[nSize] = 0;
                sal_uInt64 a = nSize,b = nSize;
                aFile.read( m_vCustomCSSDoc.data(), a, b );
                aFile.close();
                error = false;
            }

            --retry;
            if ( !retry && error && bHighContrastMode )
            {
                // fall back to default css
                aCSS = u"default"_ustr;
                retry = 2;
                bHighContrastMode = false;
            }
        }

        if( error )
        {
            m_vCustomCSSDoc.clear();
        }
    }

    if (!m_vCustomCSSDoc.empty())
        buffer.append( m_vCustomCSSDoc.data(), m_vCustomCSSDoc.size() - 1 );
}

void Databases::setActiveText( const OUString& Module,
                               const OUString& Language,
                               std::u16string_view Id,
                               OStringBuffer& buffer )
{
    DataBaseIterator aDbIt( m_xContext, *this, Module, Language, true );

    // #i84550 Cache information about failed ids
    OString id = OUStringToOString( Id, RTL_TEXTENCODING_UTF8 );
    EmptyActiveTextSet::iterator it = m_aEmptyActiveTextSet.find( id );
    bool bFoundAsEmpty = ( it != m_aEmptyActiveTextSet.end() );
    helpdatafileproxy::HDFData aHDFData;

    int nSize = 0;
    const char* pData = nullptr;

    bool bSuccess = false;
    if( !bFoundAsEmpty )
    {
        while( !bSuccess )
        {
            helpdatafileproxy::Hdf* pHdf = aDbIt.nextHdf();
            if( !pHdf )
                break;
            bSuccess = pHdf->getValueForKey( id, aHDFData );
            nSize = aHDFData.getSize();
            pData = aHDFData.getData();
        }
    }

    if( bSuccess )
    {
        // ensure existence of tmp after for
        OString tmp;
        for( int i = 0; i < nSize; ++i )
            if( pData[i] == '%' || pData[i] == '$' )
            {
                // need of replacement
                OUString temp( pData, nSize, RTL_TEXTENCODING_UTF8 );
                replaceName( temp );
                tmp = OString( temp.getStr(),
                                    temp.getLength(),
                                    RTL_TEXTENCODING_UTF8 );
                nSize = tmp.getLength();
                pData = tmp.getStr();
                break;
            }

        buffer.append( pData, nSize );
    }
    else
    {
        if( !bFoundAsEmpty )
            m_aEmptyActiveTextSet.insert( id );
    }
}

void Databases::setInstallPath( const OUString& aInstDir )
{
    std::unique_lock aGuard( m_aMutex );

    osl::FileBase::getFileURLFromSystemPath( aInstDir,m_aInstallDirectory );
        //TODO: check returned error code

    if( !m_aInstallDirectory.endsWith( "/" ) )
        m_aInstallDirectory += "/";
}

ExtensionIteratorBase::ExtensionIteratorBase( Reference< XComponentContext > const & xContext,
    Databases& rDatabases, OUString aInitialModule, OUString aLanguage )
        : m_xContext( xContext )
        , m_rDatabases( rDatabases )
        , m_eState( IteratorState::InitialModule )
        , m_aInitialModule(std::move( aInitialModule ))
        , m_aLanguage(std::move( aLanguage ))
{
    assert( m_xContext.is() );
    init();
}

ExtensionIteratorBase::ExtensionIteratorBase( Databases& rDatabases,
    OUString aInitialModule, OUString aLanguage )
        : m_xContext( comphelper::getProcessComponentContext() )
        , m_rDatabases( rDatabases )
        , m_eState( IteratorState::InitialModule )
        , m_aInitialModule(std::move( aInitialModule ))
        , m_aLanguage(std::move( aLanguage ))
{
    init();
}

void ExtensionIteratorBase::init()
{
    m_xSFA = ucb::SimpleFileAccess::create(m_xContext);
}

helpdatafileproxy::Hdf* DataBaseIterator::nextHdf()
{
    helpdatafileproxy::Hdf* pRetHdf = nullptr;

    while( !pRetHdf && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::InitialModule:
                pRetHdf = m_rDatabases.getHelpDataFile( m_aInitialModule, m_aLanguage, m_bHelpText );
                m_eState = IteratorState::EndReached;
                break;

            case IteratorState::EndReached:
                OSL_FAIL( "DataBaseIterator::nextDb(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return pRetHdf;
}
//returns a file URL
OUString KeyDataBaseFileIterator::nextDbFile(std::unique_lock<std::mutex>& rGuard)
{
    OUString aRetFile;

    while( aRetFile.isEmpty() && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::InitialModule:
                aRetFile = m_rDatabases.getInstallPathAsURL(rGuard) +
                        m_rDatabases.processLang(rGuard, m_aLanguage) +
                        "/" +
                        m_aInitialModule + ".key";

                m_eState = IteratorState::EndReached;
                break;

            case IteratorState::EndReached:
                OSL_FAIL( "DataBaseIterator::nextDbFile(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return aRetFile;
}
Reference<XHierarchicalNameAccess> JarFileIterator::nextJarFile(
        std::unique_lock<std::mutex>& rGuard )
{
    Reference< XHierarchicalNameAccess > xNA;

    while( !xNA.is() && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::InitialModule:
                xNA = m_rDatabases.jarFile(rGuard, m_aInitialModule, m_aLanguage);
                m_eState = IteratorState::EndReached;
                break;

            case IteratorState::EndReached:
                OSL_FAIL( "JarFileIterator::nextJarFile(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return xNA;
}
OUString IndexFolderIterator::nextIndexFolder()
{
    OUString aIndexFolder;

    while( aIndexFolder.isEmpty() && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::InitialModule:
                aIndexFolder = m_rDatabases.getInstallPathAsURL()
                    + m_rDatabases.processLang(m_aLanguage) + "/"
                    + m_aInitialModule + ".idxl";

                m_eState = IteratorState::EndReached;
                break;

            case IteratorState::EndReached:
                OSL_FAIL( "IndexFolderIterator::nextIndexFolder(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return aIndexFolder;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
