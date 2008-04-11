/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: namedvaluecollection.cxx,v $
 * $Revision: 1.10 $
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
#include <comphelper/namedvaluecollection.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
/** === end UNO includes === **/

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <hash_map>
#include <functional>
#include <algorithm>

//........................................................................
namespace comphelper
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::beans::NamedValue;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::cpp_acquire;
    using ::com::sun::star::uno::cpp_release;
    using ::com::sun::star::uno::cpp_queryInterface;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::NamedValue;
    using ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    /** === end UNO using === **/

    //====================================================================
    //= NamedValueCollection_Impl
    //====================================================================
    typedef ::std::hash_map< ::rtl::OUString, Any, ::rtl::OUStringHash >    NamedValueRepository;

    struct NamedValueCollection_Impl
    {
        NamedValueRepository    aValues;
    };

    //====================================================================
    //= NamedValueCollection
    //====================================================================
    //--------------------------------------------------------------------
    NamedValueCollection::NamedValueCollection()
        :m_pImpl( new NamedValueCollection_Impl )
    {
    }

    //--------------------------------------------------------------------
    NamedValueCollection::NamedValueCollection( const Any& _rElements )
        :m_pImpl( new NamedValueCollection_Impl )
    {
        Sequence< NamedValue > aNamedValues;
        Sequence< PropertyValue > aPropertyValues;
        NamedValue aNamedValue;
        PropertyValue aPropertyValue;

        if ( _rElements >>= aNamedValues )
            impl_assign( aNamedValues );
        else if ( _rElements >>= aPropertyValues )
            impl_assign( aPropertyValues );
        else if ( _rElements >>= aNamedValue )
            impl_assign( Sequence< NamedValue >( &aNamedValue, 1 ) );
        else if ( _rElements >>= aPropertyValue )
            impl_assign( Sequence< PropertyValue >( &aPropertyValue, 1 ) );
        else
            OSL_ENSURE( false, "NamedValueCollection::NamedValueCollection(Any): unsupported type!" );
    }

    //--------------------------------------------------------------------
    NamedValueCollection::NamedValueCollection( const Sequence< Any >& _rArguments )
        :m_pImpl( new NamedValueCollection_Impl )
    {
        impl_assign( _rArguments );
    }

    //--------------------------------------------------------------------
    NamedValueCollection::NamedValueCollection( const Sequence< PropertyValue >& _rArguments )
        :m_pImpl( new NamedValueCollection_Impl )
    {
        impl_assign( _rArguments );
    }

    //--------------------------------------------------------------------
    NamedValueCollection::NamedValueCollection( const Sequence< NamedValue >& _rArguments )
        :m_pImpl( new NamedValueCollection_Impl )
    {
        impl_assign( _rArguments );
    }

    //--------------------------------------------------------------------
    NamedValueCollection::~NamedValueCollection()
    {
    }

    //--------------------------------------------------------------------
    size_t NamedValueCollection::size() const
    {
        return m_pImpl->aValues.size();
    }

    //--------------------------------------------------------------------
    bool NamedValueCollection::empty() const
    {
        return m_pImpl->aValues.empty();
    }

    //--------------------------------------------------------------------
    void NamedValueCollection::impl_assign( const Sequence< Any >& _rArguments )
    {
        {
            NamedValueRepository aEmpty;
            m_pImpl->aValues.swap( aEmpty );
        }

        PropertyValue aPropertyValue;
        NamedValue aNamedValue;

        const Any* pArgument = _rArguments.getConstArray();
        const Any* pArgumentEnd = _rArguments.getConstArray() + _rArguments.getLength();
        for ( ; pArgument != pArgumentEnd; ++pArgument )
        {
            if ( *pArgument >>= aPropertyValue )
                m_pImpl->aValues[ aPropertyValue.Name ] = aPropertyValue.Value;
            else if ( *pArgument >>= aNamedValue )
                m_pImpl->aValues[ aNamedValue.Name ] = aNamedValue.Value;
            else
                OSL_ENSURE( !pArgument->hasValue(), "NamedValueCollection::impl_assign: encountered a value which I cannot handle!" );
        }
    }

    //--------------------------------------------------------------------
    void NamedValueCollection::impl_assign( const Sequence< PropertyValue >& _rArguments )
    {
        {
            NamedValueRepository aEmpty;
            m_pImpl->aValues.swap( aEmpty );
        }

        const PropertyValue* pArgument = _rArguments.getConstArray();
        const PropertyValue* pArgumentEnd = _rArguments.getConstArray() + _rArguments.getLength();
        for ( ; pArgument != pArgumentEnd; ++pArgument )
            m_pImpl->aValues[ pArgument->Name ] = pArgument->Value;
    }

    //--------------------------------------------------------------------
    void NamedValueCollection::impl_assign( const Sequence< NamedValue >& _rArguments )
    {
        {
            NamedValueRepository aEmpty;
            m_pImpl->aValues.swap( aEmpty );
        }

        const NamedValue* pArgument = _rArguments.getConstArray();
        const NamedValue* pArgumentEnd = _rArguments.getConstArray() + _rArguments.getLength();
        for ( ; pArgument != pArgumentEnd; ++pArgument )
            m_pImpl->aValues[ pArgument->Name ] = pArgument->Value;
    }

    //--------------------------------------------------------------------
    bool NamedValueCollection::get_ensureType( const ::rtl::OUString& _rValueName, void* _pValueLocation, const Type& _rExpectedValueType ) const
    {
        NamedValueRepository::const_iterator pos = m_pImpl->aValues.find( _rValueName );
        if ( pos != m_pImpl->aValues.end() )
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
            ::rtl::OUStringBuffer aBuffer;
            aBuffer.appendAscii( "Invalid value type for '" );
            aBuffer.append     ( _rValueName );
            aBuffer.appendAscii( "'.\nExpected: " );
            aBuffer.append     ( _rExpectedValueType.getTypeName() );
            aBuffer.appendAscii( "\nFound: " );
            aBuffer.append     ( pos->second.getValueType().getTypeName() );
            throw IllegalArgumentException( aBuffer.makeStringAndClear(), NULL, 0 );
        }

        // argument does not exist
        return false;
    }

    //--------------------------------------------------------------------
    const Any& NamedValueCollection::impl_get( const ::rtl::OUString& _rValueName ) const
    {
        NamedValueRepository::const_iterator pos = m_pImpl->aValues.find( _rValueName );
        if ( pos != m_pImpl->aValues.end() )
            return pos->second;

        static Any aEmptyDefault;
        return aEmptyDefault;
    }

    //--------------------------------------------------------------------
    bool NamedValueCollection::impl_has( const ::rtl::OUString& _rValueName ) const
    {
        NamedValueRepository::const_iterator pos = m_pImpl->aValues.find( _rValueName );
        return ( pos != m_pImpl->aValues.end() );
    }

    //--------------------------------------------------------------------
    bool NamedValueCollection::impl_put( const ::rtl::OUString& _rValueName, const Any& _rValue )
    {
        bool bHas = impl_has( _rValueName );
        m_pImpl->aValues[ _rValueName ] = _rValue;
        return bHas;
    }

    //--------------------------------------------------------------------
    bool NamedValueCollection::impl_remove( const ::rtl::OUString& _rValueName )
    {
        NamedValueRepository::iterator pos = m_pImpl->aValues.find( _rValueName );
        if ( pos == m_pImpl->aValues.end() )
            return false;
        m_pImpl->aValues.erase( pos );
        return true;
    }

    //--------------------------------------------------------------------
    namespace
    {
        struct Value2PropertyValue : public ::std::unary_function< NamedValueRepository::value_type, PropertyValue >
        {
            PropertyValue operator()( const NamedValueRepository::value_type& _rValue )
            {
                return PropertyValue(
                    _rValue.first, 0, _rValue.second, PropertyState_DIRECT_VALUE );
            }
        };

        struct Value2NamedValue : public ::std::unary_function< NamedValueRepository::value_type, NamedValue >
        {
            NamedValue operator()( const NamedValueRepository::value_type& _rValue )
            {
                return NamedValue( _rValue.first, _rValue.second );
            }
        };
    }

    //--------------------------------------------------------------------
    sal_Int32 NamedValueCollection::operator >>= ( Sequence< PropertyValue >& _out_rValues ) const
    {
        _out_rValues.realloc( m_pImpl->aValues.size() );
        ::std::transform( m_pImpl->aValues.begin(), m_pImpl->aValues.end(), _out_rValues.getArray(), Value2PropertyValue() );
        return _out_rValues.getLength();
    }

    //--------------------------------------------------------------------
    sal_Int32 NamedValueCollection::operator >>= ( Sequence< NamedValue >& _out_rValues ) const
    {
        _out_rValues.realloc( m_pImpl->aValues.size() );
        ::std::transform( m_pImpl->aValues.begin(), m_pImpl->aValues.end(), _out_rValues.getArray(), Value2NamedValue() );
        return _out_rValues.getLength();
    }

//........................................................................
} // namespace comphelper
//........................................................................

