/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/bitmapex.hxx>
#include <vcl/builder.hxx>
#include <vcl/layout.hxx>
#include <IPrioritable.hxx>
#include <NotebookbarPopup.hxx>

NotebookbarPopup::NotebookbarPopup(const VclPtr<VclHBox>& pParent)
    : FloatingWindow(pParent, "Popup", "sfx/ui/notebookbarpopup.ui")
    , m_pParent(pParent)
{
    m_pUIBuilder->get(m_pBox, "box");
    m_pBox->SetSizePixel(Size(100, 75));
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const BitmapEx& aPersona = rStyleSettings.GetPersonaHeader();

    if (!aPersona.IsEmpty())
        m_pBox->SetBackground(Wallpaper(aPersona));
    else
        m_pBox->SetBackground(rStyleSettings.GetDialogColor());
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

        // resize after all children of box are empty
        if (m_pParent && !m_pBox->GetChildCount())
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

    if (bHide)
    {
        sal_Int32 BoxId = 0;
        while (BoxId <= m_pBox->GetChildCount() - 1)
        {
            if (m_pBox->GetChild(BoxId))
            {
                pWindow = m_pBox->GetChild(BoxId);
                ApplyBackground(pWindow);
            }
            BoxId++;
        }
    }
    else
    {
        sal_Int32 BoxId = m_pBox->GetChildCount() - 1;
        while (BoxId >= 0)
        {
            if (m_pBox->GetChild(BoxId))
            {
                pWindow = m_pBox->GetChild(BoxId);
                RemoveBackground(pWindow);
            }
            BoxId--;
        }
    }
}

void NotebookbarPopup::dispose()
{
    PopupModeEnd();
    m_pBox.disposeAndClear();
    m_pParent.clear();

    FloatingWindow::dispose();
}

void NotebookbarPopup::ApplyBackground(vcl::Window* pWindow)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const BitmapEx& aPersona = rStyleSettings.GetPersonaHeader();

    if (!aPersona.IsEmpty())
        pWindow->SetBackground(Wallpaper(aPersona));
    else
        pWindow->SetBackground(rStyleSettings.GetDialogColor());

    sal_Int32 nNext = 0;
    VclPtr<vcl::Window> pChild = pWindow->GetChild(nNext);
    while (pChild && pWindow->GetType() == WindowType::CONTAINER)
    {
        ApplyBackground(pChild);
        nNext++;
        if (pWindow->GetChild(nNext) && pWindow->GetType() == WindowType::CONTAINER)
            pChild = pWindow->GetChild(nNext);
        else
            break;
    }
}

void NotebookbarPopup::RemoveBackground(vcl::Window* pWindow)
{
    pWindow->SetBackground(Wallpaper(COL_TRANSPARENT));

    sal_Int32 nNext = 0;
    VclPtr<vcl::Window> pChild = pWindow->GetChild(nNext);
    while (pChild && pWindow->GetType() == WindowType::CONTAINER)
    {
        RemoveBackground(pChild);
        nNext++;
        if (pWindow->GetChild(nNext) && pWindow->GetType() == WindowType::CONTAINER)
            pChild = pWindow->GetChild(nNext);
        else
            break;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
