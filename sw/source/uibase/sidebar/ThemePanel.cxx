/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "ThemePanel.hxx"
#include <sal/config.h>

#include <doc.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <ThemeColorChanger.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <docmodel/theme/Theme.hxx>
#include <svx/svdpage.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar
{

std::unique_ptr<PanelLayout> ThemePanel::Create(weld::Widget* pParent)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(u"no parent Window given to PagePropertyPanel::Create"_ustr, nullptr, 0);

    return std::make_unique<ThemePanel>(pParent);
}

ThemePanel::ThemePanel(weld::Widget* pParent)
    : PanelLayout(pParent, u"ThemePanel"_ustr, u"modules/swriter/ui/sidebartheme.ui"_ustr)
    , mxIconViewColors(m_xBuilder->weld_icon_view(u"iconview_colors"_ustr))
    , mxApplyButton(m_xBuilder->weld_button(u"apply"_ustr))
{
    mxApplyButton->connect_clicked(LINK(this, ThemePanel, ClickHdl));
    mxIconViewColors->connect_item_activated(LINK(this, ThemePanel, ItemActivatedHdl));

    auto const& rColorSets = svx::ColorSets::get();
    for (model::ColorSet const& rColorSet : rColorSets.getColorSetVector())
    {
        ScopedVclPtr<VirtualDevice> pDev = CreateImage(rColorSet);
        const Bitmap aBitmap = pDev->GetBitmap(Point(0, 0), pDev->GetOutputSize());
        const int nIndex = mxIconViewColors->n_children();
        const OUString sId = OUString::number(nIndex);
        mxIconViewColors->insert(nIndex, nullptr, &sId, &aBitmap, nullptr);
        mxIconViewColors->set_item_accessible_name(nIndex, rColorSet.getName());
    }

    if (!rColorSets.getColorSetVector().empty())
        mxIconViewColors->select(0);
}

ThemePanel::~ThemePanel()
{
    mxApplyButton.reset();
}

IMPL_LINK_NOARG(ThemePanel, ClickHdl, weld::Button&, void)
{
    DoubleClickHdl();
}

IMPL_LINK_NOARG(ThemePanel, ItemActivatedHdl, weld::IconView&, bool)
{
    DoubleClickHdl();
    return true;
}

VclPtr<VirtualDevice> ThemePanel::CreateImage(const model::ColorSet& rColorSet)
{
    constexpr tools::Long BORDER = 4;
    constexpr tools::Long SIZE = 16;
    constexpr tools::Long LABEL_HEIGHT = 16;
    constexpr tools::Long LABEL_TEXT_HEIGHT = 14;
    constexpr tools::Long constElementNumber = 8;

    const Size aMin(BORDER * 7 + SIZE * constElementNumber / 2 + BORDER * 2,
                    BORDER * 3 + SIZE * 2 + LABEL_HEIGHT);

    const Size aSize(aMin.Width() + 40, aMin.Height());
    VclPtr<VirtualDevice> pDev = mxIconViewColors->create_virtual_device();
    pDev->SetOutputSizePixel(aSize);

    tools::Long startX = (aSize.Width() / 2.0) - (aMin.Width() / 2.0);
    tools::Long x = BORDER;
    tools::Long y1 = BORDER + LABEL_HEIGHT;
    tools::Long y2 = y1 + SIZE + BORDER;

    pDev->SetLineColor(COL_LIGHTGRAY);
    pDev->SetFillColor(COL_LIGHTGRAY);
    tools::Rectangle aNameRect(Point(0, 0), Size(aSize.Width(), LABEL_HEIGHT));
    pDev->DrawRect(aNameRect);

    vcl::Font aFont;
    OUString aName = rColorSet.getName();
    aFont.SetFontHeight(LABEL_TEXT_HEIGHT);
    pDev->SetFont(aFont);

    Size aTextSize(pDev->GetTextWidth(aName), pDev->GetTextHeight());

    Point aPoint((aNameRect.GetWidth() / 2.0) - (aTextSize.Width() / 2.0),
                 (aNameRect.GetHeight() / 2.0) - (aTextSize.Height() / 2.0));

    pDev->DrawText(aPoint, aName);

    pDev->SetLineColor(COL_LIGHTGRAY);
    pDev->SetFillColor();

    for (sal_uInt32 i = 2; i < 10; i += 2)
    {
        pDev->SetFillColor(rColorSet.getColor(model::convertToThemeColorType(i)));
        pDev->DrawRect(tools::Rectangle(Point(x + startX, y1), Size(SIZE, SIZE)));

        pDev->SetFillColor(rColorSet.getColor(model::convertToThemeColorType(i + 1)));
        pDev->DrawRect(tools::Rectangle(Point(x + startX, y2), Size(SIZE, SIZE)));

        x += SIZE + BORDER;
        if (i == 2 || i == 8)
            x += BORDER;
    }

    return pDev;
}

void ThemePanel::DoubleClickHdl()
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    if (!pDocSh)
        return;

    const OUString sId = mxIconViewColors->get_selected_id();
    if (sId.isEmpty())
        return;
    const sal_uInt32 nIndex = sId.toUInt32();

    auto const& rColorSets = svx::ColorSets::get();
    model::ColorSet const& rColorSet = rColorSets.getColorSet(nIndex);

    ThemeColorChanger aChanger(pDocSh);
    aChanger.apply(std::make_shared<model::ColorSet>(rColorSet));
}

void ThemePanel::NotifyItemUpdate(const sal_uInt16 /*nSId*/,
                                         const SfxItemState /*eState*/,
                                         const SfxPoolItem* /*pState*/)
{
}

} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
