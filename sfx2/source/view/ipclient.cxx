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

#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XStateChangeListener.hpp>
#include <com/sun/star/embed/StateChangeInProgressException.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/task/StatusIndicatorFactory.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <svtools/embedhlp.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/ipclient.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include "workwin.hxx"
#include "guisaveas.hxx"
#include <cppuhelper/implbase.hxx>
#include <svtools/ehdl.hxx>

#include <vcl/timer.hxx>
#include <vcl/window.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <svl/rectitem.hxx>
#include <svtools/soerr.hxx>
#include <comphelper/processfactory.hxx>

#define SFX_CLIENTACTIVATE_TIMEOUT 100

using namespace com::sun::star;


// SfxEmbedResizeGuard
class SfxBooleanFlagGuard
{
    bool& m_rFlag;
    bool  m_bLifeValue;
public:
    SfxBooleanFlagGuard( bool& bFlag, bool bLifeValue )
    : m_rFlag( bFlag )
    , m_bLifeValue( bLifeValue )
    {
        m_rFlag = m_bLifeValue;
    }

    ~SfxBooleanFlagGuard()
    {
        m_rFlag = !m_bLifeValue;
    }
};


// SfxInPlaceClient_Impl


class SfxInPlaceClient_Impl : public ::cppu::WeakImplHelper< embed::XEmbeddedClient,
                                                              embed::XInplaceClient,
                                                              document::XEventListener,
                                                              embed::XStateChangeListener,
                                                              embed::XWindowSupplier >
{
public:
    Timer                           m_aTimer;               // activation timeout, starts after object connection
    Rectangle                       m_aObjArea;             // area of object in coordinate system of the container (without scaling)
    Fraction                        m_aScaleWidth;          // scaling that was applied to the object when it was not active
    Fraction                        m_aScaleHeight;
    SfxInPlaceClient*               m_pClient;
    sal_Int64                       m_nAspect;              // ViewAspect that is assigned from the container
    Rectangle                       m_aLastObjAreaPixel;    // area of object in coordinate system of the container (without scaling)
    bool                            m_bStoreObject;
    bool                            m_bUIActive;            // set and cleared when notification for UI (de)activation is sent
    bool                            m_bResizeNoScale;

    uno::Reference < embed::XEmbeddedObject > m_xObject;
    uno::Reference < embed::XEmbeddedClient > m_xClient;


    SfxInPlaceClient_Impl()
    : m_pClient( NULL )
    , m_nAspect( 0 )
    , m_bStoreObject( true )
    , m_bUIActive( false )
    , m_bResizeNoScale( false )
    {}

    virtual ~SfxInPlaceClient_Impl();

    void SizeHasChanged();
    DECL_LINK_TYPED(TimerHdl, Timer *, void);
    uno::Reference < frame::XFrame > GetFrame() const;

    // XEmbeddedClient
    virtual void SAL_CALL saveObject() throw ( embed::ObjectSaveVetoException, uno::Exception, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL visibilityChanged( sal_Bool bVisible ) throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;

    // XInplaceClient
    virtual sal_Bool SAL_CALL canInplaceActivate() throw ( uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL activatingInplace() throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL activatingUI() throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL deactivatedInplace() throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL deactivatedUI() throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual uno::Reference< css::frame::XLayoutManager > SAL_CALL getLayoutManager() throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual uno::Reference< frame::XDispatchProvider > SAL_CALL getInplaceDispatchProvider() throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual awt::Rectangle SAL_CALL getPlacement() throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual awt::Rectangle SAL_CALL getClipRectangle() throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL translateAccelerators( const uno::Sequence< awt::KeyEvent >& aKeys ) throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL scrollObject( const awt::Size& aOffset ) throw ( embed::WrongStateException, uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL changedPlacement( const awt::Rectangle& aPosRect ) throw ( embed::WrongStateException, uno::Exception, uno::RuntimeException, std::exception ) override;

    // XComponentSupplier
    virtual uno::Reference< util::XCloseable > SAL_CALL getComponent() throw ( uno::RuntimeException, std::exception ) override;

    // XWindowSupplier
    virtual uno::Reference< awt::XWindow > SAL_CALL getWindow() throw ( uno::RuntimeException, std::exception ) override;

    // document::XEventListener
    virtual void SAL_CALL       notifyEvent( const document::EventObject& aEvent ) throw( uno::RuntimeException, std::exception ) override;

    // XStateChangeListener
    virtual void SAL_CALL changingState( const css::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (css::embed::WrongStateException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL stateChanged( const css::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;
};

SfxInPlaceClient_Impl::~SfxInPlaceClient_Impl()
{
}

void SAL_CALL SfxInPlaceClient_Impl::changingState(
    const css::lang::EventObject& /*aEvent*/,
    ::sal_Int32 /*nOldState*/,
    ::sal_Int32 /*nNewState*/ )
throw (css::embed::WrongStateException, css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SfxInPlaceClient_Impl::stateChanged(
    const css::lang::EventObject& /*aEvent*/,
    ::sal_Int32 nOldState,
    ::sal_Int32 nNewState )
throw (css::uno::RuntimeException, std::exception)
{
    if ( m_pClient && nOldState != embed::EmbedStates::LOADED && nNewState == embed::EmbedStates::RUNNING )
    {
        // deactivation of object
        uno::Reference< frame::XModel > xDocument;
        if ( m_pClient->GetViewShell()->GetObjectShell() )
            xDocument = m_pClient->GetViewShell()->GetObjectShell()->GetModel();
        SfxObjectShell::SetCurrentComponent( xDocument );
    }
}

void SAL_CALL SfxInPlaceClient_Impl::notifyEvent( const document::EventObject& aEvent ) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    if ( m_pClient && aEvent.EventName == "OnVisAreaChanged" && m_nAspect != embed::Aspects::MSOLE_ICON )
    {
        m_pClient->FormatChanged(); // for Writer when format of the object is changed with the area
        m_pClient->ViewChanged();
        m_pClient->Invalidate();
    }
}

void SAL_CALL SfxInPlaceClient_Impl::disposing( const css::lang::EventObject& /*aEvent*/ )
throw (css::uno::RuntimeException, std::exception)
{
    DELETEZ( m_pClient );
}

// XEmbeddedClient

uno::Reference < frame::XFrame > SfxInPlaceClient_Impl::GetFrame() const
{
    if ( !m_pClient )
        throw uno::RuntimeException();
    return m_pClient->GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();
}

void SAL_CALL SfxInPlaceClient_Impl::saveObject()
    throw ( embed::ObjectSaveVetoException,
            uno::Exception,
            uno::RuntimeException, std::exception )
{
    if ( !m_bStoreObject )
        // client wants to discard the object (usually it means the container document is closed while an object is active
        // and the user didn't request saving the changes
        return;

    // the common persistence is supported by objects and links
    uno::Reference< embed::XCommonEmbedPersist > xPersist( m_xObject, uno::UNO_QUERY );
    if ( !xPersist.is() )
        throw uno::RuntimeException();

    uno::Reference< frame::XFrame >              xFrame;
    uno::Reference< task::XStatusIndicator >     xStatusIndicator;
    uno::Reference< frame::XModel >              xModel( m_xObject->getComponent(), uno::UNO_QUERY );
    uno::Reference< uno::XComponentContext >     xContext( ::comphelper::getProcessComponentContext() );

    if ( xModel.is() )
    {
        uno::Reference< frame::XController > xController = xModel->getCurrentController();
        if ( xController.is() )
            xFrame = xController->getFrame();
    }

    if ( xFrame.is() )
    {
        // set non-reschedule progress to prevent problems when asynchronous calls are made
        // during storing of the embedded object
        uno::Reference< task::XStatusIndicatorFactory > xStatusIndicatorFactory =
               task::StatusIndicatorFactory::createWithFrame( xContext, xFrame, sal_True/*DisableReschedule*/, sal_False/*AllowParentShow*/ );

        uno::Reference< beans::XPropertySet > xPropSet( xFrame, uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                xStatusIndicator = xStatusIndicatorFactory->createStatusIndicator();
                xPropSet->setPropertyValue( "IndicatorInterception" , uno::makeAny( xStatusIndicator ));
            }
            catch ( const uno::RuntimeException& )
            {
                throw;
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    try
    {
        xPersist->storeOwn();
        m_xObject->update();
    }
    catch ( uno::Exception& )
    {
        //TODO/LATER: what should happen if object can't be saved?!
    }

    // reset status indicator interception after storing
    try
    {
        uno::Reference< beans::XPropertySet > xPropSet( xFrame, uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            xStatusIndicator.clear();
            xPropSet->setPropertyValue( "IndicatorInterception" , uno::makeAny( xStatusIndicator ));
        }
    }
    catch ( const uno::RuntimeException& )
    {
        throw;
    }
    catch ( uno::Exception& )
    {
    }

    // the client can exist only in case there is a view shell
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    SfxObjectShell* pDocShell = m_pClient->GetViewShell()->GetObjectShell();
    if ( !pDocShell )
        throw uno::RuntimeException();

    pDocShell->SetModified();

    //TODO/LATER: invalidation might be necessary when object was modified, but is not
    //saved through this method
    // m_pClient->Invalidate();
}


void SAL_CALL SfxInPlaceClient_Impl::visibilityChanged( sal_Bool bVisible )
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    m_pClient->GetViewShell()->OutplaceActivated( bVisible, m_pClient );
    m_pClient->Invalidate();
}


// XInplaceClient

sal_Bool SAL_CALL SfxInPlaceClient_Impl::canInplaceActivate()
    throw ( uno::RuntimeException, std::exception )
{
    if ( !m_xObject.is() )
        throw uno::RuntimeException();

    // we don't want to switch directly from outplace to inplace mode
    if ( m_xObject->getCurrentState() == embed::EmbedStates::ACTIVE || m_nAspect == embed::Aspects::MSOLE_ICON )
        return sal_False;

    return sal_True;
}


void SAL_CALL SfxInPlaceClient_Impl::activatingInplace()
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();
}


void SAL_CALL SfxInPlaceClient_Impl::activatingUI()
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    m_pClient->GetViewShell()->ResetAllClients_Impl(m_pClient);
    m_bUIActive = true;
    m_pClient->GetViewShell()->UIActivating( m_pClient );
}


void SAL_CALL SfxInPlaceClient_Impl::deactivatedInplace()
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();
}


void SAL_CALL SfxInPlaceClient_Impl::deactivatedUI()
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    m_pClient->GetViewShell()->UIDeactivated( m_pClient );
    m_bUIActive = false;
}


uno::Reference< css::frame::XLayoutManager > SAL_CALL SfxInPlaceClient_Impl::getLayoutManager()
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    uno::Reference < beans::XPropertySet > xFrame( GetFrame(), uno::UNO_QUERY );
    if ( !xFrame.is() )
        throw uno::RuntimeException();

    uno::Reference< css::frame::XLayoutManager > xMan;
    try
    {
        uno::Any aAny = xFrame->getPropertyValue( "LayoutManager" );
        aAny >>= xMan;
    }
    catch ( uno::Exception& )
    {
        throw uno::RuntimeException();
    }

    return xMan;
}


uno::Reference< frame::XDispatchProvider > SAL_CALL SfxInPlaceClient_Impl::getInplaceDispatchProvider()
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    return uno::Reference < frame::XDispatchProvider >( GetFrame(), uno::UNO_QUERY_THROW );
}


awt::Rectangle SAL_CALL SfxInPlaceClient_Impl::getPlacement()
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    // apply scaling to object area and convert to pixels
    Rectangle aRealObjArea( m_aObjArea );
    aRealObjArea.SetSize( Size( Fraction( aRealObjArea.GetWidth() ) * m_aScaleWidth,
                                Fraction( aRealObjArea.GetHeight() ) * m_aScaleHeight ) );

    aRealObjArea = m_pClient->GetEditWin()->LogicToPixel( aRealObjArea );
    return AWTRectangle( aRealObjArea );
}


awt::Rectangle SAL_CALL SfxInPlaceClient_Impl::getClipRectangle()
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    // currently(?) same as placement
    Rectangle aRealObjArea( m_aObjArea );
    aRealObjArea.SetSize( Size( Fraction( aRealObjArea.GetWidth() ) * m_aScaleWidth,
                                Fraction( aRealObjArea.GetHeight() ) * m_aScaleHeight ) );

    aRealObjArea = m_pClient->GetEditWin()->LogicToPixel( aRealObjArea );
    return AWTRectangle( aRealObjArea );
}


void SAL_CALL SfxInPlaceClient_Impl::translateAccelerators( const uno::Sequence< awt::KeyEvent >& /*aKeys*/ )
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    // TODO/MBA: keyboard accelerators
}


void SAL_CALL SfxInPlaceClient_Impl::scrollObject( const awt::Size& /*aOffset*/ )
    throw ( embed::WrongStateException,
            uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();
}


void SAL_CALL SfxInPlaceClient_Impl::changedPlacement( const awt::Rectangle& aPosRect )
    throw ( embed::WrongStateException,
            uno::Exception,
            uno::RuntimeException, std::exception )
{
    uno::Reference< embed::XInplaceObject > xInplace( m_xObject, uno::UNO_QUERY );
    if ( !xInplace.is() || !m_pClient || !m_pClient->GetEditWin() || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    // check if the change is at least one pixel in size
    awt::Rectangle aOldRect = getPlacement();
    Rectangle aNewPixelRect = VCLRectangle( aPosRect );
    Rectangle aOldPixelRect = VCLRectangle( aOldRect );
    if ( aOldPixelRect == aNewPixelRect )
        // nothing has changed
        return;

    // new scaled object area
    Rectangle aNewLogicRect = m_pClient->GetEditWin()->PixelToLogic( aNewPixelRect );

    // all the size changes in this method should happen without scaling
    // SfxBooleanFlagGuard aGuard( m_bResizeNoScale, sal_True );

    // allow container to apply restrictions on the requested new area;
    // the container might change the object view during size calculation;
    // currently only writer does it
    m_pClient->RequestNewObjectArea( aNewLogicRect);

    if ( aNewLogicRect != m_pClient->GetScaledObjArea() )
    {
        // the calculation of the object area has not changed the object size
        // it should be done here then
        SfxBooleanFlagGuard aGuard( m_bResizeNoScale, true );

        // new size of the object area without scaling
        Size aNewObjSize( Fraction( aNewLogicRect.GetWidth() ) / m_aScaleWidth,
                          Fraction( aNewLogicRect.GetHeight() ) / m_aScaleHeight );

        // now remove scaling from new placement and keep this a the new object area
        aNewLogicRect.SetSize( aNewObjSize );
        m_aObjArea = aNewLogicRect;

        // let the window size be recalculated
        SizeHasChanged();
    }

    // notify container view about changes
    m_pClient->ObjectAreaChanged();
}

// XComponentSupplier

uno::Reference< util::XCloseable > SAL_CALL SfxInPlaceClient_Impl::getComponent()
    throw ( uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    SfxObjectShell* pDocShell = m_pClient->GetViewShell()->GetObjectShell();
    if ( !pDocShell )
        throw uno::RuntimeException();

    // all the components must implement XCloseable
    uno::Reference< util::XCloseable > xComp( pDocShell->GetModel(), uno::UNO_QUERY );
    if ( !xComp.is() )
        throw uno::RuntimeException();

    return xComp;
}


// XWindowSupplier

uno::Reference< awt::XWindow > SAL_CALL SfxInPlaceClient_Impl::getWindow()
    throw ( uno::RuntimeException, std::exception )
{
    if ( !m_pClient || !m_pClient->GetEditWin() )
        throw uno::RuntimeException();

    uno::Reference< awt::XWindow > xWin( m_pClient->GetEditWin()->GetComponentInterface(), uno::UNO_QUERY );
    return xWin;
}


// notification to the client implementation that either the object area or the scaling has been changed
// as a result the logical size of the window has changed also
void SfxInPlaceClient_Impl::SizeHasChanged()
{
    if ( !m_pClient || !m_pClient->GetViewShell() )
        throw uno::RuntimeException();

    try {
        if ( m_xObject.is()
          && ( m_xObject->getCurrentState() == embed::EmbedStates::INPLACE_ACTIVE
                || m_xObject->getCurrentState() == embed::EmbedStates::UI_ACTIVE ) )
        {
            // only possible in active states
            uno::Reference< embed::XInplaceObject > xInplace( m_xObject, uno::UNO_QUERY );
            if ( !xInplace.is() )
                throw uno::RuntimeException();

            if ( m_bResizeNoScale )
            {
                // the resizing should be done without scaling
                // set the correct size to the object to avoid the scaling
                MapMode aObjectMap( VCLUnoHelper::UnoEmbed2VCLMapUnit( m_xObject->getMapUnit( m_nAspect ) ) );
                MapMode aClientMap( m_pClient->GetEditWin()->GetMapMode().GetMapUnit() );

                // convert to logical coordinates of the embedded object
                Size aNewSize = m_pClient->GetEditWin()->LogicToLogic( m_aObjArea.GetSize(), &aClientMap, &aObjectMap );
                m_xObject->setVisualAreaSize( m_nAspect, awt::Size( aNewSize.Width(), aNewSize.Height() ) );
            }

            xInplace->setObjectRectangles( getPlacement(), getClipRectangle() );
        }
    }
    catch( uno::Exception& )
    {
        // TODO/LATER: handle error
    }
}


IMPL_LINK_NOARG_TYPED(SfxInPlaceClient_Impl, TimerHdl, Timer *, void)
{
    if ( m_pClient && m_xObject.is() )
        m_pClient->GetViewShell()->CheckIPClient_Impl( m_pClient, m_pClient->GetViewShell()->GetObjectShell()->GetVisArea() );
}



// SfxInPlaceClient


SfxInPlaceClient::SfxInPlaceClient( SfxViewShell* pViewShell, vcl::Window *pDraw, sal_Int64 nAspect ) :
    m_pImp( new SfxInPlaceClient_Impl ),
    m_pViewSh( pViewShell ),
    m_pEditWin( pDraw )
{
    m_pImp->acquire();
    m_pImp->m_pClient = this;
    m_pImp->m_nAspect = nAspect;
    m_pImp->m_aScaleWidth = m_pImp->m_aScaleHeight = Fraction(1,1);
    m_pImp->m_xClient = static_cast< embed::XEmbeddedClient* >( m_pImp );
    pViewShell->NewIPClient_Impl(this);
    m_pImp->m_aTimer.SetTimeout( SFX_CLIENTACTIVATE_TIMEOUT );
    m_pImp->m_aTimer.SetTimeoutHdl( LINK( m_pImp, SfxInPlaceClient_Impl, TimerHdl ) );
}



SfxInPlaceClient::~SfxInPlaceClient()
{
    m_pViewSh->IPClientGone_Impl(this);

    // deleting the client before storing the object means discarding all changes
    m_pImp->m_bStoreObject = false;
    SetObject(0);

    m_pImp->m_pClient = NULL;

    // the next call will destroy m_pImp if no other reference to it exists
    m_pImp->m_xClient = uno::Reference < embed::XEmbeddedClient >();
    m_pImp->release();

    // TODO/LATER:
    // the class is not intended to be used in multithreaded environment;
    // if it will this disconnection and all the parts that use the m_pClient
    // must be guarded with mutex
}


void SfxInPlaceClient::SetObjectState( sal_Int32 nState )
{
    if ( GetObject().is() )
    {
        if ( m_pImp->m_nAspect == embed::Aspects::MSOLE_ICON
          && ( nState == embed::EmbedStates::UI_ACTIVE || nState == embed::EmbedStates::INPLACE_ACTIVE ) )
        {
            OSL_FAIL( "Iconified object should not be activated inplace!\n" );
            return;
        }

        try
        {
            GetObject()->changeState( nState );
        }
        catch ( uno::Exception& )
        {}
    }
}


sal_Int64 SfxInPlaceClient::GetObjectMiscStatus() const
{
    if ( GetObject().is() )
        return GetObject()->getStatus( m_pImp->m_nAspect );
    return 0;
}


uno::Reference < embed::XEmbeddedObject > SfxInPlaceClient::GetObject() const
{
    return m_pImp->m_xObject;
}


void SfxInPlaceClient::SetObject( const uno::Reference < embed::XEmbeddedObject >& rObject )
{
    if ( m_pImp->m_xObject.is() && rObject != m_pImp->m_xObject )
    {
        DBG_ASSERT( GetObject()->getClientSite() == m_pImp->m_xClient, "Wrong ClientSite!" );
        if ( GetObject()->getClientSite() == m_pImp->m_xClient )
        {
            if ( GetObject()->getCurrentState() != embed::EmbedStates::LOADED )
                SetObjectState( embed::EmbedStates::RUNNING );
            m_pImp->m_xObject->removeEventListener( uno::Reference < document::XEventListener >( m_pImp->m_xClient, uno::UNO_QUERY ) );
            m_pImp->m_xObject->removeStateChangeListener( uno::Reference < embed::XStateChangeListener >( m_pImp->m_xClient, uno::UNO_QUERY ) );
            try
            {
                m_pImp->m_xObject->setClientSite( 0 );
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( "Can not clean the client site!\n" );
            }
        }
    }

    if ( m_pViewSh->GetViewFrame()->GetFrame().IsClosing_Impl() )
        // sometimes applications reconnect clients on shutting down because it happens in their Paint methods
        return;

    m_pImp->m_xObject = rObject;

    if ( rObject.is() )
    {
        // as soon as an object was connected to a client it has to be checked whether the object wants
        // to be activated
        rObject->addStateChangeListener( uno::Reference < embed::XStateChangeListener >( m_pImp->m_xClient, uno::UNO_QUERY ) );
        rObject->addEventListener( uno::Reference < document::XEventListener >( m_pImp->m_xClient, uno::UNO_QUERY ) );

        try
        {
            rObject->setClientSite( m_pImp->m_xClient );
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Can not set the client site!\n" );
        }

        m_pImp->m_aTimer.Start();
    }
    else
        m_pImp->m_aTimer.Stop();
}


bool SfxInPlaceClient::SetObjArea( const Rectangle& rArea )
{
    if( rArea != m_pImp->m_aObjArea )
    {
        m_pImp->m_aObjArea = rArea;
        m_pImp->SizeHasChanged();

        Invalidate();
        return true;
    }

    return false;
}


Rectangle SfxInPlaceClient::GetObjArea() const
{
    return m_pImp->m_aObjArea;
}

Rectangle SfxInPlaceClient::GetScaledObjArea() const
{
    Rectangle aRealObjArea( m_pImp->m_aObjArea );
    aRealObjArea.SetSize( Size( Fraction( aRealObjArea.GetWidth() ) * m_pImp->m_aScaleWidth,
                                Fraction( aRealObjArea.GetHeight() ) * m_pImp->m_aScaleHeight ) );
    return aRealObjArea;
}


void SfxInPlaceClient::SetSizeScale( const Fraction & rScaleWidth, const Fraction & rScaleHeight )
{
    if ( m_pImp->m_aScaleWidth != rScaleWidth || m_pImp->m_aScaleHeight != rScaleHeight )
    {
        m_pImp->m_aScaleWidth = rScaleWidth;
        m_pImp->m_aScaleHeight = rScaleHeight;

        m_pImp->SizeHasChanged();

        // TODO/LATER: Invalidate seems to trigger (wrong) recalculations of the ObjArea, so it's better
        // not to call it here, but maybe it sounds reasonable to do so.
        //Invalidate();
    }
}


bool SfxInPlaceClient::SetObjAreaAndScale( const Rectangle& rArea, const Fraction& rScaleWidth, const Fraction& rScaleHeight )
{
    if( rArea != m_pImp->m_aObjArea || m_pImp->m_aScaleWidth != rScaleWidth || m_pImp->m_aScaleHeight != rScaleHeight )
    {
        m_pImp->m_aObjArea = rArea;
        m_pImp->m_aScaleWidth = rScaleWidth;
        m_pImp->m_aScaleHeight = rScaleHeight;

        m_pImp->SizeHasChanged();

        Invalidate();
        return true;
    }

    return false;
}


const Fraction& SfxInPlaceClient::GetScaleWidth() const
{
    return m_pImp->m_aScaleWidth;
}


const Fraction& SfxInPlaceClient::GetScaleHeight() const
{
    return m_pImp->m_aScaleHeight;
}


void SfxInPlaceClient::Invalidate()
{
    // TODO/LATER: do we need both?

    // the object area is provided in logical coordinates of the window but without scaling applied
    Rectangle aRealObjArea( m_pImp->m_aObjArea );
    aRealObjArea.SetSize( Size( Fraction( aRealObjArea.GetWidth() ) * m_pImp->m_aScaleWidth,
                                Fraction( aRealObjArea.GetHeight() ) * m_pImp->m_aScaleHeight ) );
    m_pEditWin->Invalidate( aRealObjArea );

    ViewChanged();
}


bool SfxInPlaceClient::IsObjectUIActive() const
{
    try {
        return ( m_pImp->m_xObject.is() && ( m_pImp->m_xObject->getCurrentState() == embed::EmbedStates::UI_ACTIVE ) );
    }
    catch( uno::Exception& )
    {}

    return false;
}


bool SfxInPlaceClient::IsObjectInPlaceActive() const
{
    try {
        return(
               (
                m_pImp->m_xObject.is() &&
                (m_pImp->m_xObject->getCurrentState() == embed::EmbedStates::INPLACE_ACTIVE)
               ) ||
               (
                m_pImp->m_xObject.is() &&
                (m_pImp->m_xObject->getCurrentState() == embed::EmbedStates::UI_ACTIVE)
               )
              );
    }
    catch( uno::Exception& )
    {}

    return false;
}


SfxInPlaceClient* SfxInPlaceClient::GetClient( SfxObjectShell* pDoc, const css::uno::Reference < css::embed::XEmbeddedObject >& xObject )
{
    for ( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(pDoc); pFrame; pFrame=SfxViewFrame::GetNext(*pFrame,pDoc) )
    {
        if( pFrame->GetViewShell() )
        {
            SfxInPlaceClient* pClient = pFrame->GetViewShell()->FindIPClient( xObject, NULL );
            if ( pClient )
                return pClient;
        }
    }

    return NULL;
}

sal_Int64 SfxInPlaceClient::GetAspect() const
{
    return m_pImp->m_nAspect;
}

ErrCode SfxInPlaceClient::DoVerb( long nVerb )
{
    SfxErrorContext aEc( ERRCTX_SO_DOVERB, m_pViewSh->GetWindow(), RID_SO_ERRCTX );
    ErrCode nError = ERRCODE_NONE;

    if ( m_pImp->m_xObject.is() )
    {
        bool bSaveCopyAs = false;
        if ( nVerb == -8 ) // "Save Copy as..."
        {
            svt::EmbeddedObjectRef::TryRunningState( m_pImp->m_xObject );
            // TODO/LATER: this special verb should disappear when outplace activation is completely available
            uno::Reference< frame::XModel > xEmbModel( m_pImp->m_xObject->getComponent(), uno::UNO_QUERY );
            if ( xEmbModel.is() )
            {
                bSaveCopyAs = true;

                try
                {
                    SfxStoringHelper aHelper;
                    uno::Sequence< beans::PropertyValue > aDispatchArgs( 1 );
                    aDispatchArgs[0].Name = "SaveTo";
                    aDispatchArgs[0].Value <<= true;

                    aHelper.GUIStoreModel( xEmbModel,
                                            "SaveAs",
                                            aDispatchArgs,
                                            false,
                                            "" );
                }
                catch( const task::ErrorCodeIOException& aErrorEx )
                {
                    nError = (sal_uInt32)aErrorEx.ErrCode;
                }
                catch( uno::Exception& )
                {
                    nError = ERRCODE_IO_GENERAL;
                    // TODO/LATER: better error handling
                }
            }
        }

        if ( !bSaveCopyAs )
        {
            if ( m_pImp->m_nAspect == embed::Aspects::MSOLE_ICON )
            {
                if ( nVerb == embed::EmbedVerbs::MS_OLEVERB_PRIMARY || nVerb == embed::EmbedVerbs::MS_OLEVERB_SHOW )
                    nVerb = embed::EmbedVerbs::MS_OLEVERB_OPEN; // outplace activation
                else if ( nVerb == embed::EmbedVerbs::MS_OLEVERB_UIACTIVATE
                       || nVerb == embed::EmbedVerbs::MS_OLEVERB_IPACTIVATE )
                    nError = ERRCODE_SO_GENERALERROR;
            }

            if ( !nError )
            {

                m_pViewSh->GetViewFrame()->GetTopFrame().LockResize_Impl(true);
                try
                {
                    m_pImp->m_xObject->setClientSite( m_pImp->m_xClient );

                    m_pImp->m_xObject->doVerb( nVerb );
                }
                catch ( embed::UnreachableStateException& )
                {
                    if ( nVerb == 0 || nVerb == embed::EmbedVerbs::MS_OLEVERB_OPEN )
                    {
                        // a workaround for the default verb, usually makes sense for alien objects
                        try
                        {
                            m_pImp->m_xObject->doVerb( -9 ); // open own view, a workaround verb that is not visible

                            if ( m_pImp->m_xObject->getCurrentState() == embed::EmbedStates::UI_ACTIVE )
                            {
                                // the object was converted to OOo object
                                awt::Size aSize = m_pImp->m_xObject->getVisualAreaSize( m_pImp->m_nAspect );
                                MapMode aObjectMap( VCLUnoHelper::UnoEmbed2VCLMapUnit( m_pImp->m_xObject->getMapUnit( m_pImp->m_nAspect ) ) );
                                MapMode aClientMap( GetEditWin()->GetMapMode().GetMapUnit() );
                                Size aNewSize = GetEditWin()->LogicToLogic( Size( aSize.Width, aSize.Height ), &aObjectMap, &aClientMap );

                                Rectangle aScaledArea = GetScaledObjArea();
                                m_pImp->m_aObjArea.SetSize( aNewSize );
                                m_pImp->m_aScaleWidth = Fraction( aScaledArea.GetWidth(), aNewSize.Width() );
                                m_pImp->m_aScaleHeight = Fraction( aScaledArea.GetHeight(), aNewSize.Height() );
                            }
                        }
                        catch (uno::Exception const& e)
                        {
                            SAL_WARN("embeddedobj", "SfxInPlaceClient::DoVerb:"
                                " -9 fallback path: exception caught: "
                                << e.Message);
                            nError = ERRCODE_SO_GENERALERROR;
                        }
                    }
                }
                catch ( embed::StateChangeInProgressException& )
                {
                    // TODO/LATER: it would be nice to be able to provide the current target state outside
                    nError = ERRCODE_SO_CANNOT_DOVERB_NOW;
                }
                catch (uno::Exception const& e)
                {
                    SAL_WARN("embeddedobj", "SfxInPlaceClient::DoVerb:"
                            " exception caught: " << e.Message);
                    nError = ERRCODE_SO_GENERALERROR;
                    //TODO/LATER: better error handling
                }

                SfxViewFrame* pFrame = m_pViewSh->GetViewFrame();
                pFrame->GetTopFrame().LockResize_Impl(false);
                pFrame->GetTopFrame().Resize();
            }
        }
    }

    if( nError )
        ErrorHandler::HandleError( nError );

    return nError;
}

void SfxInPlaceClient::VisAreaChanged()
{
    uno::Reference < embed::XInplaceObject > xObj( m_pImp->m_xObject, uno::UNO_QUERY );
    uno::Reference < embed::XInplaceClient > xClient( m_pImp->m_xClient, uno::UNO_QUERY );
    if ( xObj.is() && xClient.is() )
        m_pImp->SizeHasChanged();
}

void SfxInPlaceClient::ObjectAreaChanged()
{
    // dummy implementation
}

void SfxInPlaceClient::RequestNewObjectArea( Rectangle& )
{
    // dummy implementation
}

void SfxInPlaceClient::ViewChanged()
{
    // dummy implementation
}

void SfxInPlaceClient::FormatChanged()
{
    // dummy implementation
}

void SfxInPlaceClient::DeactivateObject()
{
    if ( GetObject().is() )
    {
        try
        {
            m_pImp->m_bUIActive = false;
            bool bHasFocus = false;
            uno::Reference< frame::XModel > xModel( m_pImp->m_xObject->getComponent(), uno::UNO_QUERY );
            if ( xModel.is() )
            {
                uno::Reference< frame::XController > xController = xModel->getCurrentController();
                if ( xController.is() )
                {
                    vcl::Window* pWindow = VCLUnoHelper::GetWindow( xController->getFrame()->getContainerWindow() );
                    bHasFocus = pWindow->HasChildPathFocus( true );
                }
            }

            m_pViewSh->GetViewFrame()->GetTopFrame().LockResize_Impl(true);

            if ( (m_pImp->m_xObject->getStatus( m_pImp->m_nAspect ) & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE) ||
                 svt::EmbeddedObjectRef::IsGLChart(m_pImp->m_xObject) )
            {
                m_pImp->m_xObject->changeState( embed::EmbedStates::INPLACE_ACTIVE );
                if (bHasFocus)
                    m_pViewSh->GetWindow()->GrabFocus();
            }
            else
            {
                // the links should not stay in running state for long time because of locking
                uno::Reference< embed::XLinkageSupport > xLink( m_pImp->m_xObject, uno::UNO_QUERY );
                if ( xLink.is() && xLink->isLink() )
                    m_pImp->m_xObject->changeState( embed::EmbedStates::LOADED );
                else
                    m_pImp->m_xObject->changeState( embed::EmbedStates::RUNNING );
            }

            SfxViewFrame* pFrame = m_pViewSh->GetViewFrame();
            SfxViewFrame::SetViewFrame( pFrame );
            pFrame->GetTopFrame().LockResize_Impl(false);
            pFrame->GetTopFrame().Resize();
        }
        catch (css::uno::Exception& )
        {}
    }
}

void SfxInPlaceClient::ResetObject()
{
    if ( GetObject().is() )
    {
        try
        {
            m_pImp->m_bUIActive = false;
            if ( (m_pImp->m_xObject->getStatus( m_pImp->m_nAspect ) & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE) ||
                svt::EmbeddedObjectRef::IsGLChart(m_pImp->m_xObject) )
                m_pImp->m_xObject->changeState( embed::EmbedStates::INPLACE_ACTIVE );
            else
            {
                // the links should not stay in running state for long time because of locking
                uno::Reference< embed::XLinkageSupport > xLink( m_pImp->m_xObject, uno::UNO_QUERY );
                if ( xLink.is() && xLink->isLink() )
                    m_pImp->m_xObject->changeState( embed::EmbedStates::LOADED );
                else
                    m_pImp->m_xObject->changeState( embed::EmbedStates::RUNNING );
            }
        }
        catch (css::uno::Exception& )
        {}
    }
}

bool SfxInPlaceClient::IsUIActive()
{
    return m_pImp->m_bUIActive;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
