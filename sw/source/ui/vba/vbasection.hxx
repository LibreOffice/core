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


#ifndef SW_VBA_SECTION_HXX
#define SW_VBA_SECTION_HXX

#include <ooo/vba/word/XSection.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XSection > SwVbaSection_BASE;

class SwVbaSection : public SwVbaSection_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxPageProps;

public:
    SwVbaSection( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::beans::XPropertySet >& xProps ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaSection();

    // Attributes
    virtual ::sal_Bool SAL_CALL getProtectedForForms() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setProtectedForForms( ::sal_Bool _protectedforforms ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL Headers(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Footers(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL PageSetup(  ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_SECTION_HXX */
