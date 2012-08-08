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

#ifndef __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_
#define __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_

#include <threadhelp/threadhelpbase.hxx>

#include <macros/debug.hxx>

#include <macros/xinterface.hxx>

#include <macros/xtypeprovider.hxx>

#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/lang/XInitialization.hpp>

#include <com/sun/star/frame/XFrame.hpp>

#include <com/sun/star/frame/XTitle.hpp>

#include <com/sun/star/frame/XFrameActionListener.hpp>

#include <com/sun/star/frame/XTitleChangeListener.hpp>

#include <com/sun/star/lang/XEventListener.hpp>

#include <unotools/moduleoptions.hxx>

#include <cppuhelper/weak.hxx>

#include <rtl/ustrbuf.hxx>

namespace framework{

/*-************************************************************************************************************//**
    @short          helps our frame on setting title/icon on the titlebar (including updates)

    @devstatus      draft
    @threadsafe     yes
*//*-*************************************************************************************************************/
class TitleBarUpdate : // interfaces
                       public css::lang::XTypeProvider
                     , public css::lang::XInitialization
                     , public css::frame::XTitleChangeListener // => XEventListener
                     , public css::frame::XFrameActionListener // => XEventListener
                       // baseclasses (order neccessary for right initialization!)
                     , private ThreadHelpBase
                     , public  ::cppu::OWeakObject
{
    //________________________________
    // structs, types

    private:

        struct TModuleInfo
        {
            /// internal id of this module
            ::rtl::OUString sID;
            /// localized name for this module
            ::rtl::OUString sUIName;
            /// configured icon for this module
            ::sal_Int32 nIcon;
        };

    //________________________________
    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /// reference to the frame which was created by the office himself
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

    //________________________________
    // interface

    public:

        //____________________________
        // ctor/dtor
                 TitleBarUpdate(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~TitleBarUpdate(                                                                   );

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
                    contains the component, wich must be identified.

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
                    It retrieves all needed informations and update the title bar - nothing less -
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
