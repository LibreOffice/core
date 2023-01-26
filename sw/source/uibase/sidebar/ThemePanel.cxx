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
#include <docmodel/theme/Theme.hxx>
#include <svx/svdpage.hxx>
#include <svx/dialog/ThemeColorValueSet.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar
{

std::unique_ptr<PanelLayout> ThemePanel::Create(weld::Widget* pParent)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException("no parent Window given to PagePropertyPanel::Create", nullptr, 0);

    return std::make_unique<ThemePanel>(pParent);
}

ThemePanel::ThemePanel(weld::Widget* pParent)
    : PanelLayout(pParent, "ThemePanel", "modules/swriter/ui/sidebartheme.ui")
    , mxValueSetColors(new svx::ThemeColorValueSet)
    , mxValueSetColorsWin(new weld::CustomWeld(*m_xBuilder, "valueset_colors", *mxValueSetColors))
    , mxApplyButton(m_xBuilder->weld_button("apply"))
{
    mxValueSetColors->SetColCount(2);
    mxValueSetColors->SetLineCount(3);
    mxValueSetColors->SetColor(Application::GetSettings().GetStyleSettings().GetFaceColor());

    mxApplyButton->connect_clicked(LINK(this, ThemePanel, ClickHdl));
    mxValueSetColors->SetDoubleClickHdl(LINK(this, ThemePanel, DoubleClickValueSetHdl));

    maColorSets.init();

    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    SwDoc* pDocument = pDocSh->GetDoc();
    if (pDocument)
    {
        SdrPage* pPage = pDocument->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
        model::Theme* pTheme = pPage->getSdrPageProperties().GetTheme();
        if (pTheme)
            maColorSets.insert(*pTheme->GetColorSet());
    }

    const std::vector<model::ColorSet>& aColorSets = maColorSets.getColorSets();
    for (size_t i = 0; i < aColorSets.size(); ++i)
    {
        const model::ColorSet& rColorSet = aColorSets[i];
        mxValueSetColors->insert(rColorSet);
    }

    mxValueSetColors->SetOptimalSize();

    if (!aColorSets.empty())
        mxValueSetColors->SelectItem(1); // ItemId 1, position 0
}

ThemePanel::~ThemePanel()
{
    mxValueSetColorsWin.reset();
    mxValueSetColors.reset();
    mxApplyButton.reset();
}

IMPL_LINK_NOARG(ThemePanel, ClickHdl, weld::Button&, void)
{
    DoubleClickHdl();
}

IMPL_LINK_NOARG(ThemePanel, DoubleClickValueSetHdl, ValueSet*, void)
{
    DoubleClickHdl();
}

IMPL_LINK_NOARG(ThemePanel, DoubleClickHdl, weld::TreeView&, bool)
{
    DoubleClickHdl();
    return true;
}

void ThemePanel::DoubleClickHdl()
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    if (!pDocSh)
        return;

    sal_uInt32 nItemId = mxValueSetColors->GetSelectedItemId();
    if (!nItemId)
        return;
    sal_uInt32 nIndex = nItemId - 1;

    model::ColorSet const& rColorSet = maColorSets.getColorSet(nIndex);

    ThemeColorChanger aChanger(pDocSh);
    aChanger.apply(rColorSet);
}

void ThemePanel::NotifyItemUpdate(const sal_uInt16 /*nSId*/,
                                         const SfxItemState /*eState*/,
                                         const SfxPoolItem* /*pState*/)
{
}

} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
