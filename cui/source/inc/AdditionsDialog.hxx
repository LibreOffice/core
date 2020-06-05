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

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

struct AdditionsItem
{
    AdditionsItem(weld::Widget* pParent)
        : m_xBuilder(Application::CreateBuilder(pParent, "cui/ui/additionsfragment.ui"))
        , m_xContainer(m_xBuilder->weld_widget("additionsEntry"))
        , m_xImageScreenshot(m_xBuilder->weld_image("imageScreenshot"))
        , m_xButtonInstall(m_xBuilder->weld_button("buttonInstall"))
        , m_xLinkButtonName(m_xBuilder->weld_link_button("linkButtonName"))
        , m_xLabelAuthor(m_xBuilder->weld_label("labelAuthor"))
        , m_xLabelDesc(m_xBuilder->weld_label("labelDesc")) // no change (print description)
        , m_xLabelDescription(m_xBuilder->weld_label("labelDescription"))
        , m_xLabelLicense(m_xBuilder->weld_label("labelLicense"))
        , m_xLabelVersion(m_xBuilder->weld_label("labelVersion"))
        , m_xLabelComments(m_xBuilder->weld_label("labelComments")) // no change
        , m_xLinkButtonComments(m_xBuilder->weld_link_button("linkButtonComments"))
        , m_xImageVoting(m_xBuilder->weld_image("imageVoting"))
        , m_xImageDownloadNumber(m_xBuilder->weld_image("imageDownloadNumber"))
        , m_xLabelDownloadNumber(m_xBuilder->weld_label("labelDownloadNumber"))
    {
    }

    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Widget> m_xContainer;
    std::unique_ptr<weld::Image> m_xImageScreenshot;
    std::unique_ptr<weld::Button> m_xButtonInstall;
    std::unique_ptr<weld::LinkButton> m_xLinkButtonName;
    std::unique_ptr<weld::Label> m_xLabelAuthor;
    std::unique_ptr<weld::Label> m_xLabelDesc;
    std::unique_ptr<weld::Label> m_xLabelDescription;
    std::unique_ptr<weld::Label> m_xLabelLicense;
    std::unique_ptr<weld::Label> m_xLabelVersion;
    std::unique_ptr<weld::Label> m_xLabelComments;
    std::unique_ptr<weld::LinkButton> m_xLinkButtonComments;
    std::unique_ptr<weld::Image> m_xImageVoting;
    std::unique_ptr<weld::Image> m_xImageDownloadNumber;
    std::unique_ptr<weld::Label> m_xLabelDownloadNumber;
};

class AdditionsDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEntrySearch;
    std::unique_ptr<weld::MenuButton> m_xMenuButtonSettings;
    std::vector<AdditionsItem> m_aAdditionsItems;

    std::unique_ptr<weld::ScrolledWindow> m_xContentWindow;
    std::unique_ptr<weld::Container> m_xContentGrid;

    void fillGrid();

public:
    AdditionsDialog(weld::Window* pParent);
    ~AdditionsDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
