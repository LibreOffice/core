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

#include <svx/AccessibleControlShape.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include "svx/DescriptionGenerator.hxx"
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/reflection/ProxyFactory.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <svx/svdouno.hxx>
#include "svx/unoapi.hxx"
#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/accessiblewrapper.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svdstr.hrc"
#include <vcl/svapp.hxx>
#include <algorithm>

using namespace ::accessibility;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;

namespace
{
    OUString lcl_getNamePropertyName( )
    {
        return OUString( "Name" );
    }
    OUString lcl_getDescPropertyName( )
    {
        return OUString( "HelpText" );
    }
    OUString lcl_getLabelPropertyName( )
    {
        return OUString( "Label" );
    }
    OUString lcl_getLabelControlPropertyName( )
    {
        return OUString("LabelControl");
    }
    // return the property which should be used as AccessibleName
    const OUString lcl_getPreferredAccNameProperty( const Reference< XPropertySetInfo >& _rxPSI )
    {
        if ( _rxPSI.is() && _rxPSI->hasPropertyByName( lcl_getLabelPropertyName() ) )
            return lcl_getLabelPropertyName();
        else
            return lcl_getNamePropertyName();
    }

    // determines whether or not a state which belongs to the inner context needs to be forwarded to the "composed"
    // context
    bool    isComposedState( const sal_Int16 _nState )
    {
        return  (   ( AccessibleStateType::INVALID != _nState )
                &&  ( AccessibleStateType::DEFUNC != _nState )
                &&  ( AccessibleStateType::ICONIFIED != _nState )
                &&  ( AccessibleStateType::RESIZABLE != _nState )
                &&  ( AccessibleStateType::SELECTABLE != _nState )
                &&  ( AccessibleStateType::SHOWING != _nState )
                &&  ( AccessibleStateType::MANAGES_DESCENDANTS != _nState )
                &&  ( AccessibleStateType::VISIBLE != _nState )
                );
    }

    /// determines whether the given control is in alive mode
    inline  bool    isAliveMode( const Reference< XControl >& _rxControl )
    {
        OSL_PRECOND( _rxControl.is(), "AccessibleControlShape::isAliveMode: invalid control" );
        return _rxControl.is() && !_rxControl->isDesignMode();
    }
}

AccessibleControlShape::AccessibleControlShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    :      AccessibleShape (rShapeInfo, rShapeTreeInfo)
    ,   m_bListeningForName( false )
    ,   m_bListeningForDesc( false )
    ,   m_bMultiplexingStates( false )
    ,   m_bDisposeNativeContext( false )
    ,   m_bWaitingForControl( false )
{
    m_pChildManager = new comphelper::OWrappedAccessibleChildrenManager( comphelper::getProcessComponentContext() );

    osl_atomic_increment( &m_refCount );
    {
        m_pChildManager->setOwningAccessible( this );
    }
    osl_atomic_decrement( &m_refCount );
}

AccessibleControlShape::~AccessibleControlShape()
{
    m_pChildManager.clear();

    if ( m_xControlContextProxy.is() )
        m_xControlContextProxy->setDelegator( nullptr );
    m_xControlContextProxy.clear();
    m_xControlContextTypeAccess.clear();
    m_xControlContextComponent.clear();
        // this should remove the _only_ three "real" reference (means not delegated to
        // ourself) to this proxy, and thus delete it
}

namespace {
    Reference< XContainer > lcl_getControlContainer( const vcl::Window* _pWin, const SdrView* _pView )
    {
        Reference< XContainer > xReturn;
        DBG_ASSERT( _pView, "lcl_getControlContainer: invalid view!" );
        if ( _pView  && _pView->GetSdrPageView())
        {
            xReturn.set(_pView->GetSdrPageView()->GetControlContainer( *_pWin ), css::uno::UNO_QUERY);
        }
        return xReturn;
    }
}

void AccessibleControlShape::Init()
{
    AccessibleShape::Init();

    OSL_ENSURE( !m_xControlContextProxy.is(), "AccessibleControlShape::Init: already initialized!" );
    try
    {
        // What we need to do here is merge the functionality of the AccessibleContext of our UNO control
        // with our own AccessibleContext-related functionality.

        // The problem is that we do not know the interfaces our "inner" context supports - this may be any
        // XAccessibleXXX interface (or even any other) which makes sense for it.

        // In theory, we could implement all possible interfaces ourself, and re-route all functionality to
        // the inner context (except those we implement ourself, like XAccessibleComponent). But this is in no
        // way future-proof - as soon as an inner context appears which implements an additional interface,
        // we would need to adjust our implementation to support this new interface, too. Bad idea.

        // The usual solution for such a problem is aggregation. Aggregation means using UNO's own meachnisms
        // for merging an inner with an outer component, and get a component which behaves as it is exactly one.
        // This is what XAggregation is for. Unfortunately, aggregation requires _exact_ control over the ref count
        // of the inner object, which we do not have at all.
        // Bad, too.

        // But there is a solution: com.sun.star.reflection.ProxyFactory. This service is able to create a proxy
        // for any component, which supports _exactly_ the same interfaces as the component. In addition, it can
        // be aggregated, as by definition the proxy's ref count is exactly 1 when returned from the factory.
        // Sounds better. Though this yields the problem of slightly degraded performance, it's the only solution
        // I'm aware of at the moment .....

        // get the control which belongs to our model (relative to our view)
        const vcl::Window* pViewWindow = maShapeTreeInfo.GetWindow();
        SdrUnoObj* pUnoObjectImpl = dynamic_cast<SdrUnoObj*>( GetSdrObjectFromXShape(mxShape)  );
        SdrView* pView = maShapeTreeInfo.GetSdrView();
        OSL_ENSURE( pView && pViewWindow && pUnoObjectImpl, "AccessibleControlShape::Init: no view, or no view window, no SdrUnoObj!" );

        if ( pView && pViewWindow && pUnoObjectImpl )
        {
            // get the context of the control - it will be our "inner" context
            m_xUnoControl = pUnoObjectImpl->GetUnoControl( *pView, *pViewWindow );

            if ( !m_xUnoControl.is() )
            {
                // the control has not yet been created. Though speaking strictly, it is a bug that
                // our instance here is created without an existing control (because an AccessibleControlShape
                // is a representation of a view object, and can only live if the view it should represent
                // is complete, which implies a living control), it's by far the easiest and most riskless way
                // to fix this here in this class.
                // Okay, we will add as listener to the control container where we expect our control to appear.
                OSL_ENSURE( !m_bWaitingForControl, "AccessibleControlShape::Init: already waiting for the control!" );

                Reference< XContainer > xControlContainer = lcl_getControlContainer( pViewWindow, maShapeTreeInfo.GetSdrView() );
                OSL_ENSURE( xControlContainer.is(), "AccessibleControlShape::Init: unable to find my ControlContainer!" );
                if ( xControlContainer.is() )
                {
                    xControlContainer->addContainerListener( this );
                    m_bWaitingForControl = true;
                }
            }
            else
            {
                Reference< XModeChangeBroadcaster > xControlModes( m_xUnoControl, UNO_QUERY );
                Reference< XAccessible > xControlAccessible( xControlModes, UNO_QUERY );
                Reference< XAccessibleContext > xNativeControlContext;
                if ( xControlAccessible.is() )
                    xNativeControlContext = xControlAccessible->getAccessibleContext();
                OSL_ENSURE( xNativeControlContext.is(), "AccessibleControlShape::Init: no AccessibleContext for the control!" );
                m_aControlContext = WeakReference< XAccessibleContext >( xNativeControlContext );

                // add as listener to the context - we want to multiplex some states
                if ( isAliveMode( m_xUnoControl ) && xNativeControlContext.is() )
                {   // (but only in alive mode)
                    startStateMultiplexing( );
                }

                // now that we have all information about our control, do some adjustments
                adjustAccessibleRole();
                initializeComposedState();

                // some initialization for our child manager, which is used in alive mode only
                if ( isAliveMode( m_xUnoControl ) )
                {
                    Reference< XAccessibleStateSet > xStates( getAccessibleStateSet( ) );
                    OSL_ENSURE( xStates.is(), "AccessibleControlShape::AccessibleControlShape: no inner state set!" );
                    m_pChildManager->setTransientChildren( !xStates.is() || xStates->contains( AccessibleStateType::MANAGES_DESCENDANTS ) );
                }

                // finally, aggregate a proxy for the control context
                // first a factory for the proxy
                Reference< XProxyFactory > xFactory = ProxyFactory::create( comphelper::getProcessComponentContext() );
                // then the proxy itself
                if ( xNativeControlContext.is() )
                {
                    m_xControlContextProxy = xFactory->createProxy( xNativeControlContext );
                    m_xControlContextTypeAccess.set( xNativeControlContext, UNO_QUERY_THROW );
                    m_xControlContextComponent.set( xNativeControlContext, UNO_QUERY_THROW );

                    // aggregate the proxy
                    osl_atomic_increment( &m_refCount );
                    if ( m_xControlContextProxy.is() )
                    {
                        // At this point in time, the proxy has a ref count of exactly one - in m_xControlContextProxy.
                        // Remember to _not_ reset this member unless the delegator of the proxy has been reset, too!
                        m_xControlContextProxy->setDelegator( *this );
                    }
                    osl_atomic_decrement( &m_refCount );

                    m_bDisposeNativeContext = true;

                    // Finally, we need to add ourself as mode listener to the control. In case the mode switches,
                    // we need to dispose ourself.
                    xControlModes->addModeChangeListener( this );
                }
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "AccessibleControlShape::Init: could not \"aggregate\" the controls XAccessibleContext!" );
    }
}

void SAL_CALL AccessibleControlShape::grabFocus()
{
    if ( !m_xUnoControl.is() || !isAliveMode( m_xUnoControl ) )
    {
        // in design mode, we simply forward the request to the base class
        AccessibleShape::grabFocus();
    }
    else
    {
        Reference< XWindow > xWindow( m_xUnoControl, UNO_QUERY );
        OSL_ENSURE( xWindow.is(), "AccessibleControlShape::grabFocus: invalid control!" );
        if ( xWindow.is() )
            xWindow->setFocus();
    }
}

OUString SAL_CALL AccessibleControlShape::getImplementationName()
{
    return OUString( "com.sun.star.comp.accessibility.AccessibleControlShape" );
}

OUString AccessibleControlShape::CreateAccessibleBaseName()
{
    OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_CONTROL:
            sName = "ControlShape";
            break;
        default:
            sName = "UnknownAccessibleControlShape";
            Reference< XShapeDescriptor > xDescriptor (mxShape, UNO_QUERY);
            if (xDescriptor.is())
                sName += ": " + xDescriptor->getShapeType();
    }

    return sName;
}

OUString
    AccessibleControlShape::CreateAccessibleDescription()
{
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_CONTROL:
        {
            // check if we can obtain the "Desc" property from the model
            OUString sDesc( getControlModelStringProperty( lcl_getDescPropertyName() ) );
            if ( sDesc.isEmpty() )
            {   // no -> use the default
                aDG.Initialize (STR_ObjNameSingulUno);
                aDG.AddProperty ("ControlBackground", DescriptionGenerator::PropertyType::Color, "");
                aDG.AddProperty ( "ControlBorder", DescriptionGenerator::PropertyType::Integer, "");
            }
            // ensure that we are listening to the Name property
            m_bListeningForDesc = ensureListeningState( m_bListeningForDesc, true, lcl_getDescPropertyName() );
        }
        break;

        default:
            aDG.Initialize ("Unknown accessible control shape");
            Reference< XShapeDescriptor > xDescriptor (mxShape, UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString ("service name=");
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

IMPLEMENT_FORWARD_REFCOUNT( AccessibleControlShape, AccessibleShape )
IMPLEMENT_GET_IMPLEMENTATION_ID( AccessibleControlShape )

void SAL_CALL AccessibleControlShape::propertyChange( const PropertyChangeEvent& _rEvent )
{
    ::osl::MutexGuard aGuard( maMutex );

    // check if it is the name or the description
    if  (   _rEvent.PropertyName == lcl_getNamePropertyName()
            ||  _rEvent.PropertyName == lcl_getLabelPropertyName() )
    {
        SetAccessibleName(
            CreateAccessibleName(),
            AccessibleContextBase::AutomaticallyCreated);
    }
    else if ( _rEvent.PropertyName == lcl_getDescPropertyName() )
    {
        SetAccessibleDescription(
            CreateAccessibleDescription(),
            AccessibleContextBase::AutomaticallyCreated);
    }
#if OSL_DEBUG_LEVEL > 0
    else
    {
        OSL_FAIL( "AccessibleControlShape::propertyChange: where did this come from?" );
    }
#endif
}

Any SAL_CALL AccessibleControlShape::queryInterface( const Type& _rType )
{
    Any aReturn = AccessibleShape::queryInterface( _rType );
    if ( !aReturn.hasValue() )
    {
        aReturn = AccessibleControlShape_Base::queryInterface( _rType );
        if ( !aReturn.hasValue() && m_xControlContextProxy.is() )
            aReturn = m_xControlContextProxy->queryAggregation( _rType );
    }
    return aReturn;
}

Sequence< Type > SAL_CALL AccessibleControlShape::getTypes()
{
    Sequence< Type > aShapeTypes = AccessibleShape::getTypes();
    Sequence< Type > aOwnTypes = AccessibleControlShape_Base::getTypes();

    Sequence< Type > aAggregateTypes;
    if ( m_xControlContextTypeAccess.is() )
        aAggregateTypes = m_xControlContextTypeAccess->getTypes();

    Sequence< Type > aAllTypes = comphelper::concatSequences( aShapeTypes, aOwnTypes, aAggregateTypes );

    // remove duplicates
    Type* pBegin = aAllTypes.getArray();
    Type* pEnd = pBegin + aAllTypes.getLength();
    while ( pBegin != pEnd )
    {
        Type aThisRoundType = *pBegin;
        if ( ++pBegin != pEnd )
        {
            pEnd = ::std::remove( pBegin, pEnd, aThisRoundType );
            // now all types between begin and (the old) end which equal aThisRoundType
            // are moved behind the new end
        }
    }
    aAllTypes.realloc( pEnd - aAllTypes.getArray() );

    return aAllTypes;
}

void SAL_CALL AccessibleControlShape::notifyEvent( const AccessibleEventObject& _rEvent )
{
    if ( AccessibleEventId::STATE_CHANGED == _rEvent.EventId )
    {
        // multiplex this change
        sal_Int16 nLostState( 0 ), nGainedState( 0 );
        _rEvent.OldValue >>= nLostState;
        _rEvent.NewValue >>= nGainedState;

        // don't multiplex states which the inner context is not responsible for
        if  ( isComposedState( nLostState ) )
            AccessibleShape::ResetState( nLostState );

        if  ( isComposedState( nGainedState ) )
            AccessibleShape::SetState( nGainedState );
    }
    else
    {
        AccessibleEventObject aTranslatedEvent( _rEvent );

        {
            ::osl::MutexGuard aGuard( maMutex );

            // let the child manager translate the event
            aTranslatedEvent.Source = *this;
            m_pChildManager->translateAccessibleEvent( _rEvent, aTranslatedEvent );

            // see if any of these notifications affect our child manager
            m_pChildManager->handleChildNotification( _rEvent );
        }

        FireEvent( aTranslatedEvent );
    }
}

void SAL_CALL AccessibleControlShape::modeChanged(const ModeChangeEvent& rSource)
{
    // did it come from our inner context (the real one, not it's proxy!)?
    SAL_INFO("sw.uno", "AccessibleControlShape::modeChanged");
    Reference<XControl> xSource(rSource.Source, UNO_QUERY); // for faster compare
    if(xSource.get() != m_xUnoControl.get())
    {
        SAL_WARN("sw.uno", "AccessibleControlShape::modeChanged: where did this come from?");
        return;
    }
    SolarMutexGuard g;
    // If our "pseudo-aggregated" inner context does not live anymore,
    // we don't want to live, too.  This is accomplished by asking our
    // parent to replace this object with a new one.  Disposing this
    // object and sending notifications about the replacement are in
    // the responsibility of our parent.
    const bool bReplaced = mpParent->ReplaceChild(this, mxShape, mnIndex, maShapeTreeInfo);
    SAL_WARN_IF(!bReplaced, "sw.uno", "AccessibleControlShape::modeChanged: replacing ourselves away did fail");
}

void SAL_CALL AccessibleControlShape::disposing (const EventObject& _rSource)
{
    AccessibleShape::disposing( _rSource );
}

bool AccessibleControlShape::ensureListeningState(
        const bool _bCurrentlyListening, const bool _bNeedNewListening,
        const OUString& _rPropertyName )
{
    if ( ( _bCurrentlyListening == _bNeedNewListening ) || !ensureControlModelAccess() )
        //  nothing to do
        return _bCurrentlyListening;

    try
    {
        if ( !m_xModelPropsMeta.is() || m_xModelPropsMeta->hasPropertyByName( _rPropertyName ) )
        {
            // add or revoke as listener
            if ( _bNeedNewListening )
                m_xControlModel->addPropertyChangeListener( _rPropertyName, static_cast< XPropertyChangeListener* >( this ) );
            else
                m_xControlModel->removePropertyChangeListener( _rPropertyName, static_cast< XPropertyChangeListener* >( this ) );
        }
        else
            OSL_FAIL( "AccessibleControlShape::ensureListeningState: this property does not exist at this model!" );
    }
    catch( const Exception& e )
    {
        (void)e;    // make compiler happy
        OSL_FAIL( "AccessibleControlShape::ensureListeningState: could not change the listening state!" );
    }

    return _bNeedNewListening;
}

sal_Int32 SAL_CALL AccessibleControlShape::getAccessibleChildCount( )
{
    if ( !m_xUnoControl.is() )
        return 0;
    else if ( !isAliveMode( m_xUnoControl ) )
        // no special action required when in design mode
        return AccessibleShape::getAccessibleChildCount( );
    else
    {
        // in alive mode, we have the full control over our children - they are determined by the children
        // of the context of our UNO control
        Reference< XAccessibleContext > xControlContext( m_aControlContext );
        OSL_ENSURE( xControlContext.is(), "AccessibleControlShape::getAccessibleChildCount: control context already dead! How this!" );
        return xControlContext.is() ? xControlContext->getAccessibleChildCount() : 0;
    }
}

Reference< XAccessible > SAL_CALL AccessibleControlShape::getAccessibleChild( sal_Int32 i )
{
    Reference< XAccessible > xChild;
    if ( !m_xUnoControl.is() )
    {
        throw IndexOutOfBoundsException();
    }
    if ( !isAliveMode( m_xUnoControl ) )
    {
        // no special action required when in design mode - let the base class handle this
        xChild = AccessibleShape::getAccessibleChild( i );
    }
    else
    {
        // in alive mode, we have the full control over our children - they are determined by the children
        // of the context of our UNO control

        Reference< XAccessibleContext > xControlContext( m_aControlContext );
        OSL_ENSURE( xControlContext.is(), "AccessibleControlShape::getAccessibleChildCount: control context already dead! How this!" );
        if ( xControlContext.is() )
        {
            Reference< XAccessible > xInnerChild( xControlContext->getAccessibleChild( i ) );
            OSL_ENSURE( xInnerChild.is(), "AccessibleControlShape::getAccessibleChild: control context returned nonsense!" );
            if ( xInnerChild.is() )
            {
                // we need to wrap this inner child into an own implementation
                xChild = m_pChildManager->getAccessibleWrapperFor( xInnerChild );
            }
        }
    }

#if OSL_DEBUG_LEVEL > 0
    sal_Int32 nChildIndex = -1;
    Reference< XAccessibleContext > xContext;
    if ( xChild.is() )
        xContext = xChild->getAccessibleContext( );
    if ( xContext.is() )
        nChildIndex = xContext->getAccessibleIndexInParent( );
    SAL_WARN_IF( nChildIndex != i, "svx", "AccessibleControlShape::getAccessibleChild: index mismatch,"
            " nChildIndex=" << nChildIndex << " vs i=" << i );
#endif
    return xChild;
}

Reference< XAccessibleRelationSet > SAL_CALL AccessibleControlShape::getAccessibleRelationSet(  )
{
    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    ensureControlModelAccess();
    AccessibleControlShape* pCtlAccShape = GetLabeledByControlShape();
    if(pCtlAccShape)
    {
        Reference < XAccessible > xAcc (pCtlAccShape->getAccessibleContext(), UNO_QUERY);

        css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > aSequence { xAcc };
        if( getAccessibleRole() == AccessibleRole::RADIO_BUTTON )
        {
            pRelationSetHelper->AddRelation( AccessibleRelation( AccessibleRelationType::MEMBER_OF, aSequence ) );
        }
        else
        {
            pRelationSetHelper->AddRelation( AccessibleRelation( AccessibleRelationType::LABELED_BY, aSequence ) );
        }
    }
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

OUString AccessibleControlShape::CreateAccessibleName()
{
    ensureControlModelAccess();

    OUString sName;
    if ( getAccessibleRole() != AccessibleRole::SHAPE
        && getAccessibleRole() != AccessibleRole::RADIO_BUTTON  )
    {
        AccessibleControlShape* pCtlAccShape = GetLabeledByControlShape();
        if(pCtlAccShape)
        {
            sName = pCtlAccShape->CreateAccessibleName();
        }
    }

    if (sName.isEmpty())
    {
        // check if we can obtain the "Name" resp. "Label" property from the model
        const OUString& rAccNameProperty = lcl_getPreferredAccNameProperty( m_xModelPropsMeta );
        sName = getControlModelStringProperty( rAccNameProperty );
        if ( !sName.getLength() )
        {   // no -> use the default
            sName = AccessibleShape::CreateAccessibleName();
        }
    }

    // now that somebody first asked us for our name, ensure that we are listening to name changes on the model
    m_bListeningForName = ensureListeningState( m_bListeningForName, true, lcl_getPreferredAccNameProperty( m_xModelPropsMeta ) );

    return sName;
}

void SAL_CALL AccessibleControlShape::disposing()
{
    // ensure we're not listening
    m_bListeningForName = ensureListeningState( m_bListeningForName, false, lcl_getPreferredAccNameProperty( m_xModelPropsMeta ) );
    m_bListeningForDesc = ensureListeningState( m_bListeningForDesc, false, lcl_getDescPropertyName() );

    if ( m_bMultiplexingStates )
        stopStateMultiplexing( );

    // dispose the child cache/map
    m_pChildManager->dispose();

    // release the model
    m_xControlModel.clear();
    m_xModelPropsMeta.clear();
    m_aControlContext = WeakReference< XAccessibleContext >();

    // stop listening at the control container (should never be necessary here, but who knows ....)
    if ( m_bWaitingForControl )
    {
        OSL_FAIL( "AccessibleControlShape::disposing: this should never happen!" );
        Reference< XContainer > xContainer = lcl_getControlContainer( maShapeTreeInfo.GetWindow(), maShapeTreeInfo.GetSdrView() );
        if ( xContainer.is() )
        {
            m_bWaitingForControl = false;
            xContainer->removeContainerListener( this );
        }
    }

    // forward the disposal to our inner context
    if ( m_bDisposeNativeContext )
    {
        // don't listen for mode changes anymore
        Reference< XModeChangeBroadcaster > xControlModes( m_xUnoControl, UNO_QUERY );
        OSL_ENSURE( xControlModes.is(), "AccessibleControlShape::disposing: don't have an mode broadcaster anymore!" );
        if ( xControlModes.is() )
            xControlModes->removeModeChangeListener( this );

        if ( m_xControlContextComponent.is() )
            m_xControlContextComponent->dispose();
        // do _not_ clear m_xControlContextProxy! This has to be done in the dtor for correct ref-count handling

        // no need to dispose the proxy/inner context anymore
        m_bDisposeNativeContext = false;
    }

    m_xUnoControl.clear();

    // let the base do its stuff
    AccessibleShape::disposing();
}

bool AccessibleControlShape::ensureControlModelAccess()
{
    if ( m_xControlModel.is() )
        return true;

    try
    {
        Reference< XControlShape > xShape( mxShape, UNO_QUERY );
        if ( xShape.is() )
            m_xControlModel.set(xShape->getControl(), css::uno::UNO_QUERY);

        if ( m_xControlModel.is() )
            m_xModelPropsMeta = m_xControlModel->getPropertySetInfo();
    }
    catch( const Exception& e )
    {
        (void)e;    // make compiler happy
        OSL_FAIL( "AccessibleControlShape::ensureControlModelAccess: caught an exception!" );
    }

    return m_xControlModel.is();
}

void AccessibleControlShape::startStateMultiplexing()
{
    OSL_PRECOND( !m_bMultiplexingStates, "AccessibleControlShape::startStateMultiplexing: already multiplexing!" );

#if OSL_DEBUG_LEVEL > 0
    // we should have a control, and it should be in alive mode
    OSL_PRECOND( isAliveMode( m_xUnoControl ),
        "AccessibleControlShape::startStateMultiplexing: should be done in alive mode only!" );
#endif
    // we should have the native context of the control
    Reference< XAccessibleEventBroadcaster > xBroadcaster( m_aControlContext.get(), UNO_QUERY );
    OSL_ENSURE( xBroadcaster.is(), "AccessibleControlShape::startStateMultiplexing: no AccessibleEventBroadcaster on the native context!" );

    if ( xBroadcaster.is() )
    {
        xBroadcaster->addAccessibleEventListener( this );
        m_bMultiplexingStates = true;
    }
}

void AccessibleControlShape::stopStateMultiplexing()
{
    OSL_PRECOND( m_bMultiplexingStates, "AccessibleControlShape::stopStateMultiplexing: not multiplexing!" );

    // we should have the native context of the control
    Reference< XAccessibleEventBroadcaster > xBroadcaster( m_aControlContext.get(), UNO_QUERY );
    OSL_ENSURE( xBroadcaster.is(), "AccessibleControlShape::stopStateMultiplexing: no AccessibleEventBroadcaster on the native context!" );

    if ( xBroadcaster.is() )
    {
        xBroadcaster->removeAccessibleEventListener( this );
        m_bMultiplexingStates = false;
    }
}

OUString AccessibleControlShape::getControlModelStringProperty( const OUString& _rPropertyName ) const
{
    OUString sReturn;
    try
    {
        if ( const_cast< AccessibleControlShape* >( this )->ensureControlModelAccess() )
        {
            if ( !m_xModelPropsMeta.is() || m_xModelPropsMeta->hasPropertyByName( _rPropertyName ) )
                // ask only if a) the control does not have a PropertySetInfo object or b) it has, and the
                // property in question is available
                m_xControlModel->getPropertyValue( _rPropertyName ) >>= sReturn;
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "OAccessibleControlContext::getModelStringProperty: caught an exception!" );
    }
    return sReturn;
}

void AccessibleControlShape::adjustAccessibleRole( )
{
    // if we're in design mode, we are a simple SHAPE, in alive mode, we use the role of our inner context
    if ( !isAliveMode( m_xUnoControl ) )
        return;

    // we're in alive mode -> determine the role of the inner context
    Reference< XAccessibleContext > xNativeContext( m_aControlContext );
    OSL_PRECOND( xNativeContext.is(), "AccessibleControlShape::adjustAccessibleRole: no inner context!" );
    if ( xNativeContext.is() )
        SetAccessibleRole( xNativeContext->getAccessibleRole( ) );
}

#ifdef DBG_UTIL

bool AccessibleControlShape::SetState( sal_Int16 _nState )
{
    OSL_ENSURE( !isAliveMode( m_xUnoControl ) || !isComposedState( _nState ),
        "AccessibleControlShape::SetState: a state which should be determined by the control context is set from outside!" );
    return AccessibleShape::SetState( _nState );
}
#endif // DBG_UTIL

void AccessibleControlShape::initializeComposedState()
{
    if ( !isAliveMode( m_xUnoControl ) )
        // no action necessary for design mode
        return;

    // get our own state set implementation
    ::utl::AccessibleStateSetHelper* pComposedStates =
        static_cast< ::utl::AccessibleStateSetHelper* >( mxStateSet.get() );
    OSL_PRECOND( pComposedStates,
        "AccessibleControlShape::initializeComposedState: no composed set!" );

    // we need to reset some states of the composed set, because they either do not apply
    // for controls in alive mode, or are in the responsibility of the UNO-control, anyway
    pComposedStates->RemoveState( AccessibleStateType::ENABLED );       // this is controlled by the UNO-control
    pComposedStates->RemoveState( AccessibleStateType::SENSITIVE );     // this is controlled by the UNO-control
    pComposedStates->RemoveState( AccessibleStateType::FOCUSABLE );     // this is controlled by the UNO-control
    pComposedStates->RemoveState( AccessibleStateType::SELECTABLE );    // this does not hold for an alive UNO-control
#if OSL_DEBUG_LEVEL > 0
    // now, only states which are not in the responsibility of the UNO control should be part of this state set
    {
        Sequence< sal_Int16 > aInitStates = pComposedStates->getStates();
        for ( sal_Int32 i=0; i<aInitStates.getLength(); ++i )
            OSL_ENSURE( !isComposedState( aInitStates.getConstArray()[i] ),
                "AccessibleControlShape::initializeComposedState: invalid initial composed state (should be controlled by the UNO-control)!" );
    }
#endif

    // get my inner context
    Reference< XAccessibleContext > xInnerContext( m_aControlContext );
    OSL_PRECOND( xInnerContext.is(), "AccessibleControlShape::initializeComposedState: no inner context!" );
    if ( xInnerContext.is() )
    {
        // get all states of the inner context
        Reference< XAccessibleStateSet > xInnerStates( xInnerContext->getAccessibleStateSet() );
        OSL_ENSURE( xInnerStates.is(), "AccessibleControlShape::initializeComposedState: no inner states!" );
        Sequence< sal_Int16 > aInnerStates;
        if ( xInnerStates.is() )
            aInnerStates = xInnerStates->getStates();

        // look which one are to be propagated to the composed context
        const sal_Int16* pStates = aInnerStates.getConstArray();
        const sal_Int16* pStatesEnd = pStates + aInnerStates.getLength();
        for ( ; pStates != pStatesEnd; ++pStates )
        {
            if ( isComposedState( *pStates ) && !pComposedStates->contains( *pStates ) )
            {
                pComposedStates->AddState( *pStates );
            }
        }
    }
}

void SAL_CALL AccessibleControlShape::elementInserted( const css::container::ContainerEvent& _rEvent )
{
    Reference< XContainer > xContainer( _rEvent.Source, UNO_QUERY );
    Reference< XControl > xControl( _rEvent.Element, UNO_QUERY );

    OSL_ENSURE( xContainer.is() && xControl.is(),
        "AccessibleControlShape::elementInserted: invalid event description!" );

    if ( !xControl.is() )
        return;

    ensureControlModelAccess();

    Reference< XInterface > xNewNormalized( xControl->getModel(), UNO_QUERY );
    Reference< XInterface > xMyModelNormalized( m_xControlModel, UNO_QUERY );
    if ( xNewNormalized.get() && xMyModelNormalized.get() )
    {
        // now finally the control for the model we're responsible for has been inserted into the container
        Reference< XInterface > xKeepAlive( *this );

        // first, we're not interested in any more container events
        if ( xContainer.is() )
        {
            xContainer->removeContainerListener( this );
            m_bWaitingForControl = false;
        }

        // second, we need to replace ourself with a new version, which now can be based on the
        // control
        OSL_VERIFY( mpParent->ReplaceChild ( this, mxShape, mnIndex, maShapeTreeInfo ) );
    }
}

void SAL_CALL AccessibleControlShape::elementRemoved( const css::container::ContainerEvent& )
{
    // not interested in
}

void SAL_CALL AccessibleControlShape::elementReplaced( const css::container::ContainerEvent& )
{
    // not interested in
}

AccessibleControlShape* SAL_CALL AccessibleControlShape::GetLabeledByControlShape( )
{
    if(m_xControlModel.is())
    {
        const OUString& rAccLabelControlProperty = lcl_getLabelControlPropertyName();
        Any sCtlLabelBy;
        // get the "label by" property value of the control
        if (::comphelper::hasProperty(rAccLabelControlProperty, m_xControlModel))
        {
            sCtlLabelBy = m_xControlModel->getPropertyValue(rAccLabelControlProperty);
            if( sCtlLabelBy.hasValue() )
            {
                Reference< XPropertySet >  xAsSet (sCtlLabelBy, UNO_QUERY);
                AccessibleControlShape* pCtlAccShape = mpParent->GetAccControlShapeFromModel(xAsSet.get());
                return pCtlAccShape;
            }
        }
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
