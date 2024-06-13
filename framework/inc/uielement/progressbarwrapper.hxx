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

#pragma once

#include <helper/uielementwrapperbase.hxx>

#include <com/sun/star/awt/XWindow.hpp>

#include <unotools/weakref.hxx>

namespace framework{
class StatusIndicatorInterfaceWrapper;

class ProgressBarWrapper final : public UIElementWrapperBase
{
    public:

        //  constructor / destructor

        ProgressBarWrapper();
        virtual ~ProgressBarWrapper() override;

        // public interfaces
        void setStatusBar( const css::uno::Reference< css::awt::XWindow >& rStatusBar, bool bOwnsInstance = false );
        css::uno::Reference< css::awt::XWindow > getStatusBar() const;

        // wrapped methods of css::task::XStatusIndicator
        /// @throws css::uno::RuntimeException
        void start( const OUString& Text, ::sal_Int32 Range );
        /// @throws css::uno::RuntimeException
        void end();
        /// @throws css::uno::RuntimeException
        void setText( const OUString& Text );
        /// @throws css::uno::RuntimeException
        void setValue( ::sal_Int32 Value );
        /// @throws css::uno::RuntimeException
        void reset();

        // UNO interfaces
        // XComponent
        virtual void SAL_CALL dispose() override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XUpdatable
        virtual void SAL_CALL update() override;

        // XUIElement
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRealInterface() override;

    //  variables
    //  (should be private everyway!)

    private:
        css::uno::Reference< css::awt::XWindow >         m_xStatusBar;    // Reference to our status bar XWindow
        unotools::WeakReference< StatusIndicatorInterfaceWrapper >  m_xProgressBarIfacWrapper;
        bool                                             m_bOwnsInstance; // Indicator that we are owner of the XWindow
        sal_Int32                                        m_nRange;
        sal_Int32                                        m_nValue;
        OUString                                         m_aText;
};      //  class ProgressBarWrapper

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
