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
#pragma once

#include <sal/config.h>

#include <atomic>
#include <cstddef>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <connectivity/sdbcx/VCollection.hxx>

namespace dbtools
{
    class WarningsContainer;
}

namespace dbaccess
{
    class IRefreshListener;

    class OFilteredContainer : public ::connectivity::sdbcx::OCollection
    {
    private:
        mutable bool m_bConstructed;        // late ctor called

    protected:
        IRefreshListener*               m_pRefreshListener;
        std::atomic<std::size_t>&       m_nInAppend;

        // holds the original container which where set in construct but they can be null
        css::uno::Reference< css::container::XNameAccess >    m_xMasterContainer;
        css::uno::WeakReference< css::sdbc::XConnection >     m_xConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData >   m_xMetaData;

        /** returns a string denoting the only type of tables allowed in this container, or an empty string
            if there is no such restriction
        */
        virtual OUString getTableTypeRestriction() const = 0;

        virtual void addMasterContainerListener(){}
        virtual void removeMasterContainerListener(){}

        // ::connectivity::sdbcx::OCollection
        virtual void impl_refresh() override;

        virtual OUString getNameForObject(const ::connectivity::sdbcx::ObjectType& _xObject) override;

        /** tell the container to free all elements and all additional resources.<BR>
            After using this method the object may be reconstructed by calling one of the <code>construct</code> methods.
        */
        virtual void disposing() override;

        class EnsureReset
        {
        public:
            EnsureReset( std::atomic<std::size_t>& _rValueLocation)
                :m_rValue( _rValueLocation )
            {
                ++m_rValue;
            }

            ~EnsureReset()
            {
                --m_rValue;
            }

        private:
            std::atomic<std::size_t>&   m_rValue;
        };

        /** retrieve a table type filter to pass to <member scope="css::sdbc">XDatabaseMetaData::getTables</member>,
            according to the current data source settings
        */
        void    getAllTableTypeFilter( css::uno::Sequence< OUString >& /* [out] */ _rFilter ) const;

    public:
        /** ctor of the container. The parent has to support the <type scope="css::sdbc">XConnection</type>
            interface.<BR>
            @param          _rParent            the object which acts as parent for the container.
                                                all refcounting is rerouted to this object
            @param          _rMutex             the access safety object of the parent
            @param          _rTableFilter       restricts the visible tables by name
            @param          _rTableTypeFilter   restricts the visible tables by type
            @see            construct
        */
        OFilteredContainer( ::cppu::OWeakObject& _rParent,
                            ::osl::Mutex& _rMutex,
                            const css::uno::Reference< css::sdbc::XConnection >& _xCon,
                            bool _bCase,
                            IRefreshListener*   _pRefreshListener,
                            std::atomic<std::size_t>& _nInAppend
                        );

        void dispose() { disposing(); }

        /** late ctor. The container will fill itself with the data got by the connection meta data, considering the
            filters given (the connection is the parent object you passed in the ctor).
        */
        void construct(
            const css::uno::Sequence< OUString >& _rTableFilter,
            const css::uno::Sequence< OUString >& _rTableTypeFilter
            );

        /** late ctor. The container will fill itself with wrapper objects for the tables returned by the given
            name container.
        */
        void construct(
            const css::uno::Reference< css::container::XNameAccess >& _rxMasterContainer,
            const css::uno::Sequence< OUString >& _rTableFilter,
            const css::uno::Sequence< OUString >& _rTableTypeFilter
            );

        bool isInitialized() const { return m_bConstructed; }
    };
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
