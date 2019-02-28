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

#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/weld.hxx>
#include <svl/lstner.hxx>
#include <vcl/treelistbox.hxx>
#include <svl/eitem.hxx>

#include <svl/style.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/styfitem.hxx>
#include <sfx2/templdlg.hxx>

class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxBindings;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;
class SvTreeListBox ;
class StyleTreeListBox_Impl;
class SfxTemplateDialog_Impl;
class SfxCommonTemplateDialog_Impl;

namespace com { namespace sun { namespace star { namespace frame {
    class XModuleManager2;
}}}}

class DropListBox_Impl : public SvTreeListBox
{
private:
    DECL_LINK(OnAsyncExecuteDrop, void *, void);

protected:
    SfxCommonTemplateDialog_Impl* pDialog;
    sal_uInt16 nModifier;

public:
    DropListBox_Impl(vcl::Window* pParent, WinBits nWinBits, SfxCommonTemplateDialog_Impl* pD)
        : SvTreeListBox(pParent, nWinBits)
        , pDialog(pD)
        , nModifier(0)
    {
        SetHighlightRange();
    }

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override;
    using SvTreeListBox::ExecuteDrop;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override;

    sal_uInt16 GetModifier() const
    {
        return nModifier;
    }

    virtual bool EventNotify( NotifyEvent& rNEvt ) override;
};


/** TreeListBox class for displaying the hierarchical view of the templates
*/
class StyleTreeListBox_Impl : public DropListBox_Impl
{
private:
    SvTreeListEntry* pCurEntry;
    Link<LinkParamNone*,void> aDoubleClickLink;
    Link<StyleTreeListBox_Impl&,bool> aDropLink;
    OUString  aParent;
    OUString  aStyle;

protected:
    virtual bool EventNotify(NotifyEvent& rNEvt) override;
    virtual bool DoubleClickHdl() override;
    virtual bool ExpandingHdl() override;
    virtual void ExpandedHdl() override;
    virtual TriState NotifyMoving(
                SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos) override;
public:
    StyleTreeListBox_Impl( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle);

    void Recalc();

    void SetDoubleClickHdl(const Link<LinkParamNone*,void> &rLink)
    {
        aDoubleClickLink = rLink;
    }
    void SetDropHdl(const Link<StyleTreeListBox_Impl&,bool> &rLink)
    {
        aDropLink = rLink;
    }

    using SvTreeListBox::GetParent;

    const OUString& GetParent() const
    {
        return aParent;
    }
    const OUString& GetStyle() const
    {
        return aStyle;
    }
    void MakeExpanded_Impl(std::vector<OUString>& rEntries) const;

    virtual VclPtr<PopupMenu> CreateContextMenu() override;
};

class SfxActionListBox : public DropListBox_Impl
{
protected:
public:
    SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinBits );

    virtual VclPtr<PopupMenu> CreateContextMenu() override;
    void Recalc();
};

enum class StyleFlags {
    NONE=0, UpdateFamilyList=1, UpdateFamily=2
};
namespace o3tl {
    template<> struct typed_flags<StyleFlags> : is_typed_flags<StyleFlags, 3> {};
}

class SfxCommonTemplateDialog_Impl : public SfxListener
{
private:
    class DeletionWatcher;
    friend class DeletionWatcher;

    void ReadResource();
    void ClearResource();
    void impl_clear();
    DeletionWatcher* impl_setDeletionWatcher(DeletionWatcher* pNewWatcher);

protected:
#define MAX_FAMILIES            6
#define COUNT_BOUND_FUNC        14

    friend class DropListBox_Impl;
    friend class SfxTemplateControllerItem;

    SfxBindings* pBindings;
    std::array<std::unique_ptr<SfxTemplateControllerItem>, COUNT_BOUND_FUNC> pBoundItems;

    VclPtr<vcl::Window> pWindow;
    std::unique_ptr<VclBuilder> mxBuilder;
    VclPtr<PopupMenu> mxMenu;
    OString sLastItemIdent;
    SfxModule* pModule;
    std::unique_ptr<Idle> pIdle;

    std::unique_ptr<SfxStyleFamilies> pStyleFamilies;
    std::array<std::unique_ptr<SfxTemplateItem>, MAX_FAMILIES> pFamilyState;
    SfxStyleSheetBasePool* pStyleSheetPool;
    SfxObjectShell* pCurObjShell;
    css::uno::Reference<css::frame::XModuleManager2> xModuleManager;
    DeletionWatcher* m_pDeletionWatcher;

    VclPtr<SfxActionListBox> aFmtLb;
    VclPtr<StyleTreeListBox_Impl> pTreeBox;
    VclPtr<CheckBox> aPreviewCheckbox;
    VclPtr<ListBox> aFilterLb;

    sal_uInt16 nActFamily; // Id in the ToolBox = Position - 1
    sal_uInt16 nActFilter; // FilterIdx
    SfxStyleSearchBits nAppFilter; // Filter, which has set the application (for automatic)

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
    bool bHierarchical :1;
    bool m_bWantHierarchical :1;
    bool bBindingUpdate :1;

    DECL_LINK( FilterSelectHdl, ListBox&, void );
    DECL_LINK( FmtSelectHdl, SvTreeListBox*, void );
    DECL_LINK( ApplyHdl, LinkParamNone*, void );
    DECL_LINK( TreeListApplyHdl, SvTreeListBox*, bool );
    DECL_LINK( DropHdl, StyleTreeListBox_Impl&, bool );
    DECL_LINK( TimeOut, Timer*, void );
    DECL_LINK( PreviewHdl, Button*, void);

    virtual void EnableItem(sal_uInt16 /*nMesId*/, bool /*bCheck*/ = true)
    {}
    virtual void CheckItem(sal_uInt16 /*nMesId*/, bool /*bCheck*/ = true)
    {}
    virtual bool IsCheckedItem(sal_uInt16 /*nMesId*/ )
    {
        return true;
    }
    virtual void LoadedFamilies()
    {}

    void InvalidateBindings();
    virtual void InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem& rItem ) = 0;
    virtual void EnableFamilyItem( sal_uInt16 nId, bool bEnabled ) = 0;
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

    void SelectStyle( const OUString& rStyle );
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
    void ActionSelect( sal_uInt16 nId );

    sal_Int32 LoadFactoryStyleFilter( SfxObjectShell const * i_pObjSh );
    void SaveFactoryStyleFilter( SfxObjectShell const * i_pObjSh, sal_Int32 i_nFilter );
    SfxObjectShell* SaveSelection();

public:

    SfxCommonTemplateDialog_Impl( SfxBindings* pB, vcl::Window* );
    virtual ~SfxCommonTemplateDialog_Impl() override;

    DECL_LINK( MenuSelectHdl, Menu*, bool );
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

    vcl::Window* GetWindow()
    {
        return pWindow;
    }

    weld::Widget* GetFrameWeld()
    {
        return pWindow ? pWindow->GetFrameWeld() : nullptr;
    }

    void EnableTreeDrag(bool b);
    void EnableExample_Impl(sal_uInt16 nId, bool bEnable);
    SfxStyleFamily GetActualFamily() const;
    OUString GetSelectedEntry() const;

    SfxObjectShell* GetObjectShell() const
    {
        return pCurObjShell;
    }

    // normally for derivates from SvTreeListBoxes, but in this case the dialog handles context menus
    VclPtr<PopupMenu> const & CreateContextMenu();
};

class DropToolBox_Impl : public ToolBox, public DropTargetHelper
{
    SfxTemplateDialog_Impl&     rParent;
protected:
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
public:
    DropToolBox_Impl(vcl::Window* pParent, SfxTemplateDialog_Impl* pTemplateDialog);
};

class SfxTemplateDialog_Impl :  public SfxCommonTemplateDialog_Impl
{
private:
    friend class SfxTemplateControllerItem;
    friend class DropToolBox_Impl;
    friend class SfxTemplatePanelControl;

    VclPtr<vcl::Window>         m_pFloat;
    VclPtr<DropToolBox_Impl>    m_aActionTbL;
    VclPtr<ToolBox>             m_aActionTbR;

    DECL_LINK( ToolBoxLSelect, ToolBox*, void );
    DECL_LINK( ToolBoxRSelect, ToolBox*, void );
    DECL_LINK( ToolBoxRClick, ToolBox*, void );
    DECL_LINK( MenuSelectHdl, Menu*, bool );

protected:
    virtual void EnableEdit( bool ) override;
    virtual void EnableItem( sal_uInt16 nMesId, bool bCheck = true ) override;
    virtual void CheckItem( sal_uInt16 nMesId, bool bCheck = true ) override;
    virtual bool IsCheckedItem( sal_uInt16 nMesId ) override;
    virtual void LoadedFamilies() override;
    virtual void InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem& rItem ) override;
    virtual void EnableFamilyItem( sal_uInt16 nId, bool bEnabled ) override;
    virtual void ClearFamilyList() override;
    virtual void ReplaceUpdateButtonByMenu() override;

    void Resize();
    Size GetMinOutputSizePixel();

public:
    friend class SfxTemplateDialog;

    SfxTemplateDialog_Impl( SfxBindings*, SfxTemplatePanelControl* pDlgWindow );
    virtual ~SfxTemplateDialog_Impl() override;

    void Initialize();
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
