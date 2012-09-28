/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef SC_VBA_AXIS_HXX
#define SC_VBA_AXOS_HXX
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XAxis.hpp>
#include <ooo/vba/excel/XChart.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <memory>
typedef InheritedHelperInterfaceImpl1< ov::excel::XAxis >  ScVbaAxis_BASE;
class ScVbaChart;
class ScVbaAxis : public ScVbaAxis_BASE
{
    css::uno::Reference< ov::excel::XChart > moChartParent;
    css::uno::Reference< css::beans::XPropertySet > mxPropertySet;
    sal_Int32 mnType;
    sal_Int32 mnGroup;
    sal_Bool bCrossesAreCustomized;
    ScVbaChart* getChartPtr() throw( css::uno::RuntimeException );
    sal_Bool isValueAxis() throw( css::script::BasicErrorException );
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ov::ShapeHelper> oShapeHelper;
    SAL_WNODEPRECATED_DECLARATIONS_POP

public:
    ScVbaAxis( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, sal_Int32 _nType, sal_Int32 _nGroup );
    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::excel::XAxisTitle > SAL_CALL getAxisTitle(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
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
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_AXIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
