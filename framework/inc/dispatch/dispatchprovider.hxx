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



#ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_
#define __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <services/frame.hxx>

#include <classes/protocolhandlercache.hxx>
#include <threadhelp/threadhelpbase.hxx>

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_HXX_
#include <threadhelp/transactionbase.hxx>
#endif
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________

/**
    @descr          We know some special dispatch objects with different functionality.
                    The can be created internaly by the following DispatchProvider.
                    Here we define some identifier to force creation of the right one.
*/
enum EDispatchHelper
{
    E_DEFAULTDISPATCHER     ,
    E_MENUDISPATCHER        ,
    E_HELPAGENTDISPATCHER   ,
    E_CREATEDISPATCHER      ,
    E_BLANKDISPATCHER       ,
    E_SELFDISPATCHER        ,
    E_CLOSEDISPATCHER       ,
    E_STARTMODULEDISPATCHER
};

//_________________________________________________________________________________________________________________

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
    @modified       17.05.2002 07:56, as96863
*/
class DispatchProvider  :   // interfaces
                            public  css::lang::XTypeProvider            ,
                            public  css::frame::XDispatchProvider       ,
                            // baseclasses
                            // Order is necessary for right initialization!
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
        css::uno::Reference< css::frame::XDispatch > m_xHelpAgentDispatcher;
/*      css::uno::Reference< css::frame::XDispatch > m_xBlankDispatcher    ;
        css::uno::Reference< css::frame::XDispatch > m_xSelfDispatcher     ;
        css::uno::Reference< css::frame::XDispatch > m_xDefaultDispatcher  ;*/
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
