/*************************************************************************
 *
 *  $RCSfile: StyleFamilies.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
