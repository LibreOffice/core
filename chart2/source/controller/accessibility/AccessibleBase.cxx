/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <rtl/ustrbuf.hxx>

#include <vcl/svapp.hxx>
#include <rtl/uuid.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
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
#include <vcl/graph.hxx>
#include <vcl/settings.hxx>

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
                    
                }
                break;
        }
        return true;
    }
    else if( m_bMayHaveChildren )
    {
        bool bStop = false;

        ClearableMutexGuard aGuard( GetMutex() );
        
        ChildListVectorType aLocalChildList( m_aChildList );
        aGuard.clear();

        ChildListVectorType::iterator aEndIter = aLocalChildList.end();
        for( ChildListVectorType::iterator aIter = aLocalChildList.begin() ;
             ( aIter != aEndIter ) && ( ! bStop ) ;
             ++aIter )
        {
            
            
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

        
        m_aChildOIDMap.erase( aIt );

        
        ChildListVectorType::iterator aVecIter =
            ::std::find( m_aChildList.begin(), m_aChildList.end(), xChild );

        OSL_ENSURE( aVecIter != m_aChildList.end(),
                    "Inconsistent ChildMap" );

        
        m_aChildList.erase( aVecIter );
        bool bInitialized = m_bChildrenInitialized;

        
        aGuard.clear();

        
        if( bInitialized )
        {
            Any aEmpty, aOld;
            aOld <<= xChild;

            BroadcastAccEvent( AccessibleEventId::CHILD, aEmpty, aOld );
        }

        
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
        
        
        

    
    const AccessibleEventObject aEvent(
        const_cast< uno::XWeak * >( static_cast< const uno::XWeak * >( this )),
        nId, rNew, rOld );

    if ( m_nEventNotifierId ) 
        ::comphelper::AccessibleEventNotifier::addEvent( m_nEventNotifierId, aEvent );

    aGuard.clear();

    
    if( bSendGlobally )
    {
        ::vcl::unohelper::NotifyAccessibleStateEventGlobally( aEvent );
    }
}

void AccessibleBase::KillAllChildren()
{
    ClearableMutexGuard aGuard( GetMutex() );

    
    ChildListVectorType aLocalChildList( m_aChildList );

    
    m_aChildList.clear();
    m_aChildOIDMap.clear();

    aGuard.clear();

    
    
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


void SAL_CALL AccessibleBase::disposing()
{
    ClearableMutexGuard aGuard( GetMutex() );
    OSL_ENSURE( ! m_bIsDisposed, "dispose() called twice" );

    
    if ( m_nEventNotifierId )
    {
        ::comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( m_nEventNotifierId, *this );
        m_nEventNotifierId = 0;
    }

    
    m_aAccInfo.m_pParent = NULL;

    
    
    
    m_pStateSetHelper = NULL;

    
    ::utl::AccessibleStateSetHelper * pHelper = new ::utl::AccessibleStateSetHelper();
    pHelper->AddState( AccessibleStateType::DEFUNC );
    
    m_aStateSet.set( pHelper );

    m_bIsDisposed = true;

    
    aGuard.clear();

    if( m_bMayHaveChildren )
    {
        KillAllChildren();
    }
    else
        OSL_ENSURE( m_aChildList.empty(), "Child list should be empty" );
}


Reference< XAccessibleContext > SAL_CALL AccessibleBase::getAccessibleContext()
    throw (RuntimeException)
{
    return this;
}


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
    return AccessibleRole::SHAPE;
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


sal_Bool SAL_CALL AccessibleBase::containsPoint( const awt::Point& aPoint )
    throw (RuntimeException)
{
    awt::Rectangle aRect( getBounds() );

    
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

    
    aRect.X = 0;
    aRect.Y = 0;

    
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


OUString SAL_CALL AccessibleBase::getImplementationName()
    throw (RuntimeException)
{
    return OUString( "AccessibleBase" );
}

sal_Bool SAL_CALL AccessibleBase::supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return cppu::supportsService( this, ServiceName );
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


void SAL_CALL AccessibleBase::disposing( const lang::EventObject& /*Source*/ )
    throw (RuntimeException)
{
}


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
            
            ::comphelper::AccessibleEventNotifier::revokeClient( m_nEventNotifierId );
            m_nEventNotifierId = 0;
        }
    }
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
