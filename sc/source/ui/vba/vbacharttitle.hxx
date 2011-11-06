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


#ifndef SC_VBA_CHARTTITLE_HXX
#define SC_VBA_CHARTTITLE_HXX
#include "vbatitle.hxx"
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XChartTitle.hpp>


typedef TitleImpl< cppu::WeakImplHelper1< ov::excel::XChartTitle > > ChartTitleBase;

class ScVbaChartTitle : public ChartTitleBase
{
public:
    ScVbaChartTitle( const css::uno::Reference< ov::XHelperInterface >& xParent,   const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& _xTitleShape );
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif
