/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salmenu.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:51:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALMENU_H
#include <salmenu.h>
#endif


// =======================================================================

// X11SalInst factory methods

SalMenu* X11SalInstance::CreateMenu( BOOL bMenuBar )
{
    return NULL;  // no support for native menues
}

void X11SalInstance::DestroyMenu( SalMenu* pSalMenu )
{
    delete pSalMenu;
}


SalMenuItem* X11SalInstance::CreateMenuItem( const SalItemParams* pItemData )
{
    return NULL;  // no support for native menues
}

void X11SalInstance::DestroyMenuItem( SalMenuItem* pSalMenuItem )
{
    delete pSalMenuItem;
}


// =======================================================================


/*
 * X11SalMenu
 */


X11SalMenu::~X11SalMenu()
{
}

BOOL X11SalMenu::VisibleMenuBar()
{
    return FALSE;
}

void X11SalMenu::SetFrame( const SalFrame *pFrame )
{
}

void X11SalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
}

void X11SalMenu::RemoveItem( unsigned nPos )
{
}

void X11SalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
}

void X11SalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
}

void X11SalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
}

void X11SalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
}

void X11SalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
}

void X11SalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
}

void X11SalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

// =======================================================================

/*
 * SalMenuItem
 */


X11SalMenuItem::~X11SalMenuItem()
{
}

// -------------------------------------------------------------------

