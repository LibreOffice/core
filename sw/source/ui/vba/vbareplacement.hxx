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


#ifndef SW_VBA_REPLACEMENT_HXX
#define SW_VBA_REPLACEMENT_HXX

#include <ooo/vba/word/XReplacement.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/util/XPropertyReplace.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XReplacement > SwVbaReplacement_BASE;

class SwVbaReplacement : public SwVbaReplacement_BASE
{
private:
    css::uno::Reference< css::util::XPropertyReplace> mxPropertyReplace;

public:
    SwVbaReplacement( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::util::XPropertyReplace >& xPropertyReplace ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaReplacement();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const ::rtl::OUString& _text ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_REPLACEMENT_HXX */
