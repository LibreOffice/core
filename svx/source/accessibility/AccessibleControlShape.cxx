/*************************************************************************
 *
 *  $RCSfile: AccessibleControlShape.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 09:56:52 $
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

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_CONTROL_SHAPE_HXX
#include "AccessibleControlShape.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#include "AccessibleShapeInfo.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_DESCRIPTION_GENERATOR_HXX
#include "DescriptionGenerator.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _SVDOUNO_HXX
#include "svdouno.hxx"
#endif
#ifndef _SVX_UNOAPI_HXX_
#include "unoapi.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#include "ShapeTypeHandler.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_SVX_SHAPE_TYPES_HXX
#include "SvxShapeTypes.hxx"
#endif

#ifndef COMPHELPER_ACCESSIBLE_WRAPPER_HXX
#include <comphelper/accessiblewrapper.hxx>
#endif
#include "svdstr.hrc"
#include <algorithm>

using namespace ::comphelper;
using namespace ::accessibility;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::reflection;
using namespace ::com::sun::star::drawing;

//--------------------------------------------------------------------
namespace
{
    //................................................................
    const ::rtl::OUString& lcl_getNamePropertyName( )
    {
        static ::rtl::OUString s_sNamePropertyName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );
        return s_sNamePropertyName;
    }
    //................................................................
    const ::rtl::OUString& lcl_getDescPropertyName( )
    {
        static ::rtl::OUString s_sDescPropertyDesc( RTL_CONSTASCII_USTRINGPARAM( "HelpText" ) );
        return s_sDescPropertyDesc;
    }
    //................................................................
    const ::rtl::OUString& lcl_getLabelPropertyName( )
    {
        static ::rtl::OUString s_sLabelPropertyLabel( RTL_CONSTASCII_USTRINGPARAM( "Label" ) );
        return s_sLabelPropertyLabel;
    }
    //................................................................
    // return the property which should be used as AccessibleName
    const ::rtl::OUString& lcl_getPreferredAccNameProperty( const Reference< XPropertySetInfo >& _rxPSI )
    {
        if ( _rxPSI.is() && _rxPSI->hasPropertyByName( lcl_getLabelPropertyName() ) )
            return lcl_getLabelPropertyName();
        else
            return lcl_getNamePropertyName();
    }

    //................................................................
    // determines whether or not a state which belongs to the inner context needs to be forwarded to the "composed"
    // context
    sal_Bool    isComposedState( const sal_Int16 _nState )
    {
        return  (   ( AccessibleStateType::INVALID != _nState )
                &&  ( AccessibleStateType::DEFUNC != _nState )
                &&  ( AccessibleStateType::ICONIFIED != _nState )
                &&  ( AccessibleStateType::RESIZABLE != _nState )
                &&  ( AccessibleStateType::SELECTABLE != _nState )
                &&  ( AccessibleStateType::SHOWING != _nState )
                &&  ( AccessibleStateType::MANAGES_DESCENDANT != _nState )
                &&  ( AccessibleStateType::VISIBLE != _nState )
                );
    }

    //................................................................
    /** determines whether the given control is in alive mode
    */
    inline  sal_Bool    isAliveMode( const Reference< XControl >& _rxControl )
    {
        OSL_PRECOND( _rxControl.is(), "AccessibleControlShape::isAliveMode: invalid control" );
        return _rxControl.is() && !_rxControl->isDesignMode();
    }
}

//=============================================================================
//= AccessibleControlShape
//=============================================================================

//-----------------------------------------------------------------------------
AccessibleControlShape::AccessibleControlShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    :      AccessibleShape (rShapeInfo, rShapeTreeInfo)
    ,   m_bListeningForName( sal_False )
    ,   m_bListeningForDesc( sal_False )
    ,   m_bDisposeNativeContext( sal_False )
    ,   m_bMultiplexingStates( sal_False )
{
    m_pChildManager = new OWrappedAccessibleChildrenManager( getProcessServiceFactory() );
    m_pChildManager->acquire();

    osl_incrementInterlockedCount( &m_refCount );
    {
        m_pChildManager->setOwningAccessible( this );
    }
    osl_decrementInterlockedCount( &m_refCount );
}

//-----------------------------------------------------------------------------
AccessibleControlShape::~AccessibleControlShape (void)
{
    m_pChildManager->release();
    m_pChildManager = NULL;

    if ( m_xControlContextProxy.is() )
        m_xControlContextProxy->setDelegator( NULL );
    m_xControlContextProxy.clear();
        // this should remove the _one_and_only_ "real" reference (means not delegated to
        // ourself) to this proxy, and thus delete it
}

//-----------------------------------------------------------------------------
SdrObject* AccessibleControlShape::getSdrObject() const
{
    return GetSdrObjectFromXShape (mxShape);
}

//-----------------------------------------------------------------------------
void AccessibleControlShape::Init()
{
    AccessibleShape::Init();

    OSL_ENSURE( !m_xControlContextProxy.is(), "AccessibleControlShape::Init: already initialized!" );
    try
    {
        // What we need to do here is merge the functionality of the AccessibleContext of our UNO control
        // with our own AccessibleContext-related functionality.
        //
        // The problem is that we do not know the interfaces our "inner" context supports - this may be any
        // XAccessibleXXX interface (or even any other) which makes sense for it.
        //
        // In theory, we could implement all possible interfaces ourself, and re-route all functionality to
        // the inner context (except those we implement ourself, like XAccessibleComponent). But this is in no
        // way future-proof - as soon as an inner context appears which implements an additional interface,
        // we would need to adjust our implementation to support this new interface, too. Bad idea.
        //
        // The usual solution for such a problem is aggregation. Aggregation means using UNO's own meachnisms
        // for merging an inner with an outer component, an get a component which behaves as it is exactly one.
        // This is what XAggregation is for. Unfortunately, aggregation requires _exact_ control over the ref count
        // of the inner object, which we do not have at all.
        // Bad, too.
        //
        // But there is a solution: com.sun.star.reflection.ProxyFactory. This service is able to create a proxy
        // for any component, which supports _exactly_ the same interfaces as the component. In addition, it can
        // be aggregated, as by definition the proxy's ref count is exactly one when returned from the factory.
        // Sounds better. Though this yields the problem of slightly degraded performance, it's the only solution
        // I'm aware of at the moment .....
        //
        // 98750 - 30.04.2002 - fs@openoffice.org
        //

        // get the control which belongs to our model (relative to our view)
        const Window* pViewWindow = maShapeTreeInfo.GetWindow();
        SdrUnoObj* pUnoObjectImpl = PTR_CAST( SdrUnoObj, getSdrObject() );
        OSL_ENSURE( pViewWindow && pUnoObjectImpl, "AccessibleControlShape::Init: no view, or no SdrUnoObj!" );

        if ( pViewWindow && pUnoObjectImpl )
        {
            // .................................................................
            // get the context of the control - it will be our "inner" context
            m_xUnoControl = pUnoObjectImpl->GetUnoControl( pViewWindow );
            Reference< XModeChangeBroadcaster > xControlModes( m_xUnoControl, UNO_QUERY );
            Reference< XAccessible > xControlAccessible( xControlModes, UNO_QUERY );
            Reference< XAccessibleContext > xNativeControlContext;
            if ( xControlAccessible.is() )
                xNativeControlContext = xControlAccessible->getAccessibleContext();
            OSL_ENSURE( xNativeControlContext.is(), "AccessibleControlShape::Init: no AccessibleContext for the control!" );
            m_aControlContext = WeakReference< XAccessibleContext >( xNativeControlContext );

            // .................................................................
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
                m_pChildManager->setTransientChildren( !xStates.is() || xStates->contains( AccessibleStateType::MANAGES_DESCENDANT ) );
            }

            // .................................................................
            // finally, aggregate a proxy for the control context
            // first a factory for the proxy
            Reference< XProxyFactory > xFactory;
            xFactory = xFactory.query( createProcessComponent( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.reflection.ProxyFactory" ) ) ) );
            OSL_ENSURE( xFactory.is(), "AccessibleControlShape::Init: could not create a proxy factory!" );
            // then the proxy itself
            if ( xFactory.is() && xNativeControlContext.is() )
            {
                m_xControlContextProxy = xFactory->createProxy( xNativeControlContext );

                // aggregate the proxy
                osl_incrementInterlockedCount( &m_refCount );
                if ( m_xControlContextProxy.is() )
                {
                    // At this point in time, the proxy has a ref count of exactly one - in m_xControlContextProxy.
                    // Remember to _not_ reset this member unles the delegator of the proxy has been reset, too!
                    m_xControlContextProxy->setDelegator( *this );
                }
                osl_decrementInterlockedCount( &m_refCount );

                m_bDisposeNativeContext = sal_True;

                // Finally, we need to add ourself as mode listener to the control. In case the mode switches,
                // we need to dispose ourself.
                xControlModes->addModeChangeListener( this );
            }
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "AccessibleControlShape::Init: could not \"aggregate\" the controls XAccessibleContext!" );
    }
}

//-----------------------------------------------------------------------------
Reference< XAccessibleContext > SAL_CALL AccessibleControlShape::getAccessibleContext(void) throw (RuntimeException)
{
    return AccessibleShape::getAccessibleContext ();
}


//-----------------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::grabFocus(void)  throw (RuntimeException)
{
    if ( !isAliveMode( m_xUnoControl ) )
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

//-----------------------------------------------------------------------------
::rtl::OUString SAL_CALL AccessibleControlShape::getImplementationName(void) throw (RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.accessibility.AccessibleControlShape" ) );
}

//-----------------------------------------------------------------------------
::rtl::OUString AccessibleControlShape::CreateAccessibleBaseName(void) throw (RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_CONTROL:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ControlShape"));
            break;
        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleControlShape"));
            Reference< XShapeDescriptor > xDescriptor (mxShape, UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}




//--------------------------------------------------------------------
::rtl::OUString
    AccessibleControlShape::CreateAccessibleDescription (void)
    throw (RuntimeException)
{
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_CONTROL:
        {
            // check if we can obtain the "Desc" property from the model
            ::rtl::OUString sDesc( getControlModelStringProperty( lcl_getDescPropertyName() ) );
            if ( !sDesc.getLength() )
            {   // no -> use the default
                aDG.Initialize (STR_ObjNameSingulUno);
                aDG.AddProperty (::rtl::OUString::createFromAscii ("ControlBackground"),
                    DescriptionGenerator::COLOR,
                    ::rtl::OUString());
                aDG.AddProperty (::rtl::OUString::createFromAscii ("ControlBorder"),
                    DescriptionGenerator::INTEGER,
                    ::rtl::OUString());
            }
            // ensure that we are listening to the Name property
            m_bListeningForDesc = ensureListeningState( m_bListeningForDesc, sal_True, lcl_getDescPropertyName() );
        }
        break;

        default:
            aDG.Initialize (::rtl::OUString::createFromAscii (
                "Unknown accessible control shape"));
            Reference< XShapeDescriptor > xDescriptor (mxShape, UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

//--------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT( AccessibleControlShape, AccessibleShape )
IMPLEMENT_GET_IMPLEMENTATION_ID( AccessibleControlShape )

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    // check if it is the name or the description
    if  (   _rEvent.PropertyName.equals( lcl_getNamePropertyName() )
        ||  _rEvent.PropertyName.equals( lcl_getLabelPropertyName( ) )
        )
    {
        SetAccessibleName (CreateAccessibleName());
    }
    else if ( _rEvent.PropertyName.equals( lcl_getDescPropertyName() ) )
    {
        SetAccessibleDescription (CreateAccessibleDescription());
    }
#if OSL_DEBUG_LEVEL > 0
    else
    {
        OSL_ENSURE( sal_False, "AccessibleControlShape::propertyChange: where did this come from?" );
    }
#endif
}

//--------------------------------------------------------------------
Any SAL_CALL AccessibleControlShape::queryInterface( const Type& _rType ) throw (RuntimeException)
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

//--------------------------------------------------------------------
Sequence< Type > SAL_CALL AccessibleControlShape::getTypes() throw (RuntimeException)
{
    Sequence< Type > aShapeTypes = AccessibleShape::getTypes();
    Sequence< Type > aOwnTypes = AccessibleControlShape_Base::getTypes();

    Sequence< Type > aAggregateTypes;
    Reference< XTypeProvider > xAggTypes;
    if ( query_aggregation( m_xControlContextProxy, xAggTypes ) )
        aAggregateTypes = xAggTypes->getTypes();

    Sequence< Type > aAllTypes = concatSequences( aShapeTypes, aOwnTypes, aAggregateTypes );

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

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::notifyEvent( const AccessibleEventObject& _rEvent ) throw (RuntimeException)
{
    if ( AccessibleEventId::ACCESSIBLE_STATE_EVENT == _rEvent.EventId )
    {
        // multiplex this change
        sal_Int16 nLostState( 0 ), nGainedState( 0 );
        _rEvent.OldValue >>= nLostState;
        _rEvent.NewValue >>= nGainedState;

        // don't multiplex states which the inner context is not resposible for
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

            // see if any of these notifications affect our child manager
            m_pChildManager->handleChildNotification( _rEvent );

            aTranslatedEvent.Source = *this;
            m_pChildManager->translateAccessibleEvent( _rEvent, aTranslatedEvent );
        }

        FireEvent( aTranslatedEvent );
    }
}

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::modeChanged( const ModeChangeEvent& _rSource ) throw (RuntimeException)
{
    // did it come from our inner context (the real one, not it's proxy!)?
    OSL_TRACE ("AccessibleControlShape::modeChanged");
    Reference< XControl > xSource( _rSource.Source, UNO_QUERY );    // for faster compare
    if ( xSource.get() == m_xUnoControl.get() )
    {
        // If our "pseudo-aggregated" inner context does not live anymore,
        // we don't want to live, too.  This is accomplished by asking our
        // parent to replace this object with a new one.  Disposing this
        // object and sending notifications about the replacement are in
        // the responsibility of our parent.
        OSL_VERIFY( mpParent->ReplaceChild ( this, mxShape, mnIndex, maShapeTreeInfo ) );
    }
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_ENSURE( sal_False, "AccessibleControlShape::modeChanged: where did this come from?" );
#endif
}

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::disposing (const EventObject& _rSource) throw (RuntimeException)
{
    AccessibleShape::disposing( _rSource );
}

//--------------------------------------------------------------------
sal_Bool AccessibleControlShape::ensureListeningState(
        const sal_Bool _bCurrentlyListening, const sal_Bool _bNeedNewListening,
        const ::rtl::OUString& _rPropertyName )
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
            OSL_ENSURE( sal_False, "AccessibleControlShape::ensureListeningState: this property does not exist at this model!" );
    }
    catch( const Exception& e )
    {
        e;  // make compiler happy
        OSL_ENSURE( sal_False, "AccessibleControlShape::ensureListeningState: could not change the listening state!" );
    }

    return _bNeedNewListening;
}

//--------------------------------------------------------------------
sal_Int32 SAL_CALL AccessibleControlShape::getAccessibleChildCount( ) throw(RuntimeException)
{
    if ( !isAliveMode( m_xUnoControl ) )
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

//--------------------------------------------------------------------
Reference< XAccessible > SAL_CALL AccessibleControlShape::getAccessibleChild( sal_Int32 i ) throw(IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessible > xChild;
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
    return xChild;
}

//--------------------------------------------------------------------
Reference< XAccessibleRelationSet > SAL_CALL AccessibleControlShape::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    // TODO
    return AccessibleShape::getAccessibleRelationSet( );
}

//--------------------------------------------------------------------
::rtl::OUString AccessibleControlShape::CreateAccessibleName (void) throw (RuntimeException)
{
    ensureControlModelAccess();

    // check if we can obtain the "Name" resp. "Label" property from the model
    const ::rtl::OUString& rAccNameProperty = lcl_getPreferredAccNameProperty( m_xModelPropsMeta );

    ::rtl::OUString sName( getControlModelStringProperty( rAccNameProperty ) );
    if ( !sName.getLength() )
    {   // no -> use the default
        sName = AccessibleShape::CreateAccessibleName();
    }

    // now that somebody first asked us for our name, ensure that we are listening to name changes on the model
    m_bListeningForName = ensureListeningState( m_bListeningForName, sal_True, lcl_getPreferredAccNameProperty( m_xModelPropsMeta ) );

    return sName;
}

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::disposing (void)
{
    // ensure we're not listening
    m_bListeningForName = ensureListeningState( m_bListeningForName, sal_False, lcl_getPreferredAccNameProperty( m_xModelPropsMeta ) );
    m_bListeningForDesc = ensureListeningState( m_bListeningForDesc, sal_False, lcl_getDescPropertyName() );

    if ( m_bMultiplexingStates )
        stopStateMultiplexing( );

    // dispose the child cache/map
    m_pChildManager->dispose();

    // release the model
    m_xControlModel.clear();
    m_xModelPropsMeta.clear();
    m_aControlContext = WeakReference< XAccessibleContext >();

    // forward the disposel to our inner context
    if ( m_bDisposeNativeContext )
    {
        // don't listen for mode changes anymore
        Reference< XModeChangeBroadcaster > xControlModes( m_xUnoControl, UNO_QUERY );
        OSL_ENSURE( xControlModes.is(), "AccessibleControlShape::disposing: don't have an mode broadcaster anymore!" );
        if ( xControlModes.is() )
            xControlModes->removeModeChangeListener( this );

        Reference< XComponent > xInnerComponent;
        if ( query_aggregation( m_xControlContextProxy, xInnerComponent ) )
            xInnerComponent->dispose();
        // do _not_ clear m_xControlContextProxy! This has to be done in the dtor for correct ref-count handling

        // no need to dispose the proxy/inner context anymore
        m_bDisposeNativeContext = sal_False;
    }

    m_xUnoControl.clear();

    // let the base do it's stuff
    AccessibleShape::disposing();
}

//--------------------------------------------------------------------
sal_Bool AccessibleControlShape::ensureControlModelAccess() SAL_THROW(())
{
    if ( m_xControlModel.is() )
        return sal_True;

    try
    {
        Reference< XControlShape > xShape( mxShape, UNO_QUERY );
        if ( xShape.is() )
            m_xControlModel = m_xControlModel.query( xShape->getControl() );

        if ( m_xControlModel.is() )
            m_xModelPropsMeta = m_xControlModel->getPropertySetInfo();
    }
    catch( const Exception& e )
    {
        e;  // make compiler happy
        OSL_ENSURE( sal_False, "AccessibleControlShape::ensureControlModelAccess: caught an exception!" );
    }

    return m_xControlModel.is();
}

//--------------------------------------------------------------------
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
        xBroadcaster->addEventListener( this );
        m_bMultiplexingStates = sal_True;
    }
}

//--------------------------------------------------------------------
void AccessibleControlShape::stopStateMultiplexing()
{
    OSL_PRECOND( m_bMultiplexingStates, "AccessibleControlShape::stopStateMultiplexing: not multiplexing!" );

    // we should have the native context of the control
    Reference< XAccessibleEventBroadcaster > xBroadcaster( m_aControlContext.get(), UNO_QUERY );
    OSL_ENSURE( xBroadcaster.is(), "AccessibleControlShape::stopStateMultiplexing: no AccessibleEventBroadcaster on the native context!" );

    if ( xBroadcaster.is() )
    {
        xBroadcaster->removeEventListener( this );
        m_bMultiplexingStates = sal_False;
    }
}

//--------------------------------------------------------------------
::rtl::OUString AccessibleControlShape::getControlModelStringProperty( const ::rtl::OUString& _rPropertyName ) const SAL_THROW(())
{
    ::rtl::OUString sReturn;
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
        OSL_ENSURE( sal_False, "OAccessibleControlContext::getModelStringProperty: caught an exception!" );
    }
    return sReturn;
}

//--------------------------------------------------------------------
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
//--------------------------------------------------------------------
sal_Bool AccessibleControlShape::SetState( sal_Int16 _nState )
{
    OSL_ENSURE( !isAliveMode( m_xUnoControl ) || !isComposedState( _nState ),
        "AccessibleControlShape::SetState: a state which should be determined by the control context is set from outside!" );
    return AccessibleShape::SetState( _nState );
}
#endif // DBG_UTIL

//--------------------------------------------------------------------
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

    // we need to reset some states of the composed set, because the either do not apply
    // for controls in alive mode, or are in the responsibility of the UNO-control, anyway
    pComposedStates->RemoveState( AccessibleStateType::ENABLED );       // this is controlled by the UNO-control
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

