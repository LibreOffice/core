/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Axis.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:52:59 $
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
#ifndef CHART_AXIS_HXX
#define CHART_AXIS_HXX

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef CHART_MUTEXCONTAINER_HXX
#include "MutexContainer.hxx"
#endif
#ifndef CHART_OPROPERTYSET_HXX
#include "OPropertySet.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <com/sun/star/chart2/XIdentifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper4<
    ::com::sun::star::chart2::XAxis,
    ::com::sun::star::chart2::XIdentifiable,
    ::com::sun::star::chart2::XTitled,
    ::com::sun::star::lang::XServiceInfo >
    Axis_Base;
}

class Axis :
    public helper::MutexContainer,
    public impl::Axis_Base,
    public ::property::OPropertySet
{
public:
    Axis( ::com::sun::star::uno::Reference<
          ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~Axis();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( Axis )
    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

//  virtual sal_Bool SAL_CALL convertFastPropertyValue
//         ( ::com::sun::star::uno::Any & rConvertedValue,
//           ::com::sun::star::uno::Any & rOldValue,
//           sal_Int32 nHandle,
//           const ::com::sun::star::uno::Any& rValue )
//      throw (::com::sun::star::lang::IllegalArgumentException);

    // ____ XAxis ____
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > > SAL_CALL getSubTickProperties()
        throw (::com::sun::star::uno::RuntimeException);

// ____ XMeter ____
    virtual void SAL_CALL attachCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XBoundedCoordinateSystem >& xCoordSys,
        sal_Int32 nRepresentedDimension )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XBoundedCoordinateSystem > SAL_CALL getCoordinateSystem()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getRepresentedDimension()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setIncrement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XIncrement >& aIncrement )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XIncrement > SAL_CALL getIncrement()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XIdentifiable ____
    virtual ::rtl::OUString SAL_CALL getIdentifier()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XTitle > SAL_CALL getTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::rtl::OUString m_aIdentifier;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XBoundedCoordinateSystem >
                       m_xCoordinateSystem;
    sal_Int32          m_nRepresentedDimension;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XIncrement >
                       m_xIncrement;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle >
                       m_xTitle;
};

} //  namespace chart

// CHART_AXIS_HXX
#endif
