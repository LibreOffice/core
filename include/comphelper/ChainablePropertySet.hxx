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
#include <comphelper/PropertyInfoHash.hxx>
#include <comphelper/comphelperdllapi.h>
#include <comphelper/solarmutex.hxx>

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
    class COMPHELPER_DLLPUBLIC ChainablePropertySet : public css::beans::XPropertySet,
                                 public css::beans::XPropertyState,
                                 public css::beans::XMultiPropertySet
    {
        friend class MasterPropertySet;
    protected:
        ChainablePropertySetInfo *mpInfo;
        SolarMutex* mpMutex;
        css::uno::Reference < css::beans::XPropertySetInfo > mxInfo;

        virtual void _preSetValues ()
            throw (css::beans::UnknownPropertyException,
                   css::beans::PropertyVetoException,
                   css::lang::IllegalArgumentException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException) = 0;
        virtual void _setSingleValue(const comphelper::PropertyInfo & rInfo, const css::uno::Any &rValue)
            throw (css::beans::UnknownPropertyException,
                   css::beans::PropertyVetoException,
                   css::lang::IllegalArgumentException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException,
                   std::exception) = 0;
        virtual void _postSetValues ()
            throw (css::beans::UnknownPropertyException,
                   css::beans::PropertyVetoException,
                   css::lang::IllegalArgumentException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException) = 0;

        virtual void _preGetValues ()
            throw (css::beans::UnknownPropertyException,
                   css::beans::PropertyVetoException,
                   css::lang::IllegalArgumentException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException) = 0;
        virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, css::uno::Any & rValue )
            throw (css::beans::UnknownPropertyException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException) = 0;
        virtual void _postGetValues ()
            throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException ) = 0;

        static void _preGetPropertyState ()
            throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException );
        static void _getPropertyState( const comphelper::PropertyInfo& rInfo, css::beans::PropertyState& rState )
            throw(css::beans::UnknownPropertyException );
        static void _postGetPropertyState ()
            throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException );

        static void _setPropertyToDefault( const comphelper::PropertyInfo& rEntry )
            throw(css::beans::UnknownPropertyException );
        static css::uno::Any _getPropertyDefault( const comphelper::PropertyInfo& rEntry )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException );

    public:
        ChainablePropertySet( comphelper::ChainablePropertySetInfo* pInfo, SolarMutex* pMutex = nullptr )
            throw();
        virtual ~ChainablePropertySet()
            throw();

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
            throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues )
            throw(css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames )
            throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
            throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
            throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
            throw(css::uno::RuntimeException, std::exception) override;

        // XPropertyState
        virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName )
            throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName )
            throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName )
            throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )
            throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
