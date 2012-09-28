/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2011 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef __SC_CHECKLISTMENU_HXX__
#define __SC_CHECKLISTMENU_HXX__

#include "vcl/popupmenuwindow.hxx"
#include "vcl/button.hxx"
#include "vcl/scrbar.hxx"
#include "vcl/timer.hxx"
#include "svx/checklbx.hxx"

#include <boost/unordered_map.hpp>
#include <boost/scoped_ptr.hpp>

namespace com { namespace sun { namespace star {

    namespace accessibility {
        class XAccessible;
    }

}}}

class ScDocument;
class ScAccessibleFilterMenu;

class ScMenuFloatingWindow : public PopupMenuFloatingWindow
{
public:
    static size_t MENU_NOT_SELECTED;
    /**
     * Action to perform when an event takes place.  Create a sub-class of
     * this to implement the desired action.
     */
    class Action
    {
    public:
        virtual ~Action() {}
        virtual void execute() = 0;
    };

    explicit ScMenuFloatingWindow(Window* pParent, ScDocument* pDoc, sal_uInt16 nMenuStackLevel = 0);
    virtual ~ScMenuFloatingWindow();

    virtual void PopupModeEnd();
    virtual void MouseMove(const MouseEvent& rMEvt);
    virtual void MouseButtonDown(const MouseEvent& rMEvt);
    virtual void MouseButtonUp(const MouseEvent& rMEvt);
    virtual void KeyInput(const KeyEvent& rKEvt);
    virtual void Paint(const Rectangle& rRect);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    void addMenuItem(const ::rtl::OUString& rText, bool bEnabled, Action* pAction);
    void addSeparator();

    ScMenuFloatingWindow* addSubMenuItem(const ::rtl::OUString& rText, bool bEnabled);
    void setSelectedMenuItem(size_t nPos, bool bSubMenuTimer, bool bEnsureSubMenu);
    void selectMenuItem(size_t nPos, bool bSelected, bool bSubMenuTimer);
    void clearSelectedMenuItem();
    ScMenuFloatingWindow* getSubMenuWindow(size_t nPos) const;
    bool isMenuItemSelected(size_t nPos) const;
    size_t getSelectedMenuItem() const;

    void setName(const ::rtl::OUString& rName);
    const ::rtl::OUString& getName() const;

    void executeMenuItem(size_t nPos);
    void getMenuItemPosSize(size_t nPos, Point& rPos, Size& rSize) const;
    ScMenuFloatingWindow* getParentMenuWindow() const;

protected:
    virtual void handlePopupEnd();

    Size getMenuSize() const;
    void drawMenuItem(size_t nPos);
    void drawSeparator(size_t nPos);
    void drawAllMenuItems();
    const Font& getLabelFont() const;

    void queueLaunchSubMenu(size_t nPos, ScMenuFloatingWindow* pMenu);
    void queueCloseSubMenu();
    void launchSubMenu(bool bSetMenuPos);
    void endSubMenu(ScMenuFloatingWindow* pSubMenu);

    void fillMenuItemsToAccessible(ScAccessibleFilterMenu* pAccMenu) const;

    ScDocument* getDoc();

protected:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > mxAccessible;

private:
    struct SubMenuItemData;
    void handleMenuTimeout(SubMenuItemData* pTimer);

    void resizeToFitMenuItems();
    void highlightMenuItem(size_t nPos, bool bSelected);

    size_t getEnclosingMenuItem(const Point& rPos) const;
    size_t getSubMenuPos(ScMenuFloatingWindow* pSubMenu);

    /**
     * Fire a menu highlight event since the accessibility framework needs
     * this to track focus on menu items.
     */
    void fireMenuHighlightedEvent();

    /**
     * Make sure that the specified submenu is permanently up, the submenu
     * close timer is not active, and the correct menu item associated with
     * the submenu is highlighted.
     */
    void setSubMenuFocused(ScMenuFloatingWindow* pSubMenu);

    /**
     * When a menu item of an invisible submenu is selected, we need to make
     * sure that all its parent menu(s) are visible, with the right menu item
     * highlighted in each of the parents.  Calling this method ensures it.
     */
    void ensureSubMenuVisible(ScMenuFloatingWindow* pSubMenu);

    /**
     * Dismiss any visible child submenus when a menu item of a parent menu is
     * selected.
     */
    void ensureSubMenuNotVisible();

    /**
     * Dismiss all visible popup menus and set focus back to the application
     * window.  This method is called e.g. when a menu action is fired.
     */
    void terminateAllPopupMenus();

private:

    struct MenuItemData
    {
        ::rtl::OUString maText;
        bool            mbEnabled:1;
        bool            mbSeparator:1;

        ::boost::shared_ptr<Action> mpAction;
        ::boost::shared_ptr<ScMenuFloatingWindow> mpSubMenuWin;

        MenuItemData();
    };

    ::std::vector<MenuItemData>         maMenuItems;

    struct SubMenuItemData
    {
        Timer                   maTimer;
        ScMenuFloatingWindow*   mpSubMenu;
        size_t                  mnMenuPos;

        DECL_LINK( TimeoutHdl, void* );

        SubMenuItemData(ScMenuFloatingWindow* pParent);
        void reset();

    private:
        ScMenuFloatingWindow* mpParent;
    };
    SubMenuItemData   maOpenTimer;
    SubMenuItemData   maCloseTimer;

    Font    maLabelFont;

    // Name of this menu window, taken from the menu item of the parent window
    // that launches it (if this is a sub menu).  If this is a top-level menu
    // window, then this name can be anything.
    ::rtl::OUString maName;

    size_t  mnSelectedMenu;
    size_t  mnClickedMenu;

    ScDocument* mpDoc;

    ScMenuFloatingWindow* mpParentMenu;
};

/**
 * This class implements a popup window for field button, for quick access
 * of hide-item list, and possibly more stuff related to field options.
 */
class ScCheckListMenuWindow : public ScMenuFloatingWindow
{
public:
    typedef boost::unordered_map<rtl::OUString, bool, rtl::OUStringHash> ResultType;

    /**
     * Extended data that the client code may need to store.  Create a
     * sub-class of this and store data there.
     */
    struct ExtendedData {};

    /**
     * Configuration options for this popup window.
     */
    struct Config
    {
        bool mbAllowEmptySet;
        Config();
    };

    explicit ScCheckListMenuWindow(Window* pParent, ScDocument* pDoc);
    virtual ~ScCheckListMenuWindow();

    virtual void MouseMove(const MouseEvent& rMEvt);
    virtual long Notify(NotifyEvent& rNEvt);
    virtual void Paint(const Rectangle& rRect);
    virtual Window* GetPreferredKeyInputWindow();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    void setMemberSize(size_t n);
    void addMember(const ::rtl::OUString& rName, bool bVisible);
    void initMembers();
    void setConfig(const Config& rConfig);

    const Size& getWindowSize() const;

    bool isAllSelected() const;
    void getResult(ResultType& rResult);
    void launch(const Rectangle& rRect);
    void close(bool bOK);

    /**
     * Set auxiliary data that the client code might need.  Note that this
     * popup window class manages its life time; no explicit deletion of the
     * instance is needed in the client code.
     */
    void setExtendedData(ExtendedData* p);

    /**
     * Get the store auxiliary data, or NULL if no such data is stored.
     */
    ExtendedData* getExtendedData();

    void setOKAction(Action* p);
    void setPopupEndAction(Action* p);

protected:
    virtual void handlePopupEnd();

private:
    struct Member
    {
        ::rtl::OUString maName;
        bool            mbVisible;

        Member();
    };

    class CancelButton : public ::CancelButton
    {
    public:
        CancelButton(ScCheckListMenuWindow* pParent);

        virtual void Click();

    private:
        ScCheckListMenuWindow* mpParent;
    };

    enum SectionType {
        WHOLE,                // entire window
        LISTBOX_AREA_OUTER,   // box enclosing the check box items.
        LISTBOX_AREA_INNER,   // box enclosing the check box items.
        SINGLE_BTN_AREA,      // box enclosing the single-action buttons.
        CHECK_TOGGLE_ALL,     // check box for toggling all items.
        BTN_SINGLE_SELECT,
        BTN_SINGLE_UNSELECT,
        BTN_OK,               // OK button
        BTN_CANCEL,           // Cancel button
    };
    void getSectionPosSize(Point& rPos, Size& rSize, SectionType eType) const;

    /**
     * Calculate the appropriate window size, the position and size of each
     * control based on the menu items.
     */
    void packWindow();
    void setAllMemberState(bool bSet);
    void selectCurrentMemberOnly(bool bSet);
    void cycleFocus(bool bReverse = false);

    DECL_LINK( ButtonHdl, Button* );
    DECL_LINK( TriStateHdl, void* );
    DECL_LINK( CheckHdl, SvTreeListBox* );

private:
    SvxCheckListBox maChecks;

    TriStateBox     maChkToggleAll;
    ImageButton     maBtnSelectSingle;
    ImageButton     maBtnUnselectSingle;

    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

    ::std::vector<Window*>          maTabStopCtrls;
    size_t                          mnCurTabStop;

    ::std::vector<Member>           maMembers;
    boost::scoped_ptr<ExtendedData> mpExtendedData;
    boost::scoped_ptr<Action>       mpOKAction;
    boost::scoped_ptr<Action>       mpPopupEndAction;

    Config maConfig;
    Size maWndSize;  /// whole window size.
    Size maMenuSize; /// size of all menu items combined.
    TriState mePrevToggleAllState;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
