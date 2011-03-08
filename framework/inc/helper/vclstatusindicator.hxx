/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

            @attention  This member isnt synchronized using our own mutex!
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
