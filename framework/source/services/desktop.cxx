/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
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

#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

#include <tools/errinf.hxx>
#include <comphelper/extract.hxx>

namespace framework{

OUString SAL_CALL Desktop::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return OUString("com.sun.star.comp.framework.Desktop");
}

sal_Bool SAL_CALL Desktop::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL Desktop::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > aSeq(1);
    aSeq[0] = OUString("com.sun.star.frame.Desktop");
    return aSeq;
}

void Desktop::constructorInit()
{
    
    
    
    
    OFrames* pFramesHelper = new OFrames( this, &m_aChildTaskContainer );
    m_xFramesHelper = css::uno::Reference< css::frame::XFrames >( static_cast< ::cppu::OWeakObject* >(pFramesHelper), css::uno::UNO_QUERY );

    
    
    
    
    DispatchProvider* pDispatchHelper = new DispatchProvider( m_xContext, this );
    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( static_cast< ::cppu::OWeakObject* >(pDispatchHelper), css::uno::UNO_QUERY );

    
    
    
    
    
    InterceptionHelper* pInterceptionHelper = new InterceptionHelper( this, xDispatchProvider );
    m_xDispatchHelper = css::uno::Reference< css::frame::XDispatchProvider >( static_cast< ::cppu::OWeakObject* >(pInterceptionHelper), css::uno::UNO_QUERY );

    OUStringBuffer sUntitledPrefix (256);
    sUntitledPrefix.append      (FWK_RESSTR(STR_UNTITLED_DOCUMENT));
    sUntitledPrefix.appendAscii (" ");

    ::comphelper::NumberedCollection* pNumbers = new ::comphelper::NumberedCollection ();
    m_xTitleNumberGenerator = css::uno::Reference< css::frame::XUntitledNumbers >(static_cast< ::cppu::OWeakObject* >(pNumbers), css::uno::UNO_QUERY_THROW);
    pNumbers->setOwner          ( static_cast< ::cppu::OWeakObject* >(this) );
    pNumbers->setUntitledPrefix ( sUntitledPrefix.makeStringAndClear ()     );

    
    
    SAL_WARN_IF( !m_xFramesHelper.is(), "fwk", "Desktop::Desktop(): Frames helper is not valid. XFrames, XIndexAccess and XElementAcces are not supported!");
    SAL_WARN_IF( !m_xDispatchHelper.is(), "fwk", "Desktop::Desktop(): Dispatch helper is not valid. XDispatch will not work correctly!" );

    
    
    m_aTransactionManager.setWorkingMode( E_WORK );
}

/*-************************************************************************************************************
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
*
Desktop::Desktop( const css::uno::Reference< css::uno::XComponentContext >& xContext )
        :   ThreadHelpBase          ( &Application::GetSolarMutex()                 )
        ,   TransactionBase         (                                               )
        ,   Desktop_BASE            ( *static_cast<osl::Mutex *>(this)              )
        ,   cppu::OPropertySetHelper( cppu::WeakComponentImplHelperBase::rBHelper   )
        
        ,   m_bIsTerminated         ( sal_False                                     )   
        ,   m_xContext              ( xContext                                      )
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
}

/*-************************************************************************************************************
    @short      standard destructor
    @descr      This one do NOTHING! Use dispose() instaed of this.

    @seealso    method dispose()

    @param      -
    @return     -

    @onerror    -
*
Desktop::~Desktop()
{
    SAL_WARN_IF( !m_bIsTerminated, "fwk", "Desktop not terminated before being destructed" );
    SAL_WARN_IF( m_aTransactionManager.getWorkingMode()!=E_CLOSE, "fwk", "Desktop::~Desktop(): Who forgot to dispose this service?" );
}

css::uno::Any SAL_CALL Desktop::queryInterface( const css::uno::Type& _rType ) throw(css::uno::RuntimeException)
{
    css::uno::Any aRet = Desktop_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    return aRet;
}

css::uno::Sequence< css::uno::Type > SAL_CALL Desktop::getTypes(  ) throw(css::uno::RuntimeException)
{
    return comphelper::concatSequences(
        Desktop_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}


sal_Bool SAL_CALL Desktop::terminate()
    throw( css::uno::RuntimeException )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    ReadGuard aReadLock( m_aLock ); 

    css::uno::Reference< css::frame::XTerminateListener > xPipeTerminator    = m_xPipeTerminator;
    css::uno::Reference< css::frame::XTerminateListener > xQuickLauncher     = m_xQuickLauncher;
    css::uno::Reference< css::frame::XTerminateListener > xSWThreadManager   = m_xSWThreadManager;
    css::uno::Reference< css::frame::XTerminateListener > xSfxTerminator     = m_xSfxTerminator;

    css::lang::EventObject                                aEvent             ( static_cast< ::cppu::OWeakObject* >(this) );
    ::sal_Bool                                            bAskQuickStart     = !m_bSuspendQuickstartVeto                  ;

    aReadLock.unlock(); 

    
    
    Desktop::TTerminateListenerList lCalledTerminationListener;
    ::sal_Bool                      bVeto = sal_False;
    impl_sendQueryTerminationEvent(lCalledTerminationListener, bVeto);
    if ( bVeto )
    {
        impl_sendCancelTerminationEvent(lCalledTerminationListener);
        return sal_False;
    }

    
    
    
    ::sal_Bool bAllowUI      = sal_True;
    ::sal_Bool bFramesClosed = impl_closeFrames(bAllowUI);
    if ( ! bFramesClosed )
    {
        impl_sendCancelTerminationEvent(lCalledTerminationListener);
        return sal_False;
    }

    
    
    
    
    
    

    
    
    

    
    
    
    
    

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
            
            
            /* SAFE AREA --------------------------------------------------------------------------------------- */
            WriteGuard aWriteLock( m_aLock );
            m_bIsTerminated = sal_True;
            aWriteLock.unlock();
            /* UNSAFE AREA ------------------------------------------------------------------------------------- */

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


void SAL_CALL Desktop::addTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener )
    throw( css::uno::RuntimeException )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::lang::XServiceInfo > xInfo( xListener, css::uno::UNO_QUERY );
    if ( xInfo.is() )
    {
        OUString sImplementationName = xInfo->getImplementationName();

        
        WriteGuard aWriteLock( m_aLock );

        if( sImplementationName == "com.sun.star.comp.sfx2.SfxTerminateListener" )
        {
            m_xSfxTerminator = xListener;
            return;
        }
        if( sImplementationName == "com.sun.star.comp.OfficeIPCThreadController" )
        {
            m_xPipeTerminator = xListener;
            return;
        }
        if( sImplementationName == "com.sun.star.comp.desktop.QuickstartWrapper" )
        {
            m_xQuickLauncher = xListener;
            return;
        }
        if( sImplementationName == "com.sun.star.util.comp.FinalThreadManager" )
        {
            m_xSWThreadManager = xListener;
            return;
        }

        aWriteLock.unlock();
        
    }

    
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ), xListener );
}


void SAL_CALL Desktop::removeTerminateListener( const css::uno::Reference< css::frame::XTerminateListener >& xListener )
    throw( css::uno::RuntimeException )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    css::uno::Reference< css::lang::XServiceInfo > xInfo( xListener, css::uno::UNO_QUERY );
    if ( xInfo.is() )
    {
        OUString sImplementationName = xInfo->getImplementationName();

        
        WriteGuard aWriteLock( m_aLock );

        if( sImplementationName == "com.sun.star.comp.sfx2.SfxTerminateListener" )
        {
            m_xSfxTerminator.clear();
            return;
        }

        if( sImplementationName == "com.sun.star.comp.OfficeIPCThreadController" )
        {
            m_xPipeTerminator.clear();
            return;
        }

        if( sImplementationName == "com.sun.star.comp.desktop.QuickstartWrapper" )
        {
            m_xQuickLauncher.clear();
            return;
        }

        if( sImplementationName == "com.sun.star.util.comp.FinalThreadManager" )
        {
            m_xSWThreadManager.clear();
            return;
        }

        aWriteLock.unlock();
        
    }

    
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::frame::XTerminateListener >*) NULL ), xListener );
}

/*-************************************************************************************************************
    @interface  XDesktop
    @short      get access to create enumerations of all current components
    @descr      You will be the owner of the returned object and must delete it if you don't use it again.

    @seealso    class TasksAccess
    @seealso    class TasksEnumeration

    @param      -
    @return     A reference to an XEnumerationAccess-object.

    @onerror    We return a null-reference.
    @threadsafe yes
*
css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL Desktop::getComponents() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
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

    @param      -
    @return     A reference to the component.

    @onerror    We return a null-reference.
    @threadsafe yes
*
css::uno::Reference< css::lang::XComponent > SAL_CALL Desktop::getCurrentComponent() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    css::uno::Reference< css::lang::XComponent > xComponent;

    
    
    
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
*
css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::getCurrentFrame() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    
    css::uno::Reference< css::frame::XFramesSupplier > xLast = css::uno::Reference< css::frame::XFramesSupplier >( getActiveFrame(), css::uno::UNO_QUERY );
    if( xLast.is() )
    {
        css::uno::Reference< css::frame::XFramesSupplier > xNext = css::uno::Reference< css::frame::XFramesSupplier >( xLast->getActiveFrame(), css::uno::UNO_QUERY );
        while( xNext.is() )
        {
            xLast = xNext;
            xNext = css::uno::Reference< css::frame::XFramesSupplier >( xNext->getActiveFrame(), css::uno::UNO_QUERY );
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

    @seealso    -

    @param      "sURL"              , URL, which represant the content
    @param      "sTargetFrameName"  , name of target frame or special value like "_self", "_blank" ...
    @param      "nSearchFlags"      , optional arguments for frame search, if target isn't a special one
    @param      "lArguments"        , optional arguments for loading
    @return     A valid component reference, if loading was successfully.
                A null reference otherwise.

    @onerror    We return a null reference.
    @threadsafe yes
*
css::uno::Reference< css::lang::XComponent > SAL_CALL Desktop::loadComponentFromURL( const OUString&                                 sURL            ,
                                                                                     const OUString&                                 sTargetFrameName,
                                                                                           sal_Int32                                        nSearchFlags    ,
                                                                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments      ) throw(    css::io::IOException                ,
                                                                                                                                                                        css::lang::IllegalArgumentException ,
                                                                                                                                                                        css::uno::RuntimeException          )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    SAL_INFO( "fwk.desktop", "framework (as96863) ::Desktop::loadComponentFromURL" );

    css::uno::Reference< css::frame::XComponentLoader > xThis(static_cast< css::frame::XComponentLoader* >(this), css::uno::UNO_QUERY);

    SAL_INFO( "fwk.desktop", "PERFORMANCE - Desktop::loadComponentFromURL()" );
    return LoadEnv::loadComponentFromURL(xThis, m_xContext, sURL, sTargetFrameName, nSearchFlags, lArguments);
}

/*-************************************************************************************************************
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
*
css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL Desktop::getTasks() throw( css::uno::RuntimeException )
{
    SAL_INFO("fwk", "Desktop::getTasks(): Use of obsolete interface XTaskSupplier");
    return NULL;
}

/*-************************************************************************************************************
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
*
css::uno::Reference< css::frame::XTask > SAL_CALL Desktop::getActiveTask() throw( css::uno::RuntimeException )
{
    SAL_INFO("fwk", "Desktop::getActiveTask(): Use of obsolete interface XTaskSupplier");
    return NULL;
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
*
css::uno::Reference< css::frame::XDispatch > SAL_CALL Desktop::queryDispatch( const css::util::URL&  aURL             ,
                                                                              const OUString& sTargetFrameName ,
                                                                                    sal_Int32        nSearchFlags     ) throw( css::uno::RuntimeException )
{
    const char UNO_PROTOCOL[] = ".uno:";

    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    OUString aCommand( aURL.Main );
    if ( aURL.Protocol.equalsIgnoreAsciiCaseAsciiL( UNO_PROTOCOL, sizeof( UNO_PROTOCOL )-1 ))
        aCommand = aURL.Path;

    
    if ( m_aCommandOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aCommand ) )
        return css::uno::Reference< css::frame::XDispatch >();
    else
    {
        
        
        return m_xDispatchHelper->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
    }
}


css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL Desktop::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lQueries ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_xDispatchHelper->queryDispatches( lQueries );
}

/*-************************************************************************************************************
    @interface  XDipsatchProviderInterception
    @short      supports registration/deregistration of interception objects, which
                are interested on special dispatches.

    @descr      Its really provided by an internal helper, which is used inside the dispatch api too.
    @param      xInterceptor
                the interceptor object, which wish to be (de)registered.

    @threadsafe yes
*
void SAL_CALL Desktop::registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
    throw( css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->registerDispatchProviderInterceptor( xInterceptor );
}


void SAL_CALL Desktop::releaseDispatchProviderInterceptor ( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
    throw( css::uno::RuntimeException)
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

    @param      -
    @return     A reference to the helper.

    @onerror    A null reference is returned.
    @threadsafe yes
*
css::uno::Reference< css::frame::XFrames > SAL_CALL Desktop::getFrames() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_xFramesHelper;
}

/*-************************************************************************************************************
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
*
void SAL_CALL Desktop::setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    
    
    
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


css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::getActiveFrame() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    return m_aChildTaskContainer.getActive();
}

/*-************************************************************************************************************
    @interface  XFrame
    @short      non implemented methods!
    @descr      Some method make no sense for our desktop! He has no window or parent or ...
                So we should implement it empty and warn programmer, if he use it!

    @seealso    -

    @param      -
    @return     -

    @onerror    -
    @threadsafe -
*
void SAL_CALL Desktop::initialize( const css::uno::Reference< css::awt::XWindow >& ) throw( css::uno::RuntimeException )
{
}


css::uno::Reference< css::awt::XWindow > SAL_CALL Desktop::getContainerWindow() throw( css::uno::RuntimeException )
{
    return css::uno::Reference< css::awt::XWindow >();
}


void SAL_CALL Desktop::setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& /*xCreator*/ ) throw( css::uno::RuntimeException )
{
}


css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL Desktop::getCreator() throw( css::uno::RuntimeException )
{
    return css::uno::Reference< css::frame::XFramesSupplier >();
}


OUString SAL_CALL Desktop::getName() throw( css::uno::RuntimeException )
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return m_sName;
    /* } SAFE */
}


void SAL_CALL Desktop::setName( const OUString& sName ) throw( css::uno::RuntimeException )
{
    /* SAFE { */
    WriteGuard aWriteLock( m_aLock );
    m_sName = sName;
    aWriteLock.unlock();
    /* } SAFE */
}


sal_Bool SAL_CALL Desktop::isTop() throw( css::uno::RuntimeException )
{
    return sal_True;
}


void SAL_CALL Desktop::activate() throw( css::uno::RuntimeException )
{
    
    
    
}


void SAL_CALL Desktop::deactivate() throw( css::uno::RuntimeException )
{
    
    
    
}


sal_Bool SAL_CALL Desktop::isActive() throw( css::uno::RuntimeException )
{
    return sal_True;
}


sal_Bool SAL_CALL Desktop::setComponent( const css::uno::Reference< css::awt::XWindow >&       /*xComponentWindow*/ ,
                                         const css::uno::Reference< css::frame::XController >& /*xController*/      ) throw( css::uno::RuntimeException )
{
    return sal_False;
}


css::uno::Reference< css::awt::XWindow > SAL_CALL Desktop::getComponentWindow() throw( css::uno::RuntimeException )
{
    return css::uno::Reference< css::awt::XWindow >();
}


css::uno::Reference< css::frame::XController > SAL_CALL Desktop::getController() throw( css::uno::RuntimeException )
{
    return css::uno::Reference< css::frame::XController >();
}


void SAL_CALL Desktop::contextChanged() throw( css::uno::RuntimeException )
{
}


void SAL_CALL Desktop::addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& ) throw( css::uno::RuntimeException )
{
}




void SAL_CALL Desktop::removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& ) throw( css::uno::RuntimeException )
{
}

/*-************************************************************************************************************
    @interface  XFrame
    @short      try to find a frame with special parameters
    @descr      This method searches for a frame with the specified name.
                Frames may contain other frames (e.g. a frameset) and may
                be contained in other frames. This hierarchie is searched by
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
*
css::uno::Reference< css::frame::XFrame > SAL_CALL Desktop::findFrame( const OUString& sTargetFrameName ,
                                                                             sal_Int32        nSearchFlags     ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XFrame > xTarget;

    
    
    
    
    
    
    
    if (
        (sTargetFrameName==SPECIALTARGET_DEFAULT  )   ||    
        (sTargetFrameName==SPECIALTARGET_MENUBAR  )   ||    
        (sTargetFrameName==SPECIALTARGET_PARENT   )   ||    
        (sTargetFrameName==SPECIALTARGET_BEAMER   )         
                                                            
       )
    {
        return NULL;
    }

    
    
    
    

    
    
    
    
    
    if ( sTargetFrameName==SPECIALTARGET_BLANK )
    {
        TaskCreator aCreator( m_xContext );
        xTarget = aCreator.createTask(sTargetFrameName,sal_False);
    }

    
    
    
    
    else if ( sTargetFrameName==SPECIALTARGET_TOP )
    {
        xTarget = this;
    }

    
    
    
    
    else if (
             ( sTargetFrameName==SPECIALTARGET_SELF ) ||
             ( sTargetFrameName.isEmpty()           )
            )
    {
        xTarget = this;
    }

    else
    {
        
        
        
        
        
        
        
        

        
        
        
        
        
        if (
            (nSearchFlags &  css::frame::FrameSearchFlag::SELF)  &&
            (m_sName == sTargetFrameName)
           )
        {
            xTarget = this;
        }

        
        
        
        
        
        
        
        
        
        if (
            ( ! xTarget.is()                                  ) &&
            (nSearchFlags & css::frame::FrameSearchFlag::TASKS)
           )
        {
            xTarget = m_aChildTaskContainer.searchOnDirectChildrens(sTargetFrameName);
        }

        
        
        
        
        
        
        
        if (
            ( ! xTarget.is()                                     ) &&
            (nSearchFlags & css::frame::FrameSearchFlag::CHILDREN)
           )
        {
            xTarget = m_aChildTaskContainer.searchOnAllChildrens(sTargetFrameName);
        }

        
        
        
        
        
        if (
            ( ! xTarget.is()                                   )    &&
            (nSearchFlags & css::frame::FrameSearchFlag::CREATE)
           )
        {
            TaskCreator aCreator( m_xContext );
            xTarget = aCreator.createTask(sTargetFrameName,sal_False);
        }
    }

    return xTarget;
}

void SAL_CALL Desktop::disposing()
    throw( css::uno::RuntimeException )
{
    
    

    
    
    SAL_WARN_IF( !m_bIsTerminated, "fwk", "Desktop disposed before terminating it" );

    WriteGuard aWriteLock( m_aLock ); 

    
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    
    
    
    
    
    
    aTransaction.stop();

    
    
    
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    aWriteLock.unlock(); 

    
    
    
    

    
    
    css::uno::Reference< css::uno::XInterface > xThis ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::lang::EventObject                      aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    
    
    
    m_aChildTaskContainer.clear();

    
    css::uno::Reference< css::lang::XEventListener > xFramesHelper( m_xFramesHelper, css::uno::UNO_QUERY );
    if( xFramesHelper.is() )
        xFramesHelper->disposing( aEvent );

    
    m_xDispatchHelper.clear();
    m_xFramesHelper.clear();
    m_xLastFrame.clear();
    m_xContext.clear();

    m_xPipeTerminator.clear();
    m_xQuickLauncher.clear();
    m_xSWThreadManager.clear();
    m_xSfxTerminator.clear();

    
    
    m_aTransactionManager.setWorkingMode( E_CLOSE );
}

/*-************************************************************************************************************
    @interface  XComponent
    @short      add/remove listener for dispose events
    @descr      Add an event listener to this object, if you wish to get information
                about our dieing!
                You must releas ethis listener reference during your own disposing() method.

    @attention  Our container is threadsafe himeslf. So we don't need any lock here.

    @seealso    -

    @param      "xListener", reference to valid listener. We don't accept invalid values!
    @return     -

    @onerror    -
    @threadsafe yes
*
void SAL_CALL Desktop::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    
    SAL_WARN_IF( implcp_addEventListener( xListener ), "fwk", "Desktop::addEventListener(): Invalid parameter detected!" );
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >*) NULL ), xListener );
}


void SAL_CALL Desktop::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    
    SAL_WARN_IF( implcp_removeEventListener( xListener ), "fwk", "Desktop::removeEventListener(): Invalid parameter detected!" );
    
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >*) NULL ), xListener );
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
    @return     -

    @onerror    -
    @threadsafe yes
*
void SAL_CALL Desktop::dispatchFinished( const css::frame::DispatchResultEvent& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
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

/*-************************************************************************************************************
    @interface  XEventListener
    @short      not implemented!
    @descr      We are a status listener ... and so we must be an event listener too ... But we don't need it really!
                We are a temp. listener only and our lifetime isn't smaller then of our temp. used dispatcher.

    @seealso    method loadComponentFromURL()

    @param      -
    @return     -

    @onerror    -
    @threadsafe -
*
void SAL_CALL Desktop::disposing( const css::lang::EventObject& ) throw( css::uno::RuntimeException )
{
    SAL_WARN( "fwk", "Desktop::disposing(): Algorithm error! Normaly desktop is temp. listener ... not all the time. So this method shouldn't be called." );
}

/*-************************************************************************************************************
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
*
void SAL_CALL Desktop::handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    

    
    
    
    
    css::uno::Any aRequest = xRequest->getRequest();

    
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

    
    
    css::task::ErrorCodeRequest          aErrorCodeRequest     ;
    css::document::AmbigousFilterRequest aAmbigousFilterRequest;
    if( aRequest >>= aAmbigousFilterRequest )
    {
        if( xFilterSelect.is() )
        {
            xFilterSelect->setFilter( aAmbigousFilterRequest.SelectedFilter ); 
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
    
    
    
    if (bAbort)
    {
        WriteGuard aWriteLock( m_aLock );
        m_eLoadState          = E_INTERACTION;
        m_aInteractionRequest = aRequest     ;
        aWriteLock.unlock();
    }
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
}


::sal_Int32 SAL_CALL Desktop::leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    return m_xTitleNumberGenerator->leaseNumber (xComponent);
}


void SAL_CALL Desktop::releaseNumber( ::sal_Int32 nNumber )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    m_xTitleNumberGenerator->releaseNumber (nNumber);
}


void SAL_CALL Desktop::releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    m_xTitleNumberGenerator->releaseNumberForComponent (xComponent);
}


OUString SAL_CALL Desktop::getUntitledPrefix()
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    return m_xTitleNumberGenerator->getUntitledPrefix ();
}

/*-************************************************************************************************************
    @short      try to convert a property value
    @descr      This method is called from helperclass "OPropertySetHelper".
                Don't use this directly!
                You must try to convert the value of given DESKTOP_PROPHANDLE and
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
*
sal_Bool SAL_CALL Desktop::convertFastPropertyValue(       css::uno::Any&   aConvertedValue ,
                                                           css::uno::Any&   aOldValue       ,
                                                           sal_Int32        nHandle         ,
                                                     const css::uno::Any&   aValue          ) throw( css::lang::IllegalArgumentException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
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

    
    return bReturn ;
}

/*-************************************************************************************************************
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
*
void SAL_CALL Desktop::setFastPropertyValue_NoBroadcast(       sal_Int32        nHandle ,
                                                         const css::uno::Any&   aValue  ) throw( css::uno::Exception )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
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

/*-************************************************************************************************************
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
*
void SAL_CALL Desktop::getFastPropertyValue( css::uno::Any& aValue  ,
                                             sal_Int32      nHandle ) const
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
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

/*-************************************************************************************************************
    @short      return structure and information about transient properties
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @attention  You must use global lock (method use static variable) ... and it must be the shareable osl mutex of it.
                Because; our baseclass use this mutex to make his code threadsafe. We use our lock!
                So we could have two different mutex/lock mechanism at the same object.

    @seealso    class OPropertySetHelper

    @param      -
    @return     structure with property-information

    @onerror    -
    @threadsafe yes
*
::cppu::IPropertyArrayHelper& SAL_CALL Desktop::getInfoHelper()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        
        if( pInfoHelper == NULL )
        {
            
            
            
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
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

    @param      -
    @return     reference to object with information [XPropertySetInfo]

    @onerror    -
    @threadsafe yes
*
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL Desktop::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    
    
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        
        ::osl::MutexGuard aGuard( LockHelper::getGlobalLock().getShareableOslMutex() );
        
        if( pInfo == NULL )
        {
            
            
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
                But we don't check these. Its an IMPL-method and caller must use it right!
    @return     A reference to found component.

    @onerror    A null reference is returned.
    @threadsafe yes
*
css::uno::Reference< css::lang::XComponent > Desktop::impl_getFrameComponent( const css::uno::Reference< css::frame::XFrame >& xFrame ) const
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    
    css::uno::Reference< css::lang::XComponent > xComponent;
    
    css::uno::Reference< css::frame::XController > xController = xFrame->getController();
    if( !xController.is() )
    {
        
        xComponent = css::uno::Reference< css::lang::XComponent >( xFrame->getComponentWindow(), css::uno::UNO_QUERY );
    }
    else
    {
        
        css::uno::Reference< css::frame::XModel > xModel( xController->getModel(), css::uno::UNO_QUERY );
        if( xModel.is() )
        {
            
            xComponent = css::uno::Reference< css::lang::XComponent >( xModel, css::uno::UNO_QUERY );
        }
        else
        {
            
            xComponent = css::uno::Reference< css::lang::XComponent >( xController, css::uno::UNO_QUERY );
        }
    }

    return xComponent;
}

/*-************************************************************************************************************
    @short      create table with information about properties
    @descr      We use a helper class to support properties. These class need some information about this.
                These method create a new static description table with name, type, r/w-flags and so on ...

    @seealso    class OPropertySetHelper
    @seealso    method getInfoHelper()

    @param      -
    @return     Static table with information about properties.

    @onerror    -
    @threadsafe yes
*
const css::uno::Sequence< css::beans::Property > Desktop::impl_getStaticPropertyDescriptor()
{
    
    
    
    
    
    

    const css::beans::Property pProperties[] =
    {
        css::beans::Property( DESKTOP_PROPNAME_ACTIVEFRAME              , DESKTOP_PROPHANDLE_ACTIVEFRAME             , ::getCppuType((const css::uno::Reference< css::lang::XComponent >*)NULL)                , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( DESKTOP_PROPNAME_DISPATCHRECORDERSUPPLIER , DESKTOP_PROPHANDLE_DISPATCHRECORDERSUPPLIER, ::getCppuType((const css::uno::Reference< css::frame::XDispatchRecorderSupplier >*)NULL), css::beans::PropertyAttribute::TRANSIENT ),
        css::beans::Property( DESKTOP_PROPNAME_ISPLUGGED                , DESKTOP_PROPHANDLE_ISPLUGGED               , ::getBooleanCppuType()                                                                  , css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY ),
        css::beans::Property( DESKTOP_PROPNAME_SUSPENDQUICKSTARTVETO    , DESKTOP_PROPHANDLE_SUSPENDQUICKSTARTVETO   , ::getBooleanCppuType()                                                                  , css::beans::PropertyAttribute::TRANSIENT ),
        css::beans::Property( DESKTOP_PROPNAME_TITLE                    , DESKTOP_PROPHANDLE_TITLE                   , ::getCppuType((const OUString*)NULL)                                             , css::beans::PropertyAttribute::TRANSIENT ),
    };
    
    const css::uno::Sequence< css::beans::Property > lPropertyDescriptor( pProperties, DESKTOP_PROPCOUNT );
    
    return lPropertyDescriptor;
}


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
            
            bVeto = sal_True;
            return;
        }
        catch( const css::uno::Exception& )
        {
            
            
            
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
            
            
            
            aIterator.remove();
        }
    }
}


::sal_Bool Desktop::impl_closeFrames(::sal_Bool bAllowUI)
{
    ReadGuard aReadLock( m_aLock ); 
    css::uno::Sequence< css::uno::Reference< css::frame::XFrame > > lFrames = m_aChildTaskContainer.getAllElements();
    aReadLock.unlock(); 

    ::sal_Int32 c                = lFrames.getLength();
    ::sal_Int32 i                = 0;
    ::sal_Int32 nNonClosedFrames = 0;

    for( i=0; i<c; ++i )
    {
        try
        {
            css::uno::Reference< css::frame::XFrame > xFrame = lFrames[i];

            
            
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

            
            
            
            css::uno::Reference< css::util::XCloseable > xClose( xFrame, css::uno::UNO_QUERY );
            if ( xClose.is() )
            {
                try
                {
                    xClose->close(sal_False);
                }
                catch(const css::util::CloseVetoException&)
                {
                    
                    
                    ++nNonClosedFrames;

                    
                    
                    
                    
                    if (
                        (bSuspended      ) &&
                        (xController.is())
                       )
                        xController->suspend(sal_False);
                }

                
                
                continue;
            }

            
            
            css::uno::Reference< css::lang::XComponent > xDispose( xFrame, css::uno::UNO_QUERY );
            if ( xDispose.is() )
                xDispose->dispose();

            
            
        }
        catch(const css::lang::DisposedException&)
        {
            
            
        }
    }

    return (nNonClosedFrames < 1);
}


sal_Bool Desktop::implcp_addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return !xListener.is();
}


sal_Bool Desktop::implcp_removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return !xListener.is();
}

}   

namespace {

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(new framework::Desktop(context)))
    {
        static_cast<framework::Desktop *>(static_cast<cppu::OWeakObject *>
                (instance.get()))->constructorInit();
    }

    css::uno::Reference<css::uno::XInterface> instance;
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
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
