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
#include "precompiled_ucb.hxx"

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
    const rtl::OUString& ident
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
        RuntimeException
    )
{
    Any aRet =
        ::cppu::queryInterface(rType,
                               SAL_STATIC_CAST(XTypeProvider*,this),
                               SAL_STATIC_CAST(XContentIdentifier*,this));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL FTPContentIdentifier::acquire( void ) throw() {
    OWeakObject::acquire();
}


void SAL_CALL FTPContentIdentifier::release( void ) throw() {
    OWeakObject::release();
}


Sequence<sal_Int8> SAL_CALL
FTPContentIdentifier::getImplementationId()
    throw(RuntimeException)
{
    static cppu::OImplementationId* pId = NULL;
    if(!pId)
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


Sequence<Type> SAL_CALL
FTPContentIdentifier::getTypes(
    void )
    throw(RuntimeException)
{
    static cppu::OTypeCollection* pCollection = NULL;
    if ( !pCollection ) {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType(
                    static_cast<Reference<XTypeProvider>*>(0)),
                getCppuType(
                    static_cast<Reference<XContentIdentifier>*>(0)));
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}


rtl::OUString SAL_CALL
FTPContentIdentifier::getContentIdentifier(
)
    throw (
        com::sun::star::uno::RuntimeException
    )
{
    return m_ident;
}


rtl::OUString SAL_CALL
FTPContentIdentifier::getContentProviderScheme(
)
    throw (
        com::sun::star::uno::RuntimeException
    )
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ftp"));
}






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
