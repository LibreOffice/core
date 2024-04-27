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

#include <sal/config.h>

#include <string_view>

#include <comphelper/propertystatecontainer.hxx>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    namespace
    {
        OUString lcl_getUnknownPropertyErrorMessage( std::u16string_view _rPropertyName )
        {
            // TODO: perhaps it's time to think about resources in the comphelper module?
            // Would be nice to have localized exception strings (a simply resource file containing
            // strings only would suffice, and could be realized with a UNO service, so we do not
            // need the dependency to the Tools project)
            return OUString::Concat("The property \"") + _rPropertyName + "\" is unknown.";
        }
    }

    OPropertyStateContainer::OPropertyStateContainer( ::cppu::OBroadcastHelper& _rBHelper )
        :OPropertyContainer( _rBHelper )
    {
    }


    Any SAL_CALL OPropertyStateContainer::queryInterface( const Type& _rType )
    {
        Any aReturn = OPropertyContainer::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OPropertyStateContainer_TBase::queryInterface( _rType );
        return aReturn;
    }


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OPropertyStateContainer, OPropertyContainer, OPropertyStateContainer_TBase )


    sal_Int32 OPropertyStateContainer::getHandleForName( const OUString& _rPropertyName )
    {
        // look up the handle for the name
        ::cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName( _rPropertyName );

        if ( -1 == nHandle )
            throw  UnknownPropertyException( lcl_getUnknownPropertyErrorMessage( _rPropertyName ), static_cast< XPropertyState* >( this ) );

        return nHandle;
    }


    PropertyState SAL_CALL OPropertyStateContainer::getPropertyState( const OUString& _rPropertyName )
    {
        return getPropertyStateByHandle( getHandleForName( _rPropertyName ) );
    }


    Sequence< PropertyState > SAL_CALL OPropertyStateContainer::getPropertyStates( const Sequence< OUString >& _rPropertyNames )
    {
        sal_Int32 nProperties = _rPropertyNames.getLength();
        Sequence< PropertyState> aStates( nProperties );
        if ( !nProperties )
            return aStates;

#ifdef DBG_UTIL
        // precondition: property sequence is sorted (the algorithm below relies on this)
        OSL_PRECOND(std::is_sorted(_rPropertyNames.begin(), _rPropertyNames.end(),
                                   [](auto& lhs, auto& rhs) { return lhs.compareTo(rhs) < 0; }),
                    "OPropertyStateContainer::getPropertyStates: property sequence not sorted!" );
#endif

        PropertyState* pStates = aStates.getArray();

        cppu::IPropertyArrayHelper& rHelper = getInfoHelper();
        Sequence< Property> aAllProperties  = rHelper.getProperties();
#ifdef DBG_UTIL
        OSL_ENSURE(std::is_sorted(aAllProperties.begin(), aAllProperties.end(),
                                  [](auto& lhs, auto& rhs)
                                  { return lhs.Name.compareTo(rhs.Name) < 0; }),
                   "OPropertyStateContainer::getPropertyStates: all-properties not sorted!");
#endif

        auto it = aAllProperties.begin();
        const auto end = aAllProperties.end();
        osl::MutexGuard aGuard( rBHelper.rMutex );
        for (auto& propName : _rPropertyNames)
        {
            it = std::find_if(it, end, [&propName](auto& prop) { return prop.Name == propName; });
            if (it == end)
                throw UnknownPropertyException(lcl_getUnknownPropertyErrorMessage(propName),
                                               static_cast<XPropertyState*>(this));
            *pStates++ = getPropertyStateByHandle(it->Handle);
        }

        return aStates;
    }


    void SAL_CALL OPropertyStateContainer::setPropertyToDefault( const OUString& _rPropertyName )
    {
        setPropertyToDefaultByHandle( getHandleForName( _rPropertyName ) );
    }


    Any SAL_CALL OPropertyStateContainer::getPropertyDefault( const OUString& _rPropertyName )
    {
        Any aDefault;
        getPropertyDefaultByHandle( getHandleForName( _rPropertyName ), aDefault );
        return aDefault;
    }


    PropertyState OPropertyStateContainer::getPropertyStateByHandle( sal_Int32 _nHandle ) const
    {
        // simply compare the current and the default value
        Any aCurrentValue;
        getFastPropertyValue( aCurrentValue, _nHandle );
        Any aDefaultValue;
        getPropertyDefaultByHandle( _nHandle, aDefaultValue );

        bool bEqual = uno_type_equalData(
                const_cast< void* >( aCurrentValue.getValue() ), aCurrentValue.getValueType().getTypeLibType(),
                const_cast< void* >( aDefaultValue.getValue() ), aDefaultValue.getValueType().getTypeLibType(),
                reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
                reinterpret_cast< uno_ReleaseFunc >(cpp_release)
            );
        if ( bEqual )
            return PropertyState_DEFAULT_VALUE;
        else
            return PropertyState_DIRECT_VALUE;
    }


    void OPropertyStateContainer::setPropertyToDefaultByHandle( sal_Int32 _nHandle )
    {
        Any aDefault;
        getPropertyDefaultByHandle( _nHandle, aDefault );
        setFastPropertyValue( _nHandle, aDefault );
    }


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
