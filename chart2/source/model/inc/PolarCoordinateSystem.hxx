/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PolarCoordinateSystem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:52:18 $
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
#ifndef _CHART_POLARCOORDINATESYSTEM_HXX
#define _CHART_POLARCOORDINATESYSTEM_HXX

#include "ServiceMacros.hxx"

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEM_HPP_
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#endif

namespace chart
{

class PolarCoordinateSystem : public ::cppu::WeakImplHelper2<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::chart2::XCoordinateSystem
    >
{
public:
    explicit PolarCoordinateSystem( sal_Int32 nDim = 2 );
    virtual ~PolarCoordinateSystem();

    // ____ XCoordinateSystem ____
    // ___________________________
    /// @see ::com::sun::star::chart2::XCoordinateSystem
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation > SAL_CALL getTransformationToCartesian()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XCoordinateSystem
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation > SAL_CALL getTransformationFromCartesian()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XCoordinateSystem
    virtual sal_Int32 SAL_CALL getDimension() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getCoordinateSystemType() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getViewServiceName() throw (::com::sun::star::uno::RuntimeException);

    APPHELPER_XSERVICEINFO_DECL()

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >
        m_aTransformationToCartesian;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >
        m_aTransformationFromCartesian;
    sal_Int32   m_nDim;
};

}  // namespace chart

// _CHART_POLARCOORDINATESYSTEM_HXX
#endif
