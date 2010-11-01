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
#include "test_multiservicefac.hxx"


using namespace test_ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


Any SAL_CALL
Test_MultiServiceFactory::queryInterface(
    const Type& rType
)
    throw(
        RuntimeException
    )
{
    Any aRet = ::cppu::queryInterface(rType,
                                      SAL_STATIC_CAST( XMultiServiceFactory*,
                                                       this ));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );

}


void SAL_CALL Test_MultiServiceFactory::acquire( void ) throw()
{
    OWeakObject::acquire();
}


void SAL_CALL Test_MultiServiceFactory::release( void ) throw()
{
    OWeakObject::release();
}

        // XMultiServiceFactory

 Reference<
XInterface > SAL_CALL
Test_MultiServiceFactory::createInstance(
    const ::rtl::OUString& aServiceSpecifier
)
    throw (
        Exception,
        RuntimeException
    )
{
    return Reference<
        XInterface >(0);
}


Reference<
XInterface > SAL_CALL
Test_MultiServiceFactory::createInstanceWithArguments(
    const ::rtl::OUString& ServiceSpecifier,
    const Sequence
    < Any >& Arguments
)
    throw (
        Exception,
        RuntimeException
    )
{
    return Reference<
        XInterface >(0);
}

Sequence< ::rtl::OUString > SAL_CALL
Test_MultiServiceFactory::getAvailableServiceNames(
)
    throw (
        RuntimeException
    )
{
    return Sequence< ::rtl::OUString >(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
