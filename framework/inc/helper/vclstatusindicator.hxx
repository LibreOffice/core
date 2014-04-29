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



#ifndef __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_
#define __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_

//-----------------------------------------------
// includes of own modules

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <general.h>

//-----------------------------------------------
// includes of interfaces
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XWindow.hpp>

//-----------------------------------------------
// includes of external modules
#include <cppuhelper/weak.hxx>
#include <vcl/status.hxx>

//-----------------------------------------------
// namespace

namespace framework {

//-----------------------------------------------
// declaration

class VCLStatusIndicator : public  css::task::XStatusIndicator
                         , private ThreadHelpBase // must be the first real base class!
                         , public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // member

    private:

        /** can be used to create own needed uno resources. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** points to the parent window of this progress and
            hold it alive. */
        css::uno::Reference< css::awt::XWindow > m_xParentWindow;

        /** shows the progress.

            @attention  This member isn't synchronized using our own mutex!
                        Its guarded by the solarmutex only. Otherwhise
                        we have to lock two of them, which can force a deadlock ...
            */
        StatusBar* m_pStatusBar;

        /** knows the current info text of the progress. */
        ::rtl::OUString m_sText;

        /** knows the current range of the progress. */
        sal_Int32 m_nRange;

        /** knows the current value of the progress. */
        sal_Int32 m_nValue;

    //-------------------------------------------
    // interface

    public:

        FWK_DECLARE_XINTERFACE

        /// ctor
        VCLStatusIndicator(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR        ,
                           const css::uno::Reference< css::awt::XWindow >&               xParentWindow);

        /// dtor
        virtual ~VCLStatusIndicator();

        /// XStatusIndicator
        virtual void SAL_CALL start(const ::rtl::OUString& sText ,
                                          sal_Int32        nRange)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL reset()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL end()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setText(const ::rtl::OUString& sText)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setValue(sal_Int32 nValue)
            throw(css::uno::RuntimeException);

    //-------------------------------------------
    // helper

    private:

        static void impl_recalcLayout(Window* pStatusBar   ,
                                      Window* pParentWindow);
};

} // namespace framework

#endif // __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_
