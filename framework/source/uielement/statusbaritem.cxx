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

#include <uielement/statusbaritem.hxx>
#include <utility>
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/ui/ItemStyle.hpp>

using namespace com::sun::star::ui;

namespace framework
{

namespace
{
sal_uInt16 impl_convertItemBitsToItemStyle( StatusBarItemBits nItemBits )
{
    sal_uInt16 nStyle( 0 );

    if ( nItemBits & StatusBarItemBits::Right )
        nStyle |= ItemStyle::ALIGN_RIGHT;
    else if ( nItemBits & StatusBarItemBits::Left )
        nStyle |= ItemStyle::ALIGN_LEFT;
    else
        nStyle |= ItemStyle::ALIGN_CENTER;

    if ( nItemBits & StatusBarItemBits::Flat )
        nStyle |= ItemStyle::DRAW_FLAT;
    else if ( nItemBits & StatusBarItemBits::Out )
        nStyle |= ItemStyle::DRAW_OUT3D;
    else
        nStyle |= ItemStyle::DRAW_IN3D;

    if ( nItemBits & StatusBarItemBits::AutoSize )
        nStyle |= ItemStyle::AUTO_SIZE;

    if ( nItemBits & StatusBarItemBits::UserDraw )
        nStyle |= ItemStyle::OWNER_DRAW;

    return nStyle;
}
}

StatusbarItem::StatusbarItem(
    StatusBar              *pStatusBar,
    sal_uInt16              nId,
    OUString                aCommand )
    : m_pStatusBar( pStatusBar )
    , m_nId( nId )
    , m_nStyle( 0 )
    , m_aCommand(std::move( aCommand ))
{
    if ( m_pStatusBar )
        m_nStyle = impl_convertItemBitsToItemStyle(
            m_pStatusBar->GetItemBits( m_nId ) );
}

StatusbarItem::~StatusbarItem()
{
}

void StatusbarItem::disposing(std::unique_lock<std::mutex>&)
{
    m_pStatusBar = nullptr;
}

OUString SAL_CALL StatusbarItem::getCommand()
{
    std::unique_lock aGuard( m_aMutex );
    return m_aCommand;
}

::sal_uInt16 SAL_CALL StatusbarItem::getItemId()
{
    std::unique_lock aGuard( m_aMutex );
    return m_nId;
}

::sal_uInt32 SAL_CALL StatusbarItem::getWidth()
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetItemWidth( m_nId );

    return ::sal_uInt32(0);
}

::sal_uInt16 SAL_CALL StatusbarItem::getStyle()
{
    std::unique_lock aGuard( m_aMutex );
    return m_nStyle;
}

::sal_Int32 SAL_CALL StatusbarItem::getOffset()
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetItemOffset( m_nId );

    return 0;
}

css::awt::Rectangle SAL_CALL StatusbarItem::getItemRect()
{
    SolarMutexGuard aGuard;
    css::awt::Rectangle aAWTRect;
    if ( m_pStatusBar )
    {
        tools::Rectangle aRect = m_pStatusBar->GetItemRect( m_nId );
        return css::awt::Rectangle( aRect.Left(),
                                                 aRect.Top(),
                                                 aRect.GetWidth(),
                                                 aRect.GetHeight() );
    }

    return aAWTRect;
}

OUString SAL_CALL StatusbarItem::getText()
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetItemText( m_nId );

    return OUString();
}

void SAL_CALL StatusbarItem::setText( const OUString& rText )
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        m_pStatusBar->SetItemText( m_nId, rText );
}

OUString SAL_CALL StatusbarItem::getHelpText()
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetHelpText( m_nId );

    return OUString();
}

void SAL_CALL StatusbarItem::setHelpText( const OUString& rHelpText )
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        m_pStatusBar->SetHelpText( m_nId, rHelpText );
}

OUString SAL_CALL StatusbarItem::getQuickHelpText()
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetHelpText( m_nId );

    return OUString();
}

void SAL_CALL StatusbarItem::setQuickHelpText( const OUString& rQuickHelpText )
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        m_pStatusBar->SetQuickHelpText( m_nId, rQuickHelpText );
}

OUString SAL_CALL StatusbarItem::getAccessibleName()
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetAccessibleName( m_nId );

    return OUString();
}

void SAL_CALL StatusbarItem::setAccessibleName( const OUString& rAccessibleName )
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        m_pStatusBar->SetAccessibleName( m_nId, rAccessibleName );
}

sal_Bool SAL_CALL StatusbarItem::getVisible()
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->IsItemVisible( m_nId );

    return false;
}

void SAL_CALL StatusbarItem::setVisible( sal_Bool bVisible )
{
    SolarMutexGuard aGuard;
    if ( !m_pStatusBar )
        return;

    if ( bool(bVisible) != m_pStatusBar->IsItemVisible( m_nId ) )
    {
        if ( bVisible )
            m_pStatusBar->ShowItem( m_nId );
        else
            m_pStatusBar->HideItem( m_nId );
    }
}

void SAL_CALL StatusbarItem::repaint(  )
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
    {
        m_pStatusBar->RedrawItem( m_nId );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
