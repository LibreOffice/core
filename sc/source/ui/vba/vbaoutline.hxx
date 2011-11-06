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


#ifndef SC_VBA_OUTLINE_HXX
#define SC_VBA_OUTLINE_HXX

#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XOutline.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XOutline > ScVbaOutline_BASE;

class ScVbaOutline :  public ScVbaOutline_BASE
{
    css::uno::Reference< css::sheet::XSheetOutline > mxOutline;
public:
    ScVbaOutline( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
         css::uno::Reference<css::sheet::XSheetOutline> outline): ScVbaOutline_BASE( xParent, xContext) , mxOutline(outline)
    {}
    virtual ~ScVbaOutline(){}

    virtual void SAL_CALL ShowLevels( const css::uno::Any& RowLevels, const css::uno::Any& ColumnLevels ) throw (css::uno::RuntimeException) ;
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif
