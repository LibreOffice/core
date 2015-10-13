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
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/ui/ItemStyle.hpp>

using namespace com::sun::star::ui;

using rtl::OUString;
using com::sun::star::uno::RuntimeException;

namespace framework
{

namespace
{
static sal_uInt16 impl_convertItemBitsToItemStyle( sal_Int16 nItemBits )
{
    sal_uInt16 nStyle( 0 );

    if ( ( nItemBits & SIB_RIGHT ) == SIB_RIGHT )
        nStyle |= ItemStyle::ALIGN_RIGHT;
    else if ( ( nItemBits & SIB_LEFT ) == SIB_LEFT )
        nStyle |= ItemStyle::ALIGN_LEFT;
    else
        nStyle |= ItemStyle::ALIGN_CENTER;

    if ( ( nItemBits & SIB_FLAT ) == SIB_FLAT )
        nStyle |= ItemStyle::DRAW_FLAT;
    else if ( ( nItemBits & SIB_OUT ) == SIB_OUT )
        nStyle |= ItemStyle::DRAW_OUT3D;
    else
        nStyle |= ItemStyle::DRAW_IN3D;

    if ( ( nItemBits & SIB_AUTOSIZE ) == SIB_AUTOSIZE )
        nStyle |= ItemStyle::AUTO_SIZE;

    if ( ( nItemBits & SIB_USERDRAW ) == SIB_USERDRAW )
        nStyle |= ItemStyle::OWNER_DRAW;

    return nStyle;
}
}

StatusbarItem::StatusbarItem(
    StatusBar              *pStatusBar,
    AddonStatusbarItemData *pItemData,
    sal_uInt16              nId,
    const rtl::OUString&    aCommand )
    : StatusbarItem_Base( m_aMutex )
    , m_pStatusBar( pStatusBar )
    , m_pItemData( pItemData )
    , m_nId( nId )
    , m_nStyle( 0 )
    , m_aCommand( aCommand )
{
    if ( m_pStatusBar )
        m_nStyle = impl_convertItemBitsToItemStyle(
            m_pStatusBar->GetItemBits( m_nId ) );
}

StatusbarItem::~StatusbarItem()
{
}

void SAL_CALL StatusbarItem::disposing()
{
    osl::MutexGuard aGuard( m_aMutex );
    m_pItemData = 0;
    m_pStatusBar = 0;
}

OUString SAL_CALL StatusbarItem::getCommand()
throw (RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_aCommand;
}

::sal_uInt16 SAL_CALL StatusbarItem::getItemId()
throw (RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_nId;
}

::sal_uInt32 SAL_CALL StatusbarItem::getWidth()
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetItemWidth( m_nId );

    return ::sal_uInt32(0);
}

::sal_uInt16 SAL_CALL StatusbarItem::getStyle()
throw (RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_nStyle;
}

::sal_Int32 SAL_CALL StatusbarItem::getOffset()
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetItemOffset( m_nId );

    return ::sal_Int32(0);
}

css::awt::Rectangle SAL_CALL StatusbarItem::getItemRect()
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    css::awt::Rectangle aAWTRect;
    if ( m_pStatusBar )
    {
        Rectangle aRect = m_pStatusBar->GetItemRect( m_nId );
        return css::awt::Rectangle( aRect.Left(),
                                                 aRect.Top(),
                                                 aRect.GetWidth(),
                                                 aRect.GetHeight() );
    }

    return aAWTRect;
}

OUString SAL_CALL StatusbarItem::getText()
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetItemText( m_nId );

    return OUString();
}

void SAL_CALL StatusbarItem::setText( const OUString& rText )
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        m_pStatusBar->SetItemText( m_nId, rText );;
}

OUString SAL_CALL StatusbarItem::getHelpText()
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetHelpText( m_nId );

    return OUString();
}

void SAL_CALL StatusbarItem::setHelpText( const OUString& rHelpText )
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        m_pStatusBar->SetHelpText( m_nId, rHelpText );;
}

OUString SAL_CALL StatusbarItem::getQuickHelpText()
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetHelpText( m_nId );

    return OUString();
}

void SAL_CALL StatusbarItem::setQuickHelpText( const OUString& rQuickHelpText )
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        m_pStatusBar->SetQuickHelpText( m_nId, rQuickHelpText );
}

OUString SAL_CALL StatusbarItem::getAccessibleName()
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->GetAccessibleName( m_nId );

    return OUString();
}

void SAL_CALL StatusbarItem::setAccessibleName( const OUString& rAccessibleName )
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        m_pStatusBar->SetAccessibleName( m_nId, rAccessibleName );
}

sal_Bool SAL_CALL StatusbarItem::getVisible()
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
        return m_pStatusBar->IsItemVisible( m_nId );

    return sal_False;
}

void SAL_CALL StatusbarItem::setVisible( sal_Bool bVisible )
throw (RuntimeException, std::exception)
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
throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( m_pStatusBar )
    {
        m_pStatusBar->RedrawItem( m_nId );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
