/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/labelitemwindow.hxx>

LabelItemWindow::LabelItemWindow(vcl::Window* pParent, const OUString& rLabel)
    : InterimItemWindow(pParent, "svx/ui/labelbox.ui", "LabelBox")
    , m_xBox(m_xBuilder->weld_box("LabelBox"))
    , m_xLabel(m_xBuilder->weld_label("label"))
    , m_xImage(m_xBuilder->weld_image("image"))
{
    InitControlBase(m_xLabel.get());

    m_xLabel->set_label(rLabel);
    m_xImage->hide();
    m_xImage->set_size_request(24, 24); // vcl/res/infobar.png is 32x32 - too large here

    SetOptimalSize();

    m_xLabel->set_toolbar_background();
}

void LabelItemWindow::SetOptimalSize()
{
    Size aSize(m_xBox->get_preferred_size());
    aSize.AdjustWidth(12);

    SetSizePixel(aSize);
}

void LabelItemWindow::set_label(const OUString& rLabel, const LabelItemWindowType eType)
{
    m_xLabel->set_visible(false); // a11y announcement
    m_xLabel->set_label(rLabel);
    if ((eType == LabelItemWindowType::Text) || rLabel.isEmpty())
    {
        m_xImage->hide();
        m_xLabel->set_font_color(COL_AUTO);
        m_xBox->set_background(COL_AUTO);
    }
    else if (eType == LabelItemWindowType::Info)
    {
        m_xImage->show();
        m_xLabel->set_font_color(Color(0x00, 0x47, 0x85));
        m_xBox->set_background(Color(0xBD, 0xE5, 0xF8)); // same as InfobarType::INFO
    }
    m_xLabel->set_visible(
        true); // always show and not just if !rLabel.isEmpty() to not make the chevron appear
}

OUString LabelItemWindow::get_label() const { return m_xLabel->get_label(); }

void LabelItemWindow::dispose()
{
    m_xImage.reset();
    m_xLabel.reset();
    m_xBox.reset();
    InterimItemWindow::dispose();
}

LabelItemWindow::~LabelItemWindow() { disposeOnce(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
