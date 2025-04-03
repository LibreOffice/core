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

#include <sal/config.h>

#include <memory>

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svl/eitem.hxx>

#include <svl/style.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/styfitem.hxx>
#include <sfx2/templdlg.hxx>

#include "StyleList.hxx"

class SfxTemplateControllerItem;
class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxBindings;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;
class StyleTreeListBox_Impl;
class SfxTemplateDialog_Impl;
class SfxCommonTemplateDialog_Impl;
namespace com::sun::star::frame {
    class XModuleManager2;
}

class SfxCommonTemplateDialog_Impl
{
private:
    class DeletionWatcher;
    friend class DeletionWatcher;

    DeletionWatcher* impl_setDeletionWatcher(DeletionWatcher* pNewWatcher);

protected:
#define MAX_FAMILIES            6
#define COUNT_BOUND_FUNC        14

    friend class SfxTemplateControllerItem;

    SfxBindings* pBindings;

    css::uno::Reference<css::frame::XModuleManager2> xModuleManager;
    DeletionWatcher* m_pDeletionWatcher;

    StyleList m_aStyleList;
    std::unique_ptr<weld::CheckButton> mxPreviewCheckbox;
    std::unique_ptr<weld::CheckButton> mxSpotlightCheckbox;
    std::unique_ptr<weld::ComboBox> mxFilterLb;

    sal_uInt16 nActFamily; // Id in the ToolBox = Position - 1
    sal_uInt16 nActFilter; // FilterIdx

    bool bIsWater :1;
    bool bUpdate :1;
    bool bWaterDisabled :1;
    bool bNewByExampleDisabled :1;
    bool bUpdateByExampleDisabled :1;
    bool m_bWantHierarchical :1;

    Link<void*, size_t> m_aStyleListReadResource;
    Link<void*, void> m_aStyleListClear;
    Link<void*, void> m_aStyleListCleanup;
    Link<const ExecuteDropEvent&, sal_Int8> m_aStyleListExecuteDrop;
    Link<void*, void> m_aStyleListNewMenu;
    Link<void*, bool> m_aStyleListWaterCan;
    Link<void*, bool> m_aStyleListHasSelectedStyle;
    Link<void*, void> m_aStyleListUpdateFamily;
    Link<void*, void> m_aStyleListUpdateStyleDependents;
    Link<bool, void> m_aStyleListEnableTreeDrag;
    Link<void*, void> m_aStyleListEnableDelete;
    Link<const SfxBoolItem*, void> m_aStyleListSetWaterCanState;
    Link<sal_uInt16, void> m_aStyleListSetFamily;

    DECL_LINK(FilterSelectHdl, weld::ComboBox&, void );
    DECL_LINK(PreviewHdl, weld::Toggleable&, void);
    DECL_LINK(SpotlightHdl, weld::Toggleable&, void);

    virtual void InsertFamilyItem(sal_uInt16 nId, const SfxStyleFamilyItem& rItem) = 0;
    virtual void EnableFamilyItem(sal_uInt16 nId, bool bEnabled) = 0;
    virtual void ClearFamilyList() = 0;
    virtual void ReplaceUpdateButtonByMenu();

    void Initialize();
    void EnableHierarchical(bool, StyleList& rStyleList);

    void FilterSelect( sal_uInt16 nFilterIdx );
    void SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* );
    void SetWaterCanState( const SfxBoolItem* pItem );
    bool IsSafeForWaterCan() const;

    void SetFamily(SfxStyleFamily nFamily);
    void ActionSelect(const OUString& rId, StyleList& rStyleList);

    void SaveFactoryStyleFilter(SfxObjectShell const* i_pObjSh, sal_Int32 i_nFilter);

    DECL_LINK(ReadResource_Hdl, StyleList&, void);
    DECL_LINK(ClearResource_Hdl, void*, void);
    DECL_LINK(SaveSelection_Hdl, StyleList&, SfxObjectShell*);
    DECL_LINK(LoadFactoryStyleFilter_Hdl, SfxObjectShell const*, sal_Int32);
    DECL_LINK(UpdateStyles_Hdl, StyleFlags, void);
    DECL_LINK(UpdateFamily_Hdl, StyleList&, void);
    DECL_LINK(UpdateStyleDependents_Hdl, void*, void);

public:
    // Used in StyleList::NewMenuExecuteAction, StyleList::UpdateStyleDependents, StyleList::NewHdl,  EditHdl...
    // It comes into action whenever an existing style is selected for use, or a new style is created etc..
    bool Execute_Impl(sal_uInt16 nId, const OUString& rStr, const OUString& rRefStr,
                      sal_uInt16 nFamily, StyleList& rStyleList, SfxStyleSearchBits nMask = SfxStyleSearchBits::Auto,
                      sal_uInt16* pIdx = nullptr, const sal_uInt16* pModifier = nullptr);

    // This function handles drop of content into the treeview to create a new style
    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);
    // This function is used when a newstyle is created
    DECL_LINK(OnAsyncExecuteDrop, void*, void);

    // Used in StyleList::UpdateStyles, StyleList::Update
    // Whenever a new family(Eg. Character, List etc.) is selected it comes into action
    void FamilySelect(sal_uInt16 nId, StyleList& rStyleList, bool bRefresh = false);

    // Constructor
    SfxCommonTemplateDialog_Impl(SfxBindings* pB, weld::Container*, weld::Builder* pBuilder);

    // Destructor
    virtual ~SfxCommonTemplateDialog_Impl();

    // Used in StyleList::SelectStyle, StyleList::Notify, IMPL_LINK(PopupFlatMenuHdl)
    // These functions are used when a style is edited, deleted, created etc..
    virtual void EnableEdit(bool b, StyleList* rStyleList);
    void EnableDel(bool b, const StyleList* rStyleList);
    void EnableNew(bool b, const StyleList* rStyleList);
    void EnableHide(bool b, const StyleList* rStyleList);
    void EnableShow(bool b, const StyleList* rStyleList);

    // Used in TreeDrag
    void EnableTreeDrag(bool b);
    // It comes into action when a style is created or updated or newmenu is created
    void EnableExample_Impl(sal_uInt16 nId, bool bEnable);

    // This comes into action when a family is selected or a style is applied for use
    virtual void CheckItem(const OUString& /*rMesId*/, bool /*bCheck*/ = true) {}
    // This is used for watercan or when newmenu or watercan is enabled or updated
    virtual void EnableItem(const OUString& /*rMesId*/, bool /*bCheck*/ = true) {}
    // This is used for watercan
    virtual bool IsCheckedItem(const OUString& /*rMesId*/) { return true; }

    // This is used when a style is selected
    void SelectStyle(const OUString& rStyle, bool bIsCallback, StyleList& rStyleList);

    //When a new document is created, it comes into action
    void IsUpdate(StyleList&);

    // This function return the value of bUpdate in Stylelist
    // This value is used in StyleList's Notify
    bool GetNotifyUpdate() const { return bUpdate; }
    // This function sets the value of bUpdate in Dialog
    // This function is used in StyleList's Notify to update the value of bUpdate when required
    void SetNotifyupdate(bool b) { bUpdate = b; }

    void connect_stylelist_read_resource(const Link<void*, size_t>& rLink) { m_aStyleListReadResource = rLink; }
    void connect_stylelist_clear(const Link<void*, void>& rLink) { m_aStyleListClear = rLink; }
    void connect_stylelist_cleanup(const Link<void*, void>& rLink) { m_aStyleListCleanup = rLink; }
    void connect_stylelist_execute_drop(const Link<const ExecuteDropEvent&, sal_Int8>& rLink);
    void connect_stylelist_execute_new_menu(const Link<void*, void>& rLink) { m_aStyleListNewMenu = rLink; }
    void connect_stylelist_for_watercan(const Link<void*, bool>& rLink) { m_aStyleListWaterCan = rLink; }
    void connect_stylelist_has_selected_style(const Link<void*, bool>& rLink);
    void connect_stylelist_update_style_dependents(const Link<void*, void>& rLink);
    void connect_stylelist_enable_tree_drag(const Link<bool, void> rLink);
    void connect_stylelist_enable_delete(const Link<void*, void> rLink);
    void connect_stylelist_set_water_can_state(const Link<const SfxBoolItem*, void> rLink);
    void connect_set_family(const Link<sal_uInt16, void> rLink) { m_aStyleListSetFamily = rLink; }
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

    DECL_LINK(ToolBoxLSelect, const OUString&, void);
    DECL_LINK(ToolBoxRSelect, const OUString&, void);
    DECL_LINK(ToolMenuSelectHdl, const OUString&, void);

    virtual void EnableEdit( bool, StyleList* rStyleList) override;
    virtual void EnableItem(const OUString& rMesId, bool bCheck = true) override;
    virtual void CheckItem(const OUString& rMesId, bool bCheck = true) override;
    virtual bool IsCheckedItem(const OUString& rMesId) override;
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
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
