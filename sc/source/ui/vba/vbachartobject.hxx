/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbachartobject.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:46:21 $
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
#ifndef SC_VBA_CHARTOBJECT_HXX
#define SC_VBA_CHARTOBJECT_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <org/openoffice/excel/XChartObject.hpp>
#include "vbahelperinterface.hxx"
#include <memory>

typedef InheritedHelperInterfaceImpl1<oo::excel::XChartObject > ChartObjectImpl_BASE;

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
    std::auto_ptr<oo::ShapeHelper> oShapeHelper;
    css::uno::Reference< css::container::XNamed > xNamedShape;
    rtl::OUString getPersistName();
    css::uno::Reference< css::drawing::XShape > setShape() throw ( css::script::BasicErrorException );
public:
    ScVbaChartObject( const css::uno::Reference< oo::vba::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::table::XTableChart >& _xTableChart, const css::uno::Reference< css::drawing::XDrawPageSupplier >& _xDrawPageSupplier );
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& sName ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XChart > SAL_CALL getChart() throw (css::uno::RuntimeException);
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
