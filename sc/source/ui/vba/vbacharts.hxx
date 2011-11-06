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


#ifndef SC_VBA_CHARTS_HXX
#define SC_VBA_CHARTS_HXX
#include <ooo/vba/excel/XCharts.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include "excelvbahelper.hxx"
#include <hash_map>

typedef CollTestImplHelper< ov::excel::XCharts > Charts_BASE;

class ScVbaCharts : public Charts_BASE
{
    css::uno::Reference< ov::excel::XChart > xActiveChart;
    css::uno::Reference< css::sheet::XSpreadsheetDocument > xSpreadsheetDocument;
    css::uno::Reference< css::lang::XComponent > xComponent;
public:
    ScVbaCharts( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::frame::XModel >& xModel );
    // XCharts
    virtual css::uno::Any SAL_CALL Add() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XChart > SAL_CALL getActiveChart(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XCollection
    ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) { return ov::excel::XChart::static_type(0); }
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    // Charts_BASE or HelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif
