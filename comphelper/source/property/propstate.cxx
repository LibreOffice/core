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
#include <rtl/instance.hxx>

//.........................................................................
namespace comphelper
{
//.........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::XTypeProvider;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::cpp_queryInterface;
    using ::com::sun::star::uno::cpp_release;
    using ::com::sun::star::beans::PropertyState_DEFAULT_VALUE;
    using ::com::sun::star::beans::PropertyState_DIRECT_VALUE;

    //=====================================================================
    // OPropertyStateHelper
    //=====================================================================


    ::com::sun::star::uno::Any SAL_CALL OPropertyStateHelper::queryInterface(const  ::com::sun::star::uno::Type& _rType) throw( ::com::sun::star::uno::RuntimeException, std::exception)
    {
        ::com::sun::star::uno::Any aReturn = OPropertySetHelper2::queryInterface(_rType);
        // our own ifaces
        if ( !aReturn.hasValue() )
            aReturn = ::cppu::queryInterface(_rType, static_cast< ::com::sun::star::beans::XPropertyState*>(this));

        return aReturn;
    }


    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> OPropertyStateHelper::getTypes() throw( ::com::sun::star::uno::RuntimeException, std::exception)
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> aTypes(4);
        ::com::sun::star::uno::Type* pTypes = aTypes.getArray();
        // base class types
        pTypes[0] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)NULL);
        pTypes[1] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet>*)NULL);
        pTypes[2] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet>*)NULL);
        // my own type
        pTypes[3] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState>*)NULL);
        return aTypes;
    }

    OPropertyStateHelper::OPropertyStateHelper(
        ::cppu::OBroadcastHelper& rBHlp,
        ::cppu::IEventNotificationHook *i_pFireEvents)
        :   ::cppu::OPropertySetHelper2(rBHlp, i_pFireEvents) { }

    OPropertyStateHelper::~OPropertyStateHelper() {}


    void OPropertyStateHelper::firePropertyChange(sal_Int32 nHandle, const  ::com::sun::star::uno::Any& aNewValue, const  ::com::sun::star::uno::Any& aOldValue)
    {
        fire(&nHandle, &aNewValue, &aOldValue, 1, sal_False);
    }

    // XPropertyState

    ::com::sun::star::beans::PropertyState SAL_CALL OPropertyStateHelper::getPropertyState(const OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException, std::exception)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw  ::com::sun::star::beans::UnknownPropertyException();

        return getPropertyStateByHandle(nHandle);
    }


    void SAL_CALL OPropertyStateHelper::setPropertyToDefault(const OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException, std::exception)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw  ::com::sun::star::beans::UnknownPropertyException();

        setPropertyToDefaultByHandle(nHandle);
    }


    ::com::sun::star::uno::Any SAL_CALL OPropertyStateHelper::getPropertyDefault(const OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::lang::WrappedTargetException,  ::com::sun::star::uno::RuntimeException, std::exception)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw  ::com::sun::star::beans::UnknownPropertyException();

        return getPropertyDefaultByHandle(nHandle);
    }


    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> SAL_CALL OPropertyStateHelper::getPropertyStates(const  ::com::sun::star::uno::Sequence< OUString >& _rPropertyNames) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException, std::exception)
    {
        sal_Int32 nLen = _rPropertyNames.getLength();
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> aRet(nLen);
        ::com::sun::star::beans::PropertyState* pValues = aRet.getArray();
        const OUString* pNames = _rPropertyNames.getConstArray();

        cppu::IPropertyArrayHelper& rHelper = getInfoHelper();

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aProps = rHelper.getProperties();
        const  ::com::sun::star::beans::Property* pProps = aProps.getConstArray();
        sal_Int32 nPropCount       = aProps.getLength();

        osl::MutexGuard aGuard(rBHelper.rMutex);
        for (sal_Int32 i=0, j=0; i<nPropCount && j<nLen; ++i, ++pProps)
        {
            // get the values only for valid properties
            if (pProps->Name.equals(*pNames))
            {
                *pValues = getPropertyState(*pNames);
                ++pValues;
                ++pNames;
                ++j;
            }
        }

        return aRet;
    }


    ::com::sun::star::beans::PropertyState OPropertyStateHelper::getPropertyStateByHandle( sal_Int32 _nHandle )
    {
        // simply compare the current and the default value
        Any aCurrentValue = getPropertyDefaultByHandle( _nHandle );
        Any aDefaultValue;  getFastPropertyValue( aDefaultValue, _nHandle );

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


    ::com::sun::star::uno::Any OPropertyStateHelper::getPropertyDefaultByHandle( sal_Int32 ) const
    {
        return  ::com::sun::star::uno::Any();
    }

    //=====================================================================
    // OStatefulPropertySet
    //=====================================================================

    OStatefulPropertySet::OStatefulPropertySet()
        :OPropertyStateHelper( GetBroadcastHelper() )
    {
    }


    OStatefulPropertySet::~OStatefulPropertySet()
    {
    }


    Sequence< Type > SAL_CALL OStatefulPropertySet::getTypes() throw(RuntimeException, std::exception)
    {
        Sequence< Type > aOwnTypes( 2 );
        aOwnTypes[0] = cppu::UnoType<XWeak>::get();
        aOwnTypes[1] = cppu::UnoType<XTypeProvider>::get();

        return concatSequences(
            aOwnTypes,
            OPropertyStateHelper::getTypes()
        );
    }

    namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }


    Sequence< sal_Int8 > SAL_CALL OStatefulPropertySet::getImplementationId() throw(RuntimeException, std::exception)
    {
        ::cppu::OImplementationId &rID = lcl_ImplId::get();
        return rID.getImplementationId();
    }


    Any SAL_CALL OStatefulPropertySet::queryInterface( const Type& _rType ) throw(RuntimeException, std::exception)
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

//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
