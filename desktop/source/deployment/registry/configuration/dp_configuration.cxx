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


//TODO: Large parts of this file were copied from dp_component.cxx; those parts
// should be consolidated.

#include "dp_configuration.hrc"
#include "dp_backend.h"
#if !defined(ANDROID) && !defined(IOS)
#include "dp_persmap.h"
#endif
#include "dp_ucb.h"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "osl/file.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "unotools/ucbhelper.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/servicedecl.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svl/inettype.hxx"
#include "com/sun/star/configuration/Update.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/io/XActiveDataSink.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/util/XRefreshable.hpp"
#include <list>
#include <memory>

#include "dp_configurationbackenddb.hxx"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace configuration {
namespace {

typedef ::std::list<OUString> t_stringlist;

//==============================================================================
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
            Reference<XCommandEnvironment> const & xCmdEnv );
        virtual void processPackage_(
            ::osl::ResettableMutexGuard & guard,
            bool registerPackage,
            bool startup,
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );

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
    std::auto_ptr<ConfigurationBackendDb> m_backendDb;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType, sal_Bool bRemoved,
        OUString const & identifier,
        Reference<XCommandEnvironment> const & xCmdEnv );
#if !defined(ANDROID) && !defined(IOS)
    // for backwards compatibility - nil if no (compatible) back-compat db present
    ::std::auto_ptr<PersistentMap> m_registeredPackages;
#endif
    virtual void SAL_CALL disposing();

    const Reference<deployment::XPackageTypeInfo> m_xConfDataTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xConfSchemaTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

    void configmgrini_verify_init(
        Reference<XCommandEnvironment> const & xCmdEnv );
    void configmgrini_flush( Reference<XCommandEnvironment> const & xCmdEnv );

    /* The paramter isURL is false in the case of adding the conf:ini-entry
       value from the backend db. This entry already contains the path as it
       is used in the configmgr.ini.
     */
    bool addToConfigmgrIni( bool isSchema, bool isURL, OUString const & url,
                     Reference<XCommandEnvironment> const & xCmdEnv );
    bool removeFromConfigmgrIni( bool isSchema, OUString const & url,
                          Reference<XCommandEnvironment> const & xCmdEnv );

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
    getSupportedPackageTypes() throw (RuntimeException);
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType)
        throw (deployment::DeploymentException,
               uno::RuntimeException);

    using PackageRegistryBackend::disposing;
};

//______________________________________________________________________________
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
            OUSTR("caught unexpected exception while disposing..."),
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
BackendImpl::BackendImpl(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xComponentContext )
    : PackageRegistryBackend( args, xComponentContext ),
      m_configmgrini_inited( false ),
      m_configmgrini_modified( false ),
      m_xConfDataTypeInfo( new Package::TypeInfo(
                               OUSTR("application/"
                                     "vnd.sun.star.configuration-data"),
                               OUSTR("*.xcu"),
                               getResourceString(RID_STR_CONF_DATA),
                               RID_IMG_CONF_XML ) ),
      m_xConfSchemaTypeInfo( new Package::TypeInfo(
                                 OUSTR("application/"
                                       "vnd.sun.star.configuration-schema"),
                                 OUSTR("*.xcs"),
                                 getResourceString(RID_STR_CONF_SCHEMA),
                                 RID_IMG_CONF_XML ) ),
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
        OUString dbFile = makeURL(getCachePath(), OUSTR("backenddb.xml"));
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

#if !defined(ANDROID) && !defined(IOS)
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<PersistentMap> pMap;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        rtl::OUString aCompatURL( makeURL( getCachePath(), OUSTR("registered_packages.db") ) );

        // Don't create it if it doesn't exist already
        if ( ::utl::UCBContentHelper::Exists( expandUnoRcUrl( aCompatURL ) ) )
        {
            try
            {
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                pMap = ::std::auto_ptr<PersistentMap>( new PersistentMap( aCompatURL ) );
                SAL_WNODEPRECATED_DECLARATIONS_POP
            }
            catch (const Exception &e)
            {
                rtl::OUStringBuffer aStr( "Exception loading legacy package database: '" );
                aStr.append( e.Message );
                aStr.append( "' - ignoring file, please remove it.\n" );
                dp_misc::writeConsole( aStr.getStr() );
            }
        }
        m_registeredPackages = pMap;
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
//______________________________________________________________________________
Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return m_typeInfos;
}
void BackendImpl::packageRemoved(OUString const & url, OUString const & /*mediaType*/)
        throw (deployment::DeploymentException,
               uno::RuntimeException)
{
    if (m_backendDb.get())
        m_backendDb->removeEntry(url);
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    sal_Bool bRemoved, OUString const & identifier,
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
            if (title.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(".xcu") )) {
                mediaType = OUSTR("application/"
                                  "vnd.sun.star.configuration-data");
            }
            if (title.endsWithIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(".xcs") )) {
                mediaType = OUSTR("application/"
                                  "vnd.sun.star.configuration-schema");
            }
        }
        if (mediaType.isEmpty())
            throw lang::IllegalArgumentException(
                StrCannotDetectMediaType::get() + url,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }

    String type, subType;
    INetContentTypeParameterList params;
    if (INetContentTypes::parse( mediaType, type, subType, &params ))
    {
        if (type.EqualsIgnoreCaseAscii("application"))
        {
            OUString name;
            if (!bRemoved)
            {
                ::ucbhelper::Content ucbContent( url, xCmdEnv, m_xComponentContext );
                name = StrTitle::getTitle( ucbContent );
            }

            ::ucbhelper::Content ucbContent( url, xCmdEnv, m_xComponentContext );
            if (subType.EqualsIgnoreCaseAscii(
                    "vnd.sun.star.configuration-data"))
            {
                return new PackageImpl(
                    this, url, name, m_xConfDataTypeInfo, false /* data file */,
                    bRemoved, identifier);
            }
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.configuration-schema")) {
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


//______________________________________________________________________________
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
                makeURL( getCachePath(), OUSTR("configmgr.ini") ),
                xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, OUSTR("SCHEMA="), ucb_content,
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
            if (readLine( &line, OUSTR("DATA="), ucb_content,
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

//______________________________________________________________________________
void BackendImpl::configmgrini_flush(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    if (!m_configmgrini_inited || !m_configmgrini_modified)
        return;

    ::rtl::OStringBuffer buf;
    if (! m_xcs_files.empty())
    {
        t_stringlist::const_iterator iPos( m_xcs_files.begin() );
        t_stringlist::const_iterator const iEnd( m_xcs_files.end() );
        buf.append( RTL_CONSTASCII_STRINGPARAM("SCHEMA=") );
        while (iPos != iEnd) {
            // encoded ASCII file-urls:
            const ::rtl::OString item(
                ::rtl::OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
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
        buf.append( RTL_CONSTASCII_STRINGPARAM("DATA=") );
        while (iPos != iEnd) {
            // encoded ASCII file-urls:
            const ::rtl::OString item(
                ::rtl::OUStringToOString( *iPos, RTL_TEXTENCODING_ASCII_US ) );
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
            ::rtl::ByteSequence(
                reinterpret_cast<sal_Int8 const *>(buf.getStr()),
                buf.getLength() ) ) );
    ::ucbhelper::Content ucb_content(
        makeURL( getCachePath(), OUSTR("configmgr.ini") ), xCmdEnv, m_xComponentContext );
    ucb_content.writeStream( xData, true /* replace existing */ );

    m_configmgrini_modified = false;
}

//______________________________________________________________________________
bool BackendImpl::addToConfigmgrIni( bool isSchema, bool isURL, OUString const & url_,
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
        return true;
    }
    else
        return false;
}

//______________________________________________________________________________
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


// Package
//______________________________________________________________________________

BackendImpl * BackendImpl::PackageImpl::getMyBackend() const
{
    BackendImpl * pBackend = static_cast<BackendImpl *>(m_myBackend.get());
    if (NULL == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            OUSTR("Failed to get the BackendImpl"),
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
    const rtl::OUString url(getURL());

    bool bReg = false;
    if (that->hasActiveEntry(getURL()))
        bReg = true;
#if !defined(ANDROID) && !defined(IOS)
    if (!bReg && that->m_registeredPackages.get())
    {
        // fallback for user extension registered in berkeley DB
        bReg = that->m_registeredPackages->has(
            rtl::OUStringToOString( url, RTL_TEXTENCODING_UTF8 ));
    }
#endif
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true, beans::Ambiguous<sal_Bool>( bReg, false ) );
}

//------------------------------------------------------------------------------
OUString encodeForXml( OUString const & text )
{
    // encode conforming xml:
    sal_Int32 len = text.getLength();
    ::rtl::OUStringBuffer buf;
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        sal_Unicode c = text[ pos ];
        switch (c) {
        case '<':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&lt;") );
            break;
        case '>':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&gt;") );
            break;
        case '&':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&amp;") );
            break;
        case '\'':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&apos;") );
            break;
        case '\"':
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("&quot;") );
            break;
        default:
            buf.append( c );
            break;
        }
    }
    return buf.makeStringAndClear();
}

//______________________________________________________________________________
OUString replaceOrigin(
    OUString const & url, OUString const & destFolder, Reference< XCommandEnvironment > const & xCmdEnv, Reference< XComponentContext > const & xContext, bool & out_replaced)
{
    // looking for %origin%:
    ::ucbhelper::Content ucb_content( url, xCmdEnv, xContext );
    ::rtl::ByteSequence bytes( readFile( ucb_content ) );
    ::rtl::ByteSequence filtered( bytes.getLength() * 2,
                                  ::rtl::BYTESEQ_NODEFAULT );
    bool use_filtered = false;
    ::rtl::OString origin;
    sal_Char const * pBytes = reinterpret_cast<sal_Char const *>(
        bytes.getConstArray());
    sal_Size nBytes = bytes.getLength();
    sal_Int32 write_pos = 0;
    while (nBytes > 0)
    {
        sal_Int32 index = rtl_str_indexOfChar_WithLength( pBytes, nBytes, '%' );
        if (index < 0) {
            if (! use_filtered) // opt
                break;
            index = nBytes;
        }

        if ((write_pos + index) > filtered.getLength())
            filtered.realloc( (filtered.getLength() + index) * 2 );
        memcpy( filtered.getArray() + write_pos, pBytes, index );
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
                     RTL_CONSTASCII_STRINGPARAM("origin%"),
                     RTL_CONSTASCII_LENGTH("origin%")) == 0)
        {
            if (origin.isEmpty()) {
                // encode only once
                origin = ::rtl::OUStringToOString(
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
        if ((write_pos + nAdd) > filtered.getLength())
            filtered.realloc( (filtered.getLength() + nAdd) * 2 );
        memcpy( filtered.getArray() + write_pos, pAdd, nAdd );
        write_pos += nAdd;
    }
    if (!use_filtered)
        return url;
    if (write_pos < filtered.getLength())
        filtered.realloc( write_pos );
    rtl::OUString newUrl(url);
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

//______________________________________________________________________________
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
            if (that->m_eContext != CONTEXT_BUNDLED
                && !startup)
            {
                if (m_isSchema)
                {
                    com::sun::star::configuration::Update::get(
                        that->m_xComponentContext)->insertExtensionXcsFile(
                            that->m_eContext == CONTEXT_SHARED, expandUnoRcUrl(url));
                }
                else
                {
                    com::sun::star::configuration::Update::get(
                        that->m_xComponentContext)->insertExtensionXcuFile(
                            that->m_eContext == CONTEXT_SHARED, expandUnoRcUrl(url));
                }
            }
            that->addToConfigmgrIni( m_isSchema, true, url, xCmdEnv );
            data.iniEntry = dp_misc::makeRcTerm(url);
            that->addDataToDb(getURL(), data);
        }
    }
    else // revoke
    {
#if !defined(ANDROID) && !defined(IOS)
        if (!that->removeFromConfigmgrIni(m_isSchema, url, xCmdEnv) &&
            that->m_registeredPackages.get()) {
            // Obsolete package database handling - should be removed for LibreOffice 4.0
            t_string2string_map entries(
                that->m_registeredPackages->getEntries());
            for (t_string2string_map::iterator i(entries.begin());
                 i != entries.end(); ++i)
            {
                //If the xcu file was installed before the configmgr was chaned
                //to use the configmgr.ini, one needed to rebuild to whole directory
                //structur containing the xcu, xcs files from all extensions. Now,
                //we just add all other xcu/xcs files to the configmgr.ini instead of
                //rebuilding the directory structure.
                rtl::OUString url2(
                    rtl::OStringToOUString(i->first, RTL_TEXTENCODING_UTF8));
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
                    makeURL( that->getCachePath(), OUSTR("registry") ),
                    xCmdEnv, that->getComponentContext() ).executeCommand(
                        OUSTR("delete"), Any( true /* delete physically */ ) );
            }
            catch(const Exception&)
            {
                OSL_ASSERT(0);
            }
        }
#endif
        ::boost::optional<ConfigurationBackendDb::Data> data = that->readDataFromDb(url);
        //If an xcu file was life deployed then always a data entry is written.
        //If the xcu file was already in the configmr.ini then there is also
        //a data entry
        if (!m_isSchema && data)
        {
            com::sun::star::configuration::Update::get(
                that->m_xComponentContext)->removeExtensionXcuFile(expandUnoRcTerm(data->iniEntry));
        }
        that->revokeEntryFromDb(url);
    }
}

} // anon namespace

namespace sdecl = comphelper::service_decl;
sdecl::class_<BackendImpl, sdecl::with_args<true> > serviceBI;
extern sdecl::ServiceDecl const serviceDecl(
    serviceBI,
    "com.sun.star.comp.deployment.configuration.PackageRegistryBackend",
    BACKEND_SERVICE_NAME );

} // namespace configuration
} // namespace backend
} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
