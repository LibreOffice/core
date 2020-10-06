/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
    const css::uno::Reference<css::frame::XFrame>& rFrame, sal_uInt64 nWindowId)
    : m_xBuilder(new JSInstanceBuilder(pContainerWindow, AllSettings::GetUIRootDir(), rUIFilePath,
                                       rFrame, nWindowId))
{
    m_xContainer = m_xBuilder->weld_container("NotebookBar");
    m_xNotebook = m_xBuilder->weld_notebook("ContextContainer");
    m_xNotebook->set_current_page("HomeLabel");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
