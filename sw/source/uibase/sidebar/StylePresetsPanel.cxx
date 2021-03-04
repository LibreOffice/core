/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include "StylePresetsPanel.hxx"

#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/StylePreviewRenderer.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <sfx2/doctempl.hxx>

#include <shellio.hxx>
#include <docsh.hxx>

#include <sfx2/docfile.hxx>

namespace sw::sidebar {

namespace {

void renderPreview(sfx2::StyleManager* pStyleManager, OutputDevice& aOutputDevice,
                   std::u16string_view sName, sal_Int32 nHeight, tools::Rectangle const & aRect)
{
    SfxStyleSheetBase* pStyleSheet = pStyleManager->Search(sName, SfxStyleFamily::Para);

    if (pStyleSheet)
    {
        std::unique_ptr<sfx2::StylePreviewRenderer> pStylePreviewRenderer
            = pStyleManager->CreateStylePreviewRenderer(aOutputDevice, pStyleSheet, nHeight);
        pStylePreviewRenderer->recalculate();
        pStylePreviewRenderer->render(aRect, sfx2::StylePreviewRenderer::RenderAlign::TOP);
    }
}

BitmapEx GenerateStylePreview(SfxObjectShell& rSource, OUString const & aName)
{
    sfx2::StyleManager* pStyleManager = rSource.GetStyleManager();

    ScopedVclPtrInstance<VirtualDevice> pVirtualDev(*Application::GetDefaultDevice());

    float fScalingFactor = pVirtualDev->GetDPIScaleFactor();

    sal_Int32 nMargin = 6 * fScalingFactor;

    sal_Int32 nPreviewWidth = 144 * fScalingFactor;

    sal_Int32 nNameHeight = 16 * fScalingFactor;
    sal_Int32 nTitleHeight = 32 * fScalingFactor;
    sal_Int32 nHeadingHeight = 24 * fScalingFactor;
    sal_Int32 nTextBodyHeight = 16 * fScalingFactor;
    sal_Int32 nBottomMargin = 2 * fScalingFactor;

    sal_Int32 nNameFontSize = 12 * fScalingFactor;

    sal_Int32 nPreviewHeight = nNameHeight + nTitleHeight + nHeadingHeight + nTextBodyHeight + nBottomMargin;

    Size aSize(nPreviewWidth, nPreviewHeight);

    pVirtualDev->SetOutputSizePixel(aSize);

    pVirtualDev->SetLineColor(COL_LIGHTGRAY);
    pVirtualDev->SetFillColor();

    tools::Long y = 0;
    {
        pVirtualDev->SetFillColor(COL_LIGHTGRAY);
        tools::Rectangle aNameRect(0, y, nPreviewWidth, nNameHeight);
        pVirtualDev->DrawRect(aNameRect);

        vcl::Font aFont;
        aFont.SetFontSize(Size(0, nNameFontSize));

        pVirtualDev->SetFont(aFont);

        Size aTextSize(pVirtualDev->GetTextWidth(aName), pVirtualDev->GetTextHeight());

        Point aPoint((aNameRect.GetWidth()  / 2.0) - (aTextSize.Width()  / 2.0),
                     y + (aNameRect.GetHeight() / 2.0) - (aTextSize.Height() / 2.0));

        pVirtualDev->DrawText(aPoint, aName);

        y += nNameHeight;
    }

    {
        tools::Rectangle aRenderRect(Point(nMargin, y), aSize);
        renderPreview(pStyleManager, *pVirtualDev, u"Title", nTitleHeight, aRenderRect);
        y += nTitleHeight;
    }

    {
        tools::Rectangle aRenderRect(Point(nMargin, y), aSize);
        renderPreview(pStyleManager, *pVirtualDev, u"Heading 1", nHeadingHeight, aRenderRect);
        y += nHeadingHeight;
    }
    {
        tools::Rectangle aRenderRect(Point(nMargin, y), aSize);
        renderPreview(pStyleManager, *pVirtualDev, u"Text Body", nTextBodyHeight, aRenderRect);
    }

    return pVirtualDev->GetBitmapEx(Point(), aSize);
}

BitmapEx CreatePreview(OUString const & aUrl, OUString const & aName)
{
    SfxMedium aMedium(aUrl, StreamMode::STD_READWRITE);
    SfxObjectShell* pObjectShell = SfxObjectShell::Current();
    SfxObjectShellLock xTemplDoc = SfxObjectShell::CreateObjectByFactoryName(pObjectShell->GetFactory().GetFactoryName(), SfxObjectCreateMode::ORGANIZER);
    xTemplDoc->DoInitNew();
    if (xTemplDoc->LoadFrom(aMedium))
    {
        return GenerateStylePreview(*xTemplDoc, aName);
    }
    return BitmapEx();
}

}

VclPtr<PanelLayout> StylePresetsPanel::Create (vcl::Window* pParent,
                                        const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException("no parent Window given to StylePresetsPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to StylePresetsPanel::Create", nullptr, 1);

    return VclPtr<StylePresetsPanel>::Create(pParent, rxFrame);
}

StylePresetsPanel::StylePresetsPanel(vcl::Window* pParent,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "StylePresetsPanel", "modules/swriter/ui/sidebarstylepresets.ui", rxFrame)
    , mxValueSet(new ValueSet(nullptr))
    , mxValueSetWin(new weld::CustomWeld(*m_xBuilder, "valueset", *mxValueSet))
{
    mxValueSet->SetColCount(2);

    mxValueSet->SetDoubleClickHdl(LINK(this, StylePresetsPanel, DoubleClickHdl));

    RefreshList();

    m_pInitialFocusWidget = mxValueSet->GetDrawingArea();
}

void StylePresetsPanel::RefreshList()
{
    SfxDocumentTemplates aTemplates;
    sal_uInt16 nCount = aTemplates.GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString aRegionName(aTemplates.GetFullRegionName(i));
        if (aRegionName == "Styles")
        {
            for (sal_uInt16 j = 0; j < aTemplates.GetCount(i); ++j)
            {
                OUString aName = aTemplates.GetName(i,j);
                OUString aURL = aTemplates.GetPath(i,j);
                BitmapEx aPreview = CreatePreview(aURL, aName);
                sal_uInt16 nId = j + 1;
                mxValueSet->InsertItem(nId, Image(aPreview), aName);
                maTemplateEntries.push_back(std::make_unique<TemplateEntry>(aURL));
                mxValueSet->SetItemData(nId, maTemplateEntries.back().get());
            }
            mxValueSet->SetOptimalSize();
        }
    }
}

StylePresetsPanel::~StylePresetsPanel()
{
    disposeOnce();
}

void StylePresetsPanel::dispose()
{
    mxValueSetWin.reset();
    mxValueSet.reset();

    PanelLayout::dispose();
}

IMPL_LINK_NOARG(StylePresetsPanel, DoubleClickHdl, ValueSet*, void)
{
    sal_Int32 nItemId = mxValueSet->GetSelectedItemId();
    TemplateEntry* pEntry = static_cast<TemplateEntry*>(mxValueSet->GetItemData(nItemId));

    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    if (pDocSh)
    {
        SwgReaderOption aOption;
        aOption.SetTextFormats(true);
        aOption.SetNumRules(true);
        pDocSh->LoadStylesFromFile(pEntry->maURL, aOption, false);
    }
}

void StylePresetsPanel::NotifyItemUpdate(const sal_uInt16 /*nSId*/,
                                         const SfxItemState /*eState*/,
                                         const SfxPoolItem* /*pState*/)
{
}

} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
