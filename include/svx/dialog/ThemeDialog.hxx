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
#include <svx/theme/IThemeColorChanger.hxx>
#include <svx/dialog/ThemeColorValueSet.hxx>
#include <functional>

namespace model
{
class Theme;
}

class ColorListBox;

namespace svx
{
class SVX_DLLPUBLIC ThemeDialog final : public weld::GenericDialogController
{
private:
    weld::Window* mpWindow;
    model::Theme* mpTheme;
    std::vector<model::ColorSet> maColorSets;

    std::unique_ptr<svx::ThemeColorValueSet> mxValueSetThemeColors;
    std::unique_ptr<weld::CustomWeld> mxValueSetThemeColorsWindow;
    std::unique_ptr<weld::Button> mxAdd;

    std::optional<std::reference_wrapper<model::ColorSet>> moCurrentColorSet;

    void runThemeColorEditDialog();
    void initColorSets();

public:
    ThemeDialog(weld::Window* pParent, model::Theme* pTheme);
    virtual ~ThemeDialog() override;

    DECL_LINK(DoubleClickValueSetHdl, ValueSet*, void);
    DECL_LINK(SelectItem, ValueSet*, void);
    DECL_LINK(ButtonClicked, weld::Button&, void);

    std::optional<std::reference_wrapper<model::ColorSet>> const& getCurrentColorSet()
    {
        return moCurrentColorSet;
    }
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
