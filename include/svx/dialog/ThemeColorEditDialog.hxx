/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_options.h>

#include <svx/svxdllapi.h>
#include <vcl/weld.hxx>
#include <docmodel/theme/ColorSet.hxx>

class ColorListBox;

namespace svx
{
class UNLESS_MERGELIBS(SVX_DLLPUBLIC) ThemeColorEditDialog final
    : public weld::GenericDialogController
{
private:
    model::ColorSet maColorSet;

    std::unique_ptr<weld::Entry> mxThemeColorsNameEntry;
    std::unique_ptr<ColorListBox> mxDark1;
    std::unique_ptr<ColorListBox> mxLight1;
    std::unique_ptr<ColorListBox> mxDark2;
    std::unique_ptr<ColorListBox> mxLight2;
    std::unique_ptr<ColorListBox> mxAccent1;
    std::unique_ptr<ColorListBox> mxAccent2;
    std::unique_ptr<ColorListBox> mxAccent3;
    std::unique_ptr<ColorListBox> mxAccent4;
    std::unique_ptr<ColorListBox> mxAccent5;
    std::unique_ptr<ColorListBox> mxAccent6;
    std::unique_ptr<ColorListBox> mxHyperlink;
    std::unique_ptr<ColorListBox> mxFollowHyperlink;

public:
    ThemeColorEditDialog(weld::Window* pParent, const model::ColorSet& rColorSet);
    virtual ~ThemeColorEditDialog() override;
    model::ColorSet getColorSet();
};

} // end svx namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
