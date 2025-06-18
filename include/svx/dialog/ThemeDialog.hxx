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
#include <vcl/weld.hxx>
#include <svx/svdpage.hxx>
#include <svx/dialog/ThemeColorEditDialog.hxx>

namespace model
{
class Theme;
}

namespace svx
{
class SVX_DLLPUBLIC ThemeDialog final : public weld::GenericDialogController
{
private:
    weld::Window* mpWindow;
    model::Theme* mpTheme;
    std::shared_ptr<svx::ThemeColorEditDialog> mxSubDialog;
    std::vector<model::ColorSet> maColorSets;

    std::unique_ptr<weld::IconView> mxIconViewThemeColors;
    std::unique_ptr<weld::Button> mxAdd;

    std::shared_ptr<model::ColorSet> mpCurrentColorSet;

    void runThemeColorEditDialog();
    void initColorSets();
    static VclPtr<VirtualDevice> CreateColorSetPreview(const model::ColorSet& rColorSet);

public:
    ThemeDialog(weld::Window* pParent, model::Theme* pTheme);
    virtual ~ThemeDialog() override;

    DECL_LINK(ItemActivatedHdl, weld::IconView&, bool);
    DECL_LINK(SelectionChangedHdl, weld::IconView&, void);
    DECL_LINK(ButtonClicked, weld::Button&, void);

    std::shared_ptr<model::ColorSet> const& getCurrentColorSet() { return mpCurrentColorSet; }
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
