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
#ifndef DBACCESS_CORE_FILTERED_CONTAINER_HXX
#define DBACCESS_CORE_FILTERED_CONTAINER_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <connectivity/sdbcx/VCollection.hxx>

namespace dbtools
{
    class IWarningsContainer;
}

namespace dbaccess
{
    class IRefreshListener;

    class OFilteredContainer : public ::connectivity::sdbcx::OCollection
    {
    private:
        mutable sal_Bool m_bConstructed;        // late ctor called

    protected:
        ::dbtools::IWarningsContainer*  m_pWarningsContainer;
        IRefreshListener*               m_pRefreshListener;
        oslInterlockedCount&            m_nInAppend;

        // holds the original container which where set in construct but they can be null
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xMasterContainer;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection >     m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xMetaData;

        /** returns a string denoting the only type of tables allowed in this container, or an empty string
            if there is no such restriction
        */
        virtual ::rtl::OUString getTableTypeRestriction() const = 0;

        inline virtual void addMasterContainerListener(){}
        inline virtual void removeMasterContainerListener(){}

        // ::connectivity::sdbcx::OCollection
        virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString getNameForObject(const ::connectivity::sdbcx::ObjectType& _xObject);

        /** tell the container to free all elements and all additional resources.<BR>
            After using this method the object may be reconstructed by calling one of the <code>constrcuct</code> methods.
        */
        virtual void SAL_CALL disposing();

        class EnsureReset
        {
        public:
            EnsureReset( oslInterlockedCount& _rValueLocation)
                :m_rValue( _rValueLocation )
            {
                osl_incrementInterlockedCount(&m_rValue);
            }

            ~EnsureReset()
            {
                osl_decrementInterlockedCount(&m_rValue);
            }

        private:
            oslInterlockedCount&   m_rValue;
        };

        /** retrieve a table type filter to pass to <member scope="com::sun::star::sdbc">XDatabaseMetaData::getTables</member>,
            according to the current data source settings
        */
        void    getAllTableTypeFilter( ::com::sun::star::uno::Sequence< ::rtl::OUString >& /* [out] */ _rFilter ) const;

    public:
        /** ctor of the container. The parent has to support the <type scope="com::sun::star::sdbc">XConnection</type>
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
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xCon,
                            sal_Bool _bCase,
                            IRefreshListener*   _pRefreshListener,
                            ::dbtools::IWarningsContainer* _pWarningsContainer,
                            oslInterlockedCount& _nInAppend
                        );

        inline void dispose() { disposing(); }

        /** late ctor. The container will fill itself with the data got by the connection meta data, considering the
            filters given (the connection is the parent object you passed in the ctor).
        */
        void construct(
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter
            );

        /** late ctor. The container will fill itself with wrapper objects for the tables returned by the given
            name container.
        */
        void construct(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxMasterContainer,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableFilter,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTableTypeFilter
            );

        inline sal_Bool isInitialized() const { return m_bConstructed; }
    };
// ..............................................................................
} // namespace
// ..............................................................................

#endif // DBACCESS_CORE_FILTERED_CONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
