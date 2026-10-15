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

#include <cuitabarea.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

#include <sfx2/basedlgs.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xtable.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wall.hxx>

using namespace com::sun::star;

namespace
{
constexpr OUString PATTERNID_NONE = u"none"_ustr;

// The size of a pattern in the dropdown, and in the button that shows the selected one.
constexpr Size PATTERN_ITEM_SIZE(52, 36);
constexpr Size PATTERN_BUTTON_SIZE(32, 16);

bool lclIsPattern(const Bitmap& rBitmap)
{
    Color aBackground;
    Color aForeground;
    return vcl::bitmap::isHistorical8x8(rBitmap, aBackground, aForeground);
}

Bitmap lclGetPresetBitmap(const XPatternListRef& pPatternList, sal_Int32 nPreset)
{
    return pPatternList->GetBitmap(nPreset)->GetGraphicObject().GetGraphic().GetBitmap();
}

// A pattern tiled over a white box with a border, or the empty box that stands for None.
Bitmap lclCreatePreview(const Bitmap* pPattern, const Size& rSize)
{
    ScopedVclPtrInstance<VirtualDevice> pDevice;
    pDevice->SetOutputSizePixel(rSize);
    const tools::Rectangle aRectangle(Point(), rSize);
    pDevice->SetLineColor();
    pDevice->SetFillColor(COL_WHITE);
    pDevice->DrawRect(aRectangle);
    if (pPattern)
        pDevice->DrawWallpaper(aRectangle, Wallpaper(*pPattern));
    pDevice->SetLineColor(Application::GetSettings().GetStyleSettings().GetShadowColor());
    pDevice->SetFillColor();
    pDevice->DrawRect(aRectangle);
    return pDevice->GetBitmap(Point(), rSize);
}
}

/// The dropdown of Pattern Style: None and the shipped patterns in a grid.
class SvxPatternStylePopup final : public WeldToolbarPopup
{
    SvxPatternStyleBox& m_rBox;
    std::unique_ptr<weld::IconView> m_xPatterns;

    DECL_LINK(ActivatedHdl, weld::IconView&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);

public:
    SvxPatternStylePopup(weld::Widget* pParent, SvxPatternStyleBox& rBox);

    void Fill();
    void Select(sal_Int32 nPreset);
    virtual void GrabFocus() override { m_xPatterns->grab_focus(); }
};

SvxPatternStylePopup::SvxPatternStylePopup(weld::Widget* pParent, SvxPatternStyleBox& rBox)
    : WeldToolbarPopup(nullptr, pParent, u"cui/ui/patternstylepopup.ui"_ustr,
                       u"PatternStylePopup"_ustr)
    , m_rBox(rBox)
    , m_xPatterns(m_xBuilder->weld_icon_view(u"patternstylelist"_ustr))
{
    m_xPatterns->connect_item_activated(LINK(this, SvxPatternStylePopup, ActivatedHdl));
    m_xPatterns->connect_query_tooltip(LINK(this, SvxPatternStylePopup, QueryTooltipHdl));
}

void SvxPatternStylePopup::Fill()
{
    const XPatternListRef& pPatternList = m_rBox.GetPatternList();
    m_xPatterns->freeze();
    m_xPatterns->clear();

    // The entries carry no text, so the name of each one is its tooltip.
    const Bitmap aNone = lclCreatePreview(nullptr, PATTERN_ITEM_SIZE);
    m_xPatterns->insert(-1, nullptr, &PATTERNID_NONE, &aNone, nullptr);
    for (tools::Long nPreset = 0; pPatternList.is() && nPreset < pPatternList->Count(); ++nPreset)
    {
        const Bitmap aPattern = lclGetPresetBitmap(pPatternList, nPreset);
        const Bitmap aPreview = lclCreatePreview(&aPattern, PATTERN_ITEM_SIZE);
        const OUString sId = OUString::number(nPreset);
        m_xPatterns->insert(-1, nullptr, &sId, &aPreview, nullptr);
    }

    m_xPatterns->thaw();
}

void SvxPatternStylePopup::Select(sal_Int32 nPreset)
{
    const OUString sId = nPreset < 0 ? PATTERNID_NONE : OUString::number(nPreset);
    for (int nEntry = 0; nEntry < m_xPatterns->n_children(); ++nEntry)
    {
        if (m_xPatterns->get_id(nEntry) == sId)
        {
            m_xPatterns->select(nEntry);
            return;
        }
    }
    m_xPatterns->unselect_all();
}

IMPL_LINK_NOARG(SvxPatternStylePopup, ActivatedHdl, weld::IconView&, bool)
{
    const OUString sId = m_xPatterns->get_selected_id();
    if (!sId.isEmpty())
        m_rBox.SelectPreset(sId == PATTERNID_NONE ? -1 : sId.toInt32());
    return true;
}

IMPL_LINK(SvxPatternStylePopup, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    const OUString sId = m_xPatterns->get_id(rIter);
    if (sId == PATTERNID_NONE)
        return CuiResId(RID_CUISTR_FILL_NONE);
    const XPatternListRef& pPatternList = m_rBox.GetPatternList();
    const sal_Int32 nPreset = sId.isEmpty() ? -1 : sId.toInt32();
    if (!pPatternList.is() || nPreset < 0 || nPreset >= pPatternList->Count())
        return OUString();
    return pPatternList->GetBitmap(nPreset)->GetName();
}

SvxPatternStyleBox::SvxPatternStyleBox(weld::Container* pParent, SfxTabPage& rPage)
    : m_rPage(rPage)
    , m_xBuilder(Application::CreateBuilder(pParent, u"cui/ui/patternstylepage.ui"_ustr))
    , m_xWidget(m_xBuilder->weld_widget(u"PatternStylePage"_ustr))
    , m_xStyleButton(m_xBuilder->weld_menu_button(u"patternstyle"_ustr))
{
    m_xPopup = std::make_unique<SvxPatternStylePopup>(m_xStyleButton.get(), *this);
    m_xStyleButton->set_popover(m_xPopup->getTopLevel());
    ShowSelection();
}

SvxPatternStyleBox::~SvxPatternStyleBox()
{
    m_xStyleButton->set_popover(nullptr);
    m_xPopup.reset();
}

void SvxPatternStyleBox::SetLists(const XColorListRef& pColorList,
                                  const XPatternListRef& pPatternList,
                                  ChangeType* pnColorListState)
{
    m_pColorList = pColorList;
    m_pPatternList = pPatternList;
    m_pnColorListState = pnColorListState;
    m_xPopup->Fill();
    ShowSelection();
}

sal_Int32 SvxPatternStyleBox::FindPreset() const
{
    if (!m_oBitmap || !m_pPatternList.is())
        return -1;

    // A preset of the same name comes first, since two presets can hold the same pixels.
    const BitmapChecksum nChecksum = m_oBitmap->GetChecksum();
    sal_Int32 nFound = -1;
    for (tools::Long nPreset = 0; nPreset < m_pPatternList->Count(); ++nPreset)
    {
        if (lclGetPresetBitmap(m_pPatternList, nPreset).GetChecksum() != nChecksum)
            continue;
        if (m_pPatternList->GetBitmap(nPreset)->GetName() == m_sName)
            return nPreset;
        if (nFound < 0)
            nFound = nPreset;
    }
    return nFound;
}

void SvxPatternStyleBox::ShowSelection()
{
    ScopedVclPtrInstance<VirtualDevice> pPreview;
    pPreview->SetOutputSizePixel(PATTERN_BUTTON_SIZE);
    pPreview->DrawBitmap(Point(),
                         lclCreatePreview(m_oBitmap ? &*m_oBitmap : nullptr, PATTERN_BUTTON_SIZE));
    m_xStyleButton->set_image(pPreview.get());
    m_xStyleButton->set_label(m_oBitmap ? m_sName : CuiResId(RID_CUISTR_FILL_NONE));
    m_xPopup->Select(m_oBitmap ? FindPreset() : -1);
}

bool SvxPatternStyleBox::Load(const SfxItemSet& rSet)
{
    const XFillBitmapItem& rItem = rSet.Get(XATTR_FILLBITMAP);
    const Bitmap aBitmap = rItem.GetGraphicObject().GetGraphic().GetBitmap();
    if (!lclIsPattern(aBitmap))
    {
        Clear();
        return false;
    }

    m_oBitmap = aBitmap;
    m_sName = rItem.GetName();
    const sal_Int32 nPreset = FindPreset();
    if (nPreset >= 0)
        m_sName = m_pPatternList->GetBitmap(nPreset)->GetName();
    else if (m_sName.isEmpty())
        m_sName = CuiResId(RID_CUISTR_PATTERN_OWN);
    ShowSelection();
    return true;
}

void SvxPatternStyleBox::Clear()
{
    m_oBitmap.reset();
    m_sName.clear();
    ShowSelection();
}

void SvxPatternStyleBox::FillItemSet(SfxItemSet& rSet) const
{
    if (!m_oBitmap)
        return;

    // A preset keeps its name, and any other pattern gets one from the document.
    const sal_Int32 nPreset = FindPreset();
    const OUString sName = nPreset >= 0 ? m_sName : OUString();
    rSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));
    rSet.Put(XFillBitmapItem(sName, Graphic(*m_oBitmap)));
    rSet.Put(XFillBmpTileItem(true));
}

void SvxPatternStyleBox::SelectPreset(sal_Int32 nPreset)
{
    m_xStyleButton->set_active(false);
    if (nPreset < 0 || !m_pPatternList.is() || nPreset >= m_pPatternList->Count())
    {
        Clear();
    }
    else
    {
        m_oBitmap = lclGetPresetBitmap(m_pPatternList, nPreset);
        m_sName = m_pPatternList->GetBitmap(nPreset)->GetName();
        ShowSelection();
    }
    m_aChangedHdl.Call(*this);
}

void SvxPatternStyleBox::SelectCustom()
{
    m_xStyleButton->set_active(false);
    OpenPatternDialog();
}

void SvxPatternStyleBox::OpenPatternDialog()
{
    auto pSet = std::make_shared<SfxItemSet>(
        m_rPage.GetItemSet().getPool(), WhichRangesContainer(XATTR_FILL_FIRST, XATTR_FILL_LAST));
    FillItemSet(*pSet);

    auto xDialog = std::make_shared<SfxSingleTabDialogController>(
        m_rPage.GetFrameWeld(), pSet.get(), u"content"_ustr, u"cui/ui/patterndialog.ui"_ustr,
        u"PatternDialog"_ustr);
    std::unique_ptr<SfxTabPage> xPage
        = SvxPatternTabPage::Create(xDialog->get_content_area(), xDialog.get(), pSet.get());
    auto* pPatternPage = static_cast<SvxPatternTabPage*>(xPage.get());
    pPatternPage->SetColorList(m_pColorList);
    pPatternPage->SetPatternList(m_pPatternList);
    pPatternPage->SetColorChgd(m_pnColorListState);
    pPatternPage->Construct();
    xDialog->SetTabPage(std::move(xPage));
    pPatternPage->ActivatePage(*pSet);

    weld::DialogController::runAsync(xDialog, [this, xDialog, pSet](sal_Int32 nResult) {
        // Patterns added in the dialog are presets now, so the dropdown is filled again.
        m_xPopup->Fill();

        const SfxItemSet* pResult = xDialog->GetOutputItemSet();
        const XFillBitmapItem* pItem
            = nResult == RET_OK && pResult ? pResult->GetItemIfSet(XATTR_FILLBITMAP) : nullptr;
        const Bitmap aBitmap
            = pItem ? pItem->GetGraphicObject().GetGraphic().GetBitmap() : Bitmap();
        if (!pItem || !lclIsPattern(aBitmap))
        {
            ShowSelection();
            return;
        }

        m_oBitmap = aBitmap;
        // The editor names the preset that is selected in it, and leaves a new drawing unnamed.
        m_sName = pItem->GetName();
        if (m_sName.isEmpty())
        {
            const sal_Int32 nPreset = FindPreset();
            m_sName = nPreset >= 0 ? m_pPatternList->GetBitmap(nPreset)->GetName()
                                   : CuiResId(RID_CUISTR_PATTERN_OWN);
        }
        ShowSelection();
        m_aChangedHdl.Call(*this);
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
