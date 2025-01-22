/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceLinkButton.hxx>
#include <QtInstanceLinkButton.moc>

#include <vcl/qt/QtUtils.hxx>

#include <QtGui/QDesktopServices>

QtInstanceLinkButton::QtInstanceLinkButton(QtHyperlinkLabel* pLabel)
    : QtInstanceWidget(pLabel)
    , m_pLabel(pLabel)
{
    assert(m_pLabel);

    connect(m_pLabel, &QtHyperlinkLabel::linkActivated, this, &QtInstanceLinkButton::linkActivated);
}

void QtInstanceLinkButton::set_label(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pLabel->setDisplayText(toQString(rText)); });
}

OUString QtInstanceLinkButton::get_label() const
{
    SolarMutexGuard g;
    OUString sLabel;
    GetQtInstance().RunInMainThread([&] { sLabel = toOUString(m_pLabel->displayText()); });
    return sLabel;
}

void QtInstanceLinkButton::set_label_wrap(bool bWrap)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pLabel->setWordWrap(bWrap); });
}

void QtInstanceLinkButton::set_uri(const OUString& rUri)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pLabel->setUri(toQString(rUri)); });
}

OUString QtInstanceLinkButton::get_uri() const
{
    SolarMutexGuard g;
    OUString sUri;
    GetQtInstance().RunInMainThread([&] { sUri = toOUString(m_pLabel->uri()); });
    return sUri;
}

void QtInstanceLinkButton::linkActivated(const QString& rUrl)
{
    SolarMutexGuard g;
    if (signal_activate_link())
        return;

    QDesktopServices::openUrl(rUrl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
