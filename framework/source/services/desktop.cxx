/*************************************************************************
 *
 *  $RCSfile: desktop.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2000-11-23 14:52:10 $
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

#ifndef __FRAMEWORK_DESKTOP_HXX_
#include <baeh_services/desktop.hxx>
#endif

#ifndef __FRAMEWORK_FRAME_HXX_
#include <services/frame.hxx>
#endif

#ifndef __FRAMEWORK_OTASKSACCESS_HXX_
#include <helper/otasksaccess.hxx>
#endif

#ifndef __FRAMEWORK_OCOMPONENTACCESS_HXX_
#include <helper/ocomponentaccess.hxx>
#endif

#ifndef __FRAMEWORK_ODESKTOPDISPATCHER_HXX_
#include <helper/odesktopdispatcher.hxx>
#endif

#ifndef __FRAMEWORK_DEFINES_HXX_
#include <defines.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
#include <classes/targetfinder.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWDESCRIPTOR_HPP_
#include <com/sun/star/awt/WindowDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_PROPTYPEHLP_HXX
#include <cppuhelper/proptypehlp.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

using namespace ::com::sun::star::awt                       ;
using namespace ::com::sun::star::beans                     ;
using namespace ::com::sun::star::container                 ;
using namespace ::com::sun::star::frame                     ;
using namespace ::com::sun::star::io                        ;
using namespace ::com::sun::star::lang                      ;
using namespace ::com::sun::star::mozilla                   ;
using namespace ::com::sun::star::task                      ;
using namespace ::com::sun::star::uno                       ;
using namespace ::com::sun::star::util                      ;
using namespace ::cppu                                      ;
using namespace ::osl                                       ;
using namespace ::rtl                                       ;

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

// names of properties
#define PROPERTYNAME_ACTIVECOMPONENT            DECLARE_ASCII("ActiveComponent" )
#define PROPERTYNAME_ACTIVEFRAME                DECLARE_ASCII("ActiveFrame"     )
#define PROPERTYNAME_HASBEAMER                  DECLARE_ASCII("HasBeamer"       )
#define PROPERTYNAME_HASCOMMONTASKBAR           DECLARE_ASCII("HasCommonTaskBar")
#define PROPERTYNAME_HASDESIGNER                DECLARE_ASCII("HasDesigner"     )
#define PROPERTYNAME_HASEXPLORER                DECLARE_ASCII("HasExplorer"     )
#define PROPERTYNAME_HASFUNCTIONBAR             DECLARE_ASCII("HasFunctionBar"  )
#define PROPERTYNAME_HASMACROBAR                DECLARE_ASCII("HasMacroBar"     )
#define PROPERTYNAME_HASNAVIGATOR               DECLARE_ASCII("HasNavigator"    )
#define PROPERTYNAME_HASOBJECTBAR               DECLARE_ASCII("HasObjectBar"    )
#define PROPERTYNAME_HASOPTIONBAR               DECLARE_ASCII("HasOptionBar"    )
#define PROPERTYNAME_HASSTATUSBAR               DECLARE_ASCII("HasStatusBar"    )
#define PROPERTYNAME_HASTOOLBAR                 DECLARE_ASCII("HasToolBar"      )
#define PROPERTYNAME_ISOLOCALE                  DECLARE_ASCII("ISOLocale"       )
#define PROPERTYNAME_ISPLUGGED                  DECLARE_ASCII("IsPlugged"       )

// handle of properties
#define PROPERTYHANDLE_ACTIVECOMPONENT          1
#define PROPERTYHANDLE_ACTIVEFRAME              2
#define PROPERTYHANDLE_HASBEAMER                3
#define PROPERTYHANDLE_HASCOMMONTASKBAR         4
#define PROPERTYHANDLE_HASDESIGNER              5
#define PROPERTYHANDLE_HASEXPLORER              6
#define PROPERTYHANDLE_HASFUNCTIONBAR           7
#define PROPERTYHANDLE_HASMACROBAR              8
#define PROPERTYHANDLE_HASNAVIGATOR             9
#define PROPERTYHANDLE_HASOBJECTBAR             10
#define PROPERTYHANDLE_HASOPTIONBAR             11
#define PROPERTYHANDLE_HASSTATUSBAR             12
#define PROPERTYHANDLE_HASTOOLBAR               13
#define PROPERTYHANDLE_ISOLOCALE                14
#define PROPERTYHANDLE_ISPLUGGED                15

// count of ALL properties
#define PROPERTYCOUNT                           15

// Defines default name for desktop
#define DEFAULT_NAME                            DECLARE_ASCII("Desktop")
#define DEFAULT_LOADSTATE                       UNKNOWN

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
Desktop::Desktop( const Reference< XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        //  Attention:
        //      Don't change order of initialization!
        //      OMutexMember is a struct with a mutex as member. We can't use a mutex as member, while
        //      we must garant right initialization and a valid value of this! First initialize
        //      baseclasses and then members. And we need the mutex for other baseclasses !!!
        :   OMutexMember            (                                                                                       )
        ,   OBroadcastHelper        ( ((OMutexMember*)this)->m_aMutex                                                       )
        ,   OPropertySetHelper      ( *SAL_STATIC_CAST( OBroadcastHelper *, this )  )
        ,   OWeakObject             (                                                                                       )
        // Init member
        ,   m_xFactory              ( xFactory                                                                              )
        ,   m_aChildTaskContainer   (                                                                                       )
        ,   m_aListenerContainer    ( m_aMutex                                                                              )
        ,   m_sName                 ( DEFAULT_NAME                                                                          )
        ,   m_eLoadState            ( DEFAULT_LOADSTATE                                                                     )
        ,   m_aTaskCreator          ( xFactory                                                                              )
        // Init Properties
        ,   m_bHasBeamer            ( sal_True                                                                              )
        ,   m_bHasCommonTaskBar     ( sal_True                                                                              )
        ,   m_bHasDesigner          ( sal_True                                                                              )
        ,   m_bHasExplorer          ( sal_True                                                                              )
        ,   m_bHasFunctionBar       ( sal_True                                                                              )
        ,   m_bHasMacroBar          ( sal_True                                                                              )
        ,   m_bHasNavigator         ( sal_True                                                                              )
        ,   m_bHasObjectBar         ( sal_True                                                                              )
        ,   m_bHasOptionBar         ( sal_True                                                                              )
        ,   m_bHasStatusBar         ( sal_True                                                                              )
        ,   m_bHasToolbar           ( sal_True                                                                              )
        ,   m_aISOLocale            (                                                                                       )
        #ifdef ENABLE_ASSERTIONS
        ,   m_bIsTerminated         ( sal_False )   // see dispose() for further informations!
        #endif
{
    // Safe impossible cases
    // We don't accept all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_DesktopCtor( xFactory ), "Desktop::Desktop()\nInvalid parameter detected!\n")

    // We cant create the frameshelper, because he hold a wekreference to us!
    // But with a HACK (++refcount) its "OK" :-(
    ++m_refCount ;

    // Initialize a new frameshelper-object to handle indexaccess and elementaccess!
    // Attention: OFrames need the this-pointer for initializing. You must use "this" directly.
    // If you define an extra variable to do that (like: Reference< XFrame > xTHIS( ... )) and
    // forget to clear this reference BEFORE "--m_refCount" (!), your refcount will be less then 0
    // and the new Desktop-instance will be destroyed instantly!!!...
    OFrames* pFramesHelper = new OFrames( m_xFactory, m_aMutex, this, &m_aChildTaskContainer );
    m_xFramesHelper = Reference< XFrames >( (OWeakObject*)pFramesHelper, UNO_QUERY );

    // Safe impossible cases
    // We can't work without this helper!
    LOG_ASSERT( !(m_xFramesHelper.is()==sal_False), "Desktop::Desktop()\nFramesHelper is not valid. XFrames, XIndexAccess and XElementAcces are not supported!\n")

    // Create a new helper to dispatch "_blank" in a correctly way.
    ODesktopDispatcher* pDispatchHelper = new ODesktopDispatcher( m_xFactory, this, m_aMutex );
    m_xDispatchHelper = Reference< XDispatch >( (OWeakObject*)pDispatchHelper, UNO_QUERY );
    LOG_ASSERT( !(m_xDispatchHelper.is()==sal_False), "Desktop::Desktop()\nDispatchHelper is not valid. XDispatch will not work correctly for \"_blank\"!\n")

    // I'am the desktop - and use my frame container in a special mode.
    // If last child task is removed I must die!
    // My container should terminate me asynchronous by using a timer.
    // Enable timer at container.
    // (The timer will be initialized with right timeout value automaticly by himself. see class AsyncQuit for further informations!)
    m_aChildTaskContainer.enableQuitTimer( this );

    // Don't forget these - or we live for ever!
    --m_refCount ;
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
Desktop::~Desktop()
{
    // Reset instance, free memory ....
    impl_resetObject();
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//
//  Attention:
//  If any testmode is set (different from TEST_NOTHING!) the special debug interface XDebugging is automaticly
//  used in queryInterface() and getTypes()!
//  (see PRIVATE_DEFINE_TYPE_1 and PRIVATE_DEFINE_INTERFACE_1 in privatemacors.hxx for further informations)
//*****************************************************************************************************************
DEFINE_XINTERFACE_15                    (   Desktop                                     ,
                                            OWeakObject                                 ,
                                            DIRECT_INTERFACE( XTypeProvider             ),
                                            DIRECT_INTERFACE( XServiceInfo              ),
                                            DIRECT_INTERFACE( XDesktop                  ),
                                            DIRECT_INTERFACE( XComponentLoader          ),
                                            DIRECT_INTERFACE( XTasksSupplier            ),
                                            DIRECT_INTERFACE( XDispatchProvider         ),
                                            DIRECT_INTERFACE( XFramesSupplier           ),
                                            DIRECT_INTERFACE( XStatusIndicatorFactory   ),
                                            DIRECT_INTERFACE( XFrame                    ),
                                            DIRECT_INTERFACE( XPropertySet              ),
                                            DIRECT_INTERFACE( XFastPropertySet          ),
                                            DIRECT_INTERFACE( XMultiPropertySet         ),
                                            DIRECT_INTERFACE( XComponent                ),
                                            DIRECT_INTERFACE( XStatusListener           ),
                                            DIRECT_INTERFACE( XEventListener            )
                                        )

DEFINE_XTYPEPROVIDER_15                 (   Desktop                 ,
                                            XTypeProvider           ,
                                            XServiceInfo            ,
                                            XDesktop                ,
                                            XComponentLoader        ,
                                            XTasksSupplier          ,
                                            XDispatchProvider       ,
                                            XFramesSupplier         ,
                                            XStatusIndicatorFactory ,
                                            XFrame                  ,
                                            XPropertySet            ,
                                            XFastPropertySet        ,
                                            XMultiPropertySet       ,
                                            XComponent              ,
                                            XStatusListener         ,
                                            XEventListener
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   Desktop                     ,
                                            SERVICENAME_DESKTOP         ,
                                            IMPLEMENTATIONNAME_DESKTOP
                                        )

// Attention: If you have enabled any testmode different from TEST_NOTHING => you have declared XDebugging-interface automaticly!
// Bhere is no macro to define and implement the right methods. You are the only one, who know - how you will use this mechanism.
// It exist a macro to switch on or off your implementation only.
// ENABLE_SERVICEDEBUG must be defined, to activate your code!
/*
#ifdef ENABLE_SERVICEDEBUG

    // In these implementation we use "dumpVariable()" to activate some special debug-operations.
    OUString SAL_CALL Desktop::dumpVariable(    const   OUString&   sVariableName   ,
                                                        sal_Int16   nCallStackPos   ) throw( RuntimeException )
    {
        // Set default return value.
        OUString sReturn;

        // We have a private impldbg-method to print out the current content of ouer frame hierarchy - the tree.
        // This mechanism is started here. We collect the names of all frames and write the informations to a logfile.
        if ( sVariableName.equals( DUMPVARIABLE_TREEINFO ) == sal_True )
        {
            sReturn = impldbg_getTreeNames();
        }
        else
        // Safe impossible cases
        // This method and his interface is used in a special context.
        // If somebody don't know this and these implementation is invalid (zB. interface exported in release version!?)
        // => we have problems!
        {
            LOG_ASSERT( sal_True, "Desktop::dumpVariable()\nERROR\nSpecial debug mode of service is used in another context!Please contact programmer.\n\n" )
        }

        // Return result of DEBUG.
        return sReturn;
    }

    // Follow methods are unused in these implementation!
    sal_Int32 SAL_CALL Desktop::setBreakPoint( const OUString& sModuleName, sal_Int32 nSourceCodeLine, sal_Bool bOn ) throw( RuntimeException ) { return 0; }
    void SAL_CALL Desktop::clearAllBreakPoints( const OUString& sModuleName ) throw( RuntimeException ) {}
    OUString SAL_CALL Desktop::eval( const OUString& sSourceCode, sal_Int16 nCallStackPos ) throw( RuntimeException ) { return OUString(); }
    Sequence< OUString > SAL_CALL Desktop::getStackTrace() throw( RuntimeException ) { return Sequence< OUString >(); }
    CONTEXTINFORMATION SAL_CALL Desktop::getContextInformation( sal_Int16 nCallStackPos) throw( RuntimeException ) { return CONTEXTINFORMATION(); }
    void SAL_CALL Desktop::setVariable( const OUString& sVariableName, const OUSTRING& aValue, sal_Int16 nCallStackPos ) throw( RuntimeException ) {}
    sal_Bool SAL_CALL Desktop::isVariable( const OUString& sVariableName, sal_Int16 nCallStackPos ) throw( RuntimeException ) { return sal_False; }
    void SAL_CALL Desktop::stop() throw( RuntimeException ) {}
    void SAL_CALL Desktop::stepOver() throw( RuntimeException ) {}
    void SAL_CALL Desktop::stepIn() throw( RuntimeException ) {}
    void SAL_CALL Desktop::stepOut() throw( RuntimeException ) {}
    void SAL_CALL Desktop::doContinue() throw( RuntimeException ) {}


#endif // #ifdef ENABLE_SERVICEDEBUG
*/
//*****************************************************************************************************************
//  XDesktop
//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::terminate() throw( RuntimeException )
{
    // Set default return value, if method failed.
    // Default is "YES, YOU CAN TERMINATE ME!"
    // If some of childs will not terminate, we reset this value to "NO".
    sal_Bool bReturn = sal_True;
    Reference < XDesktop > xThis( this );

    // block for locked mutex
    {
        LOCK_MUTEX( aGuard, m_aMutex, "Desktop::terminate()" )

        // We don't need the quit timer any more!
        // We stand in terminate ... and these timer call terminate at us ... WE MUST DISABLE IT!
        // He hold a reference (no weak reference!) to us.
        // If we forget to disable it, nobody can't release us ...
        m_aChildTaskContainer.disableQuitTimer();

        // Step over all child tasks and ask they "CAN I TERMINATE YOU AND ME?"
        Sequence< Reference< XFrame > > seqTasks = m_aChildTaskContainer.getAllElements();
        sal_Int32 nCount = seqTasks.getLength();
        for( sal_Int32 nPosition=0; nPosition<nCount; ++nPosition )
        {
            // Get an element from container and cast it to task.
            // IT MUST BE A TASK! Childs of desktop everytime tasks. No pure frames accepted!
            // It can be a plugin too, but a plugin is derived from a task ...!
            Reference< XFrame > xFrame  = seqTasks[nPosition];
            Reference< XTask >  xTask   ( xFrame, UNO_QUERY );

            // (xFrame and xTask must be valid every time or container is not valid!)
            // Control this ...
            LOG_ASSERT( !(xFrame.is() == sal_False), "Desktop::terminate()\nInvalid element in child container detected!\n")
            LOG_ASSERT( !(xTask.is()  == sal_False), "Desktop::terminate()\nElement in container is not a task!\n"         )

            // Ask task for terminating. If anyone say "NO" ...
            if ( xTask->close() == sal_False )
            {
                // ... we must reset ouer default return value to "NO" too!
                // But we don't break this loop ... we will close all task, which accept it.
                bReturn = sal_False;
            }
        }
    }

    // If termination of tasks successful ask listener for his agreement.
    if( bReturn == sal_True )
    {
        try
        {
            // A vetoable listener can throw an TerminateVetoException.
            // Then we return false!
            impl_sendQueryTerminationEvent();
        }
        catch( TerminationVetoException& )
        {
            bReturn = sal_False;
        }
    }

    // All listener has no problem with termination!
    // Send NotifyTermination event to all and return with true.
    if( bReturn == sal_True )
    {
        #ifdef ENABLE_ASSERTIONS
            // "Protect" us against dispose before terminate calls!
            // see dispose() for further informations.
            // Follow notify will start shutdown of office and somewhere call dispose() at us ...
            // Set debug variable BEFORE notify!
            m_bIsTerminated = sal_True ;
        #endif

        impl_sendNotifyTerminationEvent();
    }

    // Return result of this question.
    return bReturn;
}

//*****************************************************************************************************************
//  XDesktop
//*****************************************************************************************************************
void SAL_CALL Desktop::addTerminateListener( const Reference< XTerminateListener >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::addTerminateListener()" )
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_addTerminateListener( xListener ), "Desktop::addTerminateListener()\nInvalid parameter detected!\n" )
    // Add terminate-listener to container.
    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XTerminateListener >*) NULL ), xListener );
}

//*****************************************************************************************************************
//  XDesktop
//*****************************************************************************************************************
void SAL_CALL Desktop::removeTerminateListener( const Reference< XTerminateListener >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::removeTerminateListener()" )
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_removeTerminateListener( xListener ), "Desktop::removeTerminateListener()\nInvalid parameter detected!\n" )
    // Remove terminate-listener from container.
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XTerminateListener >*) NULL ), xListener );
}

//*****************************************************************************************************************
//  XDesktop
//*****************************************************************************************************************
Reference< XEnumerationAccess > SAL_CALL Desktop::getComponents() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::getComponents()" )

    // We use a helper class OComponentAccess to have access on all child components.
    OComponentAccess* pAccessor = new OComponentAccess( this, m_aMutex );
    Reference< XEnumerationAccess > xAccessor( (OWeakObject*)pAccessor, UNO_QUERY );
    return xAccessor;
}

//*****************************************************************************************************************
//  XDesktop
//*****************************************************************************************************************
Reference< XComponent > SAL_CALL Desktop::getCurrentComponent() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::getCurrentComponent()" )

    // Set return value if method failed.
    Reference< XComponent > xComponent;

    // Get reference to current frame ...
    // ... get component of this frame ... (It can be the window, the model or the controller.)
    Reference< XFrame > xCurrentFrame = getCurrentFrame();
    if( xCurrentFrame.is() == sal_True )
    {
        xComponent = impl_getFrameComponent( xCurrentFrame );
    }

    // ... and return result.
    return xComponent;
}

//*****************************************************************************************************************
//  XDesktop
//*****************************************************************************************************************
Reference< XFrame > SAL_CALL Desktop::getCurrentFrame() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::getCurrentFrame()" )

    // Set default return value, if method failed.
    Reference< XFrame > xReturn = Reference< XFrame >();

    // Start search with ouer direct active frame (if it exist!).
    Reference< XFrame > xActiveFrame = getActiveFrame();
    if ( xActiveFrame.is() == sal_True )
    {
        // The current frame is the most active frame.
        // Search from there other active frames in hierarchy.
        Reference< XFramesSupplier > xLast = Reference< XFramesSupplier >( xActiveFrame, UNO_QUERY );
        Reference< XFramesSupplier > xNext = Reference< XFramesSupplier >( xLast->getActiveFrame(), UNO_QUERY );

        // If a other frame in hierarchy is active to ...
        while ( xNext.is() == sal_True )
        {
            // ... safe this new one as current and try to get a other active frame.
            xLast   = xNext;
            xNext   = Reference< XFramesSupplier >( xNext->getActiveFrame(), UNO_QUERY );
        }

        // Cast current active frame to XFrame.
        xReturn = Reference< XFrame >( xLast, UNO_QUERY );
    }

    // Return result of search.
    return xReturn;
}

//*****************************************************************************************************************
//  XComponentLoader
//*****************************************************************************************************************
Reference< XComponent > SAL_CALL Desktop::loadComponentFromURL( const   OUString&                   sURL            ,
                                                                const   OUString&                   sTargetFrameName,
                                                                        sal_Int32                   nSearchFlags    ,
                                                                const   Sequence< PropertyValue >&  seqArguments    ) throw(    IOException             ,
                                                                                                                                IllegalArgumentException,
                                                                                                                                RuntimeException        )
{
    // Ready for multithreading
    LOCK_MUTEX( aBeforeGuard, m_aMutex, "Desktop::loadComponentFromURL() start" )
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_loadComponentFromURL( sURL, sTargetFrameName, nSearchFlags, seqArguments ), "Desktop::loadComponentFromURL()\nInvalid parameter detected!\n" )

    // Set default return value, if method failed.
    Reference< XComponent > xReturn;

    URL aURL;
    aURL.Complete = sURL;
    Reference< XDispatch > xDispatcher = queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    // Now we have found any destination frame with a valid dispatcher ... dispatch URL on it.
    if( xDispatcher.is() == sal_True )
    {
        // ... dispatch URL at this dispatcher.
        // Set us as listener for status events from this dispatcher.
        // The dispatcher get a event loadFinished() or loadCancelled() from his frameloader
        // and map this information for us! (see classes ODispatchProvider/ODesktopDispatcher for further informations)
        xDispatcher->addStatusListener( static_cast< XStatusListener* >( this ), aURL );
        // Attention: We must unlock our mutex before next call!
        // Somewhere can call us back (zB. getActiveFrame() for set/unset focus)
        // AND ...
        m_eLoadState = UNKNOWN; // Reset loader state to default, because we must yield for a valid result! See next WHILE condition.
                                // And we must do it before we call dispatch AND before we unlock our mutex!
        UNLOCK_MUTEX( aBeforeGuard, "Desktop::loadComponentFromURL() before dispatch" )
        xDispatcher->dispatch( aURL, seqArguments );
        // ... we must wait for asynchron result of this dispatch()-operation!
        while( m_eLoadState == UNKNOWN )
        {
            Application::Yield();
        }
        // Try to get new current component.
        // Look for result of loading.
        // If loading was successful we can return a component.
        // Otherwise our ODesktopDispatcher will delete these zombie task without any component automaticly!
        if( m_eLoadState == SUCCESSFUL && m_xLastFrame.is() )
        {
            xReturn = impl_getFrameComponent( m_xLastFrame );
        }
        // We have all informations about result state of loading document.
        // We can remove us as listener.
        xDispatcher->removeStatusListener( static_cast< XStatusListener* >( this ), aURL );
    }


/*
    // Search frame for given parameter.
    // Attention: Don't use queryDispatch() to find this frame and get the dispatcher (see follow operations too)!
    // We must the owner of new frames to dispose it if loading failed ... and if we use queryDispatch() we are not this user ...
    // (We will be owner of found frame, if chance for creating of a new one is very high ...
    // and this case is true if "_blank" is one of our parameters!)
    sal_Bool bIAmOwnerOfFrame = (sTargetFrameName==FRAMETYPE_BLANK);

    // We will forward all "_blank" or HIDDEN requests to one of our plugin frames if any exist!
    // Search for any plugin frame.

    // Now we must search for a valid dispatcher.
    Reference< XFrame >     xDestinationFrame;
    Reference< XDispatch >  xDispatcher;
    URL                     aURL;
    {
        // Else we must search for a normal frame as target.
        // Find normal frame of our hierarchy to dispatch this URL.
        // Get the dispatcher of this frame to load the component.
        // Use "_self" to get his own dispatcher!
        xDestinationFrame = findFrame( sTargetFrameName, nSearchFlags );
        if( xDestinationFrame.is() == sal_True )
        {
            aURL.Complete = sURL;
            Reference< XDispatchProvider > xFrameProvider( xDestinationFrame, UNO_QUERY );
            xDispatcher = xFrameProvider->queryDispatch( aURL, FRAMETYPE_SELF, 0 );
            LOG_ASSERT( !(xDispatcher.is()==sal_False), "Desktop::loadComponentFromURL()\nNormal founded frame support no dispatcher interface ...?!\n" )
        }
    }
    // Now we have found any destination frame with a valid dispatcher ... dispatch URL on it.
    if( xDispatcher.is() == sal_True )
    {
        // ... dispatch URL at this dispatcher.
        // Set us as listener for status events from this dispatcher.
        // The dispatcher get a event loadFinished() or loadCancelled() from his frameloader
        // and map this information for us! (see class ODispatchProvider for further informations)
        xDispatcher->addStatusListener( static_cast< XStatusListener* >( this ), aURL );

        // Attention: We must unlock our mutex before next call!
        // Somewhere can call us back (zB. getActiveFrame() for set/unset focus)
        // AND ...

        m_eLoadState = UNKNOWN; // Reset loader state to default, because we must yield for a valid result! See next WHILE condition.
                                // And we must do it before we call dispatch AND before we unlock our mutex!

        UNLOCK_MUTEX( aBeforeGuard, "Desktop::loadComponentFromURL() before dispatch" )

        xDispatcher->dispatch( aURL, seqArguments );

        // ... we must wait for asynchron result of this dispatch()-operation!
        while( m_eLoadState == UNKNOWN )
        {
            Application::Yield();
        }

        // Try to get new current component.
        // Look for result of loading.
        switch( m_eLoadState )
        {
            case SUCCESSFUL :   // If loading was successful we can return a component.
                                xReturn = impl_getFrameComponent( xDestinationFrame );
                                break;
            case FAILED     :   // If loading was not successful we cant return a valid component ... but ...
                                if( bIAmOwnerOfFrame == sal_True )
                                {
                                    // ... this frame was new created for this operation!
                                    // We must dispose this task(!) It must be a task!
                                    Reference< XTask > xTask( xDestinationFrame, UNO_QUERY );
                                    LOG_ASSERT( !(xTask.is()==sal_False), "Desktop::loadComponentFromURL()\nWith \"_blank\" new created frame is no task ... ?!\n" )
                                    xTask->close();
                                }
                                break;
        }

        // We have all informations about result state of loading document.
        // We can remove us as listener.
        xDispatcher->removeStatusListener( static_cast< XStatusListener* >( this ), aURL );
    }
*/
    // Return result of this operation.
    return xReturn;
}

//*****************************************************************************************************************
//  XTasksSupplier
//*****************************************************************************************************************
Reference< XEnumerationAccess > SAL_CALL Desktop::getTasks() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::getTasks()" )

    // Create a new EnumerationAccess and return it.
    // Caller will be the new owner of this and HE must destroy the returned instance!
    OTasksAccess* pTasksAccess = new OTasksAccess( this, &m_aChildTaskContainer, m_aMutex );
    Reference< XEnumerationAccess > xAccess( (OWeakObject*)pTasksAccess, UNO_QUERY );

    // Return result of operation.
    return xAccess;
}

//*****************************************************************************************************************
//  XTasksSupplier
//*****************************************************************************************************************
Reference< XTask > SAL_CALL Desktop::getActiveTask() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::getActiveTask()" )

    // The current active task can only be an active child of us.
    // And tasks allowed only as direct childs of desktop.
    return Reference< XTask >( m_aChildTaskContainer.getActive(), UNO_QUERY );
}

//*****************************************************************************************************************
//  XDispatchProvider
//*****************************************************************************************************************
Reference< XDispatch > SAL_CALL Desktop::queryDispatch( const   URL&        aURL            ,
                                                        const   OUString&   sTargetFrameName,
                                                                sal_Int32   nSearchFlags    ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::queryDispatch()" )
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_queryDispatch( aURL, sTargetFrameName, nSearchFlags ), "Desktop::queryDispatch()\nInvalid parameter detected!\n" )

    // Set default return value.
    Reference< XDispatch > xReturn;

    // Use helper to classify search direction.
    IMPL_ETargetClass eDirection = TargetFinder::classify(  this                ,
                                                            sTargetFrameName    ,
                                                            nSearchFlags        );
    // Use returned recommendation to search right frame!
    switch( eDirection )
    {
        case eCREATE    :   {
                                xReturn = m_xDispatchHelper;
                            }
                            break;

        case eDOWN      :   {
                                Reference< XDispatchProvider > xTarget( TargetFinder::helpDownSearch( m_xFramesHelper, sTargetFrameName ), UNO_QUERY );
                                if( xTarget.is() == sal_True )
                                {
                                    xReturn = xTarget->queryDispatch( aURL, SPECIALTARGET_SELF, 0 );
                                }
                            }
                            break;
    }

/* TODO
    If new implementation of findFrame/queryDispatch works correctly we can delete these code!

    //*************************************************************************************************************
    //  1)  Handle special mode "_blank"/CREATE TASK!
    //      We create a special dispatcher which create new tasks on demand if a new dispatch() is called.
    //*************************************************************************************************************

//      Praeprozessor Bug!
//      Wenn nach CREATE ein Space steht wird versucht es durch das Define CREATE aus tools/rtti.hxx zu ersetzen
//      was fehlschlaegt und die naechsten 3 Klammern ")){" unterschlaegt!
//      Dann meckert der Compiler das natuerlich an ...

    if((sTargetFrameName==FRAMETYPE_BLANK)||(nSearchFlags&FrameSearchFlag::CREATE))
    {
        xReturn = m_xDispatchHelper;
    }
    else
    //*************************************************************************************************************
    //  2)  We search for an existing frame to dispatch this URL.
    //*************************************************************************************************************
    {
         // Forbid creation of new tasks at follow calls! We have handled this before.
//      OUString sNewTargetFrameName;
//      if( sTargetFrameName != FRAMETYPE_BLANK )
//      {
//          sNewTargetFrameName = sTargetFrameName;
//      }
//      nSearchFlags |= UNMASK_CREATE   ;

        // Try to find right frame in current hierarchy to dispatch given URL to it.
        Reference< XFrame > xDispatchFrame = findFrame( sTargetFrameName, nSearchFlags );
        // Do the follow only, if we have any valid frame for dispatch!
        // Otherwise ouer return value will be empty ...
        if ( xDispatchFrame.is() == sal_True )
        {
            // Dispatch given URL on found frame.
            Reference< XDispatchProvider > xDispatchProvider( xDispatchFrame, UNO_QUERY );
            // Safe impossible cases.
            // A frame of ouer hierarchy must support XDispatchProvider interface.
            LOG_ASSERT( !(xDispatchProvider.is()==sal_False), "Desktop::queryDispatch()\nEvery frame of ouer hieararchy must support XDispatchProvider interface. But I have found a negative example ...!\n" )
            xReturn = xDispatchProvider->queryDispatch( aURL, OUString(), FrameSearchFlag::SELF );
        }
    }
*/
    // Return dispatcher for given URL.
    return xReturn;
}

//*****************************************************************************************************************
//  XDispatchProvider
//*****************************************************************************************************************
Sequence< Reference< XDispatch > > SAL_CALL Desktop::queryDispatches( const Sequence< DispatchDescriptor >& seqDescripts ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::queryDispatches()" )

    // Set default return value if no dispatcher will be found.
    Sequence< Reference< XDispatch > > seqDispatcher;

    // Get count of all given discriptors ...
    sal_Int32 nDescriptorCount = seqDescripts.getLength();
    if ( nDescriptorCount > 0 )
    {
        // ... to get enough memory for expected dispatcher.
        // We can't get more dispatcher as descriptors exist!
        Reference< XDispatch >* pDispatcher = new Reference< XDispatch >[nDescriptorCount];
        // Safe first position in "dynamic memory list" for later access!
        Reference< XDispatch >* pAnchor     = pDispatcher;
        // We must count all getted dispatcher to create return sequence.
        // There can't be more but fewer dispatcher then given descriptors!
        sal_Int32               nDispatcher = 0;
        // This is a helper variable to hold current returned dispatcher of query.
        Reference< XDispatch >  xDispatcher;

        // Step over all descriptors and try to get any dispatcher for it.
        for ( sal_Int32 nPosition=0; nPosition<nDescriptorCount; ++nPosition )
        {
            xDispatcher = queryDispatch(    seqDescripts[nPosition].FeatureURL  ,
                                            seqDescripts[nPosition].FrameName   ,
                                            seqDescripts[nPosition].SearchFlags );
            // If any dispatcher for given parameters found ...
            if ( xDispatcher.is() == sal_True )
            {
                // ... safe it temporaly in "memory list" ...
                *pDispatcher = xDispatcher;
                // .. actualize position in these list ...
                ++pDispatcher;
                // ... and count of found dispatcher.
                ++nDispatcher;
            }
        }

        // Safe impossible cases.
        // If we have more dispatcher then descriptors => we have a problem in algorithm!
        LOG_ASSERT( !(nDispatcher>nDescriptorCount), "Desktop::queryDispatch()\nAlgorithm error. There are more dispatcher as reserved fields in memory!\n" )

        // Copy "memory list" to return sequence.
        // We copy all valid dispatcher to return sequence! (nDispatcher is count of these objects!)
        seqDispatcher = Sequence< Reference< XDispatch > >( pDispatcher, nDispatcher );
    }

    // Return result of this operation.
    return seqDispatcher;
}

//*****************************************************************************************************************
//  XFramesSupplier
//*****************************************************************************************************************
Reference< XFrames > SAL_CALL Desktop::getFrames() throw( RuntimeException )
{
    // Return helper for XFrames interface to handle operations on XFrames.
    return m_xFramesHelper;
}

//*****************************************************************************************************************
//  XFramesSupplier
//*****************************************************************************************************************
void SAL_CALL Desktop::setActiveFrame( const Reference< XFrame >& xFrame ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::setActiveFrame()" )

    // We don't must control incoming parameter! We use a helperclass to safe the current active frame.
    // And these implementation do this for us!

    // Set new active task.
    // It must be an already existing child in container!

    Reference< XFrame > xActiveChild = m_aChildTaskContainer.getActive();

    // Don't work, if "new" active frame is'nt different from current one!
    if ( xActiveChild != xFrame )
    {
        // Set the new active child frame.
        m_aChildTaskContainer.setActive( xFrame );
        if  ( xActiveChild.is() )
            xActiveChild->deactivate();
    }
}

//*****************************************************************************************************************
//  XFramesSupplier
//*****************************************************************************************************************
Reference< XFrame > SAL_CALL Desktop::getActiveFrame() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::getActiveFrame()" )

    // Return current active frame.
    // Ths information exist at the container!
    return m_aChildTaskContainer.getActive();
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::initialize( const Reference< XWindow >& xWindow ) throw( RuntimeException )
{
    LOG_ASSERT( sal_False, "Desktop::initialize()\nDesktop has no window! I think so ... !?\n")
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
Reference< XWindow > SAL_CALL Desktop::getContainerWindow() throw( RuntimeException )
{
    LOG_ASSERT( sal_False, "Desktop::getContainerWindow()\nDesktop has no window! I think so ... !?\n")
    return Reference< XWindow >();
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::setCreator( const Reference< XFramesSupplier >& xCreator ) throw( RuntimeException )
{
    // The desktop is the topframe! He has no parent.
    LOG_ASSERT( sal_False, "Desktop::setCreator()\nThe desktop is the topframe! He has no parent.\n" )
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
Reference< XFramesSupplier > SAL_CALL Desktop::getCreator() throw( RuntimeException )
{
    // The desktop is the topframe! He has no parent.
    LOG_ASSERT( sal_False, "Desktop::getCreator()\nThe desktop is the topframe! He has no parent.\n" )
    return Reference< XFramesSupplier >();
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
OUString SAL_CALL Desktop::getName() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::getName()" )

    // Return name of THIS frame.
    return m_sName;
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::setName( const OUString& sName ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::setName()" )

    // Safe new name of THIS frame.
    m_sName = sName;
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
Reference< XFrame > SAL_CALL Desktop::findFrame(    const   OUString&   sTargetFrameName    ,
                                                                sal_Int32   nSearchFlags        ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::findFrame()" )
    // Safe impossible cases
    LOG_ASSERT( impldbg_checkParameter_findFrame( sTargetFrameName, nSearchFlags ), "Desktop::findFrame()\nInvalid parameter detected.\n" )

    // Set default return value if method failed.
    Reference< XFrame > xSearchedFrame;

    // Use helper to classify search direction.
    IMPL_ETargetClass eDirection = TargetFinder::classify(  this            ,
                                                            sTargetFrameName,
                                                            nSearchFlags    );
    // Use returned recommendation to search right frame or create a new one!
    switch( eDirection )
    {
        case eDOWN      :   {
                                xSearchedFrame = TargetFinder::helpDownSearch( m_xFramesHelper, sTargetFrameName );
                            }
                            break;

        case eCREATE    :   {
                                OUString sFrameName = sTargetFrameName;
                                if( sFrameName == SPECIALTARGET_BLANK )
                                {
                                    sFrameName = OUString();
                                }
                                xSearchedFrame = m_aTaskCreator.createNewSystemTask( sFrameName );
                            }
                            break;
    }
    // return result of operation.
    return xSearchedFrame;
}
/* TODO
    If new implementation of findFrame/queryDispatch works correctly we can delete these code!

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
Reference< XFrame > SAL_CALL Desktop::findFrame(    const   OUString&   sTargetFrameName    ,
                                                                sal_Int32   nSearchFlags        ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::findFrame()" )
    // Safe impossible cases
    LOG_ASSERT( impldbg_checkParameter_findFrame( sTargetFrameName, nSearchFlags ), "Desktop::findFrame()\nInvalid parameter detected.\n" )
    // Log some special informations about search. (Active in debug version only, if special mode is set!)
    LOG_PARAMETER_FINDFRAME( "Desktop", m_sName, sTargetFrameName, nSearchFlags )

    // Set default return Value, if method failed
    Reference< XFrame > xReturn = Reference< XFrame >();

    //*************************************************************************************************************
    //  1)  If "_blank" searched we must create a new task!
    //  Attention:
    //      Don't set a name at new created frame! sTargetFrameName is not a normal name yet.
    //      "_blank" is not allowed as frame name.
    //      The helper method will create the new task, initialize it with an empty window and append it on
    //      ouer frame-hierarchy.
    //*************************************************************************************************************
    if( sTargetFrameName == FRAMETYPE_BLANK )
    {
        LOG_TARGETINGSTEP( "Desktop", m_sName, "react to \"_blank\"" )
        //xReturn = impl_createNewTask( OUString() );
        xReturn = m_aTaskCreator.createNewSystemTask( OUString() );
    }
    else
    //*************************************************************************************************************
    //  ATTENTION!
    //  We have searched for special targets only ... but now we must search for any named frames and use search
    //  flags to do that!
    //*************************************************************************************************************
    {
        //*********************************************************************************************************
        //  At first we must filter all other special target names!
        //  You can disable this statement if all these cases are handled before ...
        //*********************************************************************************************************
        if  (
                ( sTargetFrameName              !=  FRAMETYPE_SELF  )   &&
                ( sTargetFrameName              !=  FRAMETYPE_PARENT)   &&
                ( sTargetFrameName              !=  FRAMETYPE_TOP   )   &&
                ( sTargetFrameName.getLength()  >   0               )
            )
        {
            //*****************************************************************************************************
            //  2)  Search for TASKS.
            //*****************************************************************************************************
            if  (
                    ( nSearchFlags                          &   FrameSearchFlag::TASKS  )   &&
                    ( m_aChildTaskContainer.hasElements()   ==  sal_True                )
                )
            {
                LOG_TARGETINGSTEP( "Desktop", m_sName, "react to TASKS" )
                // Step over all direct childtasks and search in it.
                // Lock container for exclusiv access.
                // The container is not threadsafe and shared with some helper classes.
                // It must be! But don't forget to unlock it.
                m_aChildTaskContainer.lock();

                sal_uInt32  nCount      = m_aChildTaskContainer.getCount();
                sal_uInt32  nPosition   = 0;
                while   (
                            ( nPosition     <   nCount      )   &&
                            ( xReturn.is()  ==  sal_False   )
                        )
                {
                    // Get next child and compare with searched name.
                    // But allow task to search at himself only!
                    xReturn = m_aChildTaskContainer[nPosition]->findFrame( sTargetFrameName, FrameSearchFlag::SELF );
                    ++nPosition;
                }
                // Unlock the container.
                m_aChildTaskContainer.unlock();
            }
            //*************************************************************************************************************
            //  3)  Search for CHILDREN
            //  Attention:
            //      We search for ouer childs and his subtrees. That is the reason for using of SELF and CHILDREN as searchflags.
            //      Never use SIBLINGS for searching. We step over ouer own container. Search for brothers at ouer direct
            //      childs will do the same and it can be a problem ... RECURSIVE SEARCH ...!
            //*************************************************************************************************************
            if  (
                    ( xReturn.is()                          ==  sal_False                   )   &&
                    ( nSearchFlags                          &   FrameSearchFlag::CHILDREN   )   &&
                    ( m_aChildTaskContainer.hasElements()   ==  sal_True                    )
                )
            {
                LOG_TARGETINGSTEP( "Desktop", m_sName, "react to CHILDREN" )
                // Lock container for exclusiv access.
                // The container is not threadsafe and shared with some helper classes.
                // It must be! But don't forget to unlock it.
                m_aChildTaskContainer.lock();

                sal_uInt32  nCount      = m_aChildTaskContainer.getCount();
                sal_uInt32  nPosition   = 0;
                while   (
                            ( nPosition     <   nCount      )   &&
                            ( xReturn.is()  ==  sal_False   )
                        )
                {
                    // Get next child and search on it for subframes with searched name.
                       xReturn = m_aChildTaskContainer[nPosition]->findFrame( sTargetFrameName, FrameSearchFlag::SELF | FrameSearchFlag::CHILDREN );
                    ++nPosition;
                }
                m_aChildTaskContainer.unlock();
            }
            //*************************************************************************************************************
            //  4)  If we have the license to create a new task then we do it.
            //      Create a new task, initialize it with an empty window, set default parameters and append it on desktop!
            //*************************************************************************************************************
//              Praeprozessor Bug!
//              Wenn nach CREATE ein Space steht wird versucht es durch das Define CREATE aus tools/rtti.hxx zu ersetzen
//              was fehlschlaegt und die naechsten 3 Klammern ")){" unterschlaegt!
//              Dann meckert der Compiler das natuerlich an ...
            if((xReturn.is()==sal_False)&&(nSearchFlags&FrameSearchFlag::CREATE))
            {
                LOG_TARGETINGSTEP( "Desktop", m_sName, "react to CREATE" )
                //xReturn = impl_createNewTask( sTargetFrameName );
                xReturn = m_aTaskCreator.createNewSystemTask( sTargetFrameName );
            }
        }
    }

    // Log some special informations about search. (Active in debug version only, if special mode is set!)
    LOG_RESULT_FINDFRAME( "Desktop", m_sName, xReturn )
    // Return with result of operation.
    return xReturn;
}
*/
//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::isTop() throw( RuntimeException )
{
    // A desktop is the topframe everytime.
    return sal_True;
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::activate() throw( RuntimeException )
{
    // Desktop is active everytime! Do nothing.
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::deactivate() throw( RuntimeException )
{
    // Desktop is active everytime! Do nothing.
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::isActive() throw( RuntimeException )
{
    // Desktop is active everytime!
    return sal_True;
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::setComponent(    const   Reference< XWindow >&   xComponentWindow,
                                              const Reference< XController >&   xController     ) throw( RuntimeException )
{
    LOG_ASSERT( sal_False, "Desktop::setComponent()\nNo component allowed on desktop. We are the root of frame hierarchy only!\n")
    return sal_False;
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
Reference< XWindow > SAL_CALL Desktop::getComponentWindow() throw( RuntimeException )
{
    LOG_ASSERT( sal_False, "Desktop::getComponentWindow()\nNo component allowed on desktop. We are the root of frame hierarchy only!\n")
    return Reference< XWindow >();
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
Reference< XController > SAL_CALL Desktop::getController() throw( RuntimeException )
{
    LOG_ASSERT( sal_False, "Desktop::getController()\nNo controller allowed on desktop. We are the root of frame hierarchy only!\n")
    return Reference< XController >();
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::contextChanged() throw( RuntimeException )
{
    LOG_ASSERT( sal_False, "Desktop::contextChanged()\nNot implemented yet!\n")
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::addFrameActionListener( const Reference< XFrameActionListener >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::addFrameActionListener()" )

    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_addFrameActionListener( xListener ), "Desktop::addFrameActionListener()\nInvalid parameter detected!\n" )

    // Add frameaction-listener to container.
    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XFrameActionListener >*) NULL ), xListener );
}

//*****************************************************************************************************************
//   XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::removeFrameActionListener( const Reference< XFrameActionListener >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::removeFrameActionListener()" )

    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_removeFrameActionListener( xListener ), "Desktop::removeFrameActionListener()\nInvalid parameter detected!\n" )

    // Remove frameaction-listener from container.
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XFrameActionListener >*) NULL ), xListener );
}

//*****************************************************************************************************************
//   XComponent
//*****************************************************************************************************************
void SAL_CALL Desktop::dispose() throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::dispose()" )

    // Safe impossible cases
    // It's an programming error if dispose is called before terminate!
    LOG_ASSERT( !(m_bIsTerminated==sal_False), "Desktop::dispose()\nIt's not allowed to dispose the desktop before terminate() is called!\n" )

    // Release all used references, delete all listener
    // and free our child task container.

    /* Attention 1:
            We must disable the special quit timer of our frame container.
            Because it will call terminate at THESE instance (last task is removed!) ...
    */
    m_aChildTaskContainer.disableQuitTimer();

    /* Attention 2:
            We must send the dispose message to all listener BEFORE we clear our container.
            Otherwise our child tasks are disposed and try to remove it by himself at our container ...
            Next call remove all frames which components are dispose listener at these desktop;
             and then we clear all other frames from the container which are not listener!
    */
    EventObject aDisposeEvent( (OWeakObject*)this );
    m_aListenerContainer.disposeAndClear( aDisposeEvent );

    m_aChildTaskContainer.clear();

    // Release some other references.
    m_xFactory          = Reference< XMultiServiceFactory >();
    m_xFramesHelper     = Reference< XFrames >();
    m_xDispatchHelper   = Reference< XDispatch >();
}

//*****************************************************************************************************************
//   XComponent
//*****************************************************************************************************************
void SAL_CALL Desktop::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::addEventListener()" )

    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_addEventListener( xListener ), "Desktop::addEventListener()\nInvalid parameter detected!\n" )

    // Add listener to container.
    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >*) NULL ), xListener );
}

//*****************************************************************************************************************
//   XComponent
//*****************************************************************************************************************
void SAL_CALL Desktop::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::removeEventListener()" )
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_removeEventListener( xListener ), "Desktop::removeEventListener()\nInvalid parameter detected!\n" )

    // Remove listener to container.
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >*) NULL ), xListener );
}

//*****************************************************************************************************************
//   XStatusIndicatorFactory
//*****************************************************************************************************************
Reference< XStatusIndicator > SAL_CALL Desktop::createStatusIndicator() throw( RuntimeException )
{
    LOG_ASSERT( sal_False, "Desktop::createStatusIndicator()\nNot implemented yet!\n")
    return Reference< XStatusIndicator >();
}
//*****************************************************************************************************************
//   XStatusListener
//*****************************************************************************************************************
void SAL_CALL Desktop::statusChanged( const FeatureStateEvent& aEvent ) throw( RuntimeException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::statusChanged()" )
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT( impldbg_checkParameter_statusChanged( aEvent ), "Desktop::statusChanged()\nInvalid parameter detected!\n" )

    // Is message for me?
    if( aEvent.FeatureDescriptor == FEATUREDESCRIPTOR_LOADSTATE )
    {
        // Map state of event to internal notation.
        if( aEvent.IsEnabled == sal_True )
        {
            m_eLoadState = SUCCESSFUL;
            aEvent.State >>= m_xLastFrame;
        }
        else
        if( aEvent.IsEnabled == sal_False )
        {
            m_eLoadState = FAILED;
        }
    }
}

//*****************************************************************************************************************
//   XEventListener
//*****************************************************************************************************************
void SAL_CALL Desktop::disposing( const EventObject& aSource ) throw( RuntimeException )
{
    LOG_ASSERT( sal_False, "Desktop::disposing()\nNot implemented yet!\n" )
}

//*****************************************************************************************************************
//   protected method
//*****************************************************************************************************************
void Desktop::impl_resetObject()
{
}

//*****************************************************************************************************************
//  OPropertySetHelper
//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::convertFastPropertyValue(            Any&        aConvertedValue ,
                                                                  Any&      aOldValue       ,
                                                                sal_Int32   nHandle         ,
                                                        const   Any&        aValue          ) throw( IllegalArgumentException )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::convertFastPropertyValue()" )

    //  Check, if value of property will changed in method "setFastPropertyValue_NoBroadcast()".
    //  Return TRUE, if changed - else return FALSE.
    //  Attention:
    //      Method "impl_tryToChangeProperty()" can throw the IllegalArgumentException !!!

    //  Initialize state with FALSE !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch ( nHandle )
    {
        case PROPERTYHANDLE_ACTIVECOMPONENT     :
        case PROPERTYHANDLE_ACTIVEFRAME         :
        case PROPERTYHANDLE_ISPLUGGED           :
        case PROPERTYHANDLE_ISOLOCALE           :   bReturn = sal_False; // These variables are readonly(!) and can't be changed.
                                                    break;
        case PROPERTYHANDLE_HASBEAMER           :   bReturn = impl_tryToChangeProperty( m_bHasBeamer        , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASCOMMONTASKBAR    :   bReturn = impl_tryToChangeProperty( m_bHasCommonTaskBar , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASDESIGNER         :   bReturn = impl_tryToChangeProperty( m_bHasDesigner      , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASEXPLORER         :   bReturn = impl_tryToChangeProperty( m_bHasExplorer      , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASFUNCTIONBAR      :   bReturn = impl_tryToChangeProperty( m_bHasFunctionBar   , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASMACROBAR         :   bReturn = impl_tryToChangeProperty( m_bHasMacroBar      , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASNAVIGATOR        :   bReturn = impl_tryToChangeProperty( m_bHasNavigator     , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASOBJECTBAR        :   bReturn = impl_tryToChangeProperty( m_bHasObjectBar     , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASOPTIONBAR        :   bReturn = impl_tryToChangeProperty( m_bHasOptionBar     , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASSTATUSBAR        :   bReturn = impl_tryToChangeProperty( m_bHasStatusBar     , aValue, aOldValue, aConvertedValue );
                                                    break;
        case PROPERTYHANDLE_HASTOOLBAR          :   bReturn = impl_tryToChangeProperty( m_bHasToolbar       , aValue, aOldValue, aConvertedValue );
                                                    break;
    }

    // Return state of operation.
    return bReturn ;
}

//*****************************************************************************************************************
//  OPropertySetHelper
//*****************************************************************************************************************
void SAL_CALL Desktop::setFastPropertyValue_NoBroadcast(            sal_Int32   nHandle ,
                                                              const Any&        aValue  ) throw( Exception )
{
    // Ready for multithreading
    LOCK_MUTEX( aGuard, m_aMutex, "Desktop::setFastPropertyValue_NoBroadCast()" )

    // Search for right handle ... and try to set property value.
    switch ( nHandle )
    {
        case PROPERTYHANDLE_ACTIVECOMPONENT     :
        case PROPERTYHANDLE_ACTIVEFRAME         :
        case PROPERTYHANDLE_ISPLUGGED           :
        case PROPERTYHANDLE_ISOLOCALE           :   LOG_ASSERT( sal_False, "Desktop::setFastPropertyValue_NoBroadcast()\nSet of readonly property not allowed.\n" )
                                                    break;
        case PROPERTYHANDLE_HASBEAMER           :   aValue >>= m_bHasBeamer         ;
                                                    break;
        case PROPERTYHANDLE_HASCOMMONTASKBAR    :   aValue >>= m_bHasCommonTaskBar  ;
                                                    break;
        case PROPERTYHANDLE_HASDESIGNER         :   aValue >>= m_bHasDesigner       ;
                                                    break;
        case PROPERTYHANDLE_HASEXPLORER         :   aValue >>= m_bHasExplorer       ;
                                                    break;
        case PROPERTYHANDLE_HASFUNCTIONBAR      :   aValue >>= m_bHasFunctionBar    ;
                                                    break;
        case PROPERTYHANDLE_HASMACROBAR         :   aValue >>= m_bHasMacroBar       ;
                                                    break;
        case PROPERTYHANDLE_HASNAVIGATOR        :   aValue >>= m_bHasNavigator      ;
                                                    break;
        case PROPERTYHANDLE_HASOBJECTBAR        :   aValue >>= m_bHasObjectBar      ;
                                                    break;
        case PROPERTYHANDLE_HASOPTIONBAR        :   aValue >>= m_bHasOptionBar      ;
                                                    break;
        case PROPERTYHANDLE_HASSTATUSBAR        :   aValue >>= m_bHasStatusBar      ;
                                                    break;
        case PROPERTYHANDLE_HASTOOLBAR          :   aValue >>= m_bHasToolbar        ;
                                                    break;
    }
}

//*****************************************************************************************************************
//  OPropertySetHelper
//*****************************************************************************************************************
void SAL_CALL Desktop::getFastPropertyValue(    Any&        aValue  ,
                                                  sal_Int32 nHandle ) const
{
    // Ready for multithreading
    LOCK_GLOBALMUTEX( aGuard, "Desktop::getFastPropertyValue()" )

    // Search for right handle ... and try to set property value.
    switch ( nHandle )
    {
        case PROPERTYHANDLE_ACTIVECOMPONENT     :   //aValue <<= getCurrentComponent();
                                                    aValue <<= Reference< XComponent >();
                                                    break;
        case PROPERTYHANDLE_ACTIVEFRAME         :   aValue <<= m_aChildTaskContainer.getActive();
                                                    break;
        case PROPERTYHANDLE_HASBEAMER           :   aValue <<= m_bHasBeamer         ;
                                                    break;
        case PROPERTYHANDLE_HASCOMMONTASKBAR    :   aValue <<= m_bHasCommonTaskBar  ;
                                                    break;
        case PROPERTYHANDLE_HASDESIGNER         :   aValue <<= m_bHasDesigner       ;
                                                    break;
        case PROPERTYHANDLE_HASEXPLORER         :   aValue <<= m_bHasExplorer       ;
                                                    break;
        case PROPERTYHANDLE_HASFUNCTIONBAR      :   aValue <<= m_bHasFunctionBar    ;
                                                    break;
        case PROPERTYHANDLE_HASMACROBAR         :   aValue <<= m_bHasMacroBar       ;
                                                    break;
        case PROPERTYHANDLE_HASNAVIGATOR        :   aValue <<= m_bHasNavigator      ;
                                                    break;
        case PROPERTYHANDLE_HASOBJECTBAR        :   aValue <<= m_bHasObjectBar      ;
                                                    break;
        case PROPERTYHANDLE_HASOPTIONBAR        :   aValue <<= m_bHasOptionBar      ;
                                                    break;
        case PROPERTYHANDLE_HASSTATUSBAR        :   aValue <<= m_bHasStatusBar      ;
                                                    break;
        case PROPERTYHANDLE_HASTOOLBAR          :   aValue <<= m_bHasToolbar        ;
                                                    break;
        case PROPERTYHANDLE_ISOLOCALE           :   aValue <<= m_aISOLocale         ;
                                                    break;
        case PROPERTYHANDLE_ISPLUGGED           :   {
//TODO!                                                 sal_Bool bState = impl_checkPlugInState();
//We can't ask a non const method from these one!       aValue <<= bState;
                                                        aValue <<= sal_False;
                                                    }
                                                    break;
    }
}

//*****************************************************************************************************************
//  OPropertySetHelper
//*****************************************************************************************************************
IPropertyArrayHelper& SAL_CALL Desktop::getInfoHelper()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static OPropertyArrayHelper* pInfoHelper = NULL;

    if ( pInfoHelper == NULL )
    {
        // Ready for multithreading
        LOCK_GLOBALMUTEX( aGuard, "Desktop::getInfoHelper()" )
        // Control this pointer again, another instance can be faster then these!
        if ( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return (*pInfoHelper);
}

//*****************************************************************************************************************
//  OPropertySetHelper
//*****************************************************************************************************************
Reference< XPropertySetInfo > SAL_CALL Desktop::getPropertySetInfo ()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static Reference< XPropertySetInfo >* pInfo = NULL ;

    if ( pInfo == NULL )
    {
        // Ready for multithreading
        LOCK_GLOBALMUTEX( aGuard, "Desktop::getPropertySetInfo()" )
        // Control this pointer again, another instance can be faster then these!
        if ( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Reference< XComponent > Desktop::impl_getFrameComponent( const Reference< XFrame >& xFrame ) const
{
    // Set default return value, if method failed.
    Reference< XComponent > xComponent = Reference< XComponent >();
    // Does no controller exists?
    Reference< XController > xController = xFrame->getController();
    if ( xController.is() == sal_False )
    {
        // Controller not exist - use the VCL-component.
        xComponent = Reference< XComponent >( xFrame->getComponentWindow(), UNO_QUERY );
    }
    else
    {
        // Does no model exists?
        Reference< XModel > xModel( xController->getModel(), UNO_QUERY );
        if ( xModel.is() == sal_True )
        {
            // Model exist - use the model as component.
            xComponent = Reference< XComponent >( xModel, UNO_QUERY );
        }
        else
        {
            // Model not exist - use the controller as component.
            xComponent = Reference< XComponent >( xController, UNO_QUERY );
        }
    }

    return xComponent;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool Desktop::impl_tryToChangeProperty(         sal_Bool    bProperty       ,
                                            const   Any&        aNewValue       ,
                                                    Any&        aOldValue       ,
                                                    Any&        aConvertedValue ) throw( IllegalArgumentException )
{
    // Set default return value.
    sal_Bool bReturn = sal_False;

    // Clear information of return parameter!
    aOldValue.clear();
    aConvertedValue.clear();

    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    sal_Bool bNewValue;
    convertPropertyValue( bNewValue, aNewValue );

    // If value change ...
    if ( bNewValue != bProperty )
    {
        // ... set information of change.
        // Attention:   Use setValue and getCppuBooleanType to set value in any!
        //              It's necessary, if BOOL can be int or char.
        aOldValue.setValue      ( &bProperty, ::getCppuBooleanType() );
        aConvertedValue.setValue( &bNewValue, ::getCppuBooleanType() );
        // Return OK - "value will be change ..."
        bReturn = sal_True;
    }

    return bReturn;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
const Sequence< Property > Desktop::impl_getStaticPropertyDescriptor()
{
    // Create a new static property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME !!!

    static const Property pPropertys[] =
    {
        Property( PROPERTYNAME_ACTIVECOMPONENT  , PROPERTYHANDLE_ACTIVECOMPONENT    , ::getCppuType((const Reference< XComponent >*)NULL)   , PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY),
        Property( PROPERTYNAME_ACTIVEFRAME      , PROPERTYHANDLE_ACTIVEFRAME        , ::getCppuType((const Reference< XComponent >*)NULL)   , PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY),
        Property( PROPERTYNAME_HASBEAMER        , PROPERTYHANDLE_HASBEAMER          , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASCOMMONTASKBAR , PROPERTYHANDLE_HASCOMMONTASKBAR   , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASDESIGNER      , PROPERTYHANDLE_HASDESIGNER        , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASEXPLORER      , PROPERTYHANDLE_HASEXPLORER        , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASFUNCTIONBAR   , PROPERTYHANDLE_HASFUNCTIONBAR     , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASMACROBAR      , PROPERTYHANDLE_HASMACROBAR        , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASNAVIGATOR     , PROPERTYHANDLE_HASNAVIGATOR       , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASOBJECTBAR     , PROPERTYHANDLE_HASOBJECTBAR       , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASOPTIONBAR     , PROPERTYHANDLE_HASOPTIONBAR       , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASSTATUSBAR     , PROPERTYHANDLE_HASSTATUSBAR       , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_HASTOOLBAR       , PROPERTYHANDLE_HASTOOLBAR         , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT                              ),
        Property( PROPERTYNAME_ISOLOCALE        , PROPERTYHANDLE_ISOLOCALE          , ::getCppuType((const Locale*)NULL)                    , PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY),
        Property( PROPERTYNAME_ISPLUGGED        , PROPERTYHANDLE_ISPLUGGED          , ::getBooleanCppuType()                                , PropertyAttribute::TRANSIENT | PropertyAttribute::READONLY),
    };
    // Use it to initialize sequence!
    static const Sequence< Property > seqPropertyDescriptor( pPropertys, PROPERTYCOUNT );
    // Return static "PropertyDescriptor"
    return seqPropertyDescriptor ;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void Desktop::impl_sendQueryTerminationEvent() throw( TerminationVetoException )
{
    // Send QueryTermination event to all listener.
    // Get container for right listener.
    OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const Reference< XTerminateListener >*) NULL ) );
    if( pContainer != NULL )
    {
        // Build QueryTermination event.
        EventObject aEvent( (OWeakObject*)this );
        // Get iterator for access to listener.
        OInterfaceIteratorHelper aIterator( *pContainer );
        // Send message to all listener.
        // Somewhere can throw a TerminationVetoException.
        // We don't look for that(!) ... caller of this method will catch these.
        while( aIterator.hasMoreElements() == sal_True )
        {
            ((XTerminateListener*)aIterator.next())->queryTermination( aEvent );
        }
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void Desktop::impl_sendNotifyTerminationEvent()
{
    // Send NotifyTermination event to all listener.
    // Get container for right listener.
    OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const Reference< XTerminateListener >*) NULL ) );
    if( pContainer != NULL )
    {
        // Build QueryTermination event.
        EventObject aEvent( (OWeakObject*)this );
        // Get iterator for access to listener.
        OInterfaceIteratorHelper aIterator( *pContainer );
        // Send message to all listener.
        while( aIterator.hasMoreElements() == sal_True )
        {
            ((XTerminateListener*)aIterator.next())->notifyTermination( aEvent );
        }
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool Desktop::impl_checkPlugInState()
{
    // set default return value if search failed or no plugin could be detected.
    sal_Bool bReturn = sal_False;

    // We must search at ouer childs. We make a deep search.
    // Lock the container. Nobody should append or remove elements during next time.
    // But don't forget to unlock it again!
    m_aChildTaskContainer.lock();
    // Break loop, if something was found or all container items was compared.
    sal_uInt32 nCount       = m_aChildTaskContainer.getCount();
    sal_uInt32 nPosition    = 0;
    while   (
                ( bReturn   ==  sal_False   )   &&
                ( nPosition <   nCount      )
            )
    {
        Reference< XPluginInstance > xPlugInFrame( m_aChildTaskContainer[nPosition], UNO_QUERY );
        bReturn = xPlugInFrame.is();
        ++nPosition;
    }
    // Don't forget to unlock the container!
    m_aChildTaskContainer.unlock();

    return bReturn;
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
//  We work with valid servicemanager only.
sal_Bool Desktop::impldbg_checkParameter_DesktopCtor( const Reference< XMultiServiceFactory >& xFactory )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xFactory     ==  NULL        )   ||
            ( xFactory.is() ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::impldbg_checkParameter_addTerminateListener( const Reference< XTerminateListener >& xListener )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::impldbg_checkParameter_removeTerminateListener( const Reference< XTerminateListener >& xListener )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  We need a url to work. But the target frame name and arguments are optional.
//  Flags must in range of right enum!
sal_Bool Desktop::impldbg_checkParameter_loadComponentFromURL(  const   OUString&                   sURL            ,
                                                                const   OUString&                   sTargetFrameName,
                                                                           sal_Int32                    nSearchFlags    ,
                                                                const   Sequence< PropertyValue >&  seqArguments    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &sURL                         ==  NULL        )   ||
            ( &sTargetFrameName             ==  NULL        )   ||
            ( &seqArguments                 ==  NULL        )   ||
            ( sURL.getLength()              <   1           )   ||
            (
                (    nSearchFlags != FrameSearchFlag::AUTO        ) &&
                ( !( nSearchFlags &  FrameSearchFlag::PARENT    ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::SELF      ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::CHILDREN  ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::CREATE    ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::SIBLINGS  ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::TASKS     ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::ALL       ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::GLOBAL    ) )
            )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  We need a url to work. But the target frame name is optional.
//  Flags must in range of right enum!
sal_Bool Desktop::impldbg_checkParameter_queryDispatch( const   URL&        aURL            ,
                                                        const   OUString&   sTargetFrameName,
                                                                sal_Int32   nSearchFlags    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aURL                         ==  NULL        )   ||
            ( &sTargetFrameName             ==  NULL        )   ||
            ( aURL.Complete.getLength()     <   1           )   ||
            (
                (    nSearchFlags != FrameSearchFlag::AUTO        ) &&
                ( !( nSearchFlags &  FrameSearchFlag::PARENT    ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::SELF      ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::CHILDREN  ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::CREATE    ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::SIBLINGS  ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::TASKS     ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::ALL       ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::GLOBAL    ) )
            )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  The traget frame name is optional, but flags must be in range of right enum.
sal_Bool Desktop::impldbg_checkParameter_findFrame( const   OUString&   sTargetFrameName,
                                                            sal_Int32   nSearchFlags    )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &sTargetFrameName == NULL )   ||
            (
                (    nSearchFlags != FrameSearchFlag::AUTO        ) &&
                ( !( nSearchFlags &  FrameSearchFlag::PARENT    ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::SELF      ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::CHILDREN  ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::CREATE    ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::SIBLINGS  ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::TASKS     ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::ALL       ) ) &&
                ( !( nSearchFlags &  FrameSearchFlag::GLOBAL    ) )
            )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::impldbg_checkParameter_addFrameActionListener( const Reference< XFrameActionListener >& xListener )
{
    // Set default return value.
    sal_Bool bOK = sal_True;

    // Check parameter.
    if  (
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }

    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::impldbg_checkParameter_removeFrameActionListener( const Reference< XFrameActionListener >& xListener )
{
    // Set default return value.
    sal_Bool bOK = sal_True;

    // Check parameter.
    if  (
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }

    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::impldbg_checkParameter_addEventListener( const Reference< XEventListener >& xListener )
{
    // Set default return value.
    sal_Bool bOK = sal_True;

    // Check parameter.
    if  (
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }

    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::impldbg_checkParameter_removeEventListener( const Reference< XEventListener >& xListener )
{
    // Set default return value.
    sal_Bool bOK = sal_True;

    // Check parameter.
    if  (
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }

    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
sal_Bool Desktop::impldbg_checkParameter_statusChanged( const FeatureStateEvent& aEvent )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aEvent                               ==  NULL    )   ||
            ( aEvent.FeatureDescriptor.getLength()  <   1       )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

//*****************************************************************************************************************
sal_Bool Desktop::impldbg_checkParameter_disposing( const EventObject& aSource )
{
    // Set default return value.
    sal_Bool bOK = sal_True;
    // Check parameter.
    if  (
            ( &aSource              ==  NULL        )   ||
            ( aSource.Source.is()   ==  sal_False   )
        )
    {
        bOK = sal_False ;
    }
    // Return result of check.
    return bOK ;
}

#endif  // #ifdef ENABLE_ASSERTIONS

/*-----------------------------------------------------------------------------------------------------------------
    Follow method is used to print out the content of current container.
    Use this to get information about the tree.
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_SERVICEDEBUG  // Is defined in debug version only.

//*****************************************************************************************************************
OUString Desktop::impldbg_getTreeNames()
{
    // We start from here (the top) to bottom.
    // Create an "empty stream" with enough place for ouer own container informations.
    OUStringBuffer sOutPut(1024);

    // Start with level 0! We are on the top.
    sal_Int16 nLevel = 0;

    // Add my own information to stream.
    // Format of output : "<Level*TAB>[<level>:<name>:<extra informations>]\n"
    sOutPut.appendAscii( "[0:\"" );
    sOutPut.append( m_sName );
    sOutPut.appendAscii( "\":ACTIVE]\n" );

    // Then start with ouer own container.
    // We must lock the container, to have exclusiv access to elements!
    m_aChildTaskContainer.lock();
    // Step over all elements in current container and add names.
    sal_uInt32 nCount = m_aChildTaskContainer.getCount();
    for ( sal_uInt32 nPosition=0; nPosition<nCount; ++nPosition )
    {
        // Step during tree deep first - from the left site to the right one.
        // Print subtree of this child to stream!
        Reference< XFrame > xItem = m_aChildTaskContainer[nPosition];
        Reference< XSPECIALDEBUGINTERFACE > xDebug( xItem, UNO_QUERY );
        sOutPut.append( xDebug->dumpVariable( DUMPVARIABLE_TREEINFO, nLevel+1 ) );
    }
    // Don't forget to unlock the container!
    m_aChildTaskContainer.unlock();

    // Now we have anough informations about tree.
    // Return it to caller. He must print this informations!
    return sOutPut.makeStringAndClear();
}

#endif  // #ifdef ENABLE_SERVICEDEBUG

}   // namespace framework
