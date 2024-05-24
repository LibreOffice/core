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

#include <sal/config.h>

#include <vector>
#include <mutex>

// include files of own module
#include <helper/wakeupthread.hxx>

// include uno interfaces
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

namespace framework{

/**
    @descr  This struct hold some information about all currently running progress processes.
            Because the can be used on a stack, we must cache her states but must paint only
            the top most one.
 */
struct IndicatorInfo
{

    // member
    public:

        /** @short  points to the indicator child, where we hold its states
                    alive here. */
        css::uno::Reference< css::task::XStatusIndicator > m_xIndicator;

        /** @short  the last set text for this indicator */
        OUString m_sText;

        /** @short  the last set value for this indicator */
        sal_Int32 m_nValue;

    // interface
    public:

        /** @short  initialize new instance of this class

            @param  xIndicator
                    the new child indicator of our factory.

            @param  sText
                    its initial text.

            @param  nRange
                    the max range for this indicator.
         */
        IndicatorInfo(const css::uno::Reference< css::task::XStatusIndicator >& xIndicator,
                      const OUString&                                    sText    )
        {
            m_xIndicator = xIndicator;
            m_sText      = sText;
            m_nValue     = 0;
        }

        /** @short  Used to locate an info struct inside a stl structure...

            @descr  The indicator object itself is used as key. Its values
                    are not interesting then. Because more than one child
                    indicator can use the same values...
         */
        bool operator==(const css::uno::Reference< css::task::XStatusIndicator >& xIndicator) const
        {
            return (m_xIndicator == xIndicator);
        }
};

/** @descr  Define a list of child indicator objects and its data. */
typedef ::std::vector< IndicatorInfo > IndicatorStack;

/** @short          implement a factory service to create new status indicator objects

    @descr          Internally it uses:
                    - a vcl based
                    - or a uno based and by the frame layouted
                    progress implementation.

                    This factory create different indicators and control his access
                    to a shared output device! Only the last activated component
                    can write its state to this device. All other requests will be
                    cached only.

    @devstatus      ready to use
    @threadsafe     yes
 */
class StatusIndicatorFactory final : public  ::cppu::WeakImplHelper<
                                             css::lang::XServiceInfo
                                           , css::lang::XInitialization
                                           , css::task::XStatusIndicatorFactory
                                           , css::util::XUpdatable >
{

    // member
    private:
        std::mutex m_mutex;

        /** stack with all current indicator children. */
        IndicatorStack m_aStack;

        /** uno service manager to create own needed uno resources. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** most active indicator child, which could work with our shared indicator window only. */
        css::uno::Reference< css::task::XStatusIndicator > m_xActiveChild;

        /** used to show the progress on the frame (layouted!) or
            as a plugged vcl window. */
        css::uno::Reference< css::task::XStatusIndicator > m_xProgress;

        /** points to the frame, where we show the progress (in case
            m_xProgress points to a frame progress. */
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /** points to an outside window, where we show the progress (in case
            we are plugged into such window). */
        css::uno::WeakReference< css::awt::XWindow > m_xPluggWindow;

        /** Notify us if a fix time is over. We use it to implement an
            intelligent "Reschedule" ... */
        std::unique_ptr<WakeUpThread> m_pWakeUp;

        /** Our WakeUpThread calls us in our interface method "XUpdatable::update().
            There we set this member m_bAllowReschedule to sal_True. Next time if our impl_reschedule()
            method is called, we know, that an Application::Reschedule() should be made.
            Because the last made Reschedule can be was taken long time ago ... may be.*/
        bool m_bAllowReschedule;

        /** enable/disable automatic showing of our parent window. */
        bool m_bAllowParentShow;

        /** enable/disable rescheduling. Default=enabled*/
        bool m_bDisableReschedule;

        /** prevent recursive calling of Application::Reschedule(). */
        static sal_Int32 m_nInReschedule;

    // interface

    public:
        StatusIndicatorFactory(css::uno::Reference< css::uno::XComponentContext > xContext);

        virtual OUString SAL_CALL getImplementationName() override
        {
            return u"com.sun.star.comp.framework.StatusIndicatorFactory"_ustr;
        }

        virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
        {
            return cppu::supportsService(this, ServiceName);
        }

        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
        {
           return { u"com.sun.star.task.StatusIndicatorFactory"_ustr };
        }

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments) override;

        // XStatusIndicatorFactory
        virtual css::uno::Reference< css::task::XStatusIndicator > SAL_CALL createStatusIndicator() override;

        // XUpdatable
        virtual void SAL_CALL update() override;

        // similar (XStatusIndicator)
        void start(const css::uno::Reference< css::task::XStatusIndicator >& xChild,
                           const OUString&                                    sText ,
                                 sal_Int32                                           nRange);

        void reset(const css::uno::Reference< css::task::XStatusIndicator >& xChild);

        void end(const css::uno::Reference< css::task::XStatusIndicator >& xChild);

        void setText(const css::uno::Reference< css::task::XStatusIndicator >& xChild,
                                      const OUString&                                    sText );

        void setValue(const css::uno::Reference< css::task::XStatusIndicator >& xChild,
                                             sal_Int32                                           nValue);

    // specials

    private:

        virtual ~StatusIndicatorFactory() override;

    // helper

        /** @short  show the parent window of this progress ...
                    if it's allowed to do so.

            @descr  By default we show the parent window automatically
                    if this progress is used.
                    If that isn't a valid operation, the user of this
                    progress can suppress this feature by initializing
                    us with a special parameter.

            @seealso    initialize()
         */
        void implts_makeParentVisibleIfAllowed();

        /** @short  creates a new internal used progress.
            @descr  This factory does not paint the progress itself.
                    It uses helper for that. They can be vcl based or
                    layouted by the frame and provided as a uno interface.
         */
        void impl_createProgress();

        /** @short  shows the internal used progress.
            @descr  This factory does not paint the progress itself.
                    It uses helper for that. They can be vcl based or
                    layouted by the frame and provided as a uno interface.
         */
        void impl_showProgress();

        /** @short  hides the internal used progress.
            @descr  This factory does not paint the progress itself.
                    It uses helper for that. They can be vcl based or
                    layouted by the frame and provided as a uno interface.
         */
        void impl_hideProgress();

        /** @short  try to "share the current thread in an intelligent manner" :-)

            @param  Overwrites our algorithm for Reschedule and force it to be sure
                    that our progress was painted right.
         */
        void impl_reschedule(bool bForceUpdate);

        void impl_startWakeUpThread();
        void impl_stopWakeUpThread();

}; // class StatusIndicatorFactory

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
