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

#include <time.h>
#include <sfx2/sfxbasecontroller.hxx>

#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/document/XCmisDocument.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XBorderResizeListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <basic/sbstar.hxx>
#include <uno/mapping.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/userinputinterception.hxx>

#include <viewimp.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxresid.hxx>
#include <workwin.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/infobar.hxx>

#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/convert.hxx>
#include <framework/titlehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/msgbox.hxx>

#include <boost/unordered_map.hpp>

#include <sfx2/event.hxx>
#include "sfx2/viewfac.hxx"

#define TIMEOUT_START_RESCHEDULE    10L /* 10th s */

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::lang::DisposedException;
using ::com::sun::star::awt::XWindow;
using ::com::sun::star::frame::XController;
using ::com::sun::star::frame::XDispatchProvider;
using ::com::sun::star::document::XViewDataSupplier;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XFrameActionListener;
using ::com::sun::star::util::XCloseListener;
using ::com::sun::star::task::XStatusIndicator;
using ::com::sun::star::frame::XTitle;

struct GroupIDToCommandGroup
{
    sal_Int16   nGroupID;
    sal_Int16   nCommandGroup;
};

// Please update when a new command group is added
const sal_Int16 MAX_COMMANDGROUP = frame::CommandGroup::CONTROLS;

static bool                 bGroupIDMapInitialized = false;
static const GroupIDToCommandGroup    GroupIDCommandGroupMap[] =
{
    { GID_INTERN        ,   frame::CommandGroup::INTERNAL       },
    { GID_APPLICATION   ,   frame::CommandGroup::APPLICATION    },
    { GID_DOCUMENT      ,   frame::CommandGroup::DOCUMENT       },
    { GID_VIEW          ,   frame::CommandGroup::VIEW           },
    { GID_EDIT          ,   frame::CommandGroup::EDIT           },
    { GID_MACRO         ,   frame::CommandGroup::MACRO          },
    { GID_OPTIONS       ,   frame::CommandGroup::OPTIONS        },
    { GID_MATH          ,   frame::CommandGroup::MATH           },
    { GID_NAVIGATOR     ,   frame::CommandGroup::NAVIGATOR      },
    { GID_INSERT        ,   frame::CommandGroup::INSERT         },
    { GID_FORMAT        ,   frame::CommandGroup::FORMAT         },
    { GID_TEMPLATE      ,   frame::CommandGroup::TEMPLATE       },
    { GID_TEXT          ,   frame::CommandGroup::TEXT           },
    { GID_FRAME         ,   frame::CommandGroup::FRAME          },
    { GID_GRAPHIC       ,   frame::CommandGroup::GRAPHIC        },
    { GID_TABLE         ,   frame::CommandGroup::TABLE          },
    { GID_ENUMERATION   ,   frame::CommandGroup::ENUMERATION    },
    { GID_DATA          ,   frame::CommandGroup::DATA           },
    { GID_SPECIAL       ,   frame::CommandGroup::SPECIAL        },
    { GID_IMAGE         ,   frame::CommandGroup::IMAGE          },
    { GID_CHART         ,   frame::CommandGroup::CHART          },
    { GID_EXPLORER      ,   frame::CommandGroup::EXPLORER       },
    { GID_CONNECTOR     ,   frame::CommandGroup::CONNECTOR      },
    { GID_MODIFY        ,   frame::CommandGroup::MODIFY         },
    { GID_DRAWING       ,   frame::CommandGroup::DRAWING        },
    { GID_CONTROLS      ,   frame::CommandGroup::CONTROLS       },
    { 0                 ,   0                                                   }
};

typedef boost::unordered_map< sal_Int16, sal_Int16 > GroupHashMap;


sal_Int16 MapGroupIDToCommandGroup( sal_Int16 nGroupID )
{
    static GroupHashMap mHashMap;

    if ( !bGroupIDMapInitialized )
    {
        sal_Int32 i = 0;
        while ( GroupIDCommandGroupMap[i].nGroupID != 0 )
        {
            mHashMap.insert( GroupHashMap::value_type(
                GroupIDCommandGroupMap[i].nGroupID,
                GroupIDCommandGroupMap[i].nCommandGroup ));
            ++i;
        }
        bGroupIDMapInitialized = true;
    }

    GroupHashMap::const_iterator pIter = mHashMap.find( nGroupID );
    if ( pIter != mHashMap.end() )
        return pIter->second;
    else
        return frame::CommandGroup::INTERNAL;
}

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

class SfxStatusIndicator : public ::cppu::WeakImplHelper2< task::XStatusIndicator, lang::XEventListener >
{
friend class SfxBaseController;
    Reference < XController > xOwner;
    Reference < task::XStatusIndicator > xProgress;
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
                                Reference< lang::XComponent > xComponent(
                                    (static_cast< ::cppu::OWeakObject* >(pController)), uno::UNO_QUERY );
                                if (xComponent.is())
                                    xComponent->addEventListener(this);
                                --m_refCount;
                            }

    virtual void SAL_CALL   start(const OUString& aText, sal_Int32 nRange) throw(RuntimeException);
    virtual void SAL_CALL   end(void) throw(RuntimeException);
    virtual void SAL_CALL   setText(const OUString& aText) throw(RuntimeException);
    virtual void SAL_CALL   setValue(sal_Int32 nValue) throw(RuntimeException);
    virtual void SAL_CALL   reset() throw(RuntimeException);

    virtual void SAL_CALL   disposing( const lang::EventObject& Source ) throw(RuntimeException);
};

void SAL_CALL SfxStatusIndicator::start(const OUString& aText, sal_Int32 nRange) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( xOwner.is() )
    {
        _nRange = nRange;
        _nValue = 0;

        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->start( aText, nRange );

        _nStartTime = Get10ThSec();
        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::end(void) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( xOwner.is() )
    {
        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->end();

        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::setText(const OUString& aText) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( xOwner.is() )
    {
        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->setText( aText );

        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::setValue( sal_Int32 nValue ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( xOwner.is() )
    {
        _nValue = nValue;

        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->setValue( nValue );

        bool bReschedule = (( Get10ThSec() - _nStartTime ) > TIMEOUT_START_RESCHEDULE );
        if ( bReschedule )
            reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::reset() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( xOwner.is() )
    {
        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->reset();

        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::disposing( const lang::EventObject& /*Source*/ ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    xOwner = 0;
    xProgress.clear();
}

//________________________________________________________________________________________________________
//  declaration IMPL_SfxBaseController_ListenerHelper
//________________________________________________________________________________________________________

class IMPL_SfxBaseController_ListenerHelper : public ::cppu::WeakImplHelper1< frame::XFrameActionListener >
{
public:
    IMPL_SfxBaseController_ListenerHelper(  SfxBaseController*  pController ) ;
    virtual ~IMPL_SfxBaseController_ListenerHelper() ;
    virtual void SAL_CALL frameAction( const frame::FrameActionEvent& aEvent ) throw (RuntimeException) ;
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) throw (RuntimeException) ;

private:

    SfxBaseController*      m_pController   ;

} ; // class IMPL_SfxBaseController_ListenerContainer

class IMPL_SfxBaseController_CloseListenerHelper : public ::cppu::WeakImplHelper1< util::XCloseListener >
{
public:
    IMPL_SfxBaseController_CloseListenerHelper( SfxBaseController*  pController ) ;
    virtual ~IMPL_SfxBaseController_CloseListenerHelper() ;
    virtual void SAL_CALL queryClosing( const lang::EventObject& aEvent, sal_Bool bDeliverOwnership )
        throw (RuntimeException, util::CloseVetoException) ;
    virtual void SAL_CALL notifyClosing( const lang::EventObject& aEvent ) throw (RuntimeException) ;
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) throw (RuntimeException) ;

private:

    SfxBaseController*      m_pController;

} ; // class IMPL_SfxBaseController_ListenerContainer

IMPL_SfxBaseController_CloseListenerHelper::IMPL_SfxBaseController_CloseListenerHelper( SfxBaseController*  pController )
        : m_pController ( pController   )
{
}

IMPL_SfxBaseController_CloseListenerHelper::~IMPL_SfxBaseController_CloseListenerHelper()
{
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::disposing( const lang::EventObject& /*aEvent*/ ) throw( RuntimeException )
{
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::queryClosing( const lang::EventObject& aEvent, sal_Bool bDeliverOwnership )
    throw (RuntimeException, util::CloseVetoException)
{
    SolarMutexGuard aGuard;
    SfxViewShell* pShell = m_pController->GetViewShell_Impl();
    if  ( m_pController !=  NULL &&  pShell )
    {
        sal_Bool bCanClose = (sal_Bool) pShell->PrepareClose( sal_False );
        if ( !bCanClose )
        {
            if ( bDeliverOwnership && ( !pShell->GetWindow() || !pShell->GetWindow()->IsReallyVisible() ) )
            {
                // ignore Ownership in case of visible frame (will be closed by user)
                Reference < frame::XModel > xModel( aEvent.Source, uno::UNO_QUERY );
                if ( xModel.is() )
                    pShell->TakeOwnership_Impl();
                else
                    pShell->TakeFrameOwnership_Impl();
            }

            throw util::CloseVetoException("Controller disagree ...",static_cast< ::cppu::OWeakObject*>(this));
        }
    }
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::notifyClosing( const lang::EventObject& /*aEvent*/ ) throw (RuntimeException)
{
}

//________________________________________________________________________________________________________
//  declaration IMPL_SfxBaseController_DataContainer
//________________________________________________________________________________________________________

struct IMPL_SfxBaseController_DataContainer
{
    Reference< XFrame >                     m_xFrame                ;
    Reference< XFrameActionListener >       m_xListener             ;
    Reference< XCloseListener >             m_xCloseListener        ;
    ::sfx2::UserInputInterception           m_aUserInputInterception;
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aListenerContainer    ;
    ::cppu::OInterfaceContainerHelper               m_aInterceptorContainer ;
    Reference< XStatusIndicator >           m_xIndicator            ;
    SfxViewShell*                           m_pViewShell            ;
    SfxBaseController*                      m_pController           ;
    bool                                    m_bDisposing            ;
    sal_Bool                                m_bSuspendState         ;
    Reference< XTitle >                     m_xTitleHelper          ;
    Sequence< PropertyValue >               m_aCreationArgs         ;

    IMPL_SfxBaseController_DataContainer(   ::osl::Mutex&              aMutex      ,
                                            SfxViewShell*       pViewShell  ,
                                            SfxBaseController*  pController )
            :   m_xListener                     ( new IMPL_SfxBaseController_ListenerHelper( pController ) )
            ,   m_xCloseListener                ( new IMPL_SfxBaseController_CloseListenerHelper( pController ) )
            ,   m_aUserInputInterception        ( *pController, aMutex                                  )
            ,   m_aListenerContainer            ( aMutex                                                )
            ,   m_aInterceptorContainer         ( aMutex                                                )
            ,   m_pViewShell            ( pViewShell                                            )
            ,   m_pController           ( pController                                           )
            ,   m_bDisposing            ( false                                                 )
            ,   m_bSuspendState                 ( sal_False                                              )
    {
    }

} ; // struct IMPL_SfxBaseController_DataContainer

//________________________________________________________________________________________________________
//  IMPL_SfxBaseController_ListenerHelper constructor
//________________________________________________________________________________________________________

IMPL_SfxBaseController_ListenerHelper::IMPL_SfxBaseController_ListenerHelper(   SfxBaseController*  pController )
        : m_pController ( pController   )
{
}

//________________________________________________________________________________________________________
//  IMPL_SfxBaseController_ListenerHelper destructor
//________________________________________________________________________________________________________

IMPL_SfxBaseController_ListenerHelper::~IMPL_SfxBaseController_ListenerHelper()
{
}

void SAL_CALL IMPL_SfxBaseController_ListenerHelper::frameAction( const frame::FrameActionEvent& aEvent ) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if  (
            ( m_pController !=  NULL ) &&
            ( aEvent.Frame  ==  m_pController->getFrame() ) &&
            ( m_pController->GetViewShell_Impl() && m_pController->GetViewShell_Impl()->GetWindow() !=  NULL                                                    )
        )
    {
        if ( aEvent.Action == frame::FrameAction_FRAME_UI_ACTIVATED )
        {
            if ( !m_pController->GetViewShell_Impl()->GetUIActiveIPClient_Impl() )
                m_pController->GetViewShell_Impl()->GetViewFrame()->MakeActive_Impl( sal_False );
        }
        else if ( aEvent.Action == frame::FrameAction_CONTEXT_CHANGED )
        {
            m_pController->GetViewShell_Impl()->GetViewFrame()->GetBindings().ContextChanged_Impl();
        }
    }
}

//________________________________________________________________________________________________________
//  IMPL_SfxBaseController_ListenerHelper -> XEventListener
//________________________________________________________________________________________________________

void SAL_CALL IMPL_SfxBaseController_ListenerHelper::disposing( const lang::EventObject& /*aEvent*/ ) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( m_pController && m_pController->getFrame().is() )
        m_pController->getFrame()->removeFrameActionListener( this ) ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> constructor
//________________________________________________________________________________________________________
DBG_NAME(sfx2_SfxBaseController)
SfxBaseController::SfxBaseController( SfxViewShell* pViewShell )
    :   m_pData ( new IMPL_SfxBaseController_DataContainer( m_aMutex, pViewShell, this ))
{
    DBG_CTOR(sfx2_SfxBaseController,NULL);
    m_pData->m_pViewShell->SetController( this );
}

//________________________________________________________________________________________________________
//  SfxBaseController -> destructor
//________________________________________________________________________________________________________

SfxBaseController::~SfxBaseController()
{
    DBG_DTOR(sfx2_SfxBaseController,NULL);
    delete m_pData;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController2
//________________________________________________________________________________________________________

Reference< XWindow > SAL_CALL SfxBaseController::getComponentWindow() throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( !m_pData->m_pViewShell )
        throw DisposedException();

    return Reference< XWindow >( GetViewFrame_Impl().GetFrame().GetWindow().GetComponentInterface(), UNO_QUERY_THROW );
}

OUString SAL_CALL SfxBaseController::getViewControllerName() throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( !m_pData->m_pViewShell || !m_pData->m_pViewShell->GetObjectShell() )
        throw DisposedException();

    const SfxObjectFactory& rDocFac( m_pData->m_pViewShell->GetObjectShell()->GetFactory() );
    sal_uInt16 nViewNo = rDocFac.GetViewNo_Impl( GetViewFrame_Impl().GetCurViewId(), rDocFac.GetViewFactoryCount() );
    OSL_ENSURE( nViewNo < rDocFac.GetViewFactoryCount(), "SfxBaseController::getViewControllerName: view ID not found in view factories!" );

    OUString sViewName;
    if ( nViewNo < rDocFac.GetViewFactoryCount() )
        sViewName = rDocFac.GetViewFactory( nViewNo ).GetAPIViewName();

    return sViewName;
}

Sequence< PropertyValue > SAL_CALL SfxBaseController::getCreationArguments() throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( !m_pData->m_pViewShell || !m_pData->m_pViewShell->GetObjectShell() )
        throw DisposedException();

    return m_pData->m_aCreationArgs;
}

void SfxBaseController::SetCreationArguments_Impl( const Sequence< PropertyValue >& i_rCreationArgs )
{
    OSL_ENSURE( m_pData->m_aCreationArgs.getLength() == 0, "SfxBaseController::SetCreationArguments_Impl: not intended to be called twice!" );
    m_pData->m_aCreationArgs = i_rCreationArgs;
}

SfxViewFrame& SfxBaseController::GetViewFrame_Impl() const
{
    ENSURE_OR_THROW( m_pData->m_pViewShell, "not to be called without a view shell" );
    SfxViewFrame* pActFrame = m_pData->m_pViewShell->GetFrame();
    ENSURE_OR_THROW( pActFrame, "a view shell without a view frame is pretty pathological" );
    return *pActFrame;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController2 -> XController
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::attachFrame( const Reference< frame::XFrame >& xFrame ) throw( RuntimeException )
{
    Reference< frame::XFrame > xTemp( getFrame() ) ;

    SolarMutexGuard aGuard;
    if ( xTemp.is() )
    {
        xTemp->removeFrameActionListener( m_pData->m_xListener ) ;
        Reference < util::XCloseBroadcaster > xCloseable( xTemp, uno::UNO_QUERY );
        if ( xCloseable.is() )
            xCloseable->removeCloseListener( m_pData->m_xCloseListener );
    }

    m_pData->m_xFrame = xFrame;

    if ( xFrame.is() )
    {
        xFrame->addFrameActionListener( m_pData->m_xListener ) ;
        Reference < util::XCloseBroadcaster > xCloseable( xFrame, uno::UNO_QUERY );
        if ( xCloseable.is() )
            xCloseable->addCloseListener( m_pData->m_xCloseListener );

        if ( m_pData->m_pViewShell )
        {
            ConnectSfxFrame_Impl( E_CONNECT );
            ShowInfoBars( );

            // attaching the frame to the controller is the last step in the creation of a new view, so notify this
            SfxViewEventHint aHint( SFX_EVENT_VIEWCREATED, GlobalEventConfig::GetEventName( STR_EVENT_VIEWCREATED ), m_pData->m_pViewShell->GetObjectShell(), Reference< frame::XController2 >( this ) );
            SFX_APP()->NotifyEvent( aHint );
        }
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseController::attachModel( const Reference< frame::XModel >& xModel ) throw( RuntimeException )
{
    if ( m_pData->m_pViewShell && xModel.is() && xModel != m_pData->m_pViewShell->GetObjectShell()->GetModel() )
    {
        // don't allow to reattach a model!
        OSL_FAIL("Can't reattach model!");
        return sal_False;
    }

    Reference < util::XCloseBroadcaster > xCloseable( xModel, uno::UNO_QUERY );
    if ( xCloseable.is() )
        xCloseable->addCloseListener( m_pData->m_xCloseListener );
    return sal_True;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseController::suspend( sal_Bool bSuspend ) throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    // ignore dublicate calls, which doesnt change anything real
    if (bSuspend == m_pData->m_bSuspendState)
       return sal_True;

    if ( bSuspend == sal_True )
    {
        if ( !m_pData->m_pViewShell )
        {
            m_pData->m_bSuspendState = sal_True;
            return sal_True;
        }

        if ( !m_pData->m_pViewShell->PrepareClose() )
            return sal_False;

        if ( getFrame().is() )
            getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;
        SfxViewFrame* pActFrame = m_pData->m_pViewShell->GetFrame() ;

        // More Views on the same document?
        SfxObjectShell* pDocShell   =   m_pData->m_pViewShell->GetObjectShell() ;
        bool            bOther      =   false                                   ;

        for ( const SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell ); !bOther && pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell ) )
            bOther = (pFrame != pActFrame);

        sal_Bool bRet = bOther || pDocShell->PrepareClose();
        if ( bRet )
        {
            ConnectSfxFrame_Impl( E_DISCONNECT );
            m_pData->m_bSuspendState = sal_True;
        }

        return bRet;
    }
    else
    {
        if ( getFrame().is() )
            getFrame()->addFrameActionListener( m_pData->m_xListener ) ;

        if ( m_pData->m_pViewShell )
        {
            ConnectSfxFrame_Impl( E_RECONNECT );
        }

        m_pData->m_bSuspendState = sal_False;
        return sal_True ;
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

uno::Any SfxBaseController::getViewData() throw( RuntimeException )
{
    uno::Any         aAny;
    OUString sData;
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        m_pData->m_pViewShell->WriteUserData( sData ) ;
        aAny <<= sData ;
    }

    return aAny ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::restoreViewData( const uno::Any& aValue ) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        OUString sData;
        aValue >>= sData ;
        m_pData->m_pViewShell->ReadUserData( sData ) ;
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

Reference< frame::XFrame > SAL_CALL SfxBaseController::getFrame() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return m_pData->m_xFrame;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XController
//________________________________________________________________________________________________________

Reference< frame::XModel > SAL_CALL SfxBaseController::getModel() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return m_pData->m_pViewShell ? m_pData->m_pViewShell->GetObjectShell()->GetModel() : Reference < frame::XModel > () ;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XDispatchProvider
//________________________________________________________________________________________________________

Reference< frame::XDispatch > SAL_CALL SfxBaseController::queryDispatch(   const   util::URL&             aURL            ,
                                                                    const   OUString&            sTargetFrameName,
                                                                            sal_Int32           eSearchFlags    ) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< frame::XDispatch >  xDisp;
    if ( m_pData->m_pViewShell )
    {
        SfxViewFrame*           pAct    = m_pData->m_pViewShell->GetViewFrame() ;
        if ( !m_pData->m_bDisposing )
        {
            if ( sTargetFrameName == "_beamer" )
            {
                SfxViewFrame *pFrame = m_pData->m_pViewShell->GetViewFrame();
                if ( eSearchFlags & ( frame::FrameSearchFlag::CREATE ))
                    pFrame->SetChildWindow( SID_BROWSER, sal_True );
                SfxChildWindow* pChildWin = pFrame->GetChildWindow( SID_BROWSER );
                Reference < frame::XFrame > xFrame;
                if ( pChildWin )
                    xFrame = ( pChildWin->GetFrame() );
                if ( xFrame.is() )
                    xFrame->setName( sTargetFrameName );

                Reference< XDispatchProvider > xProv( xFrame, uno::UNO_QUERY );
                if ( xProv.is() )
                    return xProv->queryDispatch( aURL, sTargetFrameName, frame::FrameSearchFlag::SELF );
            }

            if ( aURL.Protocol == ".uno:" )
            {
                OUString aMasterCommand = SfxOfficeDispatch::GetMasterUnoCommand( aURL );
                sal_Bool      bMasterCommand( !aMasterCommand.isEmpty() );

                pAct = m_pData->m_pViewShell->GetViewFrame() ;
                SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( pAct );

                const SfxSlot* pSlot( 0 );
                if ( bMasterCommand )
                    pSlot = rSlotPool.GetUnoSlot( aMasterCommand );
                else
                    pSlot = rSlotPool.GetUnoSlot( aURL.Path );
                if ( pSlot && ( !pAct->GetFrame().IsInPlace() || !pSlot->IsMode( SFX_SLOT_CONTAINER ) ) )
                    return pAct->GetBindings().GetDispatch( pSlot, aURL, bMasterCommand );
                else
                {
                    // try to find parent SfxViewFrame
                    Reference< frame::XFrame > xParentFrame;
                    Reference< frame::XFrame > xOwnFrame = pAct->GetFrame().GetFrameInterface();
                    if ( xOwnFrame.is() )
                        xParentFrame = Reference< frame::XFrame >( xOwnFrame->getCreator(), uno::UNO_QUERY );

                    if ( xParentFrame.is() )
                    {
                        // TODO/LATER: in future probably SfxViewFrame hirarchy should be the same as XFrame hirarchy
                        // SfxViewFrame* pParentFrame = pAct->GetParentViewFrame();

                        // search the related SfxViewFrame
                        SfxViewFrame* pParentFrame = NULL;
                        for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst();
                                pFrame;
                                pFrame = SfxViewFrame::GetNext( *pFrame ) )
                        {
                            if ( pFrame->GetFrame().GetFrameInterface() == xParentFrame )
                            {
                                pParentFrame = pFrame;
                                break;
                            }
                        }

                        if ( pParentFrame )
                        {
                            SfxSlotPool& rFrameSlotPool = SfxSlotPool::GetSlotPool( pParentFrame );
                            const SfxSlot* pSlot2( 0 );
                            if ( bMasterCommand )
                                pSlot2 = rFrameSlotPool.GetUnoSlot( aMasterCommand );
                            else
                                pSlot2 = rFrameSlotPool.GetUnoSlot( aURL.Path );

                            if ( pSlot2 )
                                return pParentFrame->GetBindings().GetDispatch( pSlot2, aURL, bMasterCommand );
                        }
                    }
                }
            }
            else if ( aURL.Protocol == "slot:" )
            {
                sal_uInt16 nId = (sal_uInt16) aURL.Path.toInt32();

                pAct = m_pData->m_pViewShell->GetViewFrame() ;
                if (nId >= SID_VERB_START && nId <= SID_VERB_END)
                {
                    const SfxSlot* pSlot = m_pData->m_pViewShell->GetVerbSlot_Impl(nId);
                    if ( pSlot )
                        return pAct->GetBindings().GetDispatch( pSlot, aURL, sal_False );
                }

                SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( pAct );
                const SfxSlot* pSlot = rSlotPool.GetSlot( nId );
                if ( pSlot && ( !pAct->GetFrame().IsInPlace() || !pSlot->IsMode( SFX_SLOT_CONTAINER ) ) )
                    return pAct->GetBindings().GetDispatch( pSlot, aURL, sal_False );
                else
                {
                    // try to find parent SfxViewFrame
                    Reference< frame::XFrame > xParentFrame;
                    Reference< frame::XFrame > xOwnFrame = pAct->GetFrame().GetFrameInterface();
                    if ( xOwnFrame.is() )
                        xParentFrame = Reference< frame::XFrame >( xOwnFrame->getCreator(), uno::UNO_QUERY );

                    if ( xParentFrame.is() )
                    {
                        // TODO/LATER: in future probably SfxViewFrame hirarchy should be the same as XFrame hirarchy
                        // SfxViewFrame* pParentFrame = pAct->GetParentViewFrame();

                        // search the related SfxViewFrame
                        SfxViewFrame* pParentFrame = NULL;
                        for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst();
                                pFrame;
                                pFrame = SfxViewFrame::GetNext( *pFrame ) )
                        {
                            if ( pFrame->GetFrame().GetFrameInterface() == xParentFrame )
                            {
                                pParentFrame = pFrame;
                                break;
                            }
                        }

                        if ( pParentFrame )
                        {
                            SfxSlotPool& rSlotPool2 = SfxSlotPool::GetSlotPool( pParentFrame );
                            const SfxSlot* pSlot2 = rSlotPool2.GetUnoSlot( aURL.Path );
                            if ( pSlot2 )
                                return pParentFrame->GetBindings().GetDispatch( pSlot2, aURL, sal_False );
                        }
                    }
                }
            }
            else if( sTargetFrameName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("_self")) || sTargetFrameName.isEmpty() )
            {
                // check for already loaded URL ... but with additional jumpmark!
                Reference< frame::XModel > xModel = getModel();
                if( xModel.is() && !aURL.Mark.isEmpty() )
                {
                    SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( pAct );
                    const SfxSlot* pSlot = rSlotPool.GetSlot( SID_JUMPTOMARK );
                    if( !aURL.Main.isEmpty() && aURL.Main == xModel->getURL() && pSlot )
                        return Reference< frame::XDispatch >( new SfxOfficeDispatch( pAct->GetBindings(), pAct->GetDispatcher(), pSlot, aURL) );
                }
            }
        }
    }

    return xDisp;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XDispatchProvider
//________________________________________________________________________________________________________

uno::Sequence< Reference< frame::XDispatch > > SAL_CALL SfxBaseController::queryDispatches( const uno::Sequence< frame::DispatchDescriptor >& seqDescripts ) throw( RuntimeException )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    sal_Int32 nCount = seqDescripts.getLength();
    uno::Sequence< Reference< frame::XDispatch > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL  ,
                                        seqDescripts[i].FrameName   ,
                                        seqDescripts[i].SearchFlags );
    }

    return lDispatcher;
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XControllerBorder
//________________________________________________________________________________________________________

frame::BorderWidths SAL_CALL SfxBaseController::getBorder()
    throw ( RuntimeException )
{
    frame::BorderWidths aResult;

    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        SvBorder aBorder = m_pData->m_pViewShell->GetBorderPixel();
        aResult.Left = aBorder.Left();
        aResult.Top = aBorder.Top();
        aResult.Right = aBorder.Right();
        aResult.Bottom = aBorder.Bottom();
    }

    return aResult;
}

void SAL_CALL SfxBaseController::addBorderResizeListener( const Reference< frame::XBorderResizeListener >& xListener )
    throw ( RuntimeException )
{
    m_pData->m_aListenerContainer.addInterface( ::getCppuType((const Reference< frame::XBorderResizeListener >*)0),
                                                xListener );
}

void SAL_CALL SfxBaseController::removeBorderResizeListener( const Reference< frame::XBorderResizeListener >& xListener )
    throw ( RuntimeException )
{
    m_pData->m_aListenerContainer.removeInterface( ::getCppuType((const Reference< frame::XBorderResizeListener >*)0),
                                                xListener );
}

awt::Rectangle SAL_CALL SfxBaseController::queryBorderedArea( const awt::Rectangle& aPreliminaryRectangle )
    throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        Rectangle aTmpRect = VCLRectangle( aPreliminaryRectangle );
        m_pData->m_pViewShell->QueryObjAreaPixel( aTmpRect );
        return AWTRectangle( aTmpRect );
    }

    return aPreliminaryRectangle;
}

void SfxBaseController::BorderWidthsChanged_Impl()
{
       ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aListenerContainer.getContainer(
                        ::getCppuType( ( const Reference< frame::XBorderResizeListener >*) NULL ) );
    if ( pContainer )
    {
        frame::BorderWidths aBWidths = getBorder();
        Reference< uno::XInterface > xThis( static_cast< ::cppu::OWeakObject* >(this), uno::UNO_QUERY );

        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((frame::XBorderResizeListener*)pIterator.next())->borderWidthsChanged( xThis, aBWidths );
            }
            catch (const RuntimeException&)
            {
                pIterator.remove();
            }
        }
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::dispose() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XController > xTmp( this );
    m_pData->m_bDisposing = true ;

    lang::EventObject aEventObject;
    aEventObject.Source = *this ;
    m_pData->m_aListenerContainer.disposeAndClear( aEventObject ) ;

    if ( m_pData->m_pController && m_pData->m_pController->getFrame().is() )
        m_pData->m_pController->getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;

    if ( m_pData->m_pViewShell )
    {
        SfxViewFrame* pFrame = m_pData->m_pViewShell->GetViewFrame() ;
        if ( pFrame && pFrame->GetViewShell() == m_pData->m_pViewShell )
            pFrame->GetFrame().SetIsClosing_Impl();
        m_pData->m_pViewShell->DiscardClients_Impl();
        m_pData->m_pViewShell->pImp->m_bControllerSet = false;

        if ( pFrame )
        {
            lang::EventObject aObject;
            aObject.Source = *this ;

            SfxObjectShell* pDoc = pFrame->GetObjectShell() ;
            SfxViewFrame *pView = SfxViewFrame::GetFirst(pDoc);
            while( pView )
            {
                // if there is another ViewFrame or currently the ViewShell in my ViewFrame is switched (PagePreview)
                if ( pView != pFrame || pView->GetViewShell() != m_pData->m_pViewShell )
                    break;
                pView = SfxViewFrame::GetNext( *pView, pDoc );
            }

            SFX_APP()->NotifyEvent( SfxViewEventHint(SFX_EVENT_CLOSEVIEW, GlobalEventConfig::GetEventName( STR_EVENT_CLOSEVIEW ), pDoc, Reference< frame::XController2 >( this ) ) );
            if ( !pView )
                SFX_APP()->NotifyEvent( SfxEventHint(SFX_EVENT_CLOSEDOC, GlobalEventConfig::GetEventName( STR_EVENT_CLOSEDOC ), pDoc) );

            Reference< frame::XModel > xModel = pDoc->GetModel();
            Reference < util::XCloseable > xCloseable( xModel, uno::UNO_QUERY );
            if ( xModel.is() )
            {
                xModel->disconnectController( this );
                if ( xCloseable.is() )
                    xCloseable->removeCloseListener( m_pData->m_xCloseListener );
            }

            Reference < frame::XFrame > aXFrame;
            attachFrame( aXFrame );

            m_pData->m_xListener->disposing( aObject );
            SfxViewShell *pShell = m_pData->m_pViewShell;
            m_pData->m_pViewShell = NULL;
            if ( pFrame->GetViewShell() == pShell )
            {
                // Enter registrations only allowed if we are the owner!
                if ( pFrame->GetFrame().OwnsBindings_Impl() )
                    pFrame->GetBindings().ENTERREGISTRATIONS();
                pFrame->GetFrame().SetFrameInterface_Impl(  aXFrame );
                pFrame->GetFrame().DoClose_Impl();
            }
        }
    }
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::addEventListener( const Reference< lang::XEventListener >& aListener ) throw( RuntimeException )
{
    m_pData->m_aListenerContainer.addInterface( ::getCppuType((const Reference< lang::XEventListener >*)0), aListener );
}

//________________________________________________________________________________________________________
//  SfxBaseController -> XComponent
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseController::removeEventListener( const Reference< lang::XEventListener >& aListener ) throw( RuntimeException )
{
    m_pData->m_aListenerContainer.removeInterface( ::getCppuType((const Reference< lang::XEventListener >*)0), aListener );
}

void SfxBaseController::ReleaseShell_Impl()
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        SfxObjectShell* pDoc = m_pData->m_pViewShell->GetObjectShell() ;
        Reference< frame::XModel > xModel = pDoc->GetModel();
        Reference < util::XCloseable > xCloseable( xModel, uno::UNO_QUERY );
        if ( xModel.is() )
        {
            xModel->disconnectController( this );
            if ( xCloseable.is() )
                xCloseable->removeCloseListener( m_pData->m_xCloseListener );
        }
        m_pData->m_pViewShell = 0;

        Reference < frame::XFrame > aXFrame;
        attachFrame( aXFrame );
    }
}

SfxViewShell* SfxBaseController::GetViewShell_Impl() const
{
    return m_pData->m_pViewShell;
}

Reference< task::XStatusIndicator > SAL_CALL SfxBaseController::getStatusIndicator(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell && !m_pData->m_xIndicator.is() )
        m_pData->m_xIndicator = new SfxStatusIndicator( this, m_pData->m_pViewShell->GetViewFrame()->GetFrame().GetWorkWindow_Impl() );
    return m_pData->m_xIndicator;
}

void SAL_CALL SfxBaseController::registerContextMenuInterceptor( const Reference< ui::XContextMenuInterceptor >& xInterceptor ) throw( RuntimeException )

{
    m_pData->m_aInterceptorContainer.addInterface( xInterceptor );

    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->AddContextMenuInterceptor_Impl( xInterceptor );
}

void SAL_CALL SfxBaseController::releaseContextMenuInterceptor( const Reference< ui::XContextMenuInterceptor >& xInterceptor ) throw( RuntimeException )

{
    m_pData->m_aInterceptorContainer.removeInterface( xInterceptor );

    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->RemoveContextMenuInterceptor_Impl( xInterceptor );
}

void SAL_CALL SfxBaseController::addKeyHandler( const Reference< awt::XKeyHandler >& xHandler ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    m_pData->m_aUserInputInterception.addKeyHandler( xHandler );
}

void SAL_CALL SfxBaseController::removeKeyHandler( const Reference< awt::XKeyHandler >& xHandler ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    m_pData->m_aUserInputInterception.removeKeyHandler( xHandler );
}

void SAL_CALL SfxBaseController::addMouseClickHandler( const Reference< awt::XMouseClickHandler >& xHandler ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    m_pData->m_aUserInputInterception.addMouseClickHandler( xHandler );
}

void SAL_CALL SfxBaseController::removeMouseClickHandler( const Reference< awt::XMouseClickHandler >& xHandler ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    m_pData->m_aUserInputInterception.removeMouseClickHandler( xHandler );
}

uno::Sequence< sal_Int16 > SAL_CALL SfxBaseController::getSupportedCommandGroups()
throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    std::list< sal_Int16 > aGroupList;
    SfxViewFrame* pViewFrame( m_pData->m_pViewShell->GetFrame() );
    SfxSlotPool*  pPool = &SfxSlotPool::GetSlotPool( pViewFrame );

    SfxSlotPool* pSlotPool = pPool ? pPool : &SFX_SLOTPOOL();
    const sal_uIntPtr nMode( SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_MENUCONFIG );

    // Select Group ( Group 0 is internal )
    for ( sal_uInt16 i=0; i<pSlotPool->GetGroupCount(); i++ )
    {
        pSlotPool->SeekGroup( i );
        const SfxSlot* pSfxSlot = pSlotPool->FirstSlot();
        while ( pSfxSlot )
        {
            if ( pSfxSlot->GetMode() & nMode )
            {
                sal_Int16 nCommandGroup = MapGroupIDToCommandGroup( pSfxSlot->GetGroupId() );
                aGroupList.push_back( nCommandGroup );
                break;
            }
            pSfxSlot = pSlotPool->NextSlot();
        }
    }

    uno::Sequence< sal_Int16 > aSeq =
        comphelper::containerToSequence< sal_Int16 >( aGroupList );
    return aSeq;
}

uno::Sequence< frame::DispatchInformation > SAL_CALL SfxBaseController::getConfigurableDispatchInformation( sal_Int16 nCmdGroup )
throw (RuntimeException)
{
    std::list< frame::DispatchInformation > aCmdList;

    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        const sal_uIntPtr nMode( SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_MENUCONFIG );

        SfxViewFrame* pViewFrame( m_pData->m_pViewShell->GetFrame() );
        SfxSlotPool*  pPool( &SfxSlotPool::GetSlotPool( pViewFrame ));
        OUString aCmdPrefix( ".uno:" );

        SfxSlotPool* pSlotPool = pPool ? pPool : &SFX_SLOTPOOL();
        for ( sal_uInt16 i=0; i<pSlotPool->GetGroupCount(); i++ )
        {
            pSlotPool->SeekGroup( i );
            const SfxSlot* pSfxSlot = pSlotPool->FirstSlot();
            if ( pSfxSlot )
            {
                sal_Int16 nCommandGroup = MapGroupIDToCommandGroup( pSfxSlot->GetGroupId() );
                if ( nCommandGroup == nCmdGroup )
                {
                    while ( pSfxSlot )
                    {
                        if ( pSfxSlot->GetMode() & nMode )
                        {
                            frame::DispatchInformation aCmdInfo;
                            OUStringBuffer aBuf( aCmdPrefix );
                            aBuf.appendAscii( pSfxSlot->GetUnoName() );
                            aCmdInfo.Command = aBuf.makeStringAndClear();
                            aCmdInfo.GroupId = nCommandGroup;
                            aCmdList.push_back( aCmdInfo );
                        }
                        pSfxSlot = pSlotPool->NextSlot();
                    }
                }
            }
        }
    }

    uno::Sequence< frame::DispatchInformation > aSeq =
        comphelper::containerToSequence< frame::DispatchInformation, std::list< frame::DispatchInformation > >( aCmdList );

    return aSeq;
}

sal_Bool SfxBaseController::HandleEvent_Impl( NotifyEvent& rEvent )
{
    return m_pData->m_aUserInputInterception.handleNotifyEvent( rEvent );
}

sal_Bool SfxBaseController::HasKeyListeners_Impl()
{
    return m_pData->m_aUserInputInterception.hasKeyHandlers();
}

sal_Bool SfxBaseController::HasMouseClickListeners_Impl()
{
    return m_pData->m_aUserInputInterception.hasMouseClickListeners();
}

void SfxBaseController::ConnectSfxFrame_Impl( const ConnectSfxFrame i_eConnect )
{
    ENSURE_OR_THROW( m_pData->m_pViewShell, "not to be called without a view shell" );
    SfxViewFrame* pViewFrame = m_pData->m_pViewShell->GetFrame();
    ENSURE_OR_THROW( pViewFrame, "a view shell without a view frame is pretty pathological" );

    const bool bConnect = ( i_eConnect != E_DISCONNECT );

    // disable window and dispatcher
    pViewFrame->Enable( bConnect );
    pViewFrame->GetDispatcher()->Lock( !bConnect );

    if ( bConnect )
    {
        if ( i_eConnect == E_CONNECT )
        {
            if  (   ( m_pData->m_pViewShell->GetObjectShell() != NULL )
                &&  ( m_pData->m_pViewShell->GetObjectShell()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
                )
            {
                SfxViewFrame* pViewFrm = m_pData->m_pViewShell->GetViewFrame();
                if ( !pViewFrm->GetFrame().IsInPlace() )
                {
                    // for outplace embedded objects, we want the layout manager to keep the content window
                    // size constant, if possible
                    try
                    {
                        Reference< beans::XPropertySet > xFrameProps( m_pData->m_xFrame, uno::UNO_QUERY_THROW );
                        Reference< beans::XPropertySet > xLayouterProps(
                            xFrameProps->getPropertyValue("LayoutManager"), uno::UNO_QUERY_THROW );
                        xLayouterProps->setPropertyValue("PreserveContentSize", uno::makeAny( sal_True ) );
                    }
                    catch (const uno::Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }

        // upon DISCONNECT, we did *not* pop the shells from the stack (this is done elsewhere), so upon
        // RECONNECT, we're not allowed to push them
        if ( i_eConnect != E_RECONNECT )
        {
            pViewFrame->GetDispatcher()->Push( *m_pData->m_pViewShell );
            if ( m_pData->m_pViewShell->GetSubShell() )
                pViewFrame->GetDispatcher()->Push( *m_pData->m_pViewShell->GetSubShell() );
            m_pData->m_pViewShell->PushSubShells_Impl();
            pViewFrame->GetDispatcher()->Flush();
        }

        Window* pEditWin = m_pData->m_pViewShell->GetWindow();
        if ( pEditWin && m_pData->m_pViewShell->IsShowView_Impl() )
            pEditWin->Show();

        if ( SfxViewFrame::Current() == pViewFrame )
            pViewFrame->GetDispatcher()->Update_Impl( sal_True );

        Window* pFrameWin = &pViewFrame->GetWindow();
        if ( pFrameWin != &pViewFrame->GetFrame().GetWindow() )
            pFrameWin->Show();

        if ( i_eConnect == E_CONNECT )
        {
            ::comphelper::NamedValueCollection aDocumentArgs( getModel()->getArgs() );

            const sal_Int16 nPluginMode = aDocumentArgs.getOrDefault( "PluginMode", sal_Int16( 0 ) );
            const bool bHasPluginMode = ( nPluginMode != 0 );

            SfxFrame& rFrame = pViewFrame->GetFrame();
            SfxObjectShell& rDoc = *m_pData->m_pViewShell->GetObjectShell();
            if ( !rFrame.IsMarkedHidden_Impl() )
            {
                if ( rDoc.IsHelpDocument() || ( nPluginMode == 2 ) )
                    pViewFrame->GetDispatcher()->HideUI( sal_True );
                else
                    pViewFrame->GetDispatcher()->HideUI( sal_False );

                if ( rFrame.IsInPlace() )
                    pViewFrame->LockAdjustPosSizePixel();

                if ( nPluginMode == 3 )
                    rFrame.GetWorkWindow_Impl()->SetInternalDockingAllowed( sal_False );

                if ( !rFrame.IsInPlace() )
                    pViewFrame->GetDispatcher()->Update_Impl();
                pViewFrame->Show();
                rFrame.GetWindow().Show();
                if ( !rFrame.IsInPlace() || ( nPluginMode == 3 ) )
                    pViewFrame->MakeActive_Impl( rFrame.GetFrameInterface()->isActive() );

                if ( rFrame.IsInPlace() )
                {
                    pViewFrame->UnlockAdjustPosSizePixel();
                    // force resize for OLE server to fix layout problems of writer and math
                    // see i53651
                    if ( nPluginMode == 3 )
                        pViewFrame->Resize( sal_True );
                }
            }
            else
            {
                DBG_ASSERT( !rFrame.IsInPlace() && !bHasPluginMode, "Special modes not compatible with hidden mode!" );
                rFrame.GetWindow().Show();
            }

            // UpdateTitle now, hidden TopFrames have otherwise no Name!
            pViewFrame->UpdateTitle();

            if ( !rFrame.IsInPlace() )
                pViewFrame->Resize( sal_True );

            // if there's a JumpMark given, then, well, jump to it
            ::comphelper::NamedValueCollection aViewArgs( getCreationArguments() );
            const OUString sJumpMark = aViewArgs.getOrDefault( "JumpMark", OUString() );
            const bool bHasJumpMark = !sJumpMark.isEmpty();
            OSL_ENSURE( ( !m_pData->m_pViewShell->GetObjectShell()->IsLoading() )
                    ||  ( sJumpMark.isEmpty() ),
                "SfxBaseController::ConnectSfxFrame_Impl: so this code wasn't dead?" );
                // Before CWS autorecovery, there was code which postponed jumping to the Mark to a later time
                // (SfxObjectShell::PositionView_Impl), but it seems this branch was never used, since this method
                // here is never called before the load process finished. At least not with a non-empty jump mark
            if ( !sJumpMark.isEmpty() )
                m_pData->m_pViewShell->JumpToMark( sJumpMark );

            // if no plugin mode and no jump mark was supplied, check whether the document itself can provide view data, and
            // if so, forward it to the view/shell.
            if ( !bHasPluginMode && !bHasJumpMark )
            {
                // Note that this might not be the ideal place here. Restoring view data should, IMO, be the
                // responsibility of the loader, not an implementation detail burried here deep within the controller's
                // implementation.
                // What I think should be done to replace the below code:
                // - change SfxBaseController::restoreViewData to also accept a PropertyValue[] (it currently accepts
                //   a string only), and forward it to its ViewShell's ReadUserDataSequence
                // - change the frame loader so that when a new document is loaded (as opposed to an existing
                //   document being loaded into a new frame), the model's view data is examine the very same
                //   way as below, and the proper view data is set via XController::restoreViewData
                // - extend SfxViewFrame::SwitchToViewShell_Impl. Currently, it cares for the case where a non-PrintPreview
                //   view is exchanged, and sets the old view's data at the model. It should also care for the other
                //   way, were the PrintPreview view is left: in this case, the new view should also be initialized
                //   with the model's view data
                try
                {
                    Reference< XViewDataSupplier > xViewDataSupplier( getModel(), UNO_QUERY_THROW );
                    Reference< XIndexAccess > xViewData( xViewDataSupplier->getViewData() );

                    // find the view data item whose ViewId matches the ID of the view we're just connecting to
                    const SfxObjectFactory& rDocFactory( rDoc.GetFactory() );
                    const sal_Int32 nCount = xViewData.is() ? xViewData->getCount() : 0;
                    sal_Int32 nViewDataIndex = 0;
                    for ( sal_Int32 i=0; i<nCount; ++i )
                    {
                        const ::comphelper::NamedValueCollection aViewData( xViewData->getByIndex(i) );
                        OUString sViewId( aViewData.getOrDefault( "ViewId", OUString() ) );
                        if ( sViewId.isEmpty() )
                            continue;

                        const SfxViewFactory* pViewFactory = rDocFactory.GetViewFactoryByViewName( sViewId );
                        if ( pViewFactory == NULL )
                            continue;

                        if ( pViewFactory->GetOrdinal() == pViewFrame->GetCurViewId() )
                        {
                            nViewDataIndex = i;
                            break;
                        }
                    }
                    if ( nViewDataIndex < nCount )
                    {
                        Sequence< PropertyValue > aViewData;
                        OSL_VERIFY( xViewData->getByIndex( nViewDataIndex ) >>= aViewData );
                        if ( aViewData.getLength() > 0 )
                            m_pData->m_pViewShell->ReadUserDataSequence( aViewData, sal_True );
                    }
                }
                catch (const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
    }

    // invalidate slot corresponding to the view shell
    const sal_uInt16 nViewNo = m_pData->m_pViewShell->GetObjectShell()->GetFactory().GetViewNo_Impl( pViewFrame->GetCurViewId(), USHRT_MAX );
    DBG_ASSERT( nViewNo != USHRT_MAX, "view shell id not found" );
    if ( nViewNo != USHRT_MAX )
        pViewFrame->GetBindings().Invalidate( nViewNo + SID_VIEWSHELL0 );
}

void SfxBaseController::ShowInfoBars( )
{
    if ( m_pData->m_pViewShell )
    {
        // CMIS verifications
        Reference< document::XCmisDocument > xCmisDoc( m_pData->m_pViewShell->GetObjectShell()->GetModel(), uno::UNO_QUERY );
        if ( xCmisDoc.is( ) && xCmisDoc->canCheckOut( ) )
        {
            beans::PropertyValues aCmisProperties = xCmisDoc->getCmisPropertiesValues( );

            if ( xCmisDoc->isVersionable( ) && aCmisProperties.hasElements( ) )
            {
                // Loop over the CMIS Properties to find cmis:isVersionSeriesCheckedOut
                bool bFoundCheckedout = false;
                sal_Bool bCheckedOut = sal_False;
                for ( sal_Int32 i = 0; i < aCmisProperties.getLength() && !bFoundCheckedout; ++i )
                {
                    if ( aCmisProperties[i].Name == "cmis:isVersionSeriesCheckedOut" )
                    {
                        bFoundCheckedout = true;
                        aCmisProperties[i].Value >>= bCheckedOut;
                    }
                }

                if ( !bCheckedOut )
                {
                    // Get the Frame and show the InfoBar if not checked out
                    SfxViewFrame* pViewFrame = m_pData->m_pViewShell->GetFrame();
                    std::vector< PushButton* > aButtons;
                    PushButton* pBtn = new PushButton( &pViewFrame->GetWindow(), SfxResId( BT_CHECKOUT ) );
                    pBtn->SetClickHdl( LINK( this, SfxBaseController, CheckOutHandler ) );
                    aButtons.push_back( pBtn );
                    pViewFrame->AppendInfoBar( "checkout", SfxResId( STR_NONCHECKEDOUT_DOCUMENT ), aButtons );
                }
            }
        }
    }
}

IMPL_LINK_NOARG ( SfxBaseController, CheckOutHandler )
{
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->GetObjectShell()->CheckOut( );
    return 0;
}


//=============================================================================
Reference< frame::XTitle > SfxBaseController::impl_getTitleHelper ()
{
    SolarMutexGuard aGuard;

    if ( ! m_pData->m_xTitleHelper.is ())
    {
        Reference< frame::XModel >           xModel           = getModel ();
        Reference< frame::XUntitledNumbers > xUntitledProvider(xModel                                       , uno::UNO_QUERY      );
        Reference< frame::XController >      xThis            (static_cast< frame::XController* >(this), uno::UNO_QUERY_THROW);

        ::framework::TitleHelper* pHelper                 = new ::framework::TitleHelper(::comphelper::getProcessComponentContext());
                                  m_pData->m_xTitleHelper = Reference< frame::XTitle >(static_cast< ::cppu::OWeakObject* >(pHelper), uno::UNO_QUERY_THROW);

        pHelper->setOwner                   (xThis            );
        pHelper->connectWithUntitledNumbers (xUntitledProvider);
    }

    return m_pData->m_xTitleHelper;
}

//=============================================================================
// frame::XTitle
OUString SAL_CALL SfxBaseController::getTitle()
    throw (RuntimeException)
{
    return impl_getTitleHelper()->getTitle ();
}

//=============================================================================
// frame::XTitle
void SAL_CALL SfxBaseController::setTitle(const OUString& sTitle)
    throw (RuntimeException)
{
    impl_getTitleHelper()->setTitle (sTitle);
}

//=============================================================================
// frame::XTitleChangeBroadcaster
void SAL_CALL SfxBaseController::addTitleChangeListener(const Reference< frame::XTitleChangeListener >& xListener)
    throw (RuntimeException)
{
    Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

//=============================================================================
// frame::XTitleChangeBroadcaster
void SAL_CALL SfxBaseController::removeTitleChangeListener(const Reference< frame::XTitleChangeListener >& xListener)
    throw (RuntimeException)
{
    Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
