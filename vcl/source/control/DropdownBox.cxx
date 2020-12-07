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

#include <vcl/toolkit/button.hxx>
#include <vcl/layout.hxx>
#include <DropdownBox.hxx>

#define NOTEBOOK_HEADER_HEIGHT 30

/*
 * DropdownBox - shows content or moves it to the popup
 * which can be opened by clicking on a button
 */

DropdownBox::DropdownBox(vcl::Window* pParent)
    : VclHBox(pParent)
    , IPrioritable()
    , m_bInFullView(true)
{
    m_pButton = VclPtr<PushButton>::Create(this, WB_FLATBUTTON);
    m_pButton->SetClickHdl(LINK(this, DropdownBox, PBClickHdl));
    m_pButton->SetSymbol(SymbolType::MENU);
    m_pButton->set_width_request(15);
    m_pButton->SetQuickHelpText(GetQuickHelpText());
    m_pButton->Resize();
}

DropdownBox::~DropdownBox() { disposeOnce(); }

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

bool DropdownBox::IsHidden() { return !m_bInFullView; }

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

IMPL_LINK(DropdownBox, PBClickHdl, Button*, /*pButton*/, void)
{
    if (m_pPopup)
        m_pPopup.disposeAndClear();

    m_pPopup = VclPtr<NotebookbarPopup>::Create(this);

    for (int i = 0; i < GetChildCount(); i++)
    {
        if (GetChild(i) != m_pButton)
        {
            Window* pChild = GetChild(i);
            pChild->Show();

            pChild->SetParent(m_pPopup->getBox());
            // count is decreased because we moved child
            i--;
        }
    }

    m_pPopup->hideSeparators(true);

    m_pPopup->getBox()->set_height_request(GetSizePixel().Height());

    tools::Long x = GetPosPixel().getX();
    tools::Long y = GetPosPixel().getY() + NOTEBOOK_HEADER_HEIGHT + GetSizePixel().Height();
    tools::Rectangle aRect(x, y, x, y);

    m_pPopup->StartPopupMode(aRect, FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus
                                        | FloatWinPopupFlags::AllMouseButtonClose);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
