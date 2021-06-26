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

    //Handles drop of content in treeview when creating a new style
    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

    //This is the first sub part of UpdateStyles_Impl in Dialog.
    //It is used whenever a family like Paragraph, Character, List etc. is selected.
    //It updates the Current Documents's pStyleSheetPool and further operates on it.
    void UpdateFamily_Impl();

    //Part of dialog's Read Resource
    //Used to read values of certain variables like m_pCurObjectShell(current object shell),
    //m_xStyleFamilies(It consists of 6 families(Paragarph, Character etc.)) etc.
    size_t ReadResource();

    //Returns size of m_xStyleFamilies
    //Used in Dialog's SetWaterCanState
    size_t mxStyleFamilySize() { return m_xStyleFamilies->size(); }

    //Part of impl_clear of Dialog
    //Resets parameters of StyleList
    void stylist_impl_clear();

    /**
    * Is it safe to show the water-can / fill icon. If we've a
    * hierarchical widget - we have only single select, otherwise
    * we need to check if we have a multi-selection. We either have
    * a m_aStyleList.mxTreeBox showing or an m_aStyleList.mxFmtLb (which we hide when not shown)
    */
    bool IsSafeForWaterCan() const;

    //This is a part of Dilog's Select Style
    // It selects the style in treeview
    void SelectStyle(const OUString& rStr, bool bIsCallback);

    //Selects the style at the passed index of m_pFamilyState
    std::unique_ptr<SfxTemplateItem>* SelectStyleByIndex(sal_uInt16 idx)
    {
        return &m_pFamilyState[idx];
    }

    // Used to get the current selected entry in treeview
    OUString GetSelectedEntry() const;

    // Helper function: Access to the current family item
    const SfxStyleFamilyItem* GetFamilyItem_Impl() const;

    bool EnableExecute_impl()
    {
        return m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1;
    }

    //This is the second subpart of Dialog's UpdateFamily_Impl
    //Used when a family is selected
    //Filling of treeview is called here after bTreeDrag is enabled
    void UpdateFamImpl();

    void DisableExample(bool m_newByExampleDisabled)
    {
        m_bNewByExampleDisabled = m_newByExampleDisabled;
    }
    // Id in the ToolBox = Position - 1
    void CurToolBoxId(sal_uInt16 m_ActFam) { m_nActFamily = m_ActFam; }

    //Enable/Disable Preview?
    void Preview_Hdl(bool bCustomPreview);

    // Part of Action select in Dialog
    // When new or new menu is selected, it comes into action
    void ExecuteAction();

    //Part of Initialize in Dialog
    //Connects m_xTreeBox and m_xFmtLb with certain LINKs
    void Initialize();

    bool HasSelectedStyle() const;

    //Set Hierarchical view?
    void SetHierarchical();

    //Set a Filter Non-Hierarchical?
    void SetFilter();

    //Comes into action when the current style is changed
    void UpdateStyles_Impl(SfxStyleFamily eFam, SfxStyleSearchBits nFilter);

    //Comes into action when the current style is changed
    const SfxStyleFamilyItem* UpdateStyles();

    //Enable Hierarchical view by default when treebox is visible
    bool DefaultEnableHierarchy() { return m_xTreeBox->get_visible(); }

    //UpdateFamily?
    void BUpdateFamily(bool m_updatefamily) { m_bUpdateFamily = m_updatefamily; }

    void EnableDelete();

    void TimeOut_Impl();

    void Notify_Impl(SfxHintId nId);

    bool DontUpdate() { return m_bDontUpdate; }

    void FilterSelect_Impl(sal_uInt16 m_nActFilter);

    void EnableTreeDrag_Impl(bool m_bEnable);

    void SetFilterIdx(sal_uInt16 filter) { m_nActFilter = filter; }

    SfxStyleFamily GetFamilyByIndex(size_t i) { return m_xStyleFamilies->at(i).GetFamily(); }

    SfxStyleFamilyItem& GetFamilyItemByIndex(size_t i) { return m_xStyleFamilies->at(i); }

    void EnableDel(bool candel) { m_bCanDel = candel; }
    void Enablehide(bool canhide) { m_bCanHide = canhide; }
    void Enableshow(bool canshow) { m_bCanShow = canshow; }
    void Enablenew(bool cannew) { m_bCanNew = cannew; }
    void Enableedit(bool canedit) { m_bCanEdit = canedit; }

    void Cleanup();

    void SetFamilyState_Impl(sal_uInt16 nSlotId, const SfxTemplateItem* pItem);

    void UpdateStyleDependents_Impl();

private:
    void FillTreeBox(SfxStyleFamily eFam);
    OUString getDefaultStyleName(const SfxStyleFamily eFam);
    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);
    SfxStyleFamily GetActualFamily() const;
    void DropHdl(const OUString& rStyle, const OUString& rParent);

    void MenuSelect(const OString& rIdent);
    void PrepareMenu(const Point& rPos);
    void GetSelectedStyle() const;

    // In which FamilyState do I have to look, in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16 StyleNrToInfoOffset(sal_uInt16 i);

    bool IsInitialized() const { return m_nActFamily != 0xffff; }

    void Update_Impl();

    //Getter for the shell
    SfxObjectShell* GetObjectShell() const { return m_pCurObjShell; }

    sal_Int32 LoadFactoryStyleFilter(SfxObjectShell const* i_pObjSh);

    void NewHdl();
    void EditHdl();
    void DeleteHdl();
    void HideHdl();
    void ShowHdl();

    void ShowMenu(const CommandEvent& rCEvt);
    void CreateContextMenu();

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
