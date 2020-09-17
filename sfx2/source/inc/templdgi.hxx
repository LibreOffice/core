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

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svl/lstner.hxx>
#include <svl/eitem.hxx>

#include <svl/style.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/styfitem.hxx>
#include <sfx2/templdlg.hxx>

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

enum class StyleFlags {
    NONE=0, UpdateFamilyList=1, UpdateFamily=2
};
namespace o3tl {
    template<> struct typed_flags<StyleFlags> : is_typed_flags<StyleFlags, 3> {};
}

class TreeViewDropTarget;

class SfxCommonTemplateDialog_Impl : public SfxListener
{
private:
    class DeletionWatcher;
    friend class DeletionWatcher;

    void ReadResource();
    void ClearResource();
    void impl_clear();
    DeletionWatcher* impl_setDeletionWatcher(DeletionWatcher* pNewWatcher);
    OUString getDefaultStyleName( const SfxStyleFamily eFam );

protected:
#define MAX_FAMILIES            6
#define COUNT_BOUND_FUNC        14

    friend class SfxTemplateControllerItem;

    SfxBindings* pBindings;
    std::array<std::unique_ptr<SfxTemplateControllerItem>, COUNT_BOUND_FUNC> pBoundItems;

    weld::Container* mpContainer;
    std::unique_ptr<weld::Builder> mxMenuBuilder;
    std::unique_ptr<weld::Menu> mxMenu;
    OString sLastItemIdent;
    SfxModule* pModule;
    std::unique_ptr<Idle> pIdle;

    std::unique_ptr<SfxStyleFamilies> pStyleFamilies;
    std::array<std::unique_ptr<SfxTemplateItem>, MAX_FAMILIES> pFamilyState;
    SfxStyleSheetBasePool* pStyleSheetPool;
    SfxObjectShell* pCurObjShell;
    css::uno::Reference<css::frame::XModuleManager2> xModuleManager;
    DeletionWatcher* m_pDeletionWatcher;

    std::unique_ptr<weld::TreeView> mxFmtLb;
    std::unique_ptr<weld::TreeView> mxTreeBox;
    std::unique_ptr<weld::CheckButton> mxPreviewCheckbox;
    std::unique_ptr<weld::ComboBox> mxFilterLb;
    std::unique_ptr<TreeViewDropTarget> m_xTreeView1DropTargetHelper;
    std::unique_ptr<TreeViewDropTarget> m_xTreeView2DropTargetHelper;

    sal_uInt16 nActFamily; // Id in the ToolBox = Position - 1
    sal_uInt16 nActFilter; // FilterIdx
    SfxStyleSearchBits nAppFilter; // Filter, which has set the application (for automatic)

    sal_uInt16 m_nModifier;
    bool bDontUpdate :1;
    bool bIsWater :1;
    bool bUpdate :1;
    bool bUpdateFamily :1;
    bool bCanEdit :1;
    bool bCanDel :1;
    bool bCanNew :1;
    bool bCanHide :1;
    bool bCanShow :1;
    bool bWaterDisabled :1;
    bool bNewByExampleDisabled :1;
    bool bUpdateByExampleDisabled :1;
    bool bTreeDrag :1;
    bool bAllowReParentDrop:1;
    bool bHierarchical :1;
    bool m_bWantHierarchical :1;
    bool bBindingUpdate :1;

    DECL_LINK(FilterSelectHdl, weld::ComboBox&, void );
    DECL_LINK(FmtSelectHdl, weld::TreeView&, void);
    DECL_LINK(TreeListApplyHdl, weld::TreeView&, bool);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_LINK(TimeOut, Timer*, void );
    DECL_LINK(PreviewHdl, weld::Button&, void);
    DECL_LINK(PopupFlatMenuHdl, const CommandEvent&, bool);
    DECL_LINK(PopupTreeMenuHdl, const CommandEvent&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);
    DECL_LINK(OnAsyncExecuteDrop, void *, void);
    DECL_LINK(DragBeginHdl, bool&, bool);
    DECL_LINK(CustomRenderHdl, weld::TreeView::render_args, void);
    DECL_STATIC_LINK(SfxCommonTemplateDialog_Impl, CustomGetSizeHdl, weld::TreeView::get_size_args, Size);

    void DropHdl(const OUString& rStyle, const OUString& rParent);

    virtual void EnableItem(const OString& /*rMesId*/, bool /*bCheck*/ = true)
    {}
    virtual void CheckItem(const OString& /*rMesId*/, bool /*bCheck*/ = true)
    {}
    virtual bool IsCheckedItem(const OString& /*rMesId*/ )
    {
        return true;
    }

    void InvalidateBindings();
    virtual void InsertFamilyItem(sal_uInt16 nId, const SfxStyleFamilyItem& rItem) = 0;
    virtual void EnableFamilyItem(sal_uInt16 nId, bool bEnabled) = 0;
    virtual void ClearFamilyList() = 0;
    virtual void ReplaceUpdateButtonByMenu();

    void NewHdl();
    void EditHdl();
    void DeleteHdl();
    void HideHdl();
    void ShowHdl();

    bool Execute_Impl(sal_uInt16 nId, const OUString& rStr, const OUString& rRefStr,
                      sal_uInt16 nFamily, SfxStyleSearchBits nMask = SfxStyleSearchBits::Auto,
                      sal_uInt16* pIdx = nullptr, const sal_uInt16* pModifier = nullptr );

    void UpdateStyles_Impl(StyleFlags nFlags);
    const SfxStyleFamilyItem* GetFamilyItem_Impl() const;
    bool IsInitialized() const
    {
        return nActFamily != 0xffff;
    }
    void EnableDelete();
    void Initialize();
    void EnableHierarchical(bool);

    void FilterSelect( sal_uInt16 nFilterIdx, bool bForce );
    void SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* );
    void SetWaterCanState( const SfxBoolItem* pItem );
    bool IsSafeForWaterCan() const;

    void SelectStyle(const OUString& rStyle, bool bIsCallback);
    void UpdateStyleDependents();
    bool HasSelectedStyle() const;
    void GetSelectedStyle() const;
    void FillTreeBox();
    void Update_Impl();
    void UpdateFamily_Impl();

    // In which FamilyState do I have to look, in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16 StyleNrToInfoOffset( sal_uInt16 i );

    void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    void FamilySelect( sal_uInt16 nId, bool bPreviewRefresh = false );
    void SetFamily(SfxStyleFamily nFamily);
    void ActionSelect(const OString& rId);

    sal_Int32 LoadFactoryStyleFilter( SfxObjectShell const * i_pObjSh );
    void SaveFactoryStyleFilter( SfxObjectShell const * i_pObjSh, sal_Int32 i_nFilter );
    SfxObjectShell* SaveSelection();

    void PrepareMenu(const Point& rPos);
    void ShowMenu(const CommandEvent& rCEvt);

public:

    SfxCommonTemplateDialog_Impl(SfxBindings* pB, weld::Container*, weld::Builder* pBuilder);
    virtual ~SfxCommonTemplateDialog_Impl() override;

    void MenuSelect(const OString& rIdent);
    DECL_LINK( MenuSelectAsyncHdl, void*, void );

    virtual void EnableEdit( bool b )
    {
        bCanEdit = b;
    }
    void EnableDel( bool b )
    {
        bCanDel = b;
    }
    void EnableNew( bool b )
    {
        bCanNew = b;
    }
    void EnableHide( bool b )
    {
        bCanHide = b;
    }
    void EnableShow( bool b )
    {
        bCanShow = b;
    }

    void EnableTreeDrag(bool b);
    void EnableExample_Impl(sal_uInt16 nId, bool bEnable);
    SfxStyleFamily GetActualFamily() const;
    OUString GetSelectedEntry() const;

    SfxObjectShell* GetObjectShell() const
    {
        return pCurObjShell;
    }

    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);
    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

    void CreateContextMenu();
};

class ToolbarDropTarget;
class DropTargetHelper;

class SfxTemplateDialog_Impl :  public SfxCommonTemplateDialog_Impl
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

protected:
    virtual void EnableEdit( bool ) override;
    virtual void EnableItem(const OString& rMesId, bool bCheck = true) override;
    virtual void CheckItem(const OString& rMesId, bool bCheck = true) override;
    virtual bool IsCheckedItem(const OString& rMesId) override;
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
