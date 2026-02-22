/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/svxdllapi.h>
#include <vcl/weld/weld.hxx>
#include <vcl/weld/IconView.hxx>
#include <vcl/vclptr.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <memory>
#include <vector>

namespace model
{
class Theme;
}
class VirtualDevice;

namespace svx
{
/// Base class for theme color selection functionality
class SVX_DLLPUBLIC ThemeColorsPaneBase
{
protected:
    std::unique_ptr<weld::IconView> mxIconViewThemeColors;
    std::vector<model::ColorSet> maColorSets;
    std::shared_ptr<model::ColorSet> mpCurrentColorSet;

    void initColorSets(model::Theme* pTheme = nullptr);
    static VclPtr<VirtualDevice> CreateColorSetPreview(const model::ColorSet& rColorSet);

public:
    explicit ThemeColorsPaneBase(std::unique_ptr<weld::IconView> xIconView);
    virtual ~ThemeColorsPaneBase();

    DECL_LINK(SelectionChangedHdl, weld::IconView&, void);
    DECL_LINK(ItemActivatedHdl, weld::IconView&, bool);

    std::shared_ptr<model::ColorSet> const& getCurrentColorSet() { return mpCurrentColorSet; }

protected:
    /// Override this to handle theme color activation
    virtual void onColorSetActivated() {}
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
