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

#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <svl/lstner.hxx>
#include <svtools/treelistbox.hxx>
#include <svl/eitem.hxx>

#include <rsc/rscsfx.hxx>
#include <tools/rtti.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/templdlg.hxx>

class SfxStyleFamilies;
class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxBindings;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;
class SvTreeListBox ;
class StyleTreeListBox_Impl;
class SfxTemplateDialog_Impl;
class SfxCommonTemplateDialog_Impl;
class SfxDockingWindow;

namespace com { namespace sun { namespace star { namespace frame {
    class XModuleManager2;
}}}}

class DropListBox_Impl : public SvTreeListBox
{
private:
    DECL_LINK_TYPED(OnAsyncExecuteDrop, void *, void);

protected:
    SfxCommonTemplateDialog_Impl* pDialog;
    sal_uInt16 nModifier;

public:
    DropListBox_Impl(vcl::Window* pParent, const ResId& rId, SfxCommonTemplateDialog_Impl* pD)
        : SvTreeListBox(pParent, rId)
        , pDialog(pD)
        , nModifier(0)
    {
        SetHighlightRange();
    }

    DropListBox_Impl(vcl::Window* pParent, WinBits nWinBits, SfxCommonTemplateDialog_Impl* pD)
        : SvTreeListBox(pParent, nWinBits)
        , pDialog(pD)
        , nModifier(0)
    {
        SetHighlightRange();
    }

    virtual void MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) SAL_OVERRIDE;
    using SvTreeListBox::ExecuteDrop;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) SAL_OVERRIDE;

    sal_uInt16 GetModifier() const
    {
        return nModifier;
    }

    virtual bool Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

typedef std::vector<OUString> ExpandedEntries_t;

/** TreeListBox class for displaying the hierarchical view of the templates
*/
class StyleTreeListBox_Impl : public DropListBox_Impl
{
private:
    SvTreeListEntry* pCurEntry;
    Link<LinkParamNone*,void> aDoubleClickLink;
    Link<> aDropLink;
    OUString  aParent;
    OUString aStyle;

protected:
    virtual void Command(const CommandEvent& rMEvt) SAL_OVERRIDE;
    virtual bool Notify(NotifyEvent& rNEvt) SAL_OVERRIDE;
    virtual bool DoubleClickHdl() SAL_OVERRIDE;
    virtual bool ExpandingHdl() SAL_OVERRIDE;
    virtual void ExpandedHdl() SAL_OVERRIDE;
    virtual TriState NotifyMoving(
                SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                SvTreeListEntry*& rpNewParent, sal_uIntPtr& rNewChildPos) SAL_OVERRIDE;
public:
    StyleTreeListBox_Impl( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinStyle = 0);

    void Recalc();

    void SetDoubleClickHdl(const Link<LinkParamNone*,void> &rLink)
    {
        aDoubleClickLink = rLink;
    }
    void SetDropHdl(const Link<> &rLink)
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
    void MakeExpanded_Impl(ExpandedEntries_t& rEntries) const;

    virtual PopupMenu* CreateContextMenu() SAL_OVERRIDE;
};

class SfxActionListBox : public DropListBox_Impl
{
protected:
public:
    SfxActionListBox( SfxCommonTemplateDialog_Impl* pParent, WinBits nWinBits );

    virtual PopupMenu*  CreateContextMenu() SAL_OVERRIDE;
    void Recalc();
};

class SfxCommonTemplateDialog_Impl : public SfxListener
{
private:
    class DeletionWatcher;
    friend class DeletionWatcher;

    bool mbIgnoreSelect;

    void ReadResource();
    void ClearResource();
    void impl_clear();
    DeletionWatcher* impl_setDeletionWatcher(DeletionWatcher* pNewWatcher);

protected:
#define MAX_FAMILIES            5
#define COUNT_BOUND_FUNC        13

#define UPDATE_FAMILY_LIST      0x0001
#define UPDATE_FAMILY           0x0002

    friend class DropListBox_Impl;
    friend class SfxTemplateControllerItem;

    SfxBindings* pBindings;
    SfxTemplateControllerItem* pBoundItems[COUNT_BOUND_FUNC];

    VclPtr<vcl::Window> pWindow;
    SfxModule* pModule;
    Idle* pIdle;

    ResId* m_pStyleFamiliesId;
    SfxStyleFamilies* pStyleFamilies;
    SfxTemplateItem* pFamilyState[MAX_FAMILIES];
    SfxStyleSheetBasePool* pStyleSheetPool;
    VclPtr<StyleTreeListBox_Impl> pTreeBox;
    SfxObjectShell* pCurObjShell;
    css::uno::Reference<css::frame::XModuleManager2> xModuleManager;
    DeletionWatcher* m_pDeletionWatcher;

    VclPtr<SfxActionListBox> aFmtLb;
    VclPtr<ListBox> aFilterLb;
    Size aSize;

    sal_uInt16 nActFamily; // Id in the ToolBox = Position - 1
    sal_uInt16 nActFilter; // FilterIdx
    sal_uInt16 nAppFilter; // Filter, which has set the application (for automatic)

    bool bDontUpdate :1;
    bool bIsWater :1;
    bool bEnabled :1;
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

    DECL_LINK( FilterSelectHdl, ListBox* );
    DECL_LINK_TYPED( FmtSelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( ApplyHdl, LinkParamNone*, void );
    DECL_LINK_TYPED( TreeListApplyHdl, SvTreeListBox*, bool );
    DECL_LINK( DropHdl, StyleTreeListBox_Impl* );
    DECL_LINK_TYPED( TimeOut, Idle*, void );

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
    virtual void InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem* pItem ) = 0;
    virtual void EnableFamilyItem( sal_uInt16 nId, bool bEnabled = true ) = 0;
    virtual void ClearFamilyList() = 0;
    virtual void ReplaceUpdateButtonByMenu();

    void NewHdl( void* );
    void EditHdl( void* );
    void DeleteHdl( void* );
    void HideHdl( void* );
    void ShowHdl( void* );

    bool Execute_Impl(sal_uInt16 nId, const OUString& rStr, const OUString& rRefStr,
                      sal_uInt16 nFamily, sal_uInt16 nMask = 0,
                      sal_uInt16* pIdx = NULL, const sal_uInt16* pModifier = NULL );

    void UpdateStyles_Impl(sal_uInt16 nFlags);
    const SfxStyleFamilyItem* GetFamilyItem_Impl() const;
    bool IsInitialized() const
    {
        return nActFamily != 0xffff;
    }
    void ResetFocus();
    void EnableDelete();
    void Initialize();
    void EnableHierarchical(bool);

    void FilterSelect( sal_uInt16 nFilterIdx, bool bForce = false );
    void SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* );
    void SetWaterCanState( const SfxBoolItem* pItem );
    bool IsSafeForWaterCan() const;

    void SelectStyle( const OUString& rStyle );
    bool HasSelectedStyle() const;
    SfxStyleSheetBase* GetSelectedStyle() const;
    void FillTreeBox();
    void Update_Impl();
    void UpdateFamily_Impl();

    // In which FamilyState do I have to look , in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16 StyleNrToInfoOffset( sal_uInt16 i );

    void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    void FamilySelect( sal_uInt16 nId );
    void SetFamily( sal_uInt16 nId );
    void ActionSelect( sal_uInt16 nId );

    sal_Int32 LoadFactoryStyleFilter( SfxObjectShell* i_pObjSh );
    void SaveFactoryStyleFilter( SfxObjectShell* i_pObjSh, sal_Int32 i_nFilter );
    SfxObjectShell* SaveSelection();

public:
    TYPEINFO_OVERRIDE();

    SfxCommonTemplateDialog_Impl( SfxBindings* pB, vcl::Window*, bool );
    virtual ~SfxCommonTemplateDialog_Impl();

    DECL_LINK_TYPED( MenuSelectHdl, Menu*, bool );
    DECL_LINK_TYPED( MenuSelectAsyncHdl, void*, void );

    virtual void EnableEdit( bool b = true )
    {
        bCanEdit = b;
    }
    void EnableDel( bool b = true )
    {
        bCanDel = b;
    }
    void EnableNew( bool b = true )
    {
        bCanNew = b;
    }
    void EnableHide( bool b = true )
    {
        bCanHide = b;
    }
    void EnableShow( bool b = true )
    {
        bCanShow = b;
    }

    vcl::Window* GetWindow()
    {
        return pWindow;
    }

    void EnableTreeDrag(bool b = true);
    void EnableExample_Impl(sal_uInt16 nId, bool bEnable);
    SfxStyleFamily GetActualFamily() const;
    OUString GetSelectedEntry() const;

    SfxObjectShell* GetObjectShell() const
    {
        return pCurObjShell;
    }

    // normally for derivates from SvTreeListBoxes, but in this case the dialog handles context menus
    PopupMenu*  CreateContextMenu();
};

class DropToolBox_Impl : public ToolBox, public DropTargetHelper
{
    SfxTemplateDialog_Impl&     rParent;
protected:
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;
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
    bool                        m_bZoomIn;
    VclPtr<DropToolBox_Impl>    m_aActionTbL;
    VclPtr<ToolBox>             m_aActionTbR;

    DECL_LINK_TYPED( ToolBoxLSelect, ToolBox*, void );
    DECL_LINK_TYPED( ToolBoxRSelect, ToolBox*, void );
    DECL_LINK_TYPED( ToolBoxRClick, ToolBox*, void );
    DECL_LINK_TYPED( MenuSelectHdl, Menu*, bool );

protected:
    virtual void EnableEdit( bool = true ) SAL_OVERRIDE;
    virtual void EnableItem( sal_uInt16 nMesId, bool bCheck = true ) SAL_OVERRIDE;
    virtual void CheckItem( sal_uInt16 nMesId, bool bCheck = true ) SAL_OVERRIDE;
    virtual bool IsCheckedItem( sal_uInt16 nMesId ) SAL_OVERRIDE;
    virtual void LoadedFamilies() SAL_OVERRIDE;
    virtual void InsertFamilyItem( sal_uInt16 nId, const SfxStyleFamilyItem* pIten ) SAL_OVERRIDE;
    virtual void EnableFamilyItem( sal_uInt16 nId, bool bEnabled = true ) SAL_OVERRIDE;
    virtual void ClearFamilyList() SAL_OVERRIDE;
    virtual void ReplaceUpdateButtonByMenu() SAL_OVERRIDE;

    void Resize();
    Size GetMinOutputSizePixel();

    void updateFamilyImages();
    void updateNonFamilyImages();

public:
    friend class SfxTemplateDialog;
    TYPEINFO_OVERRIDE();

    SfxTemplateDialog_Impl( SfxBindings*, SfxTemplatePanelControl* pDlgWindow );
    virtual ~SfxTemplateDialog_Impl();

    void Initialize();
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
