/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dockwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <unordered_set>
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
    double                   mnValue; // number value of filter condition
    bool                     mbVisible;
    bool                     mbDate;
    bool                     mbLeaf;
    bool                     mbValue; // true if the filter condition is value
    DatePartType             meDatePartType;
    // To store Year and Month if the member if DAY type
    std::vector<OUString>    maDateParts;
    ScCheckListMember();
    std::unique_ptr<weld::TreeIter> mxParent;
};

class ScCheckListMenuWindow;

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
        // return true to dismiss the popup
        virtual bool execute() = 0;
    };

    struct ResultEntry
    {
        OUString aName;
        double nValue; // number value of filter condition
        bool bValid;
        bool bDate;
        bool bValue; // true if the filter condition is value

        bool operator<(const ResultEntry& rhs) const
        {
            return aName < rhs.aName;
        }

        bool operator == (const ResultEntry& rhs) const
        {
            return aName == rhs.aName &&
                   bValid == rhs.bValid &&
                   bDate == rhs.bDate &&
                   bValue == rhs.bValue &&
                   nValue == rhs.nValue;
        }
    };
    typedef std::set<ResultEntry> ResultType;

    struct MenuItemData
    {
        bool     mbEnabled:1;
        std::shared_ptr<Action> mxAction;
        VclPtr<ScCheckListMenuWindow> mxSubMenuWin;

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

    ScCheckListMenuControl(ScCheckListMenuWindow* pParent, vcl::Window* pContainer, ScDocument* pDoc,
                           bool bCanHaveSubMenu, bool bTreeMode, int nWidth);
    ~ScCheckListMenuControl();

    void addMenuItem(const OUString& rText, Action* pAction);
    void addSeparator();
    ScCheckListMenuWindow* addSubMenuItem(const OUString& rText, bool bEnabled);
    void resizeToFitMenuItems();

    void selectMenuItem(size_t nPos, bool bSubMenuTimer);
    void queueLaunchSubMenu(size_t nPos, ScCheckListMenuWindow* pMenu);

    void setMemberSize(size_t n);
    void addDateMember(const OUString& rName, double nVal, bool bVisible);
    void addMember(const OUString& rName, const double nVal, bool bVisible,
                   bool bValue = false);
    size_t initMembers(int nMaxMemberWidth = -1);
    void setConfig(const Config& rConfig);

    bool isAllSelected() const;
    void getResult(ResultType& rResult);
    void launch(const tools::Rectangle& rRect);
    void close(bool bOK);

    void StartPopupMode(const tools::Rectangle& rRect, FloatWinPopupFlags eFlags);
    void EndPopupMode();

    size_t getSubMenuPos(const ScCheckListMenuControl* pSubMenu);
    void setSubMenuFocused(const ScCheckListMenuControl* pSubMenu);
    void queueCloseSubMenu();
    void clearSelectedMenuItem();

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

    int GetTextWidth(const OUString& rsName) const;
    int IncreaseWindowWidthToFitText(int nMaxTextWidth);

    /**
     * Dismiss all visible popup menus and set focus back to the application
     * window.  This method is called e.g. when a menu action is fired.
     */
    void terminateAllPopupMenus();

private:

    std::vector<MenuItemData>         maMenuItems;

    /**
     * Calculate the appropriate window size based on the menu items.
     */
    void prepWindow();
    void setAllMemberState(bool bSet);
    void selectCurrentMemberOnly(bool bSet);
    void updateMemberParents(const weld::TreeIter* pLeaf, size_t nIdx);

    std::unique_ptr<weld::TreeIter> ShowCheckEntry(const OUString& sName, ScCheckListMember& rMember, bool bShow = true, bool bCheck = true);
    void CheckEntry(std::u16string_view sName, const weld::TreeIter* pParent, bool bCheck);
    void CheckEntry(const weld::TreeIter& rEntry, bool bCheck);
    void GetRecursiveChecked(const weld::TreeIter* pEntry, std::unordered_set<OUString>& vOut, OUString& rLabel);
    std::unordered_set<OUString> GetAllChecked();
    bool IsChecked(std::u16string_view sName, const weld::TreeIter* pParent);
    int GetCheckedEntryCount() const;
    void CheckAllChildren(const weld::TreeIter& rEntry, bool bCheck);

    void setSelectedMenuItem(size_t nPos, bool bSubMenuTimer);

    std::unique_ptr<weld::TreeIter> FindEntry(const weld::TreeIter* pParent, std::u16string_view sNode);

    void executeMenuItem(size_t nPos);

    void endSubMenu(ScCheckListMenuControl& rSubMenu);

    struct SubMenuItemData;

    void handleMenuTimeout(const SubMenuItemData* pTimer);

    void launchSubMenu(bool bSetMenuPos);

    void CreateDropDown();

    void NotifyCloseLOK();

    DECL_LINK(ButtonHdl, weld::Button&, void);
    DECL_LINK(TriStateHdl, weld::Toggleable&, void);

    void Check(const weld::TreeIter* pIter);

    DECL_LINK(CheckHdl, const weld::TreeView::iter_col&, void);

    DECL_LINK(PopupModeEndHdl, FloatingWindow*, void);

    DECL_LINK(EdModifyHdl, weld::Entry&, void);
    DECL_LINK(EdActivateHdl, weld::Entry&, bool);

    DECL_LINK(FocusHdl, weld::Widget&, void);
    DECL_LINK(RowActivatedHdl, weld::TreeView& rMEvt, bool);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(TreeSizeAllocHdl, const Size&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(MenuKeyInputHdl, const KeyEvent&, bool);

    DECL_LINK(PostPopdownHdl, void*, void);

private:
    VclPtr<ScCheckListMenuWindow> mxFrame;
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Container> mxContainer;
    std::unique_ptr<weld::TreeView> mxMenu;
    std::unique_ptr<weld::TreeIter> mxScratchIter;
    std::unique_ptr<weld::Entry> mxEdSearch;
    std::unique_ptr<weld::Widget> mxBox;
    std::unique_ptr<weld::TreeView> mxListChecks;
    std::unique_ptr<weld::TreeView> mxTreeChecks;
    weld::TreeView* mpChecks;

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
    int mnCheckWidthReq; /// matching width request for mxChecks
    int mnWndWidth;  /// whole window width.
    TriState mePrevToggleAllState;

    size_t  mnSelectedMenu;

    ScDocument* mpDoc;

    ImplSVEvent* mnAsyncPostPopdownId;

    bool mbHasDates;
    bool mbCanHaveSubMenu;

    struct SubMenuItemData
    {
        Timer                   maTimer;
        VclPtr<ScCheckListMenuWindow>   mpSubMenu;
        size_t                  mnMenuPos;

        DECL_LINK( TimeoutHdl, Timer*, void );

        SubMenuItemData(ScCheckListMenuControl* pParent);
        void reset();

    private:
        ScCheckListMenuControl* mpParent;
    };

    SubMenuItemData   maOpenTimer;
    SubMenuItemData   maCloseTimer;
};

/**
 * This class implements a popup window for the auto filter dropdown.
 */
class ScCheckListMenuWindow : public DropdownDockingWindow
{
public:
    explicit ScCheckListMenuWindow(vcl::Window* pParent, ScDocument* pDoc,
                                   bool bCanHaveSubMenu, bool bTreeMode, int nWidth = -1,
                                   ScCheckListMenuWindow* pParentMenu = nullptr,
                                   const vcl::ILibreOfficeKitNotifier* pNotifier = nullptr);
    virtual void dispose() override;
    virtual ~ScCheckListMenuWindow() override;

    virtual void GetFocus() override;
    virtual bool EventNotify(NotifyEvent& rNEvt) override;

    ScCheckListMenuWindow* GetParentMenu() { return mxParentMenu; }
    ScCheckListMenuControl& get_widget() { return *mxControl; }

private:
    VclPtr<ScCheckListMenuWindow> mxParentMenu;
    std::unique_ptr<ScCheckListMenuControl, o3tl::default_delete<ScCheckListMenuControl>> mxControl;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
