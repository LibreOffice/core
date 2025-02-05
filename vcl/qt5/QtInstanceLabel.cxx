/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceLabel.hxx>
#include <QtInstanceLabel.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceLabel::QtInstanceLabel(QLabel* pLabel)
    : QtInstanceWidget(pLabel)
    , m_pLabel(pLabel)
{
    assert(m_pLabel);
}

void QtInstanceLabel::set_label(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pLabel->setText(vclToQtStringWithAccelerator(rText)); });
}

OUString QtInstanceLabel::get_label() const
{
    SolarMutexGuard g;
    OUString sLabel;
    GetQtInstance().RunInMainThread([&] { sLabel = toOUString(m_pLabel->text()); });

    return sLabel;
}

void QtInstanceLabel::set_mnemonic_widget(Widget* pTarget)
{
    SolarMutexGuard g;

    QtInstanceWidget* pTargetWidget = dynamic_cast<QtInstanceWidget*>(pTarget);
    if (!pTargetWidget)
        return;

    GetQtInstance().RunInMainThread([&] { m_pLabel->setBuddy(pTargetWidget->getQWidget()); });
}

void QtInstanceLabel::set_font(const vcl::Font&) { assert(false && "Not implemented yet"); }

void QtInstanceLabel::set_label_type(weld::LabelType) { assert(false && "Not implemented yet"); }

void QtInstanceLabel::set_font_color(const Color& rColor) { setFontColor(rColor); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
