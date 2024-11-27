/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceSpinButton.hxx>

#include <vcl/qt/QtUtils.hxx>

QtInstanceSpinButton::QtInstanceSpinButton(QtDoubleSpinBox* pSpinBox)
    : QtInstanceWidget(pSpinBox)
    , m_pSpinBox(pSpinBox)
{
    assert(pSpinBox);
}

void QtInstanceSpinButton::set_text(const OUString&) { assert(false && "Not implemented yet"); }

OUString QtInstanceSpinButton::get_text() const
{
    SolarMutexGuard g;
    OUString sText;
    GetQtInstance().RunInMainThread([&] { sText = toOUString(m_pSpinBox->text()); });
    return sText;
}

void QtInstanceSpinButton::set_width_chars(int) { assert(false && "Not implemented yet"); }

int QtInstanceSpinButton::get_width_chars() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceSpinButton::set_max_length(int) { assert(false && "Not implemented yet"); }

void QtInstanceSpinButton::select_region(int nStartPos, int nEndPos)
{
    GetQtInstance().RunInMainThread([&] {
        if (nEndPos == -1)
            nEndPos = m_pSpinBox->text().length();

        const int nLength = nEndPos - nStartPos;
        m_pSpinBox->setSelection(nStartPos, nLength);
    });
}

bool QtInstanceSpinButton::get_selection_bounds(int& rStartPos, int& rEndPos)
{
    SolarMutexGuard g;

    bool bHasSelection = false;
    GetQtInstance().RunInMainThread([&] {
        bHasSelection = m_pSpinBox->hasSelectedText();
        rStartPos = m_pSpinBox->selectionStart();
        rEndPos = m_pSpinBox->selectionEnd();
    });

    return bHasSelection;
}

void QtInstanceSpinButton::replace_selection(const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceSpinButton::set_position(int nCursorPos)
{
    SolarMutexGuard g;
    if (nCursorPos == -1)
        nCursorPos = m_pSpinBox->text().length();

    GetQtInstance().RunInMainThread([&] { m_pSpinBox->setCursorPosition(nCursorPos); });
}

int QtInstanceSpinButton::get_position() const
{
    SolarMutexGuard g;
    int nCursorPos = 0;
    GetQtInstance().RunInMainThread([&] { nCursorPos = m_pSpinBox->cursorPosition(); });
    return nCursorPos;
}

void QtInstanceSpinButton::set_editable(bool) { assert(false && "Not implemented yet"); }

bool QtInstanceSpinButton::get_editable() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceSpinButton::set_message_type(weld::EntryMessageType)
{
    assert(false && "Not implemented yet");
}

void QtInstanceSpinButton::set_placeholder_text(const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceSpinButton::set_overwrite_mode(bool) { assert(false && "Not implemented yet"); }

bool QtInstanceSpinButton::get_overwrite_mode() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceSpinButton::set_font(const vcl::Font&) { assert(false && "Not implemented yet"); }

void QtInstanceSpinButton::set_font_color(const Color&) { assert(false && "Not implemented yet"); }

void QtInstanceSpinButton::cut_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceSpinButton::copy_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceSpinButton::paste_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceSpinButton::set_alignment(TxtAlign) { assert(false && "Not implemented yet"); }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
