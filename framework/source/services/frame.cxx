/*************************************************************************
 *
 *  $RCSfile: frame.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: as $ $Date: 2001-08-16 12:16:12 $
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

#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#include <services/frame.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_DISPATCHPROVIDER_HXX_
#include <dispatch/dispatchprovider.hxx>
#endif

#ifndef __FRAMEWORK_DISPATCH_INTERCEPTIONHELPER_HXX_
#include <dispatch/interceptionhelper.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_OFRAMES_HXX_
#include <helper/oframes.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_STATUSINDICATORFACTORY_HXX_
#include <helper/statusindicatorfactory.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_TARGETFINDER_HXX_
#include <classes/targetfinder.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_TRANSACTIONGUARD_HXX_
#include <threadhelp/transactionguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_CLASSES_DROPTARGETLISTENER_HXX_
#include <classes/droptargetlistener.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTASK_HPP_
#include <com/sun/star/frame/XTask.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDATATRANSFERPROVIDERACCESS_HPP_
#include <com/sun/star/awt/XDataTransferProviderAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
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

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifdef ENABLE_ASSERTIONS
    #ifndef _RTL_STRBUF_HXX_
    #include <rtl/strbuf.hxx>
    #endif
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

#define PROPERTYNAME_TITLE          DECLARE_ASCII("Title")

#define PROPERTYHANDLE_TITLE        1

#define PROPERTYCOUNT               1

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_17                (   Frame                                                                   ,
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
                                        DIRECT_INTERFACE(css::beans::XMultiPropertySet                          ),
                                        DIRECT_INTERFACE(css::beans::XFastPropertySet                           ),
                                        DIRECT_INTERFACE(css::beans::XPropertySet                               ),
                                        DIRECT_INTERFACE(css::awt::XWindowListener                              ),
                                        DIRECT_INTERFACE(css::awt::XTopWindowListener                           ),
                                        DIRECT_INTERFACE(css::awt::XFocusListener                               ),
                                        DERIVED_INTERFACE(css::lang::XEventListener, css::awt::XWindowListener  ),
                                        DIRECT_INTERFACE(css::document::XActionLockable                         )
                                    )

DEFINE_XTYPEPROVIDER_17             (   Frame                                                                   ,
                                        css::lang::XTypeProvider                                                ,
                                        css::lang::XServiceInfo                                                 ,
                                        css::frame::XFramesSupplier                                             ,
                                        css::frame::XFrame                                                      ,
                                        css::lang::XComponent                                                   ,
                                        css::task::XStatusIndicatorFactory                                      ,
                                        css::beans::XMultiPropertySet                                           ,
                                        css::beans::XFastPropertySet                                            ,
                                        css::beans::XPropertySet                                                ,
                                        css::frame::XDispatchProvider                                           ,
                                        css::frame::XDispatchInformationProvider                                ,
                                        css::frame::XDispatchProviderInterception                               ,
                                        css::awt::XWindowListener                                               ,
                                        css::awt::XTopWindowListener                                            ,
                                        css::awt::XFocusListener                                                ,
                                        css::lang::XEventListener                                               ,
                                        css::document::XActionLockable
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
                                            DropTargetListener* pDropListener = new DropTargetListener( this );
                                            m_xDropTargetListener = css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >( static_cast< ::cppu::OWeakObject* >(pDropListener), css::uno::UNO_QUERY );

                                            // Safe impossible cases
                                            // We can't work without these helpers!
                                            LOG_ASSERT2( xDispatchProvider.is    ()==sal_False, "Frame::impl_initService()", "Slowest slave for dispatch- and interception helper isn't valid. XDispatchProvider, XDispatch, XDispatchProviderInterception are not full supported!" )
                                            LOG_ASSERT2( m_xDispatchHelper.is    ()==sal_False, "Frame::impl_initService()", "Interception helper isn't valid. XDispatchProvider, XDispatch, XDispatchProviderInterception are not full supported!"                                 )
                                            LOG_ASSERT2( m_xFramesHelper.is      ()==sal_False, "Frame::impl_initService()", "Frames helper isn't valid. XFrames, XIndexAccess and XElementAcces are not supported!"                                                                )
                                            LOG_ASSERT2( m_xDropTargetListener.is()==sal_False, "Frame::impl_initService()", "DropTarget helper isn't valid. Drag and drop without functionality!"                                                                                  )
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
        //  init baseclasses first!
        //  Attention: Don't change order of initialization!
        :   ThreadHelpBase              ( &Application::GetSolarMutex()                     )
        ,   TransactionBase             (                                                   )
        ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >           ( m_aLock.getShareableOslMutex()         )
        ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
        ,   ::cppu::OWeakObject         (                                                   )
        //  init member
        ,   m_xFactory                  ( xFactory                                          )
        ,   m_aListenerContainer        ( m_aLock.getShareableOslMutex()                    )
        ,   m_aChildFrameContainer      (                                                   )
        ,   m_xParent                   (                                                   )
        ,   m_xContainerWindow          (                                                   )
        ,   m_xComponentWindow          (                                                   )
        ,   m_xController               (                                                   )
        ,   m_eActiveState              ( E_INACTIVE                                        )
        ,   m_sName                     (                                                   )
        ,   m_bIsFrameTop               ( sal_True                                          ) // I think we are top without a parent ... and there is no parent yet!
        ,   m_bConnected                ( sal_False                                         ) // There exist no component inside of use => sal_False, we are not connected!
        ,   m_nExternalLockCount        ( 0                                                 )
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

/*-****************************************************************************************************//**
    @short      return access to append or remove childs on desktop
    @descr      We don't implement these interface directly. We use a helper class to do this.
                If you wish to add or delete childs to/from the container, call these method to get
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
    @descr      It must be a frameto. Direct childs of a frame are frames only! No task or desktop is accepted.
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
    @descr      It must be a frame to. Direct childs of frame are frames only! No task or desktop is accepted.
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
    EActiveState                              eActiveState = m_eActiveState                    ;

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
    // Check incoming parameter.
    LOG_ASSERT2( implcp_initialize( xWindow ), "Frame::initialize()", "Invalid parameter detected!" )

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    // Look for rejected calls first!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    // Enable object for real working ... so follow impl methods don't must handle it special! (e.g. E_SOFTEXCEPTIONS for rejected calls)
    m_aTransactionManager.setWorkingMode( E_WORK );

    // This must be the first call of this method!
    // We should initialize our object and open it for working.
    // Set the new window.
    LOG_ASSERT2( m_xContainerWindow.is()==sal_True, "Frame::initialize()", "Leak detected! This state should never occure ..." )
    m_xContainerWindow = xWindow;

    // Now we can use our indicator factory helper to support XStatusIndicatorFactory interface.
    // We have a valid parent window for it!
    // Initialize helper.
    if( m_xContainerWindow.is() == sal_True )
    {
        StatusIndicatorFactory* pIndicatorFactoryHelper = new StatusIndicatorFactory( m_xFactory, this, m_xContainerWindow );
        m_xIndicatorFactoryHelper = css::uno::Reference< css::task::XStatusIndicatorFactory >( static_cast< ::cppu::OWeakObject* >( pIndicatorFactoryHelper ), css::uno::UNO_QUERY );
    }

    // Release lock ... because we call some impl methods, which are threadsafe by himself.
    // If we hold this lock - we will produce our own deadlock!
    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // Start listening for events after setting it on helper class ...
    // So superflous messages are filtered to NULL :-)
    implts_startWindowListening();
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

    @seealso    method getCreator()
    @seealso    method findFrame()
    @seealso    method queryDispatch()

    @param      "xCreator", valid reference to our owner frame, which should implement a supplier interface.
    @return     -

    @onerror    We do nothing.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_setCreator( xCreator ), "Frame::setCreator()", "Invalid parameter detected!" )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    // Safe new reference to different parent.
    m_xParent = xCreator;
    // Set/reset "IsTop" flag, if ouer new parent is a frame, task or desktop ....
    // or if no parent exist!
    css::uno::Reference< css::frame::XTask >      xIsTask     ( m_xParent, css::uno::UNO_QUERY );
    css::uno::Reference< css::frame::XDesktop >   xIsDesktop  ( m_xParent, css::uno::UNO_QUERY );
    if  (
            ( xIsTask.is()      ==  sal_True    )   ||
            ( xIsDesktop.is()   ==  sal_True    )   ||
            ( m_xParent.is()    ==  sal_False   )
        )
    {
        m_bIsFrameTop = sal_True;
    }
    else
    {
        m_bIsFrameTop = sal_False;
    }
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
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    return m_sName;
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
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_setName( sName ), "Frame::setName()", "Invalid parameter detected!" )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    // Set new name ... but look for invalid special target names!
    // They are not allowed to set.
    m_sName = sName;
    impl_filterSpecialTargets( m_sName );
}

/*-****************************************************************************************************//**
    @short      search for frames
    @descr      This method searches for a frame with the specified name.
                Frames may contain other frames (e.g. a frameset) and may
                be contained in other frames. This hierarchie ist searched by
                this method.
                First some special names are taken into account, i.e. "",
                 "_self", "_top", "_active" etc. The nSearchFlags are ignored
                when comparing these names with sTargetFrameName, further steps are
                controlled by the search flags. If allowed, the name of the frame
                itself is compared with the desired one, then ( again if allowed )
                the method findFrame() is called for all children of the frame.
                At last findFrame may be called for the parent frame ( if allowed ).
                If no frame with the given name is found until the top frames container,
                a new top one is created, if this is allowed by a special
                flag. The new frame also gets the desired name.

    @seealso    class TargetFinder

    @param      "sTargetFrameName", special names (_blank, _self) or real name of target frame
    @return     css::uno::Reference to found or may be new created frame.

    @onerror    A null reference is returned.
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame > SAL_CALL Frame::findFrame( const ::rtl::OUString&  sTargetFrameName,
                                                                           sal_Int32         nSearchFlags    ) throw( css::uno::RuntimeException )
{
    /*ATTENTION
        This method has a problem!
        Sometimes we must search recursive if user combine flags PARENT and CHILDREN.
        We search at our children first and forward findFrame() to our parent then.
        It could be that he call us back. But we have already searched at our children!
        I think it's a problem of performance ... errors couldn't occure.
        Please don't use a bool "bProtectRecursivSearches" or something like that
        in this method. Otherwise some calls failed or blocked if findFrame() is called
        from different threads! First call set bool to "true" all other threads
        do nothing and return NULL as search result due to first caller reset this bool.
        This will be a bug. Without this bool-member may be we have some performance problems
        but no errors!!!
     */

    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_findFrame( sTargetFrameName, nSearchFlags ), "Frame::findFrame()", "Invalid parameter detected." )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Copy neccessary member and unlock the read lock ...
    // It's not neccessary for m_aChildFrameContainer ... because
    // he is threadsafe himself and live if we live.
    // We use a registered transaction to prevent us against
    // breaks during this operation!
    css::uno::Reference< css::frame::XFrame > xSearchedFrame                                                                     ;
    css::uno::Reference< css::frame::XFrame > xThis           ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY  );
    css::uno::Reference< css::frame::XFrame > xParent         ( m_xParent, css::uno::UNO_QUERY )                                 ;
    sal_Bool                                  bParentExist    = xParent.is()                                                     ;
    sal_Bool                                  bChildrenExist  = m_aChildFrameContainer.hasElements()                             ;
    ::rtl::OUString                           sMyName         = m_sName                                                          ;
    ::rtl::OUString                           sParentName                                                                        ;
    if( bParentExist == sal_True )
    {
        sParentName = xParent->getName();
    }

    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    LOG_PARAMETER_FINDFRAME( "Frame", sMyName, sTargetFrameName, nSearchFlags )

    // Use helper to classify search direction.
    // Attention: If he return ...BOTH -> please search down first ... and upper direction then!
    TargetInfo   aInfo   ( sTargetFrameName, nSearchFlags, E_FRAME, bChildrenExist, bParentExist, sMyName, sParentName );
    ETargetClass eResult = TargetFinder::classifyFindFrame( aInfo );
    switch( eResult )
    {
        case E_SELF         :   {
                                    xSearchedFrame = xThis;
                                }
                                break;
        case E_PARENT       :   {
                                    xSearchedFrame = xParent;
                                }
                                break;
        case E_FORWARD_UP   :   {
                                    xSearchedFrame = xParent->findFrame( sTargetFrameName, nSearchFlags );
                                }
                                break;
        case E_DEEP_DOWN    :   {
                                    xSearchedFrame = m_aChildFrameContainer.searchDeepDown( sTargetFrameName );
                                }
                                break;
        case E_DEEP_BOTH    :   {
                                    xSearchedFrame = m_aChildFrameContainer.searchDeepDown( sTargetFrameName );
                                    if( xSearchedFrame.is() == sal_False )
                                    {
                                        xSearchedFrame = xParent->findFrame( sTargetFrameName, nSearchFlags );
                                    }
                                }
                                break;
        case E_FLAT_DOWN    :   {
                                    xSearchedFrame = m_aChildFrameContainer.searchFlatDown( sTargetFrameName );
                                }
                                break;
        case E_FLAT_BOTH    :   {
                                    xSearchedFrame = m_aChildFrameContainer.searchFlatDown( sTargetFrameName );
                                    if( xSearchedFrame.is() == sal_False )
                                    {
                                        xSearchedFrame = xParent->findFrame( sTargetFrameName, nSearchFlags );
                                    }
                                }
                                break;
        #ifdef ENABLE_WARNINGS
        default             :   {
                                    if( eResult != E_UNKNOWN )
                                    {
                                        LOG_ERROR( "Frame::findFrame()", "Unexpected result of TargetFinder::classify() detected!" )
                                    }
                                }
                                break;
        #endif
    }
    LOG_RESULT_FINDFRAME( "Frame", sMyName, xSearchedFrame )
    // Return result of operation.
    return xSearchedFrame;
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
    @descr      This feature is used to mark active pathes in our frame hierarchy.
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
    css::uno::Reference< css::frame::XFrame >           xActiveChild    = m_aChildFrameContainer.getActive()                              ;
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
        Window* pWindow = VCLUnoHelper::GetWindow( xComponentWindow );
        if( pWindow != NULL )
        {
            Application::SetDefModalDialogParent( pWindow );
        }
    }
}

/*-****************************************************************************************************//**
    @short      deactivate frame in hierarchy
    @descr      This feature is used to deactive pathes in our frame hierarchy.
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
    css::uno::Reference< css::frame::XFrame >           xActiveChild    = m_aChildFrameContainer.getActive()                              ;
    css::uno::Reference< css::frame::XFramesSupplier >  xParent         ( m_xParent, css::uno::UNO_QUERY )                                ;
    css::uno::Reference< css::frame::XFrame >           xThis           ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    EActiveState                                        eState          = m_eActiveState                                                  ;

    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // Work only, if there something to do!
    if( eState != E_INACTIVE )
    {
        //_____________________________________________________________________________________________________
        //  1)  Deactivate all active childs.
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

    @attention  A current set component can disagree with suspending call!
                We don't set the new one and return with false.

    @seealso    method getComponentWindow()
    @seealso    method getController()

    @param      "xComponentWindow"  , valid reference to new component window as child of internal container window
    @param      "xController"       , valid reference to new component controller
    @return     true if operation was successful, false otherwise.

    @onerror    -
*//*-*****************************************************************************************************/
sal_Bool SAL_CALL Frame::setComponent(  const   css::uno::Reference< css::awt::XWindow >&      xComponentWindow ,
                                        const   css::uno::Reference< css::frame::XController >& xController      ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_setComponent( xComponentWindow, xController ), "Frame::setComponent()", "Invalid parameter detected." )
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Use threadsafe impl-method!
    return implts_setComponent( xComponentWindow, xController );
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
    // Register transaction and reject wrong calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

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
    /*ATTENTION
        Make it threadsafe ... but this method is a special one!
        We must close objet for working BEFORE we dispose it realy ...
        After successful closing all interface calls are rejected by our
        transaction manager automaticly.

        But there exist something to do ... which should be done BEFORE we start realy disposing of this instance!
        So we should clear listener and window mechanism first.
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

    // We should hold a reference to ourself ...
    // because our owner dispose us and release our reference ...
    // May be we will die before we could finish this method ...
    // Make snapshot of other neecessary member too.
    css::uno::Reference< css::frame::XFrame > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    #ifdef ENABLE_EVENTDEBUG
    // We should be threadsafe in special debug sessions too :-)
    ::rtl::OUString sName = m_sName;
    #endif

    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // First operation should be ... "stopp all listening for window events on our container window".
    // These events are superflous but can make trouble!
    // We will die, die and die ...
    implts_stopWindowListening();

    // It's neccessary to forget our component and her window too...
    // because this mechanism could start different callback mechanism!
    // And it's not a good idea to disable this object for real working by setting mode to E_BEFORECLOSE here.
    // Otherwise to much calls must be handle the special "IN-DISPOSING" case.
    implts_setComponent( css::uno::Reference< css::awt::XWindow >      () ,
                         css::uno::Reference< css::frame::XController >() );

    // Send message to all listener and forget her references.
    // Attention: Don't do it before you forget currently set component!
    // Because - our dispatch helper are listener on this frame. They will die, if this instance die!
    // But "implts_setComponent()" needs the dispatch mechanism ... So we should
    // forget our listener after that.
    LOG_DISPOSEEVENT( "Frame", sName )
    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Lock it again to disable object for real working!
    aWriteLock.lock();

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

    // Now we can release our lock!
    // We should be alone for ever and further dispose calls are rejected by lines before ...
    // I hope it :-)
    aWriteLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    /*ATTENTION
        It's neccessary to release our StatusIndicatorFactory-helper at first!
        He share our container window as parent for any created status indicator objects ...
        and if we dispose this container window before we release this helper ...
        we will run into some trouble!
     */
    m_xIndicatorFactoryHelper = css::uno::Reference< css::task::XStatusIndicatorFactory >();
    impl_disposeContainerWindow( m_xContainerWindow );

    // Free references of our frame tree.
    // Force parent container to forget this frame too ...
    // ( It's contained in m_xParent and so no css::lang::XEventListener for m_xParent! )
    if( m_xParent.is() == sal_True )
    {
        m_xParent->getFrames()->remove( xThis );
        m_xParent = css::uno::Reference< css::frame::XFramesSupplier >();
    }
    /*ATTENTION
        Clear container after successful removing from parent container ...
        because our parent could be a task and stand in dispose too!
        If we have already cleared our own container we lost our child before this could be
        remove himself at this instance ...
        Release m_xFramesHelper after that ... it's the same problem between parent and child!
        "m_xParent->getFrames()->remove( xThis );" needs this helper ...
        Otherwise we get a null reference and could finish removing successfuly.
        => You see: Order of calling operations is important!!!
     */
    m_aChildFrameContainer.clear();
    m_xFramesHelper = css::uno::Reference< css::frame::XFrames >();

    // Release some other references.
    // This calls should be easy ... I hope it :-)
    m_xDispatchHelper     = css::uno::Reference< css::frame::XDispatchProvider >();
    m_xFactory            = css::uno::Reference< css::lang::XMultiServiceFactory >();
    m_xDropTargetListener = css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >();

    // Disable this instance for further working realy!
    m_aTransactionManager.setWorkingMode( E_CLOSE );
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
    css::uno::Reference< css::task::XStatusIndicator >           xIndicator                                        ;
    css::uno::Reference< css::task::XStatusIndicatorSupplier >   xSupplier   ( m_xController, css::uno::UNO_QUERY );
    css::uno::Reference< css::task::XStatusIndicatorFactory >    xFactory    = m_xIndicatorFactoryHelper           ;

    aReadLock.unlock();
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    // If controller can create this status indicator ... use it for return.
    // Otherwise use our own indicator factory helper!
    if( xSupplier.is() == sal_True )
    {
        xIndicator = xSupplier->getStatusIndicator();
    }

    if(
        ( xIndicator.is() == sal_False ) &&
        ( xFactory.is()   == sal_True  )
      )
    {
        xIndicator = xFactory->createStatusIndicator();
    }

    return xIndicator;
}

/*-****************************************************************************************************//**
    @interface  XDispatchInformationProvider
    @short      get informations about supported dispatch commands
    @descr      Use this interface to get informations about possibility to dispatch special commands.
                ( e.g. "print", "close" ... )
                In current implmentation we forward requests to our internal controller ...
                but in following implementaions we could add own infos to returned values.

    @seealso    interface XDispatchInformationProvider

    @param      "sURL"          , queried command
    @param      "lURLs"         , list of commands for faster access
    @param      "lDescriptions" , result set if more then one URLs was queried
    @return     Configuration of dispatch informations at method getConfigurableDispatchInformation().

    @onerror    -
    @threadsafe yes
*//*-*****************************************************************************************************/
::rtl::OUString SAL_CALL Frame::queryDescription( const ::rtl::OUString& sURL ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    // Look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // Declare default return value if method failed.
    ::rtl::OUString sInfo;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider( m_xController, css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

    if( xProvider.is() == sal_True )
    {
        sInfo = xProvider->queryDescription( sURL );
    }
    return sInfo;
}

//*****************************************************************************************************************
void SAL_CALL Frame::queryDescriptions( const css::uno::Sequence< ::rtl::OUString >& lURLs         ,
                                              css::uno::Sequence< ::rtl::OUString >& lDescriptions ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    // Look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider( m_xController, css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

    if( xProvider.is() == sal_True )
    {
        xProvider->queryDescriptions( lURLs, lDescriptions );
    }
}

//*****************************************************************************************************************
css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL Frame::getConfigurableDispatchInformation() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    // Look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider( m_xController, css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

    css::uno::Sequence< css::frame::DispatchInformation > lInfos;
    if( xProvider.is() == sal_True )
    {
        lInfos = xProvider->getConfigurableDispatchInformation();
    }
    return lInfos;
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
    // Don't check incoming parameter here! Our helper do it for us and it isn't a good idea to do it more then ones!
    // But look for rejected calls!
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    // We use a helper to support these interface and an interceptor mechanism.
    // Our helper is threadsafe by himself!
    return m_xDispatchHelper->queryDispatch( aURL, sTargetFrameName, nSearchFlags );
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
    @short      notifications for window events
    @descr      We are a listener on our container window to forward it to our component window.

    @seealso    method setComponent()
    @seealso    member m_xContainerWindow
    @seealso    member m_xComponentWindow

    @param      "aEvent" describe source of detected event
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException )
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
void SAL_CALL Frame::focusGained( const css::awt::FocusEvent& aEvent ) throw( css::uno::RuntimeException )
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
void SAL_CALL Frame::windowActivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
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
void SAL_CALL Frame::windowDeactivated( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Check incoming parameter.
    LOG_ASSERT2( implcp_windowDeactivated( aEvent ), "Frame::windowDeactivated()", "Invalid parameter detected." )
    // Look for rejected calls.
    // Sometimes called during dispose() => soft exceptions
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    if( m_eActiveState != E_INACTIVE )
    {
        // Deactivation is always done implicitely by activation of another frame.
        // Only if no activation is done, deactivations have to be processed if the activated window
        // is a parent window of the last active Window!
        Window* pFocusWindow = Application::GetFocusWindow();
        if  (
                ( m_xContainerWindow.is()                                                              ==  sal_True    )   &&
                ( pFocusWindow                                                                         !=  NULL        )   &&
                ( m_xParent.is()                                                                       ==  sal_True    )   &&
                ( (css::uno::Reference< css::frame::XDesktop >( m_xParent, css::uno::UNO_QUERY )).is() ==  sal_False   )
            )
        {
            css::uno::Reference< css::awt::XWindow >  xParentWindow   = m_xParent->getContainerWindow()             ;
            Window*                                   pOwnWindow      = VCLUnoHelper::GetWindow( m_xContainerWindow );
            Window*                                   pParentWindow   = VCLUnoHelper::GetWindow( xParentWindow      );
            if( pParentWindow->IsChild( pFocusWindow ) )
            {
                css::uno::Reference< css::frame::XFramesSupplier > xSupplier( m_xParent, css::uno::UNO_QUERY );
                if( xSupplier.is() == sal_True )
                {
                    /* UNSAFE AREA ----------------------------------------------------------------------------- */
                    aReadLock.unlock();
                    xSupplier->setActiveFrame( css::uno::Reference< css::frame::XFrame >() );
                }
            }
        }
    }
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
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    return( m_nExternalLockCount>0 );
}

//*****************************************************************************************************************
void SAL_CALL Frame::addActionLock() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    ++m_nExternalLockCount;
}

//*****************************************************************************************************************
void SAL_CALL Frame::removeActionLock() throw( css::uno::RuntimeException )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    LOG_ASSERT2( m_nExternalLockCount<=0, "Frame::removeActionLock()", "Wrong using of frame lock detected! You remove a unregistered lock ... " )
    if( m_nExternalLockCount>0 )
    {
        --m_nExternalLockCount;
    }
}

//*****************************************************************************************************************
void SAL_CALL Frame::setActionLocks( sal_Int16 nLock ) throw( css::uno::RuntimeException )
{
    LOG_WARNING( "Frame::resetActionLocks()", "Not supported yet!" )
}

//*****************************************************************************************************************
sal_Int16 SAL_CALL Frame::resetActionLocks() throw( css::uno::RuntimeException )
{
    LOG_WARNING( "Frame::resetActionLocks()", "Not supported yet!" )
    return 0;
}

/*-****************************************************************************************************//**
    @short      try to convert a property value
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!
                You must try to convert the value of given propertyhandle and
                return results of this operation. This will be use to ask vetoable
                listener. If no listener have a veto, we will change value realy!
                ( in method setFastPropertyValue_NoBroadcast(...) )

    @seealso    OPropertySetHelper
    @seealso    setFastPropertyValue_NoBroadcast()

    @param      "aConvertedValue"   new converted value of property
    @param      "aOldValue"         old value of property
    @param      "nHandle"           handle of property
    @param      "aValue"            new value of property
    @return     sal_True if value will be changed, sal_FALSE otherway

    @onerror    IllegalArgumentException, if you call this with an invalid argument
*//*-*****************************************************************************************************/
sal_Bool SAL_CALL Frame::convertFastPropertyValue(          css::uno::Any&        aConvertedValue ,
                                                            css::uno::Any&        aOldValue       ,
                                                            sal_Int32             nHandle         ,
                                                    const   css::uno::Any&        aValue          ) throw( css::lang::IllegalArgumentException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // We don't need any mutex or lock here ... if we work with our properties only!
    // The propertyset helper synchronize for us.
    // Bur if we try to work with some other member ... we must make it threadsafe!!!

    //  Check, if value of property will changed in method "setFastPropertyValue_NoBroadcast()".
    //  Return TRUE, if changed - else return FALSE.
    //  Attention:
    //      Method "impl_tryToChangeProperty()" can throw the IllegalArgumentException !!!

    //  Initialize state with FALSE !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case PROPERTYHANDLE_TITLE   :   bReturn = impl_tryToChangeProperty( implts_getTitleFromWindow(), aValue, aOldValue, aConvertedValue );
                                        break;
        #ifdef ENABLE_WARNINGS
        default :   LOG_WARNING( "Frame::convertFastPropertyValue()", "Invalid handle detected!" )
                    break;
        #endif
    }

    // Return state of operation.
    return bReturn ;
}

/*-****************************************************************************************************//**
    @short      set value of a transient property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!
                Handle and value are valid everyway! You must set the new value only.
                After this, baseclass send messages to all listener automaticly.

    @seealso    OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    new value of property
    @return     -

    @onerror    An exception is thrown.
*//*-*****************************************************************************************************/
void SAL_CALL Frame::setFastPropertyValue_NoBroadcast(          sal_Int32       nHandle ,
                                                        const   css::uno::Any&  aValue  ) throw( css::uno::Exception )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // We don't need any mutex or lock here ... if we work with our properties only!
    // The propertyset helper synchronize for us.
    // Bur if we try to work with some other member ... we must make it threadsafe!!!

    // Search for right handle ... and try to set property value.
    switch ( nHandle )
    {
        case PROPERTYHANDLE_TITLE   :   {
                                            ::rtl::OUString sTitle;
                                            aValue >>= sTitle;
                                            implts_setTitleOnWindow( sTitle );
                                        }
                                        break;
        #ifdef ENABLE_WARNINGS
        default :   LOG_WARNING( "Frame::setFastPropertyValue_NoBroadcast()", "Invalid handle detected!" )
                    break;
        #endif
    }
}

/*-****************************************************************************************************//**
    @short      get value of a transient property
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @seealso    OPropertySetHelper

    @param      "nHandle"   handle of property to change
    @param      "aValue"    current value of property
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
void SAL_CALL Frame::getFastPropertyValue(  css::uno::Any&  aValue  ,
                                            sal_Int32       nHandle ) const
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // We don't need any mutex or lock here ... if we work with our properties only!
    // The propertyset helper synchronize for us.
    // Bur if we try to work with some other member ... we must make it threadsafe!!!

    // Search for right handle ... and try to set property value.
    switch( nHandle )
    {
        case PROPERTYHANDLE_TITLE   :   aValue <<= implts_getTitleFromWindow();
                                        break;
        #ifdef ENABLE_WARNINGS
        default :   LOG_WARNING( "Frame::getFastPropertyValue()", "Invalid handle detected!" )
                    break;
        #endif
    }
}

/*-****************************************************************************************************//**
    @short      return structure and information about transient properties
    @descr      This method is calling from helperclass "OPropertySetHelper".
                Don't use this directly!

    @seealso    OPropertySetHelper

    @param      -
    @return     structure with property-informations

    @onerror    -
*//*-*****************************************************************************************************/
::cppu::IPropertyArrayHelper& SAL_CALL Frame::getInfoHelper()
{
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

    return (*pInfoHelper);
}

/*-****************************************************************************************************//**
    @short      return propertysetinfo
    @descr      You can call this method to get information about transient properties
                of this object.

    @seealso    OPropertySetHelper
    @seealso    XPropertySet
    @seealso    XMultiPropertySet

    @param      -
    @return     reference to object with information [XPropertySetInfo]

    @onerror    -
*//*-*****************************************************************************************************/
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL Frame::getPropertySetInfo ()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = NULL ;

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

/*-****************************************************************************************************//**
    @short      build information struct of our supported properties
    @descr      We create it one times only and return it every time.

    @seealso    OPropertySetHelper
    @seealso    XPropertySet
    @seealso    XMultiPropertySet

    @param      -
    @return     const struct with information about supported properties

    @onerror    -
*//*-*****************************************************************************************************/
const css::uno::Sequence< css::beans::Property > Frame::impl_getStaticPropertyDescriptor()
{
    // Create a new static property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME !!!

    static const css::beans::Property pPropertys[] =
    {
        css::beans::Property( PROPERTYNAME_TITLE, PROPERTYHANDLE_TITLE, ::getCppuType((const ::rtl::OUString*)NULL), css::beans::PropertyAttribute::TRANSIENT ),
    };
    // Use it to initialize sequence!
    static const css::uno::Sequence< css::beans::Property > lPropertyDescriptor( pPropertys, PROPERTYCOUNT );
    // Return static "PropertyDescriptor"
    return lPropertyDescriptor;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool Frame::impl_tryToChangeProperty(   const   ::rtl::OUString&    sProperty       ,
                                            const   css::uno::Any&      aValue          ,
                                                    css::uno::Any&      aOldValue       ,
                                                    css::uno::Any&      aConvertedValue ) throw( css::lang::IllegalArgumentException )
{
    // Set default return value.
    sal_Bool bReturn = sal_False;

    // Clear information of return parameter!
    aOldValue.clear();
    aConvertedValue.clear();

    // Get new value from any.
    // IllegalArgumentException() can be thrown!
    ::rtl::OUString sNewValue;
    ::cppu::convertPropertyValue( sNewValue, aValue );

    // If value change ...
    if( sNewValue != sProperty )
    {
        // ... set information of change.
        aOldValue.setValue      ( &sProperty, ::getCppuType((const ::rtl::OUString*)NULL) );
        aConvertedValue.setValue( &sNewValue, ::getCppuType((const ::rtl::OUString*)NULL) );
        // Return OK - "value will be change ..."
        bReturn = sal_True;
    }

    return bReturn;
}

/*-****************************************************************************************************//**
    @short      helper to release old and set new container window
    @descr      This method is threadsafe AND can be called by our dispose method too!

    @seealso    -

    @param      "xNewWindow", reference to new window or null if window should be destroyed
    @return     -

    @onerror    -
*//*-*****************************************************************************************************/
/*
void Frame::implts_setContainerWindow( const css::uno::Reference< css::awt::XWindow>& xNewWindow )
{
    // Algorithm:
    //  a)  Safe current set reference as old one.
    //      We dispose it later to avaoid flickering!
    //  b)  Set new window as member.
    //  c)  Deregister old window as listener!
    //  d)  Register new window as new listener.
    //  e)  Dispose old window and free his reference.

    // UNSAFE AREA --------------------------------------------------------------------------------------------- //
    // Sometimes used by dispose() => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    // SAFE AREA ----------------------------------------------------------------------------------------------- //
    ReadGuard aReadLock( m_aLock );

    // a,b)
    // Use it to make snapshot of neccessary member and work on it after
    // releasing our read lock!
    css::uno::Reference< css::awt::XWindow >             xOldWindow              = m_xContainerWindow                                              ;
                                                         m_xContainerWindow      = xNewWindow                                                      ;
    css::uno::Reference< css::awt::XWindowListener >     xThisAsWindowListener   ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XFocusListener >      xThisAsFocusListener    ( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XTopWindowListener >  xThisAsTopWindowListener( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );

    aReadLock.unlock();
    // UNSAFE AREA --------------------------------------------------------------------------------------------- //

    // c)
    if( xOldWindow.is() == sal_True )
    {
        xOldWindow->removeWindowListener( xThisAsWindowListener );
        xOldWindow->removeFocusListener ( xThisAsFocusListener  );
    }

    // d)
    if( xNewWindow.is() == sal_True )
    {
        xNewWindow->addWindowListener( xThisAsWindowListener);
        xNewWindow->addFocusListener ( xThisAsFocusListener );

        // If possible register as TopWindowListener
        css::uno::Reference< css::awt::XTopWindow > xTopWindow( xNewWindow, css::uno::UNO_QUERY );
        if( xTopWindow.is() == sal_True )
        {
            xTopWindow->addTopWindowListener( xThisAsTopWindowListener );
        }
    }

    // e)
    if( xOldWindow.is() == sal_True )
    {
        // All VclComponents are XComponents; so call dispose before discarding
        // a css::uno::Reference< XVclComponent >, because this frame is the owner of the window
        Window* pOldWindow = VCLUnoHelper::GetWindow( xOldWindow );
        if  (
                ( pOldWindow                                !=  NULL        )   &&
                ( Application::GetDefModalDialogParent()    ==  pOldWindow  )
            )
        {
            Application::SetDefModalDialogParent( NULL );
        }

        xOldWindow->setVisible( sal_False );
        xOldWindow->dispose();
        xOldWindow = css::uno::Reference< css::awt::XWindow >();
    }
}
*/

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
        // All VclComponents are XComponents; so call dispose before discarding
        // a css::uno::Reference< XVclComponent >, because this frame is the owner of the window
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if  (
                ( pWindow                                !=  NULL     )   &&
                ( Application::GetDefModalDialogParent() ==  pWindow  )
            )
        {
            Application::SetDefModalDialogParent( NULL );
        }

        xWindow->setVisible( sal_False );
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
            ((css::frame::XFrameActionListener *)aIterator.next())->frameAction( aFrameActionEvent );
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
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Sometimes used by dispose() => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Make snapshot of both windows.
    css::uno::Reference< css::awt::XWindow >   xContainerWindow    =   m_xContainerWindow  ;
    css::uno::Reference< css::awt::XWindow >   xComponentWindow    =   m_xComponentWindow  ;

    aReadLock.unlock();
    /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

    // Work only if container window is set!
    if  (
            ( xContainerWindow.is() == sal_True )   &&
            ( xComponentWindow.is() == sal_True )
        )
    {
        // Get reference to his device.
        css::uno::Reference< css::awt::XDevice > xDevice( xContainerWindow, css::uno::UNO_QUERY );
        // Convert relativ size to output size.
        css::awt::Rectangle  aRectangle  = xContainerWindow->getPosSize();
        css::awt::DeviceInfo aInfo       = xDevice->getInfo();
        css::awt::Size       aSize       (   aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                             aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );
        // Resize ouer component window.
        xComponentWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, css::awt::PosSize::SIZE );
    }
}

/*-****************************************************************************************************//**
    @short      helper to set/get a title on/from our container window
    @descr      We need this impl method to make it threadsafe. Another reason is - we can't hold this value
                by using an own member ... because if somewhere change the title by using the vcl-window directly ...
                we never get this information! That's why we write and rewad this property direct via toolkit and vcl!

    @seealso    interface XVclWindowPeer

    @param      "sTitle", new value to set it on our window
    @return     Current title of our window.

    @onerror    We do nothing or return an empty value.
*//*-*****************************************************************************************************/
void Frame::implts_setTitleOnWindow( const ::rtl::OUString& sTitle )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Look for rejected calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Make snapshot of neccessary members and release lock.
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XVclWindowPeer > xContainerWindow( m_xContainerWindow, css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // Use non well known feature of toolkit to set property on window!
    // -> cast window to XVclWindowPeer interface and call setProperty() ...
    if( xContainerWindow.is() == sal_True )
    {
        css::uno::Any aValue;
        aValue <<= sTitle;
        xContainerWindow->setProperty( DECLARE_ASCII("Title"), aValue );
    }
}

//*****************************************************************************************************************
const ::rtl::OUString Frame::implts_getTitleFromWindow() const
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Look for rejected calls.
    TransactionGuard aTransaction( m_aTransactionManager, E_HARDEXCEPTIONS );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    // Make snapshot of neccessary members and release lock.
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::awt::XVclWindowPeer > xContainerWindow( m_xContainerWindow, css::uno::UNO_QUERY );
    aReadLock.unlock();
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */

    // Use non well known feature of toolkit to get property from window!
    // -> cast window to XVclWindowPeer interface and call getProperty() ...
    ::rtl::OUString sTitle;
    if( xContainerWindow.is() == sal_True )
    {
        css::uno::Any aValue = xContainerWindow->getProperty( DECLARE_ASCII("Title") );
        aValue >>= sTitle;
    }
    return sTitle;
}

/*-****************************************************************************************************//**
    @short      helper to change current component with window
    @descr      This method is used by two different interface methods - dispose() and setComponent().
                It's a threadsafe one and handle calling by dispose by using E_SOFTEXCEPTIONS too!

    @seealso    method setComponent()
    @seealso    method dispose()

    @param      "xComponentWindow", new window of our component or NULL if component should be destroyed
    @param      "xController"     , new controller of our component or NULL if component should be destroyed
    @return     Successful state of operation.

    @onerror    We return false.
*//*-*****************************************************************************************************/
sal_Bool Frame::implts_setComponent(  const   css::uno::Reference< css::awt::XWindow >&        xComponentWindow ,
                                      const   css::uno::Reference< css::frame::XController >&  xController      )
{
    /* UNSAFE AREA --------------------------------------------------------------------------------------------- */
    // Sometimes used by dispose() => soft exceptions!
    TransactionGuard aTransaction( m_aTransactionManager, E_SOFTEXCEPTIONS );

    /*HACK
        ... for sfx2! Sometimes he call me by using this combination.
    */
    if  (
            (
                ( xController.is()      ==  sal_True    )   &&
                ( xComponentWindow.is() ==  sal_False   )
            ) == sal_False
        )
    {
        /* SAFE AREA ------------------------------------------------------------------------------------------- */
        // Normaly a ReadLock is enough ... but we need some write locks at later time.
        // It's better to create it yet und use it later to create read AND write locks!!!
        WriteGuard aWriteLock( m_aLock );

        // Make snapshot of our internal member and states and release lock!
        css::uno::Reference< css::awt::XWindow >        xContainerWindow    = m_xContainerWindow                                                    ;
        css::uno::Reference< css::awt::XWindow >        xOldComponentWindow = m_xComponentWindow                                                    ;
        css::uno::Reference< css::frame::XController >  xOldController      = m_xController                                                         ;
        sal_Bool                                        bControllerChange   = ( m_xController       !=  xController         )                       ;
        sal_Bool                                        bWindowChange       = ( m_xComponentWindow  !=  xComponentWindow    )                       ;
        sal_Bool                                        bConnected          = m_bConnected                                                          ;
        sal_Bool                                        bHasFocus           = ( m_eActiveState==E_FOCUS && m_xComponentWindow.is()==sal_True    )   ;

        aWriteLock.unlock();
        /* UNSAFE AREA ----------------------------------------------------------------------------------------- */

        // Release current component, if there is any.
        if  (
                ( xOldComponentWindow.is()  ==  sal_True    )   ||
                ( xOldController.is()       ==  sal_True    )
            )
        {
            implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_DETACHING );
        }

        // Always release controller before releasing window, because controller may want to access its window!
        if( bControllerChange == sal_True )
        {
            if( xOldController.is() == sal_True )
            {
                /* SAFE AREA ----------------------------------------------------------------------------------- */
                aWriteLock.lock();
                m_xController->dispose();
                m_xController   = css::uno::Reference< css::frame::XController >();
                xOldController  = css::uno::Reference< css::frame::XController >(); // Don't forget to release last reference to m_xController!
                aWriteLock.unlock();
                /* UNSAFE AREA --------------------------------------------------------------------------------- */
            }
        }

        // Release component window.
        if( bWindowChange == sal_True )
        {
            // Set new one ...
            // resize it to fill our containerwindow ...
            // and dispose the old one.

            /* SAFE AREA --------------------------------------------------------------------------------------- */
            aWriteLock.lock();
            // We can set the new one here ... because we hold it as xOldComponentWindow too!
            m_xComponentWindow = xComponentWindow;
            aWriteLock.unlock();
            /* UNSAFE AREA ------------------------------------------------------------------------------------- */

            implts_resizeComponentWindow();
            if( xOldComponentWindow.is() == sal_True )
            {
                // All VclComponents are XComponents; so call dispose before discarding
                // a css::uno::Reference< XVclComponent >, because this frame is the owner of the Component.
                Window* pContainerWindow    = VCLUnoHelper::GetWindow( xContainerWindow     );
                Window* pOldComponentWindow = VCLUnoHelper::GetWindow( xOldComponentWindow  );
                if  (
                        ( pOldComponentWindow                       !=  NULL                )   &&
                        ( Application::GetDefModalDialogParent()    ==  pOldComponentWindow )
                    )
                {
                    Application::SetDefModalDialogParent( pContainerWindow );
                }
                xOldComponentWindow->dispose();
                xOldComponentWindow = css::uno::Reference< css::awt::XWindow >();
            }
        }

        // Set new controller.
        if( bControllerChange == sal_True )
        {
            /* SAFE AREA --------------------------------------------------------------------------------------- */
            aWriteLock.lock();
            m_xController = xController;
            aWriteLock.unlock();
            /* UNSAFE AREA ------------------------------------------------------------------------------------- */
        }

        // Send action events to all listener - depends from our connect state
        if  (
                ( xController.is()        ==  sal_True    )   ||
                ( xComponentWindow.is()   ==  sal_True    )
            )
        {
            if( bConnected == sal_True )
            {
                implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_REATTACHED );
            }
            else
            {
                implts_sendFrameActionEvent( css::frame::FrameAction_COMPONENT_ATTACHED   );
            }
        }

        // A new component doesn't know anything about current active/focus states.
        // Give her this information!
        if  (
                ( bHasFocus             == sal_True )   &&
                ( xComponentWindow.is() == sal_True )
            )
        {
            xComponentWindow->setFocus();
            Window* pWindow = VCLUnoHelper::GetWindow( xComponentWindow );
            if( pWindow != NULL )
            {
                Application::SetDefModalDialogParent( pWindow );
            }
        }

        /* SAFE AREA ------------------------------------------------------------------------------------------- */
        aWriteLock.lock();
        m_bConnected = sal_True;
        aWriteLock.unlock();
        /* UNSAFE AREA ----------------------------------------------------------------------------------------- */
    }

    return sal_True;
}

/*-************************************************************************************************************//**
    @short          filter special names
    @attention      If somewhere have a name value ... but don't know if he can set it on a frame ...
                    he should call this helper to clear all questions.
                    Some special target names are not allowed as frame name!
*//*-*************************************************************************************************************/
void Frame::impl_filterSpecialTargets( ::rtl::OUString& sTarget )
{
    if  (
            ( sTarget   ==  SPECIALTARGET_SELF      )   ||
            ( sTarget   ==  SPECIALTARGET_PARENT    )   ||
            ( sTarget   ==  SPECIALTARGET_TOP       )   ||
            ( sTarget   ==  SPECIALTARGET_BLANK     )
        )
    {
        sTarget = ::rtl::OUString();
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
                ( css::uno::Reference< css::frame::XTask >( xFrame, css::uno::UNO_QUERY ).is()              ==  sal_True    )   ||
                ( css::uno::Reference< css::frame::XDesktop >( xFrame, css::uno::UNO_QUERY ).is()           ==  sal_True    )   ||
                ( css::uno::Reference< css::mozilla::XPluginInstance >( xFrame, css::uno::UNO_QUERY ).is()  ==  sal_True    )
            );
}

//*****************************************************************************************************************
// We don't accept null pointer ... but NULL-References are allowed!
sal_Bool Frame::implcp_initialize( const css::uno::Reference< css::awt::XWindow >& xWindow )
{
    return( &xWindow == NULL );
}

//*****************************************************************************************************************
// We don't accept null pointer or references!
sal_Bool Frame::implcp_setCreator( const css::uno::Reference< css::frame::XFramesSupplier >& xCreator )
{
    return  (
                ( &xCreator     ==  NULL        )   ||
                ( xCreator.is() ==  sal_False   )
            );
}

//*****************************************************************************************************************
// We don't accept null pointer or references!
sal_Bool Frame::implcp_setName( const ::rtl::OUString& sName )
{
    return( &sName == NULL );
}

//*****************************************************************************************************************
// We don't accept null pointer or references!
// An empty target name is allowed => is the same like "_self"
sal_Bool Frame::implcp_findFrame(  const   ::rtl::OUString& sTargetFrameName,
                                            sal_Int32        nSearchFlags    )
{
    return( &sTargetFrameName == NULL );
}

//*****************************************************************************************************************
// We don't accept null pointer!
sal_Bool Frame::implcp_setComponent(   const   css::uno::Reference< css::awt::XWindow >&       xComponentWindow    ,
                                        const   css::uno::Reference< css::frame::XController >& xController         )
{
    return  (
                ( &xComponentWindow ==  NULL    )   ||
                ( &xController      ==  NULL    )
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
