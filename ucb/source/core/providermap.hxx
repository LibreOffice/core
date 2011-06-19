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

#ifndef _UCB_PROVIDERMAP_HXX_
#define _UCB_PROVIDERMAP_HXX_

#include <list>
#include <com/sun/star/uno/Reference.h>
#include <regexpmap.hxx>

namespace com { namespace sun { namespace star { namespace ucb {
    class XContentProvider;
} } } }

//============================================================================
class ProviderListEntry_Impl
{
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > m_xProvider;
    mutable com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > m_xResolvedProvider;

private:
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > resolveProvider() const;

public:
    ProviderListEntry_Impl(
        const com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProvider >& xProvider )
    : m_xProvider( xProvider ) {}

    com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > getProvider() const
    { return m_xProvider; }
    inline com::sun::star::uno::Reference<
        com::sun::star::ucb::XContentProvider > getResolvedProvider() const;
};

inline com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
ProviderListEntry_Impl::getResolvedProvider() const
{
    return m_xResolvedProvider.is() ? m_xResolvedProvider : resolveProvider();
}

//============================================================================
typedef std::list< ProviderListEntry_Impl > ProviderList_Impl;

//============================================================================
typedef ucb_impl::RegexpMap< ProviderList_Impl > ProviderMap_Impl;

#endif // _UCB_PROVIDERMAP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
