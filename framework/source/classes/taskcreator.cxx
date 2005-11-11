/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: taskcreator.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 12:04:35 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
#include <classes/taskcreator.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FRAMELISTANALYZER_HXX_
#include <classes/framelistanalyzer.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_
#include <helper/persistentwindowstate.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_TARGETS_H_
#include <targets.h>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_WINDOWDESCRIPTOR_HPP_
#include <com/sun/star/awt/WindowDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

//_________________________________________________________________________________________________________________
//  includes of my own project
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-****************************************************************************************************//**
    @short      initialize instance with neccessary informations
    @descr      We need a valid uno service manager to create or instanciate new services.
                All other informations to create frames or tasks come in on right interface methods.

    @param      xSMGR
                    points to the valid uno service manager

    @modified   16.05.2002 09:25, as96863
*//*-*****************************************************************************************************/
TaskCreator::TaskCreator( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
    : ThreadHelpBase(       )
    , m_xSMGR       ( xSMGR )
{
}

/*-****************************************************************************************************//**
    @short      deinitialize instance
    @descr      We should release all used ressource which are not needed any longer.

    @modified   16.05.2002 09:33, as96863
*//*-*****************************************************************************************************/
TaskCreator::~TaskCreator()
{
    m_xSMGR = NULL;
}

/*-****************************************************************************************************//**
    @short      create a new task on desktop
    @descr      We use the global desktop instance as parent of the new created frame
                and initialize it with some state values (name, visible).
                But this function don't create such task directly. It decide only if it must be
                a system or browser task. Because it depends from th office environment which one
                is required.

    @param      sName
                    the name of this new created frame
                    Note: Special ones like e.g. "_blank" are not allowed here. We check it and ignore
                    such names.

    @param      bVisible
                    We use it to show or hide the new created container window inside this frame.

    @modified   16.05.2002 09:36, as96863
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > TaskCreator::createTask( const ::rtl::OUString& sName    ,
                                                                         sal_Bool         bVisible )
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XFramesSupplier > xDesktop(
        xSMGR->createInstance(SERVICENAME_DESKTOP),
        css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::frame::XFrame > xTask = implts_createSystemTask(xDesktop, sName, bVisible);
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
css::uno::Reference< css::frame::XFrame > TaskCreator::implts_createSystemTask( const css::uno::Reference< css::frame::XFramesSupplier >&   xDesktop ,
                                                                                const ::rtl::OUString&                                      sName    ,
                                                                                      sal_Bool                                              bVisible )
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    /* } SAFE */

    // check if it's a special target name.
    // Such values indicates creation of document windows
    // and such target names are not allowed to be set as
    // frame name!

    // On the other side all real named frames indicates using of these frames
    // as non document-windows (e.g. help window, life-previews of wizards etcpp).

    ::rtl::OUString sFilteredName = impl_filterNames(sName);
    sal_Bool        bIsDocument   = (! sFilteredName.getLength());

    // get toolkit to create task container window
    css::uno::Reference< css::awt::XToolkit > xToolkit(
        xSMGR->createInstance(SERVICENAME_VCLTOOLKIT),
        css::uno::UNO_QUERY_THROW);

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
    css::uno::Reference< css::frame::XFrame >    xTask;

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
            xContainer->append ( xTask   );

            if (bVisible)
                xWindow->setVisible(bVisible);

            if (sFilteredName.getLength())
                xTask->setName(sFilteredName);

            if (bIsDocument)
            {
                // SOLAR SAFE ->
                // #125187# + #i53630#
                // Mark all document windows as "special ones", so VCL can bind
                // special features to it. Because VCL doesnt know anything about documents ...
                // Note: Doing so it's no longer supported, that e.g. our wizards can use findFrame(_blank)
                // to create it's previes frames. They must do it manually by using WindowDescriptor+Toolkit!
                ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());
                Window* pVCLWindow = VCLUnoHelper::GetWindow(xWindow);
                if (pVCLWindow)
                    pVCLWindow->SetExtendedStyle(WB_EXT_DOCUMENT);
                aSolarGuard.clear();
                // <- SOLAR SAFE
            }

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
::rtl::OUString TaskCreator::impl_filterNames( const ::rtl::OUString& sName )
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
