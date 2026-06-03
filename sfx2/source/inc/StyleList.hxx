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
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/TreeView.hxx>

#include <sfx2/module.hxx>
#include <unordered_map>
#include <set>

class SfxObjectShell;
class SfxTemplateItem;
class SfxCommonTemplateDialog_Impl;
class SfxTemplateControllerItem;

class TreeViewDropTarget;

constexpr int MAX_FAMILIES = 6;
constexpr int COUNT_BOUND_FUNC = 14;

class StyleList final : public SfxListener
{
    friend class TreeViewDropTarget;
    friend class SfxTemplateControllerItem;
    friend class SfxCommonTemplateDialog_Impl;

public:
    // Constructor
    StyleList(weld::Builder* pBuilder, SfxBindings* pBindings, SfxCommonTemplateDialog_Impl* Parent,
              weld::Container* pC, const OUString& treeviewname, const OUString& flatviewname);

    // Destructor
    ~StyleList();

    // This function connects m_xTreeBox, m_xFmtLb and m_pParentDialog with certain LINKs
    void Initialize();

    // It selects the style in treeview
    void SelectStyle(std::u16string_view rStr);
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
    void ShowHierarchicalView();
    // This function hides the hierarchical tree then shows the flat tree.
    void ShowFlatView();

    // Helper function: Access to the current family item
    // Used in Dialog's updateStyleHandler, Execute_Impl etc...
    SfxStyleFamilyItem* GetFamilyItem();
    // Used to get the current selected entry in treeview
    // Used in Dialog's Execute_Impl, Action_Select etc...
    OUString GetSelectedEntry() const;
    // Returns the Family Item at ith index
    // Used in Dialog's ReadResource_Hdl
    const SfxStyleFamilyItem& GetFamilyItemByIndex(size_t i) const;

    void Enabledel(bool candel) { m_bCanDel = candel; }
    void Enablehide(bool canhide) { m_bCanHide = canhide; }
    void Enableshow(bool canshow) { m_bCanShow = canshow; }
    void Enablenew(bool cannew) { m_bCanNew = cannew; }
    void Enableedit(bool canedit) { m_bCanEdit = canedit; }

    // Used in Dialog's Execute_Impl
    // It is a necessary condition to execute a style
    bool EnableExecute();

    void ShowPreviews(bool bEnable);

    void connect_ReadResource(const Link<StyleList&, void>& rLink) { m_aReadResource = rLink; }
    void connect_ClearResource(const Link<void*, void>& rLink) { m_aClearResource = rLink; }
    void connect_UpdateFamily(const Link<StyleList&, void> rLink) { m_aUpdateFamily = rLink; }

    void FamilySelect(sal_uInt16 nEntry, bool bFillTreeView);

    DECL_LINK(NewMenuExecuteAction, void*, void);
    DECL_LINK(OnPopupEnd, const OUString&, void);

    bool HasStylesSpotlightFeature() { return m_bModuleHasStylesSpotlightFeature; }
    void SetSpotlightParaStyles(bool bSet) { m_bSpotlightParaStyles = bSet; }
    bool IsSpotlightParaStyles() { return m_bSpotlightParaStyles; }
    void SetSpotlightCharStyles(bool bSet) { m_bSpotlightCharStyles = bSet; }
    bool IsSpotlightCharStyles() { return m_bSpotlightCharStyles; }

    sal_uInt16 GetActiveFilter();

private:
    void FillHierarchicalTreeView(bool bExpandRootEntries = false);
    void FillFlatTreeView();

    void SelectUpdate(const OUString& rStr);

    OUString getDefaultStyleName(const SfxStyleFamily eFam);
    SfxStyleFamily GetActualFamily();

    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);
    void DropHdl(const OUString& rStyle, const OUString& rParent);

    void MenuSelect(const OUString& rIdent);
    Point PrepareMenu(const CommandEvent& rContextMenuEvent);
    void ShowMenu(const Point& rPos);
    void CreateContextMenu();

    void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

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

    void InvalidateBindings();

    void Update();

    Link<StyleList&, void> m_aReadResource;
    Link<void*, void> m_aClearResource;
    Link<StyleList&, SfxObjectShell*> m_aSaveSelection;
    Link<StyleList&, void> m_aUpdateFamily;

    void NewHdl();
    void EditHdl();
    void DeleteHdl();
    void HideHdl();
    void ShowHdl();

    DECL_LINK(ExpandHdl, const weld::TreeIter&, bool);
    DECL_LINK(CollapseHdl, const weld::TreeIter&, bool);

    DECL_LINK(DragBeginHdl, bool&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);
    DECL_LINK(CustomRenderHdl, weld::TreeView::render_args, void);
    DECL_LINK(FmtSelectHdl, weld::ItemView&, void);
    DECL_LINK(TreeListApplyHdl, const weld::TreeIter&, bool);
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
    bool m_bCanDel : 1;
    bool m_bBindingUpdate : 1;
    SfxStyleSheetBasePool* m_pStyleSheetPool;

    std::unique_ptr<weld::TreeView> m_xFmtLb;
    std::unique_ptr<weld::TreeView> m_xTreeBox;

    std::unique_ptr<weld::Builder> mxMenuBuilder;
    std::unique_ptr<weld::Menu> mxMenu;

    SfxStyleFamilies m_aStyleFamilies;
    std::array<std::unique_ptr<SfxTemplateItem>, MAX_FAMILIES> m_pFamilyState;
    SfxObjectShell* m_pCurObjShell;
    sal_uInt16 m_nActFamily;

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

    bool m_bModuleHasStylesSpotlightFeature = false;
    bool m_bSpotlightParaStyles = false;
    bool m_bSpotlightCharStyles = false;

    std::unordered_map<SfxStyleFamily, std::set<SfxStyleSearchBits>> m_aFamilySelectedFiltersSet;
    std::unordered_map<SfxStyleFamily, std::set<OUString>> m_aFamilyExpandedStyleEntriesSet;
    // used to expand all root level entries on first selection
    std::set<SfxStyleFamily> m_aFamiliesThatHaveBeenSelectedSet;
};
