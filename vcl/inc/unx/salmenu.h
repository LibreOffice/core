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



#ifndef _SV_SALMENU_H
#define _SV_SALMENU_H

#include <vcl/sv.h>
#include <vcl/bitmap.hxx>
#include <vcl/salmenu.hxx>


class X11SalMenu : public SalMenu
{
public:
    X11SalMenu() {}
    virtual ~X11SalMenu();

    virtual BOOL VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void RemoveItem( unsigned nPos );
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void SetFrame( const SalFrame* pFrame );
    virtual void CheckItem( unsigned nPos, BOOL bCheck );
    virtual void EnableItem( unsigned nPos, BOOL bEnable );
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage);
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName );
    virtual void GetSystemMenuData( SystemMenuData* pData );
};

class X11SalMenuItem : public SalMenuItem
{
public:
    X11SalMenuItem() {}
        virtual ~X11SalMenuItem();
};

#endif // _SV_SALMENU_H

