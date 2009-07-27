/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_configuration.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

//TODO: Large parts of this file were copied from dp_component.cxx; those parts
// should be consolidated.

#include "dp_configuration.hrc"
#include "dp_backend.h"
#include "dp_persmap.h"
#include "dp_ucb.h"
#include "dp_xml.h"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "rtl/memory.h"
#include "osl/file.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/servicedecl.hxx"
#include "configmgr/update.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svtools/inettype.hxx"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/io/XActiveDataSink.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/util/XRefreshable.hpp"
#include <list>
#include <memory>


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
            ::rtl::Reference<AbortChannel> const & abortChannel,
            Reference<XCommandEnvironment> const & xCmdEnv );

    public:
        inline PackageImpl(
            ::rtl::Reference<PackageRegistryBackend> const & myBackend,
            OUString const & url, OUString const & name,
            Reference<deployment::XPackageTypeInfo> const & xPackageType,
            bool isSchema )
            : Package( myBackend, url, name, name /* display-name */,
                       xPackageType ),
              m_isSchema( isSchema )
            {}
    };
    friend class PackageImpl;

    t_stringlist m_xcs_files;
    t_stringlist m_xcu_files;
    t_stringlist & getFiles( bool xcs ) {
        return xcs ? m_xcs_files : m_xcu_files;
    }

    bool m_configmgrrc_inited;
    bool m_configmgrrc_modified;

    // PackageRegistryBackend
    virtual Reference<deployment::XPackage> bindPackage_(
        OUString const & url, OUString const & mediaType,
        Reference<XCommandEnvironment> const & xCmdEnv );

    virtual void SAL_CALL disposing();

    const Reference<deployment::XPackageTypeInfo> m_xConfDataTypeInfo;
    const Reference<deployment::XPackageTypeInfo> m_xConfSchemaTypeInfo;
    Sequence< Reference<deployment::XPackageTypeInfo> > m_typeInfos;

    void configmgrrc_verify_init(
        Reference<XCommandEnvironment> const & xCmdEnv );
    void configmgrrc_flush( Reference<XCommandEnvironment> const & xCmdEnv );

    bool addToConfigmgrRc( bool isSchema, OUString const & url,
                     Reference<XCommandEnvironment> const & xCmdEnv );
    bool removeFromConfigmgrRc( bool isSchema, OUString const & url,
                          Reference<XCommandEnvironment> const & xCmdEnv );
    bool hasInConfigmgrRc( bool isSchema, OUString const & url );

public:
    BackendImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XPackageRegistry
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);

    using PackageRegistryBackend::disposing;
};

//______________________________________________________________________________
void BackendImpl::disposing()
{
    try {
        configmgrrc_flush( Reference<XCommandEnvironment>() );

        PackageRegistryBackend::disposing();
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
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
      m_configmgrrc_inited( false ),
      m_configmgrrc_modified( false ),
      m_xConfDataTypeInfo( new Package::TypeInfo(
                               OUSTR("application/"
                                     "vnd.sun.star.configuration-data"),
                               OUSTR("*.xcu"),
                               getResourceString(RID_STR_CONF_DATA),
                               RID_IMG_CONF_XML, RID_IMG_CONF_XML_HC ) ),
      m_xConfSchemaTypeInfo( new Package::TypeInfo(
                                 OUSTR("application/"
                                       "vnd.sun.star.configuration-schema"),
                                 OUSTR("*.xcs"),
                                 getResourceString(RID_STR_CONF_SCHEMA),
                                 RID_IMG_CONF_XML, RID_IMG_CONF_XML_HC ) ),
      m_typeInfos( 2 )
{
    m_typeInfos[ 0 ] = m_xConfDataTypeInfo;
    m_typeInfos[ 1 ] = m_xConfSchemaTypeInfo;

    const Reference<XCommandEnvironment> xCmdEnv;

    if (transientMode()) {
        //TODO
    }
    else {
        configmgrrc_verify_init( xCmdEnv );
    }
}

// XPackageRegistry
//______________________________________________________________________________
Sequence< Reference<deployment::XPackageTypeInfo> >
BackendImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return m_typeInfos;
}

// PackageRegistryBackend
//______________________________________________________________________________
Reference<deployment::XPackage> BackendImpl::bindPackage_(
    OUString const & url, OUString const & mediaType_,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    OUString mediaType( mediaType_ );
    if (mediaType.getLength() == 0)
    {
        // detect media-type:
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, url, xCmdEnv ))
        {
            const OUString title( ucbContent.getPropertyValue(
                                      StrTitle::get() ).get<OUString>() );
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
        if (mediaType.getLength() == 0)
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
            ::ucbhelper::Content ucbContent( url, xCmdEnv );
            if (subType.EqualsIgnoreCaseAscii(
                    "vnd.sun.star.configuration-data")) {
                return new PackageImpl(
                    this, url, ucbContent.getPropertyValue(
                        StrTitle::get() ).get<OUString>(),
                    m_xConfDataTypeInfo, false /* data file */ );
            }
            else if (subType.EqualsIgnoreCaseAscii(
                         "vnd.sun.star.configuration-schema")) {
                return new PackageImpl(
                    this, url, ucbContent.getPropertyValue(
                        StrTitle::get() ).get<OUString>(),
                    m_xConfSchemaTypeInfo, true /* schema file */ );
            }
        }
    }
    throw lang::IllegalArgumentException(
        StrUnsupportedMediaType::get() + mediaType,
        static_cast<OWeakObject *>(this),
        static_cast<sal_Int16>(-1) );
}

//##############################################################################

//______________________________________________________________________________
void BackendImpl::configmgrrc_verify_init(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    const ::osl::MutexGuard guard( getMutex() );
    if (! m_configmgrrc_inited)
    {
        // common rc:
        ::ucbhelper::Content ucb_content;
        if (create_ucb_content(
                &ucb_content,
                makeURL( getCachePath(), OUSTR("configmgrrc") ),
                xCmdEnv, false /* no throw */ ))
        {
            OUString line;
            if (readLine( &line, OUSTR("SCHEMA="), ucb_content,
                          RTL_TEXTENCODING_UTF8 ))
            {
                sal_Int32 index = sizeof ("SCHEMA=") - 1;
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0) {
                        // cleanup, check if existing:
                        if (create_ucb_content(
                                0, expandUnoRcTerm(token), xCmdEnv,
                                false /* no throw */ )) {
                            m_xcs_files.push_back( token );
                        }
                        else
                            OSL_ENSURE(
                                0, "### invalid SCHEMA entry!" );
                    }
                }
                while (index >= 0);
            }
            if (readLine( &line, OUSTR("DATA="), ucb_content,
                          RTL_TEXTENCODING_UTF8 )) {
                sal_Int32 index = sizeof ("DATA=") - 1;
                do {
                    OUString token( line.getToken( 0, ' ', index ).trim() );
                    if (token.getLength() > 0) {
                        if (token[ 0 ] == '?')
                            token = token.copy( 1 );
                        // cleanup, check if existing:
                        if (create_ucb_content(
                                0, expandUnoRcTerm(token),
                                xCmdEnv, false /* no throw */ )) {
                            m_xcu_files.push_back( token );
                        }
                        else
                            OSL_ENSURE( 0, "### invalid DATA entry!" );
                    }
                }
                while (index >= 0);
            }
        }
        m_configmgrrc_modified = false;
        m_configmgrrc_inited = true;
    }
}

//______________________________________________________________________________
void BackendImpl::configmgrrc_flush(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (transientMode())
        return;
    if (!m_configmgrrc_inited || !m_configmgrrc_modified)
        return;

    ::rtl::OStringBuffer buf;
    // UNO_USER_PACKAGES_CACHE, UNO_SHARED_PACKAGES_CACHE have to be resolved
    // locally:
    if (m_eContext == CONTEXT_USER) {
        buf.append( RTL_CONSTASCII_STRINGPARAM(
                        "UNO_USER_PACKAGES_CACHE=$ORIGIN/../..") );
    }
    else if (m_eContext == CONTEXT_SHARED) {
        buf.append( RTL_CONSTASCII_STRINGPARAM(
                        "UNO_SHARED_PACKAGES_CACHE=$ORIGIN/../..") );
    }
    else
        OSL_ASSERT(0);
    buf.append(LF);

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

    // write configmgrrc:
    const Reference<io::XInputStream> xData(
        ::xmlscript::createInputStream(
            ::rtl::ByteSequence(
                reinterpret_cast<sal_Int8 const *>(buf.getStr()),
                buf.getLength() ) ) );
    ::ucbhelper::Content ucb_content(
        makeURL( getCachePath(), OUSTR("configmgrrc") ), xCmdEnv );
    ucb_content.writeStream( xData, true /* replace existing */ );

    m_configmgrrc_modified = false;
}

//------------------------------------------------------------------------------
inline OUString makeRcTerm( OUString const & url )
{
    OSL_ASSERT( url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                     "vnd.sun.star.expand:") ) );
    if (url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") )) {
        // cut protocol:
        OUString rcterm( url.copy( sizeof ("vnd.sun.star.expand:") - 1 ) );
        // decode uric class chars:
        rcterm = ::rtl::Uri::decode(
            rcterm, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        return rcterm;
    }
    else
        return url;
}

//______________________________________________________________________________
bool BackendImpl::addToConfigmgrRc( bool isSchema, OUString const & url_,
                              Reference<XCommandEnvironment> const & xCmdEnv )
{
    const OUString rcterm( makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    configmgrrc_verify_init( xCmdEnv );
    t_stringlist & rSet = getFiles(isSchema);
    if (::std::find( rSet.begin(), rSet.end(), rcterm ) == rSet.end()) {
        rSet.push_front( rcterm ); // prepend to list, thus overriding
        // write immediately:
        m_configmgrrc_modified = true;
        configmgrrc_flush( xCmdEnv );
        return true;
    }
    else
        return false;
}

//______________________________________________________________________________
bool BackendImpl::removeFromConfigmgrRc(
    bool isSchema, OUString const & url_,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    const OUString rcterm( makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    configmgrrc_verify_init( xCmdEnv );
    getFiles(isSchema).remove( rcterm );
    if (!isSchema) { //TODO: see replaceOrigin()
        getFiles(isSchema).remove(
            rcterm + OUString(RTL_CONSTASCII_USTRINGPARAM(".mod")));
    }
    // write immediately:
    m_configmgrrc_modified = true;
    configmgrrc_flush( xCmdEnv );
    return true;
}

//______________________________________________________________________________
bool BackendImpl::hasInConfigmgrRc(
    bool isSchema, OUString const & url_ )
{
    const OUString rcterm( makeRcTerm(url_) );
    const ::osl::MutexGuard guard( getMutex() );
    t_stringlist const & rSet = getFiles(isSchema);
    return ::std::find( rSet.begin(), rSet.end(), rcterm ) != rSet.end()
        || (!isSchema && //TODO: see replaceOrigin()
            ::std::find(
                rSet.begin(), rSet.end(),
                rcterm + OUString(RTL_CONSTASCII_USTRINGPARAM(".mod"))) !=
            rSet.end());
}

//##############################################################################

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
    return beans::Optional< beans::Ambiguous<sal_Bool> >(
        true /* IsPresent */,
        beans::Ambiguous<sal_Bool>(
            that->hasInConfigmgrRc( m_isSchema, getURL() ),
            false /* IsAmbiguous */ ) );
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
    OUString const & url, Reference< XCommandEnvironment > const & xCmdEnv )
{
    // looking for %origin%:
    ::ucbhelper::Content ucb_content( url, xCmdEnv );
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
        rtl_copyMemory( filtered.getArray() + write_pos, pBytes, index );
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
                     sizeof ("origin%") - 1 ) == 0)
        {
            if (origin.getLength() == 0) {
                // encode only once
                origin = ::rtl::OUStringToOString(
                    encodeForXml( url.copy( 0, url.lastIndexOf( '/' ) ) ),
                    // xxx todo: encode always for UTF-8? => lookup doc-header?
                    RTL_TEXTENCODING_UTF8 );
            }
            pAdd = origin.getStr();
            nAdd = origin.getLength();
            pBytes += (sizeof ("origin%") - 1);
            nBytes -= (sizeof ("origin%") - 1);
            use_filtered = true;
        }
        if ((write_pos + nAdd) > filtered.getLength())
            filtered.realloc( (filtered.getLength() + nAdd) * 2 );
        rtl_copyMemory( filtered.getArray() + write_pos, pAdd, nAdd );
        write_pos += nAdd;
    }
    if (!use_filtered)
        return url;
    if (write_pos < filtered.getLength())
        filtered.realloc( write_pos );
    rtl::OUString newUrl(url + OUString(RTL_CONSTASCII_USTRINGPARAM(".mod")));
        //TODO: unique name
    ucbhelper::Content(newUrl, xCmdEnv).writeStream(
        xmlscript::createInputStream(filtered), true);
    return newUrl;
}

//______________________________________________________________________________
void BackendImpl::PackageImpl::processPackage_(
    ::osl::ResettableMutexGuard &,
    bool doRegisterPackage,
    ::rtl::Reference<AbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    BackendImpl * that = getMyBackend();
    OUString url( getURL() );

    if (doRegisterPackage)
    {
        if (m_isSchema)
        {
            configmgr::update::insertXcsFile(13/*TODO*/, expandUnoRcUrl(url));
        }
        else
        {
            url = replaceOrigin(url, xCmdEnv);
            configmgr::update::insertXcuFile(13/*TODO*/, expandUnoRcUrl(url));
        }

        that->addToConfigmgrRc( m_isSchema, url, xCmdEnv );
    }
    else // revoke
    {
        that->removeFromConfigmgrRc( m_isSchema, url, xCmdEnv );

        //TODO: revoking at runtime, possible, sensible?
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

