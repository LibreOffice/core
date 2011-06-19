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
#ifndef CONNECTIVITY_FILTERMANAGER_HXX
#define CONNECTIVITY_FILTERMANAGER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
/** === end UNO includes === **/

#include <rtl/ustrbuf.hxx>

#include <vector>
#include "connectivity/dbtoolsdllapi.hxx"

//........................................................................
namespace dbtools
{
//........................................................................

    //====================================================================
    //= FilterManager
    //====================================================================
    /** manages the filter of a database component with filter properties

        The idea is that the filter which a database component actually really uses is composed of several single
        filter components (which all are conjunctive).

        First, there is a component which is visible to the clients of the database component itself - if they ask
        the database component for the Filter property, they will get this public filter.

        Second, there is an implicit filter, which is (to be) created from the MasterFields and DetailFields
        property of the database component, if the latter denote columns.<br/>
        For instance, if there is a link-pair CustomerID->cid, where |CustomerID| is a column of the master
        database component, and |cid| is a column of the detail database component (the database component we're responsible for), then there will
        be an implicit filter "cid = :param_cid_link" (or something like this), which is never visible
        to the clients of the database component, but nevertheless needs to be propagated to the aggregated RowSet.<br/>
        Actually, this implicit filter is maintained by the FormParameterManager.

        Potentially, there could be more filter components (for instance, you could imagine database component
        controls which act as live filter, which could be implemented with a third component), but
        at the moment there are only these two.
    */
    class OOO_DLLPUBLIC_DBTOOLS FilterManager
    {
    public:
        enum FilterComponent
        {
            fcPublicFilter = 0,     // the filter which is to be published as "Filter" property of the database component
            fcLinkFilter,           // the filter part which is implicitly created for a database component when connecting
                                    // master and detail database components via column names

            FC_COMPONENT_COUNT      // boundary delimiter, not to be used from outside
        };

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                            m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                            m_xComponentAggregate;
        ::std::vector< ::rtl::OUString >    m_aFilterComponents;
        sal_Bool                            m_bApplyPublicFilter;

    public:
        /// ctor
        explicit FilterManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        /// late ctor
        void    initialize(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxComponentAggregate );

        /// makes the object forgetting the references to the database component
        void    dispose( );

        const ::rtl::OUString&  getFilterComponent( FilterComponent _eWhich ) const;
        void                    setFilterComponent( FilterComponent _eWhich, const ::rtl::OUString& _rComponent );

        inline sal_Bool isApplyPublicFilter( ) const { return m_bApplyPublicFilter; }
               void     setApplyPublicFilter( sal_Bool _bApply );

    private:
        /** retrieves a filter which is a conjunction of all single filter components
        */
        ::rtl::OUString         getComposedFilter( ) const;

        /** appends one filter component to the statement in our composer
        */
        void    appendFilterComponent( ::rtl::OUStringBuffer& io_appendTo, const ::rtl::OUString& i_component ) const;

        /// checks whether there is only one (or even no) non-empty filter component
        bool    isThereAtMostOneComponent( ::rtl::OUStringBuffer& o_singleComponent ) const;

        /// returns the index of the first filter component which should be considered when building the composed filter
        inline  sal_Int32   getFirstApplicableFilterIndex() const
        {
            return m_bApplyPublicFilter ? fcPublicFilter : fcPublicFilter + 1;
        }
    };

//........................................................................
} // namespacefrm
//........................................................................

#endif // CONNECTIVITY_FORMFILTERMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
