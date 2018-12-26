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

#include <standard/vclxaccessiblescrollbar.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <helper/accresmgr.hxx>
#include <strings.hrc>

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclevent.hxx>
#include <strings.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// VCLXAccessibleScrollBar


void VCLXAccessibleScrollBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ScrollbarScroll:
        {
            NotifyAccessibleEvent( AccessibleEventId::VALUE_CHANGED, Any(), Any() );
        }
        break;
        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}


void VCLXAccessibleScrollBar::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
    {
        // IA2 CWS: scroll bar should not have FOCUSABLE state.
        // rStateSet.AddState( AccessibleStateType::FOCUSABLE );
        if ( pVCLXScrollBar->getOrientation() == ScrollBarOrientation::HORIZONTAL )
            rStateSet.AddState( AccessibleStateType::HORIZONTAL );
        else if ( pVCLXScrollBar->getOrientation() == ScrollBarOrientation::VERTICAL )
            rStateSet.AddState( AccessibleStateType::VERTICAL );
    }
}


// XInterface


IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleScrollBar, VCLXAccessibleComponent, VCLXAccessibleScrollBar_BASE )


// XTypeProvider


IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleScrollBar, VCLXAccessibleComponent, VCLXAccessibleScrollBar_BASE )


// XServiceInfo


OUString VCLXAccessibleScrollBar::getImplementationName()
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleScrollBar" );
}


Sequence< OUString > VCLXAccessibleScrollBar::getSupportedServiceNames()
{
    return { "com.sun.star.awt.AccessibleScrollBar" };
}


// XAccessibleAction

static constexpr sal_Int32 ACCESSIBLE_ACTION_COUNT=4;

sal_Int32 VCLXAccessibleScrollBar::getAccessibleActionCount( )
{
    OExternalLockGuard aGuard( this );

    return ACCESSIBLE_ACTION_COUNT;
}


sal_Bool VCLXAccessibleScrollBar::doAccessibleAction ( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= ACCESSIBLE_ACTION_COUNT )
        throw IndexOutOfBoundsException();

    bool bReturn = false;
    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
    {
        ScrollType eScrollType;
        switch ( nIndex )
        {
            case 0:     eScrollType = ScrollType::LineUp;    break;
            case 1:     eScrollType = ScrollType::LineDown;  break;
            case 2:     eScrollType = ScrollType::PageUp;    break;
            case 3:     eScrollType = ScrollType::PageDown;  break;
            default:    eScrollType = ScrollType::DontKnow;  break;
        }
        if ( pScrollBar->DoScrollAction( eScrollType ) )
            bReturn = true;
    }

    return bReturn;
}


OUString VCLXAccessibleScrollBar::getAccessibleActionDescription ( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= ACCESSIBLE_ACTION_COUNT )
        throw IndexOutOfBoundsException();

    OUString sDescription;

    switch ( nIndex )
    {
        case 0:     sDescription = OUString(RID_STR_ACC_ACTION_DECLINE);      break;
        case 1:     sDescription = OUString(RID_STR_ACC_ACTION_INCLINE);      break;
        case 2:     sDescription = OUString(RID_STR_ACC_ACTION_DECBLOCK);     break;
        case 3:     sDescription = OUString(RID_STR_ACC_ACTION_INCBLOCK);     break;
        default:                                                              break;
    }

    return sDescription;
}


Reference< XAccessibleKeyBinding > VCLXAccessibleScrollBar::getAccessibleActionKeyBinding( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= ACCESSIBLE_ACTION_COUNT )
        throw IndexOutOfBoundsException();

    return Reference< XAccessibleKeyBinding >();
}


// XAccessibleValue


Any VCLXAccessibleScrollBar::getCurrentValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
        aValue <<=  pVCLXScrollBar->getValue();

    return aValue;
}


sal_Bool VCLXAccessibleScrollBar::setCurrentValue( const Any& aNumber )
{
    OExternalLockGuard aGuard( this );

    bool bReturn = false;

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
    {
        sal_Int32 nValue = 0, nValueMin = 0, nValueMax = 0;
        OSL_VERIFY( aNumber >>= nValue );
        OSL_VERIFY( getMinimumValue() >>= nValueMin );
        OSL_VERIFY( getMaximumValue() >>= nValueMax );

        if ( nValue < nValueMin )
            nValue = nValueMin;
        else if ( nValue > nValueMax )
            nValue = nValueMax;

        pVCLXScrollBar->setValue( nValue );
        bReturn = true;
    }

    return bReturn;
}


Any VCLXAccessibleScrollBar::getMaximumValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
        aValue <<= pVCLXScrollBar->getMaximum();

    return aValue;
}


Any VCLXAccessibleScrollBar::getMinimumValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= sal_Int32(0);

    return aValue;
}


OUString VCLXAccessibleScrollBar::getAccessibleName(  )
{
    OExternalLockGuard aGuard( this );

    OUString aName;
    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
    {
        if ( pVCLXScrollBar->getOrientation() == ScrollBarOrientation::HORIZONTAL )
            aName = AccResId( RID_STR_ACC_SCROLLBAR_NAME_HORIZONTAL );
        else if ( pVCLXScrollBar->getOrientation() == ScrollBarOrientation::VERTICAL )
            aName = AccResId( RID_STR_ACC_SCROLLBAR_NAME_VERTICAL );
    }
    return aName;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
