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

#ifndef INCLUDED_COMPHELPER_CHAINABLEPROPERTYSET_HXX
#define INCLUDED_COMPHELPER_CHAINABLEPROPERTYSET_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/PropertyInfoHash.hxx>
#include <comphelper/comphelperdllapi.h>
#include <comphelper/solarmutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

namespace comphelper
{
    class ChainablePropertySetInfo;
}

/*
 * A ChainablePropertySet has the following features:
 *
 * 1. It implements both the PropertySet and MultiPropertySet interfaces.
 * 2. It can be 'included' in a MasterPropertySet to seamlessly appear as if
 *    if it's properties were in the master.
 *
 * To be used as a base class for PropertySets, the subclass must implement
 * the 6 protected pure virtual functions. If a mutex is passed to the
 * constructor, this is locked before any call to _getSingleValue or
 * _setSingleValue and released after all processing has completed
 * (including _postSetValues or _postGetValues )
 *
 * Any MasterPropertySet implementations that can include an
 * implementation of a given ChainablePropertySet must be
 * declared as a 'friend' in the implementation of the ChainablePropertySet.
 *
 */

namespace comphelper
{
    typedef cppu::WeakImplHelper
    <
        css::beans::XPropertySet,
        css::beans::XMultiPropertySet,
        css::lang::XServiceInfo
    >
    ChainablePropertySetBase;
    class COMPHELPER_DLLPUBLIC ChainablePropertySet : public ChainablePropertySetBase,
                                 public css::beans::XPropertyState
    {
        friend class MasterPropertySet;
    protected:
        SolarMutex* const mpMutex;
        rtl::Reference < ChainablePropertySetInfo > mxInfo;

        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        virtual void _preSetValues () = 0;
        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        virtual void _setSingleValue(const comphelper::PropertyInfo & rInfo, const css::uno::Any &rValue) = 0;
        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        virtual void _postSetValues () = 0;

        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        virtual void _preGetValues () = 0;
        /// @throws css::beans::UnknownPropertyException
        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, css::uno::Any & rValue ) = 0;
        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        virtual void _postGetValues () = 0;

    public:
        ChainablePropertySet( comphelper::ChainablePropertySetInfo* pInfo, SolarMutex* pMutex )
            throw();
        virtual ~ChainablePropertySet()
            throw() override;

        css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override
        { return ChainablePropertySetBase::queryInterface( aType ); }
        void SAL_CALL acquire(  ) throw () override
        { ChainablePropertySetBase::acquire( ); }
        void SAL_CALL release(  ) throw () override
        { ChainablePropertySetBase::release( ); }

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
        virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
        virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

        // XPropertyState
        virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
        virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
        virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
        virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
