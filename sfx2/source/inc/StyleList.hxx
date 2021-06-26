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

enum class StyleFlags {
    NONE=0, UpdateFamilyList=1, UpdateFamily=2
};

namespace o3tl {
    template<> struct typed_flags<StyleFlags> : is_typed_flags<StyleFlags, 3> {};
}

constexpr int MAX_FAMILIES = 6;

class StyleList
{
public:

    StyleList(weld::Builder* pBuilder, const OString& flatviewname, const OString& hierViewName,
              bool bAllow, SfxStyleSheetBasePool* pPool, bool exampleDisabled, bool dontUpdate,
              sal_uInt16 nActfam, bool btreeDrag, std::optional<SfxStyleFamilies> mxFamilies,
              bool bCanEdit, bool bCanHide, bool bCanShow, bool bUpdateDisabled,
              SfxStyleSearchBits sfxStyleSearchBits, bool m_bWantHierarchical,
              const OString& filtername, const OString& checkboxname);

    void setStyleFamily(SfxStyleFamily family);
    void setPreview(bool bPreview);
    void setFilter(weld::ComboBox& combobox);

    ~StyleList();

    bool IsSafeForWaterCan() const;
    void SelectStyle(const OUString& rStr, bool bIsCallback, SfxStyleFamily eFam);
    void FillTreeBox(SfxStyleFamily eFam);
    void EnableHierarchical(bool const bEnable);
    OUString getDefaultStyleName(const SfxStyleFamily eFam);
    weld::TreeView* PrepareMenu(const Point& rPos);
    weld::TreeView* ShowMenu();
    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt, const DropTargetHelper& rHelper);
    const SfxStyleFamilyItem* GetFamilyItem_Impl() const;
    void GetSelectedStyle() const;
    OUString GetSelectedEntry() const;
    bool HasSelectedStyle() const;
    SfxStyleFamily GetActualFamily() const;
    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);
    void DropHdl(const OUString& rStyle, const OUString& rParent);
    SfxStyleSearchBits nAppFilter;
    void FilterSelect(
        sal_uInt16 nEntry,
        bool bForce);

    // In which FamilyState do I have to look, in order to get the
    // information of the ith Family in the pStyleFamilies.
    sal_uInt16 StyleNrToInfoOffset( sal_uInt16 i );

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
    void UpdateStyles_Impl(StyleFlags nFlags);

    void NewHdl();
    void EditHdl();
    void DeleteHdl();
    void HideHdl();
    void ShowHdl();

    bool bHierarchical :1;
    bool m_bWantHierarchical;
    SfxCommonTemplateDialog_Impl* pParentDialog;

    bool bAllowReParentDrop:1;
    bool bNewByExampleDisabled :1;
    bool bUpdateByExampleDisabled :1;
    bool bDontUpdate :1;
    bool bTreeDrag :1;
    SfxStyleSheetBasePool* pStyleSheetPool;
    sal_uInt16 nActFilter;
    std::unique_ptr<weld::TreeView> mxFmtLb;
    std::unique_ptr<weld::TreeView> mxTreeBox;

    std::optional<SfxStyleFamilies> mxStyleFamilies;
    bool bCanEdit :1;
    bool bCanHide :1;
    bool bCanShow :1;
    std::array<std::unique_ptr<SfxTemplateItem>, MAX_FAMILIES> pFamilyState;
    std::unique_ptr<weld::ComboBox> mxFilterLb;
    std::unique_ptr<weld::CheckButton> mxPreviewCheckbox;
    SfxObjectShell* pCurObjShell;
    sal_uInt16 nActFamily;
};
