/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salmenu.cxx,v $
 * $Revision: 1.4 $
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

#define INCL_DOS
#define INCL_PM
#define INCL_WIN
#include <svpm.h>
#include <saldata.hxx>
#include <salinst.h>
#include <salmenu.h>


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

