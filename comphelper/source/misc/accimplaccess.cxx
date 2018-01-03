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

#include <comphelper/accimplaccess.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <cppuhelper/typeprovider.hxx>

#include <set>
#include <string.h>
#include <memory>


namespace comphelper
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::accessibility::XAccessible;

    OAccessibleImplementationAccess::OAccessibleImplementationAccess( )
    {
    }


    OAccessibleImplementationAccess::~OAccessibleImplementationAccess( )
    {
    }

    const Sequence< sal_Int8 > OAccessibleImplementationAccess::getUnoTunnelImplementationId()
    {
        static cppu::OImplementationId implID;

        return implID.getImplementationId();
    }


    sal_Int64 SAL_CALL OAccessibleImplementationAccess::getSomething( const Sequence< sal_Int8 >& _rIdentifier )
    {
        sal_Int64 nReturn( 0 );

        if  (   ( _rIdentifier.getLength() == 16 )
            &&  ( 0 == memcmp( getUnoTunnelImplementationId().getConstArray(), _rIdentifier.getConstArray(), 16 ) )
            )
            nReturn = reinterpret_cast< sal_Int64 >( this );

        return nReturn;
    }

}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
