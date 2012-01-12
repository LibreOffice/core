/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_SALMENU_HXX
#define _SV_SALMENU_HXX

#include <vcl/sv.h>
#include "vcl/dllapi.h"
#include <vcl/menu.hxx>
#include <vcl/keycod.hxx>
#include <vcl/image.hxx>

struct SystemMenuData;
class FloatingWindow;
class SalFrame;

struct SalItemParams
{
    sal_uInt16          nId;                    // item Id
    MenuItemType    eType;                  // MenuItem-Type
    MenuItemBits    nBits;                  // MenuItem-Bits
    Menu*           pMenu;                  // Pointer to Menu
    XubString       aText;                  // Menu-Text
    Image           aImage;                 // Image
};


struct SalMenuButtonItem
{
    sal_uInt16              mnId;
    Image               maImage;
    rtl::OUString       maToolTipText;

    SalMenuButtonItem() : mnId( 0 ) {}
    SalMenuButtonItem( sal_uInt16 i_nId, const Image& rImg, const rtl::OUString& i_rTTText = rtl::OUString() )
    : mnId( i_nId ), maImage( rImg ), maToolTipText( i_rTTText ) {}
};

class VCL_PLUGIN_PUBLIC SalMenuItem
{
public:
    SalMenuItem() {}
    virtual ~SalMenuItem();
};

class VCL_PLUGIN_PUBLIC SalMenu
{
public:
    SalMenu() {}
    virtual ~SalMenu();

    virtual sal_Bool VisibleMenuBar() = 0;  // must return sal_True to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) = 0;
    virtual void RemoveItem( unsigned nPos ) = 0;
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) = 0;
    virtual void SetFrame( const SalFrame* pFrame ) = 0;
    virtual void CheckItem( unsigned nPos, sal_Bool bCheck ) = 0;
    virtual void EnableItem( unsigned nPos, sal_Bool bEnable ) = 0;
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )= 0;
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage ) = 0;
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName ) = 0;
    virtual void GetSystemMenuData( SystemMenuData* pData ) = 0;
    virtual bool ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, sal_uLong nFlags);
    virtual bool AddMenuBarButton( const SalMenuButtonItem& ); // return false if not implemented or failure
    virtual void RemoveMenuBarButton( sal_uInt16 nId );

    // return an empty rectangle if not implemented
    // return Rectangle( Point( -1, -1 ), Size( 1, 1 ) ) if menu bar buttons implemented
    // but rectangle cannot be determined
    virtual Rectangle GetMenuBarButtonRectPixel( sal_uInt16 i_nItemId, SalFrame* i_pReferenceFrame );
};


#endif // _SV_SALMENU_HXX

