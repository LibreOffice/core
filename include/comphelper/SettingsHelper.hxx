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
        ::com::sun::star::beans::XPropertySet,
        ::com::sun::star::beans::XMultiPropertySet,
        ::com::sun::star::lang::XServiceInfo
    >
    HelperBaseNoState;
    class ChainableHelperNoState :
        public HelperBaseNoState,
        public ChainablePropertySet
    {
    public:
        ChainableHelperNoState ( ChainablePropertySetInfo *pInfo, SolarMutex* pMutex = NULL)
        : ChainablePropertySet ( pInfo, pMutex )
        {}
        virtual ~ChainableHelperNoState () throw( ) {}
        com::sun::star::uno::Any SAL_CALL queryInterface( const com::sun::star::uno::Type& aType ) throw (com::sun::star::uno::RuntimeException, std::exception) override
        { return HelperBaseNoState::queryInterface( aType ); }
        void SAL_CALL acquire(  ) throw () override
        { HelperBaseNoState::acquire( ); }
        void SAL_CALL release(  ) throw () override
        { HelperBaseNoState::release( ); }

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override
        { return ChainablePropertySet::getPropertySetInfo(); }
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::setPropertyValue ( aPropertyName, aValue ); }
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override
        { return ChainablePropertySet::getPropertyValue ( PropertyName ); }
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::addPropertyChangeListener ( aPropertyName, xListener ); }
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::removePropertyChangeListener ( aPropertyName, aListener ); }
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::addVetoableChangeListener ( PropertyName, aListener ); }
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::removeVetoableChangeListener ( PropertyName, aListener ); }

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
            throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::setPropertyValues ( aPropertyNames, aValues ); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override
        { return ChainablePropertySet::getPropertyValues ( aPropertyNames ); }
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::addPropertiesChangeListener ( aPropertyNames, xListener ); }
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::removePropertiesChangeListener ( xListener ); }
        virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override
        { ChainablePropertySet::firePropertiesChangeEvent ( aPropertyNames, xListener ); }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
