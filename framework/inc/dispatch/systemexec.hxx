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

#ifndef __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_
#define __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
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

#include <cppuhelper/weak.hxx>

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
class SystemExec : // interfaces
                       public  css::lang::XTypeProvider      ,
                       public  css::lang::XServiceInfo       ,
                       public  css::frame::XDispatchProvider ,
                       public  css::frame::XNotifyingDispatch, // => XDispatch
                       // baseclasses
                       // Order is neccessary for right initialization!
                       private ThreadHelpBase                ,
                       public  cppu::OWeakObject
{
    /* member */
    private:

        /// reference to global uno service manager which had created us
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /* interface */
    public:

        // ctor/dtor
                 SystemExec( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~SystemExec(                                                                        );

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL                       queryDispatch  ( const css::util::URL&                                       aURL        ,
                                                                                                             const ::rtl::OUString&                                      sTarget     ,
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
