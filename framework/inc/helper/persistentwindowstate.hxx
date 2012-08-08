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

#ifndef __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_
#define __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <unotools/moduleoptions.hxx>
#include <cppuhelper/weak.hxx>

namespace framework{

/*-************************************************************************************************************//**
    @short          listener for closing document frames to make her window state persistent
    @descr          It's a feature of our office. If a document window was created by ourself (and not from
                    any external process e.g. the office bean) we save and restore the window state of it
                    corresponding to the document service factory. That means: one instance of this class will be
                    a listener on one frame which container window was created by ourself.
                    We listen for frame action events and everytimes a component will deattached from a frame
                    we store its current position and size to the configuration. Everytimes a new component is
                    attached to a frame first time(!) we restore this informations again.

    @base           ThreadHelpBase
                        guarantee right initialized lock member during startup of instances of this class.

    @base           OWeakObject
                        implements ref counting for this class.

    @devstatus      ready
    @threadsafe     yes
*//*-*************************************************************************************************************/
class PersistentWindowState :   // interfaces
                                public css::lang::XTypeProvider,
                                public css::lang::XInitialization,
                                public css::frame::XFrameActionListener, // => XEventListener
                                // baseclasses (order neccessary for right initialization!)
                                private ThreadHelpBase,
                                public  ::cppu::OWeakObject
{
    //________________________________
    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /// reference to the frame which was created by the office himself
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /// we call SetWindowState one times only for the same frame!
        sal_Bool m_bWindowStateAlreadySet;

    //________________________________
    // interface

    public:

        //____________________________
        // ctor/dtor
                 PersistentWindowState(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~PersistentWindowState(                                                                   );

        //____________________________
        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //____________________________
        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        //____________________________
        // XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent)
            throw(css::uno::RuntimeException);

        //____________________________
        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    //________________________________
    // helper

    private:
        //____________________________
        /** @short  identify the application module, which  is used behind the component
                    of our frame.

            @param  xSMGR
                    needed to create needed uno resources.

            @param  xFrame
                    contains the component, wich must be identified.

            @return [string]
                    a module identifier for the current frame component.
         */
        static ::rtl::OUString implst_identifyModule(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                                     const css::uno::Reference< css::frame::XFrame >&              xFrame);

        //____________________________
        /** @short  retrieve the window state from the configuration.

            @param  xSMGR
                    needed to create the configuration access.

            @param  sModuleName
                    identifies the application module, where the
                    information should be getted for.

            @return [string]
                    contains the information about position and size.
         */
        static ::rtl::OUString implst_getWindowStateFromConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR      ,
                                                               const ::rtl::OUString&                                        sModuleName);

        //____________________________
        /** @short  retrieve the window state from the container window.

            @param  xWindow
                    must point to the container window of the frame.
                    We use it VCL part here - because the toolkit doesnt
                    provide the right functionality!

            @return [string]
                    contains the information about position and size.
         */
        static ::rtl::OUString implst_getWindowStateFromWindow(const css::uno::Reference< css::awt::XWindow >& xWindow);

        //____________________________
        /** @short  restore the position and size on the container window.

            @param  xSMGR
                    needed to create the configuration access.

            @param  sModuleName
                    identifies the application module, where the
                    information should be setted on.

            @param  sWindowState
                    contains the information about position and size.
         */
        static void implst_setWindowStateOnConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR         ,
                                                  const ::rtl::OUString&                                        sModuleName   ,
                                                  const ::rtl::OUString&                                        sWindowState  );

        //____________________________
        /** @short  restore the position and size on the container window.

            @param  xWindow
                    must point to the container window of the frame.
                    We use it VCL part here - because the toolkit doesnt
                    provide the right functionality!

            @param  sWindowState
                    contains the information about position and size.
         */
        static void implst_setWindowStateOnWindow(const css::uno::Reference< css::awt::XWindow >& xWindow     ,
                                                  const ::rtl::OUString&                          sWindowState);

}; // class PersistentWindowState

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
