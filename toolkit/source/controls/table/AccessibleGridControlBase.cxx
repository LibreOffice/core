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

#include <controls/table/AccessibleGridControlBase.hxx>

#include <utility>
#include <vcl/accessibletable.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/window.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/types.h>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <sal/log.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;
using namespace ::vcl;
using namespace ::vcl::table;


namespace accessibility {

using namespace com::sun::star::accessibility::AccessibleStateType;


AccessibleGridControlBase::AccessibleGridControlBase(
        css::uno::Reference< css::accessibility::XAccessible > xParent,
        svt::table::TableControl& rTable,
        ::vcl::table::AccessibleTableControlObjType      eObjType ) :
    AccessibleGridControlImplHelper( m_aMutex ),
    m_xParent(std::move( xParent )),
    m_aTable( rTable),
    m_eObjType( eObjType ),
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
{
    SolarMutexGuard g;

    ensureIsAlive();
    return m_xParent;
}

sal_Int64 SAL_CALL AccessibleGridControlBase::getAccessibleIndexInParent()
{
    SolarMutexGuard g;

    ensureIsAlive();

    // -1 for child not found/no parent (according to specification)
    sal_Int64 nRet = -1;

    css::uno::Reference< uno::XInterface > xMeMyselfAndI( static_cast< css::accessibility::XAccessibleContext* >( this ), uno::UNO_QUERY );

    //  iterate over parent's children and search for this object
    if( m_xParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( m_xParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            css::uno::Reference< uno::XInterface > xChild;

            sal_Int64 nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int64 nChild = 0; nChild < nChildCount; ++nChild )
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
{
    SolarMutexGuard g;

    ensureIsAlive();
    return m_aTable.GetAccessibleObjectDescription(m_eObjType);
}

OUString SAL_CALL AccessibleGridControlBase::getAccessibleName()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return m_aTable.GetAccessibleObjectName(m_eObjType, 0, 0);
}

css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
AccessibleGridControlBase::getAccessibleRelationSet()
{
   SolarMutexGuard g;

   ensureIsAlive();
   // GridControl does not have relations.
   return new utl::AccessibleRelationSetHelper;
}

sal_Int64 SAL_CALL
AccessibleGridControlBase::getAccessibleStateSet()
{
    SolarMutexGuard aSolarGuard;

    // don't check whether alive -> StateSet may contain DEFUNC
    return implCreateStateSet();
}

lang::Locale SAL_CALL AccessibleGridControlBase::getLocale()
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
{
    return tools::Rectangle(Point(), getBoundingBox().GetSize())
        .Contains(vcl::unohelper::ConvertToVCLPoint(rPoint));
}

awt::Rectangle SAL_CALL AccessibleGridControlBase::getBounds()
{
    return vcl::unohelper::ConvertToAWTRect(getBoundingBox());
}

awt::Point SAL_CALL AccessibleGridControlBase::getLocation()
{
    return vcl::unohelper::ConvertToAWTPoint(getBoundingBox().TopLeft());
}

awt::Point SAL_CALL AccessibleGridControlBase::getLocationOnScreen()
{
    return vcl::unohelper::ConvertToAWTPoint(getBoundingBoxOnScreen().TopLeft());
}

awt::Size SAL_CALL AccessibleGridControlBase::getSize()
{
    return vcl::unohelper::ConvertToAWTSize(getBoundingBox().GetSize());
}

// css::accessibility::XAccessibleEventBroadcaster

void SAL_CALL AccessibleGridControlBase::addAccessibleEventListener(
        const css::uno::Reference< css::accessibility::XAccessibleEventListener>& _rxListener )
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
{
    if( !(_rxListener.is() && getClientId( )) )
        return;

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

// XTypeProvider

Sequence< sal_Int8 > SAL_CALL AccessibleGridControlBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo

sal_Bool SAL_CALL AccessibleGridControlBase::supportsService(
        const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL AccessibleGridControlBase::getSupportedServiceNames()
{
    return { u"com.sun.star.accessibility.AccessibleContext"_ustr };
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
            bShowing = implGetBoundingBox().Overlaps(
                vcl::unohelper::ConvertToVCLRect(xParentComp->getBounds()));
    }
    return bShowing;
}

sal_Int64 AccessibleGridControlBase::implCreateStateSet()
{
    sal_Int64 nStateSet = 0;

    if( isAlive() )
    {
        // SHOWING done with m_xParent
        if( implIsShowing() )
            nStateSet |= AccessibleStateType::SHOWING;
        // GridControl fills StateSet with states depending on object type
        m_aTable.FillAccessibleStateSet( nStateSet, getType() );
    }
    else
        nStateSet |= AccessibleStateType::DEFUNC;
    return nStateSet;
}

// internal helper methods

bool AccessibleGridControlBase::isAlive() const
{
    ::osl::MutexGuard g(m_aMutex); // guards rBHelper members
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

void AccessibleGridControlBase::ensureIsAlive() const
{
    if( !isAlive() )
        throw lang::DisposedException();
}

tools::Rectangle AccessibleGridControlBase::getBoundingBox()
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
    tools::Rectangle aRect = implGetBoundingBox();
    if ( aRect.Left() == 0 && aRect.Top() == 0 && aRect.Right() == 0 && aRect.Bottom() == 0 )
    {
        SAL_WARN( "accessibility", "rectangle doesn't exist" );
    }
    return aRect;
}

AbsoluteScreenPixelRectangle AccessibleGridControlBase::getBoundingBoxOnScreen()
{
    SolarMutexGuard aSolarGuard;
    ensureIsAlive();
    AbsoluteScreenPixelRectangle aRect = implGetBoundingBoxOnScreen();
    if ( aRect.Left() == 0 && aRect.Top() == 0 && aRect.Right() == 0 && aRect.Bottom() == 0 )
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
    AccessibleEventObject aEvent(*this, _nEventId, _rNewValue, _rOldValue, -1);

    // let the notifier handle this event

    AccessibleEventNotifier::addEvent( getClientId( ), aEvent );
}

sal_Int16 SAL_CALL AccessibleGridControlBase::getAccessibleRole()
{
    ensureIsAlive();
    sal_Int16 nRole = AccessibleRole::UNKNOWN;
    switch ( m_eObjType )
    {
        case AccessibleTableControlObjType::ROWHEADERCELL:
        nRole = AccessibleRole::ROW_HEADER;
        break;
    case AccessibleTableControlObjType::COLUMNHEADERCELL:
        nRole = AccessibleRole::COLUMN_HEADER;
        break;
    case AccessibleTableControlObjType::COLUMNHEADERBAR:
    case AccessibleTableControlObjType::ROWHEADERBAR:
    case AccessibleTableControlObjType::TABLE:
        nRole = AccessibleRole::TABLE;
        break;
    case AccessibleTableControlObjType::TABLECELL:
        nRole = AccessibleRole::TABLE_CELL;
        break;
    case AccessibleTableControlObjType::GRIDCONTROL:
        nRole = AccessibleRole::PANEL;
        break;
    }
    return nRole;
}

css::uno::Reference<css::accessibility::XAccessible > SAL_CALL AccessibleGridControlBase::getAccessibleAtPoint( const css::awt::Point& )
{
    return nullptr;
}

sal_Int32 SAL_CALL AccessibleGridControlBase::getForeground(  )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();

    Color nColor;
    if (m_aTable.IsControlForeground())
        nColor = m_aTable.GetControlForeground();
    else
    {
        vcl::Font aFont;
        if (m_aTable.IsControlFont() )
            aFont = m_aTable.GetControlFont();
        else
            aFont = m_aTable.GetFont();
        nColor = aFont.GetColor();
    }
    return sal_Int32(nColor);
}

sal_Int32 SAL_CALL AccessibleGridControlBase::getBackground(  )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    Color nColor;
    if (m_aTable.IsControlBackground())
        nColor = m_aTable.GetControlBackground();
    else
        nColor = m_aTable.GetBackground().GetColor();
    return sal_Int32(nColor);
}


GridControlAccessibleElement::GridControlAccessibleElement( const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
                        svt::table::TableControl& rTable,
                        ::vcl::table::AccessibleTableControlObjType  eObjType )
    : GridControlAccessibleElement_Base(rxParent, rTable, eObjType)
{
}

// css::accessibility::XAccessible

css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL GridControlAccessibleElement::getAccessibleContext()
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
