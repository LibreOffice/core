/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tablestyledlg.hxx>

#include <array>

#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/colorwindow.hxx>
#include <svx/svxids.hrc>
#include <vcl/virdev.hxx>
#include <tools/color.hxx>

#include <document.hxx>
#include <patattr.hxx>
#include <scitems.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <tablestyleswatchrender.hxx>

namespace
{
// The regions shown in the dialog, in display order, each with its label.
struct RegionEntry
{
    ScTableStyleElement eElement;
    TranslateId pLabel;
};

constexpr std::array<RegionEntry, 11> gaRegions{ {
    { ScTableStyleElement::WholeTable, STR_TABLESTYLE_ELEM_WHOLETABLE },
    { ScTableStyleElement::HeaderRow, STR_TABLESTYLE_ELEM_HEADERROW },
    { ScTableStyleElement::TotalRow, STR_TABLESTYLE_ELEM_TOTALROW },
    { ScTableStyleElement::FirstColumn, STR_TABLESTYLE_ELEM_FIRSTCOLUMN },
    { ScTableStyleElement::LastColumn, STR_TABLESTYLE_ELEM_LASTCOLUMN },
    { ScTableStyleElement::FirstRowStripe, STR_TABLESTYLE_ELEM_FIRSTROWSTRIPE },
    { ScTableStyleElement::SecondRowStripe, STR_TABLESTYLE_ELEM_SECONDROWSTRIPE },
    { ScTableStyleElement::FirstColumnStripe, STR_TABLESTYLE_ELEM_FIRSTCOLUMNSTRIPE },
    { ScTableStyleElement::SecondColumnStripe, STR_TABLESTYLE_ELEM_SECONDCOLUMNSTRIPE },
    { ScTableStyleElement::FirstHeaderCell, STR_TABLESTYLE_ELEM_FIRSTHEADERCELL },
    { ScTableStyleElement::LastHeaderCell, STR_TABLESTYLE_ELEM_LASTHEADERCELL },
} };

// The border-style combo steps and their line width in twips, in ascending
// width. borderWidthFor and borderIdForWidth are inverses of this one table.
struct BorderWidthPreset
{
    std::u16string_view pId;
    tools::Long nWidth;
};

constexpr std::array<BorderWidthPreset, 3> gaBorderWidths{ {
    { u"thin", 15 },
    { u"medium", 35 },
    { u"thick", 88 },
} };

tools::Long borderWidthFor(std::u16string_view rId)
{
    for (const BorderWidthPreset& rPreset : gaBorderWidths)
        if (rId == rPreset.pId)
            return rPreset.nWidth;
    return 0;
}

// The preset step whose width a line read back from a style falls into.
OUString borderIdForWidth(tools::Long nWidth)
{
    OUString aId{ gaBorderWidths.front().pId };
    for (const BorderWidthPreset& rPreset : gaBorderWidths)
        if (nWidth >= rPreset.nWidth)
            aId = OUString{ rPreset.pId };
    return aId;
}
}

ScTableStyleDlg::ScTableStyleDlg(weld::Window* pParent, ScDocument& rDoc)
    : GenericDialogController(pParent, u"modules/scalc/ui/newtablestyle.ui"_ustr,
                              u"NewTableStyleDialog"_ustr)
    , mrDoc(rDoc)
    , mxName(m_xBuilder->weld_entry(u"name"_ustr))
    , mxRegions(m_xBuilder->weld_tree_view(u"regions"_ustr))
    , mxBold(m_xBuilder->weld_check_button(u"bold"_ustr))
    , mxBorderStyle(m_xBuilder->weld_combo_box(u"borderstyle"_ustr))
    , mxClear(m_xBuilder->weld_button(u"clear"_ustr))
    , mxWarning(m_xBuilder->weld_label(u"warning"_ustr))
    , mxPreview(m_xBuilder->weld_image(u"preview"_ustr))
    , mxOk(m_xBuilder->weld_button(u"ok"_ustr))
    , mxBackColor(new ColorListBox(m_xBuilder->weld_menu_button(u"backcolor"_ustr),
                                   [this] { return m_xDialog.get(); }))
    , mxFontColor(new ColorListBox(m_xBuilder->weld_menu_button(u"fontcolor"_ustr),
                                   [this] { return m_xDialog.get(); }))
    , mxBorderColor(new ColorListBox(m_xBuilder->weld_menu_button(u"bordercolor"_ustr),
                                     [this] { return m_xDialog.get(); }))
{
    // The background offers a "None" choice (no fill); the text and border
    // colours default to automatic.
    mxBackColor->SetSlotId(SID_BACKGROUND_COLOR, true);
    mxFontColor->SetSlotId(SID_ATTR_CHAR_COLOR);
    mxBorderColor->SetSlotId(SID_ATTR_CHAR_COLOR);

    for (const RegionEntry& rRegion : gaRegions)
        mxRegions->append(OUString::number(static_cast<int>(rRegion.eElement)),
                          ScResId(rRegion.pLabel));

    mxName->set_text(ScResId(STR_TABLESTYLE_NEW_DEFAULTNAME));

    mxRegions->connect_selection_changed(LINK(this, ScTableStyleDlg, RegionSelectHdl));
    mxBackColor->SetSelectHdl(LINK(this, ScTableStyleDlg, ColorHdl));
    mxFontColor->SetSelectHdl(LINK(this, ScTableStyleDlg, ColorHdl));
    mxBorderColor->SetSelectHdl(LINK(this, ScTableStyleDlg, ColorHdl));
    mxBold->connect_toggled(LINK(this, ScTableStyleDlg, BoldHdl));
    mxBorderStyle->connect_changed(LINK(this, ScTableStyleDlg, BorderHdl));
    mxClear->connect_clicked(LINK(this, ScTableStyleDlg, ClearHdl));
    mxName->connect_changed(LINK(this, ScTableStyleDlg, NameHdl));
    mxOk->connect_clicked(LINK(this, ScTableStyleDlg, OkHdl));

    mxRegions->select(0);
    LoadRegion();
    UpdateOkState();
    RefreshPreview();
}

ScTableStyleDlg::~ScTableStyleDlg() {}

ScTableStyleElement ScTableStyleDlg::GetSelectedElement() const
{
    return static_cast<ScTableStyleElement>(mxRegions->get_selected_id().toInt32());
}

bool ScTableStyleDlg::HasSelectedElement() const { return mxRegions->get_selected_index() != -1; }

ScPatternAttr& ScTableStyleDlg::EnsurePattern(ScTableStyleElement eElement)
{
    auto it = maElementPatterns.find(eElement);
    if (it == maElementPatterns.end())
        it = maElementPatterns
                 .emplace(eElement, std::make_unique<ScPatternAttr>(mrDoc.getCellAttributeHelper()))
                 .first;
    return *it->second;
}

void ScTableStyleDlg::LoadRegion()
{
    mbLoading = true;

    const bool bHasRegion = HasSelectedElement();
    mxBackColor->set_sensitive(bHasRegion);
    mxFontColor->set_sensitive(bHasRegion);
    mxBold->set_sensitive(bHasRegion);
    mxBorderStyle->set_sensitive(bHasRegion);
    mxBorderColor->set_sensitive(bHasRegion);
    mxClear->set_sensitive(bHasRegion);

    // Defaults for an unstyled region. Selecting the "None" entry (rather than
    // SetNoSelection) also refreshes the button preview, so switching regions
    // and clearing a region reset the shown background.
    mxBackColor->SelectEntry(COL_NONE_COLOR);
    mxFontColor->SelectEntry(COL_AUTO);
    mxBorderColor->SelectEntry(COL_AUTO);
    mxBold->set_active(false);
    mxBorderStyle->set_active_id(u"none"_ustr);

    if (bHasRegion)
    {
        auto it = maElementPatterns.find(GetSelectedElement());
        if (it != maElementPatterns.end())
        {
            const SfxItemSet& rSet = it->second->GetItemSet();
            if (const SvxBrushItem* pBrush = rSet.GetItemIfSet(ATTR_BACKGROUND, false))
                mxBackColor->SelectEntry(pBrush->GetColor());
            if (const SvxColorItem* pColor = rSet.GetItemIfSet(ATTR_FONT_COLOR, false))
                mxFontColor->SelectEntry(pColor->GetValue());
            if (const SvxWeightItem* pWeight = rSet.GetItemIfSet(ATTR_FONT_WEIGHT, false))
                mxBold->set_active(pWeight->GetWeight() == WEIGHT_BOLD);
            if (const SvxBoxItem* pBox = rSet.GetItemIfSet(ATTR_BORDER, false))
            {
                if (const editeng::SvxBorderLine* pLine = pBox->GetTop())
                {
                    mxBorderColor->SelectEntry(pLine->GetColor());
                    mxBorderStyle->set_active_id(borderIdForWidth(pLine->GetWidth()));
                }
            }
        }
    }

    mbLoading = false;
}

void ScTableStyleDlg::ApplyPickersToRegion()
{
    if (mbLoading || !HasSelectedElement())
        return;

    SfxItemSet& rSet = EnsurePattern(GetSelectedElement()).GetItemSetWritable();

    if (mxBackColor->GetSelectEntryColor() == COL_NONE_COLOR)
        rSet.ClearItem(ATTR_BACKGROUND);
    else
        rSet.Put(SvxBrushItem(mxBackColor->GetSelectEntryColor(), ATTR_BACKGROUND));

    const Color aFontColor = mxFontColor->GetSelectEntryColor();
    if (aFontColor == COL_AUTO)
        rSet.ClearItem(ATTR_FONT_COLOR);
    else
        rSet.Put(SvxColorItem(aFontColor, ATTR_FONT_COLOR));

    if (mxBold->get_active())
    {
        rSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_FONT_WEIGHT));
        rSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_CJK_FONT_WEIGHT));
        rSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_CTL_FONT_WEIGHT));
    }
    else
    {
        rSet.ClearItem(ATTR_FONT_WEIGHT);
        rSet.ClearItem(ATTR_CJK_FONT_WEIGHT);
        rSet.ClearItem(ATTR_CTL_FONT_WEIGHT);
    }

    const tools::Long nBorderWidth = borderWidthFor(mxBorderStyle->get_active_id());
    if (nBorderWidth == 0)
    {
        rSet.ClearItem(ATTR_BORDER);
    }
    else
    {
        Color aBorderColor = mxBorderColor->GetSelectEntryColor();
        if (aBorderColor == COL_AUTO)
            aBorderColor = COL_BLACK;
        editeng::SvxBorderLine aLine(&aBorderColor);
        aLine.SetBorderLineStyle(SvxBorderLineStyle::SOLID);
        aLine.SetWidth(nBorderWidth);
        SvxBoxItem aBox(ATTR_BORDER);
        aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
        aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);
        aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
        aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
        rSet.Put(aBox);
    }

    RefreshPreview();
}

void ScTableStyleDlg::RefreshPreview()
{
    std::unique_ptr<ScTableStyle> pStyle = BuildStyle(u"preview"_ustr);
    const Size aSize(64, 48);
    Bitmap aSwatch = ScRenderTableStyleSwatchBitmap(*pStyle, aSize);

    ScopedVclPtr<VirtualDevice> pDevice(VclPtr<VirtualDevice>::Create());
    pDevice->SetOutputSizePixel(aSize);
    if (!aSwatch.IsEmpty())
        pDevice->DrawBitmap(Point(), aSwatch);
    mxPreview->set_image(pDevice.get());
}

bool ScTableStyleDlg::IsNameFree(std::u16string_view rUIName) const
{
    const ScTableStyles* pStyles = mrDoc.GetTableStyles();
    if (!pStyles)
        return true;
    for (const ScTableStyle* pStyle : pStyles->GetSortedTableStyles())
    {
        if (pStyle->GetUIName() == rUIName)
            return false;
    }
    return true;
}

void ScTableStyleDlg::UpdateOkState()
{
    const OUString aName = mxName->get_text().trim();
    const bool bFree = !aName.isEmpty() && IsNameFree(aName);
    mxWarning->set_visible(!aName.isEmpty() && !bFree);
    mxOk->set_sensitive(bFree);
}

std::unique_ptr<ScTableStyle> ScTableStyleDlg::BuildStyle(const OUString& rProgrammaticName) const
{
    auto pStyle = std::make_unique<ScTableStyle>(
        rProgrammaticName, std::optional<OUString>(mxName->get_text().trim()));
    for (const auto & [ eElement, pPattern ] : maElementPatterns)
    {
        if (pPattern && pPattern->GetItemSet().Count() > 0)
            pStyle->SetPattern(eElement, std::make_unique<ScPatternAttr>(*pPattern));
    }
    return pStyle;
}

IMPL_LINK_NOARG(ScTableStyleDlg, RegionSelectHdl, weld::TreeView&, void) { LoadRegion(); }

IMPL_LINK_NOARG(ScTableStyleDlg, ColorHdl, ColorListBox&, void) { ApplyPickersToRegion(); }

IMPL_LINK_NOARG(ScTableStyleDlg, BoldHdl, weld::Toggleable&, void) { ApplyPickersToRegion(); }

IMPL_LINK_NOARG(ScTableStyleDlg, BorderHdl, weld::ComboBox&, void) { ApplyPickersToRegion(); }

IMPL_LINK_NOARG(ScTableStyleDlg, ClearHdl, weld::Button&, void)
{
    if (HasSelectedElement())
        maElementPatterns.erase(GetSelectedElement());
    LoadRegion();
    RefreshPreview();
}

IMPL_LINK_NOARG(ScTableStyleDlg, NameHdl, weld::Entry&, void) { UpdateOkState(); }

IMPL_LINK_NOARG(ScTableStyleDlg, OkHdl, weld::Button&, void)
{
    const ScTableStyles* pStyles = mrDoc.GetTableStyles();
    if (!pStyles)
        return;

    // A new style gets a fresh programmatic id outside the built-in families so
    // it counts as Custom. The dialog only builds the style here; the caller
    // registers it with the document.
    mxResultStyle = BuildStyle(pStyles->GetUnusedCustomStyleName());
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
