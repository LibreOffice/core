/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/WeldedTabbedNotebookbar.hxx>
#include <vcl/svapp.hxx>
#include <jsdialog/jsdialogbuilder.hxx>

WeldedTabbedNotebookbar::WeldedTabbedNotebookbar(
    const VclPtr<vcl::Window>& pContainerWindow, const OUString& rUIFilePath,
    const css::uno::Reference<css::frame::XFrame>& rFrame, sal_uInt64 nWindowId,
    const std::vector<ExtraPanel>& rExtraPanels)
    : m_xBuilder(JSInstanceBuilder::CreateNotebookbarBuilder(
          pContainerWindow, AllSettings::GetUIRootDir(), rUIFilePath, rFrame, nWindowId))
{
    m_xContainer = m_xBuilder->weld_container(u"NotebookBar"_ustr);
    m_xWeldedToolbar = m_xBuilder->weld_toolbar(u"WeldedToolbar"_ustr);

    for (const ExtraPanel& rPanel : rExtraPanels)
    {
        ExtraToolbar aExtra;
        aExtra.m_aControllerService = rPanel.m_aControllerService;
        aExtra.m_xBuilder = JSInstanceBuilder::CreateNotebookbarBuilder(
            pContainerWindow, AllSettings::GetUIRootDir(), rPanel.m_aUIFilePath, rFrame, nWindowId);
        aExtra.m_xToolbar = aExtra.m_xBuilder->weld_toolbar(rPanel.m_aToolbarId);
        m_aExtraToolbars.push_back(std::move(aExtra));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
