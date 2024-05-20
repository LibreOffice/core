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
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <cppuhelper/supportsservice.hxx>
#include "accpage.hxx"

#include <strings.hrc>
#include <pagefrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using uno::Sequence;

constexpr OUStringLiteral sImplementationName = u"com.sun.star.comp.Writer.SwAccessiblePageView";

bool SwAccessiblePage::IsSelected()
{
    return GetMap()->IsPageSelected( static_cast < const SwPageFrame * >( GetFrame() ) );
}

void SwAccessiblePage::GetStates( sal_Int64& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // FOCUSABLE
    rStateSet |= AccessibleStateType::FOCUSABLE;

    // FOCUSED
    if( IsSelected() )
    {
        OSL_ENSURE( m_bIsSelected, "bSelected out of sync" );
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );

        vcl::Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
            rStateSet |= AccessibleStateType::FOCUSED;
    }
}

void SwAccessiblePage::InvalidateCursorPos_()
{
    bool bNewSelected = IsSelected();
    bool bOldSelected;

    {
        std::scoped_lock aGuard( m_Mutex );
        bOldSelected = m_bIsSelected;
        m_bIsSelected = bNewSelected;
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

void SwAccessiblePage::InvalidateFocus_()
{
    vcl::Window *pWin = GetWindow();
    if( !pWin )
        return;

    bool bSelected;

    {
        std::scoped_lock aGuard( m_Mutex );
        bSelected = m_bIsSelected;
    }
    OSL_ENSURE( bSelected, "focus object should be selected" );

    FireStateChangedEvent( AccessibleStateType::FOCUSED,
                           pWin->HasFocus() && bSelected );
}

SwAccessiblePage::SwAccessiblePage(std::shared_ptr<SwAccessibleMap> const& pInitMap,
                                    const SwFrame* pFrame )
    : SwAccessibleContext( pInitMap, AccessibleRole::PANEL, pFrame )
    , m_bIsSelected( false )
{
    assert(pFrame != nullptr);
    assert(pInitMap != nullptr);
    assert(pFrame->IsPageFrame());

    OUString sPage = OUString::number(
        static_cast<const SwPageFrame*>( GetFrame() )->GetPhyPageNum() );
    SetName( GetResource( STR_ACCESS_PAGE_NAME, &sPage ) );
}

SwAccessiblePage::~SwAccessiblePage()
{
}

bool SwAccessiblePage::HasCursor()
{
    std::scoped_lock aGuard( m_Mutex );
    return m_bIsSelected;
}

OUString SwAccessiblePage::getImplementationName( )
{
    return sImplementationName;
}

sal_Bool SwAccessiblePage::supportsService( const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SwAccessiblePage::getSupportedServiceNames( )
{
    return { u"com.sun.star.text.AccessiblePageView"_ustr, sAccessibleServiceName };
}

Sequence< sal_Int8 > SAL_CALL SwAccessiblePage::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SwAccessiblePage::getAccessibleDescription( )
{
    ThrowIfDisposed();

    OUString sArg( GetFormattedPageNumber() );
    return GetResource( STR_ACCESS_PAGE_DESC, &sArg );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
