/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <cppuhelper/weak.hxx>

#include <unordered_map>
#include <utility>

namespace com::sun::star::frame { class XDispatch; }
namespace com::sun::star::frame { class XFrame; }
namespace cpo::uno { class XComponentContext; }

namespace svt
{

class FrameStatusListener : public css::frame::XStatusListener,
                            public css::frame::XFrameActionListener,
                            public css::lang::XComponent,
                            public ::cppu::OWeakObject
{
    public:
        FrameStatusListener( const css::uno::Reference< cpo::uno::XComponentContext >& rxContext,
                             const css::uno::Reference< css::frame::XFrame >& xFrame );
        virtual ~FrameStatusListener() override;

        // methods to support status forwarder, known by the old sfx2 toolbox controller implementation
        void addStatusListener( const OUString& aCommandURL );
        void bindListener();

        // XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

        // XComponent
        virtual void dispose() override;
        virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
        virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

        // XEventListener
        virtual void disposing( const css::lang::EventObject& Source ) override;

        // XStatusListener
        virtual void statusChanged( const css::frame::FeatureStateEvent& Event ) override = 0;

        // XFrameActionListener
        virtual void frameAction( const css::frame::FrameActionEvent& Action ) override;

    private:
        struct Listener
        {
            Listener( css::util::URL _aURL, css::uno::Reference< css::frame::XDispatch > _xDispatch ) :
                aURL(std::move( _aURL )), xDispatch(std::move( _xDispatch )) {}

            css::util::URL aURL;
            css::uno::Reference< css::frame::XDispatch > xDispatch;
        };

        typedef std::unordered_map< OUString,
                                    css::uno::Reference< css::frame::XDispatch > > URLToDispatchMap;

        bool                                                      m_bDisposed : 1;
        css::uno::Reference< css::frame::XFrame >                 m_xFrame;
        css::uno::Reference< cpo::uno::XComponentContext >        m_xContext;
        URLToDispatchMap                                          m_aListenerMap;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
