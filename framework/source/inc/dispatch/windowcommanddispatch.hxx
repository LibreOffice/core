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

#ifndef __FRAMEWORK_DISPATCH_WINDOWCOMMANDDISPATCH_HXX_
#define __FRAMEWORK_DISPATCH_WINDOWCOMMANDDISPATCH_HXX_

#include <threadhelp/threadhelpbase.hxx>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/implbase1.hxx>
#include <tools/link.hxx>

namespace framework{

namespace css = ::com::sun::star;

/** @short  internal helper to bind e.g. MAC-Menu events to our internal dispatch API.

    @descr  On e.g. MAC platform system menus are merged together with some fix entries as
            e.g. "Pereferences" or "About". These menu entries trigger hard coded commands.
            Here we map these commands to the right URLs and dispatch them.

            This helper knows a frame and its container window (where VCL provide the hard coded
            commands). We hold those objects weak so there is no need to react for complex UNO dispose/ing()
            scenarios. On the other side VCL does not hold us alive (because it doesn't know our UNO reference).
            So we register at the VCL level as an event listener and
 */
class WindowCommandDispatch : private ThreadHelpBase
{
    private:

        /// can be used to create own needed services on demand.
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /// knows the frame, where we dispatch our commands as weak reference
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /// knows the VCL window (where the hard coded commands occurred) as weak XWindow reference
        css::uno::WeakReference< css::awt::XWindow > m_xWindow;

    //___________________________________________
    // native interface

    public:

        //_______________________________________

        /** @short  creates a new instance and initialize it with all necessary parameters.

            @descr  Every instance of such MACDispatch can be used for the specified context only.
                    Means: 1 MACDispatch object is bound to 1 Frame/Window pair in which context
                    the detected commands will be executed.

            @param  xSMGR
                    will be used to create own needed services on demand.

            @param  xFrame
                    used as for new detected commands.
         */
        WindowCommandDispatch(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                              const css::uno::Reference< css::frame::XFrame >&              xFrame);

        //_______________________________________

        /** @short  used to free internal resources.
         */
        virtual ~WindowCommandDispatch();

    //___________________________________________
    // implementation

    private:

        //_______________________________________

        /** @short  establish all listener connections we need.

            @descr  Those listener connections will be created one times only (see ctor).
                    Afterwards we listen for incoming events till our referred frame/window pair
                    will be closed.
         */
        void impl_startListening();

        /** @short  drop all listener connections we need.

         */
        void impl_stopListening();

        //_______________________________________

        /** @short  callback from VCL to notify new commands
         */
        DECL_LINK( impl_notifyCommand, void* );

        //_______________________________________

        /** @short  dispatch right command URLs into our frame context.

            @param  sCommand
                    the command for dispatch
         */
        void impl_dispatchCommand(const ::rtl::OUString& sCommand);

}; // class MACDispatch

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_MACDISPATCH_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
