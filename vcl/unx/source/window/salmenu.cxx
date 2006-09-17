/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salmenu.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:42:28 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"


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

