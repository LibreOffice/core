/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salmenu.cxx,v $
 * $Revision: 1.14 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/svwin.h>
#include <wincomp.hxx>
#include <saldata.hxx>
#include <salinst.h>
#include <vcl/salgdi.hxx>
#include <salmenu.h>
#include <vcl/menu.hxx>
#include <vcl/sysdata.hxx>
#include <salframe.h>
#include <vcl/impbmp.hxx>

// uncomment the following line to have ownerdrawn menues, ie, with bitmaps
// however, this is incompatible with OLE inplace editing
// so it is not activated by default
//#define OWNERDRAW

static DWORD myerr=0;

// =======================================================================

BOOL SalData::IsKnownMenuHandle( HMENU hMenu )
{
    if( mhMenuSet.find( hMenu ) == mhMenuSet.end() )
        return FALSE;
    else
        return TRUE;
}

// =======================================================================

// WinSalInst factory methods

SalMenu* WinSalInstance::CreateMenu( BOOL bMenuBar )
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
        pSalMenuItem->mText.SearchAndReplace( '~', '&' );

        pSalMenuItem->mInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_DATA;
        pSalMenuItem->mInfo.fType = MFT_STRING;
#ifdef OWNERDRAW
        if( pItemData->pMenu && !pItemData->pMenu->IsMenuBar() )
            pSalMenuItem->mInfo.fType |= MFT_OWNERDRAW;
        pSalMenuItem->mInfo.fState = MFS_ENABLED;
#endif
        pSalMenuItem->mInfo.dwTypeData = (LPWSTR) pSalMenuItem->mText.GetBuffer();
        pSalMenuItem->mInfo.cch = pSalMenuItem->mText.Len();

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

BOOL WinSalMenu::VisibleMenuBar()
{
    // The Win32 implementation never shows a native
    // menubar. Thus, native menues are only visible
    // when the menu is merged with an OLE container.
    // The reason are missing tooltips, ownerdraw
    // issues and accessibility which are better supported
    // by VCL menues.
    // Nevertheless, the native menues are always created
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
            if( nPos == -1 )
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

void WinSalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
    if( -1 != ::CheckMenuItem( mhMenu, nPos, MF_BYPOSITION|(bCheck ? MF_CHECKED : MF_UNCHECKED) ) )
        ImplDrawMenuBar( this );
}

void WinSalMenu::EnableItem( unsigned nPos, BOOL bEnable )
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

void WinSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
    if( pSalMenuItem )
    {
    WinSalMenuItem* pWItem = static_cast<WinSalMenuItem*>(pSalMenuItem);
        pWItem->mText = rText;
        // 'translate' mnemonics
        pWItem->mText.SearchAndReplace( '~', '&' );
        pWItem->mInfo.fMask = MIIM_TYPE | MIIM_DATA;
        pWItem->mInfo.fType = MFT_STRING;
#ifdef OWNERDRAW
        if( pWItem->mpMenu && !((Menu*) pWItem->mpMenu)->IsMenuBar() )
            pWItem->mInfo.fType |= MFT_OWNERDRAW;
#endif

        // combine text and accelerator text
        XubString aStr( pWItem->mText );
        if( pWItem->mAccelText.Len() )
        {
            aStr.AppendAscii("\t");
            aStr.Append( pWItem->mAccelText );
        }
        pWItem->mInfo.dwTypeData = (LPWSTR) aStr.GetBuffer();
        pWItem->mInfo.cch = aStr.Len();

        if(!::SetMenuItemInfoW( mhMenu, nPos, TRUE, &pWItem->mInfo ))
            myerr = GetLastError();
        else
            ImplDrawMenuBar( this );
    }
}

void WinSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode&, const XubString& rKeyName )
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
        XubString aStr( pWItem->mText );
        if( pWItem->mAccelText.Len() )
        {
            aStr.AppendAscii("\t");
            aStr.Append( pWItem->mAccelText );
        }
        pWItem->mInfo.dwTypeData = (LPWSTR) aStr.GetBuffer();
        pWItem->mInfo.cch = aStr.Len();

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

