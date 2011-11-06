/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
