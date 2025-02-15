/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceSpinButton.hxx>
#include <QtInstanceSpinButton.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceSpinButton::QtInstanceSpinButton(QtDoubleSpinBox* pSpinBox)
    : QtInstanceEntry(pSpinBox->lineEdit())
    , m_pSpinBox(pSpinBox)
{
    assert(pSpinBox);

    connect(m_pSpinBox, QOverload<double>::of(&QtDoubleSpinBox::valueChanged), this,
            &QtInstanceSpinButton::handleValueChanged);

    // While QtInstanceEntry generally takes care of handling signals
    // for the spinbox's QLineEdit, this doesn't work when the value
    // is changed as a result of setting a new spinbox value (e.g.
    // by using the spinbox buttons), as the QLineEdit signals are blocked
    // then, see QAbstractSpinBoxPrivate::updateEdit in qtbase:
    // https://code.qt.io/cgit/qt/qtbase.git/tree/src/widgets/widgets/qabstractspinbox.cpp?id=ced47a590aeb85953a16eaf362887f14c2815c45#n1790
    // Therefore, connect the QDoubleSpinBox::textChanged signal
    // to the slot that calls signal_changed() instead to ensure
    // it gets called nonetheless, and disconnect from the other signal.
    disconnect(pSpinBox->lineEdit(), &QLineEdit::textChanged, this, nullptr);
    connect(m_pSpinBox, &QDoubleSpinBox::textChanged, this,
            &QtInstanceSpinButton::handleTextChanged);

    // set functions to convert between value and formatted text
    m_pSpinBox->setFormatValueFunction([this](int nValue) { return format_value(nValue); });
    m_pSpinBox->setParseTextFunction(
        [this](const OUString& rText, int* pResult) { return parse_text(rText, pResult); });
}

QWidget* QtInstanceSpinButton::getQWidget() const { return m_pSpinBox; }

void QtInstanceSpinButton::set_value(sal_Int64 nValue)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { (m_pSpinBox->setValue(nValue)); });
}

sal_Int64 QtInstanceSpinButton::get_value() const
{
    SolarMutexGuard g;

    sal_Int64 nValue;
    GetQtInstance().RunInMainThread([&] { nValue = std::round(m_pSpinBox->value()); });
    return nValue;
}

void QtInstanceSpinButton::set_range(sal_Int64 nMin, sal_Int64 nMax)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { (m_pSpinBox->setRange(nMin, nMax)); });
}

void QtInstanceSpinButton::get_range(sal_Int64& rMin, sal_Int64& rMax) const
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        rMin = std::round(m_pSpinBox->minimum());
        rMax = std::round(m_pSpinBox->maximum());
    });
}

void QtInstanceSpinButton::set_increments(sal_Int64 nStep, sal_Int64)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pSpinBox->setSingleStep(nStep); });
}

void QtInstanceSpinButton::get_increments(sal_Int64& rStep, sal_Int64& rPage) const
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        rStep = std::round(m_pSpinBox->singleStep());
        rPage = rStep;
    });
}

void QtInstanceSpinButton::set_digits(unsigned int nDigits)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { m_pSpinBox->setDecimals(nDigits); });
}

unsigned int QtInstanceSpinButton::get_digits() const
{
    SolarMutexGuard g;

    unsigned int nDigits = 0;
    GetQtInstance().RunInMainThread([&] { nDigits = o3tl::make_unsigned(m_pSpinBox->decimals()); });
    return nDigits;
}

void QtInstanceSpinButton::handleValueChanged()
{
    SolarMutexGuard aGuard;
    signal_value_changed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
