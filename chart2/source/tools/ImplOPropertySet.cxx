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
#include "precompiled_chart2.hxx"
#include "ImplOPropertySet.hxx"
#include "CloneHelper.hxx"

#include <algorithm>
#include <functional>
#include <com/sun/star/beans/XFastPropertySet.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

namespace
{

struct lcl_getPropertyStateByHandle :
        public ::std::unary_function< sal_Int32,  beans::PropertyState >
{
    lcl_getPropertyStateByHandle(
        const ::property::impl::ImplOPropertySet::tPropertyMap & rMap )
            : m_rMap( rMap )
    {}

    inline beans::PropertyState operator() ( sal_Int32 nHandle )
    {
        if( m_rMap.end() == m_rMap.find( nHandle ))
            return beans::PropertyState_DEFAULT_VALUE;
        return beans::PropertyState_DIRECT_VALUE;
    }

private:
    const ::property::impl::ImplOPropertySet::tPropertyMap & m_rMap;
};

template< typename K, typename V >
struct lcl_eraseMapEntry :
        public ::std::unary_function< K, void >
{
    lcl_eraseMapEntry( ::std::map< K, V > & rMap )
            : m_rMap( rMap )
    {}

    inline void operator() ( const K & aKey )
    {
        m_rMap.erase( aKey );
    }

private:
    ::std::map< K, V > m_rMap;
};

struct lcl_replaceInterfacePropertiesByClones :
    public ::std::unary_function< ::property::impl::ImplOPropertySet::tPropertyMap::value_type, void >
{
    inline void operator() ( ::property::impl::ImplOPropertySet::tPropertyMap::value_type & rProp )
    {
        if( rProp.second.hasValue() &&
            rProp.second.getValueType().getTypeClass() == uno::TypeClass_INTERFACE )
        {
            Reference< util::XCloneable > xCloneable;
            if( rProp.second >>= xCloneable )
                rProp.second <<= xCloneable->createClone();
        }
    }
};

} //  anonymous namespace

namespace property
{
namespace impl
{

ImplOPropertySet::ImplOPropertySet()
{}

ImplOPropertySet::ImplOPropertySet( const ImplOPropertySet & rOther )
{
    ::std::copy( rOther.m_aProperties.begin(), rOther.m_aProperties.end(),
                 ::std::inserter( m_aProperties, m_aProperties.begin() ));
    cloneInterfaceProperties();
    m_xStyle.set( ::chart::CloneHelper::CreateRefClone< Reference< style::XStyle > >()( rOther.m_xStyle ));
}

beans::PropertyState ImplOPropertySet::GetPropertyStateByHandle( sal_Int32 nHandle ) const
{
    return lcl_getPropertyStateByHandle( m_aProperties ) ( nHandle );
}

Sequence< beans::PropertyState > ImplOPropertySet::GetPropertyStatesByHandle(
    const ::std::vector< sal_Int32 > & aHandles ) const
{
    Sequence< beans::PropertyState > aResult( aHandles.size());

    ::std::transform( aHandles.begin(), aHandles.end(),
                      aResult.getArray(),
                      lcl_getPropertyStateByHandle( m_aProperties ));

    return aResult;
}

void ImplOPropertySet::SetPropertyToDefault( sal_Int32 nHandle )
{
    tPropertyMap::iterator aFoundIter( m_aProperties.find( nHandle ) );

    if( m_aProperties.end() != aFoundIter )
    {
        m_aProperties.erase( aFoundIter );
    }
}

void ImplOPropertySet::SetPropertiesToDefault(
    const ::std::vector< sal_Int32 > & aHandles )
{
    ::std::for_each( aHandles.begin(), aHandles.end(),
                     lcl_eraseMapEntry< sal_Int32, Any >( m_aProperties ) );
}

void ImplOPropertySet::SetAllPropertiesToDefault()
{
    m_aProperties.clear();
}

bool ImplOPropertySet::GetPropertyValueByHandle(
    Any & rValue,
    sal_Int32 nHandle ) const
{
    bool bResult = false;

    tPropertyMap::const_iterator aFoundIter( m_aProperties.find( nHandle ) );

    if( m_aProperties.end() != aFoundIter )
    {
        rValue = (*aFoundIter).second;
        bResult = true;
    }

    return bResult;
}

void ImplOPropertySet::SetPropertyValueByHandle(
    sal_Int32 nHandle, const Any & rValue, Any * pOldValue )
{
    if( pOldValue != NULL )
    {
        tPropertyMap::const_iterator aFoundIter( m_aProperties.find( nHandle ) );
        if( m_aProperties.end() != aFoundIter )
            (*pOldValue) = (*aFoundIter).second;
    }

    m_aProperties[ nHandle ] = rValue;
}

bool ImplOPropertySet::SetStyle( const Reference< style::XStyle > & xStyle )
{
    if( ! xStyle.is())
        return false;

    m_xStyle = xStyle;
    return true;
}

Reference< style::XStyle > ImplOPropertySet::GetStyle() const
{
    return m_xStyle;
}

void ImplOPropertySet::cloneInterfaceProperties()
{
    ::std::for_each( m_aProperties.begin(), m_aProperties.end(),
                     lcl_replaceInterfacePropertiesByClones());
}


} //  namespace impl
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
