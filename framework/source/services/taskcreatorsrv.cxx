/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: taskcreatorsrv.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-30 08:11:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_SERVICES_TASKCREATORSRV_HXX_
#include "services/taskcreatorsrv.hxx"
#endif

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_
#include <helper/persistentwindowstate.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_TARGETS_H_
#include <targets.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWDESCRIPTOR_HPP_
#include <com/sun/star/awt/WindowDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif

//_______________________________________________
// other includes

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_______________________________________________
// namespaces

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework
{

//-----------------------------------------------
DEFINE_XINTERFACE_3(TaskCreatorService                                       ,
                    OWeakObject                                       ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider        ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo         ),
                    DIRECT_INTERFACE(css::lang::XSingleServiceFactory))

//-----------------------------------------------
DEFINE_XTYPEPROVIDER_3(TaskCreatorService                     ,
                       css::lang::XTypeProvider        ,
                       css::lang::XServiceInfo         ,
                       css::lang::XSingleServiceFactory)

//-----------------------------------------------
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE(TaskCreatorService                       ,
                                       ::cppu::OWeakObject               ,
                                       SERVICENAME_TASKCREATOR           ,
                                       IMPLEMENTATIONNAME_FWK_TASKCREATOR)

//-----------------------------------------------
DEFINE_INIT_SERVICE(
                    TaskCreatorService,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//-----------------------------------------------
TaskCreatorService::TaskCreatorService(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase     (&Application::GetSolarMutex())
    , ::cppu::OWeakObject(                             )
    , m_xSMGR            (xSMGR                        )
{
}

//-----------------------------------------------
TaskCreatorService::~TaskCreatorService()
{
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > SAL_CALL TaskCreatorService::createInstance()
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(css::uno::Sequence< css::uno::Any >());
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > SAL_CALL TaskCreatorService::createInstanceWithArguments(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    static ::rtl::OUString PROP_TARGETNAME       = ::rtl::OUString::createFromAscii("TargetName");
    static ::rtl::OUString PROP_FRAME            = ::rtl::OUString::createFromAscii("Frame"     );
    static ::rtl::OUString PROP_VISIBLE          = ::rtl::OUString::createFromAscii("Visible"   );
    static ::rtl::OUString DEFAULTVAL_TARGETNAME = ::rtl::OUString::createFromAscii("_default"  );
    static sal_Bool        DEFAULTVAL_VISIBLE    = sal_False                                     ;

    ::comphelper::SequenceAsHashMap lArgs(lArguments);

    ::rtl::OUString                           sTarget  = lArgs.getUnpackedValueOrDefault(PROP_TARGETNAME, DEFAULTVAL_TARGETNAME );
    sal_Bool                                  bVisible = lArgs.getUnpackedValueOrDefault(PROP_VISIBLE   , DEFAULTVAL_VISIBLE);
    css::uno::Reference< css::frame::XFrame > xFrame   = lArgs.getUnpackedValueOrDefault(PROP_FRAME     , css::uno::Reference< css::frame::XFrame >());

    // Check incoming parameter. We don't allow special target names like e.g. "_blank"
    ::rtl::OUString sRightName = impl_filterNames(sTarget);

    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XFramesSupplier > xDesktop( xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::uno::XInterface >        xTask   ( implts_createSystemTask(xDesktop, sRightName, bVisible), css::uno::UNO_QUERY_THROW);
    return xTask;
}

/*-****************************************************************************************************//**
    @short      create a new task with a system window inside
    @descr      With this method you can create a new empty system task. We create the task and the container
                window inside of it. Created node will be a child of given parent - which can be the desktop only.

    @param      xDesktop
                    only the desktop can be the parent of such new created task frame
    @param      sName
                    the new name for this task (filtered!)
    @param      bVisible
                    used to set the state of frame container window after creation

    @return     A reference to the new created task or <NULL/> if it failed.

    @threadsafe yes
    @modified   16.05.2002 10:44, as96863
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > TaskCreatorService::implts_createSystemTask( const css::uno::Reference< css::frame::XFramesSupplier >&   xDesktop ,
                                                                                const ::rtl::OUString&                                      sName    ,
                                                                                      sal_Bool                                              bVisible )
{
    css::uno::Reference< css::frame::XFrame > xTask;

    // get toolkit to create task container window
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    /* } SAFE */
    css::uno::Reference< css::awt::XToolkit > xToolkit( xSMGR->createInstance( SERVICENAME_VCLTOOLKIT ), css::uno::UNO_QUERY_THROW);

    // describe window properties.
    css::awt::WindowDescriptor aDescriptor;
    aDescriptor.Type                =   css::awt::WindowClass_TOP                       ;
    aDescriptor.WindowServiceName   =   DECLARE_ASCII("window")                         ;
    aDescriptor.ParentIndex         =   -1                                              ;
    aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >()  ;
    aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0)                    ;
    aDescriptor.WindowAttributes    =   css::awt::WindowAttribute::BORDER               |
                                        css::awt::WindowAttribute::MOVEABLE             |
                                        css::awt::WindowAttribute::SIZEABLE             |
                                        css::awt::WindowAttribute::CLOSEABLE            |
                                        css::awt::VclWindowPeerAttribute::CLIPCHILDREN  ;
    // create a new blank container window and get access to parent container to append new created task.
    css::uno::Reference< css::awt::XWindowPeer > xPeer      = xToolkit->createWindow( aDescriptor );
    css::uno::Reference< css::awt::XWindow >     xWindow    ( xPeer, css::uno::UNO_QUERY );
    xPeer->setBackground(::svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor);
    css::uno::Reference< css::frame::XFrames >   xContainer = xDesktop->getFrames();
    if (
        ( xWindow.is()    ) &&
        ( xContainer.is() )
       )
    {
        // create new top level frame.
        xTask = css::uno::Reference< css::frame::XFrame >( xSMGR->createInstance( SERVICENAME_FRAME ), css::uno::UNO_QUERY );
        if (xTask.is())
        {
            // Set window on task.
            // Do it before you call other interface methods on task-object ...
            // because this object must be initialized before you can do such things.
            // Otherwise he throw an exception for UNINITIALIZED working mode!

            // Don't forget to create tree-bindings! use given parent as parent node of new task ...
            // ... and append it to his container.
            // (task member xParent will automaticly set by "append()" call!)

            xTask->initialize  ( xWindow );
            xTask->setName     ( sName   );
            xContainer->append ( xTask   );

            if (bVisible)
                xWindow->setVisible(bVisible);

            // Special feature: It's allowed for system tasks only - not for browser plugged ones!
            // We must create a special listener service and couple it with the new created task frame.
            // He will restore or save the window state of it ...
            // See used classes for further informations too.
            PersistentWindowState* pPersistentStateHandler = new PersistentWindowState(xSMGR);
            css::uno::Reference< css::lang::XInitialization > xInit(static_cast< ::cppu::OWeakObject* >(pPersistentStateHandler), css::uno::UNO_QUERY);
            // This will start listening at the task frame ... and then these two objects hold herself alive!
            // We can forget xInit without any problems.
            css::uno::Sequence< css::uno::Any > lInitData(1);
            lInitData[0] <<= xTask;
            xInit->initialize(lInitData);
        }
    }

    return xTask;
}

/*-****************************************************************************************************//**
    @short      decide which names are correct frame names
    @descr      Not all names are allowed as frame name. e.g. special targets like "_blank" are forbidden.
                They are used to force creation of new tasks and can make trouble during search off already
                existing ones.

    @attention  "_beamer" is a valid name - because:
                It exist one beamer for one task tree only.
                If he exist, we can find it - otherwhise he will be created by our task-frame!

    @param      sName
                    whished name by user

    @return     The given name of user if it is an allowed one - or an empty string if not.

    @threadsafe not neccessary
    @modified   16.05.2002 10:32, as96863
*//*-*****************************************************************************************************/
::rtl::OUString TaskCreatorService::impl_filterNames( const ::rtl::OUString& sName )
{
    ::rtl::OUString sFiltered( sName );
    if(
        ( sName == SPECIALTARGET_BLANK      )   ||
        ( sName == SPECIALTARGET_DEFAULT    )   ||
        ( sName == SPECIALTARGET_SELF       )   ||
        ( sName == SPECIALTARGET_PARENT     )   ||
        ( sName == SPECIALTARGET_TOP        )   ||
        ( sName == SPECIALTARGET_MENUBAR    )   ||
        ( sName == SPECIALTARGET_HELPAGENT  )
       )
    {
        sFiltered = ::rtl::OUString();
    }
    return sFiltered;
}

} // namespace framework
