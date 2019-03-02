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

#include <comphelper/propstate.hxx>
#include <com/sun/star/uno/genfunc.h>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/sequence.hxx>

namespace comphelper
{


    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::XTypeProvider;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::cpp_queryInterface;
    using ::com::sun::star::uno::cpp_release;
    using ::com::sun::star::beans::PropertyState_DEFAULT_VALUE;
    using ::com::sun::star::beans::PropertyState_DIRECT_VALUE;


    // OPropertyStateHelper


    css::uno::Any SAL_CALL OPropertyStateHelper::queryInterface(const  css::uno::Type& _rType)
    {
        css::uno::Any aReturn = OPropertySetHelper2::queryInterface(_rType);
        // our own ifaces
        if ( !aReturn.hasValue() )
            aReturn = ::cppu::queryInterface(_rType, static_cast< css::beans::XPropertyState*>(this));

        return aReturn;
    }


    css::uno::Sequence<css::uno::Type> OPropertyStateHelper::getTypes()
    {
        return {
            cppu::UnoType<css::beans::XPropertySet>::get(),
            cppu::UnoType<css::beans::XMultiPropertySet>::get(),
            cppu::UnoType<css::beans::XFastPropertySet>::get(),
            cppu::UnoType<css::beans::XPropertySetOption>::get(),
            cppu::UnoType<css::beans::XPropertyState>::get()};
    }

    OPropertyStateHelper::OPropertyStateHelper(
        ::cppu::OBroadcastHelper& rBHlp,
        ::cppu::IEventNotificationHook *i_pFireEvents)
        :   ::cppu::OPropertySetHelper2(rBHlp, i_pFireEvents) { }

    OPropertyStateHelper::~OPropertyStateHelper() {}


    void OPropertyStateHelper::firePropertyChange(sal_Int32 nHandle, const  css::uno::Any& aNewValue, const  css::uno::Any& aOldValue)
    {
        fire(&nHandle, &aNewValue, &aOldValue, 1, false);
    }

    // XPropertyState

    css::beans::PropertyState SAL_CALL OPropertyStateHelper::getPropertyState(const OUString& _rsName)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw  css::beans::UnknownPropertyException();

        return getPropertyStateByHandle(nHandle);
    }


    void SAL_CALL OPropertyStateHelper::setPropertyToDefault(const OUString& _rsName)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw css::beans::UnknownPropertyException();

        setPropertyToDefaultByHandle(nHandle);
    }


    css::uno::Any SAL_CALL OPropertyStateHelper::getPropertyDefault(const OUString& _rsName)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw css::beans::UnknownPropertyException();

        return getPropertyDefaultByHandle(nHandle);
    }


    css::uno::Sequence< css::beans::PropertyState> SAL_CALL OPropertyStateHelper::getPropertyStates(const  css::uno::Sequence< OUString >& _rPropertyNames)
    {
        sal_Int32 nLen = _rPropertyNames.getLength();
        css::uno::Sequence< css::beans::PropertyState> aRet(nLen);
        css::beans::PropertyState* pValues = aRet.getArray();
        const OUString* pNames = _rPropertyNames.getConstArray();

        cppu::IPropertyArrayHelper& rHelper = getInfoHelper();

        css::uno::Sequence< css::beans::Property> aProps = rHelper.getProperties();
        const css::beans::Property* pProps = aProps.getConstArray();
        sal_Int32 nPropCount       = aProps.getLength();

        osl::MutexGuard aGuard(rBHelper.rMutex);
        for (sal_Int32 i=0, j=0; i<nPropCount && j<nLen; ++i, ++pProps)
        {
            // get the values only for valid properties
            if (pProps->Name == *pNames)
            {
                *pValues = getPropertyState(*pNames);
                ++pValues;
                ++pNames;
                ++j;
            }
        }

        return aRet;
    }


    css::beans::PropertyState OPropertyStateHelper::getPropertyStateByHandle( sal_Int32 _nHandle )
    {
        // simply compare the current and the default value
        Any aCurrentValue = getPropertyDefaultByHandle( _nHandle );
        Any aDefaultValue;
        getFastPropertyValue( aDefaultValue, _nHandle );

        bool bEqual = uno_type_equalData(
                const_cast< void* >( aCurrentValue.getValue() ), aCurrentValue.getValueType().getTypeLibType(),
                const_cast< void* >( aDefaultValue.getValue() ), aDefaultValue.getValueType().getTypeLibType(),
                reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
                reinterpret_cast< uno_ReleaseFunc >(cpp_release)
            );
        return bEqual ? PropertyState_DEFAULT_VALUE : PropertyState_DIRECT_VALUE;
    }


    void OPropertyStateHelper::setPropertyToDefaultByHandle( sal_Int32 _nHandle )
    {
        setFastPropertyValue( _nHandle, getPropertyDefaultByHandle( _nHandle ) );
    }


    css::uno::Any OPropertyStateHelper::getPropertyDefaultByHandle( sal_Int32 ) const
    {
        return css::uno::Any();
    }


    // OStatefulPropertySet


    OStatefulPropertySet::OStatefulPropertySet()
        :OPropertyStateHelper( GetBroadcastHelper() )
    {
    }


    OStatefulPropertySet::~OStatefulPropertySet()
    {
    }


    Sequence< Type > SAL_CALL OStatefulPropertySet::getTypes()
    {
        return concatSequences(
            Sequence {
                cppu::UnoType<XWeak>::get(),
                cppu::UnoType<XTypeProvider>::get() },
            OPropertyStateHelper::getTypes()
        );
    }

    Sequence< sal_Int8 > SAL_CALL OStatefulPropertySet::getImplementationId()
    {
        return css::uno::Sequence<sal_Int8>();
    }


    Any SAL_CALL OStatefulPropertySet::queryInterface( const Type& _rType )
    {
        Any aReturn = OWeakObject::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = ::cppu::queryInterface( _rType, static_cast< XTypeProvider* >( this ) );
        if ( !aReturn.hasValue() )
            aReturn = OPropertyStateHelper::queryInterface( _rType );
        return aReturn;
    }


    void SAL_CALL OStatefulPropertySet::acquire() throw()
    {
        ::cppu::OWeakObject::acquire();
    }


    void SAL_CALL OStatefulPropertySet::release() throw()
    {
        ::cppu::OWeakObject::release();
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
