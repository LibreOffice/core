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
    StyleList(weld::Builder* pBuilder, std::optional<SfxStyleFamilies> mxFamilies,
              SfxBindings* pBindings, SfxCommonTemplateDialog_Impl* m_Parent, SfxModule* m_Module,
              weld::Container* pC);

    ~StyleList();

    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

    void SetObjectShell(SfxObjectShell* shell) { m_pCurObjShell = shell; }
    void UpdateFamily_Impl();

    void MenuSelect(const OString& rIdent);
    //Get number of rows in m_xFmtLb
    int getSelectedRowsM_xFmtlb() { return m_xFmtLb->count_selected_rows(); }

    void PrepareMenu(const Point& rPos);

    //Part of dialog's Read Resource
    void ReadResource();

    //Size ofm_xStyleFamilies
    size_t getM_xStyleFamilySize() { return m_xStyleFamilies->size(); }

    //Part of impl_clear of Dialog
    void stylist_impl_clear()
    {
        m_xStyleFamilies.reset();
        for (auto& i : m_pFamilyState)
            i.reset();
    }
    bool IsSafeForWaterCan() const;
    void SelectStyle(const OUString& rStr, bool bIsCallback);

    //Get Nth element of pFamilyState
    std::unique_ptr<SfxTemplateItem>* NthElem(sal_uInt16 idx)
    {
        if (m_pFamilyState[idx])
            return &m_pFamilyState[idx];
        return nullptr;
    }
    void GetSelectedStyle() const;
    OUString GetSelectedEntry() const;
    const SfxStyleFamilyItem* GetFamilyItem_Impl() const;

    bool
    bCanNewGetter_updateFamImpl() // This value is used in many other places other than being the getter.
    {
        return m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1;
    }

    bool bTreeDragSetter_updateFam_Impl() { return true; }
    //Part of Update Family Impl
    void UpdateFamImpl();

    //setter for m_bNewByExampleDisabled
    void bNewByExampleDisabledsetter(bool m_newByExampleDisabled)
    {
        m_bNewByExampleDisabled = m_newByExampleDisabled;
    }
    //setter for m_nActFamily
    void nActFamilySetter(bool m_ActFam) { m_nActFamily = m_ActFam; }

    DECL_LINK(MenuSelectAsyncHdl, void*, void);

    void SetRIndent(OString m_LastItemIndent) { sLastItemIdent = m_LastItemIndent; }

    //Part of Dialog's Preview Hdl
    void for_preview_hdl(bool bCustomPreview)
    {
        m_xFmtLb->clear();
        m_xFmtLb->set_column_custom_renderer(0, bCustomPreview);
        m_xTreeBox->clear();
        m_xTreeBox->set_column_custom_renderer(0, bCustomPreview);
    }

    // Part of Action select in Dialog
    void ExecuteAction();

    //Part of Initialize in Dialog
    void Initialize();

    //Setter of variable m_bHierarchical
    void SetbHierarchical(bool bHierarchical) { m_bHierarchical = bHierarchical; }

    bool HasSelectedStyle() const;

    //Part of Enable Hierarchical
    void SetHierarchical();
    //Part of Enable Hierarchical
    void SetFilter();

    //Part of UpdateStyles_Impl
    void UpdateStyles_Impl(SfxStyleFamily eFam, SfxStyleSearchBits nFilter);

    //Setter for variable m_nAppFilter
    void NAppFilter(SfxStyleSearchBits filter) { m_nAppFilter = filter; }

    //Returns whether M_XtreeBox is visible or not
    bool M_XtreeBoxVisible() { return m_xTreeBox->get_visible(); }

    // In which FamilyState do I have to look, in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16 StyleNrToInfoOffset(sal_uInt16 i);

    void Update_Impl();

    //Setter for variable bUpdateFamily
    void BUpdateFamily(bool m_updatefamily) { m_bUpdateFamily = m_updatefamily; }

    void EnableDelete();

    void TimeOut_Impl();

    void Notify_Impl(SfxHintId nId);

    //Getter for the variable bDontUpdate
    bool bDontUpdateGetter() { return m_bDontUpdate; }

    //Part of filter select in dialog
    void FilterSelect_Impl();

    //Used for OSL Insurance in update styles
    void OSL_ensurance_UpdateStyles() { OSL_ENSURE(m_pStyleSheetPool, "no StyleSheetPool"); }

    //is m_pStyleSheetPool null?
    bool null_ptr_m_pStylesheetpool()
    {
        if (!m_pStyleSheetPool)
            return true;
        return false;
    }

    //Setter for variable bTreeDrag
    void BTreeDrag(bool treedrag) { m_bTreeDrag = treedrag; }

    void EnableTreeDrag_Impl(bool m_bEnable)
    {
        if (m_pStyleSheetPool)
        {
            const SfxStyleFamilyItem* pItem = GetFamilyItem_Impl();
            SfxStyleSheetBase* pStyle
                = pItem ? m_pStyleSheetPool->First(pItem->GetFamily()) : nullptr;
            m_bAllowReParentDrop = pStyle && pStyle->HasParentSupport() && m_bEnable;
        }
    }

    //Setter for variable m_nActFilter
    void NActFilter(sal_uInt16 filter) { m_nActFilter = filter; }

    //Used in ReadResource
    SfxStyleFamily ReturnFamforReadRes(size_t i) { return m_xStyleFamilies->at(i).GetFamily(); }

    SfxStyleFamilyItem& GetFamilyItem(size_t i) { return m_xStyleFamilies->at(i); }

private:
    void FillTreeBox(SfxStyleFamily eFam);
    OUString getDefaultStyleName(const SfxStyleFamily eFam);
    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);
    SfxStyleFamily GetActualFamily() const;
    void DropHdl(const OUString& rStyle, const OUString& rParent);

    //Getter for the shell
    SfxObjectShell* GetObjectShell() const { return m_pCurObjShell; }

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

    std::array<std::unique_ptr<SfxTemplateControllerItem>, COUNT_BOUND_FUNC> pBoundItems;

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
    std::unique_ptr<weld::ComboBox> m_xFilterLb;
    std::unique_ptr<weld::CheckButton> m_xPreviewCheckbox;
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

    void ShowMenu(const CommandEvent& rCEvt);
    void CreateContextMenu();
};
