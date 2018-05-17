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

#include <QtWidgets/QtWidgets>

#include <vcl/svapp.hxx>

Qt5Menu::Qt5Menu( bool bMenuBar ) :
    mpVCLMenu( nullptr ),
    mpParentSalMenu( nullptr ),
    mpFrame( nullptr ),
    mbMenuBar( bMenuBar )
{
}

Qt5Menu::~Qt5Menu()
{
    maItems.clear();
}

bool Qt5Menu::VisibleMenuBar()
{
    return false;
}

void Qt5Menu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    SolarMutexGuard aGuard;
    Qt5MenuItem *pItem = static_cast<Qt5MenuItem*>( pSalMenuItem );

    if ( nPos == MENU_APPEND )
        maItems.push_back( pItem );
    else
        maItems.insert( maItems.begin() + nPos, pItem );

    pItem->mpParentMenu = this;
}

void Qt5Menu::RemoveItem( unsigned nPos )
{
    SolarMutexGuard aGuard;
    maItems.erase( maItems.begin() + nPos );
}

void Qt5Menu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
}

void Qt5Menu::SetFrame( const SalFrame* pFrame )
{
    SolarMutexGuard aGuard;
    assert(mbMenuBar);
    mpFrame = const_cast<Qt5Frame*>( static_cast<const Qt5Frame*>( pFrame ) );

    mpFrame->SetMenu( this );

    QWidget* pWidget = mpFrame->GetQWidget();
    QMainWindow* pMainWindow = dynamic_cast<QMainWindow*>(pWidget);
    if( pMainWindow )
        mpQMenuBar = pMainWindow->menuBar();

    ActivateAllSubMenus( mpVCLMenu );

    Update();
}

void Qt5Menu::ActivateAllSubMenus( Menu* pMenuBar )
{
    for (Qt5MenuItem* pSalItem : maItems)
    {
        if ( pSalItem->mpSubMenu != nullptr )
        {
            pMenuBar->HandleMenuActivateEvent(pSalItem->mpSubMenu->GetMenu());
            pSalItem->mpSubMenu->ActivateAllSubMenus(pMenuBar);
            pSalItem->mpSubMenu->Update();
            pMenuBar->HandleMenuDeActivateEvent(pSalItem->mpSubMenu->GetMenu());
        }
    }
}

void Qt5Menu::Update()
{
    Menu* pVCLMenu = mpVCLMenu;

    for ( sal_Int32 nItem = 0; nItem < static_cast<sal_Int32>(GetItemCount()); nItem++ )
    {
        Qt5MenuItem *pSalMenuItem = GetItemAtPos( nItem );
        sal_uInt16 nId = pSalMenuItem->mnId;
        OUString aText = pVCLMenu->GetItemText( nId );

        if (mbMenuBar && mpQMenuBar)
        {
             NativeItemText( aText );
             mpQMenuBar->addMenu( toQString(aText) );
        }
    }
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

void Qt5Menu::NativeItemText( OUString& rItemText )
{
    rItemText = rItemText.replace( '~', '&' );
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
