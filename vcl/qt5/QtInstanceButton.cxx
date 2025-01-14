/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceButton.hxx>

#include <vcl/qt/QtUtils.hxx>

// Name of QObject property to indicate whether a click handler
// was set on the button: If that property is set and has a value
// of true, then a custom click handler is set, otherwise not.
const char* const PROPERTY_CLICK_HANDLER_SET = "click-handler-set";

QtInstanceButton::QtInstanceButton(QAbstractButton* pButton)
    : QtInstanceWidget(pButton)
    , m_pButton(pButton)
{
    assert(m_pButton);

    connect(m_pButton, &QAbstractButton::clicked, this, &QtInstanceButton::buttonClicked);
}

void QtInstanceButton::set_label(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pButton->setText(vclToQtStringWithAccelerator(rText)); });
}

void QtInstanceButton::set_image(VirtualDevice* pDevice)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        if (pDevice)
            m_pButton->setIcon(toQPixmap(*pDevice));
    });
}

void QtInstanceButton::set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pButton->setIcon(toQPixmap(rImage)); });
}

void QtInstanceButton::set_from_icon_name(const OUString& rIconName)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] {
        QPixmap aIcon = loadQPixmapIcon(rIconName);
        m_pButton->setIcon(aIcon);
    });
}

OUString QtInstanceButton::get_label() const
{
    SolarMutexGuard g;
    QtInstance& rQtInstance = GetQtInstance();
    if (!rQtInstance.IsMainThread())
    {
        OUString sLabel;
        rQtInstance.RunInMainThread([&] { sLabel = get_label(); });
        return sLabel;
    }

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

void QtInstanceButton::connect_clicked(const Link<Button&, void>& rLink)
{
    weld::Button::connect_clicked(rLink);
    m_pButton->setProperty(PROPERTY_CLICK_HANDLER_SET, QVariant::fromValue(rLink.IsSet()));
}

bool QtInstanceButton::hasCustomClickHandler(QAbstractButton& rButton)
{
    QVariant aProp = rButton.property(PROPERTY_CLICK_HANDLER_SET);
    if (!aProp.isValid())
        return false;

    assert(aProp.canConvert<bool>());
    return aProp.toBool();
}

QAbstractButton& QtInstanceButton::getButton() const
{
    assert(m_pButton);
    return *m_pButton;
}

void QtInstanceButton::buttonClicked()
{
    SolarMutexGuard g;
    signal_clicked();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
