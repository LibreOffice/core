/*************************************************************************
 *
 *  $RCSfile: taskcreator.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: as $ $Date: 2001-03-09 14:42:25 $
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

#ifndef __FRAMEWORK_DEFINES_HXX_
#include <defines.hxx>
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

using namespace ::com::sun::star::awt       ;
using namespace ::com::sun::star::frame     ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::uno       ;
using namespace ::rtl                       ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
TaskCreator::TaskCreator( const Reference< XMultiServiceFactory >& xFactory )
        // Init member
        :   m_xFactory( xFactory )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
TaskCreator::~TaskCreator()
{
    // Free memory and release used references.
    m_xFactory = Reference< XMultiServiceFactory >();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > TaskCreator::createNewSystemTask( const OUString& sName )
{
    // Safe impossible cases.
    // Method is not designed for all incoming parameter!
    LOG_ASSERT( impldbg_checkParameter_createNewSystemTask( sName ), "TaskCreator::createNewSystemTask()\nInvalid parameter detected!\n" )

    OUString sFrameName = impl_filterNames( sName );

    // We must append a new task at our desktop.
    // If no desktop exist we cant work correctly.
    // Desktop is an one instance service. We will get a reference to it only!
    Reference< XFramesSupplier > xDesktop( m_xFactory->createInstance( SERVICENAME_DESKTOP ), UNO_QUERY );
    // Create the new task.
    Reference< XFrame > xTask( m_xFactory->createInstance( SERVICENAME_TASK ), UNO_QUERY );
    // Get the toolkit to create a blank window for the new task.
    Reference< XToolkit > xToolkit( m_xFactory->createInstance( SERVICENAME_VCLTOOLKIT ), UNO_QUERY );
    // Safe impossible cases
    LOG_ASSERT( !(xDesktop.is()==sal_False), "TaskCreator::createNewSystemTask()\nServicename of Desktop is unknown!\n"     )
    LOG_ASSERT( !(xTask.is()   ==sal_False), "TaskCreator::createNewSystemTask()\nServicename of Task is unknown!\n"        )
    LOG_ASSERT( !(xToolkit.is()==sal_False), "TaskCreator::createNewSystemTask()\nServicename of VCLToolkit is unknown!\n"  )
    if  (
            ( xDesktop.is() ==  sal_True    )   &&
            ( xTask.is()    ==  sal_True    )   &&
            ( xToolkit.is() ==  sal_True    )
        )
    {
        // Describe the window properties.
        WindowDescriptor aDescriptor;
        aDescriptor.Type                =   WindowClass_TOP             ;
        aDescriptor.WindowServiceName   =   DECLARE_ASCII("window")     ;
        aDescriptor.ParentIndex         =   -1                          ;
        aDescriptor.Parent              =   Reference< XWindowPeer >()  ;
        aDescriptor.Bounds              =   Rectangle(0,0,0,0)          ;
        aDescriptor.WindowAttributes    =   WindowAttribute::BORDER     |
                                            WindowAttribute::MOVEABLE   |
                                            WindowAttribute::SIZEABLE   |
                                            WindowAttribute::CLOSEABLE  ;
        // Create a new blank window and set it on this instance.
        Reference< XWindowPeer >    xPeer   = xToolkit->createWindow( aDescriptor );
        Reference< XWindow >        xWindow ( xPeer, UNO_QUERY );
        if  (
                ( xWindow.is()  ==  sal_True    )   &&
                ( xTask.is()    ==  sal_True    )
            )
        {
            // Don't forget to create tree-bindings! Set this desktop as parent of new task ...
            // ... and append it to his container.
            // (Parent will automaticly set by "append()"!)
            xTask->setName( sFrameName );
            xDesktop->getFrames()->append( xTask );
            // Set window on task.
            xTask->initialize( xWindow );
            xWindow->setEnable( sal_True );
        }
    }

    // Return result of this operation.
    return xTask;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Reference< XFrame > TaskCreator::createNewBrowserTask( const OUString& sName )
{
    // Safe impossible cases.
    // Method is not designed for all incoming parameter!
    LOG_ASSERT( impldbg_checkParameter_createNewBrowserTask( sName ), "TaskCreator::createNewBrowserTask()\nInvalid parameter detected!\n" )
    // Set default return value if method failed.
    Reference< XFrame > xPlugInFrame;

    OUString sFrameName = impl_filterNames( sName );

    LOG_ASSERT( sal_False, "TaskCreator::createNewBrowserTask()\nNot supported yet! Return empty reference.\n" )

    // Return result of operation.
    return xPlugInFrame;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
OUString TaskCreator::impl_filterNames( const OUString& sName )
{
    // Filter special names which can't be a valid frame name!
    // Attention: "_beamer" is a valid name - because:
    //  It exist one beamer for one task tree only.
    //  If he exist we can find it - otherwhise he will be created by our task-frame!
    OUString sReturn = sName;
    if  (
            ( sName == SPECIALTARGET_BLANK  )   ||
            ( sName == SPECIALTARGET_SELF   )   ||
            ( sName == SPECIALTARGET_PARENT )   ||
            ( sName == SPECIALTARGET_TOP    )
        )
    {
        sReturn = OUString();
    }
    return sReturn;
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_False". (else sal_True) This mechanism is used to throw an ASSERT!

    ATTENTION

        If you miss a test for one of this parameters, contact the autor or add it himself !(?)
        But ... look for right testing! See using of this methods!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
// We look for invalid pointer only. An empty name is allowd!
sal_Bool TaskCreator::impldbg_checkParameter_createNewSystemTask( const OUString& sName )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &sName == NULL )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
// We look for invalid pointer only. An empty name is allowd!
sal_Bool TaskCreator::impldbg_checkParameter_createNewBrowserTask( const OUString& sName )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &sName == NULL )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  // #ifdef ENABLE_ASSERTIONS

}   //  namespace framework
