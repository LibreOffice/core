/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BaseCoordinateSystem.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:58:14 $
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
#ifndef CHART_COORDINATESYSTEM_HXX
#define CHART_COORDINATESYSTEM_HXX

#include "ServiceMacros.hxx"
#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include "ModifyListenerHelper.hxx"

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEM_HPP_
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPECONTAINER_HPP_
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper6
    < ::com::sun::star::lang::XServiceInfo,
      ::com::sun::star::chart2::XCoordinateSystem,
      ::com::sun::star::chart2::XChartTypeContainer,
      ::com::sun::star::util::XCloneable,
      ::com::sun::star::util::XModifyBroadcaster,
      ::com::sun::star::util::XModifyListener >
    BaseCoordinateSystem_Base;
}

class BaseCoordinateSystem :
        public impl::BaseCoordinateSystem_Base,
        public MutexContainer,
        public ::property::OPropertySet
{
public:
    BaseCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        sal_Int32 nDimensionCount = 2,
        sal_Bool bSwapXAndYAxis = sal_False );
    explicit BaseCoordinateSystem( const BaseCoordinateSystem & rSource );
    virtual ~BaseCoordinateSystem();

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ XCoordinateSystem ____
    virtual ::sal_Int32 SAL_CALL getDimension()
        throw (::com::sun::star::uno::RuntimeException);
    // not implemented
//     virtual ::rtl::OUString SAL_CALL getCoordinateSystemType()
//         throw (::com::sun::star::uno::RuntimeException);
    // not implemented
//     virtual ::rtl::OUString SAL_CALL getViewServiceName()
//         throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAxisByDimension(
        ::sal_Int32 nDimension,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis,
        ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > SAL_CALL getAxisByDimension(
        ::sal_Int32 nDimension, ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getMaximumAxisIndexByDimension( ::sal_Int32 nDimension )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XChartTypeContainer ____
    virtual void SAL_CALL addChartType(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& aChartType )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChartType(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& aChartType )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > > SAL_CALL getChartTypes()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setChartTypes(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > >& aChartTypes )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    // not implemented
//     virtual ::com::sun::star::uno::Reference<
//         ::com::sun::star::util::XCloneable > SAL_CALL createClone()
//         throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceInfo ____
    // not implemented
//     virtual ::rtl::OUString SAL_CALL getImplementationName()
//         throw (::com::sun::star::uno::RuntimeException);
//     virtual ::sal_Bool SAL_CALL supportsService(
//         const ::rtl::OUString& ServiceName )
//         throw (::com::sun::star::uno::RuntimeException);
//     virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
//         throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();
    using OPropertySet::disposing;

    void fireModifyEvent();

protected:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >        m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;

private:
    sal_Int32                                             m_nDimensionCount;
    typedef ::std::vector< ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XAxis > > > tAxisVecVecType;
    tAxisVecVecType m_aAllAxis; //outer sequence is the dimension; inner sequence is the axis index that indicates main or secondary axis
    ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any >                  m_aOrigin;
    ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartType > >          m_aChartTypes;
};

} //  namespace chart

// CHART_COORDINATESYSTEM_HXX
#endif
