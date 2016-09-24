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
#include "TConnection.hxx"
#include <osl/diagnose.h>
#include <connectivity/dbtools.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>


namespace dbtools
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::lang;
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
            m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_APPLYFILTER), makeAny( true ) );
    }


    void FilterManager::dispose( )
    {
        m_xComponentAggregate.clear();
    }


    const OUString& FilterManager::getFilterComponent( FilterComponent _eWhich ) const
    {
        return _eWhich == FilterComponent::PublicFilter ? m_aPublicFilterComponent : m_aLinkFilterComponent;
    }


    void FilterManager::setFilterComponent( FilterComponent _eWhich, const OUString& _rComponent )
    {
        if (_eWhich == FilterComponent::PublicFilter)
            m_aPublicFilterComponent = _rComponent;
        else
            m_aLinkFilterComponent = _rComponent;
        try
        {
            if ( m_xComponentAggregate.is() && (( _eWhich != FilterComponent::PublicFilter ) || m_bApplyPublicFilter ) )
                m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FILTER), makeAny( getComposedFilter() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void FilterManager::setApplyPublicFilter( bool _bApply )
    {
        if ( m_bApplyPublicFilter == _bApply )
            return;

        m_bApplyPublicFilter = _bApply;

        try
        {
            if ( m_xComponentAggregate.is() && !getFilterComponent( FilterComponent::PublicFilter ).isEmpty() )
            {   // only if there changed something
                m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FILTER), makeAny( getComposedFilter() ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void FilterManager::appendFilterComponent( OUStringBuffer& io_appendTo, const OUString& i_component )
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


    bool FilterManager::isThereAtMostOneComponent( OUStringBuffer& o_singleComponent ) const
    {
        if (m_bApplyPublicFilter) {
            if (!m_aPublicFilterComponent.isEmpty() && !m_aLinkFilterComponent.isEmpty())
                return false;
            if (!m_aPublicFilterComponent.isEmpty())
                o_singleComponent = m_aPublicFilterComponent;
            else if (!m_aLinkFilterComponent.isEmpty())
                o_singleComponent = m_aLinkFilterComponent;
            else
                o_singleComponent.makeStringAndClear();
            return true;
        }
        else
        {
            if (m_aLinkFilterComponent.isEmpty())
                o_singleComponent.makeStringAndClear();
            else
                o_singleComponent = m_aLinkFilterComponent;
            return true;
        }
    }


    OUString FilterManager::getComposedFilter( ) const
    {
        OUStringBuffer aComposedFilter;

        // if we have only one non-empty component, then there's no need to compose anything
        if ( !isThereAtMostOneComponent( aComposedFilter ) )
        {
            // append the single components
            if (m_bApplyPublicFilter)
                appendFilterComponent( aComposedFilter, m_aPublicFilterComponent );
            appendFilterComponent( aComposedFilter, m_aLinkFilterComponent );
        }

        return aComposedFilter.makeStringAndClear();
    }


}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
