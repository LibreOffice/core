/*************************************************************************
 *
 *  $RCSfile: frame.hxx,v $
 *
 *  $Revision: 1.6 $
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

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
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

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

// This enum can be used to set differnt active states of frames, tasks.
enum eACTIVESTATE
{
    INACTIVE        ,   // I'am not a member of active path in tree and i don't have the focus.
    ACTIVE          ,   // I'am in the middle of an active path in tree and i don't have the focus.
    FOCUS               // I have the focus now. I must a member of an active path!
};

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
    @base       OMutexMember
                OWeakObject

    @devstatus  deprecated
*//*-*************************************************************************************************************/

class Frame :   public css::lang::XTypeProvider                 ,
                public css::lang::XServiceInfo                  ,
                public css::frame::XFramesSupplier              ,   // => XFrame      , XComponent
                public css::task::XStatusIndicatorFactory       ,
                public css::frame::XDispatchProvider            ,
                public css::frame::XDispatchProviderInterception,
                public css::frame::XBrowseHistoryRegistry       ,
                public css::awt::XWindowListener                ,   // => XEventListener
                public css::awt::XTopWindowListener             ,
                public css::awt::XFocusListener                 ,
                public OMutexMember                             ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
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
            @short      standard constructor to create instance by factory
            @descr      This constructor initialize a new instance of this class by valid factory,
                        and will be set valid values on his member and baseclasses.

            @seealso    -

            @param      "xFactory" is the multi service manager, which create this instance.
                        The value must be different from NULL!
            @return     -

            @onerror    ASSERT in debug version or nothing in relaese version.
        *//*-*****************************************************************************************************/

         Frame( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );

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

        virtual css::uno::Reference< css::frame::XFrames > SAL_CALL getFrames() throw( css::uno::RuntimeException );

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

        virtual css::uno::Reference< css::frame::XFrame > SAL_CALL getActiveFrame() throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException );

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

        virtual css::uno::Reference< css::task::XStatusIndicator > SAL_CALL createStatusIndicator() throw( css::uno::RuntimeException );

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

        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(    const   css::util::URL&     aURL            ,
                                                                                        const   ::rtl::OUString&    sTargetFrameName,
                                                                                                sal_Int32           nSearchFlags    ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      Returns a sequence of dispatches. For details see the queryDispatch method.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& seqDescriptor ) throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL updateViewData( const css::uno::Any& aValue ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL createNewEntry(   const   ::rtl::OUString&                                    sURL        ,
                                                const   css::uno::Sequence< css::beans::PropertyValue >&    seqArguments,
                                                const   ::rtl::OUString&                                    sTitle      ) throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL windowOpened( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowClosing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowClosed( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowMinimized( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowNormalized( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowActivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL windowDeactivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL initialize( const css::uno::Reference< css::awt::XWindow >& xWindow ) throw( css::uno::RuntimeException );

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

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getContainerWindow() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      The Creator is the parent frame container. If it is NULL, the frame is the uppermost one.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL getCreator() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ::rtl::OUString SAL_CALL getName() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException );

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

        virtual css::uno::Reference< css::frame::XFrame > SAL_CALL findFrame(   const   ::rtl::OUString&    sTargetFrameName    ,
                                                                                        sal_Int32           nSearchFlags        ) throw( css::uno::RuntimeException );

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

        virtual sal_Bool SAL_CALL isTop() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL activate() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL deactivate() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL isActive() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL setComponent( const   css::uno::Reference< css::awt::XWindow >&       xComponentWindow    ,
                                                const   css::uno::Reference< css::frame::XController >& xController         ) throw( css::uno::RuntimeException );

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

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getComponentWindow() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::frame::XController > SAL_CALL getController() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL contextChanged() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -
            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL dispose() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      Adds an EventListener to this object. If the dispose method is called on
                        this object, the disposing method of the listener is called.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -

            @descr      Removes an EventListener. If another object that has registered itself as
                        an EventListener is disposed it must deregister by calling this method.

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

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

        virtual void SAL_CALL focusGained( const css::awt::FocusEvent& aEvent ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL focusLost( const css::awt::FocusEvent& aEvent ) throw( css::uno::RuntimeException );

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

        void impl_setContainerWindow( const css::uno::Reference< css::awt::XWindow >& xWindow );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_setComponentWindow( const css::uno::Reference< css::awt::XWindow >& xWindow );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_setController( const css::uno::Reference< css::frame::XController >& xController );

        /*-****************************************************************************************************//**
            @short      -

            @descr      -

            @seealso    -

            @param      -

            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_sendFrameActionEvent( const css::frame::FrameAction& aAction );

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

        sal_Bool impl_willFrameTop( const css::uno::Reference< css::frame::XFramesSupplier >& xParent );

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

        static sal_Bool impldbg_checkParameter_append                               (   const   css::uno::Reference< css::frame::XFrame >&                  xFrame              );
        static sal_Bool impldbg_checkParameter_queryFrames                          (           sal_Int32                                                   nSearchFlags        );
        static sal_Bool impldbg_checkParameter_remove                               (   const   css::uno::Reference< css::frame::XFrame >&                  xFrame              );
        static sal_Bool impldbg_checkParameter_setActiveFrame                       (   const   css::uno::Reference< css::frame::XFrame >&                  xFrame              );
        static sal_Bool impldbg_checkParameter_updateViewData                       (   const   css::uno::Any&                                              aValue              );
        static sal_Bool impldbg_checkParameter_createNewEntry                       (   const   ::rtl::OUString&                                            sURL                ,
                                                                                          const css::uno::Sequence< css::beans::PropertyValue >&            seqArguments        ,
                                                                                        const   ::rtl::OUString&                                            sTitle              );
        static sal_Bool impldbg_checkParameter_windowResized                        (   const   css::awt::WindowEvent&                                      aEvent              );
        static sal_Bool impldbg_checkParameter_windowActivated                      (   const   css::lang::EventObject&                                     aEvent              );
        static sal_Bool impldbg_checkParameter_windowDeactivated                    (   const   css::lang::EventObject&                                     aEvent              );
        static sal_Bool impldbg_checkParameter_initialize                           (   const   css::uno::Reference< css::awt::XWindow >&                   xWindow             );
        static sal_Bool impldbg_checkParameter_setCreator                           (   const   css::uno::Reference< css::frame::XFramesSupplier >&         xCreator            );
        static sal_Bool impldbg_checkParameter_setName                              (   const   ::rtl::OUString&                                            sName               );
        static sal_Bool impldbg_checkParameter_findFrame                            (   const   ::rtl::OUString&                                            sTargetFrameName    ,
                                                                                                 sal_Int32                                                  nSearchFlags        );
        static sal_Bool impldbg_checkParameter_setComponent                         (   const   css::uno::Reference< css::awt::XWindow >&                   xComponentWindow    ,
                                                                                          const css::uno::Reference< css::frame::XController >&             xController         );
        static sal_Bool impldbg_checkParameter_addFrameActionListener               (   const   css::uno::Reference< css::frame::XFrameActionListener >&    xListener           );
        static sal_Bool impldbg_checkParameter_removeFrameActionListener            (   const   css::uno::Reference< css::frame::XFrameActionListener >&    xListener           );
        static sal_Bool impldbg_checkParameter_addEventListener                     (   const   css::uno::Reference< css::lang::XEventListener >&           xListener           );
        static sal_Bool impldbg_checkParameter_removeEventListener                  (   const   css::uno::Reference< css::lang::XEventListener >&           xListener           );
        static sal_Bool impldbg_checkParameter_disposing                            (   const   css::lang::EventObject&                                     aEvent              );
        static sal_Bool impldbg_checkParameter_focusGained                          (   const   css::awt::FocusEvent&                                       aEvent              );
        static sal_Bool impldbg_checkParameter_focusLost                            (   const   css::awt::FocusEvent&                                       aEvent              );

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

        ::rtl::OUString impldbg_getTreeNames( sal_Int16 nLevel );

    #endif  // #ifdef ENABLE_SERVICEDEBUG

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    protected:

        // But some variables are used in derived classes!
        // Make it protected for directly access.
        ::rtl::OUString                                                     m_sName                             ;   /// name of this frame
        css::uno::Reference< css::frame::XFramesSupplier >                  m_xParent                           ;   /// parent of this frame
        css::uno::Reference< css::frame::XDispatchProvider >                m_xDispatchHelper                   ;   /// helper for XDispatchProvider, XDispatch, XDispatchProviderInterception interfaces
        css::uno::Reference< css::lang::XMultiServiceFactory >              m_xFactory                          ;   /// reference to factory, which has create this instance
        css::uno::Reference< css::awt::XWindow >                            m_xContainerWindow                  ;   /// containerwindow of this frame for embedded components
        sal_Bool                                                            m_bRecursiveSearchProtection        ;   /// protect against recursion while searching in parent frames
        FrameContainer                                                      m_aChildFrameContainer              ;   /// array of child frames

    private:

        css::uno::Reference< css::task::XStatusIndicatorFactory >           m_xIndicatorFactoryHelper           ;   /// reference to factory helper to create status indicator objects
        css::uno::Reference< css::awt::XWindow >                            m_xComponentWindow                  ;   /// window of the actual component
        css::uno::Reference< css::frame::XController >                      m_xController                       ;   /// controller of the actual frame
        eACTIVESTATE                                                        m_eActiveState                      ;   /// state, if i'am a member of active path in tree or i have the focus or ...
        sal_Bool                                                            m_bIsFrameTop                       ;   /// frame has no parent or the parent is a taskor the desktop
        sal_Bool                                                            m_bConnected                        ;   /// due to FrameActionEvent
        sal_Bool                                                            m_bAlreadyDisposed                  ;   /// protect egainst recursive dispose calls
        ::cppu::OMultiTypeInterfaceContainerHelper                          m_aListenerContainer                ;   /// container for ALL Listener
        css::uno::Reference< css::frame::XFrames >                          m_xFramesHelper                     ;   /// helper for XFrames, XIndexAccess and XElementAccess interfaces

};      // class Frame

}       // namespace framework

#endif  // #ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
