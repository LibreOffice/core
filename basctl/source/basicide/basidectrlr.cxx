/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basidectrlr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:57:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BASCTL_BASIDECTRLR_HXX_
#include <basidectrlr.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

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
