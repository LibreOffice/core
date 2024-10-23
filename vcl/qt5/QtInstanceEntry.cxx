/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceEntry.hxx>

#include <vcl/qt/QtUtils.hxx>

QtInstanceEntry::QtInstanceEntry(QLineEdit* pLineEdit)
    : QtInstanceWidget(pLineEdit)
    , m_pLineEdit(pLineEdit)
{
    assert(m_pLineEdit);
}

void QtInstanceEntry::set_text(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pLineEdit->setText(toQString(rText)); });
}

OUString QtInstanceEntry::get_text() const
{
    SolarMutexGuard g;
    OUString sText;
    GetQtInstance().RunInMainThread([&] { sText = toOUString(m_pLineEdit->text()); });
    return sText;
}

void QtInstanceEntry::set_width_chars(int) { assert(false && "Not implemented yet"); }

int QtInstanceEntry::get_width_chars() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceEntry::set_max_length(int nChars)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pLineEdit->setMaxLength(nChars); });
}

void QtInstanceEntry::select_region(int nStartPos, int nEndPos)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (nEndPos == -1)
            nEndPos = m_pLineEdit->text().length();

        const int nLength = nEndPos - nStartPos;
        m_pLineEdit->setSelection(nStartPos, nLength);
    });
}

bool QtInstanceEntry::get_selection_bounds(int& rStartPos, int& rEndPos)
{
    SolarMutexGuard g;

    bool bHasSelection = false;
    GetQtInstance().RunInMainThread([&] {
        bHasSelection = m_pLineEdit->hasSelectedText();
        rStartPos = m_pLineEdit->selectionStart();
        rEndPos = m_pLineEdit->selectionEnd();
    });

    return bHasSelection;
}

void QtInstanceEntry::replace_selection(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pLineEdit->insert(toQString(rText)); });
}

void QtInstanceEntry::set_position(int nCursorPos)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pLineEdit->setCursorPosition(nCursorPos); });
}

int QtInstanceEntry::get_position() const
{
    SolarMutexGuard g;
    int nCursorPos = 0;
    GetQtInstance().RunInMainThread([&] { nCursorPos = m_pLineEdit->cursorPosition(); });

    return nCursorPos;
}

void QtInstanceEntry::set_editable(bool bEditable)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pLineEdit->setReadOnly(!bEditable); });
}

bool QtInstanceEntry::get_editable() const
{
    SolarMutexGuard g;
    bool bEditable = false;
    GetQtInstance().RunInMainThread([&] { bEditable = !m_pLineEdit->isReadOnly(); });

    return bEditable;
}

void QtInstanceEntry::set_message_type(weld::EntryMessageType)
{
    assert(false && "Not implemented yet");
}

void QtInstanceEntry::set_placeholder_text(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pLineEdit->setPlaceholderText(toQString(rText)); });
}

void QtInstanceEntry::set_overwrite_mode(bool) { assert(false && "Not implemented yet"); }

bool QtInstanceEntry::get_overwrite_mode() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceEntry::set_font(const vcl::Font&) { assert(false && "Not implemented yet"); }

void QtInstanceEntry::set_font_color(const Color&) { assert(false && "Not implemented yet"); }

void QtInstanceEntry::cut_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceEntry::copy_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceEntry::paste_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceEntry::set_alignment(TxtAlign) { assert(false && "Not implemented yet"); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
