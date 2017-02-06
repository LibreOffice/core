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

//TODO: Large parts of this file were copied from dp_component.cxx; those parts
// should be consolidated.

#include <config_features.h>

#include "dp_configuration.hrc"
#include "dp_backend.h"
#if HAVE_FEATURE_EXTENSIONS
#include "dp_persmap.h"
#endif
#include "dp_services.hxx"
#include "dp_ucb.h"
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <osl/file.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/servicedecl.hxx>
#include <xmlscript/xml_helper.hxx>
#include <svl/inettype.hxx>
#include <com/sun/star/configuration/Update.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <list>
#include <memory>
#include <utility>

#include "dp_configurationbackenddb.hxx"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace dp_registry {
namespace backend {
namespace configuration {
namespace {

typedef ::std::list<OUString> t_stringlist;


class BackendImpl : public ::dp_registry::backend::PackageRegistryBackend
{
    class PackageImpl : public ::dp_registry::backend::Package
    {
        BackendImpl * getMyBackend() const ;

        const bool m_isSchema;

        // Package
        virtual beans::Optional< beans::Ambiguous<sal_Bool> > isRegistered_(
            ::osl::ResettableMutexGuard & guard,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv ) override;

    public:
        inline PackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool isSchema, bool bRemoved, OUString const & identifier)
            : Package( myBackend, url, name, name /* display-name */,
                       xPackageType, bRemoved, identifier),
              m_isSchema( isSchema )
            {}
    };
    friend class PackageImpl;

    t_stringlist m_xcs_files;
    t_stringlist m_xcu_files;
    t_stringlist & getFiles( bool xcs ) {
        return xcs ? m_xcs_files : m_xcu_files;
    }

    bool m_configmgrini_inited;
    bool m_configmgrini_modified;
    std::unique_ptr<ConfigurationBackendDb> m_backendDb;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType, bool bRemoved,
        OUString const & identifier,
        Reference<XCommandEnvironment> const & xCmdEnv ) override;

#if HAVE_FEATURE_EXTENSIONS
    // for backwards compatibility - nil if no (compatible) back-compat db present
    ::std::unique_ptr<PersistentMap> m_registeredPackages;
#endif

    virtual void SAL_CALL disposing() override;

    const Reference<deployment::XPackageTypeInfo> m_xConfDataTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xConfSchemaTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

    void configmgrini_verify_init(
        Reference<XCommandEnvironment> const & xCmdEnv );
    void configmgrini_flush( Reference<XCommandEnvironment> const & xCmdEnv );

    /* The parameter isURL is false in the case of adding the conf:ini-entry
       value from the backend db. This entry already contains the path as it
       is used in the configmgr.ini.
     */
    void addToConfigmgrIni( bool isSchema, bool isURL, OUString const & url,
                     Reference<XCommandEnvironment> const & xCmdEnv );
#if HAVE_FEATURE_EXTENSIONS
    bool removeFromConfigmgrIni( bool isSchema, OUString const & url,
                          Reference<XCommandEnvironment> const & xCmdEnv );
#endif
    void addDataToDb(OUString const & url, ConfigurationBackendDb::Data const & data);
    ::boost::optional<ConfigurationBackendDb::Data> readDataFromDb(OUString const & url);
    void revokeEntryFromDb(OUString const & url);
    bool hasActiveEntry(OUString const & url);
    bool activateEntry(OUString const & url);

public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() override;
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType) override;

    using PackageRegistryBackend::disposing;
};


void BackendImpl::disposing()
{
    try {
        configmgrini_flush( Reference<XCommandEnvironment>() );

        PackageRegistryBackend::disposing();
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            "caught unexpected exception while disposing...",
            static_cast<OWeakObject *>(this), exc );
    }
}


BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_configmgrini_inited( false ),
      m_configmgrini_modified( false ),
      m_xConfDataTypeInfo( new Package::TypeInfo(
                               "application/vnd.sun.star.configuration-data",
                               "*.xcu",
                               getResourceString(RID_STR_CONF_DATA)
                               ) ),
      m_xConfSchemaTypeInfo( new Package::TypeInfo(
                                 "application/vnd.sun.star.configuration-schema",
                                 "*.xcs",
                                 getResourceString(RID_STR_CONF_SCHEMA)
                                 ) ),
      m_typeInfos( 2 )
{
    m_typeInfos[ 0 ] = m_xConfDataTypeInfo;
    m_typeInfos[ 1 ] = m_xConfSchemaTypeInfo;

    const Reference<XCommandEnvironment> xCmdEnv;

    if (transientMode())
    {
        // TODO
    }
    else
    {
        OUString dbFile = makeURL(getCachePath(), "backenddb.xml");
        m_backendDb.reset(
            new ConfigurationBackendDb(getComponentContext(), dbFile));
        //clean up data folders which are no longer used.
        //This must not be done in the same process where the help files
        //are still registers. Only after revoking and restarting OOo the folders
        //can be removed. This works now, because the extension manager is a singleton
        //and the backends are only create once per process.
        ::std::list<OUString> folders = m_backendDb->getAllDataUrls();
        deleteUnusedFolders(OUString(), folders);

        configmgrini_verify_init( xCmdEnv );

#if HAVE_FEATURE_EXTENSIONS
        ::std::unique_ptr<PersistentMap> pMap;
        OUString aCompatURL( makeURL( getCachePath(), "registered_packages.pmap" ) );

        // Don't create it if it doesn't exist already
        if ( ::utl::UCBContentHelper::Exists( expandUnoRcUrl( aCompatURL ) ) )
        {
            try
            {
                pMap.reset( new PersistentMap( aCompatURL ) );
            }
            catch (const Exception &e)
            {
                OUStringBuffer aStr( "Exception loading legacy package database: '" );
                aStr.append( e.Message );
                aStr.append( "' - ignoring file, please remove it.\n" );
                dp_misc::writeConsole( aStr.getStr() );
            }
        }
        m_registeredPackages = std::move(pMap);
#endif
     }
}

void BackendImpl::addDataToDb(
    OUString const & url, ConfigurationBackendDb::Data const & data)
{
    if (m_backendDb.get())
        m_backendDb->addEntry(url, data);
}

::boost::optional<ConfigurationBackendDb::Data> BackendImpl::readDataFromDb(
    OUString const & url)
{
    ::boost::optional<ConfigurationBackendDb::Data> data;
    if (m_backendDb.get())
        data = m_backendDb->getEntry(url);
    return data;
}

void BackendImpl::revokeEntryFromDb(OUString const & url)
{
    if (m_backendDb.get())
        m_backendDb->revokeEntry(url);
}

bool BackendImpl::hasActiveEntry(OUString const & url)
{
    if (m_backendDb.get())
        return m_backendDb->hasActiveEntry(url);
    return false;
}

bool BackendImpl::activateEntry(OUString const & url)
{
    if (m_backendDb.get())
        return m_backendDb->activateEntry(url);
    return false;
}


// XPackageRegistry

Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes()
{
    return m_typeInfos;
}
void BackendImpl::packageRemoved(OUString const & url, OUString const & /*mediaType*/)
{
    if (m_backendDb.get())
        m_backendDb->removeEntry(url);
}

// PackageRegistryBackend

Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    bool bRemoved, OUString const & identifier,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType( mediaType_ );
    if (mediaType.isEmpty())
    {
        // detect media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ))
        {
            const OUString title( StrTitle::getTitle( ucbContent ) );
            if (title.endsWithIgnoreAsciiCase( ".xcu" )) {
                mediaType = "application/vnd.sun.star.configuration-data";
            }
            if (title.endsWithIgnoreAsciiCase( ".xcs" )) {
                mediaType = "application/vnd.sun.star.configuration-schema";
            }
        }
        if (mediaType.isEmpty())
            throw lang::IllegalArgumentException(
                StrCannotDetectMediaType::get() + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    OUString type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.equalsIgnoreAsciiCase("application"))
        {
            OUString name;
            if (!bRemoved)
            {
                ::ucbhelper::Content ucbContent( url, xCmdEnv, m_xComponentContext );
                name = StrTitle::getTitle( ucbContent );
            }

            ::ucbhelper::Content ucbContent( url, xCmdEnv, m_xComponentContext );
            if (subType.equalsIgnoreAsciiCase( "vnd.sun.star.configuration-data"))
            {
                return new PackageImpl(
                    this, url, name, m_xConfDataTypeInfo, false /* data file */,
                    bRemoved, identifier);
            }
            else if (subType.equalsIgnoreAsciiCase( "vnd.sun.star.configuration-schema")) {
                return new PackageImpl(
                    this, url, name, m_xConfSchemaTypeInfo, true /* schema file */,
                    bRemoved, identifier);
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}


void BackendImpl::configmgrini_verify_init(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    const ::osl::MutexGuard guard( getMutex() );
    if (! m_configmgrini_inited)
    {
        // common rc:
        ::ucbhelper::Content ucb_content;
        if (create_ucb_content(
                &ucb_content,
                makeURL( getCachePath(), "configmgr.ini" ),
                xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, "SCHEMA=", ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                sal_Int32 index = RTL_CONSTASCII_LENGTH("SCHEMA=");
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (!token.isEmpty()) {
                        //The  file may not exist anymore if a shared or bundled
                        //extension was removed, but it can still be in the configmgrini.
                        //After running XExtensionManager::synchronize, the configmgrini is
                        //cleaned up
                        m_xcs_files.push_back( token );
                    }
                }
                while (index >= 0);
            }
            if (readLine( &line, "DATA=", ucb_content,
                          RTL_TEXTENCODING_UTF8 )) {
                sal_Int32 index = RTL_CONSTASCII_LENGTH("DATA=");
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (!token.isEmpty())
                    {
                        if (token[ 0 ] == '?')
                            token = token.copy( 1 );
                        //The  file may not exist anymore if a shared or bundled
                        //extension was removed, but it can still be in the configmgrini.
                        //After running XExtensionManager::synchronize, the configmgrini is
                        //cleaned up
                        m_xcu_files.push_back( token );
                    }
                }
                while (index >= 0);
            }
        }
        m_configmgrini_modified = false;
        m_configmgrini_inited = true;
    }
}


void BackendImpl::configmgrini_flush(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    if (!m_configmgrini_inited || !m_configmgrini_modified)
        return;

    OStringBuffer buf;
    if (! m_xcs_files.empty())
    {
        t_stringlist::const_iterator iPos( m_xcs_files.begin() );
        t_stringlist::const_iterator const iEnd( m_xcs_files.end() );
        buf.append( "SCHEMA=" );
        while (iPos != iEnd) {
            // encoded ASCII file-urls:
            const OString item(
                OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append(LF);
    }
    if (! m_xcu_files.empty())
    {
        t_stringlist::const_iterator iPos( m_xcu_files.begin() );
        t_stringlist::const_iterator const iEnd( m_xcu_files.end() );
        buf.append( "DATA=" );
        while (iPos != iEnd) {
            // encoded ASCII file-urls:
            const OString item(
                OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( item );
            ++iPos;
            if (iPos != iEnd)
                buf.append( ' ' );
        }
        buf.append(LF);
    }

    // write configmgr.ini:
    const Reference<io::XInputStream> xData(
        ::xmlscript::createInputStream(
                reinterpret_cast<sal_Int8 const *>(buf.getStr()),
                buf.getLength() ) );
    ::ucbhelper::Content ucb_content(
        makeURL( getCachePath(), "configmgr.ini" ), xCmdEnv, m_xComponentContext );
    ucb_content.writeStream( xData, true /* replace existing */ );

    m_configmgrini_modified = false;
}


void BackendImpl::addToConfigmgrIni( bool isSchema, bool isURL, OUString const & url_,
                              Reference<XCommandEnvironment> const & xCmdEnv )
{
    const OUString rcterm( isURL ? dp_misc::makeRcTerm(url_) : url_ );
    const ::osl::MutexGuard guard( getMutex() );
    configmgrini_verify_init( xCmdEnv );
    t_stringlist & rSet = getFiles(isSchema);
    if (::std::find( rSet.begin(), rSet.end(), rcterm ) == rSet.end()) {
        rSet.push_front( rcterm ); // prepend to list, thus overriding
        // write immediately:
        m_configmgrini_modified = true;
        configmgrini_flush( xCmdEnv );
    }
}

#if HAVE_FEATURE_EXTENSIONS
bool BackendImpl::removeFromConfigmgrIni(
    bool isSchema, OUString const & url_,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    const OUString rcterm( dp_misc::makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    configmgrini_verify_init( xCmdEnv );
    t_stringlist & rSet = getFiles(isSchema);
    t_stringlist::iterator i(std::find(rSet.begin(), rSet.end(), rcterm));
    if (i == rSet.end() && !isSchema)
    {
        //in case the xcu contained %origin% then the configmr.ini contains the
        //url to the file in the user installation (e.g. $BUNDLED_EXTENSIONS_USER)
        //However, m_url (getURL()) contains the URL for the file in the actual
        //extension installatation.
        ::boost::optional<ConfigurationBackendDb::Data> data = readDataFromDb(url_);
        if (data)
            i = std::find(rSet.begin(), rSet.end(), data->iniEntry);
    }
    if (i == rSet.end()) {
        return false;
    }
    rSet.erase(i);
    // write immediately:
    m_configmgrini_modified = true;
    configmgrini_flush( xCmdEnv );
    return true;
}
#endif

// Package


BackendImpl * BackendImpl::PackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (nullptr == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            "Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<PackageImpl *>(this)));
    }
    return pBackend;
}

beans::Optional< beans::Ambiguous<sal_Bool> >
BackendImpl::PackageImpl::isRegistered_(
    ::osl::ResettableMutexGuard &,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    BackendImpl * that = getMyBackend();

    bool bReg = false;
    if (that->hasActiveEntry(getURL()))
        bReg = true;

#if HAVE_FEATURE_EXTENSIONS
    const OUString url(getURL());
    if (!bReg && that->m_registeredPackages.get())
    {
        // fallback for user extension registered in berkeley DB
        bReg = that->m_registeredPackages->has(
            OUStringToOString( url, RTL_TEXTENCODING_UTF8 ));
    }
#endif
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true, beans::Ambiguous<sal_Bool>( bReg, false ) );
}


OUString encodeForXml( OUString const & text )
{
    // encode conforming xml:
    sal_Int32 len = text.getLength();
    OUStringBuffer buf;
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        sal_Unicode c = text[ pos ];
        switch (c) {
        case '<':
            buf.append( "&lt;" );
            break;
        case '>':
            buf.append( "&gt;" );
            break;
        case '&':
            buf.append( "&amp;" );
            break;
        case '\'':
            buf.append( "&apos;" );
            break;
        case '\"':
            buf.append( "&quot;" );
            break;
        default:
            buf.append( c );
            break;
        }
    }
    return buf.makeStringAndClear();
}


OUString replaceOrigin(
    OUString const & url, OUString const & destFolder, Reference< XCommandEnvironment > const & xCmdEnv, Reference< XComponentContext > const & xContext, bool & out_replaced)
{
    // looking for %origin%:
    ::ucbhelper::Content ucb_content( url, xCmdEnv, xContext );
    std::vector<sal_Int8> bytes( readFile( ucb_content ) );
    std::vector<sal_Int8> filtered( bytes.size() * 2 );
    bool use_filtered = false;
    OString origin;
    sal_Char const * pBytes = reinterpret_cast<sal_Char const *>(
        bytes.data());
    std::size_t nBytes = bytes.size();
    size_t write_pos = 0;
    while (nBytes > 0)
    {
        sal_Int32 index = rtl_str_indexOfChar_WithLength( pBytes, nBytes, '%' );
        if (index < 0) {
            if (! use_filtered) // opt
                break;
            index = nBytes;
        }

        if ((write_pos + index) > filtered.size())
            filtered.resize( (filtered.size() + index) * 2 );
        memcpy( filtered.data() + write_pos, pBytes, index );
        write_pos += index;
        pBytes += index;
        nBytes -= index;
        if (nBytes == 0)
            break;

        // consume %:
        ++pBytes;
        --nBytes;
        sal_Char const * pAdd = "%";
        sal_Int32 nAdd = 1;
        if (nBytes > 1 && pBytes[ 0 ] == '%')
        {
            // %% => %
            ++pBytes;
            --nBytes;
            use_filtered = true;
        }
        else if (rtl_str_shortenedCompare_WithLength(
                     pBytes, nBytes,
                     "origin%",
                     RTL_CONSTASCII_LENGTH("origin%"),
                     RTL_CONSTASCII_LENGTH("origin%")) == 0)
        {
            if (origin.isEmpty()) {
                // encode only once
                origin = OUStringToOString(
                    encodeForXml( url.copy( 0, url.lastIndexOf( '/' ) ) ),
                    // xxx todo: encode always for UTF-8? => lookup doc-header?
                    RTL_TEXTENCODING_UTF8 );
            }
            pAdd = origin.getStr();
            nAdd = origin.getLength();
            pBytes += RTL_CONSTASCII_LENGTH("origin%");
            nBytes -= RTL_CONSTASCII_LENGTH("origin%");
            use_filtered = true;
        }
        if ((write_pos + nAdd) > filtered.size())
            filtered.resize( (filtered.size() + nAdd) * 2 );
        memcpy( filtered.data() + write_pos, pAdd, nAdd );
        write_pos += nAdd;
    }
    if (!use_filtered)
        return url;
    if (write_pos < filtered.size())
        filtered.resize( write_pos );
    OUString newUrl(url);
    if (!destFolder.isEmpty())
    {
        //get the file name of the xcu and add it to the url of the temporary folder
        sal_Int32 i = url.lastIndexOf('/');
        newUrl = destFolder + url.copy(i);
    }

    ucbhelper::Content(newUrl, xCmdEnv, xContext).writeStream(
        xmlscript::createInputStream(filtered), true);
    out_replaced = true;
    return newUrl;
}


void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    bool startup,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    OUString url( getURL() );

    if (doRegisterPackage)
    {
        if (getMyBackend()->activateEntry(getURL()))
        {
            ::boost::optional<ConfigurationBackendDb::Data> data = that->readDataFromDb(url);
            OSL_ASSERT(data);
            that->addToConfigmgrIni( m_isSchema, false, data->iniEntry, xCmdEnv );
        }
        else
        {
            ConfigurationBackendDb::Data data;
            if (!m_isSchema)
            {
                const OUString sModFolder = that->createFolder(OUString(), xCmdEnv);
                bool out_replaced = false;
                url = replaceOrigin(url, sModFolder, xCmdEnv, that->getComponentContext(), out_replaced);
                if (out_replaced)
                    data.dataUrl = sModFolder;
                else
                    deleteTempFolder(sModFolder);
            }
            //No need for live-deployment for bundled extension, because OOo
            //restarts after installation
            if (that->m_eContext != Context::Bundled
                && !startup)
            {
                if (m_isSchema)
                {
                    css::configuration::Update::get(
                        that->m_xComponentContext)->insertExtensionXcsFile(
                            that->m_eContext == Context::Shared, expandUnoRcUrl(url));
                }
                else
                {
                    css::configuration::Update::get(
                        that->m_xComponentContext)->insertExtensionXcuFile(
                            that->m_eContext == Context::Shared, expandUnoRcUrl(url));
                }
            }
            that->addToConfigmgrIni( m_isSchema, true, url, xCmdEnv );
            data.iniEntry = dp_misc::makeRcTerm(url);
            that->addDataToDb(getURL(), data);
        }
    }
    else // revoke
    {
#if HAVE_FEATURE_EXTENSIONS
        if (!that->removeFromConfigmgrIni(m_isSchema, url, xCmdEnv) &&
            that->m_registeredPackages.get()) {
            // Obsolete package database handling - should be removed for LibreOffice 4.0
            t_string2string_map entries(
                that->m_registeredPackages->getEntries());
            for (t_string2string_map::iterator i(entries.begin());
                 i != entries.end(); ++i)
            {
                //If the xcu file was installed before the configmgr was changed
                //to use the configmgr.ini, one needed to rebuild to whole directory
                //structure containing the xcu, xcs files from all extensions. Now,
                //we just add all other xcu/xcs files to the configmgr.ini instead of
                //rebuilding the directory structure.
                OUString url2(
                    OStringToOUString(i->first, RTL_TEXTENCODING_UTF8));
                if (url2 != url) {
                   bool schema = i->second.equalsIgnoreAsciiCase(
                       "vnd.sun.star.configuration-schema");
                   OUString url_replaced(url2);
                   ConfigurationBackendDb::Data data;
                   if (!schema)
                   {
                       const OUString sModFolder = that->createFolder(OUString(), xCmdEnv);
                       bool out_replaced = false;
                       url_replaced = replaceOrigin(
                           url2, sModFolder, xCmdEnv, that->getComponentContext(), out_replaced);
                       if (out_replaced)
                           data.dataUrl = sModFolder;
                       else
                           deleteTempFolder(sModFolder);
                   }
                   that->addToConfigmgrIni(schema, true, url_replaced, xCmdEnv);
                   data.iniEntry = dp_misc::makeRcTerm(url_replaced);
                   that->addDataToDb(url2, data);
                }
                that->m_registeredPackages->erase(i->first);
            }
            try
            {
                ::ucbhelper::Content(
                    makeURL( that->getCachePath(), "registry" ),
                    xCmdEnv, that->getComponentContext() ).executeCommand(
                        "delete", Any( true /* delete physically */ ) );
            }
            catch(const Exception&)
            {
                OSL_ASSERT(false);
            }
        }
#endif
        ::boost::optional<ConfigurationBackendDb::Data> data = that->readDataFromDb(url);
        //If an xcu file was life deployed then always a data entry is written.
        //If the xcu file was already in the configmr.ini then there is also
        //a data entry
        if (!m_isSchema && data)
        {
            css::configuration::Update::get(
                that->m_xComponentContext)->removeExtensionXcuFile(expandUnoRcTerm(data->iniEntry));
        }
        that->revokeEntryFromDb(url);
    }
}

} // anon namespace

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.configuration.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} // namespace configuration
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
