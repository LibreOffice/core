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

#include <connectivity/filtermanager.hxx>

#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <TConnection.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/ustrbuf.hxx>


namespace dbtools
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace connectivity;

    FilterManager::FilterManager( )
        :m_bApplyPublicFilter( true )
    {
    }


    void FilterManager::initialize( const Reference< XPropertySet >& _rxComponentAggregate )
    {
        m_xComponentAggregate = _rxComponentAggregate;
        OSL_ENSURE( m_xComponentAggregate.is(), "FilterManager::initialize: invalid arguments!" );

        if ( m_xComponentAggregate.is() )
            m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_APPLYFILTER), Any( true ) );
    }


    void FilterManager::dispose( )
    {
        m_xComponentAggregate.clear();
    }


    const OUString& FilterManager::getFilterComponent( FilterComponent _eWhich ) const
    {
        switch (_eWhich)
        {
        case FilterComponent::PublicFilter:
            return m_aPublicFilterComponent;
        case FilterComponent::PublicHaving:
            return m_aPublicHavingComponent;
        case FilterComponent::LinkFilter:
            return m_aLinkFilterComponent;
        case FilterComponent::LinkHaving:
            return m_aLinkHavingComponent;
        }
        assert(false);

        static constexpr OUString sErr(u"#FilterManager::getFilterComponent unknown component#"_ustr);
        return sErr;
    }


    void FilterManager::setFilterComponent( FilterComponent _eWhich, const OUString& _rComponent )
    {
        switch (_eWhich)
        {
        case FilterComponent::PublicFilter:
            m_aPublicFilterComponent = _rComponent;
            break;
        case FilterComponent::PublicHaving:
            m_aPublicHavingComponent = _rComponent;
            break;
        case FilterComponent::LinkFilter:
            m_aLinkFilterComponent = _rComponent;
            break;
        case FilterComponent::LinkHaving:
            m_aLinkHavingComponent = _rComponent;
            break;
        }
        try
        {
            if ( m_xComponentAggregate.is() )
            {
                bool propagate(true);
                switch (_eWhich)
                {
                case FilterComponent::PublicFilter:
                    propagate = propagate && m_bApplyPublicFilter;
                    [[fallthrough]];
                case FilterComponent::LinkFilter:
                    if (propagate)
                        m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FILTER), Any( getComposedFilter() ) );
                    break;
                case FilterComponent::PublicHaving:
                    propagate = propagate && m_bApplyPublicFilter;
                    [[fallthrough]];
                case FilterComponent::LinkHaving:
                    if (propagate)
                        m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HAVINGCLAUSE), Any( getComposedHaving() ) );
                    break;
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("connectivity.commontools");
        }
    }


    void FilterManager::setApplyPublicFilter( bool _bApply )
    {
        if ( m_bApplyPublicFilter == _bApply )
            return;

        m_bApplyPublicFilter = _bApply;

        try
        {
            if ( m_xComponentAggregate.is())
            {
                // only where/if something changed
                if (!getFilterComponent( FilterComponent::PublicFilter ).isEmpty())
                    m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FILTER), Any( getComposedFilter() ) );
                if (!getFilterComponent( FilterComponent::PublicHaving ).isEmpty())
                    m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HAVINGCLAUSE), Any( getComposedHaving() ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("connectivity.commontools");
        }
    }


    void FilterManager::appendFilterComponent( OUStringBuffer& io_appendTo, std::u16string_view i_component )
    {
        if ( !io_appendTo.isEmpty() )
        {
            io_appendTo.insert( 0, '(' );
            io_appendTo.insert( 1, ' ' );
            io_appendTo.append( " ) AND " );
        }

        io_appendTo.append( "( " );
        io_appendTo.append( i_component );
        io_appendTo.append( " )" );
    }


    bool FilterManager::isThereAtMostOneFilterComponent( OUString& o_singleComponent ) const
    {
        if (m_bApplyPublicFilter) {
            if (!m_aPublicFilterComponent.isEmpty() && !m_aLinkFilterComponent.isEmpty())
                return false;
            if (!m_aPublicFilterComponent.isEmpty())
                o_singleComponent = m_aPublicFilterComponent;
            else if (!m_aLinkFilterComponent.isEmpty())
                o_singleComponent = m_aLinkFilterComponent;
            else
                o_singleComponent.clear();
            return true;
        }
        else
        {
            if (m_aLinkFilterComponent.isEmpty())
                o_singleComponent.clear();
            else
                o_singleComponent = m_aLinkFilterComponent;
            return true;
        }
    }

    bool FilterManager::isThereAtMostOneHavingComponent( OUString& o_singleComponent ) const
    {
        if (m_bApplyPublicFilter) {
            if (!m_aPublicHavingComponent.isEmpty() && !m_aLinkHavingComponent.isEmpty())
                return false;
            if (!m_aPublicHavingComponent.isEmpty())
                o_singleComponent = m_aPublicHavingComponent;
            else if (!m_aLinkHavingComponent.isEmpty())
                o_singleComponent = m_aLinkHavingComponent;
            else
                o_singleComponent.clear();
            return true;
        }
        else
        {
            if (m_aLinkHavingComponent.isEmpty())
                o_singleComponent.clear();
            else
                o_singleComponent = m_aLinkHavingComponent;
            return true;
        }
    }


    OUString FilterManager::getComposedFilter( ) const
    {
        // if we have only one non-empty component, then there's no need to compose anything
        OUString singleComponent;
        if ( isThereAtMostOneFilterComponent( singleComponent ) )
        {
            return singleComponent;
        }
        // append the single components
        OUStringBuffer aComposedFilter(singleComponent);
        if (m_bApplyPublicFilter)
            appendFilterComponent( aComposedFilter, m_aPublicFilterComponent );
        appendFilterComponent( aComposedFilter, m_aLinkFilterComponent );
        return aComposedFilter.makeStringAndClear();
    }


    OUString FilterManager::getComposedHaving( ) const
    {
        // if we have only one non-empty component, then there's no need to compose anything
        OUString singleComponent;
        if ( isThereAtMostOneHavingComponent( singleComponent ) )
        {
            return singleComponent;
        }
        // append the single components
        OUStringBuffer aComposedFilter(singleComponent);
        if (m_bApplyPublicFilter)
            appendFilterComponent( aComposedFilter, m_aPublicHavingComponent );
        appendFilterComponent( aComposedFilter, m_aLinkHavingComponent );
        return aComposedFilter.makeStringAndClear();
    }


}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
