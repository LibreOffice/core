/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

