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

#include <vcl/builderfactory.hxx>
#include <vcl/layout.hxx>
#include <sfx2/dllapi.h>
#include <sfx2/viewfrm.hxx>
#include "DropdownBox.hxx"

#define NOTEBOOK_HEADER_HEIGHT 30

/*
 * Popup - shows hidden content, controls are moved to this popup
 * and after close moved to the original parent
 */

class Popup : public FloatingWindow
{
private:
    VclPtr<VclHBox> m_pBox;
    ScopedVclPtr<DropdownBox> m_pParent;

public:
    explicit Popup(VclPtr<DropdownBox> pParent)
    : FloatingWindow(pParent, "Popup", "sfx/ui/notebookbarpopup.ui")
    , m_pParent(pParent)
    {
        get(m_pBox, "box");
        m_pBox->SetSizePixel(Size(100, 75));
    }

    virtual ~Popup()
    {
        disposeOnce();
    }

    VclHBox* getBox()
    {
        return m_pBox.get();
    }

    virtual void PopupModeEnd() override
    {
        for (int i = 0; i < m_pBox->GetChildCount(); i++)
        {
            m_pBox->GetChild(i)->Hide();
            m_pBox->GetChild(i)->SetParent(m_pParent);
        }
        FloatingWindow::PopupModeEnd();
    }

    void dispose() override
    {
        m_pBox.disposeAndClear();
        m_pParent.clear();

        FloatingWindow::dispose();
    }
};

/*
 * DropdownBox - shows content or moves it to the popup
 * which can be opened by clicking on a button
 */

DropdownBox::DropdownBox(vcl::Window *pParent)
    : VclHBox(pParent)
    , m_bInFullView(true)
{
    m_pButton = VclPtr<PushButton>::Create(this, WB_FLATBUTTON);
    m_pButton->SetClickHdl(LINK(this, DropdownBox, PBClickHdl));
    m_pButton->SetSymbol(SymbolType::MENU);
    m_pButton->set_width_request(15);
    m_pButton->SetQuickHelpText(GetQuickHelpText());
    m_pButton->Resize();
}

DropdownBox::~DropdownBox()
{
    disposeOnce();
}

void DropdownBox::dispose()
{
    m_pButton.disposeAndClear();
    if (m_pPopup)
        m_pPopup.disposeAndClear();

    VclHBox::dispose();
}

void DropdownBox::HideContent()
{
    if (m_bInFullView)
    {
        m_bInFullView = false;

        for (int i = 0; i < GetChildCount(); i++)
            GetChild(i)->Hide();

        m_pButton->Show();
        SetOutputSizePixel(Size(m_pButton->GetSizePixel().Width(), GetSizePixel().Height()));
    }
}

void DropdownBox::ShowContent()
{
    if (!m_bInFullView)
    {
        m_bInFullView = true;

        for (int i = 0; i < GetChildCount(); i++)
            GetChild(i)->Show();

        m_pButton->Hide();
    }
}

IMPL_LINK_TYPED(DropdownBox, PBClickHdl, Button*, /*pButton*/, void)
{
    if (m_pPopup)
        m_pPopup.disposeAndClear();

    m_pPopup = VclPtr<Popup>::Create(this);

    for (int i = 0; i < GetChildCount(); i++)
    {
        if (GetChild(i) != m_pButton)
        {
            Window* pChild = GetChild(i);
            pChild->Show();

            pChild->SetParent(m_pPopup->getBox());
        }
    }

    m_pPopup->getBox()->set_height_request(GetSizePixel().Height());

    long x = GetPosPixel().getX();
    long y = GetPosPixel().getY() + NOTEBOOK_HEADER_HEIGHT + GetSizePixel().Height();
    Rectangle aRect(x, y, x, y);

    m_pPopup->StartPopupMode(aRect, FloatWinPopupFlags::Down
                                    |FloatWinPopupFlags::GrabFocus
                                    |FloatWinPopupFlags::AllMouseButtonClose);
}

VCL_BUILDER_FACTORY(DropdownBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
