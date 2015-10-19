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
#ifndef INCLUDED_CONNECTIVITY_FILTERMANAGER_HXX
#define INCLUDED_CONNECTIVITY_FILTERMANAGER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <rtl/ustrbuf.hxx>

#include <vector>
#include <connectivity/dbtoolsdllapi.hxx>


namespace dbtools
{



    //= FilterManager

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
        css::uno::Reference< css::beans::XPropertySet >
                                            m_xComponentAggregate;
        ::std::vector< OUString >           m_aFilterComponents;
        bool                                m_bApplyPublicFilter;

    public:
        /// ctor
        explicit FilterManager();

        /// late ctor
        void    initialize(const css::uno::Reference< css::beans::XPropertySet >& _rxComponentAggregate );

        /// makes the object forgetting the references to the database component
        void    dispose( );

        const OUString&  getFilterComponent( FilterComponent _eWhich ) const;
        void                    setFilterComponent( FilterComponent _eWhich, const OUString& _rComponent );

        inline bool     isApplyPublicFilter( ) const { return m_bApplyPublicFilter; }
               void     setApplyPublicFilter( bool _bApply );

    private:
        /** retrieves a filter which is a conjunction of all single filter components
        */
        OUString         getComposedFilter( ) const;

        /** appends one filter component to the statement in our composer
        */
        static void      appendFilterComponent( OUStringBuffer& io_appendTo, const OUString& i_component );

        /// checks whether there is only one (or even no) non-empty filter component
        bool    isThereAtMostOneComponent( OUStringBuffer& o_singleComponent ) const;

        /// returns the index of the first filter component which should be considered when building the composed filter
        inline  sal_Int32   getFirstApplicableFilterIndex() const
        {
            return m_bApplyPublicFilter ? fcPublicFilter : fcPublicFilter + 1;
        }
    };


} // namespacefrm


#endif // CONNECTIVITY_FORMFILTERMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
