/*************************************************************************
 *
 *  $RCSfile: opluginframedispatcher.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:22 $
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

#define DISPATCHDESCRIPTOR                      ::com::sun::star::frame::DispatchDescriptor
#define MUTEX                                   ::osl::Mutex
#define OMULTITYPEINTERFACECONTAINERHELPERVAR   ::cppu::OMultiTypeInterfaceContainerHelperVar
#define OUSTRING                                ::rtl::OUString
#define OWEAKOBJECT                             ::cppu::OWeakObject
#define PROPERTYVALUE                           ::com::sun::star::beans::PropertyValue
#define REFERENCE                               ::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION                        ::com::sun::star::uno::RuntimeException
#define SEQUENCE                                ::com::sun::star::uno::Sequence
#define UNOURL                                  ::com::sun::star::util::URL
#define WEAKREFERENCE                           ::com::sun::star::uno::WeakReference
#define XDISPATCH                               ::com::sun::star::frame::XDispatch
#define XDISPATCHPROVIDER                       ::com::sun::star::frame::XDispatchProvider
#define XFRAME                                  ::com::sun::star::frame::XFrame
#define XINPUTSTREAM                            ::com::sun::star::io::XInputStream
#define XMULTISERVICEFACTORY                    ::com::sun::star::lang::XMultiServiceFactory
#define XPLUGININSTANCE                         ::com::sun::star::mozilla::XPluginInstance
#define XPLUGININSTANCENOTIFYSINK               ::com::sun::star::mozilla::XPluginInstanceNotifySink
#define XPLUGININSTANCEPEER                     ::com::sun::star::mozilla::XPluginInstancePeer
#define XSTATUSLISTENER                         ::com::sun::star::frame::XStatusListener

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

struct tIMPLExtractedArguments
{
    OUSTRING                        sReferrer       ;   // Value of extracted referrer argument
    REFERENCE< XINPUTSTREAM >       xPostDataStream ;   // Value of extracted data stream for posting

    sal_Int32                       nValidMask      ;   // state of extracted arguments
};

/*-************************************************************************************************************//**
    Use OMultiTypeInterfaceContainerHelperVar-template to create new class to get a container
    to combine string values with listeners.
*//*-*************************************************************************************************************/

struct IMPL_hashCode
{
    size_t operator()(const OUSTRING& sString) const
    {
        return sString.hashCode();
    }
};

typedef OMULTITYPEINTERFACECONTAINERHELPERVAR<  OUSTRING                  ,
                                                IMPL_hashCode             ,
                                                std::equal_to< OUSTRING > > IMPL_ListenerContainerHelper ;

/*-************************************************************************************************************//**
    @short          -
    @descr          -

    @implements     XInterface
                    XDispatchProvider
                    XDispatch
                    XPluginInstanceNotifySink
                    [ XDebugging if ENABLE_SERVICEDEBUG is defined! ]
    @base           OWeakObject

    @devstatus      deprecated
*//*-*************************************************************************************************************/

//class OPlugInFrameDispatcher  :   DERIVE_FROM_XSPECIALDEBUGINTERFACE      // => These macro will expand to nothing, if no testmode is set in debug.h!
class OPlugInFrameDispatcher    :   public XDISPATCHPROVIDER            ,
                                      public XDISPATCH                  ,
                                    public XPLUGININSTANCENOTIFYSINK    ,
                                    public OWEAKOBJECT
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
            @descr      These initialize a new instance of ths class with needed informations for work.

            @seealso    using at owner

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         OPlugInFrameDispatcher(    const   REFERENCE< XMULTISERVICEFACTORY >&      xFactory    ,
                                        MUTEX&                                  aMutex      ,
                                const   REFERENCE< XPLUGININSTANCE >&           xOwner      ,
                                const   REFERENCE< XPLUGININSTANCEPEER >&       xPlugInDLL  );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
//      DECLARE_XSPECIALDEBUGINTERFACE  // => These macro will expand to nothing, if no testmode is set in debug.h!

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

        virtual REFERENCE< XDISPATCH > SAL_CALL queryDispatch(  const   UNOURL&     aURL                ,
                                                                const   OUSTRING&   sTargetFrameName    ,
                                                                        sal_Int32   nSearchFlags        ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      search dispatcher for more the one URL
            @descr      Call this method, if you search dispatcher for more then one URL at the same time.
                        ( But these mode is not supported yet! )

            @seealso    -

            @param      "seqDescripts", descriptor to search more then one dispatcher.
            @return     A list of founded dispatcher. (if somewhere exist!)

            @onerror    An empty list is returned.
        *//*-*****************************************************************************************************/

        virtual SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescripts ) throw( RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL dispatch( const   UNOURL&                     aURL        ,
                                        const   SEQUENCE< PROPERTYVALUE >&  seqArguments) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      add listener for state events
            @descr      -

            @seealso    -

            @param      "xControl"  , reference to a valid listener for state events.
            @param      "aURL"      , URL about listener will be informed, if something occured.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addStatusListener(    const   REFERENCE< XSTATUSLISTENER >&   xControl,
                                                    const   UNOURL&                         aURL    ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      remove listener
            @descr      -

            @seealso    -

            @param      "xControl"  , reference to a valid listener.
            @param      "aURL"      , URL on which listener has registered.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeStatusListener( const   REFERENCE< XSTATUSLISTENER >&   xControl,
                                                    const   UNOURL&                         aURL    ) throw( RUNTIMEEXCEPTION );

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

        void SAL_CALL notifyURL( const OUSTRING& sURL ) throw( RUNTIMEEXCEPTION );

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

        tIMPLExtractedArguments impl_extractArguments( const SEQUENCE< PROPERTYVALUE >& seqArguments );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_sendStatusEvent(  const   OUSTRING&   sURL                ,
                                            sal_Bool    bLoadingSuccessful  );

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

         sal_Bool impldbg_checkParameter_OPlugInFrameDispatcherCtor (   const   REFERENCE< XMULTISERVICEFACTORY >&      xFactory            ,
                                                                                MUTEX&                                  aMutex              ,
                                                                        const   REFERENCE< XPLUGININSTANCE >&           xOwner              ,
                                                                        const   REFERENCE< XPLUGININSTANCEPEER >&       xPlugInDLL          );
        sal_Bool impldbg_checkParameter_queryDispatch               (   const   UNOURL&                                 aURL                ,
                                                                        const   OUSTRING&                               sTargetFrameName    ,
                                                                                sal_Int32                               nSearchFlags        );
        sal_Bool impldbg_checkParameter_queryDispatches             (   const   SEQUENCE< DISPATCHDESCRIPTOR >&         seqDescriptor       );
        sal_Bool impldbg_checkParameter_dispatch                    (   const   UNOURL&                                 aURL                ,
                                                                        const   SEQUENCE< PROPERTYVALUE >&              seqArguments        );
        sal_Bool impldbg_checkParameter_addStatusListener           (   const   REFERENCE< XSTATUSLISTENER >&           xControl            ,
                                                                        const   UNOURL&                                 aURL                );
        sal_Bool impldbg_checkParameter_removeStatusListener        (   const   REFERENCE< XSTATUSLISTENER >&           xControl            ,
                                                                        const   UNOURL&                                 aURL                );
        sal_Bool impldbg_checkParameter_notifyURL                   (   const   OUSTRING&                               sURL                );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        REFERENCE< XMULTISERVICEFACTORY >       m_xFactory              ;   /// servicemanager to create uno services
        MUTEX&                                  m_aMutex                ;   /// shared mutex with owner instance
        WEAKREFERENCE< XPLUGININSTANCE >        m_xOwnerWeak            ;   /// weakreference to owner (Don't use a hard reference. Owner can't delete us then!)
        REFERENCE< XPLUGININSTANCEPEER >        m_xPlugInDLL            ;   /// Interface for communication with browser.
        OUSTRING                                m_sTargetFrameName      ;   /// We need it as target for get/postURL.
        IMPL_ListenerContainerHelper            m_aListenerContainer    ;   /// Container to combine listeners and URLs.

};      //  class OPlugInFrameDispatcher

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_HELPER_OPLUGINFRAMEDISPATCHER_HXX_
