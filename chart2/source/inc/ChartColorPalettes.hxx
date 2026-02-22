/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/ChartColorPaletteType.hxx>
#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <vcl/weld/customweld.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/IconView.hxx>
#include <vcl/weld/ScrolledWindow.hxx>

class MouseEvent;

namespace chart
{
class ChartColorPalettes final
{
public:
    typedef Link<const MouseEvent&, bool> MouseEventHandler;

private:
    std::unique_ptr<weld::IconView> mxIconView;
    std::unique_ptr<weld::ScrolledWindow> mxWindow;
    std::vector<ChartColorPalette> maColorSets;
    sal_uInt16 mnHighlightedItemId;
    MouseEventHandler maMouseMoveHdl;

public:
    ChartColorPalettes(weld::Builder& rBuilder, const OUString& id, const OUString& winId);

    sal_uInt16 GetSelectedItemId();
    sal_uInt16 GetHighlightedItemId();
    void SetSelectHdl(const Link<weld::IconView&, bool>& rLink);
    void SetNoSelection();
    void SelectItem(sal_uInt16 nItemId);
    bool IsNoSelection();
    void GrabFocus();

    void insert(ChartColorPalette const& rColorSet);
    const ChartColorPalette* getPalette(sal_uInt32 nItem) const;
    void setMouseMoveHdl(const MouseEventHandler& rLink);

    void Fill();

private:
    DECL_LINK(OnQueryTooltip, const weld::TreeIter&, OUString);
    DECL_LINK(OnMouseMove, const MouseEvent&, bool);
};

} // end namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
