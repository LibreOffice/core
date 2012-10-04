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
    Any aRet = ::cppu::queryInterface(rType, (static_cast< XMultiServiceFactory* >( this )) );

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
