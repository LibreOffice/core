/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <tools/link.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>

class WidgetTestDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Button> m_xOKButton;
    std::unique_ptr<weld::Button> m_xCancelButton;

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);

public:
    WidgetTestDialog(weld::Window* pParent);
    ~WidgetTestDialog();
};
