/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"

#include <QtCore/QObject>
#include <QtWidgets/QPushButton>

class QtInstanceButton : public QObject, public QtInstanceWidget, public virtual weld::Button
{
    QPushButton* m_pButton;

public:
    QtInstanceButton(QPushButton* pButton);

    virtual void set_label(const OUString& rText) override;
    virtual void set_image(VirtualDevice* pDevice) override;
    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override;
    virtual void set_from_icon_name(const OUString& rIconName) override;
    virtual OUString get_label() const override;
    virtual void set_font(const vcl::Font& rFont) override;
    virtual void set_custom_button(VirtualDevice* pDevice) override;

private slots:
    void buttonClicked();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
