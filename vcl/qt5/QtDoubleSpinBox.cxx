/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtDoubleSpinBox.hxx>
#include <QtDoubleSpinBox.moc>

QtDoubleSpinBox::QtDoubleSpinBox(QWidget* pParent)
    : QDoubleSpinBox(pParent)
{
    // don't notify about value/text changes while typing
    setKeyboardTracking(false);
}

QLineEdit* QtDoubleSpinBox::lineEdit() const { return QDoubleSpinBox::lineEdit(); }

QString QtDoubleSpinBox::textFromValue(double fValue) const
{
    if (m_aFormatValueFunction)
    {
        std::optional<QString> aText = m_aFormatValueFunction(fValue);
        if (aText.has_value())
            return aText.value();
    }

    return QDoubleSpinBox::textFromValue(fValue);
}

double QtDoubleSpinBox::valueFromText(const QString& rText) const
{
    if (m_aParseTextFunction)
    {
        double fValue = 0;
        const TriState eState = m_aParseTextFunction(rText, &fValue);
        if (eState == TRISTATE_TRUE)
            return fValue;
    }

    return QDoubleSpinBox::valueFromText(rText);
}

QValidator::State QtDoubleSpinBox::validate(QString& rInput, int& rPos) const
{
    if (m_aParseTextFunction)
    {
        double fValue = 0;
        const TriState eState = m_aParseTextFunction(rInput, &fValue);
        switch (eState)
        {
            case TRISTATE_TRUE:
                return QValidator::Acceptable;
            case TRISTATE_FALSE:
                return QValidator::Intermediate;
            case TRISTATE_INDET:
                // no parser function set, use default QDoubleSpinBox logic
                break;
        }
    }

    return QDoubleSpinBox::validate(rInput, rPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
