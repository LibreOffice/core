/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <view/SlsILayerPainter.hxx>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <vcl/font.hxx>
#include <memory>

namespace sd::slidesorter
{
class SlideSorter;
}

namespace sd::slidesorter::view
{
/** Paints section name headers above each section's first slide row
    in the slide sorter panel.

    Style (font, colors) is cached and rebuilt only when system
    settings change, so Paint() stays lightweight.
*/
class SectionHeaderPainter final : public ILayerPainter,
                                   public std::enable_shared_from_this<SectionHeaderPainter>
{
public:
    explicit SectionHeaderPainter(SlideSorter& rSlideSorter);
    virtual ~SectionHeaderPainter() override;

    virtual void SetLayerInvalidator(std::unique_ptr<ILayerInvalidator> pInvalidator) override;
    virtual void Paint(OutputDevice& rDevice, const ::tools::Rectangle& rRepaintArea) override;

    /// Call when system colors/fonts may have changed (e.g. theme switch).
    void HandleDataChanged();

private:
    SlideSorter& mrSlideSorter;
    std::unique_ptr<ILayerInvalidator> mpLayerInvalidator;

    // Cached style — rebuilt by HandleDataChanged() or on first Paint()
    bool mbStyleValid = false;
    vcl::Font maFont;
    Color maBackgroundColor;
    Color maTextColor;

    void EnsureStyle(const OutputDevice& rDevice);
};

} // end of namespace sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
