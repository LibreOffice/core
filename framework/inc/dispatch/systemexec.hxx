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

#ifndef __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_
#define __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

#include <cppuhelper/implbase3.hxx>

namespace framework{

/**
    @short          protocol handler for "systemexec:*" URLs
    @descr          It's a special dispatch/provider object which is registered for such URL pattern and will
                    be automaticly used by the framework dispatch mechanism if such URL occurred.
                    It forwards all URL's to the underlying operating system.
                    So it would e.g. be possible to open HTML files outside the office within a web browser.

    @base           ThreadHelpBase
                        exports a lock member to guarantee right initialize value of it
    @base           OWeakObject
                        provides XWeak and ref count mechanism

    @devstatus      ready to use
*/
class SystemExec :     // baseclasses
                       // Order is necessary for right initialization!
                       private ThreadHelpBase                ,
                       // interfaces
                       public  ::cppu::WeakImplHelper3<
                                   css::lang::XServiceInfo       ,
                                   css::frame::XDispatchProvider ,
                                   css::frame::XNotifyingDispatch > // => XDispatch
{
    /* member */
    private:

        /// reference to global uno service manager which had created us
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /* interface */
    public:

        // ctor/dtor
                 SystemExec( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~SystemExec(                                                                     );

        // XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL                       queryDispatch  ( const css::util::URL&                                       aURL        ,
                                                                                                             const OUString&                                      sTarget     ,
                                                                                                                   sal_Int32                                             nFlags      ) throw( css::uno::RuntimeException );
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException );

        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw( css::uno::RuntimeException );

        // XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                     aURL       ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&    lArguments ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL addStatusListener   ( const css::uno::Reference< css::frame::XStatusListener >& xListener  ,
                                                    const css::util::URL&                                     aURL       ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener  ,
                                                    const css::util::URL&                                     aURL       ) throw( css::uno::RuntimeException );

    /* internal */
    private:

        void impl_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                       const sal_Int16                                                   nState   );

};      //  class SystemExec

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
