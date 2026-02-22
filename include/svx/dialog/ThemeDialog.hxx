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
#include <vcl/weld/DialogController.hxx>
#include <vcl/weld/IconView.hxx>
#include <vcl/weld/weld.hxx>
#include <svx/svdpage.hxx>
#include <svx/dialog/ThemeColorEditDialog.hxx>
#include <svx/dialog/ThemeColorsPaneBase.hxx>
namespace model
{
class Theme;
}

namespace svx
{
class SVX_DLLPUBLIC ThemeDialog final : public weld::GenericDialogController,
                                        public ThemeColorsPaneBase
{
private:
    model::Theme* mpTheme;
    std::shared_ptr<svx::ThemeColorEditDialog> mxSubDialog;

    std::unique_ptr<weld::Button> mxAdd;

    void runThemeColorEditDialog();

protected:
    void onColorSetActivated() override;

public:
    ThemeDialog(weld::Window* pParent, model::Theme* pTheme);
    virtual ~ThemeDialog() override;

    DECL_LINK(ButtonClicked, weld::Button&, void);
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
