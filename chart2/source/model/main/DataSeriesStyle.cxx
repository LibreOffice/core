/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataSeriesStyle.cxx,v $
 * $Revision: 1.6 $
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
#include "DataSeriesStyle.hxx"
#include "DataSeriesProperties.hxx"
#include "CharacterProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

// ____________________________________________________________

namespace
{
const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::DataSeriesProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}
} // anonymous namespace

// ____________________________________________________________

namespace chart
{

DataSeriesStyle::DataSeriesStyle(
    const uno::Reference< container::XNameAccess > & xStyleFamily,
    ::osl::Mutex & par_rMutex ) :
        ::property::OStyle( xStyleFamily, par_rMutex ),
    m_rMutex( par_rMutex )
{}

DataSeriesStyle::~DataSeriesStyle()
{}

// ____ XInterface ____
uno::Any SAL_CALL DataSeriesStyle::queryInterface(
    const uno::Type & rType )
    throw (uno::RuntimeException)
{
    return ::cppu::OWeakObject::queryInterface( rType );
}

void SAL_CALL DataSeriesStyle::acquire() throw ()
{
    ::cppu::OWeakObject::acquire();
}

void SAL_CALL DataSeriesStyle::release() throw ()
{
    ::cppu::OWeakObject::release();
}

// ____ OPropertySet ____
uno::Any DataSeriesStyle::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        DataSeriesProperties::AddDefaultsToMap( aStaticDefaults );
        CharacterProperties::AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        throw beans::UnknownPropertyException();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL DataSeriesStyle::getInfoHelper()
{
    return getInfoHelperConst();
}

::cppu::IPropertyArrayHelper & SAL_CALL DataSeriesStyle::getInfoHelperConst() const
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL
    DataSeriesStyle::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static Reference< beans::XPropertySetInfo > xInfo;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

::osl::Mutex & DataSeriesStyle::GetMutex()
{
    return m_rMutex;
}

} //  namespace chart
