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


#ifndef SC_VBA_INTERIOR_HXX
#define SC_VBA_INTERIOR_HXX

#include <ooo/vba/excel/XWorksheetFunction.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "vbarange.hxx"

#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

typedef InheritedHelperInterfaceImpl1< ov::excel::XWorksheetFunction > ScVbaWSFunction_BASE;

class ScVbaWSFunction :  public ScVbaWSFunction_BASE
{
public:
    ScVbaWSFunction( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext);
    virtual ~ScVbaWSFunction(){}

    virtual css::uno::Reference< css::beans::XIntrospectionAccess >  SAL_CALL getIntrospection(void)  throw(css::uno::RuntimeException);
    virtual css::uno::Any  SAL_CALL invoke(const rtl::OUString& FunctionName, const css::uno::Sequence< css::uno::Any >& Params, css::uno::Sequence< sal_Int16 >& OutParamIndex, css::uno::Sequence< css::uno::Any >& OutParam) throw(css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual void  SAL_CALL setValue(const rtl::OUString& PropertyName, const css::uno::Any& Value) throw(css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual css::uno::Any  SAL_CALL getValue(const rtl::OUString& PropertyName) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual sal_Bool  SAL_CALL hasMethod(const rtl::OUString& Name)  throw(css::uno::RuntimeException);
    virtual sal_Bool  SAL_CALL hasProperty(const rtl::OUString& Name)  throw(css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getExactName( const ::rtl::OUString& aApproximateName ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif

