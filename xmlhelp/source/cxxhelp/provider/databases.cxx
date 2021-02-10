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

#include <memory>
#include "db.hxx"
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <rtl/character.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/i18n/Collator.hpp>
#include <comphelper/propertysequence.hxx>
#include "inputstream.hxx"
#include <algorithm>
#include <cassert>
#include <string.h>
#include <string_view>

#include <helpcompiler/HelpIndexer.hxx>

// Extensible help
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/ExtensionRemovedException.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
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

#include <comphelper/storagehelper.hxx>

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
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::container;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;
using namespace com::sun::star::deployment;
using namespace com::sun::star::beans;

OUString Databases::expandURL( const OUString& aURL )
{
    osl::MutexGuard aGuard( m_aMutex );
    OUString aRetURL = expandURL( aURL, m_xContext );
    return aRetURL;
}

OUString Databases::expandURL( const OUString& aURL, const Reference< uno::XComponentContext >& xContext )
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

const char vendVersion[] = "%VENDORVERSION";
const char vendName[] = "%VENDORNAME";
const char prodVersion[] = "%PRODUCTVERSION";
const char vendShort[] = "%VENDORSHORT";
const char prodName[] = "%PRODUCTNAME";
const char newProdVersion[] = "$[officeversion]";
const char newProdName[] = "$[officename]";

Databases::Databases( bool showBasic,
                      const OUString& instPath,
                      const OUString& productName,
                      const OUString& productVersion,
                      const OUString& styleSheet,
                      Reference< uno::XComponentContext > const & xContext )
    : m_xContext( xContext ),
      m_bShowBasic(showBasic),
      m_aCSS(styleSheet.toAsciiLowerCase())
{
    m_xSMgr = m_xContext->getServiceManager();

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

OString Databases::getImageTheme() const
{
    uno::Reference< lang::XMultiServiceFactory > xConfigProvider =
        configuration::theDefaultProvider::get(m_xContext);

    // set root path
    uno::Sequence<uno::Any> lParams(comphelper::InitAnyPropertySequence(
    {
        {"nodepath", uno::Any(OUString("org.openoffice.Office.Common"))}
    }));

    // open it
    uno::Reference< uno::XInterface > xCFG( xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                lParams) );

    uno::Reference< container::XHierarchicalNameAccess > xAccess(xCFG, uno::UNO_QUERY_THROW);
    uno::Any aResult = xAccess->getByHierarchicalName("Misc/SymbolStyle");
    OUString aSymbolsStyleName;
    aResult >>= aSymbolsStyleName;

    if ( aSymbolsStyleName.isEmpty() || aSymbolsStyleName == "auto" )
    {
        aSymbolsStyleName = "colibre";
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

        if( oustring.indexOf( prodName,idx ) == idx )
            off = PRODUCTNAME;
        else if( oustring.indexOf( prodVersion,idx ) == idx )
            off = PRODUCTVERSION;
        else if( oustring.indexOf( vendName,idx ) == idx )
            off = VENDORNAME;
        else if( oustring.indexOf( vendVersion,idx ) == idx )
            off = VENDORVERSION;
        else if( oustring.indexOf( vendShort,idx ) == idx )
            off = VENDORSHORT;
        else if( oustring.indexOf( newProdName,idx ) == idx )
            off = NEWPRODUCTNAME;
        else if( oustring.indexOf( newProdVersion,idx ) == idx )
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
    osl::MutexGuard aGuard( m_aMutex );

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
            if (!fileName.endsWithIgnoreAsciiCase(".cfg", &fileName)) {
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
    osl::MutexGuard aGuard( m_aMutex );

    OUString key = processLang(Language) + "/" + Module;

    std::pair< ModInfoTable::iterator,bool > aPair =
        m_aModInfo.emplace(key,nullptr);

    ModInfoTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {
        osl::File cfgFile( getInstallPathAsURL() + key + ".cfg" );

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
            OUString order( "1" );

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
    osl::MutexGuard aGuard( m_aMutex );

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
            if (osl::FileBase::E_None == osl::DirectoryItem::get( getInstallPathAsURL() + rFB, aDirItem))
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

helpdatafileproxy::Hdf* Databases::getHelpDataFile( std::u16string_view Database,
                            const OUString& Language, bool helpText,
                            const OUString* pExtensionPath )
{
    if( Database.empty() || Language.isEmpty() )
        return nullptr;

    osl::MutexGuard aGuard( m_aMutex );

    OUString aFileExt( helpText ? OUString(".ht") : OUString(".db") );
    OUString dbFileName = OUString::Concat("/") + Database + aFileExt;
    OUString key;
    if( pExtensionPath == nullptr )
        key = processLang( Language ) + dbFileName;
    else
        key = *pExtensionPath + Language + dbFileName;      // make unique, don't change language

    std::pair< DatabasesTable::iterator,bool > aPair =
        m_aDatabases.emplace( key, nullptr);

    DatabasesTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {
        std::unique_ptr<helpdatafileproxy::Hdf> pHdf;

        OUString fileURL;
        if( pExtensionPath )
            fileURL = expandURL(*pExtensionPath) + Language + dbFileName;
        else
            fileURL = getInstallPathAsURL() + key;

        OUString fileNameHDFHelp( fileURL );
        //Extensions always use the new format
        if( pExtensionPath != nullptr )
            fileNameHDFHelp += "_";
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
Databases::getCollator( const OUString& Language )
{
    OUString key = Language;

    osl::MutexGuard aGuard( m_aMutex );

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
                countryStr = "DE";
            else if( langStr == "en" )
                countryStr = "US";
            else if( langStr == "es" )
                countryStr = "ES";
            else if( langStr == "it" )
                countryStr = "IT";
            else if( langStr == "fr" )
                countryStr = "FR";
            else if( langStr == "sv" )
                countryStr = "SE";
            else if( langStr == "ja" )
                countryStr = "JP";
            else if( langStr == "ko" )
                countryStr = "KR";

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
                                             OUString const & ky,
                                             OUString const & data )
    : key( ky )
{
    pDatabases->replaceName( key );
    init( pDatabases,pHdf,data );
}

void KeywordInfo::KeywordElement::init( Databases const *pDatabases,helpdatafileproxy::Hdf* pHdf,const OUString& ids )
{
    std::vector< OUString > id,anchor;
    int idx = -1,k;
    for (;;)
    {
        k = ++idx;
        idx = ids.indexOf( ';', k );
        if( idx == -1 )
            break;
        int h = ids.indexOf( '#', k );
        if( h < idx )
        {
            // found an anchor
            id.push_back( ids.copy( k, h-k ) );
            anchor.push_back( ids.copy( h+1, idx-h-1 ) );
        }
        else
        {
            id.push_back( ids.copy( k, idx-k ) );
            anchor.emplace_back( );
        }
    }

    listId.realloc( id.size() );
    listAnchor.realloc( id.size() );
    listTitle.realloc( id.size() );

    for( size_t i = 0; i < id.size(); ++i )
    {
        listId[i] = id[i];
        listAnchor[i] = anchor[i];

        helpdatafileproxy::HDFData aHDFData;
        const char* pData = nullptr;

        if( pHdf )
        {
            OString idi( id[i].getStr(),id[i].getLength(),RTL_TEXTENCODING_UTF8 );
            bool bSuccess = pHdf->getValueForKey( idi, aHDFData );
            if( bSuccess )
                pData = aHDFData.getData();
        }

        DbtToStringConverter converter( pData );

        OUString title = converter.getTitle();
        pDatabases->replaceName( title );
        listTitle[i] = title;
    }
}

KeywordInfo::KeywordInfo( const std::vector< KeywordElement >& aVec )
    : listKey( aVec.size() ),
      listId( aVec.size() ),
      listAnchor( aVec.size() ),
      listTitle( aVec.size() )
{
    for( size_t i = 0; i < aVec.size(); ++i )
    {
        listKey[i] = aVec[i].key;
        listId[i] = aVec[i].listId;
        listAnchor[i] = aVec[i].listAnchor;
        listTitle[i] = aVec[i].listTitle;
    }
}

bool Databases::checkModuleMatchForExtension
    ( std::u16string_view Database, const OUString& doclist )
{
    bool bBelongsToDatabase = true;

    // Analyse doclist string to find module assignments
    bool bFoundAtLeastOneModule = false;
    bool bModuleMatch = false;
    sal_Int32 nLen = doclist.getLength();
    sal_Int32 nLastFound = doclist.lastIndexOf( ';' );
    if( nLastFound == -1 )
        nLastFound = nLen;
    const sal_Unicode* pStr = doclist.getStr();
    sal_Int32 nFound = doclist.lastIndexOf( '_' );
    while( nFound != -1 )
    {
        // Simple optimization, stop if '_' is followed by "id"
        if( nLen - nFound > 2 )
        {
            if( pStr[ nFound + 1 ] == 'i' &&
                pStr[ nFound + 2 ] == 'd' )
                    break;
        }

        OUString aModule = doclist.copy( nFound + 1, nLastFound - nFound - 1 );
        std::vector< OUString >::iterator result = std::find( m_avModules.begin(), m_avModules.end(), aModule );
        if( result != m_avModules.end() )
        {
            bFoundAtLeastOneModule = true;
            if( Database == aModule )
            {
                bModuleMatch = true;
                break;
            }
        }

        nLastFound = nFound;
        if( nLastFound == 0 )
            break;
        nFound = doclist.lastIndexOf( '_', nLastFound - 1 );
    }

    if( bFoundAtLeastOneModule && !bModuleMatch )
        bBelongsToDatabase = false;

    return bBelongsToDatabase;
}

KeywordInfo* Databases::getKeyword( const OUString& Database,
                                    const OUString& Language )
{
    osl::MutexGuard aGuard( m_aMutex );

    OUString key = processLang(Language) + "/" + Database;

    std::pair< KeywordInfoTable::iterator,bool > aPair =
        m_aKeywordInfo.emplace( key,nullptr );

    KeywordInfoTable::iterator it = aPair.first;

    if( aPair.second && ! it->second )
    {
        std::vector<KeywordInfo::KeywordElement> aVector;

        KeyDataBaseFileIterator aDbFileIt( m_xContext, *this, Database, Language );
        OUString fileURL;
        bool bExtension = false;
        for (;;)
        {
            fileURL = aDbFileIt.nextDbFile( bExtension );
            if( fileURL.isEmpty() )
                break;
            OUString fileNameHDFHelp( fileURL );
            if( bExtension )
                fileNameHDFHelp += "_";
            if( m_xSFA->exists( fileNameHDFHelp ) )
            {
                helpdatafileproxy::Hdf aHdf( fileNameHDFHelp, m_xSFA );
                helpdatafileproxy::HDFData aKey;
                helpdatafileproxy::HDFData aValue;
                if( aHdf.startIteration() )
                {
                    helpdatafileproxy::Hdf* pHdf = getHelpDataFile( Database,Language );
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

                        bool bBelongsToDatabase = true;
                        if( bExtension )
                            bBelongsToDatabase = checkModuleMatchForExtension( Database, doclist );

                        if( !bBelongsToDatabase )
                            continue;

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
        Reference< XCollator > xCollator = getCollator( Language );
        KeywordElementComparator aComparator( xCollator );
        std::sort(aVector.begin(),aVector.end(),aComparator);

        it->second.reset(new KeywordInfo( aVector ));
    }

    return it->second.get();
}

Reference< XHierarchicalNameAccess > Databases::jarFile( const OUString& jar,
                                                         const OUString& Language )
{
    if( jar.isEmpty() || Language.isEmpty() )
    {
        return Reference< XHierarchicalNameAccess >( nullptr );
    }
    OUString key = processLang(Language) + "/" + jar;

    osl::MutexGuard aGuard( m_aMutex );

    ZipFileTable::iterator it =
        m_aZipFileTable.emplace( key,Reference< XHierarchicalNameAccess >(nullptr) ).first;

    if( ! it->second.is() )
    {
        try
        {
            OUString zipFile;
            // Extension jar file? Search for ?
            sal_Int32 nQuestionMark1 = jar.indexOf( '?' );
            sal_Int32 nQuestionMark2 = jar.lastIndexOf( '?' );
            if( nQuestionMark1 != -1 && nQuestionMark2 != -1 && nQuestionMark1 != nQuestionMark2 )
            {
                OUString aExtensionPath = jar.copy( nQuestionMark1 + 1, nQuestionMark2 - nQuestionMark1 - 1 );
                OUString aPureJar = jar.copy( nQuestionMark2 + 1 );

                zipFile = expandURL( aExtensionPath + "/" + aPureJar );
            }
            else
            {
                zipFile = getInstallPathAsURL() + key;
            }

            Sequence< Any > aArguments( 2 );

            rtl::Reference<XInputStream_impl> p(new XInputStream_impl( zipFile ));
            if( p->CtorSuccess() )
            {
                aArguments[ 0 ] <<= Reference< XInputStream >( p.get() );
            }
            else
            {
                p.clear();
                aArguments[ 0 ] <<= zipFile;
            }

            // let ZipPackage be used ( no manifest.xml is required )
            beans::NamedValue aArg;
            aArg.Name = "StorageFormat";
            aArg.Value <<= OUString(ZIP_STORAGE_FORMAT_STRING);
            aArguments[ 1 ] <<= aArg;

            Reference< XInterface > xIfc
                = m_xSMgr->createInstanceWithArgumentsAndContext(
                    "com.sun.star.packages.comp.ZipPackage",
                    aArguments, m_xContext );

            if ( xIfc.is() )
            {
                it->second.set( xIfc, UNO_QUERY );

                OSL_ENSURE( it->second.is(),
                            "ContentProvider::createPackage - "
                            "Got no hierarchical name access!" );

            }
        }
        catch ( RuntimeException & )
        {
        }
        catch ( Exception & )
        {
        }
    }

    return it->second;
}

Reference< XHierarchicalNameAccess > Databases::findJarFileForPath
    ( const OUString& jar, const OUString& Language,
      const OUString& path, OUString* o_pExtensionPath,
      OUString* o_pExtensionRegistryPath )
{
    Reference< XHierarchicalNameAccess > xNA;
    if( jar.isEmpty() || Language.isEmpty() )
    {
        return xNA;
    }

    JarFileIterator aJarFileIt( m_xContext, *this, jar, Language );
    Reference< XHierarchicalNameAccess > xTestNA;
    Reference< deployment::XPackage > xParentPackageBundle;
    for (;;)
    {
        xTestNA = aJarFileIt.nextJarFile( xParentPackageBundle, o_pExtensionPath, o_pExtensionRegistryPath );
        if( !xTestNA.is() )
            break;
        if( xTestNA.is() && xTestNA->hasByHierarchicalName( path ) )
        {
            bool bSuccess = true;
            if( xParentPackageBundle.is() )
            {
                OUString aIdentifierInPath;
                sal_Int32 nFindSlash = path.indexOf( '/' );
                if( nFindSlash != -1 )
                    aIdentifierInPath = path.copy( 0, nFindSlash );

                beans::Optional<OUString> aIdentifierOptional = xParentPackageBundle->getIdentifier();
                if( !aIdentifierInPath.isEmpty() && aIdentifierOptional.IsPresent )
                {
                    OUString aUnencodedIdentifier = aIdentifierOptional.Value;
                    OUString aIdentifier = rtl::Uri::encode( aUnencodedIdentifier,
                        rtl_UriCharClassPchar, rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8 );

                    if( aIdentifierInPath != aIdentifier )
                    {
                        // path does not start with extension identifier -> ignore
                        bSuccess = false;
                    }
                }
                else
                {
                    // No identifier -> ignore
                    bSuccess = false;
                }
            }

            if( bSuccess )
            {
                xNA = xTestNA;
                break;
            }
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
            uno::Reference< awt::XToolkit2 > xToolkit =
                   awt::Toolkit::create( ::comphelper::getProcessComponentContext() );
            uno::Reference< awt::XTopWindow > xTopWindow = xToolkit->getActiveTopWindow();
            if ( xTopWindow.is() )
            {
                uno::Reference< awt::XVclWindowPeer > xVclWindowPeer( xTopWindow, uno::UNO_QUERY );
                if ( xVclWindowPeer.is() )
                {
                    uno::Any aHCMode = xVclWindowPeer->getProperty( "HighContrastMode" );
                    if ( ( aHCMode >>= bHighContrastMode ) && bHighContrastMode )
                    {
                        aCSS = "highcontrastblack";
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
                                    aCSS = "highcontrast1";
                                if ( wcscmp( szBuffer, L"High Contrast #2" ) == 0 )
                                    aCSS = "highcontrast2";
                                if ( wcscmp( szBuffer, L"High Contrast White" ) == 0 )
                                    aCSS = "highcontrastwhite";
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
                aCSS = "default";
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
                               const OUString& Id,
                               OStringBuffer& buffer )
{
    DataBaseIterator aDbIt( m_xContext, *this, Module, Language, true );

    // #i84550 Cache information about failed ids
    OString id( Id.getStr(),Id.getLength(),RTL_TEXTENCODING_UTF8 );
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
    osl::MutexGuard aGuard( m_aMutex );

    osl::FileBase::getFileURLFromSystemPath( aInstDir,m_aInstallDirectory );
        //TODO: check returned error code

    if( !m_aInstallDirectory.endsWith( "/" ) )
        m_aInstallDirectory += "/";
}


ExtensionHelpExistenceMap ExtensionIteratorBase::aHelpExistenceMap;

ExtensionIteratorBase::ExtensionIteratorBase( Reference< XComponentContext > const & xContext,
    Databases& rDatabases, const OUString& aInitialModule, const OUString& aLanguage )
        : m_xContext( xContext )
        , m_rDatabases( rDatabases )
        , m_eState( IteratorState::InitialModule )
        , m_aInitialModule( aInitialModule )
        , m_aLanguage( aLanguage )
{
    assert( m_xContext.is() );
    init();
}

ExtensionIteratorBase::ExtensionIteratorBase( Databases& rDatabases,
    const OUString& aInitialModule, const OUString& aLanguage )
        : m_xContext( comphelper::getProcessComponentContext() )
        , m_rDatabases( rDatabases )
        , m_eState( IteratorState::InitialModule )
        , m_aInitialModule( aInitialModule )
        , m_aLanguage( aLanguage )
{
    init();
}

void ExtensionIteratorBase::init()
{
    m_xSFA = ucb::SimpleFileAccess::create(m_xContext);

    m_bUserPackagesLoaded = false;
    m_bSharedPackagesLoaded = false;
    m_bBundledPackagesLoaded = false;
    m_iUserPackage = 0;
    m_iSharedPackage = 0;
    m_iBundledPackage = 0;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetHelpPackageFromPackage
    ( const Reference< deployment::XPackage >& xPackage, Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    o_xParentPackageBundle.clear();

    Reference< deployment::XPackage > xHelpPackage;
    if( !xPackage.is() )
        return xHelpPackage;

    // #i84550 Cache information about help content in extension
    OUString aExtensionPath = xPackage->getURL();
    ExtensionHelpExistenceMap::iterator it = aHelpExistenceMap.find( aExtensionPath );
    bool bFound = ( it != aHelpExistenceMap.end() );
    bool bHasHelp = bFound && it->second;
    if( bFound && !bHasHelp )
        return xHelpPackage;

    // Check if parent package is registered
    beans::Optional< beans::Ambiguous<sal_Bool> > option( xPackage->isRegistered
        ( Reference<task::XAbortChannel>(), Reference<ucb::XCommandEnvironment>() ) );
    bool bRegistered = false;
    if( option.IsPresent )
    {
        beans::Ambiguous<sal_Bool> const & reg = option.Value;
        if( !reg.IsAmbiguous && reg.Value )
            bRegistered = true;
    }
    if( bRegistered )
    {
        OUString aHelpMediaType( "application/vnd.sun.star.help" );
        if( xPackage->isBundle() )
        {
            Sequence< Reference< deployment::XPackage > > aPkgSeq = xPackage->getBundle
                ( Reference<task::XAbortChannel>(), Reference<ucb::XCommandEnvironment>() );
            auto pSubPkg = std::find_if(aPkgSeq.begin(), aPkgSeq.end(),
                [&aHelpMediaType](const Reference< deployment::XPackage >& xSubPkg) {
                    const Reference< deployment::XPackageTypeInfo > xPackageTypeInfo = xSubPkg->getPackageType();
                    OUString aMediaType = xPackageTypeInfo->getMediaType();
                    return aMediaType == aHelpMediaType;
                });
            if (pSubPkg != aPkgSeq.end())
            {
                xHelpPackage = *pSubPkg;
                o_xParentPackageBundle = xPackage;
            }
        }
        else
        {
            const Reference< deployment::XPackageTypeInfo > xPackageTypeInfo = xPackage->getPackageType();
            OUString aMediaType = xPackageTypeInfo->getMediaType();
            if( aMediaType == aHelpMediaType )
                xHelpPackage = xPackage;
        }
    }

    if( !bFound )
        aHelpExistenceMap[ aExtensionPath ] = xHelpPackage.is();

    return xHelpPackage;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetNextUserHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bUserPackagesLoaded )
    {
        Reference< XExtensionManager > xExtensionManager = ExtensionManager::get(m_xContext);
        m_aUserPackagesSeq = xExtensionManager->getDeployedExtensions
            ( "user", Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );
        m_bUserPackagesLoaded = true;
    }

    if( m_iUserPackage == m_aUserPackagesSeq.getLength() )
    {
        m_eState = IteratorState::SharedExtensions;       // Later: SHARED_MODULE
    }
    else
    {
        const Reference< deployment::XPackage >* pUserPackages = m_aUserPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pUserPackages[ m_iUserPackage++ ];
        OSL_ENSURE( xPackage.is(), "ExtensionIteratorBase::implGetNextUserHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetNextSharedHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bSharedPackagesLoaded )
    {
        Reference< XExtensionManager > xExtensionManager = ExtensionManager::get(m_xContext);
        m_aSharedPackagesSeq = xExtensionManager->getDeployedExtensions
            ( "shared", Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );
        m_bSharedPackagesLoaded = true;
    }

    if( m_iSharedPackage == m_aSharedPackagesSeq.getLength() )
    {
        m_eState = IteratorState::BundledExtensions;
    }
    else
    {
        const Reference< deployment::XPackage >* pSharedPackages = m_aSharedPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pSharedPackages[ m_iSharedPackage++ ];
        OSL_ENSURE( xPackage.is(), "ExtensionIteratorBase::implGetNextSharedHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

Reference< deployment::XPackage > ExtensionIteratorBase::implGetNextBundledHelpPackage
    ( Reference< deployment::XPackage >& o_xParentPackageBundle )
{
    Reference< deployment::XPackage > xHelpPackage;

    if( !m_bBundledPackagesLoaded )
    {
        Reference< XExtensionManager > xExtensionManager = ExtensionManager::get(m_xContext);
        m_aBundledPackagesSeq = xExtensionManager->getDeployedExtensions
            ( "bundled", Reference< task::XAbortChannel >(), Reference< ucb::XCommandEnvironment >() );
        m_bBundledPackagesLoaded = true;
    }

    if( m_iBundledPackage == m_aBundledPackagesSeq.getLength() )
    {
        m_eState = IteratorState::EndReached;
    }
    else
    {
        const Reference< deployment::XPackage >* pBundledPackages =
            m_aBundledPackagesSeq.getConstArray();
        Reference< deployment::XPackage > xPackage = pBundledPackages[ m_iBundledPackage++ ];
        OSL_ENSURE( xPackage.is(), "ExtensionIteratorBase::implGetNextBundledHelpPackage(): Invalid package" );
        xHelpPackage = implGetHelpPackageFromPackage( xPackage, o_xParentPackageBundle );
    }

    return xHelpPackage;
}

OUString ExtensionIteratorBase::implGetFileFromPackage(
    std::u16string_view rFileExtension, const Reference< deployment::XPackage >& xPackage )
{
    // No extension -> search for pure language folder
    bool bLangFolderOnly = rFileExtension.empty();

    OUString aFile;
    OUString aLanguage = m_aLanguage;
    for( sal_Int32 iPass = 0 ; iPass < 2 ; ++iPass )
    {
        OUString aStr = xPackage->getRegistrationDataURL().Value + "/" + aLanguage;
        if( !bLangFolderOnly )
        {
            aStr += OUString::Concat("/help") + rFileExtension;
        }

        aFile = m_rDatabases.expandURL( aStr );
        if( iPass == 0 )
        {
            if( m_xSFA->exists( aFile ) )
                break;

            ::std::vector< OUString > av;
            implGetLanguageVectorFromPackage( av, xPackage );
            ::std::vector< OUString >::const_iterator pFound = LanguageTag::getFallback( av, m_aLanguage );
            if( pFound != av.end() )
                aLanguage = *pFound;
        }
    }
    return aFile;
}

static bool isLetter( sal_Unicode c )
{
    return rtl::isAsciiAlpha(c);
}

void ExtensionIteratorBase::implGetLanguageVectorFromPackage( ::std::vector< OUString > &rv,
    const css::uno::Reference< css::deployment::XPackage >& xPackage )
{
    rv.clear();
    OUString aExtensionPath = xPackage->getURL();
    const Sequence< OUString > aEntrySeq = m_xSFA->getFolderContents( aExtensionPath, true );

    for( const OUString& aEntry : aEntrySeq )
    {
        if( m_xSFA->isFolder( aEntry ) )
        {
            sal_Int32 nLastSlash = aEntry.lastIndexOf( '/' );
            if( nLastSlash != -1 )
            {
                OUString aPureEntry = aEntry.copy( nLastSlash + 1 );

                // Check language scheme
                int nLen = aPureEntry.getLength();
                const sal_Unicode* pc = aPureEntry.getStr();
                bool bStartCanBeLanguage = ( nLen >= 2 && isLetter( pc[0] ) && isLetter( pc[1] ) );
                bool bIsLanguage = bStartCanBeLanguage &&
                    ( nLen == 2 || (nLen == 5 && pc[2] == '-' && isLetter( pc[3] ) && isLetter( pc[4] )) );
                if( bIsLanguage )
                    rv.push_back( aPureEntry );
            }
        }
    }
}


helpdatafileproxy::Hdf* DataBaseIterator::nextHdf( OUString* o_pExtensionPath, OUString* o_pExtensionRegistryPath )
{
    helpdatafileproxy::Hdf* pRetHdf = nullptr;

    while( !pRetHdf && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::InitialModule:
                pRetHdf = m_rDatabases.getHelpDataFile( m_aInitialModule, m_aLanguage, m_bHelpText );
                m_eState = IteratorState::UserExtensions;     // Later: SHARED_MODULE
                break;

            // Later:
            //case SHARED_MODULE


            case IteratorState::UserExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextUserHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;
                pRetHdf = implGetHdfFromPackage( xHelpPackage, o_pExtensionPath, o_pExtensionRegistryPath );
                break;
            }

            case IteratorState::SharedExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextSharedHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                pRetHdf = implGetHdfFromPackage( xHelpPackage, o_pExtensionPath, o_pExtensionRegistryPath );
                break;
            }

               case IteratorState::BundledExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextBundledHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                pRetHdf = implGetHdfFromPackage( xHelpPackage, o_pExtensionPath, o_pExtensionRegistryPath );
                break;
            }

            case IteratorState::EndReached:
                OSL_FAIL( "DataBaseIterator::nextDb(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return pRetHdf;
}

helpdatafileproxy::Hdf* DataBaseIterator::implGetHdfFromPackage( const Reference< deployment::XPackage >& xPackage,
            OUString* o_pExtensionPath, OUString* o_pExtensionRegistryPath )
{

    beans::Optional< OUString> optRegData;
    try
    {
        optRegData = xPackage->getRegistrationDataURL();
    }
    catch ( deployment::ExtensionRemovedException&)
    {
        return nullptr;
    }

    helpdatafileproxy::Hdf* pRetHdf = nullptr;
    if (optRegData.IsPresent && !optRegData.Value.isEmpty())
    {
        OUString aRegDataUrl = optRegData.Value + "/";

        OUString aHelpFilesBaseName("help");

        OUString aUsedLanguage = m_aLanguage;
        pRetHdf = m_rDatabases.getHelpDataFile(
            aHelpFilesBaseName, aUsedLanguage, m_bHelpText, &aRegDataUrl);

        // Language fallback
        if( !pRetHdf )
        {
            ::std::vector< OUString > av;
            implGetLanguageVectorFromPackage( av, xPackage );
            ::std::vector< OUString >::const_iterator pFound = LanguageTag::getFallback( av, m_aLanguage );
            if( pFound != av.end() )
            {
                aUsedLanguage = *pFound;
                pRetHdf = m_rDatabases.getHelpDataFile(
                    aHelpFilesBaseName, aUsedLanguage, m_bHelpText, &aRegDataUrl);
            }
        }

        if( o_pExtensionPath )
            *o_pExtensionPath = aRegDataUrl + aUsedLanguage;

        if( o_pExtensionRegistryPath )
            *o_pExtensionRegistryPath = xPackage->getURL() + "/" + aUsedLanguage;
    }

    return pRetHdf;
}


//returns a file URL
OUString KeyDataBaseFileIterator::nextDbFile( bool& o_rbExtension )
{
    OUString aRetFile;

    while( aRetFile.isEmpty() && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::InitialModule:
                aRetFile = OUStringBuffer(m_rDatabases.getInstallPathAsURL()).
                    append(m_rDatabases.processLang(m_aLanguage)).append('/').
                    append(m_aInitialModule).append(".key").makeStringAndClear();

                o_rbExtension = false;

                m_eState = IteratorState::UserExtensions;     // Later: SHARED_MODULE
                break;

            // Later:
            //case SHARED_MODULE


            case IteratorState::UserExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextUserHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetDbFileFromPackage( xHelpPackage );
                o_rbExtension = true;
                break;
            }

            case IteratorState::SharedExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextSharedHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetDbFileFromPackage( xHelpPackage );
                o_rbExtension = true;
                break;
            }

            case IteratorState::BundledExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextBundledHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aRetFile = implGetDbFileFromPackage( xHelpPackage );
                o_rbExtension = true;
                break;
            }

            case IteratorState::EndReached:
                OSL_FAIL( "DataBaseIterator::nextDbFile(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return aRetFile;
}

//Returns a file URL, that does not contain macros
OUString KeyDataBaseFileIterator::implGetDbFileFromPackage
    ( const Reference< deployment::XPackage >& xPackage )
{
    OUString aExpandedURL =
        implGetFileFromPackage( u".key", xPackage );

    return aExpandedURL;
}


Reference< XHierarchicalNameAccess > JarFileIterator::nextJarFile
    ( Reference< deployment::XPackage >& o_xParentPackageBundle,
        OUString* o_pExtensionPath, OUString* o_pExtensionRegistryPath )
{
    Reference< XHierarchicalNameAccess > xNA;

    while( !xNA.is() && m_eState != IteratorState::EndReached )
    {
        switch( m_eState )
        {
            case IteratorState::InitialModule:
                xNA = m_rDatabases.jarFile( m_aInitialModule, m_aLanguage );
                m_eState = IteratorState::UserExtensions;     // Later: SHARED_MODULE
                break;

            // Later:
            //case SHARED_MODULE


            case IteratorState::UserExtensions:
            {
                Reference< deployment::XPackage > xHelpPackage = implGetNextUserHelpPackage( o_xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                xNA = implGetJarFromPackage( xHelpPackage, o_pExtensionPath, o_pExtensionRegistryPath );
                break;
            }

            case IteratorState::SharedExtensions:
            {
                Reference< deployment::XPackage > xHelpPackage = implGetNextSharedHelpPackage( o_xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                xNA = implGetJarFromPackage( xHelpPackage, o_pExtensionPath, o_pExtensionRegistryPath );
                break;
            }

            case IteratorState::BundledExtensions:
            {
                Reference< deployment::XPackage > xHelpPackage = implGetNextBundledHelpPackage( o_xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                xNA = implGetJarFromPackage( xHelpPackage, o_pExtensionPath, o_pExtensionRegistryPath );
                break;
            }

            case IteratorState::EndReached:
                OSL_FAIL( "JarFileIterator::nextJarFile(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return xNA;
}

Reference< XHierarchicalNameAccess > JarFileIterator::implGetJarFromPackage
( const Reference< deployment::XPackage >& xPackage, OUString* o_pExtensionPath, OUString* o_pExtensionRegistryPath )
{
    Reference< XHierarchicalNameAccess > xNA;

    OUString zipFile =
        implGetFileFromPackage( u".jar", xPackage );

    try
    {
        Sequence< Any > aArguments( 2 );
        aArguments[ 0 ] <<= zipFile;

        // let ZipPackage be used ( no manifest.xml is required )
        beans::NamedValue aArg;
        aArg.Name = "StorageFormat";
        aArg.Value <<= OUString(ZIP_STORAGE_FORMAT_STRING);
        aArguments[ 1 ] <<= aArg;

        Reference< XMultiComponentFactory >xSMgr = m_xContext->getServiceManager();
        Reference< XInterface > xIfc
            = xSMgr->createInstanceWithArgumentsAndContext(
                "com.sun.star.packages.comp.ZipPackage",
                aArguments, m_xContext );

        if ( xIfc.is() )
        {
            xNA.set( xIfc, UNO_QUERY );

            OSL_ENSURE( xNA.is(),
                "JarFileIterator::implGetJarFromPackage() - "
                "Got no hierarchical name access!" );
        }
    }
    catch ( RuntimeException & )
    {}
    catch ( Exception & )
    {}

    if( xNA.is() && o_pExtensionPath != nullptr )
    {
        // Extract path including language from file name
        sal_Int32 nLastSlash = zipFile.lastIndexOf( '/' );
        if( nLastSlash != -1 )
            *o_pExtensionPath = zipFile.copy( 0, nLastSlash );

        if( o_pExtensionRegistryPath != nullptr )
        {
            OUString& rPath = *o_pExtensionPath;
            sal_Int32 nLastSlashInPath = rPath.lastIndexOf( '/', rPath.getLength() - 1 );

            *o_pExtensionRegistryPath = xPackage->getURL();
            *o_pExtensionRegistryPath += rPath.subView( nLastSlashInPath);
        }
    }

    return xNA;
}


OUString IndexFolderIterator::nextIndexFolder( bool& o_rbExtension, bool& o_rbTemporary )
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

                o_rbTemporary = false;
                o_rbExtension = false;

                m_eState = IteratorState::UserExtensions;     // Later: SHARED_MODULE
                break;

            // Later:
            //case SHARED_MODULE


            case IteratorState::UserExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextUserHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aIndexFolder = implGetIndexFolderFromPackage( o_rbTemporary, xHelpPackage );
                o_rbExtension = true;
                break;
            }

            case IteratorState::SharedExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextSharedHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aIndexFolder = implGetIndexFolderFromPackage( o_rbTemporary, xHelpPackage );
                o_rbExtension = true;
                break;
            }

            case IteratorState::BundledExtensions:
            {
                Reference< deployment::XPackage > xParentPackageBundle;
                Reference< deployment::XPackage > xHelpPackage = implGetNextBundledHelpPackage( xParentPackageBundle );
                if( !xHelpPackage.is() )
                    break;

                aIndexFolder = implGetIndexFolderFromPackage( o_rbTemporary, xHelpPackage );
                o_rbExtension = true;
                break;
            }

            case IteratorState::EndReached:
                OSL_FAIL( "IndexFolderIterator::nextIndexFolder(): Invalid case IteratorState::EndReached" );
                break;
        }
    }

    return aIndexFolder;
}

OUString IndexFolderIterator::implGetIndexFolderFromPackage( bool& o_rbTemporary, const Reference< deployment::XPackage >& xPackage )
{
    OUString aIndexFolder =
        implGetFileFromPackage( u".idxl", xPackage );

    o_rbTemporary = false;
    if( !m_xSFA->isFolder( aIndexFolder ) )
    {
        // i98680: Missing index? Try to generate now
        OUString aLangURL = implGetFileFromPackage( std::u16string_view(), xPackage );
        if( m_xSFA->isFolder( aLangURL ) )
        {
            // Test write access (shared extension may be read only)
            bool bIsWriteAccess = false;
            try
            {
                OUString aCreateTestFolder = aLangURL + "CreateTestFolder";
                m_xSFA->createFolder( aCreateTestFolder );
                if( m_xSFA->isFolder( aCreateTestFolder  ) )
                    bIsWriteAccess = true;

                m_xSFA->kill( aCreateTestFolder );
            }
            catch (const Exception &)
            {
            }

            // TEST
            //bIsWriteAccess = false;

            try
            {
                OUString aLang;
                sal_Int32 nLastSlash = aLangURL.lastIndexOf( '/' );
                if( nLastSlash != -1 )
                    aLang = aLangURL.copy( nLastSlash + 1 );
                else
                    aLang = "en";

                OUString aZipDir = aLangURL;
                if( !bIsWriteAccess )
                {
                    OUString aTempFileURL;
                    ::osl::FileBase::RC eErr = ::osl::File::createTempFile( nullptr, nullptr, &aTempFileURL );
                    if( eErr == ::osl::FileBase::E_None )
                    {
                        try
                        {
                            m_xSFA->kill( aTempFileURL );
                        }
                        catch (const Exception &)
                        {
                        }
                        m_xSFA->createFolder( aTempFileURL );

                        aZipDir = aTempFileURL;
                        o_rbTemporary = true;
                    }
                }

                HelpIndexer aIndexer(aLang, "help", aLangURL, aZipDir);
                aIndexer.indexDocuments();

                if( bIsWriteAccess )
                    aIndexFolder = implGetFileFromPackage( u".idxl", xPackage );
                else
                    aIndexFolder = aZipDir + "/help.idxl";
            }
            catch (const Exception &)
            {
            }
        }
    }

    return aIndexFolder;
}

void IndexFolderIterator::deleteTempIndexFolder( const OUString& aIndexFolder )
{
    sal_Int32 nLastSlash = aIndexFolder.lastIndexOf( '/' );
    if( nLastSlash != -1 )
    {
        OUString aTmpFolder = aIndexFolder.copy( 0, nLastSlash );
        try
        {
            m_xSFA->kill( aTmpFolder );
        }
        catch (const Exception &)
        {
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
