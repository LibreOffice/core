/*************************************************************************
 *
 *  $RCSfile: sfxbasecontroller.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:29:24 $
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

//________________________________________________________________________________________________________
//  my own includes
//________________________________________________________________________________________________________

#include <time.h>

#ifndef _SFX_SFXBASECONTROLLER_HXX_
#include <sfxbasecontroller.hxx>
#endif

//________________________________________________________________________________________________________
//  include of other projects
//________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HPP_
#include <com/sun/star/awt/MouseButton.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_CLOSEVETOEXCEPTION_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HPP_
#include <com/sun/star/frame/FrameActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

#ifndef _SFXVIEWSH_HXX
#include <viewsh.hxx>
#endif

#ifndef _SFXVIEWFRM_HXX
#include <viewfrm.hxx>
#endif

#ifndef _SFX_OBJSH_HXX
#include <objsh.hxx>
#endif

#ifndef _SFXAPP_HXX
#include <app.hxx>
#endif

#ifndef _SFXMSGPOOL_HXX
#include <msgpool.hxx>
#endif

#ifndef _SFXDISPATCH_HXX
#include <dispatch.hxx>
#endif

#include <viewimp.hxx>
#include <unoctitm.hxx>
#include <childwin.hxx>
#include <sfxsids.hrc>
#include <workwin.hxx>
#include <stbmgr.hxx>
#include <objface.hxx>

#include <vos/mutex.hxx>
#include <osl/mutex.hxx>

#define OMULTITYPEINTERFACECONTAINERHELPER      ::cppu::OMultiTypeInterfaceContainerHelper
#define OINTERFACECONTAINERHELPER               ::cppu::OInterfaceContainerHelper
#define XFRAMEACTIONLISTENER                    ::com::sun::star::frame::XFrameActionListener
#define XCLOSELISTENER                          ::com::sun::star::util::XCloseListener
#define FRAMEACTIONEVENT                        ::com::sun::star::frame::FrameActionEvent
#define EVENTOBJECT                             ::com::sun::star::lang::EventObject
#define OTYPECOLLECTION                         ::cppu::OTypeCollection
#define OIMPLEMENTATIONID                       ::cppu::OImplementationId
#define MUTEXGUARD                              ::osl::MutexGuard
#define UNOQUERY                                ::com::sun::star::uno::UNO_QUERY
#define MAPPING                                 ::com::sun::star::uno::Mapping
#define XSTATUSINDICATORSUPPLIER                ::com::sun::star::task::XStatusIndicatorSupplier
#define XCOMPONENT                              ::com::sun::star::lang::XComponent
#define XINTERFACE                              ::com::sun::star::uno::XInterface
#define XKEYHANDLER                             ::drafts::com::sun::star::awt::XKeyHandler
#define XMOUSECLICKHANDLER                      ::drafts::com::sun::star::awt::XMouseClickHandler

#define TIMEOUT_START_RESCHEDULE    10L /* 10th s */

sal_uInt32 Get10ThSec()
{
    sal_uInt32 n10Ticks = 10 * (sal_uInt32)clock();
    return n10Ticks / CLOCKS_PER_SEC;
}

sal_Int32 m_nInReschedule = 0;  /// static counter for rescheduling

void reschedule()
{
    if ( m_nInReschedule == 0 )
    {
        ++m_nInReschedule;
        Application::Reschedule();
        --m_nInReschedule;
    }
}

class SfxStatusIndicator : public ::cppu::WeakImplHelper2< ::com::sun::star::task::XStatusIndicator, ::com::sun::star::lang::XEventListener >
{
friend class SfxBaseController;
    ::com::sun::star::uno::Reference < XCONTROLLER > xOwner;
    SfxWorkWindow*          pWorkWindow;
    sal_Int32               _nRange;
    sal_Int32               _nValue;
    long                    _nStartTime;
public:
                            SfxStatusIndicator(SfxBaseController* pController, SfxWorkWindow* pWork)
                                : xOwner( pController )
                                , pWorkWindow( pWork )
                            {
                                ++m_refCount;
                                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComponent(
                                    SAL_STATIC_CAST(::cppu::OWeakObject*, pController ), ::com::sun::star::uno::UNO_QUERY );
                                if (xComponent.is())
                                    xComponent->addEventListener(this);
                                --m_refCount;
                            }

    virtual void SAL_CALL   start(const ::rtl::OUString& aText, sal_Int32 nRange) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   end(void) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setText(const ::rtl::OUString& aText) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setValue(sal_Int32 nValue) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   reset() throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL   disposing( const com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
};

void SAL_CALL SfxStatusIndicator::start(const ::rtl::OUString& aText, sal_Int32 nRange) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
    {
        _nRange = nRange;
        _nValue = 0;
        SfxStatusBarManager* pMgr = pWorkWindow->GetStatusBarManager_Impl();
        if ( !pMgr )
            pWorkWindow->SetTempStatusBar_Impl( TRUE );
        pMgr = pWorkWindow->GetStatusBarManager_Impl();
        if ( pMgr && !pMgr->IsProgressMode() )
            pMgr->StartProgressMode( aText, nRange );
        _nStartTime = Get10ThSec();
        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::end(void) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
    {
        SfxStatusBarManager* pMgr = pWorkWindow->GetStatusBarManager_Impl();
        if ( pMgr && pMgr->IsProgressMode() )
            pMgr->EndProgressMode();
        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::setText(const ::rtl::OUString& aText) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
    {
        SfxStatusBarManager* pMgr = pWorkWindow->GetStatusBarManager_Impl();
        if ( !pMgr )
            pWorkWindow->SetTempStatusBar_Impl( TRUE );
        pMgr = pWorkWindow->GetStatusBarManager_Impl();
        if ( pMgr )
        {
            if( pMgr->IsProgressMode() )
            {
                // anders kann VCL das leider nicht
                pMgr->GetStatusBar()->SetUpdateMode( FALSE );
                pMgr->EndProgressMode();
                pMgr->StartProgressMode( aText, _nRange );
                pMgr->SetProgressState( _nValue );
                pMgr->GetStatusBar()->SetUpdateMode( TRUE );
            }
            else
            {
                if ( aText.getLength() )
                    pMgr->ShowHelpText( aText );
                else
                {
                    pMgr->ShowItems();
                    reset();
                }
            }
        }

        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::setValue( sal_Int32 nValue ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
    {
        _nValue = nValue;
        SfxStatusBarManager* pMgr = pWorkWindow->GetStatusBarManager_Impl();
        if ( pMgr && pMgr->IsProgressMode() )
            pMgr->SetProgressState( nValue );
        sal_Bool bReschedule = (( Get10ThSec() - _nStartTime ) > TIMEOUT_START_RESCHEDULE );
        if ( bReschedule )
            reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::reset() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xOwner.is() )
    {
        SfxStatusBarManager* pMgr = pWorkWindow->GetStatusBarManager_Impl();
        if ( pMgr )
            pMgr->ShowItems();
        pWorkWindow->SetTempStatusBar_Impl( FALSE );
        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::disposing( const com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    xOwner = 0;
}

//________________________________________________________________________________________________________
//________________________________________________________________________________________________________
//  declaration IMPL_SfxBaseController_ListenerHelper
//________________________________________________________________________________________________________

class IMPL_SfxBaseController_ListenerHelper : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XFrameActionListener >
{
public:
    IMPL_SfxBaseController_ListenerHelper(  MUTEX&              aMutex      ,
                                            SfxBaseController*  pController ) ;
    virtual ~IMPL_SfxBaseController_ListenerHelper() ;
    virtual void SAL_CALL frameAction( const FRAMEACTIONEVENT& aEvent ) throw (RUNTIMEEXCEPTION) ;
    virtual void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw (RUNTIMEEXCEPTION) ;

private:

    MUTEX&                  m_aMutex        ;
    SfxBaseController*      m_pController   ;

} ; // class IMPL_SfxBaseController_ListenerContainer

class IMPL_SfxBaseController_CloseListenerHelper : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XCloseListener >
{
public:
    IMPL_SfxBaseController_CloseListenerHelper( MUTEX&              aMutex      ,
                                            SfxBaseController*  pController ) ;
    virtual ~IMPL_SfxBaseController_CloseListenerHelper() ;
    virtual void SAL_CALL queryClosing( const EVENTOBJECT& aEvent, sal_Bool bDeliverOwnership ) throw (RUNTIMEEXCEPTION) ;
    virtual void SAL_CALL notifyClosing( const EVENTOBJECT& aEvent ) throw (RUNTIMEEXCEPTION) ;
    virtual void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw (RUNTIMEEXCEPTION) ;

private:

    MUTEX&                  m_aMutex;
    SfxBaseController*      m_pController;

} ; // class IMPL_SfxBaseController_ListenerContainer

IMPL_SfxBaseController_CloseListenerHelper::IMPL_SfxBaseController_CloseListenerHelper( MUTEX&              aMutex      ,
                                                                                SfxBaseController*  pController )
        : m_aMutex      ( aMutex        )
        , m_pController ( pController   )
{
}

IMPL_SfxBaseController_CloseListenerHelper::~IMPL_SfxBaseController_CloseListenerHelper()
{
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::disposing( const EVENTOBJECT& aEvent ) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::queryClosing( const EVENTOBJECT& aEvent, sal_Bool bDeliverOwnership ) throw (RUNTIMEEXCEPTION)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if  ( m_pController !=  NULL )
    {
        BOOL bCanClose = (BOOL) m_pController->GetViewShell_Impl()->PrepareClose( FALSE );
        if ( !bCanClose )
        {
            if ( bDeliverOwnership )
                m_pController->TakeOwnerShip_Impl();
            throw com::sun::star::util::CloseVetoException(::rtl::OUString::createFromAscii("Controller disagree ..."),static_cast< ::cppu::OWeakObject*>(this));
        }
    }
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::notifyClosing( const EVENTOBJECT& aEvent ) throw (RUNTIMEEXCEPTION)
{
}

//________________________________________________________________________________________________________
//  declaration IMPL_SfxBaseController_DataContainer
//________________________________________________________________________________________________________

struct IMPL_SfxBaseController_DataContainer
{
    REFERENCE < XFRAME >    m_xFrame;
    REFERENCE < XFRAMEACTIONLISTENER >      m_xListener       ;
    REFERENCE < XCLOSELISTENER >      m_xCloseListener       ;
    OMULTITYPEINTERFACECONTAINERHELPER      m_aListenerContainer    ;
    OINTERFACECONTAINERHELPER               m_aInterceptorContainer    ;
    REFERENCE < ::com::sun::star::task::XStatusIndicator > m_xIndicator;
    SfxViewShell*                           m_pViewShell            ;
    SfxBaseController*                      m_pController           ;
    sal_Bool                                m_bDisposing            ;
    sal_Bool                                m_bGotOwnerShip;
    sal_Bool                                m_bHasKeyListeners;
    sal_Bool                                m_bHasMouseClickListeners;

    IMPL_SfxBaseController_DataContainer(   MUTEX&              aMutex      ,
                                            SfxViewShell*       pViewShell  ,
                                            SfxBaseController*  pController )
            :   m_xListener       ( new IMPL_SfxBaseController_ListenerHelper( aMutex, pController ) )
            ,   m_xCloseListener       ( new IMPL_SfxBaseController_CloseListenerHelper( aMutex, pController ) )
            ,   m_aListenerContainer    ( aMutex                                                )
            ,   m_aInterceptorContainer ( aMutex                                                )
            ,   m_pViewShell            ( pViewShell                                            )
            ,   m_pController           ( pController                                           )
            ,   m_bDisposing            ( sal_False                                             )
            ,   m_bGotOwnerShip         ( sal_False                                             )
            ,   m_bHasKeyListeners      ( sal_False                                             )
            ,   m_bHasMouseClickListeners( sal_False                                                )
    {
    }

} ; // struct IMPL_SfxBaseController_DataContainer

//________________________________________________________________________________________________________
//  IMPL_SfxBaseController_ListenerHelper constructor
//________________________________________________________________________________________________________

IMPL_SfxBaseController_ListenerHelper::IMPL_SfxBaseController_ListenerHelper(   MUTEX&              aMutex      ,
                                                                                SfxBaseController*  pController )
        : m_aMutex      ( aMutex        )
        , m_pController ( pController   )
{
}

//________________________________________________________________________________________________________
//  IMPL_SfxBaseController_ListenerHelper destructor
//________________________________________________________________________________________________________

IMPL_SfxBaseController_ListenerHelper::~IMPL_SfxBaseController_ListenerHelper()
{
}

void SAL_CALL IMPL_SfxBaseController_ListenerHelper::frameAction( const FRAMEACTIONEVENT& aEvent ) throw( RUNTIMEEXCEPTION )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if  (
            ( m_pController !=  NULL ) &&
            ( aEvent.Frame  ==  m_pController->getFrame() ) &&
            ( m_pController->GetViewShell_Impl() && m_pController->GetViewShell_Impl()->GetWindow() !=  NULL                                                    )
        )
    {
        if ( aEvent.Action == ::com::sun::star::frame::FrameAction_FRAME_UI_ACTIVATED )
        {
            m_pController->GetViewShell_Impl()->GetViewFrame()->MakeActive_Impl( FALSE );
        }
        if ( aEvent.Action == ::com::sun::star::frame::FrameAction_FRAME_ACTIVATED )
        {
            SfxViewFrame* pFrame = m_pController->GetViewShell_Impl()->GetViewFrame();
            if ( !pFrame->GetActiveChildFrame_Impl() )
                pFrame->MakeActive_Impl( FALSE );
        }
        else if ( aEvent.Action == ::com::sun::star::frame::FrameAction_CONTEXT_CHANGED )
        {
            m_pController->GetViewShell_Impl()->GetViewFrame()->GetBindings().ContextChanged_Impl();
        }
    }
}

//________________________________________________________________________________________________________
//  IMPL_SfxBaseController_ListenerHelper -> XEventListener
//________________________________________________________________________________________________________

void SAL_CALL IMPL_SfxBaseController_ListenerHelper::disposing( const EVENTOBJECT& aEvent ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pController && m_pController->getFrame().is() )
        m_pController->getFrame()->removeFrameActionListener( this ) ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> constructor
//________________________________________________________________________________________________________

SfxBaseController::SfxBaseController( SfxViewShell* pViewShell )
    :   IMPL_SfxBaseController_MutexContainer   (                                                                       )
    ,   m_pData                                 ( new IMPL_SfxBaseController_DataContainer( m_aMutex, pViewShell, this ))
{
    m_pData->m_pViewShell->SetController( this );
}

//________________________________________________________________________________________________________
//  SfxBaseController -> destructor
//________________________________________________________________________________________________________

SfxBaseController::~SfxBaseController()
{
    delete m_pData;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XInterface
//________________________________________________________________________________________________________
ANY SAL_CALL SfxBaseController::queryInterface( const UNOTYPE& rType ) throw( RUNTIMEEXCEPTION )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Ask for my own supported interfaces ...
    ANY aReturn( ::cppu::queryInterface(    rType                                       ,
                                               static_cast< XTYPEPROVIDER*      > ( this )  ,
                                            static_cast< XCOMPONENT*       > ( this )  ,
                                               static_cast< XCONTROLLER*        > ( this )  ,
                                               static_cast< XUSERINPUTINTERCEPTION*     > ( this )  ,
                                            static_cast< XSTATUSINDICATORSUPPLIER* > ( this )  ,
                                            static_cast< XCONTEXTMENUINTERCEPTION* > ( this ) ,
                                               static_cast< XDISPATCHPROVIDER*  > ( this )  ) ) ;

    // If searched interface supported by this class ...
    if ( aReturn.hasValue() == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return OWeakObject::queryInterface( rType ) ;
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XInterface
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire() ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XInterface
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::release() ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XTypeProvider
//________________________________________________________________________________________________________

SEQUENCE< UNOTYPE > SAL_CALL SfxBaseController::getTypes() throw( RUNTIMEEXCEPTION )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static OTYPECOLLECTION* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MUTEXGUARD aGuard( MUTEX::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static OTYPECOLLECTION aTypeCollection( ::getCppuType(( const REFERENCE< XTYPEPROVIDER      >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XCONTROLLER      >*)NULL ) ,
                                                      ::getCppuType(( const REFERENCE< XDISPATCHPROVIDER    >*)NULL ) ,
                                                    ::getCppuType(( const REFERENCE< XSTATUSINDICATORSUPPLIER >*)NULL ) ,
                                                    ::getCppuType(( const REFERENCE< XCONTEXTMENUINTERCEPTION   >*)NULL ) ,
                                                    ::getCppuType(( const REFERENCE< XUSERINPUTINTERCEPTION   >*)NULL ) );
            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XTypeProvider
//________________________________________________________________________________________________________

SEQUENCE< sal_Int8 > SAL_CALL SfxBaseController::getImplementationId() throw( RUNTIMEEXCEPTION )
{
    // Create one Id for all instances of this class.
    // Use ethernet address to do this! (sal_True)

    // Optimize this method
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pID is NULL - for the second call pID is different from NULL!
    static OIMPLEMENTATIONID* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MUTEXGUARD aGuard( MUTEX::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static OIMPLEMENTATIONID aID( sal_False ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::attachFrame( const REFERENCE< XFRAME >& xFrame ) throw( ::com::sun::star::uno::RuntimeException )
{
    REFERENCE< XFRAME > xTemp( getFrame() ) ;

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( xTemp.is() )
    {
        xTemp->removeFrameActionListener( m_pData->m_xListener ) ;
        REFERENCE < ::com::sun::star::util::XCloseBroadcaster > xCloseable( xTemp, com::sun::star::uno::UNO_QUERY );
        if ( xCloseable.is() )
            xCloseable->removeCloseListener( m_pData->m_xCloseListener );
    }

    m_pData->m_xFrame = xFrame;

    if ( xFrame.is() )
    {
        xFrame->addFrameActionListener( m_pData->m_xListener ) ;
        REFERENCE < ::com::sun::star::util::XCloseBroadcaster > xCloseable( xFrame, com::sun::star::uno::UNO_QUERY );
        if ( xCloseable.is() )
            xCloseable->addCloseListener( m_pData->m_xCloseListener );
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseController::attachModel( const REFERENCE< XMODEL >& xModel ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( m_pData->m_pViewShell && xModel.is() && xModel != m_pData->m_pViewShell->GetObjectShell()->GetModel() )
    {
        // don't allow to reattach a model!
        DBG_ERROR("Can't reattach model!");
        return sal_False;
    }

    REFERENCE < ::com::sun::star::util::XCloseBroadcaster > xCloseable( xModel, com::sun::star::uno::UNO_QUERY );
    if ( xCloseable.is() )
        xCloseable->addCloseListener( m_pData->m_xCloseListener );
    return sal_True;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseController::suspend( sal_Bool bSuspend ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( bSuspend == sal_True )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if ( !m_pData->m_pViewShell )
            return sal_True;

        if ( m_pData->m_pViewShell->PrepareClose() )
        {
            if ( getFrame().is() )
                getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;
            SfxViewFrame* pActFrame = m_pData->m_pViewShell->GetFrame() ;

            // weitere View auf dasselbe Doc?
            SfxObjectShell* pDocShell   =   m_pData->m_pViewShell->GetObjectShell() ;
            sal_Bool        bOther      =   sal_False                               ;

            for ( const SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell ); !bOther && pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell ) )
            {
                bOther = (pFrame != pActFrame);
            }

            // Doc braucht nur gefragt zu werden, wenn keine weitere ::com::sun::star::sdbcx::View
            return ( bOther || pDocShell->PrepareClose() ) ;
        }
        else
        {
            return sal_False ;
        }
    }
    else
    {
        if ( getFrame().is() )
            getFrame()->addFrameActionListener( m_pData->m_xListener ) ;
        return sal_True ;
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

ANY SfxBaseController::getViewData() throw( ::com::sun::star::uno::RuntimeException )
{
    ANY         aAny;
    String      sData1;
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
    {
        m_pData->m_pViewShell->WriteUserData( sData1 ) ;
        OUSTRING    sData( sData1 );
        aAny <<= sData ;
    }

    return aAny ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::restoreViewData( const ANY& aValue ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
    {
        OUSTRING sData;
        aValue >>= sData ;
        m_pData->m_pViewShell->ReadUserData( sData ) ;
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

REFERENCE< XFRAME > SAL_CALL SfxBaseController::getFrame() throw( ::com::sun::star::uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    return m_pData->m_xFrame;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

REFERENCE< XMODEL > SAL_CALL SfxBaseController::getModel() throw( ::com::sun::star::uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    return m_pData->m_pViewShell ? m_pData->m_pViewShell->GetObjectShell()->GetModel() : REFERENCE < XMODEL > () ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XDispatchProvider
//________________________________________________________________________________________________________

REFERENCE< XDISPATCH > SAL_CALL SfxBaseController::queryDispatch(   const   UNOURL&             aURL            ,
                                                                    const   OUSTRING&           sTargetFrameName,
                                                                            sal_Int32           eSearchFlags    ) throw( RUNTIMEEXCEPTION )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    REFERENCE< XDISPATCH >  xDisp;
    if ( m_pData->m_pViewShell )
    {
        SfxViewFrame*           pAct    = m_pData->m_pViewShell->GetViewFrame() ;
        if ( !m_pData->m_bDisposing )
        {
            sal_uInt16 nId = 0;
            if ( sTargetFrameName.compareToAscii( "_beamer" ) == COMPARE_EQUAL )
            {
                SfxViewFrame *pFrame = m_pData->m_pViewShell->GetViewFrame();
                if ( eSearchFlags & ( ::com::sun::star::frame::FrameSearchFlag::CREATE ))
                    pFrame->SetChildWindow( SID_BROWSER, TRUE );
                SfxChildWindow* pChildWin = pFrame->GetChildWindow( SID_BROWSER );
                REFERENCE < XFRAME > xFrame;
                if ( pChildWin )
                    xFrame = ( pChildWin->GetFrame() );
                if ( xFrame.is() )
                    xFrame->setName( sTargetFrameName );

                REFERENCE < XDISPATCHPROVIDER > xProv( xFrame, ::com::sun::star::uno::UNO_QUERY );
                if ( xProv.is() )
                    return xProv->queryDispatch( aURL, sTargetFrameName, ::com::sun::star::frame::FrameSearchFlag::SELF );
            }

            if ( aURL.Protocol.compareToAscii( ".uno:" ) == COMPARE_EQUAL )
            {
                SfxShell *pShell=0;
                USHORT nIdx;
                for (nIdx=0; (pShell=pAct->GetDispatcher()->GetShell(nIdx)); nIdx++)
                {
                    const SfxInterface *pIFace = pShell->GetInterface();
                    const SfxSlot* pSlot = pIFace->GetSlot( aURL.Path );
                    if ( pSlot )
                    {
                        nId = pSlot->GetSlotId();
                        break;
                    }
                }
            }
            else if ( aURL.Protocol.compareToAscii( "slot:" ) == COMPARE_EQUAL )
            {
                nId = (USHORT) aURL.Path.toInt32();
            }
            else if( sTargetFrameName.compareToAscii( "_self" )==COMPARE_EQUAL || sTargetFrameName.getLength()==0 )
            {
                // check for already loaded URL ... but with additional jumpmark!
                REFERENCE< XMODEL > xModel = getModel();
                if( xModel.is() && aURL.Mark.getLength() )
                {
                    if( aURL.Main.getLength() && aURL.Main == xModel->getURL() )
                        nId = SID_JUMPTOMARK;
                }
            }

            if ( nId && pAct->GetDispatcher()->HasSlot_Impl( nId ) )
                xDisp = new SfxOfficeDispatch( pAct->GetBindings(), pAct->GetDispatcher(), nId, aURL) ;
        }
    }

    return xDisp;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XDispatchProvider
//________________________________________________________________________________________________________

SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL SfxBaseController::queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescripts ) throw( ::com::sun::star::uno::RuntimeException )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    sal_Int32 nCount = seqDescripts.getLength();
    SEQUENCE< REFERENCE< XDISPATCH > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL  ,
                                        seqDescripts[i].FrameName   ,
                                        seqDescripts[i].SearchFlags );
    }

    return lDispatcher;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    REFERENCE < XCONTROLLER > xTmp( this );
    m_pData->m_bDisposing = sal_True ;

    EVENTOBJECT aObject ;
    aObject.Source = (XCONTROLLER*)this ;
    m_pData->m_aListenerContainer.disposeAndClear( aObject ) ;

    if ( m_pData->m_pController && m_pData->m_pController->getFrame().is() )
        m_pData->m_pController->getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;

    if ( m_pData->m_pViewShell )
    {
        m_pData->m_pViewShell->DiscardClients_Impl();
        SfxViewFrame* pFrame = m_pData->m_pViewShell->GetViewFrame() ;
        m_pData->m_pViewShell->pImp->bControllerSet = sal_False ;
        if ( pFrame )
        {
            EVENTOBJECT aObject;
            aObject.Source = (OWEAKOBJECT*)this ;

            // Bei Reload hat die alte ViewShell keinen Frame!
            SfxObjectShell* pDoc = pFrame->GetObjectShell() ;
            REFERENCE< XMODEL > xModel = pDoc->GetModel();
            REFERENCE < ::com::sun::star::util::XCloseable > xCloseable( xModel, com::sun::star::uno::UNO_QUERY );
            if ( xModel.is() )
            {
                xModel->disconnectController( this );
                if ( xCloseable.is() )
                    xCloseable->removeCloseListener( m_pData->m_xCloseListener );
            }

            REFERENCE < XFRAME > aXFrame;
            attachFrame( aXFrame );

            m_pData->m_xListener->disposing( aObject );
            SfxViewShell *pShell = m_pData->m_pViewShell;
            m_pData->m_pViewShell = NULL;
            if ( pFrame->GetViewShell() == pShell )
            {
                pFrame->GetBindings().ENTERREGISTRATIONS();
                pFrame->GetFrame()->SetFrameInterface_Impl(  aXFrame );
                pFrame->GetFrame()->DoClose_Impl();
            }
        }
    }
}

void SfxBaseController::TakeOwnerShip_Impl()
{
    if ( m_pData->m_pViewShell && ( !m_pData->m_pViewShell->GetWindow() || !m_pData->m_pViewShell->GetWindow()->IsReallyVisible() ) )
        // ignore OwnerShip for model in case of visible frame
        m_pData->m_pViewShell->TakeOwnerShip_Impl();
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::addEventListener( const REFERENCE< XEVENTLISTENER >& aListener ) throw( ::com::sun::star::uno::RuntimeException )
{
    m_pData->m_aListenerContainer.addInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::removeEventListener( const REFERENCE< XEVENTLISTENER >& aListener ) throw( ::com::sun::star::uno::RuntimeException )
{
    m_pData->m_aListenerContainer.removeInterface( ::getCppuType((const REFERENCE< XEVENTLISTENER >*)0), aListener );
}

void SfxBaseController::ReleaseShell_Impl()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
    {
        SfxObjectShell* pDoc = m_pData->m_pViewShell->GetObjectShell() ;
        REFERENCE< XMODEL > xModel = pDoc->GetModel();
        if ( xModel.is() )
            xModel->disconnectController( this );
        m_pData->m_pViewShell = 0;

        REFERENCE < XFRAME > aXFrame;
        attachFrame( aXFrame );
    }
}

SfxViewShell* SfxBaseController::GetViewShell_Impl() const
{
    return m_pData->m_pViewShell;
}

::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > SAL_CALL SfxBaseController::getStatusIndicator(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell && !m_pData->m_xIndicator.is() )
        m_pData->m_xIndicator = new SfxStatusIndicator( this, m_pData->m_pViewShell->GetViewFrame()->GetFrame()->GetWorkWindow_Impl() );
    return m_pData->m_xIndicator;
}

void SAL_CALL SfxBaseController::registerContextMenuInterceptor( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION )

{
    m_pData->m_aInterceptorContainer.addInterface( xInterceptor );

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->AddContextMenuInterceptor_Impl( xInterceptor );
}

void SAL_CALL SfxBaseController::releaseContextMenuInterceptor( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor ) throw( RUNTIMEEXCEPTION )

{
    m_pData->m_aInterceptorContainer.removeInterface( xInterceptor );

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->RemoveContextMenuInterceptor_Impl( xInterceptor );
}

void SAL_CALL SfxBaseController::addKeyHandler( const ::com::sun::star::uno::Reference< XKEYHANDLER >& xHandler ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !m_pData->m_bHasKeyListeners )
        m_pData->m_bHasKeyListeners = sal_True;
    m_pData->m_aListenerContainer.addInterface( ::getCppuType((const REFERENCE< XKEYHANDLER >*)0), xHandler );
}

void SAL_CALL SfxBaseController::removeKeyHandler( const ::com::sun::star::uno::Reference< XKEYHANDLER >& xHandler ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_pData->m_aListenerContainer.removeInterface( ::getCppuType((const REFERENCE< XKEYHANDLER >*)0), xHandler );
    m_pData->m_bHasKeyListeners = sal_False;
    ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aListenerContainer.getContainer( ::getCppuType( ( const REFERENCE < XKEYHANDLER >*) NULL ) );
    if ( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        if (pIterator.hasMoreElements())
            m_pData->m_bHasKeyListeners = sal_True;
    }
}

void SAL_CALL SfxBaseController::addMouseClickHandler( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !m_pData->m_bHasMouseClickListeners )
        m_pData->m_bHasMouseClickListeners = sal_True;
    m_pData->m_aListenerContainer.addInterface( ::getCppuType((const REFERENCE< XMOUSECLICKHANDLER >*)0), xHandler );
}

void SAL_CALL SfxBaseController::removeMouseClickHandler( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    m_pData->m_aListenerContainer.removeInterface( ::getCppuType((const REFERENCE< XMOUSECLICKHANDLER >*)0), xHandler );
    m_pData->m_bHasMouseClickListeners = sal_False;
    ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aListenerContainer.getContainer( ::getCppuType( ( const REFERENCE < XMOUSECLICKHANDLER >*) NULL ) );
    if ( pContainer )
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        if (pIterator.hasMoreElements())
            m_pData->m_bHasMouseClickListeners = sal_True;
    }
}

void ImplInitKeyEvent( ::com::sun::star::awt::KeyEvent& rEvent, const KeyEvent& rEvt )
{
    rEvent.Modifiers = 0;
    if ( rEvt.GetKeyCode().IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if ( rEvt.GetKeyCode().IsMod1() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if ( rEvt.GetKeyCode().IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;

    rEvent.KeyCode = rEvt.GetKeyCode().GetCode();
    rEvent.KeyChar = (unsigned char)rEvt.GetCharCode();
    rEvent.KeyFunc = rEvt.GetKeyCode().GetFunction();
}

void ImplInitMouseEvent( ::com::sun::star::awt::MouseEvent& rEvent, const MouseEvent& rEvt )
{
    rEvent.Modifiers = 0;
    if ( rEvt.IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if ( rEvt.IsMod1() )
    rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if ( rEvt.IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;

    rEvent.Buttons = 0;
    if ( rEvt.IsLeft() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::LEFT;
    if ( rEvt.IsRight() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::RIGHT;
    if ( rEvt.IsMiddle() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::MIDDLE;

    rEvent.X = rEvt.GetPosPixel().X();
    rEvent.Y = rEvt.GetPosPixel().Y();
    rEvent.ClickCount = rEvt.GetClicks();
    rEvent.PopupTrigger = sal_False;
}

BOOL SfxBaseController::HandleEvent_Impl( NotifyEvent& rEvent )
{
    REFERENCE < ::com::sun::star::uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );
    com::sun::star::lang::EventObject aSource(static_cast< ::cppu::OWeakObject*>(this));
    USHORT nType = rEvent.GetType();
    BOOL bHandled = FALSE;
    if ( nType == EVENT_KEYINPUT || nType == EVENT_KEYUP )
    {
        ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aListenerContainer.getContainer( ::getCppuType( ( const REFERENCE < XKEYHANDLER >*) NULL ) );
        if ( pContainer )
        {
            ::com::sun::star::awt::KeyEvent aEvent;
            ImplInitKeyEvent( aEvent, *rEvent.GetKeyEvent() );
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    if ( nType == EVENT_KEYINPUT )
                        bHandled = ((XKEYHANDLER*)pIterator.next())->keyPressed( aEvent );
                    else
                        bHandled = ((XKEYHANDLER*)pIterator.next())->keyReleased( aEvent );
                }
                catch( RUNTIMEEXCEPTION& )
                {
                    pIterator.remove();
                }
            }
        }
    }
    else if ( nType == EVENT_MOUSEBUTTONUP || nType == EVENT_MOUSEBUTTONDOWN )
    {
        ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aListenerContainer.getContainer( ::getCppuType( ( const REFERENCE < XMOUSECLICKHANDLER >*) NULL ) );
        if ( pContainer )
        {
            ::com::sun::star::awt::MouseEvent aEvent;
            ImplInitMouseEvent( aEvent, *rEvent.GetMouseEvent() );
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    if ( nType == EVENT_MOUSEBUTTONDOWN )
                        bHandled = ((XMOUSECLICKHANDLER*)pIterator.next())->mousePressed( aEvent );
                    else
                        bHandled = ((XMOUSECLICKHANDLER*)pIterator.next())->mouseReleased( aEvent );
                }
                catch( RUNTIMEEXCEPTION& )
                {
                    pIterator.remove();
                }
            }
        }
    }

    return bHandled;
}

BOOL SfxBaseController::HasKeyListeners_Impl()
{
    return m_pData->m_bHasKeyListeners;
}

BOOL SfxBaseController::HasMouseClickListeners_Impl()
{
    return m_pData->m_bHasMouseClickListeners;
}
