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

#ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_
#define __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_

#include <services/frame.hxx>

#include <classes/protocolhandlercache.hxx>
#include <threadhelp/threadhelpbase.hxx>

#include <threadhelp/transactionbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/generic.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework{

/**
    @descr          We know some special dispatch objects with different functionality.
                    The can be created internaly by the following DispatchProvider.
                    Here we define some identifier to force creation of the right one.
*/
enum EDispatchHelper
{
    E_DEFAULTDISPATCHER     ,
    E_MENUDISPATCHER        ,
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
                    XInterface will be ambigous and we hold a weakreference to ouer OWNER - not to ouer SUPERCLASS!

    @base           ThreadHelpBase
                        supports threadsafe mechanism
    @base           OWeakObject
                        provides ref count and weak mechanism

    @devstatus      ready to use
    @threadsafe     yes
*/
class DispatchProvider  :   // baseclasses
                            // Order is necessary for right initialization!
                            private ThreadHelpBase                      ,
                            private TransactionBase                     ,
                            // interfaces
                            public  ::cppu::WeakImplHelper1< css::frame::XDispatchProvider >
{
    /* member */
    private:
        /// reference to global service manager to create new services
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
        /// weakreference to owner frame (Don't use a hard reference. Owner can't delete us then!)
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;
        /// different dispatcher to handle special dispatch calls, protocols or URLs (they will be created on demand.)
        css::uno::Reference< css::frame::XDispatch > m_xMenuDispatcher     ;
        /// cache of some other dispatch provider which are registered inside configuration to handle special URL protocols
        HandlerCache m_aProtocolHandlerCache;

    /* interface */
    public:

        DispatchProvider( const css::uno::Reference< css::uno::XComponentContext >&     xContext ,
                          const css::uno::Reference< css::frame::XFrame >&              xFrame   );

        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL                       queryDispatch  ( const css::util::URL&                                       aURL             ,
                                                                                                             const OUString&                                      sTargetFrameName ,
                                                                                                                   sal_Int32                                             nSearchFlags     ) throw( css::uno::RuntimeException );
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) throw( css::uno::RuntimeException );

    /* helper */
    protected:
        // Let him protected! So nobody can use us as base ...
        virtual ~DispatchProvider();

    private:
        css::uno::Reference< css::frame::XDispatch > implts_getOrCreateDispatchHelper   (       EDispatchHelper                            eHelper                       ,
                                                                                          const css::uno::Reference< css::frame::XFrame >& xOwner                        ,
                                                                                          const OUString&                           sTarget = OUString()   ,
                                                                                                sal_Int32                                  nSearchFlags = 0              );
        sal_Bool                                     implts_isLoadableContent           ( const css::util::URL&                            aURL                          );
        css::uno::Reference< css::frame::XDispatch > implts_queryDesktopDispatch        ( const css::uno::Reference< css::frame::XFrame >  xDesktop                      ,
                                                                                          const css::util::URL&                            aURL                          ,
                                                                                          const OUString&                           sTargetFrameName              ,
                                                                                                sal_Int32                                  nSearchFlags                  );
        css::uno::Reference< css::frame::XDispatch > implts_queryFrameDispatch          ( const css::uno::Reference< css::frame::XFrame >  xFrame                        ,
                                                                                          const css::util::URL&                            aURL                          ,
                                                                                          const OUString&                           sTargetFrameName              ,
                                                                                                sal_Int32                                  nSearchFlags                  );
        css::uno::Reference< css::frame::XDispatch > implts_searchProtocolHandler       ( const css::util::URL&                            aURL                          );

}; // class DispatchProvider

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
