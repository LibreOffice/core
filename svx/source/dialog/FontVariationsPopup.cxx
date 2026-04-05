/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/FontVariationsPopup.hxx>
#include <vcl/font/Feature.hxx>
#include <map>

namespace svx
{
FontVariationItem::FontVariationItem(weld::Grid* pParentGrid)
    : nTag(0)
    , fDefaultValue(0)
    , xBuilder(Application::CreateBuilder(pParentGrid, u"svx/ui/fontvariationfragment.ui"_ustr))
    , xContainer(xBuilder->weld_widget(u"variationentry"_ustr))
    , xLabel(xBuilder->weld_label(u"label"_ustr))
    , xScale(xBuilder->weld_scale(u"scale"_ustr))
    , xSpin(xBuilder->weld_spin_button(u"spin"_ustr))
{
    xScale->connect_value_changed(LINK(this, FontVariationItem, ScaleChangedHdl));
    xSpin->connect_value_changed(LINK(this, FontVariationItem, SpinChangedHdl));
}

IMPL_LINK_NOARG(FontVariationItem, ScaleChangedHdl, weld::Scale&, void)
{
    xSpin->set_value(xScale->get_value() * 100);
    aChangedHdl.Call(*this);
}

IMPL_LINK_NOARG(FontVariationItem, SpinChangedHdl, weld::SpinButton&, void)
{
    xScale->set_value(xSpin->get_value() / 100);
    aChangedHdl.Call(*this);
}

FontVariationsControl::FontVariationsControl(
    weld::ScrolledWindow& rContentWindow, weld::Grid& rContentGrid, weld::Button& rResetButton,
    const std::vector<vcl::font::VariationAxis>& rAxes,
    const std::vector<vcl::font::Variation>& rCurrentVariations)
    : m_rContentWindow(rContentWindow)
    , m_rContentGrid(rContentGrid)
{
    rResetButton.connect_clicked(LINK(this, FontVariationsControl, ResetClickedHdl));

    std::map<uint32_t, float> aCurrentValues;
    for (const auto& rVar : rCurrentVariations)
        aCurrentValues[rVar.nTag] = rVar.fValue;

    int nRow = 0;
    for (const auto& rAxis : rAxes)
    {
        auto pItem = std::make_unique<FontVariationItem>(&m_rContentGrid);
        pItem->nTag = rAxis.nTag;
        pItem->fDefaultValue = rAxis.fDefaultValue;

        m_rContentGrid.set_child_left_attach(*pItem->xContainer, 0);
        m_rContentGrid.set_child_top_attach(*pItem->xContainer, nRow);

        pItem->xLabel->set_label(rAxis.aName);

        // every row is built from the same fragment, so make the ids unique
        const OUString sTag = "-" + vcl::font::featureCodeAsString(rAxis.nTag);
        pItem->xLabel->set_buildable_name(u"label"_ustr + sTag);
        pItem->xScale->set_buildable_name(u"scale"_ustr + sTag);
        pItem->xSpin->set_buildable_name(u"spin"_ustr + sTag);

        float fCurrentValue = rAxis.fDefaultValue;
        auto it = aCurrentValues.find(rAxis.nTag);
        if (it != aCurrentValues.end())
            fCurrentValue = it->second;

        // Slider uses integer values
        pItem->xScale->set_range(static_cast<int>(rAxis.fMinValue),
                                 static_cast<int>(rAxis.fMaxValue));
        pItem->xScale->set_value(static_cast<int>(fCurrentValue));

        // Spin button uses x100 values for two decimal places
        pItem->xSpin->set_range(static_cast<int>(rAxis.fMinValue * 100),
                                static_cast<int>(rAxis.fMaxValue * 100));
        pItem->xSpin->set_value(static_cast<int>(fCurrentValue * 100));

        pItem->aChangedHdl = LINK(this, FontVariationsControl, ValueChangedHdl);
        pItem->xSpin->connect_activate(LINK(this, FontVariationsControl, ActivateHdl));
        m_aItems.push_back(std::move(pItem));
        ++nRow;
    }

    auto nContentHeight = m_rContentGrid.get_preferred_size().Height();
    m_rContentWindow.set_size_request(-1, std::min(nContentHeight, static_cast<tools::Long>(300)));
}

FontVariationsControl::~FontVariationsControl() {}

std::vector<vcl::font::Variation> FontVariationsControl::getVariations() const
{
    std::vector<vcl::font::Variation> aVariations;
    for (const auto& pItem : m_aItems)
        aVariations.push_back({ pItem->nTag, pItem->xSpin->get_value() / 100.0f });
    return aVariations;
}

IMPL_LINK_NOARG(FontVariationsControl, ResetClickedHdl, weld::Button&, void)
{
    for (const auto& pItem : m_aItems)
    {
        pItem->xScale->set_value(static_cast<int>(pItem->fDefaultValue));
        pItem->xSpin->set_value(static_cast<int>(pItem->fDefaultValue * 100));
    }
    m_aChangedHdl.Call(*this);
}

IMPL_LINK_NOARG(FontVariationsControl, ValueChangedHdl, FontVariationItem&, void)
{
    m_aChangedHdl.Call(*this);
}

IMPL_LINK(FontVariationsControl, ActivateHdl, weld::Entry&, rEntry, bool)
{
    return m_aActivateHdl.IsSet() && m_aActivateHdl.Call(rEntry);
}

FontVariationsPopup::FontVariationsPopup(
    weld::Widget* pParent, const std::vector<vcl::font::VariationAxis>& rAxes,
    const std::vector<vcl::font::Variation>& rCurrentVariations)
    : m_xBuilder(Application::CreateBuilder(pParent, u"svx/ui/fontvariationspopover.ui"_ustr))
    , m_xContentWindow(m_xBuilder->weld_scrolled_window(u"contentWindow"_ustr))
    , m_xContentGrid(m_xBuilder->weld_grid(u"contentGrid"_ustr))
    , m_xResetButton(m_xBuilder->weld_button(u"reset"_ustr))
    , m_xPopover(m_xBuilder->weld_popover(u"FontVariationsPopover"_ustr))
{
    // Built after the popover, so that the rows go into a toplevel the builder
    // already owns
    m_xControl.reset(new FontVariationsControl(*m_xContentWindow, *m_xContentGrid, *m_xResetButton,
                                               rAxes, rCurrentVariations));
    m_xControl->connect_changed(LINK(this, FontVariationsPopup, ChangedHdl));
    m_xControl->connect_activate(LINK(this, FontVariationsPopup, ActivateHdl));
}

FontVariationsPopup::~FontVariationsPopup() {}

void FontVariationsPopup::popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect)
{
    m_xPopover->popup_at_rect(pParent, rRect);
}

std::vector<vcl::font::Variation> FontVariationsPopup::getVariations() const
{
    return m_xControl->getVariations();
}

IMPL_LINK_NOARG(FontVariationsPopup, ChangedHdl, FontVariationsControl&, void)
{
    m_aChangedHdl.Call(*this);
}

IMPL_LINK_NOARG(FontVariationsPopup, ActivateHdl, weld::Entry&, bool)
{
    m_xPopover->popdown();
    return true;
}

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
