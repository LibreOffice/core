/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <QtWidgets/QAbstractButton>

class QtInstanceButton : public QtInstanceWidget, public virtual weld::Button
{
    QAbstractButton* m_pButton;

public:
    QtInstanceButton(QAbstractButton* pButton);

    virtual void set_label(const OUString& rText) override;
    virtual void set_image(VirtualDevice* pDevice) override;
    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override;
    virtual void set_from_icon_name(const OUString& rIconName) override;
    virtual OUString get_label() const override;
    virtual void set_font(const vcl::Font& rFont) override;
    virtual void set_custom_button(VirtualDevice* pDevice) override;

    virtual void connect_clicked(const Link<Button&, void>& rLink) override;

    // whether a custom click handler is set for the given button
    // (s.a. weld::Button::is_custom_handler_set which would give the
    // same result if called on the corresponding QtInstanceButton)
    static bool hasCustomClickHandler(const QAbstractButton& rButton);

protected:
    QAbstractButton& getButton() const;

private slots:
    void buttonClicked();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
