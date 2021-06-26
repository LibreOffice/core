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

#include <StyleList.hxx>

class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxBindings;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;
class StyleTreeListBox_Impl;
class SfxTemplateDialog_Impl;
class SfxCommonTemplateDialog_Impl;

namespace weld {
    class StyleList;
}

namespace com::sun::star::frame {
    class XModuleManager2;
}

class SfxCommonTemplateDialog_Impl : public SfxListener
{
private:
    class DeletionWatcher;
    friend class DeletionWatcher;
;
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

    bool bIsWater :1;
    bool bUpdate :1;
    bool bUpdateFamily :1;
    bool bWaterDisabled :1;
    bool bNewByExampleDisabled :1;
    bool bUpdateByExampleDisabled :1;
    bool bTreeDrag : 1;
    bool bHierarchical :1;
    bool m_bWantHierarchical :1;
    bool bCanEdit :1;
    bool bCanDel : 1;
    bool bCanNew : 1;
    bool bCanHide : 1;
    bool bCanShow : 1;


    DECL_LINK(FilterSelectHdl, weld::ComboBox&, void );
    DECL_LINK(TimeOut, Timer*, void );
    DECL_LINK(PreviewHdl, weld::Toggleable&, void);

    void InvalidateBindings();
    virtual void InsertFamilyItem(sal_uInt16 nId, const SfxStyleFamilyItem& rItem) = 0;
    virtual void EnableFamilyItem(sal_uInt16 nId, bool bEnabled) = 0;
    virtual void ClearFamilyList() = 0;
    virtual void ReplaceUpdateButtonByMenu();

    void Initialize();
    void EnableHierarchical(bool);

    void FilterSelect(sal_uInt16 nFilterIdx, bool bForce);
    void SetFamilyState( sal_uInt16 nSlotId, const SfxTemplateItem* );
    void SetWaterCanState( const SfxBoolItem* pItem );
    bool IsSafeForWaterCan() const;

    void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    void SetFamily(SfxStyleFamily nFamily);
    void ActionSelect(const OString& rId);

    void SaveFactoryStyleFilter( SfxObjectShell const * i_pObjSh, sal_Int32 i_nFilter );

public:

    void ReadResource();
    void ClearResource();
    bool IsInitialized() const
    {
        return nActFamily != 0xffff;
    }

    bool Execute_Impl(sal_uInt16 nId, const OUString& rStr, const OUString& rRefStr,
                      sal_uInt16 nFamily, SfxStyleSearchBits nMask = SfxStyleSearchBits::Auto,
                      sal_uInt16* pIdx = nullptr, const sal_uInt16* pModifier = nullptr );

    DECL_LINK(OnAsyncExecuteDrop, void *, void);
    SfxCommonTemplateDialog_Impl(SfxBindings* pB, weld::Container*, weld::Builder* pBuilder);
    void FamilySelect( sal_uInt16 nId, bool bPreviewRefresh = false );
    SfxObjectShell* SaveSelection();
    virtual ~SfxCommonTemplateDialog_Impl() override;

    virtual void EnableEdit( bool b )
    {
        bCanEdit = b;
    }
    void EnableDel(bool b)
    {
        bCanDel = b;
    }
     void EnableNew(bool b)
    {
        bCanNew = b;
    }
    void EnableHide(bool b)
    {
        bCanHide = b;
    }
    void EnableShow(bool b)
    {
        bCanShow = b;
    }
    void EnableTreeDrag(bool bEnable);
    virtual void EnableItem(const OString& /*rMesId*/, bool /*bCheck*/ = true)
    {}
    void EnableExample_Impl(sal_uInt16 nId, bool bEnable);

    SfxObjectShell* GetObjectShell() const
    {
        return pCurObjShell;
    }

    void SetObjectShell(SfxObjectShell* shell) { pCurObjShell = shell; }

    virtual void CheckItem(const OString& /*rMesId*/, bool /*bCheck*/ = true)
    {}

    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

    void UpdateStyleDependents();
    sal_Int32 LoadFactoryStyleFilter(SfxObjectShell const* i_pObjSh);

    void UpdateStyles_Impl(StyleFlags nFlags);
    void UpdateFamily_Impl();
    void SelectStyle(const OUString& rStyle, bool bIsCallback);

    //Setter for variable nAppFilter
    void NAppFilter(SfxStyleSearchBits filter) { nAppFilter = filter; }

    //Setter for variable nActFilter
    void NActFilter(sal_uInt16 filter) { nActFilter = filter; }

    //Setter for variable bUpdateFamily
    void BUpdateFamily(bool b_updatefamily) { bUpdateFamily = b_updatefamily; }

    virtual bool IsCheckedItem(const OString& /*rMesId*/) { return true; }

    //Getter for variable bUpdate
    bool bupdate() { return bUpdate; }

    //Setter for variable bUpdate
    void setBupdate(bool b) { bUpdate = b; }

    //Setters for bCandel, Hide, Show, New
    void SetBCanDel(bool candel) { bCanDel = candel; }
    void SetBCanhide(bool canhide) { bCanHide = canhide; }
    void SetBCanshow(bool canshow) { bCanShow = canshow; }
    void SetBCannew(bool cannew) { bCanNew = cannew; }

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

    virtual void CheckItem(const OString& rMesId, bool bCheck = true) override;

    SfxTemplateDialog_Impl( SfxBindings*, SfxTemplatePanelControl* pDlgWindow );
    virtual ~SfxTemplateDialog_Impl() override;

    sal_Int8 AcceptToolbarDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);

    void Initialize();
    virtual void EnableItem(const OString& rMesId, bool bCheck = true) override;

    virtual bool IsCheckedItem(const OString& rMesId) override;
};

#endif // INCLUDED_SFX2_SOURCE_INC_TEMPLDGI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
