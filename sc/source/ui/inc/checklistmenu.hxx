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

#include <vcl/InterimItemWindow.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <set>

class ScDocument;

class ScCheckListMenuControl;

struct ScCheckListMember;

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
    std::unique_ptr<weld::TreeIter> mxParent;
};

class ScCheckListMenuControl final
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

    struct MenuItemData;

    class SubMenuEntry
    {
    private:
        std::unique_ptr<weld::Builder> mxBuilder;
        std::unique_ptr<weld::Menu> mxSubMenuWin;
        std::vector<MenuItemData> maMenuItems;
        ScCheckListMenuControl* mpParent;

        DECL_LINK(MenuHdl, const OString&, void);
    public:
        SubMenuEntry(weld::Widget* pParent, ScCheckListMenuControl* pControl);
        void addMenuItem(const OUString& rText, Action* pAction);
        void popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect)
        {
            mxSubMenuWin->popup_at_rect(pParent, rRect);
        }
    };

    struct MenuItemData
    {
        OUString maText;
        bool     mbEnabled:1;
        bool     mbSeparator:1;

        std::shared_ptr<Action> mxAction;
        std::unique_ptr<SubMenuEntry> mxSubMenuWin;

        MenuItemData();
    };

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

    explicit ScCheckListMenuControl(DockingWindow* pParent, vcl::Window* pContainer, ScDocument* pDoc, int nWidth = -1);
    ~ScCheckListMenuControl();

#if 0
    virtual void MouseMove(const MouseEvent& rMEvt) override;
    virtual bool EventNotify(NotifyEvent& rNEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
#endif

    void addMenuItem(const OUString& rText, Action* pAction);
    void addSeparator();
    ScCheckListMenuControl::SubMenuEntry& addSubMenuItem(const OUString& rText, bool bEnabled);

    void selectMenuItem(size_t nPos, bool bSubMenuTimer);

    void setMemberSize(size_t n);
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

    void GrabFocus();

    void setOKAction(Action* p);
    void setPopupEndAction(Action* p);

private:

    std::vector<MenuItemData>         maMenuItems;

    void handlePopupEnd();

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
    void updateMemberParents(const weld::TreeIter* pLeaf, size_t nIdx);

    std::unique_ptr<weld::TreeIter> ShowCheckEntry(const OUString& sName, ScCheckListMember& rMember, bool bShow = true, bool bCheck = true);
    void CheckEntry(const OUString& sName, const weld::TreeIter* pParent, bool bCheck);
    void CheckEntry(const weld::TreeIter* pEntry, bool bCheck);
    void GetRecursiveChecked(const weld::TreeIter* pEntry, std::unordered_set<OUString>& vOut, OUString& rLabel);
    std::unordered_set<OUString> GetAllChecked();
    bool IsChecked(const OUString& sName, const weld::TreeIter* pParent);
    int GetCheckedEntryCount() const;
    void CheckAllChildren(const weld::TreeIter* pEntry, bool bCheck);

    void setSelectedMenuItem(size_t nPos, bool bSubMenuTimer, bool bEnsureSubMenu);

    std::unique_ptr<weld::TreeIter> FindEntry(const weld::TreeIter* pParent, const OUString& sNode);

    void clearSelectedMenuItem();
    void executeMenuItem(size_t nPos);

    /**
     * Dismiss all visible popup menus and set focus back to the application
     * window.  This method is called e.g. when a menu action is fired.
     */
    void terminateAllPopupMenus();

    void CreateDropDown();

    DECL_LINK(ButtonHdl, weld::Button&, void);
    DECL_LINK(TriStateHdl, weld::ToggleButton&, void);

    void Check(const weld::TreeIter* pIter);

    DECL_LINK(CheckHdl, const weld::TreeView::iter_col&, void);

    DECL_LINK(EdModifyHdl, weld::Entry&, void);
    DECL_LINK(EdActivateHdl, weld::Entry&, bool);

    DECL_LINK(FocusHdl, weld::Widget&, void);
    DECL_LINK(RowActivatedHdl, weld::TreeView& rMEvt, bool);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(TreeSizeAllocHdl, const Size&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

private:
    VclPtr<DockingWindow> mxParent;
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    std::unique_ptr<weld::TreeView> mxMenu;
    std::unique_ptr<weld::TreeIter> mxScratchIter;
    std::unique_ptr<weld::Entry> mxEdSearch;
    std::unique_ptr<weld::Widget> mxBox;
    std::unique_ptr<weld::TreeView> mxChecks;

    std::unique_ptr<weld::CheckButton> mxChkToggleAll;
    std::unique_ptr<weld::Button> mxBtnSelectSingle;
    std::unique_ptr<weld::Button> mxBtnUnselectSingle;

    std::unique_ptr<weld::Box> mxButtonBox;
    std::unique_ptr<weld::Button> mxBtnOk;
    std::unique_ptr<weld::Button> mxBtnCancel;

    ScopedVclPtr<VirtualDevice> mxDropDown;

    std::vector<ScCheckListMember> maMembers;
    // For Dates
    std::map<OUString, size_t>    maYearMonthMap;

    std::unique_ptr<ExtendedData> mxExtendedData;
    std::unique_ptr<Action>       mxOKAction;
    std::unique_ptr<Action>       mxPopupEndAction;

    Config maConfig;
    int mnWidthHint; /// min width hint
    Size maWndSize;  /// whole window size.
    Size maMenuSize; /// size of all menu items combined.
    TriState mePrevToggleAllState;

    size_t  mnSelectedMenu;

    ScDocument* mpDoc;
};

/**
 * This class implements a popup window for field button, for quick access
 * of hide-item list, and possibly more stuff related to field options.
 */
class ScCheckListMenuWindow : public DockingWindow
{
public:
    explicit ScCheckListMenuWindow(vcl::Window* pParent, ScDocument* pDoc, int nWidth = -1);
    virtual void dispose() override;
    virtual ~ScCheckListMenuWindow() override;

    virtual void GetFocus() override;

    ScCheckListMenuControl& get_widget() { return *mxControl; }

private:
    VclPtr<vcl::Window> mxBox;
    std::unique_ptr<ScCheckListMenuControl> mxControl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
