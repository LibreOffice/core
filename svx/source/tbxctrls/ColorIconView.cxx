/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/ColorIconView.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld/IconView.hxx>

ColorIconView::ColorIconView(std::unique_ptr<weld::IconView> pIconView)
    : m_pIconView(std::move(pIconView))
{
    m_pIconView->connect_item_activated(LINK(this, ColorIconView, ItemActivatedHdl));
    m_pIconView->connect_selection_changed(LINK(this, ColorIconView, SelectionChangedHdl));
}

void ColorIconView::insert(int nIndex, const Color& rColor, const OUString& rColorName)
{
    assert(nIndex >= 0 && nIndex <= m_pIconView->n_children() && "Invalid index");

    const OUString* pNullIconName = nullptr;
    m_pIconView->insert(nIndex, nullptr, nullptr, pNullIconName, nullptr);
    ScopedVclPtr<VirtualDevice> pIcon = createIcon(rColor);
    m_pIconView->set_image(nIndex, *pIcon);
    m_pIconView->set_item_tooltip_text(nIndex, rColorName);
    m_pIconView->set_item_accessible_name(nIndex, rColorName);
    m_pIconView->set_id(nIndex, rColor.AsRGBHEXString());
}

int ColorIconView::getItemCount() const { return m_pIconView->n_children(); }

Color ColorIconView::getColor(int nIndex) const
{
    if (nIndex < 0 || nIndex >= m_pIconView->n_children())
        return Color();

    Color aColor;
    color::createFromString("#" + m_pIconView->get_id(nIndex).toUtf8(), aColor);
    return aColor;
}

OUString ColorIconView::getColorName(int nIndex) const
{
    return m_pIconView->get_item_tooltip_text(nIndex);
}

int ColorIconView::get_selected_index() const { return m_pIconView->get_selected_index(); }

void ColorIconView::select(int nIndex) { m_pIconView->select(nIndex); };

void ColorIconView::unselect_all() { m_pIconView->unselect_all(); }

void ColorIconView::remove(int nIndex) { m_pIconView->remove(nIndex); }

void ColorIconView::clear() { m_pIconView->clear(); }

void ColorIconView::grab_focus() { m_pIconView->grab_focus(); }

void ColorIconView::set_sensitive(bool bSensitive) { m_pIconView->set_sensitive(bSensitive); }

void ColorIconView::set_help_id(const OUString& rName) { m_pIconView->set_help_id(rName); }

IMPL_LINK(ColorIconView, ItemActivatedHdl, const weld::TreeIter&, rIter, bool)
{
    Color aColor = getColor(m_pIconView->get_iter_index_in_parent(rIter));
    m_aColorActivatedHdl.Call(aColor);
    return true;
}

IMPL_LINK_NOARG(ColorIconView, SelectionChangedHdl, weld::ItemView&, void)
{
    m_aSelectionChangedHdl.Call(*this);
}

ScopedVclPtr<VirtualDevice> ColorIconView::createIcon(const Color& rColor)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    const tools::Long nEdgeLength = rStyleSettings.GetListBoxPreviewDefaultPixelSize().Height() + 1;
    const Size aSize(nEdgeLength, nEdgeLength);

    ScopedVclPtr<VirtualDevice> pDev = m_pIconView->create_virtual_device();
    pDev->SetOutputSizePixel(aSize);

    pDev->SetFillColor(rColor);
    pDev->SetLineColor(rStyleSettings.GetFieldTextColor());
    pDev->DrawRect(pDev->GetOutputRectPixel());

    return pDev;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
