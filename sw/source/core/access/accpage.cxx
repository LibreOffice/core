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

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "accpage.hxx"

#include "access.hrc"
#include <pagefrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using uno::Reference;
using uno::RuntimeException;
using uno::Sequence;

const sal_Char sServiceName[] = "com.sun.star.text.AccessiblePageView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessiblePageView";

bool SwAccessiblePage::IsSelected()
{
    return GetMap()->IsPageSelected( static_cast < const SwPageFrame * >( GetFrame() ) );
}

void SwAccessiblePage::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // FOCUSABLE
    rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    // FOCUSED
    if( IsSelected() )
    {
        OSL_ENSURE( bIsSelected, "bSelected out of sync" );
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );

        vcl::Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );
    }
}

void SwAccessiblePage::_InvalidateCursorPos()
{
    bool bNewSelected = IsSelected();
    bool bOldSelected;

    {
        osl::MutexGuard aGuard( m_Mutex );
        bOldSelected = bIsSelected;
        bIsSelected = bNewSelected;
    }

    if( bNewSelected )
    {
        // remember that object as the one that has the caret. This is
        // necessary to notify that object if the cursor leaves it.
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    if( bOldSelected != bNewSelected )
    {
        vcl::Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            FireStateChangedEvent( AccessibleStateType::FOCUSED, bNewSelected );
    }
}

void SwAccessiblePage::_InvalidateFocus()
{
    vcl::Window *pWin = GetWindow();
    if( pWin )
    {
        bool bSelected;

        {
            osl::MutexGuard aGuard( m_Mutex );
            bSelected = bIsSelected;
        }
        OSL_ENSURE( bSelected, "focus object should be selected" );

        FireStateChangedEvent( AccessibleStateType::FOCUSED,
                               pWin->HasFocus() && bSelected );
    }
}

SwAccessiblePage::SwAccessiblePage( SwAccessibleMap* pInitMap,
                                    const SwFrame* pFrame )
    : SwAccessibleContext( pInitMap, AccessibleRole::PANEL, pFrame )
    , bIsSelected( false )
{
    OSL_ENSURE( pFrame != nullptr, "need frame" );
    OSL_ENSURE( pInitMap != nullptr, "need map" );
    OSL_ENSURE( pFrame->IsPageFrame(), "need page frame" );

    SolarMutexGuard aGuard;

    OUString sPage = OUString::number(
        static_cast<const SwPageFrame*>( GetFrame() )->GetPhyPageNum() );
    SetName( GetResource( STR_ACCESS_PAGE_NAME, &sPage ) );
}

SwAccessiblePage::~SwAccessiblePage()
{
}

bool SwAccessiblePage::HasCursor()
{
    osl::MutexGuard aGuard( m_Mutex );
    return bIsSelected;
}

OUString SwAccessiblePage::getImplementationName( )
    throw( RuntimeException, std::exception )
{
    return OUString(sImplementationName);
}

sal_Bool SwAccessiblePage::supportsService( const OUString& rServiceName)
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SwAccessiblePage::getSupportedServiceNames( )
    throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = sServiceName;
    pArray[1] = sAccessibleServiceName;
    return aRet;
}

Sequence< sal_Int8 > SAL_CALL SwAccessiblePage::getImplementationId()
        throw(RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SwAccessiblePage::getAccessibleDescription( )
    throw( RuntimeException, std::exception )
{
    CHECK_FOR_DEFUNC( css::accessibility::XAccessibleContext );

    OUString sArg( GetFormattedPageNumber() );
    return GetResource( STR_ACCESS_PAGE_DESC, &sArg );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
