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

PropertySetContainer::PropertySetContainer()
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
                (static_cast< XIndexContainer* >(this)),
                (static_cast< XIndexReplace* >(this)),
                (static_cast< XIndexAccess* >(this)),
                (static_cast< XElementAccess* >(this)) );

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
                OUString( WRONG_TYPE_EXCEPTION ),
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
                OUString( WRONG_TYPE_EXCEPTION ),
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
