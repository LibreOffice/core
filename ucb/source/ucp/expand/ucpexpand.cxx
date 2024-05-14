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


#include <rtl/uri.hxx>
#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <comphelper/diagnose_ex.hxx>


using namespace ::com::sun::star;

namespace
{

typedef comphelper::WeakComponentImplHelper<
    lang::XServiceInfo, ucb::XContentProvider > t_impl_helper;


class ExpandContentProviderImpl : public t_impl_helper
{
    uno::Reference< uno::XComponentContext > m_xComponentContext;
    uno::Reference< util::XMacroExpander >   m_xMacroExpander;
    OUString expandUri(
        uno::Reference< ucb::XContentIdentifier > const & xIdentifier ) const;

protected:
    void check() const;

public:
    explicit ExpandContentProviderImpl(
        uno::Reference< uno::XComponentContext > const & xComponentContext )
        : m_xComponentContext( xComponentContext ),
          m_xMacroExpander( util::theMacroExpander::get(xComponentContext) )
        {}

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XContentProvider
    virtual uno::Reference< ucb::XContent > SAL_CALL queryContent(
        uno::Reference< ucb::XContentIdentifier > const & xIdentifier ) override;
    virtual sal_Int32 SAL_CALL compareContentIds(
        uno::Reference< ucb::XContentIdentifier > const & xId1,
        uno::Reference< ucb::XContentIdentifier > const & xId2 ) override;
};


void ExpandContentProviderImpl::check() const
{
    // xxx todo guard?
//     MutexGuard guard( m_mutex );
    if (m_bDisposed)
    {
        throw lang::DisposedException(
            u"expand content provider instance has "
            "already been disposed!"_ustr,
            const_cast< ExpandContentProviderImpl * >(this)->getXWeak() );
    }
}

// XServiceInfo

OUString ExpandContentProviderImpl::getImplementationName()
{
    check();
    return u"com.sun.star.comp.ucb.ExpandContentProvider"_ustr;
}


uno::Sequence< OUString > ExpandContentProviderImpl::getSupportedServiceNames()
{
    check();
    return {
        u"com.sun.star.ucb.ExpandContentProvider"_ustr,
        u"com.sun.star.ucb.ContentProvider"_ustr
    };
}

sal_Bool ExpandContentProviderImpl::supportsService(OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

OUString ExpandContentProviderImpl::expandUri(
    uno::Reference< ucb::XContentIdentifier > const & xIdentifier ) const
{
    OUString uri( xIdentifier->getContentIdentifier() );
    if (!uri.startsWithIgnoreAsciiCase("vnd.sun.star.expand:", &uri))
    {
        throw ucb::IllegalIdentifierException(
            u"expected protocol vnd.sun.star.expand!"_ustr,
            const_cast< ExpandContentProviderImpl * >(this)->getXWeak() );
    }
    // decode uric class chars
    OUString str = ::rtl::Uri::decode(uri, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
    // expand macro string
    return m_xMacroExpander->expandMacros( str );
}

// XContentProvider

uno::Reference< ucb::XContent > ExpandContentProviderImpl::queryContent(
    uno::Reference< ucb::XContentIdentifier > const & xIdentifier )
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
{
    check();
    try
    {
        OUString uri1( expandUri( xId1 ) );
        OUString uri2( expandUri( xId2 ) );
        return uri1.compareTo( uri2 );
    }
    catch (const ucb::IllegalIdentifierException &)
    {
        TOOLS_WARN_EXCEPTION( "ucb", "" );
        return -1;
    }
}


}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_expand_ExpandContentProviderImpl_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ExpandContentProviderImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
