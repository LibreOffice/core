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

#ifndef INCLUDED_COMPHELPER_SETTINGSHELPER_HXX
#define INCLUDED_COMPHELPER_SETTINGSHELPER_HXX
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/ChainablePropertySet.hxx>
#include <comphelper/solarmutex.hxx>
#include <cppuhelper/implbase.hxx>

namespace comphelper
{
    typedef  cppu::WeakImplHelper
    <
        css::beans::XPropertySet,
        css::beans::XMultiPropertySet,
        css::lang::XServiceInfo
    >
    HelperBaseNoState;
    class ChainableHelperNoState :
        public HelperBaseNoState,
        public ChainablePropertySet
    {
    public:
        ChainableHelperNoState ( ChainablePropertySetInfo *pInfo, SolarMutex* pMutex = nullptr)
        : ChainablePropertySet ( pInfo, pMutex )
        {}
        css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override
        { return HelperBaseNoState::queryInterface( aType ); }
        void SAL_CALL acquire(  ) throw () override
        { HelperBaseNoState::acquire( ); }
        void SAL_CALL release(  ) throw () override
        { HelperBaseNoState::release( ); }

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override
        { return ChainablePropertySet::getPropertySetInfo(); }
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override
        { ChainablePropertySet::setPropertyValue ( aPropertyName, aValue ); }
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override
        { return ChainablePropertySet::getPropertyValue ( PropertyName ); }
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override
        { ChainablePropertySet::addPropertyChangeListener ( aPropertyName, xListener ); }
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override
        { ChainablePropertySet::removePropertyChangeListener ( aPropertyName, aListener ); }
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override
        { ChainablePropertySet::addVetoableChangeListener ( PropertyName, aListener ); }
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override
        { ChainablePropertySet::removeVetoableChangeListener ( PropertyName, aListener ); }

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override
        { ChainablePropertySet::setPropertyValues ( aPropertyNames, aValues ); }
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override
        { return ChainablePropertySet::getPropertyValues ( aPropertyNames ); }
        virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override
        { ChainablePropertySet::addPropertiesChangeListener ( aPropertyNames, xListener ); }
        virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override
        { ChainablePropertySet::removePropertiesChangeListener ( xListener ); }
        virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override
        { ChainablePropertySet::firePropertiesChangeEvent ( aPropertyNames, xListener ); }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
