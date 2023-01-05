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
#include <docstyle.hxx>
#include <drawdoc.hxx>
#include <ndnotxt.hxx>
#include <ndtxt.hxx>
#include <fmtcol.hxx>
#include <format.hxx>
#include <charatr.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdpage.hxx>
#include <svx/ColorSets.hxx>
#include <svx/dialog/ThemeColorValueSet.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/colritem.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace
{

void changeColor(SwTextFormatColl* pCollection, svx::ColorSet const& rColorSet)
{
    SvxColorItem aColorItem(pCollection->GetColor());
    model::ThemeColor const& rThemeColor = aColorItem.GetThemeColor();
    auto eThemeType = rThemeColor.getType();
    if (eThemeType != model::ThemeColorType::Unknown)
    {
        Color aColor = rColorSet.getColor(eThemeType);
        aColor = rThemeColor.applyTransformations(aColor);
        aColorItem.SetValue(aColor);
        pCollection->SetFormatAttr(aColorItem);
    }
}

void applyTheme(SfxStyleSheetBasePool* pPool, svx::ColorSet const& rColorSet)
{
    SwDocStyleSheet* pStyle;

    pStyle = static_cast<SwDocStyleSheet*>(pPool->First(SfxStyleFamily::Para));
    while (pStyle)
    {
        SwTextFormatColl* pCollection = pStyle->GetCollection();
        changeColor(pCollection, rColorSet);
        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }
}

} // end anonymous namespace

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
        svx::Theme* pTheme = pPage->getSdrPageProperties().GetTheme();
        if (pTheme)
            maColorSets.insert(*pTheme->GetColorSet());
    }

    const std::vector<svx::ColorSet>& aColorSets = maColorSets.getColorSets();
    for (size_t i = 0; i < aColorSets.size(); ++i)
    {
        const svx::ColorSet& rColorSet = aColorSets[i];
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

    svx::ColorSet const& rColorSet = maColorSets.getColorSet(nIndex);

    applyTheme(pDocSh->GetStyleSheetPool(), rColorSet);
}

void ThemePanel::NotifyItemUpdate(const sal_uInt16 /*nSId*/,
                                         const SfxItemState /*eState*/,
                                         const SfxPoolItem* /*pState*/)
{
}

} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
