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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_registry.hrc"
#include "dp_misc.h"
#include "dp_resource.h"
#include "dp_interact.h"
#include "dp_ucb.h"
#include "osl/diagnose.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/sequence.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/util/XUpdatable.hpp"
#include "com/sun/star/container/XContentEnumerationAccess.hpp"
#include "com/sun/star/deployment/PackageRegistryBackend.hpp"
#include <boost/unordered_map.hpp>
#include <set>
#include <boost/unordered_set.hpp>
#include <memory>

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;


namespace dp_registry {

namespace backend {
namespace bundle {
Reference<deployment::XPackageRegistry> create(
    Reference<deployment::XPackageRegistry> const & xRootRegistry,
    OUString const & context, OUString const & cachePath, bool readOnly,
    Reference<XComponentContext> const & xComponentContext );
}
}

namespace {

typedef ::cppu::WeakComponentImplHelper2<
    deployment::XPackageRegistry, util::XUpdatable > t_helper;

//==============================================================================
class PackageRegistryImpl : private MutexHolder, public t_helper
{
    struct ci_string_hash {
        ::std::size_t operator () ( OUString const & str ) const {
            return str.toAsciiLowerCase().hashCode();
        }
    };
    struct ci_string_equals {
        bool operator () ( OUString const & str1, OUString const & str2 ) const{
            return str1.equalsIgnoreAsciiCase( str2 );
        }
    };
    typedef ::boost::unordered_map<
        OUString, Reference<deployment::XPackageRegistry>,
        ci_string_hash, ci_string_equals > t_string2registry;
    typedef ::boost::unordered_map<
        OUString, OUString,
        ci_string_hash, ci_string_equals > t_string2string;
    typedef ::std::set<
        Reference<deployment::XPackageRegistry> > t_registryset;

    t_string2registry m_mediaType2backend;
    t_string2string m_filter2mediaType;
    t_registryset m_ambiguousBackends;
    t_registryset m_allBackends;
    ::std::vector< Reference<deployment::XPackageTypeInfo> > m_typesInfos;

    void insertBackend(
        Reference<deployment::XPackageRegistry> const & xBackend );

protected:
    inline void check();
    virtual void SAL_CALL disposing();

    virtual ~PackageRegistryImpl();
    PackageRegistryImpl() : t_helper( getMutex() ) {}


public:
    static Reference<deployment::XPackageRegistry> create(
        OUString const & context,
        OUString const & cachePath, bool readOnly,
        Reference<XComponentContext> const & xComponentContext );

    // XUpdatable
    virtual void SAL_CALL update() throw (RuntimeException);

    // XPackageRegistry
    virtual Reference<deployment::XPackage> SAL_CALL bindPackage(
        OUString const & url, OUString const & mediaType, sal_Bool bRemoved,
        OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv )
        throw (deployment::DeploymentException,
               deployment::InvalidRemovedParameterException,
               CommandFailedException,
               lang::IllegalArgumentException, RuntimeException);
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (RuntimeException);
};

//______________________________________________________________________________
inline void PackageRegistryImpl::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            OUSTR("PackageRegistry instance has already been disposed!"),
            static_cast<OWeakObject *>(this) );
    }
}

//______________________________________________________________________________
void PackageRegistryImpl::disposing()
{
    // dispose all backends:
    t_registryset::const_iterator iPos( m_allBackends.begin() );
    t_registryset::const_iterator const iEnd( m_allBackends.end() );
    for ( ; iPos != iEnd; ++iPos ) {
        try_dispose( *iPos );
    }
    m_mediaType2backend = t_string2registry();
    m_ambiguousBackends = t_registryset();
    m_allBackends = t_registryset();

    t_helper::disposing();
}

//______________________________________________________________________________
PackageRegistryImpl::~PackageRegistryImpl()
{
}

//______________________________________________________________________________
OUString normalizeMediaType( OUString const & mediaType )
{
    ::rtl::OUStringBuffer buf;
    sal_Int32 index = 0;
    for (;;) {
        buf.append( mediaType.getToken( 0, '/', index ).trim() );
        if (index < 0)
            break;
        buf.append( static_cast< sal_Unicode >('/') );
    }
    return buf.makeStringAndClear();
}

//______________________________________________________________________________

void PackageRegistryImpl::insertBackend(
    Reference<deployment::XPackageRegistry> const & xBackend )
{
    m_allBackends.insert( xBackend );
    typedef ::boost::unordered_set<OUString, ::rtl::OUStringHash> t_stringset;
    t_stringset ambiguousFilters;

    const Sequence< Reference<deployment::XPackageTypeInfo> > packageTypes(
        xBackend->getSupportedPackageTypes() );
    for ( sal_Int32 pos = 0; pos < packageTypes.getLength(); ++pos )
    {
        Reference<deployment::XPackageTypeInfo> const & xPackageType =
            packageTypes[ pos ];
        m_typesInfos.push_back( xPackageType );

        const OUString mediaType( normalizeMediaType(
                                      xPackageType->getMediaType() ) );
        ::std::pair<t_string2registry::iterator, bool> mb_insertion(
            m_mediaType2backend.insert( t_string2registry::value_type(
                                            mediaType, xBackend ) ) );
        if (mb_insertion.second)
        {
            // add parameterless media-type, too:
            sal_Int32 semi = mediaType.indexOf( ';' );
            if (semi >= 0) {
                m_mediaType2backend.insert(
                    t_string2registry::value_type(
                        mediaType.copy( 0, semi ), xBackend ) );
            }
            const OUString fileFilter( xPackageType->getFileFilter() );
            //The package backend shall also be called to determine the mediatype
            //(XPackageRegistry.bindPackage) when the URL points to a directory.
            const bool bExtension = mediaType.equals(OUSTR("application/vnd.sun.star.package-bundle"));
            if (fileFilter.getLength() == 0 ||
                fileFilter.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("*.*") ) ||
                fileFilter.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("*") ) ||
                bExtension)
            {
                m_ambiguousBackends.insert( xBackend );
            }
            else
            {
                sal_Int32 nIndex = 0;
                do {
                    OUString token( fileFilter.getToken( 0, ';', nIndex ) );
                    if (token.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("*.") ))
                        token = token.copy( 1 );
                    if (token.getLength() == 0)
                        continue;
                    // mark any further wildcards ambig:
                    bool ambig = (token.indexOf('*') >= 0 ||
                                  token.indexOf('?') >= 0);
                    if (! ambig) {
                        ::std::pair<t_string2string::iterator, bool> ins(
                            m_filter2mediaType.insert(
                                t_string2string::value_type(
                                    token, mediaType ) ) );
                        ambig = !ins.second;
                        if (ambig) {
                            // filter has already been in: add previously
                            // added backend to ambig set
                            const t_string2registry::const_iterator iFind(
                                m_mediaType2backend.find(
                                    /* media-type of pr. added backend */
                                    ins.first->second ) );
                            OSL_ASSERT(
                                iFind != m_mediaType2backend.end() );
                            if (iFind != m_mediaType2backend.end())
                                m_ambiguousBackends.insert( iFind->second );
                        }
                    }
                    if (ambig) {
                        m_ambiguousBackends.insert( xBackend );
                        // mark filter to be removed later from filters map:
                        ambiguousFilters.insert( token );
                    }
                }
                while (nIndex >= 0);
            }
        }
#if OSL_DEBUG_LEVEL > 0
        else {
            ::rtl::OUStringBuffer buf;
            buf.appendAscii(
                RTL_CONSTASCII_STRINGPARAM(
                    "more than one PackageRegistryBackend for "
                    "media-type=\"") );
            buf.append( mediaType );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" => ") );
            buf.append( Reference<lang::XServiceInfo>(
                            xBackend, UNO_QUERY_THROW )->
                        getImplementationName() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            buf.makeStringAndClear(),
                            RTL_TEXTENCODING_UTF8 ) );
        }
#endif
    }

    // cut out ambiguous filters:
    t_stringset::const_iterator iPos( ambiguousFilters.begin() );
    const t_stringset::const_iterator iEnd( ambiguousFilters.end() );
    for ( ; iPos != iEnd; ++iPos ) {
        m_filter2mediaType.erase( *iPos );
    }
}

//______________________________________________________________________________
Reference<deployment::XPackageRegistry> PackageRegistryImpl::create(
    OUString const & context,
    OUString const & cachePath, bool readOnly,
    Reference<XComponentContext> const & xComponentContext )
{
    PackageRegistryImpl * that = new PackageRegistryImpl;
    Reference<deployment::XPackageRegistry> xRet(that);

    // auto-detect all registered package registries:
    Reference<container::XEnumeration> xEnum(
        Reference<container::XContentEnumerationAccess>(
            xComponentContext->getServiceManager(),
            UNO_QUERY_THROW )->createContentEnumeration(
                OUSTR("com.sun.star.deployment.PackageRegistryBackend") ) );
    if (xEnum.is())
    {
        while (xEnum->hasMoreElements())
        {
            Any element( xEnum->nextElement() );
            Sequence<Any> registryArgs(
                cachePath.getLength() == 0 ? 1 : 3 );
            registryArgs[ 0 ] <<= context;
            if (cachePath.getLength() > 0)
            {
                Reference<lang::XServiceInfo> xServiceInfo(
                    element, UNO_QUERY_THROW );
                OUString registryCachePath(
                    makeURL( cachePath,
                             ::rtl::Uri::encode(
                                 xServiceInfo->getImplementationName(),
                                 rtl_UriCharClassPchar,
                                 rtl_UriEncodeIgnoreEscapes,
                                 RTL_TEXTENCODING_UTF8 ) ) );
                registryArgs[ 1 ] <<= registryCachePath;
                registryArgs[ 2 ] <<= readOnly;
                if (! readOnly)
                    create_folder( 0, registryCachePath,
                                   Reference<XCommandEnvironment>() );
            }

            Reference<deployment::XPackageRegistry> xBackend;
            Reference<lang::XSingleComponentFactory> xFac( element, UNO_QUERY );
            if (xFac.is()) {
                xBackend.set(
                    xFac->createInstanceWithArgumentsAndContext(
                        registryArgs, xComponentContext ), UNO_QUERY );
            }
            else {
                Reference<lang::XSingleServiceFactory> xSingleServiceFac(
                    element, UNO_QUERY_THROW );
                xBackend.set(
                    xSingleServiceFac->createInstanceWithArguments(
                        registryArgs ), UNO_QUERY );
            }
            if (! xBackend.is()) {
                throw DeploymentException(
                    OUSTR("cannot instantiate PackageRegistryBackend service: ")
                    + Reference<lang::XServiceInfo>(
                        element, UNO_QUERY_THROW )->getImplementationName(),
                    static_cast<OWeakObject *>(that) );
            }

            that->insertBackend( xBackend );
        }
    }

    // Insert bundle back-end.
    // Always register as last, because we want to add extensions also as folders
    // and as a default we accept every folder, which was not recognized by the other
    // backends.
    Reference<deployment::XPackageRegistry> extensionBackend =
        ::dp_registry::backend::bundle::create(
            that, context, cachePath, readOnly, xComponentContext);
    that->insertBackend(extensionBackend);

    Reference<lang::XServiceInfo> xServiceInfo(
        extensionBackend, UNO_QUERY_THROW );

    OSL_ASSERT(xServiceInfo.is());
    OUString registryCachePath(
        makeURL( cachePath,
                 ::rtl::Uri::encode(
                     xServiceInfo->getImplementationName(),
                     rtl_UriCharClassPchar,
                     rtl_UriEncodeIgnoreEscapes,
                     RTL_TEXTENCODING_UTF8 ) ) );
    create_folder( 0, registryCachePath, Reference<XCommandEnvironment>());


#if OSL_DEBUG_LEVEL > 1
    // dump tables:
    {
        t_registryset allBackends;
        dp_misc::TRACE("> [dp_registry.cxx] media-type detection:\n\n" );
        for ( t_string2string::const_iterator iPos(
                  that->m_filter2mediaType.begin() );
              iPos != that->m_filter2mediaType.end(); ++iPos )
        {
            ::rtl::OUStringBuffer buf;
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("extension \"") );
            buf.append( iPos->first );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                 "\" maps to media-type \"") );
            buf.append( iPos->second );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                 "\" maps to backend ") );
            const Reference<deployment::XPackageRegistry> xBackend(
                that->m_mediaType2backend.find( iPos->second )->second );
            allBackends.insert( xBackend );
            buf.append( Reference<lang::XServiceInfo>(
                            xBackend, UNO_QUERY_THROW )
                        ->getImplementationName() );
            dp_misc::writeConsole( buf.makeStringAndClear() + OUSTR("\n"));
        }
        dp_misc::TRACE( "> [dp_registry.cxx] ambiguous backends:\n\n" );
        for ( t_registryset::const_iterator iPos(
                  that->m_ambiguousBackends.begin() );
              iPos != that->m_ambiguousBackends.end(); ++iPos )
        {
            ::rtl::OUStringBuffer buf;
            buf.append(
                Reference<lang::XServiceInfo>(
                    *iPos, UNO_QUERY_THROW )->getImplementationName() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(": ") );
            const Sequence< Reference<deployment::XPackageTypeInfo> > types(
                (*iPos)->getSupportedPackageTypes() );
            for ( sal_Int32 pos = 0; pos < types.getLength(); ++pos ) {
                Reference<deployment::XPackageTypeInfo> const & xInfo =
                    types[ pos ];
                buf.append( xInfo->getMediaType() );
                const OUString filter( xInfo->getFileFilter() );
                if (filter.getLength() > 0) {
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(" (") );
                    buf.append( filter );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(")") );
                }
                if (pos < (types.getLength() - 1))
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(", ") );
            }
            dp_misc::TRACE(buf.makeStringAndClear() + OUSTR("\n\n"));
        }
        allBackends.insert( that->m_ambiguousBackends.begin(),
                            that->m_ambiguousBackends.end() );
        OSL_ASSERT( allBackends == that->m_allBackends );
    }
#endif

    return xRet;
}

// XUpdatable: broadcast to backends
//______________________________________________________________________________
void PackageRegistryImpl::update() throw (RuntimeException)
{
    check();
    t_registryset::const_iterator iPos( m_allBackends.begin() );
    const t_registryset::const_iterator iEnd( m_allBackends.end() );
    for ( ; iPos != iEnd; ++iPos ) {
        const Reference<util::XUpdatable> xUpdatable( *iPos, UNO_QUERY );
        if (xUpdatable.is())
            xUpdatable->update();
    }
}

// XPackageRegistry
//______________________________________________________________________________
Reference<deployment::XPackage> PackageRegistryImpl::bindPackage(
    OUString const & url, OUString const & mediaType_, sal_Bool bRemoved,
    OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException, deployment::InvalidRemovedParameterException,
           CommandFailedException,
           lang::IllegalArgumentException, RuntimeException)
{
    check();
    OUString mediaType(mediaType_);
    if (mediaType.getLength() == 0)
    {
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content(
                &ucbContent, url, xCmdEnv, false /* no throw */ )
                && !ucbContent.isFolder())
        {
            OUString title( ucbContent.getPropertyValue(
                                StrTitle::get() ).get<OUString>() );
            for (;;)
            {
                const t_string2string::const_iterator iFind(
                    m_filter2mediaType.find(title) );
                if (iFind != m_filter2mediaType.end()) {
                    mediaType = iFind->second;
                    break;
                }
                sal_Int32 point = title.indexOf( '.', 1 /* consume . */ );
                if (point < 0)
                    break;
                title = title.copy(point);
            }
        }
    }
    if (mediaType.getLength() == 0)
    {
        // try ambiguous backends:
        t_registryset::const_iterator iPos( m_ambiguousBackends.begin() );
        const t_registryset::const_iterator iEnd( m_ambiguousBackends.end() );
        for ( ; iPos != iEnd; ++iPos )
        {
            try {
                return (*iPos)->bindPackage( url, mediaType, bRemoved,
                    identifier, xCmdEnv );
            }
            catch (lang::IllegalArgumentException &) {
            }
        }
        throw lang::IllegalArgumentException(
            getResourceString(RID_STR_CANNOT_DETECT_MEDIA_TYPE) + url,
            static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
    }
    else
    {
        // get backend by media-type:
        t_string2registry::const_iterator iFind(
            m_mediaType2backend.find( normalizeMediaType(mediaType) ) );
        if (iFind == m_mediaType2backend.end()) {
            // xxx todo: more sophisticated media-type argument parsing...
            sal_Int32 q = mediaType.indexOf( ';' );
            if (q >= 0) {
                iFind = m_mediaType2backend.find(
                    normalizeMediaType(
                        // cut parameters:
                        mediaType.copy( 0, q ) ) );
            }
        }
        if (iFind == m_mediaType2backend.end()) {
            throw lang::IllegalArgumentException(
                getResourceString(RID_STR_UNSUPPORTED_MEDIA_TYPE) + mediaType,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
        }
        return iFind->second->bindPackage( url, mediaType, bRemoved,
            identifier, xCmdEnv );
    }
}

//______________________________________________________________________________
Sequence< Reference<deployment::XPackageTypeInfo> >
PackageRegistryImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    return comphelper::containerToSequence(m_typesInfos);
}
} // anon namespace

//==============================================================================
Reference<deployment::XPackageRegistry> SAL_CALL create(
    OUString const & context,
    OUString const & cachePath, bool readOnly,
    Reference<XComponentContext> const & xComponentContext )
{
    return PackageRegistryImpl::create(
        context, cachePath, readOnly, xComponentContext );
}

} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
