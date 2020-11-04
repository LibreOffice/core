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

#include <comphelper/namedvaluecollection.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <rtl/instance.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <unordered_map>

namespace comphelper
{


    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::beans::NamedValue;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::cpp_acquire;
    using ::com::sun::star::uno::cpp_release;
    using ::com::sun::star::uno::cpp_queryInterface;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::PropertyState_DIRECT_VALUE;

    NamedValueCollection::NamedValueCollection( const Any& _rElements )
    {
        impl_assign( _rElements );
    }


    NamedValueCollection::NamedValueCollection( const Sequence< Any >& _rArguments )
    {
        impl_assign( _rArguments );
    }


    NamedValueCollection::NamedValueCollection( const Sequence< PropertyValue >& _rArguments )
    {
        impl_assign( _rArguments );
    }


    NamedValueCollection::NamedValueCollection( const Sequence< NamedValue >& _rArguments )
    {
        impl_assign( _rArguments );
    }


    bool NamedValueCollection::canExtractFrom( css::uno::Any const & i_value )
    {
        Type const & aValueType = i_value.getValueType();
        return aValueType.equals( ::cppu::UnoType< PropertyValue >::get() )
            ||  aValueType.equals( ::cppu::UnoType< NamedValue >::get() )
            ||  aValueType.equals( ::cppu::UnoType< Sequence< PropertyValue > >::get() )
            ||  aValueType.equals( ::cppu::UnoType< Sequence< NamedValue > >::get() );
    }


    NamedValueCollection& NamedValueCollection::merge( const NamedValueCollection& _rAdditionalValues, bool _bOverwriteExisting )
    {
        for (auto const& value : _rAdditionalValues.maValues)
        {
            if ( _bOverwriteExisting || !impl_has( value.first ) )
                impl_put( value.first, value.second );
        }

        return *this;
    }


    size_t NamedValueCollection::size() const
    {
        return maValues.size();
    }


    bool NamedValueCollection::empty() const
    {
        return maValues.empty();
    }


    std::vector< OUString > NamedValueCollection::getNames() const
    {
        std::vector< OUString > aNames;
        for (auto const& value : maValues)
        {
            aNames.push_back( value.first );
        }
        return aNames;
    }


    void NamedValueCollection::impl_assign( const Any& i_rWrappedElements )
    {
        Sequence< NamedValue > aNamedValues;
        Sequence< PropertyValue > aPropertyValues;
        NamedValue aNamedValue;
        PropertyValue aPropertyValue;

        if ( i_rWrappedElements >>= aNamedValues )
            impl_assign( aNamedValues );
        else if ( i_rWrappedElements >>= aPropertyValues )
            impl_assign( aPropertyValues );
        else if ( i_rWrappedElements >>= aNamedValue )
            impl_assign( Sequence< NamedValue >( &aNamedValue, 1 ) );
        else if ( i_rWrappedElements >>= aPropertyValue )
            impl_assign( Sequence< PropertyValue >( &aPropertyValue, 1 ) );
        else
            SAL_WARN_IF( i_rWrappedElements.hasValue(), "comphelper", "NamedValueCollection::impl_assign(Any): unsupported type!" );
    }


    void NamedValueCollection::impl_assign( const Sequence< Any >& _rArguments )
    {
        maValues.clear();

        PropertyValue aPropertyValue;
        NamedValue aNamedValue;

        for ( auto const & argument : _rArguments )
        {
            if ( argument >>= aPropertyValue )
                maValues[ aPropertyValue.Name ] = aPropertyValue.Value;
            else if ( argument >>= aNamedValue )
                maValues[ aNamedValue.Name ] = aNamedValue.Value;
            else
            {
                SAL_WARN_IF(
                    argument.hasValue(), "comphelper",
                    ("NamedValueCollection::impl_assign: encountered a value"
                     " type which I cannot handle: "
                     + argument.getValueTypeName()));
            }
        }
    }


    void NamedValueCollection::impl_assign( const Sequence< PropertyValue >& _rArguments )
    {
        maValues.clear();

        for ( auto const & argument : _rArguments )
            maValues[ argument.Name ] = argument.Value;
    }


    void NamedValueCollection::impl_assign( const Sequence< NamedValue >& _rArguments )
    {
        maValues.clear();

        for ( auto const & argument : _rArguments )
            maValues[ argument.Name ] = argument.Value;
    }


    bool NamedValueCollection::get_ensureType( const OUString& _rValueName, void* _pValueLocation, const Type& _rExpectedValueType ) const
    {
        auto pos = maValues.find( _rValueName );
        if ( pos != maValues.end() )
        {
            if ( uno_type_assignData(
                    _pValueLocation, _rExpectedValueType.getTypeLibType(),
                    const_cast< void* >( pos->second.getValue() ), pos->second.getValueType().getTypeLibType(),
                    reinterpret_cast< uno_QueryInterfaceFunc >( cpp_queryInterface ),
                    reinterpret_cast< uno_AcquireFunc >( cpp_acquire ),
                    reinterpret_cast< uno_ReleaseFunc >( cpp_release )
                ) )
                // argument exists, and could be extracted
                return true;

            // argument exists, but is of wrong type
            throw IllegalArgumentException(
                "Invalid value type for '" + _rValueName
                + "'.\nExpected: " + _rExpectedValueType.getTypeName()
                + "\nFound: " + pos->second.getValueType().getTypeName(),
                nullptr, 0 );
        }

        // argument does not exist
        return false;
    }

    namespace
    {
        class theEmptyDefault : public rtl::Static<Any, theEmptyDefault> {};
    }


    const Any& NamedValueCollection::impl_get( const OUString& _rValueName ) const
    {
        auto pos = maValues.find( _rValueName );
        if ( pos != maValues.end() )
            return pos->second;

        return theEmptyDefault::get();
    }


    bool NamedValueCollection::impl_has( const OUString& _rValueName ) const
    {
        auto pos = maValues.find( _rValueName );
        return ( pos != maValues.end() );
    }


    bool NamedValueCollection::impl_put( const OUString& _rValueName, const Any& _rValue )
    {
        bool bHas = impl_has( _rValueName );
        maValues[ _rValueName ] = _rValue;
        return bHas;
    }


    bool NamedValueCollection::impl_remove( const OUString& _rValueName )
    {
        auto pos = maValues.find( _rValueName );
        if ( pos == maValues.end() )
            return false;
        maValues.erase( pos );
        return true;
    }


    sal_Int32 NamedValueCollection::operator >>= ( Sequence< PropertyValue >& _out_rValues ) const
    {
        _out_rValues.realloc( maValues.size() );
        std::transform( maValues.begin(), maValues.end(), _out_rValues.getArray(),
                [](const std::pair< OUString, css::uno::Any >& _rValue)
                    { return PropertyValue( _rValue.first, 0, _rValue.second, PropertyState_DIRECT_VALUE ); } );
        return _out_rValues.getLength();
    }


    sal_Int32 NamedValueCollection::operator >>= ( Sequence< NamedValue >& _out_rValues ) const
    {
        _out_rValues.realloc( maValues.size() );
        std::transform( maValues.begin(), maValues.end(), _out_rValues.getArray(),
                [](const std::pair< OUString, css::uno::Any >& _rValue)
                    { return NamedValue( _rValue.first, _rValue.second ); } );
        return _out_rValues.getLength();
    }


} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
