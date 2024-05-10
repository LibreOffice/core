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
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/document/XCmisDocument.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XBorderResizeListener.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/multicontainer2.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/userinputinterception.hxx>

#include <unoctitm.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfxresid.hxx>
#include <workwin.hxx>
#include <sfx2/infobar.hxx>

#include <osl/mutex.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/helper/convert.hxx>
#include <framework/titlehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <tools/svborder.hxx>

#include <sfx2/event.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/strings.hrc>
#include <sfxbasecontroller_internal.hxx>

#include <unordered_map>

#include <com/sun/star/ui/XSidebarProvider.hpp>
#include <sidebar/UnoSidebar.hxx>

#define TIMEOUT_START_RESCHEDULE    10L /* 10th s */

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::DisposedException;
using ::com::sun::star::awt::XWindow;
using ::com::sun::star::frame::XController;
using ::com::sun::star::frame::XDispatchProvider;
using ::com::sun::star::document::XViewDataSupplier;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::StringPair;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XFrameActionListener;
using ::com::sun::star::util::XCloseListener;
using ::com::sun::star::task::XStatusIndicator;
using ::com::sun::star::frame::XTitle;
using ::com::sun::star::ui::XSidebarProvider;


typedef std::unordered_map< SfxGroupId, sal_Int16 > GroupHashMap;

sal_Int16 MapGroupIDToCommandGroup( SfxGroupId nGroupID )
{
    static GroupHashMap s_aHashMap
    {
        { SfxGroupId::Intern        ,   frame::CommandGroup::INTERNAL       },
        { SfxGroupId::Application   ,   frame::CommandGroup::APPLICATION    },
        { SfxGroupId::Document      ,   frame::CommandGroup::DOCUMENT       },
        { SfxGroupId::View          ,   frame::CommandGroup::VIEW           },
        { SfxGroupId::Edit          ,   frame::CommandGroup::EDIT           },
        { SfxGroupId::Macro         ,   frame::CommandGroup::MACRO          },
        { SfxGroupId::Options       ,   frame::CommandGroup::OPTIONS        },
        { SfxGroupId::Math          ,   frame::CommandGroup::MATH           },
        { SfxGroupId::Navigator     ,   frame::CommandGroup::NAVIGATOR      },
        { SfxGroupId::Insert        ,   frame::CommandGroup::INSERT         },
        { SfxGroupId::Format        ,   frame::CommandGroup::FORMAT         },
        { SfxGroupId::Template      ,   frame::CommandGroup::TEMPLATE       },
        { SfxGroupId::Text          ,   frame::CommandGroup::TEXT           },
        { SfxGroupId::Frame         ,   frame::CommandGroup::FRAME          },
        { SfxGroupId::Graphic       ,   frame::CommandGroup::GRAPHIC        },
        { SfxGroupId::Table         ,   frame::CommandGroup::TABLE          },
        { SfxGroupId::Enumeration   ,   frame::CommandGroup::ENUMERATION    },
        { SfxGroupId::Data          ,   frame::CommandGroup::DATA           },
        { SfxGroupId::Special       ,   frame::CommandGroup::SPECIAL        },
        { SfxGroupId::Image         ,   frame::CommandGroup::IMAGE          },
        { SfxGroupId::Chart         ,   frame::CommandGroup::CHART          },
        { SfxGroupId::Explorer      ,   frame::CommandGroup::EXPLORER       },
        { SfxGroupId::Connector     ,   frame::CommandGroup::CONNECTOR      },
        { SfxGroupId::Modify        ,   frame::CommandGroup::MODIFY         },
        { SfxGroupId::Drawing       ,   frame::CommandGroup::DRAWING        },
        { SfxGroupId::Controls      ,   frame::CommandGroup::CONTROLS       },
    };


    GroupHashMap::const_iterator pIter = s_aHashMap.find( nGroupID );
    if ( pIter != s_aHashMap.end() )
        return pIter->second;
    else
        return frame::CommandGroup::INTERNAL;
}

sal_uInt32 Get10ThSec()
{
    sal_uInt32 n10Ticks = 10 * static_cast<sal_uInt32>(clock());
    return n10Ticks / CLOCKS_PER_SEC;
}

static sal_Int32 m_nInReschedule = 0;  /// static counter for rescheduling

static void reschedule()
{
    if ( m_nInReschedule == 0 )
    {
        ++m_nInReschedule;
        Application::Reschedule();
        --m_nInReschedule;
    }
}

namespace {

class SfxStatusIndicator : public ::cppu::WeakImplHelper< task::XStatusIndicator, lang::XEventListener >
{
    Reference < XController > xOwner;
    Reference < task::XStatusIndicator > xProgress;
    SfxWorkWindow*          pWorkWindow;
    tools::Long                    _nStartTime;
public:
                            SfxStatusIndicator(SfxBaseController* pController, SfxWorkWindow* pWork)
                                : xOwner( pController )
                                , pWorkWindow( pWork )
                                , _nStartTime(0)
                            {
                                osl_atomic_increment(&m_refCount);
                                Reference< lang::XComponent > xComponent = pController;
                                if (xComponent.is())
                                    xComponent->addEventListener(this);
                                osl_atomic_decrement(&m_refCount);
                            }

    virtual void SAL_CALL   start(const OUString& aText, sal_Int32 nRange) override;
    virtual void SAL_CALL   end() override;
    virtual void SAL_CALL   setText(const OUString& aText) override;
    virtual void SAL_CALL   setValue(sal_Int32 nValue) override;
    virtual void SAL_CALL   reset() override;

    virtual void SAL_CALL   disposing( const lang::EventObject& Source ) override;
};

}

void SAL_CALL SfxStatusIndicator::start(const OUString& aText, sal_Int32 nRange)
{
    SolarMutexGuard aGuard;
    if ( xOwner.is() )
    {
        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->start( aText, nRange );

        _nStartTime = Get10ThSec();
        reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::end()
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

void SAL_CALL SfxStatusIndicator::setText(const OUString& aText)
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

void SAL_CALL SfxStatusIndicator::setValue( sal_Int32 nValue )
{
    SolarMutexGuard aGuard;
    if ( xOwner.is() )
    {
        if ( !xProgress.is() )
            xProgress = pWorkWindow->GetStatusIndicator();

        if ( xProgress.is() )
            xProgress->setValue( nValue );

        bool bReschedule = (( Get10ThSec() - _nStartTime ) > TIMEOUT_START_RESCHEDULE );
        if ( bReschedule )
            reschedule();
    }
}

void SAL_CALL SfxStatusIndicator::reset()
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

void SAL_CALL SfxStatusIndicator::disposing( const lang::EventObject& /*Source*/ )
{
    SolarMutexGuard aGuard;
    xOwner = nullptr;
    xProgress.clear();
}


//  declaration IMPL_SfxBaseController_ListenerHelper

namespace {

class IMPL_SfxBaseController_ListenerHelper : public ::cppu::WeakImplHelper< frame::XFrameActionListener >
{
public:
    explicit IMPL_SfxBaseController_ListenerHelper(  SfxBaseController*  pController ) ;

    virtual void SAL_CALL frameAction( const frame::FrameActionEvent& aEvent ) override ;
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) override ;

private:

    SfxBaseController*      m_pController   ;

} ; // class IMPL_SfxBaseController_ListenerContainer

class IMPL_SfxBaseController_CloseListenerHelper : public ::cppu::WeakImplHelper< util::XCloseListener >
{
public:
    explicit IMPL_SfxBaseController_CloseListenerHelper( SfxBaseController*  pController ) ;

    virtual void SAL_CALL queryClosing( const lang::EventObject& aEvent, sal_Bool bDeliverOwnership ) override ;
    virtual void SAL_CALL notifyClosing( const lang::EventObject& aEvent ) override ;
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) override ;

private:

    SfxBaseController*      m_pController;

} ; // class IMPL_SfxBaseController_ListenerContainer

}

IMPL_SfxBaseController_CloseListenerHelper::IMPL_SfxBaseController_CloseListenerHelper( SfxBaseController*  pController )
        : m_pController ( pController   )
{
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::disposing( const lang::EventObject& /*aEvent*/ )
{
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::queryClosing( const lang::EventObject& /*aEvent*/, sal_Bool /*bDeliverOwnership*/ )
{
    SolarMutexGuard aGuard;
    SfxViewShell* pShell = m_pController->GetViewShell_Impl();
    if  (pShell)
    {
        bool bCanClose = pShell->PrepareClose( false );
        if ( !bCanClose )
        {
            throw util::CloseVetoException(u"Controller disagree ..."_ustr,getXWeak());
        }
    }
}

void SAL_CALL IMPL_SfxBaseController_CloseListenerHelper::notifyClosing( const lang::EventObject& /*aEvent*/ )
{
}


//  declaration IMPL_SfxBaseController_DataContainer


struct IMPL_SfxBaseController_DataContainer
{
    Reference< XFrame >                     m_xFrame                ;
    Reference< XFrameActionListener >       m_xListener             ;
    Reference< XCloseListener >             m_xCloseListener        ;
    ::sfx2::UserInputInterception           m_aUserInputInterception;
    ::comphelper::OMultiTypeInterfaceContainerHelper2      m_aListenerContainer    ;
    ::comphelper::OInterfaceContainerHelper3<ui::XContextMenuInterceptor> m_aInterceptorContainer ;
    Reference< XStatusIndicator >           m_xIndicator            ;
    SfxViewShell*                           m_pViewShell            ;
    SfxBaseController*                      m_pController           ;
    bool                                    m_bDisposing            ;
    bool                                    m_bSuspendState         ;
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
            ,   m_bSuspendState         ( false                                              )
    {
    }

} ; // struct IMPL_SfxBaseController_DataContainer


//  IMPL_SfxBaseController_ListenerHelper constructor


IMPL_SfxBaseController_ListenerHelper::IMPL_SfxBaseController_ListenerHelper(   SfxBaseController*  pController )
        : m_pController ( pController   )
{
}

void SAL_CALL IMPL_SfxBaseController_ListenerHelper::frameAction( const frame::FrameActionEvent& aEvent )
{
    SolarMutexGuard aGuard;
    if  (
            ( m_pController !=  nullptr ) &&
            ( aEvent.Frame  ==  m_pController->getFrame() ) &&
            ( m_pController->GetViewShell_Impl() && m_pController->GetViewShell_Impl()->GetWindow() !=  nullptr                                                    )
        )
    {
        if ( aEvent.Action == frame::FrameAction_FRAME_UI_ACTIVATED )
        {
            if ( !m_pController->GetViewShell_Impl()->GetUIActiveIPClient_Impl() )
                m_pController->GetViewShell_Impl()->GetViewFrame().MakeActive_Impl( false );
        }
        else if ( aEvent.Action == frame::FrameAction_CONTEXT_CHANGED )
        {
            m_pController->GetViewShell_Impl()->GetViewFrame().GetBindings().ContextChanged_Impl();
        }
    }
}


//  IMPL_SfxBaseController_ListenerHelper -> XEventListener


void SAL_CALL IMPL_SfxBaseController_ListenerHelper::disposing( const lang::EventObject& /*aEvent*/ )
{
    SolarMutexGuard aGuard;
    if ( m_pController && m_pController->getFrame().is() )
        m_pController->getFrame()->removeFrameActionListener( this ) ;
}

SfxBaseController::SfxBaseController( SfxViewShell* pViewShell )
    :   m_pData ( new IMPL_SfxBaseController_DataContainer( m_aMutex, pViewShell, this ))
{
    m_pData->m_pViewShell->SetController( this );
}


//  SfxBaseController -> destructor


SfxBaseController::~SfxBaseController()
{
}


//  SfxBaseController -> XController2


Reference< XWindow > SAL_CALL SfxBaseController::getComponentWindow()
{
    SolarMutexGuard aGuard;
    if ( !m_pData->m_pViewShell )
        throw DisposedException();

    return Reference< XWindow >( GetViewFrame_Impl().GetFrame().GetWindow().GetComponentInterface(), UNO_QUERY_THROW );
}

OUString SAL_CALL SfxBaseController::getViewControllerName()
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

Sequence< PropertyValue > SAL_CALL SfxBaseController::getCreationArguments()
{
    SolarMutexGuard aGuard;
    if ( !m_pData->m_pViewShell || !m_pData->m_pViewShell->GetObjectShell() )
        throw DisposedException();

    return m_pData->m_aCreationArgs;
}

void SfxBaseController::SetCreationArguments_Impl( const Sequence< PropertyValue >& i_rCreationArgs )
{
    OSL_ENSURE( !m_pData->m_aCreationArgs.hasElements(), "SfxBaseController::SetCreationArguments_Impl: not intended to be called twice!" );
    m_pData->m_aCreationArgs = i_rCreationArgs;
}

SfxViewFrame& SfxBaseController::GetViewFrame_Impl() const
{
    ENSURE_OR_THROW( m_pData->m_pViewShell, "not to be called without a view shell" );
    SfxViewFrame* pActFrame = m_pData->m_pViewShell->GetFrame();
    ENSURE_OR_THROW( pActFrame, "a view shell without a view frame is pretty pathological" );
    return *pActFrame;
}


Reference<XSidebarProvider> SAL_CALL SfxBaseController::getSidebar()
{
        SfxViewFrame& rViewFrame = GetViewFrame_Impl();
        SfxFrame& rFrame = rViewFrame.GetFrame();

        Reference<XSidebarProvider> rSidebar = new SfxUnoSidebar(rFrame.GetFrameInterface());
        return rSidebar;
}


//  SfxBaseController -> XController2 -> XController


void SAL_CALL SfxBaseController::attachFrame( const Reference< frame::XFrame >& xFrame )
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

    if ( !xFrame.is() )
        return;

    xFrame->addFrameActionListener( m_pData->m_xListener ) ;
    Reference < util::XCloseBroadcaster > xCloseable( xFrame, uno::UNO_QUERY );
    if ( xCloseable.is() )
        xCloseable->addCloseListener( m_pData->m_xCloseListener );

    if ( m_pData->m_pViewShell )
    {
        ConnectSfxFrame_Impl( E_CONNECT );
        ShowInfoBars( );

        // attaching the frame to the controller is the last step in the creation of a new view, so notify this
        SfxViewEventHint aHint( SfxEventHintId::ViewCreated, GlobalEventConfig::GetEventName( GlobalEventId::VIEWCREATED ), m_pData->m_pViewShell->GetObjectShell(), Reference< frame::XController2 >( this ) );
        SfxGetpApp()->NotifyEvent( aHint );
    }
}


//  SfxBaseController -> XController


sal_Bool SAL_CALL SfxBaseController::attachModel( const Reference< frame::XModel >& xModel )
{
    if ( m_pData->m_pViewShell && xModel.is() && xModel != m_pData->m_pViewShell->GetObjectShell()->GetModel() )
    {
        // don't allow to reattach a model!
        OSL_FAIL("Can't reattach model!");
        return false;
    }

    Reference < util::XCloseBroadcaster > xCloseable( xModel, uno::UNO_QUERY );
    if ( xCloseable.is() )
        xCloseable->addCloseListener( m_pData->m_xCloseListener );
    return true;
}


//  SfxBaseController -> XController


sal_Bool SAL_CALL SfxBaseController::suspend( sal_Bool bSuspend )
{
    SolarMutexGuard aGuard;

    // ignore duplicate calls, which doesn't change anything real
    if (bool(bSuspend) == m_pData->m_bSuspendState)
       return true;

    if ( bSuspend )
    {
        if ( !m_pData->m_pViewShell )
        {
            m_pData->m_bSuspendState = true;
            return true;
        }

        if ( !m_pData->m_pViewShell->PrepareClose() )
            return false;

        if ( getFrame().is() )
            getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;
        SfxViewFrame* pActFrame = m_pData->m_pViewShell->GetFrame() ;

        // More Views on the same document?
        SfxObjectShell* pDocShell   =   m_pData->m_pViewShell->GetObjectShell() ;
        bool            bOther      =   false                                   ;

        for ( const SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell ); !bOther && pFrame; pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell ) )
            bOther = (pFrame != pActFrame);

        bool bRet = bOther || pDocShell->PrepareClose();
        if ( bRet )
        {
            ConnectSfxFrame_Impl( E_DISCONNECT );
            m_pData->m_bSuspendState = true;
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

        m_pData->m_bSuspendState = false;
        return true ;
    }
}


//  SfxBaseController -> XController


uno::Any SfxBaseController::getViewData()
{
    uno::Any         aAny;
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        OUString sData;
        m_pData->m_pViewShell->WriteUserData( sData ) ;
        aAny <<= sData ;
    }

    return aAny ;
}


//  SfxBaseController -> XController


void SAL_CALL SfxBaseController::restoreViewData( const uno::Any& aValue )
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        OUString sData;
        aValue >>= sData ;
        m_pData->m_pViewShell->ReadUserData( sData ) ;
    }
}


//  SfxBaseController -> XController


Reference< frame::XFrame > SAL_CALL SfxBaseController::getFrame()
{
    SolarMutexGuard aGuard;
    return m_pData->m_xFrame;
}


//  SfxBaseController -> XController


Reference< frame::XModel > SAL_CALL SfxBaseController::getModel()
{
    SolarMutexGuard aGuard;
    return m_pData->m_pViewShell ? m_pData->m_pViewShell->GetObjectShell()->GetModel() : Reference < frame::XModel > () ;
}


//  SfxBaseController -> XDispatchProvider

static css::uno::Reference<css::frame::XDispatch>
GetSlotDispatchWithFallback(SfxViewFrame* pViewFrame, const css::util::URL& aURL,
                            const OUString& sActCommand, bool bMasterCommand, const SfxSlot* pSlot)
{
    assert(pViewFrame);

    if (pSlot && (!pViewFrame->GetFrame().IsInPlace() || !pSlot->IsMode(SfxSlotMode::CONTAINER)))
        return pViewFrame->GetBindings().GetDispatch(pSlot, aURL, bMasterCommand);

    // try to find parent SfxViewFrame
    if (const auto& xOwnFrame = pViewFrame->GetFrame().GetFrameInterface())
    {
        if (const auto& xParentFrame = xOwnFrame->getCreator())
        {
            // TODO/LATER: in future probably SfxViewFrame hierarchy should be the same as XFrame hierarchy
            // SfxViewFrame* pParentFrame = pViewFrame->GetParentViewFrame();

            // search the related SfxViewFrame
            SfxViewFrame* pParentFrame = nullptr;
            for (SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame;
                 pFrame = SfxViewFrame::GetNext(*pFrame))
            {
                if (pFrame->GetFrame().GetFrameInterface() == xParentFrame)
                {
                    pParentFrame = pFrame;
                    break;
                }
            }

            if (pParentFrame)
            {
                const SfxSlotPool& rFrameSlotPool = SfxSlotPool::GetSlotPool(pParentFrame);
                if (const SfxSlot* pSlot2 = rFrameSlotPool.GetUnoSlot(sActCommand))
                    return pParentFrame->GetBindings().GetDispatch(pSlot2, aURL, bMasterCommand);
            }
        }
    }

    return {};
}

Reference< frame::XDispatch > SAL_CALL SfxBaseController::queryDispatch(   const   util::URL&             aURL            ,
                                                                    const   OUString&            sTargetFrameName,
                                                                            sal_Int32           eSearchFlags    )
{
    SolarMutexGuard aGuard;

    if (!m_pData->m_bDisposing && m_pData->m_pViewShell)
    {
        SfxViewFrame& rAct = m_pData->m_pViewShell->GetViewFrame() ;
        if ( sTargetFrameName == "_beamer" )
        {
            if ( eSearchFlags & frame::FrameSearchFlag::CREATE )
                rAct.SetChildWindow( SID_BROWSER, true );
            if (SfxChildWindow* pChildWin = rAct.GetChildWindow(SID_BROWSER))
            {
                if (Reference<frame::XFrame> xFrame{ pChildWin->GetFrame() })
                {
                    xFrame->setName(sTargetFrameName);
                    if (Reference<XDispatchProvider> xProv{ xFrame, uno::UNO_QUERY })
                        return xProv->queryDispatch(aURL, sTargetFrameName, frame::FrameSearchFlag::SELF);
                }
            }
        }

        if ( aURL.Protocol == ".uno:" )
        {
            OUString aActCommand = SfxOfficeDispatch::GetMasterUnoCommand(aURL);
            bool bMasterCommand(!aActCommand.isEmpty());
            if (!bMasterCommand)
                aActCommand = aURL.Path;
            const SfxSlot* pSlot = SfxSlotPool::GetSlotPool(&rAct).GetUnoSlot(aActCommand);
            return GetSlotDispatchWithFallback(&rAct, aURL, aActCommand, bMasterCommand, pSlot);
        }
        else if ( aURL.Protocol == "slot:" )
        {
            sal_uInt16 nId = static_cast<sal_uInt16>(aURL.Path.toInt32());

            if (nId >= SID_VERB_START && nId <= SID_VERB_END)
            {
                const SfxSlot* pSlot = m_pData->m_pViewShell->GetVerbSlot_Impl(nId);
                if ( pSlot )
                    return rAct.GetBindings().GetDispatch( pSlot, aURL, false );
            }

            const SfxSlot* pSlot = SfxSlotPool::GetSlotPool(&rAct).GetSlot(nId);
            return GetSlotDispatchWithFallback(&rAct, aURL, aURL.Path, false, pSlot);
        }
        else if( sTargetFrameName == "_self" || sTargetFrameName.isEmpty() )
        {
            // check for already loaded URL ... but with additional jumpmark!
            Reference< frame::XModel > xModel = getModel();
            if( xModel.is() && !aURL.Mark.isEmpty() )
            {
                SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool( &rAct );
                const SfxSlot* pSlot = rSlotPool.GetSlot( SID_JUMPTOMARK );
                if( !aURL.Main.isEmpty() && aURL.Main == xModel->getURL() && pSlot )
                    return Reference< frame::XDispatch >( new SfxOfficeDispatch( rAct.GetBindings(), rAct.GetDispatcher(), pSlot, aURL) );
            }
        }
    }

    return {};
}


//  SfxBaseController -> XDispatchProvider


uno::Sequence< Reference< frame::XDispatch > > SAL_CALL SfxBaseController::queryDispatches( const uno::Sequence< frame::DispatchDescriptor >& seqDescripts )
{
    // Create return list - which must have same size then the given descriptor
    // It's not allowed to pack it!
    sal_Int32 nCount = seqDescripts.getLength();
    uno::Sequence< Reference< frame::XDispatch > > lDispatcher( nCount );

    std::transform(seqDescripts.begin(), seqDescripts.end(), lDispatcher.getArray(),
        [this](const frame::DispatchDescriptor& rDesc) -> Reference< frame::XDispatch > {
            return queryDispatch(rDesc.FeatureURL, rDesc.FrameName, rDesc.SearchFlags); });

    return lDispatcher;
}


//  SfxBaseController -> XControllerBorder


frame::BorderWidths SAL_CALL SfxBaseController::getBorder()
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
{
    m_pData->m_aListenerContainer.addInterface( cppu::UnoType<frame::XBorderResizeListener>::get(),
                                                xListener );
}

void SAL_CALL SfxBaseController::removeBorderResizeListener( const Reference< frame::XBorderResizeListener >& xListener )
{
    m_pData->m_aListenerContainer.removeInterface( cppu::UnoType<frame::XBorderResizeListener>::get(),
                                                xListener );
}

awt::Rectangle SAL_CALL SfxBaseController::queryBorderedArea( const awt::Rectangle& aPreliminaryRectangle )
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        tools::Rectangle aTmpRect = VCLRectangle( aPreliminaryRectangle );
        m_pData->m_pViewShell->QueryObjAreaPixel( aTmpRect );
        return AWTRectangle( aTmpRect );
    }

    return aPreliminaryRectangle;
}

void SfxBaseController::BorderWidthsChanged_Impl()
{
    ::comphelper::OInterfaceContainerHelper2* pContainer = m_pData->m_aListenerContainer.getContainer(
                        cppu::UnoType<frame::XBorderResizeListener>::get());
    if ( !pContainer )
        return;

    frame::BorderWidths aBWidths = getBorder();
    Reference< uno::XInterface > xThis( getXWeak() );

    ::comphelper::OInterfaceIteratorHelper2 pIterator(*pContainer);
    while (pIterator.hasMoreElements())
    {
        try
        {
            static_cast<frame::XBorderResizeListener*>(pIterator.next())->borderWidthsChanged( xThis, aBWidths );
        }
        catch (const RuntimeException&)
        {
            pIterator.remove();
        }
    }
}


//  SfxBaseController -> XComponent


void SAL_CALL SfxBaseController::dispose()
{
    SolarMutexGuard aGuard;
    Reference< XController > xKeepAlive( this );
    m_pData->m_bDisposing = true ;

    lang::EventObject aEventObject;
    aEventObject.Source = *this ;
    m_pData->m_aListenerContainer.disposeAndClear( aEventObject ) ;

    if ( m_pData->m_pController && m_pData->m_pController->getFrame().is() )
        m_pData->m_pController->getFrame()->removeFrameActionListener( m_pData->m_xListener ) ;

    if ( !m_pData->m_pViewShell )
        return;

    SfxViewFrame& rFrame = m_pData->m_pViewShell->GetViewFrame() ;
    if (rFrame.GetViewShell() == m_pData->m_pViewShell )
        rFrame.GetFrame().SetIsClosing_Impl();
    m_pData->m_pViewShell->DisconnectAllClients();

    lang::EventObject aObject;
    aObject.Source = *this ;

    SfxObjectShell* pDoc = rFrame.GetObjectShell() ;
    SfxViewFrame *pView = SfxViewFrame::GetFirst(pDoc);
    while( pView )
    {
        // if there is another ViewFrame or currently the ViewShell in my ViewFrame is switched (PagePreview)
        if ( pView != &rFrame || pView->GetViewShell() != m_pData->m_pViewShell )
            break;
        pView = SfxViewFrame::GetNext( *pView, pDoc );
    }

    SfxGetpApp()->NotifyEvent( SfxViewEventHint(SfxEventHintId::CloseView, GlobalEventConfig::GetEventName( GlobalEventId::CLOSEVIEW ), pDoc, Reference< frame::XController2 >( this ) ) );
    if ( !pView )
        SfxGetpApp()->NotifyEvent( SfxEventHint(SfxEventHintId::CloseDoc, GlobalEventConfig::GetEventName( GlobalEventId::CLOSEDOC ), pDoc) );

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
    m_pData->m_pViewShell = nullptr;
    if (rFrame.GetViewShell() == pShell)
    {
        // Enter registrations only allowed if we are the owner!
        if ( rFrame.GetFrame().OwnsBindings_Impl() )
            rFrame.GetBindings().ENTERREGISTRATIONS();
        rFrame.GetFrame().SetFrameInterface_Impl(  aXFrame );
        rFrame.GetFrame().DoClose_Impl();
    }
}


//  SfxBaseController -> XComponent


void SAL_CALL SfxBaseController::addEventListener( const Reference< lang::XEventListener >& aListener )
{
    m_pData->m_aListenerContainer.addInterface( cppu::UnoType<lang::XEventListener>::get(), aListener );
}


//  SfxBaseController -> XComponent


void SAL_CALL SfxBaseController::removeEventListener( const Reference< lang::XEventListener >& aListener )
{
    m_pData->m_aListenerContainer.removeInterface( cppu::UnoType<lang::XEventListener>::get(), aListener );
}

void SfxBaseController::ReleaseShell_Impl()
{
    SolarMutexGuard aGuard;
    if ( !m_pData->m_pViewShell )
        return;

    SfxObjectShell* pDoc = m_pData->m_pViewShell->GetObjectShell() ;
    Reference< frame::XModel > xModel = pDoc->GetModel();
    Reference < util::XCloseable > xCloseable( xModel, uno::UNO_QUERY );
    if ( xModel.is() )
    {
        xModel->disconnectController( this );
        if ( xCloseable.is() )
            xCloseable->removeCloseListener( m_pData->m_xCloseListener );
    }
    m_pData->m_pViewShell = nullptr;

    Reference < frame::XFrame > aXFrame;
    attachFrame( aXFrame );
}

void SfxBaseController::CopyLokViewCallbackFromFrameCreator()
{
    if (!m_pData->m_pViewShell)
        return;
    SfxLokCallbackInterface* pCallback = nullptr;
    if (m_pData->m_xFrame)
        if (auto xCreator = m_pData->m_xFrame->getCreator())
            if (auto parentVS = SfxViewShell::Get(xCreator->getController()))
                pCallback = parentVS->getLibreOfficeKitViewCallback();
    m_pData->m_pViewShell->setLibreOfficeKitViewCallback(pCallback);
}

SfxViewShell* SfxBaseController::GetViewShell_Impl() const
{
    return m_pData->m_pViewShell;
}

Reference< task::XStatusIndicator > SAL_CALL SfxBaseController::getStatusIndicator(  )
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell && !m_pData->m_xIndicator.is() )
        m_pData->m_xIndicator = new SfxStatusIndicator( this, m_pData->m_pViewShell->GetViewFrame().GetFrame().GetWorkWindow_Impl() );
    return m_pData->m_xIndicator;
}

void SAL_CALL SfxBaseController::registerContextMenuInterceptor( const Reference< ui::XContextMenuInterceptor >& xInterceptor )

{
    m_pData->m_aInterceptorContainer.addInterface( xInterceptor );

    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->AddContextMenuInterceptor_Impl( xInterceptor );
}

void SAL_CALL SfxBaseController::releaseContextMenuInterceptor( const Reference< ui::XContextMenuInterceptor >& xInterceptor )

{
    m_pData->m_aInterceptorContainer.removeInterface( xInterceptor );

    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->RemoveContextMenuInterceptor_Impl( xInterceptor );
}

void SAL_CALL SfxBaseController::addKeyHandler( const Reference< awt::XKeyHandler >& xHandler )
{
    SolarMutexGuard aGuard;
    m_pData->m_aUserInputInterception.addKeyHandler( xHandler );
}

void SAL_CALL SfxBaseController::removeKeyHandler( const Reference< awt::XKeyHandler >& xHandler )
{
    SolarMutexGuard aGuard;
    m_pData->m_aUserInputInterception.removeKeyHandler( xHandler );
}

void SAL_CALL SfxBaseController::addMouseClickHandler( const Reference< awt::XMouseClickHandler >& xHandler )
{
    SolarMutexGuard aGuard;
    m_pData->m_aUserInputInterception.addMouseClickHandler( xHandler );
}

void SAL_CALL SfxBaseController::removeMouseClickHandler( const Reference< awt::XMouseClickHandler >& xHandler )
{
    SolarMutexGuard aGuard;
    m_pData->m_aUserInputInterception.removeMouseClickHandler( xHandler );
}

uno::Sequence< sal_Int16 > SAL_CALL SfxBaseController::getSupportedCommandGroups()
{
    SolarMutexGuard aGuard;

    std::vector< sal_Int16 > aGroupList;
    SfxViewFrame* pViewFrame = m_pData->m_pViewShell ? m_pData->m_pViewShell->GetFrame() : nullptr;
    SfxSlotPool* pSlotPool = pViewFrame ? &SfxSlotPool::GetSlotPool(pViewFrame) : &SFX_SLOTPOOL();
    const SfxSlotMode nMode( SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::MENUCONFIG );

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

    return comphelper::containerToSequence( aGroupList );
}

uno::Sequence< frame::DispatchInformation > SAL_CALL SfxBaseController::getConfigurableDispatchInformation( sal_Int16 nCmdGroup )
{
    std::vector< frame::DispatchInformation > aCmdVector;

    SolarMutexGuard aGuard;
    if ( m_pData->m_pViewShell )
    {
        const SfxSlotMode nMode( SfxSlotMode::TOOLBOXCONFIG|SfxSlotMode::ACCELCONFIG|SfxSlotMode::MENUCONFIG );

        SfxViewFrame* pViewFrame( m_pData->m_pViewShell->GetFrame() );
        SfxSlotPool* pSlotPool
            = pViewFrame ? &SfxSlotPool::GetSlotPool(pViewFrame) : &SFX_SLOTPOOL();
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
                            aCmdInfo.Command = pSfxSlot->GetCommand();
                            aCmdInfo.GroupId = nCommandGroup;
                            aCmdVector.push_back( aCmdInfo );
                        }
                        pSfxSlot = pSlotPool->NextSlot();
                    }
                }
            }
        }
    }

    return comphelper::containerToSequence( aCmdVector );
}

bool SfxBaseController::HandleEvent_Impl( NotifyEvent const & rEvent )
{
    return m_pData->m_aUserInputInterception.handleNotifyEvent( rEvent );
}

bool SfxBaseController::HasKeyListeners_Impl() const
{
    return m_pData->m_aUserInputInterception.hasKeyHandlers();
}

bool SfxBaseController::HasMouseClickListeners_Impl() const
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
            if  (   ( m_pData->m_pViewShell->GetObjectShell() != nullptr )
                &&  ( m_pData->m_pViewShell->GetObjectShell()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
                )
            {
                SfxViewFrame& rViewFrm = m_pData->m_pViewShell->GetViewFrame();
                if ( !rViewFrm.GetFrame().IsInPlace() )
                {
                    // for outplace embedded objects, we want the layout manager to keep the content window
                    // size constant, if possible
                    try
                    {
                        Reference< beans::XPropertySet > xFrameProps( m_pData->m_xFrame, uno::UNO_QUERY_THROW );
                        Reference< beans::XPropertySet > xLayouterProps(
                            xFrameProps->getPropertyValue(u"LayoutManager"_ustr), uno::UNO_QUERY_THROW );
                        xLayouterProps->setPropertyValue(u"PreserveContentSize"_ustr, uno::Any( true ) );
                    }
                    catch (const uno::Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("sfx.view");
                    }
                }
            }
        }

        // upon DISCONNECT, we did *not* pop the shells from the stack (this is done elsewhere), so upon
        // RECONNECT, we're not allowed to push them
        if ( i_eConnect != E_RECONNECT )
        {
            pViewFrame->GetDispatcher()->Push( *m_pData->m_pViewShell );
            m_pData->m_pViewShell->PushSubShells_Impl();
            pViewFrame->GetDispatcher()->Flush();
        }

        vcl::Window* pEditWin = m_pData->m_pViewShell->GetWindow();
        if ( pEditWin )
            pEditWin->Show();

        if ( SfxViewFrame::Current() == pViewFrame )
            pViewFrame->GetDispatcher()->Update_Impl( true );

        vcl::Window* pFrameWin = &pViewFrame->GetWindow();
        if ( pFrameWin != &pViewFrame->GetFrame().GetWindow() )
            pFrameWin->Show();

        if ( i_eConnect == E_CONNECT )
        {
            css::uno::Reference<css::frame::XModel3> xModel(getModel(), css::uno::UNO_QUERY_THROW);
            const sal_Int16 nPluginMode = ::comphelper::NamedValueCollection::getOrDefault( xModel->getArgs2( { u"PluginMode"_ustr } ), u"PluginMode", sal_Int16( 0 ) );
            const bool bHasPluginMode = ( nPluginMode != 0 );

            SfxFrame& rFrame = pViewFrame->GetFrame();
            SfxObjectShell& rDoc = *m_pData->m_pViewShell->GetObjectShell();
            if ( !rFrame.IsMarkedHidden_Impl() )
            {
                if ( rDoc.IsHelpDocument() || ( nPluginMode == 2 ) )
                    pViewFrame->GetDispatcher()->HideUI();
                else
                    pViewFrame->GetDispatcher()->HideUI( false );

                if ( rFrame.IsInPlace() )
                    pViewFrame->LockAdjustPosSizePixel();

                if ( nPluginMode == 3 )
                    rFrame.GetWorkWindow_Impl()->SetInternalDockingAllowed( false );

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
                        pViewFrame->Resize( true );
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
                pViewFrame->Resize( true );

            ::comphelper::NamedValueCollection aViewArgs(getCreationArguments());

            // sometimes we want to avoid adding to the recent documents
            bool bAllowPickListEntry = aViewArgs.getOrDefault(u"PickListEntry"_ustr, true);
            m_pData->m_pViewShell->GetObjectShell()->AvoidRecentDocs(!bAllowPickListEntry);

            // if there's a JumpMark given, then, well, jump to it
            const OUString sJumpMark = aViewArgs.getOrDefault( u"JumpMark"_ustr, OUString() );
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
                // responsibility of the loader, not an implementation detail buried here deep within the controller's
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
                        OUString sViewId( aViewData.getOrDefault( u"ViewId"_ustr, OUString() ) );
                        if ( sViewId.isEmpty() )
                            continue;

                        const SfxViewFactory* pViewFactory = rDocFactory.GetViewFactoryByViewName( sViewId );
                        if ( pViewFactory == nullptr )
                            continue;

                        if ( pViewFactory->GetOrdinal() == pViewFrame->GetCurViewId() )
                        {
                            nViewDataIndex = i;
                            break;
                        }
                    }
                    if (nViewDataIndex < nCount || !xViewData.is())
                    {
                        Sequence< PropertyValue > aViewData;
                        if (xViewData.is())
                        {
                            OSL_VERIFY(xViewData->getByIndex(nViewDataIndex) >>= aViewData);
                        }
                        if (aViewData.hasElements() || !xViewData.is())
                        {
                            // Tolerate empty xViewData, ReadUserDataSequence() has side effects.
                            m_pData->m_pViewShell->ReadUserDataSequence( aViewData );
                        }
                    }
                }
                catch (const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("sfx.view");
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
    if ( !m_pData->m_pViewShell )
        return;

    // CMIS verifications
    Reference< document::XCmisDocument > xCmisDoc( m_pData->m_pViewShell->GetObjectShell()->GetModel(), uno::UNO_QUERY );
    if ( !xCmisDoc.is( ) || !xCmisDoc->canCheckOut( ) )
        return;

    const uno::Sequence< document::CmisProperty> aCmisProperties = xCmisDoc->getCmisProperties( );

    if ( !(xCmisDoc->isVersionable( ) && aCmisProperties.hasElements( )) )
        return;

    // Loop over the CMIS Properties to find cmis:isVersionSeriesCheckedOut
    // and find if it is a Google Drive file.
    bool bIsGoogleFile = false;
    bool bCheckedOut = false;
    for ( const auto& rCmisProp : aCmisProperties )
    {
        if ( rCmisProp.Id == "cmis:isVersionSeriesCheckedOut" ) {
            uno::Sequence< sal_Bool > bTmp;
            rCmisProp.Value >>= bTmp;
            bCheckedOut = bTmp[0];
        }
        // if it is a Google Drive file, we don't need the checkout bar,
        // still need the checkout feature for the version dialog.
        if ( rCmisProp.Name == "title" )
            bIsGoogleFile = true;
    }

    if ( bCheckedOut || bIsGoogleFile )
        return;

    // Get the Frame and show the InfoBar if not checked out
    SfxViewFrame* pViewFrame = m_pData->m_pViewShell->GetFrame();
    auto pInfoBar = pViewFrame->AppendInfoBar(u"checkout"_ustr, u""_ustr, SfxResId(STR_NONCHECKEDOUT_DOCUMENT),
                                              InfobarType::WARNING);
    if (pInfoBar)
    {
        weld::Button &rBtn = pInfoBar->addButton();
        rBtn.set_label(SfxResId(STR_CHECKOUT));
        rBtn.connect_clicked(LINK(this, SfxBaseController, CheckOutHandler));
    }
}

IMPL_LINK_NOARG ( SfxBaseController, CheckOutHandler, weld::Button&, void )
{
    if ( m_pData->m_pViewShell )
        m_pData->m_pViewShell->GetObjectShell()->CheckOut( );
}


Reference< frame::XTitle > SfxBaseController::impl_getTitleHelper ()
{
    SolarMutexGuard aGuard;

    if ( ! m_pData->m_xTitleHelper.is ())
    {
        Reference< frame::XModel >           xModel           = getModel ();
        Reference< frame::XUntitledNumbers > xUntitledProvider(xModel                                       , uno::UNO_QUERY      );

        m_pData->m_xTitleHelper = new ::framework::TitleHelper(::comphelper::getProcessComponentContext(),
                                        Reference< frame::XController >(this), xUntitledProvider);
    }

    return m_pData->m_xTitleHelper;
}


// frame::XTitle
OUString SAL_CALL SfxBaseController::getTitle()
{
    return impl_getTitleHelper()->getTitle ();
}


// frame::XTitle
void SAL_CALL SfxBaseController::setTitle(const OUString& sTitle)
{
    impl_getTitleHelper()->setTitle (sTitle);
}


// frame::XTitleChangeBroadcaster
void SAL_CALL SfxBaseController::addTitleChangeListener(const Reference< frame::XTitleChangeListener >& xListener)
{
    Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}


// frame::XTitleChangeBroadcaster
void SAL_CALL SfxBaseController::removeTitleChangeListener(const Reference< frame::XTitleChangeListener >& xListener)
{
    Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

void SfxBaseController::initialize( const css::uno::Sequence< css::uno::Any >& /*aArguments*/ )
{
}

void SAL_CALL SfxBaseController::appendInfobar(const OUString& sId, const OUString& sPrimaryMessage,
                                               const OUString& sSecondaryMessage,
                                               sal_Int32 aInfobarType,
                                               const Sequence<StringPair>& actionButtons,
                                               sal_Bool bShowCloseButton)
{
    SolarMutexGuard aGuard;

    if (aInfobarType < static_cast<sal_Int32>(InfobarType::INFO)
        || aInfobarType > static_cast<sal_Int32>(InfobarType::DANGER))
        throw lang::IllegalArgumentException("Undefined InfobarType: "
                                                 + OUString::number(aInfobarType),
                                             getXWeak(), 0);
    SfxViewFrame* pViewFrame = m_pData->m_pViewShell->GetFrame();
    if (pViewFrame->HasInfoBarWithID(sId))
        throw lang::IllegalArgumentException("Infobar with ID '" + sId + "' already existing.",
                                             getXWeak(), 0);

    auto pInfoBar
        = pViewFrame->AppendInfoBar(sId, sPrimaryMessage, sSecondaryMessage,
                                    static_cast<InfobarType>(aInfobarType), bShowCloseButton);
    if (!pInfoBar)
        throw uno::RuntimeException(u"Could not create Infobar"_ustr);

    for (const StringPair& actionButton : actionButtons)
    {
        if (actionButton.First.isEmpty() || actionButton.Second.isEmpty())
            continue;
        weld::Button& rBtn = pInfoBar->addButton(&actionButton.Second);
        rBtn.set_label(actionButton.First);
    }
}

void SAL_CALL SfxBaseController::updateInfobar(const OUString& sId, const OUString& sPrimaryMessage,
                                               const OUString& sSecondaryMessage,
                                               sal_Int32 aInfobarType)
{
    SolarMutexGuard aGuard;

    if (aInfobarType < static_cast<sal_Int32>(InfobarType::INFO)
        || aInfobarType > static_cast<sal_Int32>(InfobarType::DANGER))
        throw lang::IllegalArgumentException("Undefined InfobarType: "
                                                 + OUString::number(aInfobarType),
                                             getXWeak(), 0);
    SfxViewFrame* pViewFrame = m_pData->m_pViewShell->GetFrame();
    if (!pViewFrame->HasInfoBarWithID(sId))
        throw css::container::NoSuchElementException("Infobar with ID '" + sId + "' not found.");

    pViewFrame->UpdateInfoBar(sId, sPrimaryMessage, sSecondaryMessage,
                              static_cast<InfobarType>(aInfobarType));
}

void SAL_CALL SfxBaseController::removeInfobar(const OUString& sId)
{
    SolarMutexGuard aGuard;

    SfxViewFrame* pViewFrame = m_pData->m_pViewShell->GetFrame();
    if (!pViewFrame->HasInfoBarWithID(sId))
        throw css::container::NoSuchElementException("Infobar with ID '" + sId + "' not found.");
    pViewFrame->RemoveInfoBar(sId);
}

sal_Bool SAL_CALL SfxBaseController::hasInfobar(const OUString& sId)
{
    SolarMutexGuard aGuard;
    SfxViewFrame* pViewFrame = m_pData->m_pViewShell->GetFrame();
    return pViewFrame->HasInfoBarWithID(sId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
