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

#ifndef __FRAMEWORK_DISPATCH_BASEDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_BASEDISPATCHER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <classes/taskcreator.hxx>
#include <threadhelp/resetableguard.hxx>
#include <threadhelp/threadhelpbase.hxx>

#include <threadhelp/transactionbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/interfacecontainer.h>
/*DRAFT
#include <unotools/historyoptions.hxx>
*/

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

/*-************************************************************************************************************//**
    @descr      We must support loading of different URLs with different handler or loader into different tasks simultaniously.
                They call us back to return state of operation. We need some informations to distinguish
                between these different "loading threads".
                This is the reason to implement this dynamicly list.

    @attention  I maked class LoaderThreads threadsafe! Using will be easier in a multithreaded environment.
                struct DispatchBinding doesn't need that!
*//*-*************************************************************************************************************/
struct LoadBinding
{
    //-------------------------------------------------------------------------------------------------------------
    public:

        //---------------------------------------------------------------------------------------------------------
        inline LoadBinding()
        {
            free();
        }

        //---------------------------------------------------------------------------------------------------------
        // use to initialize struct for asynchronous dispatching by using handler
        inline LoadBinding( const css::util::URL&                                   aNewURL         ,
                            const css::uno::Sequence< css::beans::PropertyValue >   lNewDescriptor  ,
                            const css::uno::Reference< css::frame::XDispatch >&     xNewHandler     ,
                            const css::uno::Any&                                    aNewAsyncInfo   )
        {
            free();
            xHandler    = xNewHandler   ;
            aURL        = aNewURL       ;
            lDescriptor = lNewDescriptor;
            aAsyncInfo  = aNewAsyncInfo ;
        }

        //---------------------------------------------------------------------------------------------------------
        // use to initialize struct for asynchronous loading by using frame loader
        inline LoadBinding( const css::util::URL&                                   aNewURL         ,
                            const css::uno::Sequence< css::beans::PropertyValue >   lNewDescriptor  ,
                            const css::uno::Reference< css::frame::XFrame >&        xNewFrame       ,
                            const css::uno::Reference< css::frame::XFrameLoader >&  xNewLoader      ,
                            const css::uno::Any&                                    aNewAsyncInfo   )
        {
            free();
            xLoader     = xNewLoader    ;
            xFrame      = xNewFrame     ;
            aURL        = aNewURL       ;
            lDescriptor = lNewDescriptor;
            aAsyncInfo  = aNewAsyncInfo ;
        }

        //---------------------------------------------------------------------------------------------------------
        // dont forget toe release used references
        inline ~LoadBinding()
        {
            free();
        }

        //---------------------------------------------------------------------------------------------------------
        inline void free()
        {
            xHandler    = css::uno::Reference< css::frame::XDispatch >()   ;
            xLoader     = css::uno::Reference< css::frame::XFrameLoader >();
            xFrame      = css::uno::Reference< css::frame::XFrame >()      ;
            aURL        = css::util::URL()                                 ;
            lDescriptor = css::uno::Sequence< css::beans::PropertyValue >();
            aAsyncInfo  = css::uno::Any()                                  ;
        }

    //-------------------------------------------------------------------------------------------------------------
    public:
        css::uno::Reference< css::frame::XDispatch >        xHandler    ;   // if handler was used, this reference will be valid
        css::uno::Reference< css::frame::XFrameLoader >     xLoader     ;   // if loader was used, this reference will be valid
        css::uno::Reference< css::frame::XFrame >           xFrame      ;   // Target of loading
        css::util::URL                                      aURL        ;   // dispatched URL - neccessary to find listener for status event!
        css::uno::Sequence< css::beans::PropertyValue >     lDescriptor ;   // dispatched arguments - neccessary for "reactForLoadingState()"!
        css::uno::Any                                       aAsyncInfo  ;   // superclasses could use them to save her own user specific data for these asynchron call-info
        css::uno::Reference< css::frame::XDispatchResultListener > xListener;
};

//*****************************************************************************************************************
class LoaderThreads : private ::std::vector< LoadBinding >
                    , private ThreadHelpBase
{
    //-------------------------------------------------------------------------------------------------------------
    public:

        //---------------------------------------------------------------------------------------------------------
        inline LoaderThreads()
                : ThreadHelpBase()
        {
        }

        //---------------------------------------------------------------------------------------------------------
        inline void append( const LoadBinding& aBinding )
        {
            ResetableGuard aGuard( m_aLock );
            push_back( aBinding );
        }

        //---------------------------------------------------------------------------------------------------------
        /// search for handler thread in list wich match given parameter and delete it
        inline sal_Bool searchAndForget( const css::uno::Reference < css::frame::XDispatchResultListener >& rListener, LoadBinding& aBinding )
        {
            ResetableGuard aGuard( m_aLock );
            sal_Bool bFound = sal_False;
            for( iterator pItem=begin(); pItem!=end(); ++pItem )
            {
                if( pItem->xListener == rListener )
                {
                    aBinding = *pItem;
                    erase( pItem );
                    bFound = sal_True;
                    break;
                }
            }
            return bFound;
        }

        //---------------------------------------------------------------------------------------------------------
        /// search for loader thread in list wich match given parameter and delete it
        inline sal_Bool searchAndForget( const css::uno::Reference< css::frame::XFrameLoader > xLoader, LoadBinding& aBinding )
        {
            ResetableGuard aGuard( m_aLock );
            sal_Bool bFound = sal_False;
            for( iterator pItem=begin(); pItem!=end(); ++pItem )
            {
                if( pItem->xLoader == xLoader )
                {
                    aBinding = *pItem;
                    erase( pItem );
                    bFound = sal_True;
                    break;
                }
            }
            return bFound;
        }

        //---------------------------------------------------------------------------------------------------------
        // free ALL memory ... I hope it
        inline void free()
        {
            ResetableGuard aGuard( m_aLock );
            LoaderThreads().swap( *this );
        }
};

/*-************************************************************************************************************//**
    @short          base class for dispatcher implementations
    @descr          Most of our dispatch implementations do everytime the same. They try to handle or load
                    somethinmg into a target ... normaly a frame/task/pluginframe!
                    They must do it synchron or sometimes asynchron. They must wait for callbacks and
                    notify registered listener with right status events.
                    All these things are implemented by this baseclass. You should override some methods
                    to change something.

                    "dispatch()"                =>  should be you dispatch algorithm
                    "reactForLoadingState()"    =>  do something depending from loading state ...

    @implements     XInterface
                    XDispatch
                    XLoadEventListener
                    XEventListener

    @base           ThreadHelpBase
                    TransactionBase
                    OWeakObject

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class BaseDispatcher    :   // interfaces
                            public    css::lang::XTypeProvider                 ,
                            public    css::frame::XNotifyingDispatch           ,
                            public    css::frame::XLoadEventListener           ,   // => XEventListener too!
                            // baseclasses
                            // Order is neccessary for right initialization!
                            protected ThreadHelpBase                           ,
                            protected TransactionBase                          ,
                            public    ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //  constructor / destructor
        BaseDispatcher( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory    ,
                        const css::uno::Reference< css::frame::XFrame >&              xOwnerFrame );

        void dispatchFinished ( const css::frame::DispatchResultEvent& aEvent, const css::uno::Reference < css::frame::XDispatchResultListener >& rListener );

        //  XInterface
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //  XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification ( const css::util::URL& aURL,
                                                      const css::uno::Sequence< css::beans::PropertyValue >& aArgs,
                                                      const css::uno::Reference< css::frame::XDispatchResultListener >& Listener ) throw ( css::uno::RuntimeException);

        //  XDispatch
        virtual void SAL_CALL dispatch              ( const css::util::URL&                                     aURL       ,
                                                      const css::uno::Sequence< css::beans::PropertyValue >&    lArguments ) throw( css::uno::RuntimeException ) = 0;
        virtual void SAL_CALL addStatusListener     ( const css::uno::Reference< css::frame::XStatusListener >& xListener  ,
                                                      const css::util::URL&                                     aURL       ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL removeStatusListener  ( const css::uno::Reference< css::frame::XStatusListener >& xListener  ,
                                                      const css::util::URL&                                     aURL       ) throw( css::uno::RuntimeException );

        //   XLoadEventListener
        virtual void SAL_CALL loadFinished          ( const css::uno::Reference< css::frame::XFrameLoader >&    xLoader    ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL loadCancelled         ( const css::uno::Reference< css::frame::XFrameLoader >&    xLoader    ) throw( css::uno::RuntimeException );

        //   XEventListener
        virtual void SAL_CALL disposing             ( const css::lang::EventObject&                             aEvent     ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:
        virtual ~BaseDispatcher();

        /*-****************************************************************************************************//**
            @short      you should react for successfully or failed load/handle operations.
            @descr      These baseclass implement handling of dispatched URLs and synchronous/asynchronous loading
                        of it into a target frame. It implement the complete listener mechanism to get events from
                        used loader or handler and sending of status events to registered listener too!
                        But we couldn't react for this events in all cases.
                        May be - you wish to reactivate suspended controllers or wish to delete a new created
                        task if operation failed ...!?
                        By overwriting these pure virtual methods it's possible to do such things.
                        We call you with all available informations ... you should react for it.
                        BUT - don't send any status events to your listener! We will do it everytime.
                        (other listener could be informed as well!)

                        You will called back in: a) "reactForLoadingState()" , if URL was loaded into a frame
                                                 b) "reactForHandlingState()", if URL was handled by a registered content handler
                                                                               (without using a target frame!)

            @seealso    method statusChanged()
            @seealso    method loadFinished()
            @seealso    method loadCancelled()

            @param      "aURL"         , original dispatched URL
            @param      "lDescriptor"  , original dispatched arguments
            @param      "xTarget"      , target of operation (could be NULL if URL was handled not loaded!)
            @param      "bState"       , state of operation
            @return     -

            @onerror    -
            @threadsafe -
        *//*-*****************************************************************************************************/
        virtual void SAL_CALL reactForLoadingState ( const css::util::URL&                                  aURL          ,
                                                     const css::uno::Sequence< css::beans::PropertyValue >& lDescriptor   ,
                                                     const css::uno::Reference< css::frame::XFrame >&       xTarget       ,
                                                           sal_Bool                                         bState        ,
                                                     const css::uno::Any&                                   aAsyncInfo    ) = 0;

        virtual void SAL_CALL reactForHandlingState( const css::util::URL&                                  aURL          ,
                                                     const css::uno::Sequence< css::beans::PropertyValue >& lDescriptor   ,
                                                           sal_Bool                                         bState        ,
                                                     const css::uno::Any&                                   aAsyncInfo    ) = 0;

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:
        ::rtl::OUString implts_detectType           ( const css::util::URL&                                   aURL            ,
                                                            css::uno::Sequence< css::beans::PropertyValue >&  lDescriptor     ,
                                                            sal_Bool                                          bDeep           );
        sal_Bool        implts_handleIt             ( const css::util::URL&                                   aURL            ,
                                                            css::uno::Sequence< css::beans::PropertyValue >&  lDescriptor     ,
                                                      const ::rtl::OUString&                                  sTypeName       ,
                                                      const css::uno::Any&                                    aAsyncInfo      = css::uno::Any() );
        sal_Bool        implts_loadIt               ( const css::util::URL&                                   aURL            ,
                                                            css::uno::Sequence< css::beans::PropertyValue >&  lDescriptor     ,
                                                      const ::rtl::OUString&                                  sTypeName       ,
                                                      const css::uno::Reference< css::frame::XFrame >&        xTarget         ,
                                                      const css::uno::Any&                                    aAsyncInfo      = css::uno::Any() );
        void            implts_enableFrame          ( const css::uno::Reference< css::frame::XFrame >&        xFrame          ,
                                                      const css::uno::Sequence< css::beans::PropertyValue >&  lDescriptor     );
        void            implts_disableFrame         ( const css::uno::Reference< css::frame::XFrame >&        xFrame          );
        sal_Bool        implts_deactivateController ( const css::uno::Reference< css::frame::XController >&   xController     );
        sal_Bool        implts_reactivateController ( const css::uno::Reference< css::frame::XController >&   xController     );
        void            implts_sendResultEvent      ( const css::uno::Reference< css::frame::XFrame >&        xEventSource    ,
                                                      const ::rtl::OUString&                                  sURL            ,
                                                            sal_Bool                                          bLoadState      );

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  - should be private normaly ...
    //  - but some super classes need access to some of them => protected!
    //-------------------------------------------------------------------------------------------------------------
    protected:
        css::uno::Reference< css::lang::XMultiServiceFactory >      m_xFactory            ;   /// global uno service manager to create new services
        css::uno::WeakReference< css::frame::XFrame >               m_xOwner              ;   /// weakreference to owner (Don't use a hard reference. Owner can't delete us then!)

    private:
        LoaderThreads                                               m_aLoaderThreads      ;   /// list of bindings between handler/loader, tasks and loaded URLs
        ListenerHash                                                m_aListenerContainer  ;   /// hash table for listener at specified URLs

};      //  class BaseDispatcher

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_DISPATCH_BASEDISPATCHER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
