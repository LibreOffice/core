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

#include <services/frame.hxx>
#include <dispatch/dispatchprovider.hxx>

#include <dispatch/interceptionhelper.hxx>
#include <dispatch/closedispatcher.hxx>
#include <dispatch/windowcommanddispatch.hxx>
#include <loadenv/loadenv.hxx>
#include <helper/oframes.hxx>
#include <helper/statusindicatorfactory.hxx>
#include <framework/titlehelper.hxx>
#include <classes/droptargetlistener.hxx>
#include <classes/taskcreator.hxx>
#include <loadenv/targethelper.hxx>
#include <framework/framelistanalyzer.hxx>
#include <helper/dockingareadefaultacceptor.hxx>
#include <dispatch/dispatchinformationprovider.hxx>
#include <threadhelp/transactionguard.hxx>
#include <pattern/window.hxx>
#include <services.h>
#include <properties.h>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/task/JobExecutor.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/awt/XDataTransferProviderAccess.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>

#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>

#include <toolkit/unohlp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/diagnose_ex.h>

#ifdef ENABLE_ASSERTIONS
    #include <rtl/strbuf.hxx>
#endif

#include <vcl/menu.hxx>

namespace framework{

css::uno::WeakReference< css::frame::XFrame > Frame::m_xCloserFrame = css::uno::WeakReference< css::frame::XFrame >();

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_21                (   Frame                                                                   ,
                                        OWeakObject                                                             ,
                                        DIRECT_INTERFACE(css::lang::XTypeProvider                               ),
                                        DIRECT_INTERFACE(css::lang::XServiceInfo                                ),
                                        DIRECT_INTERFACE(css::frame::XFramesSupplier                            ),
                                        DIRECT_INTERFACE(css::frame::XFrame                                     ),
                                        DIRECT_INTERFACE(css::lang::XComponent                                  ),
                                        DIRECT_INTERFACE(css::task::XStatusIndicatorFactory                     ),
                                        DIRECT_INTERFACE(css::frame::XDispatchProvider                          ),
                                        DIRECT_INTERFACE(css::frame::XDispatchInformationProvider               ),
                                        DIRECT_INTERFACE(css::frame::XDispatchProviderInterception              ),
                                        DIRECT_INTERFACE(css::beans::XPropertySet                               ),
                                        DIRECT_INTERFACE(css::beans::XPropertySetInfo                           ),
                                        DIRECT_INTERFACE(css::awt::XWindowListener                              ),
                                        DIRECT_INTERFACE(css::awt::XTopWindowListener                           ),
                                        DIRECT_INTERFACE(css::awt::XFocusListener                               ),
                                        DERIVED_INTERFACE(css::lang::XEventListener, css::awt::XWindowListener  ),
                                        DIRECT_INTERFACE(css::document::XActionLockable                         ),
                                        DIRECT_INTERFACE(css::util::XCloseable                                  ),
                                        DIRECT_INTERFACE(css::util::XCloseBroadcaster                           ),
                                        DIRECT_INTERFACE(css::frame::XComponentLoader                           ),
                                        DIRECT_INTERFACE(css::frame::XTitle                                     ),
                                        DIRECT_INTERFACE(css::frame::XTitleChangeBroadcaster                    )
                                    )

DEFINE_XTYPEPROVIDER_20             (   Frame                                                                   ,
                                        css::lang::XTypeProvider                                                ,
                                        css::lang::XServiceInfo                                                 ,
                                        css::frame::XFramesSupplier                                             ,
                                        css::frame::XFrame                                                      ,
                                        css::lang::XComponent                                                   ,
                                        css::task::XStatusIndicatorFactory                                      ,
                                        css::beans::XPropertySet                                                ,
                                        css::beans::XPropertySetInfo                                            ,
                                        css::frame::XDispatchProvider                                           ,
                                        css::frame::XDispatchInformationProvider                                ,
                                        css::frame::XDispatchProviderInterception                               ,
                                        css::awt::XWindowListener                                               ,
                                        css::awt::XTopWindowListener                                            ,
                                        css::awt::XFocusListener                                                ,
                                        css::lang::XEventListener                                               ,
                                        css::util::XCloseable                                                   ,
                                        css::util::XCloseBroadcaster                                            ,
                                        css::frame::XComponentLoader                                            ,
                                        css::frame::XTitle                                                      ,
                                        css::frame::XTitleChangeBroadcaster
                                    )

DEFINE_XSERVICEINFO_MULTISERVICE    (   Frame                                                                   ,
                                        ::cppu::OWeakObject                                                     ,
                                        SERVICENAME_FRAME                                                       ,
                                        IMPLEMENTATIONNAME_FRAME
                                    )

DEFINE_INIT_SERVICE                 (   Frame,
                                        {
                                            /*Attention
                                                I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                                                to create a new instance of this class by our own supported service factory.
                                                see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                                            */
                                            css::uno::Reference< css::uno::XInterface > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY_THROW);

                                            //-------------------------------------------------------------------------------------------------------------
                                            // Initialize a new dispatchhelper-object to handle dispatches.
                                            // We use these helper as slave for our interceptor helper ... not directly!
                                            // But he is event listener on THIS instance!
                                            DispatchProvider* pDispatchHelper = new DispatchProvider( m_xFactory, this );
                                            css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( static_cast< ::cppu::OWeakObject* >(pDispatchHelper), css::uno::UNO_QUERY );

                                            //-------------------------------------------------------------------------------------------------------------
                                            DispatchInformationProvider* pInfoHelper = new DispatchInformationProvider(m_xFactory, this);
                                            m_xDispatchInfoHelper = css::uno::Reference< css::frame::XDispatchInformationProvider >( static_cast< ::cppu::OWeakObject* >(pInfoHelper), css::uno::UNO_QUERY );

                                            //-------------------------------------------------------------------------------------------------------------
                                            // Initialize a new interception helper object to handle dispatches and implement an interceptor mechanism.
                                            // Set created dispatch provider as slowest slave of it.
                                            // Hold interception helper by reference only - not by pointer!
                                            // So it's easiear to destroy it.
                                            InterceptionHelper* pInterceptionHelper = new InterceptionHelper( this, xDispatchProvider );
                                            m_xDispatchHelper = css::uno::Reference< css::frame::XDispatchProvider >( static_cast< ::cppu::OWeakObject* >(pInterceptionHelper), css::uno::UNO_QUERY );

                                            //-------------------------------------------------------------------------------------------------------------
                                            // Initialize a new XFrames-helper-object to handle XIndexAccess and XElementAccess.
                                            // We hold member as reference ... not as pointer too!
                                            // Attention: We share our frame container with this helper. Container is threadsafe himself ... So I think we can do that.
                                            // But look on dispose() for right order of deinitialization.
                                            OFrames* pFramesHelper = new OFrames( m_xFactory, this, &m_aChildFrameContainer );
                                            m_xFramesHelper = css::uno::Reference< css::frame::XFrames >( static_cast< ::cppu::OWeakObject* >(pFramesHelper), css::uno::UNO_QUERY );

                                            //-------------------------------------------------------------------------------------------------------------
                                            // Initialize a the drop target listener.
                                            // We hold member as reference ... not as pointer too!
                                            DropTargetListener* pDropListener = new DropTargetListener( m_xFactory, this );
                                            m_xDropTargetListener = css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >( static_cast< ::cppu::OWeakObject* >(pDropListener), css::uno::UNO_QUERY );

                                            // Safe impossible cases
                                            // We can't work without these helpers!
                                            LOG_ASSERT2( xDispatchProvider.is    ()==sal_False, "Frame::impl_initService()", "Slowest slave for dispatch- and interception helper isn't valid. XDispatchProvider, XDispatch, XDispatchProviderInterception are not full supported!" )
                                            LOG_ASSERT2( m_xDispatchHelper.is    ()==sal_False, "Frame::impl_initService()", "Interception helper isn't valid. XDispatchProvider, XDispatch, XDispatchProviderInterception are not full supported!"                                 )
                                            LOG_ASSERT2( m_xFramesHelper.is      ()==sal_False, "Frame::impl_initService()", "Frames helper isn't valid. XFrames, XIndexAccess and XElementAcces are not supported!"                                                                )
                                            LOG_ASSERT2( m_xDropTargetListener.is()==sal_False, "Frame::impl_initService()", "DropTarget helper isn't valid. Drag and drop without functionality!"                                                                                  )

                                            //-------------------------------------------------------------------------------------------------------------
                                            // establish notifies for changing of "disabled commands" configuration during runtime
                                            m_aCommandOptions.EstablisFrameCallback(this);

                                            //-------------------------------------------------------------------------------------------------------------
                                            // Create an initial layout manager
                                            // Create layout manager and connect it to the newly created frame
                                            m_xLayoutManager = css::uno::Reference< css::frame::XLayoutManager >(m_xFactory->createInstance(SERVICENAME_LAYOUTMANAGER), css::uno::UNO_QUERY);

                                            //-------------------------------------------------------------------------------------------------------------
                                            // set information about all supported properties at the base class helper PropertySetHelper
                                            impl_initializePropInfo();
                                        }
                                    )

/*-****************************************************************************************************//**
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

    @onerror    ASSERT in debug version or nothing in relaese version.
*//*-*****************************************************************************************************/
Frame::Frame( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        :   ThreadHelpBase              ( &Application::GetSolarMutex()                     )
        ,   TransactionBase             (                                                   )
        ,   PropertySetHelper           ( xFactory,
                                          &m_aLock,
                                          &m_aTransactionManager,
                                          sal_False) // sal_False => dont release shared mutex on calling us!
        ,   ::cppu::OWeakObject         (                                                   )
        //  init member
        ,   m_xFactory                  ( xFactory                                          )
        ,   m_aListenerContainer        ( m_aLock.getShareableOslMutex()                    )
        ,   m_xParent                   (                                                   )
        ,   m_xContainerWindow          (                                                   )
        ,   m_xComponentWindow          (                                                   )
        ,   m_xController               (                                                   )
        ,   m_eActiveState              ( E_INACTIVE                                        )
        ,   m_sName                     (                                                   )
        ,   m_bIsFrameTop               ( sal_True                                          ) // I think we are top without a parent ... and there is no parent yet!
        ,   m_bConnected                ( sal_False                                         ) // There exist no component inside of use => sal_False, we are not connected!
        ,   m_nExternalLockCount        ( 0                                                 )
        ,   m_bSelfClose                ( sal_False                                         ) // Important!
        ,   m_bIsHidden                 ( sal_True                                          )
        ,   m_xTitleHelper              (                                                   )
        ,   m_pWindowCommandDispatch    ( 0                                                 )
        ,   m_aChildFrameContainer      (                                                   )
{
    // Check incoming parameter to avoid against wrong initialization.
    LOG_ASSERT2( implcp_ctor( xFactory ), "Frame::Frame()", "Invalid parameter detected!" )

    /* Please have a look on "@attentions" of description before! */
}

/*-****************************************************************************************************//**
    @short      standard destructor
    @descr      This one do NOTHING! Use dispose() instaed of this.

    @seealso    method dispose()

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
Frame::~Frame()
{
    LOG_ASSERT2( m_aTransactionManager.getWorkingMode()!=E_CLOSE, "Frame::~Frame()", "Who forgot to dispose this service?" )
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
css::uno::Reference< css::lang::XComponent > SAL_CALL Frame::loadComponentFromURL( const ::rtl::OUString&                                 sURL            ,
                                                                                   const ::rtl::OUString&                                 sTargetFrameName,
                                                                                         sal_Int32                                        nSearchFlags    ,
                                                                                   const css::uno::Sequence< css::beans::PropertyValue >& lArguments      ) throw( css::io::IOException                ,
                                                                                                                                                                   css::lang::IllegalArgumentException ,
                                                                                                                                                                   css::uno::RuntimeException          )
{
    {
        // If the frame is closed the call might lead to crash even with target "_blank",
        // so the DisposedException should be thrown in this case
        // It still looks to be too dangerous to set the transaction for the whole loading process
        // so the guard is used in scopes to let the standard check be used

        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    }

    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XComponentLoader > xThis(static_cast< css::frame::XComponentLoader* >(this), css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xFactory;
    aReadLock.unlock();

    return LoadEnv::loadComponentFromURL(xThis, xSMGR, sURL, sTargetFrameName, nSearchFlags, lArguments);
}

/*-****************************************************************************************************//**
    @short      return access to append or remove children on desktop
    @descr      We don't implement these interface directly. We use a helper class to do this.
                If you wish to add or delete children to/from the container, call these method to get
                a reference to the helper.

    @seealso    class OFrames

    @param      -
    @return     A reference to the helper which answer your queries.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrames > SAL_CALL Frame::getFrames() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.

/*TODO
    This is a temp. HACK!
    Our parent (a Task!) stand in close/dispose and set working mode to E_BEFOERECLOSE
    and call dispose on us! We tra to get this xFramesHelper and are reject by an "already closed" pranet instance ....
    => We use SOFTEXCEPTIONS here ... but we should make it right in further times ....
 */

    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Return access to all child frames to caller.
    // Ouer childframe container is implemented in helper class OFrames and used as a reference m_xFramesHelper!
    return m_xFramesHelper;
}

/*-****************************************************************************************************//**
    @short      get the current active child frame
    @descr      It must be a frameto. Direct children of a frame are frames only! No task or desktop is accepted.
                We don't save this information directly in this class. We use ouer container-helper
                to do that.

    @seealso    class OFrameContainer
    @seealso    method setActiveFrame()

    @param      -
    @return     A reference to ouer current active childframe, if anyone exist.
    @return     A null reference, if nobody is active.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > SAL_CALL Frame::getActiveFrame() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Return current active frame.
    // This information is avaliable on the container.
    return m_aChildFrameContainer.getActive();
}

/*-****************************************************************************************************//**
    @short      set the new active direct child frame
    @descr      It must be a frame to. Direct children of frame are frames only! No task or desktop is accepted.
                We don't save this information directly in this class. We use ouer container-helper
                to do that.

    @seealso    class OFrameContainer
    @seealso    method getActiveFrame()

    @param      "xFrame", reference to new active child. It must be an already existing child!
    @return     -

    @onerror    An assertion is thrown and element is ignored, if given frame is'nt already a child of us.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameters.
    LOG_ASSERT2( implcp_setActiveFrame( xFrame ), "Frame::setActiveFrame()", "Invalid parameter detected!" )
    // Look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    // Copy neccessary member for threadsafe access!
    // m_aChildFrameContainer is threadsafe himself and he live if we live!!!
    // ...and our transaction is non breakable too ...
    css::uno::Reference< css::frame::XFrame > xActiveChild = m_aChildFrameContainer.getActive();
    EActiveState                              eActiveState = m_eActiveState             ;

    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // Don't work, if "new" active frame is'nt different from current one!
    // (xFrame==NULL is allowed to UNSET it!)
    if( xActiveChild != xFrame )
    {
        // ... otherwise set new and deactivate old one.
        m_aChildFrameContainer.setActive( xFrame );
        if  (
                ( eActiveState      !=  E_INACTIVE  )   &&
                ( xActiveChild.is() ==  sal_True    )
            )
        {
            xActiveChild->deactivate();
        }
    }

    if( xFrame.is() == sal_True )
    {
        // If last active frame had focus ...
        // ... reset state to ACTIVE and send right FrameActionEvent for focus lost.
        if( eActiveState == E_FOCUS )
        {
            aWriteLock.lock();
            eActiveState   = E_ACTIVE    ;
            m_eActiveState = eActiveState;
            aWriteLock.unlock();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_DEACTIVATING );
        }

        // If last active frame was active ...
        // but new one isn't it ...
        // ... set it as active one.
        if  (
                ( eActiveState          ==  E_ACTIVE    )   &&
                ( xFrame->isActive()    ==  sal_False   )
            )
        {
            xFrame->activate();
        }
    }
    else
    // If this frame is active and has no active subframe anymore it is UI active too
    if( eActiveState == E_ACTIVE )
    {
        aWriteLock.lock();
        eActiveState   = E_FOCUS     ;
        m_eActiveState = eActiveState;
        aWriteLock.unlock();
        implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_ACTIVATED );
    }
}

/*-****************************************************************************************************//**
   initialize new created layout manager
**/
void lcl_enableLayoutManager(const css::uno::Reference< css::frame::XLayoutManager >& xLayoutManager,
                             const css::uno::Reference< css::frame::XFrame >&         xFrame        )
{
    // Provide container window to our layout manager implementation
    xLayoutManager->attachFrame(xFrame);

    css::uno::Reference< css::frame::XFrameActionListener > xListen(xLayoutManager, css::uno::UNO_QUERY_THROW);
    xFrame->addFrameActionListener(xListen);

    DockingAreaDefaultAcceptor* pAcceptor = new DockingAreaDefaultAcceptor(xFrame);
    css::uno::Reference< css::ui::XDockingAreaAcceptor > xDockingAreaAcceptor( static_cast< ::cppu::OWeakObject* >(pAcceptor), css::uno::UNO_QUERY_THROW);
    xLayoutManager->setDockingAreaAcceptor(xDockingAreaAcceptor);
}

/*-****************************************************************************************************//**
   deinitialize layout manager
**/
void lcl_disableLayoutManager(const css::uno::Reference< css::frame::XLayoutManager >& xLayoutManager,
                              const css::uno::Reference< css::frame::XFrame >&         xFrame        )
{
    css::uno::Reference< css::frame::XFrameActionListener > xListen(xLayoutManager, css::uno::UNO_QUERY_THROW);
    xFrame->removeFrameActionListener(xListen);
    xLayoutManager->setDockingAreaAcceptor(css::uno::Reference< css::ui::XDockingAreaAcceptor >());
    xLayoutManager->attachFrame(css::uno::Reference< css::frame::XFrame >());
}

/*-****************************************************************************************************//**
    @short      initialize frame instance
    @descr      A frame needs a window. This method set a new one ... but should called one times only!
                We use this window to listen for window events and forward it to our set component.
                Its used as parent of component window too.

    @seealso    method getContainerWindow()
    @seealso    method setComponent()
    @seealso    member m_xContainerWindow

    @param      "xWindow", reference to new container window - must be valid!
    @return     -

    @onerror    We do nothing.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::initialize( const css::uno::Reference< css::awt::XWindow >& xWindow ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    if (!xWindow.is())
        throw css::uno::RuntimeException(
                    ::rtl::OUString("Frame::initialize() called without a valid container window reference."),
                    static_cast< css::frame::XFrame* >(this));

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( m_xContainerWindow.is() )
        throw css::uno::RuntimeException(
                ::rtl::OUString("Frame::initialized() is called more then once, which isnt usefull nor allowed."),
                static_cast< css::frame::XFrame* >(this));

    // Look for rejected calls first!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    // Enable object for real working ... so follow impl methods don't must handle it special! (e.g. E_SOFTEXCEPTIONS for rejected calls)
    m_aTransactionManager.setWorkingMode( E_WORK );

    // This must be the first call of this method!
    // We should initialize our object and open it for working.
    // Set the new window.
    LOG_ASSERT2( m_xContainerWindow.is()==sal_True, "Frame::initialize()", "Leak detected! This state should never occure ..." )
    m_xContainerWindow = xWindow;

    // if window is initially visible, we will never get a windowShowing event
    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if (pWindow && pWindow->IsVisible())
        m_bIsHidden = sal_False;

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR          = m_xFactory;
    css::uno::Reference< css::frame::XLayoutManager >     xLayoutManager = m_xLayoutManager;

    // Release lock ... because we call some impl methods, which are threadsafe by himself.
    // If we hold this lock - we will produce our own deadlock!
    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if (xLayoutManager.is())
        lcl_enableLayoutManager(xLayoutManager, this);

    // create progress helper
    css::uno::Reference< css::frame::XFrame >                 xThis            (static_cast< css::frame::XFrame* >(this)                        , css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xIndicatorFactory(xSMGR->createInstance(IMPLEMENTATIONNAME_STATUSINDICATORFACTORY), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::lang::XInitialization >         xIndicatorInit   (xIndicatorFactory                                               , css::uno::UNO_QUERY_THROW);
    css::uno::Sequence< css::uno::Any > lArgs(2);
    css::beans::NamedValue aArg;
    aArg.Name    = STATUSINDICATORFACTORY_PROPNAME_FRAME;
    aArg.Value <<= xThis;
    lArgs[0]   <<= aArg;
    aArg.Name    = STATUSINDICATORFACTORY_PROPNAME_ALLOWPARENTSHOW;
    aArg.Value <<= sal_True;
    lArgs[1]   <<= aArg;
    xIndicatorInit->initialize(lArgs);

    // SAFE -> ----------------------------------
    aWriteLock.lock();
    m_xIndicatorFactoryHelper = xIndicatorFactory;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // Start listening for events after setting it on helper class ...
    // So superflous messages are filtered to NULL :-)
    implts_startWindowListening();

    impl_enablePropertySet();

    m_pWindowCommandDispatch = new WindowCommandDispatch(xSMGR, this);

    // Initialize title functionality
    TitleHelper* pTitleHelper = new TitleHelper(xSMGR);
    m_xTitleHelper = css::uno::Reference< css::frame::XTitle >(static_cast< ::cppu::OWeakObject* >(pTitleHelper), css::uno::UNO_QUERY_THROW);
    pTitleHelper->setOwner(xThis);
}

/*-****************************************************************************************************//**
    @short      returns current set container window
    @descr      The ContainerWindow property is used as a container for the component
                in this frame. So this object implements a container interface too.
                The instantiation of the container window is done by the user of this class.
                The frame is the owner of its container window.

    @seealso    method initialize()

    @param      -
    @return     A reference to current set containerwindow.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::awt::XWindow > SAL_CALL Frame::getContainerWindow() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return m_xContainerWindow;
}

/*-****************************************************************************************************//**
    @short      set parent frame
    @descr      We need a parent to support some functionality! e.g. findFrame()
                By the way we use the chance to set an internal information about our top state.
                So we must not check this information during every isTop() call.
                We are top, if our parent is the desktop instance or we havent any parent.

    @seealso    getCreator()
    @seealso    findFrame()
    @seealso    isTop()
    @seealos    m_bIsFrameTop

    @param      xCreator
                    valid reference to our new owner frame, which should implement a supplier interface

    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL Frame::setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator ) throw( css::uno::RuntimeException )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE { */
        WriteGuard aWriteLock( m_aLock );
            m_xParent = xCreator;
        aWriteLock.unlock();
    /* } SAFE */

    css::uno::Reference< css::frame::XDesktop > xIsDesktop( xCreator, css::uno::UNO_QUERY );
    m_bIsFrameTop = ( xIsDesktop.is() || ! xCreator.is() );
}

/*-****************************************************************************************************//**
    @short      returns current parent frame
    @descr      The Creator is the parent frame container. If it is NULL, the frame is the uppermost one.

    @seealso    method setCreator()

    @param      -
    @return     A reference to current set parent frame container.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFramesSupplier > SAL_CALL Frame::getCreator() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return m_xParent;
}

/*-****************************************************************************************************//**
    @short      returns current set name of frame
    @descr      This name is used to find target of findFrame() or queryDispatch() calls.

    @seealso    method setName()

    @param      -
    @return     Current set name of frame.

    @onerror    An empty string is returned.
*//*-*****************************************************************************************************/
::rtl::OUString SAL_CALL Frame::getName() throw( css::uno::RuntimeException )
{
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    return m_sName;
    /* } SAFE */
}

/*-****************************************************************************************************//**
    @short      set new name for frame
    @descr      This name is used to find target of findFrame() or queryDispatch() calls.

    @attention  Special names like "_blank", "_self" aren't allowed ...
                "_beamer" or "_menubar" excepts this rule!

    @seealso    method getName()

    @param      "sName", new frame name.
    @return     -

    @onerror    We do nothing.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::setName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException )
{
    /* SAFE { */
    WriteGuard aWriteLock( m_aLock );
    // Set new name ... but look for invalid special target names!
    // They are not allowed to set.
    if (TargetHelper::isValidNameForFrame(sName))
        m_sName = sName;
    aWriteLock.unlock();
    /* } SAFE */
}

/*-****************************************************************************************************//**
    @short      search for frames
    @descr      This method searches for a frame with the specified name.
                Frames may contain other frames (e.g. a frameset) and may
                be contained in other frames. This hierarchie ist searched by
                this method.
                First some special names are taken into account, i.e. "",
                "_self", "_top", "_blank" etc. The nSearchFlags are ignored
                when comparing these names with sTargetFrameName, further steps are
                controlled by the search flags. If allowed, the name of the frame
                itself is compared with the desired one, then ( again if allowed )
                the method findFrame() is called for all children, for siblings
                and as last for the parent frame.
                If no frame with the given name is found until the top frames container,
                a new top one is created, if this is allowed by a special
                flag. The new frame also gets the desired name.

    @param      sTargetFrameName
                    special names (_blank, _self) or real name of target frame
    @param      nSearchFlags
                    optional flags which regulate search for non special target frames

    @return     A reference to found or may be new created frame.
    @threadsafe yes
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > SAL_CALL Frame::findFrame( const ::rtl::OUString&  sTargetFrameName,
                                                                           sal_Int32         nSearchFlags    ) throw( css::uno::RuntimeException )
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
        (sTargetFrameName==SPECIALTARGET_MENUBAR  )         // valid for dispatches - not for findFrame()!
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
    css::uno::Reference< css::frame::XFrame >              xParent      ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory     = m_xFactory;
    sal_Bool                                               bIsTopFrame  = m_bIsFrameTop;
    sal_Bool                                               bIsTopWindow = WindowHelper::isTopWindow(m_xContainerWindow);
    aReadLock.unlock();
    /* } SAFE */

    //-----------------------------------------------------------------------------------------------------
    // I.I) "_blank"
    //  Not allowed for a normal frame - but for the desktop.
    //  Use helper class to do so. It use the desktop automaticly.
    //-----------------------------------------------------------------------------------------------------
    if ( sTargetFrameName==SPECIALTARGET_BLANK )
    {
        TaskCreator aCreator(xFactory);
        xTarget = aCreator.createTask(sTargetFrameName,sal_False);
    }

    //-----------------------------------------------------------------------------------------------------
    // I.II) "_parent"
    //  It doesn't matter if we have a valid parent or not. User ask for him and get it.
    //  An empty result is a valid result too.
    //-----------------------------------------------------------------------------------------------------
    else
    if ( sTargetFrameName==SPECIALTARGET_PARENT )
    {
        xTarget = xParent;
    }

    //-----------------------------------------------------------------------------------------------------
    // I.III) "_top"
    //  If we are not the top frame in this hierarchy, we must forward request to our parent.
    //  Otherwhise we must return ourself.
    //-----------------------------------------------------------------------------------------------------
    else
    if ( sTargetFrameName==SPECIALTARGET_TOP )
    {
        if (bIsTopFrame)
            xTarget = this;
        else
        if (xParent.is()) // If we are not top - the parent MUST exist. But may it's better to check it again .-)
            xTarget = xParent->findFrame(SPECIALTARGET_TOP,0);
    }

    //-----------------------------------------------------------------------------------------------------
    // I.IV) "_self", ""
    //  This mean this frame in every case.
    //-----------------------------------------------------------------------------------------------------
    else
    if (
        ( sTargetFrameName==SPECIALTARGET_SELF ) ||
        ( sTargetFrameName.isEmpty()           )
       )
    {
        xTarget = this;
    }

    //-----------------------------------------------------------------------------------------------------
    // I.V) "_beamer"
    //  This is a special sub frame of any task. We must return it if we found it on our direct childrens
    //  or create it there if it not already exists.
    //  Note: Such beamer exists for task(top) frames only!
    //-----------------------------------------------------------------------------------------------------
    else
    if ( sTargetFrameName==SPECIALTARGET_BEAMER )
    {
        // We are a task => search or create the beamer
        if (bIsTopWindow)
        {
            xTarget = m_aChildFrameContainer.searchOnDirectChildrens(SPECIALTARGET_BEAMER);
            if ( ! xTarget.is() )
            {
                /* TODO
                    Creation not supported yet!
                    Wait for new layout manager service because we can't plug it
                    inside already opened document of this frame ...
                */
            }
        }
        // We arent a task => forward request to our parent or ignore it.
        else
        if (xParent.is())
            xTarget = xParent->findFrame(SPECIALTARGET_BEAMER,0);
    }

    else
    {
        //-------------------------------------------------------------------------------------------------
        // II) otherwhise use optional given search flags
        //  force using of combinations of such flags. means no "else" part of use if() statements.
        //  But we ust break further searches if target was already found.
        //  Order of using flags is fix: SELF - CHILDREN - SIBLINGS - PARENT
        //  TASK and CREATE are handled special.
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
        // II.II) CHILDREN
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
            xTarget = m_aChildFrameContainer.searchOnAllChildrens(sTargetFrameName);
        }

        //-------------------------------------------------------------------------------------------------
        // II.III) TASKS
        //  This is a special flag. It regulate search on this task tree only or allow search on
        //  all other ones (which are sibling trees of us) too.
        //  Upper search must stop at this frame if we are the topest one and the TASK flag isn't set
        //  or we can ignore it if we have no valid parent.
        //-------------------------------------------------------------------------------------------------
        if (
            (   bIsTopFrame && (nSearchFlags & css::frame::FrameSearchFlag::TASKS) )   ||
            ( ! bIsTopFrame                                                        )
           )
        {
            //-------------------------------------------------------------------------------------------------
            // II.III.I) SIBLINGS
            //  Search on all our direct siblings - means all childrens of our parent.
            //  Use this flag in combination with TASK. We must supress such upper search if
            //  user has not set it and if we are a top frame.
            //  Attention: Don't forward this request to our parent as a findFrame() call.
            //  In such case we must protect us against recursive calls.
            //  Use snapshot of our parent. But don't use queryFrames() of XFrames interface.
            //  Because it's return all siblings and all her childrens including our children too
            //  if we call it with the CHILDREN flag. We doesn't need that - we need the direct container
            //  items of our parent only to start searches there. So we must use the container interface
            //  XIndexAccess instead of XFrames.
            //-------------------------------------------------------------------------------------------------
            if (
                ( ! xTarget.is()                                      ) &&
                (nSearchFlags &  css::frame::FrameSearchFlag::SIBLINGS) &&
                (   xParent.is()                                      ) // search on siblings is impossible without a parent
               )
            {
                css::uno::Reference< css::frame::XFramesSupplier > xSupplier( xParent, css::uno::UNO_QUERY );
                if (xSupplier.is())
                {
                    css::uno::Reference< css::container::XIndexAccess > xContainer( xSupplier->getFrames(), css::uno::UNO_QUERY );
                    if (xContainer.is())
                    {
                        sal_Int32 nCount = xContainer->getCount();
                        for( sal_Int32 i=0; i<nCount; ++i )
                        {
                            css::uno::Reference< css::frame::XFrame > xSibling;
                            if (
                                ( !(xContainer->getByIndex(i)>>=xSibling)                                 ) ||  // control unpacking
                                ( ! xSibling.is()                                     ) ||  // check for valid items
                                ( xSibling==static_cast< ::cppu::OWeakObject* >(this) )     // ignore ourself! (We are a part of this container too - but search on our children was already done.)
                            )
                            {
                                continue;
                            }

                            // Don't allow upper search here! Use rigth flags to regulate it.
                            // And allow deep search on children only - if it was allowed for us too.
                            sal_Int32 nRightFlags = css::frame::FrameSearchFlag::SELF;
                            if (nSearchFlags & css::frame::FrameSearchFlag::CHILDREN)
                                nRightFlags |= css::frame::FrameSearchFlag::CHILDREN;
                            xTarget = xSibling->findFrame(sTargetFrameName, nRightFlags );
                            // perform search be breaking further search if a result exist.
                            if (xTarget.is())
                                break;
                        }
                    }
                }
            }

            //-------------------------------------------------------------------------------------------------
            // II.III.II) PARENT
            //  Forward search to our parent (if he exists.)
            //  To prevent us against recursive and superflous calls (which can occure if we allow him
            //  to search on his childrens too) we must change used search flags.
            //-------------------------------------------------------------------------------------------------
            if (
                ( ! xTarget.is()                                    ) &&
                (nSearchFlags &  css::frame::FrameSearchFlag::PARENT) &&
                (   xParent.is()                                    )
               )
            {
                if (xParent->getName() == sTargetFrameName)
                    xTarget = xParent;
                else
                {
                    sal_Int32 nRightFlags  = nSearchFlags;
                              nRightFlags &= ~css::frame::FrameSearchFlag::CHILDREN;
                    xTarget = xParent->findFrame(sTargetFrameName, nRightFlags);
                }
            }
        }

        //-------------------------------------------------------------------------------------------------
        // II.IV) CREATE
        //  If we haven't found any valid target frame by using normal flags - but user allowed us to create
        //  a new one ... we should do that. Used TaskCreator use Desktop instance automaticly as parent!
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

/*-****************************************************************************************************//**
    @short      -
    @descr      Returns sal_True, if this frame is a "top frame", otherwise sal_False.
                The "m_bIsFrameTop" member must be set in the ctor or setCreator() method.
                A top frame is a member of the top frame container or a member of the
                task frame container. Both containers can create new frames if the findFrame()
                method of their css::frame::XFrame interface is called with a frame name not yet known.

    @seealso    ctor
    @seealso    method setCreator()
    @seealso    method findFrame()

    @param      -
    @return     true, if is it a top frame ... false otherwise.

    @onerror    No error should occure!
*//*-*****************************************************************************************************/
sal_Bool SAL_CALL Frame::isTop() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // This information is set in setCreator().
    // We are top, if ouer parent is a task or the desktop or if no parent exist!
    return m_bIsFrameTop;
}

/*-****************************************************************************************************//**
    @short      activate frame in hierarchy
    @descr      This feature is used to mark active paths in our frame hierarchy.
                You can be a listener for this event to react for it ... change some internal states or something else.

    @seealso    method deactivate()
    @seealso    method isActivate()
    @seealso    enum EActiveState
    @seealso    listener mechanism

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::activate() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    // Copy neccessary member and free the lock.
    // It's not neccessary for m_aChildFrameContainer ... because
    // he is threadsafe himself and live if we live.
    // We use a registered transaction to prevent us against
    // breaks during this operation!
    css::uno::Reference< css::frame::XFrame >           xActiveChild    = m_aChildFrameContainer.getActive()                                ;
    css::uno::Reference< css::frame::XFramesSupplier >  xParent         ( m_xParent, css::uno::UNO_QUERY )                                ;
    css::uno::Reference< css::frame::XFrame >           xThis           ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow >            xComponentWindow( m_xComponentWindow, css::uno::UNO_QUERY )                       ;
    EActiveState                                        eState          = m_eActiveState                                                  ;

    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    //_________________________________________________________________________________________________________
    //  1)  If I'am not active before ...
    if( eState == E_INACTIVE )
    {
        // ... do it then.
        aWriteLock.lock();
        eState         = E_ACTIVE;
        m_eActiveState = eState;
        aWriteLock.unlock();
        // Deactivate sibling path and forward activation to parent ... if any parent exist!
        if( xParent.is() == sal_True )
        {
            // Everytime set THIS frame as active child of parent and activate it.
            // We MUST have a valid path from bottom to top as active path!
            // But we must deactivate the old active sibling path first.

            // Attention: Deactivation of an active path, deactivate the whole path ... from bottom to top!
            // But we wish to deactivate founded sibling-tree only.
            // [ see deactivate() / step 4) for further informations! ]

            xParent->setActiveFrame( xThis );

            // Then we can activate from here to top.
            // Attention: We are ACTIVE now. And the parent will call activate() at us!
            // But we do nothing then! We are already activated.
            xParent->activate();
        }
        // Its neccessary to send event NOW - not before.
        // Activation goes from bottom to top!
        // Thats the reason to activate parent first and send event now.
        implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_ACTIVATED );
    }

    //_________________________________________________________________________________________________________
    //  2)  I was active before or current activated and there is a path from here to bottom, who CAN be active.
    //      But ouer direct child of path is not active yet.
    //      (It can be, if activation occur in the middle of a current path!)
    //      In these case we activate path to bottom to set focus on right frame!
    if  (
            ( eState                    ==  E_ACTIVE    )   &&
            ( xActiveChild.is()         ==  sal_True    )   &&
            ( xActiveChild->isActive()  ==  sal_False   )
        )
    {
        xActiveChild->activate();
    }

    //_________________________________________________________________________________________________________
    //  3)  I was active before or current activated. But if I have no active child => I will get the focus!
    if  (
            ( eState            ==  E_ACTIVE    )   &&
            ( xActiveChild.is() ==  sal_False   )
        )
    {
        aWriteLock.lock();
        eState         = E_FOCUS;
        m_eActiveState = eState;
        aWriteLock.unlock();
        implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_ACTIVATED );
    }
}

/*-****************************************************************************************************//**
    @short      deactivate frame in hierarchy
    @descr      This feature is used to deactive paths in our frame hierarchy.
                You can be a listener for this event to react for it ... change some internal states or something else.

    @seealso    method activate()
    @seealso    method isActivate()
    @seealso    enum EActiveState
    @seealso    listener mechanism

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::deactivate() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    // Copy neccessary member and free the lock.
    css::uno::Reference< css::frame::XFrame >           xActiveChild    = m_aChildFrameContainer.getActive()                                     ;
    css::uno::Reference< css::frame::XFramesSupplier >  xParent         ( m_xParent, css::uno::UNO_QUERY )                                ;
    css::uno::Reference< css::frame::XFrame >           xThis           ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    EActiveState                                        eState          = m_eActiveState                                                  ;

    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // Work only, if there something to do!
    if( eState != E_INACTIVE )
    {
        //_____________________________________________________________________________________________________
        //  1)  Deactivate all active children.
        if  (
                ( xActiveChild.is()         ==  sal_True    )   &&
                ( xActiveChild->isActive()  ==  sal_True    )
            )
        {
            xActiveChild->deactivate();
        }

        //_____________________________________________________________________________________________________
        //  2)  If I have the focus - I will lost it now.
        if( eState == E_FOCUS )
        {
            // Set new state INACTIVE(!) and send message to all listener.
            // Don't set ACTIVE as new state. This frame is deactivated for next time - due to activate().
            aWriteLock.lock();
            eState          = E_ACTIVE;
            m_eActiveState  = eState  ;
            aWriteLock.unlock();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_UI_DEACTIVATING );
        }

        //_____________________________________________________________________________________________________
        //  3)  If I'am active - I will be deactivated now.
        if( eState == E_ACTIVE )
        {
            // Set new state and send message to all listener.
            aWriteLock.lock();
            eState          = E_INACTIVE;
            m_eActiveState  = eState    ;
            aWriteLock.unlock();
            implts_sendFrameActionEvent( css::frame::FrameAction_FRAME_DEACTIVATING );
        }

        //_____________________________________________________________________________________________________
        //  4)  If there is a path from here to my parent ...
        //      ... I'am on the top or in the middle of deactivated subtree and action was started here.
        //      I must deactivate all frames from here to top, which are members of current path.
        //      Stop, if THESE frame not the active frame of ouer parent!
        if  (
                ( xParent.is()              ==  sal_True    )   &&
                ( xParent->getActiveFrame() ==  xThis       )
            )
        {
            // We MUST break the path - otherwise we will get the focus - not ouer parent! ...
            // Attention: Ouer parent don't call us again - WE ARE NOT ACTIVE YET!
            // [ see step 3 and condition "if ( m_eActiveState!=INACTIVE ) ..." in this method! ]
            xParent->deactivate();
        }
    }
}

/*-****************************************************************************************************//**
    @short      returns active state
    @descr      Call it to get informations about current active state of this frame.

    @seealso    method activate()
    @seealso    method deactivate()
    @seealso    enum EActiveState

    @param      -
    @return     true if active, false otherwise.

    @onerror    No error should occure.
*//*-*****************************************************************************************************/
sal_Bool SAL_CALL Frame::isActive() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return  (
                ( m_eActiveState    ==  E_ACTIVE    )   ||
                ( m_eActiveState    ==  E_FOCUS     )
            );
}

/*-****************************************************************************************************//**
    @short      ???
    @descr      -

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::contextChanged() throw( css::uno::RuntimeException )
{
    // Look for rejected calls!
    // Sometimes called during closing object... => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );
    // Impl-method is threadsafe himself!
    // Send event to all listener for frame actions.
    implts_sendFrameActionEvent( css::frame::FrameAction_CONTEXT_CHANGED );
}

/*-****************************************************************************************************//**
    @short      set new component inside the frame
    @descr      A frame is a container for a component. Use this method to set, change or realease it!
                We accept null references! The xComponentWindow will be a child of our container window
                and get all window events from us.

    @attention  (a) A current set component can disagree with the suspend() request!
                    We don't set the new one and return with false then.
                (b) It's possible to set:
                        (b1) a simple component here which supports the window only - no controller;
                        (b2) a full featured component which supports window and controller;
                        (b3) or both to NULL if outside code which to forget this component.

    @seealso    method getComponentWindow()
    @seealso    method getController()

    @param      xComponentWindow
                    valid reference to new component window which will be a child of internal container window
                    May <NULL/> for releasing.
    @param      xController
                    reference to new component controller
                    (may <NULL/> for relasing or setting of a simple component)

    @return     <TRUE/> if operation was successful, <FALSE/> otherwise.

    @onerror    We return <FALSE/>.
    @threadsafe yes
*//*-*****************************************************************************************************/
sal_Bool SAL_CALL Frame::setComponent(  const   css::uno::Reference< css::awt::XWindow >&       xComponentWindow ,
                                        const   css::uno::Reference< css::frame::XController >& xController      ) throw( css::uno::RuntimeException )
{
    //_____________________________________________________________________________________________________
    // Ignore this HACK of sfx2!
    // He call us with an valid controller without a valid window ... Thats not allowed!
    if  ( xController.is() && ! xComponentWindow.is() )
        return sal_True;

    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    //_____________________________________________________________________________________________________
    // Get threadsafe some copies of used members.
    /* SAFE { */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XWindow >       xContainerWindow    = m_xContainerWindow;
    css::uno::Reference< css::awt::XWindow >       xOldComponentWindow = m_xComponentWindow;
    css::uno::Reference< css::frame::XController > xOldController      = m_xController;
    Window*                                        pOwnWindow = VCLUnoHelper::GetWindow( xContainerWindow );
    sal_Bool                                       bHadFocus           = pOwnWindow->HasChildPathFocus();
    sal_Bool                                       bWasConnected       = m_bConnected;
    aReadLock.unlock();
    /* } SAFE */

    //_____________________________________________________________________________________________________
    // stop listening on old window
    // May it produce some trouble.
    // But don't forget to listen on new window again ... or reactivate listening
    // if we reject this setComponent() request and leave this method without changing the old window.
    implts_stopWindowListening();

    // Notify all listener, that this component (if current one exist) will be unloaded.
    if (bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_DETACHING );

    //_____________________________________________________________________________________________________
    // otherwhise release old component first
    // Always release controller before releasing window,
    // because controller may want to access its window!
    // But check for real changes - may the new controller is the old one.
    if (
        (xOldController.is()          )   &&
        (xOldController != xController)
       )
    {
        /* ATTENTION
            Don't suspend the old controller here. Because the outside caller must do that
            by definition. We have to dispose it here only.
         */

        // Before we dispose this controller we should hide it inside this frame instance.
        // We hold it alive for next calls by using xOldController!
        /* SAFE {*/
        WriteGuard aWriteLock( m_aLock );
        m_xController = NULL;
        aWriteLock.unlock();
        /* } SAFE */

        css::uno::Reference< css::lang::XComponent > xDisposable( xOldController, css::uno::UNO_QUERY );
        if (xDisposable.is())
        {
            try
            {
                xDisposable->dispose();
            }
            catch(const css::lang::DisposedException&)
                {}
        }
        xOldController = NULL;
    }

    //_____________________________________________________________________________________________________
    // Now it's time to release the component window.
    // If controller wasn't released successfully - this code line shouldn't be reached.
    // Because in case of "suspend()==false" we return immediately with false ...
    // see before
    // Check for real changes too.
    if (
        (xOldComponentWindow.is()               )   &&
        (xOldComponentWindow != xComponentWindow)
       )
    {
        /* SAFE { */
        WriteGuard aWriteLock( m_aLock );
        m_xComponentWindow = NULL;
        aWriteLock.unlock();
        /* } SAFE */

        css::uno::Reference< css::lang::XComponent > xDisposable( xOldComponentWindow, css::uno::UNO_QUERY );
        if (xDisposable.is())
        {
            try
            {
                xDisposable->dispose();
            }
            catch(const css::lang::DisposedException&)
                {}
        }
        xOldComponentWindow = NULL;
    }

    //_____________________________________________________________________________________________________
    // Now it's time to set the new component ...
    // By the way - find out our new "load state" - means if we have a valid component inside.
    /* SAFE { */
    WriteGuard aWriteLock( m_aLock );
    m_xComponentWindow = xComponentWindow;
    m_xController      = xController     ;
    m_bConnected       = (m_xComponentWindow.is() || m_xController.is());
    sal_Bool bIsConnected       = m_bConnected;
    aWriteLock.unlock();
    /* } SAFE */

    //_____________________________________________________________________________________________________
    // notifies all interest listener, that current component was changed or a new one was loaded
    if (bIsConnected && bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_REATTACHED );
    else
    if (bIsConnected && !bWasConnected)
        implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_ATTACHED   );

    //_____________________________________________________________________________________________________
    // A new component window doesn't know anything about current active/focus states.
    // Set this information on it!
    if (
        (bHadFocus            ) &&
        (xComponentWindow.is())
       )
    {
        xComponentWindow->setFocus();
    }

    // If it was a new component window - we must resize it to fill out
    // our container window.
    implts_resizeComponentWindow();
    // New component should change our current icon ...
    implts_setIconOnWindow();
    // OK - start listening on new window again - or do nothing if it is an empty one.
    implts_startWindowListening();

    /* SAFE { */
    aWriteLock.lock();
    impl_checkMenuCloser();
    aWriteLock.unlock();
    /* } SAFE */

    return sal_True;
}

/*-****************************************************************************************************//**
    @short      returns current set component window
    @descr      Frames are used to display components. The actual displayed component is
                held by the m_xComponentWindow property. If the component implements only a
                XComponent interface, the communication between the frame and the
                component is very restricted. Better integration is achievable through a
                XController interface.
                If the component wants other objects to be able to get information about its
                ResourceDescriptor it has to implement a XModel interface.
                This frame is the owner of the component window.

    @seealso    method setComponent()

    @param      -
    @return     css::uno::Reference to current set component window.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::awt::XWindow > SAL_CALL Frame::getComponentWindow() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return m_xComponentWindow;
}

/*-****************************************************************************************************//**
    @short      returns current set controller
    @descr      Frames are used to display components. The actual displayed component is
                held by the m_xComponentWindow property. If the component implements only a
                XComponent interface, the communication between the frame and the
                component is very restricted. Better integration is achievable through a
                XController interface.
                If the component wants other objects to be able to get information about its
                ResourceDescriptor it has to implement a XModel interface.
                This frame is the owner of the component window.

    @seealso    method setComponent()

    @param      -
    @return     css::uno::Reference to current set controller.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XController > SAL_CALL Frame::getController() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return m_xController;
}

/*-****************************************************************************************************//**
    @short      add/remove listener for activate/deactivate/contextChanged events
    @descr      -

    @seealso    method activate()
    @seealso    method deactivate()
    @seealso    method contextChanged()

    @param      "xListener" reference to your listener object
    @return     -

    @onerror    Listener is ignored.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_addFrameActionListener( xListener ), "Frame::addFrameActionListener()", "Invalid parameter detected." )
    // Listener container is threadsafe by himself ... but we must look for rejected calls!
    // Our OMenuDispatch-helper (is a member of ODispatchProvider!) is create at startup of this frame BEFORE initialize!
    // => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.addInterface( ::getCppuType( (const css::uno::Reference< css::frame::XFrameActionListener >*)NULL ), xListener );
}

//*****************************************************************************************************************
void SAL_CALL Frame::removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_removeFrameActionListener( xListener ), "Frame::removeFrameActionListener()", "Invalid parameter detected." )
    // Listener container is threadsafe by himself ... but we must look for rejected calls after disposing!
    // But we must work with E_SOFTEXCEPTIONS ... because sometimes we are called from our listeners
    // during dispose! Our work mode is E_BEFORECLOSE then ... and E_HARDEXCEPTIONS whould throw a DisposedException.
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( (const css::uno::Reference< css::frame::XFrameActionListener >*)NULL ), xListener );
}

/*-****************************************************************************************************//**
    @short      support two way mechanism to release a frame
    @descr      This method ask internal component (controller) if he accept this close request.
                In case of <TRUE/> nothing will be happen (from point of caller of this close method).
                In case of <FALSE/> a CloseVetoException is thrown. After such exception given parameter
                <var>bDeliverOwnerShip</var> regulate which will be the new owner of this instance.

    @attention  It's the replacement for XTask::close() which is marked as obsolete method.

    @param      bDeliverOwnerShip
                    If parameter is set to <FALSE/> the original caller will be the owner after thrown
                    veto exception and must try to close this frame at later time again. Otherwhise the
                    source of throwed exception is the right one. May it will be the frame himself.

    @thrown     CloseVetoException
                    if any internal things willn't be closed

    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL Frame::close( sal_Bool bDeliverOwnerShip ) throw( css::util::CloseVetoException,
                                                                css::uno::RuntimeException   )
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // At the end of this method may we must dispose ourself ...
    // and may nobody from outside hold a reference to us ...
    // then it's a good idea to do that by ourself.
    css::uno::Reference< css::uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );

    // Check any close listener before we look for currently running internal processes.
    // Because if a listener disagree with this close() request - we hace time to finish this
    // internal operations too ...
    // Note: container is threadsafe himself.
    css::lang::EventObject             aSource    (static_cast< ::cppu::OWeakObject*>(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((css::util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnerShip );
            }
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }

    // Ok - no listener disagreed with this close() request
    // check if this frame is used for any load process currently
    if (isActionLocked())
    {
        if (bDeliverOwnerShip)
        {
            /* SAFE */
            WriteGuard aWriteLock( m_aLock );
            m_bSelfClose = sal_True;
            aWriteLock.unlock();
            /* SAFE */
        }

        throw css::util::CloseVetoException(DECLARE_ASCII("Frame in use for loading document ..."),static_cast< ::cppu::OWeakObject*>(this));
    }

    if ( ! setComponent(NULL,NULL) )
        throw css::util::CloseVetoException(DECLARE_ASCII("Component couldn't be deattached ..."),static_cast< ::cppu::OWeakObject*>(this));

    // If closing is allowed ... inform all istener and dispose this frame!
    pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((css::util::XCloseListener*)pIterator.next())->notifyClosing( aSource );
            }
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }

    /* SAFE { */
    WriteGuard aWriteLock( m_aLock );
    m_bIsHidden = sal_True;
    aWriteLock.unlock();
    /* } SAFE */
    impl_checkMenuCloser();

    // Attention: We must release our own registered transaction here. Otherwhise following dispose() call
    // wait for us too ....
    aTransaction.stop();
    dispose();
}

/*-****************************************************************************************************//**
    @short      be a listener for close events!
    @descr      Adds/remove a CloseListener at this frame instance. If the close() method is called on
                this object, the such listener are informed and can disagree with that by throwing
                a CloseVetoException.

    @seealso    Frame::close()

    @param      xListener
                    reference to your listener object

    @onerror    Listener is ignored.

    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL Frame::addCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
    // We doesn't need any lock here ...
    // Container lives if we live and is threadsafe by himself.
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::util::XCloseListener >* ) NULL ), xListener );
}

//*****************************************************************************************************************
void SAL_CALL Frame::removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) throw (css::uno::RuntimeException)
{
    // Use soft exception mode - moslty this method is called during disposing of this frame ...
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );
    // We doesn't need any lock here ...
    // Container lives if we live and is threadsafe by himself.
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::util::XCloseListener >* ) NULL ), xListener );
}

//*****************************************************************************************************************
::rtl::OUString SAL_CALL Frame::getTitle()
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XTitle > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    // <- SAFE

    return xTitle->getTitle();
}

//*****************************************************************************************************************
void SAL_CALL Frame::setTitle( const ::rtl::OUString& sTitle )
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XTitle > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    // <- SAFE

    xTitle->setTitle(sTitle);
}

//*****************************************************************************************************************
void SAL_CALL Frame::addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    // <- SAFE

    xTitle->addTitleChangeListener(xListener);
}

//*****************************************************************************************************************
void SAL_CALL Frame::removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
    throw (css::uno::RuntimeException)
{
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xTitle(m_xTitleHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    // <- SAFE

    xTitle->removeTitleChangeListener(xListener);
}

/*-****************************************************************************************************/
void Frame::implts_forgetSubFrames()
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::container::XIndexAccess > xContainer(m_xFramesHelper, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    // <- SAFE

    sal_Int32 c = xContainer->getCount();
    sal_Int32 i = 0;

    for (i=0; i<c; ++i)
    {
        try
        {
            css::uno::Reference< css::frame::XFrame > xFrame;
            xContainer->getByIndex(i) >>= xFrame;
            if (xFrame.is())
                xFrame->setCreator(css::uno::Reference< css::frame::XFramesSupplier >());
        }
        catch(const css::uno::Exception&)
        {
            // Ignore errors here.
            // Nobody can guarantee a stable index in multi threaded environments .-)
        }
    }

    // SAFE ->
    WriteGuard aWriteLock(m_aLock);
    m_xFramesHelper.clear(); // clear uno reference
    m_aChildFrameContainer.clear(); // clear container content
    aWriteLock.unlock();
    // <- SAFE
}

/*-****************************************************************************************************//**
    @short      destroy instance
    @descr      The owner of this object calles the dispose method if the object
                should be destroyed. All other objects and components, that are registered
                as an EventListener are forced to release their references to this object.
                Furthermore this frame is removed from its parent frame container to release
                this reference. The reference attributes are disposed and released also.

    @attention  Look for globale description at beginning of file too!
                (DisposedException, FairRWLock ..., initialize, dispose)

    @seealso    method initialize()
    @seealso    baseclass FairRWLockBase!

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::dispose() throw( css::uno::RuntimeException )
{
    // We should hold a reference to ourself ...
    // because our owner dispose us and release our reference ...
    // May be we will die before we could finish this method ...
    css::uno::Reference< css::frame::XFrame > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    LOG_DISPOSEEVENT( "Frame", sName )

    // First operation should be ... "stopp all listening for window events on our container window".
    // These events are superflous but can make trouble!
    // We will die, die and die ...
    implts_stopWindowListening();

    if (m_xLayoutManager.is())
        lcl_disableLayoutManager(m_xLayoutManager, this);

    delete m_pWindowCommandDispatch;

    // Send message to all listener and forget her references.
    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    // set "end of live" for our property set helper
    impl_disablePropertySet();

    // interception/dispatch chain must be destructed explicitly
    // Otherwhise some dispatches and/or interception objects wont die.
    css::uno::Reference< css::lang::XEventListener > xDispatchHelper(m_xDispatchHelper, css::uno::UNO_QUERY_THROW);
    xDispatchHelper->disposing(aEvent);
    xDispatchHelper.clear();

    // Disable this instance for further work.
    // This will wait for all current running ones ...
    // and reject all further requests!
    m_aTransactionManager.setWorkingMode( E_BEFORECLOSE );

    // Don't show any dialogs, errors or something else any more!
    // If somewhere called dispose() whitout close() before - normaly no dialogs
    // should exist. Otherwhise it's the problem of the outside caller.
    // Note:
    //      (a) Do it after stopWindowListening(). May that force some active/deactive
    //          notifications which we doesn't need here realy.
    //      (b) Don't forget to save the old value of IsDialogCancelEnabled() to
    //          restore it afterwards (to not kill headless mode).
    Application::DialogCancelMode old = Application::GetDialogCancelMode();
    Application::SetDialogCancelMode( Application::DIALOG_CANCEL_SILENT );

    // We should be alone for ever and further dispose calls are rejected by lines before ...
    // I hope it :-)

    // Free references of our frame tree.
    // Force parent container to forget this frame too ...
    // ( It's contained in m_xParent and so no css::lang::XEventListener for m_xParent! )
    // It's important to do that before we free some other internal structures.
    // Because if our parent gets an activate and found us as last possible active frame
    // he try to deactivate us ... and we run into some trouble (DisposedExceptions!).
    if( m_xParent.is() == sal_True )
    {
        m_xParent->getFrames()->remove( xThis );
        m_xParent = css::uno::Reference< css::frame::XFramesSupplier >();
    }

    /* } SAFE */
    // Forget our internal component and her window first.
    // So we can release our container window later without problems.
    // Because this container window is the parent of the component window ...
    // Note: Dispose it hard - because suspending must be done inside close() call!
    // But try to dispose the controller first befor you destroy the window.
    // Because the window is used by the controller too ...
    if (m_xController.is())
    {
        css::uno::Reference< css::lang::XComponent > xDisposable( m_xController, css::uno::UNO_QUERY );
        if (xDisposable.is())
            xDisposable->dispose();
    }

    if (m_xComponentWindow.is())
    {
        css::uno::Reference< css::lang::XComponent > xDisposable( m_xComponentWindow, css::uno::UNO_QUERY );
        if (xDisposable.is())
            xDisposable->dispose();
    }

    impl_checkMenuCloser();

    impl_disposeContainerWindow( m_xContainerWindow );

    /*ATTENTION
        Clear container after successful removing from parent container ...
        because our parent could be the desktop which stand in dispose too!
        If we have already cleared our own container we lost our child before this could be
        remove himself at this instance ...
        Release m_xFramesHelper after that ... it's the same problem between parent and child!
        "m_xParent->getFrames()->remove( xThis );" needs this helper ...
        Otherwise we get a null reference and could finish removing successfuly.
        => You see: Order of calling operations is important!!!
     */
    implts_forgetSubFrames();

    // Release some other references.
    // This calls should be easy ... I hope it :-)
    m_xDispatchHelper.clear();
    m_xFactory.clear();
    m_xDropTargetListener.clear();
    m_xDispatchRecorderSupplier.clear();
    m_xLayoutManager.clear();
    m_xIndicatorFactoryHelper.clear();

    // It's important to set default values here!
    // If may be later somewhere change the disposed-behaviour of this implementation
    // and doesn't throw any DisposedExceptions we must guarantee best matching default values ...
    m_eActiveState       = E_INACTIVE;
    m_sName              = ::rtl::OUString();
    m_bIsFrameTop        = sal_False;
    m_bConnected         = sal_False;
    m_nExternalLockCount = 0;
    m_bSelfClose         = sal_False;
    m_bIsHidden          = sal_True;

    // Disable this instance for further working realy!
    m_aTransactionManager.setWorkingMode( E_CLOSE );

    // Don't forget it restore old value -
    // otherwhise no dialogs can be shown anymore in other frames.
    Application::SetDialogCancelMode( old );
}

/*-****************************************************************************************************//**
    @short      Be a listener for dispose events!
    @descr      Adds/remove an EventListener to this object. If the dispose method is called on
                this object, the disposing method of the listener is called.

    @seealso    -

    @param      "xListener" reference to your listener object.
    @return     -

    @onerror    Listener is ignored.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_addEventListener( xListener ), "Frame::addEventListener()", "Invalid parameter detected." )
    // Look for rejected calls only!
    // Container is threadsafe.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

//*****************************************************************************************************************
void SAL_CALL Frame::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_removeEventListener( xListener ), "Frame::removeEventListener()", "Invalid parameter detected." )
    // Look for rejected calls only!
    // Container is threadsafe.
    // Use E_SOFTEXCEPTIONS to allow removing listeners during dispose call!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

/*-****************************************************************************************************//**
    @short      create new status indicator
    @descr      Use returned status indicator to show progresses and some text informations.
                All created objects share the same dialog! Only the last one can show his information.

    @seealso    class StatusIndicatorFactory
    @seealso    class StatusIndicator

    @param      -
    @return     A reference to created object.

    @onerror    We return a null reference.
*//*-*****************************************************************************************************/
css::uno::Reference< css::task::XStatusIndicator > SAL_CALL Frame::createStatusIndicator() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    // Look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Make snapshot of neccessary member and define default return value.
    css::uno::Reference< css::task::XStatusIndicator >        xExternal(m_xIndicatorInterception.get(), css::uno::UNO_QUERY);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory = m_xIndicatorFactoryHelper;

    aReadLock.unlock();
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

    // Was set from outside to intercept any progress activities!
    if (xExternal.is())
        return xExternal;

    // Or use our own factory as fallback, to create such progress.
    if (xFactory.is())
        return xFactory->createStatusIndicator();

    return css::uno::Reference< css::task::XStatusIndicator >();
}

/*-****************************************************************************************************//**
    @short      search for target to load URL
    @descr      This method searches for a dispatch for the specified DispatchDescriptor.
                The FrameSearchFlags and the FrameName of the DispatchDescriptor are
                treated as described for findFrame.

    @seealso    method findFrame()
    @seealso    method queryDispatches()
    @seealso    method set/getName()
    @seealso    class TargetFinder

    @param      "aURL"              , URL for loading
    @param      "sTargetFrameName"  , name of target frame
    @param      "nSearchFlags"      , additional flags to regulate search if sTargetFrameName isn't clear
    @return     css::uno::Reference to dispatch handler.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XDispatch > SAL_CALL Frame::queryDispatch( const css::util::URL&   aURL            ,
                                                                            const ::rtl::OUString&  sTargetFrameName,
                                                                                  sal_Int32         nSearchFlags    ) throw( css::uno::RuntimeException )
{
    const char UNO_PROTOCOL[] = ".uno:";

    // Don't check incoming parameter here! Our helper do it for us and it isn't a good idea to do it more then ones!
    // But look for rejected calls!
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

/*-****************************************************************************************************//**
    @short      handle more then ones dispatches at same call
    @descr      Returns a sequence of dispatches. For details see the queryDispatch method.
                For failed dispatches we return empty items in list!

    @seealso    method queryDispatch()

    @param      "lDescriptor" list of dispatch arguments for queryDispatch()!
    @return     List of dispatch references. Some elements can be NULL!

    @onerror    An empty list is returned.
*//*-*****************************************************************************************************/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL Frame::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException )
{
    // Don't check incoming parameter here! Our helper do it for us and it isn't a good idea to do it more then ones!
    // But look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // We use a helper to support these interface and an interceptor mechanism.
    // Our helper is threadsafe by himself!
    return m_xDispatchHelper->queryDispatches( lDescriptor );
}

/*-****************************************************************************************************//**
    @short      register/unregister interceptor for dispatch calls
    @descr      If you whish to handle some dispatches by himself ... you should be
                an interceptor for it. Please see class OInterceptionHelper for further informations.

    @seealso    class OInterceptionHelper

    @param      "xInterceptor", reference to your interceptor implementation.
    @return     -

    @onerror    Interceptor is ignored.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException )
{
    // We use a helper to support these interface and an interceptor mechanism.
    // This helper is threadsafe himself and check incoming parameter too.
    // I think we don't need any lock here!
    // But we must look for rejected calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->registerDispatchProviderInterceptor( xInterceptor );
}

//*****************************************************************************************************************
void SAL_CALL Frame::releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException )
{
    // We use a helper to support these interface and an interceptor mechanism.
    // This helper is threadsafe himself and check incoming parameter too.
    // I think we don't need any lock here!
    // But we must look for rejected calls ...
    // Sometimes we are called during our dispose() method ... => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterceptionHelper( m_xDispatchHelper, css::uno::UNO_QUERY );
    xInterceptionHelper->releaseDispatchProviderInterceptor( xInterceptor );
}

/*-****************************************************************************************************//**
    @short      provides information about all possible dispatch functions
                inside the currnt frame environment
*//*-*****************************************************************************************************/
css::uno::Sequence< sal_Int16 > SAL_CALL Frame::getSupportedCommandGroups()
    throw(css::uno::RuntimeException)
{
    return m_xDispatchInfoHelper->getSupportedCommandGroups();
}

//*****************************************************************************************************************
css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL Frame::getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
    throw(css::uno::RuntimeException)
{
    return m_xDispatchInfoHelper->getConfigurableDispatchInformation(nCommandGroup);
}

/*-****************************************************************************************************//**
    @short      notifications for window events
    @descr      We are a listener on our container window to forward it to our component window.

    @seealso    method setComponent()
    @seealso    member m_xContainerWindow
    @seealso    member m_xComponentWindow

    @param      "aEvent" describe source of detected event
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::windowResized( const css::awt::WindowEvent&
#if OSL_DEBUG_LEVEL > 0
aEvent
#endif
) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_windowResized( aEvent ), "Frame::windowResized()", "Invalid parameter detected." )
    // Look for rejected calls.
    // Part of dispose-mechanism => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Impl-method is threadsafe!
    // If we have a current component window - we must resize it!
    implts_resizeComponentWindow();
}

//*****************************************************************************************************************
void SAL_CALL Frame::focusGained( const css::awt::FocusEvent&
#if OSL_DEBUG_LEVEL > 0
aEvent
#endif
) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_focusGained( aEvent ), "Frame::focusGained()", "Invalid parameter detected." )
    // Look for rejected calls.
    // Part of dispose() mechanism ... => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    // Make snapshot of member!
    css::uno::Reference< css::awt::XWindow > xComponentWindow = m_xComponentWindow;
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xComponentWindow.is() == sal_True )
    {
        xComponentWindow->setFocus();
    }
}

/*-****************************************************************************************************//**
    @short      notifications for window events
    @descr      We are a listener on our container window to forward it to our component window ...
                but a XTopWindowListener we are only if we are a top frame!

    @seealso    method setComponent()
    @seealso    member m_xContainerWindow
    @seealso    member m_xComponentWindow

    @param      "aEvent" describe source of detected event
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::windowActivated( const css::lang::EventObject&
#if OSL_DEBUG_LEVEL > 0
aEvent
#endif
) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_windowActivated( aEvent ), "Frame::windowActivated()", "Invalid parameter detected." )
    // Look for rejected calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    // Make snapshot of member!
    EActiveState eState = m_eActiveState;
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Activate the new active path from here to top.
    if( eState == E_INACTIVE )
    {
        setActiveFrame( css::uno::Reference< css::frame::XFrame >() );
        activate();
    }
}

//*****************************************************************************************************************
void SAL_CALL Frame::windowDeactivated( const css::lang::EventObject&
#if OSL_DEBUG_LEVEL > 0
aEvent
#endif
) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_windowDeactivated( aEvent ), "Frame::windowDeactivated()", "Invalid parameter detected." )
    // Look for rejected calls.
    // Sometimes called during dispose() => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    css::uno::Reference< css::frame::XFrame > xParent          ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow >  xContainerWindow = m_xContainerWindow;
    EActiveState                              eActiveState     = m_eActiveState    ;

    aReadLock.unlock();

    if( eActiveState != E_INACTIVE )
    {
        // Deactivation is always done implicitely by activation of another frame.
        // Only if no activation is done, deactivations have to be processed if the activated window
        // is a parent window of the last active Window!
        SolarMutexClearableGuard aSolarGuard;
        Window* pFocusWindow = Application::GetFocusWindow();
        if  (
                ( xContainerWindow.is()                                                              ==  sal_True    )   &&
                ( xParent.is()                                                                       ==  sal_True    )   &&
                ( (css::uno::Reference< css::frame::XDesktop >( xParent, css::uno::UNO_QUERY )).is() ==  sal_False   )
            )
        {
            css::uno::Reference< css::awt::XWindow >  xParentWindow   = xParent->getContainerWindow()             ;
            Window*                                   pParentWindow   = VCLUnoHelper::GetWindow( xParentWindow    );
            //#i70261#: dialogs opend from an OLE object will cause a deactivate on the frame of the OLE object
            // on Solaris/Linux at that time pFocusWindow is still NULL because the focus handling is different; right after
            // the deactivation the focus will be set into the dialog!
            // currently I see no case where a sub frame could get a deactivate with pFocusWindow being NULL permanently
            // so for now this case is omitted from handled deactivations
            if( pFocusWindow && pParentWindow->IsChild( pFocusWindow ) )
            {
                css::uno::Reference< css::frame::XFramesSupplier > xSupplier( xParent, css::uno::UNO_QUERY );
                if( xSupplier.is() == sal_True )
                {
                    aSolarGuard.clear();
                    xSupplier->setActiveFrame( css::uno::Reference< css::frame::XFrame >() );
                }
            }
        }
    }
}

//*****************************************************************************************************************
void SAL_CALL Frame::windowClosing( const css::lang::EventObject& ) throw( css::uno::RuntimeException )
{
    /* #i62088#
        Some interceptor objects intercept our "internaly asynchronoues implemented" dispatch call.
        And they close this frame directly (means synchronous then).
        Means: Frame::windowClosing()->Frame::close()
        In such situation its not a good idea to hold this transaction count alive .-)
    */
    {
        // Look for rejected calls.
        TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );
        // deactivate this frame ...
        deactivate();
    }

    // ... and try to close it
    // But do it asynchron inside the main thread.
    // VCL has no fun to do such things outside his main thread :-(
    // Note: The used dispatch make it asynchronous for us .-)

    /*ATTENTION!
        Don't try to suspend the controller here! Because it's done inside used dispatch().
        Otherwhise the dialog "would you save your changes?" will be shown more then once ...
     */

    /* SAFE */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory = m_xFactory;
    aReadLock.unlock();
    /* SAFE */

    css::util::URL aURL;
    aURL.Complete = DECLARE_ASCII(".uno:CloseFrame");
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(::comphelper::getComponentContext(xFactory)));
    xParser->parseStrict(aURL);

    css::uno::Reference< css::frame::XDispatch > xCloser = queryDispatch(aURL, SPECIALTARGET_SELF, 0);
    if (xCloser.is())
        xCloser->dispatch(aURL, css::uno::Sequence< css::beans::PropertyValue >());

    // Attention: If this dispatch works synchronous ... and full fill its job ...
    // this line of code will never be reached ...
    // Or if it will be reached it will be for sure that all your member are gone .-)
}

/*-****************************************************************************************************//**
    @short      react for a show event for the internal container window
    @descr      Normaly we doesn't need this information realy. But we can use it to
                implement the special feature "trigger first visible task".

                Algorithm: - first we have to check if we are a top (task) frame
                             It's not enough to be a top frame! Because we MUST have the desktop as parent.
                             But frames without a parent are top too. So it's not possible to check isTop() here!
                             We have to look for the type of our parent.
                           - if we are a task frame, then we have to check if we are the first one.
                             We use a static variable to do so. They will be reset to afterwards be shure
                             that further calls of this method doesn't do anything then.
                           - Then we have to trigger the right event string on the global job executor.

    @seealso    css::task::JobExecutor

    @param      aEvent
                    describes the source of this event
                    We are not interested on this information. We are interested on the visible state only.

    @threadsafe yes
*//*-*****************************************************************************************************/
void SAL_CALL Frame::windowShown( const css::lang::EventObject& ) throw(css::uno::RuntimeException)
{
    static sal_Bool bFirstVisibleTask = sal_True;

    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XDesktop >             xDesktopCheck( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::lang::XMultiServiceFactory >  xFactory     = m_xFactory;
    m_bIsHidden = sal_False;
    aReadLock.unlock();
    /* } SAFE */

    impl_checkMenuCloser();

    if (xDesktopCheck.is())
    {
        /* STATIC SAFE { */
        WriteGuard aStaticWriteLock( LockHelper::getGlobalLock() );
        sal_Bool bMustBeTriggered  = bFirstVisibleTask;
                 bFirstVisibleTask = sal_False;
        aStaticWriteLock.unlock();
        /* } STATIC SAFE */

        if (bMustBeTriggered)
        {
            css::uno::Reference< css::task::XJobExecutor > xExecutor
                = css::task::JobExecutor::create( comphelper::getComponentContext(xFactory) );
            xExecutor->trigger( DECLARE_ASCII("onFirstVisibleTask") );
        }
    }
}

void SAL_CALL Frame::windowHidden( const css::lang::EventObject& ) throw(css::uno::RuntimeException)
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    m_bIsHidden = sal_True;
    aReadLock.unlock();
    /* } SAFE */

    impl_checkMenuCloser();
}

/*-****************************************************************************************************//**
    @short      called by dispose of our windows!
    @descr      This object is forced to release all references to the interfaces given
                by the parameter source. We are a listener at our container window and
                should listen for his diposing.

    @seealso    XWindowListener
    @seealso    XTopWindowListener
    @seealso    XFocusListener

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_disposing( aEvent ), "Frame::disposing()", "Invalid parameter detected." )
    // Look for rejected calls.
    // May be we are called during releasing our windows in our in dispose call!? => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if( aEvent.Source == m_xContainerWindow )
    {
        // NECCESSARY: Impl-method is threadsafe by himself!
        aWriteLock.unlock();
        implts_stopWindowListening();
        aWriteLock.lock();
        m_xContainerWindow = css::uno::Reference< css::awt::XWindow >();
    }
}

/*-************************************************************************************************************//**
    @interface  com.sun.star.document.XActionLockable
    @short      implement locking of frame/task from outside
    @descr      Sometimes we have problems to decide if closing of task is allowed. Because; frame/task
                could be used for pending loading jobs. So you can lock this object from outside and
                prevent instance against closing during using! But - don't do it in a wrong or expensive manner.
                Otherwise task couldn't die anymore!!!

    @seealso    interface XActionLockable
    @seeelso    method BaseDispatcher::implts_loadIt()
    @seeelso    method Desktop::loadComponentFromURL()

    @param      -
    @return     true if frame/task is locked
                false otherwise

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool SAL_CALL Frame::isActionLocked() throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    return( m_nExternalLockCount!=0);
}

//*****************************************************************************************************************
void SAL_CALL Frame::addActionLock() throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    ++m_nExternalLockCount;
}

//*****************************************************************************************************************
void SAL_CALL Frame::removeActionLock() throw( css::uno::RuntimeException )
{
    // Register no transaction here! Otherwhise we wait for ever inside possible
    // implts_checkSuicide()/dispose() request ...

    /* SAFE AREA */{
        WriteGuard aWriteLock( m_aLock );
        LOG_ASSERT2( m_nExternalLockCount<=0, "Frame::removeActionLock()", "Frame isn't locked! Possible multithreading problem detected." )
        --m_nExternalLockCount;
    }/* SAFE */

    implts_checkSuicide();
}

//*****************************************************************************************************************
void SAL_CALL Frame::setActionLocks( sal_Int16 nLock ) throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    // Attention: If somewhere called resetActionLocks() before and get e.g. 5 locks ...
    //            and tried to set these 5 ones here after his operations ...
    //            we can't ignore setted requests during these two calls!
    //            So we must add(!) these 5 locks here.
    m_nExternalLockCount = m_nExternalLockCount + nLock;
}

//*****************************************************************************************************************
sal_Int16 SAL_CALL Frame::resetActionLocks() throw( css::uno::RuntimeException )
{
    // Register no transaction here! Otherwhise we wait for ever inside possible
    // implts_checkSuicide()/dispose() request ...

    sal_Int16 nCurrentLocks = 0;
    /* SAFE */{
        WriteGuard aWriteLock( m_aLock );
        nCurrentLocks = m_nExternalLockCount;
        m_nExternalLockCount = 0;
    }/* SAFE */

    // Attention:
    // external lock count is 0 here every time ... but if
    // member m_bSelfClose is set to true too .... we call our own close()/dispose().
    // See close() for further informations
    implts_checkSuicide();

    return nCurrentLocks;
}

//*****************************************************************************************************************
void Frame::impl_initializePropInfo()
{
    impl_setPropertyChangeBroadcaster(static_cast< css::frame::XFrame* >(this));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_DISPATCHRECORDERSUPPLIER,
            FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER,
            ::getCppuType((const css::uno::Reference< css::frame::XDispatchRecorderSupplier >*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_INDICATORINTERCEPTION,
            FRAME_PROPHANDLE_INDICATORINTERCEPTION,
            ::getCppuType((const css::uno::Reference< css::task::XStatusIndicator >*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_ISHIDDEN,
            FRAME_PROPHANDLE_ISHIDDEN,
            ::getBooleanCppuType(),
            css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_LAYOUTMANAGER,
            FRAME_PROPHANDLE_LAYOUTMANAGER,
            ::getCppuType((const css::uno::Reference< ::com::sun::star::frame::XLayoutManager >*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));

    impl_addPropertyInfo(
        css::beans::Property(
            FRAME_PROPNAME_TITLE,
            FRAME_PROPHANDLE_TITLE,
            ::getCppuType((const ::rtl::OUString*)NULL),
            css::beans::PropertyAttribute::TRANSIENT));
}

//*****************************************************************************************************************
void SAL_CALL Frame::impl_setPropertyValue(const ::rtl::OUString& /*sProperty*/,
                                                 sal_Int32        nHandle  ,
                                           const css::uno::Any&   aValue   )

{
    /* There is no need to lock any mutex here. Because we share the
       solar mutex with our base class. And we said to our base class: "dont release it on calling us" .-)
       see ctor of PropertySetHelper for further informations.
    */

    /* Attention: You can use nHandle only, if you are sure that all supported
                  properties has an unique handle. That must be guaranteed
                  inside method impl_initializePropInfo()!
    */
    switch (nHandle)
    {
        case FRAME_PROPHANDLE_TITLE :
                {
                    ::rtl::OUString sExternalTitle;
                    aValue >>= sExternalTitle;
                    setTitle (sExternalTitle);
                }
                break;

        case FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER :
                aValue >>= m_xDispatchRecorderSupplier;
                break;

        case FRAME_PROPHANDLE_LAYOUTMANAGER :
                {
                    css::uno::Reference< css::frame::XLayoutManager > xOldLayoutManager = m_xLayoutManager;
                    css::uno::Reference< css::frame::XLayoutManager > xNewLayoutManager;
                    aValue >>= xNewLayoutManager;

                    if (xOldLayoutManager != xNewLayoutManager)
                    {
                        m_xLayoutManager = xNewLayoutManager;
                        if (xOldLayoutManager.is())
                            lcl_disableLayoutManager(xOldLayoutManager, this);
                        if (xNewLayoutManager.is())
                            lcl_enableLayoutManager(xNewLayoutManager, this);
                    }
                }
                break;

        case FRAME_PROPHANDLE_INDICATORINTERCEPTION :
                {
                    css::uno::Reference< css::task::XStatusIndicator > xProgress;
                    aValue >>= xProgress;
                    m_xIndicatorInterception = xProgress;
                }
                break;

        #ifdef ENABLE_WARNINGS
        default :
                LOG_WARNING( "Frame::setFastPropertyValue_NoBroadcast()", "Invalid handle detected!" )
                break;
        #endif
    }
}

//*****************************************************************************************************************
css::uno::Any SAL_CALL Frame::impl_getPropertyValue(const ::rtl::OUString& /*sProperty*/,
                                                          sal_Int32        nHandle  )
{
    /* There is no need to lock any mutex here. Because we share the
       solar mutex with our base class. And we said to our base class: "dont release it on calling us" .-)
       see ctor of PropertySetHelper for further informations.
    */

    /* Attention: You can use nHandle only, if you are sure that all supported
                  properties has an unique handle. That must be guaranteed
                  inside method impl_initializePropInfo()!
    */
    css::uno::Any aValue;
    switch (nHandle)
    {
        case FRAME_PROPHANDLE_TITLE :
                aValue <<= getTitle ();
                break;

        case FRAME_PROPHANDLE_DISPATCHRECORDERSUPPLIER :
                aValue <<= m_xDispatchRecorderSupplier;
                break;

        case FRAME_PROPHANDLE_ISHIDDEN :
                aValue <<= m_bIsHidden;
                break;

        case FRAME_PROPHANDLE_LAYOUTMANAGER :
                aValue <<= m_xLayoutManager;
                break;

        case FRAME_PROPHANDLE_INDICATORINTERCEPTION :
                {
                    css::uno::Reference< css::task::XStatusIndicator > xProgress(m_xIndicatorInterception.get(), css::uno::UNO_QUERY);
                    aValue = css::uno::makeAny(xProgress);
                }
                break;

        #ifdef ENABLE_WARNINGS
        default :
                LOG_WARNING( "Frame::getFastPropertyValue()", "Invalid handle detected!" )
                break;
        #endif
    }

    return aValue;
}

/*-****************************************************************************************************//**
    @short      dispose old container window and forget his reference
    @descr      Sometimes we must repair our "modal dialog parent mechanism" too!

    @seealso    -

    @param      "xWindow", reference to old container window to dispose it
    @return     An empty reference.

    @onerror    -
    @threadsafe NO!
*//*-*****************************************************************************************************/
void Frame::impl_disposeContainerWindow( css::uno::Reference< css::awt::XWindow >& xWindow )
{
    if( xWindow.is() == sal_True )
    {
        xWindow->setVisible( sal_False );
        // All VclComponents are XComponents; so call dispose before discarding
        // a css::uno::Reference< XVclComponent >, because this frame is the owner of the window
        xWindow->dispose();
        xWindow = css::uno::Reference< css::awt::XWindow >();
    }
}

/*-****************************************************************************************************//**
    @short      send frame action event to our listener
    @descr      This method is threadsafe AND can be called by our dispose method too!

    @seealso    -

    @param      "aAction", describe the event for sending
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void Frame::implts_sendFrameActionEvent( const css::frame::FrameAction& aAction )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Sometimes used by dispose() => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    // Log informations about order of events to file!
    // (only activated in debug version!)
    LOG_FRAMEACTIONEVENT( "Frame", m_sName, aAction )

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Send css::frame::FrameAction event to all listener.
    // Get container for right listener.
    // FOLLOW LINES ARE THREADSAFE!!!
    // ( OInterfaceContainerHelper is synchronized with m_aListenerContainer! )
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::frame::XFrameActionListener >*) NULL ) );

    if( pContainer != NULL )
    {
        // Build action event.
        css::frame::FrameActionEvent aFrameActionEvent( static_cast< ::cppu::OWeakObject* >(this), this, aAction );

        // Get iterator for access to listener.
        ::cppu::OInterfaceIteratorHelper aIterator( *pContainer );
        // Send message to all listener.
        while( aIterator.hasMoreElements() == sal_True )
        {
            try
            {
                ((css::frame::XFrameActionListener*)aIterator.next())->frameAction( aFrameActionEvent );
            }
            catch( const css::uno::RuntimeException& )
            {
                aIterator.remove();
            }
        }
    }
}

/*-****************************************************************************************************//**
    @short      helper to resize our component window
    @descr      A frame contains 2 windows - a container ~ and a component window.
                This method resize inner component window to full size of outer container window.
                This method is threadsafe AND can be called by our dispose method too!

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void Frame::implts_resizeComponentWindow()
{
    // usually the LayoutManager does the resizing
    // in case there is no LayoutManager resizing has to be done here
    if ( !m_xLayoutManager.is() )
    {
        css::uno::Reference< css::awt::XWindow > xComponentWindow( getComponentWindow() );
        if( xComponentWindow.is() == sal_True )
        {
            css::uno::Reference< css::awt::XDevice > xDevice( getContainerWindow(), css::uno::UNO_QUERY );

            // Convert relativ size to output size.
            css::awt::Rectangle  aRectangle  = getContainerWindow()->getPosSize();
            css::awt::DeviceInfo aInfo       = xDevice->getInfo();
            css::awt::Size       aSize       (  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                                aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

            // Resize our component window.
            xComponentWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, css::awt::PosSize::POSSIZE );
        }
    }
}

/*-****************************************************************************************************//**
    @short      helper to set icon on our container window (if it is a system window!)
    @descr      We use our internal set controller (if it exist) to specify which factory he represanted.
                These information can be used to find right icon. But our controller can say it us directly
                too ... we should ask his optional property set first ...

    @seealso    method Window::SetIcon()

    @param      -
    @return     -

    @onerror    We do nothing.
*//*-*****************************************************************************************************/
void Frame::implts_setIconOnWindow()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Look for rejected calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Make snapshot of neccessary members and release lock.
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XWindow >       xContainerWindow( m_xContainerWindow, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XController > xController     ( m_xController     , css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if(
        ( xContainerWindow.is() == sal_True )   &&
        ( xController.is()      == sal_True )
      )
    {
        //-------------------------------------------------------------------------------------------------------------
        // a) set default value to an invalid one. So we can start further searches for right icon id, if
        //    first steps failed!
        //    We must reset it to any fallback value - if no search step returns a valid result.
        sal_Int32 nIcon = -1;

        //-------------------------------------------------------------------------------------------------------------
        // b) try to find information on controller propertyset directly
        //    Don't forget to catch possible exceptions - because these property is an optional one!
        css::uno::Reference< css::beans::XPropertySet > xSet( xController, css::uno::UNO_QUERY );
        if( xSet.is() == sal_True )
        {
            try
            {
                css::uno::Reference< css::beans::XPropertySetInfo > const xPSI( xSet->getPropertySetInfo(), css::uno::UNO_SET_THROW );
                if ( xPSI->hasPropertyByName( "IconId" ) )
                    xSet->getPropertyValue( "IconId" ) >>= nIcon;
            }
            catch( css::uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        //-------------------------------------------------------------------------------------------------------------
        // c) if b) failed ... analyze argument list of currently loaded document insde the frame to find the filter.
        //    He can be used to detect right factory - and these can be used to match factory to icon ...
        if( nIcon == -1 )
        {
            css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
            if( xModel.is() == sal_True )
            {
                SvtModuleOptions::EFactory eFactory = SvtModuleOptions::ClassifyFactoryByModel(xModel);
                if (eFactory != SvtModuleOptions::E_UNKNOWN_FACTORY)
                    nIcon = SvtModuleOptions().GetFactoryIcon( eFactory );
            }
        }

        //-------------------------------------------------------------------------------------------------------------
        // d) if all steps failed - use fallback!
        if( nIcon == -1 )
        {
            nIcon = 0;
        }

        //-------------------------------------------------------------------------------------------------------------
        // e) set icon on container window now
        //    Don't forget SolarMutex! We use vcl directly :-(
        //    Check window pointer for right WorkWindow class too!!!
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        {
            SolarMutexGuard aSolarGuard;
            Window* pWindow = (VCLUnoHelper::GetWindow( xContainerWindow ));
            if(
                ( pWindow            != NULL              ) &&
                ( pWindow->GetType() == WINDOW_WORKWINDOW )
                )
            {
                WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
                pWorkWindow->SetIcon( (sal_uInt16)nIcon );
            }
        }
        /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    }
}

/*-************************************************************************************************************//**
    @short      helper to start/stop listeneing for window events on container window
    @descr      If we get a new container window, we must set it on internal memeber ...
                and stop listening at old one ... and start listening on new one!
                But sometimes (in dispose() call!) it's neccessary to stop listeneing without starting
                on new connections. So we split this functionality to make it easier at use.

    @seealso    method initialize()
    @seealso    method dispose()

    @param      -
    @return     -

    @onerror    We do nothing!
    @threadsafe yes
*//*-*************************************************************************************************************/
void Frame::implts_startWindowListening()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Make snapshot of neccessary member!
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XWindow >                            xContainerWindow    = m_xContainerWindow   ;
    css::uno::Reference< css::lang::XMultiServiceFactory >              xFactory            = m_xFactory           ;
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >  xDragDropListener   = m_xDropTargetListener;
    css::uno::Reference< css::awt::XWindowListener >                    xWindowListener     ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XFocusListener >                     xFocusListener      ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XTopWindowListener >                 xTopWindowListener  ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xContainerWindow.is() == sal_True )
    {
        xContainerWindow->addWindowListener( xWindowListener);
        xContainerWindow->addFocusListener ( xFocusListener );

        css::uno::Reference< css::awt::XTopWindow > xTopWindow( xContainerWindow, css::uno::UNO_QUERY );
        if( xTopWindow.is() == sal_True )
        {
            xTopWindow->addTopWindowListener( xTopWindowListener );

            css::uno::Reference< css::awt::XDataTransferProviderAccess > xTransfer( xFactory->createInstance( SERVICENAME_VCLTOOLKIT ), css::uno::UNO_QUERY );
            if( xTransfer.is() == sal_True )
            {
                css::uno::Reference< css::datatransfer::dnd::XDropTarget > xDropTarget = xTransfer->getDropTarget( xContainerWindow );
                if( xDropTarget.is() == sal_True )
                {
                    xDropTarget->addDropTargetListener( xDragDropListener );
                    xDropTarget->setActive( sal_True );
                }
            }
        }
    }
}

//*****************************************************************************************************************
void Frame::implts_stopWindowListening()
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Sometimes used by dispose() => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Make snapshot of neccessary member!
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XWindow >                            xContainerWindow    = m_xContainerWindow   ;
    css::uno::Reference< css::lang::XMultiServiceFactory >              xFactory            = m_xFactory           ;
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >  xDragDropListener   = m_xDropTargetListener;
    css::uno::Reference< css::awt::XWindowListener >                    xWindowListener     ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XFocusListener >                     xFocusListener      ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XTopWindowListener >                 xTopWindowListener  ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    if( xContainerWindow.is() == sal_True )
    {
        xContainerWindow->removeWindowListener( xWindowListener);
        xContainerWindow->removeFocusListener ( xFocusListener );

        css::uno::Reference< css::awt::XTopWindow > xTopWindow( xContainerWindow, css::uno::UNO_QUERY );
        if( xTopWindow.is() == sal_True )
        {
            xTopWindow->removeTopWindowListener( xTopWindowListener );

            css::uno::Reference< css::awt::XDataTransferProviderAccess > xTransfer( xFactory->createInstance( SERVICENAME_VCLTOOLKIT ), css::uno::UNO_QUERY );
            if( xTransfer.is() == sal_True )
            {
                css::uno::Reference< css::datatransfer::dnd::XDropTarget > xDropTarget = xTransfer->getDropTarget( xContainerWindow );
                if( xDropTarget.is() == sal_True )
                {
                    xDropTarget->removeDropTargetListener( xDragDropListener );
                    xDropTarget->setActive( sal_False );
                }
            }
        }
    }
}

/*-****************************************************************************************************//**
    @short      helper to force breaked close() request again
    @descr      If we self disagree with a close() request, and detect that all external locks are gone ...
                then we must try to close this frame again.

    @seealso    XCloseable::close()
    @seealso    Frame::close()
    @seealso    Frame::removeActionLock()
    @seealso    Frame::resetActionLock()
    @seealso    m_bSelfClose
    @seealso    m_nExternalLockCount

    @threadsafe yes
*//*-*****************************************************************************************************/
void Frame::implts_checkSuicide()
{
    /* SAFE */
    ReadGuard aReadLock(m_aLock);
    // in case of lock==0 and safed state of previous close() request m_bSelfClose
    // we must force close() again. Because we had disagreed with that before.
    sal_Bool bSuicide = (m_nExternalLockCount==0 && m_bSelfClose);
    m_bSelfClose = sal_False;
    aReadLock.unlock();
    /* } SAFE */
    // force close and deliver owner ship to source of possible throwed veto exception
    // Attention: Because this method isn't designed to throw such exception we must supress
    // it for outside code!
    try
    {
        if (bSuicide)
            close(sal_True);
    }
    catch(const css::util::CloseVetoException&)
        {}
    catch(const css::lang::DisposedException&)
        {}
}

//_______________________________________________________________

/** little helper to enable/disable the menu closer at the menubar of the given frame.

    @param  xFrame
            we use its layout manager to set/reset a special callback.
            Its existence regulate visibility of this closer item.

    @param  bState
                <TRUE/> enable; <FALSE/> disable this state
 */

void Frame::impl_setCloser( /*IN*/ const css::uno::Reference< css::frame::XFrame >& xFrame ,
                            /*IN*/       sal_Bool                                   bState  )
{
    // Note: If start module isnt installed - no closer has to be shown!
    if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SSTARTMODULE))
        return;

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xFrameProps(xFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        xFrameProps->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
        css::uno::Reference< css::beans::XPropertySet > xLayoutProps(xLayoutManager, css::uno::UNO_QUERY_THROW);
        xLayoutProps->setPropertyValue(LAYOUTMANAGER_PROPNAME_MENUBARCLOSER, css::uno::makeAny(bState));
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}

//_______________________________________________________________

/** it checks, which of the top level task frames must have the special menu closer for
    switching to the backing window mode.

    It analyze the current list of visible top level frames. Only the last real document
    frame can have this symbol. Not the help frame nor the backing task itself.
    Here we do anything related to this closer. We remove it from the old frame and set it
    for the new one.
 */

void Frame::impl_checkMenuCloser()
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);

    // only top frames, which are part of our desktop hierarchy, can
    // do so! By the way - we need the desktop instance to have acess
    // to all other top level frames too.
    css::uno::Reference< css::frame::XDesktop >        xDesktop     (m_xParent, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XFramesSupplier > xTaskSupplier(xDesktop , css::uno::UNO_QUERY);
    if ( !xDesktop.is() || !xTaskSupplier.is() )
        return;

    aReadLock.unlock();
    /* } SAFE */

    // analyze the list of current open tasks
    // Suppress search for other views to the same model ...
    // It's not needed here and can be very expensive.
    FrameListAnalyzer aAnalyzer(
        xTaskSupplier,
        this,
        FrameListAnalyzer::E_HIDDEN | FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_BACKINGCOMPONENT);

    // specify the new frame, which must have this special state ...
    css::uno::Reference< css::frame::XFrame > xNewCloserFrame;

    // -----------------------------
    // a)
    // If there exist ate least one other frame - there are two frames currently open.
    // But we can enable this closer only, if one of these two tasks includes the help module.
    // The "other frame" couldn't be the help. Because then it wouldn't be part of this "other list".
    // In such case it will be seperated to the reference aAnalyzer.m_xHelp!
    // But we must check, if weself includes the help ...
    // Check aAnalyzer.m_bReferenceIsHelp!
    if (
        (aAnalyzer.m_lOtherVisibleFrames.getLength()==1)   &&
        (
            (aAnalyzer.m_bReferenceIsHelp  ) ||
            (aAnalyzer.m_bReferenceIsHidden)
        )
       )
    {
        // others[0] can't be the backing component!
        // Because it's set at the special member aAnalyzer.m_xBackingComponent ... :-)
        xNewCloserFrame = aAnalyzer.m_lOtherVisibleFrames[0];
    }
    else
    // -----------------------------
    // b)
    // There is no other frame ... means no other document frame. The help module
    // will be handled seperatly and must(!) be ignored here ... excepting weself includes the help.
    if (
        (aAnalyzer.m_lOtherVisibleFrames.getLength()==0) &&
        (!aAnalyzer.m_bReferenceIsHelp                 ) &&
        (!aAnalyzer.m_bReferenceIsHidden               ) &&
        (!aAnalyzer.m_bReferenceIsBacking              )
       )
    {
        xNewCloserFrame = this;
    }

    // Look for neccessary actions ...
    // Only if the closer state must be moved from one frame to another one
    // or must be enabled/disabled at all.
    /* STATIC SAFE { */
    WriteGuard aStaticWriteLock(LockHelper::getGlobalLock());
    css::uno::Reference< css::frame::XFrame > xCloserFrame (m_xCloserFrame.get(), css::uno::UNO_QUERY);
    if (xCloserFrame!=xNewCloserFrame)
    {
        if (xCloserFrame.is())
            impl_setCloser(xCloserFrame, sal_False);
        if (xNewCloserFrame.is())
            impl_setCloser(xNewCloserFrame, sal_True);
        m_xCloserFrame = xNewCloserFrame;
    }
    aStaticWriteLock.unlock();
    /* } STATIC SAFE */
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
// We don't accept null pointer or references!
sal_Bool Frame::implcp_ctor( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
{
    return  (
                ( &xFactory     ==  NULL        )   ||
                ( xFactory.is() ==  sal_False   )
            );
}

//*****************************************************************************************************************
// Its allowed to reset the active frame membervariable with a NULL-css::uno::Reference but not with a NULL-pointer!
// And we accept frames only! No tasks and desktops!
sal_Bool Frame::implcp_setActiveFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    return  (
                ( &xFrame                                                                                   ==  NULL        )   ||
                ( css::uno::Reference< css::frame::XDesktop >( xFrame, css::uno::UNO_QUERY ).is()           ==  sal_True    )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_addFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener )
{
    return  (
                ( &xListener        ==  NULL        )   ||
                ( xListener.is()    ==  sal_False   )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_removeFrameActionListener( const css::uno::Reference< css::frame::XFrameActionListener >& xListener )
{
    return  (
                ( &xListener        ==  NULL        )   ||
                ( xListener.is()    ==  sal_False   )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return  (
                ( &xListener        ==  NULL        )   ||
                ( xListener.is()    ==  sal_False   )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    return  (
                ( &xListener        ==  NULL        )   ||
                ( xListener.is()    ==  sal_False   )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_windowResized( const css::awt::WindowEvent& aEvent )
{
    return  (
                ( &aEvent               ==  NULL        )   ||
                ( aEvent.Source.is()    ==  sal_False   )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_focusGained( const css::awt::FocusEvent& aEvent )
{
    return  (
                ( &aEvent               ==  NULL        )   ||
                ( aEvent.Source.is()    ==  sal_False   )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_windowActivated( const css::lang::EventObject& aEvent )
{
    return  (
                ( &aEvent               ==  NULL        )   ||
                ( aEvent.Source.is()    ==  sal_False   )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_windowDeactivated( const css::lang::EventObject& aEvent )
{
    return  (
                ( &aEvent               ==  NULL        )   ||
                ( aEvent.Source.is()    ==  sal_False   )
            );
}

//*****************************************************************************************************************
sal_Bool Frame::implcp_disposing( const css::lang::EventObject& aEvent )
{
    return  (
                ( &aEvent               ==  NULL        )   ||
                ( aEvent.Source.is()    ==  sal_False   )
            );
}

#endif  // #ifdef ENABLE_ASSERTIONS

}   // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
