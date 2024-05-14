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

#include <sal/log.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include "gio_provider.hxx"
#include "gio_content.hxx"

namespace gio
{
css::uno::Reference< css::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
{
    SAL_INFO("ucb.ucp.gio", "QueryContent: " << Identifier->getContentIdentifier());
    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    css::uno::Reference< css::ucb::XContent > xContent = queryExistingContent( Identifier );
    if ( xContent.is() )
        return xContent;

    try
    {
        xContent = new ::gio::Content(m_xContext, this, Identifier);
    }
    catch ( css::ucb::ContentCreationException const & )
    {
        throw css::ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw css::ucb::IllegalIdentifierException();

    return xContent;
}

ContentProvider::ContentProvider(
    const css::uno::Reference< css::uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext )
{
}

ContentProvider::~ContentProvider()
{
}

// XInterface
void SAL_CALL ContentProvider::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProvider::release()
    noexcept
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentProvider::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< css::lang::XTypeProvider* >(this),
                                               static_cast< css::lang::XServiceInfo* >(this),
                                               static_cast< css::ucb::XContentProvider* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      css::lang::XTypeProvider,
                      css::lang::XServiceInfo,
                      css::ucb::XContentProvider );

css::uno::Sequence< OUString > SAL_CALL ContentProvider::getSupportedServiceNames()
{
    return { u"com.sun.star.ucb.GIOContentProvider"_ustr };
}

OUString SAL_CALL ContentProvider::getImplementationName()
{
    return u"com.sun.star.comp.GIOContentProvider"_ustr;
}

sal_Bool SAL_CALL ContentProvider::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}


}

// gio creates threads we don't want in online's forkit
static bool isDisabled()
{
    const char *pDisable = getenv("UNODISABLELIBRARY");
    if (!pDisable)
        return false;
    OString aDisable(pDisable, strlen(pDisable));
    return aDisable.indexOf("ucpgio1") >= 0;
}



extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_gio_ContentProvider_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    if (isDisabled())
        return nullptr;
#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
#endif
    return cppu::acquire(new gio::ContentProvider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
