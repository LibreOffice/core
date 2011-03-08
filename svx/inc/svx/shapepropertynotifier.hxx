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

#ifndef SVX_PROPERTYCHANGENOTIFIER_HXX
#define SVX_PROPERTYCHANGENOTIFIER_HXX

#include "svx/svxdllapi.h"
#include "svx/shapeproperty.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
/** === end UNO includes === **/

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
        virtual ::rtl::OUString getPropertyName() const = 0;

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
            ,m_sPropertyName( ::rtl::OUString::createFromAscii( _pAsciiPropertyName ) )
        {
        }

        virtual ::rtl::OUString getPropertyName() const;
        virtual void getCurrentValue( ::com::sun::star::uno::Any& _out_rValue ) const;

    protected:
        ::cppu::OWeakObject&    getContext() const { return m_rContext; }
    private:
        ::cppu::OWeakObject&    m_rContext;
        const ::rtl::OUString   m_sPropertyName;
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
        void addPropertyChangeListener( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener );
        void removePropertyChangeListener( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener );

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
