/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "accessibility/extended/AccessibleGridControlBase.hxx"
#include <svtools/accessibletable.hxx>
#include <comphelper/servicehelper.hxx>
//
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
using namespace ::svt::table;


// ============================================================================

namespace accessibility {

using namespace com::sun::star::accessibility::AccessibleStateType;
// ============================================================================

AccessibleGridControlBase::AccessibleGridControlBase(
        const Reference< XAccessible >& rxParent,
        svt::table::IAccessibleTable& rTable,
        AccessibleTableControlObjType      eObjType ) :
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
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}

void SAL_CALL AccessibleGridControlBase::disposing()
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    m_xParent = NULL;
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL AccessibleGridControlBase::getAccessibleParent()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return m_xParent;
}

sal_Int32 SAL_CALL AccessibleGridControlBase::getAccessibleIndexInParent()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    // -1 for child not found/no parent (according to specification)
    sal_Int32 nRet = -1;

    Reference< uno::XInterface > xMeMyselfAndI( static_cast< XAccessibleContext* >( this ), uno::UNO_QUERY );

    //  iterate over parent's children and search for this object
    if( m_xParent.is() )
    {
        Reference< XAccessibleContext >
            xParentContext( m_xParent->getAccessibleContext() );
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

OUString SAL_CALL AccessibleGridControlBase::getAccessibleDescription()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return m_aDescription;
}

OUString SAL_CALL AccessibleGridControlBase::getAccessibleName()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return m_aName;
}

Reference< XAccessibleRelationSet > SAL_CALL
AccessibleGridControlBase::getAccessibleRelationSet()
    throw ( uno::RuntimeException )
{
   ensureIsAlive();
   // GridControl does not have relations.
   return new utl::AccessibleRelationSetHelper;
}

Reference< XAccessibleStateSet > SAL_CALL
AccessibleGridControlBase::getAccessibleStateSet()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    // don't check whether alive -> StateSet may contain DEFUNC
    return implCreateStateSetHelper();
}

lang::Locale SAL_CALL AccessibleGridControlBase::getLocale()
    throw ( IllegalAccessibleComponentStateException, uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    if( m_xParent.is() )
    {
        Reference< XAccessibleContext >
            xParentContext( m_xParent->getAccessibleContext() );
        if( xParentContext.is() )
        return xParentContext->getLocale();
    }
    throw IllegalAccessibleComponentStateException();
}

// XAccessibleComponent -------------------------------------------------------

sal_Bool SAL_CALL AccessibleGridControlBase::containsPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException )
{
   return Rectangle( Point(), getBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

awt::Rectangle SAL_CALL AccessibleGridControlBase::getBounds()
    throw ( uno::RuntimeException )
{
   return AWTRectangle( getBoundingBox() );
}

awt::Point SAL_CALL AccessibleGridControlBase::getLocation()
    throw ( uno::RuntimeException )
{
    return AWTPoint( getBoundingBox().TopLeft() );
}

awt::Point SAL_CALL AccessibleGridControlBase::getLocationOnScreen()
    throw ( uno::RuntimeException )
{
    return AWTPoint( getBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL AccessibleGridControlBase::getSize()
    throw ( uno::RuntimeException )
{
    return AWTSize( getBoundingBox().GetSize() );
}

sal_Bool SAL_CALL AccessibleGridControlBase::isShowing()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return implIsShowing();
}

sal_Bool SAL_CALL AccessibleGridControlBase::isVisible()
    throw ( uno::RuntimeException )
{
    Reference< XAccessibleStateSet > xStateSet = getAccessibleStateSet();
    return xStateSet.is() ?
        xStateSet->contains( AccessibleStateType::VISIBLE ) : sal_False;
}

sal_Bool SAL_CALL AccessibleGridControlBase::isFocusTraversable()
    throw ( uno::RuntimeException )
{
    Reference< XAccessibleStateSet > xStateSet = getAccessibleStateSet();
    return xStateSet.is() ?
        xStateSet->contains( AccessibleStateType::FOCUSABLE ) : sal_False;
}
// XAccessibleEventBroadcaster ------------------------------------------------

void SAL_CALL AccessibleGridControlBase::addEventListener(
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

void SAL_CALL AccessibleGridControlBase::removeEventListener(
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
    class theAccessibleGridControlBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theAccessibleGridControlBaseImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL AccessibleGridControlBase::getImplementationId()
    throw ( uno::RuntimeException )
{
    return theAccessibleGridControlBaseImplementationId::get().getSeq();
}

// XServiceInfo ---------------------------------------------------------------

sal_Bool SAL_CALL AccessibleGridControlBase::supportsService(
        const OUString& rServiceName )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );

    Sequence< OUString > aSupportedServices( getSupportedServiceNames() );
    const OUString* pArrBegin = aSupportedServices.getConstArray();
    const OUString* pArrEnd = pArrBegin + aSupportedServices.getLength();
    const OUString* pString = pArrBegin;

    for( ; ( pString != pArrEnd ) && ( rServiceName != *pString ); ++pString )
        ;
    return pString != pArrEnd;
}

Sequence< OUString > SAL_CALL AccessibleGridControlBase::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    const OUString aServiceName( "com.sun.star.accessibility.AccessibleContext" );
    return Sequence< OUString >( &aServiceName, 1 );
}
// internal virtual methods ---------------------------------------------------

sal_Bool AccessibleGridControlBase::implIsShowing()
{
    sal_Bool bShowing = sal_False;
    if( m_xParent.is() )
    {
        Reference< XAccessibleComponent >
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

// internal helper methods ----------------------------------------------------

sal_Bool AccessibleGridControlBase::isAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose && &m_aTable;
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
    ::osl::MutexGuard aGuard( getOslMutex() );
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
    ::osl::MutexGuard aGuard( getOslMutex() );
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

sal_Int16 SAL_CALL AccessibleGridControlBase::getAccessibleRole()
    throw ( uno::RuntimeException )
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
// -----------------------------------------------------------------------------
Any SAL_CALL AccessibleGridControlBase::getAccessibleKeyBinding()
        throw ( uno::RuntimeException )
{
    return Any();
}
// -----------------------------------------------------------------------------
Reference<XAccessible > SAL_CALL AccessibleGridControlBase::getAccessibleAtPoint( const ::com::sun::star::awt::Point& )
        throw ( uno::RuntimeException )
{
    return NULL;
}
//// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL AccessibleGridControlBase::getForeground(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    sal_Int32 nColor = 0;
    Window* pInst = m_aTable.GetWindowInstance();
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
sal_Int32 SAL_CALL AccessibleGridControlBase::getBackground(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    sal_Int32 nColor = 0;
    Window* pInst = m_aTable.GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlBackground() )
            nColor = pInst->GetControlBackground().GetColor();
    else
            nColor = pInst->GetBackground().GetColor().GetColor();
    }
    return nColor;
}

//// ============================================================================
GridControlAccessibleElement::GridControlAccessibleElement( const Reference< XAccessible >& rxParent,
                        IAccessibleTable& rTable,
                        AccessibleTableControlObjType  eObjType )
    :AccessibleGridControlBase( rxParent, rTable, eObjType )
{
}

// XInterface -----------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( GridControlAccessibleElement, AccessibleGridControlBase, GridControlAccessibleElement_Base)

// XTypeProvider --------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2( GridControlAccessibleElement, AccessibleGridControlBase, GridControlAccessibleElement_Base )

// XAccessible ----------------------------------------------------------------

Reference< XAccessibleContext > SAL_CALL GridControlAccessibleElement::getAccessibleContext() throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return this;
}
// ----------------------------------------------------------------------------
GridControlAccessibleElement::~GridControlAccessibleElement( )
{
}

// ============================================================================

} // namespace accessibility

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
