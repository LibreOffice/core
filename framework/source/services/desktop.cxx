/*************************************************************************
 *
 *  $RCSfile: desktop.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: as $ $Date: 2001-07-20 08:10:52 $
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
#include <services/desktop.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OTASKSACCESS_HXX_
#include <helper/otasksaccess.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OCOMPONENTACCESS_HXX_
#include <helper/ocomponentaccess.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_
#include <dispatch/dispatchprovider.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
#include <classes/targetfinder.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
#include <threadhelp/transactionguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
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

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

// names of properties
#define PROPERTYNAME_ACTIVECOMPONENT            DECLARE_ASCII("ActiveComponent" )
#define PROPERTYNAME_ACTIVEFRAME                DECLARE_ASCII("ActiveFrame"     )
#define PROPERTYNAME_ISPLUGGED                  DECLARE_ASCII("IsPlugged"       )
/*OBSOLETE
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
*/

// handle of properties
#define PROPERTYHANDLE_ACTIVECOMPONENT          1
#define PROPERTYHANDLE_ACTIVEFRAME              2
#define PROPERTYHANDLE_ISPLUGGED                3
/*OBSOLETE
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
*/

// count of ALL properties
#define PROPERTYCOUNT                           3

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_11                    (   Desktop                                                  ,
                                            OWeakObject                                              ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider              ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo               ),
                                            DIRECT_INTERFACE( css::frame::XDesktop                  ),
                                            DIRECT_INTERFACE( css::frame::XComponentLoader          ),
                                            DIRECT_INTERFACE( css::frame::XTasksSupplier            ),
                                            DIRECT_INTERFACE( css::frame::XDispatchProvider         ),
                                            DIRECT_INTERFACE( css::frame::XFramesSupplier           ),
                                            DIRECT_INTERFACE( css::frame::XFrame                    ),
                                            DIRECT_INTERFACE( css::lang::XComponent                 ),
                                            DIRECT_INTERFACE( css::frame::XStatusListener           ),
                                            DIRECT_INTERFACE( css::lang::XEventListener             )
                                        )

DEFINE_XTYPEPROVIDER_11                 (   Desktop                                                 ,
                                            css::lang::XTypeProvider                                ,
                                            css::lang::XServiceInfo                                 ,
                                            css::frame::XDesktop                                    ,
                                            css::frame::XComponentLoader                            ,
                                            css::frame::XTasksSupplier                              ,
                                            css::frame::XDispatchProvider                           ,
                                            css::frame::XFramesSupplier                             ,
                                            css::frame::XFrame                                      ,
                                            css::lang::XComponent                                   ,
                                            css::frame::XStatusListener                             ,
                                            css::lang::XEventListener
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   Desktop                                                 ,
                                            ::cppu::OWeakObject                                     ,
                                            SERVICENAME_DESKTOP                                     ,
                                            IMPLEMENTATIONNAME_DESKTOP
                                        )

DEFINE_INIT_SERVICE                     (   Desktop,
                                            {
                                                /*Attention
                                                    I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                                                    to create a new instance of this class by our own supported service factory.
                                                    see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                                                */

                                                //-------------------------------------------------------------------------------------------------------------
                                                // Initialize a new XFrames-helper-object to handle XIndexAccess and XElementAccess.
                                                // We hold member as reference ... not as pointer too!
                                                // Attention: We share our frame container with this helper. Container is threadsafe himself ... So I think we can do that.
                                                // But look on dispose() for right order of deinitialization.
                                                OFrames* pFramesHelper = new OFrames( m_xFactory, this, &m_aChildTaskContainer );
                                                m_xFramesHelper = css::uno::Reference< css::frame::XFrames >( static_cast< ::cppu::OWeakObject* >(pFramesHelper), css::uno::UNO_QUERY );

                                                //-------------------------------------------------------------------------------------------------------------
                                                // Initialize a new dispatchhelper-object to handle dispatches.
                                                DispatchProvider* pDispatchHelper = new DispatchProvider( m_xFactory, this );
                                                m_xDispatchHelper = css::uno::Reference< css::frame::XDispatchProvider >( static_cast< ::cppu::OWeakObject* >(pDispatchHelper), css::uno::UNO_QUERY );

                                                //-------------------------------------------------------------------------------------------------------------
                                                // I'am the desktop - and use my frame container in a special mode.
                                                // If last child task is removed, I must die!
                                                // My container should terminate me asynchronous by using a timer.
                                                // Enable timer at container.
                                                // (The timer will be initialized with right timeout value automaticly by himself. see class AsyncQuit for further informations!)
                                                m_aChildTaskContainer.enableQuitTimer( this );

                                                // Safe impossible cases
                                                // We can't work without this helper!
                                                LOG_ASSERT2( m_xFramesHelper.is  ()==sal_False, "Desktop::Desktop()", "Frames helper is not valid. XFrames, XIndexAccess and XElementAcces are not supported!\n")
                                                LOG_ASSERT2( m_xDispatchHelper.is()==sal_False, "Desktop::Desktop()", "Dispatch helper is not valid. XDispatch will not work correctly!\n"                      )

                                                // Enable object for real working!
                                                // Otherwise all calls will be rejected ...
                                                m_aTransactionManager.setWorkingMode( E_WORK );
                                            }
                                        )

/*-************************************************************************************************************//**
    @short      standard constructor to create instance by factory
    @descr      This constructor initialize a new instance of this class by valid factory,
                and will be set valid values on his member and baseclasses.

    @attention  a)  Don't use your own reference during an UNO-Service-ctor! There is no guarantee, that you
                    will get over this. (e.g. using of your reference as parameter to initialize some member)
                    Do such things in DEFINE_INIT_SERVICE() method, which is called automaticly after your ctor!!!
                b)  Baseclass OBroadcastHelper is a typedef in namespace cppu!
                    The microsoft compiler has some problems to handle it right BY using namespace explicitly ::cppu::OBroadcastHelper.
                    If we write it without a namespace or expand the typedef to OBrodcastHelperVar<...> -> it will be OK!?
                    I don't know why! (other compiler not tested .. but it works!)

    @seealso    method DEFINE_INIT_SERVICE()

    @param      "xFactory" is the multi service manager, which create this instance.
                The value must be different from NULL!
    @return     -

    @onerror    We throw an ASSERT in debug version or do nothing in relaese version.
*//*-*************************************************************************************************************/
Desktop::Desktop( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        //  Attention: Don't change order of initialization!
        //      ThreadHelpBase is a struct with a lock as member. We can't use a lock as direct member!
        //      We must garant right initialization and a valid value of this to initialize other baseclasses!
        :   ThreadHelpBase          ( &Application::GetSolarMutex()                 )
        ,   TransactionBase         (                                               )
        ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >           ( m_aLock.getShareableOslMutex()         )
        ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
        ,   ::cppu::OWeakObject     (                                               )
        // Init member
        ,   m_xFactory              ( xFactory                                      )
        ,   m_aChildTaskContainer   (                                               )
        ,   m_aListenerContainer    ( m_aLock.getShareableOslMutex()                )
        ,   m_eLoadState            ( E_NOTSET                                      )
        ,   m_aTaskCreator          ( xFactory                                      )
        // Init Properties
        /*OBSOLETE
        ,   m_bHasBeamer            ( sal_True                                      )
        ,   m_bHasCommonTaskBar     ( sal_True                                      )
        ,   m_bHasDesigner          ( sal_True                                      )
        ,   m_bHasExplorer          ( sal_True                                      )
        ,   m_bHasFunctionBar       ( sal_True                                      )
        ,   m_bHasMacroBar          ( sal_True                                      )
        ,   m_bHasNavigator         ( sal_True                                      )
        ,   m_bHasObjectBar         ( sal_True                                      )
        ,   m_bHasOptionBar         ( sal_True                                      )
        ,   m_bHasStatusBar         ( sal_True                                      )
        ,   m_bHasToolbar           ( sal_True                                      )
        */
        #ifdef ENABLE_ASSERTIONS
        ,   m_bIsTerminated         ( sal_False                                     )   // see dispose() for further informations!
        #endif
{
    // Safe impossible cases
    // We don't accept all incoming parameter.
    LOG_ASSERT2( implcp_ctor( xFactory ), "Desktop::Desktop()", "Invalid parameter detected!")
}

/*-************************************************************************************************************//**
    @short      standard destructor
    @descr      This one do NOTHING! Use dispose() instaed of this.

    @seealso    method dispose()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
Desktop::~Desktop()
{
    LOG_ASSERT2( m_bIsTerminated                       ==sal_False, "Desktop::~Desktop()", "Who forgot to terminate the desktop service?" )
    LOG_ASSERT2( m_aTransactionManager.getWorkingMode()!=E_CLOSE  , "Desktop::~Desktop()", "Who forgot to dispose this service?"          )
}

/*-************************************************************************************************************//**
    @interface  XDesktop
    @short      ask desktop before terminate it
    @descr      The desktop ask his components and if all say "yes" it will destroy this components
                and return "yes" to caller of this method. Otherwhise, desktop will not be destroied!
                But a TerminateListener with a veto - will be the new owner of this service and MUST
                call terminate again, if he stop using of it!

    @seealso    interface XTerminateListener

    @param      -
    @return     sal_True  ,if all components say "yes" for terminate
                sal_False ,otherwise

    @onerror    We return sal_False.
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool SAL_CALL Desktop::terminate() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Attention, Our timer hold a reference to use ... and sometimes (e.g. for headless office mode)
    // he forget this one insteandly during his own instantiation! So we can die during next call.
    // That's why it's agood idea to hold use self alive.
    css::uno::Reference< css::frame::XDesktop > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // Set default return value to TRUE!
    // Because; if we detect a vetor of a listener or a task say "no" to our close() call ...
    // we should reset to FALSE.
    sal_Bool bTerminated = sal_True;

    // Disable our async quit timer ... we terminate ourself!
    // But if termination fail ... don't forget to enable it again!
    // Member is threadsafe himself!
    m_aChildTaskContainer.disableQuitTimer();

    // Step over all child tasks and ask they "WOULD YOU DIE?"
    css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > lTasks = m_aChildTaskContainer.getAllElements();
    sal_Int32                                                       nCount = lTasks.getLength()                    ;
    for( sal_Int32 nPosition=0; nPosition<nCount; ++nPosition )
    {
        // Get an element from container and cast it to task.
        // IT MUST BE A TASK! Childs of desktop everytime tasks. No pure frames accepted!
        // It can be a plugin too, but a plugin is derived from a task ...!
        css::uno::Reference< css::frame::XTask > xTask( lTasks[nPosition], css::uno::UNO_QUERY );
        // Ask task for terminating. If anyone say "NO" ...
        // ... we must reset ouer default return value to "NO" too!
        // But we don't break this loop ... we will close all task, which accept it.
        if( xTask->close() == sal_False )
        {
            bTerminated = sal_False;
        }
    }

    // If termination of tasks was successfully ... ask listener for her agreement.
    // A vetoable listener can throw an TerminateVetoException.
    // Then we return false!
    if( bTerminated == sal_True )
    {
        try
        {
            impl_sendQueryTerminationEvent();
        }
        catch( css::frame::TerminationVetoException& )
        {
            bTerminated = sal_False;
        }
    }

    // All listener has no problem with our termination!
    // Send NotifyTermination event to all and return with true.
    if( bTerminated == sal_True )
    {
        #ifdef ENABLE_ASSERTIONS
            // "Protect" us against dispose before terminate calls!
            // see dispose() for further informations.
            // Follow notify will start shutdown of office and somewhere call dispose() at us ...
            // Set debug variable BEFORE notify!
            /* SAFE AREA --------------------------------------------------------------------------------------- */
            WriteGuard aWriteLock( m_aLock );
            m_bIsTerminated = sal_True ;
            aWriteLock.unlock();
            /* UNSAFE AREA ------------------------------------------------------------------------------------- */
        #endif

        impl_sendNotifyTerminationEvent();
    }
    // If somewhere break this terminate operation and we must return FALSE ...
    // we must reactivate our quit timer!
    // Otherwise we live for ever.
    else
    {
        m_aChildTaskContainer.enableQuitTimer( xThis );
    }

    // Return result of this question.
    return bTerminated;
}

/*-************************************************************************************************************//**
    @interface  XDesktop
    @short      add/remove a listener for terminate events
    @descr      You can add a listener, if you wish to get an event, if desktop will be terminate.
                Then it's possible to say "NO" by using a TerminateVetoException!

    @attention  a)  We don't need any mutex or lock here ... because our helper is threadsafe himself ... and live
                    if we live. Our registered transaction guarantee that!
                b)  May be we dispose our listener during our own dispose methode ... after closing object for
                    real working. They call us back to remove her interfaces from our listener container.
                    So we should allow that by using E_SOFTEXCEPTIONS and suppress rejection of this calls!

    @seealso    method terminate()

    @param      "xListener" is a reference to the listener. His value must be valid!
    @return     -

    @onerror    We do nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::addTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_addTerminateListener( xListener ), "Desktop::addTerminateListener()", "Invalid parameter detected!" )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ), xListener );
}

//*****************************************************************************************************************
void SAL_CALL Desktop::removeTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_removeTerminateListener( xListener ), "Desktop::removeTerminateListener()", "Invalid parameter detected!" )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ), xListener );
}

/*-************************************************************************************************************//**
    @interface  XDesktop
    @short      get access to create enumerations of all current components
    @descr      You will be the owner of the returned object and must delete it if you don't use it again.

    @seealso    class TasksAccess
    @seealso    class TasksEnumeration

    @param      -
    @return     A reference to an XEnumerationAccess-object.

    @onerror    We return a null-reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL Desktop::getComponents() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // We use a helper class OComponentAccess to have access on all child components.
    // Create it on demand and return it as a reference.
    OComponentAccess* pAccess = new OComponentAccess( this );
    css::uno::Reference< css::container::XEnumerationAccess > xAccess( static_cast< ::cppu::OWeakObject* >(pAccess), css::uno::UNO_QUERY );
    return xAccess;
}

/*-************************************************************************************************************//**
    @interface  XDesktop
    @short      return the current active component
    @descr      The most current component is the window, model or the controller of the current active frame.

    @seealso    method getCurrentFrame()
    @seealso    method impl_getFrameComponent()

    @param      -
    @return     A reference to the component.

    @onerror    We return a null-reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::lang::XComponent > SAL_CALL Desktop::getCurrentComponent() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Set return value if method failed.
    css::uno::Reference< css::lang::XComponent > xComponent;

    // Get reference to current frame ...
    // ... get component of this frame ... (It can be the window, the model or the controller.)
    // ... and return the result.
    css::uno::Reference< css::frame::XFrame > xCurrentFrame = getCurrentFrame();
    if( xCurrentFrame.is() == sal_True )
    {
        xComponent = impl_getFrameComponent( xCurrentFrame );
    }
    return xComponent;
}

/*-************************************************************************************************************//**
    @interface  XDesktop
    @short      return the current active frame in hierarchy
    @descr      There can be more then one different active pathes in our frame hierarchy. But only one of them
                could be the most active frame (normal he has the focus).
                Don't mix it with getActiveFrame()! That will return our current active frame, which must be
                a direct child of us and should be a part(!) of an active path.

    @seealso    method getActiveFrame()

    @param      -
    @return     A valid reference, if there is an active frame.
                A null reference , otherwise.

    @onerror    We return a null reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::getCurrentFrame() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Start search with ouer direct active frame (if it exist!).
    // Search on his children for other active frames too.
    // Stop if no one could be found and return last of found ones.
    css::uno::Reference< css::frame::XFramesSupplier > xLast = css::uno::Reference< css::frame::XFramesSupplier >( getActiveFrame(), css::uno::UNO_QUERY );
    if( xLast.is() == sal_True )
    {
        css::uno::Reference< css::frame::XFramesSupplier > xNext = css::uno::Reference< css::frame::XFramesSupplier >( xLast->getActiveFrame(), css::uno::UNO_QUERY );
        while( xNext.is() == sal_True )
        {
            xLast = xNext;
            xNext = css::uno::Reference< css::frame::XFramesSupplier >( xNext->getActiveFrame(), css::uno::UNO_QUERY );
        }
    }
    return css::uno::Reference< css::frame::XFrame >( xLast, css::uno::UNO_QUERY );
}

/*-************************************************************************************************************//**
    @interface  XComponentLoader
    @short      try to load given URL into a task
    @descr      You can give us some informations about the content, which you will load into a frame.
                We search or create this target for you, make a type detection of given URL and try to load it.
                As result of this operation we return the new created component or nothing, if loading failed.

    @seealso    -

    @param      "sURL"              , URL, which represant the content
    @param      "sTargetFrameName"  , name of target frame or special value like "_self", "_blank" ...
    @param      "nSearchFlags"      , optional arguments for frame search, if target isn't a special one
    @param      "lArguments"        , optional arguments for loading
    @return     A valid component reference, if loading was successfully.
                A null reference otherwise.

    @onerror    We return a null reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::lang::XComponent > SAL_CALL Desktop::loadComponentFromURL( const ::rtl::OUString&                                 sURL            ,
                                                                                     const ::rtl::OUString&                                 sTargetFrameName,
                                                                                           sal_Int32                                        nSearchFlags    ,
                                                                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments      ) throw(    css::io::IOException                ,
                                                                                                                                                                        css::lang::IllegalArgumentException ,
                                                                                                                                                                        css::uno::RuntimeException          )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // CHeck incoming parameter and throw an exception for wrong values!
    if(
        ( &sURL            == NULL )   ||
        ( sURL.getLength() <  1    )
      )
    {
        throw css::lang::IllegalArgumentException( DECLARE_ASCII("Null pointer or empty URLs are not allowed!"), static_cast< ::cppu::OWeakObject* >(this), 1 );
    }
    // Attention: An empty target name is equal to "_self"!
    if( &sTargetFrameName == NULL )
    {
        throw css::lang::IllegalArgumentException( DECLARE_ASCII("Null pointer are not allowed!"), static_cast< ::cppu::OWeakObject* >(this), 2 );
    }
    /* TODO: How can I test the search flags?! */
    // Attention: Arguments are optional!
    if( &lArguments == NULL )
    {
        throw css::lang::IllegalArgumentException( DECLARE_ASCII("Null pointer are not allowed!"), static_cast< ::cppu::OWeakObject* >(this), 4 );
    }

    // Set default return value, if method failed.
    css::uno::Reference< css::lang::XComponent > xComponent;

    /*TODO:
        To support creation of pluginframes too ... we must use queryDispatch() yet!
        It's the only way to do so. If we found another way - we should implement this method better.
     */

    css::util::URL aURL;
    aURL.Complete = sURL;
    css::uno::Reference< css::frame::XDispatch > xDispatcher = queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    if( xDispatcher.is() == sal_True )
    {
        // ... dispatch URL at this dispatcher.
        // Set us as listener for status events from this dispatcher.
        // The dispatcher get a event loadFinished() or loadCancelled() from his frameloader
        // and map this information for us! (see classes DispatchProvider/BaseDispatcher for further informations)
        xDispatcher->addStatusListener( static_cast< css::frame::XStatusListener* >( this ), aURL );

        // Reset loader state to default, because we must yield for a valid result! See next WHILE condition.
        // And we must do it before we call dispatch!
        /* SAFE AREA ------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_eLoadState = E_NOTSET;
        aWriteLock.unlock();
        /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

        xDispatcher->dispatch( aURL, lArguments );

        // ... we must wait for asynchron result of this dispatch()-operation!
        // Attention: Don't use lock here ... dispatcher call us back!
        while( m_eLoadState == E_NOTSET )
        {
            Application::Yield();
        }

        // We should have all informations about result state of loading document.
        // We can remove us as listener!
        xDispatcher->removeStatusListener( static_cast< css::frame::XStatusListener* >( this ), aURL );

        // Try to get new current component.
        // Look for result of loading.
        // If loading was successful we can return a component.
        // Attention: Don't react for failed or successfully loading operations ... our dispatcher do that for us!

        // First make snapshot of our asynchron status informations by using a lock.
        /* SAFE AREA ------------------------------------------------------------------------------------------- */
        ReadGuard aReadLock( m_aLock );
        ELoadState                                eState       = m_eLoadState;
        css::uno::Reference< css::frame::XFrame > xLoadTarget  = m_xLastFrame;
                                                  m_xLastFrame = css::uno::Reference< css::frame::XFrame >(); // Don't hold last frame for ever - or he can't die!
        aReadLock.unlock();
        /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

        if(
            ( eState           == E_SUCCESSFUL  )   &&
            ( xLoadTarget.is() == sal_True      )
          )
        {
            xComponent = impl_getFrameComponent( xLoadTarget );
        }
    }
    // Return result of this operation.
    return xComponent;
}

/*-************************************************************************************************************//**
    @interface  XTasksSupplier
    @short      get access to create enumerations of ouer taskchilds
    @descr      Direct childs of desktop are tasks everytime (could be PlugInFrames too).
                Call these method to could create enumerations of it.

But; Don't forget - you will be the owner of returned object and must release it!
                We use a helper class to implement the access interface. They hold a weakreference to us.
                It can be, that the desktop is dead - but not your tasksaccess-object! Then they will do nothing!
                You can't create enumerations then.

    @attention  Normaly we don't need any lock here. We don't work on internal member!

    @seealso    class TasksAccess

    @param      -
    @return     A reference to an accessobject, which can create enumerations of ouer childtasks.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL Desktop::getTasks() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    OTasksAccess* pTasksAccess = new OTasksAccess( this, &m_aChildTaskContainer );
    css::uno::Reference< css::container::XEnumerationAccess > xAccess( static_cast< ::cppu::OWeakObject* >(pTasksAccess), css::uno::UNO_QUERY );
    return xAccess;
}

/*-************************************************************************************************************//**
    @interface  XTasksSupplier
    @short      return current active task of ouer direct childs
    @descr      Desktop childs are tasks only (could be PlugInFrames too)! If we have an active path from desktop
                as top to any frame on bottom, we must have an active direct child. His reference is returned here.

    @attention  a)  Do not confuse it with getCurrentFrame()! The current frame don't must one of ouer direct childs.
                    It can be every frame in subtree and must have the focus (Is the last one of an active path!).
                b)  We don't need any lock here. Our container is threadsafe himself and live, if we live!

    @seealso    method getCurrentFrame()

    @param      -
    @return     A reference to ouer current active taskchild.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XTask > SAL_CALL Desktop::getActiveTask() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return css::uno::Reference< css::frame::XTask >( m_aChildTaskContainer.getActive(), css::uno::UNO_QUERY );
}

/*-************************************************************************************************************//**
    @interface  XDispatchProvider
    @short      search a dispatcher for given URL
    @descr      We use a helper implementation (class DispatchProvider) to do so.
                So we don't must implement this algorithm twice!

    @attention  We don't need any lock here. Our helper is threadsafe himself and live, if we live!

    @seealso    class DispatchProvider

    @param      "aURL"              , URL to dispatch
    @param      "sTargetFrameName"  , name of target frame, who should dispatch these URL
    @param      "nSearchFlags"      , flags to regulate the search
    @param      "lQueries"          , list of queryDispatch() calls!
    @return     A reference or list of founded dispatch objects for these URL.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XDispatch > SAL_CALL Desktop::queryDispatch( const css::util::URL&  aURL             ,
                                                                              const ::rtl::OUString& sTargetFrameName ,
                                                                                    sal_Int32        nSearchFlags     ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_xDispatchHelper->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
}

//*****************************************************************************************************************
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL Desktop::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lQueries ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_xDispatchHelper->queryDispatches( lQueries );
}

/*-************************************************************************************************************//**
    @interface  XFramesSupplier
    @short      return access to append or remove childs on desktop
    @descr      We don't implement these interface directly. We use a helper class to do this.
                If you wish to add or delete childs to/from the container, call these method to get
                a reference to the helper.

    @attention  Helper is threadsafe himself. So we don't need any lock here.

    @seealso    class OFrames

    @param      -
    @return     A reference to the helper.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XFrames > SAL_CALL Desktop::getFrames() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_xFramesHelper;
}

/*-************************************************************************************************************//**
    @interface  XFramesSupplier
    @short      set/get the current active child frame
    @descr      It must be a task. Direct childs of desktop are tasks only! No frames are accepted.
                We don't save this information directly in this class. We use ouer container-helper
                to do that.

    @attention  Helper is threadsafe himself. So we don't need any lock here.

    @seealso    class OFrameContainer

    @param      "xFrame", new active frame (must be valid!)
    @return     A reference to ouer current active childtask, if anyone exist.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Get old active frame first.
    // If nothing will change - do nothing!
    // Otherwise set new active frame ...
    // and deactivate last frame.
    // It's neccessary for our FrameActionEvent listener on a frame!
    css::uno::Reference< css::frame::XFrame > xLastActiveChild = m_aChildTaskContainer.getActive();
    if( xLastActiveChild != xFrame )
    {
        m_aChildTaskContainer.setActive( xFrame );
        if( xLastActiveChild.is() == sal_True )
        {
            xLastActiveChild->deactivate();
        }
    }
}

//*****************************************************************************************************************
css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::getActiveFrame() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_aChildTaskContainer.getActive();
}

/*-************************************************************************************************************//**
    @interface  XFrame
    @short      non implemented methods!
    @descr      Some method make no sense for our desktop! He has no window or parent or ...
                So we should implement it empty and warn programmer, if he use it!

    @seealso    -

    @param      -
    @return     -

    @onerror    -
    @threadsafe -
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::initialize( const css::uno::Reference< css::awt::XWindow >& xWindow ) throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::initialize()", "Not implemented! Desktop has no window." )
}

//*****************************************************************************************************************
css::uno::Reference< css::awt::XWindow > SAL_CALL Desktop::getContainerWindow() throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::getContainerWindow()", "Not implemented! Desktop has no window." )
    return css::uno::Reference< css::awt::XWindow >();
}

//*****************************************************************************************************************
void SAL_CALL Desktop::setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator ) throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::setCreator()", "Not implemented! Desktop has no parent." )
}

//*****************************************************************************************************************
css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL Desktop::getCreator() throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::getCreator()", "Not implememted! Desktop has no parent." )
    return css::uno::Reference< css::frame::XFramesSupplier >();
}

//*****************************************************************************************************************
::rtl::OUString SAL_CALL Desktop::getName() throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::getName()", "Not implemented! Desktop could not have any valid name!" )
    return ::rtl::OUString();
}

//*****************************************************************************************************************
void SAL_CALL Desktop::setName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::setName()", "Not implemented! Desktop could not have any valid name!" )
}

//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::isTop() throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::isTop()", "Not implemented! Desktop is top every time!" )
    return sal_True;
}

//*****************************************************************************************************************
void SAL_CALL Desktop::activate() throw( css::uno::RuntimeException )
{
    // Desktop is activae always ... but sometimes our frames try to activate
    // the complete path from bottom to top ... And our desktop is the topest frame :-(
    // So - please don't show any assertions here. Do nothing!
}

//*****************************************************************************************************************
void SAL_CALL Desktop::deactivate() throw( css::uno::RuntimeException )
{
    // Desktop is activae always ... but sometimes our frames try to deactivate
    // the complete path from bottom to top ... And our desktop is the topest frame :-(
    // So - please don't show any assertions here. Do nothing!
}

//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::isActive() throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::isActive()", "Not implemented! Desktop is active every time!" )
    return sal_True;
}

//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::setComponent( const css::uno::Reference< css::awt::XWindow >&       xComponentWindow ,
                                         const css::uno::Reference< css::frame::XController >& xController      ) throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::setComponent()", "Not implemented! No component allowed on desktop." )
    return sal_False;
}

//*****************************************************************************************************************
css::uno::Reference< css::awt::XWindow > SAL_CALL Desktop::getComponentWindow() throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::getComponentWindow()", "Not implemented! No component allowed on desktop." )
    return css::uno::Reference< css::awt::XWindow >();
}

//*****************************************************************************************************************
css::uno::Reference< css::frame::XController > SAL_CALL Desktop::getController() throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::getController()", "Not implemented! No controller allowed on desktop." )
    return css::uno::Reference< css::frame::XController >();
}

//*****************************************************************************************************************
void SAL_CALL Desktop::contextChanged() throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::contextChanged()", "Not implemented! Desktop has no component." )
}

//*****************************************************************************************************************
void SAL_CALL Desktop::addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::addFrameActionListener()", "Not implemented! There is no component inside." )
}

//*****************************************************************************************************************
//   css::frame::XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::addFrameActionListener()", "Not implemented! There is no component inside." )
}

/*-************************************************************************************************************//**
    @interface  XFrame
    @short      try to find a frame with special parameters
    @descr      This method searches for a frame with the specified name.
                Frames may contain other frames (e.g. a frameset) and may
                be contained in other frames. This hierarchie ist searched by
                this method.
                First some special names are taken into account, i.e. "",
                "_self", "_top", "_parent" etc. The FrameSearchFlags are ignored
                when comparing these names with aTargetFrameName, further steps are
                controlled by the FrameSearchFlags. If allowed, the name of the frame
                itself is compared with the desired one, then ( again if allowed )
                the method findFrame is called for all children of the frame.
                If no Frame with the given name is found until the top frames container,
                a new top Frame is created, if this is allowed by a special
                FrameSearchFlag. The new Frame also gets the desired name.
                We use a helper to get right search direction and react in a right manner.

    @seealso    class TargetFinder

    @param      "sTargetFrameName"  , name of searched frame
    @param      "nSearchFlags"      , flags to regulate search
    @return     A reference to an existing frame in hierarchy, if it exist.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::findFrame( const ::rtl::OUString& sTargetFrameName ,
                                                                             sal_Int32        nSearchFlags     ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    LOG_ASSERT2( implcp_findFrame( sTargetFrameName, nSearchFlags ), "Desktop::findFrame()", "Invalid parameter detected." )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Log some important informations in special debug mode!
    LOG_PARAMETER_FINDFRAME( "Desktop", ::rtl::OUString(), sTargetFrameName, nSearchFlags )

    // Set default return value if method failed.
    css::uno::Reference< css::frame::XFrame > xSearchedFrame;

    // Ask helper for right decision for given parameter.
    TargetInfo   aInfo   ( sTargetFrameName, nSearchFlags, E_DESKTOP, m_aChildTaskContainer.hasElements(), sal_False, ::rtl::OUString(), ::rtl::OUString() );
    ETargetClass eResult = TargetFinder::classifyFindFrame( aInfo );
    switch( eResult )
    {
        case E_CREATETASK   :   {
                                    xSearchedFrame = m_aTaskCreator.createNewSystemTask( sTargetFrameName );
                                }
                                break;
        case E_TASKS        :   {
                                    xSearchedFrame = m_aChildTaskContainer.searchDirectChildren( sTargetFrameName );
                                }
                                break;
        case E_DEEP_DOWN    :   {
                                    xSearchedFrame = m_aChildTaskContainer.searchDeepDown( sTargetFrameName );
                                }
                                break;
        case E_FLAT_DOWN    :   {
                                    xSearchedFrame = m_aChildTaskContainer.searchFlatDown( sTargetFrameName );
                                }
                                break;
        #ifdef ENABLE_WARNINGS
        default             :   {
                                    if( eResult != E_UNKNOWN )
                                    {
                                        LOG_ERROR( "Desktop::findFrame()", "Unexpected result of TargetFinder::classify() detected!" )
                                    }
                                }
                                break;
        #endif
    }

    // If no right target could be found - but CREATE flag was set ... do it; create a new task.
    if  (
            ( xSearchedFrame.is()    ==  sal_False )   &&
            ( aInfo.bCreationAllowed ==  sal_True  )
        )
    {
        xSearchedFrame = m_aTaskCreator.createNewSystemTask( sTargetFrameName );
    }

    LOG_RESULT_FINDFRAME( "Desktop", m_sName, xSearchedFrame )

    // return result of operation.
    return xSearchedFrame;
}

/*-************************************************************************************************************//**
    @interface  XComponent
    @short      release all reference
    @descr      The owner of this object calles the dispose method if the object
                should be destroyed. All other objects and components, that are registered
                as an EventListener are forced to release their references to this object.
                The reference attributes are disposed and released also.

    @seealso    -

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::dispose() throw( css::uno::RuntimeException )
{
    // Safe impossible cases
    // It's an programming error if dispose is called before terminate!
    LOG_ASSERT2( m_bIsTerminated==sal_False, "Desktop::dispose()", "It's not allowed to dispose the desktop before terminate() is called!" )

    /*ATTENTION
        Make it threadsafe ... but this method is a special one!
        We must close objet for working BEFORE we dispose it realy ...
        After successful closing all interface calls are rejected by our
        transaction manager automaticly.
     */

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Create an exclusiv access!
    // It's neccessary for follow transaction check ...
    // Another reason: We can recylce these write lock at later time ..
    // and it's superflous to create read- and write- locks in combination.
    WriteGuard aWriteLock( m_aLock );

    // Look for multiple calls of this method!
    // If somewhere call dispose() twice - he will be stopped here realy!!!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Now - we are alone and its the first call of this method ...
    // otherwise call before must throw a DisposedException!
    // Don't forget to release this registered transaction here ...
    // because next "setWorkingMode()" call blocks till all current existing one
    // are finished!
    aTransaction.stop();

    // Disable this instance for further work.
    // This will wait for all current running ones ...
    // and reject all further requests!
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    // We should hold a reference to ourself ...
    // because our owner dispose us and release our reference ...
    // May be we will die before we could finish this method ...
    // Make snapshot of other neecessary member too.
    css::uno::Reference< css::uno::XInterface > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    // We don't need this lock any longer ... this method couldn't be called twice ... it was disabled for working!
    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // We must disable the special quit timer of our frame container.
    // Because it will call terminate at THESE instance if last task will be removed!.
    m_aChildTaskContainer.disableQuitTimer();

    // We must send the dispose message to all listener.
    // Otherwise our child tasks are disposed and try to remove it by himself at our container ...
    css::lang::EventObject aDisposeEvent( xThis         );
    m_aListenerContainer.disposeAndClear( aDisposeEvent );

    // Clear our child task container and forget all task references.
    m_aChildTaskContainer.clear();

    // Dispose our helper too.
    css::uno::Reference< css::lang::XEventListener > xDispatchHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    css::uno::Reference< css::lang::XEventListener > xFramesHelper  ( m_xFramesHelper  , css::uno::UNO_QUERY );
    if( xDispatchHelper.is() == sal_True )
    {
        xDispatchHelper->disposing( aDisposeEvent );
    }
    if( xFramesHelper.is() == sal_True )
    {
        xFramesHelper->disposing( aDisposeEvent );
    }

    // Release all member references.
    m_xDispatchHelper   = css::uno::Reference< css::frame::XDispatchProvider >();
    m_xFramesHelper     = css::uno::Reference< css::frame::XFrames >();
    m_xLastFrame        = css::uno::Reference< css::frame::XFrame >();
    m_xFactory          = css::uno::Reference< css::lang::XMultiServiceFactory >();

    // Disable object for further working.
    m_aTransactionManager.setWorkingMode( E_CLOSE );
}

/*-************************************************************************************************************//**
    @interface  XComponent
    @short      add/remove listener for dispose events
    @descr      Add an event listener to this object, if you whish to get informations
                about our dieing!
                You must releas ethis listener reference during your own disposing() method.

    @attention  Our container is threadsafe himeslf. So we doesn't need any lock here.

    @seealso    -

    @param      "xListener", reference to valid listener. We don't accept invalid values!
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_addEventListener( xListener ), "Desktop::addEventListener()", "Invalid parameter detected!" )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >*) NULL ), xListener );
}

//*****************************************************************************************************************
void SAL_CALL Desktop::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_removeEventListener( xListener ), "Desktop::removeEventListener()", "Invalid parameter detected!" )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >*) NULL ), xListener );
}

/*-************************************************************************************************************//**
    @interface  XStatusListener
    @short      callback for dispatches
    @descr      To support our method "loadComponentFromURL()" we are listener on temp. created dispatcher.
                They call us back in this method "statusChanged()". As source of given state event, they give us a
                reference to the target frame, in which dispatch was loaded! So we can use it to return his component
                to caller! If no target exist ... ??!!

    @seealso    method loadComponentFromURL()

    @param      "aEvent", state event which (hopefully) valid informations
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::statusChanged( const css::frame::FeatureStateEvent& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    LOG_ASSERT2( implcp_statusChanged( aEvent ), "Desktop::statusChanged()", "Invalid parameter detected!" )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Is message for me?
    if( aEvent.FeatureDescriptor == FEATUREDESCRIPTOR_LOADSTATE )
    {
        // Yes ... Map state of event to internal notation.
        /* SAFE AREA ------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );

        m_xLastFrame = css::uno::Reference< css::frame::XFrame >();
        m_eLoadState = E_FAILED                                   ;

        if(
            ( aEvent.IsEnabled  == sal_True     )    &&
            ( aEvent.State     >>= m_xLastFrame )
          )
        {
            m_eLoadState = E_SUCCESSFUL;
        }
        /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    }
}

/*-************************************************************************************************************//**
    @interface  XEventListener
    @short      not implemented!
    @descr      We are a status listener ... and so we must be an event listener too ... But we doesn't need it realy!
                We are a temp. listener only and our lifetime isn't smaller then of our temp. used dispatcher.

    @seealso    method loadComponentFromURL()

    @param      -
    @return     -

    @onerror    -
    @threadsafe -
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::disposing( const css::lang::EventObject& aSource ) throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::disposing()", "Algorithm error! Normaly desktop is temp. listener ... not all the time. So this method shouldn't be called." )
}

/*-************************************************************************************************************//**
    @short      try to convert a property value
    @descr      This method is called from helperclass "OPropertySetHelper".
                Don't use this directly!
                You must try to convert the value of given propertyhandle and
                return results of this operation. This will be used to ask vetoable
                listener. If no listener has a veto, we will change value realy!
                ( in method setFastPropertyValue_NoBroadcast(...) )

    @attention  Methods of OPropertySethelper are safed by using our shared osl mutex! (see ctor!)
                So we must use different locks to make our implementation threadsafe.

    @seealso    class OPropertySetHelper
    @seealso    method setFastPropertyValue_NoBroadcast()
    @seealso    method impl_tryToChangeProperty()

    @param      "aConvertedValue"   new converted value of property
    @param      "aOldValue"         old value of property
    @param      "nHandle"           handle of property
    @param      "aValue"            new value of property
    @return     sal_True if value will be changed, sal_FALSE otherway

    @onerror    IllegalArgumentException, if you call this with an invalid argument
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool SAL_CALL Desktop::convertFastPropertyValue(       css::uno::Any&   aConvertedValue ,
                                                           css::uno::Any&   aOldValue       ,
                                                           sal_Int32        nHandle         ,
                                                     const css::uno::Any&   aValue          ) throw( css::lang::IllegalArgumentException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    //  Attention: Method "impl_tryToChangeProperty()" can throw the IllegalArgumentException for us !!!

    //  Initialize state with FALSE !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case PROPERTYHANDLE_ACTIVECOMPONENT     :
        case PROPERTYHANDLE_ACTIVEFRAME         :
        case PROPERTYHANDLE_ISPLUGGED           :   bReturn = sal_False; // These variables are readonly(!) and can't be changed.
                                                    break;
/*OBSOLETE
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
*/
    }

    // Return state of operation.
    return bReturn ;
}

/*-************************************************************************************************************//**
    @short      set value of a transient property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!
                Handle and value are valid everyway! You must set the new value only.
                After this, baseclass send messages to all listener automaticly.

    @seealso    class OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    new value of property
    @return     -

    @onerror    An exception is thrown.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::setFastPropertyValue_NoBroadcast(       sal_Int32        nHandle ,
                                                         const css::uno::Any&   aValue  ) throw( css::uno::Exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    switch( nHandle )
    {
        case PROPERTYHANDLE_ACTIVECOMPONENT     :
        case PROPERTYHANDLE_ACTIVEFRAME         :
        case PROPERTYHANDLE_ISPLUGGED           :   LOG_ERROR( "Desktop::setFastPropertyValue_NoBroadcast()", "Set of readonly property not allowed." )
                                                    break;
/*OBSOLETE
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
*/
    }
}

/*-************************************************************************************************************//**
    @short      get value of a transient property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @attention  We don't need any mutex or lock here ... We use threadsafe container or methods here only!

    @seealso    class OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    current value of property
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::getFastPropertyValue( css::uno::Any& aValue  ,
                                             sal_Int32      nHandle ) const
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    switch( nHandle )
    {
        case PROPERTYHANDLE_ACTIVECOMPONENT     :   //aValue <<= getCurrentComponent(); ... not available in const method :-(
                                                    aValue <<= css::uno::Reference< css::lang::XComponent >();
                                                    break;
        case PROPERTYHANDLE_ACTIVEFRAME         :   aValue <<= m_aChildTaskContainer.getActive();
                                                    break;
        case PROPERTYHANDLE_ISPLUGGED           :   aValue <<= impl_checkPlugInState();
                                                    break;
/*OBSOLETE
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
*/
    }
}

/*-************************************************************************************************************//**
    @short      return structure and information about transient properties
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @attention  You must use global lock (method use static variable) ... and it must be the shareable osl mutex of it.
                Because; our baseclass use this mutex to make his code threadsafe. We use our lock!
                So we could have two different mutex/lock mechanism at the same object.

    @seealso    class OPropertySetHelper

    @param      -
    @return     structure with property-informations

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
::cppu::IPropertyArrayHelper& SAL_CALL Desktop::getInfoHelper()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        // Ready for multithreading
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

/*-************************************************************************************************************//**
    @short      return propertysetinfo
    @descr      You can call this method to get information about transient properties
                of this object.

    @attention  You must use global lock (method use static variable) ... and it must be the shareable osl mutex of it.
                Because; our baseclass use this mutex to make his code threadsafe. We use our lock!
                So we could have two different mutex/lock mechanism at the same object.

    @seealso    class OPropertySetHelper
    @seealso    interface XPropertySet
    @seealso    interface XMultiPropertySet

    @param      -
    @return     reference to object with information [XPropertySetInfo]

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL Desktop::getPropertySetInfo()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static css::uno::Reference< css::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

/*-************************************************************************************************************//**
    @short      return current component of current frame
    @descr      The desktop himself has no component. But every frame in subtree.
                If somewhere call getCurrentComponent() at this class, we try to find the right frame and
                then we try to become his component. It can be a VCL-component, the model or the controller
                of founded frame.

    @attention  We don't work on internal member ... so we doesn't need any lock here.

    @seealso    method getCurrentComponent();

    @param      "xFrame", reference to valid frame in hierarchy. Method is not defined for invalid values.
                But we don't check these. Its an IMPL-method and caller must use it right!
    @return     A reference to found component.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::lang::XComponent > Desktop::impl_getFrameComponent( const css::uno::Reference< css::frame::XFrame >& xFrame ) const
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Set default return value, if method failed.
    css::uno::Reference< css::lang::XComponent > xComponent;
    // Does no controller exists?
    css::uno::Reference< css::frame::XController > xController = xFrame->getController();
    if( xController.is() == sal_False )
    {
        // Controller not exist - use the VCL-component.
        xComponent = css::uno::Reference< css::lang::XComponent >( xFrame->getComponentWindow(), css::uno::UNO_QUERY );
    }
    else
    {
        // Does no model exists?
        css::uno::Reference< css::frame::XModel > xModel( xController->getModel(), css::uno::UNO_QUERY );
        if( xModel.is() == sal_True )
        {
            // Model exist - use the model as component.
            xComponent = css::uno::Reference< css::lang::XComponent >( xModel, css::uno::UNO_QUERY );
        }
        else
        {
            // Model not exist - use the controller as component.
            xComponent = css::uno::Reference< css::lang::XComponent >( xController, css::uno::UNO_QUERY );
        }
    }

    return xComponent;
}

/*-************************************************************************************************************//**
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
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool Desktop::impl_tryToChangeProperty(         sal_Bool        bProperty       ,
                                            const   css::uno::Any&  aNewValue       ,
                                                    css::uno::Any&  aOldValue       ,
                                                    css::uno::Any&  aConvertedValue ) throw( css::lang::IllegalArgumentException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Set default return value.
    sal_Bool bReturn = sal_False;

    // Clear information of return parameter!
    aOldValue.clear();
    aConvertedValue.clear();

    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    sal_Bool bNewValue;
    ::cppu::convertPropertyValue( bNewValue, aNewValue );

    // If value change ...
    if( bNewValue != bProperty )
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

/*-************************************************************************************************************//**
    @short      create table with information about properties
    @descr      We use a helper class to support properties. These class need some information about this.
                These method create a new static description table with name, type, r/w-flags and so on ...

    @seealso    class OPropertySetHelper
    @seealso    method getInfoHelper()

    @param      -
    @return     Static table with information about properties.

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
const css::uno::Sequence< css::beans::Property > Desktop::impl_getStaticPropertyDescriptor()
{
    // Create a new static property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    static const css::beans::Property pProperties[] =
    {
        css::beans::Property( PROPERTYNAME_ACTIVECOMPONENT, PROPERTYHANDLE_ACTIVECOMPONENT, ::getCppuType((const css::uno::Reference< css::lang::XComponent >*)NULL), css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( PROPERTYNAME_ACTIVEFRAME    , PROPERTYHANDLE_ACTIVEFRAME    , ::getCppuType((const css::uno::Reference< css::lang::XComponent >*)NULL), css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( PROPERTYNAME_ISPLUGGED      , PROPERTYHANDLE_ISPLUGGED      , ::getBooleanCppuType()                                                  , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        /*OBSOLETE
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
        */
    };
    // Use it to initialize sequence!
    static const css::uno::Sequence< css::beans::Property > lPropertyDescriptor( pProperties, PROPERTYCOUNT );
    // Return static "PropertyDescriptor"
    return lPropertyDescriptor;
}

/*-************************************************************************************************************//**
    @short      work with our terminate listener
    @descr      If somewhere call terminate on this object, we must notify our listener. They could throw a veto exception
                to break it or accept it by doing nothing. These two helper methods send right events to all registered
                listener.

    @attention  We don't need any lock here - our container is threadsafe himself!

    @seealso    method terminate()

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void Desktop::impl_sendQueryTerminationEvent() throw( css::frame::TerminationVetoException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Send QueryTermination event to all listener.
    // Get container for right listener.
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ) );
    if( pContainer != NULL )
    {
        // Build QueryTermination event.
        css::lang::EventObject aEvent( static_cast< ::cppu::OWeakObject* >(this) );
        // Get iterator for access to listener.
        ::cppu::OInterfaceIteratorHelper aIterator( *pContainer );
        // Send message to all listener.
        // Somewhere can throw a TerminationVetoException.
        // We don't look for that(!) ... caller of this method will catch these.
        while( aIterator.hasMoreElements() == sal_True )
        {
            ((css::frame::XTerminateListener*)aIterator.next())->queryTermination( aEvent );
        }
    }
}

//*****************************************************************************************************************
void Desktop::impl_sendNotifyTerminationEvent()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Send NotifyTermination event to all listener.
    // Get container for right listener.
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ) );
    if( pContainer != NULL )
    {
        // Build QueryTermination event.
        css::lang::EventObject aEvent( static_cast< ::cppu::OWeakObject* >(this) );
        // Get iterator for access to listener.
        ::cppu::OInterfaceIteratorHelper aIterator( *pContainer );
        // Send message to all listener.
        while( aIterator.hasMoreElements() == sal_True )
        {
            ((css::frame::XTerminateListener*)aIterator.next())->notifyTermination( aEvent );
        }
    }
}

/*-************************************************************************************************************//**
    @short      search for any plugin frame to return current plugin state
    @descr      For property "IsPlugged" we need information about this state.
                We search for any plugin frame in our container (it can be tasks or plugin-frames only!).
                If we found somewhere we return TRUE, FALSE otherwise.

    @attention  We don't need any lock here - our container is threadsafe himself!

    @seealso    property IsPlugged

    @param      -
    @return     sal_True, if a plugin frame exist, sal_False otherwise.

    @onerror    We return sal_False.
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool Desktop::impl_checkPlugInState() const
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // set default return value if search failed or no plugin could be detected.
    sal_Bool bPluginExist = sal_False;

    // We must search at ouer childs. We make a flat search at our direct children only.
    // Break loop, if something was found or all container items was compared.
    css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > lTasks    = m_aChildTaskContainer.getAllElements();
    sal_uInt32                                                      nCount    = lTasks.getLength();
    sal_uInt32                                                      nPosition = 0;
    while(
            ( bPluginExist  ==  sal_False   )   &&
            ( nPosition     <   nCount      )
         )
    {
        css::uno::Reference< css::mozilla::XPluginInstance > xPlugInFrame( lTasks[nPosition], css::uno::UNO_QUERY );
        bPluginExist = xPlugInFrame.is();
        ++nPosition;
    }

    return bPluginExist;
}

//_________________________________________________________________________________________________________________
//  debug methods
//_________________________________________________________________________________________________________________

/*-----------------------------------------------------------------------------------------------------------------
    The follow methods checks the parameter for other functions. If a parameter or his value is non valid,
    we return "sal_True". (otherwise sal_False) This mechanism is used to throw an ASSERT!
-----------------------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_ASSERTIONS

//*****************************************************************************************************************
//  We work with valid servicemanager only.
sal_Bool Desktop::implcp_ctor( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
{
    return(
            ( &xFactory     ==  NULL        )   ||
            ( xFactory.is() ==  sal_False   )
          );
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::implcp_addTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener )
{
    return(
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
          );
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::implcp_removeTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener )
{
    return(
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
          );
}

//*****************************************************************************************************************
//  The target frame could be ""(!), but flags must be in range of right enum.
sal_Bool Desktop::implcp_findFrame( const ::rtl::OUString&  sTargetFrameName,
                                          sal_Int32         nSearchFlags    )
{
    return(
            ( &sTargetFrameName == NULL )   ||
            (
                (    nSearchFlags != css::frame::FrameSearchFlag::AUTO        ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::PARENT    ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::SELF      ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::CHILDREN  ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::CREATE    ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::SIBLINGS  ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::TASKS     ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::ALL       ) ) &&
                ( !( nSearchFlags &  css::frame::FrameSearchFlag::GLOBAL    ) )
            )
          );
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::implcp_addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return(
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
          );
}

//*****************************************************************************************************************
//  We work with valid listener only.
sal_Bool Desktop::implcp_removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return(
            ( &xListener        ==  NULL        )   ||
            ( xListener.is()    ==  sal_False   )
          );
}

//*****************************************************************************************************************
sal_Bool Desktop::implcp_statusChanged( const css::frame::FeatureStateEvent& aEvent )
{
    return(
            ( &aEvent                               ==  NULL    )   ||
            ( aEvent.FeatureDescriptor.getLength()  <   1       )
          );
}

#endif  // #ifdef ENABLE_ASSERTIONS

}   // namespace framework
