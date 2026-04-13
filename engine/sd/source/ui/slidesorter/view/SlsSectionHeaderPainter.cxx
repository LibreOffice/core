/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SlsSectionHeaderPainter.hxx"
#include <SlideSorter.hxx>
#include <SlideSectionManager.hxx>
#include <drawdoc.hxx>
#include <view/SlideSorterView.hxx>
#include <view/SlsLayouter.hxx>
#include <model/SlideSorterModel.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>

namespace sd::slidesorter::view
{
namespace
{
constexpr sal_Int32 gnTextLeftPadding = 4;
constexpr sal_Int32 gnTextTopPadding = 2;
constexpr sal_Int32 gnFontSizePadding = 6;
constexpr sal_uInt8 gnLuminanceDecrease = 15;
}

SectionHeaderPainter::SectionHeaderPainter(SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter)
{
}

SectionHeaderPainter::~SectionHeaderPainter() {}

void SectionHeaderPainter::SetLayerInvalidator(std::unique_ptr<ILayerInvalidator> pInvalidator)
{
    mpLayerInvalidator = std::move(pInvalidator);
}

void SectionHeaderPainter::HandleDataChanged() { mbStyleValid = false; }

void SectionHeaderPainter::EnsureStyle(const OutputDevice& rDevice)
{
    if (mbStyleValid)
        return;

    const StyleSettings& rStyleSettings = rDevice.GetSettings().GetStyleSettings();

    maBackgroundColor = rStyleSettings.GetDialogColor();
    maBackgroundColor.DecreaseLuminance(gnLuminanceDecrease);
    maTextColor = rStyleSettings.GetButtonTextColor();

    const sal_Int32 nHeaderHeight = Layouter::GetSectionHeaderHeight();
    maFont = rStyleSettings.GetAppFont();
    maFont.SetFontSize(Size(0, nHeaderHeight - gnFontSizePadding));
    maFont.SetWeight(WEIGHT_BOLD);

    mbStyleValid = true;
}

void SectionHeaderPainter::Paint(OutputDevice& rDevice, const ::tools::Rectangle& rRepaintArea)
{
    SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
    if (!pDocument)
        return;

    sd::SlideSectionManager& rSectionMgr = pDocument->GetSectionManager();
    const sal_Int32 nSectionCount = rSectionMgr.GetSectionCount();
    if (nSectionCount == 0)
        return;

    Layouter& rLayouter = mrSlideSorter.GetView().GetLayouter();
    if (rLayouter.GetColumnCount() <= 0)
        return;

    const sal_Int32 nHeaderHeight = Layouter::GetSectionHeaderHeight();
    const sal_Int32 nPageCount = mrSlideSorter.GetModel().GetPageCount();

    EnsureStyle(rDevice);

    rDevice.Push(vcl::PushFlags::FONT | vcl::PushFlags::TEXTCOLOR | vcl::PushFlags::FILLCOLOR
                 | vcl::PushFlags::LINECOLOR);

    rDevice.SetFont(maFont);
    rDevice.SetTextColor(maTextColor);
    rDevice.SetFillColor(maBackgroundColor);
    rDevice.SetLineColor();

    for (sal_Int32 i = 0; i < nSectionCount; ++i)
    {
        const sd::SlideSection& rSection = rSectionMgr.GetSection(i);
        const sal_Int32 nStartIndex = rSection.mnStartIndex;
        if (nStartIndex < 0 || nStartIndex >= nPageCount)
            continue;

        const ::tools::Rectangle aSlideBox = rLayouter.GetPageObjectBox(nStartIndex, false);

        const ::tools::Rectangle aHeaderBox(
            Point(rRepaintArea.Left(), aSlideBox.Top() - nHeaderHeight),
            Size(rRepaintArea.GetWidth(), nHeaderHeight));

        if (!aHeaderBox.Overlaps(rRepaintArea))
            continue;

        rDevice.DrawRect(aHeaderBox);
        rDevice.DrawText(
            Point(aHeaderBox.Left() + gnTextLeftPadding, aHeaderBox.Top() + gnTextTopPadding),
            rSection.maName);
    }

    rDevice.Pop();
}

} // end of namespace sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
