/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <colorpicker.hxx>
#include <salinst.hxx>
#include <svdata.hxx>

#include <officecfg/Office/Common.hxx>
#include <vcl/ColorDialog.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/weld.hxx>

namespace
{
class ColorChooserDialogController : public ColorDialogController
{
    std::unique_ptr<weld::ColorChooserDialog> m_pColorChooserDialog;

public:
    ColorChooserDialogController(std::unique_ptr<weld::ColorChooserDialog> pColorChooserDialog)
        : m_pColorChooserDialog(std::move(pColorChooserDialog))
    {
    }

    virtual void SetColor(const Color& rColor) override
    {
        m_pColorChooserDialog->set_color(rColor);
    }
    virtual Color GetColor() const override { return m_pColorChooserDialog->get_color(); }

private:
    virtual weld::ColorChooserDialog* getDialog() override { return m_pColorChooserDialog.get(); }
};
}

ColorDialog::ColorDialog(weld::Window* pParent, vcl::ColorPickerMode eMode)
{
    if (!officecfg::Office::Common::Misc::UseSystemColorDialog::get())
    {
        // use custom LibreOffice color picker dialog
        m_pColorDialogController = std::make_shared<ColorPickerDialog>(pParent, COL_BLACK, eMode);
        return;
    }

    std::unique_ptr<weld::ColorChooserDialog> pDialog
        = GetSalInstance()->CreateColorChooserDialog(pParent, eMode);
    assert(pDialog);
    pDialog->set_modal(true);
    m_pColorDialogController = std::make_shared<ColorChooserDialogController>(std::move(pDialog));
}

ColorDialog::~ColorDialog() {}

void ColorDialog::SetColor(const Color& rColor) { m_pColorDialogController->SetColor(rColor); }

Color ColorDialog::GetColor() const { return m_pColorDialogController->GetColor(); }

short ColorDialog::Execute() { return m_pColorDialogController->run(); }

void ColorDialog::ExecuteAsync(const std::function<void(sal_Int32)>& func)
{
    weld::DialogController::runAsync(m_pColorDialogController, func);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
