/*************************************************************************
 *
 *  $RCSfile: desktop.hxx,v $
 *
 *  $Revision: 1.18 $
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

#ifndef __FRAMEWORK_SERVICES_DESKTOP_HXX_
#define __FRAMEWORK_SERVICES_DESKTOP_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_FRAMECONTAINER_HXX_
#include <classes/framecontainer.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
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

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRECORDERSUPPLIER_HPP_
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef INCLUDED_SVTOOLS_CMDOPTIONS_HXX
#include <svtools/cmdoptions.hxx>
#endif

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

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

enum ELoadState
{
    E_NOTSET      ,
    E_SUCCESSFUL  ,
    E_FAILED      ,
    E_INTERACTION
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
                XPropertySet
                XFastPropertySet
                XMultiPropertySet
                XDispatchResultListener
                XEventListener
                XInteractionHandler

    @base       ThreadHelpBase
                TransactionBase
                OBroadcastHelper
                OPropertySetHelper

    @devstatus  ready to use
    @threadsafe yes
*//*-*************************************************************************************************************/
class Desktop   :   // interfaces
                    public  css::lang::XTypeProvider             ,
                    public  css::lang::XServiceInfo              ,
                    public  css::frame::XDesktop                 ,
                    public  css::frame::XComponentLoader         ,
                    public  css::frame::XTasksSupplier           ,
                    public  css::frame::XDispatchProvider        ,
                    public  css::frame::XFramesSupplier          ,   // => XFrame => XComponent
                    public  css::frame::XDispatchResultListener  ,   // => XEventListener
                    public  css::task::XInteractionHandler       ,
                    // base classes
                    // Order is neccessary for right initialization!
                    private ThreadHelpBase                       ,
                    private TransactionBase                      ,
                    public  ::cppu::OBroadcastHelper             ,
                    public  ::cppu::OPropertySetHelper           ,
                    public  ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //  constructor / destructor
                 Desktop( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~Desktop(                                                                        );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //  XDesktop
        virtual sal_Bool                                                            SAL_CALL terminate                  (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL addTerminateListener       ( const css::uno::Reference< css::frame::XTerminateListener >&   xListener        ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL removeTerminateListener    ( const css::uno::Reference< css::frame::XTerminateListener >&   xListener        ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::container::XEnumerationAccess >           SAL_CALL getComponents              (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::lang::XComponent >                        SAL_CALL getCurrentComponent        (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::frame::XFrame >                           SAL_CALL getCurrentFrame            (                                                                                 ) throw( css::uno::RuntimeException          );

        //  XComponentLoader
        virtual css::uno::Reference< css::lang::XComponent >                        SAL_CALL loadComponentFromURL       ( const ::rtl::OUString&                                         sURL             ,
                                                                                                                          const ::rtl::OUString&                                         sTargetFrameName ,
                                                                                                                                sal_Int32                                                nSearchFlags     ,
                                                                                                                          const css::uno::Sequence< css::beans::PropertyValue >&         lArguments       ) throw( css::io::IOException                ,
                                                                                                                                                                                                                   css::lang::IllegalArgumentException ,
                                                                                                                                                                                                                   css::uno::RuntimeException          );

        //  XTasksSupplier
        virtual css::uno::Reference< css::container::XEnumerationAccess >           SAL_CALL getTasks                   (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::frame::XTask >                            SAL_CALL getActiveTask              (                                                                                 ) throw( css::uno::RuntimeException          );

        //  XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch >                        SAL_CALL queryDispatch              ( const css::util::URL&                                          aURL             ,
                                                                                                                          const ::rtl::OUString&                                         sTargetFrameName ,
                                                                                                                                sal_Int32                                                nSearchFlags     ) throw( css::uno::RuntimeException          );
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > >  SAL_CALL queryDispatches            ( const css::uno::Sequence< css::frame::DispatchDescriptor >&    lQueries         ) throw( css::uno::RuntimeException          );

        //  XFramesSupplier
        virtual css::uno::Reference< css::frame::XFrames >                          SAL_CALL getFrames                  (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::frame::XFrame >                           SAL_CALL getActiveFrame             (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL setActiveFrame             ( const css::uno::Reference< css::frame::XFrame >&               xFrame           ) throw( css::uno::RuntimeException          );

        //   XFrame
        //  Attention: findFrame() is implemented only! Other methods make no sense for our desktop!
        virtual css::uno::Reference< css::frame::XFrame >                           SAL_CALL findFrame                  ( const ::rtl::OUString&                                         sTargetFrameName ,
                                                                                                                                sal_Int32                                                nSearchFlags     ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL initialize                 ( const css::uno::Reference< css::awt::XWindow >&                xWindow          ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::awt::XWindow >                            SAL_CALL getContainerWindow         (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL setCreator                 ( const css::uno::Reference< css::frame::XFramesSupplier >&      xCreator         ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::frame::XFramesSupplier >                  SAL_CALL getCreator                 (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual ::rtl::OUString                                                     SAL_CALL getName                    (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL setName                    ( const ::rtl::OUString&                                         sName            ) throw( css::uno::RuntimeException          );
        virtual sal_Bool                                                            SAL_CALL isTop                      (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL activate                   (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL deactivate                 (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual sal_Bool                                                            SAL_CALL isActive                   (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual sal_Bool                                                            SAL_CALL setComponent               ( const css::uno::Reference< css::awt::XWindow >&                xComponentWindow ,
                                                                                                                          const css::uno::Reference< css::frame::XController >&          xController      ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::awt::XWindow >                            SAL_CALL getComponentWindow         (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual css::uno::Reference< css::frame::XController >                      SAL_CALL getController              (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL contextChanged             (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL addFrameActionListener     ( const css::uno::Reference< css::frame::XFrameActionListener >& xListener        ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL removeFrameActionListener  ( const css::uno::Reference< css::frame::XFrameActionListener >& xListener        ) throw( css::uno::RuntimeException          );

        //   XComponent
        virtual void                                                                SAL_CALL dispose                    (                                                                                 ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL addEventListener           ( const css::uno::Reference< css::lang::XEventListener >&        xListener        ) throw( css::uno::RuntimeException          );
        virtual void                                                                SAL_CALL removeEventListener        ( const css::uno::Reference< css::lang::XEventListener >&        xListener        ) throw( css::uno::RuntimeException          );

        //   XDispatchResultListener
        virtual void SAL_CALL dispatchFinished      ( const css::frame::DispatchResultEvent&                    aEvent     ) throw( css::uno::RuntimeException );

        //   XEventListener
        virtual void                                                                SAL_CALL disposing                  ( const css::lang::EventObject&                                  aSource          ) throw( css::uno::RuntimeException          );

        //   XInteractionHandler
        virtual void                                                                SAL_CALL handle                     ( const css::uno::Reference< css::task::XInteractionRequest >&   xRequest         ) throw( css::uno::RuntimeException          );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    protected:

        //  OPropertySetHelper
        virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        (       css::uno::Any&  aConvertedValue ,
                                                                                                                     css::uno::Any&  aOldValue       ,
                                                                                                                     sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw( css::lang::IllegalArgumentException );
        virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast(       sal_Int32       nHandle         ,
                                                                                                               const css::uno::Any&  aValue          ) throw( css::uno::Exception                 );
        virtual void                                                SAL_CALL getFastPropertyValue            (       css::uno::Any&  aValue          ,
                                                                                                                     sal_Int32       nHandle         ) const;
        virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper                   (                                       );
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo              (                                       ) throw (::com::sun::star::uno::RuntimeException);

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        css::uno::Reference< css::lang::XComponent >            impl_getFrameComponent          ( const css::uno::Reference< css::frame::XFrame >&  xFrame          ) const;
        static const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor(                                                                   );
        void                                                    impl_sendQueryTerminationEvent  (                                                                   ) throw( css::frame::TerminationVetoException );
        void                                                    impl_sendNotifyTerminationEvent (                                                                   );
        sal_Bool                                                impl_checkPlugInState           (                                                                   ) const;

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everytime!)
    //-------------------------------------------------------------------------------------------------------------
    #ifdef ENABLE_ASSERTIONS
    private:

        static sal_Bool implcp_ctor                     ( const css::uno::Reference< css::lang::XMultiServiceFactory >&     xFactory         );
        static sal_Bool implcp_addTerminateListener     ( const css::uno::Reference< css::frame::XTerminateListener >&      xListener        );
        static sal_Bool implcp_removeTerminateListener  ( const css::uno::Reference< css::frame::XTerminateListener >&      xListener        );
        static sal_Bool implcp_findFrame                ( const ::rtl::OUString&                                            sTargetFrameName ,
                                                                sal_Int32                                                   nSearchFlags     );
        static sal_Bool implcp_addEventListener         ( const css::uno::Reference< css::lang::XEventListener >&           xListener        );
        static sal_Bool implcp_removeEventListener      ( const css::uno::Reference< css::lang::XEventListener >&           xListener        );
        static sal_Bool implcp_statusChanged            ( const css::frame::FeatureStateEvent&                              aEvent           );

        sal_Bool m_bIsTerminated ;  /// check flag to protect us against dispose before terminate!
                                    /// see dispose() for further informations!

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everytime!)
    //-------------------------------------------------------------------------------------------------------------
    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >          m_xFactory                  ;   /// reference to factory, which has create this instance
        FrameContainer                                                  m_aChildTaskContainer       ;   /// array of child tasks (childs of desktop are tasks; and tasks are also frames - But pure frames are not accepted!)
        ::cppu::OMultiTypeInterfaceContainerHelper                      m_aListenerContainer        ;   /// container for ALL Listener
        css::uno::Reference< css::frame::XFrames >                      m_xFramesHelper             ;   /// helper for XFrames, XIndexAccess, XElementAccess and implementation of a childcontainer!
        css::uno::Reference< css::frame::XDispatchProvider >            m_xDispatchHelper           ;   /// helper to dispatch something for new tasks, created by "_blank"!
        ELoadState                                                      m_eLoadState                ;   /// hold information about state of asynchron loading of component for loadComponentFromURL()!
        css::uno::Reference< css::frame::XFrame >                       m_xLastFrame                ;   /// last target of "loadComponentFromURL()"!
        css::uno::Reference< css::frame::XTerminateListener >           m_xPipeTerminator           ;   /// special terminate listener to close pipe and block external requests during/after terminate
        css::uno::Reference< css::frame::XTerminateListener >           m_xQuickLauncher            ;   /// special terminate listener to block terminate if tray-icon is active
        css::uno::Any                                                   m_aInteractionRequest       ;
        sal_Bool                                                        m_bSuspendQuickstartVeto    ;   /// don't ask quickstart for a veto
        SvtCommandOptions                                               m_aCommandOptions           ;   /// ref counted class to support disabling commands defined by configuration file
        ::rtl::OUString                                                 m_sName                     ;
        ::rtl::OUString                                                 m_sTitle                    ;
        css::uno::Reference< css::frame::XDispatchRecorderSupplier >    m_xDispatchRecorderSupplier ;

};      //  class Desktop

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_DESKTOP_HXX_
