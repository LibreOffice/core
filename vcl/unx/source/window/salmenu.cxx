/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salmenu.cxx,v $
 * $Revision: 1.9 $
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


#include <saldata.hxx>
#include <salinst.h>
#include <salmenu.h>


// =======================================================================

// X11SalInst factory methods

SalMenu* X11SalInstance::CreateMenu( BOOL /*bMenuBar*/ )
{
    return NULL;  // no support for native menues
}

void X11SalInstance::DestroyMenu( SalMenu* pSalMenu )
{
    delete pSalMenu;
}


SalMenuItem* X11SalInstance::CreateMenuItem( const SalItemParams* )
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

void X11SalMenu::SetFrame( const SalFrame* )
{
}

void X11SalMenu::InsertItem( SalMenuItem*, unsigned )
{
}

void X11SalMenu::RemoveItem( unsigned )
{
}

void X11SalMenu::SetSubMenu( SalMenuItem*, SalMenu*, unsigned )
{
}

void X11SalMenu::CheckItem( unsigned, BOOL )
{
}

void X11SalMenu::EnableItem( unsigned, BOOL )
{
}

void X11SalMenu::SetItemImage( unsigned, SalMenuItem*, const Image& )
{
}

void X11SalMenu::SetItemText( unsigned, SalMenuItem*, const XubString& )
{
}

void X11SalMenu::SetAccelerator( unsigned, SalMenuItem*, const KeyCode&, const XubString& )
{
}

void X11SalMenu::GetSystemMenuData( SystemMenuData* )
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

