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

    //Part of Initialize in Dialog
    //Connects m_xTreeBox and m_xFmtLb with certain LINKs
    void Initialize();

    //This is a part of Dilog's Select Style
    // It selects the style in treeview
    //bIsCallBack is true for the selected style. For eg. if "Addressee" is selected in paragraph Styles, bIsCallBack will be true for it.
    void SelectStyle(const OUString& rStr, bool bIsCallback);
    //Selects the style at the passed index of m_pFamilyState
    std::unique_ptr<SfxTemplateItem>* SelectStyleByIndex(sal_uInt16 idx)
    {
        return &m_pFamilyState[idx];
    }

    bool DontUpdate() { return m_bDontUpdate; }
    //UpdateFamily?
    void SetUpdateFamily(bool m_updatefamily) { m_bUpdateFamily = m_updatefamily; }
    void SetFamilyState(sal_uInt16 nSlotId, const SfxTemplateItem* pItem);

    //Create new Styles by example?
    void DisableExample(bool m_newByExampleDisabled)
    {
        m_bNewByExampleDisabled = m_newByExampleDisabled;
    }
    // Id in the ToolBox = Position - 1
    void SetStyleFamily(sal_uInt16 m_ActFam) { m_nActFamily = m_ActFam; }

    //Set Hierarchical view?
    void SetHierarchical();
    //Set a Filter Non-Hierarchical?
    void SetFilterControlsHandle();
    //Enable Hierarchical view by default when treebox is visible
    bool IsTreeView() { return m_xTreeBox->get_visible(); }

    void SetFilterIdx(sal_uInt16 filter) { m_nActFilter = filter; }

    //Returns size of m_xStyleFamilies
    //Used in Dialog's SetWaterCanState
    size_t mxStyleFamilySize() { return m_xStyleFamilies->size(); }
    // Helper function: Access to the current family item
    const SfxStyleFamilyItem* GetFamilyItem() const;
    // Used to get the current selected entry in treeview
    OUString GetSelectedEntry() const;
    SfxStyleFamily GetFamilyByIndex(size_t i) { return m_xStyleFamilies->at(i).GetFamily(); }
    SfxStyleFamilyItem& GetFamilyItemByIndex(size_t i) { return m_xStyleFamilies->at(i); }

    void Enabledel(bool candel) { m_bCanDel = candel; }
    void Enablehide(bool canhide) { m_bCanHide = canhide; }
    void Enableshow(bool canshow) { m_bCanShow = canshow; }
    void Enablenew(bool cannew) { m_bCanNew = cannew; }
    void Enableedit(bool canedit) { m_bCanEdit = canedit; }
    //Enable/Disable Preview?
    void EnablePreview(bool bCustomPreview);
    bool EnableExecute()
    {
        return m_xTreeBox->get_visible() || m_xFmtLb->count_selected_rows() <= 1;
    }

    void connect_update_styles(const Link<StyleFlags, void>& rLink) { m_aUpdateStyles = rLink; }
    void connect_read_resource(const Link<void*, void>& rLink) { m_aReadResource = rLink; }
    void connect_clear_resource(const Link<void*, void>& rLink) { m_aClearResource = rLink; }
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
    void connect_UpdateFamilyImpl(const Link<void*, void> rLink) { m_aUpdateFamily_Impl = rLink; }

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

    //Part of dialog's Read Resource
    //Used to read values of certain variables like m_pCurObjectShell(current object shell),
    //m_xStyleFamilies(It consists of 6 families(Paragarph, Character etc.)) etc.
    DECL_LINK(ReadResource, void*, size_t);
    //Part of impl_clear of Dialog
    //Resets parameters of StyleList
    DECL_LINK(StyleListClear_Hdl, void*, void);
    //Called in the destructor of Dialog
    //Cleans up the StyleList individual components while closing the application
    DECL_LINK(Cleanup_Hdl, void*, void);

    //Handles drop of content in treeview when creating a new style
    DECL_LINK(ExecuteDrop_Hdl, const ExecuteDropEvent&, sal_Int8);
    // Part of Action select in Dialog
    // When new or new menu is selected, it comes into action
    DECL_LINK(NewMenu_ExecuteActionHdl, void*, void);

    /**
    * Is it safe to show the water-can / fill icon. If we've a
    * hierarchical widget - we have only single select, otherwise
    * we need to check if we have a multi-selection. We either have
    * a m_aStyleList.mxTreeBox showing or an m_aStyleList.mxFmtLb (which we hide when not shown)
    */
    DECL_LINK(IsSafeForWaterCan_Hdl, void*, bool);

    DECL_LINK(HasSelectedStyle, void*, bool);

    //Comes into action when the current style is changed
    DECL_LINK(UpdateStyles, StyleFlags, void);
    //It is used whenever a family like Paragraph, Character, List etc. is selected.
    DECL_LINK(UpdateFamily, void*, void);
    //Used for Watercan
    DECL_LINK(UpdateStyleDependents, void*, void);

    DECL_LINK(Notify, SfxHintId, void);
    DECL_LINK(TimeOut, void*, void);

    DECL_LINK(EnableTreeDrag, bool, void);
    DECL_LINK(EnableDelete, void*, void);

    //Used in Dialog's FilterSelect
    //Here it deals with StyleSheetPool
    DECL_LINK(FilterSelect, sal_uInt16, void);

    void Update();
    Link<StyleFlags, void> m_aUpdateStyles;
    Link<void*, void> m_aReadResource;
    Link<void*, void> m_aClearResource;
    Link<SfxObjectShell const*, sal_Int32> m_aLoadFactoryStyleFilter;
    Link<void*, SfxObjectShell*> m_aSaveSelection;
    Link<void*, void> m_aUpdateStyleDependents;
    Link<void*, void> m_aUpdateFamily_Impl;

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
