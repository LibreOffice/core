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

#ifndef SVX_PROPERTYCHANGENOTIFIER_HXX
#define SVX_PROPERTYCHANGENOTIFIER_HXX

#include "svx/svxdllapi.h"
#include "svx/shapeproperty.hxx"

#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>

namespace cppu
{
    class OWeakObject;
}

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= IPropertyValueProvider
    //====================================================================
    /** a provider for a property value
    */
    class SVX_DLLPUBLIC IPropertyValueProvider
    {
    public:
        /** returns the name of the property which this provider is responsible for
        */
        virtual OUString getPropertyName() const = 0;

        /** returns the current value of the property which the provider is responsible for
        */
        virtual void getCurrentValue( ::com::sun::star::uno::Any& _out_rValue ) const = 0;

        virtual ~IPropertyValueProvider();
    };
    typedef ::boost::shared_ptr< IPropertyValueProvider >  PPropertyValueProvider;

    //====================================================================
    //= PropertyValueProvider
    //====================================================================
    /** default implementation of a IPropertyValueProvider

        This default implementation queries the object which it is constructed with for the XPropertySet interface,
        and calls the getPropertyValue method.
    */
    class SVX_DLLPUBLIC PropertyValueProvider   :public IPropertyValueProvider
                                                ,public ::boost::noncopyable
    {
    public:
        PropertyValueProvider( ::cppu::OWeakObject& _rContext, const sal_Char* _pAsciiPropertyName )
            :m_rContext( _rContext )
            ,m_sPropertyName( OUString::createFromAscii( _pAsciiPropertyName ) )
        {
        }

        virtual OUString getPropertyName() const;
        virtual void getCurrentValue( ::com::sun::star::uno::Any& _out_rValue ) const;

    protected:
        ::cppu::OWeakObject&    getContext() const { return m_rContext; }
    private:
        ::cppu::OWeakObject&    m_rContext;
        const OUString   m_sPropertyName;
    };

    //====================================================================
    //= PropertyChangeNotifier
    //====================================================================
    struct PropertyChangeNotifier_Data;

    /** helper class for notifying XPropertyChangeListeners

        The class is intended to be held as member of the class which does the property change broadcasting.
    */
    class SVX_DLLPUBLIC PropertyChangeNotifier : public ::boost::noncopyable
    {
    public:
        /** constructs a notifier instance

            @param _rOwner
                the owner instance of the notifier. Will be used as css.lang.EventObject.Source when
                notifying events.
        */
        PropertyChangeNotifier( ::cppu::OWeakObject& _rOwner, ::osl::Mutex& _rMutex );
        ~PropertyChangeNotifier();

        // listener maintanance
        void addPropertyChangeListener( const OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener );
        void removePropertyChangeListener( const OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener );

        /** registers a IPropertyValueProvider
        */
        void    registerProvider( const ShapeProperty _eProperty, const PPropertyValueProvider _pProvider );

        /** notifies changes in the given property to all registered listeners

            If no property value provider for the given property ID is registered, this is worth an assertion in a
            non-product build, and otherwise ignored.
        */
        void    notifyPropertyChange( const ShapeProperty _eProperty ) const;

        /** is called to dispose the instance
        */
        void    disposing();

    private:
        ::std::auto_ptr< PropertyChangeNotifier_Data >  m_pData;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_PROPERTYCHANGENOTIFIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
