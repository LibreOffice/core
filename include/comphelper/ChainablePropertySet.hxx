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
    class COMPHELPER_DLLPUBLIC ChainablePropertySet : public ::com::sun::star::beans::XPropertySet,
                                 public ::com::sun::star::beans::XPropertyState,
                                 public ::com::sun::star::beans::XMultiPropertySet
    {
        friend class MasterPropertySet;
    protected:
        ChainablePropertySetInfo *mpInfo;
        SolarMutex* mpMutex;
        ::com::sun::star::uno::Reference < com::sun::star::beans::XPropertySetInfo > mxInfo;

        virtual void _preSetValues ()
            throw (css::beans::UnknownPropertyException,
                   css::beans::PropertyVetoException,
                   css::lang::IllegalArgumentException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException) = 0;
        virtual void _setSingleValue(const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue)
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
        virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue )
            throw (css::beans::UnknownPropertyException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException) = 0;
        virtual void _postGetValues ()
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException ) = 0;

        static void _preGetPropertyState ()
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
        static void _getPropertyState( const comphelper::PropertyInfo& rInfo, ::com::sun::star::beans::PropertyState& rState )
            throw(::com::sun::star::beans::UnknownPropertyException );
        static void _postGetPropertyState ()
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

        static void _setPropertyToDefault( const comphelper::PropertyInfo& rEntry )
            throw(::com::sun::star::beans::UnknownPropertyException );
        static ::com::sun::star::uno::Any _getPropertyDefault( const comphelper::PropertyInfo& rEntry )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

    public:
        ChainablePropertySet( comphelper::ChainablePropertySetInfo* pInfo, SolarMutex* pMutex = NULL )
            throw();
        virtual ~ChainablePropertySet()
            throw();

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
            throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
