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

#ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_
#define __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_

#include <services/frame.hxx>

#include <classes/protocolhandlercache.hxx>
#include <threadhelp/threadhelpbase.hxx>

#include <threadhelp/transactionbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework{

/**
    @descr          We know some special dispatch objects with diffrent functionality.
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
class DispatchProvider  :   // interfaces
                            public  css::lang::XTypeProvider            ,
                            public  css::frame::XDispatchProvider       ,
                            // baseclasses
                            // Order is neccessary for right initialization!
                            private ThreadHelpBase                      ,
                            private TransactionBase                     ,
                            public  ::cppu::OWeakObject
{
    /* member */
    private:
        /// reference to global service manager to create new services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
        /// weakreference to owner frame (Don't use a hard reference. Owner can't delete us then!)
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;
        /// different dispatcher to handle special dispatch calls, protocols or URLs (they will be created on demand.)
        css::uno::Reference< css::frame::XDispatch > m_xMenuDispatcher     ;
        /// cache of some other dispatch provider which are registered inside configuration to handle special URL protocols
        HandlerCache m_aProtocolHandlerCache;

    /* interface */
    public:
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        DispatchProvider( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory ,
                          const css::uno::Reference< css::frame::XFrame >&              xFrame   );

        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL                       queryDispatch  ( const css::util::URL&                                       aURL             ,
                                                                                                             const ::rtl::OUString&                                      sTargetFrameName ,
                                                                                                                   sal_Int32                                             nSearchFlags     ) throw( css::uno::RuntimeException );
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptions    ) throw( css::uno::RuntimeException );

    /* helper */
    protected:
        // Let him protected! So nobody can use us as base ...
        virtual ~DispatchProvider();

    private:
        css::uno::Reference< css::frame::XDispatch > implts_getOrCreateDispatchHelper   (       EDispatchHelper                            eHelper                       ,
                                                                                          const css::uno::Reference< css::frame::XFrame >& xOwner                        ,
                                                                                          const ::rtl::OUString&                           sTarget = ::rtl::OUString()   ,
                                                                                                sal_Int32                                  nSearchFlags = 0              );
        sal_Bool                                     implts_isLoadableContent           ( const css::util::URL&                            aURL                          );
        css::uno::Reference< css::frame::XDispatch > implts_queryDesktopDispatch        ( const css::uno::Reference< css::frame::XFrame >  xDesktop                      ,
                                                                                          const css::util::URL&                            aURL                          ,
                                                                                          const ::rtl::OUString&                           sTargetFrameName              ,
                                                                                                sal_Int32                                  nSearchFlags                  );
        css::uno::Reference< css::frame::XDispatch > implts_queryFrameDispatch          ( const css::uno::Reference< css::frame::XFrame >  xFrame                        ,
                                                                                          const css::util::URL&                            aURL                          ,
                                                                                          const ::rtl::OUString&                           sTargetFrameName              ,
                                                                                                sal_Int32                                  nSearchFlags                  );
        css::uno::Reference< css::frame::XDispatch > implts_searchProtocolHandler       ( const css::util::URL&                            aURL                          );

}; // class DispatchProvider

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
