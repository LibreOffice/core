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

#include <array>
#include <memory>
#include <optional>

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svl/lstner.hxx>
#include <svl/eitem.hxx>

#include <svl/style.hxx>

#include <vcl/idle.hxx>

#include <sfx2/docfac.hxx>
#include <vcl/svapp.hxx>

class SfxObjectShell;
class SfxStyleFamilyItem;
class SfxTemplateItem;
class SfxCommonTemplateDialog_Impl;


namespace weld
{

class VCL_DLLPUBLIC StyleList
{
private:

protected:

public:

#define MAX_FAMILIES            6

    StyleList(weld::Builder* pBuilder, const OString& flatviewname, const OString& hierViewName, bool bAllow, SfxStyleSheetBasePool* pPool, bool exampleDisabled , bool dontUpdate, sal_uInt16 nActfam, bool btreeDrag, std::optional<SfxStyleFamilies> mxFamilies, bool bCanEdit, bool bCanHide, bool bCanShow, bool bUpdateDisabled, SfxStyleSearchBits sfxStyleSearchBits, sal_uInt16 nActfilter, bool bHeirarchical, bool m_bWantHierarchical)
    {
        mxFmtLb = pBuilder->weld_tree_view(flatviewname);
        mxTreeBox = pBuilder->weld_tree_view(hierViewName);
        bAllowReParentDrop = bAllow;
        pPool = pStyleSheetPool;
        bNewByExampleDisabled = exampleDisabled;
        bDontUpdate = dontUpdate;
        nActFamily = nActfam;
        btreeDrag = bTreeDrag;
        mxStyleFamilies = mxFamilies;
        this->bCanEdit = bCanEdit;
        this->bCanHide = bCanHide;
        this->bCanShow = bCanShow;
        this->bUpdateByExampleDisabled = bUpdateDisabled;
        this->nAppFilter = sfxStyleSearchBits;
        this->nActFilter = nActFilter;
        this->bHierarchical = bHierarchical;
        this->m_bWantHierarchical = m_bWantHierarchical;
    }
    void setStyleFamily(SfxStyleFamily family);
    void setPreview(bool bPreview);
    void setFilter(weld::ComboBox& combobox);
    bool bAllowReParentDrop:1;
    bool bNewByExampleDisabled :1;
    bool bUpdateByExampleDisabled :1;
    bool bDontUpdate :1;
    bool bTreeDrag :1;
    SfxStyleSheetBasePool* pStyleSheetPool;
    bool IsSafeForWaterCan() const;
    void SelectStyle(const OUString& rStr, bool bIsCallback, SfxStyleFamily eFam);
    void FillTreeBox(SfxStyleFamily eFam);
    OUString getDefaultStyleName(const SfxStyleFamily eFam);
    weld::TreeView* PrepareMenu(const Point& rPos);
    sal_uInt16 nActFilter;

    weld::TreeView* ShowMenu(const CommandEvent& rCEvt);
    std::unique_ptr<weld::TreeView> mxFmtLb;
    std::unique_ptr<weld::TreeView> mxTreeBox;
    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);
    const SfxStyleFamilyItem* GetFamilyItem_Impl() const;
    void GetSelectedStyle() const;
    OUString GetSelectedEntry() const;
    bool HasSelectedStyle() const;
    //void UpdateStyles_Impl(StyleFlags nFlags);
    void EnableHierarchical(bool const bEnable, SfxStyleFamily eFam);
    void SetFamily(SfxStyleFamily const nFamily);
    SfxStyleFamily GetActualFamily() const;
    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);
    void DropHdl(const OUString& rStyle, const OUString& rParent);
    sal_uInt16 nActFamily;
    SfxStyleSearchBits nAppFilter;

    // In which FamilyState do I have to look, in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16 StyleNrToInfoOffset( sal_uInt16 i );
    std::optional<SfxStyleFamilies> mxStyleFamilies;
    bool bCanEdit :1;
    bool bCanHide :1;
    bool bCanShow :1;
    virtual void EnableEdit( bool b )
    {
        bCanEdit = b;
    }
    void EnableHide( bool b )
    {
        bCanHide = b;
    }
    void EnableShow( bool b )
    {
        bCanShow = b;
    }
    std::array<std::unique_ptr<SfxTemplateItem>, MAX_FAMILIES> pFamilyState;
    SfxObjectShell* pCurObjShell;

    SfxObjectShell* GetObjectShell() const
    {
        return pCurObjShell;
    }
    void SetObjectShell(SfxObjectShell* shell)
    {
        pCurObjShell = shell;
    }
    void NAppFilter(SfxStyleSearchBits filter)
    {
        nAppFilter = filter;
    }
    void NActFilter(sal_uInt16 filter)
    {
        nActFilter = filter;
    }

    bool bHierarchical :1;
    bool m_bWantHierarchical;
    SfxCommonTemplateDialog_Impl* pParentDialog;

};
}
