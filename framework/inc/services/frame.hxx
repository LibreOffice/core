/*************************************************************************
 *
 *  $RCSfile: frame.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: as $ $Date: 2000-10-18 12:22:41 $
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

#ifndef __FRAMEWORK_HELPER_OMUTEXMEMBER_HXX_
#include <helper/omutexmember.hxx>
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

#ifndef __FRAMEWORK_DEFINES_HXX_
#include <defines.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XBROWSEHISTORYREGISTRY_HPP_
#include <com/sun/star/frame/XBrowseHistoryRegistry.hpp>
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

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
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

#define ANY                                         ::com::sun::star::uno::Any
#define DISPATCHDESCRIPTOR                          ::com::sun::star::frame::DispatchDescriptor
#define EVENTOBJECT                                 ::com::sun::star::lang::EventObject
#define FOCUSEVENT                                  ::com::sun::star::awt::FocusEvent
#define FRAMEACTION                                 ::com::sun::star::frame::FrameAction
#define INDEXOUTOFBOUNDSEXCEPTION                   ::com::sun::star::lang::IndexOutOfBoundsException
#define OINTERFACECONTAINERHELPER                   ::cppu::OInterfaceContainerHelper
#define OMULTITYPEINTERFACECONTAINERHELPER          ::cppu::OMultiTypeInterfaceContainerHelper
#define OSTRING                                     ::rtl::OString
#define OUSTRING                                    ::rtl::OUString
#define OWEAKOBJECT                                 ::cppu::OWeakObject
#define PROPERTYVALUE                               ::com::sun::star::beans::PropertyValue
#define RUNTIMEEXCEPTION                            ::com::sun::star::uno::RuntimeException
#define UNOURL                                      ::com::sun::star::util::URL
#define WINDOWEVENT                                 ::com::sun::star::awt::WindowEvent
#define WRAPPEDTARGETEXCEPTION                      ::com::sun::star::lang::WrappedTargetException
#define XBROWSEHISTORYREGISTRY                      ::com::sun::star::frame::XBrowseHistoryRegistry
#define XCONTROLLER                                 ::com::sun::star::frame::XController
#define XDISPATCH                                   ::com::sun::star::frame::XDispatch
#define XDISPATCHPROVIDER                           ::com::sun::star::frame::XDispatchProvider
#define XDISPATCHPROVIDERINTERCEPTION               ::com::sun::star::frame::XDispatchProviderInterception
#define XDISPATCHPROVIDERINTERCEPTOR                ::com::sun::star::frame::XDispatchProviderInterceptor
#define XEVENTLISTENER                              ::com::sun::star::lang::XEventListener
#define XFOCUSLISTENER                              ::com::sun::star::awt::XFocusListener
#define XFRAME                                      ::com::sun::star::frame::XFrame
#define XFRAMEACTIONLISTENER                        ::com::sun::star::frame::XFrameActionListener
#define XFRAMELOADER                                ::com::sun::star::frame::XFrameLoader
#define XFRAMES                                     ::com::sun::star::frame::XFrames
#define XFRAMESSUPPLIER                             ::com::sun::star::frame::XFramesSupplier
#define XMULTISERVICEFACTORY                        ::com::sun::star::lang::XMultiServiceFactory
#define XSERVICEINFO                                ::com::sun::star::lang::XServiceInfo
#define XSTATUSINDICATOR                            ::com::sun::star::task::XStatusIndicator
#define XSTATUSINDICATORFACTORY                     ::com::sun::star::task::XStatusIndicatorFactory
#define XTOPWINDOWLISTENER                          ::com::sun::star::awt::XTopWindowListener
#define XTYPEPROVIDER                               ::com::sun::star::lang::XTypeProvider
#define XWINDOW                                     ::com::sun::star::awt::XWindow
#define XWINDOWLISTENER                             ::com::sun::star::awt::XWindowListener

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      implements a normal frame of hierarchy
    @descr      An instance of these class can be a normal node in frame tree only. The highest level to be allowed is 3!
                On 1 stand the desktop himself as the only one, on 2 are all tasks present ... and then comes frames only.
                A frame support influencing of his subtree, find of subframes, activate- and deactivate-mechanism as well as
                set/get of a frame window, component or controller.

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XFramesSupplier
                XFrame
                XComponent
                XStatusIndicatorFactory
                XDispatchProvider
                XDispatchProviderInterception
                XBrowseHistoryRegistry
                XEventListener
                XWindowListener
                XTopWindowListener
                XFocusListener
                [ XDebugging, if TEST_TREE is defined! ]
    @base       OMutexMember
                OWeakObject

    @devstatus  deprecated
*//*-*************************************************************************************************************/

class Frame :   public XTYPEPROVIDER                    ,
                public XSERVICEINFO                     ,
                public XFRAMESSUPPLIER                  ,   // => XFrame      , XComponent
                public XSTATUSINDICATORFACTORY          ,
                public XDISPATCHPROVIDER                ,
                public XDISPATCHPROVIDERINTERCEPTION    ,
                public XBROWSEHISTORYREGISTRY           ,
                public XWINDOWLISTENER                  ,   // => XEventListener
                public XTOPWINDOWLISTENER               ,
                public XFOCUSLISTENER                   ,
                public OMutexMember                     ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
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
            @short      standard constructor to create instance by factory
            @descr      This constructor initialize a new instance of this class by valid factory,
                        and will be set valid values on his member and baseclasses.

            @seealso    -

            @param      "xFactory" is the multi service manager, which create this instance.
                        The value must be different from NULL!
            @return     -

            @onerror    ASSERT in debug version or nothing in relaese version.
        *//*-*****************************************************************************************************/

         Frame( const REFERENCE< XMULTISERVICEFACTORY >& xFactory );

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~Frame();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //   XFramesSupplier
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      return access to append or remove childs on desktop
            @descr      We don't implement these interface directly. We use a helper class to do this.
                        If you wish to add or delete childs to/from the container, call these method to get
                        a reference to the helper.

            @seealso    class OFrames

            @param      -
            @return     A reference to the helper.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAMES > SAL_CALL getFrames() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      get the current active child frame
            @descr      It must be a frameto. Direct childs of a frame are frames only! No task or desktop is accepted.
                        We don't save this information directly in this class. We use ouer container-helper
                        to do that.

            @seealso    class OFrameContainer
            @seealso    method setActiveFrame()

            @param      -
            @return     A reference to ouer current active childframe, if anyone exist.
            @return     A null reference, if nobody is active.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAME > SAL_CALL getActiveFrame() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      set the new active direct child frame
            @descr      It must be a frame to. Direct childs of frame are frames only! No task or desktop is accepted.
                        We don't save this information directly in this class. We use ouer container-helper
                        to do that.

            @seealso    class OFrameContainer
            @seealso    method getActiveFrame()

            @param      "xFrame", reference to new active child. It must be an already existing child!
            @return     -

            @onerror    An assertion is thrown, if given frame is'nt already a child of us.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setActiveFrame( const REFERENCE< XFRAME >& xFrame ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XStatusIndicatorFactory
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      not implemented yet!
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XSTATUSINDICATOR > SAL_CALL createStatusIndicator() throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XDispatchProvider
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      This method searches for a dispatch for the specified DispatchDescriptor.
                        The FrameSearchFlags and the FrameName of the DispatchDescriptor are
                        treated as described for findFrame.

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

            @descr      Returns a sequence of dispatches. For details see the queryDispatch method.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescriptor ) throw( RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL registerDispatchProviderInterceptor( const REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL releaseDispatchProviderInterceptor( const REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XBrowseHistoryRegistry
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

        virtual void SAL_CALL updateViewData( const ANY& aValue ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL createNewEntry(   const   OUSTRING&                   sURL        ,
                                                const   SEQUENCE< PROPERTYVALUE >&  seqArguments,
                                                const   OUSTRING&                   sTitle      ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XWindowListener
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

        virtual void SAL_CALL windowResized( const WINDOWEVENT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowMoved( const WINDOWEVENT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowShown( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowHidden( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XTopWindowListener
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

        virtual void SAL_CALL windowOpened( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowClosing( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowClosed( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowMinimized( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowNormalized( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowActivated( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowDeactivated( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XFrame
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

        virtual void SAL_CALL initialize( const REFERENCE< XWINDOW >& xWindow ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      The ContainerWindow property is used as a VclContainer for the Component
                        in this frame. So this object implements a XVclContainer interface too.
                        The instantiation of the ContainerWindow is done by the derived frame class.
                        The frame is the owner of its ContainerWindow.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XWINDOW > SAL_CALL getContainerWindow() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setCreator( const REFERENCE< XFRAMESSUPPLIER >& xCreator ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      The Creator is the parent frame container. If it is NULL, the frame is the uppermost one.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAMESSUPPLIER > SAL_CALL getCreator() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual OUSTRING SAL_CALL getName() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setName( const OUSTRING& sName ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      This method searches for a frame with the specified name.
                        Frames may contain other frames (e.g. a frameset) and may
                        be contained in other frames. This hierarchie ist searched by
                        this method.
                        First some special names are taken into account, i.e. "",
                         "_self", "_top", "_active" etc. The FrameSearchFlags are ignored
                        when comparing these names with aTargetFrameName, further steps are
                        controlled by the FrameSearchFlags. If allowed, the name of the frame
                        itself is compared with the desired one, then ( again if allowed )
                        the method findFrame is called for all children of the frame.
                        At last findFrame may be called for the parent frame ( if allowed ).
                        If no Frame with the given name is found until the top frames container,
                        a new top Frame is created, if this is allowed by a special
                        FrameSearchFlag. The new Frame also gets the desired name.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAME > SAL_CALL findFrame( const   OUSTRING&   sTargetFrameName    ,
                                                                sal_Int32   nSearchFlags        ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      Returns sal_True, if this frame is a "top frame", otherwise sal_False.
                        The "m_bIsFrameTop" member must be set in the ctor of the derived frame class.
                        A top frame is a member of the top frame container or a member of the
                        task frame container. Both containers can create new frames if the findFrame
                        method of their XFrame interface is called with a frame name not yet known.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL isTop() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL activate() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL deactivate() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL isActive() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL setComponent( const   REFERENCE< XWINDOW >&       xComponentWindow    ,
                                                const   REFERENCE< XCONTROLLER >&   xController         ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      Frames are used to display components. The actual displayed component is
                        held by the ComponentWindow property. If the component implements only a
                        XVclComponent interface, the communication between the frame and the
                        component is very restricted. Better integration is achievable through a
                        XController interface.
                        If the component wants other objects to be able to get information about its
                        ResourceDescriptor ( e.g. for an implementation of a browse history ) it has
                        to implement a XModel interface.
                        This frame is the owner of the ComponentWindow.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XWINDOW > SAL_CALL getComponentWindow() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XCONTROLLER > SAL_CALL getController() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL contextChanged() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addFrameActionListener( const REFERENCE< XFRAMEACTIONLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeFrameActionListener( const REFERENCE< XFRAMEACTIONLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XComponent
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      The owner of this object calles the dispose method if the object
                        should be destroyed. All other objects and components, that are registered
                        as an EventListener are forced to release their references to this object.
                        Furthermore this frame is removed from its parent frame container to release
                        this reference. The reference attributes are disposed and released also.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL dispose() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      Adds an EventListener to this object. If the dispose method is called on
                        this object, the disposing method of the listener is called.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addEventListener( const REFERENCE< XEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -

            @descr      Removes an EventListener. If another object that has registered itself as
                        an EventListener is disposed it must deregister by calling this method.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeEventListener( const REFERENCE< XEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XEventListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -

            @descr      This object is forced to release all references to the interfaces given
                        by the parameter Source.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XFocusListener
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL focusGained( const FOCUSEVENT& aEvent ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL focusLost( const FOCUSEVENT& aEvent ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //  public but impl method for direct helper access of class "DispatchProvider".
        //  Don't use this in another context!
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      search for last destination of load component by means of dispatch(...)
            @descr      This is a HACK for XDispatch->dispatch(...) calls. You don't get any information about
                        the destination frame, which has loaded a component in loadComponentFromURL()!
                        But we will set a flag m_bILoadLastComponent in impl_loadComponent() if loading successfull.
                        With these impl method you can find this frame and return his component ...
                        If this method found any frame we must reset ouer special flag!

            @seealso    method impl_loadComponent()
            @seealso    method loadComponentFromURL()

            @param      -
            @return     A reference to frame, which has loaded the last component.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        REFERENCE< XFRAME > impl_searchLastLoadedComponent();

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

        void impl_setContainerWindow( const REFERENCE< XWINDOW >& xWindow );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_setComponentWindow( const REFERENCE< XWINDOW >& xWindow );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_setController( const REFERENCE< XCONTROLLER >& xController );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_sendFrameActionEvent( const FRAMEACTION& aAction );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_sendDisposeEvent();

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool impl_willFrameTop( const REFERENCE< XFRAMESSUPPLIER >& xParent );

        /*-****************************************************************************************************//**
            @short      resize ouer componentwindow
            @descr      We have get a resize event or a new component. Then we must resize ouer component window too.
                        There are two places in this implementation to do this. Thats why we implement this helper function.

            @seealso    method windowResized()
            @seealso    method impl_loadComponent()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_resizeComponentWindow();

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

        sal_Bool impldbg_checkParameter_append                              (   const   REFERENCE< XFRAME >&                        xFrame              );
        sal_Bool impldbg_checkParameter_queryFrames                         (           sal_Int32                                   nSearchFlags        );
        sal_Bool impldbg_checkParameter_remove                              (   const   REFERENCE< XFRAME >&                        xFrame              );
        sal_Bool impldbg_checkParameter_setActiveFrame                      (   const   REFERENCE< XFRAME >&                        xFrame              );
/*OBSOLETE
         sal_Bool impldbg_checkParameter_queryDispatch                      (   const   UNOURL&                                     aURL                ,
                                                                                  const OUSTRING&                                   sTargetFrameName    ,
                                                                                          sal_Int32                                 nSearchFlags        );
        sal_Bool impldbg_checkParameter_queryDispatches                     (   const   SEQUENCE< DISPATCHDESCRIPTOR >&             seqDescriptor       );
        sal_Bool impldbg_checkParameter_registerDispatchProviderInterceptor (   const   REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >&  xInterceptor        );
        sal_Bool impldbg_checkParameter_releaseDispatchProviderInterceptor  (   const   REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >&  xInterceptor        );
*/
        sal_Bool impldbg_checkParameter_updateViewData                      (   const   ANY&                                        aValue              );
        sal_Bool impldbg_checkParameter_createNewEntry                      (   const   OUSTRING&                                   sURL                ,
                                                                                  const SEQUENCE< PROPERTYVALUE >&                  seqArguments        ,
                                                                                const   OUSTRING&                                   sTitle              );
        sal_Bool impldbg_checkParameter_windowResized                       (   const   WINDOWEVENT&                                aEvent              );
        sal_Bool impldbg_checkParameter_windowActivated                     (   const   EVENTOBJECT&                                aEvent              );
        sal_Bool impldbg_checkParameter_windowDeactivated                   (   const   EVENTOBJECT&                                aEvent              );
        sal_Bool impldbg_checkParameter_initialize                          (   const   REFERENCE< XWINDOW >&                       xWindow             );
        sal_Bool impldbg_checkParameter_setCreator                          (   const   REFERENCE< XFRAMESSUPPLIER >&               xCreator            );
        sal_Bool impldbg_checkParameter_setName                             (   const   OUSTRING&                                   sName               );
        sal_Bool impldbg_checkParameter_findFrame                           (   const   OUSTRING&                                   sTargetFrameName    ,
                                                                                         sal_Int32                                  nSearchFlags        );
        sal_Bool impldbg_checkParameter_setComponent                        (   const   REFERENCE< XWINDOW >&                       xComponentWindow    ,
                                                                                  const REFERENCE< XCONTROLLER >&                   xController         );
        sal_Bool impldbg_checkParameter_addFrameActionListener              (   const   REFERENCE< XFRAMEACTIONLISTENER >&          xListener           );
        sal_Bool impldbg_checkParameter_removeFrameActionListener           (   const   REFERENCE< XFRAMEACTIONLISTENER >&          xListener           );
        sal_Bool impldbg_checkParameter_addEventListener                    (   const   REFERENCE< XEVENTLISTENER >&                xListener           );
        sal_Bool impldbg_checkParameter_removeEventListener                 (   const   REFERENCE< XEVENTLISTENER >&                xListener           );
        sal_Bool impldbg_checkParameter_disposing                           (   const   EVENTOBJECT&                                aEvent              );
        sal_Bool impldbg_checkParameter_focusGained                         (   const   FOCUSEVENT&                                 aEvent              );
        sal_Bool impldbg_checkParameter_focusLost                           (   const   FOCUSEVENT&                                 aEvent              );

    #endif  // #ifdef ENABLE_ASSERTIONS

        /*-****************************************************************************************************//**
            @short      debug-method to get information about current container content
            @descr      You can use this method to print informations about the frames in container.
                        If you call these from top to bottom in the whole tree, you will get a snapshot of the tree.

            @seealso    method Desktop::impldbg_printTree()

            @param      "nLevel" is the level in tree. The top has level 0, childs of desktop 1 and so on...
            @return     The collected informations about ouer own container AND subtree as "string-stream".

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_SERVICEDEBUG  // Only active in debug version.

        OUSTRING impldbg_getTreeNames( sal_Int16 nLevel );

    #endif  // #ifdef ENABLE_SERVICEDEBUG

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    protected:

        // But some variables are used in derived classes!
        // Make it protected for directly access.
        OUSTRING                                        m_sName                             ;   /// name of this frame
        REFERENCE< XFRAMESSUPPLIER >                    m_xParent                           ;   /// parent of this frame
//OBSOLETE      REFERENCE< XDISPATCHPROVIDERINTERCEPTOR >       m_xInterceptor                      ;   /// current interceptor for dispatches
        REFERENCE< XDISPATCHPROVIDER >                  m_xDispatchHelper                   ;   /// helper for XDispatchProvider and XDispatch interfaces
        REFERENCE< XMULTISERVICEFACTORY >               m_xFactory                          ;   /// reference to factory, which has create this instance
        REFERENCE< XWINDOW >                            m_xContainerWindow                  ;   /// containerwindow of this frame for embedded components
        sal_Bool                                        m_bRecursiveSearchProtection        ;   /// protect against recursion while searching in parent frames
        FrameContainer                                  m_aChildFrameContainer              ;   /// array of child frames

    private:

        REFERENCE< XSTATUSINDICATORFACTORY >            m_xIndicatorFactoryHelper           ;   /// reference to factory helper to create status indicator objects
        REFERENCE< XWINDOW >                            m_xComponentWindow                  ;   /// window of the actual component
        REFERENCE< XCONTROLLER >                        m_xController                       ;   /// controller of the actual frame
        eACTIVESTATE                                    m_eActiveState                      ;   /// state, if i'am a member of active path in tree or i have the focus or ...
        sal_Bool                                        m_bIsFrameTop                       ;   /// frame has no parent or the parent is a taskor the desktop
        sal_Bool                                        m_bConnected                        ;   /// due to FrameActionEvent
        sal_Bool                                        m_bAlreadyDisposed                  ;   /// protect egainst recursive dispose calls
        sal_Bool                                        m_bILoadLastComponent               ;   /// help flag to find last destination of dispatch(...) in tree! see impl_searchLastLoadedComponent() for further informations
        OMULTITYPEINTERFACECONTAINERHELPER              m_aListenerContainer                ;   /// container for ALL Listener
        REFERENCE< XFRAMES >                            m_xFramesHelper                     ;   /// helper for XFrames, XIndexAccess and XElementAccess interfaces

};      // class Frame

}       // namespace framework

#endif  // #ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
