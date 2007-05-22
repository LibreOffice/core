/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CartesianCoordinateSystem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:28:28 $
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
#ifndef _CHART_CARTESIANCOORDINATESYSTEM_HXX
#define _CHART_CARTESIANCOORDINATESYSTEM_HXX

#include "ServiceMacros.hxx"
#include "BaseCoordinateSystem.hxx"

namespace chart
{

class CartesianCoordinateSystem : public BaseCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        sal_Int32 nDimensionCount = 2,
        sal_Bool bSwapXAndYAxis = sal_False );
    explicit CartesianCoordinateSystem( const CartesianCoordinateSystem & rSource );
    virtual ~CartesianCoordinateSystem();

    // ____ XCoordinateSystem ____
    virtual ::rtl::OUString SAL_CALL getCoordinateSystemType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getViewServiceName()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

class CartesianCoordinateSystem2d : public CartesianCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem2d(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~CartesianCoordinateSystem2d();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( CartesianCoordinateSystem2d )
    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

class CartesianCoordinateSystem3d : public CartesianCoordinateSystem
{
public:
    explicit CartesianCoordinateSystem3d(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~CartesianCoordinateSystem3d();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( CartesianCoordinateSystem3d )
    // ____ XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
};

}  // namespace chart

// _CHART_CARTESIANCOORDINATESYSTEM_HXX
#endif
