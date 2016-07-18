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

#ifndef INCLUDED_UCB_SOURCE_CORE_PROVIDERMAP_HXX
#define INCLUDED_UCB_SOURCE_CORE_PROVIDERMAP_HXX

#include <list>
#include <com/sun/star/uno/Reference.h>
#include <regexpmap.hxx>

namespace com { namespace sun { namespace star { namespace ucb {
    class XContentProvider;
} } } }


class ProviderListEntry_Impl
{
    css::uno::Reference<
        css::ucb::XContentProvider > m_xProvider;
    mutable css::uno::Reference<
        css::ucb::XContentProvider > m_xResolvedProvider;

private:
    css::uno::Reference< css::ucb::XContentProvider > const & resolveProvider() const;

public:
    explicit ProviderListEntry_Impl(
        const css::uno::Reference< css::ucb::XContentProvider >& xProvider )
    : m_xProvider( xProvider ) {}

    const css::uno::Reference< css::ucb::XContentProvider >& getProvider() const
    { return m_xProvider; }
    inline css::uno::Reference< css::ucb::XContentProvider > getResolvedProvider() const;
};

inline css::uno::Reference< css::ucb::XContentProvider >
ProviderListEntry_Impl::getResolvedProvider() const
{
    return m_xResolvedProvider.is() ? m_xResolvedProvider : resolveProvider();
}


typedef std::list< ProviderListEntry_Impl > ProviderList_Impl;


typedef ucb_impl::RegexpMap< ProviderList_Impl > ProviderMap_Impl;

#endif // INCLUDED_UCB_SOURCE_CORE_PROVIDERMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
