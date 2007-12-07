/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaaxis.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:44:02 $
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
#ifndef SC_VBA_AXIS_HXX
#define SC_VBA_AXOS_HXX
#include <com/sun/star/beans/XPropertySet.hpp>
#include <org/openoffice/excel/XAxis.hpp>
#include <org/openoffice/excel/XChart.hpp>
#include "vbahelperinterface.hxx"

typedef InheritedHelperInterfaceImpl1< oo::excel::XAxis >  ScVbaAxis_BASE;
class ScVbaChart;
class ScVbaAxis : public ScVbaAxis_BASE
{
    css::uno::Reference< oo::excel::XChart > moChartParent;
    css::uno::Reference< css::beans::XPropertySet > mxPropertySet;
    sal_Int32 mnType;
    sal_Int32 mnGroup;
    sal_Int32 mnCrosses;
    sal_Bool bCrossesAreCustomized;
    ScVbaChart* getChartPtr() throw( css::uno::RuntimeException );
    sal_Bool isValueAxis() throw( css::script::BasicErrorException );
    std::auto_ptr<oo::ShapeHelper> oShapeHelper;

public:
    ScVbaAxis( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, sal_Int32 _nType, sal_Int32 _nGroup );
    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ::org::openoffice::excel::XAxisTitle > SAL_CALL getAxisTitle(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayUnit( ::sal_Int32 DisplayUnit ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getDisplayUnit(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setCrosses( ::sal_Int32 Crosses ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCrosses(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setCrossesAt( double CrossesAt ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getCrossesAt(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setType( ::sal_Int32 Type ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getType(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setHasTitle( ::sal_Bool HasTitle ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getHasTitle(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMinorUnit( double MinorUnit ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getMinorUnit(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMinorUnitIsAuto( ::sal_Bool MinorUnitIsAuto ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMinorUnitIsAuto(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setReversePlotOrder( ::sal_Bool ReversePlotOrder ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getReversePlotOrder(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMajorUnit( double MajorUnit ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getMajorUnit(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMajorUnitIsAuto( ::sal_Bool MajorUnitIsAuto ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMajorUnitIsAuto(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMaximumScale( double MaximumScale ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getMaximumScale(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMaximumScaleIsAuto( ::sal_Bool MaximumScaleIsAuto ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMaximumScaleIsAuto(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMinimumScale( double MinimumScale ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getMinimumScale(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMinimumScaleIsAuto( ::sal_Bool MinimumScaleIsAuto ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMinimumScaleIsAuto(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getAxisGroup(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScaleType( ::sal_Int32 ScaleType ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getScaleType(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getHeight(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setHeight( double height ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getWidth(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setWidth( double width ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getTop(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setTop( double top ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual double SAL_CALL getLeft(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setLeft( double left ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_AXIS_HXX
