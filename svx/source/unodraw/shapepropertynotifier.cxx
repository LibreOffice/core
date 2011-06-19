/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
#include "precompiled_svx.hxx"

#include "svx/shapepropertynotifier.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

#include <comphelper/stl_types.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/diagnose_ex.h>

#include <boost/unordered_map.hpp>

namespace
{

    struct ShapePropertyHash
    {
        size_t operator()( ::svx::ShapeProperty __x ) const
        {
            return size_t( __x );
        }
    };
}

//........................................................................
namespace svx
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::beans::XPropertySet;
    /** === end UNO using === **/

    typedef ::boost::unordered_map< ShapeProperty, PPropertyValueProvider, ShapePropertyHash  >    PropertyProviders;

    typedef ::cppu::OMultiTypeInterfaceContainerHelperVar   <   ::rtl::OUString
                                                            ,   ::comphelper::UStringHash
                                                            ,   ::comphelper::UStringEqual
                                                            >   PropertyChangeListenerContainer;

    //====================================================================
    //= IPropertyValueProvider
    //====================================================================
    IPropertyValueProvider::~IPropertyValueProvider()
    {
    }

    //====================================================================
    //= PropertyChangeNotifier_Data
    //====================================================================
    struct PropertyChangeNotifier_Data
    {
        ::cppu::OWeakObject&            m_rContext;
        PropertyProviders               m_aProviders;
        PropertyChangeListenerContainer m_aPropertyChangeListeners;

        PropertyChangeNotifier_Data( ::cppu::OWeakObject& _rContext, ::osl::Mutex& _rMutex )
            :m_rContext( _rContext )
            ,m_aPropertyChangeListeners( _rMutex )
        {
        }
    };
    //====================================================================
    //= PropertyValueProvider
    //====================================================================
    //--------------------------------------------------------------------
    ::rtl::OUString PropertyValueProvider::getPropertyName() const
    {
        return m_sPropertyName;
    }

    //--------------------------------------------------------------------
    void PropertyValueProvider::getCurrentValue( Any& _out_rValue ) const
    {
        Reference< XPropertySet > xContextProps( const_cast< PropertyValueProvider* >( this )->m_rContext, UNO_QUERY_THROW );
        _out_rValue = xContextProps->getPropertyValue( getPropertyName() );
    }

    //====================================================================
    //= PropertyChangeNotifier
    //====================================================================
    //--------------------------------------------------------------------
    PropertyChangeNotifier::PropertyChangeNotifier( ::cppu::OWeakObject& _rOwner, ::osl::Mutex& _rMutex )
        :m_pData( new PropertyChangeNotifier_Data( _rOwner, _rMutex ) )
    {
    }

    //--------------------------------------------------------------------
    PropertyChangeNotifier::~PropertyChangeNotifier()
    {
    }

    //--------------------------------------------------------------------
    void PropertyChangeNotifier::registerProvider( const ShapeProperty _eProperty, const PPropertyValueProvider _pProvider )
    {
        ENSURE_OR_THROW( _eProperty != eInvalidShapeProperty, "Illegal ShapeProperty value!" );
        ENSURE_OR_THROW( !!_pProvider, "NULL factory not allowed." );

        OSL_ENSURE( m_pData->m_aProviders.find( _eProperty ) == m_pData->m_aProviders.end(),
            "PropertyChangeNotifier::registerProvider: factory for this ID already present!" );

        m_pData->m_aProviders[ _eProperty ] = _pProvider;
    }

    //--------------------------------------------------------------------
    void PropertyChangeNotifier::notifyPropertyChange( const ShapeProperty _eProperty ) const
    {
        ENSURE_OR_THROW( _eProperty != eInvalidShapeProperty, "Illegal ShapeProperty value!" );

        PropertyProviders::const_iterator provPos = m_pData->m_aProviders.find( _eProperty );
        OSL_ENSURE( provPos != m_pData->m_aProviders.end(), "PropertyChangeNotifier::notifyPropertyChange: no factory!" );
        if ( provPos == m_pData->m_aProviders.end() )
            return;

        ::rtl::OUString sPropertyName( provPos->second->getPropertyName() );

        ::cppu::OInterfaceContainerHelper* pPropListeners = m_pData->m_aPropertyChangeListeners.getContainer( sPropertyName );
        ::cppu::OInterfaceContainerHelper* pAllListeners = m_pData->m_aPropertyChangeListeners.getContainer( ::rtl::OUString() );
        if ( !pPropListeners && !pAllListeners )
            return;

        try
        {
            PropertyChangeEvent aEvent;
            aEvent.Source = m_pData->m_rContext;
            // Handle/OldValue not supported
            aEvent.PropertyName = provPos->second->getPropertyName();
            provPos->second->getCurrentValue( aEvent.NewValue );

            if ( pPropListeners )
                pPropListeners->notifyEach( &XPropertyChangeListener::propertyChange, aEvent );
            if ( pAllListeners )
                pAllListeners->notifyEach( &XPropertyChangeListener::propertyChange, aEvent );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void PropertyChangeNotifier::addPropertyChangeListener( const ::rtl::OUString& _rPropertyName, const Reference< XPropertyChangeListener >& _rxListener )
    {
        m_pData->m_aPropertyChangeListeners.addInterface( _rPropertyName, _rxListener );
    }

    //--------------------------------------------------------------------
    void PropertyChangeNotifier::removePropertyChangeListener( const ::rtl::OUString& _rPropertyName, const Reference< XPropertyChangeListener >& _rxListener )
    {
        m_pData->m_aPropertyChangeListeners.removeInterface( _rPropertyName, _rxListener );
    }

    //--------------------------------------------------------------------
    void PropertyChangeNotifier::disposing()
    {
        EventObject aEvent;
        aEvent.Source = m_pData->m_rContext;
        m_pData->m_aPropertyChangeListeners.disposeAndClear( aEvent );
    }

//........................................................................
} // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
