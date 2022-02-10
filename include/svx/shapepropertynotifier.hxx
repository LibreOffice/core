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

#ifndef INCLUDED_SVX_SHAPEPROPERTYNOTIFIER_HXX
#define INCLUDED_SVX_SHAPEPROPERTYNOTIFIER_HXX

#include <svx/svxdllapi.h>
#include <comphelper/multiinterfacecontainer3.hxx>
#include <rtl/ustring.hxx>
#include <o3tl/enumarray.hxx>

#include <memory>
#include <unordered_map>

namespace com::sun::star::beans { class XPropertyChangeListener; }
namespace com::sun::star::uno { class Any; }
namespace com::sun::star::uno { template <typename > class Reference; }
namespace osl { class Mutex; }

namespace cppu
{
    class OWeakObject;
}


namespace svx
{

    //= ShapeProperty

    enum class ShapePropertyProviderId
    {
        // generic (UNO) shape properties
        Position,
        Size,
        // text doc shape properties
        TextDocAnchor,
        LAST = TextDocAnchor
    };

    //= PropertyValueProvider

    /** Default provider for a property value

        This default implementation queries the object which it is constructed with for the XPropertySet interface,
        and calls the getPropertyValue method.
    */
    class SVXCORE_DLLPUBLIC PropertyValueProvider
    {
    public:
        PropertyValueProvider( ::cppu::OWeakObject& _rContext, const char* _pAsciiPropertyName )
            :m_rContext( _rContext )
            ,m_sPropertyName( OUString::createFromAscii( _pAsciiPropertyName ) )
        {
        }
        virtual ~PropertyValueProvider();

        /** returns the name of the property which this provider is responsible for
        */
        const OUString & getPropertyName() const;
        /** returns the current value of the property which the provider is responsible for
        */
        virtual void getCurrentValue( css::uno::Any& _out_rValue ) const;

    protected:
        ::cppu::OWeakObject&    getContext() const { return m_rContext; }
        PropertyValueProvider(const PropertyValueProvider&) = delete;
        PropertyValueProvider& operator=(const PropertyValueProvider&) = delete;

    private:
        ::cppu::OWeakObject&    m_rContext;
        const OUString   m_sPropertyName;
    };


    /** helper class for notifying XPropertyChangeListeners

        The class is intended to be held as member of the class which does the property change broadcasting.
    */
    class SVXCORE_DLLPUBLIC PropertyChangeNotifier
    {
    public:
        /** constructs a notifier instance

            @param _rOwner
                the owner instance of the notifier. Will be used as css.lang.EventObject.Source when
                notifying events.
        */
        PropertyChangeNotifier( ::cppu::OWeakObject& _rOwner, ::osl::Mutex& _rMutex );
        ~PropertyChangeNotifier();

        // listener maintenance
        void addPropertyChangeListener( const OUString& _rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener );
        void removePropertyChangeListener( const OUString& _rPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& _rxListener );

        /** registers an PropertyValueProvider
        */
        void    registerProvider( const ShapePropertyProviderId _eProperty, std::unique_ptr<PropertyValueProvider> _rProvider );

        /** notifies changes in the given property to all registered listeners

            If no property value provider for the given property ID is registered, this is worth an assertion in a
            non-product build, and otherwise ignored.
        */
        void    notifyPropertyChange( const ShapePropertyProviderId _eProperty ) const;

        /** is called to dispose the instance
        */
        void    disposing();

    private:
        PropertyChangeNotifier(const PropertyChangeNotifier&) = delete;
        PropertyChangeNotifier& operator=(const PropertyChangeNotifier&) = delete;

        ::cppu::OWeakObject&            m_rContext;
        o3tl::enumarray<ShapePropertyProviderId, std::unique_ptr<PropertyValueProvider>>  m_aProviders;
        comphelper::OMultiTypeInterfaceContainerHelperVar3<css::beans::XPropertyChangeListener, OUString> m_aPropertyChangeListeners;
    };


}


#endif // INCLUDED_SVX_SHAPEPROPERTYNOTIFIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
