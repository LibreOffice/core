/*************************************************************************
 *
 *  $RCSfile: desktop.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: as $ $Date: 2001-03-05 08:09:04 $
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

#ifndef __FRAMEWORK_SERVICES_DESKTOP_HXX_
#define __FRAMEWORK_SERVICES_DESKTOP_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
#include <classes/framecontainer.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
#include <classes/taskcreator.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OMUTEXMEMBER_HXX_
#include <helper/omutexmember.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OFRAMES_HXX_
#include <helper/oframes.hxx>
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

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOPTASK_HPP_
#include <com/sun/star/frame/XDesktopTask.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_WINDOWARRANGE_HPP_
#include <com/sun/star/frame/WindowArrange.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_TERMINATIONVETOEXCEPTION_HPP_
#include <com/sun/star/frame/TerminationVetoException.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XWINDOWARRANGER_HPP_
#include <com/sun/star/frame/XWindowArranger.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTASK_HPP_
#include <com/sun/star/frame/XTask.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMES_HPP_
#include <com/sun/star/frame/XFrames.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

#define ANY                                         ::com::sun::star::uno::Any
#define EXCEPTION                                   ::com::sun::star::uno::Exception
#define DISPATCHDESCRIPTOR                          ::com::sun::star::frame::DispatchDescriptor
#define EVENTOBJECT                                 ::com::sun::star::lang::EventObject
#define FEATURESTATEEVENT                           ::com::sun::star::frame::FeatureStateEvent
#define FEATURESTATEEVENT                           ::com::sun::star::frame::FeatureStateEvent
#define ILLEGALARGUMENTEXCEPTION                    ::com::sun::star::lang::IllegalArgumentException
#define INDEXOUTOFBOUNDSEXCEPTION                   ::com::sun::star::lang::IndexOutOfBoundsException
#define IOEXCEPTION                                 ::com::sun::star::io::IOException
#define IPROPERTYARRAYHELPER                        ::cppu::IPropertyArrayHelper
#define LOCALE                                      ::com::sun::star::lang::Locale
#define OBROADCASTHELPER                            ::cppu::OBroadcastHelper
#define OINTERFACECONTAINERHELPER                   ::cppu::OInterfaceContainerHelper
#define OMULTITYPEINTERFACECONTAINERHELPER          ::cppu::OMultiTypeInterfaceContainerHelper
#define OPROPERTYSETHELPER                          ::cppu::OPropertySetHelper
#define OUSTRING                                    ::rtl::OUString
#define OWEAKOBJECT                                 ::cppu::OWeakObject
#define PROPERTY                                    ::com::sun::star::beans::Property
#define PROPERTYVALUE                               ::com::sun::star::beans::PropertyValue
#define TERMINATIONVETOEXCEPTION                    ::com::sun::star::frame::TerminationVetoException
#define UNOURL                                      ::com::sun::star::util::URL
#define WRAPPEDTARGETEXCEPTION                      ::com::sun::star::lang::WrappedTargetException
#define XCOMPONENT                                  ::com::sun::star::lang::XComponent
#define XCOMPONENTLOADER                            ::com::sun::star::frame::XComponentLoader
#define XCONTROLLER                                 ::com::sun::star::frame::XController
#define XDESKTOP                                    ::com::sun::star::frame::XDesktop
#define XDISPATCH                                   ::com::sun::star::frame::XDispatch
#define XDISPATCHDESCRIPTOR                         ::com::sun::star::frame::XDispatchDescriptor
#define XDISPATCHPROVIDER                           ::com::sun::star::frame::XDispatchProvider
#define XENUMERATIONACCESS                          ::com::sun::star::container::XEnumerationAccess
#define XEVENTLISTENER                              ::com::sun::star::lang::XEventListener
#define XFRAME                                      ::com::sun::star::frame::XFrame
#define XFRAMEACTIONLISTENER                        ::com::sun::star::frame::XFrameActionListener
#define XFRAMES                                     ::com::sun::star::frame::XFrames
#define XFRAMESSUPPLIER                             ::com::sun::star::frame::XFramesSupplier
#define XPROPERTYSETINFO                            ::com::sun::star::beans::XPropertySetInfo
#define XSERVICEINFO                                ::com::sun::star::lang::XServiceInfo
#define XSTATUSINDICATOR                            ::com::sun::star::task::XStatusIndicator
#define XSTATUSINDICATORFACTORY                     ::com::sun::star::task::XStatusIndicatorFactory
#define XSTATUSLISTENER                             ::com::sun::star::frame::XStatusListener
#define XTASK                                       ::com::sun::star::frame::XTask
#define XTASKSSUPPLIER                              ::com::sun::star::frame::XTasksSupplier
#define XTERMINATELISTENER                          ::com::sun::star::frame::XTerminateListener
#define XTYPEPROVIDER                               ::com::sun::star::lang::XTypeProvider
#define XWINDOW                                     ::com::sun::star::awt::XWindow

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

enum eIMPL_loadState
{
    UNKNOWN     ,
    SUCCESSFUL  ,
    FAILED
};

/*-************************************************************************************************************//**
    @short      implement the topframe of frame tree
    @descr      This is the root of the frame tree. The desktop has no window, is not visible but he is the logical
                "masternode" to build the hierarchy.

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XDesktop
                XComponentLoader
                XTasksSupplier
                XDispatchProvider
                XFramesSupplier
                XFrame
                XComponent
                XStatusIndicatorFactory
                XPropertySet
                XFastPropertySet
                XMultiPropertySet
                XStatusListener
                XEventListener
                [ XDebugging, if TEST_TREE is defined! ]
    @base       OMutexMember
                OBroadcastHelper
                OPropertySetHelper
                OWeakObject

    @devstatus  deprecated
*//*-*************************************************************************************************************/

//class Desktop :   DERIVE_FROM_XSPECIALDEBUGINTERFACE  // => These macro will expand to nothing, if no testmode is set in debug.h!
class Desktop   :   public XTYPEPROVIDER            ,
                    public XSERVICEINFO             ,
                    public XDESKTOP                 ,
                    public XCOMPONENTLOADER         ,
                    public XTASKSSUPPLIER           ,
                    public XDISPATCHPROVIDER        ,
                    public XFRAMESSUPPLIER          ,   // => XFrame => XComponent
                    public XSTATUSINDICATORFACTORY  ,
                    public XSTATUSLISTENER          ,   // => XEventListener
                    public OMutexMember             ,   // Struct for right initalization of mutex member! Must be the first one of baseclasses!
                    public OBROADCASTHELPER         ,
                    public OPROPERTYSETHELPER       ,
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

            @onerror    We throw an ASSERT in debug version or do nothing in relaese version.
        *//*-*****************************************************************************************************/

         Desktop( const REFERENCE< XMULTISERVICEFACTORY >& xFactory );

        /*-****************************************************************************************************//**
            @short      standard destructor
            @descr      This method destruct an instance of this class and clear some member.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~Desktop();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo + helper! (see macro.hxx)
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO
//      DECLARE_XSPECIALDEBUGINTERFACE  // => These macro will expand to nothing, if no testmode is set in debug.h!

        //---------------------------------------------------------------------------------------------------------
        //  XDesktop
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      ask desktop before terminate it
            @descr      The desktop ask his components and if all say "yes" it will destroy this components
                        and return "yes" to caller of this method. By the other way, desktop will not destroy!

            @seealso    -

            @param      -
            @return     sal_True  ,if all components say "yes"
            @return     sal_False ,otherwise

            @onerror    We return sal_False.
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL terminate() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      add a listener for terminate events
            @descr      You can add a listener, if you wish to get an event, if desktop will be terminate.

            @seealso    method removeTerminateListener()

            @param      "xListener" is a reference to the listener. His value must be valid!
            @return     -

            @onerror    No listener is added.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addTerminateListener( const REFERENCE< XTERMINATELISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      remove a listener for terminate events
            @descr      You can remove a listener, if you don't wish to get further event for desktop termination.

            @seealso    method removeTerminateListener()

            @param      "xListener" is a reference to the listener. His value must be valid!
            @return     -

            @onerror    No listener is removed.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeTerminateListener( const REFERENCE< XTERMINATELISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      get access to create enumerations of all current components
            @descr      Attention: You will be the owner of the returned object and must delete it
                        if you don't use it again. Use these, if you wish to create more then one enumerations.

            @seealso    class TasksAccess
            @seealso    class TasksEnumeration

            @param      -
            @return     A reference to an XEnumerationAccess-object.

            @onerror    We return a null-reference.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XENUMERATIONACCESS > SAL_CALL getComponents() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      return the current active component
            @descr      The most current component is the window, model or the controller of the current active frame.

            @seealso    method getCurrentFrame()
            @seealso    method impl_getFrameComponent()

            @param      -
            @return     A reference to the component.

            @onerror    We return a null-reference.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XCOMPONENT > SAL_CALL getCurrentComponent() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      return the current active frame in hierarchy
            @descr      There can be more then one different activiable pathes in hierarchy, but only one can be
                        active realy. These method return the most active one of ouer childs. They must have the focus!

            @seealso    method getActiveFrame()

            @param      -
            @return     A valid reference, if there is an active child task.
            @return     A null reference , otherwise.

            @onerror    We return a null reference.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAME > SAL_CALL getCurrentFrame() throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //  XComponentLoader
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      not implemented yet
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XCOMPONENT > SAL_CALL loadComponentFromURL(  const   OUSTRING&                   sURL                ,
                                                                        const   OUSTRING&                   sTargetFrameName    ,
                                                                                sal_Int32                   nSearchFlags        ,
                                                                        const   SEQUENCE< PROPERTYVALUE >&  seqArguments        ) throw(    IOEXCEPTION             ,
                                                                                                                                            ILLEGALARGUMENTEXCEPTION,
                                                                                                                                            RUNTIMEEXCEPTION        );
        //---------------------------------------------------------------------------------------------------------
        //  XTasksSupplier
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      get access to create enumerations of ouer taskchilds
            @descr      Direct childs of desktop are tasks everytime. Call these method to could create enumerations
                        of it. But; Don't forget - you will be the owner of returned object and must release it!
                        We use a helper class to implement the access interface. They hold a weakreference to us.
                        It can be, that the desktop is dead - but not your tasksaccess-object! Then they will do nothing!
                        You can't create enumerations then.

            @seealso    class OTasksAccess

            @param      -
            @return     A reference to an accessobject, which can create enumerations of ouer childtasks.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XENUMERATIONACCESS > SAL_CALL getTasks() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      return current active task of ouer direct childs
            @descr      Desktop childs are tasks only! If we have an active path from desktop as top to any frame
                        on bottom, we must have an active direct child. These reference is returned here.

            @ATTENTION  Do not confuse it with getCurrentFrame()! The current frame don't must one of ouer direct childs.
                        It can be every frame in subtree and must have the focus (Is the last one of an active path!).

            @seealso    -

            @param      -
            @return     A reference to ouer current active taskchild.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XTASK > SAL_CALL getActiveTask() throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //  XDispatchProvider
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      search a dispatcher for given URL
            @descr      These search for a dispatcher for given URL and parameter (targetname/flags).
                        If no interceptor is well known - we forward it to a helperclass. Otherwise we forward
                        call to the interceptor. They try to find a dispatcher. But if he does'nt find anyone,
                        he call ouer helperclass! (The helperclass is set as slave of interceptor.)
                        If ouer helper "must work" - he try to  find the right frame.
                        If this frame THIS implementation (the owner of helper!) - they use not us, they use ouer
                        current controller or component as dispatcher. (Because; There is the danger of recursion,
                        if the helper forward "queryDispatch()" to us!)

            @seealso    class ODispatchProvider

            @param      "aURL"              , URL to dispatch
            @param      "sTargetFrameName"  , name of target frame, who should dispatch these URL
            @param      "nSearchFlags"      , flags to regulate the search
            @return     A reference to a founded dispatcher for these URL.
            @return     A null reference, if no dispatcher was found.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XDISPATCH > SAL_CALL queryDispatch(  const   UNOURL&     aURL            ,
                                                                const   OUSTRING&   sTargetFrameName,
                                                                        sal_Int32   nSearchFlags    ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not implemented yet
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescripts ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //  XFramesSupplier
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
            @descr      It must be a task. Direct childs of desktop are tasks only! No frames are accepted.
                        We don't save this information directly in this class. We use ouer container-helper
                        to do that.

            @seealso    class OFrameContainer
            @seealso    method setActiveFrame()

            @param      -
            @return     A reference to ouer current active childtask, if anyone exist.
            @return     A null reference, if no active task exist.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAME > SAL_CALL getActiveFrame() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      set the new active direct child frame
            @descr      It must be a task. Direct childs of desktop are tasks only! No frames are accepted.
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
        //   XFrame
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      not supported!
            @descr      The desktop has no window!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL initialize( const REFERENCE< XWINDOW >& xWindow ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not supported!
            @descr      The desktop has no window!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XWINDOW > SAL_CALL getContainerWindow() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not supported!
            @descr      The desktop has no parent! We are on the top of hierarchy.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setCreator( const REFERENCE< XFRAMESSUPPLIER >& xCreator ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not supported!
            @descr      The desktop has no parent! We are on the top of hierarchy.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAMESSUPPLIER > SAL_CALL getCreator() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      get the name of the desktop
            @descr      You can set a name for desktop. But its not neccessary to do this.
                        This implementation use a default value "Desktop"!

            @seealso    method setName()

            @param      -
            @return     The name of desktop.

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual OUSTRING SAL_CALL getName() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      set the new name of the desktop
            @descr      You can set a name for desktop. But its not neccessary to do this.
                        This implementation use a default value "Desktop"!

            @seealso    method getName()

            @param      "sName", the new name.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setName( const OUSTRING& sName ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      try to find a frame with special parameters
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
                        If no Frame with the given name is found until the top frames container,
                        a new top Frame is created, if this is allowed by a special
                        FrameSearchFlag. The new Frame also gets the desired name.

            @seealso    method Frame::findFrame()

            @param      "sTargetFrameName"  , name of searched frame
            @param      "nSearchFlags"      , flags to regulate search
            @return     A reference to an existing frame in hierarchy, if it exist.
            @return     A null reference otherwise.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XFRAME > SAL_CALL findFrame( const   OUSTRING&   sTargetFrameName    ,
                                                                sal_Int32   nSearchFlags        ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      get state information about desktop
            @descr      The desktop is top everytime. We are the top of frame hierarchy!

            @seealso    -

            @param      -
            @return     sal_True, everytime!

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL isTop() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not implemented!
            @descr      The desktop is the topframe of hierarchy and is active everytime!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL activate() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not implemented!
            @descr      The desktop is the topframe of hierarchy and is active everytime!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL deactivate() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      get state information about desktop
            @descr      The desktop is active everytime. We are the top of frame hierarchy!

            @seealso    -

            @param      -
            @return     sal_True, everytime!

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL isActive() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not supported!
            @descr      The desktop has no component, no controller and no window.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL setComponent( const   REFERENCE< XWINDOW >&       xComponentWindow    ,
                                                const   REFERENCE< XCONTROLLER >&   xController         ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not supported!
            @descr      The desktop has no component, no controller and no window.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XWINDOW > SAL_CALL getComponentWindow() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not supported!
            @descr      The desktop has no component, no controller and no window.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XCONTROLLER > SAL_CALL getController() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not implemented yet!
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL contextChanged() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      add listener for frame action events
            @descr      If you wish to be informed, if something changed on frame hierarchy, add a listener here.

            @seealso    methode removeFrameActionListener()

            @param      "xListener", reference to an valid listener. We don't accept invalid values!
            @return     -

            @onerror    We do nothing or throw an ASSERT in debug version!
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addFrameActionListener( const REFERENCE< XFRAMEACTIONLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      remove listener for frame action events
            @descr      If you don't wish to be informed again, if something changed on frame hierarchy, remove
                        the listener here.

            @seealso    methode addFrameActionListener()

            @param      "xListener", reference to an valid listener. We don't accept invalid values!
            @return     -

            @onerror    We do nothing or throw an ASSERT in debug version!
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeFrameActionListener( const REFERENCE< XFRAMEACTIONLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        //---------------------------------------------------------------------------------------------------------
        //   XComponent
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      release all reference
            @descr      The owner of this object calles the dispose method if the object
                        should be destroyed. All other objects and components, that are registered
                        as an EventListener are forced to release their references to this object.
                        The reference attributes are disposed and released also.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL dispose() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      add listener for dispose events
            @descr      Adds an EventListener to this object. If the dispose method is called on
                        this object, the disposing method of the listener is called.

            @seealso    -

            @param      "xListener", reference to valid listener. We don't accept invalid values!
            @return     -

            @onerror    We do nothing or throw an ASSERT in debug version!
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL addEventListener( const REFERENCE< XEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      remove listener for dispose events

            @descr      Removes an EventListener. If another object that has registered itself as
                        an EventListener is disposed it must deregister by calling this method.

            @seealso    -

            @param      "xListener", reference to valid listener. We don't accept invalid values!
            @return     -

            @onerror    We do nothing or throw an ASSERT in debug version!
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL removeEventListener( const REFERENCE< XEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL disposing( const EVENTOBJECT& aSource ) throw( RUNTIMEEXCEPTION );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        /*-****************************************************************************************************//**
            @short      reset instance, free memory and something else ...
            @descr      Use this method to reset the current instance to default values and free used memory.
                        Best place to call these are dtor() and dispose().

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_resetObject();

        //---------------------------------------------------------------------------
        //  OPropertySetHelper
        //---------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      try to convert a property value
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!
                        You must try to convert the value of given propertyhandle and
                        return results of this operation. This will be use to ask vetoable
                        listener. If no listener have a veto, we will change value realy!
                        ( in method setFastPropertyValue_NoBroadcast(...) )

            @seealso    class OPropertySetHelper
            @seealso    method setFastPropertyValue_NoBroadcast()
            @seealso    method impl_tryToChangeProperty()

            @param      "aConvertedValue"   new converted value of property
            @param      "aOldValue"         old value of property
            @param      "nHandle"           handle of property
            @param      "aValue"            new value of property
            @return     sal_True if value will be changed, sal_FALSE otherway

            @onerror    IllegalArgumentException, if you call this with an invalid argument
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL convertFastPropertyValue(         ANY&        aConvertedValue ,
                                                                      ANY&      aOldValue       ,
                                                                    sal_Int32   nHandle         ,
                                                            const   ANY&        aValue          ) throw( ILLEGALARGUMENTEXCEPTION );

        /*-****************************************************************************************************//**
            @short      set value of a transient property
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!
                        Handle and value are valid everyway! You must set the new value only.
                        After this, baseclass send messages to all listener automaticly.

            @seealso    OPropertySetHelper

            @param      "nHandle"   handle of property to change
            @param      "aValue"    new value of property
            @return     -

            @onerror    An exception is thrown.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(         sal_Int32   nHandle ,
                                                                  const ANY&        aValue  ) throw( EXCEPTION );

        /*-****************************************************************************************************//**
            @short      get value of a transient property
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!

            @seealso    OPropertySetHelper

            @param      "nHandle"   handle of property to change
            @param      "aValue"    current value of property
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL getFastPropertyValue( ANY&        aValue  ,
                                                      sal_Int32 nHandle ) const;

        /*-****************************************************************************************************//**
            @short      return structure and information about transient properties
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!

            @seealso    OPropertySetHelper

            @param      -
            @return     structure with property-informations

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual IPROPERTYARRAYHELPER& SAL_CALL getInfoHelper();

        /*-****************************************************************************************************//**
            @short      return propertysetinfo
            @descr      You can call this method to get information about transient properties
                        of this object.

            @seealso    OPropertySetHelper
            @seealso    XPropertySet
            @seealso    XMultiPropertySet

            @param      -
            @return     reference to object with information [XPropertySetInfo]

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual REFERENCE< XPROPERTYSETINFO > SAL_CALL getPropertySetInfo();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return current component of current frame
            @descr      The desktop himself has no component. But every frame in subtree.
                        If somewhere call getCurrentComponent() at this class, we try to find the right frame and
                        then we try to become his component. It can be a VCL-component, the model or the controller
                        of founded frame.

            @seealso    method getCurrentComponent();

            @param      "xFrame", reference to valid frame in hierarchy. Method is not defined for invalid values.
                        But we don't check these. Its an IMPL-method and caller must seize that!
            @return     A reference to found component.
            @return     A null reference otherwiese.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

        REFERENCE< XCOMPONENT > impl_getFrameComponent( const REFERENCE< XFRAME >& xFrame ) const;

        /*-****************************************************************************************************//**
            @short      test, if a property will change his value
            @descr      These methods will test, if a property will change his current value, with given parameter.
                        We use a helperclass for properties. These class promote this behaviour.
                        We implement a helper function for every property-type!

            @seealso    method convertFastPropertyValue()

            @param      "...Property"       ,   the property with his current value
            @param      "aNewValue"         ,   new value for property
            @param      "aOldValue"         ,   old value of property as Any for convertFastPropertyValue
            @param      "aConvertedValue"   ,   new value of property as Any for convertFastPropertyValue(it can be the old one, if nothing is changed!)
            @return     sal_True  ,if value will be changed
            @return     sal_FALSE ,otherwise.

            @onerror    IllegalArgumentException, if convert failed.
        *//*-*****************************************************************************************************/

        sal_Bool impl_tryToChangeProperty(          sal_Bool    bProperty       ,
                                            const   ANY&        aNewValue       ,
                                                    ANY&        aOldValue       ,
                                                    ANY&        aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );

        /*-****************************************************************************************************//**
            @short      create table with information about properties
            @descr      We use a helper class to support properties. These class need some information about this.
                        These method create a new static description table with name, type, r/w-flags and so on ...

            @seealso    class OPropertySetHelper
            @seealso    method getInfoHelper()

            @param      -
            @return     Static table with information about properties.

            @onerror    -
        *//*-*****************************************************************************************************/

        static const SEQUENCE< PROPERTY > impl_getStaticPropertyDescriptor();

        /*-****************************************************************************************************//**
            @short      create a new task
            @descr      If findFrame() detect a "_blank" as targetname, he should create a new task.
                        These helper-method do this.
                        (Creation of a new task and initializing with an empty window and default values!)

            @seealso    method findFrame()

            @param      "sFrameName", new name for new taskframe.
            @return     A reference to the new created task.

            @onerror    A null reference is returned.
        *//*-*****************************************************************************************************/

//OBSOLETE      REFERENCE< XFRAME > impl_createNewTask( const OUSTRING& sFrameName );

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

//OBSOLETE      REFERENCE< XFRAME > impl_searchLastLoadedComponent();

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

        void impl_sendQueryTerminationEvent() throw( TERMINATIONVETOEXCEPTION );
        void impl_sendNotifyTerminationEvent();

        /*-****************************************************************************************************//**
            @short      search for any plugin frame to return current plugin state
            @descr      For property IsPlugged we need information about this state.
                        We search for any plugin frame in our container (it can be tasks only!).
                        If we found somewhere we return TRUE, FALSE otherwise.

            @seealso    property IsPlugged

            @param      -
            @return     sal_True, if a plugin frame exist, sal_False otherwise.

            @onerror    We return sal_False.
        *//*-*****************************************************************************************************/

        sal_Bool impl_checkPlugInState();

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

            sal_Bool impldbg_checkParameter_DesktopCtor                 (   const   REFERENCE< XMULTISERVICEFACTORY >&  xFactory        );
            sal_Bool impldbg_checkParameter_addTerminateListener        (   const   REFERENCE< XTERMINATELISTENER >&    xListener       );
            sal_Bool impldbg_checkParameter_removeTerminateListener     (   const   REFERENCE< XTERMINATELISTENER >&    xListener       );
            sal_Bool impldbg_checkParameter_loadComponentFromURL        (   const   OUSTRING&                           sURL            ,
                                                                               const    OUSTRING&                           sTargetFrameName,
                                                                                       sal_Int32                            nSearchFlags    ,
                                                                               const    SEQUENCE< PROPERTYVALUE >&          seqArguments    );
            sal_Bool impldbg_checkParameter_queryDispatch               (   const   UNOURL&                             aURL            ,
                                                                            const   OUSTRING&                           sTargetFrameName,
                                                                                    sal_Int32                           nSearchFlags    );
            sal_Bool impldbg_checkParameter_findFrame                   (   const   OUSTRING&                           sTargetFrameName,
                                                                                    sal_Int32                           nSearchFlags    );
            sal_Bool impldbg_checkParameter_addFrameActionListener      (   const   REFERENCE< XFRAMEACTIONLISTENER >&  xListener       );
            sal_Bool impldbg_checkParameter_removeFrameActionListener   (   const   REFERENCE< XFRAMEACTIONLISTENER >&  xListener       );
            sal_Bool impldbg_checkParameter_addEventListener            (   const   REFERENCE< XEVENTLISTENER >&        xListener       );
            sal_Bool impldbg_checkParameter_removeEventListener         (   const   REFERENCE< XEVENTLISTENER >&        xListener       );
            sal_Bool impldbg_checkParameter_statusChanged               (   const   FEATURESTATEEVENT&                  aEvent          );
            sal_Bool impldbg_checkParameter_disposing                   (   const   EVENTOBJECT&                        aSource         );

            sal_Bool m_bIsTerminated ;  /// check flag to protect us against dispose before terminate!
                                        /// see dispose() for further informations!

        #endif  // #ifdef ENABLE_ASSERTIONS

        /*-****************************************************************************************************//**
            @short      debug-method to get information about current tree
            @descr      You can use this method to print informations about the frames in ouer frametree.
                        The desktop is the top of this tree and this method print it from here to bottom.
                        Start in this implementation with out of tree!

            @seealso    method Frame::impldbg_getTreeNames()

            @param      -
            @return     The collected informations about ouer own container AND subtree as "string-stream".

            @onerror    -
        *//*-*****************************************************************************************************/

        #ifdef ENABLE_SERVICEDEBUG  // Only active in debug version!

            OUSTRING impldbg_getTreeNames();

        #endif  // #ifdef ENABLE_SERVICEDEBUG

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        REFERENCE< XMULTISERVICEFACTORY >           m_xFactory                  ;   /// reference to factory, which has create this instance
        OUSTRING                                    m_sName                     ;   /// desktop is a topframe and has a name
        FrameContainer                              m_aChildTaskContainer       ;   /// array of child tasks (childs of desktop are tasks; and tasks are also frames - But pure frames are not accepted!)
        OMULTITYPEINTERFACECONTAINERHELPER          m_aListenerContainer        ;   /// container for ALL Listener
        REFERENCE< XFRAMES >                        m_xFramesHelper             ;   /// helper for XFrames, XIndexAccess, XElementAccess and implementation of a childcontainer!
        REFERENCE< XDISPATCH >                      m_xDispatchHelper           ;   /// helper to dispatch something for new tasks, created by "_blank"!
        eIMPL_loadState                             m_eLoadState                ;   /// hold information about state of asynchron loading of component for loadComponentFromURL()!
        TaskCreator                                 m_aTaskCreator              ;   /// Helper to create new tasks or plugin frames!
        REFERENCE< XFRAME >                         m_xLastFrame                ;
        sal_Bool                                    m_bAlreadyDisposed          ;   /// protection against multiple dispose calls

        // Properties
//OLD   REFERENCE< XCOMPONENT >                     m_xActiveComponent          ;   Durch setActive/getActive am Container zu ersetzen! (+cast nach XComponent!)
//OLD   REFERENCE< XCOMPONENT >                     m_xActiveFrame              ;   Durch setActive/getActive am Container zu ersetzen!
        sal_Bool                                    m_bHasBeamer                ;
        sal_Bool                                    m_bHasCommonTaskBar         ;
        sal_Bool                                    m_bHasDesigner              ;
        sal_Bool                                    m_bHasExplorer              ;
        sal_Bool                                    m_bHasFunctionBar           ;
        sal_Bool                                    m_bHasMacroBar              ;
        sal_Bool                                    m_bHasNavigator             ;
        sal_Bool                                    m_bHasObjectBar             ;
        sal_Bool                                    m_bHasOptionBar             ;
        sal_Bool                                    m_bHasStatusBar             ;
        sal_Bool                                    m_bHasToolbar               ;
        LOCALE                                      m_aISOLocale                ;

};  //  class Desktop

}   //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_DESKTOP_HXX_
