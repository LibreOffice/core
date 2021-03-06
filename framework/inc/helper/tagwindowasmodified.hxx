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

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          listen for modify events on model and tag frame container window so it can react accordingly
    @descr          Used e.g. by our MAC port where such state is shown separately on some controls of the
                    title bar.

    @base           OWeakObject
                        implements ref counting for this class.

    @devstatus      draft
    @threadsafe     yes
*//*-*************************************************************************************************************/
class TagWindowAsModified final : public  ::cppu::WeakImplHelper<
                                          css::lang::XInitialization,
                                          css::frame::XFrameActionListener, // => XEventListener
                                          css::util::XModifyListener >       // => XEventListener
{

    // member

    private:

        /// reference to the frame, where we listen for new loaded documents for updating our own xModel reference
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /// reference to the frame container window, where we must set the tag
        css::uno::WeakReference< css::awt::XWindow > m_xWindow;

        /// we list on the model for modify events
        css::uno::WeakReference< css::frame::XModel > m_xModel;

    // interface

    public:

        // ctor/dtor
                 TagWindowAsModified();
        virtual ~TagWindowAsModified(                                                                   ) override;

        // XInterface, XTypeProvider

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments) override;

        // XModifyListener
        virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) override;

        // XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) override;

        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

    private:

        // @todo document me
        void impl_update(const css::uno::Reference< css::frame::XFrame >& xFrame);

}; // class TagWindowAsModified

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
