/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef SC_VBA_CHARTOBJECT_HXX
#define SC_VBA_CHARTOBJECT_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XNamedEx.hpp>
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
    css::uno::Reference< css::container::XNamedEx > xNamed;
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
