/*************************************************************************
 *
 *  $RCSfile: salmenu.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-20 13:03:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALMENU_CXX

#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALMENU_H
#include <salmenu.h>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_IMPBMP_HXX
#include <impbmp.hxx>
#endif

// uncomment the following line to have ownerdrawn menues, ie, with bitmaps
// however, this is incompatible with OLE inplace editing
// so it is not activated by default
//#define OWNERDRAW

static DWORD myerr=0;

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
        pSalMenuItem->maBitmap= !!pItemData->aImage ? pItemData->aImage.GetBitmap() : Bitmap();
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

static SalMenu* ImplFindMenuBar( SalMenu *pMenu )
{
    WinSalMenu *pMenuBar = static_cast<WinSalMenu*>(pMenu);
    while( pMenuBar->mpParentMenu )
        pMenuBar = pMenuBar->mpParentMenu;
    if( pMenuBar->mbMenuBar )
        return pMenuBar;
    else
        return NULL;
}

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
            ::DestroyMenu( pWMenuItem->mInfo.hSubMenu );

        pWMenuItem->mInfo.fMask |= MIIM_SUBMENU;
        if( !pSubMenu )
            pWMenuItem->mInfo.hSubMenu = NULL;
        else
        {
            pWMenuItem->mInfo.hSubMenu = pWSubMenu->mhMenu;
            pWSubMenu->mpParentMenu = this;
        }

        int num = ::GetMenuItemCount( mhMenu );
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

void WinSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
    if( pSalMenuItem )
    {
    WinSalMenuItem* pWItem = static_cast<WinSalMenuItem*>(pSalMenuItem);
        if( !!rImage )
            pWItem->maBitmap = rImage.GetBitmap();
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

void WinSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
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

