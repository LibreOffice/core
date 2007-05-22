/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sampleaddin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:30:06 $
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

#ifndef _SAMPLEADDIN_HXX_
#define _SAMPLEADDIN_HXX_

#include <cppuhelper/implbase9.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/chart/XDiagram.hpp>
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>

#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>

#include <com/sun/star/chart/XChartDocument.hpp>

com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
    SampleAddIn_CreateInstance(
        const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& );

class SampleAddIn : public cppu::WeakImplHelper9<
    com::sun::star::lang::XInitialization,
    com::sun::star::chart::XDiagram,
    com::sun::star::chart::XAxisXSupplier,
    com::sun::star::chart::XAxisYSupplier,
    com::sun::star::chart::XStatisticDisplay,
    com::sun::star::lang::XServiceName,
    com::sun::star::lang::XServiceInfo,
    com::sun::star::util::XRefreshable,
    com::sun::star::lang::XLocalizable  >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument > mxChartDoc;
    ::com::sun::star::lang::Locale maLocale;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxMyRedLine;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxMyText;

public:
    SampleAddIn();
    virtual ~SampleAddIn();

    // class specific code
    static ::rtl::OUString  getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

    sal_Bool getLogicalPosition( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xAxis,
                                 double fValue,
                                 sal_Bool bVertical,
                                 ::com::sun::star::awt::Point& aOutPosition );

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw( ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException );

    // XDiagram
    virtual ::rtl::OUString SAL_CALL getDiagramType() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getDataRowProperties( sal_Int32 nRow )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
        throw( ::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException );

    // XShape ( ::XDiagram )
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& )
        throw( ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& )
        throw( ::com::sun::star::uno::RuntimeException );

    // XShapeDescriptor ( ::XShape ::XDiagram )
    virtual rtl::OUString SAL_CALL getShapeType() throw( com::sun::star::uno::RuntimeException );

    // XAxisXSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > SAL_CALL getXAxisTitle()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getXAxis()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getXMainGrid()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getXHelpGrid()
        throw( ::com::sun::star::uno::RuntimeException );

    // XAxisYSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > SAL_CALL getYAxisTitle()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getYAxis()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getYHelpGrid()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getYMainGrid()
        throw( ::com::sun::star::uno::RuntimeException );

    // XStatisticDisplay
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getUpBar()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getDownBar()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getMinMaxLine()
        throw( ::com::sun::star::uno::RuntimeException );

    // XServiceName
    virtual ::rtl::OUString SAL_CALL getServiceName() throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException );

    // XRefreshable
    virtual void SAL_CALL refresh() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l )
        throw( ::com::sun::star::uno::RuntimeException );

    // XLocalizable
    virtual void SAL_CALL setLocale( const ::com::sun::star::lang::Locale& eLocale )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale()
        throw( ::com::sun::star::uno::RuntimeException );
};

#endif  // _SAMPLEADDIN_HXX_
