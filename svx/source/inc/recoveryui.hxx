/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _RECOVERYUI_HXX
#define _RECOVERYUI_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <cppuhelper/implbase2.hxx>
#include <vcl/window.hxx>
#include "docrecovery.hxx"


namespace svx
{

// Dont export this header ... Otherwise you have to remove this namespace alias!
namespace css   = ::com::sun::star;
namespace svxdr = ::svx::DocRecovery;


class RecoveryUI : public ::cppu::WeakImplHelper2< css::lang::XServiceInfo        ,
                                                   css::frame::XSynchronousDispatch > // => XDispatch!
{
    //-------------------------------------------
    // const, types, etcpp.
    private:

        /** @short TODO */
        enum EJob
        {
            E_JOB_UNKNOWN,
            E_DO_EMERGENCY_SAVE,
            E_DO_RECOVERY,
            E_DO_CRASHREPORT
        };

    //-------------------------------------------
    // member
    private:

        /** @short TODO */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short TODO */
        Window* m_pParentWindow;

        /** @short TODO */
        RecoveryUI::EJob m_eJob;

        /** @short TODO */
        css::uno::Reference< css::task::XStatusIndicatorFactory > m_xProgressFactory;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  TODO */
        RecoveryUI(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------
        /** @short  TODO */
        virtual ~RecoveryUI();

        //---------------------------------------
        // css.lang.XServiceInfo

        virtual ::rtl::OUString SAL_CALL getImplementationName()
            throw(css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& sServiceName)
            throw(css::uno::RuntimeException);

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        virtual com::sun::star::uno::Any SAL_CALL dispatchWithReturnValue(const css::util::URL& aURL,
                                            const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // css.frame.XDispatch

        virtual void SAL_CALL dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                   const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // XServiceInfo helper

        static ::rtl::OUString st_getImplementationName();
        static css::uno::Sequence< ::rtl::OUString > st_getSupportedServiceNames();
        static css::uno::Reference< css::uno::XInterface > SAL_CALL st_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

    //-------------------------------------------
    // helper
    private:

        EJob impl_classifyJob(const css::util::URL& aURL);

        sal_Bool impl_doEmergencySave();

        void impl_doRecovery();

        void impl_showAllRecoveredDocs();

        void impl_doCrashReport();

};

} // namespace svx

#endif // _RECOVERYUI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
