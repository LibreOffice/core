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

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <cppuhelper/weakref.hxx>
#include <tools/link.hxx>

namespace com::sun::star::uno {
    class XComponentContext;
}
class VclWindowEvent;

namespace framework{

/** @short  internal helper to bind e.g. MAC-Menu events to our internal dispatch API.

    @descr  On e.g. MAC platform system menus are merged together with some fix entries as
            e.g. "Pereferences" or "About". These menu entries trigger hard coded commands.
            Here we map these commands to the right URLs and dispatch them.

            This helper knows a frame and its container window (where VCL provide the hard coded
            commands). We hold those objects weak so there is no need to react for complex UNO dispose/ing()
            scenarios. On the other side VCL does not hold us alive (because it doesn't know our UNO reference).
            So we register at the VCL level as an event listener and
 */
class WindowCommandDispatch final
{
    private:
        osl::Mutex m_mutex;

        /// can be used to create own needed services on demand.
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /// knows the frame, where we dispatch our commands as weak reference
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /// knows the VCL window (where the hard coded commands occurred) as weak XWindow reference
        css::uno::WeakReference< css::awt::XWindow > m_xWindow;

    // native interface

    public:

        /** @short  creates a new instance and initialize it with all necessary parameters.

            @descr  Every instance of such MACDispatch can be used for the specified context only.
                    Means: 1 MACDispatch object is bound to 1 Frame/Window pair in which context
                    the detected commands will be executed.

            @param  xContext
                    will be used to create own needed services on demand.

            @param  xFrame
                    used as for new detected commands.
         */
        WindowCommandDispatch(const css::uno::Reference< css::uno::XComponentContext >&     xContext ,
                              const css::uno::Reference< css::frame::XFrame >&              xFrame);

        /** @short  used to free internal resources.
         */
        ~WindowCommandDispatch();

    // implementation

    private:

        /** @short  establish all listener connections we need.

            @descr  Those listener connections will be created one times only (see ctor).
                    Afterwards we listen for incoming events till our referred frame/window pair
                    will be closed.
         */
        void impl_startListening();

        /** @short  drop all listener connections we need.

         */
        void impl_stopListening();

        /** @short  callback from VCL to notify new commands
         */
        DECL_LINK( impl_notifyCommand, VclWindowEvent&, void );

}; // class WindowCommandDispatch

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
