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

#include <vcl/bitmap.hxx>
#include <salmenu.hxx>

#define INIAPP_XWPHOOK              "XWorkplace:Hook"
#define INIKEY_HOOK_CONFIG          "Config"

typedef struct _HOOKCONFIG
{
    PM_BOOL         __fSlidingFocus;
    ULONG           __ulSlidingFocusDelay;
    PM_BOOL         __fSlidingBring2Top;
    PM_BOOL         __fSlidingIgnoreDesktop;
    PM_BOOL         __fSlidingIgnoreSeamless;
    HOBJECT         ahobjDummy[4];
    PM_BYTE         bMonitorDrives[30];
    PM_BOOL         fChordWinList;
    PM_BOOL         fSysMenuMB2TitleBar;
    PM_BOOL         fMB3Scroll;
    PM_BOOL         fMB3ScrollReverse;
    USHORT          usScrollMode;
    USHORT          usMB3ScrollMin;
    SHORT           sAmplification;
    PM_BOOL         __fAutoHideMouse;
    ULONG           __ulAutoHideDelay;
    PM_BOOL         __fGlobalHotkeys;
    PM_BOOL         fRemoved1, fRemoved2;
    PM_BOOL         fSlidingMenus;
    ULONG           ulSubmenuDelay;
    PM_BOOL         fMenuImmediateHilite;
    PM_BOOL         fMB3Click2MB1DblClk;
    HOBJECT         ahobjHotCornerObjects[8];
    PM_BOOL         fConditionalCascadeSensitive;
    PM_BOOL         fRemoved3;
    ULONG           ulCornerSensitivity;
    PM_BOOL         fMB3AutoScroll;
    PM_BOOL         fMB3Push2Bottom;
    ULONG           __ulAutoHideFlags;
    PM_BOOL         __fAutoMoveMouse;
    ULONG           __ulAutoMoveFlags;
    ULONG           __ulAutoMoveDelay;
    ULONG           __ulMouseMappingsCount;
} HOOKCONFIG, *PHOOKCONFIG;
#pragma pack()

class Os2SalMenu : public SalMenu
{
public:
    Os2SalMenu() {}
    virtual ~Os2SalMenu();

    virtual sal_Bool VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void RemoveItem( unsigned nPos );
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void SetFrame( const SalFrame* pFrame );
    virtual void CheckItem( unsigned nPos, sal_Bool bCheck );
    virtual void EnableItem( unsigned nPos, sal_Bool bEnable );
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage);
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName );
    virtual void GetSystemMenuData( SystemMenuData* pData );
};

class Os2SalMenuItem : public SalMenuItem
{
public:
    Os2SalMenuItem() {}
        virtual ~Os2SalMenuItem();
};

#endif // _SV_SALMENU_H

