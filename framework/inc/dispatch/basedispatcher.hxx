/*************************************************************************
 *
 *  $RCSfile: basedispatcher.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: as $ $Date: 2001-07-20 08:08:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_DISPATCH_BASEDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_BASEDISPATCHER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
#include <classes/taskcreator.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_HXX_
#include <threadhelp/transactionbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XLOADEVENTLISTENER_HPP_
#include <com/sun/star/frame/XLoadEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
/*DRAFT
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif
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
        inline sal_Bool searchAndForget( const css::util::URL& aURL     ,
                                               LoadBinding&    aBinding )
        {
            ResetableGuard aGuard( m_aLock );
            sal_Bool bFound = sal_False;
            for( iterator pItem=begin(); pItem!=end(); ++pItem )
            {
                if( pItem->aURL.Complete == aURL.Complete )
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
        inline sal_Bool searchAndForget( const css::uno::Reference< css::frame::XFrameLoader > xLoader  ,
                                               LoadBinding&                                    aBinding )
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
                    XStatusListener
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
                            public    css::frame::XDispatch                    ,
                            public    css::frame::XStatusListener              ,   // => XEventListener
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

        //  XInterface
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //  XDispatch
        virtual void SAL_CALL dispatch              ( const css::util::URL&                                     aURL       ,
                                                      const css::uno::Sequence< css::beans::PropertyValue >&    lArguments ) throw( css::uno::RuntimeException ) = 0;
        virtual void SAL_CALL addStatusListener     ( const css::uno::Reference< css::frame::XStatusListener >& xListener  ,
                                                      const css::util::URL&                                     aURL       ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL removeStatusListener  ( const css::uno::Reference< css::frame::XStatusListener >& xListener  ,
                                                      const css::util::URL&                                     aURL       ) throw( css::uno::RuntimeException );

        //   XStatusListener
        virtual void SAL_CALL statusChanged         ( const css::frame::FeatureStateEvent&                      aEvent     ) throw( css::uno::RuntimeException );

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
        ::rtl::OUString implts_askType              (       css::uno::Sequence< css::beans::PropertyValue >&  lDescriptor     );
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
        void            implts_sendStatusEvent      ( const css::uno::Reference< css::frame::XFrame >&        xEventSource    ,
                                                      const ::rtl::OUString&                                  sURL            ,
                                                            sal_Bool                                          bLoadState      );
/*DRAFT void            implts_updateHistory        ( const SvtHistoryItem&                                   rItem           ,
                                                            sal_Bool                                          bChange         );*/

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  - normaly they should be private ...
    //    but to support access from our super classes we should make some of them it protected!
    //-------------------------------------------------------------------------------------------------------------
    #ifdef ENABLE_ASSERTIONS
    protected:
        static sal_Bool implcp_ctor                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory     ,
                                                      const css::uno::Reference< css::frame::XFrame >&              xOwnerFrame  );
        static sal_Bool implcp_dispatch             ( const css::util::URL&                                         aURL         ,
                                                      const css::uno::Sequence< css::beans::PropertyValue >&        lArguments   );
    private:
        static sal_Bool implcp_addStatusListener    ( const css::uno::Reference< css::frame::XStatusListener >&     xListener    ,
                                                      const css::util::URL&                                         aURL         );
        static sal_Bool implcp_removeStatusListener ( const css::uno::Reference< css::frame::XStatusListener >&     xListener    ,
                                                      const css::util::URL&                                         aURL         );
        static sal_Bool implcp_statusChanged        ( const css::frame::FeatureStateEvent&                          aEvent       );
        static sal_Bool implcp_loadFinished         ( const css::uno::Reference< css::frame::XFrameLoader >&        xLoader      );
        static sal_Bool implcp_loadCancelled        ( const css::uno::Reference< css::frame::XFrameLoader >&        xLoader      );
        static sal_Bool implcp_disposing            ( const css::lang::EventObject&                                 aEvent       );
    #endif  // #ifdef ENABLE_ASSERTIONS

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
