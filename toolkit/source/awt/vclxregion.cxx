/*************************************************************************
 *
 *  $RCSfile: vclxregion.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>
#include <vcl/svapp.hxx>

//  ----------------------------------------------------
//  class VCLXRegion
//  ----------------------------------------------------
VCLXRegion::VCLXRegion()
{
}

VCLXRegion::~VCLXRegion()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXRegion::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XRegion*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XUnoTunnel*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXRegion )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXRegion )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion>* ) NULL )
IMPL_XTYPEPROVIDER_END



::com::sun::star::awt::Rectangle VCLXRegion::getBounds() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return AWTRectangle( maRegion.GetBoundRect() );
}

void VCLXRegion::clear() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.SetEmpty();
}

void VCLXRegion::move( sal_Int32 nHorzMove, sal_Int32 nVertMove ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Move( nHorzMove, nVertMove );
}

void VCLXRegion::unionRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Union( VCLRectangle( rRect ) );
}

void VCLXRegion::intersectRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Intersect( VCLRectangle( rRect ) );
}

void VCLXRegion::excludeRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.Exclude( VCLRectangle( rRect ) );
}

void VCLXRegion::xOrRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    maRegion.XOr( VCLRectangle( rRect ) );
}

void VCLXRegion::unionRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Union( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::intersectRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Intersect( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::excludeRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.Exclude( VCLUnoHelper::GetRegion( rxRegion ) );
}

void VCLXRegion::xOrRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( rxRegion.is() )
        maRegion.XOr( VCLUnoHelper::GetRegion( rxRegion ) );
}

::com::sun::star::uno::Sequence< ::com::sun::star::awt::Rectangle > VCLXRegion::getRectangles() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ULONG nRects = maRegion.GetRectCount();
    ::com::sun::star::uno::Sequence< ::com::sun::star::awt::Rectangle > aRects( nRects );

    Rectangle aRect;
    sal_uInt32 nR = 0;
    RegionHandle h = maRegion.BeginEnumRects();
    while ( maRegion.GetNextEnumRect( h, aRect ) )
        aRects.getArray()[nR++] = AWTRectangle( aRect );
    maRegion.EndEnumRects( h );

    return aRects;
}



