/*************************************************************************
 *
 *  $RCSfile: frame.hxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:51 $
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

#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#define __FRAMEWORK_SERVICES_FRAME_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
#include <classes/framecontainer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONBASE_HXX_
#include <threadhelp/transactionbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRECORDERSUPPLIER_HPP_
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMES_HPP_
#include <com/sun/star/frame/XFrames.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTOPWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XTopWindowListener.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_FOCUSEVENT_HPP_
#include <com/sun/star/awt/FocusEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETELISTENER_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XACTIONLOCKABLE_HPP_
#include <com/sun/star/document/XActionLockable.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _VCL_EVNTPOST_HXX
#include <vcl/evntpost.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_CMDOPTIONS_HXX
#include <svtools/cmdoptions.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

// This enum can be used to set differnt active states of frames
enum EActiveState
{
    E_INACTIVE      ,   // I'am not a member of active path in tree and i don't have the focus.
    E_ACTIVE        ,   // I'am in the middle of an active path in tree and i don't have the focus.
    E_FOCUS             // I have the focus now. I must a member of an active path!
};

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      implements a normal frame of hierarchy
    @descr      An instance of these class can be a normal node in frame tree. A frame support influencing of his
                subtree, find of subframes, activate- and deactivate-mechanism as well as
                set/get of a frame window, component or controller.

    @attention  This implementation supports three states: a)uninitialized, b)working, c)disposed
                If you call wrong methods in modes a) or c) ... you will get some exceptions.
                How you should work with this service:
                    i)      create it by using "xServiceManager->createInstance(...)"
                    ii)     call XInitialization::initialize() with a valid window reference or use createInstanceWithArguments() at i)
                    iii)    works with object
                    iv)     dispose object by calling XComponent::dispose()
                After iv) all further requests are rejected by exceptions! (DisposedException)

    @base       ThreadHelpBase
                    help to guarantee correct initialized lock member at startup
    @base       TransactionBase
                    help to implement unbreakable interface calls
    @base       OBroadcastHelper
                OPropertySetHelper
                    implements the property set
    @base       OWeakObject
                    provides the refcount and XInterface, XWeak

    @devstatus  ready to use
    @threadsafe yes
    @modified   08.05.2002 09:38, as96863
*//*-*************************************************************************************************************/
class Frame :   // interfaces
                public  css::lang::XTypeProvider                    ,
                public  css::lang::XServiceInfo                     ,
                public  css::frame::XFramesSupplier                 ,   // => XFrame => XComponent
                public  css::frame::XDispatchProvider               ,
                public  css::frame::XDispatchProviderInterception   ,
                public  css::task::XStatusIndicatorFactory          ,
                public  css::awt::XWindowListener                   ,   // => XEventListener
                public  css::awt::XTopWindowListener                ,
                public  css::awt::XFocusListener                    ,
                public  css::document::XActionLockable              ,
                public  css::util::XCloseable                       ,   // => XCloseBroadcaster
                public  css::frame::XComponentLoader                ,
                // base classes
                // Order is neccessary for right initialization of this class!
                public  ThreadHelpBase                              ,   // helper for own threadsafe code
                public  TransactionBase                             ,   // helper for rejecting calls for wrong object states
                public  ::cppu::OBroadcastHelper                    ,   // helper for propertyset => XPropertySet, XFastPropertySet, XMultiPropertySet
                public  ::cppu::OPropertySetHelper                  ,
                public  ::cppu::OWeakObject                             // helper for refcount mechanism
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
                  Frame( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~Frame(                                                                         );

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XComponentLoader
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Reference< css::lang::XComponent >        SAL_CALL loadComponentFromURL               ( const ::rtl::OUString&                                                        sURL                ,
                                                                                                                  const ::rtl::OUString&                                                        sTargetFrameName    ,
                                                                                                                        sal_Int32                                                               nSearchFlags        ,
                                                                                                                  const css::uno::Sequence< css::beans::PropertyValue >&                        lArguments          ) throw( css::io::IOException                ,
                                                                                                                                                                                                                             css::lang::IllegalArgumentException ,
                                                                                                                                                                                                                             css::uno::RuntimeException          );


        //---------------------------------------------------------------------------------------------------------
        //  XFramesSupplier
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Reference< css::frame::XFrames >          SAL_CALL getFrames                          (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual css::uno::Reference< css::frame::XFrame >           SAL_CALL getActiveFrame                     (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL setActiveFrame                     (   const   css::uno::Reference< css::frame::XFrame >&                          xFrame              ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XFrame
        //---------------------------------------------------------------------------------------------------------
        virtual void                                                SAL_CALL initialize                         (   const   css::uno::Reference< css::awt::XWindow >&                           xWindow             ) throw( css::uno::RuntimeException );
        virtual css::uno::Reference< css::awt::XWindow >            SAL_CALL getContainerWindow                 (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL setCreator                         (   const   css::uno::Reference< css::frame::XFramesSupplier >&                 xCreator            ) throw( css::uno::RuntimeException );
        virtual css::uno::Reference< css::frame::XFramesSupplier >  SAL_CALL getCreator                         (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual ::rtl::OUString                                     SAL_CALL getName                            (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL setName                            (   const   ::rtl::OUString&                                                    sName               ) throw( css::uno::RuntimeException );
        virtual css::uno::Reference< css::frame::XFrame >           SAL_CALL findFrame                          (   const   ::rtl::OUString&                                                    sTargetFrameName    ,
                                                                                                                            sal_Int32                                                           nSearchFlags        ) throw( css::uno::RuntimeException );
        virtual sal_Bool                                            SAL_CALL isTop                              (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL activate                           (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL deactivate                         (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual sal_Bool                                            SAL_CALL isActive                           (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL contextChanged                     (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual sal_Bool                                            SAL_CALL setComponent                       (   const   css::uno::Reference< css::awt::XWindow >&                           xComponentWindow    ,
                                                                                                                    const   css::uno::Reference< css::frame::XController >&                     xController         ) throw( css::uno::RuntimeException );
        virtual css::uno::Reference< css::awt::XWindow >            SAL_CALL getComponentWindow                 (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual css::uno::Reference< css::frame::XController >      SAL_CALL getController                      (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL addFrameActionListener             (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL removeFrameActionListener          (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XComponent
        //---------------------------------------------------------------------------------------------------------
        virtual void                                                SAL_CALL dispose                            (                                                                                                   ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL addEventListener                   (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL removeEventListener                (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XStatusIndicatorFactory
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Reference< css::task::XStatusIndicator >  SAL_CALL createStatusIndicator              (                                                                                                   ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XDispatchProvider
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Reference< css::frame::XDispatch >        SAL_CALL queryDispatch                      (   const   css::util::URL&                                                     aURL                ,
                                                                                                                    const   ::rtl::OUString&                                                    sTargetFrameName    ,
                                                                                                                            sal_Int32                                                           nSearchFlags        ) throw( css::uno::RuntimeException );
        virtual css::uno::Sequence<
                    css::uno::Reference< css::frame::XDispatch > >  SAL_CALL queryDispatches                    (   const   css::uno::Sequence< css::frame::DispatchDescriptor >&               lDescriptor         ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XDispatchProviderInterception
        //---------------------------------------------------------------------------------------------------------
        virtual void                                                SAL_CALL registerDispatchProviderInterceptor(   const   css::uno::Reference< css::frame::XDispatchProviderInterceptor >&    xInterceptor        ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL releaseDispatchProviderInterceptor (   const   css::uno::Reference< css::frame::XDispatchProviderInterceptor >&    xInterceptor        ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XWindowListener
        //  Attention: windowResized() and windowShown() are implement only! All other are empty!
        //---------------------------------------------------------------------------------------------------------
        virtual void                                                SAL_CALL windowResized                      (   const   css::awt::WindowEvent&                                              aEvent              ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL windowMoved                        (   const   css::awt::WindowEvent&                                              aEvent              ) throw( css::uno::RuntimeException ) {};
        virtual void                                                SAL_CALL windowShown                        (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL windowHidden                       (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XFocusListener
        //  Attention: focusLost() not implemented yet!
        //---------------------------------------------------------------------------------------------------------
        virtual void                                                SAL_CALL focusGained                        (   const   css::awt::FocusEvent&                                               aEvent              ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL focusLost                          (   const   css::awt::FocusEvent&                                               aEvent              ) throw( css::uno::RuntimeException ) {};

        //---------------------------------------------------------------------------------------------------------
        //  XTopWindowListener
        //  Attention: windowActivated(), windowDeactivated() and windowClosing() are implement only! All other are empty!
        //---------------------------------------------------------------------------------------------------------
        virtual void                                                SAL_CALL windowActivated                    (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL windowDeactivated                  (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL windowOpened                       (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException ) {};
        virtual void                                                SAL_CALL windowClosing                      (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );
        virtual void                                                SAL_CALL windowClosed                       (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException ) {};
        virtual void                                                SAL_CALL windowMinimized                    (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException ) {};
        virtual void                                                SAL_CALL windowNormalized                   (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException ) {};

        //---------------------------------------------------------------------------------------------------------
        //  XEventListener
        //---------------------------------------------------------------------------------------------------------
        virtual void                                                SAL_CALL disposing                          (   const   css::lang::EventObject&                                             aEvent              ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XActionLockable
        //---------------------------------------------------------------------------------------------------------
        virtual sal_Bool    SAL_CALL isActionLocked  (                 ) throw( css::uno::RuntimeException );
        virtual void        SAL_CALL addActionLock   (                 ) throw( css::uno::RuntimeException );
        virtual void        SAL_CALL removeActionLock(                 ) throw( css::uno::RuntimeException );
        virtual void        SAL_CALL setActionLocks  ( sal_Int16 nLock ) throw( css::uno::RuntimeException );
        virtual sal_Int16   SAL_CALL resetActionLocks(                 ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XCloseable
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL close( sal_Bool bDeliverOwnerShip ) throw( css::util::CloseVetoException,
                                                                         css::uno::RuntimeException   );

        //---------------------------------------------------------------------------------------------------------
        //  XCloseable
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL addCloseListener   ( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException);
        virtual void SAL_CALL removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException);

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        //---------------------------------------------------------------------------------------------------------
        //  OPropertySetHelper
        //---------------------------------------------------------------------------------------------------------
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue           (           css::uno::Any&                                                      aConvertedValue     ,
                                                                                                                                 css::uno::Any&                                                     aOldValue           ,
                                                                                                                                 sal_Int32                                                          nHandle             ,
                                                                                                                    const   css::uno::Any&                                                      aValue              ) throw( css::lang::IllegalArgumentException );
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast   (           sal_Int32                                                           nHandle             ,
                                                                                                                      const css::uno::Any&                                                      aValue              ) throw( css::uno::Exception );
        virtual void                                                SAL_CALL getFastPropertyValue               (           css::uno::Any&                                                      aValue              ,
                                                                                                                              sal_Int32                                                         nHandle             ) const;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper                      (                                                                                                   );
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo                 (                                                                                                   ) throw (::com::sun::star::uno::RuntimeException);

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      helper methods
            @descr      Follow methods are needed at different points of our code (more then ones!).

            @attention  Threadsafe methods are signed by "implts_..."!
        *//*-*****************************************************************************************************/

        // threadsafe
        void                                                    implts_sendFrameActionEvent     ( const css::frame::FrameAction&                        aAction          );
        void                                                    implts_resizeComponentWindow    (                                                                        );
        void                                                    implts_setTitleOnWindow         ( const ::rtl::OUString&                                sTitle           );
        const ::rtl::OUString                                   implts_getTitleFromWindow       (                                                                        ) const;
        void                                                    implts_setIconOnWindow          (                                                                        );
        void                                                    implts_startWindowListening     (                                                                        );
        void                                                    implts_stopWindowListening      (                                                                        );
        void                                                    implts_saveWindowAttributes     (                                                                        );
        void                                                    implts_checkSuicide             (                                                                        );
        DECL_LINK( implts_windowClosing, void* );

        // non threadsafe
        void                                                    impl_checkMenuCloser            (                                                                        );
        void                                                    impl_setCloser                  ( const css::uno::Reference< css::frame::XFrame >&      xFrame           ,
                                                                                                        sal_Bool                                        bState           );
        void                                                    impl_disposeContainerWindow     (       css::uno::Reference< css::awt::XWindow >&       xWindow          );
        static const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor(                                                                        );

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @attention  This methods are static and can't use our member directly! It's better for threadsafe code...
                        because we call it with references or pointer to check variables ... and must make it safe
                        by himself!

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_True  ,on invalid parameter
            @return     sal_False ,otherwise

            @onerror    We return sal_True
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        static sal_Bool implcp_ctor                                (   const   css::uno::Reference< css::lang::XMultiServiceFactory >&             xFactory            );
        static sal_Bool implcp_setActiveFrame                      (   const   css::uno::Reference< css::frame::XFrame >&                          xFrame              );
        static sal_Bool implcp_initialize                          (   const   css::uno::Reference< css::awt::XWindow >&                           xWindow             );
        static sal_Bool implcp_setCreator                          (   const   css::uno::Reference< css::frame::XFramesSupplier >&                 xCreator            );
        static sal_Bool implcp_setName                             (   const   ::rtl::OUString&                                                    sName               );
        static sal_Bool implcp_findFrame                           (   const   ::rtl::OUString&                                                    sTargetFrameName    ,
                                                                               sal_Int32                                                           nSearchFlags        );
        static sal_Bool implcp_setComponent                        (   const   css::uno::Reference< css::awt::XWindow >&                           xComponentWindow    ,
                                                                       const   css::uno::Reference< css::frame::XController >&                     xController         );
        static sal_Bool implcp_addFrameActionListener              (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           );
        static sal_Bool implcp_removeFrameActionListener           (   const   css::uno::Reference< css::frame::XFrameActionListener >&            xListener           );
        static sal_Bool implcp_addEventListener                    (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           );
        static sal_Bool implcp_removeEventListener                 (   const   css::uno::Reference< css::lang::XEventListener >&                   xListener           );
        static sal_Bool implcp_windowResized                       (   const   css::awt::WindowEvent&                                              aEvent              );
        static sal_Bool implcp_focusGained                         (   const   css::awt::FocusEvent&                                               aEvent              );
        static sal_Bool implcp_windowActivated                     (   const   css::lang::EventObject&                                             aEvent              );
        static sal_Bool implcp_windowDeactivated                   (   const   css::lang::EventObject&                                             aEvent              );
        static sal_Bool implcp_disposing                           (   const   css::lang::EventObject&                                             aEvent              );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //*************************************************************************************************************
    //  variables
    //  -threadsafe by own read/write lock "m_aLock"
    //*************************************************************************************************************
    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >                  m_xFactory                          ;   /// reference to factory, which has create this instance
        css::uno::Reference< css::task::XStatusIndicatorFactory >               m_xIndicatorFactoryHelper           ;   /// reference to factory helper to create status indicator objects
        css::uno::Reference< css::frame::XDispatchProvider >                    m_xDispatchHelper                   ;   /// helper for XDispatch/Provider and interception interfaces
        css::uno::Reference< css::frame::XFrames >                              m_xFramesHelper                     ;   /// helper for XFrames, XIndexAccess and XElementAccess interfaces
        ::cppu::OMultiTypeInterfaceContainerHelper                              m_aListenerContainer                ;   /// container for ALL Listener
        css::uno::Reference< css::frame::XFramesSupplier >                      m_xParent                           ;   /// parent of this frame
        css::uno::Reference< css::awt::XWindow >                                m_xContainerWindow                  ;   /// containerwindow of this frame for embedded components
        css::uno::Reference< css::awt::XWindow >                                m_xComponentWindow                  ;   /// window of the actual component
        css::uno::Reference< css::frame::XController >                          m_xController                       ;   /// controller of the actual frame
        css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >      m_xDropTargetListener               ;   /// listen to drag & drop
        EActiveState                                                            m_eActiveState                      ;   /// state, if i'am a member of active path in tree or i have the focus or ...
        ::rtl::OUString                                                         m_sName                             ;   /// name of this frame
        sal_Bool                                                                m_bIsFrameTop                       ;   /// frame has no parent or the parent is a taskor the desktop
        sal_Bool                                                                m_bConnected                        ;   /// due to FrameActionEvent
        sal_Int16                                                               m_nExternalLockCount                ;
        css::uno::Reference< css::frame::XDispatchRecorderSupplier >            m_xDispatchRecorderSupplier         ;   /// is used for dispatch recording and will be set/get from outside. Frame provide it only!
        SvtCommandOptions                                                       m_aCommandOptions                   ;   /// ref counted class to support disabling commands defined by configuration file
        sal_Bool                                                                m_bSelfClose                        ;   /// in case of CloseVetoException on method close() wqs thrown by ourself - we must close ourself later if no internal processes are running
        ::vcl::EventPoster                                                      m_aPoster                           ;
        sal_Bool                                                                m_bIsHidden                         ;   /// indicates, if this frame is used in hidden mode or not
        static css::uno::WeakReference< css::frame::XFrame >                    m_xCloserFrame                      ;   /// holds the only frame, which must show the special closer menu item (can be NULL!)
        sal_Bool                                                                m_bIsBackingMode                    ;   /// indicates, that the current component must be the backing component (will be set from outside and reseted inside setComponent() hardly!)

    protected:

        sal_Bool                                                                m_bIsPlugIn                         ;   /// In objects of these class this member is set to FALSE. But in derived class PlugInFrame it's overwritten with TRUE!
        FrameContainer                                                          m_aChildFrameContainer              ;   /// array of child frames

        inline css::uno::Reference< css::lang::XMultiServiceFactory > impl_getFactory()
        {
            ReadGuard aReadLock( m_aLock );
            return m_xFactory;
        }

        inline ::rtl::OUString impl_getName()
        {
            ReadGuard aReadLock( m_aLock );
            return m_sName;
        }

        inline css::uno::Reference< css::awt::XWindow > impl_getContainerWindow()
        {
            ReadGuard aReadLock( m_aLock );
            return m_xContainerWindow;
        }

        inline css::uno::Reference< css::frame::XDispatchProvider > impl_getDispatchHelper()
        {
            ReadGuard aReadLock( m_aLock );
            return m_xDispatchHelper;
        }

        inline css::uno::Reference< css::frame::XFramesSupplier > impl_getParent()
        {
            ReadGuard aReadLock( m_aLock );
            return m_xParent;
        }

};      // class Frame

}       // namespace framework

#endif  // #ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
