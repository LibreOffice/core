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
#include "precompiled_framework.hxx"
#include <helper/propertysetcontainer.hxx>
#include <threadhelp/resetableguard.hxx>

#include <vcl/svapp.hxx>

#define WRONG_TYPE_EXCEPTION    "Only XPropertSet allowed!"

using ::rtl::OUString;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace framework
{

PropertySetContainer::PropertySetContainer( const Reference< XMultiServiceFactory >& )
        :   ThreadHelpBase( &Application::GetSolarMutex() )
        ,   OWeakObject()

{
}


PropertySetContainer::~PropertySetContainer()
{
}


// XInterface
void SAL_CALL PropertySetContainer::acquire() throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL PropertySetContainer::release() throw ()
{
    OWeakObject::release();
}

Any SAL_CALL PropertySetContainer::queryInterface( const Type& rType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST( XIndexContainer*, this ),
                SAL_STATIC_CAST( XIndexReplace*, this ),
                SAL_STATIC_CAST( XIndexAccess*, this ),
                SAL_STATIC_CAST( XElementAccess*, this ) );

    if( a.hasValue() )
    {
        return a;
    }

    return OWeakObject::queryInterface( rType );
}

// XIndexContainer
void SAL_CALL PropertySetContainer::insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
    throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    sal_Int32 nSize = m_aPropertySetVector.size();

    if ( nSize >= Index )
    {
        Reference< XPropertySet > aPropertySetElement;

        if ( Element >>= aPropertySetElement )
        {
            if ( nSize == Index )
                m_aPropertySetVector.push_back( aPropertySetElement );
            else
            {
                PropertySetVector::iterator aIter = m_aPropertySetVector.begin();
                aIter += Index;
                m_aPropertySetVector.insert( aIter, aPropertySetElement );
            }
        }
        else
        {
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM( WRONG_TYPE_EXCEPTION )),
                (OWeakObject *)this, 2 );
        }
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

void SAL_CALL PropertySetContainer::removeByIndex( sal_Int32 Index )
    throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( (sal_Int32)m_aPropertySetVector.size() > Index )
    {
        PropertySetVector::iterator aIter = m_aPropertySetVector.begin();
        aIter += Index;
        m_aPropertySetVector.erase( aIter );
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XIndexReplace
void SAL_CALL PropertySetContainer::replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
    throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if ( (sal_Int32)m_aPropertySetVector.size() > Index )
    {
        Reference< XPropertySet > aPropertySetElement;

        if ( Element >>= aPropertySetElement )
        {
            m_aPropertySetVector[ Index ] = aPropertySetElement;
        }
        else
        {
            throw IllegalArgumentException(
                OUString( RTL_CONSTASCII_USTRINGPARAM( WRONG_TYPE_EXCEPTION )),
                (OWeakObject *)this, 2 );
        }
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XIndexAccess
sal_Int32 SAL_CALL PropertySetContainer::getCount()
    throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    return m_aPropertySetVector.size();
}

Any SAL_CALL PropertySetContainer::getByIndex( sal_Int32 Index )
    throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( (sal_Int32)m_aPropertySetVector.size() > Index )
    {
        Any a;

        a <<= m_aPropertySetVector[ Index ];
        return a;
    }
    else
        throw IndexOutOfBoundsException( OUString(), (OWeakObject *)this );
}

// XElementAccess
sal_Bool SAL_CALL PropertySetContainer::hasElements()
    throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    return !( m_aPropertySetVector.empty() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
