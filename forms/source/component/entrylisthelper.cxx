/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "entrylisthelper.hxx"
#include "FormComponent.hxx"

#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <comphelper/property.hxx>
#include <algorithm>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::form::binding;

    
    
    
    
    OEntryListHelper::OEntryListHelper( OControlModel& _rControlModel )
        :m_rControlModel( _rControlModel )
        ,m_aRefreshListeners( _rControlModel.getInstanceMutex() )
    {
    }

    
    OEntryListHelper::OEntryListHelper( const OEntryListHelper& _rSource, OControlModel& _rControlModel )
        :m_rControlModel( _rControlModel )
        ,m_xListSource ( _rSource.m_xListSource  )
        ,m_aStringItems( _rSource.m_aStringItems )
        ,m_aRefreshListeners( _rControlModel.getInstanceMutex() )
    {
    }

    
    OEntryListHelper::~OEntryListHelper( )
    {
    }

    
    void SAL_CALL OEntryListHelper::setListEntrySource( const Reference< XListEntrySource >& _rxSource ) throw (RuntimeException)
    {
        ControlModelLock aLock( m_rControlModel );

        
        disconnectExternalListSource();

        
        if ( _rxSource.is() )
            connectExternalListSource( _rxSource, aLock );
    }

    
    Reference< XListEntrySource > SAL_CALL OEntryListHelper::getListEntrySource(  ) throw (RuntimeException)
    {
        return m_xListSource;
    }


    
    void SAL_CALL OEntryListHelper::entryChanged( const ListEntryEvent& _rEvent ) throw (RuntimeException)
    {
        ControlModelLock aLock( m_rControlModel );

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
            stringItemListChanged( aLock );
        }
    }

    
    void SAL_CALL OEntryListHelper::entryRangeInserted( const ListEntryEvent& _rEvent ) throw (RuntimeException)
    {
        ControlModelLock aLock( m_rControlModel );

        OSL_ENSURE( _rEvent.Source == m_xListSource,
            "OEntryListHelper::entryRangeInserted: where did this come from?" );
        OSL_ENSURE( ( _rEvent.Position > 0 ) && ( _rEvent.Position < m_aStringItems.getLength() ) && ( _rEvent.Entries.getLength() > 0 ),
            "OEntryListHelper::entryRangeRemoved: invalid count and/or position!" );

        if  (   ( _rEvent.Position > 0 )
            &&  ( _rEvent.Position < m_aStringItems.getLength() )
            &&  ( _rEvent.Entries.getLength() > 0 )
            )
        {
            
            Sequence< OUString > aKeepEntries(
                m_aStringItems.getConstArray(),
                _rEvent.Position
            );
            
            Sequence< OUString > aMovedEntries(
                m_aStringItems.getConstArray() + _rEvent.Position,
                m_aStringItems.getLength() - _rEvent.Position
            );

            
            m_aStringItems = ::comphelper::concatSequences(
                aKeepEntries,
                _rEvent.Entries,
                aMovedEntries
            );

            stringItemListChanged( aLock );
        }
    }

    
    void SAL_CALL OEntryListHelper::entryRangeRemoved( const ListEntryEvent& _rEvent ) throw (RuntimeException)
    {
        ControlModelLock aLock( m_rControlModel );

        OSL_ENSURE( _rEvent.Source == m_xListSource,
            "OEntryListHelper::entryRangeRemoved: where did this come from?" );
        OSL_ENSURE( ( _rEvent.Position > 0 ) && ( _rEvent.Count > 0 ) && ( _rEvent.Position + _rEvent.Count <= m_aStringItems.getLength() ),
            "OEntryListHelper::entryRangeRemoved: invalid count and/or position!" );

        if  (   ( _rEvent.Position > 0 )
            &&  ( _rEvent.Count > 0 )
            &&  ( _rEvent.Position + _rEvent.Count <= m_aStringItems.getLength() )
            )
        {
            
            ::std::copy(
                m_aStringItems.getConstArray() + _rEvent.Position + _rEvent.Count,
                m_aStringItems.getConstArray() + m_aStringItems.getLength(),
                m_aStringItems.getArray( ) + _rEvent.Position
            );
            
            m_aStringItems.realloc( m_aStringItems.getLength() - _rEvent.Count );

            stringItemListChanged( aLock );
        }
    }

    
    void SAL_CALL OEntryListHelper::allEntriesChanged( const EventObject& _rEvent ) throw (RuntimeException)
    {
        ControlModelLock aLock( m_rControlModel );

        OSL_ENSURE( _rEvent.Source == m_xListSource,
            "OEntryListHelper::allEntriesChanged: where did this come from?" );

        Reference< XListEntrySource > xSource( _rEvent.Source, UNO_QUERY );
        if ( _rEvent.Source == m_xListSource )
        {
            impl_lock_refreshList( aLock );
        }
    }

    
    
    void SAL_CALL OEntryListHelper::addRefreshListener(const Reference<XRefreshListener>& _rxListener) throw(RuntimeException)
    {
        if ( _rxListener.is() )
            m_aRefreshListeners.addInterface( _rxListener );
    }

    
    void SAL_CALL OEntryListHelper::removeRefreshListener(const Reference<XRefreshListener>& _rxListener) throw(RuntimeException)
    {
        if ( _rxListener.is() )
            m_aRefreshListeners.removeInterface( _rxListener );
    }

    
    void SAL_CALL OEntryListHelper::refresh() throw(RuntimeException)
    {
        {
            ControlModelLock aLock( m_rControlModel );
            impl_lock_refreshList( aLock );
        }

        EventObject aEvt( static_cast< XRefreshable* >( this ) );
        m_aRefreshListeners.notifyEach( &XRefreshListener::refreshed, aEvt );
    }

    
    void OEntryListHelper::impl_lock_refreshList( ControlModelLock& _rInstanceLock )
    {
        if ( hasExternalListSource() )
        {
            m_aStringItems = m_xListSource->getAllListEntries( );
            stringItemListChanged( _rInstanceLock );
        }
        else
            refreshInternalEntryList();
    }

    
    bool OEntryListHelper::handleDisposing( const EventObject& _rEvent )
    {
        if ( m_xListSource .is() && ( _rEvent.Source == m_xListSource ) )
        {
            disconnectExternalListSource( );
            return true;
        }
        return false;
    }

    
    void OEntryListHelper::disposing( )
    {
        EventObject aEvt( static_cast< XRefreshable* >( this ) );
        m_aRefreshListeners.disposeAndClear(aEvt);

        if ( hasExternalListSource( ) )
            disconnectExternalListSource( );
    }

    
    void OEntryListHelper::disconnectExternalListSource( )
    {
        if ( m_xListSource.is() )
            m_xListSource->removeListEntryListener( this );

        m_xListSource.clear();

        disconnectedExternalListSource();
    }

    
    void OEntryListHelper::connectedExternalListSource( )
    {
        
    }

    
    void OEntryListHelper::disconnectedExternalListSource( )
    {
        
    }

    
    void OEntryListHelper::connectExternalListSource( const Reference< XListEntrySource >& _rxSource, ControlModelLock& _rInstanceLock )
    {
        OSL_ENSURE( !hasExternalListSource(), "OEntryListHelper::connectExternalListSource: only to be called if no external source is active!" );
        OSL_ENSURE( _rxSource.is(), "OEntryListHelper::connectExternalListSource: invalid list source!" );

        
        m_xListSource = _rxSource;

        
        if ( m_xListSource.is() )
        {
            
            m_xListSource->addListEntryListener( this );

            m_aStringItems = m_xListSource->getAllListEntries( );
            stringItemListChanged( _rInstanceLock );

            
            connectedExternalListSource();
        }
    }

    
    sal_Bool OEntryListHelper::convertNewListSourceProperty( Any& _rConvertedValue,
        Any& _rOldValue, const Any& _rValue ) SAL_THROW( ( IllegalArgumentException ) )
    {
        if ( hasExternalListSource() )
            throw IllegalArgumentException( );
            

        return ::comphelper::tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_aStringItems );
    }

    
    void OEntryListHelper::setNewStringItemList( const ::com::sun::star::uno::Any& _rValue, ControlModelLock& _rInstanceLock )
    {
        OSL_PRECOND( !hasExternalListSource(), "OEntryListHelper::setNewStringItemList: this should never have survived convertNewListSourceProperty!" );
        OSL_VERIFY( _rValue >>= m_aStringItems );
        stringItemListChanged( _rInstanceLock );
    }


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
