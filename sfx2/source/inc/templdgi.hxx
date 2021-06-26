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
#ifndef INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX
#define INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX

class SfxTemplateControllerItem;

#include <sal/config.h>

#include <array>
#include <memory>
#include <optional>

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svl/lstner.hxx>
#include <svl/eitem.hxx>

#include <svl/style.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/styfitem.hxx>
#include <sfx2/templdlg.hxx>

#include <vcl/idle.hxx>

#include "StyleList.hxx"

class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxBindings;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;
class StyleTreeListBox_Impl;
class SfxTemplateDialog_Impl;
class SfxCommonTemplateDialog_Impl;
class StyleList;


namespace com::sun::star::frame {
    class XModuleManager2;
}

class SfxCommonTemplateDialog_Impl : public SfxListener
{
private:
    class DeletionWatcher;
    friend class DeletionWatcher;

    void impl_clear();
    DeletionWatcher* impl_setDeletionWatcher(DeletionWatcher* pNewWatcher);
    OUString getDefaultStyleName(const SfxStyleFamily eFam);

protected:
#define MAX_FAMILIES 6
#define COUNT_BOUND_FUNC 14

    friend class SfxTemplateControllerItem;

    SfxBindings* pBindings;
    std::array<std::unique_ptr<SfxTemplateControllerItem>, COUNT_BOUND_FUNC> pBoundItems;

    weld::Container* mpContainer;
    SfxModule* pModule;
    std::unique_ptr<Idle> pIdle;

    std::optional<SfxStyleFamilies> mxStyleFamilies;
    SfxObjectShell* pCurObjShell;
    css::uno::Reference<css::frame::XModuleManager2> xModuleManager;
    DeletionWatcher* m_pDeletionWatcher;

    StyleList m_aStyleList;
    std::unique_ptr<weld::CheckButton> mxPreviewCheckbox;
    std::unique_ptr<weld::ComboBox> mxFilterLb;

    sal_uInt16 nActFamily; // Id in the ToolBox = Position - 1
    sal_uInt16 nActFilter; // FilterIdx
    SfxStyleSearchBits nAppFilter; // Filter, which has set the application (for automatic)

    bool bIsWater : 1;
    bool bUpdate : 1;
    bool bUpdateFamily : 1;
    bool bWaterDisabled : 1;
    bool bNewByExampleDisabled : 1;
    bool bUpdateByExampleDisabled : 1;
    bool bTreeDrag : 1;
    bool bHierarchical : 1;
    bool m_bWantHierarchical : 1;

    Link<void*, size_t> m_aStyleListReadResource;
    Link<void*, void> m_aStyleListClear;
    Link<void*, void> m_aStyleListCleanup;
    Link<const ExecuteDropEvent&, sal_Int8> m_aStyleListExecuteDrop;
    Link<void*, void> m_aStyleListNewMenu;
    Link<void*, bool> m_aStyleListWaterCan;
    Link<void*, bool> m_aStyleListHasSelectedStyle;
    Link<StyleFlags, void> m_aStyleListUpdateStyles;
    Link<void*, void> m_aStyleListUpdateFamily;
    Link<SfxHintId, void> m_aStyleListNotify;
    Link<void*, void> m_aStyleListUpdateStyleDependents;
    Link<bool, void> m_aStyleListEnableTreeDrag;
    Link<void*, void> m_aStyleListTimeout;
    Link<sal_uInt16, void> m_aStyleListFilterSelect;
    Link<void*, void> m_aStyleListEnableDelete;

    DECL_LINK(FilterSelectHdl, weld::ComboBox&, void);
    DECL_LINK(TimeOut, Timer*, void);
    DECL_LINK(PreviewHdl, weld::Toggleable&, void);

    void InvalidateBindings();
    virtual void InsertFamilyItem(sal_uInt16 nId, const SfxStyleFamilyItem& rItem) = 0;
    virtual void EnableFamilyItem(sal_uInt16 nId, bool bEnabled) = 0;
    virtual void ClearFamilyList() = 0;
    virtual void ReplaceUpdateButtonByMenu();

    void Initialize();
    void EnableHierarchical(bool);

    void FilterSelect(sal_uInt16 nFilterIdx, bool bForce);
    void SetFamilyState(sal_uInt16 nSlotId, const SfxTemplateItem*);
    void SetWaterCanState(const SfxBoolItem* pItem);
    bool IsSafeForWaterCan() const;

    void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    void SetFamily(SfxStyleFamily nFamily);
    void ActionSelect(const OString& rId);

    void SaveFactoryStyleFilter(SfxObjectShell const* i_pObjSh, sal_Int32 i_nFilter);

    DECL_LINK(ReadResource_Hdl, void*, void);
    DECL_LINK(ClearResource_Hdl, void*, void);
    DECL_LINK(SaveSelection_Hdl, void*, SfxObjectShell*);
    DECL_LINK(LoadFactoryStyleFilter_Hdl, SfxObjectShell const*, sal_Int32);
    DECL_LINK(UpdateStyles_Hdl, StyleFlags, void);
    DECL_LINK(UpdateFamily_Hdl, void*, void);
    DECL_LINK(UpdateStyleDependents_Hdl, void*, void);

public:
    //Used in StyleList::NewMenu_ExecuteAction, StyleList::UpdateStyleDependents, StyleList::NewHdl,  EditHdl...
    bool Execute_Impl(sal_uInt16 nId, const OUString& rStr, const OUString& rRefStr,
                      sal_uInt16 nFamily, SfxStyleSearchBits nMask = SfxStyleSearchBits::Auto,
                      sal_uInt16* pIdx = nullptr, const sal_uInt16* pModifier = nullptr);

    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);
    DECL_LINK(OnAsyncExecuteDrop, void*, void);

    //Used in StyleList::UpdateStyles, StyleList::Update
    void FamilySelect(sal_uInt16 nId, bool bPreviewRefresh = false);

    SfxCommonTemplateDialog_Impl(SfxBindings* pB, weld::Container*, weld::Builder* pBuilder);

    virtual ~SfxCommonTemplateDialog_Impl() override;

    //Used in StyleList::SelectStyle, StyleList::Notify, IMPL_LINK(PopupFlatMenuHdl)
    virtual void EnableEdit(bool b) { m_aStyleList.Enableedit(b); }
    void EnableDel(bool b) { m_aStyleList.Enabledel(b); }
    void EnableNew(bool b) { m_aStyleList.Enablenew(b); }
    void EnableHide(bool b) { m_aStyleList.Enablehide(b); }
    void EnableShow(bool b) { m_aStyleList.Enableshow(b); }

    void EnableTreeDrag(bool b);
    //Used in StyleList::EnableTreeDrag
    void SetEnableDrag(bool treedrag) { bTreeDrag = treedrag; }
    void EnableExample_Impl(sal_uInt16 nId, bool bEnable);

    //Used in StyleList::ReadResource, StyleList::stylist_clear
    void SetObjectShell(SfxObjectShell* shell) { pCurObjShell = shell; }

    //Used in StyleList::Update
    virtual void CheckItem(const OString& /*rMesId*/, bool /*bCheck*/ = true) {}
    //Used in StyleList::FillTreeBox
    virtual void EnableItem(const OString& /*rMesId*/, bool /*bCheck*/ = true) {}
    //Used in StyleList::UpdateStyleDependents, StyleList::Notify
    virtual bool IsCheckedItem(const OString& /*rMesId*/) { return true; }

    //Used in StyleList::ExecuteDrop, StyleList::FillTreeBox, StyleList::SetHierarchical...
    void SelectStyle(const OUString& rStyle, bool bIsCallback);

    //Used in StyleList::UpdateStyles, StyleList::Update
    void SetApplicationFilter(SfxStyleSearchBits filter) { nAppFilter = filter; }
    //Used in StyleList::ReadResource, StyleList::Update
    void SetFilterByIndex(sal_uInt16 filter) { nActFilter = filter; }

    //Used in Notify in StyleList
    bool GetNotifyUpdate() { return bUpdate; }
    //Used in Notify in StyleList
    void SetNotifyupdate(bool b) { bUpdate = b; }

    void connect_stylelist_read_resource(const Link<void*, size_t>& rLink) { m_aStyleListReadResource = rLink; }
    void connect_stylelist_clear(const Link<void*, void>& rLink) { m_aStyleListClear = rLink; }
    void connect_stylelist_cleanup(const Link<void*, void>& rLink) { m_aStyleListCleanup = rLink; }
    void connect_stylelist_execute_drop(const Link<const ExecuteDropEvent&, sal_Int8>& rLink)
    {
        m_aStyleListExecuteDrop = rLink;
    }
    void connect_stylelist_execute_new_menu(const Link<void*, void>& rLink) { m_aStyleListNewMenu = rLink; }
    void connect_stylelist_for_watercan(const Link<void*, bool>& rLink) { m_aStyleListWaterCan = rLink; }
    void connect_stylelist_has_selected_style(const Link<void*, bool>& rLink)
    {
        m_aStyleListHasSelectedStyle = rLink;
    }
    void connect_stylelist_update_styles(const Link<StyleFlags, void> rLink) { m_aStyleListUpdateStyles = rLink; }
    void connect_stylelist_update_family(const Link<void*, void> rLink) { m_aStyleListUpdateFamily = rLink; }
    void connect_stylelist_update_style_dependents(const Link<void*, void>& rLink)
    {
        m_aStyleListUpdateStyleDependents = rLink;
    }
    void connect_stylelist_enable_tree_drag(const Link<bool, void> rLink)
    {
        m_aStyleListEnableTreeDrag = rLink;
    }
    void connect_stylelist_timeout(const Link<void*, void> rLink) { m_aStyleListTimeout = rLink; }
    void connect_stylelist_notify(const Link<SfxHintId, void> rLink) { m_aStyleListNotify = rLink; }
    void connect_stylelist_filter_select(Link<sal_uInt16, void> rLink)
    {
        m_aStyleListFilterSelect = rLink;
    }
    void connect_stylelist_enable_delete(const Link<void*, void> rLink)
    {
        m_aStyleListEnableDelete = rLink;
    }
};

class ToolbarDropTarget;
class DropTargetHelper;

class SfxTemplateDialog_Impl final : public SfxCommonTemplateDialog_Impl
{
private:
    friend class SfxTemplateControllerItem;
    friend class SfxTemplatePanelControl;

    std::unique_ptr<ToolbarDropTarget> m_xToolbarDropTargetHelper;
    std::unique_ptr<weld::Toolbar> m_xActionTbL;
    std::unique_ptr<weld::Toolbar> m_xActionTbR;
    std::unique_ptr<weld::Menu> m_xToolMenu;
    int m_nActionTbLVisible;

    void FillToolMenu();

    DECL_LINK(ToolBoxLSelect, const OString&, void);
    DECL_LINK(ToolBoxRSelect, const OString&, void);
    DECL_LINK(ToolMenuSelectHdl, const OString&, void);

    virtual void EnableEdit( bool ) override;
    virtual void InsertFamilyItem(sal_uInt16 nId, const SfxStyleFamilyItem& rItem) override;
    virtual void EnableFamilyItem(sal_uInt16 nId, bool bEnabled) override;
    virtual void ClearFamilyList() override;
    virtual void ReplaceUpdateButtonByMenu() override;

public:
    friend class SfxTemplateDialog;

    SfxTemplateDialog_Impl( SfxBindings*, SfxTemplatePanelControl* pDlgWindow );
    virtual ~SfxTemplateDialog_Impl() override;

    sal_Int8 AcceptToolbarDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);

    void Initialize();

    virtual void CheckItem(const OString& rMesId, bool bCheck = true) override;
    virtual void EnableItem(const OString& rMesId, bool bCheck = true) override;
    virtual bool IsCheckedItem(const OString& rMesId) override;
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
