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
#include <svx/ColorSets.hxx>
#include <svx/svdpage.hxx>
#include <svx/theme/IThemeColorChanger.hxx>
#include <svx/dialog/ThemeColorValueSet.hxx>

namespace svx
{
class SVX_DLLPUBLIC ThemeDialog final : public weld::GenericDialogController
{
private:
    svx::Theme* mpTheme;
    svx::ColorSets maColorSets;
    std::shared_ptr<IThemeColorChanger> mpChanger;

    std::unique_ptr<svx::ThemeColorValueSet> mxValueSetThemeColors;
    std::unique_ptr<weld::CustomWeld> mxValueSetThemeColorsWindow;

public:
    ThemeDialog(weld::Window* pParent, svx::Theme* pTheme,
                std::shared_ptr<IThemeColorChanger> const& pChanger);
    virtual ~ThemeDialog() override;

    DECL_LINK(DoubleClickValueSetHdl, ValueSet*, void);
    void DoubleClickHdl();
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
