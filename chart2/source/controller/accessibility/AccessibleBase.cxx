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

#include "AccessibleBase.hxx"
#include "AccessibleChartShape.hxx"
#include "ObjectHierarchy.hxx"
#include "ObjectIdentifier.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "macros.hxx"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <comphelper/serviceinfohelper.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <rtl/ustrbuf.hxx>
// for SolarMutex
#include <vcl/svapp.hxx>
#include <rtl/uuid.h>
#include <cppuhelper/queryinterface.hxx>
#include <svl/itemset.hxx>
#include <editeng/unofdesc.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdetc.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoprov.hxx>
#include <vcl/unohelp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

#include <algorithm>
#include <o3tl/compat_functional.hxx>

#include "ChartElementFactory.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::osl::MutexGuard;
using ::osl::ClearableMutexGuard;
using ::osl::ResettableMutexGuard;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;

namespace chart
{

/** @param bMayHaveChildren is false per default
 */
AccessibleBase::AccessibleBase(
    const AccessibleElementInfo & rAccInfo,
    bool bMayHaveChildren,
    bool bAlwaysTransparent /* default: false */ ) :
        impl::AccessibleBase_Base( m_aMutex ),
        m_bIsDisposed( false ),
        m_bMayHaveChildren( bMayHaveChildren ),
        m_bChildrenInitialized( false ),
        m_nEventNotifierId(0),
        m_pStateSetHelper( new ::utl::AccessibleStateSetHelper() ),
        m_aStateSet( m_pStateSetHelper ),
        m_aAccInfo( rAccInfo ),
        m_bAlwaysTransparent( bAlwaysTransparent ),
        m_bStateSetInitialized( false )
{
    // initialize some states
    OSL_ASSERT( m_pStateSetHelper );
    m_pStateSetHelper->AddState( AccessibleStateType::ENABLED );
    m_pStateSetHelper->AddState( AccessibleStateType::SHOWING );
    m_pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
    m_pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
    m_pStateSetHelper->AddState( AccessibleStateType::FOCUSABLE );
}

AccessibleBase::~AccessibleBase()
{
    OSL_ASSERT( m_bIsDisposed );
}

bool AccessibleBase::CheckDisposeState( bool bThrowException /* default: true */ ) const
    throw (lang::DisposedException)
{
    if( bThrowException &&
        m_bIsDisposed )
    {
        throw lang::DisposedException("component has state DEFUNC",
            static_cast< uno::XWeak * >( const_cast< AccessibleBase * >( this )));
    }
    return m_bIsDisposed;
}

bool AccessibleBase::NotifyEvent( EventType eEventType, const AccessibleUniqueId & rId )
{
    if( GetId() == rId )
    {
        // event is addressed to this object

        ::com::sun::star::uno::Any aEmpty;
        ::com::sun::star::uno::Any aSelected;
        aSelected <<= AccessibleStateType::SELECTED;
        switch( eEventType )
        {
            case OBJECT_CHANGE:
                {
                    BroadcastAccEvent( AccessibleEventId::VISIBLE_DATA_CHANGED, aEmpty, aEmpty );
#if OSL_DEBUG_LEVEL > 1
                    OSL_TRACE(
                        OUStringToOString(
                            OUString( "Visible data event sent by: " ) +
                            getAccessibleName(),
                            RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                }
                break;

            case GOT_SELECTION:
                {
                    AddState( AccessibleStateType::SELECTED );
                    BroadcastAccEvent( AccessibleEventId::STATE_CHANGED, aSelected, aEmpty );

                    AddState( AccessibleStateType::FOCUSED );
                    aSelected <<= AccessibleStateType::FOCUSED;
                    BroadcastAccEvent( AccessibleEventId::STATE_CHANGED, aSelected, aEmpty, true );
#if OSL_DEBUG_LEVEL > 1
                    OSL_TRACE(
                        OUStringToOString(
                            OUString( "Selection acquired by: " ) +
                            getAccessibleName(),
                            RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                }
                break;

            case LOST_SELECTION:
                {
                    RemoveState( AccessibleStateType::SELECTED );
                    BroadcastAccEvent( AccessibleEventId::STATE_CHANGED, aEmpty, aSelected );

                    AddState( AccessibleStateType::FOCUSED );
                    aSelected <<= AccessibleStateType::FOCUSED;
                    BroadcastAccEvent( AccessibleEventId::STATE_CHANGED, aEmpty, aSelected, true );
#if OSL_DEBUG_LEVEL > 1
                    OSL_TRACE(
                        OUStringToOString(
                            OUString( "Selection lost by: " ) +
                            getAccessibleName(),
                            RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
                }
                break;

            case PROPERTY_CHANGE:
                {
                    //not implemented --> rebuild all
                }
                break;
        }
        return true;
    }
    else if( m_bMayHaveChildren )
    {
        bool bStop = false;

        ClearableMutexGuard aGuard( GetMutex() );
        // make local copy for notification
        ChildListVectorType aLocalChildList( m_aChildList );
        aGuard.clear();

        ChildListVectorType::iterator aEndIter = aLocalChildList.end();
        for( ChildListVectorType::iterator aIter = aLocalChildList.begin() ;
             ( aIter != aEndIter ) && ( ! bStop ) ;
             ++aIter )
        {
            // Note: at this place we must be sure to have an AccessibleBase
            // object in the UNO reference to XAccessible !
            bStop = (*static_cast< AccessibleBase * >
                     ( (*aIter).get() )).NotifyEvent( eEventType, rId );
        }
        return bStop;
    }

    return false;
}

void AccessibleBase::AddState( sal_Int16 aState )
    throw (RuntimeException)
{
    CheckDisposeState();
    OSL_ASSERT( m_pStateSetHelper );
    m_pStateSetHelper->AddState( aState );
}

void AccessibleBase::RemoveState( sal_Int16 aState )
    throw (RuntimeException)
{
    CheckDisposeState();
    OSL_ASSERT( m_pStateSetHelper );
    m_pStateSetHelper->RemoveState( aState );
}

bool AccessibleBase::UpdateChildren()
{
    bool bMustUpdateChildren = false;
    {
        MutexGuard aGuard( GetMutex() );
        if( ! m_bMayHaveChildren ||
            m_bIsDisposed )
            return false;

        bMustUpdateChildren = ( m_bMayHaveChildren &&
                                ! m_bChildrenInitialized );
    }

    // update unguarded
    if( bMustUpdateChildren )
        m_bChildrenInitialized = ImplUpdateChildren();

    return m_bChildrenInitialized;
}

bool AccessibleBase::ImplUpdateChildren()
{
    bool bResult = false;

    if( m_aAccInfo.m_spObjectHierarchy )
    {
        ObjectHierarchy::tChildContainer aModelChildren(
            m_aAccInfo.m_spObjectHierarchy->getChildren( GetId() ));
        ::std::vector< ChildOIDMap::key_type > aAccChildren;
        aAccChildren.reserve( aModelChildren.size());
        ::std::transform( m_aChildOIDMap.begin(), m_aChildOIDMap.end(),
                          ::std::back_inserter( aAccChildren ),
                          ::o3tl::select1st< ChildOIDMap::value_type >());

        ::std::sort( aModelChildren.begin(), aModelChildren.end());

        ::std::vector< ObjectHierarchy::tOID > aChildrenToRemove, aChildrenToAdd;
        ::std::set_difference( aModelChildren.begin(), aModelChildren.end(),
                               aAccChildren.begin(), aAccChildren.end(),
                               ::std::back_inserter( aChildrenToAdd ));
        ::std::set_difference( aAccChildren.begin(), aAccChildren.end(),
                               aModelChildren.begin(), aModelChildren.end(),
                               ::std::back_inserter( aChildrenToRemove ));

        ::std::vector< ObjectHierarchy::tOID >::const_iterator aIt( aChildrenToRemove.begin());
        for( ; aIt != aChildrenToRemove.end(); ++aIt )
        {
            RemoveChildByOId( *aIt );
        }

        AccessibleElementInfo aAccInfo( GetInfo());
        aAccInfo.m_pParent = this;

        for( aIt = aChildrenToAdd.begin(); aIt != aChildrenToAdd.end(); ++aIt )
        {
            aAccInfo.m_aOID = *aIt;
            if ( aIt->isAutoGeneratedObject() )
            {
                AddChild( ChartElementFactory::CreateChartElement( aAccInfo ) );
            }
            else if ( aIt->isAdditionalShape() )
            {
                AddChild( new AccessibleChartShape( aAccInfo, true, false ) );
            }
        }
        bResult = true;
    }

    return bResult;
}

void AccessibleBase::AddChild( AccessibleBase * pChild  )
{
    OSL_ENSURE( pChild != NULL, "Invalid Child" );
    if( pChild )
    {
        ClearableMutexGuard aGuard( GetMutex() );

        Reference< XAccessible > xChild( pChild );
        m_aChildList.push_back( xChild );

        m_aChildOIDMap[ pChild->GetId() ] = xChild;

        // inform listeners of new child
        if( m_bChildrenInitialized )
        {
            Any aEmpty, aNew;
            aNew <<= xChild;

            aGuard.clear();
            BroadcastAccEvent( AccessibleEventId::CHILD, aNew, aEmpty );
        }
    }
}

/** in this method we imply that the Reference< XAccessible > elements in the
    vector are AccessibleBase objects !
 */
void AccessibleBase::RemoveChildByOId( const ObjectIdentifier& rOId )
{
    ClearableMutexGuard aGuard( GetMutex() );

    ChildOIDMap::iterator aIt( m_aChildOIDMap.find( rOId ));
    if( aIt != m_aChildOIDMap.end())
    {
        Reference< XAccessible > xChild( aIt->second );

        // remove from map
        m_aChildOIDMap.erase( aIt );

        // search child in vector
        ChildListVectorType::iterator aVecIter =
            ::std::find( m_aChildList.begin(), m_aChildList.end(), xChild );

        OSL_ENSURE( aVecIter != m_aChildList.end(),
                    "Inconsistent ChildMap" );

        // remove child from vector
        m_aChildList.erase( aVecIter );
        bool bInitialized = m_bChildrenInitialized;

        // call listeners unguarded
        aGuard.clear();

        // inform listeners of removed child
        if( bInitialized )
        {
            Any aEmpty, aOld;
            aOld <<= xChild;

            BroadcastAccEvent( AccessibleEventId::CHILD, aEmpty, aOld );
        }

        // dispose the child
        Reference< lang::XComponent > xComp( xChild, UNO_QUERY );
        if( xComp.is())
            xComp->dispose();
    }
}

awt::Point AccessibleBase::GetUpperLeftOnScreen() const
{
    awt::Point aResult;
    if( m_aAccInfo.m_pParent )
    {
        ClearableMutexGuard aGuard( GetMutex() );
        AccessibleBase * pParent = m_aAccInfo.m_pParent;
        aGuard.clear();

        if( pParent )
        {
            aResult = pParent->GetUpperLeftOnScreen();
        }
        else
            OSL_FAIL( "Default position used is probably incorrect." );
    }

    return aResult;
}

void AccessibleBase::BroadcastAccEvent(
    sal_Int16 nId,
    const Any & rNew,
    const Any & rOld,
    bool bSendGlobally ) const
{
    ClearableMutexGuard aGuard( GetMutex() );

    if ( !m_nEventNotifierId && !bSendGlobally )
        return;
        // if we don't have a client id for the notifier, then we don't have listeners, then
        // we don't need to notify anything
        //except SendGlobally for focus handling?

    // the const cast is needed, because UNO parameters are never const
    const AccessibleEventObject aEvent(
        const_cast< uno::XWeak * >( static_cast< const uno::XWeak * >( this )),
        nId, rNew, rOld );

    if ( m_nEventNotifierId ) // let the notifier handle this event
        ::comphelper::AccessibleEventNotifier::addEvent( m_nEventNotifierId, aEvent );

    aGuard.clear();

    // send event to global message queue
    if( bSendGlobally )
    {
        ::vcl::unohelper::NotifyAccessibleStateEventGlobally( aEvent );
    }
}

void AccessibleBase::KillAllChildren()
{
    ClearableMutexGuard aGuard( GetMutex() );

    // make local copy for notification
    ChildListVectorType aLocalChildList( m_aChildList );

    // remove all children
    m_aChildList.clear();
    m_aChildOIDMap.clear();

    aGuard.clear();

    // call dispose for all children
    // and notify listeners
    Reference< lang::XComponent > xComp;
    Any aEmpty, aOld;
    ChildListVectorType::const_iterator aEndIter = aLocalChildList.end();
    for( ChildListVectorType::const_iterator aIter = aLocalChildList.begin();
         aIter != aEndIter; ++aIter )
    {
        aOld <<= (*aIter);
        BroadcastAccEvent( AccessibleEventId::CHILD, aEmpty, aOld );

        xComp.set( *aIter, UNO_QUERY );
        if( xComp.is())
            xComp->dispose();
    }
    m_bChildrenInitialized = false;
}

AccessibleElementInfo AccessibleBase::GetInfo() const
{
    return m_aAccInfo;
}

void AccessibleBase::SetInfo( const AccessibleElementInfo & rNewInfo )
{
    m_aAccInfo = rNewInfo;
    if( m_bMayHaveChildren )
    {
        KillAllChildren();
    }
    BroadcastAccEvent( AccessibleEventId::INVALIDATE_ALL_CHILDREN, uno::Any(), uno::Any(),
                       true /* global notification */ );
}

AccessibleUniqueId AccessibleBase::GetId() const
{
    return m_aAccInfo.m_aOID;
}

// ________ (XComponent::dispose) ________
void SAL_CALL AccessibleBase::disposing()
{
    ClearableMutexGuard aGuard( GetMutex() );
    OSL_ENSURE( ! m_bIsDisposed, "dispose() called twice" );

    // notify disposing to all AccessibleEvent listeners asynchron
    if ( m_nEventNotifierId )
    {
        ::comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( m_nEventNotifierId, *this );
        m_nEventNotifierId = 0;
    }

    // reset pointers
    m_aAccInfo.m_pParent = NULL;

    // invalidate implementation for helper, but keep UNO reference to still
    // allow a tool to query the DEFUNC state.
    // Note: The object will be deleted when the last reference is released
    m_pStateSetHelper = NULL;

    // attach new empty state set helper to member reference
    ::utl::AccessibleStateSetHelper * pHelper = new ::utl::AccessibleStateSetHelper();
    pHelper->AddState( AccessibleStateType::DEFUNC );
    // release old helper and attach new one
    m_aStateSet.set( pHelper );

    m_bIsDisposed = true;

    // call listeners unguarded
    aGuard.clear();

    if( m_bMayHaveChildren )
    {
        KillAllChildren();
    }
    else
        OSL_ENSURE( m_aChildList.empty(), "Child list should be empty" );
}

// ________ XAccessible ________
Reference< XAccessibleContext > SAL_CALL AccessibleBase::getAccessibleContext()
    throw (RuntimeException)
{
    return this;
}

// ________ AccessibleBase::XAccessibleContext ________
sal_Int32 SAL_CALL AccessibleBase::getAccessibleChildCount()
    throw (RuntimeException)
{
    ClearableMutexGuard aGuard( GetMutex() );
    if( ! m_bMayHaveChildren ||
        m_bIsDisposed )
        return 0;

    bool bMustUpdateChildren = ( m_bMayHaveChildren &&
                                 ! m_bChildrenInitialized );

    aGuard.clear();

    // update unguarded
    if( bMustUpdateChildren )
        UpdateChildren();

    return ImplGetAccessibleChildCount();
}

sal_Int32 AccessibleBase::ImplGetAccessibleChildCount() const
    throw (RuntimeException)
{
    return m_aChildList.size();
}

Reference< XAccessible > SAL_CALL AccessibleBase::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, RuntimeException)
{
    CheckDisposeState();
    Reference< XAccessible > xResult;

    ResettableMutexGuard aGuard( GetMutex() );
    bool bMustUpdateChildren = ( m_bMayHaveChildren &&
                                 ! m_bChildrenInitialized );

    aGuard.clear();

    if( bMustUpdateChildren )
        UpdateChildren();

    xResult.set( ImplGetAccessibleChildById( i ));

    return xResult;
}

Reference< XAccessible > AccessibleBase::ImplGetAccessibleChildById( sal_Int32 i ) const
    throw (lang::IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessible > xResult;

    MutexGuard aGuard( GetMutex());
    if( ! m_bMayHaveChildren ||
        i < 0 ||
        static_cast< ChildListVectorType::size_type >( i ) >= m_aChildList.size() )
    {
        OUString aBuf = "Index " + OUString::number( i ) + " is invalid for range [ 0, " +
                        OUString::number( m_aChildList.size() - 1 ) +
                        " ]";
        lang::IndexOutOfBoundsException aEx( aBuf,
                                             const_cast< ::cppu::OWeakObject * >(
                                                 static_cast< const ::cppu::OWeakObject * >( this )));
        throw aEx;
    }
    else
        xResult.set( m_aChildList[ i ] );

    return xResult;
}

Reference< XAccessible > SAL_CALL AccessibleBase::getAccessibleParent()
    throw (RuntimeException)
{
    CheckDisposeState();
    Reference< XAccessible > aResult;
    if( m_aAccInfo.m_pParent )
        aResult.set( m_aAccInfo.m_pParent );

    return aResult;
}

sal_Int32 SAL_CALL AccessibleBase::getAccessibleIndexInParent()
    throw (RuntimeException)
{
    CheckDisposeState();

    if( m_aAccInfo.m_spObjectHierarchy )
        return m_aAccInfo.m_spObjectHierarchy->getIndexInParent( GetId() );
    return -1;
}

sal_Int16 SAL_CALL AccessibleBase::getAccessibleRole()
    throw (RuntimeException)
{
    return AccessibleRole::LIST_ITEM; // #i73747# role SHAPE seems more appropriate, but is not read
}

Reference< XAccessibleRelationSet > SAL_CALL AccessibleBase::getAccessibleRelationSet()
    throw (RuntimeException)
{
    Reference< XAccessibleRelationSet > aResult;
    return aResult;
}

Reference< XAccessibleStateSet > SAL_CALL AccessibleBase::getAccessibleStateSet()
    throw (RuntimeException)
{
    if( ! m_bStateSetInitialized )
    {
        Reference< view::XSelectionSupplier > xSelSupp( GetInfo().m_xSelectionSupplier );
        if ( xSelSupp.is() )
        {
            ObjectIdentifier aOID( xSelSupp->getSelection() );
            if ( aOID.isValid() && GetId() == aOID )
            {
                AddState( AccessibleStateType::SELECTED );
                AddState( AccessibleStateType::FOCUSED );
            }
        }
        m_bStateSetInitialized = true;
    }

    return m_aStateSet;
}

lang::Locale SAL_CALL AccessibleBase::getLocale()
    throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    CheckDisposeState();

    return Application::GetSettings().GetLanguageTag().getLocale();
}

// ________ AccessibleBase::XAccessibleComponent ________
sal_Bool SAL_CALL AccessibleBase::containsPoint( const awt::Point& aPoint )
    throw (RuntimeException)
{
    awt::Rectangle aRect( getBounds() );

    // contains() works with relative coordinates
    aRect.X = 0;
    aRect.Y = 0;

    return ( aPoint.X >= aRect.X &&
             aPoint.Y >= aRect.Y &&
             aPoint.X < (aRect.X + aRect.Width) &&
             aPoint.Y < (aRect.Y + aRect.Height) );
}

Reference< XAccessible > SAL_CALL AccessibleBase::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (RuntimeException)
{
    CheckDisposeState();
    Reference< XAccessible > aResult;
    awt::Rectangle aRect( getBounds());

    // children are positioned relative to this object, so translate bound rect
    aRect.X = 0;
    aRect.Y = 0;

    // children must be inside the own bound rect
    if( ( aRect.X <= aPoint.X && aPoint.X <= (aRect.X + aRect.Width) ) &&
        ( aRect.Y <= aPoint.Y && aPoint.Y <= (aRect.Y + aRect.Height)))
    {
        ClearableMutexGuard aGuard( GetMutex() );
        ChildListVectorType aLocalChildList( m_aChildList );
        aGuard.clear();

        Reference< XAccessibleComponent > aComp;
        for( ChildListVectorType::const_iterator aIter = aLocalChildList.begin();
             aIter != aLocalChildList.end(); ++aIter )
        {
            aComp.set( *aIter, UNO_QUERY );
            if( aComp.is())
            {
                aRect = aComp->getBounds();
                if( ( aRect.X <= aPoint.X && aPoint.X <= (aRect.X + aRect.Width) ) &&
                    ( aRect.Y <= aPoint.Y && aPoint.Y <= (aRect.Y + aRect.Height)))
                {
                    aResult = (*aIter);
                    break;
                }
            }
        }
    }

    return aResult;
}

awt::Rectangle SAL_CALL AccessibleBase::getBounds()
    throw (RuntimeException)
{
    ExplicitValueProvider *pExplicitValueProvider(
        ExplicitValueProvider::getExplicitValueProvider( m_aAccInfo.m_xView ));
    if( pExplicitValueProvider )
    {
        Window* pWindow( VCLUnoHelper::GetWindow( m_aAccInfo.m_xWindow ));
        awt::Rectangle aLogicRect( pExplicitValueProvider->getRectangleOfObject( m_aAccInfo.m_aOID.getObjectCID() ));
        if( pWindow )
        {
            Rectangle aRect( aLogicRect.X, aLogicRect.Y,
                             aLogicRect.X + aLogicRect.Width,
                             aLogicRect.Y + aLogicRect.Height );
            SolarMutexGuard aSolarGuard;
            aRect = pWindow->LogicToPixel( aRect );

            // aLogicRect ist relative to the page, but we need a value relative
            // to the parent object
            awt::Point aParentLocOnScreen;
            uno::Reference< XAccessibleComponent > xParent( getAccessibleParent(), uno::UNO_QUERY );
            if( xParent.is() )
                aParentLocOnScreen = xParent->getLocationOnScreen();

            awt::Point aULOnScreen = GetUpperLeftOnScreen();
            awt::Point aOffset( aParentLocOnScreen.X - aULOnScreen.X,
                                aParentLocOnScreen.Y - aULOnScreen.Y );

            return awt::Rectangle( aRect.getX() - aOffset.X, aRect.getY() - aOffset.Y,
                                   aRect.getWidth(), aRect.getHeight());
        }
    }

    return awt::Rectangle();
}

awt::Point SAL_CALL AccessibleBase::getLocation()
    throw (RuntimeException)
{
    CheckDisposeState();
    awt::Rectangle aBBox( getBounds() );
    return awt::Point( aBBox.X, aBBox.Y );
}

awt::Point SAL_CALL AccessibleBase::getLocationOnScreen()
    throw (RuntimeException)
{
    CheckDisposeState();

    if( m_aAccInfo.m_pParent != NULL )
    {
        AccessibleBase * pParent = m_aAccInfo.m_pParent;
        awt::Point aLocThisRel( getLocation());
        awt::Point aUpperLeft;

        if( pParent != NULL )
            aUpperLeft = pParent->getLocationOnScreen();

        return  awt::Point( aUpperLeft.X + aLocThisRel.X,
                            aUpperLeft.Y + aLocThisRel.Y );
    }
    else
        return getLocation();
}

awt::Size SAL_CALL AccessibleBase::getSize()
    throw (RuntimeException)
{
    CheckDisposeState();
    awt::Rectangle aBBox( getBounds() );
    return awt::Size( aBBox.Width, aBBox.Height );
}

void SAL_CALL AccessibleBase::grabFocus()
    throw (RuntimeException)
{
    CheckDisposeState();

    Reference< view::XSelectionSupplier > xSelSupp( GetInfo().m_xSelectionSupplier );
    if ( xSelSupp.is() )
    {
        xSelSupp->select( GetId().getAny() );
    }
}

sal_Int32 SAL_CALL AccessibleBase::getForeground()
    throw (RuntimeException)
{
    return getColor( ACC_BASE_FOREGROUND );
}

sal_Int32 SAL_CALL AccessibleBase::getBackground()
    throw (RuntimeException)
{
    return getColor( ACC_BASE_BACKGROUND );
}

sal_Int32 AccessibleBase::getColor( eColorType eColType )
{
    sal_Int32 nResult = static_cast< sal_Int32 >( Color( COL_TRANSPARENT ).GetColor());
    if( m_bAlwaysTransparent )
        return nResult;

    ObjectIdentifier aOID( m_aAccInfo.m_aOID );
    ObjectType eType( aOID.getObjectType() );
    Reference< beans::XPropertySet > xObjProp;
    OUString aObjectCID = aOID.getObjectCID();
    if( eType == OBJECTTYPE_LEGEND_ENTRY )
    {
        // for colors get the data series/point properties
        OUString aParentParticle( ObjectIdentifier::getFullParentParticle( aObjectCID ));
        aObjectCID = ObjectIdentifier::createClassifiedIdentifierForParticle( aParentParticle );
    }

    xObjProp.set(
        ObjectIdentifier::getObjectPropertySet(
            aObjectCID, Reference< chart2::XChartDocument >( m_aAccInfo.m_xChartDocument )), uno::UNO_QUERY );
    if( xObjProp.is())
    {
        try
        {
            OUString aPropName;
            OUString aStylePropName;

            switch( eType )
            {
                case OBJECTTYPE_LEGEND_ENTRY:
                case OBJECTTYPE_DATA_SERIES:
                case OBJECTTYPE_DATA_POINT:
                    if( eColType == ACC_BASE_FOREGROUND )
                    {
                        aPropName = "BorderColor";
                        aStylePropName = "BorderTransparency";
                    }
                    else
                    {
                        aPropName = "Color";
                        aStylePropName = "Transparency";
                    }
                    break;
                default:
                    if( eColType == ACC_BASE_FOREGROUND )
                    {
                        aPropName = "LineColor";
                        aStylePropName = "LineTransparence";
                    }
                    else
                    {
                        aPropName = "FillColor";
                        aStylePropName = "FillTransparence";
                    }
                    break;
            }

            bool bTransparent = m_bAlwaysTransparent;
            Reference< beans::XPropertySetInfo > xInfo( xObjProp->getPropertySetInfo(), uno::UNO_QUERY );
            if( xInfo.is() &&
                xInfo->hasPropertyByName( aStylePropName ))
            {
                if( eColType == ACC_BASE_FOREGROUND )
                {
                    drawing::LineStyle aLStyle;
                    if( xObjProp->getPropertyValue( aStylePropName ) >>= aLStyle )
                        bTransparent = (aLStyle == drawing::LineStyle_NONE);
                }
                else
                {
                    drawing::FillStyle aFStyle;
                    if( xObjProp->getPropertyValue( aStylePropName ) >>= aFStyle )
                        bTransparent = (aFStyle == drawing::FillStyle_NONE);
                }
            }

            if( !bTransparent &&
                xInfo.is() &&
                xInfo->hasPropertyByName( aPropName ))
            {
                xObjProp->getPropertyValue( aPropName ) >>= nResult;
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return nResult;
}

// ________ AccessibleBase::XServiceInfo ________
OUString SAL_CALL AccessibleBase::getImplementationName()
    throw (RuntimeException)
{
    return OUString( "AccessibleBase" );
}

sal_Bool SAL_CALL AccessibleBase::supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL AccessibleBase::getSupportedServiceNames()
    throw (RuntimeException)
{
    uno::Sequence< OUString > aSeq( 2 );
    OUString* pStr = aSeq.getArray();
    pStr[ 0 ] = "com.sun.star.accessibility.Accessible";
    pStr[ 1 ] = "com.sun.star.accessibility.AccessibleContext";

    return aSeq;
}

// ________ AccessibleBase::XEventListener ________
void SAL_CALL AccessibleBase::disposing( const lang::EventObject& /*Source*/ )
    throw (RuntimeException)
{
}

// ________ XAccessibleEventBroadcasters ________
void SAL_CALL AccessibleBase::addAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
    throw (RuntimeException)
{
    MutexGuard aGuard( GetMutex() );

    if ( xListener.is() )
    {
        if ( !m_nEventNotifierId )
            m_nEventNotifierId = ::comphelper::AccessibleEventNotifier::registerClient();

        ::comphelper::AccessibleEventNotifier::addEventListener( m_nEventNotifierId, xListener );
    }
}

void SAL_CALL AccessibleBase::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
    throw (RuntimeException)
{
    MutexGuard aGuard( GetMutex() );

    if ( xListener.is() )
    {
        sal_Int32 nListenerCount = ::comphelper::AccessibleEventNotifier::removeEventListener( m_nEventNotifierId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            ::comphelper::AccessibleEventNotifier::revokeClient( m_nEventNotifierId );
            m_nEventNotifierId = 0;
        }
    }
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
