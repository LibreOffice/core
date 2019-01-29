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

#ifndef INCLUDED_SVTOOLS_FRAMESTATUSLISTENER_HXX
#define INCLUDED_SVTOOLS_FRAMESTATUSLISTENER_HXX

#include <svtools/svtdllapi.h>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/basemutex.hxx>

#include <unordered_map>

namespace com :: sun :: star :: frame { class XDispatch; }
namespace com :: sun :: star :: frame { class XFrame; }
namespace com :: sun :: star :: uno { class XComponentContext; }

namespace svt
{

class SVT_DLLPUBLIC FrameStatusListener : public css::frame::XStatusListener,
                            public css::frame::XFrameActionListener,
                            public css::lang::XComponent,
                            public ::cppu::BaseMutex,
                            public ::cppu::OWeakObject
{
    public:
        FrameStatusListener( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                             const css::uno::Reference< css::frame::XFrame >& xFrame );
        virtual ~FrameStatusListener() override;

        // methods to support status forwarder, known by the old sfx2 toolbox controller implementation
        void addStatusListener( const OUString& aCommandURL );
        void bindListener();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XComponent
        virtual void SAL_CALL dispose() override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override = 0;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& Action ) override;

    protected:
        struct Listener
        {
            Listener( const css::util::URL& rURL, const css::uno::Reference< css::frame::XDispatch >& rDispatch ) :
                aURL( rURL ), xDispatch( rDispatch ) {}

            css::util::URL const aURL;
            css::uno::Reference< css::frame::XDispatch > xDispatch;
        };

        typedef std::unordered_map< OUString,
                                    css::uno::Reference< css::frame::XDispatch > > URLToDispatchMap;

        bool                                                      m_bDisposed : 1;
        css::uno::Reference< css::frame::XFrame >                 m_xFrame;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        URLToDispatchMap                                          m_aListenerMap;
};

}

#endif // INCLUDED_SVTOOLS_FRAMESTATUSLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
