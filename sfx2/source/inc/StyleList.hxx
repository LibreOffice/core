/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

class SfxObjectShell;
class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxCommonTemplateDialog_Impl;

enum class StyleFlags
{
    NONE = 0,
    UpdateFamilyList = 1,
    UpdateFamily = 2
};

namespace o3tl
{
template <> struct typed_flags<StyleFlags> : is_typed_flags<StyleFlags, 3>
{
};
}

class TreeViewDropTarget;

constexpr int MAX_FAMILIES = 6;
constexpr int COUNT_BOUND_FUNC = 14;

class StyleList
{
    friend class TreeViewDropTarget;
    friend class SfxTemplateControllerItem;

public:
    //Constructor
    StyleList(weld::Builder* pBuilder, std::optional<SfxStyleFamilies> mxFamilies,
              SfxBindings* pBindings, SfxCommonTemplateDialog_Impl* m_Parent, SfxModule* m_Module,
              weld::Container* pC);

    //Destructor
    ~StyleList();

    //This function connects m_xTreeBox, m_xFmtLb and m_pParentDialog with certain LINKs
    //It is the entry point of the program
    void Initialize();

    // It selects the style in treeview
    //bIsCallBack is true for the selected style. For eg. if "Addressee" is selected in paragraph Styles, bIsCallBack will be true for it.
    void SelectStyle(const OUString& rStr, bool bIsCallback);
    //Selects the style at the passed index of m_pFamilyState
    std::unique_ptr<SfxTemplateItem>* SelectStyleByIndex(sal_uInt16 idx)
    {
        return &m_pFamilyState[idx];
    }

    //This function returns the value of the parameter m_bDontUpdate
    //It is used in Dialog's Timeout and Notify
    bool DontUpdate() { return m_bDontUpdate; }
    //Dialog and StyleList have their own copies of variable m_upDateFamily.
    //It is used in Dialog's SetFamily and SetFamilyState
    //When the value of bUpdateFamily is updated there while setting a family, the corresponding value gets updated here too.
    void SetUpdateFamily(bool m_updatefamily) { m_bUpdateFamily = m_updatefamily; }
    // This function is a subpart of Dialog's SetFamilyState
    //When a new style is selected for use, it resets it.
    void SetFamilyState(sal_uInt16 nSlotId, const SfxTemplateItem* pItem);

    //Dialog and StyleList have their own copies of variable m_bNewByExampleDisabled.
    //It is used in Dialog's EnableExample_Impl
    //When the value of m_bNewbyExampleDisabled is updated there while cresting a new style by example, the corresponding value gets updated here too.
    void DisableExample(bool m_newByExampleDisabled)
    {
        m_bNewByExampleDisabled = m_newByExampleDisabled;
    }
    // m_nActFamily is : Id in the ToolBox = Position - 1
    // Dialog and StyleList have their own copies of m_nActFamily
    //When this value gets updated in Dialog's FamilySelect and SetFamily, the corresponding value gets updated here too.
    void SetStyleFamily(sal_uInt16 m_ActFam) { m_nActFamily = m_ActFam; }

    //This function is used to set a hierarchical view.
    void SetHierarchical();
    //This function handles the controls while setting a filter except heirarchical
    void SetFilterControlsHandle();
    //Return whether treeview is visible
    //It is used in StyleList's UpdateStyles_Hdl
    //It is used to defaultly set the heirarchical view
    bool IsTreeView() { return m_xTreeBox->get_visible(); }

    // Dialog and StyleList have their own copies of m_nActFilter
    //When this value gets updated in Dialog's UpdateStyles_Hdl while setting a filter, the corresponding value gets updated here too.
    void SetFilterIdx(sal_uInt16 filter) { m_nActFilter = filter; }

    //Returns size of m_xStyleFamilies
    //Used in Dialog's SetWaterCanState
    size_t NumberOfStyleFamily() { return m_xStyleFamilies->size(); }
    // Helper function: Access to the current family item
    //Used in Dialog's updateStyleHandler, Execute_Impl etc...
    const SfxStyleFamilyItem* GetFamilyItem() const;
    // Used to get the current selected entry in treeview
    //Used in Dialog's Execute_Impl, Action_Select etc...
    OUString GetSelectedEntry() const;
    //Returns the Family at ith index
    //Used in Dialog's UpdateStyles_Hdl, Execute_Impl
    SfxStyleFamily GetFamilyByIndex(size_t i) { return m_xStyleFamilies->at(i).GetFamily(); }
    //Returns the Family Item at ith index
    //Used in Dialog's ReadResource_Hdl
    SfxStyleFamilyItem& GetFamilyItemByIndex(size_t i) { return m_xStyleFamilies->at(i); }

    void Enabledel(bool candel) { m_bCanDel = candel; }
    void Enablehide(bool canhide) { m_bCanHide = canhide; }
    void Enableshow(bool canshow) { m_bCanShow = canshow; }
    void Enablenew(bool cannew) { m_bCanNew = cannew; }
    void Enableedit(bool canedit) { m_bCanEdit = canedit; }
    //Handles the enabling/Disabling of Preview
    void EnablePreview(bool bCustomPreview);
    //Used in Dialog's Execute_Impl
    //It is a necessary condition to execute a style
    bool EnableExecute()
    {
        return m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1;
    }

    void connect_UpdateStyles(const Link<StyleFlags, void>& rLink) { m_aUpdateStyles = rLink; }
    void connect_ReadResource(const Link<void*, void>& rLink) { m_aReadResource = rLink; }
    void connect_ClearResource(const Link<void*, void>& rLink) { m_aClearResource = rLink; }
    void connect_LoadFactoryStyleFilter(const Link<SfxObjectShell const*, sal_Int32>& rLink)
    {
        m_aLoadFactoryStyleFilter = rLink;
    }
    void connect_SaveSelection(const Link<void*, SfxObjectShell*> rLink)
    {
        m_aSaveSelection = rLink;
    }
    void connect_UpdateStyleDependents(const Link<void*, void> rLink)
    {
        m_aUpdateStyleDependents = rLink;
    }
    void connect_UpdateFamily(const Link<void*, void> rLink) { m_aUpdateFamily = rLink; }

private:
    void FillTreeBox(SfxStyleFamily eFam);

    OUString getDefaultStyleName(const SfxStyleFamily eFam);
    SfxStyleFamily GetActualFamily() const;
    void GetSelectedStyle() const;

    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);
    void DropHdl(const OUString& rStyle, const OUString& rParent);

    void MenuSelect(const OString& rIdent);
    void PrepareMenu(const Point& rPos);
    void ShowMenu(const CommandEvent& rCEvt);
    void CreateContextMenu();

    // In which FamilyState do I have to look, in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16 StyleNrToInfoOffset(sal_uInt16 i);

    DECL_LINK(ReadResource, void*, size_t);
    DECL_LINK(Clear, void*, void);
    DECL_LINK(Cleanup, void*, void);
    DECL_LINK(ExecuteDrop, const ExecuteDropEvent&, sal_Int8);
    DECL_LINK(NewMenuExecuteAction, void*, void);
    DECL_LINK(IsSafeForWaterCan, void*, bool);
    DECL_LINK(HasSelectedStyle, void*, bool);
    DECL_LINK(UpdateStyles, StyleFlags, void);
    DECL_LINK(UpdateFamily, void*, void);
    DECL_LINK(UpdateStyleDependents, void*, void);
    DECL_LINK(TimeOut, void*, void);
    DECL_LINK(Notify, SfxHintId, void);
    DECL_LINK(EnableTreeDrag, bool, void);
    DECL_LINK(EnableDelete, void*, void);
    DECL_LINK(FilterSelect, sal_uInt16, void);

    void Update();
    Link<StyleFlags, void> m_aUpdateStyles;
    Link<void*, void> m_aReadResource;
    Link<void*, void> m_aClearResource;
    Link<SfxObjectShell const*, sal_Int32> m_aLoadFactoryStyleFilter;
    Link<void*, SfxObjectShell*> m_aSaveSelection;
    Link<void*, void> m_aUpdateStyleDependents;
    Link<void*, void> m_aUpdateFamily;

    void NewHdl();
    void EditHdl();
    void DeleteHdl();
    void HideHdl();
    void ShowHdl();

    DECL_LINK(DragBeginHdl, bool&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);
    DECL_LINK(CustomRenderHdl, weld::TreeView::render_args, void);
    DECL_LINK(FmtSelectHdl, weld::TreeView&, void);
    DECL_LINK(TreeListApplyHdl, weld::TreeView&, bool);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_STATIC_LINK(StyleList, CustomGetSizeHdl, weld::TreeView::get_size_args, Size);
    DECL_LINK(PopupFlatMenuHdl, const CommandEvent&, bool);
    DECL_LINK(PopupTreeMenuHdl, const CommandEvent&, bool);
    DECL_LINK(MenuSelectAsyncHdl, void*, void);

    bool m_bHierarchical : 1;

    bool m_bAllowReParentDrop : 1;
    bool m_bNewByExampleDisabled : 1;
    bool m_bDontUpdate : 1;
    bool m_bTreeDrag : 1;
    bool m_bCanEdit : 1;
    bool m_bCanHide : 1;
    bool m_bCanShow : 1;
    bool m_bCanNew : 1;
    bool m_bUpdateFamily : 1;
    bool m_bCanDel : 1;
    bool m_bBindingUpdate : 1;
    SfxStyleSheetBasePool* m_pStyleSheetPool;
    sal_uInt16 m_nActFilter;
    std::unique_ptr<weld::TreeView> m_xFmtLb;
    std::unique_ptr<weld::TreeView> m_xTreeBox;

    std::unique_ptr<weld::Builder> mxMenuBuilder;
    std::unique_ptr<weld::Menu> mxMenu;

    std::optional<SfxStyleFamilies> m_xStyleFamilies;
    std::array<std::unique_ptr<SfxTemplateItem>, MAX_FAMILIES> m_pFamilyState;
    SfxObjectShell* m_pCurObjShell;
    sal_uInt16 m_nActFamily;
    SfxStyleSearchBits m_nAppFilter;

    std::unique_ptr<TreeViewDropTarget> m_xTreeView1DropTargetHelper;
    std::unique_ptr<TreeViewDropTarget> m_xTreeView2DropTargetHelper;

    SfxCommonTemplateDialog_Impl* m_pParentDialog;
    SfxBindings* m_pBindings;

    OString sLastItemIdent;
    SfxModule* m_Module;
    sal_uInt16 m_nModifier;
    weld::Container* m_pContainer;
};
