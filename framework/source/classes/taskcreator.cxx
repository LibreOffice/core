/*************************************************************************
 *
 *  $RCSfile: taskcreator.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: mba $ $Date: 2001-09-19 08:06:50 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
#include <classes/taskcreator.hxx>
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

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_WINDOWDESCRIPTOR_HPP_
#include <com/sun/star/awt/WindowDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
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
    @short      create a new task with a system window inside
    @descr      With this method you can create a new empty system task. We create the task and the container
                window inside of it. Created node will be a child of given parent frame.

    @seealso    method createBrowserTask()

    @param      "aInfo", collection of information, which are used to create task
    @return     A reference to the new created task.

    @onerror    We return a null-reference.
    @threadsafe no
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > TaskCreator::createSystemTask( const TaskInfo& aInfo )
{
    // Safe impossible cases.
    // Method is not designed for all incoming parameter!
    LOG_ASSERT2( implcp_createSystemTask( aInfo ), "TaskCreator::createNewSystemTask()", "Invalid parameter detected!" )

    // Set default return value to NULL!
    css::uno::Reference< css::frame::XFrame > xTask;

    // Get toolkit to create task container window.
    css::uno::Reference< css::awt::XToolkit > xToolkit( aInfo.xFactory->createInstance( SERVICENAME_VCLTOOLKIT ), css::uno::UNO_QUERY );
    if( xToolkit.is() == sal_True )
    {
        // Describe window properties.
        css::awt::WindowDescriptor aDescriptor;
        aDescriptor.Type                =   css::awt::WindowClass_TOP                       ;
        aDescriptor.WindowServiceName   =   DECLARE_ASCII("window")                         ;
        aDescriptor.ParentIndex         =   -1                                              ;
        aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >()  ;
        aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0)                    ;
        aDescriptor.WindowAttributes    =   css::awt::WindowAttribute::BORDER               |
                                            css::awt::WindowAttribute::MOVEABLE             |
                                            css::awt::WindowAttribute::SIZEABLE             |
                                            css::awt::WindowAttribute::CLOSEABLE            ;
        // Create a new blank container window and get access to parent container to append new created task.
        css::uno::Reference< css::awt::XWindowPeer > xPeer      = xToolkit->createWindow( aDescriptor );
        css::uno::Reference< css::awt::XWindow >     xWindow    ( xPeer, css::uno::UNO_QUERY );
        xPeer->setBackground( 0xFFFFFFFF );
        css::uno::Reference< css::frame::XFrames >   xContainer = aInfo.xParent->getFrames();
        if(
            ( xWindow.is()    == sal_True ) &&
            ( xContainer.is() == sal_True )
          )
        {
            // Create new system task.
            xTask = css::uno::Reference< css::frame::XFrame >( aInfo.xFactory->createInstance( SERVICENAME_TASK ), css::uno::UNO_QUERY );
            if( xTask.is() == sal_True )
            {
                // Set window on task.
                // Do it before you call other interface methods on task-object ...
                // because this object must be initialized before you can do such things.
                // Otherwise he throw an exception for UNINITIALIZED working mode!

                // Don't forget to create tree-bindings! use given parent as parent node of new task ...
                // ... and append it to his container.
                // (task member xParent will automaticly set by "append()" call!)

                // ! sTaskName already filtered by TaskInfo structure! Special targets are not allowed here ...

                // Disable task window first! Otherwise it's visible during showing of any progress
                // and user interaction could make some trouble ... GPF is possible!
                // So we disable it here ... and our loading proccess enable it after successfully operation.
                xTask->initialize  ( xWindow         );
                xTask->setName     ( aInfo.sTaskName );
                xContainer->append ( xTask           );
            }
        }
    }

    // Return result of this operation.
    return xTask;
}

//*****************************************************************************************************************
css::uno::Reference< css::frame::XFrame > TaskCreator::createBrowserTask( const TaskInfo& aInfo )
{
    LOG_ERROR( "TaskCreator::createNewBrowserTask()", "Not supported yet! Return empty reference." )
    return css::uno::Reference< css::frame::XFrame >();
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
sal_Bool TaskCreator::implcp_createSystemTask( const TaskInfo& aInfo )
{
    return(
            ( &aInfo              == NULL                       )   ||
            ( aInfo.xFactory.is() == sal_False                  )   ||
            ( aInfo.xParent.is()  == sal_False                  )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_SELF         )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_BLANK        )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_PARENT       )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_TOP          )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_MENUBAR      )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_HELPAGENT    )   ||
            (
                ( aInfo.bVisible  != sal_True  ) &&
                ( aInfo.bVisible  != sal_False )
            )
          );
}

//*****************************************************************************************************************
sal_Bool TaskCreator::implcp_createBrowserTask( const TaskInfo& aInfo )
{
    return(
            ( &aInfo              == NULL                       )   ||
            ( aInfo.xFactory.is() == sal_False                  )   ||
            ( aInfo.xParent.is()  == sal_False                  )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_SELF         )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_BLANK        )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_PARENT       )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_TOP          )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_MENUBAR      )   ||
            ( aInfo.sTaskName     == SPECIALTARGET_HELPAGENT    )   ||
            (
                ( aInfo.bVisible  != sal_True  ) &&
                ( aInfo.bVisible  != sal_False )
            )
          );
}

#endif  // #ifdef ENABLE_ASSERTIONS

}   //  namespace framework
