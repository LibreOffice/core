/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: entrylisthelper.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_forms.hxx"
#include "entrylisthelper.hxx"
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <comphelper/property.hxx>
#include <algorithm>

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form::binding;

    //=====================================================================
    //= OEntryListHelper
    //=====================================================================
    //---------------------------------------------------------------------
    OEntryListHelper::OEntryListHelper( ::osl::Mutex& _rMutex )
        :m_rMutex( _rMutex )
    {
    }

    //---------------------------------------------------------------------
    OEntryListHelper::OEntryListHelper( const OEntryListHelper& _rSource, ::osl::Mutex& _rMutex )
        :m_rMutex( _rMutex )
        ,m_xListSource ( _rSource.m_xListSource  )
        ,m_aStringItems( _rSource.m_aStringItems )
    {
    }

    //---------------------------------------------------------------------
    OEntryListHelper::~OEntryListHelper( )
    {
    }

    //---------------------------------------------------------------------
    void SAL_CALL OEntryListHelper::setListEntrySource( const Reference< XListEntrySource >& _rxSource ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );

        // disconnect from the current external list source
        disconnectExternalListSource();

        // and connect to the new one
        if ( _rxSource.is() )
            connectExternalListSource( _rxSource );
    }

    //---------------------------------------------------------------------
    Reference< XListEntrySource > SAL_CALL OEntryListHelper::getListEntrySource(  ) throw (RuntimeException)
    {
        return m_xListSource;
    }


    //---------------------------------------------------------------------
    void SAL_CALL OEntryListHelper::entryChanged( const ListEntryEvent& _rEvent ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        OSL_ENSURE( _rEvent.Source == m_xListSource,
            "OEntryListHelper::entryChanged: where did this come from?" );
        OSL_ENSURE( ( _rEvent.Position >= 0 ) && ( _rEvent.Position < m_aStringItems.getLength() ),
            "OEntryListHelper::entryChanged: invalid index!" );
        OSL_ENSURE( _rEvent.Entries.getLength() == 1,
            "OEntryListHelper::entryChanged: invalid string list!" );

        if  (   ( _rEvent.Position >= 0 )
            &&  ( _rEvent.Position < m_aStringItems.getLength() )
            &&  ( _rEvent.Entries.getLength() > 0 )
            )
        {
            m_aStringItems[ _rEvent.Position ] = _rEvent.Entries[ 0 ];
            stringItemListChanged();
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OEntryListHelper::entryRangeInserted( const ListEntryEvent& _rEvent ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        OSL_ENSURE( _rEvent.Source == m_xListSource,
            "OEntryListHelper::entryRangeInserted: where did this come from?" );
        OSL_ENSURE( ( _rEvent.Position > 0 ) && ( _rEvent.Position < m_aStringItems.getLength() ) && ( _rEvent.Entries.getLength() > 0 ),
            "OEntryListHelper::entryRangeRemoved: invalid count and/or position!" );

        if  (   ( _rEvent.Position > 0 )
            &&  ( _rEvent.Position < m_aStringItems.getLength() )
            &&  ( _rEvent.Entries.getLength() > 0 )
            )
        {
            // the entries *before* the insertion pos
            Sequence< ::rtl::OUString > aKeepEntries(
                m_aStringItems.getConstArray(),
                _rEvent.Position
            );
            // the entries *behind* the insertion pos
            Sequence< ::rtl::OUString > aMovedEntries(
                m_aStringItems.getConstArray() + _rEvent.Position,
                m_aStringItems.getLength() - _rEvent.Position
            );

            // concat all three parts
            m_aStringItems = ::comphelper::concatSequences(
                aKeepEntries,
                _rEvent.Entries,
                aMovedEntries
            );

            stringItemListChanged();
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OEntryListHelper::entryRangeRemoved( const ListEntryEvent& _rEvent ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        OSL_ENSURE( _rEvent.Source == m_xListSource,
            "OEntryListHelper::entryRangeRemoved: where did this come from?" );
        OSL_ENSURE( ( _rEvent.Position > 0 ) && ( _rEvent.Count > 0 ) && ( _rEvent.Position + _rEvent.Count <= m_aStringItems.getLength() ),
            "OEntryListHelper::entryRangeRemoved: invalid count and/or position!" );

        if  (   ( _rEvent.Position > 0 )
            &&  ( _rEvent.Count > 0 )
            &&  ( _rEvent.Position + _rEvent.Count <= m_aStringItems.getLength() )
            )
        {
            // copy all items after the removed ones
            ::std::copy(
                m_aStringItems.getConstArray() + _rEvent.Position + _rEvent.Count,
                m_aStringItems.getConstArray() + m_aStringItems.getLength(),
                m_aStringItems.getArray( ) + _rEvent.Position
            );
            // shrink the array
            m_aStringItems.realloc( m_aStringItems.getLength() - _rEvent.Count );

            stringItemListChanged();
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OEntryListHelper::allEntriesChanged( const EventObject& _rEvent ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        OSL_ENSURE( _rEvent.Source == m_xListSource,
            "OEntryListHelper::allEntriesChanged: where did this come from?" );

        Reference< XListEntrySource > xSource( _rEvent.Source, UNO_QUERY );
        if ( xSource.is() )
        {
            m_aStringItems = xSource->getAllListEntries( );
            stringItemListChanged();
        }
    }

    //---------------------------------------------------------------------
    bool OEntryListHelper::handleDisposing( const EventObject& _rEvent )
    {
        if ( m_xListSource .is() && ( _rEvent.Source == m_xListSource ) )
        {
            disconnectExternalListSource( );
            return true;
        }
        return false;
    }

    //---------------------------------------------------------------------
    void OEntryListHelper::disposing( )
    {
        if ( hasExternalListSource( ) )
            disconnectExternalListSource( );
    }

    //---------------------------------------------------------------------
    void OEntryListHelper::disconnectExternalListSource( )
    {
        if ( m_xListSource.is() )
            m_xListSource->removeListEntryListener( this );

        m_xListSource.clear();

        disconnectedExternalListSource();
    }

    //---------------------------------------------------------------------
    void OEntryListHelper::connectedExternalListSource( )
    {
        // nothing to do here
    }

    //---------------------------------------------------------------------
    void OEntryListHelper::disconnectedExternalListSource( )
    {
        // nothing to do here
    }

    //---------------------------------------------------------------------
    void OEntryListHelper::connectExternalListSource( const Reference< XListEntrySource >& _rxSource )
    {
        OSL_ENSURE( !hasExternalListSource(), "OEntryListHelper::connectExternalListSource: only to be called if no external source is active!" );
        OSL_ENSURE( _rxSource.is(), "OEntryListHelper::connectExternalListSource: invalid list source!" );

        // remember it
        m_xListSource = _rxSource;

        // initially fill our item list
        if ( m_xListSource.is() )
        {
            // be notified when the list changes ...
            m_xListSource->addListEntryListener( this );

            m_aStringItems = m_xListSource->getAllListEntries( );
            stringItemListChanged();

            // let derivees react on the new list source
            connectedExternalListSource();
        }
    }

    //---------------------------------------------------------------------
    sal_Bool OEntryListHelper::convertNewListSourceProperty( Any& _rConvertedValue,
        Any& _rOldValue, const Any& _rValue ) SAL_THROW( ( IllegalArgumentException ) )
    {
        if ( hasExternalListSource() )
            throw IllegalArgumentException( );
            // TODO: error message

        return ::comphelper::tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aStringItems );
    }

    //---------------------------------------------------------------------
    void OEntryListHelper::setNewStringItemList( const ::com::sun::star::uno::Any& _rValue )
    {
        OSL_PRECOND( !hasExternalListSource(), "OEntryListHelper::setNewStringItemList: this should never have survived convertNewListSourceProperty!" );
        OSL_VERIFY( _rValue >>= m_aStringItems );
        stringItemListChanged( );
    }

//.........................................................................
}   // namespace frm
//.........................................................................
