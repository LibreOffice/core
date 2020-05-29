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
#include <vcl/timer.hxx>
#include <vcl/svlbitm.hxx>

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <set>

namespace com::sun::star {
    namespace accessibility {
        class XAccessible;
    }
}

class ScDocument;
class ScAccessibleFilterMenu;

class ScMenuFloatingWindow : public PopupMenuFloatingWindow
{
public:
    static constexpr size_t MENU_NOT_SELECTED = 999;

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
    virtual ~ScMenuFloatingWindow() override;
     void dispose() override;

    virtual void PopupModeEnd() override;
    virtual void MouseMove(const MouseEvent& rMEvt) override;
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void KeyInput(const KeyEvent& rKEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    void addMenuItem(const OUString& rText, Action* pAction);
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
    void handleMenuTimeout(const SubMenuItemData* pTimer);

    void resizeToFitMenuItems();
    void highlightMenuItem(vcl::RenderContext& rRenderContext, size_t nPos, bool bSelected);

    size_t getEnclosingMenuItem(const Point& rPos) const;
    size_t getSubMenuPos(const ScMenuFloatingWindow* pSubMenu);

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
    void setSubMenuFocused(const ScMenuFloatingWindow* pSubMenu);

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

        DECL_LINK( TimeoutHdl, Timer*, void );

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

class ScCheckListMenuWindow;

template <class T> struct VclPtr_hash;
template <> struct VclPtr_hash< VclPtr<vcl::Window> >
{
    size_t operator()( const VclPtr<vcl::Window>& r ) const
    {
        return reinterpret_cast<size_t>(r.get());
    }
};

class ScTabStops
{
private:
    typedef std::unordered_map< VclPtr<vcl::Window>, size_t, VclPtr_hash<VclPtr<vcl::Window>> > ControlToPosMap;
    VclPtr<ScCheckListMenuWindow> mpMenuWindow;
    ControlToPosMap maControlToPos;
    std::vector<VclPtr<vcl::Window>> maControls;
    size_t mnCurTabStop;
public:
    ScTabStops( ScCheckListMenuWindow* mpMenuWin );
    ~ScTabStops();
    void AddTabStop( vcl::Window* pWin );
    void SetTabStop( vcl::Window* pWin );
    void CycleFocus( bool bReverse = false );
    void clear();
};

struct ScCheckListMember;

class ScCheckListBox : public SvTreeListBox
{
    std::unique_ptr<SvLBoxButtonData> mpCheckButton;
    ScTabStops*         mpTabStops;
    bool                mbSeenMouseButtonDown;
    void            CountCheckedEntries( SvTreeListEntry* pParent, sal_uLong& nCount ) const;
    void            CheckAllChildren( SvTreeListEntry* pEntry, bool bCheck );

    public:

    ScCheckListBox( vcl::Window* pParent );
    virtual ~ScCheckListBox() override { disposeOnce(); }
    virtual void dispose() override { mpCheckButton.reset(); SvTreeListBox::dispose(); }
    void Init();
    void CheckEntry( const OUString& sName, SvTreeListEntry* pParent, bool bCheck );
    void CheckEntry( SvTreeListEntry* pEntry, bool bCheck );
    SvTreeListEntry* ShowCheckEntry( const OUString& sName, ScCheckListMember& rMember, bool bShow = true, bool bCheck = true );
    void GetRecursiveChecked( SvTreeListEntry* pEntry, std::unordered_set<OUString>& vOut, OUString& rLabel );
    std::unordered_set<OUString> GetAllChecked();
    bool IsChecked( const OUString& sName, SvTreeListEntry* pParent );
    SvTreeListEntry* FindEntry( SvTreeListEntry* pParent, const OUString& sNode );
    sal_uInt16 GetCheckedEntryCount() const;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
    void SetTabStopsContainer( ScTabStops* pTabStops ) { mpTabStops = pTabStops; }
};

class ScSearchEdit : public Edit
{
private:
    ScTabStops*         mpTabStops;
public:
    ScSearchEdit(Window* pParent)
        : Edit(pParent)
        , mpTabStops(nullptr)
    {
        set_id("search_edit");
    }

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    void SetTabStopsContainer( ScTabStops* pTabStops )  { mpTabStops = pTabStops; }
};

struct ScCheckListMember
{
    enum DatePartType
    {
        YEAR,
        MONTH,
        DAY,
    };

    OUString                 maName; // node name
    OUString                 maRealName;
    bool                     mbVisible;
    bool                     mbDate;
    bool                     mbLeaf;
    DatePartType             meDatePartType;
    // To store Year and Month if the member if DAY type
    std::vector<OUString>    maDateParts;
    ScCheckListMember();
    SvTreeListEntry* mpParent;
};

/**
 * This class implements a popup window for field button, for quick access
 * of hide-item list, and possibly more stuff related to field options.
 */
class ScCheckListMenuWindow : public ScMenuFloatingWindow
{
public:
    struct ResultEntry
    {
        OUString aName;
        bool bValid;
        bool bDate;

        bool operator<(const ResultEntry& rhs) const
        {
            return aName < rhs.aName;
        }

        bool operator == (const ResultEntry& rhs) const
        {
            return aName == rhs.aName &&
                   bValid == rhs.bValid &&
                   bDate == rhs.bDate;
        }
    };
    typedef std::set<ResultEntry> ResultType;

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

    explicit ScCheckListMenuWindow(vcl::Window* pParent, ScDocument* pDoc, int nWidth = -1);
    virtual ~ScCheckListMenuWindow() override;
    virtual void dispose() override;

    virtual void MouseMove(const MouseEvent& rMEvt) override;
    virtual bool EventNotify(NotifyEvent& rNEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    void setMemberSize(size_t n);
    void setHasDates(bool bHasDates);
    void addDateMember(const OUString& rName, double nVal, bool bVisible);
    void addMember(const OUString& rName, bool bVisible);
    size_t initMembers();
    void setConfig(const Config& rConfig);

    bool isAllSelected() const;
    void getResult(ResultType& rResult);
    void launch(const tools::Rectangle& rRect);
    void close(bool bOK);

    /**
     * Set auxiliary data that the client code might need.  Note that this
     * popup window class manages its life time; no explicit deletion of the
     * instance is needed in the client code.
     */
    void setExtendedData(std::unique_ptr<ExtendedData> p);

    /**
     * Get the store auxiliary data, or NULL if no such data is stored.
     */
    ExtendedData* getExtendedData();

    void setOKAction(Action* p);
    void setPopupEndAction(Action* p);

protected:
    virtual void handlePopupEnd() override;

private:

    class CancelButton : public ::CancelButton
    {
    public:
        CancelButton(ScCheckListMenuWindow* pParent);
        virtual ~CancelButton() override;
        virtual void dispose() override;

        virtual void Click() override;

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
    void updateMemberParents( const SvTreeListEntry* pLeaf, size_t nIdx );

    DECL_LINK( ButtonHdl, Button*, void );
    DECL_LINK( TriStateHdl, Button*, void );
    DECL_LINK( CheckHdl, SvTreeListBox*, void );
    DECL_LINK( EdModifyHdl, Edit&, void );

private:
    VclPtr<ScSearchEdit>   maEdSearch;
    VclPtr<ScCheckListBox> maChecks;

    VclPtr<CheckBox>        maChkToggleAll;
    VclPtr<ImageButton>     maBtnSelectSingle;
    VclPtr<ImageButton>     maBtnUnselectSingle;

    VclPtr<OKButton>        maBtnOk;
    VclPtr<CancelButton>    maBtnCancel;

    std::vector<ScCheckListMember> maMembers;
    // For Dates
    std::map<OUString, size_t>    maYearMonthMap;

    std::unique_ptr<ExtendedData> mpExtendedData;
    std::unique_ptr<Action>       mpOKAction;
    std::unique_ptr<Action>       mpPopupEndAction;

    Config maConfig;
    Size maWndSize;  /// whole window size.
    Size maMenuSize; /// size of all menu items combined.
    TriState mePrevToggleAllState;
    ScTabStops maTabStops;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
