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


#ifndef SW_VBA_GLOBALS_HXX
#define SW_VBA_GLOBALS_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <ooo/vba/word/XGlobals.hpp>
#include <ooo/vba/word/XApplication.hpp>
#include <ooo/vba/word/XSystem.hpp>
#include <ooo/vba/word/XOptions.hpp>
#include <ooo/vba/word/XSelection.hpp>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbaglobalbase.hxx>

// =============================================================================
// class SwVbaGlobals
// =============================================================================


typedef ::cppu::ImplInheritanceHelper1< VbaGlobalsBase, ov::word::XGlobals > SwVbaGlobals_BASE;

class SwVbaGlobals : public SwVbaGlobals_BASE
{
private:
    css::uno::Reference< ooo::vba::word::XApplication > mxApplication;

    virtual css::uno::Reference< ooo::vba::word::XApplication > getApplication() throw (css::uno::RuntimeException);

public:

    SwVbaGlobals( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& rxContext );
    virtual ~SwVbaGlobals();

    // XGlobals
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XSystem > SAL_CALL getSystem() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::word::XDocument > SAL_CALL getActiveDocument() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::word::XWindow > SAL_CALL getActiveWindow() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XOptions > SAL_CALL getOptions() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XSelection > SAL_CALL getSelection() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL CommandBars( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Documents( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Addins( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Dialogs( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL CentimetersToPoints( float _Centimeters ) throw (css::uno::RuntimeException);
    // XMultiServiceFactory
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_GLOBALS_HXX */
