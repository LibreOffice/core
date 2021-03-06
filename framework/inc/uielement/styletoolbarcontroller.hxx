/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>

namespace framework {

/**
 * A dispatcher that serves as a proxy for style commands with arguments
 * i.e. .uno:StyleApply?... in order to provide useful status updates to
 * generic UI elements such as toolbars or menubar. It listens to special
 * status commands, and computes a boolean status out of them. Then it
 * forwards that boolean status to the listener, as if it was the status
 * of the original command.
 *
 * Note that the implementation is minimal: Although the UI element appears
 * to be the owner of the dispatcher, it's still responsible, as usual, to
 * call removeStatusListener same amount of times as addStatusListener,
 * otherwise the dispatcher might not be destructed. In addition this
 * implementation might hold a hard reference on the owner, and it's the
 * responsibility of the owner to destroy the dispatcher first, in order
 * to break the cycle.
 */
class StyleDispatcher final : public cppu::WeakImplHelper< css::frame::XDispatch, css::frame::XStatusListener >
{
public:
    StyleDispatcher( const css::uno::Reference< css::frame::XFrame >& rFrame,
                     const css::uno::Reference< css::util::XURLTransformer >& rUrlTransformer,
                     const css::util::URL& rURL );

    // XDispatch
    void SAL_CALL dispatch( const css::util::URL& rURL, const css::uno::Sequence< css::beans::PropertyValue >& rArguments ) override;
    void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& rListener, const css::util::URL& rURL ) override;
    void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& rListener, const css::util::URL& rURL ) override;

    // XStatusListener
    void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XEventListener
    void SAL_CALL disposing( const css::lang::EventObject& rSource ) override;

private:
    OUString m_aStyleName, m_aCommand, m_aStatusCommand;
    css::uno::Reference< css::util::XURLTransformer > m_xUrlTransformer;
    css::uno::Reference< css::frame::XDispatchProvider > m_xFrame;
    css::uno::Reference< css::frame::XDispatch > m_xStatusDispatch;
    css::uno::Reference< css::frame::XStatusListener > m_xOwner;
};

class StyleToolbarController final : public svt::ToolboxController
{
public:
    StyleToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rContext,
                            const css::uno::Reference< css::frame::XFrame >& rFrame,
                            const OUString& rCommand );

    // XUpdatable
    void SAL_CALL update() override;

    // XStatusListener
    void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XComponent
    void SAL_CALL dispose() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
