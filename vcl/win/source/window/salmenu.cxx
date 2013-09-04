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


#include <svsys.h>

#include <vcl/menu.hxx>
#include <vcl/sysdata.hxx>

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salframe.h>
#include <win/salmenu.h>

#include <impbmp.hxx>
#include <salgdi.hxx>

// uncomment the following line to have ownerdrawn menus, ie, with bitmaps
// however, this is incompatible with OLE inplace editing
// so it is not activated by default
//#define OWNERDRAW

static DWORD myerr=0;

// =======================================================================

sal_Bool SalData::IsKnownMenuHandle( HMENU hMenu )
{
    if( mhMenuSet.find( hMenu ) == mhMenuSet.end() )
        return FALSE;
    else
        return TRUE;
}

// =======================================================================

// WinSalInst factory methods

SalMenu* WinSalInstance::CreateMenu( sal_Bool bMenuBar, Menu* )
{
    WinSalMenu *pSalMenu = new WinSalMenu();

    pSalMenu->mbMenuBar = bMenuBar;
    pSalMenu->mhWnd     = NULL;
    if( bMenuBar )
        pSalMenu->mhMenu = ::CreateMenu();
    else
        pSalMenu->mhMenu = ::CreatePopupMenu();

    if( pSalMenu->mhMenu )
        GetSalData()->mhMenuSet.insert( pSalMenu->mhMenu );

    return pSalMenu;
}

void WinSalInstance::DestroyMenu( SalMenu* pSalMenu )
{
    delete pSalMenu;
}


SalMenuItem* WinSalInstance::CreateMenuItem( const SalItemParams* pItemData )
{
    if( !pItemData )
        return NULL;

    WinSalMenuItem *pSalMenuItem = new WinSalMenuItem();
    memset( &pSalMenuItem->mInfo, 0, sizeof( MENUITEMINFOW ) );
    pSalMenuItem->mInfo.cbSize = sizeof( MENUITEMINFOW );

    if( pItemData->eType == MENUITEM_SEPARATOR )
    {
        // separator
        pSalMenuItem->mInfo.fMask = MIIM_TYPE;
        pSalMenuItem->mInfo.fType = MFT_SEPARATOR;
    }
    else
    {
        // item
        pSalMenuItem->mText   = pItemData->aText;
        pSalMenuItem->mpMenu  = pItemData->pMenu;
        pSalMenuItem->maBitmap= !!pItemData->aImage ? pItemData->aImage.GetBitmapEx().GetBitmap() : Bitmap();
        pSalMenuItem->mnId    = pItemData->nId;

        // 'translate' mnemonics
        pSalMenuItem->mText = pSalMenuItem->mText.replaceAll( "~", "&" );

        pSalMenuItem->mInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_DATA;
        pSalMenuItem->mInfo.fType = MFT_STRING;
#ifdef OWNERDRAW
        if( pItemData->pMenu && !pItemData->pMenu->IsMenuBar() )
            pSalMenuItem->mInfo.fType |= MFT_OWNERDRAW;
        pSalMenuItem->mInfo.fState = MFS_ENABLED;
#endif
        pSalMenuItem->mInfo.dwTypeData = (LPWSTR) pSalMenuItem->mText.getStr();
        pSalMenuItem->mInfo.cch = pSalMenuItem->mText.getLength();

        pSalMenuItem->mInfo.wID = pItemData->nId;
        pSalMenuItem->mInfo.dwItemData = (ULONG_PTR) pSalMenuItem; // user data
    }

    return pSalMenuItem;
}

void WinSalInstance::DestroyMenuItem( SalMenuItem* pSalMenuItem )
{
    delete pSalMenuItem;
}


// =======================================================================

static void ImplDrawMenuBar( SalMenu *pMenu )
{
    if( pMenu->VisibleMenuBar() )
    {
        // redrawing the menubar all the time actually seems to be unnecessary (it just flickers)
        /*
        WinSalMenu *pMenuBar = ImplFindMenuBar( pMenu );
        if( pMenuBar && pMenuBar->mhWnd )
            ::DrawMenuBar( pMenuBar->mhWnd );
            */
    }
}

// =======================================================================


/*
 * WinSalMenu
 */

WinSalMenu::WinSalMenu()
{
    mhMenu       = NULL;
    mbMenuBar    = FALSE;
    mhWnd        = NULL;
    mpParentMenu = NULL;
}

WinSalMenu::~WinSalMenu()
{
    // only required if not associated to a window...
    GetSalData()->mhMenuSet.erase( mhMenu );
    ::DestroyMenu( mhMenu );
}

sal_Bool WinSalMenu::VisibleMenuBar()
{
    // The Win32 implementation never shows a native
    // menubar. Thus, native menus are only visible
    // when the menu is merged with an OLE container.
    // The reason are missing tooltips, ownerdraw
    // issues and accessibility which are better supported
    // by VCL menus.
    // Nevertheless, the native menus are always created
    // and the application will properly react to all native
    // menu messages.

    return FALSE;
}

void WinSalMenu::SetFrame( const SalFrame *pFrame )
{
    if( pFrame )
        mhWnd = static_cast<const WinSalFrame*>(pFrame)->mhWnd;
    else
        mhWnd = NULL;
}

void WinSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    if( pSalMenuItem )
    {
    WinSalMenuItem* pWItem = static_cast<WinSalMenuItem*>(pSalMenuItem);
        if( nPos == MENU_APPEND )
        {
            nPos = ::GetMenuItemCount( mhMenu );
            if( nPos == static_cast<unsigned>( -1 ) )
                return;
        }

        if(!::InsertMenuItemW( mhMenu, nPos, TRUE, &pWItem->mInfo ))
            myerr = GetLastError();
        else
        {
            pWItem->mpSalMenu = this;
            ImplDrawMenuBar( this );
        }
    }
}

void WinSalMenu::RemoveItem( unsigned nPos )
{
    int num = ::GetMenuItemCount( mhMenu );
    if( num != -1 && nPos < (unsigned)num )
    {
        WinSalMenuItem *pSalMenuItem = NULL;

        MENUITEMINFOW mi;
        memset( &mi, 0, sizeof(mi) );
        mi.cbSize = sizeof( mi );
        mi.fMask = MIIM_DATA;
        if( !GetMenuItemInfoW( mhMenu, nPos, TRUE, &mi) )
            myerr = GetLastError();
        else
            pSalMenuItem = (WinSalMenuItem *) mi.dwItemData;

        if( !::RemoveMenu( mhMenu, nPos, MF_BYPOSITION ) )
            myerr = GetLastError();
        else
        {
            if( pSalMenuItem )
                pSalMenuItem->mpSalMenu = NULL;
            ImplDrawMenuBar( this );
        }
    }
}

void ImplRemoveItemById( WinSalMenu *pSalMenu, unsigned nItemId )
{
    if( !pSalMenu )
        return;

    WinSalMenuItem *pSalMenuItem = NULL;

    MENUITEMINFOW mi;
    memset( &mi, 0, sizeof(mi) );
    mi.cbSize = sizeof( mi );
    mi.fMask = MIIM_DATA;
    if( !GetMenuItemInfoW( pSalMenu->mhMenu, nItemId, FALSE, &mi) )
        myerr = GetLastError();
    else
        pSalMenuItem = (WinSalMenuItem *) mi.dwItemData;

    if( !::RemoveMenu( pSalMenu->mhMenu, nItemId, MF_BYCOMMAND ) )
        myerr = GetLastError();
    else
    {
        if( pSalMenuItem )
            pSalMenuItem->mpSalMenu = NULL;
        ImplDrawMenuBar( pSalMenu );
    }
}

void WinSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    if( pSalMenuItem )
    {
        WinSalMenuItem* pWMenuItem = static_cast<WinSalMenuItem*>(pSalMenuItem);
        WinSalMenu* pWSubMenu = static_cast<WinSalMenu*>(pSubMenu);
        if( pWMenuItem->mInfo.hSubMenu )
        {
            GetSalData()->mhMenuSet.erase( pWMenuItem->mInfo.hSubMenu );
            ::DestroyMenu( pWMenuItem->mInfo.hSubMenu );
        }

        pWMenuItem->mInfo.fMask |= MIIM_SUBMENU;
        if( !pSubMenu )
            pWMenuItem->mInfo.hSubMenu = NULL;
        else
        {
            pWMenuItem->mInfo.hSubMenu = pWSubMenu->mhMenu;
            pWSubMenu->mpParentMenu = this;
        }

        if(!::SetMenuItemInfoW( mhMenu, nPos, TRUE, &pWMenuItem->mInfo ) )
            myerr = GetLastError();
        else
            ImplDrawMenuBar( this );
    }
}

void WinSalMenu::CheckItem( unsigned nPos, sal_Bool bCheck )
{
    if( static_cast<unsigned>( -1 ) != ::CheckMenuItem( mhMenu, nPos, MF_BYPOSITION|(bCheck ? MF_CHECKED : MF_UNCHECKED) ) )
        ImplDrawMenuBar( this );
}

void WinSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
    if( -1 != ::EnableMenuItem( mhMenu, nPos, MF_BYPOSITION|(bEnable ? MF_ENABLED : (MF_DISABLED|MF_GRAYED) ) ) )
        ImplDrawMenuBar( this );
}

void WinSalMenu::SetItemImage( unsigned /*nPos*/, SalMenuItem* pSalMenuItem, const Image& rImage )
{
    if( pSalMenuItem )
    {
    WinSalMenuItem* pWItem = static_cast<WinSalMenuItem*>(pSalMenuItem);
        if( !!rImage )
            pWItem->maBitmap = rImage.GetBitmapEx().GetBitmap();
        else
            pWItem->maBitmap = Bitmap();
    }
}

void WinSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText )
{
    if( pSalMenuItem )
    {
    WinSalMenuItem* pWItem = static_cast<WinSalMenuItem*>(pSalMenuItem);
        pWItem->mText = rText;
        // 'translate' mnemonics
        pWItem->mText = pWItem->mText.replaceAll( "~", "&" );
        pWItem->mInfo.fMask = MIIM_TYPE | MIIM_DATA;
        pWItem->mInfo.fType = MFT_STRING;
#ifdef OWNERDRAW
        if( pWItem->mpMenu && !((Menu*) pWItem->mpMenu)->IsMenuBar() )
            pWItem->mInfo.fType |= MFT_OWNERDRAW;
#endif

        // combine text and accelerator text
        OUString aStr( pWItem->mText );
        if( pWItem->mAccelText.getLength() )
        {
            aStr += "\t" + pWItem->mAccelText;
        }
        pWItem->mInfo.dwTypeData = (LPWSTR) aStr.getStr();
        pWItem->mInfo.cch = aStr.getLength();

        if(!::SetMenuItemInfoW( mhMenu, nPos, TRUE, &pWItem->mInfo ))
            myerr = GetLastError();
        else
            ImplDrawMenuBar( this );
    }
}

void WinSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode&, const OUString& rKeyName )
{
    if( pSalMenuItem )
    {
    WinSalMenuItem* pWItem = static_cast<WinSalMenuItem*>(pSalMenuItem);
        pWItem->mAccelText = rKeyName;
        pWItem->mInfo.fMask = MIIM_TYPE | MIIM_DATA;
        pWItem->mInfo.fType = MFT_STRING;
#ifdef OWNERDRAW
        if( pWItem->mpMenu && !((Menu*)pWItem->mpMenu)->IsMenuBar() )
            pWItem->mInfo.fType |= MFT_OWNERDRAW;
#endif
        // combine text and accelerator text
        OUString aStr( pWItem->mText );
        if( pWItem->mAccelText.getLength() )
        {
            aStr += "\t" + pWItem->mAccelText;
        }
        pWItem->mInfo.dwTypeData = (LPWSTR) aStr.getStr();
        pWItem->mInfo.cch = aStr.getLength();

        if(!::SetMenuItemInfoW( mhMenu, nPos, TRUE, &pWItem->mInfo ))
            myerr = GetLastError();
        else
            ImplDrawMenuBar( this );
    }
}

void WinSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
    if( pData )
        pData->hMenu = mhMenu;
}

// =======================================================================

/*
 * SalMenuItem
 */


WinSalMenuItem::WinSalMenuItem()
{
    memset( &mInfo, 0, sizeof( MENUITEMINFOW ) );
    mpMenu = NULL;
    mnId  = 0xFFFF;
    mpSalMenu = NULL;
}

WinSalMenuItem::~WinSalMenuItem()
{
    if( mpSalMenu )
        ImplRemoveItemById( mpSalMenu, mnId );
}

// -------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
