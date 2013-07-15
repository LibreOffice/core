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

#ifndef __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_
#define __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XTitleChangeListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <unotools/moduleoptions.hxx>
#include <cppuhelper/implbase3.hxx>
#include <rtl/ustrbuf.hxx>

namespace framework{

/*-************************************************************************************************************//**
    @short          helps our frame on setting title/icon on the titlebar (including updates)

    @devstatus      draft
    @threadsafe     yes
*//*-*************************************************************************************************************/
class TitleBarUpdate : // baseclasses (order necessary for right initialization!)
                       private ThreadHelpBase
                        // interfaces
                     , public  ::cppu::WeakImplHelper3<
                                   css::lang::XInitialization
                                 , css::frame::XTitleChangeListener // => XEventListener
                                 , css::frame::XFrameActionListener > // => XEventListener
{
    //________________________________
    // structs, types

    private:

        struct TModuleInfo
        {
            /// internal id of this module
            OUString sID;
            /// localized name for this module
            OUString sUIName;
            /// configured icon for this module
            ::sal_Int32 nIcon;
        };

    //________________________________
    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /// reference to the frame which was created by the office himself
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

    //________________________________
    // interface

    public:

        //____________________________
        // ctor/dtor
                 TitleBarUpdate(const css::uno::Reference< css::uno::XComponentContext >& xContext);
        virtual ~TitleBarUpdate(                                                                   );

        //____________________________
        // XInterface, XTypeProvider

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
        // XTitleChangeListener
        virtual void SAL_CALL titleChanged(const css::frame::TitleChangedEvent& aEvent)
            throw (css::uno::RuntimeException);

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

            @param  xFrame
                    contains the component, which must be identified.

            @param  rInfo
                    describe the module in its details.
                    Is set only if return value is true.

            @return [sal_Bool]
                    sal_True in casee module could be identified and all needed values could be read.
                    sal_False otherwise.
         */
        ::sal_Bool implst_getModuleInfo(const css::uno::Reference< css::frame::XFrame >& xFrame,
                                              TModuleInfo&                               rInfo );

        //____________________________
        /** @short  set a new icon and title on the title bar of our connected frame window.

            @descr  It does not check if an update is realy needed. That has to be done outside.
                    It retrieves all needed information and update the title bar - nothing less -
                    nothing more.
         */
        void impl_forceUpdate();

        //____________________________
        /** @short  identify the current component (inside the connected frame)
                    and set the right module icon on the title bar.

            @param  xFrame
                    the frame which contains the component and where the icon must be set
                    on the window title bar.
         */
        void impl_updateIcon(const css::uno::Reference< css::frame::XFrame >& xFrame);

        //____________________________
        /** @short  gets the current title from the frame and set it on the window.

            @param  xFrame
                    the frame which contains the component and where the title must be set
                    on the window title bar.
         */
        void impl_updateTitle(const css::uno::Reference< css::frame::XFrame >& xFrame);

        //Hook to set GNOME3/Windows 7 applicationID for toplevel frames
        //http://msdn.microsoft.com/en-us/library/dd378459(v=VS.85).aspx
        //http://live.gnome.org/GnomeShell/ApplicationBased
        void impl_updateApplicationID(const css::uno::Reference< css::frame::XFrame >& xFrame);
}; // class TitleBarUpdate

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
