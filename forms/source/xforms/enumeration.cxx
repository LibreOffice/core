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


#include "enumeration.hxx"

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XIndexAccess;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;


Enumeration::Enumeration( XIndexAccess* pContainer )
    : mxContainer( pContainer ),
      mnIndex( 0 )
{
    OSL_ENSURE( mxContainer.is(), "no container?" );
}

sal_Bool Enumeration::hasMoreElements()
    throw( RuntimeException, std::exception )
{
    if( ! mxContainer.is() )
        throw RuntimeException();

    return mnIndex < mxContainer->getCount();
}

Any Enumeration::nextElement()
    throw( NoSuchElementException,
           WrappedTargetException,
           RuntimeException, std::exception )
{
    if( ! mxContainer.is() )
        throw RuntimeException();
    if( mnIndex >= mxContainer->getCount() )
        throw NoSuchElementException();

    return mxContainer->getByIndex( mnIndex++ );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
