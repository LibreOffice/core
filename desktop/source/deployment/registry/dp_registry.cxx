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

#include <sal/config.h>
#include <sal/log.hxx>

#include <dp_shared.hxx>
#include <dp_package.hxx>
#include <strings.hrc>
#include <dp_registry.hxx>
#include <dp_misc.h>
#include <dp_ucb.h>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/sequence.hxx>
#include <ucbhelper/content.hxx>
#include <o3tl/string_view.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/deployment/XPackageTypeInfo.hpp>
#include <com/sun/star/deployment/XPackageRegistry.hpp>
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;


namespace dp_registry {

namespace {

typedef ::cppu::WeakComponentImplHelper<
    deployment::XPackageRegistry, util::XUpdatable > t_helper;


class PackageRegistryImpl : private cppu::BaseMutex, public t_helper
{
    struct ci_string_hash {
        std::size_t operator () ( OUString const & str ) const {
            return str.toAsciiLowerCase().hashCode();
        }
    };
    struct ci_string_equals {
        bool operator () ( std::u16string_view str1, std::u16string_view str2 ) const{
            return o3tl::equalsIgnoreAsciiCase( str1, str2 );
        }
    };
    typedef std::unordered_map<
        OUString, Reference<deployment::XPackageRegistry>,
        ci_string_hash, ci_string_equals > t_string2registry;
    typedef std::unordered_map<
        OUString, OUString,
        ci_string_hash, ci_string_equals > t_string2string;
    typedef std::set<
        Reference<deployment::XPackageRegistry> > t_registryset;

    t_string2registry m_mediaType2backend;
    t_string2string m_filter2mediaType;
    t_registryset m_ambiguousBackends;
    t_registryset m_allBackends;
    std::vector< Reference<deployment::XPackageTypeInfo> > m_typesInfos;

    void insertBackend(
        Reference<deployment::XPackageRegistry> const & xBackend );

protected:
    void check();
    virtual void SAL_CALL disposing() override;

    virtual ~PackageRegistryImpl() override;
    PackageRegistryImpl() : t_helper( m_aMutex ) {}


public:
    static Reference<deployment::XPackageRegistry> create(
        OUString const & context,
        OUString const & cachePath,
        Reference<XComponentContext> const & xComponentContext );

    // XUpdatable
    virtual void SAL_CALL update() override;

    // XPackageRegistry
    virtual Reference<deployment::XPackage> SAL_CALL bindPackage(
        OUString const & url, OUString const & mediaType, sal_Bool bRemoved,
        OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv ) override;
    virtual Sequence< Reference<deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() override;
    virtual void SAL_CALL packageRemoved(OUString const & url, OUString const & mediaType) override;

};


void PackageRegistryImpl::check()
{
    ::osl::MutexGuard guard( m_aMutex );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            u"PackageRegistry instance has already been disposed!"_ustr,
            static_cast<OWeakObject *>(this) );
    }
}


void PackageRegistryImpl::disposing()
{
    // dispose all backends:
    for (auto const& backend : m_allBackends)
    {
        try_dispose(backend);
    }
    m_mediaType2backend = t_string2registry();
    m_ambiguousBackends = t_registryset();
    m_allBackends = t_registryset();

    t_helper::disposing();
}


PackageRegistryImpl::~PackageRegistryImpl()
{
}


OUString normalizeMediaType( std::u16string_view mediaType )
{
    OUStringBuffer buf;
    sal_Int32 index = 0;
    for (;;) {
        buf.append( o3tl::trim(o3tl::getToken(mediaType, 0, '/', index )) );
        if (index < 0)
            break;
        buf.append( '/' );
    }
    return buf.makeStringAndClear();
}


void PackageRegistryImpl::packageRemoved(
    OUString const & url, OUString const & mediaType)
{
    const t_string2registry::const_iterator i =
        m_mediaType2backend.find(mediaType);

    if (i != m_mediaType2backend.end())
    {
        i->second->packageRemoved(url, mediaType);
    }
}

void PackageRegistryImpl::insertBackend(
    Reference<deployment::XPackageRegistry> const & xBackend )
{
    m_allBackends.insert( xBackend );
    std::unordered_set<OUString> ambiguousFilters;

    const Sequence< Reference<deployment::XPackageTypeInfo> > packageTypes(
        xBackend->getSupportedPackageTypes() );
    for ( Reference<deployment::XPackageTypeInfo> const & xPackageType : packageTypes )
    {
        m_typesInfos.push_back( xPackageType );

        const OUString mediaType( normalizeMediaType(
                                      xPackageType->getMediaType() ) );
        std::pair<t_string2registry::iterator, bool> a_insertion(
            m_mediaType2backend.emplace( mediaType, xBackend ) );
        if (a_insertion.second)
        {
            // add parameterless media-type, too:
            sal_Int32 semi = mediaType.indexOf( ';' );
            if (semi >= 0) {
                m_mediaType2backend.emplace( mediaType.copy( 0, semi ), xBackend );
            }
            const OUString fileFilter( xPackageType->getFileFilter() );
            //The package backend shall also be called to determine the mediatype
            //(XPackageRegistry.bindPackage) when the URL points to a directory.
            const bool bExtension = (mediaType == "application/vnd.sun.star.package-bundle");
            if (fileFilter.isEmpty() || fileFilter == "*.*" || fileFilter == "*" || bExtension)
            {
                m_ambiguousBackends.insert( xBackend );
            }
            else
            {
                sal_Int32 nIndex = 0;
                do {
                    OUString token( fileFilter.getToken( 0, ';', nIndex ) );
                    if (token.match( "*." ))
                        token = token.copy( 1 );
                    if (token.isEmpty())
                        continue;
                    // mark any further wildcards ambig:
                    bool ambig = (token.indexOf('*') >= 0 ||
                                  token.indexOf('?') >= 0);
                    if (! ambig) {
                        std::pair<t_string2string::iterator, bool> ins(
                            m_filter2mediaType.emplace(
                                    token, mediaType ) );
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
        else
        {
            SAL_WARN( "desktop", "more than one PackageRegistryBackend for media-type=\""
                    << mediaType
                    << "\" => "
                    <<  Reference<lang::XServiceInfo>(
                            xBackend, UNO_QUERY_THROW )->getImplementationName()
                    << "\"!" );
        }
#endif
    }

    // cut out ambiguous filters:
    for (auto const& ambiguousFilter : ambiguousFilters)
    {
        m_filter2mediaType.erase(ambiguousFilter);
    }
}


Reference<deployment::XPackageRegistry> PackageRegistryImpl::create(
    OUString const & context,
    OUString const & cachePath,
    Reference<XComponentContext> const & xComponentContext )
{
    rtl::Reference<PackageRegistryImpl> that = new PackageRegistryImpl;

    // auto-detect all registered package registries:
    Reference<container::XEnumeration> xEnum(
        Reference<container::XContentEnumerationAccess>(
            xComponentContext->getServiceManager(),
            UNO_QUERY_THROW )->createContentEnumeration(
                u"com.sun.star.deployment.PackageRegistryBackend"_ustr ) );
    if (xEnum.is())
    {
        while (xEnum->hasMoreElements())
        {
            Any element( xEnum->nextElement() );
            Sequence<Any> registryArgs(cachePath.isEmpty() ? 1 : 3 );
            auto pregistryArgs = registryArgs.getArray();
            pregistryArgs[ 0 ] <<= context;
            if (!cachePath.isEmpty())
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
                pregistryArgs[ 1 ] <<= registryCachePath;
                pregistryArgs[ 2 ] <<= false; // readOnly;
                create_folder( nullptr, registryCachePath,
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
                    "cannot instantiate PackageRegistryBackend service: "
                    + Reference<lang::XServiceInfo>(
                        element, UNO_QUERY_THROW )->getImplementationName(),
                    static_cast<OWeakObject *>(that.get()) );
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
            that, context, cachePath, xComponentContext);
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
    create_folder( nullptr, registryCachePath, Reference<XCommandEnvironment>());


#if OSL_DEBUG_LEVEL > 0
    // dump tables:
    {
        t_registryset allBackends;
        dp_misc::TRACE(u"> [dp_registry.cxx] media-type detection:\n\n"_ustr );
        for (auto const& elem : that->m_filter2mediaType)
        {
            const Reference<deployment::XPackageRegistry> xBackend(
                that->m_mediaType2backend.find( elem.second )->second );
            allBackends.insert( xBackend );
            dp_misc::TRACE(
                "extension \"" + elem.first + "\" maps to media-type \"" + elem.second
                + "\" maps to backend "
                + Reference<lang::XServiceInfo>(
                            xBackend, UNO_QUERY_THROW )
                        ->getImplementationName()
                + "\n");
        }
        dp_misc::TRACE( u"> [dp_registry.cxx] ambiguous backends:\n\n"_ustr );
        for (auto const& ambiguousBackend : that->m_ambiguousBackends)
        {
            OUStringBuffer buf;
            buf.append(
                Reference<lang::XServiceInfo>(
                    ambiguousBackend, UNO_QUERY_THROW )->getImplementationName()
                + ": " );
            const Sequence< Reference<deployment::XPackageTypeInfo> > types(
                ambiguousBackend->getSupportedPackageTypes() );
            for ( sal_Int32 pos = 0; pos < types.getLength(); ++pos ) {
                Reference<deployment::XPackageTypeInfo> const & xInfo =
                    types[ pos ];
                buf.append( xInfo->getMediaType() );
                const OUString filter( xInfo->getFileFilter() );
                if (!filter.isEmpty()) {
                    buf.append( " (" + filter + ")" );
                }
                if (pos < (types.getLength() - 1))
                    buf.append( ", " );
            }
            dp_misc::TRACE(buf + "\n\n");
        }
        allBackends.insert( that->m_ambiguousBackends.begin(),
                            that->m_ambiguousBackends.end() );
        OSL_ASSERT( allBackends == that->m_allBackends );
    }
#endif

    return that;
}

// XUpdatable: broadcast to backends

void PackageRegistryImpl::update()
{
    check();
    for (auto const& backend : m_allBackends)
    {
        const Reference<util::XUpdatable> xUpdatable(backend, UNO_QUERY);
        if (xUpdatable.is())
            xUpdatable->update();
    }
}

// XPackageRegistry

Reference<deployment::XPackage> PackageRegistryImpl::bindPackage(
    OUString const & url, OUString const & mediaType_, sal_Bool bRemoved,
    OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv )
{
    check();
    OUString mediaType(mediaType_);
    if (mediaType.isEmpty())
    {
        ::ucbhelper::Content ucbContent;
        bool bOk=true;

        try
        {
            bOk = create_ucb_content(
                    &ucbContent, url, xCmdEnv, false /* no throw */ )
                    && !ucbContent.isFolder();
        }
        catch (const css::ucb::ContentCreationException&)
        {
            bOk = false;
        }

        if (bOk)
        {
            OUString title( StrTitle::getTitle( ucbContent ) );
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
    if (mediaType.isEmpty())
    {
        // try ambiguous backends:
        for (auto const& ambiguousBackend : m_ambiguousBackends)
        {
            try {
                return ambiguousBackend->bindPackage( url, mediaType, bRemoved,
                    identifier, xCmdEnv );
            }
            catch (const lang::IllegalArgumentException &) {
            }
        }
        throw lang::IllegalArgumentException(
            DpResId(RID_STR_CANNOT_DETECT_MEDIA_TYPE) + url,
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
                        mediaType.subView( 0, q ) ) );
            }
        }
        if (iFind == m_mediaType2backend.end()) {
            throw lang::IllegalArgumentException(
                DpResId(RID_STR_UNSUPPORTED_MEDIA_TYPE) + mediaType,
                static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
        }
        return iFind->second->bindPackage( url, mediaType, bRemoved,
            identifier, xCmdEnv );
    }
}


Sequence< Reference<deployment::XPackageTypeInfo> >
PackageRegistryImpl::getSupportedPackageTypes()
{
    return comphelper::containerToSequence(m_typesInfos);
}
} // anon namespace


Reference<deployment::XPackageRegistry> create(
    OUString const & context,
    OUString const & cachePath,
    Reference<XComponentContext> const & xComponentContext )
{
    return PackageRegistryImpl::create(
        context, cachePath, xComponentContext );
}

} // namespace dp_registry

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
