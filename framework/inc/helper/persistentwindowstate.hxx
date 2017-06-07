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

#ifndef INCLUDED_FRAMEWORK_INC_HELPER_PERSISTENTWINDOWSTATE_HXX
#define INCLUDED_FRAMEWORK_INC_HELPER_PERSISTENTWINDOWSTATE_HXX

#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <unotools/moduleoptions.hxx>
#include <cppuhelper/implbase.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          listener for closing document frames to make her window state persistent
    @descr          It's a feature of our office. If a document window was created by ourself (and not from
                    any external process e.g. the office bean) we save and restore the window state of it
                    corresponding to the document service factory. That means: one instance of this class will be
                    a listener on one frame which container window was created by ourself.
                    We listen for frame action events and every time a component will deattached from a frame
                    we store its current position and size to the configuration. Every time a new component is
                    attached to a frame first time(!) we restore this information again.

    @base           OWeakObject
                        implements ref counting for this class.

    @devstatus      ready
    @threadsafe     yes
*//*-*************************************************************************************************************/
class PersistentWindowState :   public  ::cppu::WeakImplHelper<
                                           css::lang::XInitialization,
                                           css::frame::XFrameActionListener > // => XEventListener
{

    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /// reference to the frame which was created by the office himself
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /// we call SetWindowState one times only for the same frame!
        bool m_bWindowStateAlreadySet;

    // interface

    public:

        // ctor/dtor
                 PersistentWindowState(const css::uno::Reference< css::uno::XComponentContext >& xContext);
        virtual ~PersistentWindowState(                                                                   ) override;

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments) override;

        // XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) override;

        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

    // helper

    private:

        /** @short  identify the application module, which  is used behind the component
                    of our frame.

            @param  rxContext
                    needed to create needed uno resources.

            @param  xFrame
                    contains the component, which must be identified.

            @return [string]
                    a module identifier for the current frame component.
         */
        static OUString implst_identifyModule(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                     const css::uno::Reference< css::frame::XFrame >&              xFrame);

        /** @short  retrieve the window state from the configuration.

            @param  rxContext
                    needed to create the configuration access.

            @param  sModuleName
                    identifies the application module, where the
                    information should be obtained for.

            @return [string]
                    contains the information about position and size.
         */
        static OUString implst_getWindowStateFromConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext      ,
                                                               const OUString&                                    sModuleName);

        /** @short  retrieve the window state from the container window.

            @param  xWindow
                    must point to the container window of the frame.
                    We use it VCL part here - because the toolkit doesn't
                    provide the right functionality!

            @return [string]
                    contains the information about position and size.
         */
        static OUString implst_getWindowStateFromWindow(const css::uno::Reference< css::awt::XWindow >& xWindow);

        /** @short  restore the position and size on the container window.

            @param  rxContext
                    needed to create the configuration access.

            @param  sModuleName
                    identifies the application module, where the
                    information should be set on.

            @param  sWindowState
                    contains the information about position and size.
         */
        static void implst_setWindowStateOnConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                  const OUString&                                    sModuleName   ,
                                                  const OUString&                                    sWindowState  );

        /** @short  restore the position and size on the container window.

            @param  xWindow
                    must point to the container window of the frame.
                    We use it VCL part here - because the toolkit doesn't
                    provide the right functionality!

            @param  sWindowState
                    contains the information about position and size.
         */
        static void implst_setWindowStateOnWindow(const css::uno::Reference< css::awt::XWindow >& xWindow     ,
                                                  const OUString&                          sWindowState);

}; // class PersistentWindowState

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_HELPER_PERSISTENTWINDOWSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
