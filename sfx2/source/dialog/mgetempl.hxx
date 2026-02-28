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
#ifndef INCLUDED_SFX2_MGETEMPL_HXX
#define INCLUDED_SFX2_MGETEMPL_HXX

#include <sfx2/styfitem.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/weld/Toolbar.hxx>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

namespace weld { class Button; }
namespace weld { class CheckButton; }
namespace weld { class ComboBox; }
namespace weld { class Entry; }
namespace weld { class Label; }
namespace weld { class Widget; }

namespace weld { class Container; }
namespace weld { class Box; }

class SfxManageStyleSheetPage;

/// A "chip" widget representing a single style property that can be reset to parent
class PropertyChip final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Toolbar> m_xRemoveBtn;;

    SfxManageStyleSheetPage* m_pPage;
    OUString m_sText;
    sal_uInt16 m_nWhich;

    DECL_LINK(RemoveHdl, const OUString&, void);

public:
    PropertyChip(weld::Box* pParent, SfxManageStyleSheetPage* pPage,
                 sal_uInt16 nWhich, const OUString& rText);
    ~PropertyChip();

    sal_uInt16 GetWhich() const { return m_nWhich; }
    const OUString& GetText() const { return m_sText; }
    SfxManageStyleSheetPage* GetPage() const { return m_pPage; }
};
/* expected:
    SID_TEMPLATE_NAME   :   In: StringItem, Name of Template
    SID_TEMPLATE_FAMILY :   In: Family of Template
*/

/// A row containing chips for a single tab/category
class PropertyCategoryRow final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Container> m_xChipsBox;
    std::vector<std::unique_ptr<PropertyChip>> m_aChips;

    struct ChipRow
    {
        std::unique_ptr<weld::Builder> xBuilder;
        std::unique_ptr<weld::Box> xBox;
        int nTotalChars = 0;  // estimated total character width
    };
    std::vector<std::unique_ptr<ChipRow>> m_aChipRows;
    static constexpr int MAX_CHARS_PER_ROW = 100;

    weld::Box* EnsureCurrentRow(int nChipChars);

public:
    PropertyCategoryRow(weld::Box* pParentBox, std::u16string_view rLabel);
    ~PropertyCategoryRow();
    void AddChip(SfxManageStyleSheetPage* pPage, sal_uInt16 nWhich, const OUString& rText);
    void RemoveChip(sal_uInt16 nWhich);
    bool IsEmpty() const { return m_aChips.empty(); }
    void Show() { if (m_xContainer) m_xContainer->set_visible(true); }
    void Hide() { if (m_xContainer) m_xContainer->set_visible(false); }
    OUString GetLabel() const { return m_xLabel->get_label(); }
    int GetChipRowCount() const { return static_cast<int>(m_aChipRows.size()); }
};

class SfxManageStyleSheetPage final : public SfxTabPage
{
    SfxStyleSheetBase *pStyle;
    SfxStyleFamilies maFamilies;
    const SfxStyleFamilyItem *pItem;
    OUString aBuf;
    bool bModified;

    // initial data for the style
    OUString aName;
    OUString aFollow;
    OUString aParent;
    SfxStyleSearchBits nFlags;

    std::unique_ptr<weld::Entry> m_xName;
    std::unique_ptr<weld::CheckButton> m_xAutoCB;
    std::unique_ptr<weld::Label> m_xFollowFt;
    std::unique_ptr<weld::ComboBox> m_xFollowLb;
    std::unique_ptr<weld::Button> m_xEditStyleBtn;
    std::unique_ptr<weld::Label> m_xBaseFt;
    std::unique_ptr<weld::ComboBox>          m_xBaseLb;
    std::unique_ptr<weld::Button> m_xEditLinkStyleBtn;
    std::unique_ptr<weld::Label> m_xFilterFt;
    std::unique_ptr<weld::ComboBox> m_xFilterLb;
    std::unique_ptr<weld::Label> m_xDescFt;
    std::unique_ptr<weld::Box> m_xEditViewBox;
    std::unique_ptr<weld::Toggleable> m_xEditPropsBtn;
    std::unique_ptr<weld::Toggleable> m_xViewPropsBtn;
    std::unique_ptr<weld::Box> m_xPropBox;
    std::map<OUString, std::unique_ptr<PropertyCategoryRow>> m_aPropertyRows;
    std::set<sal_uInt16> m_aResetWhichIds;  // Track Which IDs that user has reset
    bool m_bEditMode = false;               // True after user clicks Edit
    bool m_bInToggleHandler = false;

    friend class SfxStyleDialogController;
    friend class PropertyChip;

    DECL_LINK(GetFocusHdl, weld::Widget&, void);
    DECL_LINK(LoseFocusHdl, weld::Widget&, void);
    DECL_LINK(EditStyleSelectHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(EditStyleHdl_Impl, weld::Button&, void);
    DECL_LINK(EditLinkStyleSelectHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(EditLinkStyleHdl_Impl, weld::Button&, void);
    DECL_LINK(EditPropsHdl_Impl, weld::Toggleable&, void);
    DECL_LINK(ViewPropsHdl_Impl, weld::Toggleable&, void);
    void    UpdateName_Impl(weld::ComboBox*, const OUString &rNew);
    void    SetDescriptionText_Impl();

    void    BuildPropertyChips_Impl();
    void    ResetPropertyToParent(sal_uInt16 nWhich);
    virtual void ActivatePage(const SfxItemSet& rSet) override;
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );

    virtual bool        FillItemSet(SfxItemSet *) override;
    virtual void        Reset(const SfxItemSet *) override;

    static bool    Execute_Impl( sal_uInt16 nId, const OUString& rStr, sal_uInt16 nFamily );
    virtual DeactivateRC DeactivatePage(SfxItemSet *) override;

public:
    SfxManageStyleSheetPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rAttrSet);
    virtual ~SfxManageStyleSheetPage() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
