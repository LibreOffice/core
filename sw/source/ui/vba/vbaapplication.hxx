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


#ifndef SW_VBA_APPLICATION_HXX
#define SW_VBA_APPLICATION_HXX

#include <ooo/vba/word/XApplication.hpp>
#include <ooo/vba/word/XDocument.hpp>
#include <ooo/vba/word/XWindow.hpp>
#include <ooo/vba/word/XSystem.hpp>
#include <ooo/vba/word/XOptions.hpp>
#include <ooo/vba/word/XSelection.hpp>
#include <ooo/vba/word/XAddins.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbaapplicationbase.hxx>
#include <cppuhelper/implbase1.hxx>

//typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XApplication > SwVbaApplication_BASE;
typedef cppu::ImplInheritanceHelper1< VbaApplicationBase, ooo::vba::word::XApplication > SwVbaApplication_BASE;

class SwVbaApplication : public SwVbaApplication_BASE
{
public:
    SwVbaApplication( css::uno::Reference< css::uno::XComponentContext >& m_xContext );
    virtual ~SwVbaApplication();

    virtual SfxObjectShell* GetDocShell( const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException);

    // XApplication
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
    virtual sal_Bool SAL_CALL getDisplayAutoCompleteTips() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDisplayAutoCompleteTips( sal_Bool _displayAutoCompleteTips ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEnableCancelKey() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEnableCancelKey( sal_Int32 _enableCancelKey ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL CentimetersToPoints( float _Centimeters ) throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
protected:
    virtual css::uno::Reference< css::frame::XModel > getCurrentDocument() throw (css::uno::RuntimeException);
};
#endif /* SW_VBA_APPLICATION_HXX */
