/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include "comphelper/propertybag.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NotRemoveableException.hpp>
/** === end UNO includes === **/

#include <map>

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::TypeClass_VOID;
    using ::com::sun::star::beans::IllegalTypeException;
    using ::com::sun::star::beans::PropertyExistException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::beans::NotRemoveableException;
    using ::com::sun::star::beans::UnknownPropertyException;
    /** === end UNO using === **/
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    //====================================================================
    //= PropertyBag_Impl
    //====================================================================
    typedef ::std::map< sal_Int32, Any >    MapInt2Any;
    struct PropertyBag_Impl
    {
        PropertyBag_Impl() : m_bAllowEmptyPropertyName(false) { }
        MapInt2Any  aDefaults;
        bool m_bAllowEmptyPropertyName;
    };

    //====================================================================
    //= PropertyBag
    //====================================================================
    //--------------------------------------------------------------------
    PropertyBag::PropertyBag()
        :m_pImpl( new PropertyBag_Impl )
    {
    }

    PropertyBag::~PropertyBag()
    {
    }

    //--------------------------------------------------------------------
    void PropertyBag::setAllowEmptyPropertyName( bool i_isAllowed )
    {
        m_pImpl->m_bAllowEmptyPropertyName = i_isAllowed;
    }

    //--------------------------------------------------------------------
    namespace
    {
        void    lcl_checkForEmptyName( const bool _allowEmpty, const ::rtl::OUString& _name )
        {
            if ( !_allowEmpty && !_name.getLength() )
                throw IllegalArgumentException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The property name must not be empty." ) ),
                        // TODO: resource
                        NULL,
                        1
                      );
        }

        void    lcl_checkNameAndHandle( const ::rtl::OUString& _name, const sal_Int32 _handle, const PropertyBag& _container )
        {
            if ( _container.hasPropertyByName( _name ) || _container.hasPropertyByHandle( _handle ) )
                throw PropertyExistException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Property name or handle already used." ) ),
                    // TODO: resource
                    NULL );

        }
    }

    //--------------------------------------------------------------------
    void PropertyBag::addVoidProperty( const ::rtl::OUString& _rName, const Type& _rType, sal_Int32 _nHandle, sal_Int32 _nAttributes )
    {
        if ( _rType.getTypeClass() == TypeClass_VOID )
            throw IllegalArgumentException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Illegal property type: VOID" ) ),
                        // TODO: resource
                    NULL,
                    1
                  );

        // check name/handle sanity
        lcl_checkForEmptyName( m_pImpl->m_bAllowEmptyPropertyName, _rName );
        lcl_checkNameAndHandle( _rName, _nHandle, *this );

        // register the property
        OSL_ENSURE( _nAttributes & PropertyAttribute::MAYBEVOID, "PropertyBag::addVoidProperty: this is for default-void properties only!" );
        registerPropertyNoMember( _rName, _nHandle, _nAttributes | PropertyAttribute::MAYBEVOID, _rType, NULL );

        // remember the default
        m_pImpl->aDefaults.insert( MapInt2Any::value_type( _nHandle, Any() ) );
    }

    //--------------------------------------------------------------------
    void PropertyBag::addProperty( const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes, const Any& _rInitialValue )
    {
        // check type sanity
        Type aPropertyType = _rInitialValue.getValueType();
        if ( aPropertyType.getTypeClass() == TypeClass_VOID )
            throw IllegalTypeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The initial value must be non-NULL to determine the property type." ) ),
                // TODO: resource
                NULL );

        // check name/handle sanity
        lcl_checkForEmptyName( m_pImpl->m_bAllowEmptyPropertyName, _rName );
        lcl_checkNameAndHandle( _rName, _nHandle, *this );

        // register the property
        registerPropertyNoMember( _rName, _nHandle, _nAttributes, aPropertyType,
            _rInitialValue.hasValue() ? _rInitialValue.getValue() : NULL );

        // remember the default
        m_pImpl->aDefaults.insert( MapInt2Any::value_type( _nHandle, _rInitialValue ) );
    }

    //--------------------------------------------------------------------
    void PropertyBag::removeProperty( const ::rtl::OUString& _rName )
    {
        const Property& rProp = getProperty( _rName );
            // will throw an UnknownPropertyException if necessary
        if ( ( rProp.Attributes & PropertyAttribute::REMOVEABLE ) == 0 )
            throw NotRemoveableException( ::rtl::OUString(), NULL );
        const sal_Int32 nHandle = rProp.Handle;

        revokeProperty( nHandle );

        m_pImpl->aDefaults.erase( nHandle );
    }

    //--------------------------------------------------------------------
    void PropertyBag::getFastPropertyValue( sal_Int32 _nHandle, Any& _out_rValue ) const
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        OPropertyContainerHelper::getFastPropertyValue( _out_rValue, _nHandle );
    }

    //--------------------------------------------------------------------
    bool PropertyBag::convertFastPropertyValue( sal_Int32 _nHandle, const Any& _rNewValue, Any& _out_rConvertedValue, Any& _out_rCurrentValue ) const
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        return const_cast< PropertyBag*  >( this )->OPropertyContainerHelper::convertFastPropertyValue(
            _out_rConvertedValue, _out_rCurrentValue, _nHandle, _rNewValue );
    }

    //--------------------------------------------------------------------
    void PropertyBag::setFastPropertyValue( sal_Int32 _nHandle, const Any& _rValue )
    {
        if ( !hasPropertyByHandle( _nHandle ) )
            throw UnknownPropertyException();

        OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );
    }

    //--------------------------------------------------------------------
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


//........................................................................
} // namespace comphelper
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
