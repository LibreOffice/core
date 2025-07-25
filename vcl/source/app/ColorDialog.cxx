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

#include <salinst.hxx>
#include <svdata.hxx>

#include <vcl/ColorDialog.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/weld.hxx>

ColorDialog::ColorDialog(weld::Window* pParent, vcl::ColorPickerMode eMode)
{
    std::unique_ptr<weld::ColorChooserDialog> pDialog
        = GetSalInstance()->CreateColorChooserDialog(pParent, eMode);
    assert(pDialog);
    m_pColorChooserDialogController
        = std::make_shared<ColorChooserDialogController>(std::move(pDialog));
}

ColorDialog::~ColorDialog() {}

void ColorDialog::SetColor(const Color& rColor)
{
    m_pColorChooserDialogController->getDialog()->set_color(rColor);
}

Color ColorDialog::GetColor() const
{
    return m_pColorChooserDialogController->getDialog()->get_color();
}

short ColorDialog::Execute() { return m_pColorChooserDialogController->run(); }

void ColorDialog::ExecuteAsync(const std::function<void(sal_Int32)>& func)
{
    weld::DialogController::runAsync(m_pColorChooserDialogController, func);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
