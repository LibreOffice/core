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

#include <colorfillpage.hxx>
#include <cuitabarea.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

#include <algorithm>

#include <comphelper/configuration.hxx>
#include <comphelper/sequence.hxx>
#include <o3tl/safeint.hxx>
#include <officecfg/Office/Common.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxdlg.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <vcl/ColorDialog.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wall.hxx>

using namespace css;

namespace
{
// The palette list entry that stands for no fill.
constexpr OUString NONE_ID = u"none"_ustr;
// The palette manager lists the user's own palette first.
constexpr sal_Int32 CUSTOM_PALETTE = 0;

// The position of the selected swatch, which each entry carries as its id, or -1.
sal_Int32 lclSelectedPosition(const weld::IconView& rView)
{
    const OUString sId = rView.get_selected_id();
    return sId.isEmpty() ? -1 : sId.toInt32();
}

// The color at nPosition, or nullptr past either end.
const NamedColor* lclColorAt(const std::vector<NamedColor>& rColors, sal_Int32 nPosition)
{
    if (nPosition < 0 || o3tl::make_unsigned(nPosition) >= rColors.size())
        return nullptr;
    return &rColors[nPosition];
}

// The name of the swatch at rIter of rView, whose entries carry their position as id.
OUString lclColorName(const weld::IconView& rView, const std::vector<NamedColor>& rColors,
                      const weld::TreeIter& rIter)
{
    const OUString sId = rView.get_id(rIter);
    const NamedColor* pColor = lclColorAt(rColors, sId.isEmpty() ? -1 : sId.toInt32());
    return pColor ? pColor->m_aName : OUString();
}

// A swatch of rColor with a gray border and rounded corners.
ScopedVclPtr<VirtualDevice> lclCreateSwatch(const Color& rColor)
{
    ScopedVclPtr<VirtualDevice> pDevice(VclPtr<VirtualDevice>::Create(DeviceFormat::WITH_ALPHA));
    pDevice->SetOutputSizePixel(Size(16, 16));
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->Erase();
    pDevice->SetLineColor(COL_GRAY);
    pDevice->SetFillColor(rColor);
    pDevice->DrawRect(tools::Rectangle(Point(1, 1), Size(14, 14)), 2, 2);
    return pDevice;
}

// The colors of the custom palette and their names, as the user profile keeps them.
struct CustomPalette
{
    std::vector<sal_Int32> m_aColors;
    std::vector<OUString> m_aNames;

    static CustomPalette Load()
    {
        return { comphelper::sequenceToContainer<std::vector<sal_Int32>>(
                     officecfg::Office::Common::UserColors::CustomColor::get()),
                 comphelper::sequenceToContainer<std::vector<OUString>>(
                     officecfg::Office::Common::UserColors::CustomColorName::get()) };
    }

    void Save() const
    {
        auto xChanges = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::UserColors::CustomColor::set(
            comphelper::containerToSequence(m_aColors), xChanges);
        officecfg::Office::Common::UserColors::CustomColorName::set(
            comphelper::containerToSequence(m_aNames), xChanges);
        xChanges->commit();
    }

    bool HasName(std::u16string_view aName) const
    {
        return std::find(m_aNames.begin(), m_aNames.end(), aName) != m_aNames.end();
    }
};
}

SvxColorFillTabPage::SvxColorFillTabPage(weld::Container* pPage,
                                         weld::DialogController* pController,
                                         const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/colorfillpage.ui"_ustr, u"ColorFillPage"_ustr,
                 &rInAttrs)
    , m_aColorFill(rInAttrs.getPool(), WhichRangesContainer(XATTR_FILL_FIRST, XATTR_FILL_LAST))
    , m_xPaletteList(m_xBuilder->weld_combo_box(u"paletteselector"_ustr))
    , m_xColors(m_xBuilder->weld_icon_view(u"iconview_colors"_ustr))
    , m_xRecentColors(m_xBuilder->weld_icon_view(u"iconview_recent_colors"_ustr))
    , m_xAdd(m_xBuilder->weld_button(u"add"_ustr))
    , m_xRemove(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xCustomColor(m_xBuilder->weld_button(u"customcolor"_ustr))
    , m_xCustomPattern(m_xBuilder->weld_button(u"custompattern"_ustr))
    , m_xMoreEffects(m_xBuilder->weld_button(u"moreeffects"_ustr))
    , m_xPatternSlot(m_xBuilder->weld_container(u"patternstyleslot"_ustr))
    , m_xCaption(m_xBuilder->weld_label(u"previewcaption"_ustr))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, m_aPreview))
{
    // A strip under the panels, as wide as the page.
    m_aPreview.set_size_request(m_xCustomColor->get_approximate_digit_width() * 40,
                                m_xCustomColor->get_text_height() * 5);

    m_aColorFill.Put(XFillStyleItem(drawing::FillStyle_SOLID));
    SetColor(NamedColor(COL_BLACK, OUString()));

    m_xPaletteList->connect_changed(LINK(this, SvxColorFillTabPage, PaletteSelectedHdl));
    m_xColors->connect_selection_changed(LINK(this, SvxColorFillTabPage, ColorSelectedHdl));
    m_xRecentColors->connect_selection_changed(LINK(this, SvxColorFillTabPage, ColorSelectedHdl));
    m_xColors->connect_query_tooltip(LINK(this, SvxColorFillTabPage, ColorTooltipHdl));
    m_xRecentColors->connect_query_tooltip(LINK(this, SvxColorFillTabPage, RecentColorTooltipHdl));
    m_xAdd->connect_clicked(LINK(this, SvxColorFillTabPage, AddHdl));
    m_xRemove->connect_clicked(LINK(this, SvxColorFillTabPage, RemoveHdl));
    m_xCustomColor->connect_clicked(LINK(this, SvxColorFillTabPage, CustomColorHdl));

    m_aPalettes.ReloadRecentColorSet(*m_xRecentColors);
    m_aRecentColors = m_aPalettes.GetRecentColors();

    SetExchangeSupport();
}

SvxColorFillTabPage::~SvxColorFillTabPage() { m_xPreview.reset(); }

std::unique_ptr<SfxTabPage> SvxColorFillTabPage::Create(weld::Container* pPage,
                                                        weld::DialogController* pController,
                                                        const SfxItemSet* pAttrs)
{
    return std::make_unique<SvxColorFillTabPage>(pPage, pController, *pAttrs);
}

void SvxColorFillTabPage::Construct()
{
    if (m_xColorList.is())
        FillPaletteList();
}

sal_Int32 SvxColorFillTabPage::SelectedPalette() const
{
    const OUString sId = m_xPaletteList->get_active_id();
    return sId.isEmpty() || sId == NONE_ID ? -1 : sId.toInt32();
}

void SvxColorFillTabPage::FillPaletteList()
{
    m_xPaletteList->clear();
    if (m_aNoneSelectedHdl.IsSet())
        m_xPaletteList->append(NONE_ID, CuiResId(RID_CUISTR_FILL_NONE));
    const std::vector<OUString> aPaletteNames = m_aPalettes.GetPaletteList();
    for (size_t nPalette = 0; nPalette < aPaletteNames.size(); ++nPalette)
        m_xPaletteList->append(OUString::number(nPalette), aPaletteNames[nPalette]);

    m_xPaletteList->set_active_text(officecfg::Office::Common::UserColors::PaletteName::get());
    const sal_Int32 nPalette = SelectedPalette();
    if (nPalette >= 0)
        ShowPalette(nPalette);
}

void SvxColorFillTabPage::ShowPalette(sal_Int32 nPalette)
{
    m_nPalette = nPalette;
    m_aPalettes.SetPalette(nPalette);
    m_xColors->clear();
    m_aPalettes.ReloadColorSet(*m_xColors);
    m_aPaletteColors = m_aPalettes.GetColors();
    ShareColorList();
    SelectCurrentColor();
    UpdateButtons();
}

void SvxColorFillTabPage::ShareColorList()
{
    // The custom palette and the last one are made by the palette manager, not read from a file.
    if (m_nPalette == CUSTOM_PALETTE || m_nPalette == m_aPalettes.GetPaletteCount() - 1)
        return;

    XColorListRef xList = XPropertyList::AsColorList(XPropertyList::CreatePropertyListFromURL(
        XPropertyListType::Color, m_aPalettes.GetSelectedPalettePath()));
    xList->SetName(m_aPalettes.GetPaletteName());
    if (!xList->Load())
        return;

    // The other pages of the dialog offer the colors of the palette too.
    m_xColorList = xList;
    if (auto* pAreaDialog = dynamic_cast<SvxAreaTabDialog*>(GetDialogController()))
        pAreaDialog->SetNewColorList(xList);
    if (m_pColorListState)
    {
        *m_pColorListState |= ChangeType::CHANGED;
        *m_pColorListState &= ~ChangeType::MODIFIED;
    }
}

void SvxColorFillTabPage::SelectCurrentColor()
{
    m_xColors->unselect_all();
    // Without a fill no swatch is selected, so a click on any of them sets one.
    if (m_bNoFill)
        return;
    for (size_t nPosition = 0; nPosition < m_aPaletteColors.size(); ++nPosition)
    {
        if (m_aPaletteColors[nPosition].m_aColor == m_aColor.m_aColor)
        {
            m_xColors->select(static_cast<int>(nPosition));
            return;
        }
    }
}

void SvxColorFillTabPage::SetColor(const NamedColor& rColor)
{
    m_aColor = rColor;
    XFillColorItem aItem(OUString(), m_aColor.m_aColor);
    aItem.setComplexColor(m_aColor.getComplexColor());
    m_aColorFill.Put(aItem);

    ScopedVclPtr<VirtualDevice> pSwatch = lclCreateSwatch(m_aColor.m_aColor);
    m_xAdd->set_image(pSwatch.get());
    m_xRemove->set_image(pSwatch.get());
}

void SvxColorFillTabPage::UpdateButtons()
{
    const bool bNoFill = m_xPaletteList->get_active_id() == NONE_ID;
    m_xAdd->set_sensitive(!bNoFill);

    // Only a color of the custom palette can be removed from it.
    const bool bCustomColor = !bNoFill && m_nPalette == CUSTOM_PALETTE
                              && lclSelectedPosition(*m_xColors) >= 0;
    m_xRemove->set_sensitive(bCustomColor);
    if (bCustomColor)
        m_xRemove->set_tooltip_text(OUString());
    else if (lclSelectedPosition(*m_xRecentColors) >= 0)
        m_xRemove->set_tooltip_text(CuiResId(RID_CUISTR_DELETEUSERCOLOR2));
    else
        m_xRemove->set_tooltip_text(CuiResId(RID_CUISTR_DELETEUSERCOLOR1));
}

void SvxColorFillTabPage::SetNoFill(bool bNoFill)
{
    if (!m_aNoneSelectedHdl.IsSet())
        return;

    m_bNoFill = bNoFill;
    if (!bNoFill)
    {
        if (m_xPaletteList->get_active_id() == NONE_ID && m_nPalette >= 0)
            m_xPaletteList->set_active_id(OUString::number(m_nPalette));
    }
    else
    {
        // The swatches of the palette stay, so a click on one fills with it again.
        m_xPaletteList->set_active_id(NONE_ID);
        m_xColors->unselect_all();
        m_xRecentColors->unselect_all();
    }
    UpdateButtons();
}

void SvxColorFillTabPage::UpdatePreview()
{
    if (m_aUpdatePreviewHdl.IsSet())
        m_aUpdatePreviewHdl.Call(*this);
    else
        ShowPreview(m_aColorFill, GetColorCaption());
}

void SvxColorFillTabPage::ShowPreview(const SfxItemSet& rFill, const OUString& rCaption)
{
    m_aPreview.SetAttributes(rFill);
    m_aPreview.Invalidate();
    m_xCaption->set_label(rCaption);
    m_aPreview.GetDrawingArea()->set_accessible_description(rCaption);
}

OUString SvxColorFillTabPage::GetColorName() const
{
    if (!m_aColor.m_aName.isEmpty())
        return m_aColor.m_aName;
    auto aFoundColor = std::find_if(
        m_aPaletteColors.begin(), m_aPaletteColors.end(),
        [this](const NamedColor& rColor) { return rColor.m_aColor == m_aColor.m_aColor; });
    return aFoundColor != m_aPaletteColors.end() ? aFoundColor->m_aName : OUString();
}

OUString SvxColorFillTabPage::GetColorCaption() const
{
    const OUString sHex = "#" + m_aColor.m_aColor.AsRGBHexString().toAsciiUpperCase();
    const OUString sName = GetColorName();
    if (sName.isEmpty() || sName == sHex)
        return CuiResId(RID_CUISTR_PREVIEW_COLOR).replaceFirst("%1", sHex);
    return CuiResId(RID_CUISTR_PREVIEW_NAMED_COLOR).replaceFirst("%1", sName).replaceFirst("%2",
                                                                                          sHex);
}

void SvxColorFillTabPage::HidePatternControls()
{
    // More effects moves to the row of Custom color, and the row it leaves holds nothing to show.
    std::unique_ptr<weld::Container> xFillButtons(m_xBuilder->weld_container(u"fillbuttonbox"_ustr));
    std::unique_ptr<weld::Container> xColorButtons(
        m_xBuilder->weld_container(u"colorbuttonbox"_ustr));
    xFillButtons->move(m_xMoreEffects.get(), xColorButtons.get());
    xFillButtons->hide();
    m_xCustomPattern->hide();
    m_xPatternSlot->hide();
}

bool SvxColorFillTabPage::FillItemSet(SfxItemSet* pSet)
{
    OUString sName = GetColorName();
    if (sName.isEmpty())
        sName = "#" + m_aColor.m_aColor.AsRGBHexString().toAsciiUpperCase();

    XFillColorItem aItem(sName, m_aColor.m_aColor);
    aItem.setComplexColor(m_aColor.getComplexColor());
    pSet->Put(aItem);
    pSet->Put(XFillStyleItem(drawing::FillStyle_SOLID));

    m_aPalettes.AddRecentColor(m_aColor.m_aColor, sName);
    m_aRecentColors = m_aPalettes.GetRecentColors();
    return true;
}

void SvxColorFillTabPage::Reset(const SfxItemSet* pSet)
{
    Color aColor;
    if (pSet->GetItemState(XATTR_FILLCOLOR) >= SfxItemState::DEFAULT)
        aColor = pSet->Get(XATTR_FILLCOLOR).GetColorValue();

    SetColor(NamedColor(aColor, OUString()));
    SelectCurrentColor();
    UpdateButtons();
    UpdatePreview();
}

void SvxColorFillTabPage::ActivatePage(const SfxItemSet&)
{
    if (!m_xColorList.is())
        return;
    SelectCurrentColor();
    UpdateButtons();
}

DeactivateRC SvxColorFillTabPage::DeactivatePage(SfxItemSet* pSet)
{
    if (pSet)
        FillItemSet(pSet);
    return DeactivateRC::LeavePage;
}

IMPL_LINK_NOARG(SvxColorFillTabPage, PaletteSelectedHdl, weld::ComboBox&, void)
{
    if (m_xPaletteList->get_active_id() == NONE_ID)
    {
        SetNoFill(true);
        m_aNoneSelectedHdl.Call(*this);
    }
    else
        ShowPalette(SelectedPalette());
    UpdatePreview();
}

IMPL_LINK(SvxColorFillTabPage, ColorSelectedHdl, weld::IconView&, rView, void)
{
    const bool bRecent = &rView == m_xRecentColors.get();
    const sal_Int32 nPosition = lclSelectedPosition(rView);
    const NamedColor* pColor = lclColorAt(bRecent ? m_aRecentColors : m_aPaletteColors, nPosition);
    if (!pColor)
    {
        UpdateButtons();
        return;
    }

    NamedColor aColor = *pColor;
    if (!bRecent && m_aPalettes.IsThemePaletteSelected())
    {
        // A theme color keeps its place in the theme, so it follows a later change of the theme.
        sal_uInt16 nTheme;
        sal_uInt16 nEffect;
        if (PaletteManager::GetThemeAndEffectIndex(nPosition, nTheme, nEffect))
        {
            aColor.m_nThemeIndex = nTheme;
            m_aPalettes.GetLumModOff(nTheme, nEffect, aColor.m_nLumMod, aColor.m_nLumOff);
        }
    }

    // One color is selected at a time, in either list.
    if (bRecent)
        m_xColors->unselect_all();
    else
        m_xRecentColors->unselect_all();

    SetColor(aColor);
    m_aColorChangedHdl.Call(*this);
    UpdateButtons();
    UpdatePreview();
}

IMPL_LINK(SvxColorFillTabPage, ColorTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    return lclColorName(*m_xColors, m_aPaletteColors, rIter);
}

IMPL_LINK(SvxColorFillTabPage, RecentColorTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    return lclColorName(*m_xRecentColors, m_aRecentColors, rIter);
}

IMPL_LINK_NOARG(SvxColorFillTabPage, AddHdl, weld::Button&, void)
{
    // The first of "Color 1", "Color 2" and so on that the custom palette does not have yet.
    const CustomPalette aPalette = CustomPalette::Load();
    const OUString sBase = SvxResId(RID_SVXSTR_COLOR);
    OUString sName;
    for (sal_Int32 nNumber = 1; sName.isEmpty() || aPalette.HasName(sName); ++nNumber)
        sName = sBase + " " + OUString::number(nNumber);
    AskForColorName(sName);
}

void SvxColorFillTabPage::AskForColorName(const OUString& rSuggestedName)
{
    SvxAbstractDialogFactory* pFactory = SvxAbstractDialogFactory::Create();
    VclPtr<AbstractSvxNameDialog> pDialog(pFactory->CreateSvxNameDialog(
        GetFrameWeld(), rSuggestedName, CuiResId(RID_CUISTR_DESC_COLOR)));
    pDialog->StartExecuteAsync([this, pDialog](sal_Int32 nResult) {
        const OUString sName = pDialog->GetName();
        pDialog->disposeOnce();
        if (nResult != RET_OK)
            return;

        if (!CustomPalette::Load().HasName(sName))
        {
            AddToCustomPalette(sName);
            return;
        }

        // The name is taken, so the user is told and asked again.
        std::shared_ptr<weld::Builder> xBuilder(
            Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
        std::shared_ptr<weld::MessageDialog> xWarning(
            xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
        xWarning->runAsync(xWarning, [this, sName, xBuilder](sal_Int32) { AskForColorName(sName); });
    });
}

void SvxColorFillTabPage::AddToCustomPalette(const OUString& rName)
{
    CustomPalette aPalette = CustomPalette::Load();
    aPalette.m_aColors.push_back(sal_Int32(m_aColor.m_aColor));
    aPalette.m_aNames.push_back(rName);
    aPalette.Save();

    // The custom palette is shown with the new color, the last one, selected.
    const NamedColor aColor(m_aColor.m_aColor, rName);
    m_xPaletteList->set_active_id(OUString::number(CUSTOM_PALETTE));
    ShowPalette(CUSTOM_PALETTE);
    m_xRecentColors->unselect_all();
    if (!m_aPaletteColors.empty())
        m_xColors->select(static_cast<int>(m_aPaletteColors.size()) - 1);
    SetColor(aColor);
    UpdateButtons();
    UpdatePreview();
}

IMPL_LINK_NOARG(SvxColorFillTabPage, RemoveHdl, weld::Button&, void)
{
    const sal_Int32 nPosition = lclSelectedPosition(*m_xColors);
    CustomPalette aPalette = CustomPalette::Load();
    if (m_nPalette != CUSTOM_PALETTE || nPosition < 0
        || o3tl::make_unsigned(nPosition) >= aPalette.m_aColors.size())
        return;

    aPalette.m_aColors.erase(aPalette.m_aColors.begin() + nPosition);
    aPalette.m_aNames.erase(aPalette.m_aNames.begin() + nPosition);
    aPalette.Save();

    ShowPalette(CUSTOM_PALETTE);
    if (!m_aPaletteColors.empty())
    {
        m_xColors->select(0);
        ColorSelectedHdl(*m_xColors);
    }
}

IMPL_LINK_NOARG(SvxColorFillTabPage, CustomColorHdl, weld::Button&, void)
{
    auto xDialog = std::make_shared<ColorDialog>(GetFrameWeld(), vcl::ColorPickerMode::Modify);
    xDialog->SetColor(m_aColor.m_aColor);
    xDialog->ExecuteAsync([this, xDialog](sal_Int32 nResult) {
        if (nResult != RET_OK)
            return;
        // A color from the picker is no swatch of the lists.
        m_xColors->unselect_all();
        m_xRecentColors->unselect_all();
        SetColor(NamedColor(xDialog->GetColor(), OUString()));
        m_aColorChangedHdl.Call(*this);
        UpdateButtons();
        UpdatePreview();
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
