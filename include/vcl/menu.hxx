/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SV_MENU_HXX
#define _SV_MENU_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <tools/rc.hxx>
#include <tools/resid.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <vcl/bitmapex.hxx>
#include <tools/color.hxx>
#include <vcl/vclevent.hxx>
#include <com/sun/star/uno/Reference.hxx>

struct MenuItemData;
class Point;
class Size;
class Rectangle;
class Menu;
class MenuItemList;
class HelpEvent;
class Image;
class PopupMenu;
class KeyCode;
class KeyEvent;
class MenuFloatingWindow;
class Window;
class SalMenu;
struct SystemMenuData;

namespace com {
namespace sun {
namespace star {
namespace accessibility {
    class XAccessible;
}}}}

namespace vcl { struct MenuLayoutData; }

// --------------
// - Menu-Types -
// --------------

#define MENU_APPEND             ((sal_uInt16)0xFFFF)
#define MENU_ITEM_NOTFOUND      ((sal_uInt16)0xFFFF)

#define POPUPMENU_EXECUTE_DOWN  ((sal_uInt16)0x0001)
#define POPUPMENU_EXECUTE_UP    ((sal_uInt16)0x0002)
#define POPUPMENU_EXECUTE_LEFT  ((sal_uInt16)0x0004)
#define POPUPMENU_EXECUTE_RIGHT ((sal_uInt16)0x0008)

#define POPUPMENU_NOMOUSEUPCLOSE ((sal_uInt16)0x0010)

#define MENU_FLAG_NOAUTOMNEMONICS       0x0001
#define MENU_FLAG_HIDEDISABLEDENTRIES   0x0002

// overrides default hiding of disabled entries in popup menus
#define MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES 0x0004

struct ImplMenuDelData
{
    ImplMenuDelData* mpNext;
    const Menu* mpMenu;

    explicit ImplMenuDelData( const Menu* );
    ~ImplMenuDelData();

    bool isDeleted() const { return mpMenu == 0; }
};

// --------
// - Menu -
// --------

struct MenuLogo
{
    BitmapEx    aBitmap;
    Color       aStartColor;
    Color       aEndColor;
};

class VCL_DLLPUBLIC Menu : public Resource
{
    friend class MenuBar;
    friend class MenuBarWindow;
    friend class MenuButton;
    friend class MenuFloatingWindow;
    friend class PopupMenu;
    friend class SystemWindow;
    friend struct ImplMenuDelData;
private:
    ImplMenuDelData*    mpFirstDel;
    MenuItemList*       pItemList;          // Liste mit den MenuItems
    MenuLogo*           pLogo;
    Menu*               pStartedFrom;
    Window*             pWindow;

    Link                aActivateHdl;       // Active-Handler
    Link                aDeactivateHdl;     // Deactivate-Handler
    Link                aHighlightHdl;      // Highlight-Handler
    Link                aSelectHdl;         // Highlight-Handler

    VclEventListeners   maEventListeners;
    VclEventListeners   maChildEventListeners;

    OUString            aTitleText;         // PopupMenu text

    sal_uLong           nEventId;
    sal_uInt16          mnHighlightedItemPos; // for native menus: keeps track of the highlighted item
    sal_uInt16          nMenuFlags;
    sal_uInt16          nDefaultItem;       // Id of default item
    sal_uInt16          nSelectedId;

    // for output:
    sal_uInt16          nImgOrChkPos;
    sal_uInt16          nTextPos;

    sal_Bool            bIsMenuBar  : 1,        // Is this a menubar?
                        bCanceled   : 1,        // Terminated during a callback
                        bInCallback : 1,        // In Activate/Deactivate
                        bKilled     : 1;        // Killed...

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > mxAccessible;
    mutable vcl::MenuLayoutData* mpLayoutData;
    SalMenu*            mpSalMenu;

protected:
    SAL_DLLPRIVATE void             ImplInit();
    SAL_DLLPRIVATE void             ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE Menu*            ImplGetStartMenu();
    SAL_DLLPRIVATE Menu*            ImplFindSelectMenu();
    SAL_DLLPRIVATE Menu*            ImplFindMenu( sal_uInt16 nId );
    SAL_DLLPRIVATE Size             ImplCalcSize( Window* pWin );
    SAL_DLLPRIVATE sal_Bool             ImplIsVisible( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE sal_Bool             ImplIsSelectable( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE sal_uInt16           ImplGetVisibleItemCount() const;
    SAL_DLLPRIVATE sal_uInt16           ImplGetFirstVisible() const;
    SAL_DLLPRIVATE sal_uInt16           ImplGetPrevVisible( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE sal_uInt16           ImplGetNextVisible( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE void             ImplPaint( Window* pWin, sal_uInt16 nBorder, long nOffY = 0, MenuItemData* pThisDataOnly = 0, sal_Bool bHighlighted = sal_False, bool bLayout = false, bool bRollover = false ) const;
    SAL_DLLPRIVATE void             ImplSelect();
    SAL_DLLPRIVATE void             ImplCallHighlight( sal_uInt16 nHighlightItem );
    SAL_DLLPRIVATE void             ImplCallEventListeners( sal_uLong nEvent, sal_uInt16 nPos );
    DECL_DLLPRIVATE_LINK(           ImplCallSelect, void* );

    SAL_DLLPRIVATE void             ImplFillLayoutData() const;
    SAL_DLLPRIVATE SalMenu*         ImplGetSalMenu() { return mpSalMenu; }
    SAL_DLLPRIVATE void             ImplSetSalMenu( SalMenu *pMenu );
    SAL_DLLPRIVATE const XubString& ImplGetHelpText( sal_uInt16 nItemId ) const;

    // returns native check and option menu symbol height in rCheckHeight and rRadioHeight
    // return value is maximum width and height of checkboxes and radiobuttons
    SAL_DLLPRIVATE Size             ImplGetNativeCheckAndRadioSize( Window*, long& rCheckHeight, long& rRadioHeight ) const;

    // returns native submenu arrow size and spacing from right border
    // return value is whether it's supported natively
    SAL_DLLPRIVATE sal_Bool         ImplGetNativeSubmenuArrowSize( Window* pWin, Size& rArrowSize, long& rArrowSpacing ) const;

    SAL_DLLPRIVATE void                ImplAddDel( ImplMenuDelData &rDel );
    SAL_DLLPRIVATE void                ImplRemoveDel( ImplMenuDelData &rDel );
public:
    SAL_DLLPRIVATE void             ImplKillLayoutData() const;
    SAL_DLLPRIVATE Menu*            ImplGetStartedFrom() const { return pStartedFrom; }

                            Menu();
    explicit                Menu( sal_Bool bMenuBar );
    SAL_DLLPRIVATE Window*  ImplGetWindow() const { return pWindow; }
    void ImplSelectWithStart( Menu* pStartMenu = NULL );

    public:
    virtual             ~Menu();

    virtual void        Activate();
    virtual void        Deactivate();
    virtual void        Highlight();
    virtual void        Select();
    virtual void        RequestHelp( const HelpEvent& rHEvt );

    void                InsertItem( sal_uInt16 nItemId, const XubString& rStr,
                                    MenuItemBits nItemBits = 0,
                                    const OString &rIdent = OString(),
                                    sal_uInt16 nPos = MENU_APPEND );
    void                InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                    MenuItemBits nItemBits = 0,
                                    const OString &rIdent = OString(),
                                    sal_uInt16 nPos = MENU_APPEND );
    void                InsertItem( sal_uInt16 nItemId,
                                    const XubString& rString, const Image& rImage,
                                    MenuItemBits nItemBits = 0,
                                    const OString &rIdent = OString(),
                                    sal_uInt16 nPos = MENU_APPEND );
    void                InsertItem( const ResId& rResId, sal_uInt16 nPos = MENU_APPEND );
    void                InsertSeparator( const OString &rIdent = OString(),
                            sal_uInt16 nPos = MENU_APPEND );
    void                RemoveItem( sal_uInt16 nPos );
    void                CopyItem( const Menu& rMenu, sal_uInt16 nPos,
                                  sal_uInt16 nNewPos = MENU_APPEND );
    void                Clear();

    void                CreateAutoMnemonics();

    void                SetMenuFlags( sal_uInt16 nFlags ) { nMenuFlags = nFlags; }
    sal_uInt16              GetMenuFlags() const { return nMenuFlags; }

    sal_uInt16              GetItemCount() const;
    sal_uInt16          GetItemId(sal_uInt16 nPos) const;
    sal_uInt16          GetItemId(const OString &rIdent) const;
    sal_uInt16          GetItemPos( sal_uInt16 nItemId ) const;
    OString             GetItemIdent(sal_uInt16 nItemId) const;
    MenuItemType        GetItemType( sal_uInt16 nPos ) const;
    sal_uInt16          GetCurItemId() const;
    OString             GetCurItemIdent() const;

    void                SetDefaultItem( sal_uInt16 nItemId )    { nDefaultItem = nItemId; }
    sal_uInt16              GetDefaultItem() const              { return nDefaultItem; }

    void                SetItemBits( sal_uInt16 nItemId, MenuItemBits nBits );
    MenuItemBits        GetItemBits( sal_uInt16 nItemId ) const;

    void                SetUserValue( sal_uInt16 nItemId, sal_uLong nValue );
    sal_uLong               GetUserValue( sal_uInt16 nItemId ) const;

    void                SetPopupMenu( sal_uInt16 nItemId, PopupMenu* pMenu );
    PopupMenu*          GetPopupMenu( sal_uInt16 nItemId ) const;

    void                SetAccelKey( sal_uInt16 nItemId, const KeyCode& rKeyCode );
    KeyCode             GetAccelKey( sal_uInt16 nItemId ) const;

    void                CheckItem( sal_uInt16 nItemId, sal_Bool bCheck = sal_True );
    sal_Bool                IsItemChecked( sal_uInt16 nItemId ) const;

    void                SelectItem( sal_uInt16 nItemId );
    void                DeSelect() { SelectItem( 0xFFFF ); } // MENUITEMPOS_INVALID

    void                EnableItem( sal_uInt16 nItemId, sal_Bool bEnable = sal_True );
    void                EnableItem(const OString &rIdent, bool bEnable = true)
    {
        EnableItem(GetItemId(rIdent), bEnable);
    }
    sal_Bool            IsItemEnabled( sal_uInt16 nItemId ) const;

    void                ShowItem( sal_uInt16 nItemId, sal_Bool bVisible = sal_True );
    void                HideItem( sal_uInt16 nItemId ) { ShowItem( nItemId, sal_False ); }

    sal_Bool            IsItemPosVisible( sal_uInt16 nItemPos ) const;
    sal_Bool            IsMenuVisible() const;
    sal_Bool            IsMenuBar() const { return bIsMenuBar; }

    void                RemoveDisabledEntries( sal_Bool bCheckPopups = sal_True, sal_Bool bRemoveEmptyPopups = sal_False );
    sal_Bool            HasValidEntries( sal_Bool bCheckPopups = sal_True );

    void                SetItemText( sal_uInt16 nItemId, const XubString& rStr );
    XubString           GetItemText( sal_uInt16 nItemId ) const;

    void                SetItemImage( sal_uInt16 nItemId, const Image& rImage );
    Image               GetItemImage( sal_uInt16 nItemId ) const;
    void                SetItemImageAngle( sal_uInt16 nItemId, long nAngle10 );
    long                GetItemImageAngle( sal_uInt16 nItemId ) const;
    void                SetItemImageMirrorMode( sal_uInt16 nItemId, sal_Bool bMirror );
    sal_Bool            GetItemImageMirrorMode( sal_uInt16 ) const;

    void                SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand );
    const OUString      GetItemCommand( sal_uInt16 nItemId ) const;

    void                SetHelpText( sal_uInt16 nItemId, const XubString& rString );
    const XubString&    GetHelpText( sal_uInt16 nItemId ) const;

    void                SetTipHelpText( sal_uInt16 nItemId, const XubString& rString );
    const XubString&    GetTipHelpText( sal_uInt16 nItemId ) const;

    void                SetHelpCommand( sal_uInt16 nItemId, const XubString& rString );
    const XubString&    GetHelpCommand( sal_uInt16 nItemId ) const;

    void                SetHelpId( sal_uInt16 nItemId, const OString& rHelpId );
    OString        GetHelpId( sal_uInt16 nItemId ) const;

    void                SetActivateHdl( const Link& rLink )     { aActivateHdl = rLink; }
    const Link&         GetActivateHdl() const                  { return aActivateHdl; }

    void                SetDeactivateHdl( const Link& rLink )   { aDeactivateHdl = rLink; }
    const Link&         GetDeactivateHdl() const                { return aDeactivateHdl; }

    void                SetHighlightHdl( const Link& rLink )    { aHighlightHdl = rLink; }
    const Link&         GetHighlightHdl() const                 { return aHighlightHdl; }

    void                SetSelectHdl( const Link& rLink )       { aSelectHdl = rLink; }
    const Link&         GetSelectHdl() const                    { return aSelectHdl; }

    void                SetLogo( const MenuLogo& rLogo );
    void                SetLogo();
    sal_Bool                HasLogo() const { return pLogo ? sal_True : sal_False; }
    MenuLogo            GetLogo() const;

    void                AddEventListener( const Link& rEventListener );
    void                RemoveEventListener( const Link& rEventListener );
    void                AddChildEventListener( const Link& rEventListener );
    void                RemoveChildEventListener( const Link& rEventListener );

    Menu&               operator =( const Menu& rMenu );

    // Fuer Menu-'Funktionen'
    MenuItemList*       GetItemList() const                     { return pItemList; }

    // returns the system's menu handle if native menus are supported
    // pData must point to a SystemMenuData structure
    sal_Bool                GetSystemMenuData( SystemMenuData* pData ) const;

    // accessibility helpers

    // returns the bounding box for the character at index nIndex
    // where nIndex is relative to the starting index of the item
    // with id nItemId (in coordinates of the displaying window)
    Rectangle GetCharacterBounds( sal_uInt16 nItemId, long nIndex ) const;
    // -1 is returned if no character is at that point
    // if an index is found the corresponding item id is filled in (else 0)
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rItemID ) const;
    // returns the bounding rectangle for an item at pos nItemPos
    Rectangle GetBoundingRectangle( sal_uInt16 nItemPos ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetAccessible();
    void SetAccessible( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible );

    // gets the activation key of the specified item
    KeyEvent GetActivationKey( sal_uInt16 nItemId ) const;

    Window*             GetWindow() const { return pWindow; }

    void                SetAccessibleName( sal_uInt16 nItemId, const XubString& rStr );
    XubString           GetAccessibleName( sal_uInt16 nItemId ) const;

    // returns whether the item a position nItemPos is highlighted or not.
    bool  IsHighlighted( sal_uInt16 nItemPos ) const;

    void                HighlightItem( sal_uInt16 nItemPos );
    void                DeHighlight() { HighlightItem( 0xFFFF ); } // MENUITEMPOS_INVALID
};

// -----------
// - MenuBar -
// -----------

class VCL_DLLPUBLIC MenuBar : public Menu
{
    Link                maCloserHdl;
    Link                maFloatHdl;
    Link                maHideHdl;
    sal_Bool            mbCloserVisible;
    sal_Bool            mbFloatBtnVisible;
    sal_Bool            mbHideBtnVisible;
    sal_Bool            mbDisplayable;

    friend class Application;
    friend class Menu;
    friend class MenuBarWindow;
    friend class MenuFloatingWindow;
    friend class SystemWindow;

    SAL_DLLPRIVATE static Window*   ImplCreate( Window* pParent, Window* pWindow, MenuBar* pMenu );
    SAL_DLLPRIVATE static void      ImplDestroy( MenuBar* pMenu, sal_Bool bDelete );
    SAL_DLLPRIVATE sal_Bool         ImplHandleKeyEvent( const KeyEvent& rKEvent, sal_Bool bFromMenu = sal_True );

public:
                        MenuBar();
                        MenuBar( const MenuBar& rMenu );
                        ~MenuBar();

    MenuBar&            operator =( const MenuBar& rMenu );

    void                ShowCloser( sal_Bool bShow = sal_True );
    sal_Bool            HasCloser() const { return mbCloserVisible; }
    sal_Bool            HasFloatButton() const { return mbFloatBtnVisible; }
    sal_Bool            HasHideButton() const { return mbHideBtnVisible; }
    void                ShowButtons( sal_Bool bClose, sal_Bool bFloat, sal_Bool bHide );

    void                SelectEntry( sal_uInt16 nId );
    sal_Bool            HandleMenuActivateEvent( Menu *pMenu ) const;
    sal_Bool            HandleMenuDeActivateEvent( Menu *pMenu ) const;
    sal_Bool            HandleMenuHighlightEvent( Menu *pMenu, sal_uInt16 nEventId ) const;
    sal_Bool            HandleMenuCommandEvent( Menu *pMenu, sal_uInt16 nEventId ) const;
    sal_Bool            HandleMenuButtonEvent( Menu *pMenu, sal_uInt16 nEventId ) const;

    void                SetCloserHdl( const Link& rLink )           { maCloserHdl = rLink; }
    const Link&         GetCloserHdl() const                        { return maCloserHdl; }
    void                SetFloatButtonClickHdl( const Link& rLink ) { maFloatHdl = rLink; }
    const Link&         GetFloatButtonClickHdl() const              { return maFloatHdl; }
    void                SetHideButtonClickHdl( const Link& rLink )  { maHideHdl = rLink; }
    const Link&         GetHideButtonClickHdl() const               { return maHideHdl; }

    //  - by default a menubar is displayable
    //  - if a menubar is not displayable, its MenuBarWindow will never be shown
    //    and it will be hidden if it was visible before
    //  - note: if a menubar is diplayable, this does not necessarily mean that it is currently visible
    void                SetDisplayable( sal_Bool bDisplayable );
    sal_Bool            IsDisplayable() const                       { return mbDisplayable; }

    struct MenuBarButtonCallbackArg
    {
        sal_uInt16      nId;             // Id of the button
        bool            bHighlight;      // highlight on/off
        MenuBar*        pMenuBar;        // menubar the button belongs to
    };
    // add an arbitrary button to the menubar (will appear next to closer)
    // passed link will be call with a MenuBarButtonCallbackArg on press
    // passed string will be set as tooltip
    sal_uInt16          AddMenuBarButton( const Image&, const Link&, const String&, sal_uInt16 nPos = 0 );
    // set the highlight link for additional button with ID nId
    // highlight link will be called with a MenuBarButtonHighlightArg
    // the bHighlight member of that struct shall contain the new state
    void                SetMenuBarButtonHighlightHdl( sal_uInt16 nId, const Link& );
    // returns the rectangle occupied by the additional button named nId
    // coordinates are relative to the systemwindiow the menubar is attached to
    // if the menubar is unattached an empty rectangle is returned
    Rectangle           GetMenuBarButtonRectPixel( sal_uInt16 nId );
    void                RemoveMenuBarButton( sal_uInt16 nId );
};

inline MenuBar& MenuBar::operator =( const MenuBar& rMenu )
{
    Menu::operator =( rMenu );
    return *this;
}


// -------------
// - PopupMenu -
// -------------

class VCL_DLLPUBLIC PopupMenu : public Menu
{
    friend class Menu;
    friend class MenuFloatingWindow;
    friend class MenuBarWindow;
    friend struct MenuItemData;

private:
    Menu**              pRefAutoSubMenu;    // keeps track if a pointer to this Menu is stored in the MenuItemData

    SAL_DLLPRIVATE MenuFloatingWindow*   ImplGetFloatingWindow() const { return (MenuFloatingWindow*)Menu::ImplGetWindow(); }

protected:
    SAL_DLLPRIVATE sal_uInt16                ImplExecute( Window* pWindow, const Rectangle& rRect, sal_uLong nPopupFlags, Menu* pStaredFrom, sal_Bool bPreSelectFirst );
    SAL_DLLPRIVATE long                  ImplCalcHeight( sal_uInt16 nEntries ) const;
    SAL_DLLPRIVATE sal_uInt16                ImplCalcVisEntries( long nMaxHeight, sal_uInt16 nStartEntry = 0, sal_uInt16* pLastVisible = NULL ) const;

public:
                        PopupMenu();
                        PopupMenu( const PopupMenu& rMenu );
    explicit            PopupMenu( const ResId& );
    virtual             ~PopupMenu();

    void                SetText( const OUString& rTitle )  { aTitleText = rTitle; }
    const OUString&     GetText() const                     { return aTitleText; }

    sal_uInt16              Execute( Window* pWindow, const Point& rPopupPos );
    sal_uInt16              Execute( Window* pWindow, const Rectangle& rRect, sal_uInt16 nFlags = 0 );

    // Fuer das TestTool
    void                EndExecute( sal_uInt16 nSelect = 0 );
    void                SelectEntry( sal_uInt16 nId );
    void                SetSelectedEntry( sal_uInt16 nId ); // for use by native submenu only

    static sal_Bool         IsInExecute();
    static PopupMenu*   GetActivePopupMenu();

    PopupMenu&          operator =( const PopupMenu& rMenu );
};

inline PopupMenu& PopupMenu::operator =( const PopupMenu& rMenu )
{
    Menu::operator =( rMenu );
    return *this;
}

#endif // _SV_MENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
