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

#include "accessibility/extended/AccessibleBrowseBoxBase.hxx"
#include <svtools/accessibletableprovider.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblerelationsethelper.hxx>

// ============================================================================

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;
using namespace ::svt;


// ============================================================================

namespace accessibility {

using namespace com::sun::star::accessibility::AccessibleStateType;
// ============================================================================

// Ctor/Dtor/disposing --------------------------------------------------------

AccessibleBrowseBoxBase::AccessibleBrowseBoxBase(
        const Reference< XAccessible >& rxParent,
        IAccessibleTableProvider&                      rBrowseBox,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
        AccessibleBrowseBoxObjType      eObjType ) :
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
        const Reference< XAccessible >& rxParent,
        IAccessibleTableProvider&                      rBrowseBox,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
        AccessibleBrowseBoxObjType      eObjType,
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
    ::osl::MutexGuard aGuard( getOslMutex() );
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

    mxParent = NULL;
    mpBrowseBox = NULL;
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL AccessibleBrowseBoxBase::getAccessibleParent()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return mxParent;
}

sal_Int32 SAL_CALL AccessibleBrowseBoxBase::getAccessibleIndexInParent()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    // -1 for child not found/no parent (according to specification)
    sal_Int32 nRet = -1;

    Reference< uno::XInterface > xMeMyselfAndI( static_cast< XAccessibleContext* >( this ), uno::UNO_QUERY );

    //  iterate over parent's children and search for this object
    if( mxParent.is() )
    {
        Reference< XAccessibleContext >
            xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            Reference< uno::XInterface > xChild;

            sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int32 nChild = 0; nChild < nChildCount; ++nChild )
            {
                xChild = xChild.query( xParentContext->getAccessibleChild( nChild ) );

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
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return maDescription;
}

OUString SAL_CALL AccessibleBrowseBoxBase::getAccessibleName()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return maName;
}

Reference< XAccessibleRelationSet > SAL_CALL
AccessibleBrowseBoxBase::getAccessibleRelationSet()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    // BrowseBox does not have relations.
       return new utl::AccessibleRelationSetHelper;
}

Reference< XAccessibleStateSet > SAL_CALL
AccessibleBrowseBoxBase::getAccessibleStateSet()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    // don't check whether alive -> StateSet may contain DEFUNC
    return implCreateStateSetHelper();
}

lang::Locale SAL_CALL AccessibleBrowseBoxBase::getLocale()
    throw ( IllegalAccessibleComponentStateException, uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    if( mxParent.is() )
    {
        Reference< XAccessibleContext >
            xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }
    throw IllegalAccessibleComponentStateException();
}

// XAccessibleComponent -------------------------------------------------------

sal_Bool SAL_CALL AccessibleBrowseBoxBase::containsPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException )
{
    return Rectangle( Point(), getBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

awt::Rectangle SAL_CALL AccessibleBrowseBoxBase::getBounds()
    throw ( uno::RuntimeException )
{
    return AWTRectangle( getBoundingBox() );
}

awt::Point SAL_CALL AccessibleBrowseBoxBase::getLocation()
    throw ( uno::RuntimeException )
{
    return AWTPoint( getBoundingBox().TopLeft() );
}

awt::Point SAL_CALL AccessibleBrowseBoxBase::getLocationOnScreen()
    throw ( uno::RuntimeException )
{
    return AWTPoint( getBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL AccessibleBrowseBoxBase::getSize()
    throw ( uno::RuntimeException )
{
    return AWTSize( getBoundingBox().GetSize() );
}

sal_Bool SAL_CALL AccessibleBrowseBoxBase::isShowing()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return implIsShowing();
}

sal_Bool SAL_CALL AccessibleBrowseBoxBase::isVisible()
    throw ( uno::RuntimeException )
{
    Reference< XAccessibleStateSet > xStateSet = getAccessibleStateSet();
    return xStateSet.is() ?
        xStateSet->contains( AccessibleStateType::VISIBLE ) : sal_False;
}

sal_Bool SAL_CALL AccessibleBrowseBoxBase::isFocusTraversable()
    throw ( uno::RuntimeException )
{
    Reference< XAccessibleStateSet > xStateSet = getAccessibleStateSet();
    return xStateSet.is() ?
        xStateSet->contains( AccessibleStateType::FOCUSABLE ) : sal_False;
}

void SAL_CALL AccessibleBrowseBoxBase::focusGained( const ::com::sun::star::awt::FocusEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aFocused;
    com::sun::star::uno::Any aEmpty;
    aFocused <<= FOCUSED;

    commitEvent(AccessibleEventId::STATE_CHANGED,aFocused,aEmpty);
}
// -----------------------------------------------------------------------------

void SAL_CALL AccessibleBrowseBoxBase::focusLost( const ::com::sun::star::awt::FocusEvent& ) throw (::com::sun::star::uno::RuntimeException)
{
    com::sun::star::uno::Any aFocused;
    com::sun::star::uno::Any aEmpty;
    aFocused <<= FOCUSED;

    commitEvent(AccessibleEventId::STATE_CHANGED,aEmpty,aFocused);
}
// XAccessibleEventBroadcaster ------------------------------------------------

void SAL_CALL AccessibleBrowseBoxBase::addEventListener(
        const Reference< XAccessibleEventListener>& _rxListener )
    throw ( uno::RuntimeException )
{
    if ( _rxListener.is() )
    {
        ::osl::MutexGuard aGuard( getOslMutex() );
        if ( !getClientId( ) )
            setClientId( AccessibleEventNotifier::registerClient( ) );

        AccessibleEventNotifier::addEventListener( getClientId( ), _rxListener );
    }
}

void SAL_CALL AccessibleBrowseBoxBase::removeEventListener(
        const Reference< XAccessibleEventListener>& _rxListener )
    throw ( uno::RuntimeException )
{
    if( _rxListener.is() && getClientId( ) )
    {
        ::osl::MutexGuard aGuard( getOslMutex() );
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

// XTypeProvider --------------------------------------------------------------

namespace
{
    class theAccessibleBrowseBoxBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theAccessibleBrowseBoxBaseImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL AccessibleBrowseBoxBase::getImplementationId()
    throw ( uno::RuntimeException )
{
    return theAccessibleBrowseBoxBaseImplementationId::get().getSeq();
}

// XServiceInfo ---------------------------------------------------------------

sal_Bool SAL_CALL AccessibleBrowseBoxBase::supportsService(
        const OUString& rServiceName )
    throw ( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL AccessibleBrowseBoxBase::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    const OUString aServiceName( "com.sun.star.accessibility.AccessibleContext" );
    return Sequence< OUString >( &aServiceName, 1 );
}

// other public methods -------------------------------------------------------

void AccessibleBrowseBoxBase::setAccessibleName( const OUString& rName )
{
    ::osl::ClearableMutexGuard aGuard( getOslMutex() );
    Any aOld;
    aOld <<= maName;
    maName = rName;

    aGuard.clear();

    commitEvent(
        AccessibleEventId::NAME_CHANGED,
        uno::makeAny( maName ),
        aOld );
}

void AccessibleBrowseBoxBase::setAccessibleDescription( const OUString& rDescription )
{
    ::osl::ClearableMutexGuard aGuard( getOslMutex() );
    Any aOld;
    aOld <<= maDescription;
    maDescription = rDescription;

    aGuard.clear();

    commitEvent(
        AccessibleEventId::DESCRIPTION_CHANGED,
        uno::makeAny( maDescription ),
        aOld );
}

// internal virtual methods ---------------------------------------------------

sal_Bool AccessibleBrowseBoxBase::implIsShowing()
{
    sal_Bool bShowing = sal_False;
    if( mxParent.is() )
    {
        Reference< XAccessibleComponent >
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

// internal helper methods ----------------------------------------------------

sal_Bool AccessibleBrowseBoxBase::isAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose && mpBrowseBox;
}

void AccessibleBrowseBoxBase::ensureIsAlive() const
    throw ( lang::DisposedException )
{
    if( !isAlive() )
        throw lang::DisposedException();
}

Rectangle AccessibleBrowseBoxBase::getBoundingBox()
    throw ( lang::DisposedException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    Rectangle aRect = implGetBoundingBox();
    if ( 0 == aRect.Left() && 0 == aRect.Top() && 0 == aRect.Right() && 0 == aRect.Bottom() )
    {
        SAL_WARN( "accessibility", "rectangle doesn't exist" );
    }
    return aRect;
}

Rectangle AccessibleBrowseBoxBase::getBoundingBoxOnScreen()
    throw ( lang::DisposedException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    Rectangle aRect = implGetBoundingBoxOnScreen();
    if ( 0 == aRect.Left() && 0 == aRect.Top() && 0 == aRect.Right() && 0 == aRect.Bottom() )
    {
        SAL_WARN( "accessibility", "rectangle doesn't exist" );
    }
    return aRect;
}

void AccessibleBrowseBoxBase::commitEvent(
        sal_Int16 _nEventId, const Any& _rNewValue, const Any& _rOldValue )
{
    ::osl::ClearableMutexGuard aGuard( getOslMutex() );
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
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    sal_Int16 nRole = AccessibleRole::UNKNOWN;
    switch ( meObjType )
    {
        case BBTYPE_ROWHEADERCELL:
            nRole = AccessibleRole::ROW_HEADER;
            break;
        case BBTYPE_COLUMNHEADERCELL:
            nRole = AccessibleRole::COLUMN_HEADER;
            break;
        case BBTYPE_COLUMNHEADERBAR:
        case BBTYPE_ROWHEADERBAR:
        case BBTYPE_TABLE:
            nRole = AccessibleRole::TABLE;
            break;
        case BBTYPE_TABLECELL:
            nRole = AccessibleRole::TABLE_CELL;
            break;
        case BBTYPE_BROWSEBOX:
            nRole = AccessibleRole::PANEL;
            break;
        case BBTYPE_CHECKBOXCELL:
            nRole = AccessibleRole::CHECK_BOX;
            break;
    }
    return nRole;
}
// -----------------------------------------------------------------------------
Any SAL_CALL AccessibleBrowseBoxBase::getAccessibleKeyBinding()
        throw ( uno::RuntimeException )
{
    return Any();
}
// -----------------------------------------------------------------------------
Reference<XAccessible > SAL_CALL AccessibleBrowseBoxBase::getAccessibleAtPoint( const ::com::sun::star::awt::Point& )
        throw ( uno::RuntimeException )
{
    return NULL;
}
// -----------------------------------------------------------------------------
void SAL_CALL AccessibleBrowseBoxBase::disposing( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xFocusWindow = NULL;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL AccessibleBrowseBoxBase::getForeground(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    sal_Int32 nColor = 0;
    Window* pInst = mpBrowseBox->GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlForeground() )
            nColor = pInst->GetControlForeground().GetColor();
        else
        {
            Font aFont;
            if ( pInst->IsControlFont() )
                aFont = pInst->GetControlFont();
            else
                aFont = pInst->GetFont();
            nColor = aFont.GetColor().GetColor();
        }
    }

    return nColor;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL AccessibleBrowseBoxBase::getBackground(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    sal_Int32 nColor = 0;
    Window* pInst = mpBrowseBox->GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlBackground() )
            nColor = pInst->GetControlBackground().GetColor();
        else
            nColor = pInst->GetBackground().GetColor().GetColor();
    }

    return nColor;
}

// ============================================================================
// XInterface -----------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( BrowseBoxAccessibleElement, AccessibleBrowseBoxBase, BrowseBoxAccessibleElement_Base )

// XTypeProvider --------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2( BrowseBoxAccessibleElement, AccessibleBrowseBoxBase, BrowseBoxAccessibleElement_Base )

// XAccessible ----------------------------------------------------------------

Reference< XAccessibleContext > SAL_CALL BrowseBoxAccessibleElement::getAccessibleContext() throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return this;
}

// ----------------------------------------------------------------------------
BrowseBoxAccessibleElement::BrowseBoxAccessibleElement( const Reference< XAccessible >& rxParent, IAccessibleTableProvider& rBrowseBox,
        const Reference< awt::XWindow >& _xFocusWindow, AccessibleBrowseBoxObjType  eObjType )
    :AccessibleBrowseBoxBase( rxParent, rBrowseBox, _xFocusWindow, eObjType )
{
}

// ----------------------------------------------------------------------------
BrowseBoxAccessibleElement::BrowseBoxAccessibleElement( const Reference< XAccessible >& rxParent, IAccessibleTableProvider& rBrowseBox,
        const Reference< awt::XWindow >& _xFocusWindow, AccessibleBrowseBoxObjType  eObjType,
        const OUString& rName, const OUString& rDescription )
    :AccessibleBrowseBoxBase( rxParent, rBrowseBox, _xFocusWindow, eObjType, rName, rDescription )
{
}

// ----------------------------------------------------------------------------
BrowseBoxAccessibleElement::~BrowseBoxAccessibleElement( )
{
}

// ============================================================================

} // namespace accessibility

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
