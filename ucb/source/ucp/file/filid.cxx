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
