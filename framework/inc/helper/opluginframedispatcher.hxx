/*************************************************************************
 *
 *  $RCSfile: opluginframedispatcher.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:10 $
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

#ifndef __FRAMEWORK_HELPER_OPLUGINFRAMEDISPATCHER_HXX_
#define __FRAMEWORK_HELPER_OPLUGINFRAMEDISPATCHER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_OMUTEXMEMBER_HXX_
#include <helper/omutexmember.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCEPEER_HPP_
#include <com/sun/star/mozilla/XPluginInstancePeer.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCENOTIFYSINK_HPP_
#include <com/sun/star/mozilla/XPluginInstanceNotifySink.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PROPERTYVALUE.hpp>
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

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
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
    Use OMultiTypeInterfaceContainerHelperVar-template to create new class to get a container
    to combine string values with listeners.
*//*-*************************************************************************************************************/

struct IMPL_hashCode
{
    size_t operator()(const ::rtl::OUString& sString) const
    {
        return sString.hashCode();
    }
};

typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<  ::rtl::OUString                     ,
                                                        IMPL_hashCode                       ,
                                                        ::std::equal_to< ::rtl::OUString > > IMPL_ListenerContainerHelper ;

/*-************************************************************************************************************//**
    @short          -
    @descr          -

    @implements     XInterface
                    XDispatchProvider
                    XDispatch
                    XPluginInstanceNotifySink
    @base           OWeakObject

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class OPlugInFrameDispatcher    :   public css::frame::XDispatchProvider            ,
                                      public css::frame::XDispatch                  ,
                                    public css::mozilla::XPluginInstanceNotifySink  ,
                                    public ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        static void                                                     AddArguments    (   const   css::uno::Sequence< css::beans::PropertyValue > &   rArgs   ,
                                                                                            const   ::rtl::OUString&                                    rURL    );
        static const css::uno::Sequence < css::beans::PropertyValue >*  GetArguments    (   const   ::rtl::OUString&                                    rURL    );
        static void                                                     RemoveArguments (   const   ::rtl::OUString&                                    rURL    );

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard ctor
            @descr      These initialize a new instance of ths class with needed informations for work.

            @seealso    using at owner

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         OPlugInFrameDispatcher(    const   css::uno::Reference< css::lang::XMultiServiceFactory >&     xFactory    ,
                                        ::osl::Mutex&                                               aMutex      ,
                                const   css::uno::Reference< css::mozilla::XPluginInstance >&       xOwner      ,
                                const   css::uno::Reference< css::mozilla::XPluginInstancePeer >&   xPlugInDLL  );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE

        //---------------------------------------------------------------------------------------------------------
        //  XDispatchProvider
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      search a dispatcher for given URL
            @descr      If no interceptor is set on owner, we search for right frame and dispatch URL to it.
                        If no frame was found, we do nothing.

            @seealso    -

            @param      "aURL"              , URL to dispatch.
            @param      "sTargetFrameName"  , name of searched frame.
            @param      "nSearchFlags"      , flags for searching.
            @return     A reference a dispatcher for these URL (if someone was found!).

            @onerror    We return a NULL-reference.
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(    const   css::util::URL&     aURL                ,
                                                                                        const   ::rtl::OUString&    sTargetFrameName    ,
                                                                                                sal_Int32           nSearchFlags        ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      search dispatcher for more the one URL
            @descr      Call this method, if you search dispatcher for more then one URL at the same time.
                        ( But these mode is not supported yet! )

            @seealso    -

            @param      "seqDescripts", descriptor to search more then one dispatcher.
            @return     A list of founded dispatcher. (if somewhere exist!)

            @onerror    An empty list is returned.
        *//*-*****************************************************************************************************/

        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& seqDescripts ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XDispatch
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      dispatch URL with arguments
            @descr      This class implement XDispatch interface to. We use the implementation of ouer owner
                        directly to do this. He must support a function impl_loadComponent().

            @seealso    methode <owner>::impl_loadComponent()

            @param      "aURL"          , URL to dispatch.
            @param      "seqArguments"  , list of optional arguments.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL dispatch( const   css::util::URL&                                     aURL        ,
                                        const   css::uno::Sequence< css::beans::PropertyValue >&    seqArguments) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      add listener for state events
            @descr      -

            @seealso    -

            @param      "xControl"  , reference to a valid listener for state events.
            @param      "aURL"      , URL about listener will be informed, if something occured.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addStatusListener(    const   css::uno::Reference< css::frame::XStatusListener >& xControl,
                                                    const   css::util::URL&                                     aURL    ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      remove listener
            @descr      -

            @seealso    -

            @param      "xControl"  , reference to a valid listener.
            @param      "aURL"      , URL on which listener has registered.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeStatusListener( const   css::uno::Reference< css::frame::XStatusListener >& xControl,
                                                    const   css::util::URL&                                     aURL    ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XPluginInstanceNotifySink
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL notifyURL( const ::rtl::OUString& sURL ) throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.
                        This method is protected, because its not allowed to use an instance of this class as a member!
                        You MUST use a pointer.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~OPlugInFrameDispatcher();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_sendStatusEvent(  const   ::rtl::OUString&    sURL                ,
                                            sal_Bool            bLoadingSuccessful  );

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
            @return     sal_False on invalid parameter<BR>
                        sal_True  otherway

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

         static sal_Bool impldbg_checkParameter_OPlugInFrameDispatcherCtor  (   const   css::uno::Reference< css::lang::XMultiServiceFactory >&     xFactory            ,
                                                                                        ::osl::Mutex&                                               aMutex              ,
                                                                                const   css::uno::Reference< css::mozilla::XPluginInstance >&       xOwner              ,
                                                                                const   css::uno::Reference< css::mozilla::XPluginInstancePeer >&   xPlugInDLL          );
        static sal_Bool impldbg_checkParameter_queryDispatch                (   const   css::util::URL&                                             aURL                ,
                                                                                const   ::rtl::OUString&                                            sTargetFrameName    ,
                                                                                        sal_Int32                                                   nSearchFlags        );
        static sal_Bool impldbg_checkParameter_queryDispatches              (   const   css::uno::Sequence< css::frame::DispatchDescriptor >&       seqDescriptor       );
        static sal_Bool impldbg_checkParameter_dispatch                     (   const   css::util::URL&                                             aURL                ,
                                                                                const   css::uno::Sequence< css::beans::PropertyValue >&            seqArguments        );
        static sal_Bool impldbg_checkParameter_addStatusListener            (   const   css::uno::Reference< css::frame::XStatusListener >&         xControl            ,
                                                                                const   css::util::URL&                                             aURL                );
        static sal_Bool impldbg_checkParameter_removeStatusListener         (   const   css::uno::Reference< css::frame::XStatusListener >&         xControl            ,
                                                                                const   css::util::URL&                                             aURL                );
        static sal_Bool impldbg_checkParameter_notifyURL                    (   const   ::rtl::OUString&                                            sURL                );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >          m_xFactory              ;   /// servicemanager to create uno services
        ::osl::Mutex&                                                   m_aMutex                ;   /// shared mutex with owner instance
        css::uno::WeakReference< css::mozilla::XPluginInstance >        m_xOwnerWeak            ;   /// weakreference to owner (Don't use a hard reference. Owner can't delete us then!)
        css::uno::Reference< css::mozilla::XPluginInstancePeer >        m_xPlugInDLL            ;   /// Interface for communication with browser.
        ::rtl::OUString                                                 m_sTargetFrameName      ;   /// We need it as target for get/postURL.
        IMPL_ListenerContainerHelper                                    m_aListenerContainer    ;   /// Container to combine listeners and URLs.

};      //  class OPlugInFrameDispatcher

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_OPLUGINFRAMEDISPATCHER_HXX_
