/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OIndexContainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:32:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "OIndexContainer.hxx"

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

using namespace ::com::sun::star;
using ::osl::MutexGuard;

namespace comphelper
{

OIndexContainer::OIndexContainer( ::osl::Mutex & rMutex ) :
        m_rMutex( rMutex )
{
}

OIndexContainer::~OIndexContainer()
{
}

::osl::Mutex & OIndexContainer::GetMutex()
{
    return m_rMutex;
}

// ____ XInterface ____
uno::Any SAL_CALL OIndexContainer::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    return ::cppu::queryInterface(
        aType,
        static_cast< container::XIndexContainer * >( this ),
        static_cast< container::XIndexReplace * >( this ),
        static_cast< container::XIndexAccess * >( this ),
        static_cast< container::XElementAccess * >( this ) );
}

// ____ XIndexContainer ____
void SAL_CALL OIndexContainer::insertByIndex( sal_Int32 Index, const uno::Any& Element )
    throw (lang::IllegalArgumentException,
           lang::IndexOutOfBoundsException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    // valid range [0..size]
    size_t nIndex( Index );

    // /--
    MutexGuard aGuard( GetMutex());

    if( nIndex < 0 ||
        nIndex > m_aContainer.size() )
        throw lang::IndexOutOfBoundsException();

    if( getElementType().isAssignableFrom( Element.getValueType()) )
        throw lang::IllegalArgumentException();

    if( nIndex == m_aContainer.size())
        m_aContainer.push_back( Element );
    else
    {
        m_aContainer.insert( m_aContainer.begin() + nIndex, Element );
    }
    // \--
}

void SAL_CALL OIndexContainer::removeByIndex( sal_Int32 Index )
    throw (lang::IndexOutOfBoundsException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    // valid range [0..size-1]
    size_t nIndex( Index );

    // /--
    MutexGuard aGuard( GetMutex());

    if( nIndex < 0 ||
        nIndex > m_aContainer.size() )
        throw lang::IndexOutOfBoundsException();

    m_aContainer.erase( m_aContainer.begin() + nIndex );
    // \--
}


// ____ XIndexReplace ____
void SAL_CALL OIndexContainer::replaceByIndex( sal_Int32 Index, const uno::Any& Element )
    throw (lang::IllegalArgumentException,
           lang::IndexOutOfBoundsException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    // valid range [0..size-1]
    size_t nIndex( Index );

    // /--
    MutexGuard aGuard( GetMutex());

    if( nIndex < 0 ||
        nIndex > m_aContainer.size() )
        throw lang::IndexOutOfBoundsException();

    if( getElementType().isAssignableFrom( Element.getValueType()) )
        throw lang::IllegalArgumentException();

    m_aContainer[ nIndex ] = Element;
    // \--
}


// ____ XIndexAccess ____
sal_Int32 SAL_CALL OIndexContainer::getCount()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());

    return m_aContainer.size();
    // \--
}

uno::Any SAL_CALL OIndexContainer::getByIndex( sal_Int32 Index )
    throw (lang::IndexOutOfBoundsException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    // valid range [0..size-1]
    size_t nIndex( Index );

    // /--
    MutexGuard aGuard( GetMutex());

    if( nIndex < 0 ||
        nIndex > m_aContainer.size() )
        throw lang::IndexOutOfBoundsException();

    return m_aContainer.at( nIndex );
    // \--
}

sal_Bool SAL_CALL OIndexContainer::hasElements()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());

    return ! m_aContainer.empty();
    // \--
}


} //  namespace comphelper
