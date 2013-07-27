/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_
#define __FRAMEWORK_DISPATCH_SYSTEMEXEC_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <macros/debug.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/**
    @short          protocol handler for "systemexec:*" URLs
    @descr          It's a special dispatch/provider object which is registered for such URL pattern and will
                    be automaticly used by the framework dispatch mechanism if such URL occured.
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
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /* interface */
    public:

        // ctor/dtor
                 SystemExec( const css::uno::Reference< css::uno::XComponentContext >& xContext );
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
