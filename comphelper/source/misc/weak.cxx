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


#include "comphelper/weak.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace comphelper
{

OWeakTypeObject::OWeakTypeObject()
{
}

OWeakTypeObject::~OWeakTypeObject()
{
}

Any SAL_CALL OWeakTypeObject::queryInterface(const Type & rType )   throw (RuntimeException, std::exception)
{
    if( rType == cppu::UnoType<XTypeProvider>::get() )
        return Any( Reference< XTypeProvider >(this) );
    else
        return ::cppu::OWeakObject::queryInterface( rType );
}

void SAL_CALL OWeakTypeObject::acquire()    throw ()
{
    ::cppu::OWeakObject::acquire();
}

void SAL_CALL OWeakTypeObject::release()    throw ()
{
    ::cppu::OWeakObject::release();
}

Sequence< Type > SAL_CALL OWeakTypeObject::getTypes(  ) throw (RuntimeException, std::exception)
{
    return Sequence< Type >();
}

Sequence< ::sal_Int8 > SAL_CALL OWeakTypeObject::getImplementationId(  ) throw (RuntimeException, std::exception)
{
    return Sequence< ::sal_Int8 >();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
