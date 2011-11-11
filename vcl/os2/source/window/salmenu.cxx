/*************************************************************************

   Copyright 2011 Yuri Dario <mc6530@mclink.it>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 ************************************************************************/

#define INCL_DOS
#define INCL_PM
#define INCL_WIN
#include <svpm.h>

#include "os2/saldata.hxx"
#include "os2/salinst.h"
#include "os2/salmenu.h"


// =======================================================================

// Os2SalInst factory methods

SalMenu* Os2SalInstance::CreateMenu( sal_Bool bMenuBar, Menu* )
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

sal_Bool Os2SalMenu::VisibleMenuBar()
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

void Os2SalMenu::CheckItem( unsigned nPos, sal_Bool bCheck )
{
}

void Os2SalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
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

