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
#ifndef SC_VBA_CHARTOBJECT_HXX
#define SC_VBA_CHARTOBJECT_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <ooo/vba/excel/XChartObject.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <memory>

typedef InheritedHelperInterfaceImpl1<ov::excel::XChartObject > ChartObjectImpl_BASE;

class ScVbaChartObject : public ChartObjectImpl_BASE
{

    css::uno::Reference< css::table::XTableChart  > xTableChart;
    css::uno::Reference< css::document::XEmbeddedObjectSupplier > xEmbeddedObjectSupplier;
    css::uno::Reference< css::beans::XPropertySet > xPropertySet;
    css::uno::Reference< css::drawing::XDrawPageSupplier > xDrawPageSupplier;
    css::uno::Reference< css::drawing::XDrawPage > xDrawPage;
    css::uno::Reference< css::drawing::XShape > xShape;
    css::uno::Reference< css::container::XNamed > xNamed;
    rtl::OUString sPersistName;
    std::auto_ptr<ov::ShapeHelper> oShapeHelper;
    css::uno::Reference< css::container::XNamed > xNamedShape;
    rtl::OUString getPersistName();
    css::uno::Reference< css::drawing::XShape > setShape() throw ( css::script::BasicErrorException );
public:
    ScVbaChartObject( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::table::XTableChart >& _xTableChart, const css::uno::Reference< css::drawing::XDrawPageSupplier >& _xDrawPageSupplier );
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& sName ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XChart > SAL_CALL getChart() throw (css::uno::RuntimeException);
    virtual void SAL_CALL Delete() throw ( css::script::BasicErrorException );
    virtual void Activate() throw ( css::script::BasicErrorException );
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    // non interface methods
    double getHeight();
    void setHeight( double _fheight ) throw ( css::script::BasicErrorException );
    double getWidth();
    void setWidth( double _fwidth ) throw ( css::script::BasicErrorException );
    double getLeft();
    void setLeft( double _fleft );
    double getTop();
    void setTop( double _ftop );
    // should make this part of the XHelperInterface with a default
    // implementation returning NULL
    css::uno::Reference< css::uno::XInterface > getUnoObject() throw ( css::script::BasicErrorException );
};

#endif //SC_VBA_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
