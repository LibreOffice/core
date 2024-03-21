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

#include <classes/protocolhandlercache.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework{

/**
    @descr          We know some special dispatch objects with different functionality.
                    The can be created internally by the following DispatchProvider.
                    Here we define some identifier to force creation of the right one.
*/
enum EDispatchHelper
{
    E_DEFAULTDISPATCHER     ,
    E_CREATEDISPATCHER      ,
    E_BLANKDISPATCHER       ,
    E_SELFDISPATCHER        ,
    E_CLOSEDISPATCHER       ,
    E_STARTMODULEDISPATCHER
};

/**
    @short          implement a helper for XDispatchProvider interface
    @descr          The result of a queryDispatch() call depends from the owner, which use an instance of this class.
                    (frame, desktop) All of them must provides different functionality.
                    E.g:    - task can be created by the desktop only
                            - a task can have a beamer as direct child
                            - a normal frame never can create a new one by himself

    @attention      Use this class as member only! Never use it as baseclass.
                    XInterface will be ambiguous and we hold a weakreference to our OWNER - not to our SUPERCLASS!

    @base           OWeakObject
                        provides ref count and weak mechanism

    @devstatus      ready to use
    @threadsafe     yes
*/
class DispatchProvider final : public ::cppu::WeakImplHelper< css::frame::XDispatchProvider >
{
    /* member */
    private:
        /// reference to global service manager to create new services
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
        /// weakreference to owner frame (Don't use a hard reference. Owner can't delete us then!)
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;
        /// cache of some other dispatch provider which are registered inside configuration to handle special URL protocols
        HandlerCache m_aProtocolHandlerCache;
        std::unordered_map<OUString, css::uno::Reference<css::frame::XDispatchProvider>>
            m_aProtocolHandlers;

    /* interface */
    public:

        DispatchProvider( css::uno::Reference< css::uno::XComponentContext >      xContext ,
                          const css::uno::Reference< css::frame::XFrame >&        xFrame   );

        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL                       queryDispatch  ( const css::util::URL&                                       aURL             ,
                                                                                                             const OUString&                                      sTargetFrameName ,
                                                                                                                   sal_Int32                                             nSearchFlags     ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) override;

        void ClearProtocolHandlers() { m_aProtocolHandlers.clear(); }

    /* helper */
    private:
        // Let him protected! So nobody can use us as base ...
        virtual ~DispatchProvider() override;

        css::uno::Reference< css::frame::XDispatch > implts_getOrCreateDispatchHelper   (       EDispatchHelper                            eHelper                       ,
                                                                                          const css::uno::Reference< css::frame::XFrame >& xOwner                        ,
                                                                                          const OUString&                           sTarget = OUString()   ,
                                                                                                sal_Int32                                  nSearchFlags = 0              );
        static bool                                  implts_isLoadableContent           ( const css::util::URL&                            aURL                          );
        css::uno::Reference< css::frame::XDispatch > implts_queryDesktopDispatch        ( const css::uno::Reference< css::frame::XFrame >&  xDesktop                      ,
                                                                                          const css::util::URL&                            aURL                          ,
                                                                                          const OUString&                           sTargetFrameName              ,
                                                                                                sal_Int32                                  nSearchFlags                  );
        css::uno::Reference< css::frame::XDispatch > implts_queryFrameDispatch          ( const css::uno::Reference< css::frame::XFrame >&  xFrame                        ,
                                                                                          const css::util::URL&                            aURL                          ,
                                                                                          const OUString&                           sTargetFrameName              ,
                                                                                                sal_Int32                                  nSearchFlags                  );
        css::uno::Reference< css::frame::XDispatch > implts_searchProtocolHandler       ( const css::util::URL&                            aURL                          );

}; // class DispatchProvider

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
