/*************************************************************************
 *
 *  $RCSfile: interceptionhelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-07-02 13:20:35 $
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

#ifndef __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_
#define __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#include <services/frame.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
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

#ifndef __SGI_STL_DEQUE
#include <deque>
#endif

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
    We must save a performant list with URL pattern for all registered interceptor objects.
    We implement this as a dynamical vector of interceptor references with a URL list for every item.
*//*-*************************************************************************************************************/

struct  IMPL_TInterceptorInfo
{
    css::uno::Reference< css::frame::XDispatchProviderInterceptor > xInterceptor    ;
    css::uno::Sequence< ::rtl::OUString >                           seqPatternList  ;
};

class IMPL_CInterceptorList : public ::std::deque< IMPL_TInterceptorInfo >
{
    public:
        // Implement our own find method to search for an interceptor in our list of structures!
        // We can't search for it directly with ::std::find_if() or something else.
        iterator find( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor )
        {
            // I hope that the iterator has a right implemented ++operator and we arrive end() exactly!
            // If it is so - we can use aItem->... without any problems and must not check it.
            iterator aItem;
            for( aItem=begin(); aItem!=end(); ++aItem )
            {
                if( aItem->xInterceptor == xInterceptor )
                {
                    break;
                }
            }
            return aItem;
        }
};

/*-************************************************************************************************************//**
    @short          implement a helper to support interception with additional functionality
    @descr          These helper implement the complete XDispatchProviderInterception interface with
                    master/slave functionality AND using of optional features like URL lists!

    @implements     XInterface
                    XDispatchProviderInterception
    @base           ThreadHelpBase
                    OWeakObject

    @attention      Don't use this class as direct member - use it dynamicly. Do not derive from this class.
                    We hold a weakreference to ouer owner not to ouer superclass.

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/

class InterceptionHelper   :   public css::frame::XDispatchProvider                ,
                                public css::frame::XDispatchProviderInterception    ,
                                public css::lang::XEventListener                    ,
                                public ThreadHelpBase                                   ,
                                public ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard ctor
            @descr      These initialize a new instance of this class with all needed informations for work.
                        We share mutex with owner implementation and hold a weakreference to it!

            @seealso    -

            @param      "xSlaveDispatcher"  , reference to a dispatch helper of our owner. We need it as slave for registered interceptors.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        InterceptionHelper(    const   css::uno::Reference< css::frame::XFrame >&              xFrame              ,
                                const   css::uno::Reference< css::frame::XDispatchProvider >&   xSlaveDispatcher    );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE

        //---------------------------------------------------------------------------------------------------------
        //  XDispatchProvider
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      query for a dispatcher for given parameter
            @descr      If somebody will dispatch a URL he must have a valid dispatch object to do it.
                        With these function you he can get these object ... if target exist or could create!
                        If an interceptor is registered for given URL we used - otherwise not.

            @seealso    interface XDispatch
            @seealso    method queryDispatches()

            @param      "aURL"              , the URL to dispatch
            @param      "sTargetFrameName"  , the name of the target frame or a special name like "_blank", "_top" ...
            @param      "nSearchFlags"      , optional search parameter for targeting
            @return     -

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(    const   css::util::URL&     aURL            ,
                                                                                        const   ::rtl::OUString&    sTargetFrameName,
                                                                                                sal_Int32           nSearchFlags    ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      query for more then one dispatcher at the same time
            @descr      These function do the same like queryDispatch() before, but for a lot of URLs at the same time.
                        A registered interceptor is used automaticly.

            @seealso    interface XDispatch
            @seealso    method queryDispatch()

            @param      "aDescriptor", list to describe more then one dispatches
            @return     -

            @onerror    An empty list is returned or if one dispatch was wrong one result is missing!
        *//*-*****************************************************************************************************/

        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& seqDescriptor ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XDispatchProviderInterception
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      register an interceptor for dispatches
            @descr      Somebody can register himself to intercept all or some special dispatches.
                        It's depend from his supported interfaces. If he implement XInterceptorInfo
                        he his called for some special URLs only - otherwise we call it for every request!

            @seealso    interface XDispatchProviderInterceptor
            @seealso    interface XInterceptorInfo
            @seealso    method releaseDispatchProviderInterceptor()

            @param      "xInterceptor", reference to interceptor for register
            @return     -

            @onerror    Listener is'nt added to our container.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      release an interceptor for dispatches
            @descr      Remove registered interceptor from our internal list and all special informations about him.

            @seealso    method registerDispatchProviderInterceptor()

            @param      "xInterceptor", reference to interceptor for release
            @return     -

            @onerror    Listener is'nt removed from our container.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XEventListener
        //---------------------------------------------------------------------------------------------------------

        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        This method is protected, because its not allowed to use this class as a direct member!
                        You MUST use a dynamical instance (pointer). That's the reason for a protected dtor.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~InterceptionHelper();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      search an interceptor which is registered for given URL
            @descr      We search in our list to get the right interceptor, which wish to intercept these URL.
                        We don't must use the highest one!

            @seealso    -

            @param      "sURL", URL which must match with a registered pattern
            @return     Reference to a registered interceptor for these URL or NULL if no object was found.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        css::uno::Reference< css::frame::XDispatchProviderInterceptor > impl_searchMatchingInterceptor( const ::rtl::OUString& sURL );

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False ,on invalid parameter
            @return     sal_True  ,otherwise

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        static sal_Bool impldbg_checkParameter_InterceptionHelper                  (   const   css::uno::Reference< css::frame::XDispatchProvider >&               xSlaveDispatcher);
        static sal_Bool impldbg_checkParameter_queryDispatch                        (   const   css::util::URL&                                                     aURL            ,
                                                                                        const   ::rtl::OUString&                                                    sTargetFrameName,
                                                                                                sal_Int32                                                           nSearchFlags    );
        static sal_Bool impldbg_checkParameter_queryDispatches                      (   const   css::uno::Sequence< css::frame::DispatchDescriptor >&               seqDescriptor   );
        static sal_Bool impldbg_checkParameter_registerDispatchProviderInterceptor  (   const   css::uno::Reference< css::frame::XDispatchProviderInterceptor >&    xInterceptor    );
        static sal_Bool impldbg_checkParameter_releaseDispatchProviderInterceptor   (   const   css::uno::Reference< css::frame::XDispatchProviderInterceptor >&    xInterceptor    );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::WeakReference< css::frame::XFrame >                   m_xOwnerWeak            ;   /// weakreference to owner (Don't use a hard reference. Owner can't delete us then!)
        css::uno::Reference< css::frame::XDispatchProvider >            m_xSlaveDispatcher      ;   /// helper for XDispatchProvider and XDispatch interfaces
        IMPL_CInterceptorList                                           m_aInterceptorList      ;   /// an interceptor can register himself for a list of URLs or URL pattern

};      //  class InterceptionHelper

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_
