/*************************************************************************
 *
 *  $RCSfile: sfxbasecontroller.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:58 $
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

#ifndef _SFX_SFXBASECONTROLLER_HXX_
#include <sfxbasecontroller.hxx>
#endif

//________________________________________________________________________________________________________
//  include of other projects
//________________________________________________________________________________________________________

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
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
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
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


#include <vos/mutex.hxx>
#include <osl/mutex.hxx>

#define OMULTITYPEINTERFACECONTAINERHELPER      ::cppu::OMultiTypeInterfaceContainerHelper
#define XFRAMEACTIONLISTENER                    ::com::sun::star::frame::XFrameActionListener
#define FRAMEACTIONEVENT                        ::com::sun::star::frame::FrameActionEvent
#define EVENTOBJECT                             ::com::sun::star::lang::EventObject
#define OTYPECOLLECTION                         ::cppu::OTypeCollection
#define OIMPLEMENTATIONID                       ::cppu::OImplementationId
#define MUTEXGUARD                              ::osl::MutexGuard
#define UNOQUERY                                ::com::sun::star::uno::UNO_QUERY
#define MAPPING                                 ::com::sun::star::uno::Mapping
#define XINTERFACE                              ::com::sun::star::uno::XInterface

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

//________________________________________________________________________________________________________
//  declaration IMPL_SfxBaseController_DataContainer
//________________________________________________________________________________________________________

struct IMPL_SfxBaseController_DataContainer
{
    REFERENCE < XFRAME >    m_xFrame;
    REFERENCE < XFRAMEACTIONLISTENER >      m_xListener       ;
    OMULTITYPEINTERFACECONTAINERHELPER      m_aListenerContainer    ;
    SfxViewShell*                           m_pViewShell            ;
    SfxBaseController*                      m_pController           ;
    sal_Bool                                m_bDisposing            ;

    IMPL_SfxBaseController_DataContainer(   MUTEX&              aMutex      ,
                                            SfxViewShell*       pViewShell  ,
                                            SfxBaseController*  pController )
            :   m_xListener       ( new IMPL_SfxBaseController_ListenerHelper( aMutex, pController ) )
            ,   m_aListenerContainer    ( aMutex                                                )
            ,   m_pViewShell            ( pViewShell                                            )
            ,   m_pController           ( pController                                           )
            ,   m_bDisposing            ( sal_False                                             )
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
    ::osl::MutexGuard aGuard( m_aMutex );
    if  (
            ( m_pController             !=  NULL                                                    )   &&
            ( aEvent.Frame              ==  m_pController->getFrame()                               )   &&
            ( aEvent.Action             ==  ::com::sun::star::frame::FrameAction_FRAME_ACTIVATED    )   &&
            ( m_pController->GetViewShell_Impl() && m_pController->GetViewShell_Impl()->GetWindow() !=  NULL                                                    )
        )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        m_pController->GetViewShell_Impl()->GetViewFrame()->MakeActive_Impl( FALSE );
    }
}

//________________________________________________________________________________________________________
//  IMPL_SfxBaseController_ListenerHelper -> XEventListener
//________________________________________________________________________________________________________

void SAL_CALL IMPL_SfxBaseController_ListenerHelper::disposing( const EVENTOBJECT& aEvent ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pController )
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
                                               static_cast< XCONTROLLER*        > ( this )  ,
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

void SAL_CALL SfxBaseController::acquire() throw( RUNTIMEEXCEPTION )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire() ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XInterface
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::release() throw( RUNTIMEEXCEPTION )
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
                                                      ::getCppuType(( const REFERENCE< XDISPATCHPROVIDER    >*)NULL ) ) ;
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

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( xTemp.is() )
        xTemp->removeFrameActionListener( m_pData->m_xListener ) ;

    m_pData->m_xFrame = xFrame;

    if ( xFrame.is() )
        xFrame->addFrameActionListener( m_pData->m_xListener ) ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseController::attachModel( const REFERENCE< XMODEL >& xModel ) throw( ::com::sun::star::uno::RuntimeException )
{
    return sal_False ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseController::suspend( sal_Bool bSuspend ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( bSuspend == sal_True )
    {
        ::osl::MutexGuard aMutexGuard( m_aMutex );
        if ( !m_pData->m_pViewShell )
            return sal_True;

        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if ( m_pData->m_pViewShell->PrepareClose() )
        {
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
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pData->m_pViewShell )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pData->m_pViewShell )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pData->m_xFrame;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

REFERENCE< XMODEL > SAL_CALL SfxBaseController::getModel() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pData->m_pViewShell ? m_pData->m_pViewShell->GetObjectShell()->GetModel() : REFERENCE < XMODEL > () ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XDispatchProvider
//________________________________________________________________________________________________________

REFERENCE< XDISPATCH > SAL_CALL SfxBaseController::queryDispatch(   const   UNOURL&             aURL            ,
                                                                    const   OUSTRING&           sTargetFrameName,
                                                                            sal_Int32           eSearchFlags    ) throw( RUNTIMEEXCEPTION )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    REFERENCE< XDISPATCH >  xDisp;
    if ( m_pData->m_pViewShell )
    {
        SfxViewFrame*           pAct    = m_pData->m_pViewShell->GetViewFrame() ;
        if ( !m_pData->m_bDisposing )
        {
            sal_uInt16 nId = 0;
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            if ( sTargetFrameName.compareToAscii( "_beamer" ) == COMPARE_EQUAL )
            {
                SfxViewFrame *pFrame = m_pData->m_pViewShell->GetViewFrame();
                if ( eSearchFlags & ( ::com::sun::star::frame::FrameSearchFlag::CREATE ))
                    pFrame->SetChildWindow( SID_PARTWIN, TRUE );
                SfxChildWindow* pChildWin = pFrame->GetChildWindow( SID_PARTWIN );
                REFERENCE < XFRAME > xFrame( pChildWin->GetFrame() );
                if ( xFrame.is() )
                {
                    xFrame->setName( sTargetFrameName );

                }

                REFERENCE < XDISPATCHPROVIDER > xProv( xFrame, ::com::sun::star::uno::UNO_QUERY );
                if ( xProv.is() )
                    return xProv->queryDispatch( aURL, sTargetFrameName, ::com::sun::star::frame::FrameSearchFlag::SELF );
            }

            if ( aURL.Protocol.compareToAscii( ".uno:" ) == COMPARE_EQUAL )
            {
                SfxSlotPool& rPool = SFX_APP()->GetSlotPool( pAct );
                const SfxSlot* pSlot = rPool.GetUnoSlot( aURL.Path );
                if ( pSlot )
                    nId = pSlot->GetSlotId();
            }
            else if ( aURL.Protocol.compareToAscii( "slot:" ) == COMPARE_EQUAL )
            {
                nId = aURL.Path.toInt32();
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
    return SEQUENCE< REFERENCE< XDISPATCH > >() ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    REFERENCE < XCONTROLLER > xTmp( this );
    m_pData->m_bDisposing = sal_True ;

    EVENTOBJECT aObject ;
    aObject.Source = (XCONTROLLER*)this ;
    m_pData->m_aListenerContainer.disposeAndClear( aObject ) ;

    if ( m_pData->m_pController )
        m_pData->m_pController->getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;

    if ( m_pData->m_pViewShell )
    {
        SfxViewFrame* pFrame = m_pData->m_pViewShell->GetViewFrame() ;
        m_pData->m_pViewShell->pImp->bControllerSet = sal_False ;
        if ( pFrame )
        {
            EVENTOBJECT aObject;
            aObject.Source = (OWEAKOBJECT*)this ;

            // Bei Reload hat die alte ViewShell keinen Frame!
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            SfxObjectShell* pDoc = pFrame->GetObjectShell() ;
            REFERENCE< XMODEL > xModel = pDoc->GetModel();
            if ( xModel.is() )
            {
                REFERENCE< XEVENTLISTENER > xRef( xModel, UNOQUERY );
                if ( xRef.is() )
                    xRef->disposing( aObject );
            }

            m_pData->m_xListener->disposing( aObject );
            SfxViewShell *pShell = m_pData->m_pViewShell;
            m_pData->m_pViewShell = NULL;
            if ( pFrame->GetViewShell() == pShell )
            {
                pFrame->GetFrame()->SetFrameInterface_Impl(  REFERENCE < XFRAME >() );
                pFrame->GetFrame()->DoClose();
            }
        }
    }
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
    ::osl::MutexGuard aGuard( m_aMutex );
    m_pData->m_pViewShell = 0;
}

SfxViewShell* SfxBaseController::GetViewShell_Impl() const
{
    return m_pData->m_pViewShell;
}
