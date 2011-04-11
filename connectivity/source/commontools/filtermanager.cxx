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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "connectivity/filtermanager.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
/** === end UNO includes === **/
#include "TConnection.hxx"
#include <osl/diagnose.h>
#include "connectivity/dbtools.hxx"
#include <tools/diagnose_ex.h>
#include <rtl/ustrbuf.hxx>

//........................................................................
namespace dbtools
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace connectivity;

    //====================================================================
    //= FilterManager
    //====================================================================
    //--------------------------------------------------------------------
    FilterManager::FilterManager( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
        ,m_aFilterComponents( FC_COMPONENT_COUNT )
        ,m_bApplyPublicFilter( true )
    {
    }

    //--------------------------------------------------------------------
    void FilterManager::initialize( const Reference< XPropertySet >& _rxComponentAggregate )
    {
        m_xComponentAggregate = _rxComponentAggregate;
        OSL_ENSURE( m_xComponentAggregate.is(), "FilterManager::initialize: invalid arguments!" );

        if ( m_xComponentAggregate.is() )
            m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_APPLYFILTER), makeAny( (sal_Bool)sal_True ) );
    }

    //--------------------------------------------------------------------
    void FilterManager::dispose( )
    {
        m_xComponentAggregate.clear();
    }

    //--------------------------------------------------------------------
    const ::rtl::OUString& FilterManager::getFilterComponent( FilterComponent _eWhich ) const
    {
        return m_aFilterComponents[ _eWhich ];
    }

    //--------------------------------------------------------------------
    void FilterManager::setFilterComponent( FilterComponent _eWhich, const ::rtl::OUString& _rComponent )
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

    //--------------------------------------------------------------------
    void FilterManager::setApplyPublicFilter( sal_Bool _bApply )
    {
        if ( m_bApplyPublicFilter == _bApply )
            return;

        m_bApplyPublicFilter = _bApply;

        try
        {
            if ( m_xComponentAggregate.is() && getFilterComponent( fcPublicFilter ).getLength() )
            {   // only if there changed something
                m_xComponentAggregate->setPropertyValue( OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FILTER), makeAny( getComposedFilter() ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void FilterManager::appendFilterComponent( ::rtl::OUStringBuffer& io_appendTo, const ::rtl::OUString& i_component ) const
    {
        if ( io_appendTo.getLength() > 0 )
        {
            io_appendTo.insert( 0, sal_Unicode( '(' ) );
            io_appendTo.insert( 1, sal_Unicode( ' ' ) );
            io_appendTo.appendAscii( " ) AND " );
        }

        io_appendTo.appendAscii( "( " );
        io_appendTo.append( i_component );
        io_appendTo.appendAscii( " )" );
    }

    //--------------------------------------------------------------------
    bool FilterManager::isThereAtMostOneComponent( ::rtl::OUStringBuffer& o_singleComponent ) const
    {
        sal_Int32 nOnlyNonEmpty = -1;
        sal_Int32 i;
        for ( i = getFirstApplicableFilterIndex(); i < FC_COMPONENT_COUNT; ++i )
        {
            if ( m_aFilterComponents[ i ].getLength() )
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

    //--------------------------------------------------------------------
    ::rtl::OUString FilterManager::getComposedFilter( ) const
    {
        ::rtl::OUStringBuffer aComposedFilter;

        // if we have only one non-empty component, then there's no need to compose anything
        if ( !isThereAtMostOneComponent( aComposedFilter ) )
        {
            // append the single components
            for ( sal_Int32 i = getFirstApplicableFilterIndex(); i < FC_COMPONENT_COUNT; ++i )
                appendFilterComponent( aComposedFilter, m_aFilterComponents[ i ] );
        }

        return aComposedFilter.makeStringAndClear();
    }

//........................................................................
}   // namespace dbtools
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
