/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceButton.hxx>

QtInstanceButton::QtInstanceButton(QPushButton* pButton)
    : QtInstanceWidget(pButton)
    , m_pButton(pButton)
{
    assert(m_pButton);

    connect(m_pButton, &QPushButton::clicked, this, &QtInstanceButton::buttonClicked);
}

void QtInstanceButton::set_label(const OUString& rText)
{
    assert(m_pButton);
    m_pButton->setText(toQString(rText));
}

void QtInstanceButton::set_image(VirtualDevice* /*pDevice*/)
{
    assert(false && "Not implemented yet");
}

void QtInstanceButton::set_image(const css::uno::Reference<css::graphic::XGraphic>& /*rImage*/)
{
    assert(false && "Not implemented yet");
}

void QtInstanceButton::set_from_icon_name(const OUString& /*rIconName*/)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceButton::get_label() const
{
    assert(m_pButton);
    return toOUString(m_pButton->text());
}

void QtInstanceButton::set_font(const vcl::Font& /*rFont*/)
{
    assert(false && "Not implemented yet");
}

void QtInstanceButton::set_custom_button(VirtualDevice* /*pDevice*/)
{
    assert(false && "Not implemented yet");
}

void QtInstanceButton::buttonClicked() { signal_clicked(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
