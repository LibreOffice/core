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

#include <swtypes.hxx>
#include <cmdid.h>

#include <svl/intitem.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>

#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <sfx2/doctempl.hxx>

#include "shellio.hxx"
#include "docsh.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/string.hxx>

namespace sw { namespace sidebar {

namespace {

void renderPreview(sfx2::StyleManager* pStyleManager, OutputDevice& aOutputDevice,
                   OUString const & sName, sal_Int32 nHeight, Rectangle& aRect)
{
    SfxStyleSheetBase* pStyleSheet = pStyleManager->Search(sName, SFX_STYLE_FAMILY_PARA);

    if (pStyleSheet)
    {
        sfx2::StylePreviewRenderer* pStylePreviewRenderer;
        pStylePreviewRenderer = pStyleManager->CreateStylePreviewRenderer(aOutputDevice, pStyleSheet, nHeight);
        pStylePreviewRenderer->recalculate();
        pStylePreviewRenderer->render(aRect, sfx2::StylePreviewRenderer::RenderAlign::TOP);
    }
}

BitmapEx GenerateStylePreview(SfxObjectShell& rSource, OUString& aName)
{
    sfx2::StyleManager* pStyleManager = rSource.GetStyleManager();

    ScopedVclPtrInstance<VirtualDevice> pVirtualDev(*Application::GetDefaultDevice());

    sal_Int32 nScalingFactor = pVirtualDev->GetDPIScaleFactor();

    sal_Int32 nMargin = 6 * nScalingFactor;

    sal_Int32 nPreviewWidth = 144 * nScalingFactor;

    sal_Int32 nNameHeight = 16 * nScalingFactor;
    sal_Int32 nTitleHeight = 32 * nScalingFactor;
    sal_Int32 nHeadingHeight = 24 * nScalingFactor;
    sal_Int32 nTextBodyHeight = 16 * nScalingFactor;
    sal_Int32 nBottomMargin = 2 * nScalingFactor;

    sal_Int32 nNameFontSize = 12 * nScalingFactor;

    sal_Int32 nPreviewHeight = nNameHeight + nTitleHeight + nHeadingHeight + nTextBodyHeight + nBottomMargin;

    Size aSize(nPreviewWidth, nPreviewHeight);

    pVirtualDev->SetOutputSizePixel(aSize);

    pVirtualDev->SetLineColor(COL_LIGHTGRAY);
    pVirtualDev->SetFillColor();

    long y = 0;
    {
        pVirtualDev->SetFillColor(COL_LIGHTGRAY);
        Rectangle aNameRect(0, y, nPreviewWidth, nNameHeight);
        pVirtualDev->DrawRect(aNameRect);

        vcl::Font aFont;
        aFont.SetSize(Size(0, nNameFontSize));

        pVirtualDev->SetFont(aFont);

        Size aTextSize(pVirtualDev->GetTextWidth(aName), pVirtualDev->GetTextHeight());

        Point aPoint((aNameRect.GetWidth()  / 2.0) - (aTextSize.Width()  / 2.0),
                     y + (aNameRect.GetHeight() / 2.0) - (aTextSize.Height() / 2.0));

        pVirtualDev->DrawText(aPoint, aName);

        y += nNameHeight;
    }

    {
        Rectangle aRenderRect(Point(nMargin, y), aSize);
        renderPreview(pStyleManager, *pVirtualDev.get(), "Title", nTitleHeight, aRenderRect);
        y += nTitleHeight;
    }

    {
        Rectangle aRenderRect(Point(nMargin, y), aSize);
        renderPreview(pStyleManager, *pVirtualDev.get(), "Heading 1", nHeadingHeight, aRenderRect);
        y += nHeadingHeight;
    }
    {
        Rectangle aRenderRect(Point(nMargin, y), aSize);
        renderPreview(pStyleManager, *pVirtualDev.get(), "Text Body", nTextBodyHeight, aRenderRect);
    }

    return pVirtualDev->GetBitmapEx(Point(), aSize);
}

BitmapEx CreatePreview(OUString& aUrl, OUString& aName)
{
    SfxMedium aMedium(aUrl, STREAM_STD_READWRITE);
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

VclPtr<vcl::Window> StylePresetsPanel::Create (vcl::Window* pParent,
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
{
    get(mpValueSet, "valueset");

    mpValueSet->SetColCount(2);

    mpValueSet->SetDoubleClickHdl(LINK(this, StylePresetsPanel, DoubleClickHdl));

    RefreshList();
}

void StylePresetsPanel::RefreshList()
{
    SfxDocumentTemplates aTemplates;
    sal_uInt16 nCount = aTemplates.GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString aRegionName(aTemplates.GetFullRegionName(i));
        if (aRegionName == "styles")
        {
            for (sal_uInt16 j = 0; j < aTemplates.GetCount(i); ++j)
            {
                OUString aName = aTemplates.GetName(i,j);
                OUString aURL = aTemplates.GetPath(i,j);
                BitmapEx aPreview = CreatePreview(aURL, aName);
                mpValueSet->InsertItem(j, Image(aPreview), aName);
                maTemplateEntries.push_back(std::unique_ptr<TemplateEntry>(new TemplateEntry(aURL)));
                mpValueSet->SetItemData(j, maTemplateEntries.back().get());
            }
        }
    }
}

StylePresetsPanel::~StylePresetsPanel()
{
    disposeOnce();
}

void StylePresetsPanel::dispose()
{
    mpValueSet.disposeAndClear();

    PanelLayout::dispose();
}

IMPL_LINK_NOARG_TYPED(StylePresetsPanel, DoubleClickHdl, ValueSet*, void)
{
    sal_Int32 nItemId = mpValueSet->GetSelectItemId();
    TemplateEntry* pEntry = static_cast<TemplateEntry*>(mpValueSet->GetItemData(nItemId));

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
                                         const SfxPoolItem* /*pState*/,
                                         const bool /*bIsEnabled*/)
{
}

}} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
