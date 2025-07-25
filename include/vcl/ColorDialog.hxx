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

#pragma once

#include <tools/color.hxx>
#include <vcl/dllapi.h>
#include <vcl/weld.hxx>

#include <functional>

namespace vcl
{
    // Select is the default.
    enum class ColorPickerMode { Select, Modify };
}

class ColorChooserDialogController : public weld::DialogController
{
    std::unique_ptr<weld::ColorChooserDialog> m_pColorChooserDialog;

public:
    ColorChooserDialogController(std::unique_ptr<weld::ColorChooserDialog> pColorChooserDialog)
        : m_pColorChooserDialog(std::move(pColorChooserDialog))
    {
    }

    virtual weld::ColorChooserDialog* getDialog() override { return m_pColorChooserDialog.get(); }
};

class VCL_DLLPUBLIC ColorDialog final
{
public:
    ColorDialog(weld::Window* pParent, vcl::ColorPickerMode eMode = vcl::ColorPickerMode::Select);
    ~ColorDialog();

    void            SetColor( const Color& rColor );
    Color           GetColor() const;

    short           Execute();
    void            ExecuteAsync(const std::function<void(sal_Int32)>& func);

private:
    std::shared_ptr<ColorChooserDialogController> m_pColorChooserDialogController;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
