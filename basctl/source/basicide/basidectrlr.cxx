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

#include "basidectrlr.hxx"

#include "basidesh.hxx"

#include <cppuhelper/queryinterface.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <vcl/syswin.hxx>

namespace basctl
{

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


#define PROPERTY_ID_ICONID      1
#define PROPERTY_ICONID         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IconId" ) )


//----------------------------------------------------------------------------

Controller::Controller (Shell* pViewShell)
    :OPropertyContainer( m_aBHelper )
    ,SfxBaseController( pViewShell )
    ,m_nIconId( ICON_MACROLIBRARY )
{
    registerProperty( PROPERTY_ICONID, PROPERTY_ID_ICONID, PropertyAttribute::READONLY, &m_nIconId, ::getCppuType( &m_nIconId ) );
}

//----------------------------------------------------------------------------

Controller::~Controller()
{ }

// XInterface
//----------------------------------------------------------------------------

Any SAL_CALL Controller::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aReturn = SfxBaseController::queryInterface( rType );
    if ( !aReturn.hasValue() )
        aReturn = OPropertyContainer::queryInterface( rType );

    return aReturn;
}

//----------------------------------------------------------------------------

void SAL_CALL Controller::acquire() throw()
{
    SfxBaseController::acquire();
}

//----------------------------------------------------------------------------

void SAL_CALL Controller::release() throw()
{
    SfxBaseController::release();
}


// XTypeProvider ( ::SfxBaseController )
//----------------------------------------------------------------------------

Sequence< Type > SAL_CALL Controller::getTypes() throw(RuntimeException)
{
    Sequence< Type > aTypes = ::comphelper::concatSequences(
        SfxBaseController::getTypes(),
        OPropertyContainer::getTypes()
    );

    return aTypes;
}

//----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL Controller::getImplementationId() throw(RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if ( !pId )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pId )
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// XPropertySet
//----------------------------------------------------------------------------

Reference< beans::XPropertySetInfo > SAL_CALL Controller::getPropertySetInfo() throw(RuntimeException)
{
    Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// OPropertySetHelper
//----------------------------------------------------------------------------

::cppu::IPropertyArrayHelper& Controller::getInfoHelper()
{
    return *getArrayHelper();
}

// OPropertyArrayUsageHelper
//----------------------------------------------------------------------------

::cppu::IPropertyArrayHelper* Controller::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

//----------------------------------------------------------------------------

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
