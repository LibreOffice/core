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

#ifndef INCLUDED_FRAMEWORK_INC_HELPER_VCLSTATUSINDICATOR_HXX
#define INCLUDED_FRAMEWORK_INC_HELPER_VCLSTATUSINDICATOR_HXX

#include <general.h>

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <cppuhelper/implbase.hxx>
#include <vcl/status.hxx>

namespace framework {

// declaration

class VCLStatusIndicator : public  ::cppu::WeakImplHelper< css::task::XStatusIndicator >
{

    // member

    private:

        /** points to the parent window of this progress and
            hold it alive. */
        css::uno::Reference< css::awt::XWindow > m_xParentWindow;

        /** shows the progress.

            @attention  This member is not synchronized using our own mutex!
                        Its guarded by the solarmutex only. Otherwise
                        we have to lock two of them, which can force a deadlock ...
            */
        VclPtr<StatusBar> m_pStatusBar;

        /** knows the current range of the progress. */
        sal_Int32 m_nRange;

        /** knows the current value of the progress. */
        sal_Int32 m_nValue;

    // interface

    public:

        /// ctor
        VCLStatusIndicator(const css::uno::Reference< css::awt::XWindow >&               xParentWindow);

        /// dtor
        virtual ~VCLStatusIndicator() override;

        /// XStatusIndicator
        virtual void SAL_CALL start(const OUString& sText ,
                                          sal_Int32        nRange) override;

        virtual void SAL_CALL reset() override;

        virtual void SAL_CALL end() override;

        virtual void SAL_CALL setText(const OUString& sText) override;

        virtual void SAL_CALL setValue(sal_Int32 nValue) override;

    // helper

    private:

        static void impl_recalcLayout(vcl::Window* pStatusBar   ,
                                      vcl::Window const * pParentWindow);
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_HELPER_VCLSTATUSINDICATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
