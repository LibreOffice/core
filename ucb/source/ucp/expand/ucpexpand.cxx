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


#include "rtl/uri.hxx"
#include "osl/mutex.hxx"
#include <cppuhelper/compbase.hxx>
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include <cppuhelper/supportsservice.hxx>
#include "ucbhelper/content.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/util/theMacroExpander.hpp"
#include "com/sun/star/ucb/XContentProvider.hpp"

#define EXPAND_PROTOCOL "vnd.sun.star.expand"
#define ARLEN(x) sizeof (x) / sizeof *(x)


using namespace ::com::sun::star;

namespace
{

struct MutexHolder
{
    mutable ::osl::Mutex m_mutex;
};

typedef ::cppu::WeakComponentImplHelper<
    lang::XServiceInfo, ucb::XContentProvider > t_impl_helper;


class ExpandContentProviderImpl : protected MutexHolder, public t_impl_helper
{
    uno::Reference< uno::XComponentContext > m_xComponentContext;
    uno::Reference< util::XMacroExpander >   m_xMacroExpander;
    OUString expandUri(
        uno::Reference< ucb::XContentIdentifier > const & xIdentifier ) const;

protected:
    inline void check() const;
    virtual void SAL_CALL disposing() override;

public:
    explicit ExpandContentProviderImpl(
        uno::Reference< uno::XComponentContext > const & xComponentContext )
        : t_impl_helper( m_mutex ),
          m_xComponentContext( xComponentContext ),
          m_xMacroExpander( util::theMacroExpander::get(xComponentContext) )
        {}
    virtual ~ExpandContentProviderImpl() throw ();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (uno::RuntimeException, std::exception) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException, std::exception) override;

    // XContentProvider
    virtual uno::Reference< ucb::XContent > SAL_CALL queryContent(
        uno::Reference< ucb::XContentIdentifier > const & xIdentifier )
        throw (ucb::IllegalIdentifierException, uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL compareContentIds(
        uno::Reference< ucb::XContentIdentifier > const & xId1,
        uno::Reference< ucb::XContentIdentifier > const & xId2 )
        throw (uno::RuntimeException, std::exception) override;
};


inline void ExpandContentProviderImpl::check() const
{
    // xxx todo guard?
//     MutexGuard guard( m_mutex );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            "expand content provider instance has "
            "already been disposed!",
            static_cast< OWeakObject * >(
                const_cast< ExpandContentProviderImpl * >(this) ) );
    }
}


ExpandContentProviderImpl::~ExpandContentProviderImpl() throw ()
{
}


void ExpandContentProviderImpl::disposing()
{
}


uno::Reference< uno::XInterface > SAL_CALL create(
    uno::Reference< uno::XComponentContext > const & xComponentContext )
{
    return static_cast< ::cppu::OWeakObject * >(
        new ExpandContentProviderImpl( xComponentContext ) );
}


OUString SAL_CALL implName()
{
    return OUString("com.sun.star.comp.ucb.ExpandContentProvider");
}


uno::Sequence< OUString > SAL_CALL supportedServices()
{
    OUString names [] = {
        OUString("com.sun.star.ucb.ExpandContentProvider"),
        OUString("com.sun.star.ucb.ContentProvider")
    };
    return uno::Sequence< OUString >( names, ARLEN(names) );
}

// XServiceInfo

OUString ExpandContentProviderImpl::getImplementationName()
    throw (uno::RuntimeException, std::exception)
{
    check();
    return implName();
}


uno::Sequence< OUString > ExpandContentProviderImpl::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
{
    check();
    return supportedServices();
}

sal_Bool ExpandContentProviderImpl::supportsService(OUString const & serviceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, serviceName);
}

OUString ExpandContentProviderImpl::expandUri(
    uno::Reference< ucb::XContentIdentifier > const & xIdentifier ) const
{
    OUString uri( xIdentifier->getContentIdentifier() );
    if (!uri.startsWith(EXPAND_PROTOCOL ":"))
    {
        throw ucb::IllegalIdentifierException(
            "expected protocol " EXPAND_PROTOCOL "!",
            static_cast< OWeakObject * >(
                const_cast< ExpandContentProviderImpl * >(this) ) );
    }
    // cut protocol
    OUString str( uri.copy( sizeof (EXPAND_PROTOCOL ":") -1 ) );
    // decode uric class chars
    str = ::rtl::Uri::decode(
        str, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
    // expand macro string
    return m_xMacroExpander->expandMacros( str );
}

// XContentProvider

uno::Reference< ucb::XContent > ExpandContentProviderImpl::queryContent(
    uno::Reference< ucb::XContentIdentifier > const & xIdentifier )
    throw (ucb::IllegalIdentifierException, uno::RuntimeException, std::exception)
{
    check();
    OUString uri( expandUri( xIdentifier ) );

    ::ucbhelper::Content ucb_content;
    if (::ucbhelper::Content::create(
            uri, uno::Reference< ucb::XCommandEnvironment >(),
            m_xComponentContext, ucb_content ))
    {
        return ucb_content.get();
    }
    else
    {
        return uno::Reference< ucb::XContent >();
    }
}


sal_Int32 ExpandContentProviderImpl::compareContentIds(
    uno::Reference< ucb::XContentIdentifier > const & xId1,
    uno::Reference< ucb::XContentIdentifier > const & xId2 )
    throw (uno::RuntimeException, std::exception)
{
    check();
    try
    {
        OUString uri1( expandUri( xId1 ) );
        OUString uri2( expandUri( xId2 ) );
        return uri1.compareTo( uri2 );
    }
    catch (const ucb::IllegalIdentifierException & exc)
    {
        (void) exc; // unused
        OSL_FAIL(
            OUStringToOString(
                exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        return -1;
    }
}

static const ::cppu::ImplementationEntry s_entries [] =
{
    {
        create,
        implName,
        supportedServices,
        ::cppu::createSingleComponentFactory,
        nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpexpand1_component_getFactory(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, s_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
