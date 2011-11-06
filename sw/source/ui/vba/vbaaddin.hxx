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


#ifndef SW_VBA_ADDIN_HXX
#define SW_VBA_ADDIN_HXX

#include <ooo/vba/word/XAddin.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XAddin > SwVbaAddin_BASE;

class SwVbaAddin : public SwVbaAddin_BASE
{
private:
    rtl::OUString msFileURL;
    sal_Bool mbAutoload;
    sal_Bool mbInstalled;

public:
    SwVbaAddin( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const rtl::OUString& rFileURL, sal_Bool bAutoload ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaAddin();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const rtl::OUString& _name ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getPath() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoload() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getInstalled() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setInstalled( ::sal_Bool _installed ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_ADDIN_HXX */
