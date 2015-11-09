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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include "ftpcontentidentifier.hxx"
#include "ftpcontentprovider.hxx"

using namespace ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;


FTPContentIdentifier::FTPContentIdentifier(
    const OUString& ident
)
    : m_ident(ident)
{
}


FTPContentIdentifier::~FTPContentIdentifier()
{
}


Any SAL_CALL
FTPContentIdentifier::queryInterface(
    const Type& rType
)
    throw(
        RuntimeException, std::exception
    )
{
    Any aRet =
        ::cppu::queryInterface(rType,
                               (static_cast< XTypeProvider* >(this)),
                               (static_cast< XContentIdentifier* >(this)));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL FTPContentIdentifier::acquire() throw() {
    OWeakObject::acquire();
}


void SAL_CALL FTPContentIdentifier::release() throw() {
    OWeakObject::release();
}


Sequence<sal_Int8> SAL_CALL
FTPContentIdentifier::getImplementationId()
    throw(RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}


Sequence<Type> SAL_CALL
FTPContentIdentifier::getTypes(
    void )
    throw(RuntimeException, std::exception)
{
    static cppu::OTypeCollection* pCollection = NULL;
    if ( !pCollection ) {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<XTypeProvider>::get(),
                cppu::UnoType<XContentIdentifier>::get());
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}


OUString SAL_CALL
FTPContentIdentifier::getContentIdentifier(
)
    throw (
        css::uno::RuntimeException, std::exception
    )
{
    return m_ident;
}


OUString SAL_CALL
FTPContentIdentifier::getContentProviderScheme(
)
    throw (
        css::uno::RuntimeException, std::exception
    )
{
    return OUString("ftp");
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
