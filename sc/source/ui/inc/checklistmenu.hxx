/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CHECKLISTMENU_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CHECKLISTMENU_HXX

#include <vcl/popupmenuwindow.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/timer.hxx>
#include <svx/checklbx.hxx>

#include <memory>
#include <unordered_map>

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

    explicit ScMenuFloatingWindow(vcl::Window* pParent, ScDocument* pDoc, sal_uInt16 nMenuStackLevel = 0);
    virtual ~ScMenuFloatingWindow();
     void dispose() SAL_OVERRIDE;

    virtual void PopupModeEnd() SAL_OVERRIDE;
    virtual void MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) SAL_OVERRIDE;
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() SAL_OVERRIDE;

    void addMenuItem(const OUString& rText, bool bEnabled, Action* pAction);
    void addSeparator();

    ScMenuFloatingWindow* addSubMenuItem(const OUString& rText, bool bEnabled);
    void setSelectedMenuItem(size_t nPos, bool bSubMenuTimer, bool bEnsureSubMenu);
    void selectMenuItem(size_t nPos, bool bSelected, bool bSubMenuTimer);
    void clearSelectedMenuItem();
    ScMenuFloatingWindow* getSubMenuWindow(size_t nPos) const;
    bool isMenuItemSelected(size_t nPos) const;
    size_t getSelectedMenuItem() const { return mnSelectedMenu;}

    void setName(const OUString& rName);
    const OUString& getName() const { return maName;}

    void executeMenuItem(size_t nPos);
    void getMenuItemPosSize(size_t nPos, Point& rPos, Size& rSize) const;
    ScMenuFloatingWindow* getParentMenuWindow() const { return mpParentMenu;}

protected:
    virtual void handlePopupEnd();

    Size getMenuSize() const;
    void drawMenuItem(vcl::RenderContext& rRenderContext, size_t nPos);
    void drawSeparator(vcl::RenderContext& rRenderContext, size_t nPos);
    void drawAllMenuItems(vcl::RenderContext& rRenderContext);
    const vcl::Font& getLabelFont() const
    {
        return maLabelFont;
    }

    void queueLaunchSubMenu(size_t nPos, ScMenuFloatingWindow* pMenu);
    void queueCloseSubMenu();
    void launchSubMenu(bool bSetMenuPos);
    void endSubMenu(ScMenuFloatingWindow* pSubMenu);

    void fillMenuItemsToAccessible(ScAccessibleFilterMenu* pAccMenu) const;

    ScDocument* getDoc() { return mpDoc;}

protected:
    css::uno::Reference<css::accessibility::XAccessible> mxAccessible;

private:
    struct SubMenuItemData;
    void handleMenuTimeout(SubMenuItemData* pTimer);

    void resizeToFitMenuItems();
    void highlightMenuItem(vcl::RenderContext& rRenderContext, size_t nPos, bool bSelected);

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
        OUString maText;
        bool     mbEnabled:1;
        bool     mbSeparator:1;

        std::shared_ptr<Action> mpAction;
        VclPtr<ScMenuFloatingWindow> mpSubMenuWin;

        MenuItemData();
    };

    std::vector<MenuItemData>         maMenuItems;

    struct SubMenuItemData
    {
        Timer                   maTimer;
        VclPtr<ScMenuFloatingWindow>   mpSubMenu;
        size_t                  mnMenuPos;

        DECL_LINK_TYPED( TimeoutHdl, Timer*, void );

        SubMenuItemData(ScMenuFloatingWindow* pParent);
        void reset();

    private:
        VclPtr<ScMenuFloatingWindow> mpParent;
    };
    SubMenuItemData   maOpenTimer;
    SubMenuItemData   maCloseTimer;

    vcl::Font         maLabelFont;

    // Name of this menu window, taken from the menu item of the parent window
    // that launches it (if this is a sub menu).  If this is a top-level menu
    // window, then this name can be anything.
    OUString maName;

    size_t  mnSelectedMenu;
    size_t  mnClickedMenu;

    ScDocument* mpDoc;

    VclPtr<ScMenuFloatingWindow> mpParentMenu;
};

class ScCheckListBox : public SvTreeListBox
{
    SvLBoxButtonData*   mpCheckButton;
    SvTreeListEntry* CountCheckedEntries( SvTreeListEntry* pParent, sal_uLong& nCount ) const;
    void            CheckAllChildren( SvTreeListEntry* pEntry, bool bCheck = true );

    public:

    ScCheckListBox( vcl::Window* pParent, WinBits nWinStyle = 0 );
    virtual ~ScCheckListBox() { disposeOnce(); }
    virtual void dispose() SAL_OVERRIDE { delete mpCheckButton; SvTreeListBox::dispose(); }
    void Init();
    void CheckEntry( const OUString& sName, SvTreeListEntry* pParent, bool bCheck = true );
    void CheckEntry( SvTreeListEntry* pEntry, bool bCheck = true );
    void ShowCheckEntry( const OUString& sName, SvTreeListEntry* pParent, bool bShow = true, bool bCheck = true );
    bool IsChecked( const OUString& sName, SvTreeListEntry* pParent );
    SvTreeListEntry* FindEntry( SvTreeListEntry* pParent, const OUString& sNode );
    sal_uInt16 GetCheckedEntryCount() const;
    void         ExpandChildren( SvTreeListEntry* pParent );
    virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
};
/**
 * This class implements a popup window for field button, for quick access
 * of hide-item list, and possibly more stuff related to field options.
 */
class ScCheckListMenuWindow : public ScMenuFloatingWindow
{
public:
    typedef std::unordered_map<OUString, bool, OUStringHash> ResultType;

    /**
     * Extended data that the client code may need to store.  Create a
     * sub-class of this and store data there.
     */
    struct ExtendedData {

    virtual ~ExtendedData() {}

    };

    /**
     * Configuration options for this popup window.
     */
    struct Config
    {
        bool mbAllowEmptySet;
        bool mbRTL;
        Config();
    };

    explicit ScCheckListMenuWindow(vcl::Window* pParent, ScDocument* pDoc);
    virtual ~ScCheckListMenuWindow();
    virtual void dispose() SAL_OVERRIDE;

    virtual void MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual bool Notify(NotifyEvent& rNEvt) SAL_OVERRIDE;
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) SAL_OVERRIDE;
    virtual vcl::Window* GetPreferredKeyInputWindow() SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

    void setMemberSize(size_t n);
    void addDateMember(const OUString& rName, double nVal, bool bVisible);
    void addMember(const OUString& rName, bool bVisible);
    void initMembers();
    void setConfig(const Config& rConfig);

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
    virtual void handlePopupEnd() SAL_OVERRIDE;

private:
    struct Member
    {
        OUString maName; // node name
        OUString maRealName;
        bool            mbVisible;
        bool            mbDate;
        bool            mbLeaf;

        Member();
        SvTreeListEntry* mpParent;
    };

    class CancelButton : public ::CancelButton
    {
    public:
        CancelButton(ScCheckListMenuWindow* pParent);
        virtual ~CancelButton();
        virtual void dispose() SAL_OVERRIDE;

        virtual void Click() SAL_OVERRIDE;

    private:
        VclPtr<ScCheckListMenuWindow> mpParent;
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
        EDIT_SEARCH,          // Search box
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

    DECL_LINK_TYPED( ButtonHdl, Button*, void );
    DECL_LINK_TYPED( TriStateHdl, Button*, void );
    DECL_LINK_TYPED( CheckHdl, SvTreeListBox*, void );
    DECL_LINK( EdModifyHdl, void* );

private:
    VclPtr<Edit>           maEdSearch;
    VclPtr<ScCheckListBox> maChecks;

    VclPtr<TriStateBox>     maChkToggleAll;
    VclPtr<ImageButton>     maBtnSelectSingle;
    VclPtr<ImageButton>     maBtnUnselectSingle;

    VclPtr<OKButton>        maBtnOk;
    VclPtr<CancelButton>    maBtnCancel;

    std::vector<VclPtr<vcl::Window> >          maTabStopCtrls;
    size_t                          mnCurTabStop;

    std::vector<Member>           maMembers;
    std::unique_ptr<ExtendedData> mpExtendedData;
    std::unique_ptr<Action>       mpOKAction;
    std::unique_ptr<Action>       mpPopupEndAction;

    Config maConfig;
    Size maWndSize;  /// whole window size.
    Size maMenuSize; /// size of all menu items combined.
    TriState mePrevToggleAllState;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
