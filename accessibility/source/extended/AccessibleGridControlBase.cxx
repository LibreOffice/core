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

#include <accessibility/extended/AccessibleGridControlBase.hxx>
#include <svtools/accessibletable.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/types.h>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblerelationsethelper.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;
using namespace ::svt;
using namespace ::svt::table;


namespace accessibility {

using namespace com::sun::star::accessibility::AccessibleStateType;


AccessibleGridControlBase::AccessibleGridControlBase(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
        ::svt::table::IAccessibleTable& rTable,
        ::svt::table::AccessibleTableControlObjType      eObjType ) :
    AccessibleGridControlImplHelper( m_aMutex ),
    m_xParent( rxParent ),
    m_aTable( rTable),
    m_eObjType( eObjType ),
    m_aName( rTable.GetAccessibleObjectName( eObjType, 0, 0 ) ),
    m_aDescription( rTable.GetAccessibleObjectDescription( eObjType ) ),
    m_aClientId(0)
{
}

AccessibleGridControlBase::~AccessibleGridControlBase()
{
    if( isAlive() )
    {
        // increment ref count to prevent double call of Dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

void SAL_CALL AccessibleGridControlBase::disposing()
{
    SolarMutexGuard g;

    if ( getClientId( ) )
    {
        AccessibleEventNotifier::TClientId nId( getClientId( ) );
        setClientId( 0 );
        AccessibleEventNotifier::revokeClientNotifyDisposing( nId, *this );
    }

    m_xParent = nullptr;
    //m_aTable = NULL;
}

// css::accessibility::XAccessibleContext

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL AccessibleGridControlBase::getAccessibleParent()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();
    return m_xParent;
}

sal_Int32 SAL_CALL AccessibleGridControlBase::getAccessibleIndexInParent()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();

    // -1 for child not found/no parent (according to specification)
    sal_Int32 nRet = -1;

    css::uno::Reference< uno::XInterface > xMeMyselfAndI( static_cast< css::accessibility::XAccessibleContext* >( this ), uno::UNO_QUERY );

    //  iterate over parent's children and search for this object
    if( m_xParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( m_xParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
        css::uno::Reference< uno::XInterface > xChild;

            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int32 nChild = 0; nChild < nChildCount; ++nChild )
            {
            xChild.set(xParentContext->getAccessibleChild( nChild ), css::uno::UNO_QUERY);
            if ( xMeMyselfAndI.get() == xChild.get() )
            {
                nRet = nChild;
                break;
            }
        }
        }
   }
   return nRet;
}

OUString SAL_CALL AccessibleGridControlBase::getAccessibleDescription()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();
    return m_aDescription;
}

OUString SAL_CALL AccessibleGridControlBase::getAccessibleName()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();
    return m_aName;
}

css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
AccessibleGridControlBase::getAccessibleRelationSet()
    throw ( uno::RuntimeException, std::exception )
{
   SolarMutexGuard g;

   ensureIsAlive();
   // GridControl does not have relations.
   return new utl::AccessibleRelationSetHelper;
}

css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
AccessibleGridControlBase::getAccessibleStateSet()
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;

    // don't check whether alive -> StateSet may contain DEFUNC
    return implCreateStateSetHelper();
}

lang::Locale SAL_CALL AccessibleGridControlBase::getLocale()
    throw ( IllegalAccessibleComponentStateException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();
    if( m_xParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( m_xParent->getAccessibleContext() );
        if( xParentContext.is() )
        return xParentContext->getLocale();
    }
    throw IllegalAccessibleComponentStateException();
}

// css::accessibility::XAccessibleComponent

sal_Bool SAL_CALL AccessibleGridControlBase::containsPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException, std::exception )
{
   return Rectangle( Point(), getBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

awt::Rectangle SAL_CALL AccessibleGridControlBase::getBounds()
    throw ( uno::RuntimeException, std::exception )
{
   return AWTRectangle( getBoundingBox() );
}

awt::Point SAL_CALL AccessibleGridControlBase::getLocation()
    throw ( uno::RuntimeException, std::exception )
{
    return AWTPoint( getBoundingBox().TopLeft() );
}

awt::Point SAL_CALL AccessibleGridControlBase::getLocationOnScreen()
    throw ( uno::RuntimeException, std::exception )
{
    return AWTPoint( getBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL AccessibleGridControlBase::getSize()
    throw ( uno::RuntimeException, std::exception )
{
    return AWTSize( getBoundingBox().GetSize() );
}

// css::accessibility::XAccessibleEventBroadcaster

void SAL_CALL AccessibleGridControlBase::addAccessibleEventListener(
        const css::uno::Reference< css::accessibility::XAccessibleEventListener>& _rxListener )
    throw ( uno::RuntimeException, std::exception )
{
    if ( _rxListener.is() )
    {
        SolarMutexGuard g;

        if ( !getClientId( ) )
            setClientId( AccessibleEventNotifier::registerClient( ) );

        AccessibleEventNotifier::addEventListener( getClientId( ), _rxListener );
    }
}

void SAL_CALL AccessibleGridControlBase::removeAccessibleEventListener(
        const css::uno::Reference< css::accessibility::XAccessibleEventListener>& _rxListener )
    throw ( uno::RuntimeException, std::exception )
{
    if( _rxListener.is() && getClientId( ) )
    {
        SolarMutexGuard g;

        sal_Int32 nListenerCount = AccessibleEventNotifier::removeEventListener( getClientId( ), _rxListener );
    if ( !nListenerCount )
    {
        // no listeners anymore
        // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
        // and at least to us not firing any events anymore, in case somebody calls
        // NotifyAccessibleEvent, again
        AccessibleEventNotifier::TClientId nId( getClientId( ) );
        setClientId( 0 );
        AccessibleEventNotifier::revokeClient( nId );
    }
    }
}

// XTypeProvider

Sequence< sal_Int8 > SAL_CALL AccessibleGridControlBase::getImplementationId()
    throw ( uno::RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo

sal_Bool SAL_CALL AccessibleGridControlBase::supportsService(
        const OUString& rServiceName )
    throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL AccessibleGridControlBase::getSupportedServiceNames()
    throw ( uno::RuntimeException, std::exception )
{
    const OUString aServiceName( "com.sun.star.accessibility.AccessibleContext" );
    return Sequence< OUString >( &aServiceName, 1 );
}
// internal virtual methods

bool AccessibleGridControlBase::implIsShowing()
{
    bool bShowing = false;
    if( m_xParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleComponent >
            xParentComp( m_xParent->getAccessibleContext(), uno::UNO_QUERY );
        if( xParentComp.is() )
            bShowing = implGetBoundingBox().IsOver(
                VCLRectangle( xParentComp->getBounds() ) );
    }
    return bShowing;
}

::utl::AccessibleStateSetHelper* AccessibleGridControlBase::implCreateStateSetHelper()
{
    ::utl::AccessibleStateSetHelper*
        pStateSetHelper = new ::utl::AccessibleStateSetHelper;

    if( isAlive() )
    {
        // SHOWING done with m_xParent
        if( implIsShowing() )
            pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        // GridControl fills StateSet with states depending on object type
        m_aTable.FillAccessibleStateSet( *pStateSetHelper, getType() );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
    return pStateSetHelper;
}

// internal helper methods

bool AccessibleGridControlBase::isAlive() const
{
    ::osl::MutexGuard g(m_aMutex); // guards rBHelper members
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

void AccessibleGridControlBase::ensureIsAlive() const
    throw ( lang::DisposedException )
{
    if( !isAlive() )
        throw lang::DisposedException();
}

Rectangle AccessibleGridControlBase::getBoundingBox()
    throw ( lang::DisposedException )
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
    Rectangle aRect = implGetBoundingBox();
    if ( 0 == aRect.Left() && 0 == aRect.Top() && 0 == aRect.Right() && 0 == aRect.Bottom() )
    {
        SAL_WARN( "accessibility", "rectangle doesn't exist" );
    }
    return aRect;
}

Rectangle AccessibleGridControlBase::getBoundingBoxOnScreen()
    throw ( lang::DisposedException )
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
    Rectangle aRect = implGetBoundingBoxOnScreen();
    if ( 0 == aRect.Left() && 0 == aRect.Top() && 0 == aRect.Right() && 0 == aRect.Bottom() )
    {
        SAL_WARN( "accessibility", "rectangle doesn't exist" );
    }
    return aRect;
}

void AccessibleGridControlBase::commitEvent(
        sal_Int16 _nEventId, const Any& _rNewValue, const Any& _rOldValue )
{
    SolarMutexGuard g;

    if ( !getClientId( ) )
            // if we don't have a client id for the notifier, then we don't have listeners, then
            // we don't need to notify anything
            return;

    // build an event object
    AccessibleEventObject aEvent;
    aEvent.Source = *this;
    aEvent.EventId = _nEventId;
    aEvent.OldValue = _rOldValue;
    aEvent.NewValue = _rNewValue;

    // let the notifier handle this event

    AccessibleEventNotifier::addEvent( getClientId( ), aEvent );
}

sal_Int16 SAL_CALL AccessibleGridControlBase::getAccessibleRole()
    throw ( uno::RuntimeException, std::exception )
{
    ensureIsAlive();
    sal_Int16 nRole = AccessibleRole::UNKNOWN;
    switch ( m_eObjType )
    {
        case TCTYPE_ROWHEADERCELL:
        nRole = AccessibleRole::ROW_HEADER;
        break;
    case TCTYPE_COLUMNHEADERCELL:
        nRole = AccessibleRole::COLUMN_HEADER;
        break;
    case TCTYPE_COLUMNHEADERBAR:
    case TCTYPE_ROWHEADERBAR:
    case TCTYPE_TABLE:
        nRole = AccessibleRole::TABLE;
        break;
    case TCTYPE_TABLECELL:
        nRole = AccessibleRole::TABLE_CELL;
        break;
    case TCTYPE_GRIDCONTROL:
        nRole = AccessibleRole::PANEL;
        break;
    }
    return nRole;
}

css::uno::Reference<css::accessibility::XAccessible > SAL_CALL AccessibleGridControlBase::getAccessibleAtPoint( const css::awt::Point& )
        throw ( uno::RuntimeException, std::exception )
{
    return nullptr;
}

sal_Int32 SAL_CALL AccessibleGridControlBase::getForeground(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();

    sal_Int32 nColor = 0;
    vcl::Window* pInst = m_aTable.GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlForeground() )
            nColor = pInst->GetControlForeground().GetColor();
    else
    {
        vcl::Font aFont;
        if ( pInst->IsControlFont() )
            aFont = pInst->GetControlFont();
        else
            aFont = pInst->GetFont();
        nColor = aFont.GetColor().GetColor();
    }
    }
    return nColor;
}

sal_Int32 SAL_CALL AccessibleGridControlBase::getBackground(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    sal_Int32 nColor = 0;
    vcl::Window* pInst = m_aTable.GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlBackground() )
            nColor = pInst->GetControlBackground().GetColor();
    else
            nColor = pInst->GetBackground().GetColor().GetColor();
    }
    return nColor;
}


GridControlAccessibleElement::GridControlAccessibleElement( const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                        ::svt::table::IAccessibleTable& rTable,
                        ::svt::table::AccessibleTableControlObjType  eObjType )
    :AccessibleGridControlBase( rxParent, rTable, eObjType )
{
}

// XInterface
IMPLEMENT_FORWARD_XINTERFACE2( GridControlAccessibleElement, AccessibleGridControlBase, GridControlAccessibleElement_Base)

// XTypeProvider
IMPLEMENT_FORWARD_XTYPEPROVIDER2( GridControlAccessibleElement, AccessibleGridControlBase, GridControlAccessibleElement_Base )

// css::accessibility::XAccessible

css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL GridControlAccessibleElement::getAccessibleContext() throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard g;

    ensureIsAlive();
    return this;
}

GridControlAccessibleElement::~GridControlAccessibleElement( )
{
}


} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
