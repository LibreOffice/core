/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "NotebookbarPopup.hxx"
#include <vcl/IPrioritable.hxx>
#include <vcl/layout.hxx>

NotebookbarPopup::NotebookbarPopup(const VclPtr<VclHBox>& pParent)
    : FloatingWindow(pParent, "Popup", "sfx/ui/notebookbarpopup.ui")
    , m_pParent(pParent)
{
    get(m_pBox, "box");
    m_pBox->SetSizePixel(Size(100, 75));
}

NotebookbarPopup::~NotebookbarPopup() { disposeOnce(); }

VclHBox* NotebookbarPopup::getBox() { return m_pBox.get(); }

void NotebookbarPopup::PopupModeEnd()
{
    hideSeparators(false);
    while (m_pBox->GetChildCount())
    {
        vcl::IPrioritable* pChild = dynamic_cast<vcl::IPrioritable*>(GetChild(0));
        if (pChild)
            pChild->HideContent();

        vcl::Window* pWindow = m_pBox->GetChild(0);
        pWindow->SetParent(m_pParent);

        if (m_pParent)
            m_pParent->Resize();
    }

    FloatingWindow::PopupModeEnd();
}

void NotebookbarPopup::hideSeparators(bool bHide)
{
    // separator on the beginning
    vcl::Window* pWindow = m_pBox->GetChild(0);
    while (pWindow && pWindow->GetType() == WindowType::CONTAINER)
    {
        pWindow = pWindow->GetChild(0);
    }
    if (pWindow && pWindow->GetType() == WindowType::FIXEDLINE)
    {
        if (bHide)
            pWindow->Hide();
        else
            pWindow->Show();
    }

    // separator on the end
    pWindow = m_pBox->GetChild(m_pBox->GetChildCount() - 1);
    while (pWindow && pWindow->GetType() == WindowType::CONTAINER)
    {
        pWindow = pWindow->GetChild(pWindow->GetChildCount() - 1);
    }
    if (pWindow && pWindow->GetType() == WindowType::FIXEDLINE)
    {
        if (bHide)
            pWindow->Hide();
        else
            pWindow->Show();
    }
}

void NotebookbarPopup::dispose()
{
    PopupModeEnd();
    m_pBox.disposeAndClear();
    m_pParent.clear();

    FloatingWindow::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
