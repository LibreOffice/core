/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <services/desktop.hxx>

#include <loadenv/loadenv.hxx>
#include <loadenv/targethelper.hxx>

#include <helper/ocomponentaccess.hxx>
#include <helper/oframes.hxx>
#include <dispatch/dispatchprovider.hxx>

#include <dispatch/interceptionhelper.hxx>
#include <classes/taskcreator.hxx>
#include <threadhelp/transactionguard.hxx>
#include <general.h>
#include <properties.h>

#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/XTerminateListener2.hpp>

#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/instance.hxx>
#include <vcl/svapp.hxx>
#include <desktop/crashreport.hxx>
#include <vcl/scheduler.hxx>

#include <tools/errinf.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/extract.hxx>

namespace framework{

enum PropHandle {
    ActiveFrame, DispatchRecorderSupplier, IsPlugged, SuspendQuickstartVeto,
    Title };

OUString SAL_CALL Desktop::getImplementationName()
{
    return OUString("com.sun.star.comp.framework.Desktop");
}

sal_Bool SAL_CALL Desktop::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL Desktop::getSupportedServiceNames()
{
    return { "com.sun.star.frame.Desktop" };
}

void Desktop::constructorInit()
{
    // Initialize a new XFrames-helper-object to handle XIndexAccess and XElementAccess.
    // We hold member as reference ... not as pointer too!
    // Attention: We share our frame container with this helper. Container is threadsafe himself ... So I think we can do that.
    // But look on dispose() for right order of deinitialization.
    OFrames* pFramesHelper = new OFrames( this, &m_aChildTaskContainer );
    m_xFramesHelper.set( static_cast< ::cppu::OWeakObject* >(pFramesHelper), css::uno::UNO_QUERY );

    // Initialize a new dispatchhelper-object to handle dispatches.
    // We use these helper as slave for our interceptor helper ... not directly!
    // But he is event listener on THIS instance!
    DispatchProvider* pDispatchHelper = new DispatchProvider( m_xContext, this );
    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( static_cast< ::cppu::OWeakObject* >(pDispatchHelper), css::uno::UNO_QUERY );

    // Initialize a new interception helper object to handle dispatches and implement an interceptor mechanism.
    // Set created dispatch provider as slowest slave of it.
    // Hold interception helper by reference only - not by pointer!
    // So it's easier to destroy it.
    InterceptionHelper* pInterceptionHelper = new InterceptionHelper( this, xDispatchProvider );
    m_xDispatchHelper.set( static_cast< ::cppu::OWeakObject* >(pInterceptionHelper), css::uno::UNO_QUERY );

    OUStringBuffer sUntitledPrefix (256);
    sUntitledPrefix.append      (FWK_RESSTR(STR_UNTITLED_DOCUMENT));
    sUntitledPrefix.append (" ");

    ::comphelper::NumberedCollection* pNumbers = new ::comphelper::NumberedCollection ();
    m_xTitleNumberGenerator.set(static_cast< ::cppu::OWeakObject* >(pNumbers), css::uno::UNO_QUERY_THROW);
    pNumbers->setOwner          ( static_cast< ::cppu::OWeakObject* >(this) );
    pNumbers->setUntitledPrefix ( sUntitledPrefix.makeStringAndClear ()     );

    // Safe impossible cases
    // We can't work without this helper!
    SAL_WARN_IF( !m_xFramesHelper.is(), "fwk.desktop", "Desktop::Desktop(): Frames helper is not valid. XFrames, XIndexAccess and XElementAcces are not supported!");
    SAL_WARN_IF( !m_xDispatchHelper.is(), "fwk.desktop", "Desktop::Desktop(): Dispatch helper is not valid. XDispatch will not work correctly!" );

    // Enable object for real working!
    // Otherwise all calls will be rejected ...
    m_aTransactionManager.setWorkingMode( E_WORK );
}

/*-************************************************************************************************************
    @short      standard constructor to create instance by factory
    @descr      This constructor initialize a new instance of this class by valid factory,
                and will be set valid values on his member and baseclasses.

    @attention  a)  Don't use your own reference during an UNO-Service-ctor! There is no guarantee, that you
                    will get over this. (e.g. using of your reference as parameter to initialize some member)
                    Do such things in DEFINE_INIT_SERVICE() method, which is called automatically after your ctor!!!
                b)  Baseclass OBroadcastHelper is a typedef in namespace cppu!
                    The microsoft compiler has some problems to handle it right BY using namespace explicitly ::cppu::OBroadcastHelper.
                    If we write it without a namespace or expand the typedef to OBroadcastHelperVar<...> -> it will be OK!?
                    I don't know why! (other compiler not tested .. but it works!)

    @seealso    method DEFINE_INIT_SERVICE()

    @param      "xFactory" is the multi service manager, which create this instance.
                The value must be different from NULL!
    @onerror    We throw an ASSERT in debug version or do nothing in release version.
*//*-*************************************************************************************************************/
Desktop::Desktop( const css::uno::Reference< css::uno::XComponentContext >& xContext )
        :   TransactionBase         (                                               )
        ,   Desktop_BASE            ( m_aMutex )
        ,   cppu::OPropertySetHelper( cppu::WeakComponentImplHelperBase::rBHelper   )
        // Init member
        ,   m_bIsTerminated         ( false                                     )   // see dispose() for further information!
        ,   m_xContext              ( xContext                                      )
        ,   m_aChildTaskContainer   (                                               )
        ,   m_aListenerContainer    ( m_aMutex )
        ,   m_xFramesHelper         (                                               )
        ,   m_xDispatchHelper       (                                               )
        ,   m_eLoadState            ( E_NOTSET                                      )
        ,   m_xLastFrame            (                                               )
        ,   m_aInteractionRequest   (                                               )
        ,   m_bSuspendQuickstartVeto( false                                     )
        ,   m_sName                 (                                               )
        ,   m_sTitle                (                                               )
        ,   m_xDispatchRecorderSupplier(                                            )
        ,   m_xPipeTerminator       (                                               )
        ,   m_xQuickLauncher        (                                               )
        ,   m_xStarBasicQuitGuard   (                                               )
        ,   m_xSWThreadManager      (                                               )
        ,   m_xSfxTerminator        (                                               )
        ,   m_xTitleNumberGenerator (                                               )
{
}

/*-************************************************************************************************************
    @short      standard destructor
    @descr      This one do NOTHING! Use dispose() instaed of this.

    @seealso    method dispose()
*//*-*************************************************************************************************************/
Desktop::~Desktop()
{
    SAL_WARN_IF( !m_bIsTerminated, "fwk.desktop", "Desktop not terminated before being destructed" );
    SAL_WARN_IF( m_aTransactionManager.getWorkingMode()!=E_CLOSE, "fwk.desktop", "Desktop::~Desktop(): Who forgot to dispose this service?" );
}

css::uno::Any SAL_CALL Desktop::queryInterface( const css::uno::Type& _rType )
{
    css::uno::Any aRet = Desktop_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    return aRet;
}

css::uno::Sequence< css::uno::Type > SAL_CALL Desktop::getTypes(  )
{
    return comphelper::concatSequences(
        Desktop_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}

sal_Bool SAL_CALL Desktop::terminate()
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexClearableGuard aReadLock;

    css::uno::Reference< css::frame::XTerminateListener > xPipeTerminator    = m_xPipeTerminator;
    css::uno::Reference< css::frame::XTerminateListener > xQuickLauncher     = m_xQuickLauncher;
    css::uno::Reference< css::frame::XTerminateListener > xStarBasicQuitGuard = m_xStarBasicQuitGuard;
    css::uno::Reference< css::frame::XTerminateListener > xSWThreadManager   = m_xSWThreadManager;
    css::uno::Reference< css::frame::XTerminateListener > xSfxTerminator     = m_xSfxTerminator;

    css::lang::EventObject                                aEvent             ( static_cast< ::cppu::OWeakObject* >(this) );
    bool                                                  bAskQuickStart     = !m_bSuspendQuickstartVeto;

    aReadLock.clear();

    // try to close all open frames.
    // Allow using of any UI ... because Desktop.terminate() was designed as UI functionality in the past.
    bool bIsEventTestingMode = Application::IsEventTestingModeEnabled();
    bool bFramesClosed = impl_closeFrames(!bIsEventTestingMode);

    // Ask normal terminate listener. They could stop terminating the process.
    Desktop::TTerminateListenerList lCalledTerminationListener;
    bool bVeto = false;
    impl_sendQueryTerminationEvent(lCalledTerminationListener, bVeto);
    if ( bVeto )
    {
        impl_sendCancelTerminationEvent(lCalledTerminationListener);
        return false;
    }

    if (bIsEventTestingMode)
    {
        Application::Quit();
        return true;
    }
    if ( ! bFramesClosed )
    {
        impl_sendCancelTerminationEvent(lCalledTerminationListener);
        return false;
    }

    // Normal listener had no problem ...
    // all frames was closed ...
    // now it's time to ask our specialized listener.
    // They are handled these way because they wish to hinder the office on termination
    // but they wish also closing of all frames.

    // Note further:
    //    We shouldn't ask quicklauncher in case it was allowed from outside only.
    //    This is special trick to "ignore existing quick starter" for debug purposes.

    // Attention:
    // Order of called listener is important!
    // Some of them are harmless,-)
    // but some can be dangerous. E.g. it would be dangerous if we close our pipe
    // and don't terminate in real because another listener throws a veto exception .-)

    bool bTerminate = false;
    try
    {
        if(
            ( bAskQuickStart      ) &&
            ( xQuickLauncher.is() )
          )
        {
            xQuickLauncher->queryTermination( aEvent );
            lCalledTerminationListener.push_back( xQuickLauncher );
        }

        if ( xStarBasicQuitGuard.is() )
        {
            xStarBasicQuitGuard->queryTermination( aEvent );
            lCalledTerminationListener.push_back( xStarBasicQuitGuard );
        }

        if ( xSWThreadManager.is() )
        {
            xSWThreadManager->queryTermination( aEvent );
            lCalledTerminationListener.push_back( xSWThreadManager );
        }

        if ( xPipeTerminator.is() )
        {
            xPipeTerminator->queryTermination( aEvent );
            lCalledTerminationListener.push_back( xPipeTerminator );
        }

        if ( xSfxTerminator.is() )
        {
            xSfxTerminator->queryTermination( aEvent );
            lCalledTerminationListener.push_back( xSfxTerminator );
        }

        bTerminate = true;
    }
    catch(const css::frame::TerminationVetoException&)
    {
        bTerminate = false;
    }

    if ( ! bTerminate )
        impl_sendCancelTerminationEvent(lCalledTerminationListener);
    else
    {
        // "Protect" us against dispose before terminate calls!
        // see dispose() for further information.
        /* SAFE AREA --------------------------------------------------------------------------------------- */
        SolarMutexClearableGuard aWriteLock;
        CrashReporter::AddKeyValue("ShutDown", OUString::boolean(true));
        m_bIsTerminated = true;
        aWriteLock.clear();
        /* UNSAFE AREA ------------------------------------------------------------------------------------- */

        impl_sendNotifyTerminationEvent();
        {
            SolarMutexGuard aGuard;
            Scheduler::ProcessEventsToIdle();
        }

        if(
            ( bAskQuickStart      ) &&
            ( xQuickLauncher.is() )
          )
        {
            xQuickLauncher->notifyTermination( aEvent );
        }

        if ( xStarBasicQuitGuard.is() )
            xStarBasicQuitGuard->notifyTermination( aEvent );

        if ( xSWThreadManager.is() )
            xSWThreadManager->notifyTermination( aEvent );

        if ( xPipeTerminator.is() )
            xPipeTerminator->notifyTermination( aEvent );

        // we need a copy here as the notifyTermination call might cause a removeTerminateListener call
        std::vector< css::uno::Reference<css::frame::XTerminateListener> > xComponentDllListeners = m_xComponentDllListeners;
        for (auto& xListener : xComponentDllListeners)
        {
            xListener->notifyTermination(aEvent);
        }
        m_xComponentDllListeners.clear();

        // Must be really the last listener to be called.
        // Because it shutdown the whole process asynchronous !
        if ( xSfxTerminator.is() )
            xSfxTerminator->notifyTermination( aEvent );
    }

    return bTerminate;
}

namespace
{
    class QuickstartSuppressor
    {
        Desktop* const m_pDesktop;
        css::uno::Reference< css::frame::XTerminateListener > m_xQuickLauncher;
        public:
            QuickstartSuppressor(Desktop* const pDesktop, css::uno::Reference< css::frame::XTerminateListener > const & xQuickLauncher)
                : m_pDesktop(pDesktop)
                , m_xQuickLauncher(xQuickLauncher)
            {
                SAL_INFO("fwk.desktop", "temporary removing Quickstarter");
                if(m_xQuickLauncher.is())
                    m_pDesktop->removeTerminateListener(m_xQuickLauncher);
            }
            ~QuickstartSuppressor()
            {
                SAL_INFO("fwk.desktop", "readding Quickstarter");
                if(m_xQuickLauncher.is())
                    m_pDesktop->addTerminateListener(m_xQuickLauncher);
            }
    };
}

bool SAL_CALL Desktop::terminateQuickstarterToo()
{
    QuickstartSuppressor aQuickstartSuppressor(this, m_xQuickLauncher);
    return terminate();
}

void SAL_CALL Desktop::addTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::lang::XServiceInfo > xInfo( xListener, css::uno::UNO_QUERY );
    if ( xInfo.is() )
    {
        OUString sImplementationName = xInfo->getImplementationName();

        SolarMutexGuard g;

        if( sImplementationName == "com.sun.star.comp.sfx2.SfxTerminateListener" )
        {
            m_xSfxTerminator = xListener;
            return;
        }
        if( sImplementationName == "com.sun.star.comp.RequestHandlerController" )
        {
            m_xPipeTerminator = xListener;
            return;
        }
        if( sImplementationName == "com.sun.star.comp.desktop.QuickstartWrapper" )
        {
            m_xQuickLauncher = xListener;
            return;
        }
        if( sImplementationName == "com.sun.star.comp.svx.StarBasicQuitGuard" )
        {
            m_xStarBasicQuitGuard = xListener;
            return;
        }
        if( sImplementationName == "com.sun.star.util.comp.FinalThreadManager" )
        {
            m_xSWThreadManager = xListener;
            return;
        }
        else if ( sImplementationName == "com.sun.star.comp.ComponentDLLListener" )
        {
            m_xComponentDllListeners.push_back(xListener);
            return;
        }
    }

    // No lock required ... container is threadsafe by itself.
    m_aListenerContainer.addInterface( cppu::UnoType<css::frame::XTerminateListener>::get(), xListener );
}

void SAL_CALL Desktop::removeTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    css::uno::Reference< css::lang::XServiceInfo > xInfo( xListener, css::uno::UNO_QUERY );
    if ( xInfo.is() )
    {
        OUString sImplementationName = xInfo->getImplementationName();

        SolarMutexGuard g;

        if( sImplementationName == "com.sun.star.comp.sfx2.SfxTerminateListener" )
        {
            m_xSfxTerminator.clear();
            return;
        }

        if( sImplementationName == "com.sun.star.comp.RequestHandlerController" )
        {
            m_xPipeTerminator.clear();
            return;
        }

        if( sImplementationName == "com.sun.star.comp.desktop.QuickstartWrapper" )
        {
            m_xQuickLauncher.clear();
            return;
        }

        if( sImplementationName == "com.sun.star.comp.svx.StarBasicQuitGuard" )
        {
            m_xStarBasicQuitGuard.clear();
            return;
        }

        if( sImplementationName == "com.sun.star.util.comp.FinalThreadManager" )
        {
            m_xSWThreadManager.clear();
            return;
        }
        else if (sImplementationName == "com.sun.star.comp.ComponentDLLListener")
        {
            m_xComponentDllListeners.erase(
                    std::remove(m_xComponentDllListeners.begin(), m_xComponentDllListeners.end(), xListener),
                    m_xComponentDllListeners.end());
            return;
        }
    }

    // No lock required ... container is threadsafe by itself.
    m_aListenerContainer.removeInterface( cppu::UnoType<css::frame::XTerminateListener>::get(), xListener );
}

/*-************************************************************************************************************
    @interface  XDesktop
    @short      get access to create enumerations of all current components
    @descr      You will be the owner of the returned object and must delete it if you don't use it again.

    @seealso    class TasksAccess
    @seealso    class TasksEnumeration
    @return     A reference to an XEnumerationAccess-object.

    @onerror    We return a null-reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL Desktop::getComponents()
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

/*-************************************************************************************************************
    @interface  XDesktop
    @short      return the current active component
    @descr      The most current component is the window, model or the controller of the current active frame.

    @seealso    method getCurrentFrame()
    @seealso    method impl_getFrameComponent()
    @return     A reference to the component.

    @onerror    We return a null-reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::lang::XComponent > SAL_CALL Desktop::getCurrentComponent()
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
    if( xCurrentFrame.is() )
    {
        xComponent = impl_getFrameComponent( xCurrentFrame );
    }
    return xComponent;
}

/*-************************************************************************************************************
    @interface  XDesktop
    @short      return the current active frame in hierarchy
    @descr      There can be more than one different active paths in our frame hierarchy. But only one of them
                could be the most active frame (normal he has the focus).
                Don't mix it with getActiveFrame()! That will return our current active frame, which must be
                a direct child of us and should be a part(!) of an active path.

    @seealso    method getActiveFrame()
    @return     A valid reference, if there is an active frame.
                A null reference , otherwise.

    @onerror    We return a null reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::getCurrentFrame()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Start search with our direct active frame (if it exist!).
    // Search on his children for other active frames too.
    // Stop if no one could be found and return last of found ones.
    css::uno::Reference< css::frame::XFramesSupplier > xLast( getActiveFrame(), css::uno::UNO_QUERY );
    if( xLast.is() )
    {
        css::uno::Reference< css::frame::XFramesSupplier > xNext( xLast->getActiveFrame(), css::uno::UNO_QUERY );
        while( xNext.is() )
        {
            xLast = xNext;
            xNext.set( xNext->getActiveFrame(), css::uno::UNO_QUERY );
        }
    }
    return css::uno::Reference< css::frame::XFrame >( xLast, css::uno::UNO_QUERY );
}

/*-************************************************************************************************************
    @interface  XComponentLoader
    @short      try to load given URL into a task
    @descr      You can give us some information about the content, which you will load into a frame.
                We search or create this target for you, make a type detection of given URL and try to load it.
                As result of this operation we return the new created component or nothing, if loading failed.
    @param      "sURL"              , URL, which represent the content
    @param      "sTargetFrameName"  , name of target frame or special value like "_self", "_blank" ...
    @param      "nSearchFlags"      , optional arguments for frame search, if target isn't a special one
    @param      "lArguments"        , optional arguments for loading
    @return     A valid component reference, if loading was successfully.
                A null reference otherwise.

    @onerror    We return a null reference.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::lang::XComponent > SAL_CALL Desktop::loadComponentFromURL( const OUString&                                 sURL            ,
                                                                                     const OUString&                                 sTargetFrameName,
                                                                                           sal_Int32                                        nSearchFlags    ,
                                                                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments      )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    SAL_INFO( "fwk.desktop", "loadComponentFromURL" );

    css::uno::Reference< css::frame::XComponentLoader > xThis(static_cast< css::frame::XComponentLoader* >(this), css::uno::UNO_QUERY);

    return LoadEnv::loadComponentFromURL(xThis, m_xContext, sURL, sTargetFrameName, nSearchFlags, lArguments);
}

/*-************************************************************************************************************
    @interface  XTasksSupplier
    @short      get access to create enumerations of our taskchildren
    @descr      Direct children of desktop are tasks every time.
                Call these method to could create enumerations of it.

But; Don't forget - you will be the owner of returned object and must release it!
                We use a helper class to implement the access interface. They hold a weakreference to us.
                It can be, that the desktop is dead - but not your tasksaccess-object! Then they will do nothing!
                You can't create enumerations then.

    @attention  Normally we don't need any lock here. We don't work on internal member!

    @seealso    class TasksAccess
    @return     A reference to an accessobject, which can create enumerations of our childtasks.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL Desktop::getTasks()
{
    SAL_INFO("fwk.desktop", "Desktop::getTasks(): Use of obsolete interface XTaskSupplier");
    return nullptr;
}

/*-************************************************************************************************************
    @interface  XTasksSupplier
    @short      return current active task of our direct children
    @descr      Desktop children are tasks only ! If we have an active path from desktop
                as top to any frame on bottom, we must have an active direct child. His reference is returned here.

    @attention  a)  Do not confuse it with getCurrentFrame()! The current frame don't must one of our direct children.
                    It can be every frame in subtree and must have the focus (Is the last one of an active path!).
                b)  We don't need any lock here. Our container is threadsafe himself and live, if we live!

    @seealso    method getCurrentFrame()
    @return     A reference to our current active taskchild.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XTask > SAL_CALL Desktop::getActiveTask()
{
    SAL_INFO("fwk.desktop", "Desktop::getActiveTask(): Use of obsolete interface XTaskSupplier");
    return nullptr;
}

/*-************************************************************************************************************
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
                                                                              const OUString& sTargetFrameName ,
                                                                                    sal_Int32        nSearchFlags     )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Remove uno and cmd protocol part as we want to support both of them. We store only the command part
    // in our hash map. All other protocols are stored with the protocol part.
    OUString aCommand( aURL.Main );
    if ( aURL.Protocol.equalsIgnoreAsciiCase(".uno:") )
        aCommand = aURL.Path;

    if (!m_xCommandOptions && !utl::ConfigManager::IsAvoidConfig())
        m_xCommandOptions.reset(new SvtCommandOptions);

    // Make std::unordered_map lookup if the current URL is in the disabled list
    if (m_xCommandOptions && m_xCommandOptions->Lookup(SvtCommandOptions::CMDOPTION_DISABLED, aCommand))
        return css::uno::Reference< css::frame::XDispatch >();
    else
    {
        // We use a helper to support these interface and an interceptor mechanism.
        // Our helper is threadsafe by himself!
        return m_xDispatchHelper->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    }
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL Desktop::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lQueries )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_xDispatchHelper->queryDispatches( lQueries );
}

/*-************************************************************************************************************
    @interface  XDispatchProviderInterception
    @short      supports registration/deregistration of interception objects, which
                are interested on special dispatches.

    @descr      Its really provided by an internal helper, which is used inside the dispatch api too.
    @param      xInterceptor
                the interceptor object, which wish to be (de)registered.

    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->registerDispatchProviderInterceptor( xInterceptor );
}

void SAL_CALL Desktop::releaseDispatchProviderInterceptor ( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->releaseDispatchProviderInterceptor( xInterceptor );
}

/*-************************************************************************************************************
    @interface  XFramesSupplier
    @short      return access to append or remove children on desktop
    @descr      We don't implement these interface directly. We use a helper class to do this.
                If you wish to add or delete children to/from the container, call these method to get
                a reference to the helper.

    @attention  Helper is threadsafe himself. So we don't need any lock here.

    @seealso    class OFrames
    @return     A reference to the helper.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
css::uno::Reference< css::frame::XFrames > SAL_CALL Desktop::getFrames()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_xFramesHelper;
}

/*-************************************************************************************************************
    @interface  XFramesSupplier
    @short      set/get the current active child frame
    @descr      It must be a task. Direct children of desktop are tasks only! No frames are accepted.
                We don't save this information directly in this class. We use our container-helper
                to do that.

    @attention  Helper is threadsafe himself. So we don't need any lock here.

    @seealso    class OFrameContainer

    @param      "xFrame", new active frame (must be valid!)
    @return     A reference to our current active childtask, if anyone exist.

    @onerror    A null reference is returned.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Get old active frame first.
    // If nothing will change - do nothing!
    // Otherwise set new active frame ...
    // and deactivate last frame.
    // It's necessary for our FrameActionEvent listener on a frame!
    css::uno::Reference< css::frame::XFrame > xLastActiveChild = m_aChildTaskContainer.getActive();
    if( xLastActiveChild != xFrame )
    {
        m_aChildTaskContainer.setActive( xFrame );
        if( xLastActiveChild.is() )
        {
            xLastActiveChild->deactivate();
        }
    }
}

css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::getActiveFrame()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_aChildTaskContainer.getActive();
}

/*
    @interface  XFrame
    @short      non implemented methods!
    @descr      Some method make no sense for our desktop! He has no window or parent or ...
                So we should implement it empty and warn programmer, if he use it!
*/
void SAL_CALL Desktop::initialize( const css::uno::Reference< css::awt::XWindow >& )
{
}

css::uno::Reference< css::awt::XWindow > SAL_CALL Desktop::getContainerWindow()
{
    return css::uno::Reference< css::awt::XWindow >();
}

void SAL_CALL Desktop::setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& /*xCreator*/ )
{
}

css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL Desktop::getCreator()
{
    return css::uno::Reference< css::frame::XFramesSupplier >();
}

OUString SAL_CALL Desktop::getName()
{
    SolarMutexGuard g;
    return m_sName;
}

void SAL_CALL Desktop::setName( const OUString& sName )
{
    SolarMutexGuard g;
    m_sName = sName;
}

sal_Bool SAL_CALL Desktop::isTop()
{
    return true;
}

void SAL_CALL Desktop::activate()
{
    // Desktop is active always... but sometimes our frames try to activate
    // the complete path from bottom to top... And our desktop is the topest frame :-(
    // So - please don't show any assertions here. Do nothing!
}

void SAL_CALL Desktop::deactivate()
{
    // Desktop is active always... but sometimes our frames try to deactivate
    // the complete path from bottom to top... And our desktop is the topest frame :-(
    // So - please don't show any assertions here. Do nothing!
}

sal_Bool SAL_CALL Desktop::isActive()
{
    return true;
}

sal_Bool SAL_CALL Desktop::setComponent( const css::uno::Reference< css::awt::XWindow >&       /*xComponentWindow*/ ,
                                         const css::uno::Reference< css::frame::XController >& /*xController*/      )
{
    return false;
}

css::uno::Reference< css::awt::XWindow > SAL_CALL Desktop::getComponentWindow()
{
    return css::uno::Reference< css::awt::XWindow >();
}

css::uno::Reference< css::frame::XController > SAL_CALL Desktop::getController()
{
    return css::uno::Reference< css::frame::XController >();
}

void SAL_CALL Desktop::contextChanged()
{
}

void SAL_CALL Desktop::addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& )
{
}

//   css::frame::XFrame
void SAL_CALL Desktop::removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& )
{
}

/*-************************************************************************************************************
    @interface  XFrame
    @short      try to find a frame with special parameters
    @descr      This method searches for a frame with the specified name.
                Frames may contain other frames (e.g. a frameset) and may
                be contained in other frames. This hierarchy is searched by
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
css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::findFrame( const OUString& sTargetFrameName ,
                                                                             sal_Int32        nSearchFlags     )
{
    css::uno::Reference< css::frame::XFrame > xTarget;

    // 0) Ignore wrong parameter!
    //    We don't support search for following special targets.
    //    If we reject these requests, we must not check for such names
    //    in following code again and again. If we do not, so wrong
    //    search results can occur!

    if (
        (sTargetFrameName==SPECIALTARGET_DEFAULT  )   ||    // valid for dispatches - not for findFrame()!
        (sTargetFrameName==SPECIALTARGET_PARENT   )   ||    // we have no parent by definition
        (sTargetFrameName==SPECIALTARGET_BEAMER   )         // beamer frames are allowed as child of tasks only -
                                                            // and they exist more than ones. We have no idea which our sub tasks is the right one
       )
    {
        return nullptr;
    }

    // I) check for special defined targets first which must be handled exclusive.
    //    force using of "if() else if() ..."

    // I.I) "_blank"
    //  create a new task as child of this desktop instance
    //  Note: Used helper TaskCreator use us automatically ...

    if ( sTargetFrameName==SPECIALTARGET_BLANK )
    {
        TaskCreator aCreator( m_xContext );
        xTarget = aCreator.createTask(sTargetFrameName);
    }

    // I.II) "_top"
    //  We are top by definition

    else if ( sTargetFrameName==SPECIALTARGET_TOP )
    {
        xTarget = this;
    }

    // I.III) "_self", ""
    //  This mean this "frame" in every case.

    else if (
             ( sTargetFrameName==SPECIALTARGET_SELF ) ||
             ( sTargetFrameName.isEmpty()           )
            )
    {
        xTarget = this;
    }

    else
    {

        // II) otherwise use optional given search flags
        //  force using of combinations of such flags. means no "else" part of use if() statements.
        //  But we ust break further searches if target was already found.
        //  Order of using flags is fix: SELF - CHILDREN - SIBLINGS - PARENT
        //  TASK and CREATE are handled special.
        //  But note: Such flags are not valid for the desktop - especially SIBLINGS or PARENT.

        // II.I) SELF
        //  Check for right name. If it's the searched one return ourself - otherwise
        //  ignore this flag.

        if (
            (nSearchFlags &  css::frame::FrameSearchFlag::SELF)  &&
            (m_sName == sTargetFrameName)
           )
        {
            xTarget = this;
        }

        // II.II) TASKS
        //  This is a special flag. Normally it regulate search inside tasks and forbid access to parent trees.
        //  But the desktop exists outside such task trees. They are our sub trees. So the desktop implement
        //  a special feature: We use it to start search on our direct children only. That means we suppress
        //  search on ALL child frames. May that can be useful to get access on opened document tasks
        //  only without filter out all non really required sub frames ...
        //  Used helper method on our container doesn't create any frame - it's a search only.

        if (
            ( ! xTarget.is()                                  ) &&
            (nSearchFlags & css::frame::FrameSearchFlag::TASKS)
           )
        {
            xTarget = m_aChildTaskContainer.searchOnDirectChildrens(sTargetFrameName);
        }

        // II.III) CHILDREN
        //  Search on all children for the given target name.
        //  An empty name value can't occur here - because it must be already handled as "_self"
        //  before. Used helper function of container doesn't create any frame.
        //  It makes a deep search only.

        if (
            ( ! xTarget.is()                                     ) &&
            (nSearchFlags & css::frame::FrameSearchFlag::CHILDREN)
           )
        {
            xTarget = m_aChildTaskContainer.searchOnAllChildrens(sTargetFrameName);
        }

        // II.IV) CREATE
        //  If we haven't found any valid target frame by using normal flags - but user allowed us to create
        //  a new one ... we should do that. Used TaskCreator use us automatically as parent!

        if (
            ( ! xTarget.is()                                   )    &&
            (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
           )
        {
            TaskCreator aCreator( m_xContext );
            xTarget = aCreator.createTask(sTargetFrameName);
        }
    }

    return xTarget;
}

void SAL_CALL Desktop::disposing()
{
    // Safe impossible cases
    // It's an programming error if dispose is called before terminate!

    // But if you just ignore the assertion (which happens in unit
    // tests for instance in sc/qa/unit) nothing bad happens.
    SAL_WARN_IF( !m_bIsTerminated, "fwk.desktop", "Desktop disposed before terminating it" );

    SolarMutexClearableGuard aWriteLock;

    {
        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    }

    // Disable this instance for further work.
    // This will wait for all current running transactions ...
    // and reject all new incoming requests!
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    aWriteLock.clear();

    // Following lines of code can be called outside a synchronized block ...
    // Because our transaction manager will block all new requests to this object.
    // So nobody can use us any longer.
    // Exception: Only removing of listener will work ... and this code can't be dangerous.

    // First we have to kill all listener connections.
    // They might rely on our member and can hinder us on releasing them.
    css::uno::Reference< css::uno::XInterface > xThis ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::lang::EventObject                      aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    // Clear our child task container and forget all task references hardly.
    // Normally all open document was already closed by our terminate() function before ...
    // New opened frames will have a problem now .-)
    m_aChildTaskContainer.clear();

    // Dispose our helper too.
    css::uno::Reference< css::lang::XEventListener > xFramesHelper( m_xFramesHelper, css::uno::UNO_QUERY );
    if( xFramesHelper.is() )
        xFramesHelper->disposing( aEvent );

    // At least clean up other member references.
    m_xDispatchHelper.clear();
    m_xFramesHelper.clear();
    m_xLastFrame.clear();
    m_xContext.clear();

    m_xPipeTerminator.clear();
    m_xQuickLauncher.clear();
    m_xStarBasicQuitGuard.clear();
    m_xSWThreadManager.clear();

    // we need a copy because the disposing might call the removeEventListener method
    std::vector< css::uno::Reference<css::frame::XTerminateListener> > xComponentDllListeners = m_xComponentDllListeners;
    for (auto& xListener: xComponentDllListeners)
    {
        xListener->disposing(aEvent);
    }
    xComponentDllListeners.clear();
    m_xComponentDllListeners.clear();
    m_xSfxTerminator.clear();
    m_xCommandOptions.reset();

    // From this point nothing will work further on this object ...
    // excepting our dtor() .-)
    m_aTransactionManager.setWorkingMode( E_CLOSE );
}

/*
    @interface  XComponent
    @short      add/remove listener for dispose events
    @descr      Add an event listener to this object, if you wish to get information
                about our dying!
                You must release this listener reference during your own disposing() method.

    @attention  Our container is threadsafe himself. So we don't need any lock here.
    @param      "xListener", reference to valid listener. We don't accept invalid values!
    @threadsafe yes
*/
void SAL_CALL Desktop::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    SAL_WARN_IF( !xListener.is(), "fwk.desktop", "Desktop::addEventListener(): Invalid parameter detected!" );
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    m_aListenerContainer.addInterface( cppu::UnoType<css::lang::XEventListener>::get(), xListener );
}

void SAL_CALL Desktop::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Safe impossible cases
    // Method not defined for all incoming parameter.
    SAL_WARN_IF( !xListener.is(), "fwk.desktop", "Desktop::removeEventListener(): Invalid parameter detected!" );
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    m_aListenerContainer.removeInterface( cppu::UnoType<css::lang::XEventListener>::get(), xListener );
}

/*-************************************************************************************************************
    @interface  XDispatchResultListener
    @short      callback for dispatches
    @descr      To support our method "loadComponentFromURL()" we are listener on temp. created dispatcher.
                They call us back in this method "statusChanged()". As source of given state event, they give us a
                reference to the target frame, in which dispatch was loaded! So we can use it to return his component
                to caller! If no target exist ... ??!!

    @seealso    method loadComponentFromURL()

    @param      "aEvent", state event which (hopefully) valid information
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::dispatchFinished( const css::frame::DispatchResultEvent& aEvent )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    SolarMutexGuard g;
    if( m_eLoadState != E_INTERACTION )
    {
        m_xLastFrame.clear();
        m_eLoadState = E_FAILED;
        if( aEvent.State == css::frame::DispatchResultState::SUCCESS )
        {
            css::uno::Reference < css::frame::XFrame > xFrame;
            if ( aEvent.Result >>= m_xLastFrame )
                m_eLoadState = E_SUCCESSFUL;
        }
    }
}

/*-************************************************************************************************************
    @interface  XEventListener
    @short      not implemented!
    @descr      We are a status listener ... and so we must be an event listener too ... But we don't need it really!
                We are a temp. listener only and our lifetime isn't smaller then of our temp. used dispatcher.

    @seealso    method loadComponentFromURL()
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::disposing( const css::lang::EventObject& )
{
    SAL_WARN( "fwk.desktop", "Desktop::disposing(): Algorithm error! Normally desktop is temp. listener ... not all the time. So this method shouldn't be called." );
}

/*-************************************************************************************************************
    @interface  XInteractionHandler
    @short      callback for loadComponentFromURL for detected exceptions during load process
    @descr      In this case we must cancel loading and throw these detected exception again as result
                of our own called method.

    @attention  a)
                Normal loop in loadComponentFromURL() breaks on setted member m_eLoadState during callback statusChanged().
                But these interaction feature implements second way to do so! So we must look on different callbacks
                for same operation ... and live with it.
                b)
                Search for given continuations too. If any XInteractionAbort exist ... use it to abort further operations
                for currently running operation!

    @seealso    method loadComponentFromURL()
    @seealso    member m_eLoadState

    @param      "xRequest", request for interaction - normal a wrapped target exception from bottom services
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Don't check incoming request!
    // If somewhere starts interaction without right parameter - he made something wrong.
    // loadComponentFromURL() waits for these event - otherwise it yield for ever!

    // get packed request and work on it first
    // Attention: Don't set it on internal member BEFORE interaction is finished - because
    // "loadComponentFromURL()" yield tills this member is changed. If we do it before
    // interaction finish we can't guarantee right functionality. May be we cancel load process to earlier...
    css::uno::Any aRequest = xRequest->getRequest();

    // extract continuations from request
    css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();
    css::uno::Reference< css::task::XInteractionAbort >                              xAbort;
    css::uno::Reference< css::task::XInteractionApprove >                            xApprove;
    css::uno::Reference< css::document::XInteractionFilterSelect >                   xFilterSelect;
    bool                                                                             bAbort         = false;

    sal_Int32 nCount=lContinuations.getLength();
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        if( ! xAbort.is() )
            xAbort.set( lContinuations[nStep], css::uno::UNO_QUERY );

        if( ! xApprove.is() )
            xApprove.set( lContinuations[nStep], css::uno::UNO_QUERY );

        if( ! xFilterSelect.is() )
            xFilterSelect.set( lContinuations[nStep], css::uno::UNO_QUERY );
    }

    // differ between abortable interactions (error, unknown filter ...)
    // and other ones (ambigous but not unknown filter ...)
    css::task::ErrorCodeRequest          aErrorCodeRequest;
    if( aRequest >>= aErrorCodeRequest )
    {
        bool bWarning = ((aErrorCodeRequest.ErrCode & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK);
        if (xApprove.is() && bWarning)
            xApprove->select();
        else
        if (xAbort.is())
        {
            xAbort->select();
            bAbort = true;
        }
    }
    else if( xAbort.is() )
    {
        xAbort->select();
        bAbort = true;
    }

    // Ok now it's time to break yield loop of loadComponentFromURL().
    // But only for really aborted requests!
    // For example warnings will be approved and we wait for any success story ...
    if (bAbort)
    {
        SolarMutexGuard g;
        m_eLoadState          = E_INTERACTION;
        m_aInteractionRequest = aRequest;
    }
}

::sal_Int32 SAL_CALL Desktop::leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    return m_xTitleNumberGenerator->leaseNumber (xComponent);
}

void SAL_CALL Desktop::releaseNumber( ::sal_Int32 nNumber )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    m_xTitleNumberGenerator->releaseNumber (nNumber);
}

void SAL_CALL Desktop::releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    m_xTitleNumberGenerator->releaseNumberForComponent (xComponent);
}

OUString SAL_CALL Desktop::getUntitledPrefix()
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    return m_xTitleNumberGenerator->getUntitledPrefix ();
}

/*-************************************************************************************************************
    @short      try to convert a property value
    @descr      This method is called from helperclass "OPropertySetHelper".
                Don't use this directly!
                You must try to convert the value of given PropHandle and
                return results of this operation. This will be used to ask vetoable
                listener. If no listener has a veto, we will change value really!
                ( in method setFastPropertyValue_NoBroadcast(...) )

    @attention  Methods of OPropertySethelper are safed by using our shared osl mutex! (see ctor!)
                So we must use different locks to make our implementation threadsafe.

    @seealso    class OPropertySetHelper
    @seealso    method setFastPropertyValue_NoBroadcast()

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
                                                     const css::uno::Any&   aValue          )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    //  Initialize state with sal_False !!!
    //  (Handle can be invalid)
    bool bReturn = false;

    switch( nHandle )
    {
        case PropHandle::SuspendQuickstartVeto:
                bReturn = PropHelper::willPropertyBeChanged(
                    css::uno::makeAny(m_bSuspendQuickstartVeto),
                    aValue,
                    aOldValue,
                    aConvertedValue);
                break;
        case PropHandle::DispatchRecorderSupplier :
                bReturn = PropHelper::willPropertyBeChanged(
                    css::uno::makeAny(m_xDispatchRecorderSupplier),
                    aValue,
                    aOldValue,
                    aConvertedValue);
                break;
        case PropHandle::Title :
                bReturn = PropHelper::willPropertyBeChanged(
                    css::uno::makeAny(m_sTitle),
                    aValue,
                    aOldValue,
                    aConvertedValue);
                break;
    }

    // Return state of operation.
    return bReturn;
}

/*-************************************************************************************************************
    @short      set value of a transient property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!
                Handle and value are valid everyway! You must set the new value only.
                After this, baseclass send messages to all listener automatically.

    @seealso    class OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    new value of property
    @onerror    An exception is thrown.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::setFastPropertyValue_NoBroadcast(       sal_Int32        nHandle ,
                                                         const css::uno::Any&   aValue  )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    switch( nHandle )
    {
        case PropHandle::SuspendQuickstartVeto:    aValue >>= m_bSuspendQuickstartVeto;
                                                    break;
        case PropHandle::DispatchRecorderSupplier:    aValue >>= m_xDispatchRecorderSupplier;
                                                    break;
        case PropHandle::Title:    aValue >>= m_sTitle;
                                                    break;
    }
}

/*-************************************************************************************************************
    @short      get value of a transient property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @attention  We don't need any mutex or lock here ... We use threadsafe container or methods here only!

    @seealso    class OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    current value of property
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
        case PropHandle::ActiveFrame           :   aValue <<= m_aChildTaskContainer.getActive();
                                                    break;
        case PropHandle::IsPlugged           :   aValue <<= false;
                                                    break;
        case PropHandle::SuspendQuickstartVeto:    aValue <<= m_bSuspendQuickstartVeto;
                                                    break;
        case PropHandle::DispatchRecorderSupplier:    aValue <<= m_xDispatchRecorderSupplier;
                                                    break;
        case PropHandle::Title:    aValue <<= m_sTitle;
                                                    break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL Desktop::getInfoHelper()
{
    struct Static:
        public rtl::StaticWithInit<cppu::OPropertyArrayHelper, Static>
    {
        cppu::OPropertyArrayHelper operator ()() {
            return {
                {{"ActiveFrame", PropHandle::ActiveFrame,
                  cppu::UnoType<css::lang::XComponent>::get(),
                  (css::beans::PropertyAttribute::TRANSIENT
                   | css::beans::PropertyAttribute::READONLY)},
                 {"DispatchRecorderSupplier",
                  PropHandle::DispatchRecorderSupplier,
                  cppu::UnoType<css::frame::XDispatchRecorderSupplier>::get(),
                  css::beans::PropertyAttribute::TRANSIENT},
                 {"IsPlugged",
                  PropHandle::IsPlugged, cppu::UnoType<bool>::get(),
                  (css::beans::PropertyAttribute::TRANSIENT
                   | css::beans::PropertyAttribute::READONLY)},
                 {"SuspendQuickstartVeto", PropHandle::SuspendQuickstartVeto,
                  cppu::UnoType<bool>::get(),
                  css::beans::PropertyAttribute::TRANSIENT},
                 {"Title", PropHandle::Title, cppu::UnoType<OUString>::get(),
                  css::beans::PropertyAttribute::TRANSIENT}},
                true};
        }
    };
    return Static::get();
}

/*-************************************************************************************************************
    @short      return propertysetinfo
    @descr      You can call this method to get information about transient properties
                of this object.

    @attention  You must use global lock (method use static variable) ... and it must be the shareable osl mutex of it.
                Because; our baseclass use this mutex to make his code threadsafe. We use our lock!
                So we could have two different mutex/lock mechanism at the same object.

    @seealso    class OPropertySetHelper
    @seealso    interface XPropertySet
    @seealso    interface XMultiPropertySet
    @return     reference to object with information [XPropertySetInfo]
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
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = nullptr;

    if( pInfo == nullptr )
    {
        SolarMutexGuard aGuard;

        // Control this pointer again, another instance can be faster then these!
        if( pInfo == nullptr )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static css::uno::Reference< css::beans::XPropertySetInfo > xInfo(
                    cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

/*-************************************************************************************************************
    @short      return current component of current frame
    @descr      The desktop himself has no component. But every frame in subtree.
                If somewhere call getCurrentComponent() at this class, we try to find the right frame and
                then we try to become his component. It can be a VCL-component, the model or the controller
                of founded frame.

    @attention  We don't work on internal member ... so we don't need any lock here.

    @seealso    method getCurrentComponent();

    @param      "xFrame", reference to valid frame in hierarchy. Method is not defined for invalid values.
                But we don't check these. It's an IMPL-method and caller must use it right!
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
    if( !xController.is() )
    {
        // Controller not exist - use the VCL-component.
        xComponent.set( xFrame->getComponentWindow(), css::uno::UNO_QUERY );
    }
    else
    {
        // Does no model exists?
        css::uno::Reference< css::frame::XModel > xModel( xController->getModel(), css::uno::UNO_QUERY );
        if( xModel.is() )
        {
            // Model exist - use the model as component.
            xComponent.set( xModel, css::uno::UNO_QUERY );
        }
        else
        {
            // Model not exist - use the controller as component.
            xComponent.set( xController, css::uno::UNO_QUERY );
        }
    }

    return xComponent;
}

void Desktop::impl_sendQueryTerminationEvent(Desktop::TTerminateListenerList& lCalledListener,
                                             bool&                      bVeto          )
{
    bVeto = false;

    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( cppu::UnoType<css::frame::XTerminateListener>::get());
    if ( ! pContainer )
        return;

    css::lang::EventObject aEvent( static_cast< ::cppu::OWeakObject* >(this) );

    ::cppu::OInterfaceIteratorHelper aIterator( *pContainer );
    while ( aIterator.hasMoreElements() )
    {
        try
        {
            css::uno::Reference< css::frame::XTerminateListener > xListener(aIterator.next(), css::uno::UNO_QUERY);
            if ( ! xListener.is() )
                continue;
            xListener->queryTermination( aEvent );
            lCalledListener.push_back(xListener);
        }
        catch( const css::frame::TerminationVetoException& )
        {
            // first veto will stop notification loop.
            bVeto = true;
            return;
        }
        catch( const css::uno::Exception& )
        {
            // clean up container.
            // E.g. dead remote listener objects can make trouble otherwise.
            // Iterator implementation allows removing objects during it's used !
            aIterator.remove();
        }
    }
}

void Desktop::impl_sendCancelTerminationEvent(const Desktop::TTerminateListenerList& lCalledListener)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::lang::EventObject                          aEvent( static_cast< ::cppu::OWeakObject* >(this) );
    Desktop::TTerminateListenerList::const_iterator pIt;
    for (  pIt  = lCalledListener.begin();
           pIt != lCalledListener.end  ();
         ++pIt                           )
    {
        try
        {
            // Note: cancelTermination() is a new and optional interface method !
            css::uno::Reference< css::frame::XTerminateListener  > xListener           = *pIt;
            css::uno::Reference< css::frame::XTerminateListener2 > xListenerGeneration2(xListener, css::uno::UNO_QUERY);
            if ( ! xListenerGeneration2.is() )
                continue;
            xListenerGeneration2->cancelTermination( aEvent );
        }
        catch( const css::uno::Exception& )
        {}
    }
}

void Desktop::impl_sendNotifyTerminationEvent()
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( cppu::UnoType<css::frame::XTerminateListener>::get());
    if ( ! pContainer )
        return;

    css::lang::EventObject aEvent( static_cast< ::cppu::OWeakObject* >(this) );

    ::cppu::OInterfaceIteratorHelper aIterator( *pContainer );
    while ( aIterator.hasMoreElements() )
    {
        try
        {
            css::uno::Reference< css::frame::XTerminateListener > xListener(aIterator.next(), css::uno::UNO_QUERY);
            if ( ! xListener.is() )
                continue;
            xListener->notifyTermination( aEvent );
        }
        catch( const css::uno::Exception& )
        {
            // clean up container.
            // E.g. dead remote listener objects can make trouble otherwise.
            // Iterator implementation allows removing objects during it's used !
            aIterator.remove();
        }
    }
}

bool Desktop::impl_closeFrames(bool bAllowUI)
{
    SolarMutexClearableGuard aReadLock;
    css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > lFrames = m_aChildTaskContainer.getAllElements();
    aReadLock.clear();

    ::sal_Int32 c                = lFrames.getLength();
    ::sal_Int32 i                = 0;
    ::sal_Int32 nNonClosedFrames = 0;

    for( i=0; i<c; ++i )
    {
        try
        {
            css::uno::Reference< css::frame::XFrame > xFrame = lFrames[i];

            // XController.suspend() will show an UI ...
            // Use it in case it was allowed from outside only.
            bool                                       bSuspended = false;
            css::uno::Reference< css::frame::XController > xController( xFrame->getController(), css::uno::UNO_QUERY );
            if (
                ( bAllowUI         ) &&
                ( xController.is() )
               )
            {
                bSuspended = xController->suspend( true );
                if ( ! bSuspended )
                {
                    ++nNonClosedFrames;
                    continue;
                }
            }

            // Try to close frame (in case no UI was allowed without calling XController->suspend() before!)
            // But don't deliver ownership to any other one!
            // This method can be called again.
            css::uno::Reference< css::util::XCloseable > xClose( xFrame, css::uno::UNO_QUERY );
            if ( xClose.is() )
            {
                try
                {
                    xClose->close(false);
                }
                catch(const css::util::CloseVetoException&)
                {
                    // Any internal process of this frame disagree with our request.
                    // Safe this state but don't break these loop. Other frames has to be closed!
                    ++nNonClosedFrames;

                    // Reactivate controller.
                    // It can happen that XController.suspend() returned true ... but a registered close listener
                    // throwed these veto exception. Then the controller has to be reactivated. Otherwise
                    // these document doesn't work any more.
                    if (
                        (bSuspended      ) &&
                        (xController.is())
                       )
                        xController->suspend(false);
                }

                // If interface XClosable interface exists and was used ...
                // it's not allowed to use XComponent->dispose() also !
                continue;
            }

            // XClosable not supported ?
            // Then we have to dispose these frame hardly.
            css::uno::Reference< css::lang::XComponent > xDispose( xFrame, css::uno::UNO_QUERY );
            if ( xDispose.is() )
                xDispose->dispose();

            // Don't remove these frame from our child container!
            // A frame do it by itself inside close()/dispose() method.
        }
        catch(const css::lang::DisposedException&)
        {
            // Dispose frames are closed frames.
            // So we can count it here .-)
        }
    }

    return (nNonClosedFrames < 1);
}

}   // namespace framework

namespace {

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(new framework::Desktop(context))
    {
        instance->constructorInit();
    }

    rtl::Reference<framework::Desktop> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_Desktop_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(Singleton::get(context).instance.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
