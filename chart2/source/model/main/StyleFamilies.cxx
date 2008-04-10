/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StyleFamilies.cxx,v $
 * $Revision: 1.4 $
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
#include "StyleFamilies.hxx"
#include "macros.hxx"

#include <algorithm>
#include <functional>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::osl::MutexGuard;

namespace chart
{

StyleFamilies::StyleFamilies()
{
}

StyleFamilies::~StyleFamilies()
{}

bool StyleFamilies::AddStyleFamily(
    const OUString & rName,
    const Reference< container::XNameAccess > & rFamily )
{
    if( m_aStyleFamilies.find( rName ) == m_aStyleFamilies.end() )
    {
        m_aStyleFamilies[ rName ] = rFamily;
        return true;
    }

    return false;
}

// container::XNameAccess
Any SAL_CALL StyleFamilies::getByName( const OUString& aName )
    throw (container::NoSuchElementException,
           lang::WrappedTargetException,
           RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );

    tStyleFamilyContainer::const_iterator aIt( m_aStyleFamilies.find( aName ));
    if( aIt == m_aStyleFamilies.end())
    {
        throw container::NoSuchElementException(
            aName,
            static_cast< ::cppu::OWeakObject* >( this ));
    }

    OSL_ASSERT( (*aIt).first.equals( aName ));
    return uno::makeAny( (*aIt).second );
    // \--
}

uno::Sequence< OUString > SAL_CALL StyleFamilies::getElementNames()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    Sequence< OUString > aResult( m_aStyleFamilies.size());

    ::std::transform( m_aStyleFamilies.begin(), m_aStyleFamilies.end(),
                      aResult.getArray(),
                      ::std::select1st< tStyleFamilyContainer::value_type >() );

    return aResult;
    // \--
}

sal_Bool SAL_CALL StyleFamilies::hasByName( const OUString& aName )
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );

    return ( m_aStyleFamilies.find( aName ) != m_aStyleFamilies.end() );
    // \--
}


// container::XIndexAccess (optional)
sal_Int32 SAL_CALL StyleFamilies::getCount()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    return m_aStyleFamilies.size();
    // \--
}

Any SAL_CALL StyleFamilies::getByIndex( sal_Int32 Index )
    throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    tStyleFamilyContainer::const_iterator aIt( m_aStyleFamilies.begin());
    sal_Int32 nCounter = 0;
    for( ;
         ( nCounter < Index ) && ( aIt != m_aStyleFamilies.end());
         ++nCounter, ++aIt );

    if( aIt == m_aStyleFamilies.end())
    {
        throw container::NoSuchElementException(
            OUString::valueOf( Index ),
            static_cast< ::cppu::OWeakObject* >( this ));
    }

    return uno::makeAny( (*aIt).second );
    // \--
}


// container::XElementAccess (base of XNameAccess and XIndexAccess)
uno::Type SAL_CALL StyleFamilies::getElementType()
    throw (RuntimeException)
{
    return ::getCppuType( reinterpret_cast< Reference< container::XNameAccess > * >(0));
}

sal_Bool SAL_CALL StyleFamilies::hasElements()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    return ! m_aStyleFamilies.empty();
    // \--
}

// lang::XServiceInfo

APPHELPER_XSERVICEINFO_IMPL( StyleFamilies, C2U( "com.sun.star.comp.chart2.StyleFamilies" ))

Sequence< OUString > StyleFamilies::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = C2U( "com.sun.star.style.StyleFamilies" );

    return aServices;
}

} //  namespace chart
