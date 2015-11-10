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

#include <cppuhelper/queryinterface.hxx>

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


FileContentIdentifier::FileContentIdentifier(
    const OUString& aUnqPath,
    bool IsNormalized )
{
    if( IsNormalized )
    {
        fileaccess::shell::getUrlFromUnq( aUnqPath,m_aContentId );
        m_aNormalizedId = aUnqPath;
        shell::getScheme( m_aProviderScheme );
    }
    else
    {
        fileaccess::shell::getUnqFromUrl( aUnqPath,m_aNormalizedId );
        m_aContentId = aUnqPath;
        shell::getScheme( m_aProviderScheme );
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
    throw( uno::RuntimeException, std::exception )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          (static_cast< lang::XTypeProvider* >(this)),
                                          (static_cast< XContentIdentifier* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


uno::Sequence< sal_Int8 > SAL_CALL
FileContentIdentifier::getImplementationId()
    throw( uno::RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}


uno::Sequence< uno::Type > SAL_CALL
FileContentIdentifier::getTypes(
    void )
    throw( uno::RuntimeException, std::exception )
{
    static cppu::OTypeCollection* pCollection = nullptr;
    if ( !pCollection ) {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<XContentIdentifier>::get() );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}


OUString
SAL_CALL
FileContentIdentifier::getContentIdentifier(
    void )
    throw( uno::RuntimeException, std::exception )
{
    return m_aContentId;
}


OUString SAL_CALL
FileContentIdentifier::getContentProviderScheme(
    void )
    throw( uno::RuntimeException, std::exception )
{
    return m_aProviderScheme;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
