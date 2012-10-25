/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <loadenv/loadenv.hxx>

#include <loadenv/targethelper.hxx>

#include <services/desktop.hxx>
#include <helper/ocomponentaccess.hxx>
#include <dispatch/dispatchprovider.hxx>

#include <dispatch/interceptionhelper.hxx>
#include <classes/taskcreator.hxx>
#include <threadhelp/transactionguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <services.h>
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
#include <com/sun/star/document/AmbigousFilterRequest.hpp>
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

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>
#include <vcl/svapp.hxx>

#include <tools/errinf.hxx>
#include <comphelper/extract.hxx>

#include <fwkdllapi.h>

namespace framework{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_15                    (   Desktop                                                  ,
                                            OWeakObject                                              ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider              ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo               ),
                                            DIRECT_INTERFACE( css::frame::XDesktop                  ),
                                            DIRECT_INTERFACE( css::frame::XComponentLoader          ),
                                            DIRECT_INTERFACE( css::frame::XTasksSupplier            ),
                                            DIRECT_INTERFACE( css::frame::XDispatchProvider         ),
                                            DIRECT_INTERFACE( css::frame::XDispatchProviderInterception),
                                            DIRECT_INTERFACE( css::frame::XFramesSupplier           ),
                                            DIRECT_INTERFACE( css::frame::XFrame                    ),
                                            DIRECT_INTERFACE( css::lang::XComponent                 ),
                                            DIRECT_INTERFACE( css::frame::XDispatchResultListener   ),
                                            DIRECT_INTERFACE( css::lang::XEventListener             ),
                                            DIRECT_INTERFACE( css::task::XInteractionHandler        ),
                                            DIRECT_INTERFACE( css::beans::XPropertySet              ),
                                            DIRECT_INTERFACE( css::frame::XUntitledNumbers          )
                                        )

DEFINE_XTYPEPROVIDER_15                 (   Desktop                                                 ,
                                            css::lang::XTypeProvider                                ,
                                            css::lang::XServiceInfo                                 ,
                                            css::frame::XDesktop                                    ,
                                            css::frame::XComponentLoader                            ,
                                            css::frame::XTasksSupplier                              ,
                                            css::frame::XDispatchProvider                           ,
                                            css::frame::XDispatchProviderInterception               ,
                                            css::frame::XFramesSupplier                             ,
                                            css::frame::XFrame                                      ,
                                            css::lang::XComponent                                   ,
                                            css::frame::XDispatchResultListener                     ,
                                            css::lang::XEventListener                               ,
                                            css::task::XInteractionHandler                          ,
                                            css::beans::XPropertySet                                ,
                                            css::frame::XUntitledNumbers
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
                                                // We use these helper as slave for our interceptor helper ... not directly!
                                                // But he is event listener on THIS instance!
                                                DispatchProvider* pDispatchHelper = new DispatchProvider( m_xFactory, this );
                                                css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( static_cast< ::cppu::OWeakObject* >(pDispatchHelper), css::uno::UNO_QUERY );

                                                //-------------------------------------------------------------------------------------------------------------
                                                // Initialize a new interception helper object to handle dispatches and implement an interceptor mechanism.
                                                // Set created dispatch provider as slowest slave of it.
                                                // Hold interception helper by reference only - not by pointer!
                                                // So it's easiear to destroy it.
                                                InterceptionHelper* pInterceptionHelper = new InterceptionHelper( this, xDispatchProvider );
                                                m_xDispatchHelper = css::uno::Reference< css::frame::XDispatchProvider >( static_cast< ::cppu::OWeakObject* >(pInterceptionHelper), css::uno::UNO_QUERY );

                                                ::rtl::OUStringBuffer sUntitledPrefix (256);
                                                sUntitledPrefix.append      (::rtl::OUString( String( FwkResId( STR_UNTITLED_DOCUMENT ))));
                                                sUntitledPrefix.appendAscii (" ");

                                                ::comphelper::NumberedCollection* pNumbers = new ::comphelper::NumberedCollection ();
                                                m_xTitleNumberGenerator = css::uno::Reference< css::frame::XUntitledNumbers >(static_cast< ::cppu::OWeakObject* >(pNumbers), css::uno::UNO_QUERY_THROW);
                                                pNumbers->setOwner          ( static_cast< ::cppu::OWeakObject* >(this) );
                                                pNumbers->setUntitledPrefix ( sUntitledPrefix.makeStringAndClear ()     );

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
        #ifdef ENABLE_ASSERTIONS
        ,   m_bIsTerminated         ( sal_False                                     )   // see dispose() for further informations!
        #endif
        ,   m_xFactory              ( xFactory                                      )
        ,   m_aChildTaskContainer   (                                               )
        ,   m_aListenerContainer    ( m_aLock.getShareableOslMutex()                )
        ,   m_xFramesHelper         (                                               )
        ,   m_xDispatchHelper       (                                               )
        ,   m_eLoadState            ( E_NOTSET                                      )
        ,   m_xLastFrame            (                                               )
        ,   m_aInteractionRequest   (                                               )
        ,   m_bSuspendQuickstartVeto( sal_False                                     )
        ,   m_aCommandOptions       (                                               )
        ,   m_sName                 (                                               )
        ,   m_sTitle                (                                               )
        ,   m_xDispatchRecorderSupplier(                                            )
        ,   m_xPipeTerminator       (                                               )
        ,   m_xQuickLauncher        (                                               )
        ,   m_xSWThreadManager      (                                               )
        ,   m_xSfxTerminator        (                                               )
        ,   m_xTitleNumberGenerator (                                               )
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
#ifdef ENABLE_ASSERTIONS
    // Perhaps we should here do use a real assertion, but make the
    // condition more specific? We don't want it to fire in unit tests
    // in sc/qa/unit for instance, that don't even have any GUI.
    if( !m_bIsTerminated )
        fprintf( stderr, "This used to be an assertion failure: Desktop not terminated before being destructed,\n"
                 "but it is probably not a real problem.\n" );
#endif
    LOG_ASSERT2( m_aTransactionManager.getWorkingMode()!=E_CLOSE  , "Desktop::~Desktop()", "Who forgot to dispose this service?"          )
}

//=============================================================================
sal_Bool SAL_CALL Desktop::terminate()
    throw( css::uno::RuntimeException )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    ReadGuard aReadLock( m_aLock ); // start synchronize

    css::uno::Reference< css::frame::XTerminateListener > xPipeTerminator    = m_xPipeTerminator;
    css::uno::Reference< css::frame::XTerminateListener > xQuickLauncher     = m_xQuickLauncher;
    css::uno::Reference< css::frame::XTerminateListener > xSWThreadManager   = m_xSWThreadManager;
    css::uno::Reference< css::frame::XTerminateListener > xSfxTerminator     = m_xSfxTerminator;

    css::lang::EventObject                                aEvent             ( static_cast< ::cppu::OWeakObject* >(this) );
    ::sal_Bool                                            bAskQuickStart     = !m_bSuspendQuickstartVeto                  ;

    aReadLock.unlock(); // end synchronize

    //-------------------------------------------------------------------------------------------------------------
    // Ask normal terminate listener. They could stop terminate without closing any open document.
    Desktop::TTerminateListenerList lCalledTerminationListener;
    ::sal_Bool                      bVeto = sal_False;
    impl_sendQueryTerminationEvent(lCalledTerminationListener, bVeto);
    if ( bVeto )
    {
        impl_sendCancelTerminationEvent(lCalledTerminationListener);
        return sal_False;
    }

    //-------------------------------------------------------------------------------------------------------------
    // try to close all open frames.
    // Allow using of any UI ... because Desktop.terminate() was designed as UI functionality in the past.
    ::sal_Bool bAllowUI      = sal_True;
    ::sal_Bool bFramesClosed = impl_closeFrames(bAllowUI);
    if ( ! bFramesClosed )
    {
        impl_sendCancelTerminationEvent(lCalledTerminationListener);
        return sal_False;
    }

    //-------------------------------------------------------------------------------------------------------------
    // Normal listener had no problem ...
    // all frames was closed ...
    // now it's time to ask our specialized listener.
    // They are handled these way because they wish to hinder the office on termination
    // but they wish also closing of all frames.

    // Note further:
    //    We shouldn't ask quicklauncher in case it was allowed from outside only.
    //    This is special trick to "ignore existing quick starter" for debug purposes.

    // Attention:
    // Order of alled listener is important !
    // some of them are harmless .-)
    // But some of them can be dangerous. E.g. it would be dangerous if we close our pipe
    // and dont terminate in real because another listener throws a veto exception .-)

    ::sal_Bool bTerminate = sal_False;
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

        bTerminate = sal_True;
    }
    catch(const css::frame::TerminationVetoException&)
    {
        bTerminate = sal_False;
    }

    if ( ! bTerminate )
        impl_sendCancelTerminationEvent(lCalledTerminationListener);
    else
    {
        #ifdef ENABLE_ASSERTIONS
            // "Protect" us against dispose before terminate calls!
            // see dispose() for further informations.
            /* SAFE AREA --------------------------------------------------------------------------------------- */
            WriteGuard aWriteLock( m_aLock );
            m_bIsTerminated = sal_True;
            aWriteLock.unlock();
            /* UNSAFE AREA ------------------------------------------------------------------------------------- */
        #endif

        impl_sendNotifyTerminationEvent();

        if(
            ( bAskQuickStart      ) &&
            ( xQuickLauncher.is() )
          )
        {
            xQuickLauncher->notifyTermination( aEvent );
        }

        if ( xSWThreadManager.is() )
            xSWThreadManager->notifyTermination( aEvent );

        if ( xPipeTerminator.is() )
            xPipeTerminator->notifyTermination( aEvent );

        // Must be realy the last listener to be called.
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
            QuickstartSuppressor(Desktop* const pDesktop, css::uno::Reference< css::frame::XTerminateListener > xQuickLauncher)
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
    throw( css::uno::RuntimeException )
{
    QuickstartSuppressor aQuickstartSuppressor(this, m_xQuickLauncher);
    return terminate();
}

//=============================================================================
void SAL_CALL Desktop::addTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener )
    throw( css::uno::RuntimeException )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::lang::XServiceInfo > xInfo( xListener, css::uno::UNO_QUERY );
    if ( xInfo.is() )
    {
        ::rtl::OUString sImplementationName = xInfo->getImplementationName();

        // SYCNHRONIZED ->
        WriteGuard aWriteLock( m_aLock );

        if( sImplementationName.equals(IMPLEMENTATIONNAME_SFXTERMINATOR) )
        {
            m_xSfxTerminator = xListener;
            return;
        }
        if( sImplementationName.equals(IMPLEMENTATIONNAME_PIPETERMINATOR) )
        {
            m_xPipeTerminator = xListener;
            return;
        }
        if( sImplementationName.equals(IMPLEMENTATIONNAME_QUICKLAUNCHER) )
        {
            m_xQuickLauncher = xListener;
            return;
        }
        if( sImplementationName.equals(IMPLEMENTATIONNAME_SWTHREADMANAGER) )
        {
            m_xSWThreadManager = xListener;
            return;
        }

        aWriteLock.unlock();
        // <- SYCNHRONIZED
    }

    // No lock required ... container is threadsafe by itself.
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ), xListener );
}

//=============================================================================
void SAL_CALL Desktop::removeTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener )
    throw( css::uno::RuntimeException )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    css::uno::Reference< css::lang::XServiceInfo > xInfo( xListener, css::uno::UNO_QUERY );
    if ( xInfo.is() )
    {
        ::rtl::OUString sImplementationName = xInfo->getImplementationName();

        // SYCNHRONIZED ->
        WriteGuard aWriteLock( m_aLock );

        if( sImplementationName.equals(IMPLEMENTATIONNAME_SFXTERMINATOR) )
        {
            m_xSfxTerminator.clear();
            return;
        }

        if( sImplementationName.equals(IMPLEMENTATIONNAME_PIPETERMINATOR) )
        {
            m_xPipeTerminator.clear();
            return;
        }

        if( sImplementationName.equals(IMPLEMENTATIONNAME_QUICKLAUNCHER) )
        {
            m_xQuickLauncher.clear();
            return;
        }

        if( sImplementationName.equals(IMPLEMENTATIONNAME_SWTHREADMANAGER) )
        {
            m_xSWThreadManager.clear();
            return;
        }

        aWriteLock.unlock();
        // <- SYCNHRONIZED
    }

    // No lock required ... container is threadsafe by itself.
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
    @descr      There can be more then one different active paths in our frame hierarchy. But only one of them
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
    RTL_LOGFILE_CONTEXT( aLog, "framework (as96863) ::Desktop::loadComponentFromURL" );

    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XComponentLoader > xThis(static_cast< css::frame::XComponentLoader* >(this), css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xFactory;
    aReadLock.unlock();

    RTL_LOGFILE_PRODUCT_CONTEXT( aLog2, "PERFORMANCE - Desktop::loadComponentFromURL()" );
    return LoadEnv::loadComponentFromURL(xThis, xSMGR, sURL, sTargetFrameName, nSearchFlags, lArguments);
}

/*-************************************************************************************************************//**
    @interface  XTasksSupplier
    @short      get access to create enumerations of ouer taskchildren
    @descr      Direct children of desktop are tasks everytime.
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
    LOG_WARNING("Desktop::getTasks()", "Use of obsolete interface XTaskSupplier")
    return NULL;
}

/*-************************************************************************************************************//**
    @interface  XTasksSupplier
    @short      return current active task of ouer direct children
    @descr      Desktop children are tasks only ! If we have an active path from desktop
                as top to any frame on bottom, we must have an active direct child. His reference is returned here.

    @attention  a)  Do not confuse it with getCurrentFrame()! The current frame don't must one of ouer direct children.
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
    LOG_WARNING("Desktop::getActiveTask()", "Use of obsolete interface XTaskSupplier")
    return NULL;
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
    const char UNO_PROTOCOL[] = ".uno:";

    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Remove uno and cmd protocol part as we want to support both of them. We store only the command part
    // in our hash map. All other protocols are stored with the protocol part.
    String aCommand( aURL.Main );
    if ( aURL.Protocol.equalsIgnoreAsciiCaseAsciiL( UNO_PROTOCOL, sizeof( UNO_PROTOCOL )-1 ))
        aCommand = aURL.Path;

    // Make boost::unordered_map lookup if the current URL is in the disabled list
    if ( m_aCommandOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aCommand ) )
        return css::uno::Reference< css::frame::XDispatch >();
    else
    {
        // We use a helper to support these interface and an interceptor mechanism.
        // Our helper is threadsafe by himself!
        return m_xDispatchHelper->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    }
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
    @interface  XDipsatchProviderInterception
    @short      supports registration/deregistration of interception objects, which
                are interested on special dispatches.

    @descr      Its realy provided by an internal helper, which is used inside the dispatch api too.
    @param      xInterceptor
                the interceptor object, which wish to be (de)registered.

    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
    throw( css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->registerDispatchProviderInterceptor( xInterceptor );
}

//*****************************************************************************************************************
void SAL_CALL Desktop::releaseDispatchProviderInterceptor ( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
    throw( css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->releaseDispatchProviderInterceptor( xInterceptor );
}

/*-************************************************************************************************************//**
    @interface  XFramesSupplier
    @short      return access to append or remove children on desktop
    @descr      We don't implement these interface directly. We use a helper class to do this.
                If you wish to add or delete children to/from the container, call these method to get
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
    @descr      It must be a task. Direct children of desktop are tasks only! No frames are accepted.
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
void SAL_CALL Desktop::initialize( const css::uno::Reference< css::awt::XWindow >& ) throw( css::uno::RuntimeException )
{
}

//*****************************************************************************************************************
css::uno::Reference< css::awt::XWindow > SAL_CALL Desktop::getContainerWindow() throw( css::uno::RuntimeException )
{
    return css::uno::Reference< css::awt::XWindow >();
}

//*****************************************************************************************************************
void SAL_CALL Desktop::setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& /*xCreator*/ ) throw( css::uno::RuntimeException )
{
}

//*****************************************************************************************************************
css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL Desktop::getCreator() throw( css::uno::RuntimeException )
{
    return css::uno::Reference< css::frame::XFramesSupplier >();
}

//*****************************************************************************************************************
::rtl::OUString SAL_CALL Desktop::getName() throw( css::uno::RuntimeException )
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return m_sName;
    /* } SAFE */
}

//*****************************************************************************************************************
void SAL_CALL Desktop::setName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException )
{
    /* SAFE { */
    WriteGuard aWriteLock( m_aLock );
    m_sName = sName;
    aWriteLock.unlock();
    /* } SAFE */
}

//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::isTop() throw( css::uno::RuntimeException )
{
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
    return sal_True;
}

//*****************************************************************************************************************
sal_Bool SAL_CALL Desktop::setComponent( const css::uno::Reference< css::awt::XWindow >&       /*xComponentWindow*/ ,
                                         const css::uno::Reference< css::frame::XController >& /*xController*/      ) throw( css::uno::RuntimeException )
{
    return sal_False;
}

//*****************************************************************************************************************
css::uno::Reference< css::awt::XWindow > SAL_CALL Desktop::getComponentWindow() throw( css::uno::RuntimeException )
{
    return css::uno::Reference< css::awt::XWindow >();
}

//*****************************************************************************************************************
css::uno::Reference< css::frame::XController > SAL_CALL Desktop::getController() throw( css::uno::RuntimeException )
{
    return css::uno::Reference< css::frame::XController >();
}

//*****************************************************************************************************************
void SAL_CALL Desktop::contextChanged() throw( css::uno::RuntimeException )
{
}

//*****************************************************************************************************************
void SAL_CALL Desktop::addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& ) throw( css::uno::RuntimeException )
{
}

//*****************************************************************************************************************
//   css::frame::XFrame
//*****************************************************************************************************************
void SAL_CALL Desktop::removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& ) throw( css::uno::RuntimeException )
{
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
    css::uno::Reference< css::frame::XFrame > xTarget;

    //-----------------------------------------------------------------------------------------------------
    // 0) Ignore wrong parameter!
    //    We doesn't support search for following special targets.
    //    If we reject this requests - we mustnt check for such names
    //    in following code again and again. If we do not so -wrong
    //    search results can occure!
    //-----------------------------------------------------------------------------------------------------
    if (
        (sTargetFrameName==SPECIALTARGET_DEFAULT  )   ||    // valid for dispatches - not for findFrame()!
        (sTargetFrameName==SPECIALTARGET_MENUBAR  )   ||    // valid for dispatches - not for findFrame()!
        (sTargetFrameName==SPECIALTARGET_PARENT   )   ||    // we have no parent by definition
        (sTargetFrameName==SPECIALTARGET_BEAMER   )         // beamer frames are allowed as child of tasks only -
                                                            // and they exist more then ones. We have no idea which our sub tasks is the right one
       )
    {
        return NULL;
    }

    //-----------------------------------------------------------------------------------------------------
    // I) check for special defined targets first which must be handled exclusive.
    //    force using of "if() else if() ..."
    //-----------------------------------------------------------------------------------------------------

    // get threadsafe some neccessary member which are neccessary for following functionality
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory = m_xFactory;
    aReadLock.unlock();
    /* } SAFE */

    //-----------------------------------------------------------------------------------------------------
    // I.I) "_blank"
    //  create a new task as child of this desktop instance
    //  Note: Used helper TaskCreator use us automaticly ...
    //-----------------------------------------------------------------------------------------------------
    if ( sTargetFrameName==SPECIALTARGET_BLANK )
    {
        TaskCreator aCreator(xFactory);
        xTarget = aCreator.createTask(sTargetFrameName,sal_False);
    }

    //-----------------------------------------------------------------------------------------------------
    // I.II) "_top"
    //  We are top by definition
    //-----------------------------------------------------------------------------------------------------
    else if ( sTargetFrameName==SPECIALTARGET_TOP )
    {
        xTarget = this;
    }

    //-----------------------------------------------------------------------------------------------------
    // I.III) "_self", ""
    //  This mean this "frame" in every case.
    //-----------------------------------------------------------------------------------------------------
    else if (
             ( sTargetFrameName==SPECIALTARGET_SELF ) ||
             ( sTargetFrameName.isEmpty()           )
            )
    {
        xTarget = this;
    }

    else
    {
        //-------------------------------------------------------------------------------------------------
        // II) otherwhise use optional given search flags
        //  force using of combinations of such flags. means no "else" part of use if() statements.
        //  But we ust break further searches if target was already found.
        //  Order of using flags is fix: SELF - CHILDREN - SIBLINGS - PARENT
        //  TASK and CREATE are handled special.
        //  But note: Such flags are not valid for the desktop - especialy SIBLINGS or PARENT.
        //-------------------------------------------------------------------------------------------------

        // get threadsafe some neccessary member which are neccessary for following functionality
        /* SAFE { */
        aReadLock.lock();
        ::rtl::OUString sOwnName = m_sName;
        aReadLock.unlock();
        /* } SAFE */

        //-------------------------------------------------------------------------------------------------
        // II.I) SELF
        //  Check for right name. If it's the searched one return ourself - otherwhise
        //  ignore this flag.
        //-------------------------------------------------------------------------------------------------
        if (
            (nSearchFlags &  css::frame::FrameSearchFlag::SELF)  &&
            (sOwnName     == sTargetFrameName                 )
           )
        {
            xTarget = this;
        }

        //-------------------------------------------------------------------------------------------------
        // II.II) TASKS
        //  This is a special flag. Normaly it regulate search inside tasks and forbid access to parent trees.
        //  But the desktop exists outside such task trees. They are our sub trees. So the desktop implement
        //  a special feature: We use it to start search on our direct childrens only. That means we supress
        //  search on ALL child frames. May that can be usefull to get access on opened document tasks
        //  only without filter out all non realy required sub frames ...
        //  Used helper method on our container doesn't create any frame - its a search only.
        //-------------------------------------------------------------------------------------------------
        if (
            ( ! xTarget.is()                                  ) &&
            (nSearchFlags & css::frame::FrameSearchFlag::TASKS)
           )
        {
            xTarget = m_aChildTaskContainer.searchOnDirectChildrens(sTargetFrameName);
        }

        //-------------------------------------------------------------------------------------------------
        // II.III) CHILDREN
        //  Search on all children for the given target name.
        //  An empty name value can't occure here - because it must be already handled as "_self"
        //  before. Used helper function of container doesn't create any frame.
        //  It makes a deep search only.
        //-------------------------------------------------------------------------------------------------
        if (
            ( ! xTarget.is()                                     ) &&
            (nSearchFlags & css::frame::FrameSearchFlag::CHILDREN)
           )
        {
            xTarget = m_aChildTaskContainer.searchOnAllChildrens(sTargetFrameName);
        }

        //-------------------------------------------------------------------------------------------------
        // II.IV) CREATE
        //  If we haven't found any valid target frame by using normal flags - but user allowed us to create
        //  a new one ... we should do that. Used TaskCreator use us automaticly as parent!
        //-------------------------------------------------------------------------------------------------
        if (
            ( ! xTarget.is()                                   )    &&
            (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
           )
        {
            TaskCreator aCreator(xFactory);
            xTarget = aCreator.createTask(sTargetFrameName,sal_False);
        }
    }

    return xTarget;
}

//=============================================================================
void SAL_CALL Desktop::dispose()
    throw( css::uno::RuntimeException )
{
    // Safe impossible cases
    // It's an programming error if dispose is called before terminate!

    // But if you just ignore the assertion (which happens in unit
    // tests for instance in sc/qa/unit) nothing bad happens.
#ifdef ENABLE_ASSERTIONS
    if( !m_bIsTerminated )
        fprintf( stderr, "This used to be an assertion failure: Desktop disposed before terminating it,\n"
                 "but nothing bad seems to happen anyway?\n" );
#endif

    WriteGuard aWriteLock( m_aLock ); // start synchronize

    // Look for multiple calls of this method!
    // If somewhere call dispose() twice - he will be stopped here realy!!!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Now - we are alone and its the first call of this method ...
    // otherwise call before had thrown a DisposedException / hopefully .-)
    // But we dont use the transaction object created before ... we reset it immediatly ...
    // two lines of code ... for what ?
    // The answer: We wished to synchronize concurrent dispose() calls -> OK
    // But next line will wait for all currently running transaction (even if they
    // are running within the same thread!) So we would block ourself there if aTransaction
    // will stay registered .-)
    aTransaction.stop();

    // Disable this instance for further work.
    // This will wait for all current running transactions ...
    // and reject all new incoming requests!
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    aWriteLock.unlock(); // end synchronize

    // Following lines of code can be called outside a synchronized block ...
    // Because our transaction manager will block all new requests to this object.
    // So nobody can use us any longer.
    // Exception: Only removing of listener will work ... and this code cant be dangerous.

    // First we has to kill all listener connections.
    // They might rely on our member and can hinder us on releasing them.
    css::uno::Reference< css::uno::XInterface > xThis ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::lang::EventObject                      aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    // Clear our child task container and forget all task references hardly.
    // Normaly all open document was already closed by our terminate() function before ...
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
    m_xFactory.clear();

    m_xPipeTerminator.clear();
    m_xQuickLauncher.clear();
    m_xSWThreadManager.clear();
    m_xSfxTerminator.clear();

    // From this point nothing will work further on this object ...
    // excepting our dtor() .-)
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
    @interface  XDispatchResultListener
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
void SAL_CALL Desktop::dispatchFinished( const css::frame::DispatchResultEvent& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    if( m_eLoadState != E_INTERACTION )
    {
        m_xLastFrame = css::uno::Reference< css::frame::XFrame >();
        m_eLoadState = E_FAILED                                   ;
        if( aEvent.State == css::frame::DispatchResultState::SUCCESS )
        {
            css::uno::Reference < css::frame::XFrame > xFrame;
            if ( aEvent.Result >>= m_xLastFrame )
                m_eLoadState = E_SUCCESSFUL;
        }
    }
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
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
void SAL_CALL Desktop::disposing( const css::lang::EventObject& ) throw( css::uno::RuntimeException )
{
    LOG_ERROR( "Desktop::disposing()", "Algorithm error! Normaly desktop is temp. listener ... not all the time. So this method shouldn't be called." )
}

/*-************************************************************************************************************//**
    @interface  XInteractionHandler
    @short      callback for loadComponentFromURL for detected exceptions during load proccess
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
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Desktop::handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Don't check incoming request!
    // If somewhere starts interaction without right parameter - he maked something wrong.
    // loadComponentFromURL() waits for thjese event - otherwise it yield for ever!

    // get packed request and work on it first
    // Attention: Don't set it on internal member BEFORE interaction is finished - because
    // "loadComponentFromURL()" yield tills this member is changed. If we do it before
    // interaction finish we can't guarantee right functionality. May be we cancel load process to erliear ...
    css::uno::Any aRequest = xRequest->getRequest();

    // extract continuations from request
    css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();
    css::uno::Reference< css::task::XInteractionAbort >                              xAbort         ;
    css::uno::Reference< css::task::XInteractionApprove >                            xApprove       ;
    css::uno::Reference< css::document::XInteractionFilterSelect >                   xFilterSelect  ;
    sal_Bool                                                                         bAbort         = sal_False;

    sal_Int32 nCount=lContinuations.getLength();
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        if( ! xAbort.is() )
            xAbort  = css::uno::Reference< css::task::XInteractionAbort >( lContinuations[nStep], css::uno::UNO_QUERY );

        if( ! xApprove.is() )
            xApprove  = css::uno::Reference< css::task::XInteractionApprove >( lContinuations[nStep], css::uno::UNO_QUERY );

        if( ! xFilterSelect.is() )
            xFilterSelect = css::uno::Reference< css::document::XInteractionFilterSelect >( lContinuations[nStep], css::uno::UNO_QUERY );
    }

    // differ between abortable interactions (error, unknown filter ...)
    // and other ones (ambigous but not unknown filter ...)
    css::task::ErrorCodeRequest          aErrorCodeRequest     ;
    css::document::AmbigousFilterRequest aAmbigousFilterRequest;
    if( aRequest >>= aAmbigousFilterRequest )
    {
        if( xFilterSelect.is() )
        {
            xFilterSelect->setFilter( aAmbigousFilterRequest.SelectedFilter ); // user selected filter wins!
            xFilterSelect->select();
        }
    }
    else if( aRequest >>= aErrorCodeRequest )
    {
        sal_Bool bWarning = ((aErrorCodeRequest.ErrCode & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK);
        if (xApprove.is() && bWarning)
            xApprove->select();
        else
        if (xAbort.is())
        {
            xAbort->select();
            bAbort = sal_True;
        }
    }
    else if( xAbort.is() )
    {
        xAbort->select();
        bAbort = sal_True;
    }

    /* SAFE AREA ------------------------------------------------------------------------------------------- */
    // Ok now it's time to break yield loop of loadComponentFromURL().
    // But only for realy aborted requests!
    // For example warnings will be approved and we wait for any success story ...
    if (bAbort)
    {
        WriteGuard aWriteLock( m_aLock );
        m_eLoadState          = E_INTERACTION;
        m_aInteractionRequest = aRequest     ;
        aWriteLock.unlock();
    }
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
}

//-----------------------------------------------------------------------------
::sal_Int32 SAL_CALL Desktop::leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    return m_xTitleNumberGenerator->leaseNumber (xComponent);
}

//-----------------------------------------------------------------------------
void SAL_CALL Desktop::releaseNumber( ::sal_Int32 nNumber )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    m_xTitleNumberGenerator->releaseNumber (nNumber);
}

//-----------------------------------------------------------------------------
void SAL_CALL Desktop::releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    m_xTitleNumberGenerator->releaseNumberForComponent (xComponent);
}

//-----------------------------------------------------------------------------
::rtl::OUString SAL_CALL Desktop::getUntitledPrefix()
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    return m_xTitleNumberGenerator->getUntitledPrefix ();
}

/*-************************************************************************************************************//**
    @short      try to convert a property value
    @descr      This method is called from helperclass "OPropertySetHelper".
                Don't use this directly!
                You must try to convert the value of given DESKTOP_PROPHANDLE and
                return results of this operation. This will be used to ask vetoable
                listener. If no listener has a veto, we will change value realy!
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
                                                     const css::uno::Any&   aValue          ) throw( css::lang::IllegalArgumentException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    //  Initialize state with sal_False !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case DESKTOP_PROPHANDLE_SUSPENDQUICKSTARTVETO:
                bReturn = PropHelper::willPropertyBeChanged(
                    css::uno::makeAny(m_bSuspendQuickstartVeto),
                    aValue,
                    aOldValue,
                    aConvertedValue);
                break;
        case DESKTOP_PROPHANDLE_DISPATCHRECORDERSUPPLIER :
                bReturn = PropHelper::willPropertyBeChanged(
                    css::uno::makeAny(m_xDispatchRecorderSupplier),
                    aValue,
                    aOldValue,
                    aConvertedValue);
                break;
        case DESKTOP_PROPHANDLE_TITLE :
                bReturn = PropHelper::willPropertyBeChanged(
                    css::uno::makeAny(m_sTitle),
                    aValue,
                    aOldValue,
                    aConvertedValue);
                break;
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
        case DESKTOP_PROPHANDLE_SUSPENDQUICKSTARTVETO:    aValue >>= m_bSuspendQuickstartVeto;
                                                    break;
        case DESKTOP_PROPHANDLE_DISPATCHRECORDERSUPPLIER:    aValue >>= m_xDispatchRecorderSupplier;
                                                    break;
        case DESKTOP_PROPHANDLE_TITLE:    aValue >>= m_sTitle;
                                                    break;
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
        case DESKTOP_PROPHANDLE_ACTIVEFRAME           :   aValue <<= m_aChildTaskContainer.getActive();
                                                    break;
        case DESKTOP_PROPHANDLE_ISPLUGGED           :   aValue <<= sal_False;
                                                    break;
        case DESKTOP_PROPHANDLE_SUSPENDQUICKSTARTVETO:    aValue <<= m_bSuspendQuickstartVeto;
                                                    break;
        case DESKTOP_PROPHANDLE_DISPATCHRECORDERSUPPLIER:    aValue <<= m_xDispatchRecorderSupplier;
                                                    break;
        case DESKTOP_PROPHANDLE_TITLE:    aValue <<= m_sTitle;
                                                    break;
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
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL Desktop::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
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
    // Create a property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    const css::beans::Property pProperties[] =
    {
        css::beans::Property( DESKTOP_PROPNAME_ACTIVEFRAME              , DESKTOP_PROPHANDLE_ACTIVEFRAME             , ::getCppuType((const css::uno::Reference< css::lang::XComponent >*)NULL)                , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( DESKTOP_PROPNAME_DISPATCHRECORDERSUPPLIER , DESKTOP_PROPHANDLE_DISPATCHRECORDERSUPPLIER, ::getCppuType((const css::uno::Reference< css::frame::XDispatchRecorderSupplier >*)NULL), css::beans::PropertyAttribute::TRANSIENT ),
        css::beans::Property( DESKTOP_PROPNAME_ISPLUGGED                , DESKTOP_PROPHANDLE_ISPLUGGED               , ::getBooleanCppuType()                                                                  , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( DESKTOP_PROPNAME_SUSPENDQUICKSTARTVETO    , DESKTOP_PROPHANDLE_SUSPENDQUICKSTARTVETO   , ::getBooleanCppuType()                                                                  , css::beans::PropertyAttribute::TRANSIENT ),
        css::beans::Property( DESKTOP_PROPNAME_TITLE                    , DESKTOP_PROPHANDLE_TITLE                   , ::getCppuType((const ::rtl::OUString*)NULL)                                             , css::beans::PropertyAttribute::TRANSIENT ),
    };
    // Use it to initialize sequence!
    const css::uno::Sequence< css::beans::Property > lPropertyDescriptor( pProperties, DESKTOP_PROPCOUNT );
    // Return "PropertyDescriptor"
    return lPropertyDescriptor;
}

//=============================================================================
void Desktop::impl_sendQueryTerminationEvent(Desktop::TTerminateListenerList& lCalledListener,
                                             ::sal_Bool&                      bVeto          )
{
    bVeto = sal_False;

    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ) );
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
            bVeto = sal_True;
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

//=============================================================================
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

//=============================================================================
void Desktop::impl_sendNotifyTerminationEvent()
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ) );
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

//=============================================================================
::sal_Bool Desktop::impl_closeFrames(::sal_Bool bAllowUI)
{
    ReadGuard aReadLock( m_aLock ); // start synchronize
    css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > lFrames = m_aChildTaskContainer.getAllElements();
    aReadLock.unlock(); // end synchronize

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
            sal_Bool                                       bSuspended = sal_False;
            css::uno::Reference< css::frame::XController > xController( xFrame->getController(), css::uno::UNO_QUERY );
            if (
                ( bAllowUI         ) &&
                ( xController.is() )
               )
            {
                bSuspended = xController->suspend( sal_True );
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
                    xClose->close(sal_False);
                }
                catch(const css::util::CloseVetoException&)
                {
                    // Any internal process of this frame disagree with our request.
                    // Safe this state but dont break these loop. Other frames has to be closed!
                    ++nNonClosedFrames;

                    // Reactivate controller.
                    // It can happen that XController.suspend() returned true ... but a registered close listener
                    // throwed these veto exception. Then the controller has to be reactivated. Otherwise
                    // these document doesnt work any more.
                    if (
                        (bSuspended      ) &&
                        (xController.is())
                       )
                        xController->suspend(sal_False);
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

#endif  // #ifdef ENABLE_ASSERTIONS

}   // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
