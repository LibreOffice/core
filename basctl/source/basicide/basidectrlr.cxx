/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basidectrlr.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_basctl.hxx"

#ifndef _BASCTL_BASIDECTRLR_HXX_
#include <basidectrlr.hxx>
#endif
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <vcl/syswin.hxx>

#include <basidesh.hxx>


using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


#define PROPERTY_ID_ICONID      1
#define PROPERTY_ICONID         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IconId" ) )


//----------------------------------------------------------------------------

BasicIDEController::BasicIDEController( BasicIDEShell* pViewShell )
    :OPropertyContainer( m_aBHelper )
    ,SfxBaseController( pViewShell )
    ,m_nIconId( ICON_MACROLIBRARY )
{
    registerProperty( PROPERTY_ICONID, PROPERTY_ID_ICONID, PropertyAttribute::READONLY, &m_nIconId, ::getCppuType( &m_nIconId ) );
}

//----------------------------------------------------------------------------

BasicIDEController::~BasicIDEController()
{
}

// XInterface
//----------------------------------------------------------------------------

Any SAL_CALL BasicIDEController::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aReturn = SfxBaseController::queryInterface( rType );
    if ( !aReturn.hasValue() )
        aReturn = OPropertyContainer::queryInterface( rType );

    return aReturn;
}

//----------------------------------------------------------------------------

void SAL_CALL BasicIDEController::acquire() throw()
{
    SfxBaseController::acquire();
}

//----------------------------------------------------------------------------

void SAL_CALL BasicIDEController::release() throw()
{
    SfxBaseController::release();
}


// XTypeProvider ( ::SfxBaseController )
//----------------------------------------------------------------------------

Sequence< Type > SAL_CALL BasicIDEController::getTypes() throw(RuntimeException)
{
    Sequence< Type > aTypes = ::comphelper::concatSequences(
        SfxBaseController::getTypes(),
        OPropertyContainer::getTypes()
        );

    return aTypes;
}

//----------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL BasicIDEController::getImplementationId() throw(RuntimeException)
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

Reference< beans::XPropertySetInfo > SAL_CALL BasicIDEController::getPropertySetInfo() throw(RuntimeException)
{
    Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// OPropertySetHelper
//----------------------------------------------------------------------------

::cppu::IPropertyArrayHelper& BasicIDEController::getInfoHelper()
{
    return *getArrayHelper();
}

// OPropertyArrayUsageHelper
//----------------------------------------------------------------------------

::cppu::IPropertyArrayHelper* BasicIDEController::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

//----------------------------------------------------------------------------
