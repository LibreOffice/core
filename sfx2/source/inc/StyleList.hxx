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

#pragma once

#include <sal/config.h>

#include <o3tl/typed_flags_set.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/styfitem.hxx>
#include <svl/eitem.hxx>
#include <svl/style.hxx>
#include <vcl/idle.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>

class SfxObjectShell;
class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxCommonTemplateDialog_Impl;
class SfxTemplateControllerItem;

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

class StyleList final : public SfxListener
{
    friend class TreeViewDropTarget;
    friend class SfxTemplateControllerItem;

public:
    // Constructor
    StyleList(weld::Builder* pBuilder, SfxBindings* pBindings, SfxCommonTemplateDialog_Impl* Parent,
              weld::Container* pC, const OUString& treeviewname, const OUString& flatviewname);

    // Destructor
    ~StyleList();

    // This function connects m_xTreeBox, m_xFmtLb and m_pParentDialog with certain LINKs
    void Initialize();

    // It selects the style in treeview
    // bIsCallBack is true for the selected style. For eg. if "Addressee" is selected in
    // styles, bIsCallBack will be true for it.
    void SelectStyle(const OUString& rStr, bool bIsCallback);
    // Checks whether a family has a saved state
    bool CurrentFamilyHasState() { return nullptr != m_pFamilyState[m_nActFamily - 1]; }

    // This function is a subpart of Dialog's SetFamilyState
    // When a new style is selected for use, it resets it.
    void SetFamilyState(sal_uInt16 nSlotId, const SfxTemplateItem* pItem);

    // It is used in Dialog's EnableExample_Impl
    // When the value of m_bNewbyExampleDisabled is updated there
    // while creating a new style by example,
    // the corresponding value gets updated here too.
    void EnableNewByExample(bool newByExampleDisabled);

    // This function is used to set a hierarchical view.
    void SetHierarchical();
    // This function handles the controls while setting a filter except hierarchical
    void SetFilterControlsHandle();
    // Return whether treeview is visible
    // It is used in StyleList's UpdateStyles_Hdl
    // It is used to defaultly set the hierarchical view
    bool IsTreeView() const { return m_xTreeBox->get_visible(); }

    // Helper function: Access to the current family item
    // Used in Dialog's updateStyleHandler, Execute_Impl etc...
    const SfxStyleFamilyItem* GetFamilyItem() const;
    // Used to get the current selected entry in treeview
    // Used in Dialog's Execute_Impl, Action_Select etc...
    OUString GetSelectedEntry() const;
    // Returns the Family Item at ith index
    // Used in Dialog's ReadResource_Hdl
    const SfxStyleFamilyItem& GetFamilyItemByIndex(size_t i) const;
    bool IsHierarchical() const { return m_bHierarchical; }

    void Enabledel(bool candel) { m_bCanDel = candel; }
    void Enablehide(bool canhide) { m_bCanHide = canhide; }
    void Enableshow(bool canshow) { m_bCanShow = canshow; }
    void Enablenew(bool cannew) { m_bCanNew = cannew; }
    void Enableedit(bool canedit) { m_bCanEdit = canedit; }

    // Used in Dialog's Execute_Impl
    // It is a necessary condition to execute a style
    bool EnableExecute();

    void connect_UpdateStyles(const Link<StyleFlags, void>& rLink) { m_aUpdateStyles = rLink; }
    void connect_ReadResource(const Link<StyleList&, void>& rLink) { m_aReadResource = rLink; }
    void connect_ClearResource(const Link<void*, void>& rLink) { m_aClearResource = rLink; }
    void connect_LoadFactoryStyleFilter(const Link<SfxObjectShell const*, sal_Int32>& rLink);
    void connect_SaveSelection(const Link<StyleList&, SfxObjectShell*> rLink);
    void connect_UpdateFamily(const Link<StyleList&, void> rLink) { m_aUpdateFamily = rLink; }

    void FamilySelect(sal_uInt16 nEntry, bool bRefresh = false);
    void FilterSelect(sal_uInt16 nActFilter, bool bsetFilter);

    DECL_LINK(NewMenuExecuteAction, void*, void);
    DECL_LINK(OnPopupEnd, const OUString&, void);

    bool HasStylesHighlighterFeature() { return m_bModuleHasStylesHighlighterFeature; }
    void SetHighlightParaStyles(bool bSet) { m_bHighlightParaStyles = bSet; }
    bool IsHighlightParaStyles() { return m_bHighlightParaStyles; }
    void SetHighlightCharStyles(bool bSet) { m_bHighlightCharStyles = bSet; }
    bool IsHighlightCharStyles() { return m_bHighlightCharStyles; }

private:
    void FillTreeBox(SfxStyleFamily eFam);

    void UpdateFamily();
    void UpdateStyles(StyleFlags nFlags);

    OUString getDefaultStyleName(const SfxStyleFamily eFam);
    SfxStyleFamily GetActualFamily() const;
    void GetSelectedStyle() const;

    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);
    void DropHdl(const OUString& rStyle, const OUString& rParent);

    void MenuSelect(const OUString& rIdent);
    void PrepareMenu(const Point& rPos);
    void ShowMenu(const CommandEvent& rCEvt);
    void CreateContextMenu();

    void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    // In which FamilyState do I have to look, in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16 StyleNrToInfoOffset(sal_uInt16 i);

    DECL_LINK(ReadResource, void*, size_t);
    DECL_LINK(Clear, void*, void);
    DECL_LINK(Cleanup, void*, void);
    DECL_LINK(ExecuteDrop, const ExecuteDropEvent&, sal_Int8);
    DECL_LINK(IsSafeForWaterCan, void*, bool);
    DECL_LINK(HasSelectedStyle, void*, bool);
    DECL_LINK(UpdateStyleDependents, void*, void);
    DECL_LINK(TimeOut, Timer*, void);
    DECL_LINK(EnableTreeDrag, bool, void);
    DECL_LINK(EnableDelete, void*, void);
    DECL_LINK(SetWaterCanState, const SfxBoolItem*, void);
    DECL_LINK(SetFamily, sal_uInt16, void);

    void InvalidateBindings();

    void Update();
    Link<StyleFlags, void> m_aUpdateStyles;
    Link<StyleList&, void> m_aReadResource;
    Link<void*, void> m_aClearResource;
    Link<SfxObjectShell const*, sal_Int32> m_aLoadFactoryStyleFilter;
    Link<StyleList&, SfxObjectShell*> m_aSaveSelection;
    Link<StyleList&, void> m_aUpdateFamily;

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
    SfxStyleSearchBits m_nAppFilter; // Filter, which has set the application (for automatic)

    std::unique_ptr<TreeViewDropTarget> m_xTreeView1DropTargetHelper;
    std::unique_ptr<TreeViewDropTarget> m_xTreeView2DropTargetHelper;

    SfxCommonTemplateDialog_Impl* m_pParentDialog;
    SfxBindings* m_pBindings;
    std::array<std::unique_ptr<SfxTemplateControllerItem>, COUNT_BOUND_FUNC> pBoundItems;

    std::unique_ptr<Idle> pIdle;

    OUString sLastItemIdent;
    SfxModule* m_Module;
    sal_uInt16 m_nModifier;
    weld::Container* m_pContainer;

    bool m_bModuleHasStylesHighlighterFeature = false;
    bool m_bHighlightParaStyles = false;
    bool m_bHighlightCharStyles = false;
};
