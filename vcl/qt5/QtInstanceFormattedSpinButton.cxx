/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceFormattedSpinButton.hxx>
#include <QtInstanceFormattedSpinButton.moc>

QtInstanceFormattedSpinButton::QtInstanceFormattedSpinButton(QtDoubleSpinBox* pSpinBox)
    : QtInstanceEntry(pSpinBox->lineEdit())
    , m_pSpinBox(pSpinBox)
    , m_pFormatter(nullptr)
{
    assert(pSpinBox);

    connect(m_pSpinBox, QOverload<double>::of(&QtDoubleSpinBox::valueChanged), this,
            &QtInstanceFormattedSpinButton::handleValueChanged);

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
            &QtInstanceFormattedSpinButton::handleTextChanged);

    // set function to convert value to text
    m_pSpinBox->setFormatValueFunction(
        [this](double fValue) { return GetFormatter().FormatValue(fValue); });
}

QWidget* QtInstanceFormattedSpinButton::getQWidget() const { return m_pSpinBox; }

Formatter& QtInstanceFormattedSpinButton::GetFormatter()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (!m_pFormatter)
        {
            auto aFocusOutHdl = m_aFocusOutHdl;
            m_aFocusOutHdl = Link<weld::Widget&, void>();
            auto aChangeHdl = m_aChangeHdl;
            m_aChangeHdl = Link<weld::Entry&, void>();

            m_xOwnFormatter = std::make_unique<weld::EntryFormatter>(*this);
            m_xOwnFormatter->SetMinValue(m_pSpinBox->minimum());
            m_xOwnFormatter->SetMaxValue(m_pSpinBox->maximum());
            m_xOwnFormatter->SetSpinSize(m_pSpinBox->singleStep());
            m_xOwnFormatter->SetValue(m_pSpinBox->value());

            m_xOwnFormatter->connect_focus_out(aFocusOutHdl);
            m_xOwnFormatter->connect_changed(aChangeHdl);

            m_pFormatter = m_xOwnFormatter.get();
        }
    });

    return *m_pFormatter;
}

void QtInstanceFormattedSpinButton::SetFormatter(weld::EntryFormatter* pFormatter)
{
    m_xOwnFormatter.reset();
    m_pFormatter = pFormatter;
    sync_range_from_formatter();
    sync_value_from_formatter();
    sync_increments_from_formatter();
}

QtInstanceFormattedSpinButton::~QtInstanceFormattedSpinButton()
{
    m_pFormatter = nullptr;
    m_xOwnFormatter.reset();
}

void QtInstanceFormattedSpinButton::sync_range_from_formatter()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (!m_pFormatter)
            return;

        const double fMin = m_pFormatter->HasMinValue() ? m_pFormatter->GetMinValue()
                                                        : std::numeric_limits<double>::lowest();
        const double fMax = m_pFormatter->HasMaxValue() ? m_pFormatter->GetMaxValue()
                                                        : std::numeric_limits<double>::max();
        m_pSpinBox->setRange(fMin, fMax);
    });
}

void QtInstanceFormattedSpinButton::sync_value_from_formatter()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (m_pFormatter)
            m_pSpinBox->setValue(m_pFormatter->GetValue());
    });
}

void QtInstanceFormattedSpinButton::sync_increments_from_formatter()
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (m_pFormatter)
            m_pSpinBox->setSingleStep(m_pFormatter->GetSpinSize());
    });
}

void QtInstanceFormattedSpinButton::handleValueChanged()
{
    SolarMutexGuard aGuard;
    signal_value_changed();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
