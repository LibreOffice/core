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

#include <extended/AccessibleBrowseBoxBase.hxx>
#include <vcl/accessibletableprovider.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>


using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


namespace accessibility {

using namespace com::sun::star::accessibility::AccessibleStateType;


// Ctor/Dtor/disposing

AccessibleBrowseBoxBase::AccessibleBrowseBoxBase(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
        ::vcl::IAccessibleTableProvider&                      rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
        ::vcl::AccessibleBrowseBoxObjType      eObjType ) :
    AccessibleBrowseBoxImplHelper( m_aMutex ),
    mxParent( rxParent ),
    mpBrowseBox( &rBrowseBox ),
    m_xFocusWindow(_xFocusWindow),
    maName( rBrowseBox.GetAccessibleObjectName( eObjType ) ),
    maDescription( rBrowseBox.GetAccessibleObjectDescription( eObjType ) ),
    meObjType( eObjType ),
    m_aClientId(0)
{
    if ( m_xFocusWindow.is() )
        m_xFocusWindow->addFocusListener( this );
}

AccessibleBrowseBoxBase::AccessibleBrowseBoxBase(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
        ::vcl::IAccessibleTableProvider&                      rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
        ::vcl::AccessibleBrowseBoxObjType      eObjType,
        const OUString&          rName,
        const OUString&          rDescription ) :
    AccessibleBrowseBoxImplHelper( m_aMutex ),
    mxParent( rxParent ),
    mpBrowseBox( &rBrowseBox ),
    m_xFocusWindow(_xFocusWindow),
    maName( rName ),
    maDescription( rDescription ),
    meObjType( eObjType ),
    m_aClientId(0)
{
    if ( m_xFocusWindow.is() )
        m_xFocusWindow->addFocusListener( this );
}

AccessibleBrowseBoxBase::~AccessibleBrowseBoxBase()
{
    if( isAlive() )
    {
        // increment ref count to prevent double call of Dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

void SAL_CALL AccessibleBrowseBoxBase::disposing()
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( m_xFocusWindow.is() )
    {
        SolarMutexGuard aSolarGuard;
        m_xFocusWindow->removeFocusListener( this );
    }

    if ( getClientId( ) )
    {
        AccessibleEventNotifier::TClientId nId( getClientId( ) );
        setClientId( 0 );
        AccessibleEventNotifier::revokeClientNotifyDisposing( nId, *this );
    }

    mxParent = nullptr;
    mpBrowseBox = nullptr;
}

// css::accessibility::XAccessibleContext

Reference< css::accessibility::XAccessible > SAL_CALL AccessibleBrowseBoxBase::getAccessibleParent()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return mxParent;
}

sal_Int32 SAL_CALL AccessibleBrowseBoxBase::getAccessibleIndexInParent()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();

    // -1 for child not found/no parent (according to specification)
    sal_Int32 nRet = -1;

    css::uno::Reference< uno::XInterface > xMeMyselfAndI( static_cast< css::accessibility::XAccessibleContext* >( this ), uno::UNO_QUERY );

    //  iterate over parent's children and search for this object
    if( mxParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( mxParent->getAccessibleContext() );
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

OUString SAL_CALL AccessibleBrowseBoxBase::getAccessibleDescription()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return maDescription;
}

OUString SAL_CALL AccessibleBrowseBoxBase::getAccessibleName()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return maName;
}

Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
AccessibleBrowseBoxBase::getAccessibleRelationSet()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    // BrowseBox does not have relations.
    return new utl::AccessibleRelationSetHelper;
}

Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
AccessibleBrowseBoxBase::getAccessibleStateSet()
{
    SolarMethodGuard aGuard( getMutex() );
    // don't check whether alive -> StateSet may contain DEFUNC
    return implCreateStateSetHelper();
}

lang::Locale SAL_CALL AccessibleBrowseBoxBase::getLocale()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    if( mxParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }
    throw IllegalAccessibleComponentStateException();
}

// css::accessibility::XAccessibleComponent

sal_Bool SAL_CALL AccessibleBrowseBoxBase::containsPoint( const css::awt::Point& rPoint )
{
    return tools::Rectangle( Point(), getBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

awt::Rectangle SAL_CALL AccessibleBrowseBoxBase::getBounds()
{
    return AWTRectangle( getBoundingBox() );
}

awt::Point SAL_CALL AccessibleBrowseBoxBase::getLocation()
{
    return AWTPoint( getBoundingBox().TopLeft() );
}

awt::Point SAL_CALL AccessibleBrowseBoxBase::getLocationOnScreen()
{
    return AWTPoint( getBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL AccessibleBrowseBoxBase::getSize()
{
    return AWTSize( getBoundingBox().GetSize() );
}

void SAL_CALL AccessibleBrowseBoxBase::focusGained( const css::awt::FocusEvent& )
{
    com::sun::star::uno::Any aFocused;
    com::sun::star::uno::Any aEmpty;
    aFocused <<= FOCUSED;

    commitEvent(AccessibleEventId::STATE_CHANGED,aFocused,aEmpty);
}


void SAL_CALL AccessibleBrowseBoxBase::focusLost( const css::awt::FocusEvent& )
{
    com::sun::star::uno::Any aFocused;
    com::sun::star::uno::Any aEmpty;
    aFocused <<= FOCUSED;

    commitEvent(AccessibleEventId::STATE_CHANGED,aEmpty,aFocused);
}
// css::accessibility::XAccessibleEventBroadcaster

void SAL_CALL AccessibleBrowseBoxBase::addAccessibleEventListener(
        const css::uno::Reference< css::accessibility::XAccessibleEventListener>& _rxListener )
{
    if ( _rxListener.is() )
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( !getClientId( ) )
            setClientId( AccessibleEventNotifier::registerClient( ) );

        AccessibleEventNotifier::addEventListener( getClientId( ), _rxListener );
    }
}

void SAL_CALL AccessibleBrowseBoxBase::removeAccessibleEventListener(
        const css::uno::Reference< css::accessibility::XAccessibleEventListener>& _rxListener )
{
    if( _rxListener.is() && getClientId( ) )
    {
        ::osl::MutexGuard aGuard( getMutex() );
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

Sequence< sal_Int8 > SAL_CALL AccessibleBrowseBoxBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo

sal_Bool SAL_CALL AccessibleBrowseBoxBase::supportsService(
        const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL AccessibleBrowseBoxBase::getSupportedServiceNames()
{
    const OUString aServiceName( "com.sun.star.accessibility.AccessibleContext" );
    return Sequence< OUString >( &aServiceName, 1 );
}

// other public methods

void AccessibleBrowseBoxBase::setAccessibleName( const OUString& rName )
{
    ::osl::ClearableMutexGuard aGuard( getMutex() );
    Any aOld;
    aOld <<= maName;
    maName = rName;

    aGuard.clear();

    commitEvent(
        AccessibleEventId::NAME_CHANGED,
        uno::Any( maName ),
        aOld );
}

void AccessibleBrowseBoxBase::setAccessibleDescription( const OUString& rDescription )
{
    ::osl::ClearableMutexGuard aGuard( getMutex() );
    Any aOld;
    aOld <<= maDescription;
    maDescription = rDescription;

    aGuard.clear();

    commitEvent(
        AccessibleEventId::DESCRIPTION_CHANGED,
        uno::Any( maDescription ),
        aOld );
}

// internal virtual methods

bool AccessibleBrowseBoxBase::implIsShowing()
{
    bool bShowing = false;
    if( mxParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleComponent >
            xParentComp( mxParent->getAccessibleContext(), uno::UNO_QUERY );
        if( xParentComp.is() )
            bShowing = implGetBoundingBox().IsOver(
                VCLRectangle( xParentComp->getBounds() ) );
    }
    return bShowing;
}

::utl::AccessibleStateSetHelper* AccessibleBrowseBoxBase::implCreateStateSetHelper()
{
    ::utl::AccessibleStateSetHelper*
        pStateSetHelper = new ::utl::AccessibleStateSetHelper;

    if( isAlive() )
    {
        // SHOWING done with mxParent
        if( implIsShowing() )
            pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        // BrowseBox fills StateSet with states depending on object type
        mpBrowseBox->FillAccessibleStateSet( *pStateSetHelper, getType() );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}

// internal helper methods

bool AccessibleBrowseBoxBase::isAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose && mpBrowseBox;
}

void AccessibleBrowseBoxBase::ensureIsAlive() const
{
    if( !isAlive() )
        throw lang::DisposedException();
}

tools::Rectangle AccessibleBrowseBoxBase::getBoundingBox()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    tools::Rectangle aRect = implGetBoundingBox();
    if ( aRect.Left() == 0 && aRect.Top() == 0 && aRect.Right() == 0 && aRect.Bottom() == 0 )
    {
        SAL_WARN( "accessibility", "rectangle doesn't exist" );
    }
    return aRect;
}

tools::Rectangle AccessibleBrowseBoxBase::getBoundingBoxOnScreen()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    tools::Rectangle aRect = implGetBoundingBoxOnScreen();
    if ( aRect.Left() == 0 && aRect.Top() == 0 && aRect.Right() == 0 && aRect.Bottom() == 0 )
    {
        SAL_WARN( "accessibility", "rectangle doesn't exist" );
    }
    return aRect;
}

void AccessibleBrowseBoxBase::commitEvent(
        sal_Int16 _nEventId, const Any& _rNewValue, const Any& _rOldValue )
{
    ::osl::ClearableMutexGuard aGuard( getMutex() );
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

sal_Int16 SAL_CALL AccessibleBrowseBoxBase::getAccessibleRole()
{
    osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    sal_Int16 nRole = AccessibleRole::UNKNOWN;
    switch ( meObjType )
    {
        case vcl::BBTYPE_ROWHEADERCELL:
            nRole = AccessibleRole::ROW_HEADER;
            break;
        case vcl::BBTYPE_COLUMNHEADERCELL:
            nRole = AccessibleRole::COLUMN_HEADER;
            break;
        case vcl::BBTYPE_COLUMNHEADERBAR:
        case vcl::BBTYPE_ROWHEADERBAR:
        case vcl::BBTYPE_TABLE:
            nRole = AccessibleRole::TABLE;
            break;
        case vcl::BBTYPE_TABLECELL:
            nRole = AccessibleRole::TABLE_CELL;
            break;
        case vcl::BBTYPE_BROWSEBOX:
            nRole = AccessibleRole::PANEL;
            break;
        case vcl::BBTYPE_CHECKBOXCELL:
            nRole = AccessibleRole::CHECK_BOX;
            break;
    }
    return nRole;
}

Reference<XAccessible > SAL_CALL AccessibleBrowseBoxBase::getAccessibleAtPoint( const css::awt::Point& )
{
    return nullptr;
}

void SAL_CALL AccessibleBrowseBoxBase::disposing( const css::lang::EventObject& )
{
    m_xFocusWindow = nullptr;
}

sal_Int32 SAL_CALL AccessibleBrowseBoxBase::getForeground(  )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Color nColor;
    vcl::Window* pInst = mpBrowseBox->GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlForeground() )
            nColor = pInst->GetControlForeground();
        else
        {
            vcl::Font aFont;
            if ( pInst->IsControlFont() )
                aFont = pInst->GetControlFont();
            else
                aFont = pInst->GetFont();
            nColor = aFont.GetColor();
        }
    }

    return sal_Int32(nColor);
}

sal_Int32 SAL_CALL AccessibleBrowseBoxBase::getBackground(  )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Color nColor;
    vcl::Window* pInst = mpBrowseBox->GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlBackground() )
            nColor = pInst->GetControlBackground();
        else
            nColor = pInst->GetBackground().GetColor();
    }

    return sal_Int32(nColor);
}


// XInterface
IMPLEMENT_FORWARD_XINTERFACE2( BrowseBoxAccessibleElement, AccessibleBrowseBoxBase, BrowseBoxAccessibleElement_Base )

// XTypeProvider
IMPLEMENT_FORWARD_XTYPEPROVIDER2( BrowseBoxAccessibleElement, AccessibleBrowseBoxBase, BrowseBoxAccessibleElement_Base )

// css::accessibility::XAccessible

Reference< css::accessibility::XAccessibleContext > SAL_CALL BrowseBoxAccessibleElement::getAccessibleContext()
{
    osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return this;
}


BrowseBoxAccessibleElement::BrowseBoxAccessibleElement( const css::uno::Reference< css::accessibility::XAccessible >& rxParent, ::vcl::IAccessibleTableProvider& rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow, ::vcl::AccessibleBrowseBoxObjType  eObjType )
    :AccessibleBrowseBoxBase( rxParent, rBrowseBox, _xFocusWindow, eObjType )
{
}


BrowseBoxAccessibleElement::BrowseBoxAccessibleElement( const css::uno::Reference< css::accessibility::XAccessible >& rxParent, ::vcl::IAccessibleTableProvider& rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow, ::vcl::AccessibleBrowseBoxObjType  eObjType,
        const OUString& rName, const OUString& rDescription )
    :AccessibleBrowseBoxBase( rxParent, rBrowseBox, _xFocusWindow, eObjType, rName, rDescription )
{
}


BrowseBoxAccessibleElement::~BrowseBoxAccessibleElement( )
{
}


} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
