/*************************************************************************
 *
 *  $RCSfile: salmenu.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:04:04 $
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
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALMENU_HXX
#include <salmenu.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
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
    SalMenu *pSalMenu = new SalMenu();

    pSalMenu->maData.mbMenuBar = bMenuBar;
    pSalMenu->maData.mhWnd     = NULL;
    if( bMenuBar )
        pSalMenu->maData.mhMenu = ::CreateMenu();
    else
        pSalMenu->maData.mhMenu = ::CreatePopupMenu();

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

    SalMenuItem *pSalMenuItem = new SalMenuItem();
    memset( &pSalMenuItem->maData.mInfo, 0, sizeof( MENUITEMINFOW ) );
    pSalMenuItem->maData.mInfo.cbSize = sizeof( MENUITEMINFOW );

    if( pItemData->eType == MENUITEM_SEPARATOR )
    {
        // separator
        pSalMenuItem->maData.mInfo.fMask = MIIM_TYPE;
        pSalMenuItem->maData.mInfo.fType = MFT_SEPARATOR;
    }
    else
    {
        // item
        pSalMenuItem->maData.mText   = pItemData->aText;
        pSalMenuItem->maData.mpMenu  = pItemData->pMenu;
        pSalMenuItem->maData.maBitmap= !!pItemData->aImage ? pItemData->aImage.GetBitmap() : Bitmap();
        pSalMenuItem->maData.mnId    = pItemData->nId;

        // 'translate' mnemonics
        pSalMenuItem->maData.mText.SearchAndReplace( '~', '&' );

        pSalMenuItem->maData.mInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_DATA;
        pSalMenuItem->maData.mInfo.fType = MFT_STRING;
#ifdef OWNERDRAW
        if( pItemData->pMenu && !pItemData->pMenu->IsMenuBar() )
            pSalMenuItem->maData.mInfo.fType |= MFT_OWNERDRAW;
        pSalMenuItem->maData.mInfo.fState = MFS_ENABLED;
#endif
        pSalMenuItem->maData.mInfo.dwTypeData = (LPWSTR) pSalMenuItem->maData.mText.GetBuffer();
        pSalMenuItem->maData.mInfo.cch = pSalMenuItem->maData.mText.Len();

        pSalMenuItem->maData.mInfo.wID = pItemData->nId;
        pSalMenuItem->maData.mInfo.dwItemData = (ULONG_PTR) pSalMenuItem; // user data
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
    SalMenu *pMenuBar = pMenu;
    while( pMenuBar->maData.mpParentMenu )
        pMenuBar = pMenuBar->maData.mpParentMenu;
    if( pMenuBar->maData.mbMenuBar )
        return pMenuBar;
    else
        return NULL;
}

static void ImplDrawMenuBar( SalMenu *pMenu )
{
    if( SalMenu::VisibleMenuBar() )
    {
        // redrawing the menubar all the time actually seems to be unnecessary (it just flickers)
        /*
        SalMenu *pMenuBar = ImplFindMenuBar( pMenu );
        if( pMenuBar && pMenuBar->maData.mhWnd )
            ::DrawMenuBar( pMenuBar->maData.mhWnd );
            */
    }
}

// =======================================================================


/*
 * SalMenu
 */

SalMenu::SalMenu()
{
    memset( &maData, 0, sizeof(maData) );
}

SalMenu::~SalMenu()
{
    // only required if not associated to a window...
    ::DestroyMenu( maData.mhMenu );
}

BOOL SalMenu::VisibleMenuBar()
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

void SalMenu::SetFrame( const SalFrame *pFrame )
{
    if( pFrame )
        maData.mhWnd = pFrame->maFrameData.mhWnd;
    else
        maData.mhWnd = NULL;
}

void SalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    if( pSalMenuItem )
    {
        if( nPos == MENU_APPEND )
        {
            nPos = ::GetMenuItemCount( maData.mhMenu );
            if( nPos == -1 )
                return;
        }

        if(!::InsertMenuItemW( maData.mhMenu, nPos, TRUE, &pSalMenuItem->maData.mInfo ))
            myerr = GetLastError();
        else
        {
            pSalMenuItem->maData.mpSalMenu = this;
            ImplDrawMenuBar( this );
        }
    }
}

void SalMenu::RemoveItem( unsigned nPos )
{
    int num = ::GetMenuItemCount( maData.mhMenu );
    if( num != -1 && nPos < (unsigned)num )
    {
        SalMenuItem *pSalMenuItem = NULL;

        MENUITEMINFOW mi;
        memset( &mi, 0, sizeof(mi) );
        mi.cbSize = sizeof( mi );
        mi.fMask = MIIM_DATA;
        if( !GetMenuItemInfoW( maData.mhMenu, nPos, TRUE, &mi) )
            myerr = GetLastError();
        else
            pSalMenuItem = (SalMenuItem *) mi.dwItemData;

        if( !::RemoveMenu( maData.mhMenu, nPos, MF_BYPOSITION ) )
            myerr = GetLastError();
        else
        {
            if( pSalMenuItem )
                pSalMenuItem->maData.mpSalMenu = NULL;
            ImplDrawMenuBar( this );
        }
    }
}

void ImplRemoveItemById( SalMenu *pSalMenu, unsigned nItemId )
{
    if( !pSalMenu )
        return;

    SalMenuItem *pSalMenuItem = NULL;

    MENUITEMINFOW mi;
    memset( &mi, 0, sizeof(mi) );
    mi.cbSize = sizeof( mi );
    mi.fMask = MIIM_DATA;
    if( !GetMenuItemInfoW( pSalMenu->maData.mhMenu, nItemId, FALSE, &mi) )
        myerr = GetLastError();
    else
        pSalMenuItem = (SalMenuItem *) mi.dwItemData;

    if( !::RemoveMenu( pSalMenu->maData.mhMenu, nItemId, MF_BYCOMMAND ) )
        myerr = GetLastError();
    else
    {
        if( pSalMenuItem )
            pSalMenuItem->maData.mpSalMenu = NULL;
        ImplDrawMenuBar( pSalMenu );
    }
}

void SalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    if( pSalMenuItem )
    {
        if( pSalMenuItem->maData.mInfo.hSubMenu )
            ::DestroyMenu( pSalMenuItem->maData.mInfo.hSubMenu );

        pSalMenuItem->maData.mInfo.fMask |= MIIM_SUBMENU;
        if( !pSubMenu )
            pSalMenuItem->maData.mInfo.hSubMenu = NULL;
        else
        {
            pSalMenuItem->maData.mInfo.hSubMenu = pSubMenu->maData.mhMenu;
            pSubMenu->maData.mpParentMenu = this;
        }

        int num = ::GetMenuItemCount( maData.mhMenu );
        if(!::SetMenuItemInfoW( maData.mhMenu, nPos, TRUE, &pSalMenuItem->maData.mInfo ) )
            myerr = GetLastError();
        else
            ImplDrawMenuBar( this );
    }
}

void SalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
    if( -1 != ::CheckMenuItem( maData.mhMenu, nPos, MF_BYPOSITION|(bCheck ? MF_CHECKED : MF_UNCHECKED) ) )
        ImplDrawMenuBar( this );
}

void SalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
    if( -1 != ::EnableMenuItem( maData.mhMenu, nPos, MF_BYPOSITION|(bEnable ? MF_ENABLED : (MF_DISABLED|MF_GRAYED) ) ) )
        ImplDrawMenuBar( this );
}

void SalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
    if( pSalMenuItem )
    {
        if( !!rImage )
            pSalMenuItem->maData.maBitmap = rImage.GetBitmap();
        else
            pSalMenuItem->maData.maBitmap = Bitmap();
    }
}

void SalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
    if( pSalMenuItem )
    {
        pSalMenuItem->maData.mText = rText;
        // 'translate' mnemonics
        pSalMenuItem->maData.mText.SearchAndReplace( '~', '&' );
        pSalMenuItem->maData.mInfo.fMask = MIIM_TYPE | MIIM_DATA;
        pSalMenuItem->maData.mInfo.fType = MFT_STRING;
#ifdef OWNERDRAW
        if( pSalMenuItem->maData.mpMenu && !((Menu*) pSalMenuItem->maData.mpMenu)->IsMenuBar() )
            pSalMenuItem->maData.mInfo.fType |= MFT_OWNERDRAW;
#endif

        // combine text and accelerator text
        XubString aStr( pSalMenuItem->maData.mText );
        if( pSalMenuItem->maData.mAccelText.Len() )
        {
            aStr.AppendAscii("\t");
            aStr.Append( pSalMenuItem->maData.mAccelText );
        }
        pSalMenuItem->maData.mInfo.dwTypeData = (LPWSTR) aStr.GetBuffer();
        pSalMenuItem->maData.mInfo.cch = aStr.Len();

        if(!::SetMenuItemInfoW( maData.mhMenu, nPos, TRUE, &pSalMenuItem->maData.mInfo ))
            myerr = GetLastError();
        else
            ImplDrawMenuBar( this );
    }
}

void SalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
    if( pSalMenuItem )
    {
        pSalMenuItem->maData.mAccelText = rKeyName;
        pSalMenuItem->maData.mInfo.fMask = MIIM_TYPE | MIIM_DATA;
        pSalMenuItem->maData.mInfo.fType = MFT_STRING;
#ifdef OWNERDRAW
        if( pSalMenuItem->maData.mpMenu && !((Menu*)pSalMenuItem->maData.mpMenu)->IsMenuBar() )
            pSalMenuItem->maData.mInfo.fType |= MFT_OWNERDRAW;
#endif
        // combine text and accelerator text
        XubString aStr( pSalMenuItem->maData.mText );
        if( pSalMenuItem->maData.mAccelText.Len() )
        {
            aStr.AppendAscii("\t");
            aStr.Append( pSalMenuItem->maData.mAccelText );
        }
        pSalMenuItem->maData.mInfo.dwTypeData = (LPWSTR) aStr.GetBuffer();
        pSalMenuItem->maData.mInfo.cch = aStr.Len();

        if(!::SetMenuItemInfoW( maData.mhMenu, nPos, TRUE, &pSalMenuItem->maData.mInfo ))
            myerr = GetLastError();
        else
            ImplDrawMenuBar( this );
    }
}

void SalMenu::GetSystemMenuData( SystemMenuData* pData )
{
    if( pData )
        pData->hMenu = maData.mhMenu;
}

// =======================================================================

/*
 * SalMenuItem
 */


SalMenuItem::SalMenuItem()
{
    memset( &maData.mInfo, 0, sizeof( MENUITEMINFOW ) );
    maData.mpMenu = NULL;
    maData.mnId  = 0xFFFF;
    maData.mpSalMenu = NULL;
}

SalMenuItem::~SalMenuItem()
{
    if( maData.mpSalMenu )
        ImplRemoveItemById( maData.mpSalMenu, maData.mnId );
}

// -------------------------------------------------------------------

