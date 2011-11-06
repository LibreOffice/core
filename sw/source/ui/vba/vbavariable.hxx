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


#ifndef SW_VBA_VARIABLE_HXX
#define SW_VBA_VARIABLE_HXX

#include <ooo/vba/word/XVariable.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertyAccess.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XVariable > SwVbaVariable_BASE;

class SwVbaVariable : public SwVbaVariable_BASE
{
private:
    css::uno::Reference< css::beans::XPropertyAccess > mxUserDefined;
    rtl::OUString maName;

public:
    SwVbaVariable( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const css::uno::Reference< css::beans::XPropertyAccess >& rUserDefined, const rtl::OUString& rName ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaVariable();

   // XVariable
    virtual rtl::OUString SAL_CALL getName() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setName( const rtl::OUString& ) throw ( css::uno::RuntimeException );
    virtual css::uno::Any SAL_CALL getValue() throw ( css::uno::RuntimeException );
    virtual void SAL_CALL setValue( const css::uno::Any& rValue ) throw ( css::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL getIndex() throw ( css::uno::RuntimeException );

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_VARIABLE_HXX */
