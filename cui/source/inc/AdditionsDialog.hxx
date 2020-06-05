/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/weld.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/fntctrl.hxx>
#include <memory>

struct AdditionsItem
{
    AdditionsItem(weld::Widget* pParent)
        : m_xBuilder(Application::CreateBuilder(pParent, "cui/ui/additionsfragment.ui"))
        , m_xContainer(m_xBuilder->weld_widget("additionsEntry"))
        , m_xImageScreenshot(m_xBuilder->weld_image("imageScreenshot"))
        , m_xButtonInstall(m_xBuilder->weld_button("buttonInstall"))
        , m_xLabelName(m_xBuilder->weld_label("labelName"))
        , m_xLabelDescription(m_xBuilder->weld_label("labelDescription"))
    {
    }

    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Widget> m_xContainer;
    std::unique_ptr<weld::Image> m_xImageScreenshot;
    std::unique_ptr<weld::Button> m_xButtonInstall;
    std::unique_ptr<weld::Label> m_xLabelName;
    std::unique_ptr<weld::Label> m_xLabelDescription;
};

class AdditionsDialog : public weld::GenericDialogController
{
private:
    std::vector<AdditionsItem> m_aAdditionsItems;

    std::unique_ptr<weld::ScrolledWindow> m_xContentWindow;
    std::unique_ptr<weld::Container> m_xContentGrid;

    void fillGrid();

    //DECL_LINK(ComboBoxSelectedHdl, weld::ComboBox&, void);
    //DECL_LINK(CheckBoxToggledHdl, weld::ToggleButton&, void);

public:
    AdditionsDialog(weld::Window* pParent);
    ~AdditionsDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
