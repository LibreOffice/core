/*************************************************************************
 *
 *  $RCSfile: StyleFamily.cxx,v $
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
#include "StyleFamily.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

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

StyleFamily::StyleFamily()
{}

StyleFamily::~StyleFamily()
{}

bool StyleFamily::AddStyle(
    const Reference< style::XStyle > & rStyle )
{
    bool bRet = false;

    if( rStyle.is())
    {
        OUString aName( rStyle->getName() );
        m_aStyles[ aName ] = rStyle;
        bRet = true;
    }

    return bRet;
}

// container::XNameAccess (also base of XNameReplace)
uno::Any SAL_CALL StyleFamily::getByName( const OUString& aName )
    throw (container::NoSuchElementException,
           lang::WrappedTargetException,
           RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );

    tStyleContainer::const_iterator aIt( m_aStyles.find( aName ));
    if( aIt == m_aStyles.end())
    {
        throw container::NoSuchElementException(
            aName,
            static_cast< ::cppu::OWeakObject* >( this ));
    }

    OSL_ASSERT( (*aIt).first.equals( aName ));
    return uno::makeAny( (*aIt).second );
    // \--
}

Sequence< OUString > SAL_CALL StyleFamily::getElementNames()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    Sequence< OUString > aResult( m_aStyles.size());

    ::std::transform( m_aStyles.begin(), m_aStyles.end(),
                      aResult.getArray(),
                      ::std::select1st< tStyleContainer::value_type >() );

    return aResult;
    // \--
}

sal_Bool SAL_CALL StyleFamily::hasByName( const OUString& aName )
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );

    return ( m_aStyles.find( aName ) != m_aStyles.end() );
    // \--
}


// container::XNameContainer (optional)
void SAL_CALL StyleFamily::insertByName( const OUString& aName, const uno::Any& aElement )
    throw (lang::IllegalArgumentException,
           container::ElementExistException,
           lang::WrappedTargetException,
           RuntimeException)
{
    uno::Reference< style::XStyle > xStyle;
    if( aElement >>= xStyle )
    {
        // /--
        MutexGuard aGuard( m_aMutex );
        if( m_aStyles.find( aName ) != m_aStyles.end() )
        {
            throw container::ElementExistException(
                aName,
                static_cast< ::cppu::OWeakObject* >( this ) );
        }

        m_aStyles[ aName ] = xStyle;
        // \--
    }
    else
    {
        throw lang::IllegalArgumentException(
            aName,
            static_cast< ::cppu::OWeakObject* >( this ),
            1 /* index */ );
    }
}

void SAL_CALL StyleFamily::removeByName( const OUString& Name )
    throw (container::NoSuchElementException,
           lang::WrappedTargetException,
           RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    tStyleContainer::iterator aIt( m_aStyles.find( Name ));
    if( aIt == m_aStyles.end())
    {
        throw container::NoSuchElementException(
            Name,
            static_cast< ::cppu::OWeakObject* >( this ) );
    }

    m_aStyles.erase( aIt );
    // \--
}


// container::XIndexAccess (optional)
sal_Int32 SAL_CALL StyleFamily::getCount()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    return m_aStyles.size();
    // \--
}

uno::Any SAL_CALL StyleFamily::getByIndex( sal_Int32 Index )
    throw (lang::IndexOutOfBoundsException,
           lang::WrappedTargetException,
           RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    tStyleContainer::const_iterator aIt( m_aStyles.begin());
    sal_Int32 nCounter = 0;
    for( ;
         ( nCounter < Index ) && ( aIt != m_aStyles.end());
         ++nCounter, ++aIt );

    if( aIt == m_aStyles.end())
    {
        throw container::NoSuchElementException(
            OUString::valueOf( Index ),
            static_cast< ::cppu::OWeakObject* >( this ));
    }

    return uno::makeAny( (*aIt).second );
    // \--
}


// container::XElementAccess (base of XNameAccess and XIndexAccess)
uno::Type SAL_CALL StyleFamily::getElementType()
    throw (RuntimeException)
{
    return ::getCppuType( reinterpret_cast< Reference< style::XStyle > * >(0));
}

sal_Bool SAL_CALL StyleFamily::hasElements()
    throw (RuntimeException)
{
    // /--
    MutexGuard aGuard( m_aMutex );
    return ! m_aStyles.empty();
    // \--
}


// container::XNameReplace
void SAL_CALL StyleFamily::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw (lang::IllegalArgumentException,
           container::NoSuchElementException,
           lang::WrappedTargetException,
           RuntimeException)
{
    uno::Reference< style::XStyle > xStyle;
    if( aElement >>= xStyle )
    {
        // /--
        MutexGuard aGuard( m_aMutex );
        tStyleContainer::const_iterator aIt( m_aStyles.begin());
        if( aIt == m_aStyles.end())
        {
            throw container::NoSuchElementException(
                aName,
                static_cast< ::cppu::OWeakObject* >( this ) );
        }

        m_aStyles[ aName ] = xStyle;
        // \--
    }
    else
    {
        throw lang::IllegalArgumentException(
            aName,
            static_cast< ::cppu::OWeakObject* >( this ),
            1 /* index */ );
    }
}

// lang::XServiceInfo

APPHELPER_XSERVICEINFO_IMPL( StyleFamily, C2U( "com.sun.star.comp.chart2.StyleFamily" ))

Sequence< OUString > StyleFamily::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 1 );
    aServices[ 0 ] = C2U( "com.sun.star.style.StyleFamily" );

    return aServices;
}

} //  namespace chart
