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


#include <svx/shapepropertynotifier.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/diagnose_ex.h>

#include <unordered_map>

namespace
{

}


namespace svx
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::beans::XPropertySet;

    IPropertyValueProvider::~IPropertyValueProvider()
    {
    }

    //= PropertyValueProvider


    OUString PropertyValueProvider::getPropertyName() const
    {
        return m_sPropertyName;
    }


    void PropertyValueProvider::getCurrentValue( Any& _out_rValue ) const
    {
        Reference< XPropertySet > xContextProps( const_cast< PropertyValueProvider* >( this )->m_rContext, UNO_QUERY_THROW );
        _out_rValue = xContextProps->getPropertyValue( getPropertyName() );
    }

    PropertyChangeNotifier::PropertyChangeNotifier( ::cppu::OWeakObject& _rOwner, ::osl::Mutex& _rMutex )
        :m_rContext( _rOwner )
        ,m_aPropertyChangeListeners( _rMutex )
    {
    }

    PropertyChangeNotifier::~PropertyChangeNotifier()
    {
    }

    void PropertyChangeNotifier::registerProvider(const ShapeProperty _eProperty, const std::shared_ptr<IPropertyValueProvider>& _rProvider)
    {
        ENSURE_OR_THROW( !!_rProvider, "NULL factory not allowed." );

        OSL_ENSURE( m_aProviders.find( _eProperty ) == m_aProviders.end(),
            "PropertyChangeNotifier::registerProvider: factory for this ID already present!" );

        m_aProviders[ _eProperty ] = _rProvider;
    }

    void PropertyChangeNotifier::notifyPropertyChange( const ShapeProperty _eProperty ) const
    {
        PropertyProviders::const_iterator provPos = m_aProviders.find( _eProperty );
        OSL_ENSURE( provPos != m_aProviders.end(), "PropertyChangeNotifier::notifyPropertyChange: no factory!" );
        if ( provPos == m_aProviders.end() )
            return;

        OUString sPropertyName( provPos->second->getPropertyName() );

        ::cppu::OInterfaceContainerHelper* pPropListeners = m_aPropertyChangeListeners.getContainer( sPropertyName );
        ::cppu::OInterfaceContainerHelper* pAllListeners = m_aPropertyChangeListeners.getContainer( OUString() );
        if ( !pPropListeners && !pAllListeners )
            return;

        try
        {
            PropertyChangeEvent aEvent;
            aEvent.Source = m_rContext;
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
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }


    void PropertyChangeNotifier::addPropertyChangeListener( const OUString& _rPropertyName, const Reference< XPropertyChangeListener >& _rxListener )
    {
        m_aPropertyChangeListeners.addInterface( _rPropertyName, _rxListener );
    }


    void PropertyChangeNotifier::removePropertyChangeListener( const OUString& _rPropertyName, const Reference< XPropertyChangeListener >& _rxListener )
    {
        m_aPropertyChangeListeners.removeInterface( _rPropertyName, _rxListener );
    }


    void PropertyChangeNotifier::disposing()
    {
        EventObject aEvent;
        aEvent.Source = m_rContext;
        m_aPropertyChangeListeners.disposeAndClear( aEvent );
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
