/*************************************************************************
 *
 *  $RCSfile: pluginframe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2001-01-26 08:39:19 $
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

#ifndef __FRAMEWORK_SERVICES_PLUGINFRAME_HXX_
#define __FRAMEWORK_SERVICES_PLUGINFRAME_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_TASK_HXX_
#include <services/task.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCEPEER_HPP_
#include <com/sun/star/mozilla/XPluginInstancePeer.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGINWINDOWPEER_HPP_
#include <com/sun/star/mozilla/XPluginWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _VCL_ATOM_HXX
#include <vcl/threadex.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#define XPLUGININSTANCE             ::com::sun::star::mozilla::XPluginInstance
#define XPLUGININSTANCEPEER         ::com::sun::star::mozilla::XPluginInstancePeer
#define XPLUGINWINDOWPEER           ::com::sun::star::mozilla::XPluginWindowPeer
#define XINPUTSTREAM                ::com::sun::star::io::XInputStream
#define XINITIALIZATION             ::com::sun::star::lang::XInitialization
#define XWINDOW                     ::com::sun::star::awt::XWindow
#define XSTATUSLISTENER             ::com::sun::star::frame::XStatusListener
#define FEATURESTATEEVENT           ::com::sun::star::frame::FeatureStateEvent
#define ANY                         ::com::sun::star::uno::Any
#define SOLARTHREADEXECUTOR         ::vcl::SolarThreadExecutor

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      implements an special frame - a plugin frame
    @descr      -

    @implements XInitialization
                XPluginInstance
                XStatusListener
    @base       Task
*//*-*************************************************************************************************************/

class PlugInFrame   :   public XINITIALIZATION              ,
                        public XPLUGININSTANCE              ,
                        public XSTATUSLISTENER              ,   // => XEVENTLISTENER
                        public Task                             // Order of baseclasses is neccessary for right initialization!
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      standard constructor to create instance
            @descr      This constructor initialize a new instance of this class,
                        and will be set valid values on his member and baseclasses.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         PlugInFrame( const REFERENCE< XMULTISERVICEFACTORY >& xFactory );

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~PlugInFrame();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XInitialization
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL initialize( const SEQUENCE< ANY >& seqArguments ) throw(  EXCEPTION           ,
                                                                                RUNTIMEEXCEPTION    );

        //---------------------------------------------------------------------------------------------------------
        //  XPluginInstance
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL start         () throw( RUNTIMEEXCEPTION );
        void SAL_CALL implcb_start  () throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL stop          () throw( RUNTIMEEXCEPTION );
        void SAL_CALL implcb_stop   () throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL destroy       () throw( RUNTIMEEXCEPTION );
        void SAL_CALL implcb_destroy() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL createWindow          (   const   ANY&        aPlatformWindowHandle   ,
                                                        sal_Bool    bEmbedded               ) throw( RUNTIMEEXCEPTION );
        void SAL_CALL implcb_createWindow   (   const   ANY&        aPlatformWindowHandle   ,
                                                        sal_Bool    bEmbedded               ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL newStream     (   const   OUSTRING&                   sMIMEDescription,
                                        const   OUSTRING&                   sURL            ,
                                        const   OUSTRING&                   sFilter         ,
                                        const   REFERENCE< XINPUTSTREAM >&  xStream         ,
                                        const   ANY&                        aSessionId      ) throw( RUNTIMEEXCEPTION );
        void SAL_CALL implcb_newStream
                                    (   const   OUSTRING&                   sMIMEDescription,
                                        const   OUSTRING&                   sURL            ,
                                        const   OUSTRING&                   sFilter         ,
                                        const   REFERENCE< XINPUTSTREAM >&  xStream         ,
                                        const   ANY&                        aSessionId      ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL newURL        (   const   OUSTRING&   sMIMEDescription,
                                        const   OUSTRING&   sURL            ,
                                        const   OUSTRING&   sFilter         ,
                                        const   ANY&        aSessionId      ) throw( RUNTIMEEXCEPTION );
        void SAL_CALL implcb_newURL (   const   OUSTRING&   sMIMEDescription,
                                        const   OUSTRING&   sURL            ,
                                        const   OUSTRING&   sFilter         ,
                                        const   ANY&        aSessionId      ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL getHttpServerURL( OUSTRING&   sHost   ,
                                                sal_uInt16& nPort   ,
                                                OUSTRING&   sPrefix ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XDispatchProvider
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XDISPATCH > SAL_CALL queryDispatch(  const   UNOURL&     aURL            ,
                                                                const   OUSTRING&   sTargetFrameName,
                                                                        sal_Int32   nSearchFlags    ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescripts ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XStatusListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL statusChanged( const FEATURESTATEEVENT& aEvent ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XEventListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XDispatchProviderInterception
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

//OBSOLETE      virtual void SAL_CALL registerDispatchProviderInterceptor( const REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

//OBSOLETE      virtual void SAL_CALL releaseDispatchProviderInterceptor( const REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

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

        void impl_tryToLoadDocument();

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

        sal_Bool impldbg_checkParameter_initialize                          (   const   SEQUENCE< ANY >&                            seqArguments            );
        sal_Bool impldbg_checkParameter_createWindow                        (   const   ANY&                                        aPlatformWindowHandle   ,
                                                                                        sal_Bool                                    bEmbedded               );
        sal_Bool impldbg_checkParameter_newStream                           (   const   OUSTRING&                                   sMIMEDescription        ,
                                                                                const   OUSTRING&                                   sURL                    ,
                                                                                const   OUSTRING&                                   sFilter                 ,
                                                                                const   REFERENCE< XINPUTSTREAM >&                  xStream                 );
        sal_Bool impldbg_checkParameter_newURL                              (   const   OUSTRING&                                   sMIMEDescription        ,
                                                                                const   OUSTRING&                                   sURL                    ,
                                                                                const   OUSTRING&                                   sFilter                 );
        sal_Bool impldbg_checkParameter_getHttpServerURL                    (           OUSTRING&                                   sHost                   ,
                                                                                        sal_uInt16&                                 nPort                   ,
                                                                                        OUSTRING&                                   sPrefix                 );
        sal_Bool impldbg_checkParameter_queryDispatch                       (   const   UNOURL&                                     aURL                    ,
                                                                                const   OUSTRING&                                   sTargetFrameName        ,
                                                                                        sal_Int32                                   nSearchFlags            );
        sal_Bool impldbg_checkParameter_queryDispatches                     (   const   SEQUENCE< DISPATCHDESCRIPTOR >&             seqDescripts            );
        sal_Bool impldbg_checkParameter_statusChanged                       (   const   FEATURESTATEEVENT&                          aEvent                  );
        sal_Bool impldbg_checkParameter_disposing                           (   const   EVENTOBJECT&                                aEvent                  );
/*OBSOLETE
         sal_Bool impldbg_checkParameter_registerDispatchProviderInterceptor    (   const   REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >&  xInterceptor            );
        sal_Bool impldbg_checkParameter_releaseDispatchProviderInterceptor  (   const   REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >&  xInterceptor            );
*/

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        REFERENCE< XPLUGININSTANCEPEER >                m_xPlugInInstancePeer   ;   /// Reference to UNO interface of PlugIn dll for communication with browser
        REFERENCE< XPLUGINWINDOWPEER >                  m_xPlugInWindowPeer     ;   /// Reference to set child window at plugin window
        SEQUENCE< PROPERTYVALUE >                       m_seqProperties         ;   /// Sequence of properties as arguments for load document
        UNOURL                                          m_aURL                  ;   /// URL for document to load
        sal_Bool                                        m_bILoad                ;   /// PlugInFrame has a valid loader which load the document and wait for finished/cancelled
        sal_Bool                                        m_bIHaveDocument        ;   /// We have a document loaded successful.
        REFERENCE< XDISPATCHPROVIDER >                  m_xPlugInDispatcher     ;   /// Dispatcher to forward dispatches to browser
        REFERENCE< XMULTISERVICEFACTORY >               m_xRemoteServiceManager ;   /// reference to the remote uno service manager of our plugin dll!

};      //  class PlugInFrame

/*-************************************************************************************************************//**
    @short      used to forward all asynchronous calls which use VCL internal to the main thread
    @descr      We need this asynchronous mechanism to prevent us against dead locks. Sometimes our main thread
                can call us for event handling like FOCUS, ACTIVATE and something else. But at the same time we will call
                the main thread to CREATE A WINDOW, SET IT VISIBLE ... Then we have a problem. We must send us himself a event
                to run our code synchronized with our main thread!

    @implements
    @base       SolarThreadExecutor
*//*-*************************************************************************************************************/

enum eIMPL_PluginCommand
{
    START           ,
    STOP            ,
    CREATEWINDOW    ,
    DESTROY         ,
    NEWSTREAM       ,
    NEWURL
};

class cIMPL_MainThreadExecutor  :   public SOLARTHREADEXECUTOR
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        cIMPL_MainThreadExecutor(           eIMPL_PluginCommand         eCommand                ,
                                            PlugInFrame*                pPluginInstance         );

        cIMPL_MainThreadExecutor(           eIMPL_PluginCommand         eCommand                ,
                                            PlugInFrame*                pPluginInstance         ,
                                    const   ANY&                        aPlatformWindowHandle   ,
                                            sal_Bool                    bEmbedded               );

        cIMPL_MainThreadExecutor(           eIMPL_PluginCommand         eCommand                ,
                                            PlugInFrame*                pPluginInstance         ,
                                    const   OUSTRING&                   sMIMEDescription        ,
                                    const   OUSTRING&                   sURL                    ,
                                    const   OUSTRING&                   sFilter                 ,
                                    const   REFERENCE< XINPUTSTREAM >&  xStream                 ,
                                    const   ANY&                        aSessionId              );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

    virtual long doIt();

    //-------------------------------------------------------------------------------------------------------------
    //  private variables
    //-------------------------------------------------------------------------------------------------------------
    private:

        eIMPL_PluginCommand                         m_eCommand              ;   /// switch to specify forward function
        PlugInFrame*                                m_pPluginInstance       ;   /// instance wich has started this swicth mechanism and wish to called back from us
        ANY                                         m_aPlatformWindowHandle ;   /// parameter for XPluginInstance->createWindow()
        sal_Bool                                    m_bEmbedded             ;   /// parameter for XPluginInstance->createWindow()
        OUSTRING                                    m_sMIMEDescription      ;   /// parameter for XPluginInstance->newStream()/newURL()
        OUSTRING                                    m_sURL                  ;   /// parameter for XPluginInstance->newStream()/newURL()
        OUSTRING                                    m_sFilter               ;   /// parameter for XPluginInstance->newStream()/newURL()
        REFERENCE< XINPUTSTREAM >                   m_xStream               ;   /// parameter for XPluginInstance->newStream()/newURL()
        ANY                                         m_aSessionId            ;   /// parameter for XPluginInstance->newStream()/newURL()
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_PLUGINFRAME_HXX_
