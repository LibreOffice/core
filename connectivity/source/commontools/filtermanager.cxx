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
        :m_aFilterComponents( FC_COMPONENT_COUNT )
        ,m_bApplyPublicFilter( true )
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
        return m_aFilterComponents[ _eWhich ];
    }


    void FilterManager::setFilterComponent( FilterComponent _eWhich, const OUString& _rComponent )
    {
        m_aFilterComponents[ _eWhich ]  = _rComponent;
        try
        {
            if ( m_xComponentAggregate.is() && (( _eWhich != fcPublicFilter ) || m_bApplyPublicFilter ) )
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
            if ( m_xComponentAggregate.is() && !getFilterComponent( fcPublicFilter ).isEmpty() )
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
            io_appendTo.appendAscii( " ) AND " );
        }

        io_appendTo.appendAscii( "( " );
        io_appendTo.append( i_component );
        io_appendTo.appendAscii( " )" );
    }


    bool FilterManager::isThereAtMostOneComponent( OUStringBuffer& o_singleComponent ) const
    {
        sal_Int32 nOnlyNonEmpty = -1;
        sal_Int32 i;
        for ( i = getFirstApplicableFilterIndex(); i < FC_COMPONENT_COUNT; ++i )
        {
            if ( !m_aFilterComponents[ i ].isEmpty() )
            {
                if ( nOnlyNonEmpty != -1 )
                    // it's the second non-empty component
                    break;
                else
                    nOnlyNonEmpty = i;
            }
        }
        if ( nOnlyNonEmpty == -1 )
        {
            o_singleComponent.makeStringAndClear();
            return true;
        }

        if ( i == FC_COMPONENT_COUNT )
        {
            // we found only one non-empty filter component
            o_singleComponent = m_aFilterComponents[ nOnlyNonEmpty ];
            return true;
        }
        return false;
    }


    OUString FilterManager::getComposedFilter( ) const
    {
        OUStringBuffer aComposedFilter;

        // if we have only one non-empty component, then there's no need to compose anything
        if ( !isThereAtMostOneComponent( aComposedFilter ) )
        {
            // append the single components
            for ( sal_Int32 i = getFirstApplicableFilterIndex(); i < FC_COMPONENT_COUNT; ++i )
                appendFilterComponent( aComposedFilter, m_aFilterComponents[ i ] );
        }

        return aComposedFilter.makeStringAndClear();
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
