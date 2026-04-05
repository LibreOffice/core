/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/svxdllapi.h>
#include <vcl/svapp.hxx>
#include <vcl/font/Variation.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Button.hxx>
#include <vcl/weld/Grid.hxx>
#include <vcl/weld/Label.hxx>
#include <vcl/weld/Popover.hxx>
#include <vcl/weld/Scale.hxx>
#include <vcl/weld/ScrolledWindow.hxx>
#include <vcl/weld/SpinButton.hxx>

namespace svx
{
struct SVX_DLLPUBLIC FontVariationItem
{
    FontVariationItem(weld::Grid* pParentGrid);

    uint32_t nTag;
    float fDefaultValue;

    std::unique_ptr<weld::Builder> xBuilder;
    std::unique_ptr<weld::Widget> xContainer;
    std::unique_ptr<weld::Label> xLabel;
    std::unique_ptr<weld::Scale> xScale;
    std::unique_ptr<weld::SpinButton> xSpin;
    Link<FontVariationItem&, void> aChangedHdl;

private:
    DECL_LINK(ScaleChangedHdl, weld::Scale&, void);
    DECL_LINK(SpinChangedHdl, weld::SpinButton&, void);
};

/** Builds one row of widgets per variation axis, in a container the caller owns.

    Separate from whatever shows it, so a popover, a toolbar popup and a
    sidebar panel can all use it.
 */
class SVX_DLLPUBLIC FontVariationsControl
{
private:
    weld::ScrolledWindow& m_rContentWindow;
    weld::Grid& m_rContentGrid;
    std::vector<std::unique_ptr<FontVariationItem>> m_aItems;

    Link<FontVariationsControl&, void> m_aChangedHdl;
    Link<weld::Entry&, bool> m_aActivateHdl;

    DECL_LINK(ResetClickedHdl, weld::Button&, void);
    DECL_LINK(ValueChangedHdl, FontVariationItem&, void);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);

public:
    FontVariationsControl(weld::ScrolledWindow& rContentWindow, weld::Grid& rContentGrid,
                          weld::Button& rResetButton,
                          const std::vector<vcl::font::VariationAxis>& rAxes,
                          const std::vector<vcl::font::Variation>& rCurrentVariations);
    ~FontVariationsControl();

    void connect_changed(const Link<FontVariationsControl&, void>& rLink) { m_aChangedHdl = rLink; }
    /// Enter was pressed on a value, so that a host can close itself.
    void connect_activate(const Link<weld::Entry&, bool>& rLink) { m_aActivateHdl = rLink; }

    std::vector<vcl::font::Variation> getVariations() const;
};

class SVX_DLLPUBLIC FontVariationsPopup
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::ScrolledWindow> m_xContentWindow;
    std::unique_ptr<weld::Grid> m_xContentGrid;
    std::unique_ptr<weld::Button> m_xResetButton;
    std::unique_ptr<FontVariationsControl> m_xControl;
    // popping down lays the contents out again, so destroy it while they are
    // still around
    std::unique_ptr<weld::Popover> m_xPopover;

    Link<FontVariationsPopup&, void> m_aChangedHdl;

    DECL_LINK(ChangedHdl, FontVariationsControl&, void);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);

public:
    FontVariationsPopup(weld::Widget* pParent, const std::vector<vcl::font::VariationAxis>& rAxes,
                        const std::vector<vcl::font::Variation>& rCurrentVariations);
    ~FontVariationsPopup();

    void popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect);
    void connect_changed(const Link<FontVariationsPopup&, void>& rLink) { m_aChangedHdl = rLink; }

    std::vector<vcl::font::Variation> getVariations() const;
};

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
