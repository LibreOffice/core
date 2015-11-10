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

#include <comphelper/propertybag.hxx>

#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NotRemoveableException.hpp>

#include <map>


namespace comphelper
{


    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::TypeClass_VOID;
    using ::com::sun::star::beans::IllegalTypeException;
    using ::com::sun::star::beans::PropertyExistException;
    using ::com::sun::star::container::ElementExistException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::beans::NotRemoveableException;
    using ::com::sun::star::beans::UnknownPropertyException;

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    typedef ::std::map< sal_Int32, Any >    MapInt2Any;
    struct PropertyBag_Impl
    {
        PropertyBag_Impl() : m_bAllowEmptyPropertyName(false) { }
        MapInt2Any  aDefaults;
        bool m_bAllowEmptyPropertyName;
    };

    PropertyBag::PropertyBag()
        :m_pImpl( new PropertyBag_Impl )
    {
    }

    PropertyBag::~PropertyBag()
    {
    }


    void PropertyBag::setAllowEmptyPropertyName( bool i_isAllowed )
    {
        m_pImpl->m_bAllowEmptyPropertyName = i_isAllowed;
    }


    namespace
    {
        void    lcl_checkForEmptyName( const bool _allowEmpty, const OUString& _name )
        {
            if ( !_allowEmpty && _name.isEmpty() )
                throw IllegalArgumentException(
                        "The property name must not be empty.",
                        // TODO: resource
                        nullptr,
                        1
                      );
        }

        void    lcl_checkNameAndHandle_PropertyExistException( const OUString& _name, const sal_Int32 _handle, const PropertyBag& _container )
        {
            if ( _container.hasPropertyByName( _name ) || _container.hasPropertyByHandle( _handle ) )
                throw PropertyExistException(
                    "Property name or handle already used.",
                    nullptr );

        }

        void    lcl_checkNameAndHandle_ElementExistException( const OUString& _name, const sal_Int32 _handle, const PropertyBag& _container )
        {
            if ( _container.hasPropertyByName( _name ) || _container.hasPropertyByHandle( _handle ) )
                throw ElementExistException(
                    "Property name or handle already used.",
                    nullptr );

        }

    }


    void PropertyBag::addVoidProperty( const OUString& _rName, const Type& _rType, sal_Int32 _nHandle, sal_Int32 _nAttributes )
    {
        if ( _rType.getTypeClass() == TypeClass_VOID )
            throw IllegalArgumentException(
                    "Illegal property type: VOID",
                        // TODO: resource
                    nullptr,
                    1
                  );

        // check name/handle sanity
        lcl_checkForEmptyName( m_pImpl->m_bAllowEmptyPropertyName, _rName );
        lcl_checkNameAndHandle_ElementExistException( _rName, _nHandle, *this );

        // register the property
        OSL_ENSURE( _nAttributes & PropertyAttribute::MAYBEVOID, "PropertyBag::addVoidProperty: this is for default-void properties only!" );
        registerPropertyNoMember( _rName, _nHandle, _nAttributes | PropertyAttribute::MAYBEVOID, _rType, nullptr );

        // remember the default
        m_pImpl->aDefaults.insert( MapInt2Any::value_type( _nHandle, Any() ) );
    }


    void PropertyBag::addProperty( const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes, const Any& _rInitialValue )
    {
        // check type sanity
        Type aPropertyType = _rInitialValue.getValueType();
        if ( aPropertyType.getTypeClass() == TypeClass_VOID )
            throw IllegalTypeException(
                "The initial value must be non-NULL to determine the property type.",
                // TODO: resource
                nullptr );

        // check name/handle sanity
        lcl_checkForEmptyName( m_pImpl->m_bAllowEmptyPropertyName, _rName );
        lcl_checkNameAndHandle_PropertyExistException( _rName, _nHandle, *this );

        // register the property
        registerPropertyNoMember( _rName, _nHandle, _nAttributes, aPropertyType,
            _rInitialValue.hasValue() ? _rInitialValue.getValue() : nullptr );

        // remember the default
        m_pImpl->aDefaults.insert( MapInt2Any::value_type( _nHandle, _rInitialValue ) );
    }


    void PropertyBag::removeProperty( const OUString& _rName )
    {
        const Property& rProp = getProperty( _rName );
            // will throw an UnknownPropertyException if necessary
        if ( ( rProp.Attributes & PropertyAttribute::REMOVABLE ) == 0 )
            throw NotRemoveableException( OUString(), nullptr );
        const sal_Int32 nHandle = rProp.Handle;

        revokeProperty( nHandle );

        m_pImpl->aDefaults.erase( nHandle );
    }


    void PropertyBag::getFastPropertyValue( sal_Int32 _nHandle, Any& _out_rValue ) const
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        OPropertyContainerHelper::getFastPropertyValue( _out_rValue, _nHandle );
    }


    bool PropertyBag::convertFastPropertyValue( sal_Int32 _nHandle, const Any& _rNewValue, Any& _out_rConvertedValue, Any& _out_rCurrentValue ) const
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        return const_cast< PropertyBag*  >( this )->OPropertyContainerHelper::convertFastPropertyValue(
            _out_rConvertedValue, _out_rCurrentValue, _nHandle, _rNewValue );
    }


    void PropertyBag::setFastPropertyValue( sal_Int32 _nHandle, const Any& _rValue )
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );
    }


    void PropertyBag::getPropertyDefaultByHandle( sal_Int32 _nHandle, Any& _out_rValue ) const
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        MapInt2Any::const_iterator pos = m_pImpl->aDefaults.find( _nHandle );
        OSL_ENSURE( pos != m_pImpl->aDefaults.end(), "PropertyBag::getPropertyDefaultByHandle: inconsistency!" );
        if ( pos != m_pImpl->aDefaults.end() )
            _out_rValue = pos->second;
        else
            _out_rValue.clear();
    }



} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
