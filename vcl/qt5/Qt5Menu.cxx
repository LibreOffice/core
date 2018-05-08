/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Qt5Frame.hxx"
#include "Qt5Menu.hxx"

Qt5Menu::Qt5Menu( bool bMenuBar ) :
    mpVCLMenu( nullptr ),
    mpParentSalMenu( nullptr ),
    mpFrame( nullptr ),
    mbMenuBar( bMenuBar )
{
}

Qt5Menu::~Qt5Menu()
{
}

bool Qt5Menu::VisibleMenuBar()
{
    return false;
}

void Qt5Menu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
}

void Qt5Menu::RemoveItem( unsigned nPos )
{
}

void Qt5Menu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
}

void Qt5Menu::SetFrame( const SalFrame* pFrame )
{

    mpFrame = const_cast<Qt5Frame*>( static_cast<const Qt5Frame*>( pFrame ) );
}

void Qt5Menu::ShowItem( unsigned nPos, bool bCheck )
{
}

void Qt5Menu::CheckItem( unsigned nPos, bool bCheck )
{
}

void Qt5Menu::EnableItem( unsigned nPos, bool bEnable )
{
}

void Qt5Menu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText )
{
}

void Qt5Menu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage)
{
}

void Qt5Menu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& rKeyName )
{
}

void Qt5Menu::GetSystemMenuData( SystemMenuData* pData )
{
}


Qt5MenuItem::Qt5MenuItem( const SalItemParams* pItemData ) :
    mnId( pItemData->nId ),
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( nullptr ),
    mpSubMenu( nullptr )
{
}

Qt5MenuItem::~Qt5MenuItem()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
