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

#include <comphelper/propertystatecontainer.hxx>
#include <rtl/ustrbuf.hxx>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    namespace
    {
        OUString lcl_getUnknownPropertyErrorMessage( const OUString& _rPropertyName )
        {
            // TODO: perhaps it's time to think about resources in the comphelper module?
            // Would be nice to have localized exception strings (a simply resource file containing
            // strings only would suffice, and could be realized with an UNO service, so we do not
            // need the dependency to the Tools project)
            OUStringBuffer sMessage;
            sMessage.append( "The property \"" );
            sMessage.append( _rPropertyName );
            sMessage.append( "\" is unknown." );
            return sMessage.makeStringAndClear();
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
        {
            const OUString* pNames = _rPropertyNames.getConstArray();
            const OUString* pNamesCompare = pNames + 1;
            const OUString* pNamesEnd = _rPropertyNames.getConstArray() + _rPropertyNames.getLength();
            for ( ; pNamesCompare != pNamesEnd; ++pNames, ++pNamesCompare )
                OSL_PRECOND( pNames->compareTo( *pNamesCompare ) < 0,
                    "OPropertyStateContainer::getPropertyStates: property sequence not sorted!" );
        }
#endif

        const OUString* pLookup = _rPropertyNames.getConstArray();
        const OUString* pLookupEnd = pLookup + nProperties;
        PropertyState* pStates = aStates.getArray();

        cppu::IPropertyArrayHelper& rHelper = getInfoHelper();
        Sequence< Property> aAllProperties  = rHelper.getProperties();
        sal_Int32 nAllProperties            = aAllProperties.getLength();
        const  Property* pAllProperties     = aAllProperties.getConstArray();
        const  Property* pAllPropertiesEnd  = pAllProperties + nAllProperties;

        osl::MutexGuard aGuard( rBHelper.rMutex );
        for ( ; ( pAllProperties != pAllPropertiesEnd ) && ( pLookup != pLookupEnd ); ++pAllProperties )
        {
#ifdef DBG_UTIL
            if ( pAllProperties < pAllPropertiesEnd - 1 )
                OSL_ENSURE( pAllProperties->Name.compareTo( (pAllProperties + 1)->Name ) < 0,
                    "OPropertyStateContainer::getPropertyStates: all-properties not sorted!" );
#endif
            if ( pAllProperties->Name == *pLookup )
            {
                *pStates++ = getPropertyState( *pLookup );
                ++pLookup;
            }
        }

        if ( pLookup != pLookupEnd )
            // we run out of properties from the IPropertyArrayHelper, but still have properties to lookup
            // -> we were asked for a nonexistent property
            throw UnknownPropertyException( lcl_getUnknownPropertyErrorMessage( *pLookup ), static_cast< XPropertyState* >( this ) );

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
