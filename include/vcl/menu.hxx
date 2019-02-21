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

#ifndef INCLUDED_VCL_MENU_HXX
#define INCLUDED_VCL_MENU_HXX

#include <memory>
#include <vcl/vclenum.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/keycod.hxx>
#include <vcl/vclreferencebase.hxx>
#include <vcl/vclevent.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <list>

class OutputDevice;
struct ImplSVEvent;
struct MenuItemData;
class Point;
class Size;
namespace tools { class Rectangle; }
class Menu;
class MenuItemList;
class Image;
class PopupMenu;
class KeyEvent;
class CommandEvent;
class MenuFloatingWindow;
class SalMenu;
class MenuBarWindow;
struct SystemMenuData;
enum class FloatWinPopupFlags;

namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
}}}}

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } }

namespace vcl
{
class Window;
struct MenuLayoutData;
typedef OutputDevice RenderContext; // same as in include/vcl/outdev.hxx
class ILibreOfficeKitNotifier;
}

#define MENU_APPEND        (sal_uInt16(0xFFFF))
#define MENU_ITEM_NOTFOUND (sal_uInt16(0xFFFF))

// Must match the definitions in css::awt::PopupMenuDirection.idl
enum class PopupMenuFlags
{
    NONE            = 0x0000,
    ExecuteDown     = 0x0001,
    ExecuteUp       = 0x0002,
    ExecuteLeft     = 0x0004,
    ExecuteRight    = 0x0008,
    NoMouseUpClose  = 0x0010,
//If there isn't enough space to put the menu where it wants
//to go, then they will be autoplaced. Toggle this bit
//on to force menus to be placed either above or below
//the starting rectangle and shrunk to fit and then scroll rather than place
//the menu beside that rectangle
    NoHorzPlacement = 0x0020,
};

namespace o3tl
{
    template<> struct typed_flags<PopupMenuFlags> : is_typed_flags<PopupMenuFlags, 0x003f> {};
}

enum class MenuFlags
{
    NONE                      = 0x0000,
    NoAutoMnemonics           = 0x0001,
    HideDisabledEntries       = 0x0002,
    // overrides default hiding of disabled entries in popup menus
    AlwaysShowDisabledEntries = 0x0004,
};

namespace o3tl
{
    template<> struct typed_flags<MenuFlags> : is_typed_flags<MenuFlags, 0x0007> {};
}

/// Invalid menu item id
#define ITEMPOS_INVALID 0xFFFF

struct ImplMenuDelData
{
    ImplMenuDelData* mpNext;
    VclPtr<const Menu> mpMenu;

    explicit ImplMenuDelData( const Menu* );
    ~ImplMenuDelData();

    bool isDeleted() const { return mpMenu == nullptr; }
};

typedef void (*MenuUserDataReleaseFunction)(void*);

class VCL_DLLPUBLIC Menu : public VclReferenceBase
{
    friend class MenuBar;
    friend class MenuBarWindow;
    friend class MenuButton;
    friend class MenuFloatingWindow;
    friend class PopupMenu;
    friend class SystemWindow;
    friend struct ImplMenuDelData;
private:
    ImplMenuDelData* mpFirstDel;
    std::unique_ptr<MenuItemList> pItemList; // list with MenuItems
    VclPtr<Menu> pStartedFrom;
    VclPtr<vcl::Window> pWindow;

    Link<Menu*, bool> aActivateHdl;       // Active-Handler
    Link<Menu*, bool> aDeactivateHdl;     // Deactivate-Handler
    Link<Menu*, bool> aSelectHdl;         // Select-Handler

    std::list<Link<VclMenuEvent&,void> >  maEventListeners;

    OUString aTitleText;         // PopupMenu text
    sal_uInt16 nTitleHeight;

    ImplSVEvent* nEventId;
    sal_uInt16 mnHighlightedItemPos; // for native menus: keeps track of the highlighted item
    MenuFlags nMenuFlags;
    sal_uInt16 nSelectedId;
    OString sSelectedIdent;

    // for output:
    sal_uInt16 nImgOrChkPos;
    sal_uInt16 nTextPos;

    bool bCanceled : 1; ///< Terminated during a callback
    bool bInCallback : 1; ///< In Activate/Deactivate
    bool bKilled : 1; ///< Killed

    css::uno::Reference<css::accessibility::XAccessible > mxAccessible;
    mutable std::unique_ptr<vcl::MenuLayoutData> mpLayoutData;
    std::unique_ptr<SalMenu> mpSalMenu;

protected:
    SAL_DLLPRIVATE Menu* ImplGetStartMenu();
    SAL_DLLPRIVATE Menu* ImplFindSelectMenu();
    SAL_DLLPRIVATE Menu* ImplFindMenu( sal_uInt16 nId );
    SAL_DLLPRIVATE Size  ImplCalcSize( vcl::Window* pWin );
    SAL_DLLPRIVATE bool  ImplIsVisible( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE bool  ImplCurrentlyHiddenOnGUI(sal_uInt16 nPos) const;
    SAL_DLLPRIVATE bool  ImplIsSelectable( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE sal_uInt16 ImplGetVisibleItemCount() const;
    SAL_DLLPRIVATE sal_uInt16 ImplGetFirstVisible() const;
    SAL_DLLPRIVATE sal_uInt16 ImplGetPrevVisible( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE sal_uInt16 ImplGetNextVisible( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE void ImplPaint(vcl::RenderContext& rRenderContext, Size const & rSize,
                                  sal_uInt16 nBorder, long nOffY = 0, MenuItemData const * pThisDataOnly = nullptr,
                                  bool bHighlighted = false, bool bLayout = false, bool bRollover = false ) const;
    SAL_DLLPRIVATE void ImplPaintMenuTitle(vcl::RenderContext&, const tools::Rectangle& rRect) const;
    SAL_DLLPRIVATE void ImplSelect();
    SAL_DLLPRIVATE void ImplCallHighlight( sal_uInt16 nHighlightItem );
    SAL_DLLPRIVATE void ImplCallEventListeners( VclEventId nEvent, sal_uInt16 nPos );
    DECL_DLLPRIVATE_LINK(ImplCallSelect, void*, void );

    SAL_DLLPRIVATE void ImplFillLayoutData() const;
    SAL_DLLPRIVATE SalMenu* ImplGetSalMenu() { return mpSalMenu.get(); }
    SAL_DLLPRIVATE void ImplClearSalMenu();
    SAL_DLLPRIVATE OUString ImplGetHelpText( sal_uInt16 nItemId ) const;

    // returns native check and option menu symbol height in rCheckHeight and rRadioHeight
    // return value is maximum width and height of checkboxes and radiobuttons
    SAL_DLLPRIVATE Size ImplGetNativeCheckAndRadioSize(vcl::RenderContext const & rRenderContext, long& rCheckHeight, long& rRadioHeight) const;

    // returns native submenu arrow size and spacing from right border
    // return value is whether it's supported natively
    SAL_DLLPRIVATE static bool ImplGetNativeSubmenuArrowSize(vcl::RenderContext const & rRenderContext, Size& rArrowSize, long& rArrowSpacing);

    SAL_DLLPRIVATE void ImplAddDel( ImplMenuDelData &rDel );
    SAL_DLLPRIVATE void ImplRemoveDel( ImplMenuDelData &rDel );

    SAL_DLLPRIVATE MenuItemData* NbcInsertItem(sal_uInt16 nId, MenuItemBits nBits,
                                               const OUString& rStr, Menu* pMenu,
                                               size_t nPos, const OString &rIdent);

    /// Close the 'pStartedFrom' menu window.
    virtual void ClosePopup(Menu* pMenu) = 0;

    /// Forward the KeyInput call to the MenuBar.
    virtual void MenuBarKeyInput(const KeyEvent& rEvent);

public:
    SAL_DLLPRIVATE void ImplKillLayoutData() const;

    SAL_DLLPRIVATE vcl::Window* ImplGetWindow() const { return pWindow; }
#if defined(MACOSX)
    void ImplSelectWithStart( Menu* pStartMenu = nullptr );
#endif

protected:

    /** The Menu constructor is protected.

    The callers are supposed to instantiate either PopupMenu or MenuBar, but
    not a Menu directly.
    */
    Menu();

public:
    virtual ~Menu() override;
    virtual void dispose() override;

    void Activate();
    void Deactivate();
    virtual void Select();

    void InsertItem(sal_uInt16 nItemId, const OUString& rStr,
                    MenuItemBits nItemBits = MenuItemBits::NONE,
                    const OString &rIdent = OString(),
                    sal_uInt16 nPos = MENU_APPEND);
    void InsertItem(sal_uInt16 nItemId, const Image& rImage,
                    MenuItemBits nItemBits = MenuItemBits::NONE,
                    const OString &rIdent = OString(),
                    sal_uInt16 nPos = MENU_APPEND);
    void InsertItem(sal_uInt16 nItemId,
                    const OUString& rString, const Image& rImage,
                    MenuItemBits nItemBits = MenuItemBits::NONE,
                    const OString &rIdent = OString(),
                    sal_uInt16 nPos = MENU_APPEND);
    void InsertItem(const OUString& rCommand,
                    const css::uno::Reference<css::frame::XFrame>& rFrame);
    void InsertSeparator(const OString &rIdent = OString(), sal_uInt16 nPos = MENU_APPEND);
    void RemoveItem( sal_uInt16 nPos );
    void Clear();

    void CreateAutoMnemonics();

    void SetMenuFlags( MenuFlags nFlags ) { nMenuFlags = nFlags; }
    MenuFlags GetMenuFlags() const { return nMenuFlags; }

    sal_uInt16 GetItemCount() const;
    sal_uInt16 GetItemId(sal_uInt16 nPos) const;
    sal_uInt16 GetItemId(const OString &rIdent) const;
    sal_uInt16 GetItemPos( sal_uInt16 nItemId ) const;
    OString GetItemIdent(sal_uInt16 nItemId) const;
    MenuItemType GetItemType( sal_uInt16 nPos ) const;
    sal_uInt16 GetCurItemId() const { return nSelectedId;}
    OString GetCurItemIdent() const { return sSelectedIdent; }
    void SetItemBits( sal_uInt16 nItemId, MenuItemBits nBits );
    MenuItemBits GetItemBits( sal_uInt16 nItemId ) const;

    void SetUserValue(sal_uInt16 nItemId, void* nUserValue, MenuUserDataReleaseFunction aFunc=nullptr);
    void* GetUserValue(sal_uInt16 nItemId) const;

    void SetPopupMenu( sal_uInt16 nItemId, PopupMenu* pMenu );
    PopupMenu* GetPopupMenu( sal_uInt16 nItemId ) const;

    void SetAccelKey( sal_uInt16 nItemId, const vcl::KeyCode& rKeyCode );
    vcl::KeyCode GetAccelKey( sal_uInt16 nItemId ) const;

    void CheckItem( sal_uInt16 nItemId, bool bCheck = true );
    void CheckItem( const OString &rIdent, bool bCheck = true );
    bool IsItemChecked( sal_uInt16 nItemId ) const;

    virtual void SelectItem(sal_uInt16 nItemId) = 0;

    void EnableItem( sal_uInt16 nItemId, bool bEnable = true );
    void EnableItem(const OString &rIdent, bool bEnable = true)
    {
        EnableItem(GetItemId(rIdent), bEnable);
    }
    bool IsItemEnabled( sal_uInt16 nItemId ) const;

    void ShowItem( sal_uInt16 nItemId, bool bVisible = true );
    void HideItem( sal_uInt16 nItemId ) { ShowItem( nItemId, false ); }

    bool IsItemPosVisible( sal_uInt16 nItemPos ) const;
    bool IsMenuVisible() const;
    virtual bool IsMenuBar() const = 0;

    void RemoveDisabledEntries( bool bCheckPopups = true, bool bRemoveEmptyPopups = false );

    void UpdateNativeMenu();

    void SetItemText( sal_uInt16 nItemId, const OUString& rStr );
    OUString GetItemText( sal_uInt16 nItemId ) const;

    void SetItemImage( sal_uInt16 nItemId, const Image& rImage );
    Image GetItemImage( sal_uInt16 nItemId ) const;

    void SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand );
    OUString GetItemCommand( sal_uInt16 nItemId ) const;

    void SetHelpText( sal_uInt16 nItemId, const OUString& rString );
    OUString GetHelpText( sal_uInt16 nItemId ) const;

    void SetTipHelpText( sal_uInt16 nItemId, const OUString& rString );
    OUString GetTipHelpText( sal_uInt16 nItemId ) const;

    void SetHelpCommand( sal_uInt16 nItemId, const OUString& rString );
    OUString GetHelpCommand( sal_uInt16 nItemId ) const;

    void SetHelpId( sal_uInt16 nItemId, const OString& rHelpId );
    OString GetHelpId( sal_uInt16 nItemId ) const;

    void SetActivateHdl( const Link<Menu *, bool>& rLink )
    {
        aActivateHdl = rLink;
    }

    void SetDeactivateHdl( const Link<Menu *, bool>& rLink )
    {
        aDeactivateHdl = rLink;
    }

    void  SetSelectHdl( const Link<Menu*,bool>& rLink )
    {
        aSelectHdl = rLink;
    }

    sal_uInt16 GetTitleHeight() const
    {
        return nTitleHeight;
    }

    void AddEventListener( const Link<VclMenuEvent&,void>& rEventListener );
    void RemoveEventListener( const Link<VclMenuEvent&,void>& rEventListener );

    Menu& operator =( const Menu& rMenu );

    // for menu functions
    MenuItemList* GetItemList() const
    {
        return pItemList.get();
    }

    // returns the system's menu handle if native menus are supported
    // pData must point to a SystemMenuData structure
    void GetSystemMenuData( SystemMenuData* pData ) const;

    // accessibility helpers

    // returns the bounding box for the character at index nIndex
    // where nIndex is relative to the starting index of the item
    // with id nItemId (in coordinates of the displaying window)
    tools::Rectangle GetCharacterBounds( sal_uInt16 nItemId, long nIndex ) const;
    // -1 is returned if no character is at that point
    // if an index is found the corresponding item id is filled in (else 0)
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rItemID ) const;
    // returns the bounding rectangle for an item at pos nItemPos
    tools::Rectangle GetBoundingRectangle( sal_uInt16 nItemPos ) const;

    css::uno::Reference<css::accessibility::XAccessible> GetAccessible();
    void SetAccessible(const css::uno::Reference<css::accessibility::XAccessible >& rxAccessible);

    // gets the activation key of the specified item
    KeyEvent GetActivationKey( sal_uInt16 nItemId ) const;

    vcl::Window* GetWindow() const { return pWindow; }

    OUString GetAccessibleName( sal_uInt16 nItemId ) const;

    // returns whether the item a position nItemPos is highlighted or not.
    bool IsHighlighted( sal_uInt16 nItemPos ) const;

    void HighlightItem( sal_uInt16 nItemPos );
    void DeHighlight() { HighlightItem( 0xFFFF ); } // MENUITEMPOS_INVALID

    bool HandleMenuCommandEvent(Menu *pMenu, sal_uInt16 nEventId) const;
    bool HandleMenuActivateEvent(Menu *pMenu) const;
    bool HandleMenuDeActivateEvent(Menu *pMenu) const;
};

class VCL_DLLPUBLIC MenuBar : public Menu
{
    Link<void*,void> maCloseHdl;
    bool mbCloseBtnVisible : 1;
    bool mbFloatBtnVisible : 1;
    bool mbHideBtnVisible : 1;
    bool mbDisplayable : 1;

    friend class Application;
    friend class Menu;
    friend class MenuBarWindow;
    friend class MenuFloatingWindow;
    friend class SystemWindow;

    SAL_DLLPRIVATE static VclPtr<vcl::Window> ImplCreate(vcl::Window* pParent, vcl::Window* pWindow, MenuBar* pMenu);
    SAL_DLLPRIVATE static void ImplDestroy(MenuBar* pMenu, bool bDelete);
    SAL_DLLPRIVATE bool ImplHandleKeyEvent(const KeyEvent& rKEvent);
    SAL_DLLPRIVATE bool ImplHandleCmdEvent(const CommandEvent& rCEvent);

protected:

    /// Return the MenuBarWindow.
    MenuBarWindow* getMenuBarWindow();

public:
    MenuBar();
    MenuBar( const MenuBar& rMenu );
    virtual ~MenuBar() override;
    virtual void dispose() override;

    MenuBar& operator =( const MenuBar& rMenu );

    virtual bool IsMenuBar() const override { return true; }

    /// Close the 'pStartedFrom' menu window.
    virtual void ClosePopup(Menu* pMenu) override;

    /// Forward the KeyInput call to the MenuBar.
    virtual void MenuBarKeyInput(const KeyEvent& rEvent) override;

    void ShowCloseButton( bool bShow );
    bool HasCloseButton() const { return mbCloseBtnVisible; }
    bool HasFloatButton() const { return mbFloatBtnVisible; }
    bool HasHideButton() const { return mbHideBtnVisible; }
    void ShowButtons( bool bClose, bool bFloat, bool bHide );

    virtual void SelectItem(sal_uInt16 nId) override;
    bool HandleMenuHighlightEvent(Menu *pMenu, sal_uInt16 nEventId) const;
    bool HandleMenuButtonEvent(sal_uInt16 nEventId);

    void SetCloseButtonClickHdl( const Link<void*,void>& rLink ) { maCloseHdl = rLink; }
    const Link<void*,void>& GetCloseButtonClickHdl() const       { return maCloseHdl; }

    //  - by default a menubar is displayable
    //  - if a menubar is not displayable, its MenuBarWindow will never be shown
    //    and it will be hidden if it was visible before
    //  - note: if a menubar is displayable, this does not necessarily mean that it is currently visible
    void SetDisplayable( bool bDisplayable );
    bool IsDisplayable() const                       { return mbDisplayable; }

    struct MenuBarButtonCallbackArg
    {
        sal_uInt16 nId;    // Id of the button
        bool bHighlight;   // highlight on/off
    };
    // add an arbitrary button to the menubar (will appear next to closer)
    // passed link will be call with a MenuBarButtonCallbackArg on press
    // passed string will be set as tooltip
    sal_uInt16 AddMenuBarButton( const Image&, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>&, const OUString& );
    // set the highlight link for additional button with ID nId
    // highlight link will be called with a MenuBarButtonHighlightArg
    // the bHighlight member of that struct shall contain the new state
    void SetMenuBarButtonHighlightHdl( sal_uInt16 nId, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>& );
    // returns the rectangle occupied by the additional button named nId
    // coordinates are relative to the systemwindow the menubar is attached to
    // if the menubar is unattached an empty rectangle is returned
    tools::Rectangle GetMenuBarButtonRectPixel( sal_uInt16 nId );
    void RemoveMenuBarButton( sal_uInt16 nId );
    void LayoutChanged();
    // get the height of the menubar, return the native menubar height if that is active or the vcl
    // one if not
    int GetMenuBarHeight() const;
};

inline MenuBar& MenuBar::operator=( const MenuBar& rMenu )
{
    Menu::operator=(rMenu);
    return *this;
}

class VCL_DLLPUBLIC PopupMenu : public Menu
{
    friend class Menu;
    friend class MenuFloatingWindow;
    friend class MenuBarWindow;
    friend struct MenuItemData;

private:
    const vcl::ILibreOfficeKitNotifier* mpLOKNotifier; ///< To emit the LOK callbacks eg. for dialog tunneling.

    SAL_DLLPRIVATE MenuFloatingWindow * ImplGetFloatingWindow() const;

protected:
    SAL_DLLPRIVATE sal_uInt16 ImplExecute( const VclPtr<vcl::Window>& pW, const tools::Rectangle& rRect, FloatWinPopupFlags nPopupModeFlags, Menu* pSFrom, bool bPreSelectFirst );
    SAL_DLLPRIVATE void ImplFlushPendingSelect();
    SAL_DLLPRIVATE long ImplCalcHeight( sal_uInt16 nEntries ) const;
    SAL_DLLPRIVATE sal_uInt16 ImplCalcVisEntries( long nMaxHeight, sal_uInt16 nStartEntry, sal_uInt16* pLastVisible = nullptr ) const;

public:
    PopupMenu();
    PopupMenu( const PopupMenu& rMenu );
    virtual ~PopupMenu() override;

    virtual bool IsMenuBar() const override { return false; }

    /// Close the 'pStartedFrom' menu window.
    virtual void ClosePopup(Menu* pMenu) override;

    void SetText( const OUString& rTitle )
    {
        aTitleText = rTitle;
    }

    sal_uInt16 Execute( vcl::Window* pWindow, const Point& rPopupPos );
    sal_uInt16 Execute( vcl::Window* pWindow, const tools::Rectangle& rRect, PopupMenuFlags nFlags = PopupMenuFlags::NONE );

    // for the TestTool
    void EndExecute();
    virtual void SelectItem(sal_uInt16 nId) override;
    void SetSelectedEntry( sal_uInt16 nId ); // for use by native submenu only

    static bool IsInExecute();
    static PopupMenu* GetActivePopupMenu();

    /// Interface to register for dialog / window tunneling.
    void SetLOKNotifier(const vcl::ILibreOfficeKitNotifier* pNotifier)
    {
        mpLOKNotifier = pNotifier;
    }

    PopupMenu& operator=( const PopupMenu& rMenu );
};

inline PopupMenu& PopupMenu::operator=( const PopupMenu& rMenu )
{
    Menu::operator=( rMenu );
    return *this;
}

#endif // INCLUDED_VCL_MENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
