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

#ifndef __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_
#define __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <general.h>

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <cppuhelper/implbase1.hxx>
#include <vcl/status.hxx>


namespace framework {

//-----------------------------------------------
// declaration

class VCLStatusIndicator : private ThreadHelpBase, // must be the first real base class!
                           public  ::cppu::WeakImplHelper1< css::task::XStatusIndicator >
{
    //-------------------------------------------
    // member

    private:

        /** points to the parent window of this progress and
            hold it alive. */
        css::uno::Reference< css::awt::XWindow > m_xParentWindow;

        /** shows the progress.

            @attention  This member isnt synchronized using our own mutex!
                        Its guarded by the solarmutex only. Otherwhise
                        we have to lock two of them, which can force a deadlock ...
            */
        StatusBar* m_pStatusBar;

        /** knows the current info text of the progress. */
        OUString m_sText;

        /** knows the current range of the progress. */
        sal_Int32 m_nRange;

        /** knows the current value of the progress. */
        sal_Int32 m_nValue;

    //-------------------------------------------
    // interface

    public:

        /// ctor
        VCLStatusIndicator(const css::uno::Reference< css::awt::XWindow >&               xParentWindow);

        /// dtor
        virtual ~VCLStatusIndicator();

        /// XStatusIndicator
        virtual void SAL_CALL start(const OUString& sText ,
                                          sal_Int32        nRange)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL reset()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL end()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setText(const OUString& sText)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
