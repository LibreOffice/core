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

#include <accessibility/standard/vclxaccessiblescrollbar.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/scrbar.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// VCLXAccessibleScrollBar


VCLXAccessibleScrollBar::VCLXAccessibleScrollBar( VCLXWindow* pVCLWindow )
    :VCLXAccessibleComponent( pVCLWindow )
{
}



VCLXAccessibleScrollBar::~VCLXAccessibleScrollBar()
{
}



void VCLXAccessibleScrollBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_SCROLLBAR_SCROLL:
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


OUString VCLXAccessibleScrollBar::getImplementationName() throw (RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleScrollBar" );
}



Sequence< OUString > VCLXAccessibleScrollBar::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    Sequence< OUString > aNames { "com.sun.star.awt.AccessibleScrollBar" };
    return aNames;
}


// XAccessibleAction


sal_Int32 VCLXAccessibleScrollBar::getAccessibleActionCount( ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return 4;
}



sal_Bool VCLXAccessibleScrollBar::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    bool bReturn = false;
    VclPtr< ScrollBar > pScrollBar = GetAs< ScrollBar >();
    if ( pScrollBar )
    {
        ScrollType eScrollType;
        switch ( nIndex )
        {
            case 0:     eScrollType = SCROLL_LINEUP;    break;
            case 1:     eScrollType = SCROLL_LINEDOWN;  break;
            case 2:     eScrollType = SCROLL_PAGEUP;    break;
            case 3:     eScrollType = SCROLL_PAGEDOWN;  break;
            default:    eScrollType = SCROLL_DONTKNOW;  break;
        }
        if ( pScrollBar->DoScrollAction( eScrollType ) )
            bReturn = true;
    }

    return bReturn;
}



OUString VCLXAccessibleScrollBar::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    OUString sDescription;

    switch ( nIndex )
    {
        case 0:     sDescription = OUString( TK_RES_STRING( RID_STR_ACC_ACTION_DECLINE ) );      break;
        case 1:     sDescription = OUString( TK_RES_STRING( RID_STR_ACC_ACTION_INCLINE ) );      break;
        case 2:     sDescription = OUString( TK_RES_STRING( RID_STR_ACC_ACTION_DECBLOCK ) );     break;
        case 3:     sDescription = OUString( TK_RES_STRING( RID_STR_ACC_ACTION_INCBLOCK ) );     break;
        default:                                                                                        break;
    }

    return sDescription;
}



Reference< XAccessibleKeyBinding > VCLXAccessibleScrollBar::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessibleKeyBinding >();
}


// XAccessibleValue


Any VCLXAccessibleScrollBar::getCurrentValue(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
        aValue <<= (sal_Int32) pVCLXScrollBar->getValue();

    return aValue;
}



sal_Bool VCLXAccessibleScrollBar::setCurrentValue( const Any& aNumber ) throw (RuntimeException, std::exception)
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



Any VCLXAccessibleScrollBar::getMaximumValue(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
        aValue <<= (sal_Int32) pVCLXScrollBar->getMaximum();

    return aValue;
}



Any VCLXAccessibleScrollBar::getMinimumValue(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 0;

    return aValue;
}



OUString VCLXAccessibleScrollBar::getAccessibleName(  ) throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    OUString aName;
    VCLXScrollBar* pVCLXScrollBar = static_cast< VCLXScrollBar* >( GetVCLXWindow() );
    if ( pVCLXScrollBar )
    {
        if ( pVCLXScrollBar->getOrientation() == ScrollBarOrientation::HORIZONTAL )
            aName = TK_RES_STRING( RID_STR_ACC_SCROLLBAR_NAME_HORIZONTAL );
        else if ( pVCLXScrollBar->getOrientation() == ScrollBarOrientation::VERTICAL )
            aName = TK_RES_STRING( RID_STR_ACC_SCROLLBAR_NAME_VERTICAL );
    }
    return aName;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
