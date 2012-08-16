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

#include "filid.hxx"
#include "shell.hxx"

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


FileContentIdentifier::FileContentIdentifier(
    shell* pMyShell,
    const rtl::OUString& aUnqPath,
    sal_Bool IsNormalized )
    : m_pMyShell( pMyShell )
{
    if( IsNormalized )
    {
        m_pMyShell->getUrlFromUnq( aUnqPath,m_aContentId );
        m_aNormalizedId = aUnqPath;
        m_pMyShell->getScheme( m_aProviderScheme );
    }
    else
    {
        m_pMyShell->getUnqFromUrl( aUnqPath,m_aNormalizedId );
        m_aContentId = aUnqPath;
        m_pMyShell->getScheme( m_aProviderScheme );
    }
}

FileContentIdentifier::~FileContentIdentifier()
{
}


void SAL_CALL
FileContentIdentifier::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
FileContentIdentifier::release(
                void )
  throw()
{
  OWeakObject::release();
}


uno::Any SAL_CALL
FileContentIdentifier::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          (static_cast< lang::XTypeProvider* >(this)),
                                          (static_cast< XContentIdentifier* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


uno::Sequence< sal_Int8 > SAL_CALL
FileContentIdentifier::getImplementationId()
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
    if ( !pId )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pId )
        {
            static cppu::OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return (*pId).getImplementationId();
}


uno::Sequence< uno::Type > SAL_CALL
FileContentIdentifier::getTypes(
    void )
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = NULL;
    if ( !pCollection ) {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast< uno::Reference< lang::XTypeProvider >* >( 0 ) ),
                getCppuType( static_cast< uno::Reference< XContentIdentifier >* >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}


rtl::OUString
SAL_CALL
FileContentIdentifier::getContentIdentifier(
    void )
    throw( uno::RuntimeException )
{
    return m_aContentId;
}


rtl::OUString SAL_CALL
FileContentIdentifier::getContentProviderScheme(
    void )
    throw( uno::RuntimeException )
{
    return m_aProviderScheme;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
