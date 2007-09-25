/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salmenu.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 10:08:19 $
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

#define INCL_DOS
#define INCL_PM
#define INCL_WIN
#include <svpm.h>

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

// Os2SalInst factory methods

SalMenu* Os2SalInstance::CreateMenu( BOOL bMenuBar )
{
    return NULL;  // no support for native menues
}

void Os2SalInstance::DestroyMenu( SalMenu* pSalMenu )
{
    delete pSalMenu;
}


SalMenuItem* Os2SalInstance::CreateMenuItem( const SalItemParams* pItemData )
{
    return NULL;  // no support for native menues
}

void Os2SalInstance::DestroyMenuItem( SalMenuItem* pSalMenuItem )
{
    delete pSalMenuItem;
}


// =======================================================================


/*
 * Os2SalMenu
 */


Os2SalMenu::~Os2SalMenu()
{
}

BOOL Os2SalMenu::VisibleMenuBar()
{
    return FALSE;
}

void Os2SalMenu::SetFrame( const SalFrame *pFrame )
{
}

void Os2SalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
}

void Os2SalMenu::RemoveItem( unsigned nPos )
{
}

void Os2SalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
}

void Os2SalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
}

void Os2SalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
}

void Os2SalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
}

void Os2SalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
}

void Os2SalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
}

void Os2SalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

// =======================================================================

/*
 * SalMenuItem
 */


Os2SalMenuItem::~Os2SalMenuItem()
{
}

// -------------------------------------------------------------------

